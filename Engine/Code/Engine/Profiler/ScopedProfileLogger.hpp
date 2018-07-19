#pragma once

#include "Engine/Core/Time.hpp"



class ScopedProfileLogger
{
public:
	ScopedProfileLogger(const char* name);
	~ScopedProfileLogger();


private:
	uint64_t	m_startHPC;
	const char* m_name;
};



#define PROFILE_LOG_SCOPE(tag) ScopedProfileLogger __timer_ ##__LINE__ ## (tag)

#define PROFILE_LOG_SCOPE_FUNCTION() ScopedProfileLogger __timer_ ##__LINE__ ## (__FUNCTION__)