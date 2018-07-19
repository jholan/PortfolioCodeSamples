#pragma once

#include <queue>

#include "Engine/Async/Mutex.hpp"



template <typename T>
class ThreadSafeQueue
{
public:
	void Enqueue(const T& v)
	{
		m_mutexLock.Lock();

		// Critical Section
		m_data.push( v ); 

		m_mutexLock.Unlock();
	}

	// return if it succeeds
	bool Dequeue(T* out_v) 
	{
		m_mutexLock.Lock();

		// Critical Section
		bool isNotEmpty = !m_data.empty();
		if (isNotEmpty) 
		{
			*out_v = m_data.front();
			m_data.pop(); 
		}

		m_mutexLock.Unlock();
		return isNotEmpty; 
	}



private:
	std::queue<T> m_data; 
	Mutex		  m_mutexLock; 
};