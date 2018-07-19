#pragma once

#include <string>
#include <deque>

#include "Engine/Async/ThreadSafeQueue.hpp"

#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"

class BitmapFont;
class Texture;
class Camera;

enum ConsoleMode
{
	CONSOLE_MODE_INVALID = -1,

	CONSOLE_MODE_BASIC = 0,
	CONSOLE_MODE_HISTORY,

	CONSOLE_MODE_COUNT
};

class LogEntry
{
public:
	RGBA		m_drawColor;
	std::string m_text;

	LogEntry() : m_drawColor(RGBA()), m_text("") {};
	LogEntry(const RGBA& color, const std::string& string)
		: m_drawColor(color)
		, m_text(string)
	{}
};



class DevConsole 
{
public:
	DevConsole(); 
	~DevConsole(); 

	void Initialize();
	void Destroy();

	void BeginFrame(float deltaSeconds);
	void EndFrame();

	void Render() const; 

	void ProcessCharacter(unsigned char c);
	void ProcessKey(unsigned char k); // 
	void ClearCommandString();

	void MoveIndicatorLeft();
	void MoveIndicatorRight();

	void TryDeleteLastCharacter();
	void TryDeleteNextCharacter();
	void DeleteRange();

	void InsertCharacter(unsigned char c);

	void CycleHistoryBackward();
	void CycleHistoryForward();

	bool ShouldSelect() const;
	bool IsCurrentlySelecting() const; 
	void ClearSelection();
	void UpdateSelectionRange(int previousIndicatorPosition);

	void ClearLog();
	void PrintToLog(const RGBA& color, const std::string& string);
	bool SaveToFile(const std::string& filename) const;

	bool IsOpen() const;
	void Open(); 
	void Close();
	bool WasJustClosed() const; // For when we want to use Esc to close the console and the game.

	void AddThreadedEntry(const std::string& entry);

private:
	void ExecuteCommand();
	bool ShouldDrawPositionIndicator() const;

	bool						m_isOpen;
	bool						m_justClosed;
	std::string					m_commandString;
	std::deque<LogEntry>		m_outputLog;

	ThreadSafeQueue<LogEntry>	m_threadedLogQueue;

	int	m_positionIndicatorPosition;
	int m_selectionStart;
	int m_selectionEnd;

	ConsoleMode m_mode;

	int m_historyIndex;



	// Rendering
	Camera*				m_camera;
	const BitmapFont*	m_font;
	float				m_fontHeight;
	float				m_fontAspect;

	Vector2 m_inputAreaPadding;
	Vector2 m_logAreaPadding;

	const Texture*	m_positionIndicator;
	float			m_positionIndicatorLifetime;

	const Texture*	m_backgroundTexture;
};



// Should add a line of coloured text to the output 
void ConsolePrintf_Command( const RGBA& color, const char* format, ... ); 

// Same as previous, be defaults to a color visible easily on your console
void ConsolePrintf_Command( const char* format, ... ); 