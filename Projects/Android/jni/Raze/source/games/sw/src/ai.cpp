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
#include "game.h"
#include "tags.h"
#include "sector.h"
#include "sprite.h"

// temp - should be moved
#include "ai.h"

#include "network.h"

BEGIN_SW_NS

ANIMATOR InitActorRunToward;
bool DropAhead(DSWActor* actor, int  min_height);

ANIMATOR* ChooseAction(DECISION decision[]);


#define CHOOSE2(value) (RANDOM_P2(1024) < (value))

int Distance(int x1, int y1, int x2, int y2)
{
    int min;

    if ((x2 = x2 - x1) < 0)
        x2 = -x2;

    if ((y2 = y2 - y1) < 0)
        y2 = -y2;

    if (x2 > y2)
        min = y2;
    else
        min = x2;

    return x2 + y2 - (min >> 1);
}



bool ActorMoveHitReact(DSWActor* actor)
{
    // Should only return true if there is a reaction to what was hit that
    // would cause the calling function to abort

    auto coll = actor->user.coll;
    if (coll.type == kHitSprite)
    {
        auto hitActor = coll.actor();
        if (hitActor->hasU() && hitActor->user.PlayerP)
        {
            // if you ran into a player - call close range functions
            DoActorPickClosePlayer(actor);
            auto action = ChooseAction(actor->user.Personality->TouchTarget);
            if (action)
            {
                (*action)(actor);
                return true;
            }
        }
    }
    return false;
}


bool ActorFlaming(DSWActor* actor)
{
    auto flame = actor->user.flameActor;
    if (flame != nullptr)
    {
        int size;

        size = ActorSizeZ(actor) - (ActorSizeZ(actor) >> 2);

        if (ActorSizeZ(flame) > size)
            return true;
    }

    return false;
}

void DoActorSetSpeed(DSWActor* actor, uint8_t speed)
{
    if (actor->spr.cstat & (CSTAT_SPRITE_RESTORE))
        return;

    ASSERT(actor->user.Attrib);

    actor->user.speed = speed;

    if (ActorFlaming(actor))
        actor->spr.xvel = actor->user.Attrib->Speed[speed] + (actor->user.Attrib->Speed[speed] >> 1);
    else
        actor->spr.xvel = actor->user.Attrib->Speed[speed];
}

/*
  !AIC - Does a table lookup based on a random value from 0 to 1023.
  These tables are defined at the top of all actor files such as ninja.c,
  goro.c etc.
*/

ANIMATOR* ChooseAction(DECISION decision[])
{
    // !JIM! Here is an opportunity for some AI, instead of randomness!
    int random_value = RANDOM_P2(1024<<5)>>5;

    for (int i = 0; true; i++)
    {
        ASSERT(i < 10);

        if (random_value <= decision[i].range)
        {
            return decision[i].action;
        }
    }
}

/*
  !AIC - Sometimes just want the offset of the action
*/

int ChooseActionNumber(int16_t decision[])
{
    int random_value = RANDOM_P2(1024<<5)>>5;

    for (int i = 0; true; i++)
    {
        if (random_value <= decision[i])
        {
            return i;
        }
    }
}

int DoActorNoise(ANIMATOR* Action, DSWActor* actor)
{
    if (Action == InitActorAmbientNoise)
    {
        PlaySpriteSound(actor, attr_ambient, v3df_follow);
    }
    else if (Action == InitActorAlertNoise)
    {
        if (actor->hasU() && !actor->user.DidAlert) // This only allowed once
            PlaySpriteSound(actor, attr_alert, v3df_follow);
    }
    else if (Action == InitActorAttackNoise)
    {
        PlaySpriteSound(actor, attr_attack, v3df_follow);
    }
    else if (Action == InitActorPainNoise)
    {
        PlaySpriteSound(actor, attr_pain, v3df_follow);
    }
    else if (Action == InitActorDieNoise)
    {
        PlaySpriteSound(actor, attr_die, v3df_none);
    }
    else if (Action == InitActorExtra1Noise)
    {
        PlaySpriteSound(actor, attr_extra1, v3df_follow);
    }
    else if (Action == InitActorExtra2Noise)
    {
        PlaySpriteSound(actor, attr_extra2, v3df_follow);
    }
    else if (Action == InitActorExtra3Noise)
    {
        PlaySpriteSound(actor, attr_extra3, v3df_follow);
    }
    else if (Action == InitActorExtra4Noise)
    {
        PlaySpriteSound(actor, attr_extra4, v3df_follow);
    }
    else if (Action == InitActorExtra5Noise)
    {
        PlaySpriteSound(actor, attr_extra5, v3df_follow);
    }
    else if (Action == InitActorExtra6Noise)
    {
        PlaySpriteSound(actor, attr_extra6, v3df_follow);
    }

    return 0;
}

bool CanSeePlayer(DSWActor* actor)
{
    // if actor can still see the player
    int look_height = ActorZOfTop(actor);

    if (actor->user.targetActor && FAFcansee(actor->spr.pos.X, actor->spr.pos.Y, look_height, actor->sector(), actor->user.targetActor->spr.pos.X, actor->user.targetActor->spr.pos.Y, ActorUpperZ(actor->user.targetActor), actor->user.targetActor->sector()))
        return true;
    else
        return false;
}

int CanHitPlayer(DSWActor* actor)
{
    HitInfo hit{};
    int xvect,yvect,zvect;
    int ang;
    // if actor can still see the player
    int zhs, zhh;

    zhs = actor->spr.pos.Z - (ActorSizeZ(actor) >> 1);


    auto targ = actor->user.targetActor;

    // get angle to target
    ang = getangle(targ->spr.pos.X - actor->spr.pos.X, targ->spr.pos.Y - actor->spr.pos.Y);

    // get x,yvect
    xvect = bcos(ang);
    yvect = bsin(ang);

    // get zvect
    zhh = targ->spr.pos.Z - (ActorSizeZ(targ) >> 1);
    if (targ->spr.pos.X - actor->spr.pos.X != 0)
        zvect = xvect * ((zhh - zhs) / (targ->spr.pos.X - actor->spr.pos.X));
    else if (targ->spr.pos.Y - actor->spr.pos.Y != 0)
        zvect = yvect * ((zhh - zhs) / (targ->spr.pos.Y - actor->spr.pos.Y));
    else
        return false;

    FAFhitscan(actor->spr.pos.X, actor->spr.pos.Y, zhs, actor->sector(),
               xvect,
               yvect,
               zvect,
               hit, CLIPMASK_MISSILE);

    if (hit.hitSector == nullptr)
        return false;

    if (hit.actor() == actor->user.targetActor)
        return true;

    return false;
}

