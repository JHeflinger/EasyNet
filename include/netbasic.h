#ifndef NETBASIC
#define NETBASIC

#include "easylog.h"
#include <stdint.h>
#include <string.h>

#define EZN_NONE 0
#define EZN_ERROR 1
typedef uint32_t EZN_STATUS;

#define EZN_FALSE 0
#define EZN_TRUE 1
typedef uint8_t EZN_BOOL;

#define EZN_TCP 0
#define EZN_UDP 1
typedef int EZN_SERVER_TYPE;
typedef int EZN_CLIENT_TYPE;

#define EZN_SERVER_OPEN 0
#define EZN_SERVER_CLOSED 1
typedef int EZN_SERVER_STATUS;

#define EZN_CLIENT_CONNECTED 0
#define EZN_CLIENT_DISCONNECTED 1 
typedef int EZN_CLIENT_STATUS;

typedef uint8_t EZN_BYTE;

#define MAX_IP_ADDR_LENGTH 64
#define MIN_PORT 1024
#define MAX_PORT 65535
#define DEFAULT_PORT 44448

#define IPV4_ADDR_LENGTH 4

#define EZN_ACCEPT_FOREVER -1

#ifdef __linux__

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <pthread.h>
#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

typedef pthread_t EZN_THREAD;
typedef pthread_mutex_t EZN_MUTEX;
typedef int EZN_SOCKET;

#define EZN_INVALID_SOCK -1
#define EZN_PROTOCOL int
#define EZN_TCP_PROTOCOL 0
#define EZN_UDP_PROTOCOL 0
#define EZN_CLOSE(...) close(__VA_ARGS__)
#define EZN_OPT_TYPE int
#define EZN_DONT_WAIT MSG_DONTWAIT
#define EZN_CREATE_THREAD(thread, func, parameters) pthread_create(&thread, NULL, (void* (*)(void*))func, parameters)
#define EZN_WAIT_THREAD(thread) pthread_join(thread, NULL)
#define EZN_CLOSE_THREAD(thread)
#define EZN_CREATE_MUTEX(mutex) pthread_mutex_init(&mutex, NULL)
#define EZN_LOCK_MUTEX(mutex) pthread_mutex_lock(&mutex)
#define EZN_RELEASE_MUTEX(mutex) pthread_mutex_unlock(&mutex)

#elif _WIN32

// basic cleanup to avoid windows lib bloat
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOKERNEL          // All KERNEL defines and routines
#define NOUSER            // All USER defines and routines
#define NOMB              // MB_* and MessageBox()
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions

#include <stdlib.h>
#include <winsock2.h>

typedef HANDLE EZN_THREAD;
typedef HANDLE EZN_MUTEX;
typedef SOCKET EZN_SOCKET;

#define EZN_INVALID_SOCK INVALID_SOCKET
#define EZN_PROTOCOL IPPROTO
#define EZN_TCP_PROTOCOL IPPROTO_TCP
#define EZN_UDP_PROTOCOL IPPROTO_UDP
#define EZN_CLOSE(...) closesocket(__VA_ARGS__)
#define EZN_OPT_TYPE char
#define EZN_DONT_WAIT 0
#define EZN_CREATE_THREAD(thread, func, parameters) thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, (LPVOID)parameters, 0, NULL)
#define EZN_WAIT_THREAD(thread) WaitForSingleObject(thread, INFINITE)
#define EZN_CLOSE_THREAD(thread) CloseHandle(thread)
#define EZN_CREATE_MUTEX(mutex) mutex = CreateMutex(NULL, FALSE, NULL)
#define EZN_LOCK_MUTEX(mutex) WaitForSingleObject(mutex, INFINITE)
#define EZN_RELEASE_MUTEX(mutex) ReleaseMutex(mutex)

#else
#error Unsupported operating system detected!
#endif

void ezn_init();
void ezn_clean();
int ezn_is_initialized();

#define EZN_SAFECHECK() {if (!ezn_is_initialized()) { EZN_WARN("EasyNet was not intialized. Please initialize it using ezn_init(). Implicitly intializing now..."); ezn_init();}}

#endif
