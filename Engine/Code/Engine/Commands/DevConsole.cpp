#include "Engine/Commands/DevConsole.hpp"

#include <cstdarg>
#include <fstream>

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Commands/Command.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Rendering/Camera.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

#define WIN32_LEAN_AND_MEAN	
#include <windows.h>

extern Window*		g_theWindow;
extern DevConsole*	g_theDevConsole;
extern Renderer*	g_theRenderer;
extern InputSystem* g_theInput;



void HandleDevConsoleInput( unsigned int msg, size_t wparam, size_t lparam )
{
	UNUSED(lparam);

	// Short Circuit
	if (!g_theDevConsole->IsOpen())
	{
		return;
	}

	if (msg == WM_CHAR)
	{
		unsigned char asChar = (unsigned char)wparam;
		g_theDevConsole->ProcessCharacter(asChar);
	}

	if (msg == WM_KEYDOWN) 
	{
		unsigned char asKey = (unsigned char)wparam;
		g_theDevConsole->ProcessKey(asKey);
	}
}



void DevConsoleClear_Command(Command& cmd)
{
	UNUSED(cmd);

	g_theDevConsole->ClearLog();
}



void DevConsoleHelp_Command(Command& cmd)
{
	UNUSED(cmd);

	std::vector<std::string> commandNames = GetAllCommandNames();
	for (size_t i = 0; i < commandNames.size(); ++i)
	{
		g_theDevConsole->PrintToLog(RGBA(), commandNames[i]);
	}
}



void DevConsoleEchoWithColor_Command(Command& cmd)
{
	RGBA color;
	std::string text;

	color.SetFromText(cmd.GetNextString().c_str());
	text = cmd.GetNextString();

	g_theDevConsole->PrintToLog(color, text);
}



void DevConsoleSaveLog_Command(Command& cmd)
{
	std::string filename = cmd.GetNextString();
	if (filename.empty())
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "A file must be specified to save the log");
		return;
	}
	if (FindFirstCharacterInString(filename.c_str(), ".") == nullptr)
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "A file extension must be specified to save the log");
		return;
	}

	bool success = g_theDevConsole->SaveToFile(filename);
	if (success)
	{
		g_theDevConsole->PrintToLog(RGBA(0,255,0), Stringf("Successfully saved log to file \"%s\"", filename.c_str()));
	}
	else
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), Stringf("Could not save log to file \"%s\"", filename.c_str()));
	}
}



void DevConsoleRunScriptFromString_Command(Command& cmd)
{
	// Totally not jank way of avoiding having to put the "script" in parens
	std::string scriptString = "";
	std::string scriptSection = cmd.GetNextString();
	while (!scriptSection.empty())
	{
		scriptString += scriptSection;
		scriptString += " ";
		scriptSection = cmd.GetNextString();
	}
	if (scriptString.empty())
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "A script string must be specified to parse from");
		return;
	}

	char* editableScriptString = CopyString(scriptString);
	std::list<char*> commands = TokenizeString(editableScriptString, ";\n");
	for (auto iter = commands.begin(); iter != commands.end(); ++iter)
	{
		bool success = RunCommand(*iter);
		if (!success)
		{
			g_theDevConsole->PrintToLog(RGBA(255,0,0), "INVALID COMMAND");
		}
	}

	g_theDevConsole->PrintToLog(RGBA(0,255,0), "Successfully parsed commands");
}



void DevConsoleRunScriptFromFile_Command(Command& cmd)
{
	std::string filename = cmd.GetNextString();
	if (filename.empty())
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "A file must be specified to read from");
		return;
	}

	bool success = false;
	std::string command;
	std::ifstream scriptFile;
	scriptFile.open(filename);
	if (scriptFile.is_open())
	{
		while ( getline (scriptFile, command) )
		{
			if (scriptFile.fail())
			{
				g_theDevConsole->PrintToLog(RGBA(255,0,0), "Could not read command");
			}
			else
			{
				RunCommand(command.c_str());
			}
		}

		scriptFile.close();
		success = true;
	}
	else
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), Stringf("Could not read commands from \"%s\"", filename.c_str()));
	}

	if (success)
	{
		g_theDevConsole->PrintToLog(RGBA(0,255,0), Stringf("Successfully read commands from file \"%s\"", filename.c_str()));
	}
}



