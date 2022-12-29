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
// Added Ninja Sliced fix
// Fixed Ninja sliced dead and rotation
// Added s_NinjaDieSlicedHack[]
//

#include "build.h"

#include "names2.h"
#include "panel.h"
#include "misc.h"
#include "tags.h"
#include "weapon.h"
#include "sprite.h"
#include "gamefuncs.h"

BEGIN_SW_NS

DSWActor swActors[MAXSPRITES];

extern int jump_grav;

extern STATE s_DebrisNinja[];
extern STATE s_DebrisRat[];
extern STATE s_DebrisCrab[];
extern STATE s_DebrisStarFish[];
extern STATE s_NinjaDieSliced[];
extern STATE s_NinjaDieSlicedHack[];

extern STATEp sg_NinjaGrabThroat[];



int DoScaleSprite(DSWActor* actor)
{
    auto u = actor->u();
    auto sp = &actor->s();
    int scale_value;

    if (u->scale_speed)
    {
        u->scale_value += u->scale_speed * ACTORMOVETICS;

        scale_value = u->scale_value >> 8;

        if (u->scale_speed > 0)
        {
            if (scale_value > u->scale_tgt)
                u->scale_speed = 0;
            else
                sp->xrepeat = sp->yrepeat = scale_value;
        }
        else
        {
            if (scale_value < u->scale_tgt)
                u->scale_speed = 0;
            else
                sp->xrepeat = sp->yrepeat = scale_value;
        }

    }

    return 0;
}

