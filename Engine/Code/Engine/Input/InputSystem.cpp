#include "Engine/Input/InputSystem.hpp"

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <Xinput.h>				// include the Xinput API
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library // #Eiserloh: Xinput 1_4 doesn't work in Windows 7; use 9_1_0 explicitly for broadest compatibility


#include "Engine/Math/MathUtils.hpp"

#include "Engine/Core/Window.hpp"
extern Window* g_theWindow;


//-------------------------------------------------------------------
void InputSystem::Initialize()
{
	// Setup clean keyboard state
	for(int i = 0; i < KEYS_COUNT; ++i)
	{
		m_inputBuffer[i] = false;
		m_inputBackBuffer[i] = false;
	}

	// Setup clean mouse state
	for(int i = 0; i < MOUSE_COUNT; ++i)
	{
		m_mouseBuffer[i] = false;
		m_mouseBackBuffer[i] = false;
	}

	for(int i = CONTROLLER_NUMBER_1; i < CONTROLLER_COUNT; ++i)
	{
		m_xboxControllers[i].Initialize((XboxControllerNumber)i);
	}
}


//-------------------------------------------------------------------
void InputSystem::Destroy()
{
	for(int i = CONTROLLER_NUMBER_1; i < CONTROLLER_COUNT; ++i)
	{
		m_xboxControllers[i].Destroy();
	}

	for(int i = 0; i < MOUSE_COUNT; ++i)
	{
		m_mouseBuffer[i] = false;
		m_mouseBackBuffer[i] = false;
	}

	for(int i = 0; i < KEYS_COUNT; ++i)
	{
		m_inputBuffer[i] = false;
		m_inputBackBuffer[i] = false;
	}
}


//-------------------------------------------------------------------
void InputSystem::BeginFrame()
{
	// Swap key buffers
	for(int i = 0; i < KEYS_COUNT; ++i)
	{
		m_inputBackBuffer[i] = m_inputBuffer[i];
	}

	// Swap mouse buffers
	for(int i = 0; i < MOUSE_COUNT; ++i)
	{
		m_mouseBackBuffer[i] = m_mouseBuffer[i];
	}

	m_mousePositionLastFrame = m_mousePositionThisFrame;
	m_mousePositionThisFrame = GetMousePosition();

	if (m_mouseMode == MOUSE_MODE_RELATIVE && g_theWindow->HasFocus()) {
		m_mousePositionLastFrame = g_theWindow->GetCenter(); 
		SetMousePosition( m_mousePositionLastFrame); 
	}

	m_mouseWheelDelta = 0;

	// Update controller internal state(swap button buffers)
	for (int i = 0; i < CONTROLLER_COUNT; ++i)
	{
		m_xboxControllers[i].Update();
	}


	// Get inputs from this frame
	RunMessagePump();
	SetControllerStates();
}


//-------------------------------------------------------------------
void InputSystem::EndFrame()
{
	// Does nothing for now
}


//-------------------------------------------------------------------
void InputSystem::SetKeyState(unsigned char key, bool state)
{
	m_inputBuffer[key] = state;
}


//-------------------------------------------------------------------
void InputSystem::IncrementWheelDelta(float delta)
{
	m_mouseWheelDelta += delta;
}


//-------------------------------------------------------------------
void InputSystem::SetMouseButtonState(unsigned char button, bool state)
{
	m_mouseBuffer[button] = state;
}


//-------------------------------------------------------------------
void InputSystem::SetMouseButtonStates(size_t mousestate)
{
	SetMouseButtonState(MOUSE_LEFT,  AreBitsSet(mousestate, MK_LBUTTON));
	SetMouseButtonState(MOUSE_RIGHT, AreBitsSet(mousestate, MK_RBUTTON));
}


