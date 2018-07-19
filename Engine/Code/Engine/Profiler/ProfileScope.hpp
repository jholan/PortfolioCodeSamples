#pragma once



// Calls ProfilePush on construction and ProfilePop on destruction
class ProfileScope
{
public:
	ProfileScope(const char* name);
	~ProfileScope();

private:
};



#define PROFILE_SCOPE(tag) ProfileScope __timer_ ##__LINE__ ## (tag)

#define PROFILE_SCOPE_FUNCTION() ProfileScope __timer_ ##__LINE__ ## (__FUNCTION__)