int DoActorDie(DSWActor* actor, DSWActor* weapActor, int meansofdeath)
{
    auto u = actor->u();
    auto sp = &actor->s();


    change_actor_stat(actor, STAT_DEAD_ACTOR);
    SET(u->Flags, SPR_DEAD);
    RESET(u->Flags, SPR_FALLING | SPR_JUMPING);
    u->floor_dist = Z(40);

    // test for gibable dead bodies
    SET(sp->extra, SPRX_BREAKABLE);
    SET(sp->cstat, CSTAT_SPRITE_BREAKABLE);

    if (weapActor == nullptr)
    {
        // killed by one of these non-sprites
        switch (meansofdeath)
        {
        case WPN_NM_LAVA:
            ChangeState(actor, u->StateEnd);
            u->RotNum = 0;
            break;

        case WPN_NM_SECTOR_SQUISH:
            ChangeState(actor, u->StateEnd);
            u->RotNum = 0;
            break;
        }

        return 0;
    }

    if (!weapActor->hasU()) return 0;
    auto wu = weapActor->u();
    auto wp = &weapActor->s();

    // killed by one of these sprites
    switch (wu->ID)
    {
    // Coolie actually explodes himself
    // he is the Sprite AND weapon
    case COOLIE_RUN_R0:
        ChangeState(actor, u->StateEnd);
        u->RotNum = 0;
        sp->xvel <<= 1;
        u->ActorActionFunc = nullptr;
        sp->ang = NORM_ANGLE(sp->ang + 1024);
        break;

    case NINJA_RUN_R0:
        if (u->ID == NINJA_RUN_R0) // Cut in half!
        {
            if (wu->WeaponNum != WPN_FIST)
            {
                if (sw_ninjahack)
                    SpawnBlood(actor, actor, -1, -1, -1, -1);
                InitPlasmaFountain(weapActor, actor);
                InitPlasmaFountain(weapActor, actor);
                PlaySound(DIGI_NINJAINHALF, actor, v3df_none);
                if (sw_ninjahack)
                    ChangeState(actor, &s_NinjaDieSlicedHack[5]);
                else
                    ChangeState(actor, &s_NinjaDieSliced[0]);
            }
            else
            {
                if (RandomRange(1000) > 500)
                {
                    InitPlasmaFountain(weapActor, actor);
                }

                ChangeState(actor, u->StateEnd);
                u->RotNum = 0;
                u->ActorActionFunc = nullptr;
                sp->xvel = 200 + RandomRange(200);
                u->jump_speed = -200 - RandomRange(250);
                DoActorBeginJump(actor);
                sp->ang = wp->ang;
            }
        }
        else
        {
            // test for gibable dead bodies
            if (RandomRange(1000) > 500)
                SET(sp->cstat, CSTAT_SPRITE_YFLIP);
            ChangeState(actor, u->StateEnd);
            sp->xvel = 0;
            u->jump_speed = 0;
            DoActorBeginJump(actor);
        }

        u->RotNum = 0;

        u->ActorActionFunc = nullptr;
        //u->ActorActionFunc = NullAnimator;
        if (!sw_ninjahack)
            sp->ang = wp->ang;
        break;

    case COOLG_RUN_R0:
    case SKEL_RUN_R0:
    case RIPPER_RUN_R0:
    case RIPPER2_RUN_R0:
    case EEL_RUN_R0:
    case STAR1:
    case SUMO_RUN_R0:
        ChangeState(actor, u->StateEnd);
        u->RotNum = 0;
        break;

    case UZI_SMOKE:
        if (RandomRange(1000) > 500)
            SET(sp->cstat, CSTAT_SPRITE_YFLIP);
        ChangeState(actor, u->StateEnd);
        u->RotNum = 0;
        // Rippers still gotta jump or they fall off walls weird
        if (u->ID == RIPPER_RUN_R0 || u->ID == RIPPER2_RUN_R0)
        {
            sp->xvel <<= 1;
            u->jump_speed = -100 - RandomRange(250);
            DoActorBeginJump(actor);
        }
        else
        {
            sp->xvel = 0;
            u->jump_speed = -10 - RandomRange(25);
            DoActorBeginJump(actor);
        }
        u->ActorActionFunc = nullptr;
        // Get angle to player
        sp->ang = NORM_ANGLE(getangle(u->targetActor->s().x - sp->x, u->targetActor->s().y - sp->y) + 1024);
        break;

    case UZI_SMOKE+1: // Shotgun
        if (RandomRange(1000) > 500)
            SET(sp->cstat, CSTAT_SPRITE_YFLIP);
        ChangeState(actor, u->StateEnd);
        u->RotNum = 0;

        // Rippers still gotta jump or they fall off walls weird
        if (u->ID == RIPPER_RUN_R0 || u->ID == RIPPER2_RUN_R0)
        {
            sp->xvel = 75 + RandomRange(100);
            u->jump_speed = -100 - RandomRange(150);
        }
        else
        {
            sp->xvel = 100 + RandomRange(200);
            u->jump_speed = -100 - RandomRange(250);
        }
        DoActorBeginJump(actor);
        u->ActorActionFunc = nullptr;
        // Get angle to player
        sp->ang = NORM_ANGLE(getangle(u->targetActor->s().x - sp->x, u->targetActor->s().y - sp->y) + 1024);
        break;

    default:
        switch (u->ID)
        {
        case SKULL_R0:
        case BETTY_R0:
            ChangeState(actor, u->StateEnd);
            break;

        default:
            if (RandomRange(1000) > 700)
            {
                InitPlasmaFountain(weapActor, actor);
            }

            if (RandomRange(1000) > 500)
                SET(sp->cstat, CSTAT_SPRITE_YFLIP);
            ChangeState(actor, u->StateEnd);
            u->RotNum = 0;
            u->ActorActionFunc = nullptr;
            sp->xvel = 300 + RandomRange(400);
            u->jump_speed = -300 - RandomRange(350);
            DoActorBeginJump(actor);
            sp->ang = wp->ang;
            break;
        }
        break;
    }

    // These are too big to flip upside down
    switch (u->ID)
    {
    case RIPPER2_RUN_R0:
    case COOLIE_RUN_R0:
    case SUMO_RUN_R0:
    case ZILLA_RUN_R0:
        RESET(sp->cstat, CSTAT_SPRITE_YFLIP);
        break;
    }

    u->ID = 0;

    return 0;
}

void DoDebrisCurrent(DSWActor* actor)
{
    int nx, ny;
    USERp u = actor->u();
    auto sp = &actor->s();
    SECT_USERp sectu = SectUser[sp->sectnum].Data();

    //sp->clipdist = (256+128)>>2;

    nx = MulScale(DIV4(sectu->speed), bcos(sectu->ang), 14);
    ny = MulScale(DIV4(sectu->speed), bsin(sectu->ang), 14);

    Collision ret = move_sprite(actor, nx, ny, 0, u->ceiling_dist, u->floor_dist, 0, ACTORMOVETICS);

    // attempt to move away from wall
    if (ret.type != kHitNone)
    {
        short rang = RANDOM_P2(2048);

        nx = MulScale(DIV4(sectu->speed), bcos(sectu->ang + rang), 14);
        nx = MulScale(DIV4(sectu->speed), bsin(sectu->ang + rang), 14);

        move_sprite(actor, nx, ny, 0, u->ceiling_dist, u->floor_dist, 0, ACTORMOVETICS);
    }

    sp->z = u->loz;
}

