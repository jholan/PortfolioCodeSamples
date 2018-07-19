#include "Engine/Logger/Logger.hpp"

#ifdef _WIN32
#define PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <stdarg.h>
#include <fstream>

#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Async/Threading.hpp"
#include "Engine/Async/ThreadSafeQueue.hpp"
#include "Engine/Async/ReaderWriterMutex.hpp"

#include "Engine/Commands/Command.hpp"
#include "Engine/Commands/DevConsole.hpp"
extern DevConsole* g_theDevConsole;



// State
static bool								s_isRunning		= true;
volatile static bool					s_shouldFlush    = true;

// Log
static ThreadHandle						s_workerThread	= nullptr;
static ThreadSafeQueue<Log_LogEntry*>	s_logQueue;

// Hooks
static ReaderWriterMutex				s_hookListMutex;
static std::vector<LogHook>				s_hookList;

// Tags
static std::vector<std::string>			s_tags;						// default empty
static bool								s_isWhiteList = false;		// default false
static ReaderWriterMutex				s_tagMutex;

// File Writing
static std::string						s_logFilepath = "Log/log.txt";
static std::ofstream					s_logFile;
static std::ofstream					s_logFile_Timestamped;



//-------------------------------------------------------------------------------------------------------
void Log_HideAll_Command(Command& cmd)
{
	UNUSED(cmd);

	Log_HideAll();
	g_theDevConsole->PrintToLog(RGBA(0,255,0), "Hide All");
}


//-------------------------------------------------------------------------------------------------------
void Log_ShowAll_Command(Command& cmd)
{
	UNUSED(cmd);

	Log_ShowAll();
	g_theDevConsole->PrintToLog(RGBA(0,255,0), "Show All");
}


//-------------------------------------------------------------------------------------------------------
void Log_ShowTag_Command(Command& cmd)
{
	std::string tag = cmd.GetNextString();

	if (!tag.empty())
	{
		Log_ShowTag(tag.c_str());
		g_theDevConsole->PrintToLog(RGBA(0,255,0), Stringf("ShowTag: %s", tag.c_str()));
	}
	else
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "Invalid Tag");
	}
}


//-------------------------------------------------------------------------------------------------------
void Log_HideTag_Command(Command& cmd)
{
	std::string tag = cmd.GetNextString();

	if (!tag.empty())
	{
		Log_HideTag(tag.c_str());
		g_theDevConsole->PrintToLog(RGBA(0,255,0), Stringf("HideTag: %s", tag.c_str()));
	}
	else
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "Invalid Tag");
	}
}


//-------------------------------------------------------------------------------------------------------
void Logger_FileWrite_Hook(const Log_LogEntry& entry, void* userData)
{
	std::ofstream* fileWriter = (std::ofstream*)userData;

	*fileWriter << Stringf("%s: %s\n", entry.m_tag.c_str(), entry.m_text.c_str());
}


//-------------------------------------------------------------------------------------------------------
void FileWrite_Hook_Initialize()
{
	// Create the folder if it doesnt exist
	if (!DoesFolderExist(GetFolderPath(s_logFilepath)))
	{
		CreateFolder(GetFolderPath(s_logFilepath));
	}

	s_logFile.open(s_logFilepath);
	if (s_logFile.is_open())
	{
		Log_Hook(Logger_FileWrite_Hook, (void*)&s_logFile);
	}

	s_logFile_Timestamped.open(AppendTimestamp(s_logFilepath));
	if (s_logFile_Timestamped.is_open())
	{
		Log_Hook(Logger_FileWrite_Hook, (void*)&s_logFile_Timestamped);
	}
}


//-------------------------------------------------------------------------------------------------------
void FileWrite_Hook_Destroy()
{
	if (s_logFile.is_open())
	{
		s_logFile.close();
	}

	if (s_logFile_Timestamped.is_open())
	{
		s_logFile_Timestamped.close();
	}
}


//-------------------------------------------------------------------------------------------------------
void VisualStudioOutput_Hook(const Log_LogEntry& entry, void* userData)
{
	UNUSED(userData);

#if defined( PLATFORM_WINDOWS )
	if( IsDebuggerAvailable() )
	{
		OutputDebugStringA( Stringf("%s: %s\n", entry.m_tag.c_str(), entry.m_text.c_str()).c_str() );
	}
#endif
}


//-------------------------------------------------------------------------------------------------------
void Logger_DevConsole_Hook(const Log_LogEntry& entry, void* userData)
{
	UNUSED(userData);

	g_theDevConsole->AddThreadedEntry(Stringf("%s: %s\n", entry.m_tag.c_str(), entry.m_text.c_str()));
}


