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
#include "misc.h"
#include "tags.h"
#include "ai.h"
#include "weapon.h"

BEGIN_SW_NS

int InitSpriteGrenade(DSWActor* actor);
int InitSpriteChemBomb(DSWActor*);
int InitFlashBomb(DSWActor* actor);
int InitCaltrops(DSWActor* actor);

//////////////////////
//
// SKULL Wait
//
//////////////////////


extern DAMAGE_DATA DamageData[];
ANIMATOR DoSkullMove,DoActorDebris;

#define SKULL_RATE 10
ANIMATOR DoSkullWait;

STATE s_SkullWait[5][1] =
{
    {
        {SKULL_R0 + 0, SKULL_RATE, DoSkullWait, &s_SkullWait[0][0]},
    },
    {
        {SKULL_R1 + 0, SKULL_RATE, DoSkullWait, &s_SkullWait[1][0]},
    },
    {
        {SKULL_R2 + 0, SKULL_RATE, DoSkullWait, &s_SkullWait[2][0]},
    },
    {
        {SKULL_R3 + 0, SKULL_RATE, DoSkullWait, &s_SkullWait[3][0]},
    },
    {
        {SKULL_R4 + 0, SKULL_RATE, DoSkullWait, &s_SkullWait[4][0]},
    }
};


STATEp sg_SkullWait[] =
{
    &s_SkullWait[0][0],
    &s_SkullWait[1][0],
    &s_SkullWait[2][0],
    &s_SkullWait[3][0],
    &s_SkullWait[4][0]
};

ATTRIBUTE SkullAttrib =
{
    {60, 80, 100, 130},                 // Speeds
    {3, 0, -2, -3},                     // Tic Adjusts
    3,                                 // MaxWeapons;
    {
        DIGI_AHAMBIENT, 0, 0, 0, DIGI_AHSCREAM,
        DIGI_AHEXPLODE,0,0,0,0
    }
};

//////////////////////
//
// SKULL for Serp God
//
//////////////////////

ANIMATOR DoSerpRing;

STATE s_SkullRing[5][1] =
{
    {
        {SKULL_R0 + 0, SKULL_RATE, DoSerpRing, &s_SkullRing[0][0]},
    },
    {
        {SKULL_R1 + 0, SKULL_RATE, DoSerpRing, &s_SkullRing[1][0]},
    },
    {
        {SKULL_R2 + 0, SKULL_RATE, DoSerpRing, &s_SkullRing[2][0]},
    },
    {
        {SKULL_R3 + 0, SKULL_RATE, DoSerpRing, &s_SkullRing[3][0]},
    },
    {
        {SKULL_R4 + 0, SKULL_RATE, DoSerpRing, &s_SkullRing[4][0]},
    }
};


STATEp sg_SkullRing[] =
{
    &s_SkullRing[0][0],
    &s_SkullRing[1][0],
    &s_SkullRing[2][0],
    &s_SkullRing[3][0],
    &s_SkullRing[4][0]
};



//////////////////////
//
// SKULL Jump
//
//////////////////////

ANIMATOR DoSkullJump;

STATE s_SkullJump[5][1] =
{
    {
        {SKULL_R0 + 0, SKULL_RATE, DoSkullJump, &s_SkullJump[0][0]},
    },
    {
        {SKULL_R1 + 0, SKULL_RATE, DoSkullJump, &s_SkullJump[1][0]},
    },
    {
        {SKULL_R2 + 0, SKULL_RATE, DoSkullJump, &s_SkullJump[2][0]},
    },
    {
        {SKULL_R3 + 0, SKULL_RATE, DoSkullJump, &s_SkullJump[3][0]},
    },
    {
        {SKULL_R4 + 0, SKULL_RATE, DoSkullJump, &s_SkullJump[4][0]},
    }
};


STATEp sg_SkullJump[] =
{
    &s_SkullJump[0][0],
    &s_SkullJump[1][0],
    &s_SkullJump[2][0],
    &s_SkullJump[3][0],
    &s_SkullJump[4][0]
};


//////////////////////
//
// SKULL Explode
//
//////////////////////

#define SKULL_EXPLODE_RATE 11
ANIMATOR DoSuicide;
ANIMATOR DoSkullSpawnShrap;

