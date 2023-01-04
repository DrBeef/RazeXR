//-------------------------------------------------------------------------
/*
Copyright (C) 1997, 2005 - 3D Realms Entertainment

This file is part of Shadow Warrior version 1.2

Shadow Warrior is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

Original Source: 1997 - Frank Maddin and Jim Norwood
Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------
#include "ns.h"

#include "build.h"

#include "names2.h"
#include "panel.h"
#include "game.h"
#include "tags.h"
#include "ai.h"
#include "pal.h"
#include "sprite.h"
#include "misc.h"

BEGIN_SW_NS

ANIMATOR InitRipperHang;
ANIMATOR DoRipperMoveJump;
ANIMATOR DoRipperHangJF;
ANIMATOR DoRipperQuickJump;

DECISION RipperBattle[] =
{
    {748, InitActorMoveCloser},
    {750, InitActorAlertNoise},
//    {900, InitRipperHang},
    {755, InitActorAttackNoise},
    {1024, InitActorAttack}
};

DECISION RipperOffense[] =
{
    {700, InitActorMoveCloser},
    {710, InitActorAlertNoise},
    {1024, InitActorAttack}
};

DECISION RipperBroadcast[] =
{
    {3, InitActorAlertNoise},
    {6, InitActorAmbientNoise},
    {1024, InitActorDecide}
};

DECISION RipperSurprised[] =
{
    {30, InitRipperHang},
    {701, InitActorMoveCloser},
    {1024, InitActorDecide}
};

DECISION RipperEvasive[] =
{
    {6, InitRipperHang},
    {1024, nullptr}
};

DECISION RipperLostTarget[] =
{
    {980, InitActorFindPlayer},
    {1024, InitActorWanderAround}
};

DECISION RipperCloseRange[] =
{
    {900,   InitActorAttack             },
    {1024,  InitActorReposition         }
};

PERSONALITY RipperPersonality =
{
    RipperBattle,
    RipperOffense,
    RipperBroadcast,
    RipperSurprised,
    RipperEvasive,
    RipperLostTarget,
    RipperCloseRange,
    RipperCloseRange
};

ATTRIBUTE RipperAttrib =
{
    {200, 220, 240, 280},               // Speeds
    {5, 0, -2, -4},                     // Tic Adjusts
    3,                                  // MaxWeapons;
    {
        DIGI_RIPPERAMBIENT, DIGI_RIPPERALERT, DIGI_RIPPERATTACK,
        DIGI_RIPPERPAIN, DIGI_RIPPERSCREAM, DIGI_RIPPERHEARTOUT,
        0,0,0,0
    }
};

//////////////////////
//
// RIPPER RUN
//
//////////////////////

#define RIPPER_RUN_RATE 16

ANIMATOR DoRipperMove, NullRipper, DoActorDebris;

STATE s_RipperRun[5][4] =
{
    {
        {RIPPER_RUN_R0 + 0, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[0][1]},
        {RIPPER_RUN_R0 + 1, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[0][2]},
        {RIPPER_RUN_R0 + 2, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[0][3]},
        {RIPPER_RUN_R0 + 3, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[0][0]},
    },
    {
        {RIPPER_RUN_R1 + 0, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[1][1]},
        {RIPPER_RUN_R1 + 1, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[1][2]},
        {RIPPER_RUN_R1 + 2, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[1][3]},
        {RIPPER_RUN_R1 + 3, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[1][0]},
    },
    {
        {RIPPER_RUN_R2 + 0, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[2][1]},
        {RIPPER_RUN_R2 + 1, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[2][2]},
        {RIPPER_RUN_R2 + 2, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[2][3]},
        {RIPPER_RUN_R2 + 3, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[2][0]},
    },
    {
        {RIPPER_RUN_R3 + 0, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[3][1]},
        {RIPPER_RUN_R3 + 1, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[3][2]},
        {RIPPER_RUN_R3 + 2, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[3][3]},
        {RIPPER_RUN_R3 + 3, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[3][0]},
    },
    {
        {RIPPER_RUN_R4 + 0, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[4][1]},
        {RIPPER_RUN_R4 + 1, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[4][2]},
        {RIPPER_RUN_R4 + 2, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[4][3]},
        {RIPPER_RUN_R4 + 3, RIPPER_RUN_RATE | SF_TIC_ADJUST, DoRipperMove, &s_RipperRun[4][0]},
    }
};


STATE* sg_RipperRun[] =
{
    &s_RipperRun[0][0],
    &s_RipperRun[1][0],
    &s_RipperRun[2][0],
    &s_RipperRun[3][0],
    &s_RipperRun[4][0]
};

//////////////////////
//
// RIPPER STAND
//
//////////////////////

#define RIPPER_STAND_RATE 12

STATE s_RipperStand[5][1] =
{
    {
        {RIPPER_STAND_R0 + 0, RIPPER_STAND_RATE, DoRipperMove, &s_RipperStand[0][0]},
    },
    {
        {RIPPER_STAND_R1 + 0, RIPPER_STAND_RATE, DoRipperMove, &s_RipperStand[1][0]},
    },
    {
        {RIPPER_STAND_R2 + 0, RIPPER_STAND_RATE, DoRipperMove, &s_RipperStand[2][0]},
    },
    {
        {RIPPER_STAND_R3 + 0, RIPPER_STAND_RATE, DoRipperMove, &s_RipperStand[3][0]},
    },
    {
        {RIPPER_STAND_R4 + 0, RIPPER_STAND_RATE, DoRipperMove, &s_RipperStand[4][0]},
    },
};


STATE* sg_RipperStand[] =
{
    s_RipperStand[0],
    s_RipperStand[1],
    s_RipperStand[2],
    s_RipperStand[3],
    s_RipperStand[4]
};

//////////////////////
//
// RIPPER SWIPE
//
//////////////////////

#define RIPPER_SWIPE_RATE 8
ANIMATOR InitActorDecide;
ANIMATOR InitRipperSlash;

STATE s_RipperSwipe[5][8] =
{
    {
        {RIPPER_SWIPE_R0 + 0, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[0][1]},
        {RIPPER_SWIPE_R0 + 1, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[0][2]},
        {RIPPER_SWIPE_R0 + 1, 0 | SF_QUICK_CALL, InitRipperSlash, &s_RipperSwipe[0][3]},
        {RIPPER_SWIPE_R0 + 2, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[0][4]},
        {RIPPER_SWIPE_R0 + 3, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[0][5]},
        {RIPPER_SWIPE_R0 + 3, 0 | SF_QUICK_CALL, InitRipperSlash, &s_RipperSwipe[0][6]},
        {RIPPER_SWIPE_R0 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_RipperSwipe[0][7]},
        {RIPPER_SWIPE_R0 + 3, RIPPER_SWIPE_RATE, DoRipperMove, &s_RipperSwipe[0][7]},
    },
    {
        {RIPPER_SWIPE_R1 + 0, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[1][1]},
        {RIPPER_SWIPE_R1 + 1, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[1][2]},
        {RIPPER_SWIPE_R1 + 1, 0 | SF_QUICK_CALL, InitRipperSlash, &s_RipperSwipe[1][3]},
        {RIPPER_SWIPE_R1 + 2, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[1][4]},
        {RIPPER_SWIPE_R1 + 3, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[1][5]},
        {RIPPER_SWIPE_R1 + 3, 0 | SF_QUICK_CALL, InitRipperSlash, &s_RipperSwipe[1][6]},
        {RIPPER_SWIPE_R1 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_RipperSwipe[1][7]},
        {RIPPER_SWIPE_R1 + 3, RIPPER_SWIPE_RATE, DoRipperMove, &s_RipperSwipe[1][7]},
    },
    {
        {RIPPER_SWIPE_R2 + 0, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[2][1]},
        {RIPPER_SWIPE_R2 + 1, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[2][2]},
        {RIPPER_SWIPE_R2 + 1, 0 | SF_QUICK_CALL, InitRipperSlash, &s_RipperSwipe[2][3]},
        {RIPPER_SWIPE_R2 + 2, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[2][4]},
        {RIPPER_SWIPE_R2 + 3, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[2][5]},
        {RIPPER_SWIPE_R2 + 3, 0 | SF_QUICK_CALL, InitRipperSlash, &s_RipperSwipe[2][6]},
        {RIPPER_SWIPE_R2 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_RipperSwipe[2][7]},
        {RIPPER_SWIPE_R2 + 3, RIPPER_SWIPE_RATE, DoRipperMove, &s_RipperSwipe[2][7]},
    },
    {
        {RIPPER_SWIPE_R3 + 0, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[3][1]},
        {RIPPER_SWIPE_R3 + 1, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[3][2]},
        {RIPPER_SWIPE_R3 + 1, 0 | SF_QUICK_CALL, InitRipperSlash, &s_RipperSwipe[3][3]},
        {RIPPER_SWIPE_R3 + 2, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[3][4]},
        {RIPPER_SWIPE_R3 + 3, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[3][5]},
        {RIPPER_SWIPE_R3 + 3, 0 | SF_QUICK_CALL, InitRipperSlash, &s_RipperSwipe[3][6]},
        {RIPPER_SWIPE_R3 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_RipperSwipe[3][7]},
        {RIPPER_SWIPE_R3 + 3, RIPPER_SWIPE_RATE, DoRipperMove, &s_RipperSwipe[3][7]},
    },
    {
        {RIPPER_SWIPE_R4 + 0, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[4][1]},
        {RIPPER_SWIPE_R4 + 1, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[4][2]},
        {RIPPER_SWIPE_R4 + 1, 0 | SF_QUICK_CALL, InitRipperSlash, &s_RipperSwipe[4][3]},
        {RIPPER_SWIPE_R4 + 2, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[4][4]},
        {RIPPER_SWIPE_R4 + 3, RIPPER_SWIPE_RATE, NullRipper, &s_RipperSwipe[4][5]},
        {RIPPER_SWIPE_R4 + 3, 0 | SF_QUICK_CALL, InitRipperSlash, &s_RipperSwipe[4][6]},
        {RIPPER_SWIPE_R4 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_RipperSwipe[4][7]},
        {RIPPER_SWIPE_R4 + 3, RIPPER_SWIPE_RATE, DoRipperMove, &s_RipperSwipe[4][7]},
    }
};


STATE* sg_RipperSwipe[] =
{
    &s_RipperSwipe[0][0],
    &s_RipperSwipe[1][0],
    &s_RipperSwipe[2][0],
    &s_RipperSwipe[3][0],
    &s_RipperSwipe[4][0]
};


//////////////////////
//
// RIPPER SPEW
//
//////////////////////

#define RIPPER_SPEW_RATE 8
ANIMATOR InitActorDecide;
ANIMATOR InitCoolgFire;

STATE s_RipperSpew[5][7] =
{
    {
        {RIPPER_SWIPE_R0 + 0, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[0][1]},
        {RIPPER_SWIPE_R0 + 1, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[0][2]},
        {RIPPER_SWIPE_R0 + 1, 0 | SF_QUICK_CALL, InitCoolgFire, &s_RipperSpew[0][3]},
        {RIPPER_SWIPE_R0 + 2, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[0][4]},
        {RIPPER_SWIPE_R0 + 3, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[0][5]},
        {RIPPER_SWIPE_R0 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_RipperSpew[0][6]},
        {RIPPER_SWIPE_R0 + 3, RIPPER_SPEW_RATE, DoRipperMove, &s_RipperSpew[0][6]},
    },
    {
        {RIPPER_SWIPE_R1 + 0, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[1][1]},
        {RIPPER_SWIPE_R1 + 1, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[1][2]},
        {RIPPER_SWIPE_R1 + 1, 0 | SF_QUICK_CALL, InitCoolgFire, &s_RipperSpew[1][3]},
        {RIPPER_SWIPE_R1 + 2, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[1][4]},
        {RIPPER_SWIPE_R1 + 3, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[1][5]},
        {RIPPER_SWIPE_R1 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_RipperSpew[1][6]},
        {RIPPER_SWIPE_R1 + 3, RIPPER_SPEW_RATE, DoRipperMove, &s_RipperSpew[1][6]},
    },
    {
        {RIPPER_SWIPE_R2 + 0, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[2][1]},
        {RIPPER_SWIPE_R2 + 1, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[2][2]},
        {RIPPER_SWIPE_R2 + 1, 0 | SF_QUICK_CALL, InitCoolgFire, &s_RipperSpew[2][3]},
        {RIPPER_SWIPE_R2 + 2, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[2][4]},
        {RIPPER_SWIPE_R2 + 3, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[2][5]},
        {RIPPER_SWIPE_R2 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_RipperSpew[2][6]},
        {RIPPER_SWIPE_R2 + 3, RIPPER_SPEW_RATE, DoRipperMove, &s_RipperSpew[2][6]},
    },
    {
        {RIPPER_SWIPE_R3 + 0, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[3][1]},
        {RIPPER_SWIPE_R3 + 1, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[3][2]},
        {RIPPER_SWIPE_R3 + 1, 0 | SF_QUICK_CALL, InitCoolgFire, &s_RipperSpew[3][3]},
        {RIPPER_SWIPE_R3 + 2, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[3][4]},
        {RIPPER_SWIPE_R3 + 3, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[3][5]},
        {RIPPER_SWIPE_R3 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_RipperSpew[3][6]},
        {RIPPER_SWIPE_R3 + 3, RIPPER_SPEW_RATE, DoRipperMove, &s_RipperSpew[3][6]},
    },
    {
        {RIPPER_SWIPE_R4 + 0, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[4][1]},
        {RIPPER_SWIPE_R4 + 1, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[4][2]},
        {RIPPER_SWIPE_R4 + 1, 0 | SF_QUICK_CALL, InitCoolgFire, &s_RipperSpew[4][3]},
        {RIPPER_SWIPE_R4 + 2, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[4][4]},
        {RIPPER_SWIPE_R4 + 3, RIPPER_SPEW_RATE, NullRipper, &s_RipperSpew[4][5]},
        {RIPPER_SWIPE_R4 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_RipperSpew[4][6]},
        {RIPPER_SWIPE_R4 + 3, RIPPER_SPEW_RATE, DoRipperMove, &s_RipperSpew[4][6]},
    }
};


STATE* sg_RipperSpew[] =
{
    &s_RipperSpew[0][0],
    &s_RipperSpew[1][0],
    &s_RipperSpew[2][0],
    &s_RipperSpew[3][0],
    &s_RipperSpew[4][0]
};


//////////////////////
//
// RIPPER HEART - show players heart
//
//////////////////////

#define RIPPER_HEART_RATE 14
ANIMATOR DoRipperStandHeart;

STATE s_RipperHeart[5][4] =
{
    {
        {RIPPER_HEART_R0 + 0, RIPPER_HEART_RATE, DoRipperStandHeart, &s_RipperHeart[0][0]},
    },
    {
        {RIPPER_HEART_R1 + 0, RIPPER_HEART_RATE, DoRipperStandHeart, &s_RipperHeart[1][0]},
    },
    {
        {RIPPER_HEART_R2 + 0, RIPPER_HEART_RATE, DoRipperStandHeart, &s_RipperHeart[2][0]},
    },
    {
        {RIPPER_HEART_R3 + 0, RIPPER_HEART_RATE, DoRipperStandHeart, &s_RipperHeart[3][0]},
    },
    {
        {RIPPER_HEART_R4 + 0, RIPPER_HEART_RATE, DoRipperStandHeart, &s_RipperHeart[4][0]},
    }
};

STATE* sg_RipperHeart[] =
{
    &s_RipperHeart[0][0],
    &s_RipperHeart[1][0],
    &s_RipperHeart[2][0],
    &s_RipperHeart[3][0],
    &s_RipperHeart[4][0]
};

//////////////////////
//
// RIPPER HANG
//
//////////////////////

#define RIPPER_HANG_RATE 14
ANIMATOR DoRipperHang;

STATE s_RipperHang[5][4] =
{
    {
        {RIPPER_HANG_R0 + 0, RIPPER_HANG_RATE, DoRipperHang, &s_RipperHang[0][0]},
    },
    {
        {RIPPER_HANG_R1 + 0, RIPPER_HANG_RATE, DoRipperHang, &s_RipperHang[1][0]},
    },
    {
        {RIPPER_HANG_R2 + 0, RIPPER_HANG_RATE, DoRipperHang, &s_RipperHang[2][0]},
    },
    {
        {RIPPER_HANG_R3 + 0, RIPPER_HANG_RATE, DoRipperHang, &s_RipperHang[3][0]},
    },
    {
        {RIPPER_HANG_R4 + 0, RIPPER_HANG_RATE, DoRipperHang, &s_RipperHang[4][0]},
    }
};


STATE* sg_RipperHang[] =
{
    &s_RipperHang[0][0],
    &s_RipperHang[1][0],
    &s_RipperHang[2][0],
    &s_RipperHang[3][0],
    &s_RipperHang[4][0]
};


//////////////////////
//
// RIPPER PAIN
//
//////////////////////

#define RIPPER_PAIN_RATE 38
ANIMATOR DoRipperPain;

STATE s_RipperPain[5][1] =
{
    {
        {RIPPER_JUMP_R0 + 0, RIPPER_PAIN_RATE, DoRipperPain, &s_RipperPain[0][0]},
    },
    {
        {RIPPER_JUMP_R0 + 0, RIPPER_PAIN_RATE, DoRipperPain, &s_RipperPain[1][0]},
    },
    {
        {RIPPER_JUMP_R0 + 0, RIPPER_PAIN_RATE, DoRipperPain, &s_RipperPain[2][0]},
    },
    {
        {RIPPER_JUMP_R0 + 0, RIPPER_PAIN_RATE, DoRipperPain, &s_RipperPain[3][0]},
    },
    {
        {RIPPER_JUMP_R0 + 0, RIPPER_PAIN_RATE, DoRipperPain, &s_RipperPain[4][0]},
    }
};

STATE* sg_RipperPain[] =
{
    &s_RipperPain[0][0],
    &s_RipperPain[1][0],
    &s_RipperPain[2][0],
    &s_RipperPain[3][0],
    &s_RipperPain[4][0]
};

//////////////////////
//
// RIPPER JUMP
//
//////////////////////

#define RIPPER_JUMP_RATE 25

STATE s_RipperJump[5][6] =
{
    {
        {RIPPER_JUMP_R0 + 0, RIPPER_JUMP_RATE, NullRipper, &s_RipperJump[0][1]},
        {RIPPER_JUMP_R0 + 1, RIPPER_JUMP_RATE, DoRipperMoveJump, &s_RipperJump[0][1]},
    },
    {
        {RIPPER_JUMP_R1 + 0, RIPPER_JUMP_RATE, NullRipper, &s_RipperJump[1][1]},
        {RIPPER_JUMP_R1 + 1, RIPPER_JUMP_RATE, DoRipperMoveJump, &s_RipperJump[1][1]},
    },
    {
        {RIPPER_JUMP_R2 + 0, RIPPER_JUMP_RATE, NullRipper, &s_RipperJump[2][1]},
        {RIPPER_JUMP_R2 + 1, RIPPER_JUMP_RATE, DoRipperMoveJump, &s_RipperJump[2][1]},
    },
    {
        {RIPPER_JUMP_R3 + 0, RIPPER_JUMP_RATE, NullRipper, &s_RipperJump[3][1]},
        {RIPPER_JUMP_R3 + 1, RIPPER_JUMP_RATE, DoRipperMoveJump, &s_RipperJump[3][1]},
    },
    {
        {RIPPER_JUMP_R4 + 0, RIPPER_JUMP_RATE, NullRipper, &s_RipperJump[4][1]},
        {RIPPER_JUMP_R4 + 1, RIPPER_JUMP_RATE, DoRipperMoveJump, &s_RipperJump[4][1]},
    }
};


STATE* sg_RipperJump[] =
{
    &s_RipperJump[0][0],
    &s_RipperJump[1][0],
    &s_RipperJump[2][0],
    &s_RipperJump[3][0],
    &s_RipperJump[4][0]
};


//////////////////////
//
// RIPPER FALL
//
//////////////////////

#define RIPPER_FALL_RATE 25

STATE s_RipperFall[5][6] =
{
    {
        {RIPPER_FALL_R0 + 0, RIPPER_FALL_RATE, DoRipperMoveJump, &s_RipperFall[0][0]},
    },
    {
        {RIPPER_FALL_R1 + 0, RIPPER_FALL_RATE, DoRipperMoveJump, &s_RipperFall[1][0]},
    },
    {
        {RIPPER_FALL_R2 + 0, RIPPER_FALL_RATE, DoRipperMoveJump, &s_RipperFall[2][0]},
    },
    {
        {RIPPER_FALL_R3 + 0, RIPPER_FALL_RATE, DoRipperMoveJump, &s_RipperFall[3][0]},
    },
    {
        {RIPPER_FALL_R4 + 0, RIPPER_FALL_RATE, DoRipperMoveJump, &s_RipperFall[4][0]},
    }
};


STATE* sg_RipperFall[] =
{
    &s_RipperFall[0][0],
    &s_RipperFall[1][0],
    &s_RipperFall[2][0],
    &s_RipperFall[3][0],
    &s_RipperFall[4][0]
};


//////////////////////
//
// RIPPER JUMP ATTACK
//
//////////////////////

#define RIPPER_JUMP_ATTACK_RATE 35
int DoRipperBeginJumpAttack(DSWActor* actor);

STATE s_RipperJumpAttack[5][6] =
{
    {
        {RIPPER_JUMP_R0 + 0, RIPPER_JUMP_ATTACK_RATE, NullRipper, &s_RipperJumpAttack[0][1]},
        {RIPPER_JUMP_R0 + 0, 0 | SF_QUICK_CALL, DoRipperBeginJumpAttack, &s_RipperJumpAttack[0][2]},
        {RIPPER_JUMP_R0 + 1, RIPPER_JUMP_ATTACK_RATE, DoRipperMoveJump, &s_RipperJumpAttack[0][2]},
    },
    {
        {RIPPER_JUMP_R1 + 0, RIPPER_JUMP_ATTACK_RATE, NullRipper, &s_RipperJumpAttack[1][1]},
        {RIPPER_JUMP_R1 + 0, 0 | SF_QUICK_CALL, DoRipperBeginJumpAttack, &s_RipperJumpAttack[1][2]},
        {RIPPER_JUMP_R1 + 1, RIPPER_JUMP_ATTACK_RATE, DoRipperMoveJump, &s_RipperJumpAttack[1][2]},
    },
    {
        {RIPPER_JUMP_R2 + 0, RIPPER_JUMP_ATTACK_RATE, NullRipper, &s_RipperJumpAttack[2][1]},
        {RIPPER_JUMP_R2 + 0, 0 | SF_QUICK_CALL, DoRipperBeginJumpAttack, &s_RipperJumpAttack[2][2]},
        {RIPPER_JUMP_R2 + 1, RIPPER_JUMP_ATTACK_RATE, DoRipperMoveJump, &s_RipperJumpAttack[2][2]},
    },
    {
        {RIPPER_JUMP_R3 + 0, RIPPER_JUMP_ATTACK_RATE, NullRipper, &s_RipperJumpAttack[3][1]},
        {RIPPER_JUMP_R3 + 0, 0 | SF_QUICK_CALL, DoRipperBeginJumpAttack, &s_RipperJumpAttack[3][2]},
        {RIPPER_JUMP_R3 + 1, RIPPER_JUMP_ATTACK_RATE, DoRipperMoveJump, &s_RipperJumpAttack[3][2]},
    },
    {
        {RIPPER_JUMP_R4 + 0, RIPPER_JUMP_ATTACK_RATE, NullRipper, &s_RipperJumpAttack[4][1]},
        {RIPPER_JUMP_R4 + 0, 0 | SF_QUICK_CALL, DoRipperBeginJumpAttack, &s_RipperJumpAttack[4][2]},
        {RIPPER_JUMP_R4 + 1, RIPPER_JUMP_ATTACK_RATE, DoRipperMoveJump, &s_RipperJumpAttack[4][2]},
    }
};


STATE* sg_RipperJumpAttack[] =
{
    &s_RipperJumpAttack[0][0],
    &s_RipperJumpAttack[1][0],
    &s_RipperJumpAttack[2][0],
    &s_RipperJumpAttack[3][0],
    &s_RipperJumpAttack[4][0]
};


//////////////////////
//
// RIPPER HANG_JUMP
//
//////////////////////

#define RIPPER_HANG_JUMP_RATE 20

STATE s_RipperHangJump[5][6] =
{
    {
        {RIPPER_JUMP_R0 + 0, RIPPER_HANG_JUMP_RATE, NullRipper, &s_RipperHangJump[0][1]},
        {RIPPER_JUMP_R0 + 1, RIPPER_HANG_JUMP_RATE, DoRipperHangJF, &s_RipperHangJump[0][1]},
    },
    {
        {RIPPER_JUMP_R1 + 0, RIPPER_HANG_JUMP_RATE, NullRipper, &s_RipperHangJump[1][1]},
        {RIPPER_JUMP_R1 + 1, RIPPER_HANG_JUMP_RATE, DoRipperHangJF, &s_RipperHangJump[1][1]},
    },
    {
        {RIPPER_JUMP_R2 + 0, RIPPER_HANG_JUMP_RATE, NullRipper, &s_RipperHangJump[2][1]},
        {RIPPER_JUMP_R2 + 1, RIPPER_HANG_JUMP_RATE, DoRipperHangJF, &s_RipperHangJump[2][1]},
    },
    {
        {RIPPER_JUMP_R3 + 0, RIPPER_HANG_JUMP_RATE, NullRipper, &s_RipperHangJump[3][1]},
        {RIPPER_JUMP_R3 + 1, RIPPER_HANG_JUMP_RATE, DoRipperHangJF, &s_RipperHangJump[3][1]},
    },
    {
        {RIPPER_JUMP_R4 + 0, RIPPER_HANG_JUMP_RATE, NullRipper, &s_RipperHangJump[4][1]},
        {RIPPER_JUMP_R4 + 1, RIPPER_HANG_JUMP_RATE, DoRipperHangJF, &s_RipperHangJump[4][1]},
    }
};


STATE* sg_RipperHangJump[] =
{
    &s_RipperHangJump[0][0],
    &s_RipperHangJump[1][0],
    &s_RipperHangJump[2][0],
    &s_RipperHangJump[3][0],
    &s_RipperHangJump[4][0]
};

//////////////////////
//
// RIPPER HANG_FALL
//
//////////////////////

#define RIPPER_FALL_RATE 25

STATE s_RipperHangFall[5][6] =
{
    {
        {RIPPER_FALL_R0 + 0, RIPPER_FALL_RATE, DoRipperHangJF, &s_RipperHangFall[0][0]},
    },
    {
        {RIPPER_FALL_R1 + 0, RIPPER_FALL_RATE, DoRipperHangJF, &s_RipperHangFall[1][0]},
    },
    {
        {RIPPER_FALL_R2 + 0, RIPPER_FALL_RATE, DoRipperHangJF, &s_RipperHangFall[2][0]},
    },
    {
        {RIPPER_FALL_R3 + 0, RIPPER_FALL_RATE, DoRipperHangJF, &s_RipperHangFall[3][0]},
    },
    {
        {RIPPER_FALL_R4 + 0, RIPPER_FALL_RATE, DoRipperHangJF, &s_RipperHangFall[4][0]},
    }
};


STATE* sg_RipperHangFall[] =
{
    &s_RipperHangFall[0][0],
    &s_RipperHangFall[1][0],
    &s_RipperHangFall[2][0],
    &s_RipperHangFall[3][0],
    &s_RipperHangFall[4][0]
};



//////////////////////
//
// RIPPER DIE
//
//////////////////////

#define RIPPER_DIE_RATE 16

STATE s_RipperDie[] =
{
    {RIPPER_DIE + 0, RIPPER_DIE_RATE, NullRipper, &s_RipperDie[1]},
    {RIPPER_DIE + 1, RIPPER_DIE_RATE, NullRipper, &s_RipperDie[2]},
    {RIPPER_DIE + 2, RIPPER_DIE_RATE, NullRipper, &s_RipperDie[3]},
    {RIPPER_DIE + 3, RIPPER_DIE_RATE, NullRipper, &s_RipperDie[4]},
    {RIPPER_DEAD, RIPPER_DIE_RATE, DoActorDebris, &s_RipperDie[4]},
};

#define RIPPER_DEAD_RATE 8

STATE s_RipperDead[] =
{
    {RIPPER_DIE + 2, RIPPER_DEAD_RATE, NullAnimator, &s_RipperDead[1]},
    {RIPPER_DIE + 3, RIPPER_DEAD_RATE, NullAnimator, &s_RipperDead[2]},
    {RIPPER_DEAD, SF_QUICK_CALL, QueueFloorBlood, &s_RipperDead[3]},
    {RIPPER_DEAD, RIPPER_DEAD_RATE, DoActorDebris, &s_RipperDead[3]},
};

STATE* sg_RipperDie[] =
{
    s_RipperDie
};

STATE* sg_RipperDead[] =
{
    s_RipperDead
};

STATE s_RipperDeathJump[] =
{
    {RIPPER_DIE + 0, RIPPER_DIE_RATE, DoActorDeathMove, &s_RipperDeathJump[0]}
};

STATE s_RipperDeathFall[] =
{
    {RIPPER_DIE + 1, RIPPER_DIE_RATE, DoActorDeathMove, &s_RipperDeathFall[0]}
};


STATE* sg_RipperDeathJump[] =
{
    s_RipperDeathJump
};

STATE* sg_RipperDeathFall[] =
{
    s_RipperDeathFall
};


/*
STATE* *Stand[MAX_WEAPONS];
STATE* *Run;
STATE* *Jump;
STATE* *Fall;
STATE* *Crawl;
STATE* *Swim;
STATE* *Fly;
STATE* *Rise;
STATE* *Sit;
STATE* *Look;
STATE* *Climb;
STATE* *Pain;
STATE* *Death1;
STATE* *Death2;
STATE* *Dead;
STATE* *DeathJump;
STATE* *DeathFall;
STATE* *CloseAttack[2];
STATE* *Attack[6];
STATE* *Special[2];
*/


