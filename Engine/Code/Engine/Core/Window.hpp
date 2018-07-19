#pragma once

#include <vector>

#include "Engine/Math/IntVector2.hpp"

typedef void (*windows_message_handler_cb)( unsigned int msg, size_t wparam, size_t lparam ); 



class Window
{
public:
	// Do all the window creation work is currently in App
	Window(); 
	~Window();

	void Initialize( int width, int height );
	void Initialize( float aspectRatio );
	void Destroy();

	// Callbacks
	void RegisterCallback( windows_message_handler_cb cb ); 
	void UnregisterCallback( windows_message_handler_cb cb ); 
	const std::vector<windows_message_handler_cb>& GetCallbackFunctions() const;

	// This is safely castable to an HWND
	void* GetHandle() const;

	IntVector2 GetDimensions() const;
	IntVector2 GetCenter() const;

	void SetFocusState(bool state);
	bool HasFocus() const;

private:
	void*	   m_windowHandle; // intptr_t  
	IntVector2 m_dimensions;

	bool		m_hasFocus = true;

	// When the WindowsProcedure is called - let all listeners get first crack at the message
	// Giving us better code modularity. 
	std::vector<windows_message_handler_cb> m_callbackFunctions;  
};