STATE s_SkullExplode[] =
{
    {SKULL_EXPLODE + 0, 1,                  NullAnimator, &s_SkullExplode[1]},
    {SKULL_EXPLODE + 0, SF_QUICK_CALL,      DoDamageTest, &s_SkullExplode[2]},
    {SKULL_EXPLODE + 0, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[3]},
    {SKULL_EXPLODE + 1, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[4]},
    {SKULL_EXPLODE + 2, SF_QUICK_CALL,      DoSkullSpawnShrap, &s_SkullExplode[5]},
    {SKULL_EXPLODE + 2, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[6]},
    {SKULL_EXPLODE + 3, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[7]},
    {SKULL_EXPLODE + 4, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[8]},
    {SKULL_EXPLODE + 5, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[9]},
    {SKULL_EXPLODE + 6, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[10]},
    {SKULL_EXPLODE + 7, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[11]},
    {SKULL_EXPLODE + 8, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[12]},
    {SKULL_EXPLODE + 9, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[13]},
    {SKULL_EXPLODE +10, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[14]},
    {SKULL_EXPLODE +11, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[15]},
    {SKULL_EXPLODE +12, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[16]},
    {SKULL_EXPLODE +13, SKULL_EXPLODE_RATE, NullAnimator, &s_SkullExplode[17]},
    {SKULL_EXPLODE +13, SKULL_EXPLODE_RATE, DoSuicide,    &s_SkullExplode[17]}
};

STATEp sg_SkullExplode[] =
{
    s_SkullExplode,
};


int SetupSkull(DSWActor* actor)
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
        u = SpawnUser(actor,SKULL_R0,s_SkullWait[0]);
        u->Health = HEALTH_SKULL;
    }

    ChangeState(actor, s_SkullWait[0]);
    u->Attrib = &SkullAttrib;
    DoActorSetSpeed(actor, NORM_SPEED);
    u->StateEnd = s_SkullExplode;
    u->Rot = sg_SkullWait;

    u->ID = SKULL_R0;

    EnemyDefaults(actor, nullptr, nullptr);
    sp->clipdist = (128+64) >> 2;
    SET(u->Flags, SPR_XFLIP_TOGGLE);
    SET(sp->cstat, CSTAT_SPRITE_YCENTER);

    u->Radius = 400;

    if (SPRITEp_BOS(sp) > u->loz - Z(16))
    {
        sp->z = u->loz + Z(tileTopOffset(sp->picnum));

        u->loz = sp->z;
        // leave 8 pixels above the ground
        sp->z += SPRITEp_SIZE_TOS(sp) - Z(3);;
    }
    else
    {
        u->Counter = RANDOM_P2(2048);
        u->sz = sp->z;
    }


    return 0;
}

int DoSkullMove(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();
    int32_t dax, day, daz;

    dax = MOVEx(sp->xvel, sp->ang);
    day = MOVEy(sp->xvel, sp->ang);
    daz = sp->zvel;

    u->coll = move_missile(actor, dax, day, daz, Z(16), Z(16), CLIPMASK_MISSILE, ACTORMOVETICS);

    DoFindGroundPoint(actor);
    return 0;
}

int DoSkullBeginDeath(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();
    int16_t i,num_ord=0;

    // Decrease for Serp God
    auto own = GetOwner(actor);
    if (own != nullptr && own->hasU())
        own->u()->Counter--;

    // starts the explosion that does the actual damage

    switch (sp->hitag)
    {
    case 1:
        if (sp->lotag) num_ord = sp->lotag;
        else
            num_ord = 2;
        if (num_ord > 3) num_ord = 3;
        for (i=0; i<num_ord; i++)
        {
            sp->ang = NORM_ANGLE(sp->ang+(i*1024));
            InitSpriteChemBomb(actor);
        }
        break;

    case 2:
        if (sp->lotag) num_ord = sp->lotag;
        else
            num_ord = 5;
        if (num_ord > 10) num_ord = 10;
        for (i=0; i<num_ord; i++)
        {
            sp->ang = NORM_ANGLE(RandomRange(2048));
            InitCaltrops(actor);
        }
        break;

    case 3:
        UpdateSinglePlayKills(actor);
        InitFlashBomb(actor);
        break;

    case 4:
        if (sp->lotag) num_ord = sp->lotag;
        else
            num_ord = 5;
        if (num_ord > 10) num_ord = 10;
        for (i=0; i<num_ord; i++)
        {
            sp->ang = NORM_ANGLE(sp->ang+(i*(2048/num_ord)));
            InitSpriteGrenade(actor);
        }
        break;
    default:
        SpawnMineExp(actor);
        for (i=0; i<3; i++)
        {
            sp->ang = NORM_ANGLE(RandomRange(2048));
            InitPhosphorus(actor);
        }
        break;
    }

    RESET(sp->cstat, CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN);
    u->RotNum = 0;
    u->Tics = 0;
    u->ID = SKULL_R0;
    u->Radius = DamageData[DMG_SKULL_EXP].radius; //*DamageRadiusSkull;
    u->OverlapZ = Z(64);
    change_actor_stat(actor, STAT_DEAD_ACTOR);
    sp->shade = -40;

    SpawnLittleExp(actor);
    SetSuicide(actor);
    return 0;
}


