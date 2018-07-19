#include "Engine/Async/ReaderWriterMutex.hpp"

#include <shared_mutex>


ReaderWriterMutex::ReaderWriterMutex()
{
	m_handle = (void*)(new std::shared_mutex());
}



ReaderWriterMutex::~ReaderWriterMutex()
{
	delete m_handle;
	m_handle = nullptr;
}



void ReaderWriterMutex::LockForWrite()
{
	std::shared_mutex* lock = (std::shared_mutex*)m_handle;
	lock->lock();
}



void ReaderWriterMutex::LockForRead()
{
	std::shared_mutex* lock = (std::shared_mutex*)m_handle;
	lock->lock_shared();
}



void ReaderWriterMutex::UnlockForWrite()
{
	std::shared_mutex* lock = (std::shared_mutex*)m_handle;
	lock->unlock();
}



void ReaderWriterMutex::UnlockForRead()
{
	std::shared_mutex* lock = (std::shared_mutex*)m_handle;
	lock->unlock_shared();
}