/*
  !AIC - Pick a nearby player to be the actors target
*/

int DoActorPickClosePlayer(DSWActor* actor)
{
    int dist, near_dist = MAX_ACTIVE_RANGE, a,b,c;
    int pnum;
    PLAYER* pp;
    // if actor can still see the player
    int look_height = ActorZOfTop(actor);
    bool found = false;
    int i;

    if (actor->user.ID == ZOMBIE_RUN_R0 && gNet.MultiGameType == MULTI_GAME_COOPERATIVE)
        goto TARGETACTOR;

    // Set initial target to Player 0
    actor->user.targetActor = Player->actor;

    if (actor->user.Flags2 & (SPR2_DONT_TARGET_OWNER))
    {
        TRAVERSE_CONNECT(pnum)
        {
            pp = &Player[pnum];

            if (GetOwner(actor) == pp->actor)
                continue;

            actor->user.targetActor = pp->actor;
            break;
        }
    }


    // Set initial target to the closest player
    near_dist = MAX_ACTIVE_RANGE;
    TRAVERSE_CONNECT(pnum)
    {
        pp = &Player[pnum];

        // Zombies don't target their masters!
        if (actor->user.Flags2 & (SPR2_DONT_TARGET_OWNER))
        {
            if (GetOwner(actor) == pp->actor)
                continue;

            if (!PlayerTakeDamage(pp, actor))
                continue;

            // if co-op don't hurt teammate
            // if (gNet.MultiGameType == MULTI_GAME_COOPERATIVE && !gNet.HurtTeammate && actor->user.spal == pp->actor->spr.spal)
            //    continue;
        }

        DISTANCE(actor->spr.pos.X, actor->spr.pos.Y, pp->pos.X, pp->pos.Y, dist, a, b, c);

        if (dist < near_dist)
        {
            near_dist = dist;
            actor->user.targetActor = pp->actor;
        }
    }

    // see if you can find someone close that you can SEE
    near_dist = MAX_ACTIVE_RANGE;
    found = false;
    TRAVERSE_CONNECT(pnum)
    {
        pp = &Player[pnum];

        // Zombies don't target their masters!
        if (actor->user.Flags2 & (SPR2_DONT_TARGET_OWNER))
        {
            if (GetOwner(actor) == pp->actor)
                continue;

            if (!PlayerTakeDamage(pp, actor))
                continue;
        }

        DISTANCE(actor->spr.pos.X, actor->spr.pos.Y, pp->pos.X, pp->pos.Y, dist, a, b, c);

        DSWActor* plActor = pp->actor;
        if (dist < near_dist && FAFcansee(actor->spr.pos.X, actor->spr.pos.Y, look_height, actor->sector(), plActor->spr.pos.X, plActor->spr.pos.Y, ActorUpperZ(plActor), plActor->sector()))
        {
            near_dist = dist;
            actor->user.targetActor = pp->actor;
            found = true;
        }
    }


TARGETACTOR:
    // this is only for Zombies right now
    // zombie target other actors
    if (!found && (actor->user.Flags2 & SPR2_DONT_TARGET_OWNER))
    {
        near_dist = MAX_ACTIVE_RANGE;
        SWStatIterator it(STAT_ENEMY);
        while (auto itActor = it.Next())
        {
            if (itActor == actor || !itActor->hasU())
                continue;

            if ((itActor->user.Flags & (SPR_SUICIDE | SPR_DEAD)))
                continue;

            DISTANCE(actor->spr.pos.X, actor->spr.pos.Y, itActor->spr.pos.X, itActor->spr.pos.Y, dist, a, b, c);

            if (dist < near_dist && FAFcansee(actor->spr.pos.X, actor->spr.pos.Y, look_height, actor->sector(), itActor->spr.pos.X, itActor->spr.pos.Y, ActorUpperZ(itActor), itActor->sector()))
            {
                near_dist = dist;
                actor->user.targetActor = itActor;
            }
        }
    }

    return 0;
}

DSWActor* GetPlayerSpriteNum(DSWActor* actor)
{
    int pnum;
    PLAYER* pp;

    TRAVERSE_CONNECT(pnum)
    {
        pp = &Player[pnum];

        if (pp->actor == actor->user.targetActor)
        {
            return pp->actor;
        }
    }
    return nullptr;
}

int CloseRangeDist(DSWActor* actor1, DSWActor* actor2)
{
    int clip1 = actor1->spr.clipdist;
    int clip2 = actor2->spr.clipdist;

    // add clip boxes and a fudge factor
    const int DIST_CLOSE_RANGE = 400;

    return (clip1 << 2) + (clip2 << 2) + DIST_CLOSE_RANGE;
}

int DoActorOperate(DSWActor* actor)
{
    HitInfo near{};
    int z[2];
    unsigned int i;

    if (actor->user.ID == HORNET_RUN_R0 || actor->user.ID == EEL_RUN_R0 || actor->user.ID == BUNNY_RUN_R0)
        return false;

    if (actor->user.Rot == actor->user.ActorActionSet->Sit || actor->user.Rot == actor->user.ActorActionSet->Stand)
        return false;

    if ((actor->user.WaitTics -= ACTORMOVETICS) > 0)
        return false;

    z[0] = actor->spr.pos.Z - ActorSizeZ(actor) + Z(5);
    z[1] = actor->spr.pos.Z - (ActorSizeZ(actor) >> 1);

    for (i = 0; i < SIZ(z); i++)
    {
        neartag({ actor->spr.pos.X, actor->spr.pos.Y, z[i] }, actor->sector(), actor->spr.ang, near, 1024, NTAG_SEARCH_LO_HI);
    }

    if (near.hitSector != nullptr && near.hitpos.X < 1024)
    {
        if (OperateSector(near.hitSector, false))
        {
            actor->user.WaitTics = 2 * 120;

            NewStateGroup(actor, actor->user.ActorActionSet->Sit);
        }
    }

    return true;

}

