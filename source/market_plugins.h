/**
 * Simple_Plugins.h
 *
 * Platform-specific definitions for the sample IAF plugins.
 *
 * $Copyright(c) 2007-2009 Progress Software Corporation (PSC). All rights reserved.$
 *
 * $Id: $
 */

#ifndef PROTOCOL_PLUGINS_H
#define PROTOCOL_PLUGINS_H


#include <errno.h>
#include <AP_Logger.h>

#if defined(_APBUILD_WIN32_ALL__) || defined(__WIN32__) || defined (WIN32)

#ifndef _APBUILD_WIN32_ALL__
#define _APBUILD_WIN32_ALL__
#endif

#ifndef _AMD64_
#define _X86_
#endif
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <process.h>
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/timeb.h>


#define ssize_t int
#define strcasecmp _stricmp
#define WRITE_PMODE _S_IWRITE

#define MUTEX_LOCK(m) EnterCriticalSection(m)
#define MUTEX_UNLOCK(m) LeaveCriticalSection(m)
#define MUTEX_INIT(m) InitializeCriticalSection(m)
#define MUTEX_DESTROY(m) DeleteCriticalSection(m)
#define MUTEX_T CRITICAL_SECTION

#define SEMA_WAIT(s) WaitForSingleObject(s, INFINITE)
#define SEMA_POST(s) ReleaseSemaphore(s, 1, NULL)
#define SEMA_INIT(s) s = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL)
#define SEMA_DESTROY(s) CloseHandle(s)
#define SEMA_T HANDLE

#define THREAD_CREATE(id, func, params) (!(id = (HANDLE)_beginthreadex(NULL, 0, func, params, 0, NULL)))
#define THREAD_JOIN(id) WaitForSingleObject(id, INFINITE)
#define THREAD_RETURN return 0;
#define THREAD_T HANDLE 
#define THREAD_RET_T unsigned int __stdcall 

#define SOCKET_CLOSE(__SOCKET__) closesocket(__SOCKET__)
#define SOCKET_IS_INVALID(__SOC__) (__SOC__ == INVALID_SOCKET)

//#define snprintf _snprintf

#elif defined(_APBUILD_UNIX_ALL__) || defined(__unix__)

#ifndef _APBUILD_UNIX_ALL__
#define _APBUILD_UNIX_ALL__
#endif

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

#define WRITE_PMODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH

#define MUTEX_LOCK(m) pthread_mutex_lock(m)
#define MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#define MUTEX_INIT(m) pthread_mutex_init(m, NULL)
#define MUTEX_DESTROY(m) pthread_mutex_destroy(m)
#define MUTEX_T pthread_mutex_t

#define SEMA_WAIT(s) {while (sem_wait(&s));}
#define SEMA_POST(s) sem_post(&s)
#define SEMA_INIT(s) sem_init(&s, 0, 0)
#define SEMA_DESTROY(s) sem_destroy(&s)
#define SEMA_T sem_t

#define THREAD_CREATE(id, func, params) pthread_create(& id, NULL, func, params)
#define THREAD_JOIN(id) pthread_join(id, NULL)
#define THREAD_RETURN return NULL;
#define THREAD_T pthread_t
#define THREAD_RET_T void*

typedef int SOCKET;

#define SOCKET_ERROR (-1)
#define SOCKET_CLOSE(__SOCKET__) close(__SOCKET__)
#define SOCKET_IS_INVALID(__SOC__) (__SOC__ < 0)
#define INVALID_SOCKET (-1)

#define GETTIME_DECLARE(var) struct timeval var
#define GETTIME_GET(var) gettimeofday(&var, NULL);
#define GETTIME_VALUE(var) ((var.tv_sec * 1000) + var.tv_usec)


#else

#error "Can't determine platform!"
#error "Try #define_APBUILD_WIN32_ALL__ or #define _APBUILD_UNIX_ALL__"

#endif

#endif  /* PROTOCOL_PLUGINS_H */
