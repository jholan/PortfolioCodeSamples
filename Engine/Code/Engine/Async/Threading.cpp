#include "Engine/Async/Threading.hpp"

#include <windows.h>

#include "Engine/Core/EngineCommon.h"


#define STACK_SIZE (0)


struct ThreadData
{
	Thread_Callback m_callback;
	void*			m_data;
};



//-------------------------------------------------------------------------------------------------------
static DWORD WINAPI WindowsThreadEntryPoint(void* td)
{
	ThreadData* threadData = (ThreadData*)td;

	// Do our threads actual work
	threadData->m_callback(threadData->m_data);

	// Clean up the thread
	delete threadData;

	return 0;
}



//-------------------------------------------------------------------------------------------------------
ThreadHandle Thread_Create(Thread_Callback callback, void* data)
{
	// Create the bundle to pass to the windows thread entry point
	// Destruction is handled in the windows thread entry point
	ThreadData* threadData = new ThreadData();
	threadData->m_callback = callback;
	threadData->m_data = data;

	// Create the thread
	DWORD id = 0;			// Whats this for?
	ThreadHandle handle = (ThreadHandle)CreateThread(NULL, STACK_SIZE, WindowsThreadEntryPoint, threadData, 0, &id);

	// Return the handle
	return handle;
}


//-------------------------------------------------------------------------------------------------------
void Thread_CreateAndDetach(Thread_Callback callback, void* data)
{
	ThreadHandle handle = Thread_Create(callback, data);
	Thread_Detach(handle);
}


//-------------------------------------------------------------------------------------------------------																					// Release
void Thread_Join(ThreadHandle handle)
{
	if (handle != nullptr)
	{
		WaitForSingleObject(handle, INFINITE);
		CloseHandle(handle);
	}
}


//-------------------------------------------------------------------------------------------------------
void Thread_Detach(ThreadHandle handle)
{
	if (handle != nullptr)
	{
		CloseHandle((HANDLE)handle);
	}
}


//-------------------------------------------------------------------------------------------------------
void Thread_Sleep(unsigned int milliseconds)
{
	Sleep((DWORD)milliseconds);
}


//-------------------------------------------------------------------------------------------------------
void Thread_Yield()
{
	SwitchToThread();
}


//-------------------------------------------------------------------------------------------------------
void Thread_SetName(const std::string& name)
{
	UNUSED(name);
	UNIMPLEMENTED();
}


//-------------------------------------------------------------------------------------------------------
void Thread_SetCoreAffinity(unsigned int core_bitfield)
{
	UNUSED(core_bitfield);
	UNIMPLEMENTED();
}


//-------------------------------------------------------------------------------------------------------
void Thread_SetPriority(unsigned int priority)
{
	UNUSED(priority);
	UNIMPLEMENTED();
}
