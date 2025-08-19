/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 OpenW3D.
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
#include "FFMpegPlayer.h"
#include "FFMpegFile.h"
#include "formconv.h"
#include "dx8wrapper.h"
#include "subtitlemanager.h"
#include "wwdebug.h"
#include <chrono>
#include <d3d9types.h>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
}

void FFMpegMovieClass::On_Frame(AVFrame *frame, int stream_idx, int stream_type, void *user_data)
{
	FFMpegMovieClass *movie = static_cast<FFMpegMovieClass *>(user_data);
	if (stream_type == AVMEDIA_TYPE_VIDEO) {
		av_frame_free(&movie->CurrentFrame);
		movie->CurrentFrame = av_frame_clone(frame);
		movie->GotFrame = true;
	}
}

FFMpegMovieClass::FFMpegMovieClass(const char *filename, const char *subtitlename,FontCharsClass *font)	:
	TicksPerFrame(0),
	Bink(nullptr),
	CurrentFrame(nullptr),
	ScalingContext(nullptr),
	GotFrame(false),
	FrameChanged(true),
	SubTitleManager(nullptr)
{
	Bink = new FFmpegFile(filename);

	if (Bink == nullptr) {
		return;
	}

	// Can't play a movie if no video in the file.
	if (!Bink->hasVideo())
	{
		delete Bink;
		Bink = nullptr;
		return;
	}

	Bink->setFrameCallback(On_Frame);
	Bink->setUserData(this);

	bool good = true;
	// Decode until we have our first video frame
	while (good && GotFrame == false)
		good = Bink->decodePacket();

	const D3DCAPS9& dx8caps = DX8Wrapper::Get_Current_Caps()->Get_DX8_Caps();
	unsigned poweroftwowidth = 1;

	while (poweroftwowidth < static_cast<unsigned>(Bink->getWidth())) {
		poweroftwowidth <<= 1;
	}

	unsigned poweroftwoheight = 1;
	
	while (poweroftwoheight < static_cast<unsigned>(Bink->getHeight())) {
		poweroftwoheight <<= 1;
	}

	if (poweroftwowidth > dx8caps.MaxTextureWidth) {
		poweroftwowidth = dx8caps.MaxTextureWidth;
	}
	
	if (poweroftwoheight > dx8caps.MaxTextureHeight) {
		poweroftwoheight = dx8caps.MaxTextureHeight;
	}

	TextureCount = 0;
	unsigned max_width = poweroftwowidth;
	unsigned max_height = poweroftwoheight;
	int x, y;

	for (y = 0; y < Bink->getHeight(); y += max_height-2) {		// Two pixels are lost due to duplicated edges to prevent bilinear artifacts
		for (x = 0; x < Bink->getWidth(); x += max_width-2) {
			++TextureCount;
		}
	}

	TextureInfos = new TextureInfoStruct[TextureCount];
	unsigned cnt = 0;
	
	for (y = 0; y < Bink->getHeight(); y += max_height-1) {
		for (x = 0; x < Bink->getWidth(); x += max_width-1) {
			TextureInfos[cnt].Texture = new TextureClass(
				max_width, max_height, D3DFormat_To_WW3DFormat(D3DFMT_R5G6B5),
				TextureClass::MIP_LEVELS_1, TextureClass::POOL_MANAGED, false);

			TextureInfos[cnt].TextureLocX = x;
			TextureInfos[cnt].TextureLocY = y;
			TextureInfos[cnt].TextureWidth = max_width;
			TextureInfos[cnt].UV.Right = float(max_width) / float(max_width);

			if ((TextureInfos[cnt].TextureWidth + x) > Bink->getWidth()) {
				TextureInfos[cnt].TextureWidth = Bink->getWidth() - x;
				TextureInfos[cnt].UV.Right = float(TextureInfos[cnt].TextureWidth - 1) / float(max_width);
			}

			TextureInfos[cnt].TextureHeight = max_height;
			TextureInfos[cnt].UV.Bottom = float(max_height) / float(max_height);

			if ((TextureInfos[cnt].TextureHeight + y) > Bink->getHeight()) {
				TextureInfos[cnt].TextureHeight = Bink->getHeight() - y;
				TextureInfos[cnt].UV.Bottom = float(TextureInfos[cnt].TextureHeight + 1) / float(max_height);
			}

			TextureInfos[cnt].UV.Left = 1.0f / float(max_width);
			TextureInfos[cnt].UV.Top = 1.0f / float(max_height);

			TextureInfos[cnt].Rect.Left = float(TextureInfos[cnt].TextureLocX) / float(Bink->getWidth());
			TextureInfos[cnt].Rect.Top = float(TextureInfos[cnt].TextureLocY) / float(Bink->getHeight());
			TextureInfos[cnt].Rect.Right = float(TextureInfos[cnt].TextureLocX + TextureInfos[cnt].TextureWidth) / float(Bink->getWidth());
			TextureInfos[cnt].Rect.Bottom = float(TextureInfos[cnt].TextureLocY + TextureInfos[cnt].TextureHeight) / float(Bink->getHeight());

			++cnt;
		}
	}

	Renderer.Reset();

	// Calculate the time per frame of video
	unsigned int rate = Bink->getFrameTime();
	TicksPerFrame = (60 / rate);
	StartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	if (subtitlename && font) {
		SubTitleManager = SubTitleManagerClass::Create(filename, subtitlename, font);
	}
}

