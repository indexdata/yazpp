/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <assert.h>
#include <string.h>
#include <yaz/log.h>
#include <yaz/tcpip.h>

#include <yazpp/pdu-assoc.h>

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

using namespace yazpp_1;

namespace yazpp_1 {
    class PDU_Assoc_priv {
        friend class PDU_Assoc;
    private:
        enum {
            Connecting,
            Listen,
            Ready,
            Closed,
            Writing,
            Accepting
        } state;
        class PDU_Queue {
        public:
            PDU_Queue(const char *buf, int len);
            ~PDU_Queue();
            char *m_buf;
            int m_len;
            PDU_Queue *m_next;
        };
        PDU_Assoc *pdu_parent;
        PDU_Assoc *pdu_children;
        PDU_Assoc *pdu_next;
        COMSTACK cs;
        yazpp_1::ISocketObservable *m_socketObservable;
        char *input_buf;
        int input_len;
        PDU_Queue *queue_out;
        PDU_Queue *queue_in;
        int *destroyed;
        int idleTime;
        int log;
        void init(yazpp_1::ISocketObservable *socketObservable);
        COMSTACK comstack(const char *type_and_host, void **vp);
        bool m_session_is_dead;
        char *cert_fname;
    };
}

void PDU_Assoc_priv::init(ISocketObservable *socketObservable)
{
    state = Closed;
    cs = 0;
    m_socketObservable = socketObservable;
    queue_out = 0;
    queue_in = 0;
    input_buf = 0;
    input_len = 0;
    pdu_children = 0;
    pdu_parent = 0;
    pdu_next = 0;
    destroyed = 0;
    idleTime = 0;
    log = YLOG_DEBUG;
    m_session_is_dead = false;
    cert_fname = 0;
}

PDU_Assoc::~PDU_Assoc()
{
    xfree(m_p->cert_fname);
    delete m_p;
}

PDU_Assoc::PDU_Assoc(ISocketObservable *socketObservable)
{
    m_PDU_Observer = 0;
    m_p = new PDU_Assoc_priv;
    m_p->init(socketObservable);
}

PDU_Assoc::PDU_Assoc(ISocketObservable *socketObservable,
                     COMSTACK cs)
{
    m_PDU_Observer = 0;
    m_p = new PDU_Assoc_priv;
    m_p->init(socketObservable);
    m_p->cs = cs;
    unsigned mask = 0;
    if (cs->io_pending & CS_WANT_WRITE)
        mask |= SOCKET_OBSERVE_WRITE;
    if (cs->io_pending & CS_WANT_READ)
        mask |= SOCKET_OBSERVE_READ;
    m_p->m_socketObservable->addObserver(cs_fileno(cs), this);
    if (!mask)
    {
        yaz_log(m_p->log, "new PDU_Assoc. Ready");
        m_p->state = PDU_Assoc_priv::Ready;
        flush_PDU();
    }
    else
    {
        yaz_log(m_p->log, "new PDU_Assoc. Accepting");
        // assume comstack is accepting...
        m_p->state = PDU_Assoc_priv::Accepting;
        m_p->m_socketObservable->addObserver(cs_fileno(cs), this);
        yaz_log(m_p->log, "maskObserver 1");
        m_p->m_socketObservable->maskObserver(this,
                                              mask|SOCKET_OBSERVE_EXCEPT);
    }
}


IPDU_Observable *PDU_Assoc::clone()
{
    PDU_Assoc *copy = new PDU_Assoc(m_p->m_socketObservable);
    return copy;
}