DECISION GenericFlaming[] =
{
    {30, InitActorAttack},
    {512, InitActorRunToward},
    {1024, InitActorRunAway},
};

/*
 !AIC KEY - This routine decides what the actor will do next.  It is not called
 every time through the loop.  This would be too slow.  It is only called when
 the actor needs to know what to do next such as running into something or being
 targeted.  It makes decisions based on the distance and viewablity of its target
 (actor->user.targetActor).  When it figures out the situatation with its target it calls
 ChooseAction which does a random table lookup to decide what action to initialize.
 Once this action is initialized it will be called until it can't figure out what to
 do anymore and then this routine is called again.
*/

ANIMATOR* DoActorActionDecide(DSWActor* actor)
{
    int dist;
    ANIMATOR* action;
    bool ICanSee=false;

    // REMINDER: This function is not even called if SpriteControl doesn't let
    // it get called

    ASSERT(actor->user.Personality);

    actor->user.Dist = 0;
    action = InitActorDecide;

    // target is gone.
    if (actor->user.targetActor == nullptr)
    {
        return action;
    }

    if (actor->user.Flags & (SPR_JUMPING | SPR_FALLING))
    {
        //CON_Message("Jumping or falling");
        return action;
    }

    // everybody on fire acts like this
    if (ActorFlaming(actor))
    {
        action = ChooseAction(&GenericFlaming[0]);
        //CON_Message("On Fire");
        return action;
    }

    ICanSee = CanSeePlayer(actor);  // Only need to call once
    // But need the result multiple times

    // !AIC KEY - If aware of player - var is changed in SpriteControl
    if (actor->user.Flags & (SPR_ACTIVE))
    {

        // Try to operate stuff
        DoActorOperate(actor);

        // if far enough away and cannot see the player
        dist = Distance(actor->spr.pos.X, actor->spr.pos.Y, actor->user.targetActor->spr.pos.X, actor->user.targetActor->spr.pos.Y);

        if (dist > 30000 && !ICanSee)
        {
            // Enemy goes inactive - he is still allowed to roam about for about
            // 5 seconds trying to find another player before his active_range is
            // bumped down
            actor->user.Flags &= ~(SPR_ACTIVE);

            // You've lost the player - now decide what to do
            action = ChooseAction(actor->user.Personality->LostTarget);
            //CON_Message("LostTarget");
            return action;
        }


        auto pActor = GetPlayerSpriteNum(actor);
        // check for short range attack possibility
        if ((dist < CloseRangeDist(actor, actor->user.targetActor) && ICanSee) ||
            (pActor && pActor->hasU() && pActor->user.WeaponNum == WPN_FIST && actor->user.ID != RIPPER2_RUN_R0 && actor->user.ID != RIPPER_RUN_R0))
        {
            if ((actor->user.ID == COOLG_RUN_R0 && (actor->spr.cstat & CSTAT_SPRITE_TRANSLUCENT)) || (actor->spr.cstat & CSTAT_SPRITE_INVISIBLE))
                action = ChooseAction(actor->user.Personality->Evasive);
            else
                action = ChooseAction(actor->user.Personality->CloseRange);
            //CON_Message("CloseRange");
            return action;
        }

        // if player is facing me and I'm being attacked
        if (Facing(actor, actor->user.targetActor) && (actor->user.Flags & SPR_ATTACKED) && ICanSee)
        {
            // if I'm a target - at least one missile comming at me
            if (actor->user.Flags & (SPR_TARGETED))
            {
                // not going to evade, reset the target bit
                actor->user.Flags &= ~(SPR_TARGETED);        // as far as actor
                // knows, its not a
                // target any more
                if (actor->user.ActorActionSet->Duck && RANDOM_P2(1024<<8)>>8 < 100)
                    action = InitActorDuck;
                else
                {
                    if ((actor->user.ID == COOLG_RUN_R0 && (actor->spr.cstat & CSTAT_SPRITE_TRANSLUCENT)) || (actor->spr.cstat & CSTAT_SPRITE_INVISIBLE))
                        action = ChooseAction(actor->user.Personality->Evasive);
                    else
                        action = ChooseAction(actor->user.Personality->Battle);
                }
                //CON_Message("Battle 1");
                return action;
            }
            // if NOT a target - don't bother with evasive action and start
            // fighting
            else
            {
                if ((actor->user.ID == COOLG_RUN_R0 && (actor->spr.cstat & CSTAT_SPRITE_TRANSLUCENT)) || (actor->spr.cstat & CSTAT_SPRITE_INVISIBLE))
                    action = ChooseAction(actor->user.Personality->Evasive);
                else
                    action = ChooseAction(actor->user.Personality->Battle);
                //CON_Message("Battle 2");
                return action;
            }

        }
        // if player is NOT facing me he is running or unaware of actor
        else if (ICanSee)
        {
            if ((actor->user.ID == COOLG_RUN_R0 && (actor->spr.cstat & CSTAT_SPRITE_TRANSLUCENT)) || (actor->spr.cstat & CSTAT_SPRITE_INVISIBLE))
                action = ChooseAction(actor->user.Personality->Evasive);
            else
                action = ChooseAction(actor->user.Personality->Offense);
            //CON_Message("Offense");
            return action;
        }
        else
        {
            // You've lost the player - now decide what to do
            action = ChooseAction(actor->user.Personality->LostTarget);
            //CON_Message("Close but cant see, LostTarget");
            return action;
        }
    }
    // Not active - not aware of player and cannot see him
    else
    {
        // try and find another player
        // pick a closeby player as the (new) target
        if (actor->spr.hitag != TAG_SWARMSPOT)
            DoActorPickClosePlayer(actor);

        // if close by
        dist = Distance(actor->spr.pos.X, actor->spr.pos.Y, actor->user.targetActor->spr.pos.X, actor->user.targetActor->spr.pos.Y);
        if (dist < 15000 || ICanSee)
        {
            if ((Facing(actor, actor->user.targetActor) && dist < 10000) || ICanSee)
            {
                DoActorOperate(actor);

                // Don't let player completely sneek up behind you
                action = ChooseAction(actor->user.Personality->Surprised);
                //CON_Message("Surprised");
                if (!actor->user.DidAlert && ICanSee)
                {
                    DoActorNoise(InitActorAlertNoise, actor);
                    actor->user.DidAlert = true;
                }
                return action;

            }
            else
            {
                // Player has not seen actor, to be fair let him know actor
                // are there
                DoActorNoise(ChooseAction(actor->user.Personality->Broadcast),actor);
                //CON_Message("Actor Noise");
                return action;
            }
        }
    }

    //CON_Message("Couldn't resolve decide, InitActorDecide");
    return action;
}

