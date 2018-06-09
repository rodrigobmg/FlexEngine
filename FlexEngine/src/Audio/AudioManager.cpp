#include "stdafx.hpp"

#include "Audio/AudioManager.hpp"

#include "Logger.hpp"
#include "Helpers.hpp"
#include "..\..\include\Audio\AudioManager.hpp"

namespace flex
{
	std::vector<AudioManager::Source> AudioManager::s_Sources;

	ALCcontext* AudioManager::s_Context = nullptr;
	ALCdevice* AudioManager::s_Device = nullptr;

	ALuint AudioManager::s_Buffers[NUM_BUFFERS];

	void AudioManager::Initialize()
	{
		// Retrieve preferred device
		s_Device = alcOpenDevice(NULL);

		if (!s_Device)
		{
			Logger::LogError("Failed to create an openAL device!");
			return;
		}

		bool printAvailableAudioDeviceNames = false;
		if (printAvailableAudioDeviceNames)
		{
			PrintAudioDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
		}

		const ALchar* deviceName = alcGetString(s_Device, ALC_DEVICE_SPECIFIER);
		Logger::LogInfo("Chosen audio device name: \"" + std::string(deviceName) + "\"");

		s_Context = alcCreateContext(s_Device, NULL);
		alcMakeContextCurrent(s_Context);

		bool eaxPresent = alIsExtensionPresent("EAX2.0") == GL_TRUE;

		alGenBuffers(NUM_BUFFERS, s_Buffers);
		ALenum error = alGetError();
		if (error != AL_NO_ERROR)
		{
			DisplayALError("alGenBuffers: ", error);
			return;
		}
	}

	void AudioManager::Destroy()
	{
		ClearAllAudioSources();
		alcMakeContextCurrent(NULL);
		alcDestroyContext(s_Context);
		alcCloseDevice(s_Device);
	}

	AudioSourceID AudioManager::AddAudioSource(const std::string& filePath)
	{
		AudioSourceID newID = s_Sources.size();
		// TODO: Reuse buffers after sounds stop playing?
		assert(newID < NUM_BUFFERS);

		std::string friendlyName = filePath;
		StripLeadingDirectories(friendlyName);
		Logger::LogInfo("Loading audio source " + friendlyName);

		// WAVE file
		i32 format;
		void* data;
		i32 size;
		i32 freq;
		if (!ParseWAVFile(filePath, &format, &data, &size, &freq))
		{
			Logger::LogError("Failed to open or parse WAVE file");
			return InvalidAudioSourceID;
		}

		ALenum error = alGetError();
		if (error != AL_NO_ERROR)
		{
			DisplayALError("OpenAndParseWAVFile: ", error);
			alDeleteBuffers(NUM_BUFFERS, s_Buffers);
			return InvalidAudioSourceID;
		}


		// Buffer
		alBufferData(s_Buffers[newID], format, data, size, freq);
		error = alGetError();
		if (error != AL_NO_ERROR)
		{
			DisplayALError("alBufferData: ", error);
			alDeleteBuffers(NUM_BUFFERS, s_Buffers);
			return InvalidAudioSourceID;
		}
		delete[] data;


		// Source
		s_Sources.resize(s_Sources.size() + 1);

		alGenSources(1, &s_Sources[newID].source);
		error = alGetError();
		if (error != AL_NO_ERROR)
		{
			DisplayALError("alGenSources 1: ", error);
			return InvalidAudioSourceID;
		}

		alSourcei(s_Sources[newID].source, AL_BUFFER, s_Buffers[newID]);
		DisplayALError("alSourcei: ", alGetError());

		return newID;
	}

	void AudioManager::ClearAllAudioSources()
	{
		for (Source& source : s_Sources)
		{
			alDeleteSources(1, &source.source);
		}
		s_Sources.clear();
	}

	void AudioManager::PlaySource(AudioSourceID sourceID, bool forceRestart)
	{
		assert(sourceID < s_Sources.size());

		alGetSourcei(s_Sources[sourceID].source, AL_SOURCE_STATE, &s_Sources[sourceID].state);

		if (forceRestart || s_Sources[sourceID].state != AL_PLAYING)
		{
			alSourcePlay(s_Sources[sourceID].source);
			alGetSourcei(s_Sources[sourceID].source, AL_SOURCE_STATE, &s_Sources[sourceID].state);
			DisplayALError("PlaySource: ", alGetError());
		}
	}

	void AudioManager::PauseSource(AudioSourceID sourceID)
	{
		assert(sourceID < s_Sources.size());

		alGetSourcei(s_Sources[sourceID].source, AL_SOURCE_STATE, &s_Sources[sourceID].state);

		if (s_Sources[sourceID].state != AL_PAUSED)
		{
			alSourcePause(s_Sources[sourceID].source);
			alGetSourcei(s_Sources[sourceID].source, AL_SOURCE_STATE, &s_Sources[sourceID].state);
			DisplayALError("PauseSource: ", alGetError());
		}
	}

