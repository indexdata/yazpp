/*
 * Copyright (c) 1998-2004, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-pdu-assoc-thread.cpp,v 1.10 2005-06-02 06:40:21 adam Exp $
 */

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

#include <yaz++/pdu-assoc.h>
#include <yaz++/socket-manager.h>

using namespace yazpp_1;

Yaz_PDU_AssocThread::Yaz_PDU_AssocThread(
    IYazSocketObservable *socketObservable)
    : Yaz_PDU_Assoc(socketObservable)
{
    
}

#ifdef WIN32
void __cdecl
#else
void *
#endif 
events(void *p)
{
    Yaz_SocketManager *s = (Yaz_SocketManager *) p;
    
    yaz_log (YLOG_LOG, "thread started");
    while (s->processEvent() > 0)
	;
    yaz_log (YLOG_LOG, "thread finished");
#ifdef WIN32
#else
    return 0;
#endif
}

void Yaz_PDU_AssocThread::childNotify(COMSTACK cs)
{
    Yaz_SocketManager *socket_observable = new Yaz_SocketManager;
    Yaz_PDU_Assoc *new_observable = new Yaz_PDU_Assoc (socket_observable, cs);

    new_observable->m_next = m_children;
    m_children = new_observable;
    new_observable->m_parent = this;

    /// Clone PDU Observer
    new_observable->m_PDU_Observer =
	m_PDU_Observer->sessionNotify(new_observable, cs_fileno(cs));
#ifdef WIN32
    long t_id;
    t_id = _beginthread (events, 0, socket_observable);
    if (t_id == -1)
    {
        yaz_log (YLOG_FATAL|YLOG_ERRNO, "_beginthread failed");
        exit (1);
    }
#else
    pthread_t tid;

    int id = pthread_create (&tid, 0, events, socket_observable);
    if (id)
	yaz_log (YLOG_ERRNO|YLOG_FATAL, "pthread_create returned id=%d", id);
    else
	pthread_detach (tid);
#endif
}
#endif