/*
  !AIC - Setup to do the decision
*/


int InitActorDecide(DSWActor* actor)
{
    actor->user.ActorActionFunc = DoActorDecide;
    return DoActorDecide(actor);
}

int DoActorDecide(DSWActor* actor)
{
    ANIMATOR* actor_action;

    // See what to do next
    actor_action = DoActorActionDecide(actor);

    // Fix for the GenericFlaming bug for actors that don't have attack states
    if (actor_action == InitActorAttack && actor->user.WeaponNum == 0)
        return 0;   // Just let the actor do as it was doing before in this case

    // Target is gone.
    if (actor->user.targetActor == nullptr)
        return 0;

    // zombie is attacking a player
    if (actor_action == InitActorAttack && actor->user.ID == ZOMBIE_RUN_R0 && actor->user.targetActor->user.PlayerP)
    {
        // Don't let zombies shoot at master
        if (GetOwner(actor) == actor->user.targetActor)
            return 0;

        // if this player cannot take damage from this zombie(weapon) return out
        if (!PlayerTakeDamage(actor->user.targetActor->user.PlayerP, actor))
            return 0;
    }

    ASSERT(actor_action != nullptr);

    if (actor_action != InitActorDecide)
    {
        // NOT staying put
        (*actor_action)(actor);
        //CON_Message("DoActorDecide: NOT Staying put");
    }
    else
    {
        // Actually staying put
        NewStateGroup(actor, actor->user.ActorActionSet->Stand);
        //CON_Message("DoActorDecide: Staying put");
    }

    return 0;
}

// Important note: The functions below are being checked for as state identifiers.
// But they are all identical content wise which makes MSVC merge them together into one.
// Assigning 'sw_snd_scratch' different values makes them different so that merging does not occur.
int sw_snd_scratch = 0;


int InitActorAlertNoise(DSWActor* actor)
{
    sw_snd_scratch = 1;
    actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}


int InitActorAmbientNoise(DSWActor* actor)
{
    sw_snd_scratch = 2;
    actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}

int InitActorAttackNoise(DSWActor* actor)
{
    sw_snd_scratch = 3;
    actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}

int InitActorPainNoise(DSWActor* actor)
{
    sw_snd_scratch = 4;
    actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}

int InitActorDieNoise(DSWActor* actor)
{
    sw_snd_scratch = 5;
    actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}

int InitActorExtra1Noise(DSWActor* actor)
{
    sw_snd_scratch = 6;
    actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}

int InitActorExtra2Noise(DSWActor* actor)
{
    sw_snd_scratch = 7;
    actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}

int InitActorExtra3Noise(DSWActor* actor)
{
    sw_snd_scratch = 8;
    actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}

int InitActorExtra4Noise(DSWActor* actor)
{
    sw_snd_scratch = 9;
    actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}

int InitActorExtra5Noise(DSWActor* actor)
{
    sw_snd_scratch = 10;
    actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}

int InitActorExtra6Noise(DSWActor* actor)
{
    sw_snd_scratch = 11;
    actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}


/*
  !AIC KEY - Routines handle moving toward the player.
*/

int InitActorMoveCloser(DSWActor* actor)
{
    actor->user.ActorActionFunc = DoActorMoveCloser;

    if (actor->user.Rot != actor->user.ActorActionSet->Run)
        NewStateGroup(actor, actor->user.ActorActionSet->Run);

    (*actor->user.ActorActionFunc)(actor);

    return 0;
}

int DoActorCantMoveCloser(DSWActor* actor)
{
    actor->user.track = FindTrackToPlayer(actor);

    if (actor->user.track >= 0)
    {
        actor->spr.ang = getangle((Track[actor->user.track].TrackPoint + actor->user.point)->x - actor->spr.pos.X, (Track[actor->user.track].TrackPoint + actor->user.point)->y - actor->spr.pos.Y);

        DoActorSetSpeed(actor, MID_SPEED);
        actor->user.Flags |= (SPR_FIND_PLAYER);

        actor->user.ActorActionFunc = DoActorDecide;
        NewStateGroup(actor, actor->user.ActorActionSet->Run);
    }
    else
    {
        // Try to move closer
        InitActorReposition(actor);
    }
    return 0;
}

int DoActorMoveCloser(DSWActor* actor)
{
    int nx, ny;

    nx = MulScale(actor->spr.xvel, bcos(actor->spr.ang), 14);
    ny = MulScale(actor->spr.xvel, bsin(actor->spr.ang), 14);

    // if cannot move the sprite
    if (!move_actor(actor, nx, ny, 0))
    {
        if (ActorMoveHitReact(actor))
            return 0;

        DoActorCantMoveCloser(actor);
        return 0;
    }

    // Do a noise if ok
    DoActorNoise(ChooseAction(actor->user.Personality->Broadcast), actor);

    // after moving a ways check to see if player is still in sight
    if (actor->user.DistCheck > 550)
    {
        actor->user.DistCheck = 0;

        // If player moved out of sight
        if (!CanSeePlayer(actor))
        {
            // stay put and choose another option
            InitActorDecide(actor);
            return 0;
        }
        else
        {
            // turn to face player
            actor->spr.ang = getangle(actor->user.targetActor->spr.pos.X - actor->spr.pos.X, actor->user.targetActor->spr.pos.Y - actor->spr.pos.Y);
        }
    }

    // Should be a random value test
    if (actor->user.Dist > 512 * 3)
    {
        InitActorDecide(actor);
    }

    return 0;

}

/*
  !AIC - Find tracks of different types.  Toward target, away from target, etc.
*/