ACTOR_ACTION_SET RipperActionSet =
{
    sg_RipperStand,
    sg_RipperRun,
    sg_RipperJump,
    sg_RipperFall,
    nullptr,                               // sg_RipperCrawl,
    nullptr,                               // sg_RipperSwim,
    nullptr,                               // sg_RipperFly,
    nullptr,                               // sg_RipperRise,
    nullptr,                               // sg_RipperSit,
    nullptr,                               // sg_RipperLook,
    nullptr,                               // climb
    sg_RipperPain,
    sg_RipperDie,
    nullptr,                               // sg_RipperHariKari,
    sg_RipperDead,
    sg_RipperDeathJump,
    sg_RipperDeathFall,
    {sg_RipperSwipe,sg_RipperSpew},
    {800,1024},
    {sg_RipperJumpAttack, sg_RipperSpew},
    {400, 1024},
    {sg_RipperHeart, sg_RipperHang},
    nullptr,
    nullptr
};

ACTOR_ACTION_SET RipperBrownActionSet =
{
    sg_RipperStand,
    sg_RipperRun,
    sg_RipperJump,
    sg_RipperFall,
    nullptr,                               // sg_RipperCrawl,
    nullptr,                               // sg_RipperSwim,
    nullptr,                               // sg_RipperFly,
    nullptr,                               // sg_RipperRise,
    nullptr,                               // sg_RipperSit,
    nullptr,                               // sg_RipperLook,
    nullptr,                               // climb
    sg_RipperPain,                      // pain
    sg_RipperDie,
    nullptr,                               // sg_RipperHariKari,
    sg_RipperDead,
    sg_RipperDeathJump,
    sg_RipperDeathFall,
    {sg_RipperSwipe},
    {1024},
    {sg_RipperJumpAttack, sg_RipperSwipe},
    {800, 1024},
    {sg_RipperHeart, sg_RipperHang},
    nullptr,
    nullptr
};