DevConsole::DevConsole()
{

}



DevConsole::~DevConsole()
{

}



void DevConsole::Initialize()
{
	g_theWindow->RegisterCallback(HandleDevConsoleInput);

	m_isOpen = false;
	m_justClosed = false;


	m_camera = new Camera();
	m_camera->SetProjection(Matrix4::MakeOrtho2D(Vector2(0.0f, 0.0f), (Vector2)g_theWindow->GetDimensions()));
	m_camera->SetColorTarget(g_theRenderer->GetDefaultColorTarget());
	m_camera->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());

	m_font = BitmapFont::CreateOrGet("Data/Fonts/SquirrelFixedFont.png");
	m_fontHeight = 20.0f;
	m_fontAspect = 1.0f;

	m_inputAreaPadding = Vector2(16.0f, 16.0f);
	m_logAreaPadding = Vector2(16.0f, 4.0f);

	m_positionIndicator = Texture::CreateOrGet("Data/Fonts/TextPositionIndicator.png");
	m_positionIndicatorLifetime = 0.0f;

	m_positionIndicatorPosition = 0;
	m_selectionStart = m_positionIndicatorPosition;
	m_selectionEnd = m_positionIndicatorPosition;

	m_mode = CONSOLE_MODE_BASIC;

	m_backgroundTexture = Texture::CreateOrGet("Data/BuiltIns/DEFAULT_TEXTURE.png");

	GUARANTEE_OR_DIE(m_font != nullptr, "DevConsole: Could not find font");
	GUARANTEE_OR_DIE(m_positionIndicator != nullptr, "DevConsole: Could not find Data/Fonts/TextPositonIndicator.png");
	GUARANTEE_OR_DIE(m_backgroundTexture != nullptr, "DevConsole: Could not find Data/Images/Default.png");

	RegisterCommand("Clear", DevConsoleClear_Command);
	RegisterCommand("Help", DevConsoleHelp_Command);
	RegisterCommand("EchoWithColor", DevConsoleEchoWithColor_Command);
	RegisterCommand("SaveLog", DevConsoleSaveLog_Command);
	RegisterCommand("RunScriptFile", DevConsoleRunScriptFromFile_Command);
	RegisterCommand("RunScriptString", DevConsoleRunScriptFromString_Command);
}



void DevConsole::Destroy()
{

}



void DevConsole::BeginFrame(float deltaSeconds)
{
	m_positionIndicatorLifetime += deltaSeconds;
}



void DevConsole::EndFrame()
{
	//if (IsOpen())
	//{
	//	Render();
	//}

	m_justClosed = false;
}


