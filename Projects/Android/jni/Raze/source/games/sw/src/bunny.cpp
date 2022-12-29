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
#include "tags.h"
#include "ai.h"
#include "pal.h"
#include "sprite.h"
#include "weapon.h"
#include "misc.h"

BEGIN_SW_NS

int Bunny_Count = 0;
ANIMATOR DoActorMoveJump;
ANIMATOR DoBunnyMoveJump;
ANIMATOR DoBunnyQuickJump;

DECISION BunnyBattle[] =
{
    {748, InitActorMoveCloser},
    {750, InitActorAlertNoise},
    {760, InitActorAttackNoise},
    {1024, InitActorMoveCloser}
};

DECISION BunnyOffense[] =
{
    {600, InitActorMoveCloser},
    {700, InitActorAlertNoise},
    {1024, InitActorMoveCloser}
};

DECISION BunnyBroadcast[] =
{
    {21, InitActorAlertNoise},
    {51, InitActorAmbientNoise},
    {1024, InitActorDecide}
};

DECISION BunnySurprised[] =
{
    {500, InitActorRunAway},
    {701, InitActorMoveCloser},
    {1024, InitActorDecide}
};

DECISION BunnyEvasive[] =
{
    {500,  InitActorWanderAround},
    {1020, InitActorRunAway},
    {1024, InitActorAmbientNoise}
};

DECISION BunnyLostTarget[] =
{
    {900, InitActorFindPlayer},
    {1024, InitActorWanderAround}
};

DECISION BunnyCloseRange[] =
{
    {1024,  InitActorAttack             },
//    {1024,  InitActorReposition         }
};

DECISION BunnyWander[] =
{
    {1024, InitActorReposition}
};

PERSONALITY WhiteBunnyPersonality =
{
    BunnyBattle,
    BunnyOffense,
    BunnyBroadcast,
    BunnySurprised,
    BunnyEvasive,
    BunnyLostTarget,
    BunnyCloseRange,
    BunnyCloseRange
};

PERSONALITY BunnyPersonality =
{
    BunnyEvasive,
    BunnyEvasive,
    BunnyEvasive,
    BunnyWander,
    BunnyWander,
    BunnyWander,
    BunnyEvasive,
    BunnyEvasive
};

ATTRIBUTE BunnyAttrib =
{
    {100, 120, 140, 180},               // Speeds
    {5, 0, -2, -4},                     // Tic Adjusts
    3,                                  // MaxWeapons;
    {
        DIGI_BUNNYAMBIENT, 0, DIGI_BUNNYATTACK,
        DIGI_BUNNYATTACK, DIGI_BUNNYDIE2, 0,
        0,0,0,0
    }
};

ATTRIBUTE WhiteBunnyAttrib =
{
    {200, 220, 340, 380},               // Speeds
    {5, 0, -2, -4},                     // Tic Adjusts
    3,                                  // MaxWeapons;
    {
        DIGI_BUNNYAMBIENT, 0, DIGI_BUNNYATTACK,
        DIGI_BUNNYATTACK, DIGI_BUNNYDIE2, 0,
        0,0,0,0
    }
};

//////////////////////
//
// BUNNY RUN
//
//////////////////////

#define BUNNY_RUN_RATE 10

ANIMATOR DoBunnyMove, NullBunny, DoActorDebris, DoBunnyGrowUp;

STATE s_BunnyRun[5][6] =
{
    {
        {BUNNY_RUN_R0 + 0, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[0][1]},
        {BUNNY_RUN_R0 + 1, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[0][2]},
        {BUNNY_RUN_R0 + 2, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[0][3]},
        {BUNNY_RUN_R0 + 3, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[0][4]},
        {BUNNY_RUN_R0 + 4, SF_QUICK_CALL,                DoBunnyGrowUp, &s_BunnyRun[0][5]},
        {BUNNY_RUN_R0 + 4, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[0][0]},
    },
    {
        {BUNNY_RUN_R1 + 0, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[1][1]},
        {BUNNY_RUN_R1 + 1, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[1][2]},
        {BUNNY_RUN_R1 + 2, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[1][3]},
        {BUNNY_RUN_R1 + 3, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[1][4]},
        {BUNNY_RUN_R1 + 4, SF_QUICK_CALL,                DoBunnyGrowUp, &s_BunnyRun[1][5]},
        {BUNNY_RUN_R1 + 4, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[1][0]},
    },
    {
        {BUNNY_RUN_R2 + 0, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[2][1]},
        {BUNNY_RUN_R2 + 1, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[2][2]},
        {BUNNY_RUN_R2 + 2, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[2][3]},
        {BUNNY_RUN_R2 + 3, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[2][4]},
        {BUNNY_RUN_R2 + 4, SF_QUICK_CALL,                DoBunnyGrowUp, &s_BunnyRun[2][5]},
        {BUNNY_RUN_R2 + 4, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[2][0]},
    },
    {
        {BUNNY_RUN_R3 + 0, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[3][1]},
        {BUNNY_RUN_R3 + 1, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[3][2]},
        {BUNNY_RUN_R3 + 2, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[3][3]},
        {BUNNY_RUN_R3 + 3, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[3][4]},
        {BUNNY_RUN_R3 + 4, SF_QUICK_CALL,                DoBunnyGrowUp, &s_BunnyRun[3][5]},
        {BUNNY_RUN_R3 + 4, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[3][0]},
    },
    {
        {BUNNY_RUN_R4 + 0, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[4][1]},
        {BUNNY_RUN_R4 + 1, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[4][2]},
        {BUNNY_RUN_R4 + 2, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[4][3]},
        {BUNNY_RUN_R4 + 3, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[4][4]},
        {BUNNY_RUN_R4 + 4, SF_QUICK_CALL,                DoBunnyGrowUp, &s_BunnyRun[4][5]},
        {BUNNY_RUN_R4 + 4, BUNNY_RUN_RATE | SF_TIC_ADJUST, DoBunnyMove, &s_BunnyRun[4][0]},
    }
};


STATEp sg_BunnyRun[] =
{
    &s_BunnyRun[0][0],
    &s_BunnyRun[1][0],
    &s_BunnyRun[2][0],
    &s_BunnyRun[3][0],
    &s_BunnyRun[4][0]
};

//////////////////////
//
// BUNNY STAND
//
//////////////////////

#define BUNNY_STAND_RATE 12
ANIMATOR DoBunnyEat;

