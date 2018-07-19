#include "Engine/Commands/Command.hpp"

#include <map>
#include <deque>

#include "Engine/Core/StringUtils.hpp"

void AddToCommandHistory(const std::string& commandAndParameters);


Command::Command( char const *str )
{
	std::string command = str;

	// Is the string is the nullstring
	if (command.empty())
	{
		return;
	}


	m_argStart = std::string::npos;
	size_t nameStart = command.find_first_not_of(" ");
	size_t nameEnd = command.find_first_of(" ", nameStart);
	if (nameStart == std::string::npos && nameEnd == std::string::npos)
	{
		// We have a string with only whitespace
		return;
	}
	if (nameEnd == std::string::npos)
	{
		m_name = str;
	}
	else
	{
		m_name = command.substr(nameStart, nameEnd - nameStart);
	
		size_t argsStart = command.find_first_not_of(" ", nameEnd);
		if (argsStart != std::string::npos)
		{
			size_t argsLength = command.length() - argsStart;
			m_arguments = command.substr(argsStart, argsLength);
			m_argStart = 0;
		}
	}
}



Command::~Command()
{

}



std::string Command::GetName()
{
	return m_name;
}



std::string Command::GetNextString()
{
	if (m_argStart == std::string::npos)
	{
		return "";
	}

	std::string argument;

	bool isParen = false;
	bool isEmpty = false;
	size_t argEnd = std::string::npos;
	switch(m_arguments[m_argStart])
	{
	case '\"':
	{
		argEnd = m_arguments.find_first_of("\"", m_argStart + 1) + 1;

		isParen = true;
		if (m_argStart + 1 == argEnd - 1)
		{
			isEmpty = true;
		}
		break;
	}

	case '(':
	{
		argEnd = m_arguments.find_first_of(")", m_argStart) + 1;
		break;
	}

	default:
	{
		argEnd = m_arguments.find_first_of(" ", m_argStart);
		break;
	}
	};

	if (argEnd == std::string::npos)
	{
		argument = m_arguments.substr(m_argStart, m_arguments.length() - m_argStart);
		m_argStart = std::string::npos;
	}
	else if (isParen)
	{
		if (isEmpty)
		{
			m_argStart = m_arguments.find_first_not_of(" ", argEnd);
		}
		else
		{
			argument = m_arguments.substr(m_argStart + 1, argEnd - m_argStart - 2);
		}
	}
	else
	{
		argument = m_arguments.substr(m_argStart, argEnd - m_argStart);
		m_argStart = m_arguments.find_first_not_of(" ", argEnd);
	}

	return argument;
}



static bool								 s_commandsInitialized = false;
static std::map<std::string, command_cb> s_commands;
static std::deque<std::string>			 s_commandHistory;



void RegisterCommand( char const* commandName, command_cb callback )
{
	if (!s_commandsInitialized)
	{
		s_commandsInitialized = true;
		s_commands = std::map<std::string, command_cb>();
	}

	std::string commandNameLower = ToLower(commandName);
	auto iter = s_commands.find(commandNameLower);
	if (iter == s_commands.end())
	{
		// No command with that name registered so we can register this one
		s_commands[commandNameLower] = callback;
	}
}



bool RunCommand( char const* commandNameAndParameters )
{
	bool succeeded = false;

	Command tempCommand = Command(commandNameAndParameters);
	std::string commandName = ToLower(tempCommand.GetName());
	auto iter = s_commands.find(commandName);
	if (iter != s_commands.end())
	{
		// Found a command
		succeeded = true;
		iter->second(tempCommand);
	}

	AddToCommandHistory(commandNameAndParameters);
	return succeeded;
}



std::vector<std::string> GetAllCommandNames()
{
	std::vector<std::string> commandNames;

	for (auto iter = s_commands.begin(); iter != s_commands.end(); ++iter)
	{
		commandNames.push_back(iter->first);
	}

	return commandNames;
}



void AddToCommandHistory(const std::string& commandAndParameters)
{
	// Remove the command from the history if it is already present
	for (size_t i = 0; i < s_commandHistory.size(); ++i)
	{
		if (s_commandHistory[i] == commandAndParameters)
		{
			s_commandHistory.erase(s_commandHistory.begin() + i);
			break;
		}
	}

	s_commandHistory.push_front(commandAndParameters);
}


std::deque<std::string> GetCommandHistory()
{
	return s_commandHistory;
}