int DoActorSectorDamage(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();
    SECT_USERp sectu = SectUser[sp->sectnum].Data();
    SECTORp sectp = &sector[sp->sectnum];

    if (u->Health <= 0)
        return false;

    if (sectu && sectu->damage)
    {
        if (TEST(sectu->flags, SECTFU_DAMAGE_ABOVE_SECTOR))
        {
            if ((u->DamageTics -= synctics) < 0)
            {
                u->DamageTics = 60;
                u->Health -= sectu->damage;

                if (u->Health <= 0)
                {
                    UpdateSinglePlayKills(actor);
                    DoActorDie(actor, nullptr, WPN_NM_LAVA);
                    return true;
                }
            }
        }
        else if (SPRITEp_BOS(sp) >= sectp->floorz)
        {
            if ((u->DamageTics -= synctics) < 0)
            {
                u->DamageTics = 60;
                u->Health -= sectu->damage;

                if (u->Health <= 0)
                {
                    UpdateSinglePlayKills(actor);
                    DoActorDie(actor, nullptr, WPN_NM_LAVA);
                    return true;
                }
            }
        }
    }

    // note that most squishing is done in vator.c
    if (u->lo_sectp && u->hi_sectp && labs(u->loz - u->hiz) < DIV2(SPRITEp_SIZE_Z(sp)))
    //if (u->lo_sectp && u->hi_sectp && labs(u->loz - u->hiz) < SPRITEp_SIZE_Z(sp))
    {
        u->Health = 0;
        if (SpawnShrap(actor, nullptr, WPN_NM_SECTOR_SQUISH))
        {
            UpdateSinglePlayKills(actor);
            SetSuicide(actor);
        }
        else
        {
            ASSERT(true == false);
            //DoActorDie(actor, nullptr, WPN_NM_SECTOR_SQUISH);
        }

        return true;
    }

    return false;
}


bool move_debris(DSWActor* actor, int xchange, int ychange, int zchange)
{
    USERp u = actor->u();

    u->coll = move_sprite(actor, xchange, ychange, zchange,
                         u->ceiling_dist, u->floor_dist, 0, ACTORMOVETICS);

    return u->coll.type == kHitNone;
}

// !AIC - Supposed to allow floating of DEBRIS (dead bodies, flotsam, jetsam).  Or if water has
// current move with the current.

int DoActorDebris(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();
    SECTORp sectp = &sector[sp->sectnum];
    int nx, ny;

    // This was move from DoActorDie so actor's can't be walked through until they are on the floor
    RESET(sp->cstat, CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN);

    // Don't let some actors float
    switch (u->ID)
    {
    case HORNET_RUN_R0:
    case BUNNY_RUN_R0:
        KillActor(actor);
        return 0;
    case ZILLA_RUN_R0:
        getzsofslope(sp->sectnum, sp->x, sp->y, &u->hiz, &u->loz);
        u->lo_sectp = &sector[sp->sectnum];
        u->hi_sectp = &sector[sp->sectnum];
        u->lowActor = nullptr;
        u->highActor = nullptr;
        break;
    }

    if (TEST(sectp->extra, SECTFX_SINK))
    {
        if (TEST(sectp->extra, SECTFX_CURRENT))
        {
            DoDebrisCurrent(actor);
        }
        else
        {
            //nx = sp->xvel * ACTORMOVETICS * bcos(sp->ang) >> 14;
            //ny = sp->xvel * ACTORMOVETICS * bsin(sp->ang) >> 14;
            nx = MulScale(ACTORMOVETICS, bcos(sp->ang), 14);
            ny = MulScale(ACTORMOVETICS, bsin(sp->ang), 14);

            //sp->clipdist = (256+128)>>2;

            if (!move_debris(actor, nx, ny, 0L))
            {
                sp->ang = RANDOM_P2(2048);
            }
        }

        if (SectUser[sp->sectnum].Data() && FixedToInt(SectUser[sp->sectnum]->depth_fixed) > 10) // JBF: added null check
        {
            u->WaitTics = (u->WaitTics + (ACTORMOVETICS << 3)) & 1023;
            //sp->z = Z(2) + u->loz + ((Z(4) * (int) bsin(u->WaitTics)) >> 14);
            sp->z = u->loz - MulScale(Z(2), bsin(u->WaitTics), 14);
        }
    }
    else
    {
        sp->z = u->loz;
    }

    return 0;
}