STATE s_BunnyStand[5][3] =
{
    {
        {BUNNY_STAND_R0 + 0, BUNNY_STAND_RATE, DoBunnyEat, &s_BunnyStand[0][1]},
        {BUNNY_STAND_R0 + 4, SF_QUICK_CALL, DoBunnyGrowUp, &s_BunnyStand[0][2]},
        {BUNNY_STAND_R0 + 4, BUNNY_STAND_RATE, DoBunnyEat, &s_BunnyStand[0][0]},
    },
    {
        {BUNNY_STAND_R1 + 0, BUNNY_STAND_RATE, DoBunnyEat, &s_BunnyStand[1][1]},
        {BUNNY_STAND_R1 + 4, SF_QUICK_CALL, DoBunnyGrowUp, &s_BunnyStand[1][2]},
        {BUNNY_STAND_R1 + 4, BUNNY_STAND_RATE, DoBunnyEat, &s_BunnyStand[1][0]},
    },
    {
        {BUNNY_STAND_R2 + 0, BUNNY_STAND_RATE, DoBunnyEat, &s_BunnyStand[2][1]},
        {BUNNY_STAND_R2 + 4, SF_QUICK_CALL, DoBunnyGrowUp, &s_BunnyStand[2][2]},
        {BUNNY_STAND_R2 + 4, BUNNY_STAND_RATE, DoBunnyEat, &s_BunnyStand[2][0]},
    },
    {
        {BUNNY_STAND_R3 + 0, BUNNY_STAND_RATE, DoBunnyEat, &s_BunnyStand[3][1]},
        {BUNNY_STAND_R3 + 4, SF_QUICK_CALL, DoBunnyGrowUp, &s_BunnyStand[3][2]},
        {BUNNY_STAND_R3 + 4, BUNNY_STAND_RATE, DoBunnyEat, &s_BunnyStand[3][0]},
    },
    {
        {BUNNY_STAND_R4 + 0, BUNNY_STAND_RATE, DoBunnyEat, &s_BunnyStand[4][1]},
        {BUNNY_STAND_R4 + 4, SF_QUICK_CALL, DoBunnyGrowUp, &s_BunnyStand[4][2]},
        {BUNNY_STAND_R4 + 4, BUNNY_STAND_RATE, DoBunnyEat, &s_BunnyStand[4][0]},
    },
};


STATEp sg_BunnyStand[] =
{
    s_BunnyStand[0],
    s_BunnyStand[1],
    s_BunnyStand[2],
    s_BunnyStand[3],
    s_BunnyStand[4]
};

//////////////////////
//
// BUNNY GET LAYED
//
//////////////////////

#define BUNNY_SCREW_RATE 16
ANIMATOR DoBunnyScrew;

STATE s_BunnyScrew[5][2] =
{
    {
        {BUNNY_STAND_R0 + 0, BUNNY_SCREW_RATE, DoBunnyScrew, &s_BunnyScrew[0][1]},
        {BUNNY_STAND_R0 + 2, BUNNY_SCREW_RATE, DoBunnyScrew, &s_BunnyScrew[0][0]},
    },
    {
        {BUNNY_STAND_R1 + 0, BUNNY_SCREW_RATE, DoBunnyScrew, &s_BunnyScrew[1][1]},
        {BUNNY_STAND_R1 + 2, BUNNY_SCREW_RATE, DoBunnyScrew, &s_BunnyScrew[1][0]},
    },
    {
        {BUNNY_STAND_R2 + 0, BUNNY_SCREW_RATE, DoBunnyScrew, &s_BunnyScrew[2][1]},
        {BUNNY_STAND_R2 + 2, BUNNY_SCREW_RATE, DoBunnyScrew, &s_BunnyScrew[2][0]},
    },
    {
        {BUNNY_STAND_R3 + 0, BUNNY_SCREW_RATE, DoBunnyScrew, &s_BunnyScrew[3][1]},
        {BUNNY_STAND_R3 + 2, BUNNY_SCREW_RATE, DoBunnyScrew, &s_BunnyScrew[3][0]},
    },
    {
        {BUNNY_STAND_R4 + 0, BUNNY_SCREW_RATE, DoBunnyScrew, &s_BunnyScrew[4][1]},
        {BUNNY_STAND_R4 + 2, BUNNY_SCREW_RATE, DoBunnyScrew, &s_BunnyScrew[4][0]},
    },
};


STATEp sg_BunnyScrew[] =
{
    s_BunnyScrew[0],
    s_BunnyScrew[1],
    s_BunnyScrew[2],
    s_BunnyScrew[3],
    s_BunnyScrew[4]
};

//////////////////////
//
// BUNNY SWIPE
//
//////////////////////

#define BUNNY_SWIPE_RATE 8
ANIMATOR InitActorDecide;
ANIMATOR InitBunnySlash;

STATE s_BunnySwipe[5][8] =
{
    {
        {BUNNY_SWIPE_R0 + 0, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[0][1]},
        {BUNNY_SWIPE_R0 + 1, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[0][2]},
        {BUNNY_SWIPE_R0 + 1, 0 | SF_QUICK_CALL, InitBunnySlash, &s_BunnySwipe[0][3]},
        {BUNNY_SWIPE_R0 + 2, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[0][4]},
        {BUNNY_SWIPE_R0 + 3, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[0][5]},
        {BUNNY_SWIPE_R0 + 3, 0 | SF_QUICK_CALL, InitBunnySlash, &s_BunnySwipe[0][6]},
        {BUNNY_SWIPE_R0 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_BunnySwipe[0][7]},
        {BUNNY_SWIPE_R0 + 3, BUNNY_SWIPE_RATE, DoBunnyMove, &s_BunnySwipe[0][7]},
    },
    {
        {BUNNY_SWIPE_R1 + 0, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[1][1]},
        {BUNNY_SWIPE_R1 + 1, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[1][2]},
        {BUNNY_SWIPE_R1 + 1, 0 | SF_QUICK_CALL, InitBunnySlash, &s_BunnySwipe[1][3]},
        {BUNNY_SWIPE_R1 + 2, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[1][4]},
        {BUNNY_SWIPE_R1 + 3, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[1][5]},
        {BUNNY_SWIPE_R1 + 3, 0 | SF_QUICK_CALL, InitBunnySlash, &s_BunnySwipe[1][6]},
        {BUNNY_SWIPE_R1 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_BunnySwipe[1][7]},
        {BUNNY_SWIPE_R1 + 3, BUNNY_SWIPE_RATE, DoBunnyMove, &s_BunnySwipe[1][7]},
    },
    {
        {BUNNY_SWIPE_R2 + 0, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[2][1]},
        {BUNNY_SWIPE_R2 + 1, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[2][2]},
        {BUNNY_SWIPE_R2 + 1, 0 | SF_QUICK_CALL, InitBunnySlash, &s_BunnySwipe[2][3]},
        {BUNNY_SWIPE_R2 + 2, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[2][4]},
        {BUNNY_SWIPE_R2 + 3, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[2][5]},
        {BUNNY_SWIPE_R2 + 3, 0 | SF_QUICK_CALL, InitBunnySlash, &s_BunnySwipe[2][6]},
        {BUNNY_SWIPE_R2 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_BunnySwipe[2][7]},
        {BUNNY_SWIPE_R2 + 3, BUNNY_SWIPE_RATE, DoBunnyMove, &s_BunnySwipe[2][7]},
    },
    {
        {BUNNY_SWIPE_R3 + 0, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[3][1]},
        {BUNNY_SWIPE_R3 + 1, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[3][2]},
        {BUNNY_SWIPE_R3 + 1, 0 | SF_QUICK_CALL, InitBunnySlash, &s_BunnySwipe[3][3]},
        {BUNNY_SWIPE_R3 + 2, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[3][4]},
        {BUNNY_SWIPE_R3 + 3, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[3][5]},
        {BUNNY_SWIPE_R3 + 3, 0 | SF_QUICK_CALL, InitBunnySlash, &s_BunnySwipe[3][6]},
        {BUNNY_SWIPE_R3 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_BunnySwipe[3][7]},
        {BUNNY_SWIPE_R3 + 3, BUNNY_SWIPE_RATE, DoBunnyMove, &s_BunnySwipe[3][7]},
    },
    {
        {BUNNY_SWIPE_R4 + 0, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[4][1]},
        {BUNNY_SWIPE_R4 + 1, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[4][2]},
        {BUNNY_SWIPE_R4 + 1, 0 | SF_QUICK_CALL, InitBunnySlash, &s_BunnySwipe[4][3]},
        {BUNNY_SWIPE_R4 + 2, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[4][4]},
        {BUNNY_SWIPE_R4 + 3, BUNNY_SWIPE_RATE, NullBunny, &s_BunnySwipe[4][5]},
        {BUNNY_SWIPE_R4 + 3, 0 | SF_QUICK_CALL, InitBunnySlash, &s_BunnySwipe[4][6]},
        {BUNNY_SWIPE_R4 + 3, 0 | SF_QUICK_CALL, InitActorDecide, &s_BunnySwipe[4][7]},
        {BUNNY_SWIPE_R4 + 3, BUNNY_SWIPE_RATE, DoBunnyMove, &s_BunnySwipe[4][7]},
    }
};