void PDU_Assoc::socketNotify(int event)
{
    yaz_log(m_p->log, "PDU_Assoc::socketNotify p=%p state=%d event = %d",
            this, m_p->state, event);
    if (event & SOCKET_OBSERVE_EXCEPT)
    {
        shutdown();
        m_PDU_Observer->failNotify();
        return;
    }
    else if (event & SOCKET_OBSERVE_TIMEOUT)
    {
        m_PDU_Observer->timeoutNotify();
        return;
    }
    switch (m_p->state)
    {
    case PDU_Assoc_priv::Accepting:
        if (!cs_accept(m_p->cs))
        {
            yaz_log(m_p->log, "PDU_Assoc::cs_accept failed");
            m_p->cs = 0;
            shutdown();
            m_PDU_Observer->failNotify();
        }
        else
        {
            unsigned mask = 0;
            if (m_p->cs->io_pending & CS_WANT_WRITE)
                mask |= SOCKET_OBSERVE_WRITE;
            if (m_p->cs->io_pending & CS_WANT_READ)
                mask |= SOCKET_OBSERVE_READ;
            if (!mask)
            {   // accept is complete. turn to ready state and write if needed
                m_p->state = PDU_Assoc_priv::Ready;
                flush_PDU();
            }
            else
            {   // accept still incomplete.
                yaz_log(m_p->log, "maskObserver 2");
                m_p->m_socketObservable->maskObserver(this,
                                             mask|SOCKET_OBSERVE_EXCEPT);
            }
        }
        break;
    case PDU_Assoc_priv::Connecting:
        if (event & SOCKET_OBSERVE_READ &&
            event & SOCKET_OBSERVE_WRITE)
        {
            // For Unix: if both read and write is set, then connect failed.
            shutdown();
            m_PDU_Observer->failNotify();
        }
        else
        {
            yaz_log(m_p->log, "cs_rcvconnect");
            int res = cs_rcvconnect(m_p->cs);
            if (res == 1)
            {
                unsigned mask = SOCKET_OBSERVE_EXCEPT;
                if (m_p->cs->io_pending & CS_WANT_WRITE)
                    mask |= SOCKET_OBSERVE_WRITE;
                if (m_p->cs->io_pending & CS_WANT_READ)
                    mask |= SOCKET_OBSERVE_READ;
                yaz_log(m_p->log, "maskObserver 3");
                m_p->m_socketObservable->maskObserver(this, mask);
            }
            else
            {
                m_p->state = PDU_Assoc_priv::Ready;
                if (m_PDU_Observer)
                    m_PDU_Observer->connectNotify();
                flush_PDU();
            }
        }
        break;
    case PDU_Assoc_priv::Listen:
        if (event & SOCKET_OBSERVE_READ)
        {
            int res;
            COMSTACK new_line;

            if ((res = cs_listen(m_p->cs, 0, 0)) == 1)
                return;
            if (res < 0)
            {
                yaz_log(YLOG_FATAL|YLOG_ERRNO, "cs_listen failed");
                return;
            }
            if (!(new_line = cs_accept(m_p->cs)))
                return;
            /* 1. create socket-manager
               2. create pdu-assoc
               3. create top-level object
                    setup observer for child fileid in pdu-assoc
               4. start thread
            */
            yaz_log(m_p->log, "new session: parent fd=%d child fd=%d",
                    cs_fileno(m_p->cs), cs_fileno(new_line));
            childNotify(new_line);
        }
        break;
    case PDU_Assoc_priv::Writing:
        if (event & (SOCKET_OBSERVE_READ|SOCKET_OBSERVE_WRITE))
            flush_PDU();
        break;
    case PDU_Assoc_priv::Ready:
        if (event & (SOCKET_OBSERVE_READ|SOCKET_OBSERVE_WRITE))
        {
            do
            {
                int res = cs_get(m_p->cs, &m_p->input_buf, &m_p->input_len);
                if (res == 1)
                {
                    unsigned mask = SOCKET_OBSERVE_EXCEPT;
                    if (m_p->cs->io_pending & CS_WANT_WRITE)
                        mask |= SOCKET_OBSERVE_WRITE;
                    if (m_p->cs->io_pending & CS_WANT_READ)
                        mask |= SOCKET_OBSERVE_READ;
                    yaz_log(m_p->log, "maskObserver 4");
                    m_p->m_socketObservable->maskObserver(this, mask);
                    return;
                }
                else if (res <= 0)
                {
                    yaz_log(m_p->log, "PDU_Assoc::Connection closed by peer");
                    shutdown();
                    if (m_PDU_Observer)
                        m_PDU_Observer->failNotify(); // problem here..
                    return;
                }
                // lock it, so we know if recv_PDU deletes it.
                int destroyed = 0;
                m_p->destroyed = &destroyed;

                if (!m_PDU_Observer)
                    return;
#if 0
                PDU_Assoc_priv::PDU_Queue **pq = &m_p->m_queue_in;
                while (*pq)
                    pq = &(*pq)->m_next;

                *pq = new PDU_Assoc_priv::PDU_Queue(m_p->m_input_buf, res);
#else
                m_PDU_Observer->recv_PDU(m_p->input_buf, res);
#endif
                if (destroyed)   // it really was destroyed, return now.
                    return;
                m_p->destroyed = 0;
            } while (m_p->cs && cs_more(m_p->cs));
            if (m_p->cs && m_p->state == PDU_Assoc_priv::Ready)
            {
                yaz_log(m_p->log, "maskObserver 5");
                m_p->m_socketObservable->maskObserver(this,
                                                      SOCKET_OBSERVE_EXCEPT|
                                                      SOCKET_OBSERVE_READ);
            }
        }
        break;
    case PDU_Assoc_priv::Closed:
        yaz_log(m_p->log, "CLOSING state=%d event was %d", m_p->state,
                event);
        shutdown();
        m_PDU_Observer->failNotify();
        break;
    default:
        yaz_log(m_p->log, "Unknown state=%d event was %d", m_p->state, event);
        shutdown();
        m_PDU_Observer->failNotify();
    }
}

