/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : wwaudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/soundstreamhandle.cpp                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/23/01 4:47p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "soundstreamhandle.h"
#include "AudibleSound.h"
#include "WWAudio.h"


//////////////////////////////////////////////////////////////////////
//
//	SoundStreamHandleClass
//
//////////////////////////////////////////////////////////////////////
SoundStreamHandleClass::SoundStreamHandleClass(void)
	: SampleHandle(WWAudioClass::Sample2D(INVALID_MILES_HANDLE)),
#ifdef W3D_HAS_MILES
	StreamHandle((HSTREAM)INVALID_MILES_HANDLE)
#endif
#ifdef W3D_HAS_OPENAL
	StreamBufferIndex(0),
	LoopCount(0)
#endif
{
#ifdef W3D_HAS_OPENAL
	alGetError();
	alGenBuffers(STREAM_BUFFER_COUNT, StreamBuffers);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to generate OpenAL buffer.\n"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	~SoundStreamHandleClass
//
//////////////////////////////////////////////////////////////////////
SoundStreamHandleClass::~SoundStreamHandleClass (void)
{
#ifdef W3D_HAS_OPENAL
	// Unbind any buffers before deleting the object.
	alSourcei(SampleHandle, AL_BUFFER, AL_NONE);
	alDeleteBuffers(STREAM_BUFFER_COUNT, StreamBuffers);
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Initialize (SoundBufferClass *buffer)
{
	SoundHandleClass::Initialize (buffer);

#ifdef W3D_HAS_MILES
	if (Buffer != NULL) {

		//
		//	Create a stream from the sample handle
		//
		StreamHandle = ::AIL_open_stream_by_sample (WWAudioClass::Get_Instance ()->Get_2D_Driver (),
								SampleHandle, buffer->Get_Filename (), 0);

		/*StreamHandle = ::AIL_open_stream (WWAudioClass::Get_Instance ()->Get_2D_Driver (),
								buffer->Get_Filename (), 0);*/
	}
#elif defined W3D_HAS_OPENAL
	LoopCount = 0;
	// Stop source and unbind any existing buffers from this source.
	alSourceStop(SampleHandle);
	alSourcei(SampleHandle, AL_BUFFER, AL_NONE);
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Start_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Start_Sample (void)
{
#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_start_stream (StreamHandle);
	}
#elif defined W3D_HAS_OPENAL
	Buffer->Reset_Buffer();
	Buffer->Refresh_Buffer();
	SoundStreamHandleClass::Queue_Audio();
	alGetError();
	alSourcePlay(SampleHandle);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't play source.\n"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Stop_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Stop_Sample (void)
{
#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_pause_stream (StreamHandle, 1);
	}
#elif defined W3D_HAS_OPENAL
	alGetError();
	alSourcePause(SampleHandle);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't pause source.\n"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Resume_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Resume_Sample (void)
{
#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_pause_stream (StreamHandle, 0);
	}
#elif defined W3D_HAS_OPENAL
	alGetError();
	alSourcePlay(SampleHandle);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't resume source.\n"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	End_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::End_Sample (void)
{
	//
	//	Stop the sample and then release our hold on the stream handle
	//
	Stop_Sample ();

#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_close_stream (StreamHandle);
		StreamHandle = (HSTREAM)INVALID_MILES_HANDLE;
	}
#elif defined W3D_HAS_OPENAL
	alGetError();
	alSourceStop(SampleHandle);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't stop source.\n"));
	}

	// Unbind any buffers.
	alSourcei(SampleHandle, AL_BUFFER, AL_NONE);
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Pan (int pan)
{
#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_pan (StreamHandle, pan);
	}
#elif defined W3D_HAS_OPENAL
	ALfloat location[3] = {pan / 127.0F, 0.0F, 0.0F };
	alGetError();
	alSourcefv(SampleHandle, AL_POSITION, location);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't set source pan.\n"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
int
SoundStreamHandleClass::Get_Sample_Pan (void)
{
	int retval = 0;

#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		retval = ::AIL_stream_pan (StreamHandle);
	}
#elif defined W3D_HAS_OPENAL
	ALfloat location[3] = { 0 };
	alGetError();
	alGetSourcefv(SampleHandle, AL_POSITION, location);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't get source pan.\n"));
	}

	retval = int(location[0] * 127.0F);
#endif
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Volume (int volume)
{
#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_volume (StreamHandle, volume);
	}
#elif defined W3D_HAS_OPENAL
	alGetError();
	alSourcef(SampleHandle, AL_GAIN, volume / 127.0f);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("SoundStreamHandleClass::Set_Sample_Volume couldn't set source gain.\n"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
int
SoundStreamHandleClass::Get_Sample_Volume (void)
{
	int retval = 0;

#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		retval = ::AIL_stream_volume (StreamHandle);
	}
#elif defined W3D_HAS_OPENAL
	ALfloat state;
	alGetError();
	alGetSourcef(SampleHandle, AL_GAIN, &state);

	if (alGetError() == AL_NO_ERROR) {
		retval = int(state * 127.0F);
	}
#endif

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Loop_Count (unsigned count)
{
#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_loop_block (StreamHandle, 0, -1);
		::AIL_set_stream_loop_count (StreamHandle, count);
	}
#elif defined W3D_HAS_OPENAL
	WWDEBUG_SAY(("Stream %s requested to loop %u times.\n", Buffer->Get_Filename(), count));
	// count 0 is special and is supposed to mean infinite... best we can do is UINT_MAX or "lots".
	LoopCount = count == 0 ? UINT_MAX : count;
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
unsigned
SoundStreamHandleClass::Get_Sample_Loop_Count (void)
{
	unsigned retval = 0;

#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_stream_loop_count (StreamHandle);
	}
#elif defined W3D_HAS_OPENAL
	retval = LoopCount;
#endif

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_MS_Position (unsigned ms)
{
#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_ms_position (StreamHandle, ms);
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Get_Sample_MS_Position (int *len, int *pos)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		S32 mlen, mpos;
		::AIL_sample_ms_position (SampleHandle, &mlen, &mpos);
		if (pos != nullptr) {
			*pos = int(mpos);
		}

		if (len != nullptr) {
			*len = int(mlen);
		}
	}
#elif defined W3D_HAS_OPENAL
	if (pos != nullptr) {
		ALfloat sec_pos;
		alGetError();
		// TODO This is probably not correct for OpenAL streamed sources.
		alGetSourcef(SampleHandle, AL_SEC_OFFSET, &sec_pos);

		if (alGetError() == AL_NO_ERROR) {
			*pos = int(sec_pos * 1000.0F);
		}
	}

	if (len != nullptr) {
		// The buffer object should already have calculated the duration in ms.
		*len = Buffer->Get_Duration();
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_User_Data
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_User_Data (int i, void *val)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_user_data (SampleHandle, i, val);
	}
#elif defined W3D_HAS_OPENAL
	ALint state;
	alGetError();
	alGetSourcei(SampleHandle, AL_SOURCE_STATE, &state);

	if (alGetError() == AL_NO_ERROR) {
		// Miles implementation uses this exclusively to associate an audible class with a handle.
		WWAudioClass::Get_Instance()->WWAudioClass::Set_2D_User(SampleHandle, static_cast<AudibleSoundClass *>(val));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_User_Data
//
//////////////////////////////////////////////////////////////////////
void *
SoundStreamHandleClass::Get_Sample_User_Data (int i)
{
	void *retval = nullptr;

#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_user_data (SampleHandle, i);
	}
#elif defined W3D_HAS_OPENAL
	ALint state;
	alGetError();
	alGetSourcei(SampleHandle, AL_SOURCE_STATE, &state);

	if (alGetError() == AL_NO_ERROR) {
		// Miles implementation uses this exclusively to associate an audible class with a handle.
		retval = WWAudioClass::Get_Instance()->WWAudioClass::Get_2D_User(SampleHandle);
	}
#endif
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pitch_Factor
//
//////////////////////////////////////////////////////////////////////
float
SoundStreamHandleClass::Get_Sample_Pitch_Factor (void)
{	
	float retval = 0;
	
#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		//
		//	Get the base rate of the sound and scale our playback rate
		// based on the factor
		//
		int rate = ::AIL_stream_playback_rate (StreamHandle);
		retval = float(rate) / Buffer->Get_Rate();
	}
#elif defined W3D_HAS_OPENAL
	ALfloat pitch;
	alGetError();
	alGetSourcef(SampleHandle, AL_PITCH, &pitch);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to retrieve OpenAL source pitch.\n"));
	}

	retval = pitch;
#endif
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pitch_Factor
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Pitch_Factor (float pitch)
{
#ifdef W3D_HAS_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		//
		//	Get the base rate of the sound and scale our playback rate
		// based on the factor
		//
		int rate = int(pitch * Buffer->Get_Rate());
		::AIL_set_stream_playback_rate (StreamHandle, rate);
	}
#elif defined W3D_HAS_OPENAL
	alGetError();
	alSourcef(SampleHandle, AL_PITCH, pitch);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source pitch.\n"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Miles_Handle
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Miles_Handle (MILES_HANDLE handle)
{
	SampleHandle = WWAudioClass::Sample2D(handle);
}


void SoundStreamHandleClass::Queue_Audio()
{
#if defined W3D_HAS_OPENAL
	// Unqueue any finished buffers.
	ALint processed;
	alGetSourcei(SampleHandle, AL_BUFFERS_PROCESSED, &processed);
	while (processed > 0) {
			ALuint buffer;
			alSourceUnqueueBuffers(SampleHandle, 1, &buffer);
			processed--;
	}

	ALint num_queued;
	alGetSourcei(SampleHandle, AL_BUFFERS_QUEUED, &num_queued);
	if (num_queued >= STREAM_BUFFER_COUNT) {
			return;
	}

	if (Buffer != NULL)
	{
		alGetError();
		alBufferData(
			StreamBuffers[StreamBufferIndex],
			WWAudioClass::Get_AL_Format(Buffer->Get_Channels(),
			Buffer->Get_Bits()),
			Buffer->Get_Raw_Buffer(),
			Buffer->Get_Raw_Length(),
			Buffer->Get_Rate());
		
		if (alGetError() != AL_NO_ERROR) {
			WWDEBUG_SAY(("Failed to buffer data for streaming sample\n"));
			return;
		}

		alSourceQueueBuffers(SampleHandle, 1, &StreamBuffers[StreamBufferIndex]);
		
		if (alGetError() != AL_NO_ERROR) {
			WWDEBUG_SAY(("Failed to bind buffer for streaming sample\n"));
			return;
		}
		
		++StreamBufferIndex;
		if (StreamBufferIndex >= STREAM_BUFFER_COUNT) {
			StreamBufferIndex = 0;
		}

		bool more_data = Buffer->Refresh_Buffer();

		if (LoopCount != 0 && !more_data) {
			--LoopCount;
			Buffer->Reset_Buffer();
		}
	}
#endif
}