//-------------------------------------------------------------------
void InputSystem::SetControllerStates()
{
	// Update controllers
	for(int i = 0; i < CONTROLLER_COUNT; ++i)
	{
		XINPUT_STATE controllerState;
		memset(&controllerState, 0, sizeof(controllerState));
		DWORD errorStatus = XInputGetState(i, &controllerState);
		XINPUT_GAMEPAD& gamepad = controllerState.Gamepad;
		if( errorStatus == ERROR_SUCCESS )
		{
			XboxController& controller = m_xboxControllers[i];

			controller.Connect();

			// Triggers
			controller.SetLeftTrigger(controllerState.Gamepad.bLeftTrigger);
			controller.SetRightTrigger(controllerState.Gamepad.bRightTrigger);

			// Joysticks
			controller.SetLeftJoystick(controllerState.Gamepad.sThumbLX, controllerState.Gamepad.sThumbLY);
			controller.SetRightJoystick(controllerState.Gamepad.sThumbRX, controllerState.Gamepad.sThumbRY);

			// Face buttons
			controller.SetButtonState(XboxController::BUTTON_A,     AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_A));
			controller.SetButtonState(XboxController::BUTTON_B,     AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_B));
			controller.SetButtonState(XboxController::BUTTON_X,     AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_X));
			controller.SetButtonState(XboxController::BUTTON_Y,     AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_Y));

			// Dpad
			controller.SetButtonState(XboxController::BUTTON_UP,    AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP));
			controller.SetButtonState(XboxController::BUTTON_DOWN,  AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN));
			controller.SetButtonState(XboxController::BUTTON_LEFT,  AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT));
			controller.SetButtonState(XboxController::BUTTON_RIGHT, AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT));

			// Bumpers
			controller.SetButtonState(XboxController::BUTTON_LB,    AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER ));
			controller.SetButtonState(XboxController::BUTTON_RB,    AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER));

			// Stick clicks
			controller.SetButtonState(XboxController::BUTTON_L3,    AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB));
			controller.SetButtonState(XboxController::BUTTON_R3,    AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB));

			// Menu buttons
			controller.SetButtonState(XboxController::BUTTON_START, AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_START));
			controller.SetButtonState(XboxController::BUTTON_BACK,  AreBitsSet(gamepad.wButtons, XINPUT_GAMEPAD_BACK));
		}
		else if( errorStatus == ERROR_DEVICE_NOT_CONNECTED )
		{
			m_xboxControllers[i].Disconnect();
		}
	}
}


//-------------------------------------------------------------------
bool InputSystem::IsKeyPressed(unsigned char key) const
{
	bool pressed = m_inputBuffer[key];
	return pressed;
}


//-------------------------------------------------------------------
bool InputSystem::WasKeyJustPressed(unsigned char key) const
{
	bool justPressed = m_inputBuffer[key] && !m_inputBackBuffer[key];
	return justPressed;
}


//-------------------------------------------------------------------
bool InputSystem::WasKeyJustReleased(unsigned char key) const
{
	bool justReleased = !m_inputBuffer[key] && m_inputBackBuffer[key];
	return justReleased;
}


//-------------------------------------------------------------------
bool InputSystem::IsMouseVisible() const
{
	return m_isMouseVisible;
}


//-------------------------------------------------------------------
void InputSystem::SetMouseVisiblity(bool visiblity)
{
	if (visiblity == m_isMouseVisible)
	{
		return;
	}

	if (visiblity == true)
	{
		ShowCursor(TRUE);
		m_isMouseVisible = true;
	}
	else
	{
		ShowCursor(FALSE);
		m_isMouseVisible = false;
	}
}


//-------------------------------------------------------------------
MouseMode InputSystem::GetMouseMode() const
{
	return m_mouseMode;
}


//-------------------------------------------------------------------
void InputSystem::SetMouseMode(MouseMode mode)
{
	m_mouseMode = mode;
}


//-------------------------------------------------------------------
bool InputSystem::IsMouseLockedToScreen() const
{
	return m_isMouseLockedToScreen;
}


//-------------------------------------------------------------------
void InputSystem::LockMouseToScreen(bool lock)
{
	if (!lock) {
		ClipCursor(nullptr); // this will unlock the mouse
		m_isMouseLockedToScreen = false;
	} else {
		HWND hwnd = GetActiveWindow();

		RECT client_rect;
		GetClientRect(hwnd, &client_rect); 

		POINT offset; 
		offset.x = 0; 
		offset.y = 0; 
		ClientToScreen(hwnd, &offset); 

		client_rect.left   += offset.x; 
		client_rect.right  += offset.x; 
		client_rect.top    += offset.y; 
		client_rect.bottom += offset.y; 

		ClipCursor(&client_rect); 
		m_isMouseLockedToScreen = true;
	}
}


//-------------------------------------------------------------------
int	InputSystem::GetMouseWheelDelta() const
{
	return (int)m_mouseWheelDelta;
}


