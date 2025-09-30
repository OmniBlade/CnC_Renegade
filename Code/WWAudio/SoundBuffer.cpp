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
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/SoundBuffer.cpp                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/01/01 11:00a                                             $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "SoundBuffer.h"
#include "wwfile.h"
#include "wwdebug.h"
#include "Utils.h"
#include "ffactory.h"
#include "wwprofile.h"
#include "w3dconfig.h"

#ifdef W3D_HAS_FFMPEG
extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/avutil.h>
}
#endif

/////////////////////////////////////////////////////////////////////////////////
//
//	SoundBufferClass
//
SoundBufferClass::SoundBufferClass (void)
	: m_Buffer (NULL),
	  m_Length (0),
	  m_Filename (NULL),
	  m_Duration (0),
	  m_Rate (0),
	  m_Bits (0),
	  m_Channels (0),
#ifdef W3D_HAS_MILES
	  m_Type (WAVE_FORMAT_IMA_ADPCM)
#else
		m_Type(0)
#endif
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	~SoundBufferClass
//
SoundBufferClass::~SoundBufferClass (void)
{
	SAFE_FREE (m_Filename);
	Free_Buffer ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Free_Buffer
//
void
SoundBufferClass::Free_Buffer (void)
{
	// Make sure we reset the length
	m_Length = 0L;

#ifdef W3D_HAS_FFMPEG
	m_Buffer.clear();
#endif
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Determine_Stats
//
void
SoundBufferClass::Determine_Stats (unsigned char *buffer)
{
	WWPROFILE ("Determine_Stats");

	MMSLockClass lock;

	m_Duration = 0;
	m_Rate = 0;
	m_Channels = 0;
	m_Bits = 0;
#ifdef W3D_HAS_MILES
	m_Type = WAVE_FORMAT_IMA_ADPCM;

	// Attempt to get statistical information about this sound
	AILSOUNDINFO info = { 0 };
	if ((buffer != NULL) && (::AIL_WAV_info (buffer, &info) != 0)) {

		// Cache this information
		m_Rate = info.rate;
		m_Channels = info.channels;
		m_Bits = info.bits;
		m_Type = info.format;

		// Determine how long this sound will play for
		float bytes_sec = float((m_Channels * m_Rate * m_Bits) >> 3);
		m_Duration = (unsigned int)((((float)m_Length) / bytes_sec) * 1000.0F);
	}
#endif
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Set_Filename
//
void
SoundBufferClass::Set_Filename (const char *name)
{
	SAFE_FREE (m_Filename);
	if (name != NULL) {
		m_Filename = ::strdup (name);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Load_From_File
//
bool
SoundBufferClass::Load_From_File (const char *filename)
{
	WWPROFILE ("SoundBufferClass::Load_From_File");
	WWDEBUG_SAY(( "Loading sound file %s.\r\n", filename));

	// Assume failure
	bool retval = false;

	// Param OK?
	WWASSERT (filename != NULL);
	if (filename != NULL) {

		// Create a file object and pass it onto the appropriate function
		FileClass *file=_TheFileFactory->Get_File(filename);
		if ( file ) {
			retval = Load_From_File(*file);
			_TheFileFactory->Return_File(file);
		}
		file=NULL;
	}

	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Load_From_File
//
bool
SoundBufferClass::Load_From_File (FileClass &file)
{
	WWPROFILE ("SoundBufferClass::Load_From_File");

	MMSLockClass lock;

	// Assume failure
	bool retval = false;

	// Start from scratch
	Free_Buffer ();
	Set_Filename (file.File_Name ());

	// Open the file if necessary
	bool we_opened = false;
	if (file.Is_Open () == false) {
		we_opened = (file.Open () == true);
	}

#ifdef W3D_HAS_FFMPEG
	FFmpegFile ffmpeg(&file);
	m_Buffer.clear();

	if (!ffmpeg.Has_Audio()) {
		return false;
	}

	m_Duration = ffmpeg.Get_Duration();
	m_Rate = ffmpeg.Get_Sample_Rate();
	m_Channels = ffmpeg.Get_Num_Channels();
	m_Bits = ffmpeg.Get_Bytes_Per_Sample() * 8;

	FFmpegFrameCallback on_frame = [](AVFrame* frame, int stream_idx, int stream_type, void* user_data) {
		SoundBufferClass* sbc = static_cast<SoundBufferClass*>(user_data);
		if (stream_type != AVMEDIA_TYPE_AUDIO) {
			return;
		}
		const int frame_data_size = av_samples_get_buffer_size(nullptr,  sbc->m_Channels, frame->nb_samples, static_cast<AVSampleFormat>(frame->format), 1);
		sbc->m_Buffer.reserve(sbc->m_Buffer.size() + frame_data_size);

		if (av_sample_fmt_is_planar(static_cast<AVSampleFormat>(frame->format))) {
			// Convert planar audio to interleaved
			int num_channels = sbc->m_Channels;
			int bytes_per_sample = sbc->m_Bits / 8;
			for (int sample = 0; sample < frame->nb_samples; ++sample) {
				for (int channel = 0; channel < num_channels; ++channel) {
					const uint8_t* src = frame->data[channel] + sample * bytes_per_sample;
					sbc->m_Buffer.insert(sbc->m_Buffer.end(), src, src + bytes_per_sample);
				}
			}
		} else {
			// Directly copy interleaved audio
			sbc->m_Buffer.insert(sbc->m_Buffer.end(), frame->data[0], frame->data[0] + frame_data_size);
		}
	};

	ffmpeg.Set_Frame_Callback(on_frame);
	ffmpeg.Set_User_Data(this);

	// Read all packets inside the file
	while (ffmpeg.Decode_Packet()) {
	}

	ffmpeg.Close();

	retval = true;
#else
	// Determine the size of the buffer
	m_Length = file.Size ();
	WWASSERT	(m_Length > 0L);
	if (m_Length > 0L) {

		// Allocate a new buffer of the correct length and read the contents
		// of the file into the buffer
		// m_Buffer = new unsigned char[m_Length];
		m_Buffer.reserve(m_Length);
		retval = bool(file.Read (m_Buffer.data(), m_Length) == (int)m_Length);

		// If we failed, free the buffer
		if (retval == false) {
			Free_Buffer ();
		}
		Determine_Stats (m_Buffer.data());
	}
#endif

	// Close the file if necessary
	if (we_opened) {
		file.Close ();
	}

	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Load_From_Memory
//
bool
SoundBufferClass::Load_From_Memory
(
	unsigned char *mem_buffer,
	unsigned int size
)
{
	MMSLockClass lock;

	// Assume failure
	bool retval = false;

	// Start from scratch
	Free_Buffer ();
	Set_Filename ("unknown.wav");

	// Params OK?
	WWASSERT (mem_buffer != NULL);
	WWASSERT (size > 0L);
	if ((mem_buffer != NULL) && (size > 0L)) {

		// Allocate a new buffer of the correct length and copy the contents
		// into the buffer
		m_Length = size;
		m_Buffer.reserve(m_Length);
		::memcpy (m_Buffer.data(), mem_buffer, size);
		retval = true;

		// If we failed, free the buffer
		if (retval == false) {
			Free_Buffer ();
		}
		Determine_Stats (m_Buffer.data());
	}

	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	StreamSoundBufferClass
//
StreamSoundBufferClass::StreamSoundBufferClass (void)	:
	  SoundBufferClass ()
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	~StreamSoundBufferClass
//
StreamSoundBufferClass::~StreamSoundBufferClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Free_Buffer
//
void
StreamSoundBufferClass::Free_Buffer (void)
{
	m_Buffer.clear();
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Load_From_File
//
/////////////////////////////////////////////////////////////////////////////////
bool
StreamSoundBufferClass::Load_From_File (const char *filename)
{
	WWPROFILE ("StreamSoundBufferClass::Load_From_File");
	return true;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Load_From_File
//
/////////////////////////////////////////////////////////////////////////////////
bool
StreamSoundBufferClass::Load_From_File (FileClass &file)
{
	WWPROFILE ("StreamSoundBufferClass::Load_From_File");
	MMSLockClass lock;

	// Start from scratch
	Free_Buffer ();
	Set_Filename (file.File_Name ());

	// Open the file if necessary
	bool we_opened = false;
	if (file.Is_Open () == false) {
		we_opened = (file.Open () == true);
	}

	m_Length = file.Size ();
	
#ifndef W3D_HAS_FFMPEG
	// Allocate a new buffer of the correct length and read the contents
	// of the file into the buffer
	unsigned char buffer[4096] = { 0 };
	file.Read (buffer, sizeof (buffer));
	Determine_Stats (buffer);
#else
	m_FileHandle.Open(&file);

	if (!m_FileHandle.Has_Audio()) {
		return false;
	}

	m_Duration = m_FileHandle.Get_Duration();
	m_Rate = m_FileHandle.Get_Sample_Rate();
	m_Channels = m_FileHandle.Get_Num_Channels();
	m_Bits = m_FileHandle.Get_Bytes_Per_Sample() * 8;
	m_FileHandle.Close();
#endif

	// Close the file if necessary
	if (we_opened) {
		file.Close ();
	}

	return true;
}

bool StreamSoundBufferClass::Refresh_Buffer()
{
#ifdef W3D_HAS_FFMPEG
	if (!m_FileHandle.Has_Audio()) {
		m_FileHandle.Open(Get_Filename());

		if (!m_FileHandle.Has_Audio()) {
			WWDEBUG_SAY(("No audio detected in %s\n", Get_Filename()));
			return false;
		}
	}
	
	m_Buffer.clear();
	
	FFmpegFrameCallback on_frame = [](AVFrame* frame, int stream_idx, int stream_type, void* user_data) {
		StreamSoundBufferClass* sbc = static_cast<StreamSoundBufferClass*>(user_data);
		if (stream_type != AVMEDIA_TYPE_AUDIO) {
			return;
		}

		const int frame_data_size = av_samples_get_buffer_size(nullptr,  sbc->m_Channels, frame->nb_samples, static_cast<AVSampleFormat>(frame->format), 1);
		sbc->m_Buffer.reserve(sbc->m_Buffer.size() + frame_data_size);

		if (av_sample_fmt_is_planar(static_cast<AVSampleFormat>(frame->format))) {
			// Convert planar audio to interleaved
			int num_channels = sbc->m_Channels;
			int bytes_per_sample = sbc->m_Bits / 8;
			for (int sample = 0; sample < frame->nb_samples; ++sample) {
				for (int channel = 0; channel < num_channels; ++channel) {
					const uint8_t* src = frame->data[channel] + sample * bytes_per_sample;
					sbc->m_Buffer.insert(sbc->m_Buffer.end(), src, src + bytes_per_sample);
				}
			}
		} else {
			// Directly copy interleaved audio
			sbc->m_Buffer.insert(sbc->m_Buffer.end(), frame->data[0], frame->data[0] + frame_data_size);
		}
	};

	m_FileHandle.Set_Frame_Callback(on_frame);
	m_FileHandle.Set_User_Data(this);

	// Read packets inside the file
	while (m_Buffer.size() < MAX_STREAM_BUFFER) {
		if (!m_FileHandle.Decode_Packet()) {
			return false;
		}
	}
	return true;
#else
	return false;
#endif
}

void StreamSoundBufferClass::Reset_Buffer()
{
#ifdef W3D_HAS_FFMPEG
	m_FileHandle.Rewind();
#endif
}