int DoSkullJump(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    if (sp->xvel)
        DoSkullMove(actor);
    else
        sp->ang = NORM_ANGLE(sp->ang + (64 * ACTORMOVETICS));

    if (TEST(u->Flags,SPR_JUMPING))
    {
        DoJump(actor);
    }
    else if (TEST(u->Flags,SPR_FALLING))
    {
        DoFall(actor);

        // jump/fall type
        if (sp->xvel)
        {

            int dist,a,b,c;

            DISTANCE(sp->x, sp->y, u->targetActor->s().x, u->targetActor->s().y, dist, a, b, c);

            if (dist < 1000 &&
                SpriteOverlapZ(actor, u->targetActor, Z(32)))
            {
                UpdateSinglePlayKills(actor);
                DoSkullBeginDeath(actor);
                return 0;
            }

            if ((sp->z > u->loz - Z(36)))
            {
                sp->z = u->loz - Z(36);
                UpdateSinglePlayKills(actor);
                DoSkullBeginDeath(actor);
                return 0;
            }
        }
        // non jumping type
        else
        {
            if (u->jump_speed > 200)
            {
                UpdateSinglePlayKills(actor);
                DoSkullBeginDeath(actor);
            }
        }

    }
    else
    {
        UpdateSinglePlayKills(actor);
        DoSkullBeginDeath(actor);
    }

    return 0;
}

int DoSkullBob(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    // actor does a sine wave about u->sz - this is the z mid point
    const int SKULL_BOB_AMT = (Z(16));

    u->Counter = (u->Counter + (ACTORMOVETICS << 3) + (ACTORMOVETICS << 1)) & 2047;
    sp->z = u->sz + MulScale(SKULL_BOB_AMT, bsin(u->Counter), 14) +
            MulScale(DIV2(SKULL_BOB_AMT), bsin(u->Counter), 14);

    return 0;
}

int DoSkullSpawnShrap(DSWActor* actor)
{
    SpawnShrap(actor, nullptr);

    //PlaySpriteSound(actor,attr_extra1,v3df_none);
    return 0;
}

