#pragma once

#include <string>
#include <vector>
#include <deque>


// A command is a single submitted commmand
// NOT the definition (which I hide internally)
// Comments will be using a Command constructed as follows; 
// Command cmd = Command( "echo_with_color (255,255,0) \"Hello \\"World\\"\" ); 
class Command
{
public:
	Command( char const *str ); 
	~Command(); 

	std::string GetName(); // would return "echo_with_color"

									// Gets the next string in the argument list.
									// Breaks on whitespace.  Quoted strings should be treated as a single return 
	std::string GetNextString();   // would return after each call...
								   // first:  "(255,255,0)""
								   // second: "Hello \"world\""
								   // third+: "

private:
	std::string	m_name;
	std::string	m_arguments;
	size_t		m_argStart;
};

// Command callbacks take a Command.
typedef void (*command_cb)( Command& cmd ); 

// Registers a command with the system
// Example, say we had a global function named...
//    void Help( Command &cmd ) { /* ... */ }  
// 
// We then, during some startup, call
//    CommandRegister( "help", Help ); 
void RegisterCommand( char const* commandName, command_cb callback ); 

// Will construct a Command object locally, and if 
// a callback is associated with its name, will call it and 
// return true, otherwise returns false.
// Name is case-insensitive
bool RunCommand( char const* commandNameAndParameters );

std::vector<std::string> GetAllCommandNames();

std::deque<std::string> GetCommandHistory();