STATEp sg_BunnySwipe[] =
{
    &s_BunnySwipe[0][0],
    &s_BunnySwipe[1][0],
    &s_BunnySwipe[2][0],
    &s_BunnySwipe[3][0],
    &s_BunnySwipe[4][0]
};


//////////////////////
//
// BUNNY HEART - show players heart
//
//////////////////////

#define BUNNY_HEART_RATE 14
ANIMATOR DoBunnyStandKill;

STATE s_BunnyHeart[5][4] =
{
    {
        {BUNNY_SWIPE_R0 + 0, BUNNY_HEART_RATE, DoBunnyStandKill, &s_BunnyHeart[0][0]},
    },
    {
        {BUNNY_SWIPE_R1 + 0, BUNNY_HEART_RATE, DoBunnyStandKill, &s_BunnyHeart[1][0]},
    },
    {
        {BUNNY_SWIPE_R2 + 0, BUNNY_HEART_RATE, DoBunnyStandKill, &s_BunnyHeart[2][0]},
    },
    {
        {BUNNY_SWIPE_R3 + 0, BUNNY_HEART_RATE, DoBunnyStandKill, &s_BunnyHeart[3][0]},
    },
    {
        {BUNNY_SWIPE_R4 + 0, BUNNY_HEART_RATE, DoBunnyStandKill, &s_BunnyHeart[4][0]},
    }
};


STATEp sg_BunnyHeart[] =
{
    &s_BunnyHeart[0][0],
    &s_BunnyHeart[1][0],
    &s_BunnyHeart[2][0],
    &s_BunnyHeart[3][0],
    &s_BunnyHeart[4][0]
};

//////////////////////
//
// BUNNY PAIN
//
//////////////////////

#define BUNNY_PAIN_RATE 38
ANIMATOR DoBunnyPain;

STATE s_BunnyPain[5][1] =
{
    {
        {BUNNY_SWIPE_R0 + 0, BUNNY_PAIN_RATE, DoBunnyPain, &s_BunnyPain[0][0]},
    },
    {
        {BUNNY_SWIPE_R0 + 0, BUNNY_PAIN_RATE, DoBunnyPain, &s_BunnyPain[1][0]},
    },
    {
        {BUNNY_SWIPE_R0 + 0, BUNNY_PAIN_RATE, DoBunnyPain, &s_BunnyPain[2][0]},
    },
    {
        {BUNNY_SWIPE_R0 + 0, BUNNY_PAIN_RATE, DoBunnyPain, &s_BunnyPain[3][0]},
    },
    {
        {BUNNY_SWIPE_R0 + 0, BUNNY_PAIN_RATE, DoBunnyPain, &s_BunnyPain[4][0]},
    }
};

STATEp sg_BunnyPain[] =
{
    &s_BunnyPain[0][0],
    &s_BunnyPain[1][0],
    &s_BunnyPain[2][0],
    &s_BunnyPain[3][0],
    &s_BunnyPain[4][0]
};

//////////////////////
//
// BUNNY JUMP
//
//////////////////////

#define BUNNY_JUMP_RATE 25

STATE s_BunnyJump[5][6] =
{
    {
        {BUNNY_RUN_R0 + 1, BUNNY_JUMP_RATE, DoBunnyMoveJump, &s_BunnyJump[0][1]},
        {BUNNY_RUN_R0 + 2, BUNNY_JUMP_RATE, DoBunnyMoveJump, &s_BunnyJump[0][1]},
    },
    {
        {BUNNY_RUN_R1 + 1, BUNNY_JUMP_RATE, DoBunnyMoveJump, &s_BunnyJump[1][1]},
        {BUNNY_RUN_R1 + 2, BUNNY_JUMP_RATE, DoBunnyMoveJump, &s_BunnyJump[1][1]},
    },
    {
        {BUNNY_RUN_R2 + 1, BUNNY_JUMP_RATE, DoBunnyMoveJump, &s_BunnyJump[2][1]},
        {BUNNY_RUN_R2 + 2, BUNNY_JUMP_RATE, DoBunnyMoveJump, &s_BunnyJump[2][1]},
    },
    {
        {BUNNY_RUN_R3 + 1, BUNNY_JUMP_RATE, DoBunnyMoveJump, &s_BunnyJump[3][1]},
        {BUNNY_RUN_R3 + 2, BUNNY_JUMP_RATE, DoBunnyMoveJump, &s_BunnyJump[3][1]},
    },
    {
        {BUNNY_RUN_R4 + 1, BUNNY_JUMP_RATE, DoBunnyMoveJump, &s_BunnyJump[4][1]},
        {BUNNY_RUN_R4 + 2, BUNNY_JUMP_RATE, DoBunnyMoveJump, &s_BunnyJump[4][1]},
    }
};


STATEp sg_BunnyJump[] =
{
    &s_BunnyJump[0][0],
    &s_BunnyJump[1][0],
    &s_BunnyJump[2][0],
    &s_BunnyJump[3][0],
    &s_BunnyJump[4][0]
};


//////////////////////
//
// BUNNY FALL
//
//////////////////////

#define BUNNY_FALL_RATE 25

