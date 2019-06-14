#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include "vgafont_tpl.h"
#include "vgafont.h"
#include "display.h"
#include "print.h"

static GXTexObj font_tex_obj;
static GXColor text_color;

void PrintInit()
{
	TPLFile fontTPL;
	TPL_OpenTPLFromMemory(&fontTPL, (void *)vgafont_tpl, vgafont_tpl_size);
	TPL_GetTexture(&fontTPL, vgafont, &font_tex_obj);
	GX_InitTexObjFilterMode(&font_tex_obj, GX_NEAR, GX_NEAR);
	SetTextColor(255, 255, 255);
}

float GetStringWidth(float scale, char *string)
{
	float width = 0.0f;
	while(*string != 0)
	{
		width += CHAR_WIDTH*scale;
		string++;
	}
	return width;
}

void SetTextColor(char r, char g, char b)
{
	text_color.r = r;
	text_color.g = g;
	text_color.b = b;
	text_color.a = 255;
}

void PrintCenteredString(float x, float y, float scale, char *string)
{
	float str_width = 0.5*GetStringWidth(scale, string);
	PrintString(x-str_width, y, scale, string);
}

void PrintRightAlignedString(float x, float y, float scale, char *string)
{
	float str_width = GetStringWidth(scale, string);
	PrintString(x-str_width, y, scale, string);
}

void PrintDropShadowString(float x, float y, float scale, char *string)
{
	SetTextColor(0, 0, 0);
	PrintString(x+scale, y+scale, scale, string);
	SetTextColor(255, 255, 255);
	PrintString(x, y, scale, string);
}

void PrintDropShadowCenteredString(float x, float y, float scale, char *string)
{
	float str_width = 0.5*GetStringWidth(scale, string);
	SetTextColor(0, 0, 0);
	PrintString(x-str_width+scale, y+scale, scale, string);
	SetTextColor(255, 255, 255);
	PrintString(x-str_width, y, scale, string);
}

void PrintDropShadowRightAlignedString(float x, float y, float scale, char *string)
{
	float str_width = GetStringWidth(scale, string);
	SetTextColor(0, 0, 0);
	PrintString(x-str_width+scale, y+scale, scale, string);
	SetTextColor(255, 255, 255);
	PrintString(x-str_width, y, scale, string);
}

void PrintRectangle(float x, float y, float width, float height, char r, char g, char b, char a)
{
	Mtx44 projection;
	Mtx modelview;
	GX_InvalidateTexAll();
	guOrtho(projection, 0, GetViewHeight(), 0, GetViewWidth(), 0, 10);
	GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);
	GX_SetViewport(0, 0, GetRenderWidth(), GetRenderHeight(), 0, 1);
	GX_SetScissor(0, 0, GetRenderWidth(), GetRenderHeight());
	guMtxIdentity(modelview);
	GX_LoadPosMtxImm(modelview, GX_PNMTX0);
	GX_SetNumTevStages(1);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZCompLoc(GX_FALSE);
	GX_SetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetNumTexGens(1);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position2f32(x, y);
	GX_Color4u8(r, g, b, a);
	GX_Position2f32(x+width, y);
	GX_Color4u8(r, g, b, a);
	GX_Position2f32(x+width, y+height);
	GX_Color4u8(r, g, b, a);
	GX_Position2f32(x, y+height);
	GX_Color4u8(r, g, b, a);
	GX_End();
}

void PrintString(float x, float y, float scale, char *string)
{
	Mtx44 projection;
	Mtx modelview;
	float scaled_width = CHAR_WIDTH*scale;
	float scaled_height = CHAR_HEIGHT*scale;
	int font_tex_width = GX_GetTexObjWidth(&font_tex_obj);
	int font_tex_height = GX_GetTexObjHeight(&font_tex_obj);
	int tex_col_count = font_tex_width/CHAR_WIDTH;
	float texcoord_x_spacing = CHAR_WIDTH/(float)font_tex_width;
	float texcoord_y_spacing = CHAR_HEIGHT/(float)font_tex_height;
	
	GX_InvalidateTexAll();
	guOrtho(projection, 0, GetViewHeight(), 0, GetViewWidth(), 0, 10);
	GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);
	GX_SetViewport(0, 0, GetRenderWidth(), GetRenderHeight(), 0, 1);
	GX_SetScissor(0, 0, GetRenderWidth(), GetRenderHeight());
	guMtxIdentity(modelview);
	GX_LoadPosMtxImm(modelview, GX_PNMTX0);
	GX_LoadTexObj(&font_tex_obj, GX_TEXMAP0);
	GX_SetNumTevStages(1);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZCompLoc(GX_FALSE);
	GX_SetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetNumTexGens(1);
	while(*string != 0)
	{
		char c = *string;
		int row = c/tex_col_count;
		int col = c%tex_col_count;
		float texcoord_x = col*texcoord_x_spacing;
		float texcoord_y = row*texcoord_y_spacing;
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position2f32(x, y);
		GX_Color4u8(text_color.r, text_color.g, text_color.b, 255);
		GX_TexCoord2f32(texcoord_x, texcoord_y);
		GX_Position2f32(x+scaled_width, y);
		GX_Color4u8(text_color.r, text_color.g, text_color.b, 255);
		GX_TexCoord2f32(texcoord_x+texcoord_x_spacing, texcoord_y);
		GX_Position2f32(x+scaled_width, y+scaled_height);
		GX_Color4u8(text_color.r, text_color.g, text_color.b, 255);
		GX_TexCoord2f32(texcoord_x+texcoord_x_spacing, texcoord_y+texcoord_y_spacing);
		GX_Position2f32(x, y+scaled_height);
		GX_Color4u8(text_color.r, text_color.g, text_color.b, 255);
		GX_TexCoord2f32(texcoord_x, texcoord_y+texcoord_y_spacing);
		GX_End();
		x += scaled_width;
		string++;
	}
}