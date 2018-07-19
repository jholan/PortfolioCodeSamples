#pragma once

#include "Engine/Math/IntVector2.hpp"

#include "Engine/Input/XboxController.hpp"

enum MouseMode
{
	MOUSE_MODE_ABSOLUTE,
	MOUSE_MODE_RELATIVE
};


class InputSystem
{
public:
	// Lifetime
	InputSystem() {};  // Does nothing
	~InputSystem() {}; // Does nothing

	void		Initialize();
	void		Destroy();

	// Owner Interface
	void		BeginFrame();
	void		EndFrame();

	void		SetKeyState(unsigned char key, bool state);
	void		IncrementWheelDelta(float delta);
	void		SetMouseButtonState(unsigned char button, bool state);
	void		SetMouseButtonStates(size_t mousestate);
	void		SetControllerStates();

	// User Interface
	bool		IsKeyPressed(unsigned char key) const;
	bool		WasKeyJustPressed(unsigned char key) const;
	bool		WasKeyJustReleased(unsigned char key) const;

	bool		IsMouseVisible() const;
	void		SetMouseVisiblity(bool visiblity);

	MouseMode   GetMouseMode() const;
	void		SetMouseMode(MouseMode mode);

	bool		IsMouseLockedToScreen() const;
	void		LockMouseToScreen(bool lock);

	int			GetMouseWheelDelta() const;

	IntVector2  GetMouseDelta() const;
	IntVector2	GetMousePosition() const;
	void		SetMousePosition(const IntVector2& position);

	bool		IsMouseButtonPressed(unsigned char button) const;
	bool		WasMouseButtonJustPressed(unsigned char button) const;
	bool		WasMouseButtonJustReleased(unsigned char button) const;	

	XboxController& GetController(XboxControllerNumber number);


	// Keys
	static const unsigned char KEY_UP;
	static const unsigned char KEY_LEFT;
	static const unsigned char KEY_RIGHT;
	static const unsigned char KEY_DOWN;

	static const unsigned char KEY_F1;
	static const unsigned char KEY_F2;
	static const unsigned char KEY_F3;
	static const unsigned char KEY_F4;
	static const unsigned char KEY_F5;
	static const unsigned char KEY_F6;
	static const unsigned char KEY_F7;
	static const unsigned char KEY_F8;
	static const unsigned char KEY_F9;
	static const unsigned char KEY_F10;
	static const unsigned char KEY_F11;
	static const unsigned char KEY_F12;

	static const unsigned char KEY_SHIFT;

	static const unsigned char KEY_ESCAPE;
	static const unsigned char KEY_ENTER;
	static const unsigned char KEY_TILDE;
	static const unsigned char KEY_DELETE;
	static const int		   KEYS_COUNT = 255;


	// Mouse Buttons
	static const unsigned char MOUSE_LEFT;
	static const unsigned char MOUSE_RIGHT;
	static const int		   MOUSE_COUNT = 2;


private:
	void RunMessagePump();

	bool	   m_inputBuffer[KEYS_COUNT];
	bool	   m_inputBackBuffer[KEYS_COUNT];
			   
	bool	   m_mouseBuffer[MOUSE_COUNT];
	bool	   m_mouseBackBuffer[MOUSE_COUNT];

	MouseMode  m_mouseMode = MOUSE_MODE_ABSOLUTE;

	IntVector2 m_mousePositionThisFrame;
	IntVector2 m_mousePositionLastFrame;

	float	   m_mouseWheelDelta;
			   
	bool	   m_isMouseVisible = true;
	bool	   m_isMouseLockedToScreen = false;

	XboxController m_xboxControllers[CONTROLLER_COUNT];
};