int FindTrackToPlayer(DSWActor* actor)
{
    int point, track_dir, track;
    int i, size;
    const uint16_t* type;
    int zdiff;

    static const uint16_t PlayerAbove[] =
    {
        BIT(TT_LADDER),
        BIT(TT_STAIRS),
        BIT(TT_JUMP_UP),
        BIT(TT_TRAVERSE),
        BIT(TT_OPERATE),
        BIT(TT_SCAN)
    };

    static const uint16_t PlayerBelow[] =
    {
        BIT(TT_JUMP_DOWN),
        BIT(TT_STAIRS),
        BIT(TT_TRAVERSE),
        BIT(TT_OPERATE),
        BIT(TT_SCAN)
    };

    static const uint16_t PlayerOnLevel[] =
    {
        BIT(TT_DUCK_N_SHOOT),
        BIT(TT_HIDE_N_SHOOT),
        BIT(TT_TRAVERSE),
        BIT(TT_EXIT),
        BIT(TT_OPERATE),
        BIT(TT_SCAN)
    };

    zdiff = ActorUpperZ(actor->user.targetActor) - (actor->spr.pos.Z - ActorSizeZ(actor) + Z(8));

    if (abs(zdiff) <= Z(20))
    {
        type = PlayerOnLevel;
        size = SIZ(PlayerOnLevel);
    }
    else
    {
        if (zdiff < 0)
        {
            type = PlayerAbove;
            size = SIZ(PlayerAbove);
        }
        else
        {
            type = PlayerBelow;
            size = SIZ(PlayerBelow);
        }
    }


    for (i = 0; i < size; i++)
    {
        track = ActorFindTrack(actor, 1, type[i], &point, &track_dir);

        if (track >= 0)
        {
            actor->user.point = point;
            actor->user.track_dir = track_dir;
            Track[track].flags |= (TF_TRACK_OCCUPIED);

            return track;
        }
    }

    return -1;

}



int FindTrackAwayFromPlayer(DSWActor* actor)
{
    int point, track_dir, track;
    unsigned int i;

    static const int16_t RunAwayTracks[] =
    {
        BIT(TT_EXIT),
        BIT(TT_LADDER),
        BIT(TT_TRAVERSE),
        BIT(TT_STAIRS),
        BIT(TT_JUMP_UP),
        BIT(TT_JUMP_DOWN),
        BIT(TT_DUCK_N_SHOOT),
        BIT(TT_HIDE_N_SHOOT),
        BIT(TT_OPERATE),
        BIT(TT_SCAN)
    };

    for (i = 0; i < SIZ(RunAwayTracks); i++)
    {
        track = ActorFindTrack(actor, -1, RunAwayTracks[i], &point, &track_dir);

        if (track >= 0)
        {
            actor->user.point = point;
            actor->user.track_dir = track_dir;
            Track[track].flags |= (TF_TRACK_OCCUPIED);

            return track;
        }
    }

    return -1;

}


int FindWanderTrack(DSWActor* actor)
{
    int point, track_dir, track;
    unsigned int i;

    static const int16_t WanderTracks[] =
    {
        BIT(TT_DUCK_N_SHOOT),
        BIT(TT_HIDE_N_SHOOT),
        BIT(TT_WANDER),
        BIT(TT_JUMP_DOWN),
        BIT(TT_JUMP_UP),
        BIT(TT_TRAVERSE),
        BIT(TT_STAIRS),
        BIT(TT_LADDER),
        BIT(TT_EXIT),
        BIT(TT_OPERATE)
    };

    for (i = 0; i < SIZ(WanderTracks); i++)
    {
        track = ActorFindTrack(actor, -1, WanderTracks[i], &point, &track_dir);

        if (track >= 0)
        {
            actor->user.point = point;
            actor->user.track_dir = track_dir;
            Track[track].flags |= (TF_TRACK_OCCUPIED);

            return track;
        }
    }

    return -1;
}


int InitActorRunAway(DSWActor* actor)
{
    actor->user.ActorActionFunc = DoActorDecide;
    NewStateGroup(actor, actor->user.ActorActionSet->Run);

    actor->user.track = FindTrackAwayFromPlayer(actor);

    if (actor->user.track >= 0)
    {
        actor->spr.ang = NORM_ANGLE(getangle((Track[actor->user.track].TrackPoint + actor->user.point)->x - actor->spr.pos.X, (Track[actor->user.track].TrackPoint + actor->user.point)->y - actor->spr.pos.Y));
        DoActorSetSpeed(actor, FAST_SPEED);
        actor->user.Flags |= (SPR_RUN_AWAY);
    }
    else
    {
        actor->user.Flags |= (SPR_RUN_AWAY);
        InitActorReposition(actor);
    }

    return 0;
}

int InitActorRunToward(DSWActor* actor)
{
    actor->user.ActorActionFunc = DoActorDecide;
    NewStateGroup(actor, actor->user.ActorActionSet->Run);

    InitActorReposition(actor);
    DoActorSetSpeed(actor, FAST_SPEED);

    return 0;
}

/*
  !AIC - Where actors do their attacks.  There is some special case code throughout
  these.  Both close and long range attacks are handled here by transitioning to
  the correct attack state.
*/


