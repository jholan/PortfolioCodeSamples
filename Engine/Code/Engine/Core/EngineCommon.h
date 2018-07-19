#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"



#define UNUSED(x) (void)(x);

#define STACK_ALLOC_ARRAY( T, count ) ((T*)_alloca(sizeof(T) * count))


#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

#define PRAGMA(p)  __pragma( p )
#define NOTE( x )  PRAGMA( message(x) )
#define FILE_LINE  NOTE( __FILE__LINE__ )

// THE IMPORANT BITS
#define TODO( x )  NOTE( __FILE__LINE__"\n"           \
        " --------------------------------------------------------------------------------------\n" \
        "|  TODO :   " ##x "\n" \
        " --------------------------------------------------------------------------------------\n" )

#define _QUOTE(x) #x
#define QUOTE(x) _QUOTE(x)

#define UNIMPLEMENTED()  TODO( "IMPLEMENT: " QUOTE(__FILE__) " (" QUOTE(__LINE__) ")" ); ASSERT_RECOVERABLE(0, "");

class  Blackboard;
extern Blackboard g_gameConfigBlackboard; 

enum Space
{
	SPACE_INVALID = -1,

	SPACE_LOCAL = 0,
	SPACE_WORLD,

	SPACE_COUNT
};


#define NUM_CASCADES 3