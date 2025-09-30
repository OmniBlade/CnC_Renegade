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
 *                     $Archive:: /Commando/Code/WWAudio/sound3dhandle.cpp                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/31/01 3:00p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "sound3dhandle.h"
#include "AudibleSound.h"
#include "wwprofile.h"


//////////////////////////////////////////////////////////////////////
//
//	Sound3DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound3DHandleClass::Sound3DHandleClass (void)
	: SampleHandle ((WWAudioClass::Sample3D)INVALID_MILES_HANDLE)
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
//	~Sound3DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound3DHandleClass::~Sound3DHandleClass (void)
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
Sound3DHandleClass::Initialize (SoundBufferClass *buffer)
{
	WWPROFILE ("Sound3DHandleClass::Initialize");

	SoundHandleClass::Initialize (buffer);

#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE && Buffer != NULL) {

		//
		//	Configure the 3D sample
		//
		unsigned success = ::AIL_set_3D_sample_file (SampleHandle, Buffer->Get_Raw_Buffer ());

		int test1 = 0;
		int test2 = 0;
		Get_Sample_MS_Position (&test1, &test2);
		
		//
		//	Check for success
		//
		WWASSERT (success != 0);
		if (success == 0) {
			WWDEBUG_SAY (("WWAudio: Couldn't set 3d sample file.  Reason %s\r\n", ::AIL_last_error ()));
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
Sound3DHandleClass::Start_Sample (void)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_start_3D_sample (SampleHandle);
	}
#elif defined W3D_HAS_OPENAL
	if (LoopCount = 0) {
		LoopCount = 1;
	}

	Sound3DHandleClass::Queue_Audio();
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
Sound3DHandleClass::Stop_Sample (void)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_stop_3D_sample (SampleHandle);
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
Sound3DHandleClass::Resume_Sample (void)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_resume_3D_sample (SampleHandle);
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
Sound3DHandleClass::End_Sample (void)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_end_3D_sample (SampleHandle);
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
Sound3DHandleClass::Set_Sample_Pan (int /*pan*/)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
int
Sound3DHandleClass::Get_Sample_Pan (void)
{
	return 64;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Set_Sample_Volume (int volume)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_3D_sample_volume (SampleHandle, volume);
	}
#elif defined W3D_HAS_OPENAL
	alGetError();
	alSourcef(SampleHandle, AL_GAIN, volume / 127.0f);

	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Sound3DHandleClass::Set_Sample_Volume couldn't set source gain.\n"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
int
Sound3DHandleClass::Get_Sample_Volume (void)
{
	int retval = 0;

#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_3D_sample_volume (SampleHandle);
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
Sound3DHandleClass::Set_Sample_Loop_Count (unsigned count)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_3D_sample_loop_count (SampleHandle, count);
	}
#elif defined W3D_HAS_OPENAL
	WWDEBUG_SAY(("Sound3D %s requested to loop %u times.\n", Buffer->Get_Filename(), count));
	// count 0 is special and is supposed to mean infinite... best we can do is UINT_MAX or "lots".
	LoopCount = count == 0 ? UINT_MAX : count;
	Sound3DHandleClass::Queue_Audio();
#endif
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
unsigned
Sound3DHandleClass::Get_Sample_Loop_Count (void)
{
	unsigned retval = 0;

#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_3D_sample_loop_count (SampleHandle);
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
Sound3DHandleClass::Set_Sample_MS_Position (unsigned ms)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {

		WWASSERT (Buffer != NULL);
		unsigned bytes_per_sec = (Buffer->Get_Rate () * Buffer->Get_Bits ()) >> 3;
		unsigned bytes = (ms * bytes_per_sec) / 1000;
		bytes += (bytes & 1);
		::AIL_set_3D_sample_offset (SampleHandle, bytes);
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
Sound3DHandleClass::Get_Sample_MS_Position (int *len, int *pos)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {

		WWASSERT (Buffer != NULL);
		if (pos != NULL) {
			unsigned bytes = ::AIL_3D_sample_offset (SampleHandle);
			unsigned bytes_per_sec = (Buffer->Get_Rate () * Buffer->Get_Bits ()) >> 3;
			unsigned ms = (bytes * 1000) / bytes_per_sec;
			(*pos) = ms;
		}

		if (len != NULL) {
			unsigned bytes = ::AIL_3D_sample_length (SampleHandle);
			unsigned bytes_per_sec = (Buffer->Get_Rate () * Buffer->Get_Bits ()) >> 3;
			unsigned ms = (bytes * 1000) / bytes_per_sec;
			(*len) = ms;
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
Sound3DHandleClass::Set_Sample_User_Data (int i, void *val)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_3D_object_user_data (SampleHandle, i, val);
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
Sound3DHandleClass::Get_Sample_User_Data (int i)
{
	void *retval = nullptr;

#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		retval = AIL_3D_object_user_data (SampleHandle, i);
	}
#elif defined W3D_HAS_OPENAL
	WWASSERT(i == 0);
	ALint state;
	alGetError();
	alGetSourcei(SampleHandle, AL_SOURCE_STATE, &state);

	if (alGetError() == AL_NO_ERROR) {
		// Miles implementation uses this exclusively to associate an audible class with a handle.
		retval = WWAudioClass::Get_Instance()->WWAudioClass::Get_3D_User(SampleHandle);
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
Sound3DHandleClass::Get_Sample_Pitch_Factor (void)
{	
	float retval = 0;
	
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		//
		//	Get the base rate of the sound and scale our playback rate
		// based on the factor
		//
		int rate = ::AIL_3D_sample_playback_rate (SampleHandle);
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
Sound3DHandleClass::Set_Sample_Pitch_Factor (float pitch)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		//
		//	Get the base rate of the sound and scale our playback rate
		// based on the factor
		//
		int rate = int(pitch * Buffer->Get_Rate());
		::AIL_set_3D_sample_playback_rate (SampleHandle, rate);
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
Sound3DHandleClass::Set_Miles_Handle (MILES_HANDLE handle)
{
	WWASSERT (SampleHandle == (WWAudioClass::Sample3D)INVALID_MILES_HANDLE);
	SampleHandle = WWAudioClass::Sample3D(handle);
	return ;
}

void Sound3DHandleClass::Queue_Audio()
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