FFMpegMovieClass::~FFMpegMovieClass()
{
	if (Bink == nullptr) {
		return;
	}

	if (Bink) {
		delete Bink;
		Bink = nullptr;
	}

	if (TextureInfos) {
		for (unsigned t = 0; t < TextureCount; ++t) {
			REF_PTR_RELEASE(TextureInfos[t].Texture);
		}

		delete[] TextureInfos;
	}

	if (SubTitleManager) {
		delete SubTitleManager;
	}
}

void FFMpegMovieClass::Update()
{
	if (Bink == nullptr)
		return;

	uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	FrameChanged = (time - StartTime) >= Bink->getFrameTime() * Bink->getCurrentFrame();
}

void FFMpegMovieClass::Render() 
{
	if (Bink == nullptr)
		return;

	if (CurrentFrame == nullptr)
		return;

	if (CurrentFrame->data == nullptr)
		return;

	// decompress a frame
	if (FrameChanged) {
		FrameChanged = false;

		for (unsigned t = 0; t < TextureCount; ++t) {
			IDirect3DTexture9* d3d_texture = TextureInfos[t].Texture->Peek_DX8_Texture();

			if (d3d_texture) {
				unsigned w = TextureInfos[t].TextureWidth;
				unsigned h = TextureInfos[t].TextureHeight;

				if (w > static_cast<unsigned>(Bink->getWidth()-TextureInfos[t].TextureLocX)) {
					w = Bink->getWidth()-TextureInfos[t].TextureLocX;
				}

				if (h > static_cast<unsigned>(Bink->getHeight()-TextureInfos[t].TextureLocY)) {
					h = Bink->getHeight()-TextureInfos[t].TextureLocY;
				}

				D3DSURFACE_DESC d3d_surf_desc;
				D3DLOCKED_RECT locked_rect;

				DX8_ErrorCode(d3d_texture->GetLevelDesc(0, &d3d_surf_desc));
				AVPixelFormat dst_pix_fmt;
				switch (d3d_surf_desc.Format) {
					case D3DFMT_R8G8B8:
						dst_pix_fmt = AV_PIX_FMT_RGB24;
						break;
					case D3DFMT_X8R8G8B8:
						dst_pix_fmt = AV_PIX_FMT_BGR0;
						break;
					case D3DFMT_R5G6B5:
						dst_pix_fmt = AV_PIX_FMT_RGB565;
						break;
					case D3DFMT_X1R5G5B5:
						dst_pix_fmt = AV_PIX_FMT_RGB555;
						break;
					default:
						return;
				}

				ScalingContext = sws_getCachedContext(ScalingContext,
					Bink->getWidth(),
					Bink->getHeight(),
					static_cast<AVPixelFormat>(CurrentFrame->format),
					w,
					h,
					dst_pix_fmt,
					SWS_BICUBIC,
					nullptr,
					nullptr,
					nullptr);

				RECT rect;
				rect.left = 0;
				rect.top = 0;
				rect.right = w;
				rect.bottom = h;
				DX8_ErrorCode(d3d_texture->LockRect(0,&locked_rect,&rect,0));
				
				int dst_strides[] = { locked_rect.Pitch };
				uint8_t *dst_data[] = { static_cast<uint8_t *>(locked_rect.pBits) };
				[[maybe_unused]] int result =
					sws_scale(ScalingContext, CurrentFrame->data, CurrentFrame->linesize, 0, h, dst_data, dst_strides);
				
				WWASSERT_PRINT(result > 0, ("Failed to scale frame"));
				DX8_ErrorCode(d3d_texture->UnlockRect(0));
			}
			GotFrame = false;
		}

		bool good = true;
		// Decode until we have our next video frame
		while (good && GotFrame == false)
			good = Bink->decodePacket();
	}

	for (unsigned t = 0; t < TextureCount; ++t) {
		Renderer.Reset();
		Renderer.Set_Texture(TextureInfos[t].Texture);
		Renderer.Set_Coordinate_Range(RectClass(0.0f, 0.0f, 1.0f, 1.0f));//Bink->getWidth(),Bink->getHeight()));

		RectClass rect(TextureInfos[t].TextureLocX, TextureInfos[t].TextureLocY, TextureInfos[t].TextureWidth, TextureInfos[t].TextureHeight);
		Renderer.Add_Quad(TextureInfos[t].Rect, TextureInfos[t].UV, 0xffffffff);
		Renderer.Render();
	}

	if (SubTitleManager) {
		unsigned long movieTime = (Bink->getCurrentFrame() * TicksPerFrame);
		SubTitleManager->Process(movieTime);
		SubTitleManager->Render();
	}
}

bool FFMpegMovieClass::Is_Complete()
{ 
	if (Bink == nullptr)
		return true;

	return (Bink->getCurrentFrame() >= Bink->getNumFrames());
}
