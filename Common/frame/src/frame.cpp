#include "pch.h"
#include "pthread.h"


#ifdef WIN32

BOOL APIENTRY DllMain( HMODULE hModule,  
                      DWORD  ul_reason_for_call,  
                      LPVOID lpReserved  
                      )  
{  
#ifdef PTW32_STATIC_LIB
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //pthread_win32_process_attach_np();
        break;
    case DLL_THREAD_ATTACH:
        //pthread_win32_thread_attach_np();
        break;
    case DLL_THREAD_DETACH:
        //pthread_win32_thread_detach_np();
        break;
    case DLL_PROCESS_DETACH:
        //pthread_win32_thread_detach_np();
        //pthread_win32_process_detach_np();
        break;
    }
#endif

    return TRUE;
}  

#endif