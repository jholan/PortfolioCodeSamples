#pragma once
#include "Engine/Core/Window.hpp"

#define WIN32_LEAN_AND_MEAN	
#include <windows.h>

#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/IntVector2.hpp"

typedef void (*windows_message_handler_cb)( unsigned int msg, size_t wparam, size_t lparam ); 

extern Window* g_theWindow;



LRESULT CALLBACK GameWndProc(HWND windowHandle, UINT messageCode, WPARAM wParam, LPARAM lParam)
{
	// NEW:  Give the custom handlers a chance to run first; 
	if (g_theWindow != nullptr) 
	{
		const std::vector<windows_message_handler_cb>& callbackFunctions = g_theWindow->GetCallbackFunctions();
		for (size_t i = 0; i < callbackFunctions.size(); ++i) 
		{
			callbackFunctions[i]( messageCode, wParam, lParam ); 
		}
	}

	// do default windows behaviour (return before this if you don't want default windows behaviour for certain messages)
	return DefWindowProc( windowHandle, messageCode, wParam, lParam );
}



IntVector2 GetDimensionsFromAspectRatio(float aspectRatio)
{
	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 0.90f;
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;
	if(aspectRatio > desktopAspect)
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / aspectRatio;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * aspectRatio;
	}

	return IntVector2((int)clientWidth, (int)clientHeight);
}



HWND CreateOpenGLWindowFromDimensions(HINSTANCE applicationInstanceHandle, int width, int height, WNDPROC messageHandlingFunction)
{
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = messageHandlingFunction; // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	// #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);

	float clientWidth = (float)width;
	float clientHeight = (float)height;

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	const char* APP_NAME = "Triangle";
	MultiByteToWideChar(GetACP(), 0, APP_NAME, -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	HWND hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL);

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);


	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);

	return hWnd;
}



Window::Window()
{
}



Window::~Window()
{
}



void Window::Initialize(int width, int height)
{
	m_windowHandle = CreateOpenGLWindowFromDimensions(GetModuleHandle(NULL), width, height, GameWndProc);
	m_dimensions = IntVector2(width, height);
}



void Window::Initialize(float aspectRatio)
{
	IntVector2 dimensions = GetDimensionsFromAspectRatio(aspectRatio);
	Initialize(dimensions.x, dimensions.y);
}



void Window::Destroy()
{

}



void Window::RegisterCallback( windows_message_handler_cb cb )
{
	bool alreadyAdded = false;
	for (size_t i = 0; i < m_callbackFunctions.size(); ++i)
	{
		if (m_callbackFunctions[i] == cb)
		{
			alreadyAdded = true;
			break;
		}
	}

	if (!alreadyAdded)
	{
		m_callbackFunctions.push_back(cb);
	}
}



void Window::UnregisterCallback( windows_message_handler_cb cb )
{
	for (size_t i = 0; i < m_callbackFunctions.size(); ++i)
	{
		if (m_callbackFunctions[i] == cb)
		{
			m_callbackFunctions.erase(m_callbackFunctions.begin() + i);
			break;
		}
	}
}



const std::vector<windows_message_handler_cb>& Window::GetCallbackFunctions() const
{
	return m_callbackFunctions;
}



void* Window::GetHandle() const
{ 
	return m_windowHandle; 
}



IntVector2 Window::GetDimensions() const
{
	return m_dimensions;
}



IntVector2 Window::GetCenter() const
{
	return m_dimensions / 2;
}



void Window::SetFocusState(bool state)
{
	m_hasFocus = state;
}



bool Window::HasFocus() const
{
	return m_hasFocus;
}
