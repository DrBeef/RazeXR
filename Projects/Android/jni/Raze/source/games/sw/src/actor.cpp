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

extern int jump_grav;

extern STATE s_DebrisNinja[];
extern STATE s_DebrisRat[];
extern STATE s_DebrisCrab[];
extern STATE s_DebrisStarFish[];
extern STATE s_NinjaDieSliced[];
extern STATE s_NinjaDieSlicedHack[];

extern STATE* sg_NinjaGrabThroat[];



int DoScaleSprite(DSWActor* actor)
{
    int scale_value;

    if (actor->user.scale_speed)
    {
        actor->user.scale_value += actor->user.scale_speed * ACTORMOVETICS;

        scale_value = actor->user.scale_value >> 8;

        if (actor->user.scale_speed > 0)
        {
            if (scale_value > actor->user.scale_tgt)
                actor->user.scale_speed = 0;
            else
                actor->spr.xrepeat = actor->spr.yrepeat = scale_value;
        }
        else
        {
            if (scale_value < actor->user.scale_tgt)
                actor->user.scale_speed = 0;
            else
                actor->spr.xrepeat = actor->spr.yrepeat = scale_value;
        }

    }

    return 0;
}

int DoActorDie(DSWActor* actor, DSWActor* weapActor, int meansofdeath)
{
    change_actor_stat(actor, STAT_DEAD_ACTOR);
    actor->user.Flags |= (SPR_DEAD);
    actor->user.Flags &= ~(SPR_FALLING | SPR_JUMPING);
    actor->user.floor_dist = Z(40);

    // test for gibable dead bodies
    actor->spr.extra |= (SPRX_BREAKABLE);
    actor->spr.cstat |= (CSTAT_SPRITE_BREAKABLE);

    if (weapActor == nullptr)
    {
        // killed by one of these non-sprites
        switch (meansofdeath)
        {
        case WPN_NM_LAVA:
            ChangeState(actor, actor->user.StateEnd);
            actor->user.RotNum = 0;
            break;

        case WPN_NM_SECTOR_SQUISH:
            ChangeState(actor, actor->user.StateEnd);
            actor->user.RotNum = 0;
            break;
        }

        return 0;
    }

    if (!weapActor->hasU()) return 0;

    // killed by one of these sprites
    switch (weapActor->user.ID)
    {
    // Coolie actually explodes himself
    // he is the Sprite AND weapon
    case COOLIE_RUN_R0:
        ChangeState(actor, actor->user.StateEnd);
        actor->user.RotNum = 0;
        actor->spr.xvel <<= 1;
        actor->user.ActorActionFunc = nullptr;
        actor->spr.ang = NORM_ANGLE(actor->spr.ang + 1024);
        break;

    case NINJA_RUN_R0:
        if (actor->user.ID == NINJA_RUN_R0) // Cut in half!
        {
            if (weapActor->user.WeaponNum != WPN_FIST)
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

                ChangeState(actor, actor->user.StateEnd);
                actor->user.RotNum = 0;
                actor->user.ActorActionFunc = nullptr;
                actor->spr.xvel = 200 + RandomRange(200);
                actor->user.jump_speed = -200 - RandomRange(250);
                DoActorBeginJump(actor);
                actor->spr.ang = weapActor->spr.ang;
            }
        }
        else
        {
            // test for gibable dead bodies
            if (RandomRange(1000) > 500)
                actor->spr.cstat |= (CSTAT_SPRITE_YFLIP);
            ChangeState(actor, actor->user.StateEnd);
            actor->spr.xvel = 0;
            actor->user.jump_speed = 0;
            DoActorBeginJump(actor);
        }

        actor->user.RotNum = 0;

        actor->user.ActorActionFunc = nullptr;
        //actor->user.ActorActionFunc = NullAnimator;
        if (!sw_ninjahack)
            actor->spr.ang = weapActor->spr.ang;
        break;

    case COOLG_RUN_R0:
    case SKEL_RUN_R0:
    case RIPPER_RUN_R0:
    case RIPPER2_RUN_R0:
    case EEL_RUN_R0:
    case STAR1:
    case SUMO_RUN_R0:
        ChangeState(actor, actor->user.StateEnd);
        actor->user.RotNum = 0;
        break;

    case UZI_SMOKE:
        if (RandomRange(1000) > 500)
            actor->spr.cstat |= (CSTAT_SPRITE_YFLIP);
        ChangeState(actor, actor->user.StateEnd);
        actor->user.RotNum = 0;
        // Rippers still gotta jump or they fall off walls weird
        if (actor->user.ID == RIPPER_RUN_R0 || actor->user.ID == RIPPER2_RUN_R0)
        {
            actor->spr.xvel <<= 1;
            actor->user.jump_speed = -100 - RandomRange(250);
            DoActorBeginJump(actor);
        }
        else
        {
            actor->spr.xvel = 0;
            actor->user.jump_speed = -10 - RandomRange(25);
            DoActorBeginJump(actor);
        }
        actor->user.ActorActionFunc = nullptr;
        // Get angle to player
        actor->spr.ang = NORM_ANGLE(getangle(actor->user.targetActor->spr.pos.X - actor->spr.pos.X, actor->user.targetActor->spr.pos.Y - actor->spr.pos.Y) + 1024);
        break;

    case UZI_SMOKE+1: // Shotgun
        if (RandomRange(1000) > 500)
            actor->spr.cstat |= (CSTAT_SPRITE_YFLIP);
        ChangeState(actor, actor->user.StateEnd);
        actor->user.RotNum = 0;

        // Rippers still gotta jump or they fall off walls weird
        if (actor->user.ID == RIPPER_RUN_R0 || actor->user.ID == RIPPER2_RUN_R0)
        {
            actor->spr.xvel = 75 + RandomRange(100);
            actor->user.jump_speed = -100 - RandomRange(150);
        }
        else
        {
            actor->spr.xvel = 100 + RandomRange(200);
            actor->user.jump_speed = -100 - RandomRange(250);
        }
        DoActorBeginJump(actor);
        actor->user.ActorActionFunc = nullptr;
        // Get angle to player
        actor->spr.ang = NORM_ANGLE(getangle(actor->user.targetActor->spr.pos.X - actor->spr.pos.X, actor->user.targetActor->spr.pos.Y - actor->spr.pos.Y) + 1024);
        break;

    default:
        switch (actor->user.ID)
        {
        case SKULL_R0:
        case BETTY_R0:
            ChangeState(actor, actor->user.StateEnd);
            break;

        default:
            if (RandomRange(1000) > 700)
            {
                InitPlasmaFountain(weapActor, actor);
            }

            if (RandomRange(1000) > 500)
                actor->spr.cstat |= (CSTAT_SPRITE_YFLIP);
            ChangeState(actor, actor->user.StateEnd);
            actor->user.RotNum = 0;
            actor->user.ActorActionFunc = nullptr;
            actor->spr.xvel = 300 + RandomRange(400);
            actor->user.jump_speed = -300 - RandomRange(350);
            DoActorBeginJump(actor);
            actor->spr.ang = weapActor->spr.ang;
            break;
        }
        break;
    }

    // These are too big to flip upside down
    switch (actor->user.ID)
    {
    case RIPPER2_RUN_R0:
    case COOLIE_RUN_R0:
    case SUMO_RUN_R0:
    case ZILLA_RUN_R0:
        actor->spr.cstat &= ~(CSTAT_SPRITE_YFLIP);
        break;
    }

    actor->user.ID = 0;

    return 0;
}