int SetupRipper(DSWActor* actor)
{
    ANIMATOR DoActorDecide;

    if (!(actor->spr.cstat & CSTAT_SPRITE_RESTORE))
    {
        SpawnUser(actor, RIPPER_RUN_R0, s_RipperRun[0]);
        actor->user.Health = HEALTH_RIPPER/2; // Baby rippers are weaker
    }

    ChangeState(actor, s_RipperRun[0]);
    actor->user.Attrib = &RipperAttrib;
    DoActorSetSpeed(actor, FAST_SPEED);
    actor->user.StateEnd = s_RipperDie;
    actor->user.Rot = sg_RipperRun;
    actor->spr.xrepeat = 64;
    actor->spr.yrepeat = 64;

    if (actor->spr.pal == PALETTE_BROWN_RIPPER)
    {
        EnemyDefaults(actor, &RipperBrownActionSet, &RipperPersonality);
        actor->spr.xrepeat = 106;
        actor->spr.yrepeat = 90;

        if (!(actor->spr.cstat & CSTAT_SPRITE_RESTORE))
            actor->user.Health = HEALTH_MOMMA_RIPPER;

        actor->spr.clipdist += 128 >> 2;
    }
    else
    {
        EnemyDefaults(actor, &RipperActionSet, &RipperPersonality);
    }

    actor->user.Flags |= (SPR_XFLIP_TOGGLE);

    return 0;
}

