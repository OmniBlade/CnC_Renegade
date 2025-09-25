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
#include "wwprofile.h"


//////////////////////////////////////////////////////////////////////
//
//	Sound2DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound2DHandleClass::Sound2DHandleClass (void)
#ifdef W3D_HAS_MILES
	: SampleHandle ((HSAMPLE)INVALID_MILES_HANDLE)
#endif
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~Sound2DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound2DHandleClass::~Sound2DHandleClass (void)
{
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
#endif
	return ;
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
		::AIL_sample_ms_position (SampleHandle, len, pos);
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
Sound2DHandleClass::Set_Sample_User_Data (int i, void *val)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_user_data (SampleHandle, i, val);
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
Sound2DHandleClass::Get_Sample_User_Data (int i)
{
	void * retval = nullptr;

#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_user_data (SampleHandle, i);
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
Sound2DHandleClass::Get_Sample_Playback_Rate (void)
{	
	int retval = 0;

#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_playback_rate (SampleHandle);
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
Sound2DHandleClass::Set_Sample_Playback_Rate (int rate)
{
#ifdef W3D_HAS_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_playback_rate (SampleHandle, rate);
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
Sound2DHandleClass::Set_Miles_Handle (void *handle)
{
#ifdef W3D_HAS_MILES
	SampleHandle = (HSAMPLE)handle;
#endif
	return ;
}
