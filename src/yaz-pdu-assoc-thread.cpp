/*
 * Copyright (c) 1998-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-pdu-assoc-thread.cpp,v $
 * Revision 1.2  2001-03-27 14:47:45  adam
 * New server facility scheme.
 *
 * Revision 1.1  2001/03/26 14:43:49  adam
 * New threaded PDU association.
 *
 */

#ifdef WIN32
#include <process.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif


#include <errno.h>
#include <yaz/log.h>
#include <yaz/tcpip.h>

#include <yaz++/yaz-pdu-assoc.h>
#include <yaz++/yaz-socket-manager.h>



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
    
    logf (LOG_LOG, "thread started");
    while (s->processEvent() > 0)
	;
    logf (LOG_LOG, "thread finished");
#ifdef WIN32
#else
    return 0;
#endif
}

void Yaz_PDU_AssocThread::childNotify(COMSTACK cs)
{
    Yaz_SocketManager *socket_observable = new Yaz_SocketManager;
    Yaz_PDU_Assoc *new_observable = new Yaz_PDU_Assoc (socket_observable, cs);
    
    /// Clone PDU Observer
    new_observable->m_PDU_Observer =
	m_PDU_Observer->sessionNotify(new_observable, cs_fileno(cs));
#ifdef WIN32
    long t_id;
    t_id = _beginthread (events, 0, socket_observable);
    if (t_id == -1)
    {
        yaz_log (LOG_FATAL|LOG_ERRNO, "_beginthread failed");
        exit (1);
    }
#else
    pthread_t tid;

    int id = pthread_create (&tid, 0, events, socket_observable);
    if (id)
	yaz_log (LOG_ERRNO|LOG_FATAL, "pthread_create returned id=%d", id);
    else
	pthread_detach (tid);
#endif
}