STATE s_BunnyFall[5][6] =
{
    {
        {BUNNY_RUN_R0 + 3, BUNNY_FALL_RATE, DoBunnyMoveJump, &s_BunnyFall[0][0]},
    },
    {
        {BUNNY_RUN_R1 + 3, BUNNY_FALL_RATE, DoBunnyMoveJump, &s_BunnyFall[1][0]},
    },
    {
        {BUNNY_RUN_R2 + 3, BUNNY_FALL_RATE, DoBunnyMoveJump, &s_BunnyFall[2][0]},
    },
    {
        {BUNNY_RUN_R3 + 3, BUNNY_FALL_RATE, DoBunnyMoveJump, &s_BunnyFall[3][0]},
    },
    {
        {BUNNY_RUN_R4 + 3, BUNNY_FALL_RATE, DoBunnyMoveJump, &s_BunnyFall[4][0]},
    }
};


STATEp sg_BunnyFall[] =
{
    &s_BunnyFall[0][0],
    &s_BunnyFall[1][0],
    &s_BunnyFall[2][0],
    &s_BunnyFall[3][0],
    &s_BunnyFall[4][0]
};


//////////////////////
//
// BUNNY JUMP ATTACK
//
//////////////////////

#define BUNNY_JUMP_ATTACK_RATE 35
int DoBunnyBeginJumpAttack(DSWActor* actor);

STATE s_BunnyJumpAttack[5][6] =
{
    {
        {BUNNY_RUN_R0 + 1, BUNNY_JUMP_ATTACK_RATE, NullBunny, &s_BunnyJumpAttack[0][1]},
        {BUNNY_RUN_R0 + 1, 0 | SF_QUICK_CALL, DoBunnyBeginJumpAttack, &s_BunnyJumpAttack[0][2]},
        {BUNNY_RUN_R0 + 2, BUNNY_JUMP_ATTACK_RATE, DoBunnyMoveJump, &s_BunnyJumpAttack[0][2]},
    },
    {
        {BUNNY_RUN_R1 + 1, BUNNY_JUMP_ATTACK_RATE, NullBunny, &s_BunnyJumpAttack[1][1]},
        {BUNNY_RUN_R1 + 1, 0 | SF_QUICK_CALL, DoBunnyBeginJumpAttack, &s_BunnyJumpAttack[1][2]},
        {BUNNY_RUN_R1 + 2, BUNNY_JUMP_ATTACK_RATE, DoBunnyMoveJump, &s_BunnyJumpAttack[1][2]},
    },
    {
        {BUNNY_RUN_R2 + 1, BUNNY_JUMP_ATTACK_RATE, NullBunny, &s_BunnyJumpAttack[2][1]},
        {BUNNY_RUN_R2 + 1, 0 | SF_QUICK_CALL, DoBunnyBeginJumpAttack, &s_BunnyJumpAttack[2][2]},
        {BUNNY_RUN_R2 + 2, BUNNY_JUMP_ATTACK_RATE, DoBunnyMoveJump, &s_BunnyJumpAttack[2][2]},
    },
    {
        {BUNNY_RUN_R3 + 1, BUNNY_JUMP_ATTACK_RATE, NullBunny, &s_BunnyJumpAttack[3][1]},
        {BUNNY_RUN_R3 + 1, 0 | SF_QUICK_CALL, DoBunnyBeginJumpAttack, &s_BunnyJumpAttack[3][2]},
        {BUNNY_RUN_R3 + 2, BUNNY_JUMP_ATTACK_RATE, DoBunnyMoveJump, &s_BunnyJumpAttack[3][2]},
    },
    {
        {BUNNY_RUN_R4 + 1, BUNNY_JUMP_ATTACK_RATE, NullBunny, &s_BunnyJumpAttack[4][1]},
        {BUNNY_RUN_R4 + 1, 0 | SF_QUICK_CALL, DoBunnyBeginJumpAttack, &s_BunnyJumpAttack[4][2]},
        {BUNNY_RUN_R4 + 2, BUNNY_JUMP_ATTACK_RATE, DoBunnyMoveJump, &s_BunnyJumpAttack[4][2]},
    }
};


STATEp sg_BunnyJumpAttack[] =
{
    &s_BunnyJumpAttack[0][0],
    &s_BunnyJumpAttack[1][0],
    &s_BunnyJumpAttack[2][0],
    &s_BunnyJumpAttack[3][0],
    &s_BunnyJumpAttack[4][0]
};


//////////////////////
//
// BUNNY DIE
//
//////////////////////

#define BUNNY_DIE_RATE 16
ANIMATOR BunnySpew;

STATE s_BunnyDie[] =
{
    {BUNNY_DIE + 0, BUNNY_DIE_RATE, NullBunny, &s_BunnyDie[1]},
    {BUNNY_DIE + 0, SF_QUICK_CALL,  BunnySpew, &s_BunnyDie[2]},
    {BUNNY_DIE + 1, BUNNY_DIE_RATE, NullBunny, &s_BunnyDie[3]},
    {BUNNY_DIE + 2, BUNNY_DIE_RATE, NullBunny, &s_BunnyDie[4]},
    {BUNNY_DIE + 2, BUNNY_DIE_RATE, NullBunny, &s_BunnyDie[5]},
    {BUNNY_DEAD, BUNNY_DIE_RATE, DoActorDebris, &s_BunnyDie[5]},
};

#define BUNNY_DEAD_RATE 8

STATE s_BunnyDead[] =
{
    {BUNNY_DIE + 0, BUNNY_DEAD_RATE, NullAnimator, &s_BunnyDie[1]},
    {BUNNY_DIE + 0, SF_QUICK_CALL,  BunnySpew, &s_BunnyDie[2]},
    {BUNNY_DIE + 1, BUNNY_DEAD_RATE, NullAnimator, &s_BunnyDead[3]},
    {BUNNY_DIE + 2, BUNNY_DEAD_RATE, NullAnimator, &s_BunnyDead[4]},
    {BUNNY_DEAD, SF_QUICK_CALL, QueueFloorBlood, &s_BunnyDead[5]},
    {BUNNY_DEAD, BUNNY_DEAD_RATE, DoActorDebris, &s_BunnyDead[5]},
};

STATEp sg_BunnyDie[] =
{
    s_BunnyDie
};

STATEp sg_BunnyDead[] =
{
    s_BunnyDead
};

STATE s_BunnyDeathJump[] =
{
    {BUNNY_DIE + 0, BUNNY_DIE_RATE, DoActorDeathMove, &s_BunnyDeathJump[0]}
};

STATE s_BunnyDeathFall[] =
{
    {BUNNY_DIE + 1, BUNNY_DIE_RATE, DoActorDeathMove, &s_BunnyDeathFall[0]}
};

STATEp sg_BunnyDeathJump[] =
{
    s_BunnyDeathJump
};

STATEp sg_BunnyDeathFall[] =
{
    s_BunnyDeathFall
};


