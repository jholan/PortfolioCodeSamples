#include "Engine/Rendering/SpriteAnim.hpp"

#include "Engine/Rendering/SpriteSheet.hpp"
#include "Engine/Rendering/SpriteAnimDefinition.hpp"

#include <string>



SpriteAnim::SpriteAnim(const SpriteAnimDefinition* definition)
	: m_definition(definition)
{
}



SpriteAnim::~SpriteAnim()
{

}



void SpriteAnim::Update( float deltaSeconds )
{
	if (IsFinished())
	{
		return;
	}

	if (IsPlaying())
	{
		m_elapsedSeconds += deltaSeconds;

		switch(m_definition->GetPlaybackMode())
		{
		case SPRITE_ANIM_MODE_PLAY_TO_END:
		{
			if(m_elapsedSeconds >= GetDurationSeconds())
			{
				m_elapsedSeconds = GetDurationSeconds();
				m_isFinished = true;
				m_isPlaying = false;
			}
			break;
		}
		case SPRITE_ANIM_MODE_LOOPING:
		{
			if (m_elapsedSeconds > GetDurationSeconds())
			{
				m_elapsedSeconds -= GetDurationSeconds();
			}
			break;
		}
		default:
			break;
		}
	}
}



AABB2D SpriteAnim::GetCurrentTexCoords() const
{
	float fractionElapsed = GetFractionElapsed();
	AABB2D textureCoordinates = m_definition->GetTexCoordsForFractionCompleted(fractionElapsed);
	return textureCoordinates;
}



const Texture* SpriteAnim::GetTexture() const
{
	return m_definition->GetTexture();
}



const std::string& SpriteAnim::GetName() const
{
	return m_definition->GetName();
}



void SpriteAnim::SetPauseState(bool isPaused)
{
	m_isPlaying = !isPaused;
}



void SpriteAnim::Pause()
{
	m_isPlaying = false;
}



void SpriteAnim::Resume()
{
	m_isPlaying = true;
}



void SpriteAnim::Reset()
{
	m_isPlaying = true;
	m_isFinished = false;
	m_elapsedSeconds = 0.0f;
}



bool SpriteAnim::IsFinished() const	
{
	return m_isFinished; 
}



bool SpriteAnim::IsPlaying() const			
{
	return m_isPlaying; 
}



float SpriteAnim::GetDurationSeconds() const
{
	return m_definition->GetDurationSeconds(); 
}



bool SpriteAnim::IsAutoOrient() const
{
	return m_definition->IsAutoOrient();
}



float SpriteAnim::GetSecondsElapsed() const
{
	return m_elapsedSeconds; 
}



float SpriteAnim::GetSecondsRemaining() const
{
	float remainingSeconds = GetDurationSeconds() - m_elapsedSeconds;
	return remainingSeconds;
}



float SpriteAnim::GetFractionElapsed() const
{
	float fraction = m_elapsedSeconds / GetDurationSeconds();
	return fraction;
}



float SpriteAnim::GetFractionRemaining() const
{
	float fractionElapsed = GetFractionElapsed();
	float fractionRemaining = 1.0f - fractionElapsed;
	return fractionRemaining;
}



void SpriteAnim::SetSecondsElapsed( float secondsElapsed )
{
	m_elapsedSeconds = secondsElapsed;
}



void SpriteAnim::SetFractionElapsed( float fractionElapsed )
{
	float elapsedSeconds = fractionElapsed * GetDurationSeconds();
	m_elapsedSeconds = elapsedSeconds;
}
