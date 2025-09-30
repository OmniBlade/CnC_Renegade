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
 *                     $Archive:: /Commando/Code/WWAudio/sound2dhandle.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/31/01 3:02p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "sound2dhandle.h"
#include "AudibleSound.h"
#include "WWAudio.h"
#include "soundhandle.h"
#include "wwdebug.h"
#include "wwprofile.h"

//////////////////////////////////////////////////////////////////////
//
//	Sound2DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound2DHandleClass::Sound2DHandleClass (void)
	: SampleHandle ((WWAudioClass::Sample2D)INVALID_MILES_HANDLE)
#ifdef W3D_HAS_OPENAL
	, LoopCount(0)
#endif
{
#ifdef W3D_HAS_OPENAL
	alGetError();
	alGenBuffers(1, &OpenALBuffer);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to generate OpenAL buffer.\n"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	~Sound2DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound2DHandleClass::~Sound2DHandleClass (void)
{
#ifdef W3D_HAS_OPENAL
	// Unbind any buffers before deleting the object.
	alSourcei(SampleHandle, AL_BUFFER, AL_NONE);
	alDeleteBuffers(1, &OpenALBuffer);
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Initialize (SoundBufferClass *buffer)
{
	WWPROFILE ("Sound2DHandleClass::Initialize");

	SoundHandleClass::Initialize (buffer);

#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {

		//
		// Make sure this handle is fresh
		//
		::AIL_init_sample (SampleHandle);

		//
		// Pass the actual sound data onto the sample
		//
		if (Buffer != NULL) {
			::AIL_set_named_sample_file (SampleHandle, (char *)Buffer->Get_Filename (),
					Buffer->Get_Raw_Buffer (), Buffer->Get_Raw_Length (), 0);
		}
	}
#elif defined W3D_HAS_OPENAL
	LoopCount = 0;
	// Stop source and unbind any existing buffers from this source.
	alSourceStop(SampleHandle);
	alSourcei(SampleHandle, AL_BUFFER, AL_NONE);

	if (Buffer != NULL)
	{
		alGetError();
		alBufferData(
			OpenALBuffer,
			WWAudioClass::Get_AL_Format(Buffer->Get_Channels(),
			Buffer->Get_Bits()),
			Buffer->Get_Raw_Buffer(),
			Buffer->Get_Raw_Length(),
			Buffer->Get_Rate());
		
		if (alGetError() != AL_NO_ERROR) {
			WWDEBUG_SAY(("Failed to buffer data for 2D sample\n"));
		}
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Start_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Start_Sample (void)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_start_sample (SampleHandle);
	}
#elif defined W3D_HAS_OPENAL
	if (LoopCount = 0) {
		LoopCount = 1;
	}

	Sound2DHandleClass::Queue_Audio();
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
Sound2DHandleClass::Stop_Sample (void)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_stop_sample (SampleHandle);
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
Sound2DHandleClass::Resume_Sample (void)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_resume_sample (SampleHandle);
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
Sound2DHandleClass::End_Sample (void)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_end_sample (SampleHandle);
	}
#elif defined W3D_HAS_OPENAL
	alGetError();
	alSourceStop(SampleHandle);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't stop source.\n"));
	}

	// Dissociated any queued data.
	alSourcei(SampleHandle, AL_BUFFER, AL_NONE);
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_Pan (int pan)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_pan (SampleHandle, pan);
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
Sound2DHandleClass::Get_Sample_Pan (void)
{
	int retval = 0;

#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_pan (SampleHandle);
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
Sound2DHandleClass::Set_Sample_Volume (int volume)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_volume (SampleHandle, volume);
	}
#elif defined W3D_HAS_OPENAL
	alGetError();
	alSourcef(SampleHandle, AL_GAIN, volume / 127.0f);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Sound2DHandleClass::Set_Sample_Volume couldn't set source gain.\n"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
int
Sound2DHandleClass::Get_Sample_Volume (void)
{
	int retval = 0;

#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_volume (SampleHandle);
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
Sound2DHandleClass::Set_Sample_Loop_Count (unsigned count)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_loop_count (SampleHandle, count);
	}
#elif defined W3D_HAS_OPENAL
	WWDEBUG_SAY(("Sound2D %s requested to loop %u times.\n", Buffer->Get_Filename(), count));
	// count 0 is special and is supposed to mean infinite... best we can do is UINT_MAX or "lots".
	LoopCount = count == 0 ? UINT_MAX : count + 1;
	Sound2DHandleClass::Queue_Audio();
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
unsigned
Sound2DHandleClass::Get_Sample_Loop_Count (void)
{
	unsigned retval = 0;

#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_loop_count (SampleHandle);
	}
#elif defined W3D_HAS_OPENAL
	// First clean up any finished buffers.
	ALint processed;
	alGetSourcei(SampleHandle, AL_BUFFERS_PROCESSED, &processed);
	while (processed > 0) {
			ALuint buffer;
			alSourceUnqueueBuffers(SampleHandle, 1, &buffer);
			processed--;
	}

	// Next query how many buffers are still to play.
	ALint num_queued;
	alGetSourcei(SampleHandle, AL_BUFFERS_QUEUED, &num_queued);
	if (num_queued < 0)
	{
		num_queued = 0;
	}

	retval = num_queued + LoopCount;
#endif
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_MS_Position (unsigned ms)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_ms_position (SampleHandle, ms);
	}
#elif defined W3D_HAS_OPENAL
	alGetError();
	alSourcef(SampleHandle, AL_SEC_OFFSET, ms / 1000.0F);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source position.\n"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Get_Sample_MS_Position (int *len, int *pos)
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
Sound2DHandleClass::Set_Sample_User_Data (int i, void *val)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_user_data (SampleHandle, i, val);
	}
#elif defined W3D_HAS_OPENAL
	WWASSERT(i == 0);
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
Sound2DHandleClass::Get_Sample_User_Data (int i)
{
	void * retval = nullptr;

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
Sound2DHandleClass::Get_Sample_Pitch_Factor (void)
{	
	float retval = 0;
	
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		//
		//	Get the base rate of the sound and scale our playback rate
		// based on the factor
		//
		int rate = ::AIL_sample_playback_rate (SampleHandle);
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
Sound2DHandleClass::Set_Sample_Pitch_Factor (float pitch)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		//
		//	Get the base rate of the sound and scale our playback rate
		// based on the factor
		//
		int rate = int(pitch * Buffer->Get_Rate());
		::AIL_set_sample_playback_rate (SampleHandle, rate);
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
Sound2DHandleClass::Set_Miles_Handle (MILES_HANDLE handle)
{
	SampleHandle = WWAudioClass::Sample2D(handle);
}

void Sound2DHandleClass::Queue_Audio()
{
#if defined W3D_HAS_OPENAL
	// First clean up any finished buffers.
	ALint processed;
	alGetSourcei(SampleHandle, AL_BUFFERS_PROCESSED, &processed);
	while (processed > 0) {
		ALuint buffer;
		alSourceUnqueueBuffers(SampleHandle, 1, &buffer);
		processed--;
	}

	// Next query how many buffers are still to play.
	ALint num_queued;
	alGetSourcei(SampleHandle, AL_BUFFERS_QUEUED, &num_queued);
	if (num_queued < 0)
	{
		num_queued = 0;
	}

	unsigned count = min(LoopCount, 10u);

	// Repeatedly queue our sample.
	if (unsigned(num_queued) < count) {
		count -= num_queued;

		LoopCount -= count;

		while (count--)
		{
			alSourceQueueBuffers(SampleHandle, 1, &OpenALBuffer);
		}
	}
#endif
}