int GetJumpHeight(int jump_speed, int jump_grav)
{
    int jump_iterations;
    int height;

    jump_speed = abs(jump_speed);

    jump_iterations = jump_speed / (jump_grav * ACTORMOVETICS);

    height = jump_speed * jump_iterations * ACTORMOVETICS;

    return height >> 9;
}

int PickJumpSpeed(DSWActor* actor, int pix_height)
{
    actor->user.jump_speed = -600;
    actor->user.jump_grav = 8;

    while (true)
    {
        if (GetJumpHeight(actor->user.jump_speed, actor->user.jump_grav) > pix_height + 20)
            break;

        actor->user.jump_speed -= 100;

        ASSERT(actor->user.jump_speed > -3000);
    }

    return actor->user.jump_speed;
}


int PickJumpMaxSpeed(DSWActor* actor, short max_speed)
{
    int zh;

    ASSERT(max_speed < 0);

    actor->user.jump_speed = max_speed;
    actor->user.jump_grav = 8;

    zh = ActorZOfTop(actor);

    while (true)
    {
        if (zh - Z(GetJumpHeight(actor->user.jump_speed, actor->user.jump_grav)) - Z(16) > actor->user.hiz)
            break;

        actor->user.jump_speed += 100;

        if (actor->user.jump_speed > -200)
            break;
    }

    return actor->user.jump_speed;
}