void PDU_Assoc::close_session()
{
    m_p->m_session_is_dead = true;
    if (!m_p->queue_out)
    {
        shutdown();
        m_PDU_Observer->failNotify();
    }
}

void PDU_Assoc::shutdown()
{
    PDU_Assoc *ch;
    for (ch = m_p->pdu_children; ch; ch = ch->m_p->pdu_next)
        ch->shutdown();

    m_p->m_socketObservable->deleteObserver(this);
    m_p->state = PDU_Assoc_priv::Closed;
    if (m_p->cs)
    {
        yaz_log(m_p->log, "PDU_Assoc::close fd=%d", cs_fileno(m_p->cs));
        cs_close(m_p->cs);
    }
    m_p->cs = 0;
    while (m_p->queue_out)
    {
        PDU_Assoc_priv::PDU_Queue *q_this = m_p->queue_out;
        m_p->queue_out = m_p->queue_out->m_next;
        delete q_this;
    }
    xfree(m_p->input_buf);
    m_p->input_buf = 0;
    m_p->input_len = 0;
}

void PDU_Assoc::destroy()
{
    shutdown();

    if (m_p->destroyed)
        *m_p->destroyed = 1;
    PDU_Assoc **c;

    // delete from parent's child list (if any)
    if (m_p->pdu_parent)
    {
        c = &m_p->pdu_parent->m_p->pdu_children;
        while (*c != this)
        {
            assert (*c);
            c = &(*c)->m_p->pdu_next;
        }
        *c = (*c)->m_p->pdu_next;
    }
    // delete all children ...
    c = &m_p->pdu_children;
    while (*c)
    {
        PDU_Assoc *here = *c;
        *c = (*c)->m_p->pdu_next;
        here->m_p->pdu_parent = 0;
        delete here;
    }
    yaz_log(m_p->log, "PDU_Assoc::destroy this=%p", this);
}

PDU_Assoc_priv::PDU_Queue::PDU_Queue(const char *buf, int len)
{
    m_buf = (char *) xmalloc(len);
    memcpy(m_buf, buf, len);
    m_len = len;
    m_next = 0;
}

PDU_Assoc_priv::PDU_Queue::~PDU_Queue()
{
    xfree(m_buf);
}

