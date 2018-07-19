#include "Game/Menus/Menu.hpp"

#include "Game/GameCommon.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Camera.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Rendering/Shader.hpp"
#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/Renderer.hpp"



void Menu::Initialize(Vector2 lowerLeft, bool isSelecting, bool showBackground)
{
	m_isSelecting = isSelecting;
	m_corner = lowerLeft;

	m_showBackground = showBackground;
	m_backgroundTexture = Texture::CreateOrGet("Data/BuiltIns/DEFAULT_TEXTURE.png", Texture::TEXTURE_FORMAT_RGBA8);
	m_font = BitmapFont::CreateOrGet("Data/Fonts/SquirrelFixedFont.png");

	m_camera = new Camera();
	m_camera->SetProjection(Matrix4::MakeOrtho2D(Vector2(0.0f, 0.0f), (Vector2)g_theWindow->GetDimensions()));
	m_camera->SetColorTarget(0, g_theRenderer->GetDefaultColorTarget());
	m_camera->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());

	m_shader = Shader::Get("Default_2D");
}



void Menu::Destroy()
{
	delete m_camera;
	m_camera = nullptr;
}



void Menu::ProcessInput()
{
	XboxController& controller = g_theInput->GetController(CONTROLLER_NUMBER_1);

	if (g_theInput->WasKeyJustPressed('W') || (controller.IsConnected() && controller.WasButtonJustPressed(XboxController::BUTTON_UP)))
	{
		IncrementIndex();
	}
	if (g_theInput->WasKeyJustPressed('S') || (controller.IsConnected() && controller.WasButtonJustPressed(XboxController::BUTTON_DOWN)))
	{
		DecrementIndex();
	}
}



void Menu::IncrementIndex()
{
	if (m_options.size() == 0)
	{
		return;
	}

	// We dont want to select an inactive menu option we want to jump over it
	for (size_t attempts = 0; attempts < m_options.size(); ++attempts)
	{
		// increment the index
		m_currentIndex += 1;
		if(m_currentIndex >= (int)m_options.size())
		{
			m_currentIndex = 0;
		}

		// if the option is active we are good
		if(m_options[m_currentIndex].m_active)
		{
			break;
		}
	}
}



void Menu::DecrementIndex()
{
	if (m_options.size() == 0)
	{
		return;
	};

	// We dont want to select an inactive menu option we want to jump over it
	for (size_t attempts = 0; attempts < m_options.size(); ++attempts)
	{
		// decrement the index
		m_currentIndex -= 1;
		if(m_currentIndex < 0)
		{
			m_currentIndex = m_options.size() - 1;
		}

		// if the option is active we are good
		if(m_options[m_currentIndex].m_active)
		{
			break;
		}
	}
}



void Menu::Render() const
{
	g_theRenderer->BindCamera(m_camera);
	g_theRenderer->BindShader(m_shader);
	g_theRenderer->BindTexture(Renderer::DIFFUSE_TEXTURE_BIND_POINT, m_backgroundTexture);

	if (m_showBackground)
	{
		Vector2 dimensions = GetDimensions();
		Mesh* backgroundMesh = CreateAABB2D(AABB2D(m_corner + m_offsetsFromCorner, m_corner + m_offsetsFromCorner + dimensions), m_backgroundColor);
		g_theRenderer->DrawMesh(backgroundMesh->GetSubMesh(0));
		delete backgroundMesh;
	}


	Vector2 optionDrawPosition = m_corner + m_offsetsFromCorner + Vector2(m_menuPadding, m_menuPadding);
	for (size_t i = 0; i < m_options.size(); ++i)
	{
		RGBA fontColor = m_activeColor;
		if (GetHoverIndex() == i && m_isSelecting)
		{
			fontColor = m_hoverColor;
		}
		else if (!m_options[i].m_active)
		{
			fontColor = m_inactiveColor;
		}

		g_theRenderer->DrawText2D(optionDrawPosition, m_options[i].m_name, m_fontHeight, fontColor, 1.0f, m_font);
		optionDrawPosition.y += (m_fontHeight + m_optionsPadding);
	}
}



void Menu::AddOption( const std::string& optionName, unsigned int value, bool isEnabled )
{
	MenuOption newOption;
	newOption.m_name = optionName;
	newOption.m_value = value;
	newOption.m_active = isEnabled;

	m_options.push_back(newOption);

	if (!m_options[m_currentIndex].m_active)
	{
		DecrementIndex();
	}
}



void Menu::DisableOption( const std::string& optionName )
{
	for (size_t i = 0; i < m_options.size(); ++i)
	{
		if (m_options[i].m_name == optionName)
		{
			m_options[i].m_active = false;
		}
	}
}



void Menu::SelectOption( const std::string& optionName )
{
	for (size_t i = 0; i < m_options.size(); ++i)
	{
		if (m_options[i].m_name == optionName)
		{
			m_currentIndex = (int)i;
			break;
		}
	}
}



unsigned int Menu::GetHoverIndex() const        
{ 
	return m_currentIndex; 
}



unsigned int Menu::GetFrameSelection() const
{
	unsigned int value = INVALID_INDEX;

	// If our action key was pressed
	XboxController& controller = g_theInput->GetController(CONTROLLER_NUMBER_1);
	if (g_theInput->WasKeyJustPressed(' ') || (controller.IsConnected() && controller.WasButtonJustPressed(XboxController::BUTTON_A)))
	{
		if (m_options[GetHoverIndex()].m_active)
		{
			value = m_options[GetHoverIndex()].m_value;
		}
	}

	return value;
}



bool Menu::IsMenuVisible()
{
	return m_isVisible;
}



void Menu::ShowMenu()
{
	m_isVisible = true;
}



void Menu::HideMenu()
{
	m_isVisible = false;
}



void Menu::SetActiveColor(const RGBA& color)
{
	m_activeColor = color;
}



void Menu::SetInactiveColor(const RGBA& color)
{
	m_inactiveColor = color;
}



void Menu::SetHoverColor(const RGBA& color)
{
	m_hoverColor = color;
}



void Menu::SetFontSize(float size)
{
	m_fontHeight = size;
}



void Menu::SetBackgroundColor(const RGBA& color)
{
	m_backgroundColor = color;
}



void Menu::SetLowerLeftCorner(Vector2 newLowerLeft)
{
	m_corner = newLowerLeft;
}


void Menu::SetExternalPadding(Vector2 leftAndBottom)
{
	m_offsetsFromCorner = leftAndBottom;
}



float Menu::GetAbsoluteHeight() const
{
	float absHeight = m_corner.y + m_offsetFromCorner + (2.0f * m_menuPadding) + ((m_fontHeight + m_optionsPadding) * m_options.size());
	return absHeight;
}



Vector2 Menu::GetDimensions() const
{
	float menuWidth = 2.0f * m_menuPadding;
	float maxStringWidth = 0;
	for (size_t i = 0; i < m_options.size(); ++i)
	{
		float stringWidth = m_font->GetStringWidth(m_options[i].m_name, m_fontHeight, 1.0f);
		if (stringWidth > maxStringWidth)
		{
			maxStringWidth = stringWidth;
		}
	}
	menuWidth += maxStringWidth;
	float menuHeight = 2.0f * m_menuPadding + ((m_fontHeight + m_optionsPadding) * m_options.size());

	Vector2 dimensions = Vector2(menuWidth, menuHeight);
	return dimensions;
}