int InitActorAttack(DSWActor* actor)
{
    // zombie is attacking a player
    if (actor->user.ID == ZOMBIE_RUN_R0 && actor->user.targetActor->hasU() && actor->user.targetActor->user.PlayerP)
    {
        // Don't let zombies shoot at master
        if (GetOwner(actor) == actor->user.targetActor)
            return 0;

        // if this player cannot take damage from this zombie(weapon) return out
        if (!PlayerTakeDamage(actor->user.targetActor->user.PlayerP, actor))
            return 0;
    }

    if ((actor->user.targetActor->spr.cstat & CSTAT_SPRITE_TRANSLUCENT))
    {
        InitActorRunAway(actor);
        return 0;
    }

    if (actor->user.targetActor->hasU() && actor->user.targetActor->user.Health <= 0)
    {
        DoActorPickClosePlayer(actor);
        InitActorReposition(actor);
        return 0;
    }

    if (!CanHitPlayer(actor))
    {
        InitActorReposition(actor);
        return 0;
    }

    // if the guy you are after is dead, look for another and
    // reposition
    if (actor->user.targetActor->hasU() && actor->user.targetActor->user.PlayerP &&
        (actor->user.targetActor->user.PlayerP->Flags & PF_DEAD))
    {
        DoActorPickClosePlayer(actor);
        InitActorReposition(actor);
        return 0;
    }

    actor->user.ActorActionFunc = DoActorAttack;

    // move into standing frame
    //NewStateGroup(actor, actor->user.ActorActionSet->Stand);

    // face player when attacking
    actor->spr.ang = NORM_ANGLE(getangle(actor->user.targetActor->spr.pos.X - actor->spr.pos.X, actor->user.targetActor->spr.pos.Y - actor->spr.pos.Y));

    // If it's your own kind, lay off!
    if (actor->user.ID == actor->user.targetActor->user.ID && !actor->user.targetActor->user.PlayerP)
    {
        InitActorRunAway(actor);
        return 0;
    }

    // Hari Kari for Ninja's
    if (actor->user.ActorActionSet->Death2)
    {
        //#define SUICIDE_HEALTH_VALUE 26
#define SUICIDE_HEALTH_VALUE 38
        //#define SUICIDE_HEALTH_VALUE 50

        if (actor->user.Health < SUICIDE_HEALTH_VALUE)
        {
            if (CHOOSE2(100))
            {
                actor->user.ActorActionFunc = DoActorDecide;
                NewStateGroup(actor, actor->user.ActorActionSet->Death2);
                return 0;
            }
        }
    }


    (*actor->user.ActorActionFunc)(actor);

    return 0;
}


int DoActorAttack(DSWActor* actor)
{
    int rand_num;
    int dist,a,b,c;

    DoActorNoise(ChooseAction(actor->user.Personality->Broadcast),actor);

    DISTANCE(actor->spr.pos.X, actor->spr.pos.Y, actor->user.targetActor->spr.pos.X, actor->user.targetActor->spr.pos.Y, dist, a, b, c);

    auto pActor = GetPlayerSpriteNum(actor);
    if ((actor->user.ActorActionSet->CloseAttack[0] && dist < CloseRangeDist(actor, actor->user.targetActor)) ||
        (pActor && pActor->hasU() && pActor->user.WeaponNum == WPN_FIST))      // JBF: added null check
    {
        rand_num = ChooseActionNumber(actor->user.ActorActionSet->CloseAttackPercent);

        NewStateGroup(actor, actor->user.ActorActionSet->CloseAttack[rand_num]);
    }
    else
    {
        ASSERT(actor->user.WeaponNum != 0);

        rand_num = ChooseActionNumber(actor->user.ActorActionSet->AttackPercent);

        ASSERT(rand_num < actor->user.WeaponNum);

        NewStateGroup(actor, actor->user.ActorActionSet->Attack[rand_num]);
        actor->user.ActorActionFunc = DoActorDecide;
    }

    //actor->user.ActorActionFunc = DoActorDecide;

    return 0;
}

int InitActorEvade(DSWActor* actor)
{
    // Evade is same thing as run away except when you get to the end of the track
    // you stop and take up the fight again.

    actor->user.ActorActionFunc = DoActorDecide;
    NewStateGroup(actor, actor->user.ActorActionSet->Run);

    actor->user.track = FindTrackAwayFromPlayer(actor);

    if (actor->user.track >= 0)
    {
        actor->spr.ang = NORM_ANGLE(getangle((Track[actor->user.track].TrackPoint + actor->user.point)->x - actor->spr.pos.X, (Track[actor->user.track].TrackPoint + actor->user.point)->y - actor->spr.pos.Y));
        DoActorSetSpeed(actor, FAST_SPEED);
        // NOT doing a RUN_AWAY
        actor->user.Flags &= ~(SPR_RUN_AWAY);
    }

    return 0;
}

int InitActorWanderAround(DSWActor* actor)
{
    actor->user.ActorActionFunc = DoActorDecide;
    NewStateGroup(actor, actor->user.ActorActionSet->Run);

    DoActorPickClosePlayer(actor);

    actor->user.track = FindWanderTrack(actor);

    if (actor->user.track >= 0)
    {
        actor->spr.ang = getangle((Track[actor->user.track].TrackPoint + actor->user.point)->x - actor->spr.pos.X, (Track[actor->user.track].TrackPoint + actor->user.point)->y - actor->spr.pos.Y);
        DoActorSetSpeed(actor, NORM_SPEED);
    }

    return 0;
}

int InitActorFindPlayer(DSWActor* actor)
{
    actor->user.ActorActionFunc = DoActorDecide;
    NewStateGroup(actor, actor->user.ActorActionSet->Run);

    actor->user.track = FindTrackToPlayer(actor);

    if (actor->user.track >= 0)
    {
        actor->spr.ang = getangle((Track[actor->user.track].TrackPoint + actor->user.point)->x - actor->spr.pos.X, (Track[actor->user.track].TrackPoint + actor->user.point)->y - actor->spr.pos.Y);
        DoActorSetSpeed(actor, MID_SPEED);
        actor->user.Flags |= (SPR_FIND_PLAYER);

        actor->user.ActorActionFunc = DoActorDecide;
        NewStateGroup(actor, actor->user.ActorActionSet->Run);
    }
    else
    {
        InitActorReposition(actor);
    }


    return 0;
}

int InitActorDuck(DSWActor* actor)
{
    int dist;

    if (!actor->user.ActorActionSet->Duck)
    {
        actor->user.ActorActionFunc = DoActorDecide;
        return 0;
    }

    actor->user.ActorActionFunc = DoActorDuck;
    NewStateGroup(actor, actor->user.ActorActionSet->Duck);

    dist = Distance(actor->spr.pos.X, actor->spr.pos.Y, actor->user.targetActor->spr.pos.X, actor->user.targetActor->spr.pos.Y);

    if (dist > 8000)
    {
        actor->user.WaitTics = 190;
    }
    else
    {
        //actor->user.WaitTics = 120;
        actor->user.WaitTics = 60;
    }


    (*actor->user.ActorActionFunc)(actor);

    return 0;
}

int DoActorDuck(DSWActor* actor)
{
    if ((actor->user.WaitTics -= ACTORMOVETICS) < 0)
    {
        NewStateGroup(actor, actor->user.ActorActionSet->Rise);
        actor->user.ActorActionFunc = DoActorDecide;
        actor->user.Flags &= ~(SPR_TARGETED);
    }

    return 0;
}