int DoSkullWait(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();
    int a,b,c,dist;

    DISTANCE(sp->x, sp->y, u->targetActor->s().x, u->targetActor->s().y, dist, a, b, c);

    DoActorPickClosePlayer(actor);

    //if (dist < u->active_range)
    //    return(0);

    if ((u->WaitTics -= ACTORMOVETICS) <= 0)
    {
        PlaySound(DIGI_AHSCREAM, actor, v3df_none);
        u->WaitTics = SEC(3) + RandomRange(360);
    }

    // below the floor type
    if (sp->z > u->loz)
    {
        // look for closest player every once in a while
        if (dist < 3500)
        {
            sp->xvel = 0;
            u->jump_speed = -600;
            NewStateGroup(actor, sg_SkullJump);
            DoBeginJump(actor);
        }
    }
    else
    // above the floor type
    {
        sp->ang = NORM_ANGLE(sp->ang + (48 * ACTORMOVETICS));

        DoSkullBob(actor);

        if (dist < 8000)
        {
            sp->ang = getangle(u->targetActor->s().x - sp->x, u->targetActor->s().y - sp->y);
            sp->xvel = 128 + (RANDOM_P2(256<<8)>>8);
            u->jump_speed = -700;
            NewStateGroup(actor, sg_SkullJump);
            DoBeginJump(actor);
        }
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////
//
// BETTY Wait
//
//////////////////////


ANIMATOR DoBettyMove,DoActorDebris;

#define BETTY_RATE 10
ANIMATOR DoBettyWait;

STATE s_BettyWait[5][3] =
{
    {
        {BETTY_R0 + 0, BETTY_RATE, DoBettyWait, &s_BettyWait[0][1]},
        {BETTY_R0 + 1, BETTY_RATE, DoBettyWait, &s_BettyWait[0][2]},
        {BETTY_R0 + 2, BETTY_RATE, DoBettyWait, &s_BettyWait[0][0]},
    },
    {
        {BETTY_R1 + 0, BETTY_RATE, DoBettyWait, &s_BettyWait[1][1]},
        {BETTY_R1 + 1, BETTY_RATE, DoBettyWait, &s_BettyWait[1][2]},
        {BETTY_R1 + 2, BETTY_RATE, DoBettyWait, &s_BettyWait[1][0]},
    },
    {
        {BETTY_R2 + 0, BETTY_RATE, DoBettyWait, &s_BettyWait[2][1]},
        {BETTY_R2 + 1, BETTY_RATE, DoBettyWait, &s_BettyWait[2][2]},
        {BETTY_R2 + 2, BETTY_RATE, DoBettyWait, &s_BettyWait[2][0]},
    },
    {
        {BETTY_R3 + 0, BETTY_RATE, DoBettyWait, &s_BettyWait[3][1]},
        {BETTY_R3 + 1, BETTY_RATE, DoBettyWait, &s_BettyWait[3][2]},
        {BETTY_R3 + 2, BETTY_RATE, DoBettyWait, &s_BettyWait[3][0]},
    },
    {
        {BETTY_R4 + 0, BETTY_RATE, DoBettyWait, &s_BettyWait[4][1]},
        {BETTY_R4 + 1, BETTY_RATE, DoBettyWait, &s_BettyWait[4][2]},
        {BETTY_R4 + 2, BETTY_RATE, DoBettyWait, &s_BettyWait[4][0]},
    }
};


STATEp sg_BettyWait[] =
{
    &s_BettyWait[0][0],
    &s_BettyWait[1][0],
    &s_BettyWait[2][0],
    &s_BettyWait[3][0],
    &s_BettyWait[4][0]
};

ATTRIBUTE BettyAttrib =
{
    {60, 80, 100, 130},                 // Speeds
    {3, 0, -2, -3},                     // Tic Adjusts
    3,                                 // MaxWeapons;
    {0, 0, 0, 0, 0, 0,0,0,0,0}
};

//////////////////////
//
// BETTY Jump
//
//////////////////////

ANIMATOR DoBettyJump;

STATE s_BettyJump[5][1] =
{
    {
        {BETTY_R0 + 0, BETTY_RATE, DoBettyJump, &s_BettyJump[0][0]},
    },
    {
        {BETTY_R1 + 0, BETTY_RATE, DoBettyJump, &s_BettyJump[1][0]},
    },
    {
        {BETTY_R2 + 0, BETTY_RATE, DoBettyJump, &s_BettyJump[2][0]},
    },
    {
        {BETTY_R3 + 0, BETTY_RATE, DoBettyJump, &s_BettyJump[3][0]},
    },
    {
        {BETTY_R4 + 0, BETTY_RATE, DoBettyJump, &s_BettyJump[4][0]},
    }
};


STATEp sg_BettyJump[] =
{
    &s_BettyJump[0][0],
    &s_BettyJump[1][0],
    &s_BettyJump[2][0],
    &s_BettyJump[3][0],
    &s_BettyJump[4][0]
};


//////////////////////
//
// BETTY Explode
//
//////////////////////

#define BETTY_EXPLODE_RATE 11
#define BETTY_EXPLODE BETTY_R0
ANIMATOR DoSuicide;
ANIMATOR DoBettySpawnShrap;

STATE s_BettyExplode[] =
{
    {BETTY_EXPLODE + 0, SF_QUICK_CALL,      DoDamageTest, &s_BettyExplode[1]},
    {BETTY_EXPLODE + 0, BETTY_EXPLODE_RATE, DoSuicide, &s_BettyExplode[0]}
};

STATEp sg_BettyExplode[] =
{
    s_BettyExplode,
};


int SetupBetty(DSWActor* actor)
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
        u = SpawnUser(actor,BETTY_R0,s_BettyWait[0]);
        u->Health = HEALTH_SKULL;
    }

    ChangeState(actor, s_BettyWait[0]);
    u->Attrib = &BettyAttrib;
    DoActorSetSpeed(actor, NORM_SPEED);
    u->StateEnd = s_BettyExplode;
    u->Rot = sg_BettyWait;

    u->ID = BETTY_R0;

    EnemyDefaults(actor, nullptr, nullptr);
    sp->clipdist = (128+64) >> 2;
    SET(u->Flags, SPR_XFLIP_TOGGLE);
    SET(sp->cstat, CSTAT_SPRITE_YCENTER);

    u->Radius = 400;

    if (SPRITEp_BOS(sp) > u->loz - Z(16))
    {
        sp->z = u->loz + Z(tileTopOffset(sp->picnum));

        u->loz = sp->z;
        // leave 8 pixels above the ground
        sp->z += SPRITEp_SIZE_TOS(sp) - Z(3);;
    }
    else
    {
        u->Counter = RANDOM_P2(2048);
        u->sz = sp->z;
    }


    return 0;
}