//
// HANGING - Jumping/Falling/Stationary
//

int InitRipperHang(DSWActor* actor)
{
    int dist;

    HitInfo hit{};

    bool Found = false;
    short dang, tang;

    for (dang = 0; dang < 2048; dang += 128)
    {
        tang = NORM_ANGLE(actor->spr.ang + dang);

        FAFhitscan(actor->spr.pos.X, actor->spr.pos.Y, actor->spr.pos.Z - ActorSizeZ(actor), actor->sector(),  // Start position
                   bcos(tang),   // X vector of 3D ang
                   bsin(tang),   // Y vector of 3D ang
                   0,            // Z vector of 3D ang
                   hit, CLIPMASK_MISSILE);

        if (hit.hitSector == nullptr)
            continue;

        dist = Distance(actor->spr.pos.X, actor->spr.pos.Y, hit.hitpos.X, hit.hitpos.Y);

        if (hit.hitWall == nullptr || dist < 2000 || dist > 7000)
        {
            continue;
        }

        Found = true;
        actor->spr.ang = tang;
        break;
    }

    if (!Found)
    {
        InitActorDecide(actor);
        return 0;
    }

    NewStateGroup(actor, sg_RipperHangJump);
    actor->user.StateFallOverride = sg_RipperHangFall;
    DoActorSetSpeed(actor, FAST_SPEED);

    //actor->user.jump_speed = -800;
    PickJumpMaxSpeed(actor, -800);

    actor->user.Flags |= (SPR_JUMPING);
    actor->user.Flags &= ~(SPR_FALLING);

    // set up individual actor jump gravity
    actor->user.jump_grav = 8;

    DoJump(actor);

    return 0;
}