#include "Engine/Rendering/Shader.hpp"
void DevConsole::Render() const
{
	IntVector2 windowDim = g_theWindow->GetDimensions();

	// Setup camera
	g_theRenderer->BindCamera(m_camera);
	g_theRenderer->ClearDepth();
	
	TODO("DevConsole is missing a shader");
	//g_theRenderer->SetShader(nullptr);
	Shader* ds = Shader::Get("Default_2D");
	g_theRenderer->BindShader(ds);

	// Draw translucent background covering the whole screen
	//g_theRenderer->BindDiffuseTexture(m_backgroundTexture);
	g_theRenderer->BindTexture(Renderer::DIFFUSE_TEXTURE_BIND_POINT, m_backgroundTexture);
	Mesh* bg = CreateAABB2D(AABB2D(Vector2(0.0f, 0.0f), (Vector2)windowDim), RGBA(0,0,0,64));
	g_theRenderer->DrawMesh(bg->GetSubMesh(0));
	delete bg;

	// Input Area
	AABB2D inputAreaBounds = AABB2D(Vector2(0.0f, 0.0f), Vector2((float)windowDim.x, m_fontHeight + (m_inputAreaPadding.y * 2.0f)));
	//g_theRenderer->BindDiffuseTexture(m_backgroundTexture);
	g_theRenderer->BindTexture(Renderer::DIFFUSE_TEXTURE_BIND_POINT, m_backgroundTexture);
	Mesh* inputAreaMesh = CreateAABB2D(inputAreaBounds, RGBA(0,0,0));
	g_theRenderer->DrawMesh(inputAreaMesh->GetSubMesh(0));
	delete inputAreaMesh;

	AABB2D inputAreaPaddedBounds = inputAreaBounds;
	inputAreaPaddedBounds.AddPaddingToSides(-m_inputAreaPadding.x, -m_inputAreaPadding.y);
	g_theRenderer->DrawTextInBox2D(inputAreaPaddedBounds, Stringf("> %s", m_commandString.c_str()), m_fontHeight, RGBA(), m_fontAspect, m_font, Vector2(0.0f, 0.0f), TEXT_DRAW_OVERRUN);

	// Selection
	if (IsCurrentlySelecting())
	{
		float selectionStart = (float)(2 + m_selectionStart) * m_fontHeight * m_fontAspect; // NOTE: If we ever support non mono space fonts this will break 
		float selectionWidth = (float)(m_selectionEnd - m_selectionStart) * m_fontHeight * m_fontAspect;
		Vector2 mins = Vector2(inputAreaPaddedBounds.mins.x, inputAreaPaddedBounds.mins.y - 4) + Vector2(selectionStart, 0.0f);
		Vector2 maxs = mins + Vector2(selectionWidth, m_fontHeight + 12);
		AABB2D selectionBounds = AABB2D(mins, maxs);
		//g_theRenderer->BindDiffuseTexture(m_backgroundTexture);
		g_theRenderer->BindTexture(Renderer::DIFFUSE_TEXTURE_BIND_POINT, m_backgroundTexture);
		Mesh* selectionHighlightBox = CreateAABB2D(selectionBounds, RGBA(128, 128, 128, 128));
		g_theRenderer->DrawMesh(selectionHighlightBox->GetSubMesh(0));
		delete selectionHighlightBox;
	}

	// Position Indicator
	if (ShouldDrawPositionIndicator())
	{
		float commandStringWidth = (float)(2 + m_positionIndicatorPosition) * m_fontHeight * m_fontAspect; // NOTE: If we ever support non mono space fonts this will break 
		Vector2 mins = inputAreaPaddedBounds.mins + Vector2(commandStringWidth, 0.0f);
		Vector2 maxs = mins + Vector2(m_fontHeight * m_fontAspect, m_fontHeight);
		AABB2D positionIndicatorBounds = AABB2D(mins, maxs);
		//g_theRenderer->BindDiffuseTexture(m_positionIndicator);
		g_theRenderer->BindTexture(Renderer::DIFFUSE_TEXTURE_BIND_POINT, m_positionIndicator);
		Mesh* positionIndicatorMesh = CreateAABB2D(positionIndicatorBounds);
		g_theRenderer->DrawMesh(positionIndicatorMesh->GetSubMesh(0));
		delete positionIndicatorMesh;
	}

	// Log Area
	AABB2D logLineBounds = AABB2D(Vector2(0.0f, inputAreaBounds.maxs.y), Vector2((float)windowDim.x, m_fontHeight + (m_logAreaPadding.y * 2.0f)));
	logLineBounds.AddPaddingToSides(-m_inputAreaPadding.x, -m_logAreaPadding.y);
	for (size_t i = 0; i < m_outputLog.size(); ++i)
	{
		g_theRenderer->DrawTextInBox2D(logLineBounds, m_outputLog[i].m_text, m_fontHeight, m_outputLog[i].m_drawColor, m_fontAspect, m_font, Vector2(0.0f, 0.0f), TEXT_DRAW_OVERRUN);

		float yIncrement = (m_fontHeight + (2.0f * m_logAreaPadding.y));
		logLineBounds.mins.y += yIncrement; 
		logLineBounds.maxs.y += yIncrement;
	}
}


void DevConsole::ProcessCharacter(unsigned char c)
{
	switch(c)
	{
	case '\r':
	{
		if (m_mode == CONSOLE_MODE_HISTORY)
		{
			m_mode = CONSOLE_MODE_BASIC;
		}

		ExecuteCommand();
		break;
	}

	case '\b':
	{
		if (IsCurrentlySelecting())
		{
			DeleteRange();
		}
		else
		{
			TryDeleteLastCharacter();
		}
		break;
	}

	case VK_ESCAPE:
	{
		if (IsCurrentlySelecting())
		{
			ClearSelection();
		}
		else if (m_mode == CONSOLE_MODE_HISTORY)
		{
			m_mode = CONSOLE_MODE_BASIC;
			ClearCommandString();
		}
		else if (m_commandString.empty())
		{
			Close();
		}
		else
		{
			ClearCommandString();
		}
		break;
	}

	default:
	{
		if (IsCurrentlySelecting())
		{
			DeleteRange();
		}
		InsertCharacter(c);
		break;
	}
	}
}


