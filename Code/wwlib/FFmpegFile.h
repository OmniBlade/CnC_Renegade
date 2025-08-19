/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 OpenW3D
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

//////// FFmpegFile.h ///////////////////////////
// Stephan Vedder, April 2025
/////////////////////////////////////////////////

#pragma once

#include <functional>
#include <vector>

struct AVFormatContext;
struct AVIOContext;
struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
class FileClass;
class FileFactoryClass;

using FFmpegFrameCallback = std::function<void(AVFrame *, int, int, void *)>;

class FFmpegFile
{
public:
	FFmpegFile();
	// The constructur takes ownership of the file
	explicit FFmpegFile(const char *file);
	~FFmpegFile();

	bool open(const char *file);
	void close();
	void setFrameCallback(FFmpegFrameCallback callback) { m_frameCallback = callback; }
	void setUserData(void *user_data) { m_userData = user_data; }
	// Read & decode a packet from the container. Note that we could/should split this step
	bool decodePacket();
	void seekFrame(int frame_idx);
	bool hasAudio() const;

	// Audio specific
	int getSizeForSamples(int numSamples) const;
	int getNumChannels() const;
	int getSampleRate() const;
	int getBytesPerSample() const;

	// Video specific
	int getWidth() const;
	int getHeight() const;
	int getNumFrames() const;
	int getCurrentFrame() const;
	int getPixelFormat() const;
	unsigned getFrameTime() const;

private:
	struct FFmpegStream
	{
		AVCodecContext *codec_ctx = nullptr;
		const AVCodec *codec = nullptr;
		int stream_idx = -1;
		int stream_type = -1;
		AVFrame *frame = nullptr;
	};

	static int readPacket(void *opaque, unsigned char *buf, int buf_size);
	const FFmpegStream *findMatch(int type) const;

	FFmpegFrameCallback 		m_frameCallback = nullptr; ///< Callback for frame processing
	AVFormatContext 			*m_fmtCtx = nullptr; ///< Format context for AVFormat
	AVIOContext 				*m_avioCtx = nullptr; ///< IO context for AVFormat
	AVPacket 					*m_packet = nullptr; ///< Current packet
	std::vector<FFmpegStream> 	m_streams; ///< List of streams in the file
	FileClass 					*m_file = nullptr;	///< File handle for the file
	FileFactoryClass			*m_factory = nullptr;
	void 						*m_userData = nullptr; ///< User data for the callback
};
