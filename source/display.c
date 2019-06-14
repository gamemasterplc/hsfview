#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include "display.h"

#define FIFO_SIZE (256*1024)
#define MAX_Z 0x00FFFFFF

static GXRModeObj *rmode;
static GXColor background_color;
static bool widescreen;

void InitDisplay()
{
	bool widescreen_flag = false;
	
	SetBackgroundColor(0, 0, 0);
	InitVI();
	InitGX();
	
	#if defined(HW_RVL)
	if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
	{
		widescreen_flag = true;
	}
	#endif
	SetWidescreenFlag(widescreen_flag);
}

void InitVI()
{
	VIDEO_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	framebuffer_idx = 0;
	
	rmode->viWidth = 672;
 	rmode->viXOrigin = (VI_MAX_WIDTH_NTSC-672)/2;
	
	#if defined(HW_RVL)
	if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
	{
		rmode->viWidth = 678;
		rmode->viXOrigin = (VI_MAX_WIDTH_NTSC-678)/2;
	}
	#endif
	
	//Get Double Buffer Framebuffer
	framebuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	framebuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(framebuffer[framebuffer_idx]);
	VIDEO_SetBlack(true);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if((rmode->viTVMode&VI_NON_INTERLACE) != 0)
	{	
		VIDEO_WaitVSync(); //Non-Interlace Modes Wait Extra Frame
	}
	VIDEO_SetBlack(false);
	framebuffer_idx ^= 1; //Flip Framebuffer
}

void InitGX()
{
	void *gp_fifo = NULL;
	f32 yscale;
	u32 xfbHeight;
	
	//Allocate Command Buffer for GX
	gp_fifo = memalign(32, FIFO_SIZE);
	memset(gp_fifo, 0, FIFO_SIZE);
	GX_Init(gp_fifo, FIFO_SIZE);
	GX_SetCopyClear(background_color, MAX_Z);
	
	GX_SetViewport(0, 0, GetRenderWidth(), GetRenderHeight(), 0, 1);
	yscale = GX_GetYScaleFactor(GetRenderHeight(), GetVideoHeight());
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0,0,GetRenderWidth(), GetRenderHeight());
	GX_SetDispCopySrc(0,0,GetRenderWidth(), GetRenderHeight());
	GX_SetDispCopyDst(GetRenderWidth(), xfbHeight);
	GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight==2*GetVideoHeight())?GX_ENABLE:GX_DISABLE));
	
	if (rmode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
	
	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(framebuffer[framebuffer_idx], GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);
	
	//Setup the Vertex Descriptor
	//Tells the GX to Expect Direct Data
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	
	//Texture Initialization
	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetNumTevStages(1);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_InvalidateTexAll();
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_SetZCompLoc(GX_FALSE);
}

void SetBackgroundColor(u8 r, u8 g, u8 b)
{
	background_color.r = r;
	background_color.g = g;
	background_color.b = b;
	background_color.a = 255;
}

void SetWidescreenFlag(bool widescreen_flag)
{
	widescreen = widescreen_flag;
}

bool GetWidescreenFlag()
{
	return widescreen;
}

int GetRenderWidth()
{
	return rmode->fbWidth;
}

int GetRenderHeight()
{
	return rmode->efbHeight;
}

int GetVideoHeight()
{
	return rmode->xfbHeight;
}

float GetViewWidth()
{
	if(widescreen == true)
	{
		return WIDE_VIEW_WIDTH;
	}
	else
	{
		return NORMAL_VIEW_WIDTH;
	}
}

float GetViewHeight()
{
	return VIEW_HEIGHT;
}

float GetViewAspectRatioX()
{
	return GetViewWidth()/(float)GetViewHeight();
}

float GetViewAspectRatioY()
{
	if (GetViewAspectRatioX() < (4.0f / 3.0f))
	{
		return GetViewAspectRatioX() / (4.0f / 3.0f);
	}
	else
	{
		return (4.0f / 3.0f) / (4.0f / 3.0f);
	}
}

void EndFrame()
{
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_DrawDone();
	GX_SetCopyClear(background_color, MAX_Z);
	GX_CopyDisp(framebuffer[framebuffer_idx], GX_TRUE);
	VIDEO_SetNextFramebuffer(framebuffer[framebuffer_idx]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	framebuffer_idx ^= 1; //Flip Framebuffer Index
}
