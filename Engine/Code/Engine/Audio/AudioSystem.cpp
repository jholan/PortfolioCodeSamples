#include "Engine/Audio/AudioSystem.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Core/EngineCommon.h"

//-----------------------------------------------------------------------------------------------
// To disable audio entirely (and remove requirement for fmod.dll / fmod64.dll) for any game,
//	#define ENGINE_DISABLE_AUDIO in your game's Code/Game/EngineBuildPreferences.hpp file.
//
// Note that this #include is an exception to the rule "engine code doesn't know about game code".
//	Purpose: Each game can now direct the engine via #defines to build differently for that game.
//	Downside: ALL games must now have this Code/Game/EngineBuildPreferences.hpp file.
//
#include "Game/EngineBuildPreferences.hpp"
#if !defined( ENGINE_DISABLE_AUDIO )


//-----------------------------------------------------------------------------------------------
// Link in the appropriate FMOD static library (32-bit or 64-bit)
//
#if defined( _WIN64 )
#pragma comment( lib, "ThirdParty/fmod/fmod64_vc.lib" )
#else
#pragma comment( lib, "ThirdParty/fmod/fmod_vc.lib" )
#endif


//-----------------------------------------------------------------------------------------------
SoundGroup::SoundGroup(AudioSystem* owner, const XMLElement* groupElement)
{
	m_owner = owner;

	float combinedWeight = 0.0f;

	const XMLElement* soundElement = groupElement->FirstChildElement("Sound");
	while(soundElement != nullptr)
	{
		// Retrieve all data from the element
		std::string filepath = ParseXmlAttribute(*soundElement, "filepath", "");
		float weight = ParseXmlAttribute(*soundElement, "weight", 0.0f);
		
		// If it is valid(has a path) add it
		if (!filepath.empty())
		{
			SoundGroupPair sgPair = SoundGroupPair(filepath, weight);
			m_pairs.push_back(sgPair);
			combinedWeight += weight;
		}


		// Get next sound
		soundElement = soundElement->NextSiblingElement("Sound");
	}


	// Normalize weights
	if (combinedWeight != 0.0f)
	{
		for (int i = 0; i < (int)m_pairs.size(); ++i)
		{
			m_pairs[i].m_probability /= combinedWeight;
		}
	}
}


//-----------------------------------------------------------------------------------------------
SoundID SoundGroup::GetRandomSound() const
{
	// Get a random value != 0.0f
	float randomValue = GetRandomFloatInRange(0.00001f, 1.0f);

	SoundID sound = MISSING_SOUND_ID;

	for (int i = 0; i < (int)m_pairs.size(); ++i)
	{
		const SoundGroupPair& sgPair = m_pairs[i];

		if (randomValue < sgPair.m_probability)
		{
			sound = m_owner->CreateOrGetSound(sgPair.m_name);
			break;
		}
		else
		{
			randomValue -= sgPair.m_probability;
		}
	}

	return sound;
}


//-----------------------------------------------------------------------------------------------
// Initialization code based on example from "FMOD Studio Programmers API for Windows"
//
AudioSystem::AudioSystem()
	: m_fmodSystem( nullptr )
{
	FMOD_RESULT result;
	result = FMOD::System_Create( &m_fmodSystem );
	ValidateResult( result );

	result = m_fmodSystem->init( 512, FMOD_INIT_NORMAL, nullptr );
	ValidateResult( result );
}