// Some WM_CHAR chars are the same as WM_KEYDOWN keys
//	ie: WM_CHAR '.' == 46 and WM_KEYDOWN VK_DELETE == 46 so they should be treated differently
void DevConsole::ProcessKey(unsigned char k)
{
	switch(k)
	{
	case VK_LEFT:
	{
		MoveIndicatorLeft();
		break;
	}

	case VK_RIGHT:
	{
		MoveIndicatorRight();
		break;
	}

	case VK_UP:
	{
		CycleHistoryBackward();
		break;
	}

	case VK_DOWN:
	{
		CycleHistoryForward();
		break;
	}

	case VK_DELETE:
	{
		if (IsCurrentlySelecting())
		{
			DeleteRange();
		}
		else
		{
			TryDeleteNextCharacter();
		}
		break;
	}

	default:
		break;
	}
}



void DevConsole::ClearCommandString()
{
	m_commandString = "";
	m_positionIndicatorPosition = 0;
}


void DevConsole::MoveIndicatorLeft()
{
	if (!ShouldSelect() && IsCurrentlySelecting())
	{
		m_positionIndicatorPosition = m_selectionStart;
		ClearSelection();
	}
	else 
	{
		int previousIndicationPosition = m_positionIndicatorPosition--;
		m_positionIndicatorPosition = ClampInt(m_positionIndicatorPosition, 0, (int)m_commandString.length());
		UpdateSelectionRange(previousIndicationPosition);
	}
}



void DevConsole::MoveIndicatorRight()
{
	if (!ShouldSelect() && IsCurrentlySelecting())
	{
		m_positionIndicatorPosition = m_selectionEnd;
		ClearSelection();
	}
	else
	{
		int previousIndicationPosition = m_positionIndicatorPosition++;
		m_positionIndicatorPosition = ClampInt(m_positionIndicatorPosition, 0, (int)m_commandString.length());
		UpdateSelectionRange(previousIndicationPosition);
	}
}



void DevConsole::TryDeleteLastCharacter()
{
	if (!m_commandString.empty() && m_positionIndicatorPosition != 0)
	{
		m_commandString.erase(m_positionIndicatorPosition - 1, 1);
		--m_positionIndicatorPosition;
	}
}



void DevConsole::TryDeleteNextCharacter()
{
	if (m_positionIndicatorPosition + 1 <= (int)m_commandString.length())
	{
		m_commandString.erase(m_positionIndicatorPosition, 1);
		m_positionIndicatorPosition = ClampInt(m_positionIndicatorPosition, 0, (int)m_commandString.length());
	}
}



void DevConsole::DeleteRange()
{
	m_commandString.erase(m_selectionStart, (m_selectionEnd - m_selectionStart));
	m_positionIndicatorPosition = m_selectionStart;
	ClearSelection();
}



void DevConsole::InsertCharacter(unsigned char c)
{
	m_commandString.insert(m_positionIndicatorPosition, 1, c);
	++m_positionIndicatorPosition;
}



void DevConsole::CycleHistoryBackward()
{
	ClearSelection();
	std::deque<std::string> history = GetCommandHistory();

	// Short Circuit
	if (history.size() == 0)
	{
		return;
	}

	if (m_mode != CONSOLE_MODE_HISTORY)
	{
		m_mode = CONSOLE_MODE_HISTORY;
		m_historyIndex = 0;
	}
	else
	{
		++m_historyIndex;
	}

	if (m_historyIndex >= (int)history.size())
	{
		m_historyIndex = 0;
	}
	m_commandString = history[m_historyIndex];
	m_positionIndicatorPosition = (int)m_commandString.length();
}



