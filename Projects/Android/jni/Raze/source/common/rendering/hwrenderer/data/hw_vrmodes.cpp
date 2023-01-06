// 
//---------------------------------------------------------------------------
//
// Copyright(C) 2015 Christopher Bruns
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
//--------------------------------------------------------------------------
//
/*
** gl_stereo_leftright.cpp
** Offsets for left and right eye views
**
*/

#include "vectors.h" // RAD2DEG
#include "hw_cvars.h"
#include "hw_vrmodes.h"
#include "v_video.h"
#include "version.h"
#include "i_interface.h"
#include "RazeXR/mathlib.h"

extern vec3_t hmdPosition;
extern vec3_t hmdOrigin;

// Set up 3D-specific console variables:
CVAR(Int, vr_mode, 15, CVAR_GLOBALCONFIG|CVAR_ARCHIVE)

// switch left and right eye views
CVAR(Bool, vr_swap_eyes, false, CVAR_GLOBALCONFIG | CVAR_ARCHIVE)


// intraocular distance in meters
CVAR(Float, vr_ipd, 0.062f, CVAR_ARCHIVE|CVAR_GLOBALCONFIG) // METERS

// distance between viewer and the display screen
CVAR(Float, vr_screendist, 0.80f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG) // METERS

CVAR(Float, vr_hunits_per_meter, 24.0f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG) // METERS

CVAR(Float, vr_height_adjust, 0.0f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG) // METERS
CVAR(Int, vr_control_scheme, 0, CVAR_ARCHIVE | CVAR_GLOBALCONFIG | CVAR_NOINITCALL)
CVAR(Bool, vr_move_use_offhand, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, vr_teleport, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, vr_weaponRotate, -30, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, vr_weaponScale, 1.02f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, vr_snapTurn, 45.0f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, vr_move_speed, 19, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, vr_run_multiplier, 1.5, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, vr_switch_sticks, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, vr_secondary_button_mappings, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, vr_two_handed_weapons, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, vr_positional_tracking, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, vr_crouch_use_button, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)

CVAR(Float, vr_pickup_haptic_level, 0.2, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, vr_quake_haptic_level, 0.8, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)

int playerHeight = 0;

float getHmdAdjustedHeightInMapUnit()
{
	if (playerHeight != 0)
	{
		return ((hmdPosition[1] + vr_height_adjust) * vr_hunits_per_meter) -
			   (float) (playerHeight >> 8);
	}

	//Just use offset from origin
	return ((hmdPosition[1] - hmdOrigin[1]) * vr_hunits_per_meter);
}

#define isqrt2 0.7071067812f
static VRMode vrmi_mono = { 1, 1.f, 1.f, 1.f,{ { 0.f, 1.f },{ 0.f, 0.f } } };
static VRMode vrmi_stereo = { 2, 1.f, 1.f, 1.f,{ { -.5f, 1.f },{ .5f, 1.f } } };
static VRMode vrmi_sbsfull = { 2, .5f, 1.f, 2.f,{ { -.5f, .5f },{ .5f, .5f } } };
static VRMode vrmi_sbssquished = { 2, .5f, 1.f, 1.f,{ { -.5f, 1.f },{ .5f, 1.f } } };
static VRMode vrmi_lefteye = { 1, 1.f, 1.f, 1.f, { { -.5f, 1.f },{ 0.f, 0.f } } };
static VRMode vrmi_righteye = { 1, 1.f, 1.f, 1.f,{ { .5f, 1.f },{ 0.f, 0.f } } };
static VRMode vrmi_topbottom = { 2, 1.f, .5f, 1.f,{ { -.5f, 1.f },{ .5f, 1.f } } };
static VRMode vrmi_checker = { 2, isqrt2, isqrt2, 1.f,{ { -.5f, 1.f },{ .5f, 1.f } } };

const VRMode *VRMode::GetVRMode(bool toscreen)
{
	int mode = !toscreen || (sysCallbacks.DisableTextureFilter && sysCallbacks.DisableTextureFilter()) ? 0 : vr_mode;

	switch (mode)
	{
	default:
	case VR_MONO:
		return &vrmi_mono;

	case VR_GREENMAGENTA:
	case VR_REDCYAN:
	case VR_QUADSTEREO:
	case VR_AMBERBLUE:
	case VR_SIDEBYSIDELETTERBOX:
	case VR_OPENXR:
		return &vrmi_stereo;

	case VR_SIDEBYSIDESQUISHED:
	case VR_COLUMNINTERLEAVED:
		return &vrmi_sbssquished;

	case VR_SIDEBYSIDEFULL:
		return &vrmi_sbsfull;

	case VR_TOPBOTTOM:
	case VR_ROWINTERLEAVED:
		return &vrmi_topbottom;

	case VR_LEFTEYEVIEW:
		return &vrmi_lefteye;

	case VR_RIGHTEYEVIEW:
		return &vrmi_righteye;

	case VR_CHECKERINTERLEAVED:
		return &vrmi_checker;
	}
}