//-------------------------------------------------------------------
IntVector2 InputSystem::GetMouseDelta() const
{
	IntVector2 delta = m_mousePositionThisFrame - m_mousePositionLastFrame;
	return delta;
}


//-------------------------------------------------------------------
IntVector2 RemapMouseY(const IntVector2& mousePosition)
{
	Vector2 mousePos = Vector2((float)mousePosition.x, (float)mousePosition.y);
	float winDim_y = (float)(g_theWindow->GetDimensions().y);
	mousePos.y = RangeMapFloat(mousePos.y, 0.0f, winDim_y, winDim_y, 0.0f);

	return IntVector2((int)mousePos.x, (int)mousePos.y);
}


//-------------------------------------------------------------------
IntVector2 InputSystem::GetMousePosition() const
{
	POINT desktopPos;
	GetCursorPos(&desktopPos);

	HWND hwnd = GetActiveWindow();
	ScreenToClient(hwnd, &desktopPos);
	POINT clientPos = desktopPos;
	
	IntVector2 rawClientPos = IntVector2(clientPos.x, clientPos.y);
	IntVector2 remappedClientPos = RemapMouseY(rawClientPos);

	return remappedClientPos;
}


//-------------------------------------------------------------------
void InputSystem::SetMousePosition(const IntVector2& position)
{
	IntVector2 remappedPosition = RemapMouseY(position);

	POINT clientPos;
	clientPos.x = remappedPosition.x;
	clientPos.y = remappedPosition.y;

	HWND hwnd = GetActiveWindow();
	ClientToScreen(hwnd, &clientPos);
	POINT desktopPos = clientPos;

	SetCursorPos(desktopPos.x, desktopPos.y);
}


//-------------------------------------------------------------------
bool InputSystem::IsMouseButtonPressed(unsigned char button) const
{
	bool pressed = m_mouseBuffer[button];
	return pressed;
}


//-------------------------------------------------------------------
bool InputSystem::WasMouseButtonJustPressed(unsigned char button) const
{
	bool justPressed = m_mouseBuffer[button] && !m_mouseBackBuffer[button];
	return justPressed;
}


//-------------------------------------------------------------------
bool InputSystem::WasMouseButtonJustReleased(unsigned char button) const
{
	bool justReleased = !m_mouseBuffer[button] && m_mouseBackBuffer[button];
	return justReleased;
}


//-------------------------------------------------------------------
XboxController& InputSystem::GetController(XboxControllerNumber number)
{
	return m_xboxControllers[number];
}


//-------------------------------------------------------------------
void InputSystem::RunMessagePump()
{
	MSG queuedMessage;
	for(;;)
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if(!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" function
	}
}


//-------------------------------------------------------------------
const unsigned char InputSystem::KEY_UP = VK_UP;
const unsigned char InputSystem::KEY_LEFT = VK_LEFT;
const unsigned char InputSystem::KEY_RIGHT = VK_RIGHT;
const unsigned char InputSystem::KEY_DOWN = VK_DOWN;

const unsigned char InputSystem::KEY_F1 = VK_F1;
const unsigned char InputSystem::KEY_F2 = VK_F2;
const unsigned char InputSystem::KEY_F3 = VK_F3;
const unsigned char InputSystem::KEY_F4 = VK_F4;
const unsigned char InputSystem::KEY_F5 = VK_F5;
const unsigned char InputSystem::KEY_F6 = VK_F6;
const unsigned char InputSystem::KEY_F7 = VK_F7;
const unsigned char InputSystem::KEY_F8 = VK_F8;
const unsigned char InputSystem::KEY_F9 = VK_F9;
const unsigned char InputSystem::KEY_F10 = VK_F10;
const unsigned char InputSystem::KEY_F11 = VK_F11;
const unsigned char InputSystem::KEY_F12 = VK_F12;
const unsigned char InputSystem::KEY_SHIFT = VK_SHIFT;
const unsigned char InputSystem::KEY_ESCAPE = VK_ESCAPE;
const unsigned char InputSystem::KEY_ENTER = VK_RETURN;
const unsigned char InputSystem::KEY_TILDE = VK_OEM_3;
const unsigned char InputSystem::KEY_DELETE = VK_DELETE;



const unsigned char InputSystem::MOUSE_LEFT = 0;
const unsigned char InputSystem::MOUSE_RIGHT = 1;