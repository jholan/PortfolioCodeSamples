#pragma once

#include <vector>
#include <string>

#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"

class Texture;
class BitmapFont;
class Camera;
class Shader;

#define INVALID_INDEX (0xffffffffU)

//------------------------------------------------------------------------
// quick and easy menu object
struct MenuOption
{
	MenuOption() {};
	MenuOption(const std::string& name, unsigned int value, bool isActive) 
		: m_name(name), m_value(value), m_active(isActive) {};

	std::string  m_name; 
	unsigned int m_value; 
	bool		 m_active;
}; 



class Menu 
{
public:
	// Composition
	void Initialize(Vector2 lowerLeft = Vector2(0.0f, 0.0f), bool isSelecting = true, bool showBackground = true);
	void Destroy();

	// Core
	void ProcessInput();		  // process input for this menu
	void Render() const;          // render the menu

	// Options
	void AddOption( const std::string& optionName, unsigned int value, bool isEnabled = true); 
	void DisableOption( const std::string& optionName );
	void SelectOption( const std::string& optionName );

	// Selection
	unsigned int GetHoverIndex() const;

	// if the user selected an option this frame
	// returns the value of the current selection, otherwise 
	// returns INVALID_INDEX (0xffffffffU)
	unsigned int GetFrameSelection() const; 

	// Visibility
	bool IsMenuVisible();
	void ShowMenu();
	void HideMenu();

	// Fonts
	void SetActiveColor(const RGBA& color);
	void SetInactiveColor(const RGBA& color);
	void SetHoverColor(const RGBA& color);

	void SetFontSize(float size);

	// Background
	void SetBackgroundColor(const RGBA& color);

	// Positioning
	void SetLowerLeftCorner(Vector2 newLowerLeft);
	void SetExternalPadding(Vector2 leftAndBottom);

	// Dimensions
	float	GetAbsoluteHeight() const;
	Vector2 GetDimensions() const;

public:
	void IncrementIndex();
	void DecrementIndex();

	Camera*		m_camera = nullptr;

	// Render data
	Shader*			  m_shader				= nullptr;
	const Texture*	  m_backgroundTexture	= nullptr;
	const BitmapFont* m_font				= nullptr;
	RGBA			  m_activeColor			= RGBA();
	RGBA			  m_inactiveColor		= RGBA(192, 192, 192);
	RGBA			  m_hoverColor			= RGBA(255, 255, 0);
	RGBA			  m_backgroundColor		= RGBA(0, 0, 0);

	// Positioning
	Vector2 m_corner;

	// Padding
	Vector2 m_offsetsFromCorner = Vector2(32.0f, 32.0f);
	float	m_offsetFromCorner	= 32.0f;
	float	m_menuPadding		= 16.0f;
	float	m_fontHeight		= 16.0f;
	float	m_optionsPadding	= 8.0f;

	bool m_isSelecting		= true;
	bool m_isVisible		= true;
	bool m_showBackground	= true;
	int m_currentIndex		= 0; 
	std::vector<MenuOption> m_options; 
};