void DoDebrisCurrent(DSWActor* actor)
{
    int nx, ny;
    auto sectp = actor->sector();

    //actor->spr.clipdist = (256+128)>>2;

    nx = MulScale((sectp->speed >> 2), bcos(sectp->ang), 14);
    ny = MulScale((sectp->speed >> 2), bsin(sectp->ang), 14);

    Collision ret = move_sprite(actor, nx, ny, 0, actor->user.ceiling_dist, actor->user.floor_dist, 0, ACTORMOVETICS);

    // attempt to move away from wall
    if (ret.type != kHitNone)
    {
        int rang = RANDOM_P2(2048);

        nx = MulScale((sectp->speed >> 2), bcos(sectp->ang + rang), 14);
        nx = MulScale((sectp->speed >> 2), bsin(sectp->ang + rang), 14);

        move_sprite(actor, nx, ny, 0, actor->user.ceiling_dist, actor->user.floor_dist, 0, ACTORMOVETICS);
    }

    actor->spr.pos.Z = actor->user.loz;
}

int DoActorSectorDamage(DSWActor* actor)
{
    sectortype* sectp = actor->sector();

    if (actor->user.Health <= 0)
        return false;

    if (sectp->hasU() && sectp->damage)
    {
        if ((sectp->flags & SECTFU_DAMAGE_ABOVE_SECTOR))
        {
            if ((actor->user.DamageTics -= synctics) < 0)
            {
                actor->user.DamageTics = 60;
                actor->user.Health -= sectp->damage;

                if (actor->user.Health <= 0)
                {
                    UpdateSinglePlayKills(actor);
                    DoActorDie(actor, nullptr, WPN_NM_LAVA);
                    return true;
                }
            }
        }
        else if (ActorZOfBottom(actor) >= sectp->floorz)
        {
            if ((actor->user.DamageTics -= synctics) < 0)
            {
                actor->user.DamageTics = 60;
                actor->user.Health -= sectp->damage;

                if (actor->user.Health <= 0)
                {
                    UpdateSinglePlayKills(actor);
                    DoActorDie(actor, nullptr, WPN_NM_LAVA);
                    return true;
                }
            }
        }
    }

    // note that most squishing is done in vator.c
    if (actor->user.lo_sectp && actor->user.hi_sectp && labs(actor->user.loz - actor->user.hiz) < (ActorSizeZ(actor) >> 1))
    {
        actor->user.Health = 0;
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
    actor->user.coll = move_sprite(actor, xchange, ychange, zchange,
                         actor->user.ceiling_dist, actor->user.floor_dist, 0, ACTORMOVETICS);

    return actor->user.coll.type == kHitNone;
}

// !AIC - Supposed to allow floating of DEBRIS (dead bodies, flotsam, jetsam).  Or if water has
// current move with the current.

int DoActorDebris(DSWActor* actor)
{
    sectortype* sectp = actor->sector();
    int nx, ny;

    // This was move from DoActorDie so actor's can't be walked through until they are on the floor
    actor->spr.cstat &= ~(CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN);

    // Don't let some actors float
    switch (actor->user.ID)
    {
    case HORNET_RUN_R0:
    case BUNNY_RUN_R0:
        KillActor(actor);
        return 0;
    case ZILLA_RUN_R0:
        getzsofslopeptr(actor->sector(), actor->spr.pos.X, actor->spr.pos.Y, &actor->user.hiz, &actor->user.loz);
        actor->user.lo_sectp = actor->sector();
        actor->user.hi_sectp = actor->sector();
        actor->user.lowActor = nullptr;
        actor->user.highActor = nullptr;
        break;
    }

    if ((sectp->extra & SECTFX_SINK))
    {
        if ((sectp->extra & SECTFX_CURRENT))
        {
            DoDebrisCurrent(actor);
        }
        else
        {
            //nx = actor->spr.xvel * ACTORMOVETICS * bcos(actor->spr.ang) >> 14;
            //ny = actor->spr.xvel * ACTORMOVETICS * bsin(actor->spr.ang) >> 14;
            nx = MulScale(ACTORMOVETICS, bcos(actor->spr.ang), 14);
            ny = MulScale(ACTORMOVETICS, bsin(actor->spr.ang), 14);

            //actor->spr.clipdist = (256+128)>>2;

            if (!move_debris(actor, nx, ny, 0L))
            {
                actor->spr.ang = RANDOM_P2(2048);
            }
        }

        if (actor->sector()->hasU() && FixedToInt(actor->sector()->depth_fixed) > 10) // JBF: added null check
        {
            actor->user.WaitTics = (actor->user.WaitTics + (ACTORMOVETICS << 3)) & 1023;
            actor->spr.pos.Z = actor->user.loz - MulScale(Z(2), bsin(actor->user.WaitTics), 14);
        }
    }
    else
    {
        actor->spr.pos.Z = actor->user.loz;
    }

    return 0;
}


int DoFireFly(DSWActor* actor)
{
    int nx, ny;

    nx = 4 * ACTORMOVETICS * bcos(actor->spr.ang) >> 14;
    ny = 4 * ACTORMOVETICS * bsin(actor->spr.ang) >> 14;

    actor->spr.clipdist = 256>>2;
    if (!move_actor(actor, nx, ny, 0L))
    {
        actor->spr.ang = NORM_ANGLE(actor->spr.ang + 1024);
    }

    actor->user.WaitTics = (actor->user.WaitTics + (ACTORMOVETICS << 1)) & 2047;

    actor->spr.pos.Z = actor->user.pos.Z + MulScale(Z(32), bsin(actor->user.WaitTics), 14);
    return 0;
}

int DoGenerateSewerDebris(DSWActor* actor)
{
    static STATE* Debris[] =
    {
        s_DebrisNinja,
        s_DebrisRat,
        s_DebrisCrab,
        s_DebrisStarFish
    };

    actor->user.Tics -= ACTORMOVETICS;

    if (actor->user.Tics <= 0)
    {
        actor->user.Tics = actor->user.WaitTics;

        auto spawned = SpawnActor(STAT_DEAD_ACTOR, 0, Debris[RANDOM_P2(4<<8)>>8], actor->sector(), actor->spr.pos.X, actor->spr.pos.Y, actor->spr.pos.Z, actor->spr.ang, 200);

        SetOwner(actor, spawned);
    }

    return 0;
}

// !AIC - Tries to keep actors correctly on the floor.  More that a bit messy.

void KeepActorOnFloor(DSWActor* actor)
{
    sectortype* sectp;
    int depth;

    sectp = actor->sector();

    actor->spr.cstat &= ~(CSTAT_SPRITE_YFLIP); // If upside down, reset it

    if (actor->user.Flags & (SPR_JUMPING | SPR_FALLING))
        return;

    if (actor->user.lo_sectp && actor->user.lo_sectp->hasU())
        depth = FixedToInt(actor->user.lo_sectp->depth_fixed);
    else
        depth = 0;

    if ((sectp->extra & SECTFX_SINK) &&
        depth > 35 &&
        actor->user.ActorActionSet && actor->user.ActorActionSet->Swim)
    {
        if (actor->user.Flags & (SPR_SWIMMING))
        {
            if (actor->user.Rot != actor->user.ActorActionSet->Run && actor->user.Rot != actor->user.ActorActionSet->Swim && actor->user.Rot != actor->user.ActorActionSet->Stand)
            {
                // was swimming but have now stopped
                actor->user.Flags &= ~(SPR_SWIMMING);
                actor->spr.cstat &= ~(CSTAT_SPRITE_YCENTER);
                actor->user.oz = actor->spr.pos.Z = actor->user.loz;
                actor->backupz();
                return;
            }

            if (actor->user.Rot == actor->user.ActorActionSet->Run)
            {
                NewStateGroup(actor, actor->user.ActorActionSet->Swim);
            }

            // are swimming
            actor->user.oz = actor->spr.pos.Z = actor->user.loz - Z(depth);
            actor->backupz();
        }
        else
        {
            // only start swimming if you are running
            if (actor->user.Rot == actor->user.ActorActionSet->Run || actor->user.Rot == actor->user.ActorActionSet->Swim)
            {
                NewStateGroup(actor, actor->user.ActorActionSet->Swim);
                actor->user.oz = actor->spr.pos.Z = actor->user.loz - Z(depth);
                actor->backupz();
                actor->user.Flags |= (SPR_SWIMMING);
                actor->spr.cstat |= (CSTAT_SPRITE_YCENTER);
            }
            else
            {
                actor->user.Flags &= ~(SPR_SWIMMING);
                actor->spr.cstat &= ~(CSTAT_SPRITE_YCENTER);
                actor->user.oz = actor->spr.pos.Z = actor->user.loz;
                actor->backupz();
            }
        }

        return;
    }

    // NOT in a swimming situation
    actor->user.Flags &= ~(SPR_SWIMMING);
    actor->spr.cstat &= ~(CSTAT_SPRITE_YCENTER);

#if 1
    if (actor->user.Flags & (SPR_MOVED))
    {
        actor->user.oz = actor->spr.pos.Z = actor->user.loz;
        actor->backupz();
    }
    else
    {
        int ceilz, florz;
        Collision ctrash, ftrash;
        FAFgetzrangepoint(actor->spr.pos.X, actor->spr.pos.Y, actor->spr.pos.Z, actor->sector(),
                          &ceilz, &ctrash, &florz, &ftrash);

        actor->user.oz = actor->spr.pos.Z = florz;
        actor->backupz();
    }
#endif


}

int DoActorBeginSlide(DSWActor* actor, int ang, int vel, int dec)
{
    actor->user.Flags |= (SPR_SLIDING);

    actor->user.slide_ang = ang;
    actor->user.slide_vel = vel;
    actor->user.slide_dec = dec;

    //DoActorSlide(actor);

    return 0;
}

// !AIC - Sliding can occur in different directions from movement of the actor.
// Has its own set of variables

int DoActorSlide(DSWActor* actor)
{
    int nx, ny;

    nx = MulScale(actor->user.slide_vel, bcos(actor->user.slide_ang), 14);
    ny = MulScale(actor->user.slide_vel, bsin(actor->user.slide_ang), 14);

    if (!move_actor(actor, nx, ny, 0L))
    {
        actor->user.Flags &= ~(SPR_SLIDING);
        return false;
    }

    actor->user.slide_vel -= actor->user.slide_dec * ACTORMOVETICS;

    if (actor->user.slide_vel < 20)
    {
        actor->user.Flags &= ~(SPR_SLIDING);
    }

    return true;
}

// !AIC - Actor jumping and falling

int DoActorBeginJump(DSWActor* actor)
{
    actor->user.Flags |= (SPR_JUMPING);
    actor->user.Flags &= ~(SPR_FALLING);

    // actor->user.jump_speed = should be set before calling

    // set up individual actor jump gravity
    actor->user.jump_grav = ACTOR_GRAVITY;

    // Change sprites state to jumping
    if (actor->user.ActorActionSet)
    {
        if (actor->user.Flags & (SPR_DEAD))
            NewStateGroup(actor, actor->user.ActorActionSet->DeathJump);
        else
            NewStateGroup(actor, actor->user.ActorActionSet->Jump);
    }
    actor->user.StateFallOverride = nullptr;

    //DO NOT CALL DoActorJump! DoActorStopFall can cause an infinite loop and
    //stack overflow if it is called.

    return 0;
}

int DoActorJump(DSWActor* actor)
{
    int jump_adj;

    // precalculate jump value to adjust jump speed by
    jump_adj = actor->user.jump_grav * ACTORMOVETICS;

    // adjust jump speed by gravity - if jump speed greater than 0 player
    // have started falling
    if ((actor->user.jump_speed += jump_adj) > 0)
    {
        // Start falling
        DoActorBeginFall(actor);
        return 0;
    }

    // adjust height by jump speed
    actor->spr.pos.Z += actor->user.jump_speed * ACTORMOVETICS;

    // if player gets to close the ceiling while jumping
    int minh = actor->user.hiz + (tileHeight(actor->spr.picnum) << 8);
    if (actor->spr.pos.Z < minh)
    {
        // put player at the ceiling
        actor->spr.pos.Z = minh;

        // reverse your speed to falling
        actor->user.jump_speed = -actor->user.jump_speed;

        // Change sprites state to falling
        DoActorBeginFall(actor);
    }

    return 0;
}


int DoActorBeginFall(DSWActor* actor)
{
    actor->user.Flags |= (SPR_FALLING);
    actor->user.Flags &= ~(SPR_JUMPING);

    actor->user.jump_grav = ACTOR_GRAVITY;

    // Change sprites state to falling
    if (actor->user.ActorActionSet)
    {
        if (actor->user.Flags & (SPR_DEAD))
        {
            NewStateGroup(actor, actor->user.ActorActionSet->DeathFall);
        }
        else
            NewStateGroup(actor, actor->user.ActorActionSet->Fall);

        if (actor->user.StateFallOverride)
        {
            NewStateGroup(actor, actor->user.StateFallOverride);
        }
    }

    DoActorFall(actor);

    return 0;
}


int DoActorFall(DSWActor* actor)
{
    // adjust jump speed by gravity
    actor->user.jump_speed += actor->user.jump_grav * ACTORMOVETICS;

    // adjust player height by jump speed
    actor->spr.pos.Z += actor->user.jump_speed * ACTORMOVETICS;

    // Stick like glue when you hit the ground
    if (actor->spr.pos.Z > actor->user.loz)
    {
        DoActorStopFall(actor);
    }

    return 0;
}

int DoActorStopFall(DSWActor* actor)
{
    actor->spr.pos.Z = actor->user.loz;

    actor->user.Flags &= ~(SPR_FALLING | SPR_JUMPING);
    actor->spr.cstat &= ~(CSTAT_SPRITE_YFLIP);


    // don't stand on face or wall sprites - jump again
    if (actor->user.lowActor && !(actor->user.lowActor->spr.cstat & CSTAT_SPRITE_ALIGNMENT_FLOOR))
    {
        //actor->spr.ang = NORM_ANGLE(actor->spr.ang + (RANDOM_P2(64<<8)>>8) - 32);
        actor->spr.ang = NORM_ANGLE(actor->spr.ang + 1024 + (RANDOM_P2(512<<8)>>8));
        actor->user.jump_speed = -350;

        DoActorBeginJump(actor);
        return 0;
    }

    // Change sprites state to running
    if (actor->user.ActorActionSet)
    {
        if (actor->user.Flags & (SPR_DEAD))
        {
            NewStateGroup(actor, actor->user.ActorActionSet->Dead);
            PlaySound(DIGI_ACTORBODYFALL1, actor, v3df_none);
        }
        else
        {
            PlaySound(DIGI_ACTORHITGROUND, actor, v3df_none);

            NewStateGroup(actor, actor->user.ActorActionSet->Run);

            if ((actor->user.track >= 0) && (actor->user.jump_speed) > 800 && (actor->user.ActorActionSet->Sit))
            {
                actor->user.WaitTics = 80;
                NewStateGroup(actor, actor->user.ActorActionSet->Sit);
            }
        }
    }

    return 0;
}

int DoActorDeathMove(DSWActor* actor)
{
    int nx, ny;

    if (actor->user.Flags & (SPR_JUMPING | SPR_FALLING))
    {
        if (actor->user.Flags & (SPR_JUMPING))
            DoActorJump(actor);
        else
            DoActorFall(actor);
    }

    nx = MulScale(actor->spr.xvel, bcos(actor->spr.ang), 14);
    ny = MulScale(actor->spr.xvel, bsin(actor->spr.ang), 14);

    actor->spr.clipdist = (128+64)>>2;
    move_actor(actor, nx, ny, 0);

    // only fall on top of floor sprite or sector
    DoFindGroundPoint(actor);

    return 0;
}

// !AIC - Jumping a falling for shrapnel and other stuff, not actors.

int DoBeginJump(DSWActor* actor)
{
    actor->user.Flags |= (SPR_JUMPING);
    actor->user.Flags &= ~(SPR_FALLING);

    // set up individual actor jump gravity
    actor->user.jump_grav = ACTOR_GRAVITY;

    DoJump(actor);

    return 0;
}

int DoJump(DSWActor* actor)
{
    int jump_adj;

    // precalculate jump value to adjust jump speed by
    jump_adj = actor->user.jump_grav * ACTORMOVETICS;

    // adjust jump speed by gravity - if jump speed greater than 0 player
    // have started falling
    if ((actor->user.jump_speed += jump_adj) > 0)
    {
        // Start falling
        DoBeginFall(actor);
        return 0;
    }

    // adjust height by jump speed
    actor->spr.pos.Z += actor->user.jump_speed * ACTORMOVETICS;

    // if player gets to close the ceiling while jumping
    int minh = actor->user.hiz + (tileHeight(actor->spr.picnum) << 8);
    if (actor->spr.pos.Z < minh)
    {
        // put player at the ceiling
        actor->spr.pos.Z = minh;

        // reverse your speed to falling
        actor->user.jump_speed = -actor->user.jump_speed;

        // Change sprites state to falling
        DoBeginFall(actor);
    }

    return 0;
}


int DoBeginFall(DSWActor* actor)
{
    actor->user.Flags |= (SPR_FALLING);
    actor->user.Flags &= ~(SPR_JUMPING);

    actor->user.jump_grav = ACTOR_GRAVITY;

    DoFall(actor);

    return 0;
}

int DoFall(DSWActor* actor)
{
    // adjust jump speed by gravity
    actor->user.jump_speed += actor->user.jump_grav * ACTORMOVETICS;

    // adjust player height by jump speed
    actor->spr.pos.Z += actor->user.jump_speed * ACTORMOVETICS;

    // Stick like glue when you hit the ground
    if (actor->spr.pos.Z > actor->user.loz - actor->user.floor_dist)
    {
        actor->spr.pos.Z = actor->user.loz - actor->user.floor_dist;
        actor->user.Flags &= ~(SPR_FALLING);
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
