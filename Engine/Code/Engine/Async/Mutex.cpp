#include "Engine/Async/Mutex.hpp"

#include <windows.h>



Mutex::Mutex()
{
	m_handle = (void*)CreateMutex(	NULL,              // default security attributes
									FALSE,             // initially not owned
									NULL);             // unnamed mutex
}



Mutex::~Mutex()
{
	CloseHandle((HANDLE)m_handle); // Release the handle
}



void Mutex::Lock()
{
	WaitForSingleObject( (HANDLE)m_handle,  // handle to mutex
						 INFINITE);			// no time-out interval
}



void Mutex::Unlock()
{
	ReleaseMutex((HANDLE)m_handle);
}
