#pragma once
#include "ThirdParty/fmod/fmod.hpp"

#include "Engine/Core/XmlUtilities.hpp"

#include <string>
#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
typedef size_t SoundID;
typedef size_t SoundPlaybackID;
constexpr size_t MISSING_SOUND_ID = (size_t)(-1); // for bad SoundIDs and SoundPlaybackIDs


//-----------------------------------------------------------------------------------------------
class AudioSystem;



class SoundGroupPair
{
public:
	SoundGroupPair(const std::string& name, float probability) : m_name(name), m_probability(probability) {};

	std::string m_name;
	float		m_probability;
};



class SoundGroup
{
public:
	SoundGroup() {};
	SoundGroup(AudioSystem* owner, const XMLElement* groupElement);

	SoundID GetRandomSound() const;


private:
	AudioSystem*				m_owner = nullptr;
	std::vector<SoundGroupPair> m_pairs;
};


/////////////////////////////////////////////////////////////////////////////////////////////////
class AudioSystem
{
public:
	AudioSystem();
	virtual ~AudioSystem();

public:
	virtual void				BeginFrame();
	virtual void				EndFrame();

	virtual SoundID				CreateOrGetSound( const std::string& soundFilePath );
	virtual SoundID				GetSoundFromGroup( const std::string& groupName );
	virtual SoundPlaybackID		PlaySound( SoundID soundID, bool isLooped=false, float volume = 1.0f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
	virtual SoundID				PlayRandomSoundFromGroup(const std::string& groupName, float volume = 1.0f, float balance = 0.0f, float speed = 1.0f);
	virtual SoundPlaybackID		Crossfade( SoundID soundID, SoundPlaybackID currentSoundPID, float fadeDurationSeconds, bool isLooped=false, float volume = 1.0f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
	virtual void				StopSound( SoundPlaybackID soundPlaybackID );
	virtual void				PauseSound( SoundPlaybackID soundPlaybackID , bool pauseState = true);
	virtual void				SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume );	// volume is in [0,1]
	virtual void				SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance );	// balance is in [-1,1], where 0 is L/R centered
	virtual void				SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed );		// speed is frequency multiplier (1.0 == normal)


	virtual void				ValidateResult( FMOD_RESULT result );

	virtual void				LoadSoundGroups(const std::string& soundGroupFilepath);

protected:
	FMOD::System*						m_fmodSystem;
	std::map< std::string, SoundID >	m_registeredSoundIDs;
	std::vector< FMOD::Sound* >			m_registeredSounds;
	std::map<std::string, SoundGroup>	m_soundGroups;
};