int DoBettyMove(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();
    int32_t dax, day, daz;

    dax = MOVEx(sp->xvel, sp->ang);
    day = MOVEy(sp->xvel, sp->ang);
    daz = sp->zvel;

    u->coll = move_missile(actor, dax, day, daz, Z(16), Z(16), CLIPMASK_MISSILE, ACTORMOVETICS);

    DoFindGroundPoint(actor);
    return 0;
}

int DoBettyBeginDeath(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();
    int16_t i,num_ord=0;

    // starts the explosion that does the actual damage

    switch (sp->hitag)
    {
    case 1:
        if (sp->lotag) num_ord = sp->lotag;
        else
            num_ord = 2;
        if (num_ord > 3) num_ord = 3;
        for (i=0; i<num_ord; i++)
        {
            sp->ang = NORM_ANGLE(sp->ang+(i*1024));
            InitSpriteChemBomb(actor);
        }
        break;

    case 2:
        if (sp->lotag) num_ord = sp->lotag;
        else
            num_ord = 5;
        if (num_ord > 10) num_ord = 10;
        for (i=0; i<num_ord; i++)
        {
            sp->ang = NORM_ANGLE(RandomRange(2048));
            InitCaltrops(actor);
        }
        break;

    case 3:
        InitFlashBomb(actor);
        break;

    case 4:
        if (sp->lotag) num_ord = sp->lotag;
        else
            num_ord = 5;
        if (num_ord > 10) num_ord = 10;
        for (i=0; i<num_ord; i++)
        {
            sp->ang = NORM_ANGLE(sp->ang + (i*(2048/num_ord)));
            InitSpriteGrenade(actor);
        }
        break;
    default:
        for (i=0; i<5; i++)
        {
            sp->ang = NORM_ANGLE(RandomRange(2048));
            InitPhosphorus(actor);
            SpawnMineExp(actor);
        }
        break;
    }

    RESET(sp->cstat, CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN);
    u->RotNum = 0;
    u->Tics = 0;
    u->ID = BETTY_R0;
    u->Radius = DamageData[DMG_SKULL_EXP].radius; //*DamageRadiusBetty;
    u->OverlapZ = Z(64);
    change_actor_stat(actor, STAT_DEAD_ACTOR);
    sp->shade = -40;

    SpawnLittleExp(actor);
    SetSuicide(actor);
    return 0;
}


int DoBettyJump(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    if (sp->xvel)
        DoBettyMove(actor);
    else
        sp->ang = NORM_ANGLE(sp->ang + (64 * ACTORMOVETICS));

    if (TEST(u->Flags,SPR_JUMPING))
    {
        DoJump(actor);
    }
    else if (TEST(u->Flags,SPR_FALLING))
    {
        DoFall(actor);

        // jump/fall type
        if (sp->xvel)
        {
            int dist,a,b,c;

            DISTANCE(sp->x, sp->y, u->targetActor->s().x, u->targetActor->s().y, dist, a, b, c);

            if (dist < 1000 &&
                SpriteOverlapZ(actor, u->targetActor, Z(32)))
            {
                UpdateSinglePlayKills(actor);
                DoBettyBeginDeath(actor);
                return 0;
            }

            if ((sp->z > u->loz - Z(36)))
            {
                sp->z = u->loz - Z(36);
                UpdateSinglePlayKills(actor);
                DoBettyBeginDeath(actor);
                return 0;
            }
        }
        // non jumping type
        else
        {
            if (u->jump_speed > 200)
            {
                UpdateSinglePlayKills(actor);
                DoBettyBeginDeath(actor);
            }
        }

    }
    else
    {
        UpdateSinglePlayKills(actor);
        DoBettyBeginDeath(actor);
    }
    return 0;
}

