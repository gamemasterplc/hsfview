#ifndef DISPLAY_H
#define DISPLAY_H

#define NORMAL_VIEW_WIDTH 640
#define WIDE_VIEW_WIDTH 852
#define SQUARE_PAR 1.0f
#define VIEW_HEIGHT 480
#define PAL_FRAMERATE 50.0f
#define NTSC_FRAMERATE 60.0f

u32	framebuffer_idx; //Framebuffer Index Variable
void *framebuffer[2];
int frame_delay;

void InitDisplay();

void InitVI();

void InitGX();

void SetBackgroundColor(u8 r, u8 g, u8 b);

void SetWidescreenFlag(bool widescreen_flag);

bool GetWidescreenFlag();

int GetRenderWidth();

int GetRenderHeight();

int GetVideoHeight();

float GetViewWidth();

float GetViewHeight();

float GetViewAspectRatio();

float GetFrameRate();

void EndFrame();

#endif