int DoRipperHang(DSWActor* actor)
{
    if ((actor->user.WaitTics -= ACTORMOVETICS) > 0)
        return 0;

    NewStateGroup(actor, sg_RipperJumpAttack);
    // move to the 2nd frame - past the pause frame
    actor->user.Tics += actor->user.State->Tics;
    return 0;
}

int DoRipperMoveHang(DSWActor* actor)
{
    int nx, ny;

    // Move while jumping
    nx = MulScale(actor->spr.xvel, bcos(actor->spr.ang), 14);
    ny = MulScale(actor->spr.xvel, bsin(actor->spr.ang), 14);

    // if cannot move the sprite
    if (!move_actor(actor, nx, ny, 0L))
    {
        if (actor->user.coll.type == kHitWall)
        {
            NewStateGroup(actor, actor->user.ActorActionSet->Special[1]);
            actor->user.WaitTics = 2 + ((RANDOM_P2(4 << 8) >> 8) * 120);

            // hang flush with the wall
            actor->spr.ang = NORM_ANGLE(getangle(actor->user.coll.hitWall->delta()) - 512);

            return 0;
        }
    }

    return 0;
}


int DoRipperHangJF(DSWActor* actor)
{
    if (actor->user.Flags & (SPR_JUMPING | SPR_FALLING))
    {
        if (actor->user.Flags & (SPR_JUMPING))
            DoJump(actor);
        else
            DoFall(actor);
    }

    if (!(actor->user.Flags & (SPR_JUMPING | SPR_FALLING)))
    {
        if (DoRipperQuickJump(actor))
            return 0;

        InitActorDecide(actor);
    }

    DoRipperMoveHang(actor);

    return 0;

}