//-----------------------------------------------------------------------------------------------
AudioSystem::~AudioSystem()
{
	FMOD_RESULT result = m_fmodSystem->release();
	ValidateResult( result );

	m_fmodSystem = nullptr; // #Fixme: do we delete/free the object also, or just do this?
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::BeginFrame()
{
	m_fmodSystem->update();
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
SoundID AudioSystem::CreateOrGetSound( const std::string& soundFilePath )
{
	std::map< std::string, SoundID >::iterator found = m_registeredSoundIDs.find( soundFilePath );
	if( found != m_registeredSoundIDs.end() )
	{
		return found->second;
	}
	else
	{
		FMOD::Sound* newSound = nullptr;
		m_fmodSystem->createSound( soundFilePath.c_str(), FMOD_DEFAULT, nullptr, &newSound );
		if( newSound )
		{
			SoundID newSoundID = m_registeredSounds.size();
			m_registeredSoundIDs[ soundFilePath ] = newSoundID;
			m_registeredSounds.push_back( newSound );
			return newSoundID;
		}
	}

	return MISSING_SOUND_ID;
}


//-----------------------------------------------------------------------------------------------
SoundID AudioSystem::GetSoundFromGroup( const std::string& groupName )
{
	SoundID sound = MISSING_SOUND_ID;

	std::map< std::string, SoundGroup >::iterator found = m_soundGroups.find( groupName );
	if( found != m_soundGroups.end() )
	{
		sound = found->second.GetRandomSound();
	}

	return sound;
}


//-----------------------------------------------------------------------------------------------
SoundPlaybackID AudioSystem::PlaySound( SoundID soundID, bool isLooped, float volume, float balance, float speed, bool isPaused )
{
	size_t numSounds = m_registeredSounds.size();
	if( soundID < 0 || soundID >= numSounds )
		return MISSING_SOUND_ID;

	FMOD::Sound* sound = m_registeredSounds[ soundID ];
	if( !sound )
		return MISSING_SOUND_ID;

	FMOD::Channel* channelAssignedToSound = nullptr;
	m_fmodSystem->playSound( sound, nullptr, isPaused, &channelAssignedToSound );
	if( channelAssignedToSound )
	{
		int loopCount = isLooped ? -1 : 0;
		unsigned int playbackMode = isLooped ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		float frequency;
		channelAssignedToSound->setMode(playbackMode);
		channelAssignedToSound->getFrequency( &frequency );
		channelAssignedToSound->setFrequency( frequency * speed );
		channelAssignedToSound->setVolume( volume );
		channelAssignedToSound->setPan( balance );
		channelAssignedToSound->setLoopCount( loopCount );
	}

	return (SoundPlaybackID) channelAssignedToSound;
}


//-----------------------------------------------------------------------------------------------
SoundID	AudioSystem::PlayRandomSoundFromGroup(const std::string& groupName, float volume, float balance, float speed)
{
	SoundID sound = GetSoundFromGroup(groupName);
	PlaySound(sound, false, volume, balance, speed);

	return sound;
}


//-----------------------------------------------------------------------------------------------
SoundPlaybackID AudioSystem::Crossfade( SoundID soundID, SoundPlaybackID currentSoundPID, float fadeDurationSeconds, bool isLooped, float volume, float balance, float speed, bool isPaused )
{
	UNUSED(isPaused);

	if( currentSoundPID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to crossfade current sound missing sound playback ID!" );
		return MISSING_SOUND_ID;
	}

	SoundPlaybackID newSoundPID = PlaySound(soundID, isLooped, 1.0f, balance, speed, true);
	if( newSoundPID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to crossfade new sound missing sound playback ID!" );
		return MISSING_SOUND_ID;
	}

	int sampleRate;
	FMOD_RESULT result = m_fmodSystem->getSoftwareFormat(&sampleRate, nullptr, nullptr); 

	FMOD::Channel* channelAssignedToCurrentSound = (FMOD::Channel*) currentSoundPID;
	FMOD::Channel* channelAssignedToNewSound	 = (FMOD::Channel*) newSoundPID;

	unsigned long long currentClock;
	result = channelAssignedToCurrentSound->getDSPClock(nullptr, &currentClock);
	channelAssignedToCurrentSound->addFadePoint(currentClock, 1.0f);
	channelAssignedToCurrentSound->addFadePoint(currentClock + (unsigned long long)(sampleRate * fadeDurationSeconds), 0.0f);

	unsigned long long newClock;
	result = channelAssignedToNewSound->getDSPClock(nullptr, &newClock);
	channelAssignedToNewSound->addFadePoint(newClock, 0.0f);
	channelAssignedToNewSound->addFadePoint(newClock + (unsigned long long)(sampleRate * fadeDurationSeconds), volume);
	PauseSound(newSoundPID, false);

	return newSoundPID;
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::StopSound( SoundPlaybackID soundPlaybackID )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to stop on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->stop();
}


void AudioSystem::PauseSound( SoundPlaybackID soundPlaybackID , bool pauseState)
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to pause on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->setPaused(pauseState);
}


//-----------------------------------------------------------------------------------------------
// Volume is in [0,1]
//
void AudioSystem::SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set volume on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->setVolume( volume );
}


//-----------------------------------------------------------------------------------------------
// Balance is in [-1,1], where 0 is L/R centered
//
void AudioSystem::SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set balance on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->setPan( balance );
}


//-----------------------------------------------------------------------------------------------
// Speed is frequency multiplier (1.0 == normal)
//	A speed of 2.0 gives 2x frequency, i.e. exactly one octave higher
//	A speed of 0.5 gives 1/2 frequency, i.e. exactly one octave lower
//
void AudioSystem::SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set speed on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	float frequency;
	FMOD::Sound* currentSound = nullptr;
	channelAssignedToSound->getCurrentSound( &currentSound );
	if( !currentSound )
		return;

	int ignored = 0;
	currentSound->getDefaults( &frequency, &ignored );
	channelAssignedToSound->setFrequency( frequency * speed );
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::ValidateResult( FMOD_RESULT result )
{
	if( result != FMOD_OK )
	{
		ERROR_RECOVERABLE( Stringf( "Engine/Audio SYSTEM ERROR: Got error result code %i - error codes listed in fmod_common.h\n", (int) result ) );
	}
}



void AudioSystem::LoadSoundGroups(const std::string& soundGroupFilepath)
{
	// Load the document
	XMLDocument doc;
	doc.LoadFile(soundGroupFilepath.c_str());
	XMLElement* rootElement = doc.FirstChildElement("SoundGroups");

	// Loop through and process each sound group in the file
	const XMLElement* soundGroupElement = rootElement->FirstChildElement("SoundGroup");
	while(soundGroupElement != nullptr)
	{
		std::string name = ParseXmlAttribute(*soundGroupElement, "name", "");
		if (!name.empty())
		{
			// If we found a name.
			auto locationInSoundGroups = m_soundGroups.find(name);
			if (locationInSoundGroups == m_soundGroups.end())
			{
				// The sound has not already been defined
				SoundGroup sg = SoundGroup(this, soundGroupElement);
				m_soundGroups[name] = sg;
			}
		}


		// Get next sound group
		soundGroupElement = soundGroupElement->NextSiblingElement("SoundGroup");
	}
}


#endif // !defined( ENGINE_DISABLE_AUDIO )