//-------------------------------------------------------------------------------------------------------
void LogTest_ThreadFunction(void* data)
{
	int* threadNumber = (int*)data;

	bool success = false;
	std::string line;
	std::ifstream bigFile;
	bigFile.open("Data/BuiltIns/Big.txt");
	if (bigFile.is_open())
	{
		int lineNumber = 0;
		while ( getline (bigFile, line) )
		{
			Log( "[%u:%u] %s", *threadNumber, lineNumber, line.c_str()); 

			++lineNumber;	
		}

		bigFile.close();
		success = true;

	}

	delete threadNumber;
}


//-------------------------------------------------------------------------------------------------------
void Log_Test(unsigned int numThreads)
{
	for (int i = 0; i < (int)numThreads; ++i)
	{
		int* threadNumber = new int;
		*threadNumber = i;
		Thread_CreateAndDetach(LogTest_ThreadFunction, (void*)threadNumber);
	}
}


//-------------------------------------------------------------------------------------------------------
void Log_Test_Command(Command& cmd)
{
	int numThreads = StringToInt(cmd.GetNextString().c_str());

	if (numThreads != 0)
	{
		Log_Test(numThreads);
	}
	else
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "Must specify at least 1 thread");
	}
}


void Log_FlushTest_Command(Command& cmd)
{
	UNUSED(cmd);

	Log_FlushTest();
}


//-------------------------------------------------------------------------------------------------------
void LoggerWorkerThread_Callback(void* data)
{
	UNUSED(data);


	// While the logger is running
	while(Log_IsRunning())
	{
		Log_DoAllWork();
		Thread_Sleep(10);
	}


	// One last flush after the logger has been shut down
	Log_DoAllWork();
}


//-------------------------------------------------------------------------------------------------------
void Log_Initialize()
{
	FileWrite_Hook_Initialize();
	Log_Hook(VisualStudioOutput_Hook);
	Log_Hook(Logger_DevConsole_Hook);

	RegisterCommand("Log_ShowAll",		Log_ShowAll_Command);
	RegisterCommand("Log_HideAll",		Log_HideAll_Command);
	RegisterCommand("Log_ShowTag",		Log_ShowTag_Command);
	RegisterCommand("Log_HideTag",		Log_HideTag_Command);
	RegisterCommand("Log_Test",			Log_Test_Command);
	RegisterCommand("Log_FlushTest",	Log_FlushTest_Command);

	s_workerThread = Thread_Create(LoggerWorkerThread_Callback);
}


//-------------------------------------------------------------------------------------------------------
void Log_Destroy()
{
	// Stop the worker thread
	Log_SetRunningState(false);
	Thread_Join(s_workerThread);
	s_workerThread = nullptr;


	FileWrite_Hook_Destroy();
}


//-------------------------------------------------------------------------------------------------------
bool Log_IsRunning()
{
	return s_isRunning;
}


//-------------------------------------------------------------------------------------------------------
void Log_SetRunningState(bool isRunning)
{
	s_isRunning = isRunning;
}


//-------------------------------------------------------------------------------------------------------
bool HasValidTag(const Log_LogEntry& entry)
{
	bool isValid = false;
	s_tagMutex.LockForRead();


	// Not so critical section
	if (s_isWhiteList)
	{
		// Whitelist so default to false
		isValid = false; 

		// If the tag is present in the list we are good
		for (int i = 0; i < (int)s_tags.size(); ++i)
		{
			if (s_tags[i] == entry.m_tag)
			{
				isValid = true;
				break;
			}
		}
	}
	else
	{
		// Blacklist so default to true
		isValid = true;

		// If the tag is NOT present in the list we are good
		for (int i = 0; i < (int)s_tags.size(); ++i)
		{
			if (s_tags[i] == entry.m_tag)
			{
				isValid = false;
				break;
			}
		}
	}


	s_tagMutex.UnlockForRead();
	return isValid;
}


//-------------------------------------------------------------------------------------------------------
void ProcessEntry(const Log_LogEntry& entry)
{
	s_hookListMutex.LockForRead();

	bool hasValidTag = HasValidTag(entry);
	if (hasValidTag)
	{
		// Not so critical section
		for (int i = 0; i < (int)s_hookList.size(); ++i)
		{
			// Call all callbacks with this entry
			s_hookList[i].m_callback(entry, s_hookList[i].m_userData);
		}
	}

	s_hookListMutex.UnlockForRead();
}


//-------------------------------------------------------------------------------------------------------
void Log_DoAllWork()
{
	bool shouldFlush = s_shouldFlush;

	Log_LogEntry* entry = nullptr; 
	while (s_logQueue.Dequeue(&entry))
	{
		ProcessEntry(*entry); 
		delete entry; 
	}

	if (shouldFlush)
	{
		if (s_logFile.is_open())
		{
			s_logFile.flush();
		}

		if (s_logFile_Timestamped.is_open())
		{
			s_logFile_Timestamped.flush();
		}

		s_shouldFlush = false;
	}
}