/*
STATEp *Stand[MAX_WEAPONS];
STATEp *Run;
STATEp *Jump;
STATEp *Fall;
STATEp *Crawl;
STATEp *Swim;
STATEp *Fly;
STATEp *Rise;
STATEp *Sit;
STATEp *Look;
STATEp *Climb;
STATEp *Pain;
STATEp *Death1;
STATEp *Death2;
STATEp *Dead;
STATEp *DeathJump;
STATEp *DeathFall;
STATEp *CloseAttack[2];
STATEp *Attack[6];
STATEp *Special[2];
*/

ACTOR_ACTION_SET BunnyActionSet =
{
    sg_BunnyStand,
    sg_BunnyRun,
    sg_BunnyJump,
    sg_BunnyFall,
    nullptr,                               // sg_BunnyCrawl,
    nullptr,                               // sg_BunnySwim,
    nullptr,                               // sg_BunnyFly,
    nullptr,                               // sg_BunnyRise,
    nullptr,                               // sg_BunnySit,
    nullptr,                               // sg_BunnyLook,
    nullptr,                               // climb
    sg_BunnyPain,
    sg_BunnyDie,
    nullptr,
    sg_BunnyDead,
    sg_BunnyDeathJump,
    sg_BunnyDeathFall,
    {nullptr},
    {1024},
    {nullptr},
    {1024},
    {sg_BunnyHeart, sg_BunnyRun},
    nullptr,
    nullptr
};

ACTOR_ACTION_SET BunnyWhiteActionSet =
{
    sg_BunnyStand,
    sg_BunnyRun,
    sg_BunnyJump,
    sg_BunnyFall,
    nullptr,                               // sg_BunnyCrawl,
    nullptr,                               // sg_BunnySwim,
    nullptr,                               // sg_BunnyFly,
    nullptr,                               // sg_BunnyRise,
    nullptr,                               // sg_BunnySit,
    nullptr,                               // sg_BunnyLook,
    nullptr,                               // climb
    sg_BunnyPain,                       // pain
    sg_BunnyDie,
    nullptr,
    sg_BunnyDead,
    sg_BunnyDeathJump,
    sg_BunnyDeathFall,
    {sg_BunnySwipe},
    {1024},
//    {sg_BunnyJumpAttack, sg_BunnySwipe},
//    {800, 1024},
    {sg_BunnySwipe},
    {1024},
    {sg_BunnyHeart, sg_BunnySwipe},
    nullptr,
    nullptr
};

int SetupBunny(DSWActor* actor)
{
    SPRITEp sp = &actor->s();
    USERp u;
    ANIMATOR DoActorDecide;

    if (TEST(sp->cstat, CSTAT_SPRITE_RESTORE))
    {
        u = actor->u();
        ASSERT(u);
    }
    else
    {
        u = SpawnUser(actor, BUNNY_RUN_R0, s_BunnyRun[0]);
        u->Health = 10;
    }

    Bunny_Count++;

    ChangeState(actor, s_BunnyRun[0]);
    u->StateEnd = s_BunnyDie;
    u->Rot = sg_BunnyRun;
    u->ShellNum = 0; // Not Pregnant right now
    u->FlagOwner = 0;

    sp->clipdist = (150) >> 2;

    if (sp->pal == PALETTE_PLAYER1)
    {
        EnemyDefaults(actor, &BunnyWhiteActionSet, &WhiteBunnyPersonality);
        u->Attrib = &WhiteBunnyAttrib;
        sp->xrepeat = 96;
        sp->yrepeat = 90;

        sp->clipdist = 200>>2;

        if (!TEST(sp->cstat, CSTAT_SPRITE_RESTORE))
            u->Health = 60;
    }
    else if (sp->pal == PALETTE_PLAYER8) // Male Rabbit
    {
        EnemyDefaults(actor, &BunnyActionSet, &BunnyPersonality);
        u->Attrib = &BunnyAttrib;
        //sp->xrepeat = 76;
        //sp->yrepeat = 70;

        //sp->shade = 0; // darker
        if (!TEST(sp->cstat, CSTAT_SPRITE_RESTORE))
            u->Health = 20;
        u->Flag1 = 0;
    }
    else
    {
        // Female Rabbit
        EnemyDefaults(actor, &BunnyActionSet, &BunnyPersonality);
        u->Attrib = &BunnyAttrib;
        u->spal = sp->pal = PALETTE_PLAYER0;
        u->Flag1 = SEC(5);
        //sp->shade = 0; // darker
    }

    DoActorSetSpeed(actor, FAST_SPEED);

    SET(u->Flags, SPR_XFLIP_TOGGLE);


    u->zclip = Z(16);
    u->floor_dist = Z(8);
    u->ceiling_dist = Z(8);
    u->lo_step = Z(16);

    return 0;
}

int GetBunnyJumpHeight(int jump_speed, int jump_grav)
{
    int jump_iterations;
    int height;

    jump_speed = abs(jump_speed);

    jump_iterations = jump_speed / (jump_grav * ACTORMOVETICS);

    height = jump_speed * jump_iterations * ACTORMOVETICS;

    height = DIV256(height);

    return DIV2(height);
}

int PickBunnyJumpSpeed(DSWActor* actor, int pix_height)
{
    USERp u = actor->u();

    ASSERT(pix_height < 128);

    u->jump_speed = -600;
    u->jump_grav = 8;

    while (true)
    {
        if (GetBunnyJumpHeight(u->jump_speed, u->jump_grav) > pix_height + 20)
            break;

        u->jump_speed -= 100;

        ASSERT(u->jump_speed > -3000);
    }

    return u->jump_speed;
}

//
// JUMP ATTACK
//

int DoBunnyBeginJumpAttack(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();
    SPRITEp psp = &u->targetActor->s();
    short tang;

    tang = getangle(psp->x - sp->x, psp->y - sp->y);

    Collision coll = move_sprite(actor, bcos(tang, -7), bsin(tang, -7),
        0L, u->ceiling_dist, u->floor_dist, CLIPMASK_ACTOR, ACTORMOVETICS);

    if (coll.type != kHitNone)
        sp->ang = NORM_ANGLE(sp->ang + 1024) + (RANDOM_NEG(256, 6) >> 6);
    else
        sp->ang = NORM_ANGLE(tang + (RANDOM_NEG(256, 6) >> 6));

    DoActorSetSpeed(actor, FAST_SPEED);

    //u->jump_speed = -800;
    PickJumpMaxSpeed(actor, -400); // was -800

    SET(u->Flags, SPR_JUMPING);
    RESET(u->Flags, SPR_FALLING);

    // set up individual actor jump gravity
    u->jump_grav = 17; // was 8

    // if I didn't do this here they get stuck in the air sometimes
    DoActorZrange(actor);

    DoJump(actor);

    return 0;
}