	void AudioManager::StopSource(AudioSourceID sourceID)
	{
		assert(sourceID < s_Sources.size());

		alGetSourcei(s_Sources[sourceID].source, AL_SOURCE_STATE, &s_Sources[sourceID].state);

		if (s_Sources[sourceID].state != AL_STOPPED)
		{
			alSourceStop(s_Sources[sourceID].source);
			alGetSourcei(s_Sources[sourceID].source, AL_SOURCE_STATE, &s_Sources[sourceID].state);
			DisplayALError("StopSource: ", alGetError());
		}
	}

	void AudioManager::ScaleSourceGain(AudioSourceID sourceID, real gainScale, bool preventZero)
	{
		assert(sourceID < s_Sources.size());

		const real epsilon = 0.00001f;

		if (preventZero && s_Sources[sourceID].gain == 0.0f)
		{
			s_Sources[sourceID].gain = epsilon;
		}

		real newGain = s_Sources[sourceID].gain * gainScale;
		if (preventZero && newGain < epsilon)
		{
			// Prevent gain from reaching 0, so it can be scaled up again
			newGain = epsilon;
		}
		SetSourceGain(sourceID, newGain);
	}

	void AudioManager::SetSourceGain(AudioSourceID sourceID, real gain)
	{
		assert(sourceID < s_Sources.size());

		gain = glm::clamp(gain, 0.0f, 1.0f);

		if (s_Sources[sourceID].gain != gain)
		{
			s_Sources[sourceID].gain = gain;
			//Logger::LogInfo("gain: " + std::to_string(gain));
			alSourcef(s_Sources[sourceID].source, AL_GAIN, gain);

			DisplayALError("SetSourceGain: ", alGetError());
		}
	}

	real AudioManager::GetSourceGain(AudioSourceID sourceID)
	{
		assert(sourceID < s_Sources.size());

		return s_Sources[sourceID].gain;
	}

	void AudioManager::AddToSourcePitch(AudioSourceID sourceID, real deltaPitch)
	{
		assert(sourceID < s_Sources.size());

		SetSourcePitch(sourceID, s_Sources[sourceID].pitch + deltaPitch);
	}

	void AudioManager::SetSourceLooping(AudioSourceID sourceID, bool looping)
	{
		assert(sourceID < s_Sources.size());

		if (s_Sources[sourceID].bLooping != looping)
		{
			s_Sources[sourceID].bLooping = looping;
			alSourcei(s_Sources[sourceID].source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);

			DisplayALError("SetSourceLooping: ", alGetError());
		}
	}

	bool AudioManager::GetSourceLooping(AudioSourceID sourceID)
	{
		assert(sourceID < s_Sources.size());

		return s_Sources[sourceID].bLooping;
	}

	bool AudioManager::IsSourcePlaying(AudioSourceID sourceID)
	{
		assert(sourceID < s_Sources.size());

		alGetSourcei(s_Sources[sourceID].source, AL_SOURCE_STATE, &s_Sources[sourceID].state);
		return (s_Sources[sourceID].state == AL_PLAYING);
	}

	void AudioManager::SetSourcePitch(AudioSourceID sourceID, real pitch)
	{
		assert(sourceID < s_Sources.size());

		// openAL range (found in al.h)
		pitch = glm::clamp(pitch, 0.5f, 2.0f);

		if (s_Sources[sourceID].pitch != pitch)
		{
			s_Sources[sourceID].pitch = pitch;
			//Logger::LogInfo("pitch: " + std::to_string(pitch));
			alSourcef(s_Sources[sourceID].source, AL_PITCH, pitch);

			DisplayALError("SetSourcePitch: ", alGetError());
		}
	}

	real AudioManager::GetSourcePitch(AudioSourceID sourceID)
	{
		assert(sourceID < s_Sources.size());

		return s_Sources[sourceID].pitch;
	}

	void AudioManager::DisplayALError(const std::string& str, ALenum error)
	{
		switch (error)
		{
		case AL_NO_ERROR:			return;
		case AL_INVALID_NAME:		Logger::LogError(str + "Invalid name"); break;
		case AL_ILLEGAL_ENUM:		Logger::LogError(str + "Invalid enum"); break;
		case AL_INVALID_VALUE:		Logger::LogError(str + "Invalid value"); break;
		case AL_INVALID_OPERATION:	Logger::LogError(str + "Invalid operation"); break;
		case AL_OUT_OF_MEMORY:		Logger::LogError(str + "Out of memory"); break;
		default:					Logger::LogError(str + "Unknown error"); break;
		}
	}

	void AudioManager::PrintAudioDevices(const ALCchar* devices)
	{
		const ALCchar* device = devices;
		const ALCchar* next = devices + 1;

		Logger::LogInfo("Available OpenAL devices:");
		while (device && *device != '\0' && 
			   next && *next != '\0')
		{
			Logger::LogInfo("\t\t" + std::string(device));
			size_t len = strlen(device);
			device += (len + 1);
			next += (len + 2);
		}
	}
} // namespace flex
