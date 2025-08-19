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
#include "FFmpegFile.h"
#include "formconv.h"
#include "dx8wrapper.h"
#include "subtitlemanager.h"
#include <chrono>

FFMpegMovieClass::FFMpegMovieClass(const char *filename, const char *subtitlename,FontCharsClass *font)	:
	Filename(filename),
	FrameChanged(true),
	TicksPerFrame(0),
	Bink(nullptr),
	SubTitleManager(NULL)
{
	Bink = new FFmpegFile(Filename);

	if (Bink == nullptr) {
		return;
	}

	TempBuffer = new unsigned char[Bink->getWidth() * Bink->getHeight() * 2];

	const D3DCAPS8& dx8caps = DX8Wrapper::Get_Current_Caps()->Get_DX8_Caps();
	unsigned poweroftwowidth = 1;

	while (poweroftwowidth < Bink->getWidth()) {
		poweroftwowidth <<= 1;
	}

	unsigned poweroftwoheight = 1;
	
	while (poweroftwoheight < Bink->getHeight()) {
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
	unsigned x, y;

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

	delete[] TempBuffer;

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

void FFMpegMovieClass::Render() {}

bool FFMpegMovieClass::Is_Complete()
{ 
	if (!Bink)
		return true;

	return (Bink->getCurrentFrame() >= Bink->getNumFrames());
}
