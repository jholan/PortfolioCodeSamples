#pragma once

#include <string>

struct ThreadHandleBaseType;					// We never need to define a ThreadHandleBaseType, as it is never actually used, just the pointer is 
typedef ThreadHandleBaseType* ThreadHandle; 
// typedef void* thread_handle_t;

typedef void(*Thread_Callback)(void* data); 



// Creation
ThreadHandle	Thread_Create(Thread_Callback callback, void* data = nullptr);		
void			Thread_CreateAndDetach(Thread_Callback callback, void* data = nullptr);	// Start an OS managed thread


// Release
void			Thread_Join(ThreadHandle handle);	// Wait for the thread to finish
void			Thread_Detach(ThreadHandle handle);	// Pass the thread to the OS


// Control
void			Thread_Sleep(unsigned int milliseconds); 
void			Thread_Yield(); 


// Debug
void			Thread_SetName(const std::string& name); 


// Optimization/ Tuning
void			Thread_SetCoreAffinity(unsigned int core_bitfield);  // What cores we want the thread to run on
void			Thread_SetPriority(unsigned int priority);			// 0(low) - 10(high)