int DoFireFly(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();
    int nx, ny;

    nx = 4 * ACTORMOVETICS * bcos(sp->ang) >> 14;
    ny = 4 * ACTORMOVETICS * bsin(sp->ang) >> 14;

    sp->clipdist = 256>>2;
    if (!move_actor(actor, nx, ny, 0L))
    {
        sp->ang = NORM_ANGLE(sp->ang + 1024);
    }

    u->WaitTics = (u->WaitTics + (ACTORMOVETICS << 1)) & 2047;

    sp->z = u->sz + MulScale(Z(32), bsin(u->WaitTics), 14);
    return 0;
}

int DoGenerateSewerDebris(DSWActor* actor)
{
    SPRITEp sp = &actor->s();
    USERp u = actor->u();

    static STATEp Debris[] =
    {
        s_DebrisNinja,
        s_DebrisRat,
        s_DebrisCrab,
        s_DebrisStarFish
    };

    u->Tics -= ACTORMOVETICS;

    if (u->Tics <= 0)
    {
        u->Tics = u->WaitTics;

        auto spawned = SpawnActor(STAT_DEAD_ACTOR, 0, Debris[RANDOM_P2(4<<8)>>8], sp->sectnum, sp->x, sp->y, sp->z, sp->ang, 200);

        SetOwner(actor, spawned);
    }

    return 0;
}

// !AIC - Tries to keep actors correctly on the floor.  More that a bit messy.

void KeepActorOnFloor(DSWActor* actor)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();
    SECTORp sectp;
    int depth;

    sectp = &sector[sp->sectnum];

    RESET(sp->cstat, CSTAT_SPRITE_YFLIP); // If upside down, reset it

    if (TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
        return;

    if (u->lo_sectp && SectUser[sectnum(u->lo_sectp)].Data())
        depth = FixedToInt(SectUser[sectnum(u->lo_sectp)]->depth_fixed);
    else
        depth = 0;

    if (TEST(sectp->extra, SECTFX_SINK) &&
        depth > 35 &&
        u->ActorActionSet && u->ActorActionSet->Swim)
    {
        if (TEST(u->Flags, SPR_SWIMMING))
        {
            if (u->Rot != u->ActorActionSet->Run && u->Rot != u->ActorActionSet->Swim && u->Rot != u->ActorActionSet->Stand)
            {
                // was swimming but have now stopped
                RESET(u->Flags, SPR_SWIMMING);
                RESET(sp->cstat, CSTAT_SPRITE_YCENTER);
                u->oz = sp->z = u->loz;
                sp->backupz();
                return;
            }

            if (u->Rot == u->ActorActionSet->Run)
            {
                NewStateGroup(actor, u->ActorActionSet->Swim);
            }

            // are swimming
            u->oz = sp->z = u->loz - Z(depth);
            sp->backupz();
        }
        else
        {
            // only start swimming if you are running
            if (u->Rot == u->ActorActionSet->Run || u->Rot == u->ActorActionSet->Swim)
            {
                NewStateGroup(actor, u->ActorActionSet->Swim);
                u->oz = sp->z = u->loz - Z(depth);
                sp->backupz();
                SET(u->Flags, SPR_SWIMMING);
                SET(sp->cstat, CSTAT_SPRITE_YCENTER);
            }
            else
            {
                RESET(u->Flags, SPR_SWIMMING);
                RESET(sp->cstat, CSTAT_SPRITE_YCENTER);
                u->oz = sp->z = u->loz;
                sp->backupz();
            }
        }

        return;
    }

    // NOT in a swimming situation
    RESET(u->Flags, SPR_SWIMMING);
    RESET(sp->cstat, CSTAT_SPRITE_YCENTER);

#if 1
    if (TEST(u->Flags, SPR_MOVED))
    {
        u->oz = sp->z = u->loz;
        sp->backupz();
    }
    else
    {
        int ceilz, florz;
        Collision ctrash, ftrash;
        FAFgetzrangepoint(sp->x, sp->y, sp->z, sp->sectnum,
                          &ceilz, &ctrash, &florz, &ftrash);

        u->oz = sp->z = florz;
        sp->backupz();
    }
#endif


}