int DoActorMoveJump(DSWActor* actor)
{
    int nx, ny;

    // Move while jumping

    nx = MulScale(actor->spr.xvel, bcos(actor->spr.ang), 14);
    ny = MulScale(actor->spr.xvel, bsin(actor->spr.ang), 14);

    move_actor(actor, nx, ny, 0L);

    if (!(actor->user.Flags & (SPR_JUMPING|SPR_FALLING)))
    {
        InitActorDecide(actor);
    }

    return 0;
}


Collision move_scan(DSWActor* actor, int ang, int dist, int *stopx, int *stopy, int *stopz)
{
    int nx,ny;
    uint32_t cliptype = CLIPMASK_ACTOR;

    int sang;
    int x, y, z, loz, hiz;
    DSWActor* highActor;
    DSWActor* lowActor;
    sectortype* lo_sectp,* hi_sectp, *ssp;


    // moves out a bit but keeps the sprites original postion/sector.

    // save off position info
    x = actor->spr.pos.X;
    y = actor->spr.pos.Y;
    z = actor->spr.pos.Z;
    sang = actor->spr.ang;
    loz = actor->user.loz;
    hiz = actor->user.hiz;
    lowActor = actor->user.lowActor;
    highActor = actor->user.highActor;
    lo_sectp = actor->user.lo_sectp;
    hi_sectp = actor->user.hi_sectp;
    ssp = actor->sector();

    // do the move
    actor->spr.ang = ang;
    nx = MulScale(dist, bcos(actor->spr.ang), 14);
    ny = MulScale(dist, bsin(actor->spr.ang), 14);

    Collision ret = move_sprite(actor, nx, ny, 0, actor->user.ceiling_dist, actor->user.floor_dist, cliptype, 1);
    // move_sprite DOES do a getzrange point?

    // should I look down with a FAFgetzrange to see where I am?

    // remember where it stopped
    *stopx = actor->spr.pos.X;
    *stopy = actor->spr.pos.Y;
    *stopz = actor->spr.pos.Z;

    // reset position information
    actor->spr.pos.X = x;
    actor->spr.pos.Y = y;
    actor->spr.pos.Z = z;
    actor->spr.ang = sang;
    actor->user.loz = loz;
    actor->user.hiz = hiz;
    actor->user.lowActor = lowActor;
    actor->user.highActor = highActor;
    actor->user.lo_sectp = lo_sectp;
    actor->user.hi_sectp = hi_sectp;
    ChangeActorSect(actor, ssp);

    return ret;
}

enum
{
    TOWARD = 1,
    AWAY = -1
};

int FindNewAngle(DSWActor* actor, int dir, int DistToMove)
{
    static const int16_t toward_angle_delta[4][9] =
    {
        { -160, -384, 160, 384, -256, 256, -512, 512, -99},
        { -384, -160, 384, 160, -256, 256, -512, 512, -99},
        { 160, 384, -160, -384, 256, -256, 512, -512, -99},
        { 384, 160, -384, -160, 256, -256, 512, -512, -99}
    };

    static const int16_t away_angle_delta[4][8] =
    {
        { -768, 768, -640, 640, -896, 896, 1024, -99},
        { 768, -768, 640, -640, -896, 896, 1024, -99},
        { 896, -896, -768, 768, -640, 640, 1024, -99},
        { 896, -896, 768, -768, 640, -640, 1024, -99}
    };


    const int16_t* adp = nullptr;

    int new_ang, oang;
    int save_ang = -1;
    int set;

    int dist, stopx, stopy, stopz;
    // start out with mininum distance that will be accepted as a move
    int save_dist = 500;

    // if on fire, run shorter distances
    if (ActorFlaming(actor))
        DistToMove = (DistToMove >> 2) + (DistToMove >> 3);

    // Find angle to from the player
    oang = NORM_ANGLE(getangle(actor->user.targetActor->spr.pos.X - actor->spr.pos.X, actor->user.targetActor->spr.pos.Y - actor->spr.pos.Y));

    // choose a random angle array
    switch (dir)
    {
    case TOWARD:
        set = RANDOM_P2(4<<8)>>8;
        adp = &toward_angle_delta[set][0];
        break;
    case AWAY:
        set = RANDOM_P2(4<<8)>>8;
        if (CanHitPlayer(actor))
        {
            adp = &toward_angle_delta[set][0];
        }
        else
        {
            adp = &away_angle_delta[set][0];
        }
        break;
    default:
        Printf("FindNewAngle called with dir=%d!\n",dir);
        return 0;
    }

    for (; *adp != -99; adp++)
    {
        new_ang = NORM_ANGLE(oang + *adp);

#if 1
        // look directly ahead for a ledge
        if (!(actor->user.Flags & (SPR_NO_SCAREDZ | SPR_JUMPING | SPR_FALLING | SPR_SWIMMING | SPR_DEAD)))
        {
            actor->spr.ang = new_ang;
            if (DropAhead(actor, actor->user.lo_step))
            {
                actor->spr.ang = oang;
                continue;
            }
            actor->spr.ang = oang;
        }
#endif

        // check to see how far we can move
        auto ret = move_scan(actor, new_ang, DistToMove, &stopx, &stopy, &stopz);

        if (ret.type == kHitNone)
        {
            // cleanly moved in new direction without hitting something
            actor->user.TargetDist = Distance(actor->spr.pos.X, actor->spr.pos.Y, stopx, stopy);
            return new_ang;
        }
        else
        {
            // hit something
            dist = Distance(actor->spr.pos.X, actor->spr.pos.Y, stopx, stopy);

            if (dist > save_dist)
            {
                save_ang = new_ang;
                save_dist = dist;
            }
        }
    }

    if (save_ang != -1)
    {
        actor->user.TargetDist = save_dist;

        // If actor moved to the TargetDist it would look like he was running
        // into things.

        // To keep this from happening make the TargetDist is less than the
        // point you would hit something

        if (actor->user.TargetDist > 4000)
            actor->user.TargetDist -= 3500;

        actor->spr.ang = save_ang;
        return save_ang;
    }

    return -1;
}