int PDU_Assoc::flush_PDU()
{
    int r;

    if (m_p->state != PDU_Assoc_priv::Ready && m_p->state != PDU_Assoc_priv::Writing)
    {
        yaz_log(m_p->log, "YAZ_PDU_Assoc::flush_PDU, not ready");
        return 1;
    }
    PDU_Assoc_priv::PDU_Queue *q = m_p->queue_out;
    if (!q)
    {
        m_p->state = PDU_Assoc_priv::Ready;
        yaz_log(m_p->log, "YAZ_PDU_Assoc::flush_PDU queue empty");
        yaz_log(m_p->log, "maskObserver 6");
        m_p->m_socketObservable->maskObserver(this, SOCKET_OBSERVE_READ|
                                              SOCKET_OBSERVE_WRITE|
                                              SOCKET_OBSERVE_EXCEPT);
        if (m_p->m_session_is_dead)
        {
            shutdown();
            m_PDU_Observer->failNotify();
        }
        return 0;
    }
    r = cs_put(m_p->cs, q->m_buf, q->m_len);
    if (r < 0)
    {
        yaz_log(m_p->log, "PDU_Assoc::flush_PDU cs_put failed");
        shutdown();
        m_PDU_Observer->failNotify();
        return r;
    }
    if (r == 1)
    {
        unsigned mask = SOCKET_OBSERVE_EXCEPT;
        m_p->state = PDU_Assoc_priv::Writing;
        if (m_p->cs->io_pending & CS_WANT_WRITE)
            mask |= SOCKET_OBSERVE_WRITE;
        if (m_p->cs->io_pending & CS_WANT_READ)
            mask |= SOCKET_OBSERVE_READ;

        mask |= SOCKET_OBSERVE_WRITE;
        yaz_log(m_p->log, "maskObserver 7");
        m_p->m_socketObservable->maskObserver(this, mask);
        yaz_log(m_p->log, "PDU_Assoc::flush_PDU cs_put %d bytes fd=%d (inc)",
                q->m_len, cs_fileno(m_p->cs));
        return r;
    }
    yaz_log(m_p->log, "PDU_Assoc::flush_PDU cs_put %d bytes", q->m_len);
    // whole packet sent... delete this and proceed to next ...
    m_p->queue_out = q->m_next;
    delete q;
    // don't select on write if queue is empty ...
    if (!m_p->queue_out)
    {
        m_p->state = PDU_Assoc_priv::Ready;
        yaz_log(m_p->log, "maskObserver 8");
        m_p->m_socketObservable->maskObserver(this, SOCKET_OBSERVE_READ|
                                              SOCKET_OBSERVE_EXCEPT);
        if (m_p->m_session_is_dead)
            shutdown();
    }
    return r;
}

int PDU_Assoc::send_PDU(const char *buf, int len)
{
    yaz_log(m_p->log, "PDU_Assoc::send_PDU");
    PDU_Assoc_priv::PDU_Queue **pq = &m_p->queue_out;
    int is_idle = (*pq ? 0 : 1);

    if (!m_p->cs)
    {
        yaz_log(m_p->log, "PDU_Assoc::send_PDU failed, cs == 0");
        return -1;
    }
    while (*pq)
        pq = &(*pq)->m_next;
    *pq = new PDU_Assoc_priv::PDU_Queue(buf, len);
    if (is_idle)
        return flush_PDU();
    else
        yaz_log(m_p->log, "PDU_Assoc::cannot send_PDU fd=%d",
                cs_fileno(m_p->cs));
    return 0;
}

COMSTACK PDU_Assoc_priv::comstack(const char *type_and_host, void **vp)
{
    return cs_create_host(type_and_host, 2, vp);
}

int PDU_Assoc::listen(IPDU_Observer *observer, const char *addr)
{
    if (*addr == '\0')
    {
        m_p->m_socketObservable->deleteObserver(this);
        m_p->state = PDU_Assoc_priv::Closed;
        if (m_p->cs)
        {
            yaz_log(m_p->log, "PDU_Assoc::close fd=%d", cs_fileno(m_p->cs));
            cs_close(m_p->cs);
        }
        m_p->cs = 0;
        while (m_p->queue_out)
        {
            PDU_Assoc_priv::PDU_Queue *q_this = m_p->queue_out;
            m_p->queue_out = m_p->queue_out->m_next;
            delete q_this;
        }
        xfree(m_p->input_buf);
        m_p->input_buf = 0;
        m_p->input_len = 0;

        return 0;
    }

    shutdown();

    m_PDU_Observer = observer;
    void *ap;
    m_p->cs = m_p->comstack(addr, &ap);

    if (!m_p->cs)
        return -1;

    if (m_p->cert_fname)
        cs_set_ssl_certificate_file(m_p->cs, m_p->cert_fname);

    if (cs_bind(m_p->cs, ap, CS_SERVER) < 0)
        return -2;

    int fd = cs_fileno(m_p->cs);
#if HAVE_FCNTL_H
    int oldflags = fcntl(fd, F_GETFD, 0);
    if (oldflags >= 0)
    {
        oldflags |= FD_CLOEXEC;
        fcntl(fd, F_SETFD, oldflags);
    }
#endif
    m_p->m_socketObservable->addObserver(fd, this);
    yaz_log(m_p->log, "maskObserver 9");
    m_p->m_socketObservable->maskObserver(this, SOCKET_OBSERVE_READ|
                                          SOCKET_OBSERVE_EXCEPT);
    yaz_log(m_p->log, "PDU_Assoc::listen ok fd=%d", fd);
    m_p->state = PDU_Assoc_priv::Listen;
    return 0;
}

