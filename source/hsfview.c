#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <fat.h>
#ifdef HW_RVL
#include <wiiuse/wpad.h>
#include <wiiuse/wiiuse.h>
#include <sdcard/wiisd_io.h>
#endif
#include <sdcard/gcsd.h>
#include "alignedalloc.h"
#include "display.h"
#include "hsffile.h"
#include "hsfload.h"
#include "hsfdraw.h"
#include "model_hsf.h"
#include "print.h"

void CalcLookAt(guVector *rot, guVector *center, float dist, Mtx *view);

typedef struct global_settings
{
	float cam_near;
	float cam_far;
	float cam_fov;
	float cam_zoom;
	float rot_speed;
	float zoom_speed;
	float move_speed;
	int widescreen_flag;
} GlobalSettings;

static GlobalSettings globals =
{
	20.0f,
	20000.0f,
	45.0f,
	1.0f,
	30.0f,
	28.0f,
	30.0f,
	false
};

int main(int argc, char **argv)
{
	Mtx44 projection;
	Mtx mv;
	FILE *hsf_fp;
	float cam_rot_speed_x;
	float cam_rot_speed_y;
	float move_x_speed;
	float move_y_speed;
	int file_size;
	guVector cam_rot = { -20.0f, 0.0f, 0.0f };
	guVector cam_center = { 0.0f, 100.0f, 0.0f };
	float dist = 400.0f;
	HSFHeader *hsf_file = (HSFHeader *)model_hsf;
	
	InitDisplay();
	PAD_Init();
	__io_gcsdb.startup();
	if (__io_gcsdb.isInserted() == true)
	{
		fatMountSimple("sdb", &__io_gcsdb);
		hsf_fp = fopen("sdb:/model.hsf", "rb");
		if(hsf_fp != NULL)
		{
			fseek(hsf_fp, 0L, SEEK_END);
			file_size = ftell(hsf_fp);
			hsf_file = malloc_aligned(32, file_size);
			fread(hsf_file, file_size, 1, hsf_fp);
		}
	}
	#ifdef HW_RVL
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	__io_wiisd.startup();
	if (__io_wiisd.isInserted() == true)
	{
		fatMountSimple("sd", &__io_wiisd);
		hsf_fp = fopen("sd:/model.hsf", "rb");
		if(hsf_fp != NULL)
		{
			fseek(hsf_fp, 0L, SEEK_END);
			file_size = ftell(hsf_fp);
			hsf_file = malloc_aligned(32, file_size);
			fread(hsf_file, file_size, 1, hsf_fp);
		}
	}
	#endif
	LoadHsfModel(hsf_file);
	PrintInit();
	while(1)
	{
		cam_rot_speed_x = 0.0f;
		cam_rot_speed_y = 0.0f;
		move_x_speed = 0.0f;
		move_y_speed = 0.0f;
		if((PAD_ScanPads() & 0x1) != 0) //Check if P1's GameCube Controller is Connected
		{
			if(PAD_ButtonsHeld(0) & PAD_TRIGGER_L)
			{
				dist += globals.zoom_speed;
			}
			if(PAD_ButtonsHeld(0) & PAD_TRIGGER_R)
			{
				dist -= globals.zoom_speed;
			}
			if(PAD_ButtonsDown(0) & PAD_TRIGGER_Z)
			{
				globals.widescreen_flag ^= 1;
				SetWidescreenFlag(globals.widescreen_flag);
			}
			move_x_speed = PAD_SubStickX(0);
			move_y_speed = PAD_SubStickY(0);
			move_x_speed *= (globals.move_speed/900);
			move_y_speed *= (globals.move_speed/900);
			if(move_x_speed < 0.5f && move_x_speed > -0.5f)
			{
				move_x_speed = 0.0f;
			}
			if(move_y_speed < 0.5f && move_y_speed > -0.5f)
			{
				move_y_speed = 0.0f;
			}
			cam_center.z += (move_x_speed*sin(DegToRad(-cam_rot.y)));
			cam_center.x += (move_x_speed*cos(DegToRad(-cam_rot.y)));
			cam_center.z += (move_y_speed*sin(DegToRad(cam_rot.x)));
			cam_center.y += (move_y_speed*cos(DegToRad(cam_rot.x)));
			cam_rot_speed_x = PAD_StickX(0);
			cam_rot_speed_y = PAD_StickY(0);
			cam_rot_speed_x *= (globals.rot_speed/900);
			cam_rot_speed_y *= (globals.rot_speed/900);
		}
		#ifdef HW_RVL
		WPAD_ScanPads();
		if(WPAD_Probe(0, NULL) == WPAD_ERR_NONE)
		{
			WPADData *wpad = WPAD_Data(0);
			if(wpad->btns_h & WPAD_BUTTON_1)
			{
				dist += globals.zoom_speed;
			}
			if(wpad->btns_h & WPAD_BUTTON_2)
			{
				dist -= globals.zoom_speed;
			}
			if(wpad->btns_h & WPAD_BUTTON_LEFT)
			{
				move_x_speed = -128*(globals.move_speed/900);
			}
			if(wpad->btns_h & WPAD_BUTTON_RIGHT)
			{
				move_x_speed = 128*(globals.move_speed/900);
			}
			if(wpad->btns_h & WPAD_BUTTON_UP)
			{
				move_y_speed = 128*(globals.move_speed/900);
			}
			if(wpad->btns_h & WPAD_BUTTON_DOWN)
			{
				move_y_speed = -128*(globals.move_speed/900);
			}
			if(wpad->btns_h & WPAD_BUTTON_MINUS)
			{
				globals.widescreen_flag ^= 1;
				SetWidescreenFlag(globals.widescreen_flag);
			}
			cam_center.z += (move_x_speed*sin(DegToRad(-cam_rot.y)));
			cam_center.x += (move_x_speed*cos(DegToRad(-cam_rot.y)));
			cam_center.y += (move_y_speed*cos(DegToRad(cam_rot.x)));
			cam_center.z += (move_y_speed*sin(DegToRad(cam_rot.x)));
			if(wpad->exp.type == EXP_NUNCHUK)
			{
				cam_rot_speed_x = wpad->exp.nunchuk.js.pos.x-128;
				cam_rot_speed_y = wpad->exp.nunchuk.js.pos.y-128;
				cam_rot_speed_x *= (globals.rot_speed/900);
				cam_rot_speed_y *= (globals.rot_speed/900);
			}
		}
		#endif
		if(dist < 0.0f)
		{
			dist += globals.zoom_speed;
		}
		if(cam_rot_speed_x < 0.5f && cam_rot_speed_x > -0.5f)
		{
			cam_rot_speed_x = 0.0f;
		}
		if(cam_rot_speed_y < 0.5f && cam_rot_speed_y > -0.5f)
		{
			cam_rot_speed_y = 0.0f;
		}
		cam_rot.y += cam_rot_speed_x;
		cam_rot.x += cam_rot_speed_y;
		if(cam_rot.y >= 360.0f)
		{
			cam_rot.y -= 360.0f;
		}
		if(cam_rot.x >= 360.0f)
		{
			cam_rot.x -= 360.0f;
		}
		if(cam_rot.y < 0.0f)
		{
			cam_rot.y += 360.0f;
		}
		if(cam_rot.x < 0.0f)
		{
			cam_rot.x += 360.0f;
		}
		GX_SetViewport(0, 0, GetRenderWidth(), GetRenderHeight(), 0, 1);
		GX_SetScissor(0, 0, GetRenderWidth(), GetRenderHeight());
		guPerspective(projection, RadToDeg(2.0f * atan(tan(DegToRad(globals.cam_fov / 2.0f)) / GetViewAspectRatioY() / globals.cam_zoom)), GetViewAspectRatioX(), globals.cam_near, globals.cam_far);
		GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);
		guMtxIdentity(mv);
		CalcLookAt(&cam_rot, &cam_center, dist, &mv);
		DrawHsfModel((HSFModel *)hsf_file, &mv);
		EndFrame();
	}
	return 0;
}

void CalcLookAt(guVector *rot, guVector *center, float dist, Mtx *view)
{
	guVector pos = {0.0f, 0.0f, 0.0f},
		up = {0.0f, 1.0f, 0.0f},
	target = {0.0f, 0.0f, -1.0f};
	
	pos.x = (cos(DegToRad(rot->x))*sin(DegToRad(rot->y))*dist)+center->x;
	pos.y = (-dist*sin(DegToRad(rot->x)))+center->y;
	pos.z = (cos(DegToRad(rot->x))*cos(DegToRad(rot->y))*dist)+center->z;
	target.x = center->x;
	target.y = center->y;
	target.z = center->z;
	up.x = sin(DegToRad(rot->x))*sin(DegToRad(rot->y));
	up.y = cos(DegToRad(rot->x));
	up.z = sin(DegToRad(rot->x))*cos(DegToRad(rot->y));
	guLookAt(*view, &pos, &up, &target);
}
