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
#pragma once

#include "rect.h"
#include "render2dsentence.h"
#include "wwstring.h"

class FontCharsClass;
class SubTitleManagerClass;
class FFmpegFile;
struct AVFrame;
struct SwsContext;

class FFMpegMovieClass
{
private:
	StringClass Filename;
	bool FrameChanged;
	unsigned TextureCount;
	unsigned TicksPerFrame;
	FFmpegFile *Bink;
	AVFrame *CurrentFrame;
	SwsContext *ScalingContext;
	uint64_t StartTime;
	bool StreamValid;

	struct TextureInfoStruct {
		TextureClass* Texture;
		int TextureWidth;
		int TextureHeight;
		int TextureLocX;
		int TextureLocY;
		RectClass UV;
		RectClass Rect;
	};

	TextureInfoStruct* TextureInfos;
	unsigned char* TempBuffer;
	Render2DClass Renderer;
	SubTitleManagerClass* SubTitleManager;

	static void On_Frame(AVFrame *frame, int stream_idx, int stream_type, void *user_data);
public:
	FFMpegMovieClass(const char* filename,const char* subtitlename,FontCharsClass* font);
	~FFMpegMovieClass();

	void Update();
	void Render();
	bool Is_Complete();
};