int DoActorBeginSlide(DSWActor* actor, short ang, short vel, short dec)
{
    USERp u = actor->u();

    SET(u->Flags, SPR_SLIDING);

    u->slide_ang = ang;
    u->slide_vel = vel;
    u->slide_dec = dec;

    //DoActorSlide(actor);

    return 0;
}

// !AIC - Sliding can occur in different directions from movement of the actor.
// Has its own set of variables

int DoActorSlide(DSWActor* actor)
{
    USER* u = actor->u();
    int nx, ny;

    nx = MulScale(u->slide_vel, bcos(u->slide_ang), 14);
    ny = MulScale(u->slide_vel, bsin(u->slide_ang), 14);

    if (!move_actor(actor, nx, ny, 0L))
    {
        RESET(u->Flags, SPR_SLIDING);
        return false;
    }

    u->slide_vel -= u->slide_dec * ACTORMOVETICS;

    if (u->slide_vel < 20)
    {
        RESET(u->Flags, SPR_SLIDING);
    }

    return true;
}

// !AIC - Actor jumping and falling

int DoActorBeginJump(DSWActor* actor)
{
    USER* u = actor->u();

    SET(u->Flags, SPR_JUMPING);
    RESET(u->Flags, SPR_FALLING);

    // u->jump_speed = should be set before calling

    // set up individual actor jump gravity
    u->jump_grav = ACTOR_GRAVITY;

    // Change sprites state to jumping
    if (u->ActorActionSet)
    {
        if (TEST(u->Flags, SPR_DEAD))
            NewStateGroup(actor, u->ActorActionSet->DeathJump);
        else
            NewStateGroup(actor, u->ActorActionSet->Jump);
    }
    u->StateFallOverride = nullptr;

    //DO NOT CALL DoActorJump! DoActorStopFall can cause an infinite loop and
    //stack overflow if it is called.
    //DoActorJump(actor);

    return 0;
}

int DoActorJump(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    int jump_adj;

    // precalculate jump value to adjust jump speed by
    jump_adj = u->jump_grav * ACTORMOVETICS;

    // adjust jump speed by gravity - if jump speed greater than 0 player
    // have started falling
    if ((u->jump_speed += jump_adj) > 0)
    {
        // Start falling
        DoActorBeginFall(actor);
        return 0;
    }

    // adjust height by jump speed
    sp->z += u->jump_speed * ACTORMOVETICS;

    // if player gets to close the ceiling while jumping
    if (sp->z < u->hiz + Z(PIC_SIZY(sp)))
    {
        // put player at the ceiling
        sp->z = u->hiz + Z(PIC_SIZY(sp));

        // reverse your speed to falling
        u->jump_speed = -u->jump_speed;

        // Change sprites state to falling
        DoActorBeginFall(actor);
    }

    return 0;
}


int DoActorBeginFall(DSWActor* actor)
{
    USER* u = actor->u();
    SET(u->Flags, SPR_FALLING);
    RESET(u->Flags, SPR_JUMPING);

    u->jump_grav = ACTOR_GRAVITY;

    // Change sprites state to falling
    if (u->ActorActionSet)
    {
        if (TEST(u->Flags, SPR_DEAD))
        {
            NewStateGroup(actor, u->ActorActionSet->DeathFall);
        }
        else
            NewStateGroup(actor, u->ActorActionSet->Fall);

        if (u->StateFallOverride)
        {
            NewStateGroup(actor, u->StateFallOverride);
        }
    }

    DoActorFall(actor);

    return 0;
}


int DoActorFall(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    // adjust jump speed by gravity
    u->jump_speed += u->jump_grav * ACTORMOVETICS;

    // adjust player height by jump speed
    sp->z += u->jump_speed * ACTORMOVETICS;

    // Stick like glue when you hit the ground
    if (sp->z > u->loz)
    {
        DoActorStopFall(actor);
    }

    return 0;
}

