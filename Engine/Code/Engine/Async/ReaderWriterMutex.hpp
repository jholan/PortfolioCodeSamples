#pragma once



class ReaderWriterMutex
{
public:
	ReaderWriterMutex();
	~ReaderWriterMutex();

	void LockForWrite();	// Singular
	void LockForRead();		// Multiple

	void UnlockForWrite();
	void UnlockForRead();


private:
	void*	m_handle = nullptr;
};