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
#ifdef W3D_HAS_MILES
	: SampleHandle ((H3DSAMPLE)INVALID_MILES_HANDLE)
#endif
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~Sound3DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound3DHandleClass::~Sound3DHandleClass (void)
{
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Playback_Rate
//
//////////////////////////////////////////////////////////////////////
int
Sound3DHandleClass::Get_Sample_Playback_Rate (void)
{	
	int retval = 0;

#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_3D_sample_playback_rate (SampleHandle);
	}
#endif
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Playback_Rate
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Set_Sample_Playback_Rate (int rate)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_3D_sample_playback_rate (SampleHandle, rate);
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Miles_Handle
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Set_Miles_Handle (void *handle)
{
#ifdef W3D_HAS_MILES
	WWASSERT (SampleHandle == (H3DSAMPLE)INVALID_MILES_HANDLE);

	SampleHandle = (H3DSAMPLE)handle;
#endif
	return ;
}