int DoActorStopFall(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    sp->z = u->loz;

    RESET(u->Flags, SPR_FALLING | SPR_JUMPING);
    RESET(sp->cstat, CSTAT_SPRITE_YFLIP);


    // don't stand on face or wall sprites - jump again
    if (u->lowActor && !TEST(u->lowActor->s().cstat, CSTAT_SPRITE_ALIGNMENT_FLOOR))
    {
        //sp->ang = NORM_ANGLE(sp->ang + (RANDOM_P2(64<<8)>>8) - 32);
        sp->ang = NORM_ANGLE(sp->ang + 1024 + (RANDOM_P2(512<<8)>>8));
        u->jump_speed = -350;

        DoActorBeginJump(actor);
        return 0;
    }

    // Change sprites state to running
    if (u->ActorActionSet)
    {
        if (TEST(u->Flags, SPR_DEAD))
        {
            NewStateGroup(actor, u->ActorActionSet->Dead);
            PlaySound(DIGI_ACTORBODYFALL1, actor, v3df_none);
        }
        else
        {
            PlaySound(DIGI_ACTORHITGROUND, actor, v3df_none);

            NewStateGroup(actor, u->ActorActionSet->Run);

            if ((u->track >= 0) && (u->jump_speed) > 800 && (u->ActorActionSet->Sit))
            {
                u->WaitTics = 80;
                NewStateGroup(actor, u->ActorActionSet->Sit);
            }
        }
    }

    return 0;
}

int DoActorDeathMove(DSWActor* actor)
{
    USER* u = actor->u();

    SPRITEp sp = &actor->s();
    int nx, ny;

    if (TEST(u->Flags, SPR_JUMPING | SPR_FALLING))
    {
        if (TEST(u->Flags, SPR_JUMPING))
            DoActorJump(actor);
        else
            DoActorFall(actor);
    }

    nx = MulScale(sp->xvel, bcos(sp->ang), 14);
    ny = MulScale(sp->xvel, bsin(sp->ang), 14);

    sp->clipdist = (128+64)>>2;
    move_actor(actor, nx, ny, 0);

    // only fall on top of floor sprite or sector
    DoFindGroundPoint(actor);

    return 0;
}

// !AIC - Jumping a falling for shrapnel and other stuff, not actors.

int DoBeginJump(DSWActor* actor)
{
    USERp u = actor->u();

    SET(u->Flags, SPR_JUMPING);
    RESET(u->Flags, SPR_FALLING);

    // set up individual actor jump gravity
    u->jump_grav = ACTOR_GRAVITY;

    DoJump(actor);

    return 0;
}

int DoJump(DSWActor* actor)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();

    int jump_adj;

    // precalculate jump value to adjust jump speed by
    jump_adj = u->jump_grav * ACTORMOVETICS;

    // adjust jump speed by gravity - if jump speed greater than 0 player
    // have started falling
    if ((u->jump_speed += jump_adj) > 0)
    {
        // Start falling
        DoBeginFall(actor);
        return 0;
    }

    // adjust height by jump speed
    sp->z += u->jump_speed * ACTORMOVETICS;

    // if player gets to close the ceiling while jumping
    if (sp->z < u->hiz + Z(PIC_SIZY(sp)))
    {
        // put player at the ceiling
        sp->z = u->hiz + Z(PIC_SIZY(sp));

        // reverse your speed to falling
        u->jump_speed = -u->jump_speed;

        // Change sprites state to falling
        DoBeginFall(actor);
    }

    return 0;
}


int DoBeginFall(DSWActor* actor)
{
    USERp u = actor->u();

    SET(u->Flags, SPR_FALLING);
    RESET(u->Flags, SPR_JUMPING);

    u->jump_grav = ACTOR_GRAVITY;

    DoFall(actor);

    return 0;
}

int DoFall(DSWActor* actor)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();

    // adjust jump speed by gravity
    u->jump_speed += u->jump_grav * ACTORMOVETICS;

    // adjust player height by jump speed
    sp->z += u->jump_speed * ACTORMOVETICS;

    // Stick like glue when you hit the ground
    if (sp->z > u->loz - u->floor_dist)
    {
        sp->z = u->loz - u->floor_dist;
        RESET(u->Flags, SPR_FALLING);
    }

    return 0;
}


#include "saveable.h"

static saveable_code saveable_actor_code[] =
{
    SAVE_CODE(DoActorDebris),
    SAVE_CODE(DoFireFly),
    SAVE_CODE(DoGenerateSewerDebris),
    SAVE_CODE(DoActorDeathMove),
};

saveable_module saveable_actor =
{
    // code
    saveable_actor_code,
    SIZ(saveable_actor_code),

    // data
    nullptr,0
};

END_SW_NS
