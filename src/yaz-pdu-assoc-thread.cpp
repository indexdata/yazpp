/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef WIN32
#define USE_THREADS 1
#endif

#if YAZ_POSIX_THREADS
#define USE_THREADS 1
#endif

#if USE_THREADS

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef WIN32
#include <process.h>
#else
#include <pthread.h>
#endif


#include <errno.h>
#include <yaz/log.h>
#include <yaz/tcpip.h>

#include <yazpp/pdu-assoc.h>
#include <yazpp/socket-manager.h>

using namespace yazpp_1;

class worker {
public:
    SocketManager *m_mgr;
    PDU_Assoc *m_assoc;
    void run();
};

PDU_AssocThread::PDU_AssocThread(
    ISocketObservable *socketObservable)
    : PDU_Assoc(socketObservable)
{

}

PDU_AssocThread::~PDU_AssocThread()
{

}

void worker::run()
{
    yaz_log (YLOG_LOG, "thread started");
    while (this->m_mgr->processEvent() > 0)
        ;
    yaz_log (YLOG_LOG, "thread finished");
    delete this->m_mgr;
    delete this;
}

#ifdef WIN32
void __cdecl
#else
void *
#endif
events(void *p)
{
    worker *w = (worker *) p;
    w->run();
#ifdef WIN32
#else
    return 0;
#endif
}

void PDU_AssocThread::childNotify(COMSTACK cs)
{
    SocketManager *socket_observable = new SocketManager;
    PDU_Assoc *new_observable = new PDU_Assoc (socket_observable, cs);

    /// Clone PDU Observer
    new_observable->m_PDU_Observer =
        m_PDU_Observer->sessionNotify(new_observable, cs_fileno(cs));

    if (!new_observable->m_PDU_Observer)
    {
        new_observable->shutdown();
        delete new_observable;
        delete socket_observable;
        return;
    }

    worker *w = new worker;
    w->m_assoc = new_observable;
    w->m_mgr = socket_observable;

#ifdef WIN32
    long t_id;
    t_id = _beginthread (events, 0, w);
    if (t_id == -1)
    {
        yaz_log (YLOG_FATAL|YLOG_ERRNO, "_beginthread failed");
        exit (1);
    }
#else
    pthread_t tid;

    int id = pthread_create (&tid, 0, events, w);
    if (id)
        yaz_log (YLOG_ERRNO|YLOG_FATAL, "pthread_create returned id=%d", id);
    else
        pthread_detach (tid);
#endif
}
#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

