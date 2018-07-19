#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"


//-------------------------------------------------------------------
AnalogJoystick::AnalogJoystick()
{
	SetFromRawValues(0, 0);
}


//-------------------------------------------------------------------
AnalogJoystick::~AnalogJoystick()
{
}


//-------------------------------------------------------------------
void AnalogJoystick::SetFromRawValues(short xValue, short yValue)
{
	m_rawCoordinates = Vector2(RangeMapFloat(xValue, SHORT_MIN, SHORT_MAX, -1.0f, 1.0f),
								RangeMapFloat(yValue, SHORT_MIN, SHORT_MAX, -1.0f, 1.0f));
	Vector2 normalizedRawCoords = m_rawCoordinates.GetNormalized();

	float rawRadius = m_rawCoordinates.GetLength();
	m_angleDegrees = ATan2Degrees(normalizedRawCoords.y, normalizedRawCoords.x);

	float correctedRadius = RangeMapFloat(rawRadius, 0.3f, 0.9f, 0.0f, 1.0f);
	correctedRadius = ClampFloatZeroToOne(correctedRadius);
	m_radius = correctedRadius;

	m_correctedCoordinates = Vector2(correctedRadius * CosDegrees(m_angleDegrees), correctedRadius * SinDegrees(m_angleDegrees));
}


//-------------------------------------------------------------------
Vector2 AnalogJoystick::GetNormalizedRawCoordinates() const
{
	return m_rawCoordinates;
}


//-------------------------------------------------------------------
Vector2 AnalogJoystick::GetNormalizedDeadZoneCorrectedCoordinates() const
{
	return m_correctedCoordinates;
}


//-------------------------------------------------------------------
float AnalogJoystick::GetRadius() const
{
	return m_radius;
}


//-------------------------------------------------------------------
float AnalogJoystick::GetAngleDegrees() const
{
	return m_angleDegrees;
}


//-------------------------------------------------------------------
const float AnalogJoystick::INNER_DEAD_ZONE = 0.20f;
const float AnalogJoystick::OUTER_DEAD_ZONE = 0.95f;
const float AnalogJoystick::SHORT_MAX = 32767.0f;
const float AnalogJoystick::SHORT_MIN = -32768.0f;




//-------------------------------------------------------------------
//XBOX-CONTROLLER----------------------------------------------------
//-------------------------------------------------------------------
void XboxController::Initialize(XboxControllerNumber controllerNumber)
{
	m_isConnected = false;
	m_controllerNumber = controllerNumber;

	for(int i = 0; i < BUTTON_COUNT; ++i)
	{
		m_buttonFrontBuffer[i] = false;
		m_buttonBackBuffer[i] = false;
	}

	m_normalizedLeftTrigger = 0.0f;
	m_normalizedRightTrigger = 0.0f;

	m_leftJoystick = AnalogJoystick();
	m_rightJoystick = AnalogJoystick();
}


//-------------------------------------------------------------------
void XboxController::Destroy()
{
	m_leftJoystick = AnalogJoystick();
	m_rightJoystick = AnalogJoystick();

	for(int i = 0; i < BUTTON_COUNT; ++i)
	{
		m_buttonFrontBuffer[i] = false;
		m_buttonBackBuffer[i] = false;
	}

	m_normalizedLeftTrigger = 0.0f;
	m_normalizedRightTrigger = 0.0f;

	m_controllerNumber = CONTROLLER_INVALID;
	m_isConnected = false;
}


//-------------------------------------------------------------------
void XboxController::Update()
{
	for(int i = 0; i < BUTTON_COUNT; ++i)
	{
		m_buttonBackBuffer[i] = m_buttonFrontBuffer[i];
	}
}


//-------------------------------------------------------------------
void XboxController::SetButtonState(unsigned char button, bool state)
{
	m_buttonFrontBuffer[button] = state;
}


//-------------------------------------------------------------------
void XboxController::SetLeftTrigger(unsigned char value)
{
	float normalizedValue = (float)value / 255.0f;
	m_normalizedLeftTrigger = normalizedValue;
}


//-------------------------------------------------------------------
void XboxController::SetRightTrigger(unsigned char value)
{
	float normalizedValue = (float)value / 255.0f;
	m_normalizedRightTrigger = normalizedValue;
}


//-------------------------------------------------------------------
void XboxController::SetLeftJoystick(short xValue, short yValue)
{
	m_leftJoystick.SetFromRawValues(xValue, yValue);
}


//-------------------------------------------------------------------
void XboxController::SetRightJoystick(short xValue, short yValue)
{
	m_rightJoystick.SetFromRawValues(xValue, yValue);
}


//-------------------------------------------------------------------
bool XboxController::IsConnected() const
{
	return m_isConnected;
}


//-------------------------------------------------------------------
void XboxController::Connect()
{
	m_isConnected = true;
}


//-------------------------------------------------------------------
void XboxController::Disconnect()
{
	m_isConnected = false;
}


//-------------------------------------------------------------------
XboxControllerNumber XboxController::GetControllerNumber() const
{
	return m_controllerNumber;
}


//-------------------------------------------------------------------
bool XboxController::IsButtonPressed(unsigned char button) const
{
	bool pressed = m_buttonFrontBuffer[button];
	return pressed;
}


//-------------------------------------------------------------------
bool XboxController::WasButtonJustPressed(unsigned char button) const
{
	bool justPressed = m_buttonFrontBuffer[button] && !m_buttonBackBuffer[button];
	return justPressed;
}


//-------------------------------------------------------------------
bool XboxController::WasButtonJustReleased(unsigned char button) const
{
	bool justReleased = !m_buttonFrontBuffer[button] && m_buttonBackBuffer[button];
	return justReleased;
}


//-------------------------------------------------------------------
float XboxController::GetLeftTrigger() const
{
	return m_normalizedLeftTrigger;
}


//-------------------------------------------------------------------
float XboxController::GetRightTrigger() const
{
	return m_normalizedRightTrigger;
}


//-------------------------------------------------------------------
const AnalogJoystick& XboxController::GetLeftJoystick() const
{
	return m_leftJoystick;
}


//-------------------------------------------------------------------
const AnalogJoystick& XboxController::GetRightJoystick() const
{
	return m_rightJoystick;
}


//-------------------------------------------------------------------
const unsigned char XboxController::BUTTON_UP = 0;
const unsigned char XboxController::BUTTON_DOWN = 1;
const unsigned char XboxController::BUTTON_LEFT = 2;
const unsigned char XboxController::BUTTON_RIGHT = 3;
const unsigned char XboxController::BUTTON_START = 4;
const unsigned char XboxController::BUTTON_BACK = 5;
const unsigned char XboxController::BUTTON_L3 = 6;
const unsigned char XboxController::BUTTON_R3 = 7;
const unsigned char XboxController::BUTTON_LB = 8;
const unsigned char XboxController::BUTTON_RB = 9;
const unsigned char XboxController::BUTTON_A = 10;
const unsigned char XboxController::BUTTON_B = 11;
const unsigned char XboxController::BUTTON_X = 12;
const unsigned char XboxController::BUTTON_Y = 13;