void DevConsole::CycleHistoryForward()
{
	ClearSelection();
	std::deque<std::string> history = GetCommandHistory();

	// Short Circuit
	if (history.size() == 0)
	{
		return;
	}

	if (m_mode != CONSOLE_MODE_HISTORY)
	{
		m_mode = CONSOLE_MODE_HISTORY;
		m_historyIndex = -1;
	}
	else
	{
		--m_historyIndex;
	}

	if (m_historyIndex < 0)
	{
		m_historyIndex = (int)history.size() - 1;
	}
	m_commandString = history[m_historyIndex];
	m_positionIndicatorPosition = (int)m_commandString.length();
}



bool DevConsole::ShouldSelect() const
{
	bool isSelecting = false;

	if (g_theInput->IsKeyPressed(InputSystem::KEY_SHIFT))
	{
		isSelecting = true;
	}

	return isSelecting;
}



bool DevConsole::IsCurrentlySelecting() const
{
	bool isSelecting = false;

	if (m_selectionStart != m_selectionEnd)
	{
		isSelecting = true;
	}

	return isSelecting;
}



void DevConsole::ClearSelection()
{
	m_selectionStart = 0;
	m_selectionEnd = 0;
}



void DevConsole::UpdateSelectionRange(int previousIndicatorPosition)
{
	bool moveLeft = (m_positionIndicatorPosition < previousIndicatorPosition);

	if (ShouldSelect())
	{
		if (IsCurrentlySelecting())
		{
			if (previousIndicatorPosition == m_selectionStart)
			{
				m_selectionStart = m_positionIndicatorPosition;
			}
			else
			{
				m_selectionEnd = m_positionIndicatorPosition;
			}
		}
		else
		{
			if (moveLeft)
			{
				m_selectionStart = m_positionIndicatorPosition;
				m_selectionEnd   = previousIndicatorPosition;
			}
			else
			{
				m_selectionStart = previousIndicatorPosition;
				m_selectionEnd   = m_positionIndicatorPosition;
			}
		}
	}
	else
	{
		ClearSelection();
	}
	DebuggerPrintf("SelectionRange: (%i, %i)   PIP:%i\n", m_selectionStart, m_selectionEnd, m_positionIndicatorPosition);
}



void DevConsole::PrintToLog(const RGBA& color, const std::string& string)
{
	m_outputLog.push_front(LogEntry(color, string));
}



void DevConsole::ClearLog()
{
	m_outputLog.clear();
}



bool DevConsole::SaveToFile(const std::string& filename) const
{
	bool success = false;

	std::ofstream logFile;
	logFile.open(filename, std::ofstream::out | std::ofstream::trunc);
	if (logFile.is_open())
	{
		for (int i = (int)m_outputLog.size() - 1; i >= 0; --i)
		{
			logFile << m_outputLog[i].m_text << std::endl;
		}
		
		if (!logFile.fail())
		{
			success = true;
		}
		logFile.close();
	}

	return success;
}



void DevConsole::ExecuteCommand()
{
	if (!m_commandString.empty())
	{
		bool success = RunCommand(m_commandString.c_str());
		if (!success)
		{
			PrintToLog(RGBA(255,0,0), "INVALID COMMAND");
		}

		ClearCommandString();
	}
}



bool DevConsole::ShouldDrawPositionIndicator() const
{
	bool shouldDraw = false;

	float remainder = m_positionIndicatorLifetime - (int)m_positionIndicatorLifetime;
	if (remainder <= 0.5f)
	{
		shouldDraw = true;
	}

	return shouldDraw;
}



bool DevConsole::IsOpen() const
{
	return m_isOpen;
}



void DevConsole::Open()
{
	m_isOpen = true;
	ClearCommandString();
}



void DevConsole::Close()
{
	m_isOpen = false;
	m_justClosed = true;
	ClearCommandString();
}



bool DevConsole::WasJustClosed() const
{
	return m_justClosed;
}



void ConsolePrintf_Command( const RGBA& color, const char* format, ... )
{
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	std::string formattedString = Stringf(format, variableArgumentList);
	va_end(variableArgumentList);
	g_theDevConsole->PrintToLog(color, formattedString);
}



void ConsolePrintf_Command( const char* format, ... )
{
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	std::string formattedString = Stringf(format, variableArgumentList);
	va_end(variableArgumentList);
	g_theDevConsole->PrintToLog(RGBA(), formattedString);
}