void VRMode::AdjustViewport(DFrameBuffer *screen) const
{
	screen->mSceneViewport.height = (int)(screen->mSceneViewport.height * mVerticalViewportScale);
	screen->mSceneViewport.top = (int)(screen->mSceneViewport.top * mVerticalViewportScale);
	screen->mSceneViewport.width = (int)(screen->mSceneViewport.width * mHorizontalViewportScale);
	screen->mSceneViewport.left = (int)(screen->mSceneViewport.left * mHorizontalViewportScale);

	screen->mScreenViewport.height = (int)(screen->mScreenViewport.height * mVerticalViewportScale);
	screen->mScreenViewport.top = (int)(screen->mScreenViewport.top * mVerticalViewportScale);
	screen->mScreenViewport.width = (int)(screen->mScreenViewport.width * mHorizontalViewportScale);
	screen->mScreenViewport.left = (int)(screen->mScreenViewport.left * mHorizontalViewportScale);
}

float getViewpointYaw()
{
	return 0.0f;
}

VSMatrix VRMode::GetHUDSpriteProjection() const
{
	VSMatrix mat;
	int w = screen->GetWidth();
	int h = screen->GetHeight();
	float scaled_w = w / mWeaponProjectionScale;
	float left_ofs = (w - scaled_w) / 2.f;
	mat.ortho(left_ofs, left_ofs + scaled_w, (float)h, 0, -1.0f, 1.0f);
	return mat;
}

float VREyeInfo::getShift() const
{
	return mShiftFactor * vr_ipd * vr_hunits_per_meter;
}

int VREyeInfo::getEye() const
{
	return mShiftFactor < 0 ? 0 : 1;
}

bool VR_GetVRProjection(int eye, float zNear, float zFar, float* projection);

VSMatrix VREyeInfo::GetProjection(float fov, float aspectRatio, float fovRatio) const
{
	VSMatrix result;

	if (mShiftFactor == 0)
	{
		float fovy = (float)(2 * RAD2DEG(atan(tan(DEG2RAD(fov) / 2) / fovRatio)));
		result.perspective(fovy, aspectRatio, screen->GetZNear(), screen->GetZFar());
		return result;
	}
	else
	{
		double zNear = screen->GetZNear();
		double zFar = screen->GetZFar();


		// For stereo 3D, use asymmetric frustum shift in projection matrix
		// Q: shouldn't shift vary with roll angle, at least for desktop display?
		// A: No. (lab) roll is not measured on desktop display (yet)
		double frustumShift = zNear * getShift() / vr_screendist; // meters cancel, leaving doom units
																  // double frustumShift = 0; // Turning off shift for debugging
		double fH = zNear * tan(DEG2RAD(fov) / 2) / fovRatio;
		double fW = fH * aspectRatio * mScaleFactor;
		double left = -fW - frustumShift;
		double right = fW - frustumShift;
		double bottom = -fH;
		double top = fH;

		VSMatrix result(1);
		result.frustum((float)left, (float)right, (float)bottom, (float)top, (float)zNear, (float)zFar);

		float m[16];
		VR_GetVRProjection(getEye(), zNear, zFar, m);
		result.loadMatrix(m);

		return result;
	}
}

void VR_GetMove(float *joy_forward, float *joy_side, float *hmd_forward, float *hmd_side, float *up,
				float *yaw, float *pitch, float *roll);

/* virtual */
DVector3 VREyeInfo::GetViewShift(FRotator viewAngles) const
{
	if (mShiftFactor == 0)
	{
		// pass-through for Mono view
		return { 0,0,0 };
	}
	else
	{
		vec3_t angles;
		VectorSet(angles, viewAngles.Pitch.Degrees,  viewAngles.Yaw.Degrees, viewAngles.Roll.Degrees);

		vec3_t v_forward, v_right, v_up;
		AngleVectors(angles, v_forward, v_right, v_up);

		vec3_t tmp;
		VectorScale(v_right, getShift(), tmp);

		float posforward=0;
		float posside=0;
		float dummy=0;
		VR_GetMove(&dummy, &dummy, &posforward, &posside, &dummy, &dummy, &dummy, &dummy);
		DVector3 eyeOffset(tmp[0], tmp[1], tmp[2]);

		if (vr_positional_tracking)
		{
			eyeOffset[1] += -posforward * vr_hunits_per_meter;
			eyeOffset[0] += posside * vr_hunits_per_meter;
			eyeOffset[2] += getHmdAdjustedHeightInMapUnit();
		}

		return {eyeOffset[1], eyeOffset[0], eyeOffset[2]};
	}
}