//
// JUMP ATTACK
//

int DoRipperBeginJumpAttack(DSWActor* actor)
{
    DSWActor* target = actor->user.targetActor;
    short tang;

    tang = getangle(target->spr.pos.X - actor->spr.pos.X, target->spr.pos.Y - actor->spr.pos.Y);

	Collision coll = move_sprite(actor, bcos(tang, -7), bsin(tang, -7),
							   0L, actor->user.ceiling_dist, actor->user.floor_dist, CLIPMASK_ACTOR, ACTORMOVETICS);

    if (coll.type != kHitNone)
        actor->spr.ang = NORM_ANGLE((actor->spr.ang + 1024) + (RANDOM_NEG(256, 6) >> 6));
    else
        actor->spr.ang = NORM_ANGLE(tang + (RANDOM_NEG(256, 6) >> 6));

    DoActorSetSpeed(actor, FAST_SPEED);

    //actor->user.jump_speed = -800;
    PickJumpMaxSpeed(actor, -400); // was -800

    actor->user.Flags |= (SPR_JUMPING);
    actor->user.Flags &= ~(SPR_FALLING);

    // set up individual actor jump gravity
    actor->user.jump_grav = 17; // was 8

    // if I didn't do this here they get stuck in the air sometimes
    DoActorZrange(actor);

    DoJump(actor);

    return 0;
}

int DoRipperMoveJump(DSWActor* actor)
{
    if (actor->user.Flags & (SPR_JUMPING | SPR_FALLING))
    {
        if (actor->user.Flags & (SPR_JUMPING))
            DoJump(actor);
        else
            DoFall(actor);
    }

    if (!(actor->user.Flags & (SPR_JUMPING | SPR_FALLING)))
    {
        if (DoRipperQuickJump(actor))
            return 0;

        InitActorDecide(actor);
    }

    DoRipperMoveHang(actor);
    return 0;
}

//
// STD MOVEMENT
//

int DoRipperQuickJump(DSWActor* actor)
{
    // Tests to see if ripper is on top of a player/enemy and then immediatly
    // does another jump

    DSWActor* low = actor->user.lowActor;
    if (low)
    {
        if ((low->spr.extra & SPRX_PLAYER_OR_ENEMY))
        {
            NewStateGroup(actor, sg_RipperJumpAttack);
            // move past the first state
            actor->user.Tics = 30;
            return true;
        }
    }
    return false;
}


int NullRipper(DSWActor* actor)
{
    if (actor->user.Flags & (SPR_SLIDING))
        DoActorSlide(actor);

    DoActorSectorDamage(actor);

    return 0;
}


int DoRipperPain(DSWActor* actor)
{
    NullRipper(actor);

    if ((actor->user.WaitTics -= ACTORMOVETICS) <= 0)
        InitActorDecide(actor);
    return 0;
}


int DoRipperRipHeart(DSWActor* actor)
{
    DSWActor* target = actor->user.targetActor;

    NewStateGroup(actor, sg_RipperHeart);
    actor->user.WaitTics = 6 * 120;

    // player face ripper
    target->spr.ang = getangle(actor->spr.pos.X - target->spr.pos.X, actor->spr.pos.Y - target->spr.pos.Y);
    return 0;
}