//-------------------------------------------------------------------------------------------------------
void Log_Flush()
{
	s_shouldFlush = true;

	//Log_DoAllWork();
	while(s_shouldFlush)
	{
		// Wait
	}

	return;
}


//-------------------------------------------------------------------------------------------------------
void Log_FlushTest()
{
	Log("LogFlushTest"); 
	Log_Flush();
	// breakpoint on this line; 
	GUARANTEE_RECOVERABLE(false, "Flush Test");
}


//-------------------------------------------------------------------------------------------------------
void LogTagged(char const* tag, char const* format, va_list args)
{
	Log_LogEntry* entry = new Log_LogEntry();
	entry->m_tag = tag;
	entry->m_text = Stringf(format, args);

	s_logQueue.Enqueue(entry);
}


//-------------------------------------------------------------------------------------------------------
void LogTagged(const char* tag, const char* format, ...)
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogTagged(tag, format, variableArgumentList);
	va_end( variableArgumentList );
}


//-------------------------------------------------------------------------------------------------------
void Log(const char* format, ...)
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogTagged("LOG", format, variableArgumentList);
	va_end( variableArgumentList );
}


//-------------------------------------------------------------------------------------------------------
void LogDebug(const char* format, ...)
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogTagged("DEBUG", format, variableArgumentList);
	va_end( variableArgumentList );
}


//-------------------------------------------------------------------------------------------------------
void LogWarning(char const *format, ...)
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogTagged("WARNING", format, variableArgumentList);
	va_end( variableArgumentList );
}


//-------------------------------------------------------------------------------------------------------
void LogError(char const *format, ...)
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogTagged("ERROR", format, variableArgumentList);
	va_end( variableArgumentList );

	Log_Flush();
	GUARANTEE_OR_DIE(false, "Log Error");
}


//-------------------------------------------------------------------------------------------------------
void Log_ShowAll()
{
	s_tagMutex.LockForWrite();

	// Critical Section
	s_tags.clear();
	s_isWhiteList = false;

	s_tagMutex.UnlockForWrite();
}


//-------------------------------------------------------------------------------------------------------
void Log_HideAll()
{
	s_tagMutex.LockForWrite();

	// Critical Section
	s_tags.clear();
	s_isWhiteList = true;

	s_tagMutex.UnlockForWrite();
}


//-------------------------------------------------------------------------------------------------------
void Log_ShowTag(char const* tag)
{
	s_tagMutex.LockForWrite();

	// Critical Section
	if (!s_isWhiteList)
	{
		// As blacklist
		for (int i = 0; i < (int)s_tags.size(); ++i)
		{
			// If the tag exists in the list remove it
			if (s_tags[i] == tag)
			{
				s_tags.erase(s_tags.begin() + i);
				break;
			}
		}
	}
	else
	{
		// As whitelist

		// Look for the tag in the list
		bool isPresent = false;
		for (int i = 0; i < (int)s_tags.size(); ++i)
		{
			if (s_tags[i] == tag)
			{
				isPresent = true;
			}
		}

		// If it doesn't exist add it
		if (!isPresent)
		{
			s_tags.push_back(tag);
		}
	}

	s_tagMutex.UnlockForWrite();
}


//-------------------------------------------------------------------------------------------------------
void Log_HideTag(char const* tag)
{
	s_tagMutex.LockForWrite();

	// Critical Section
	if (!s_isWhiteList)
	{
		// As blacklist

		// Look for the tag in the list
		bool isPresent = false;
		for (int i = 0; i < (int)s_tags.size(); ++i)
		{
			if (s_tags[i] == tag)
			{
				isPresent = true;
			}
		}

		// If it doesn't exist add it
		if (!isPresent)
		{
			s_tags.push_back(tag);
		}
		
	}
	else
	{
		// As whitelist
		for (int i = 0; i < (int)s_tags.size(); ++i)
		{
			// If the tag exists in the list remove it
			if (s_tags[i] == tag)
			{
				s_tags.erase(s_tags.begin() + i);
				break;
			}
		}
	}

	s_tagMutex.UnlockForWrite();
}


//-------------------------------------------------------------------------------------------------------
void Log_Hook(Log_Callback callback, void* userData)
{
	s_hookListMutex.LockForWrite();

	// Critical Section
	LogHook hook;
	hook.m_callback = callback;
	hook.m_userData = userData;
	s_hookList.push_back(hook);

	s_hookListMutex.UnlockForWrite();
}


//-------------------------------------------------------------------------------------------------------
void Log_Unhook(Log_Callback callback, void* userData)
{
	s_hookListMutex.LockForWrite();

	// Critical Section
	for (int i = 0; i < (int)s_hookList.size(); ++i)
	{
		// Remove the callback if it exists
		if (s_hookList[i].m_callback == callback && s_hookList[i].m_userData == userData)
		{
			s_hookList.erase(s_hookList.begin() + i);
			break;
		}
	}

	s_hookListMutex.UnlockForWrite();
}