/*

  !AIC KEY - Reposition code is called throughout this file.  What this does is
  pick a new direction close to the target direction (or away from the target
  direction if running away) and a distance to move in and tries to move there
  with move_scan(). If it hits something it will try again.  No movement is
  actually acomplished here. This is just testing for clear paths to move in.
  Location variables that are changed are saved and reset.  FindNewAngle() and
  move_scan() are two routines (above) that go with this.  This is definately
  not called every time through the loop.  It would be majorly slow.

*/

int InitActorReposition(DSWActor* actor)
{
    int ang;
    int rnum;
    int dist;

    static const int AwayDist[8] =
    {
        17000,
        20000,
        26000,
        26000,
        26000,
        32000,
        32000,
        42000
    };

    static const int TowardDist[8] =
    {
        10000,
        15000,
        20000,
        20000,
        25000,
        30000,
        35000,
        40000
    };

    static const int16_t PlayerDist[8] =
    {
        2000,
        3000,
        3000,
        5000,
        5000,
        5000,
        9000,
        9000
    };

    actor->user.Dist = 0;

    rnum = RANDOM_P2(8<<8)>>8;
    dist = Distance(actor->spr.pos.X, actor->spr.pos.Y, actor->user.targetActor->spr.pos.X, actor->user.targetActor->spr.pos.Y);

    if (dist < PlayerDist[rnum] || (actor->user.Flags & SPR_RUN_AWAY))
    {
        rnum = RANDOM_P2(8<<8)>>8;
        ang = FindNewAngle(actor, AWAY, AwayDist[rnum]);
        if (ang == -1)
        {
            actor->user.Vis = 8;
            InitActorPause(actor);
            return 0;
        }

        actor->spr.ang = ang;
        DoActorSetSpeed(actor, FAST_SPEED);
        actor->user.Flags &= ~(SPR_RUN_AWAY);
    }
    else
    {
        // try to move toward player
        rnum = RANDOM_P2(8<<8)>>8;
        ang = FindNewAngle(actor, TOWARD, TowardDist[rnum]);
        if (ang == -1)
        {
            // try to move away from player
            rnum = RANDOM_P2(8<<8)>>8;
            ang = FindNewAngle(actor, AWAY, AwayDist[rnum]);
            if (ang == -1)
            {
                actor->user.Vis = 8;
                InitActorPause(actor);
                return 0;
            }
        }
        else
        {
            // pick random speed to move toward the player
            if (RANDOM_P2(1024) < 512)
                DoActorSetSpeed(actor, NORM_SPEED);
            else
                DoActorSetSpeed(actor, MID_SPEED);
        }

        actor->spr.ang = ang;
    }


    actor->user.ActorActionFunc = DoActorReposition;
    if (!(actor->user.Flags & SPR_SWIMMING))
        NewStateGroup(actor, actor->user.ActorActionSet->Run);

    (*actor->user.ActorActionFunc)(actor);

    return 0;
}

int DoActorReposition(DSWActor* actor)
{
    int nx, ny;

    nx = MulScale(actor->spr.xvel, bcos(actor->spr.ang), 14);
    ny = MulScale(actor->spr.xvel, bsin(actor->spr.ang), 14);

    // still might hit something and have to handle it.
    if (!move_actor(actor, nx, ny, 0L))
    {
        if (ActorMoveHitReact(actor))
            return 0;

        actor->user.Vis = 6;
        InitActorPause(actor);
        return 0;
    }

    // if close to target distance do a Decision again
    if (actor->user.TargetDist < 50)
    {
        InitActorDecide(actor);
    }

    return 0;
}


int InitActorPause(DSWActor* actor)
{
    actor->user.ActorActionFunc = DoActorPause;

    (*actor->user.ActorActionFunc)(actor);

    return 0;
}

int DoActorPause(DSWActor* actor)
{
    // Using Vis instead of WaitTics, var name sucks, but it's the same type
    // WaitTics is used by too much other actor code and causes problems here
    if ((actor->user.Vis -= ACTORMOVETICS) < 0)
    {
        actor->user.ActorActionFunc = DoActorDecide;
        actor->user.Flags &= ~(SPR_TARGETED);
    }

    return 0;
}



#include "saveable.h"

static saveable_code saveable_ai_code[] =
{
    SAVE_CODE(InitActorDecide),
    SAVE_CODE(DoActorDecide),
    SAVE_CODE(InitActorAlertNoise),
    SAVE_CODE(InitActorAmbientNoise),
    SAVE_CODE(InitActorAttackNoise),
    SAVE_CODE(InitActorPainNoise),
    SAVE_CODE(InitActorDieNoise),
    SAVE_CODE(InitActorExtra1Noise),
    SAVE_CODE(InitActorExtra2Noise),
    SAVE_CODE(InitActorExtra3Noise),
    SAVE_CODE(InitActorExtra4Noise),
    SAVE_CODE(InitActorExtra5Noise),
    SAVE_CODE(InitActorExtra6Noise),
    SAVE_CODE(InitActorMoveCloser),
    SAVE_CODE(DoActorMoveCloser),
    SAVE_CODE(FindTrackToPlayer),
    SAVE_CODE(FindTrackAwayFromPlayer),
    SAVE_CODE(FindWanderTrack),
    SAVE_CODE(InitActorRunAway),
    SAVE_CODE(InitActorRunToward),
    SAVE_CODE(InitActorAttack),
    SAVE_CODE(DoActorAttack),
    SAVE_CODE(InitActorEvade),
    SAVE_CODE(InitActorWanderAround),
    SAVE_CODE(InitActorFindPlayer),
    SAVE_CODE(InitActorDuck),
    SAVE_CODE(DoActorDuck),
    SAVE_CODE(DoActorMoveJump),
    SAVE_CODE(InitActorReposition),
    SAVE_CODE(DoActorReposition),
    SAVE_CODE(DoActorPause)
};

static saveable_data saveable_ai_data[] =
{
    SAVE_DATA(GenericFlaming)
};

saveable_module saveable_ai =
{
    // code
    saveable_ai_code,
    SIZ(saveable_ai_code),

    // data
    saveable_ai_data,
    SIZ(saveable_ai_data)
};
END_SW_NS