int DoBettyBob(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    // actor does a sine wave about u->sz - this is the z mid point
    const int  BETTY_BOB_AMT = (Z(16));

    u->Counter = (u->Counter + (ACTORMOVETICS << 3) + (ACTORMOVETICS << 1)) & 2047;
    sp->z = u->sz + MulScale(BETTY_BOB_AMT, bsin(u->Counter), 14) +
            MulScale(DIV2(BETTY_BOB_AMT), bsin(u->Counter), 14);

    return 0;
}

int DoBettySpawnShrap(DSWActor* actor)
{
    SpawnShrap(actor, nullptr);
    //PlaySpriteSound(actor,attr_extra1,v3df_none);
    return 0;
}

int DoBettyWait(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();
    int a,b,c,dist;

    DISTANCE(sp->x, sp->y, u->targetActor->s().x, u->targetActor->s().y, dist, a, b, c);

    DoActorPickClosePlayer(actor);

    //if (dist < u->active_range)
    //    return(0);

    if ((u->WaitTics -= ACTORMOVETICS) <= 0)
    {
        PlaySound(DIGI_MINEBEEP, actor, v3df_none);
        u->WaitTics = SEC(3);
    }

    // below the floor type
    if (sp->z > u->loz)
    {
        // look for closest player every once in a while
        if (dist < 3500)
        {
            sp->xvel = 0;
            u->jump_speed = -600;
            NewStateGroup(actor, sg_BettyJump);
            DoBeginJump(actor);
        }
    }
    else
    // above the floor type
    {
        sp->ang = NORM_ANGLE(sp->ang + (48 * ACTORMOVETICS));

        DoBettyBob(actor);

        if (dist < 8000)
        {
            sp->ang = getangle(u->targetActor->s().x - sp->x, u->targetActor->s().y - sp->y);
            sp->xvel = 128 + (RANDOM_P2(256<<8)>>8);
            u->jump_speed = -700;
            NewStateGroup(actor, sg_BettyJump);
            DoBeginJump(actor);
        }
    }

    return 0;
}


#include "saveable.h"

static saveable_code saveable_skull_code[] =
{
    SAVE_CODE(DoSkullMove),
    SAVE_CODE(DoSkullBeginDeath),
    SAVE_CODE(DoSkullJump),
    SAVE_CODE(DoSkullBob),
    SAVE_CODE(DoSkullSpawnShrap),
    SAVE_CODE(DoSkullWait),

    SAVE_CODE(DoBettyMove),
    SAVE_CODE(DoBettyBeginDeath),
    SAVE_CODE(DoBettyJump),
    SAVE_CODE(DoBettyBob),
    SAVE_CODE(DoBettySpawnShrap),
    SAVE_CODE(DoBettyWait),
};

static saveable_data saveable_skull_data[] =
{
    SAVE_DATA(s_SkullWait),
    SAVE_DATA(sg_SkullWait),

    SAVE_DATA(SkullAttrib),

    SAVE_DATA(s_SkullRing),
    SAVE_DATA(sg_SkullRing),
    SAVE_DATA(s_SkullJump),
    SAVE_DATA(sg_SkullJump),
    SAVE_DATA(s_SkullExplode),
    SAVE_DATA(sg_SkullExplode),
    SAVE_DATA(s_BettyWait),
    SAVE_DATA(sg_BettyWait),

    SAVE_DATA(BettyAttrib),

    SAVE_DATA(s_BettyJump),
    SAVE_DATA(sg_BettyJump),
    SAVE_DATA(s_BettyExplode),
    SAVE_DATA(sg_BettyExplode),
};

saveable_module saveable_skull =
{
    // code
    saveable_skull_code,
    SIZ(saveable_skull_code),

    // data
    saveable_skull_data,
    SIZ(saveable_skull_data)
};
END_SW_NS