int DoRipperStandHeart(DSWActor* actor)
{
    NullRipper(actor);

    if ((actor->user.WaitTics -= ACTORMOVETICS) <= 0)
        NewStateGroup(actor, sg_RipperRun);
    return 0;
}

void RipperHatch(DSWActor* actor)
{
	const int MAX_RIPPERS = 1;
    short rip_ang[MAX_RIPPERS];

    rip_ang[0] = RANDOM_P2(2048);

    for (int i = 0; i < MAX_RIPPERS; i++)
    {
        auto actorNew = insertActor(actor->sector(), STAT_DEFAULT);
        ClearOwner(actorNew);
        actorNew->spr.pos.X = actor->spr.pos.X;
        actorNew->spr.pos.Y = actor->spr.pos.Y;
        actorNew->spr.pos.Z = actor->spr.pos.Z;
        actorNew->spr.xrepeat = actorNew->spr.yrepeat = 64;
        actorNew->spr.ang = rip_ang[i];
        actorNew->spr.pal = 0;
        SetupRipper(actorNew);

        // make immediately active
        actorNew->user.Flags |= (SPR_ACTIVE);

        NewStateGroup(actorNew, actorNew->user.ActorActionSet->Jump);
        actorNew->user.ActorActionFunc = DoActorMoveJump;
        DoActorSetSpeed(actorNew, FAST_SPEED);
        PickJumpMaxSpeed(actorNew, -600);

        actorNew->user.Flags |= (SPR_JUMPING);
        actorNew->user.Flags &= ~(SPR_FALLING);

        actorNew->user.jump_grav = 8;

        // if I didn't do this here they get stuck in the air sometimes
        DoActorZrange(actorNew);

        DoJump(actorNew);
    }
}

int DoRipperMove(DSWActor* actor)
{
    if (actor->user.scale_speed)
    {
        DoScaleSprite(actor);
    }

    if (actor->user.Flags & (SPR_JUMPING | SPR_FALLING))
    {
        if (actor->user.Flags & (SPR_JUMPING))
            DoJump(actor);
        else
            DoFall(actor);
    }

    // if on a player/enemy sprite jump quickly
    if (!(actor->user.Flags & (SPR_JUMPING | SPR_FALLING)))
    {
        if (DoRipperQuickJump(actor))
            return 0;

        KeepActorOnFloor(actor);
    }

    if (actor->user.Flags & (SPR_SLIDING))
        DoActorSlide(actor);

    if (actor->user.track >= 0)
        ActorFollowTrack(actor, ACTORMOVETICS);
    else
        (*actor->user.ActorActionFunc)(actor);

    DoActorSectorDamage(actor);
    return 0;
}


#include "saveable.h"

static saveable_code saveable_ripper_code[] =
{

    SAVE_CODE(InitRipperHang),
    SAVE_CODE(DoRipperHang),
    SAVE_CODE(DoRipperMoveHang),
    SAVE_CODE(DoRipperHangJF),

    SAVE_CODE(DoRipperBeginJumpAttack),
    SAVE_CODE(DoRipperMoveJump),

    SAVE_CODE(DoRipperQuickJump),
    SAVE_CODE(NullRipper),
    SAVE_CODE(DoRipperPain),
    SAVE_CODE(DoRipperRipHeart),
    SAVE_CODE(DoRipperStandHeart),
    SAVE_CODE(DoRipperMove),
};

static saveable_data saveable_ripper_data[] =
{
    SAVE_DATA(RipperBattle),
    SAVE_DATA(RipperOffense),
    SAVE_DATA(RipperBroadcast),
    SAVE_DATA(RipperSurprised),
    SAVE_DATA(RipperEvasive),
    SAVE_DATA(RipperLostTarget),
    SAVE_DATA(RipperCloseRange),

    SAVE_DATA(RipperPersonality),

    SAVE_DATA(RipperAttrib),

    SAVE_DATA(s_RipperRun),
    SAVE_DATA(sg_RipperRun),
    SAVE_DATA(s_RipperStand),
    SAVE_DATA(sg_RipperStand),
    SAVE_DATA(s_RipperSwipe),
    SAVE_DATA(sg_RipperSwipe),
    SAVE_DATA(s_RipperSpew),
    SAVE_DATA(sg_RipperSpew),
    SAVE_DATA(s_RipperHeart),
    SAVE_DATA(sg_RipperHeart),
    SAVE_DATA(s_RipperHang),
    SAVE_DATA(sg_RipperHang),
    SAVE_DATA(s_RipperPain),
    SAVE_DATA(sg_RipperPain),
    SAVE_DATA(s_RipperJump),
    SAVE_DATA(sg_RipperJump),
    SAVE_DATA(s_RipperFall),
    SAVE_DATA(sg_RipperFall),
    SAVE_DATA(s_RipperJumpAttack),
    SAVE_DATA(sg_RipperJumpAttack),
    SAVE_DATA(s_RipperHangJump),
    SAVE_DATA(sg_RipperHangJump),
    SAVE_DATA(s_RipperHangFall),
    SAVE_DATA(sg_RipperHangFall),
    SAVE_DATA(s_RipperDie),
    SAVE_DATA(s_RipperDead),
    SAVE_DATA(sg_RipperDie),
    SAVE_DATA(sg_RipperDead),
    SAVE_DATA(s_RipperDeathJump),
    SAVE_DATA(s_RipperDeathFall),
    SAVE_DATA(sg_RipperDeathJump),
    SAVE_DATA(sg_RipperDeathFall),

    SAVE_DATA(RipperActionSet),
    SAVE_DATA(RipperBrownActionSet),
};

saveable_module saveable_ripper =
{
    // code
    saveable_ripper_code,
    SIZ(saveable_ripper_code),

    // data
    saveable_ripper_data,
    SIZ(saveable_ripper_data)
};
END_SW_NS