COMSTACK PDU_Assoc::get_comstack()
{
    return m_p->cs;
}

void PDU_Assoc::idleTime(int idleTime)
{
    m_p->idleTime = idleTime;
    yaz_log(m_p->log, "PDU_Assoc::idleTime(%d)", idleTime);
    m_p->m_socketObservable->timeoutObserver(this, m_p->idleTime);
}

int PDU_Assoc::connect(IPDU_Observer *observer, const char *addr)
{
    yaz_log(m_p->log, "PDU_Assoc::connect %s", addr);
    shutdown();
    m_PDU_Observer = observer;
    void *ap;
    m_p->cs = m_p->comstack(addr, &ap);
    if (!m_p->cs)
        return -1;
    int res = cs_connect(m_p->cs, ap);
    yaz_log(m_p->log, "PDU_Assoc::connect fd=%d res=%d", cs_fileno(m_p->cs),
            res);
    m_p->m_socketObservable->addObserver(cs_fileno(m_p->cs), this);

    if (res == 0)
    {   // Connect complete
        m_p->state = PDU_Assoc_priv::Connecting;
        unsigned mask = SOCKET_OBSERVE_EXCEPT;
        mask |= SOCKET_OBSERVE_WRITE;
        mask |= SOCKET_OBSERVE_READ;
        yaz_log(m_p->log, "maskObserver 11");
        m_p->m_socketObservable->maskObserver(this, mask);
    }
    else if (res > 0)
    {   // Connect pending
        m_p->state = PDU_Assoc_priv::Connecting;
        unsigned mask = SOCKET_OBSERVE_EXCEPT;
        if (m_p->cs->io_pending & CS_WANT_WRITE)
            mask |= SOCKET_OBSERVE_WRITE;
        if (m_p->cs->io_pending & CS_WANT_READ)
            mask |= SOCKET_OBSERVE_READ;
        yaz_log(m_p->log, "maskObserver 11");
        m_p->m_socketObservable->maskObserver(this, mask);
    }
    else
    {   // Connect failed immediately
        // Since m_state is Closed we can distinguish this case from
        // normal connect in socketNotify handler
        yaz_log(m_p->log, "maskObserver 12");
        m_p->m_socketObservable->maskObserver(this, SOCKET_OBSERVE_WRITE|
                                              SOCKET_OBSERVE_EXCEPT);
    }
    return 0;
}

// Single-threaded... Only useful for non-blocking handlers
void PDU_Assoc::childNotify(COMSTACK cs)
{
    PDU_Assoc *new_observable =
        new PDU_Assoc(m_p->m_socketObservable, cs);

    // Clone PDU Observer
    new_observable->m_PDU_Observer = m_PDU_Observer->sessionNotify
        (new_observable, cs_fileno(cs));

    if (!new_observable->m_PDU_Observer)
    {
        new_observable->shutdown();
        delete new_observable;
        return;
    }
    new_observable->m_p->pdu_next = m_p->pdu_children;
    m_p->pdu_children = new_observable;
    new_observable->m_p->pdu_parent = this;
}

const char*PDU_Assoc::getpeername()
{
    if (!m_p->cs)
        return 0;
    return cs_addrstr(m_p->cs);
}

void PDU_Assoc::set_cert_fname(const char *fname)
{
    xfree(m_p->cert_fname);
    m_p->cert_fname = 0;
    if (fname)
        m_p->cert_fname = xstrdup(fname);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

