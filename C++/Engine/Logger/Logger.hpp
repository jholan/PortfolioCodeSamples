#pragma once

#include <string>
#include <functional>


struct Log_LogEntry
{
	std::string m_tag; 
	std::string m_text; 
};


typedef void (*Log_Callback)(const Log_LogEntry& entry, void* userData); 
struct LogHook
{
	Log_Callback	m_callback;
	void*			m_userData = nullptr;
};




// Composition
void Log_Initialize();
void Log_Destroy();

// Running
bool Log_IsRunning();
void Log_SetRunningState(bool isRunning);

// Flush
void Log_DoAllWork();
void Log_Flush();
void Log_FlushTest();

// Logging
void Log(const char* format, ...);
void LogDebug(const char* format, ...);
void LogWarning(char const *format, ...); 
void LogError(char const *format, ...); 
void LogTagged(const char* tag, const char* format, ...);
void LogTagged_va(char const *tag, char const *format, va_list args);

// Filtering
void Log_ShowAll(); 
void Log_HideAll(); 
void Log_ShowTag(char const* tag); 
void Log_HideTag(char const* tag); 

// Additional Logger Hooks
void Log_Hook(Log_Callback callback, void* userData = nullptr); 
void Log_Unhook(Log_Callback callback, void* userData = nullptr);
