#pragma once
#include "Engine/Math/Vector2.hpp"


class AnalogJoystick
{
public:
	// Composition
	AnalogJoystick();
	~AnalogJoystick();

	// Owner Interface
	void SetFromRawValues(short xValue, short yValue);
	
	// User Interface
	Vector2 GetNormalizedRawCoordinates() const;
	Vector2 GetNormalizedDeadZoneCorrectedCoordinates() const;
	float   GetRadius() const;
	float   GetAngleDegrees() const;


private:
	Vector2 m_rawCoordinates;
	Vector2 m_correctedCoordinates;
	float m_radius;
	float m_angleDegrees;

	static const float INNER_DEAD_ZONE;
	static const float OUTER_DEAD_ZONE;
	static const float SHORT_MAX;
	static const float SHORT_MIN;
};


enum XboxControllerNumber
{
	CONTROLLER_INVALID = -1,

	CONTROLLER_NUMBER_1 = 0,
	CONTROLLER_NUMBER_2,
	CONTROLLER_NUMBER_3,
	CONTROLLER_NUMBER_4,

	CONTROLLER_COUNT
};


class XboxController
{
public:
	// Composition
	XboxController() {};  // Does nothing
	~XboxController() {}; // Does nothing

	void Initialize(XboxControllerNumber controllerNumber);
	void Destroy();

	// Owner Interface
	void Update();
	void SetButtonState(unsigned char button, bool state);
	void SetLeftTrigger(unsigned char value);
	void SetRightTrigger(unsigned char value);
	void SetLeftJoystick(short xValue, short yValue);
	void SetRightJoystick(short xValue, short yValue);

	// User Interface
	// Connection
	bool IsConnected() const;
	void Connect();
	void Disconnect();

	// Number
	XboxControllerNumber GetControllerNumber() const;

	// Button states
	bool IsButtonPressed(unsigned char button) const;
	bool WasButtonJustPressed(unsigned char button) const;
	bool WasButtonJustReleased(unsigned char button) const;

	// Trigger states
	float GetLeftTrigger() const;
	float GetRightTrigger() const;

	// Joystick states
	const AnalogJoystick& GetLeftJoystick() const;
	const AnalogJoystick& GetRightJoystick() const;


	// Buttons
	static const unsigned char BUTTON_A;
	static const unsigned char BUTTON_B;
	static const unsigned char BUTTON_X;
	static const unsigned char BUTTON_Y;

	static const unsigned char BUTTON_UP;
	static const unsigned char BUTTON_DOWN;
	static const unsigned char BUTTON_LEFT;
	static const unsigned char BUTTON_RIGHT;

	static const unsigned char BUTTON_LB;
	static const unsigned char BUTTON_RB;

	static const unsigned char BUTTON_L3;
	static const unsigned char BUTTON_R3;

	static const unsigned char BUTTON_START;
	static const unsigned char BUTTON_BACK;
	static const int		   BUTTON_COUNT = 14;


private:
	bool  m_isConnected;

	XboxControllerNumber m_controllerNumber;

	float m_normalizedLeftTrigger;
	float m_normalizedRightTrigger;

	bool  m_buttonFrontBuffer[BUTTON_COUNT];
	bool  m_buttonBackBuffer[BUTTON_COUNT];

	AnalogJoystick m_leftJoystick;
	AnalogJoystick m_rightJoystick;
};