int DoBunnyMoveJump(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    if (TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
    {
        int nx, ny;

        // Move while jumping
        nx = MulScale(sp->xvel, bcos(sp->ang), 14);
        ny = MulScale(sp->xvel, bsin(sp->ang), 14);

        move_actor(actor, nx, ny, 0L);

        if (TEST(u->Flags, SPR_JUMPING))
            DoActorJump(actor);
        else
            DoActorFall(actor);
    }

    DoActorZrange(actor);

    if (!TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
    {
        InitActorDecide(actor);
    }

    return 0;
}

void DoPickCloseBunny(DSWActor* actor)
{
    auto u = actor->u();
    SPRITEp sp = &actor->s();
    int dist, near_dist = 1000, a,b,c;

    // if actor can still see the player
    int look_height = SPRITEp_TOS(sp);
    bool ICanSee = false;

    SWStatIterator it(STAT_ENEMY);
    while (auto itActor = it.Next())
    {
        auto tsp = &itActor->s();
        auto tu = itActor->u();

        if (sp == tsp) continue;

        if (tu->ID != BUNNY_RUN_R0) continue;

        DISTANCE(tsp->x, tsp->y, sp->x, sp->y, dist, a, b, c);

        if (dist > near_dist) continue;

        ICanSee = FAFcansee(sp->x, sp->y, look_height, sp->sectnum, tsp->x, tsp->y, SPRITEp_UPPER(tsp), tsp->sectnum);

        if (ICanSee && dist < near_dist && tu->ID == BUNNY_RUN_R0)
        {
            near_dist = dist;
            u->targetActor = itActor;
            u->lowActor = itActor;
            //Bunny_Result = i;
            return;
        }
    }
}

int DoBunnyQuickJump(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    if (u->spal != PALETTE_PLAYER8) return false;

    if (!u->lowActor&& u->spal == PALETTE_PLAYER8 && MoveSkip4)
        DoPickCloseBunny(actor);

    // Random Chance of like sexes fighting
    if (u->lowActor)
    {
        auto hitActor = u->lowActor;
        SPRITEp tsp = &hitActor->s();
        USERp tu = hitActor->u();

        if (!tu || tu->ID != BUNNY_RUN_R0) return false;


        // Not mature enough yet
        if (sp->xrepeat != 64 || sp->yrepeat != 64) return false;
        if (tsp->xrepeat != 64 || tsp->yrepeat != 64) return false;

        // Kill a rival
        // Only males fight
        if (tu->spal == sp->pal && RandomRange(1000) > 995)
        {
            if (u->spal == PALETTE_PLAYER8 && tu->spal == PALETTE_PLAYER8)
            {
                PlaySound(DIGI_BUNNYATTACK, actor, v3df_follow);
                PlaySound(DIGI_BUNNYDIE2, hitActor, v3df_follow);
                tu->Health = 0;

                // Blood fountains
                InitBloodSpray(hitActor, true,-1);

                if (SpawnShrap(hitActor, actor))
                {
                    SetSuicide(hitActor);
                }
                else
                    DoActorDie(hitActor, actor, 0);

                Bunny_Count--; // Bunny died

                u->lowActor = nullptr;
                return true;
            }
        }
    }

    // Get layed!
    if (u->lowActor && u->spal == PALETTE_PLAYER8) // Only males check this
    {
        auto hitActor = u->lowActor;
        SPRITEp tsp = &hitActor->s();
        USERp tu = hitActor->u();


        if (!tu || tu->ID != BUNNY_RUN_R0) return false;

        // Not mature enough to mate yet
        if (sp->xrepeat != 64 || sp->yrepeat != 64) return false;
        if (tsp->xrepeat != 64 || tsp->yrepeat != 64) return false;

        if (tu->ShellNum <= 0 && tu->WaitTics <= 0 && u->WaitTics <= 0)
        {
            if (TEST(tsp->extra, SPRX_PLAYER_OR_ENEMY))
            {
                PLAYERp pp = nullptr;

                if (RandomRange(1000) < 995 && tu->spal != PALETTE_PLAYER0) return false;

                DoActorPickClosePlayer(actor);

                if (u->targetActor->u()->PlayerP)
                    pp = u->targetActor->u()->PlayerP;

                if (tu->spal != PALETTE_PLAYER0)
                {
                    if (tu->Flag1 > 0) return false;
                    tu->FlagOwner = 1; // FAG!
                    tu->Flag1 = SEC(10);
                    if (pp)
                    {
                        int choose_snd;
                        static const int fagsnds[] = {DIGI_FAGRABBIT1,DIGI_FAGRABBIT2,DIGI_FAGRABBIT3};

                        if (pp == Player+myconnectindex)
                        {
                            choose_snd = STD_RANDOM_RANGE(2<<8)>>8;
                            if (FAFcansee(sp->x,sp->y,SPRITEp_TOS(sp),sp->sectnum,pp->posx, pp->posy, pp->posz, pp->cursectnum) && Facing(actor, u->targetActor))
                                PlayerSound(fagsnds[choose_snd], v3df_doppler|v3df_follow|v3df_dontpan,pp);
                        }
                    }
                }
                else
                {
                    if (pp && RandomRange(1000) > 200)
                    {
                        int choose_snd;
                        static const int straightsnds[] = {DIGI_RABBITHUMP1,DIGI_RABBITHUMP2, DIGI_RABBITHUMP3,DIGI_RABBITHUMP4};

                        if (pp == Player+myconnectindex)
                        {
                            choose_snd = STD_RANDOM_RANGE(3<<8)>>8;
                            if (FAFcansee(sp->x,sp->y,SPRITEp_TOS(sp),sp->sectnum,pp->posx, pp->posy, pp->posz, pp->cursectnum) && Facing(actor, u->targetActor))
                                PlayerSound(straightsnds[choose_snd], v3df_doppler|v3df_follow|v3df_dontpan,pp);
                        }
                    }
                }

                sp->x = tsp->x; // Mount up little bunny
                sp->y = tsp->y;
                sp->ang = tsp->ang;
                sp->ang = NORM_ANGLE(sp->ang + 1024);
                HelpMissileLateral(actor, 2000);
                sp->ang = tsp->ang;
                u->Vis = sp->ang;  // Remember angles for later
                tu->Vis = tsp->ang;

                NewStateGroup(actor, sg_BunnyScrew);
                NewStateGroup(hitActor, sg_BunnyScrew);
                u->WaitTics = tu->WaitTics = SEC(10);  // Mate for this long
                return true;
            }
        }
    }

    return false;
}


int NullBunny(DSWActor* actor)
{
    USER* u = actor->u();

    if (TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
    {
        if (TEST(u->Flags, SPR_JUMPING))
            DoActorJump(actor);
        else
            DoActorFall(actor);
    }

    // stay on floor unless doing certain things
    if (!TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
        KeepActorOnFloor(actor);

    if (TEST(u->Flags,SPR_SLIDING))
        DoActorSlide(actor);

    DoActorSectorDamage(actor);

    return 0;
}


int DoBunnyPain(DSWActor* actor)
{
    USER* u = actor->u();

    NullBunny(actor);

    if ((u->WaitTics -= ACTORMOVETICS) <= 0)
        InitActorDecide(actor);
    return 0;
}

int DoBunnyRipHeart(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    SPRITEp tsp = &u->targetActor->s();

    NewStateGroup(actor, sg_BunnyHeart);
    u->WaitTics = 6 * 120;

    // player face bunny
    tsp->ang = getangle(sp->x - tsp->x, sp->y - tsp->y);
    return 0;
}

int DoBunnyStandKill(DSWActor* actor)
{
    USER* u = actor->u();

    NullBunny(actor);

    // Growl like the bad ass bunny you are!
    if (RandomRange(1000) > 800)
        PlaySound(DIGI_BUNNYATTACK, actor, v3df_none);

    if ((u->WaitTics -= ACTORMOVETICS) <= 0)
        NewStateGroup(actor, sg_BunnyRun);
    return 0;
}


void BunnyHatch(DSWActor* actor)
{
    SPRITEp sp = &actor->s();
    USERp u = actor->u();

    SPRITEp np;
    USERp nu;
    const int MAX_BUNNYS = 1;
    short rip_ang[MAX_BUNNYS];

    rip_ang[0] = RANDOM_P2(2048);

    for (int i = 0; i < MAX_BUNNYS; i++)
    {
        auto actorNew = InsertActor(sp->sectnum, STAT_DEFAULT);
        np = &actorNew->s();
        np->clear();
        np->sectnum = sp->sectnum;
        np->statnum = STAT_DEFAULT;
        np->x = sp->x;
        np->y = sp->y;
        np->z = sp->z;
        np->xrepeat = 30;  // Baby size
        np->yrepeat = 24;
        np->ang = rip_ang[i];
        np->pal = 0;
        SetupBunny(actorNew);
        nu = actorNew->u();
        np->shade = sp->shade;

        // make immediately active
        SET(nu->Flags, SPR_ACTIVE);
        if (RandomRange(1000) > 500) // Boy or Girl?
            nu->spal = np->pal = PALETTE_PLAYER0; // Girl
        else
        {
            nu->spal = np->pal = PALETTE_PLAYER8; // Boy
            // Oops, mommy died giving birth to a boy
            if (RandomRange(1000) > 500)
            {
                u->Health = 0;
                Bunny_Count--; // Bunny died

                // Blood fountains
                InitBloodSpray(actor, true, -1);

                if (SpawnShrap(actor, actorNew))
                {
                    SetSuicide(actor);
                }
                else
                    DoActorDie(actor, actorNew, 0);
            }
        }

        nu->ShellNum = 0; // Not Pregnant right now

        NewStateGroup(actorNew, nu->ActorActionSet->Jump);
        nu->ActorActionFunc = DoActorMoveJump;
        DoActorSetSpeed(actorNew, FAST_SPEED);
        PickJumpMaxSpeed(actorNew, -600);

        SET(nu->Flags, SPR_JUMPING);
        RESET(nu->Flags, SPR_FALLING);

        nu->jump_grav = 8;

        // if I didn't do this here they get stuck in the air sometimes
        DoActorZrange(actorNew);

        DoActorJump(actorNew);
    }
}

DSWActor* BunnyHatch2(DSWActor* actor)
{
    SPRITEp wp = &actor->s();


    auto actorNew = InsertActor(wp->sectnum, STAT_DEFAULT);
    auto np = &actorNew->s();
    np->clear();
    np->sectnum = wp->sectnum;
    np->statnum = STAT_DEFAULT;
    np->x = wp->x;
    np->y = wp->y;
    np->z = wp->z;
    np->xrepeat = 30;  // Baby size
    np->yrepeat = 24;
    np->ang = RANDOM_P2(2048);
    np->pal = 0;
    SetupBunny(actorNew);
    auto nu = actorNew->u();
    np->shade = wp->shade;

    // make immediately active
    SET(nu->Flags, SPR_ACTIVE);
    if (RandomRange(1000) > 500) // Boy or Girl?
    {
        nu->spal = np->pal = PALETTE_PLAYER0; // Girl
        nu->Flag1 = SEC(5);
    }
    else
    {
        nu->spal = np->pal = PALETTE_PLAYER8; // Boy
        nu->Flag1 = 0;
    }

    nu->ShellNum = 0; // Not Pregnant right now

    NewStateGroup(actorNew, nu->ActorActionSet->Jump);
    nu->ActorActionFunc = DoActorMoveJump;
    DoActorSetSpeed(actorNew, FAST_SPEED);
    if (TEST_BOOL3(wp))
    {
        PickJumpMaxSpeed(actorNew, -600-RandomRange(600));
        np->xrepeat = np->yrepeat = 64;
        np->xvel = 150 + RandomRange(1000);
        nu->Health = 1; // Easy to pop. Like shootn' skeet.
        np->ang -= RandomRange(128);
        np->ang += RandomRange(128);
    }
    else
        PickJumpMaxSpeed(actorNew, -600);

    SET(nu->Flags, SPR_JUMPING);
    RESET(nu->Flags, SPR_FALLING);

    nu->jump_grav = 8;
    nu->FlagOwner = 0;

    nu->active_range = 75000; // Set it far

    // if I didn't do this here they get stuck in the air sometimes
    DoActorZrange(actorNew);

    DoActorJump(actorNew);

    return actorNew;
}

int DoBunnyMove(DSWActor* actor)
{
    USER* u = actor->u();
    auto sp = &actor->s();

    // Parental lock crap
    if (TEST(sp->cstat, CSTAT_SPRITE_INVISIBLE))
        RESET(sp->cstat, CSTAT_SPRITE_INVISIBLE); // Turn em' back on

    // Sometimes they just won't die!
    if (u->Health <= 0)
        SetSuicide(actor);

    if (u->scale_speed)
    {
        DoScaleSprite(actor);
    }

    if (TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
    {
        if (TEST(u->Flags, SPR_JUMPING))
            DoActorJump(actor);
        else
            DoActorFall(actor);
    }

    // if on a player/enemy sprite jump quickly
    if (!TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
    {
        DoBunnyQuickJump(actor);
    }

    if (TEST(u->Flags, SPR_SLIDING))
        DoActorSlide(actor);

    if (u->track >= 0)
        ActorFollowTrack(actor, ACTORMOVETICS);
    else
        (*u->ActorActionFunc)(actor);

    // stay on floor unless doing certain things
    if (!TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
        KeepActorOnFloor(actor);

    DoActorSectorDamage(actor);

    if (RandomRange(1000) > 985 && sp->pal != PALETTE_PLAYER1 && u->track < 0)
    {
        switch (sector[sp->sectnum].floorpicnum)
        {
        case 153:
        case 154:
        case 193:
        case 219:
        case 2636:
        case 2689:
        case 3561:
        case 3562:
        case 3563:
        case 3564:
            NewStateGroup(actor,sg_BunnyStand);
            break;
        default:
            sp->ang = NORM_ANGLE(RandomRange(2048 << 6) >> 6);
            u->jump_speed = -350;
            DoActorBeginJump(actor);
            u->ActorActionFunc = DoActorMoveJump;
            break;
        }
    }

    return 0;
}

int BunnySpew(DSWActor* actor)
{
    InitBloodSpray(actor, true, -1);
    return 0;
}

int DoBunnyEat(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();


    if (TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
    {
        if (TEST(u->Flags, SPR_JUMPING))
            DoActorJump(actor);
        else
            DoActorFall(actor);
    }

    // if on a player/enemy sprite jump quickly
    if (!TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
    {
        DoBunnyQuickJump(actor);
    }

    if (TEST(u->Flags, SPR_SLIDING))
        DoActorSlide(actor);

    // stay on floor unless doing certain things
    if (!TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
        KeepActorOnFloor(actor);

    DoActorSectorDamage(actor);

    switch (sector[sp->sectnum].floorpicnum)
    {
    case 153:
    case 154:
    case 193:
    case 219:
    case 2636:
    case 2689:
    case 3561:
    case 3562:
    case 3563:
    case 3564:
        if (RandomRange(1000) > 970)
            NewStateGroup(actor,sg_BunnyRun);
        break;
    default:
        NewStateGroup(actor,sg_BunnyRun);
        break;
    }
    return 0;
}

int DoBunnyScrew(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    if (TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
    {
        if (TEST(u->Flags, SPR_JUMPING))
            DoActorJump(actor);
        else
            DoActorFall(actor);
    }

    if (TEST(u->Flags, SPR_SLIDING))
        DoActorSlide(actor);

    // stay on floor unless doing certain things
    if (!TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
        KeepActorOnFloor(actor);

    DoActorSectorDamage(actor);

    if (RandomRange(1000) > 990) // Bunny sex sounds
    {
         PlaySound(DIGI_BUNNYATTACK, actor, v3df_follow);
    }

    u->WaitTics -= ACTORMOVETICS;

    if ((u->FlagOwner || u->spal == PALETTE_PLAYER0) && u->WaitTics > 0) // Keep Girl still
        NewStateGroup(actor,sg_BunnyScrew);

    if (u->spal == PALETTE_PLAYER0 && u->WaitTics <= 0) // Female has baby
    {
        u->Flag1 = SEC(5); // Count down to babies
        u->ShellNum = 1; // She's pregnant now
    }

    if (u->WaitTics <= 0)
    {
        RESET(sp->cstat, CSTAT_SPRITE_INVISIBLE); // Turn em' back on
        u->FlagOwner = 0;
        NewStateGroup(actor,sg_BunnyRun);
    }

    return 0;
}

int DoBunnyGrowUp(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    if (sp->pal == PALETTE_PLAYER1) return 0;   // Don't bother white bunnies

    if ((u->Counter -= ACTORMOVETICS) <= 0)
    {
        if ((++sp->xrepeat) > 64) sp->xrepeat = 64;
        if ((++sp->yrepeat) > 64) sp->yrepeat = 64;
        u->Counter = 60;
    }

    // Don't go homo too much!
    if (sp->pal != PALETTE_PLAYER0 && u->Flag1 > 0)
        u->Flag1 -= ACTORMOVETICS;

    // Gestation period for female rabbits
    if (sp->pal == PALETTE_PLAYER0 && u->ShellNum > 0)
    {
        if ((u->Flag1 -= ACTORMOVETICS) <= 0)
        {
            if (Bunny_Count < 20)
            {
                PlaySound(DIGI_BUNNYDIE2, actor, v3df_follow);
                BunnyHatch(actor); // Baby time
            }
            u->ShellNum = 0; // Not pregnent anymore
        }
    }

    return 0;
}


#include "saveable.h"

static saveable_code saveable_bunny_code[] =
{
    SAVE_CODE(DoBunnyBeginJumpAttack),
    SAVE_CODE(DoBunnyMoveJump),
    SAVE_CODE(DoPickCloseBunny),
    SAVE_CODE(DoBunnyQuickJump),
    SAVE_CODE(NullBunny),
    SAVE_CODE(DoBunnyPain),
    SAVE_CODE(DoBunnyRipHeart),
    SAVE_CODE(DoBunnyStandKill),
    SAVE_CODE(DoBunnyMove),
    SAVE_CODE(BunnySpew),
    SAVE_CODE(DoBunnyEat),
    SAVE_CODE(DoBunnyScrew),
    SAVE_CODE(DoBunnyGrowUp),
};

static saveable_data saveable_bunny_data[] =
{
    SAVE_DATA(BunnyBattle),
    SAVE_DATA(BunnyOffense),
    SAVE_DATA(BunnyBroadcast),
    SAVE_DATA(BunnySurprised),
    SAVE_DATA(BunnyEvasive),
    SAVE_DATA(BunnyLostTarget),
    SAVE_DATA(BunnyCloseRange),
    SAVE_DATA(BunnyWander),

    SAVE_DATA(WhiteBunnyPersonality),
    SAVE_DATA(BunnyPersonality),

    SAVE_DATA(WhiteBunnyAttrib),
    SAVE_DATA(BunnyAttrib),

    SAVE_DATA(s_BunnyRun),
    SAVE_DATA(sg_BunnyRun),
    SAVE_DATA(s_BunnyStand),
    SAVE_DATA(sg_BunnyStand),
    SAVE_DATA(s_BunnyScrew),
    SAVE_DATA(sg_BunnyScrew),
    SAVE_DATA(s_BunnySwipe),
    SAVE_DATA(sg_BunnySwipe),
    SAVE_DATA(s_BunnyHeart),
    SAVE_DATA(sg_BunnyHeart),
    SAVE_DATA(s_BunnyPain),
    SAVE_DATA(sg_BunnyPain),
    SAVE_DATA(s_BunnyJump),
    SAVE_DATA(sg_BunnyJump),
    SAVE_DATA(s_BunnyFall),
    SAVE_DATA(sg_BunnyFall),
    SAVE_DATA(s_BunnyJumpAttack),
    SAVE_DATA(sg_BunnyJumpAttack),
    SAVE_DATA(s_BunnyDie),
    SAVE_DATA(sg_BunnyDie),
    SAVE_DATA(s_BunnyDead),
    SAVE_DATA(sg_BunnyDead),
    SAVE_DATA(s_BunnyDeathJump),
    SAVE_DATA(sg_BunnyDeathJump),
    SAVE_DATA(s_BunnyDeathFall),
    SAVE_DATA(sg_BunnyDeathFall),

    SAVE_DATA(BunnyActionSet),
    SAVE_DATA(BunnyWhiteActionSet),
};

saveable_module saveable_bunny =
{
    // code
    saveable_bunny_code,
    SIZ(saveable_bunny_code),

    // data
    saveable_bunny_data,
    SIZ(saveable_bunny_data)
};
END_SW_NS
