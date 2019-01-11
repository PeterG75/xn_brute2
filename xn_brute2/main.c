#include "pch.h"
#include "mt_rand.h"
#include "xn_rand.h"

#define XN_RAND_OUTPUTS   0x8000
#define NUMBER_OF_THREADS 7
#define SEED_MAX          UINT32_MAX

unsigned __stdcall thread_start(void *param)
{
    WORD wThreadNum = LOWORD((DWORD)((UINT_PTR)param));
    WORD wThreadCount = HIWORD((DWORD)((UINT_PTR)param));
    uint32_t s = (SEED_MAX / wThreadCount) * (wThreadNum - 1);
    uint32_t end = (wThreadNum == wThreadCount)
        ? SEED_MAX
        : (((SEED_MAX / wThreadCount) * wThreadNum) - 1);
    DWORD dwThreadId = GetCurrentThreadId();
    struct mt_s *mt = malloc(sizeof *mt);
    char *buffer = malloc(XN_RAND_OUTPUTS + 1);
    char *ptr;

    (void)printf_s("[%lX] Generating outputs from seed %08lX to %08lX...\n",
        dwThreadId,
        s,
        end);

    do {
        php_mt_srand(mt, s);
        xn_rand(mt, buffer, XN_RAND_OUTPUTS + 1);
        if ( (ptr = strstr(buffer, "8MT4K54B4XFVVM5"))
            || (ptr = strstr(buffer, "PJUETXY2KHARFU8")) ) {

            // dump output buffer
            //printf_s("%hs\n\n", buffer);

            (void)printf_s(
                "[%lX] Found %.*hs at output %u [seed=%08lX]\n\n",
                dwThreadId,
                15, ptr,
                (unsigned int)(ptr - buffer),
                s);
        }
        ++s;
    } while ( end - s );
    free(buffer);
    free(mt);
    return 0;
}

int __cdecl main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    HANDLE ThreadHandles[NUMBER_OF_THREADS];

    (void)SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
    
    // start 7 threads
    for ( WORD n = 0; n < _countof(ThreadHandles); ++n ) {
        ThreadHandles[n] = (HANDLE)_beginthreadex(
            NULL,
            0,
            thread_start,
            (LPVOID)((UINT_PTR)MAKELONG(n + 1, _countof(ThreadHandles))),
            0,
            NULL);
    }
    (void)WaitForMultipleObjects(
        _countof(ThreadHandles),
        ThreadHandles,
        TRUE,
        INFINITE);
    (void)printf("Threads finished, press any key to exit...");
    (void)_getch();
    return 0;
}
