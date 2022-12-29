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
#include "misc.h"
#include "tags.h"
#include "ai.h"
#include "light.h"
#include "break.h"
#include "network.h"

#include "pal.h"

#include "sounds.h"
#include "interpolate.h"
#include "interpso.h"
#include "sprite.h"
#include "weapon.h"
#include "jsector.h"
#include "misc.h"
#include "player.h"
#include "quotemgr.h"
#include "v_text.h"
#include "gamecontrol.h"
#include "gamefuncs.h"

BEGIN_SW_NS


int SetupCoolie(DSWActor*);
int SetupNinja(DSWActor*);
int SetupGoro(DSWActor*);
int SetupCoolg(DSWActor*);
int SetupEel(DSWActor*);
int SetupSumo(DSWActor*);
int SetupZilla(DSWActor*);
int SetupToiletGirl(DSWActor*);
int SetupWashGirl(DSWActor*);
int SetupCarGirl(DSWActor*);
int SetupMechanicGirl(DSWActor*);
int SetupSailorGirl(DSWActor*);
int SetupPruneGirl(DSWActor*);
int SetupTrashCan(DSWActor*);
int SetupBunny(DSWActor*);
int SetupRipper(DSWActor*);
int SetupRipper2(DSWActor*);
int SetupSerp(DSWActor*);
int SetupLava(DSWActor* actor);
int SetupSkel(DSWActor*);
int SetupHornet(DSWActor*);
int SetupSkull(DSWActor*);
int SetupBetty(DSWActor*);
int SetupPachinkoLight(DSWActor*);
int SetupPachinko1(DSWActor*);
int SetupPachinko2(DSWActor*);
int SetupPachinko3(DSWActor*);
int SetupPachinko4(DSWActor*);
int SetupGirlNinja(DSWActor*);
ANIMATOR DoVator, DoVatorAuto;
ANIMATOR DoRotator;
ANIMATOR DoSlidor;
ANIMATOR DoSpike, DoSpikeAuto;
ANIMATOR DoLavaErupt;
int DoSlidorInstantClose(DSWActor*);

void InitWeaponRocket(PLAYERp);
void InitWeaponUzi(PLAYERp);

bool FAF_Sector(short sectnum);
int MoveSkip4, MoveSkip2, MoveSkip8;
int MinEnemySkill;

extern STATE s_CarryFlag[];
extern STATE s_CarryFlagNoDet[];

// beware of mess... :(
static int globhiz, globloz;
static Collision globhihit, globlohit;

short wait_active_check_offset;
int PlaxCeilGlobZadjust, PlaxFloorGlobZadjust;
void SetSectorWallBits(short sectnum, int bit_mask, bool set_sectwall, bool set_nextwall);
int DoActorDebris(DSWActor* actor);
void ActorWarpUpdatePos(DSWActor*,short sectnum);
void ActorWarpType(DSWActor* sp, DSWActor* act_warp);
int MissileZrange(DSWActor*);

#define ACTIVE_CHECK_TIME (3*120)

TRACK Track[MAX_TRACKS];
SECTOR_OBJECT SectorObject[MAX_SECTOR_OBJECTS];

int DirArr[] = {NORTH, NE, EAST, SE, SOUTH, SW, WEST, NW, NORTH, NE, EAST, SE, SOUTH, SW, WEST, NW};

#define SCROLL_RATE 20
#define SCROLL_FIRE_RATE 20

STATE s_DebrisNinja[] =
{
    {NINJA_DIE + 3, 100, DoActorDebris, &s_DebrisNinja[0]},
};

STATE s_DebrisRat[] =
{
    {750, 100, DoActorDebris, &s_DebrisRat[0]},
};

STATE s_DebrisCrab[] =
{
    {423, 100, DoActorDebris, &s_DebrisCrab[0]},
};

STATE s_DebrisStarFish[] =
{
    {426, 100, DoActorDebris, &s_DebrisStarFish[0]},
};

ANIMATOR DoGet, DoKey, DoSpriteFade;

// temporary
#define ICON_REPAIR_KIT 1813
#define REPAIR_KIT_RATE 1100
STATE s_RepairKit[] =
{
    {ICON_REPAIR_KIT + 0, REPAIR_KIT_RATE, DoGet, &s_RepairKit[0]}
};

STATE s_GoldSkelKey[] =
{
    {GOLD_SKELKEY, 100, DoGet, &s_GoldSkelKey[0]}
};
STATE s_BlueKey[] =
{
    {BLUE_KEY, 100, DoGet, &s_BlueKey[0]}
};
STATE s_BlueCard[] =
{
    {BLUE_CARD, 100, DoGet, &s_BlueCard[0]}
};

STATE s_SilverSkelKey[] =
{
    {SILVER_SKELKEY, 100, DoGet, &s_SilverSkelKey[0]}
};
STATE s_RedKey[] =
{
    {RED_KEY, 100, DoGet, &s_RedKey[0]}
};
STATE s_RedCard[] =
{
    {RED_CARD, 100, DoGet, &s_RedCard[0]}
};

STATE s_BronzeSkelKey[] =
{
    {BRONZE_SKELKEY, 100, DoGet, &s_BronzeSkelKey[0]}
};
STATE s_GreenKey[] =
{
    {GREEN_KEY, 100, DoGet, &s_GreenKey[0]}
};
STATE s_GreenCard[] =
{
    {GREEN_CARD, 100, DoGet, &s_GreenCard[0]}
};

STATE s_RedSkelKey[] =
{
    {RED_SKELKEY, 100, DoGet, &s_RedSkelKey[0]}
};
STATE s_YellowKey[] =
{
    {YELLOW_KEY, 100, DoGet, &s_YellowKey[0]}
};
STATE s_YellowCard[] =
{
    {YELLOW_CARD, 100, DoGet, &s_YellowCard[0]}
};

STATEp s_Key[] =
{
    s_RedKey,
    s_BlueKey,
    s_GreenKey,
    s_YellowKey,
    s_RedCard,
    s_BlueCard,
    s_GreenCard,
    s_YellowCard,
    s_GoldSkelKey,
    s_SilverSkelKey,
    s_BronzeSkelKey,
    s_RedSkelKey
};

#define KEY_RATE 25

#if 0
STATE s_BlueKey[] =
{
    {BLUE_KEY + 0, KEY_RATE, DoKey, &s_BlueKey[0]}
};

STATE s_RedKey[] =
{
    {RED_KEY + 0, KEY_RATE, DoKey, &s_RedKey[0]}
};

STATE s_GreenKey[] =
{
    {GREEN_KEY + 0, KEY_RATE, DoKey, &s_GreenKey[0]}
};

STATE s_YellowKey[] =
{
    {YELLOW_KEY + 0, KEY_RATE, DoKey, &s_YellowKey[0]}
};

STATEp s_Key[] =
{
    s_RedKey,
    s_BlueKey,
    s_GreenKey,
    s_YellowKey,
    s_RedCard,
    s_BlueCard,
    s_GreenCard,
    s_YellowCard,
    s_GoldSkelKey,
    s_SilverSkelKey,
    s_BronzeSkelKey,
    s_RedSkelKey
};
#endif

#if 0
STATE s_BlueKeyStatue[] =
{
    {BLUE_KEY_STATUE + 0, KEY_RATE, DoSpriteFade, &s_BlueKeyStatue[0]}
};

STATE s_RedKeyStatue[] =
{
    {RED_KEY_STATUE + 0, KEY_RATE, DoSpriteFade, &s_RedKeyStatue[0]}
};

STATE s_GreenKeyStatue[] =
{
    {GREEN_KEY_STATUE + 0, KEY_RATE, DoSpriteFade, &s_GreenKeyStatue[0]}
};

STATE s_YellowKeyStatue[] =
{
    {YELLOW_KEY_STATUE + 0, KEY_RATE, DoSpriteFade, &s_YellowKeyStatue[0]}
};

STATEp s_KeyStatue[] =
{
    s_RedKeyStatue,
    s_BlueKeyStatue,
    s_GreenKeyStatue,
    s_YellowKeyStatue,
};
#endif

#define Red_COIN 2440
#define Yellow_COIN 2450
#define Green_COIN 2460
#define RED_COIN_RATE 10
#define YELLOW_COIN_RATE 8
#define GREEN_COIN_RATE 6
ANIMATOR DoCoin;
STATE s_RedCoin[] =
{
    {Red_COIN + 0, RED_COIN_RATE, DoCoin, &s_RedCoin[1]},
    {Red_COIN + 1, RED_COIN_RATE, DoCoin, &s_RedCoin[2]},
    {Red_COIN + 2, RED_COIN_RATE, DoCoin, &s_RedCoin[3]},
    {Red_COIN + 3, RED_COIN_RATE, DoCoin, &s_RedCoin[4]},
    {Red_COIN + 4, RED_COIN_RATE, DoCoin, &s_RedCoin[5]},
    {Red_COIN + 5, RED_COIN_RATE, DoCoin, &s_RedCoin[6]},
    {Red_COIN + 6, RED_COIN_RATE, DoCoin, &s_RedCoin[7]},
    {Red_COIN + 7, RED_COIN_RATE, DoCoin, &s_RedCoin[0]},
};

// !JIM! Frank, I made coins go progressively faster
STATE s_YellowCoin[] =
{
    {Yellow_COIN + 0, YELLOW_COIN_RATE, DoCoin, &s_YellowCoin[1]},
    {Yellow_COIN + 1, YELLOW_COIN_RATE, DoCoin, &s_YellowCoin[2]},
    {Yellow_COIN + 2, YELLOW_COIN_RATE, DoCoin, &s_YellowCoin[3]},
    {Yellow_COIN + 3, YELLOW_COIN_RATE, DoCoin, &s_YellowCoin[4]},
    {Yellow_COIN + 4, YELLOW_COIN_RATE, DoCoin, &s_YellowCoin[5]},
    {Yellow_COIN + 5, YELLOW_COIN_RATE, DoCoin, &s_YellowCoin[6]},
    {Yellow_COIN + 6, YELLOW_COIN_RATE, DoCoin, &s_YellowCoin[7]},
    {Yellow_COIN + 7, YELLOW_COIN_RATE, DoCoin, &s_YellowCoin[0]},
};

STATE s_GreenCoin[] =
{
    {Green_COIN + 0, GREEN_COIN_RATE, DoCoin, &s_GreenCoin[1]},
    {Green_COIN + 1, GREEN_COIN_RATE, DoCoin, &s_GreenCoin[2]},
    {Green_COIN + 2, GREEN_COIN_RATE, DoCoin, &s_GreenCoin[3]},
    {Green_COIN + 3, GREEN_COIN_RATE, DoCoin, &s_GreenCoin[4]},
    {Green_COIN + 4, GREEN_COIN_RATE, DoCoin, &s_GreenCoin[5]},
    {Green_COIN + 5, GREEN_COIN_RATE, DoCoin, &s_GreenCoin[6]},
    {Green_COIN + 6, GREEN_COIN_RATE, DoCoin, &s_GreenCoin[7]},
    {Green_COIN + 7, GREEN_COIN_RATE, DoCoin, &s_GreenCoin[0]},
};

ANIMATOR DoFireFly;

#if 0
STATE s_FireFly[] =
{
    {FIRE_FLY0, 120 * 3, DoFireFly, &s_FireFly[1]},
    {FIRE_FLY1, 20, DoFireFly, &s_FireFly[2]},
    {FIRE_FLY2, 20, DoFireFly, &s_FireFly[3]},
    {FIRE_FLY3, 20, DoFireFly, &s_FireFly[4]},
    {FIRE_FLY4, 60, DoFireFly, &s_FireFly[0]},
};

#else
STATE s_FireFly[] =
{
    {FIRE_FLY0, FIRE_FLY_RATE * 4, DoFireFly, &s_FireFly[0]}
};

#endif


STATE s_IconStar[] =
{
    {ICON_STAR, 100, DoGet, &s_IconStar[0]}
};

STATE s_IconUzi[] =
{
    {ICON_UZI, 100, DoGet, &s_IconUzi[0]}
};

STATE s_IconLgUziAmmo[] =
{
    {ICON_LG_UZI_AMMO, 100, DoGet, &s_IconLgUziAmmo[0]}
};

STATE s_IconUziFloor[] =
{
    {ICON_UZIFLOOR, 100, DoGet, &s_IconUziFloor[0]}
};

STATE s_IconRocket[] =
{
    {ICON_ROCKET, 100, DoGet, &s_IconRocket[0]}
};

STATE s_IconLgRocket[] =
{
    {ICON_LG_ROCKET, 100, DoGet, &s_IconLgRocket[0]}
};

STATE s_IconShotgun[] =
{
    {ICON_SHOTGUN, 100, DoGet, &s_IconShotgun[0]}
};

STATE s_IconLgShotshell[] =
{
    {ICON_LG_SHOTSHELL, 100, DoGet, &s_IconLgShotshell[0]}
};

STATE s_IconAutoRiot[] =
{
    {ICON_AUTORIOT, 100, DoGet, &s_IconAutoRiot[0]}
};

STATE s_IconGrenadeLauncher[] =
{
    {ICON_GRENADE_LAUNCHER, 100, DoGet, &s_IconGrenadeLauncher[0]}
};

STATE s_IconLgGrenade[] =
{
    {ICON_LG_GRENADE, 100, DoGet, &s_IconLgGrenade[0]}
};


STATE s_IconLgMine[] =
{
    {ICON_LG_MINE, 100, DoGet, &s_IconLgMine[0]}
};

STATE s_IconGuardHead[] =
{
    {ICON_GUARD_HEAD + 0, 15, DoGet, &s_IconGuardHead[0]},
//    {ICON_GUARD_HEAD + 1, 15, DoGet, &s_IconGuardHead[2]},
//    {ICON_GUARD_HEAD + 2, 15, DoGet, &s_IconGuardHead[0]}
};


#define FIREBALL_LG_AMMO_RATE 12
STATE s_IconFireballLgAmmo[] =
{
    {ICON_FIREBALL_LG_AMMO + 0, FIREBALL_LG_AMMO_RATE, DoGet, &s_IconFireballLgAmmo[1]},
    {ICON_FIREBALL_LG_AMMO + 1, FIREBALL_LG_AMMO_RATE, DoGet, &s_IconFireballLgAmmo[2]},
    {ICON_FIREBALL_LG_AMMO + 2, FIREBALL_LG_AMMO_RATE, DoGet, &s_IconFireballLgAmmo[0]},
};

STATE s_IconHeart[] =
{
    {ICON_HEART + 0, 25, DoGet, &s_IconHeart[1]},
    {ICON_HEART + 1, 25, DoGet, &s_IconHeart[0]},
};

#define HEART_LG_AMMO_RATE 12
STATE s_IconHeartLgAmmo[] =
{
    {ICON_HEART_LG_AMMO + 0, HEART_LG_AMMO_RATE, DoGet, &s_IconHeartLgAmmo[1]},
    {ICON_HEART_LG_AMMO + 1, HEART_LG_AMMO_RATE, DoGet, &s_IconHeartLgAmmo[0]},
};

STATE s_IconMicroGun[] =
{
    {ICON_MICRO_GUN, 100, DoGet, &s_IconMicroGun[0]}
};

STATE s_IconMicroBattery[] =
{
    {ICON_MICRO_BATTERY, 100, DoGet, &s_IconMicroBattery[0]}
};

// !JIM!  Added rail crap
STATE s_IconRailGun[] =
{
    {ICON_RAIL_GUN, 100, DoGet, &s_IconRailGun[0]}
};

STATE s_IconRailAmmo[] =
{
    {ICON_RAIL_AMMO, 100, DoGet, &s_IconRailAmmo[0]}
};


STATE s_IconElectro[] =
{
    {ICON_ELECTRO + 0, 25, DoGet, &s_IconElectro[1]},
    {ICON_ELECTRO + 1, 25, DoGet, &s_IconElectro[0]},
};

#define ICON_SPELL_RATE 8

STATE s_IconSpell[] =
{
    {ICON_SPELL + 0, ICON_SPELL_RATE, DoGet, &s_IconSpell[1]},
    {ICON_SPELL + 1, ICON_SPELL_RATE, DoGet, &s_IconSpell[2]},
    {ICON_SPELL + 2, ICON_SPELL_RATE, DoGet, &s_IconSpell[3]},
    {ICON_SPELL + 3, ICON_SPELL_RATE, DoGet, &s_IconSpell[4]},
    {ICON_SPELL + 4, ICON_SPELL_RATE, DoGet, &s_IconSpell[5]},
    {ICON_SPELL + 5, ICON_SPELL_RATE, DoGet, &s_IconSpell[6]},
    {ICON_SPELL + 6, ICON_SPELL_RATE, DoGet, &s_IconSpell[7]},
    {ICON_SPELL + 7, ICON_SPELL_RATE, DoGet, &s_IconSpell[8]},
    {ICON_SPELL + 8, ICON_SPELL_RATE, DoGet, &s_IconSpell[9]},
    {ICON_SPELL + 9, ICON_SPELL_RATE, DoGet, &s_IconSpell[10]},
    {ICON_SPELL + 10, ICON_SPELL_RATE, DoGet, &s_IconSpell[11]},
    {ICON_SPELL + 11, ICON_SPELL_RATE, DoGet, &s_IconSpell[12]},
    {ICON_SPELL + 12, ICON_SPELL_RATE, DoGet, &s_IconSpell[13]},
    {ICON_SPELL + 13, ICON_SPELL_RATE, DoGet, &s_IconSpell[14]},
    {ICON_SPELL + 14, ICON_SPELL_RATE, DoGet, &s_IconSpell[15]},
    {ICON_SPELL + 15, ICON_SPELL_RATE, DoGet, &s_IconSpell[0]},
};

STATE s_IconArmor[] =
{
    {ICON_ARMOR + 0, 15, DoGet, &s_IconArmor[0]},
};

STATE s_IconMedkit[] =
{
    {ICON_MEDKIT + 0, 15, DoGet, &s_IconMedkit[0]},
};

STATE s_IconChemBomb[] =
{
    {ICON_CHEMBOMB, 15, DoGet, &s_IconChemBomb[0]},
};

STATE s_IconFlashBomb[] =
{
    {ICON_FLASHBOMB, 15, DoGet, &s_IconFlashBomb[0]},
};

STATE s_IconNuke[] =
{
    {ICON_NUKE, 15, DoGet, &s_IconNuke[0]},
};

STATE s_IconCaltrops[] =
{
    {ICON_CALTROPS, 15, DoGet, &s_IconCaltrops[0]},
};

#define ICON_SM_MEDKIT 1802
STATE s_IconSmMedkit[] =
{
    {ICON_SM_MEDKIT + 0, 15, DoGet, &s_IconSmMedkit[0]},
};

#define ICON_BOOSTER 1810
STATE s_IconBooster[] =
{
    {ICON_BOOSTER + 0, 15, DoGet, &s_IconBooster[0]},
};

#define ICON_HEAT_CARD 1819
STATE s_IconHeatCard[] =
{
    {ICON_HEAT_CARD + 0, 15, DoGet, &s_IconHeatCard[0]},
};

#if 0
STATE s_IconEnvironSuit[] =
{
    {ICON_ENVIRON_SUIT + 0, 20, DoGet, &s_IconEnvironSuit[0]},
};
#endif

STATE s_IconCloak[] =
{
//   {ICON_CLOAK + 0, 20, DoGet, &s_IconCloak[1]},
//   {ICON_CLOAK + 1, 20, DoGet, &s_IconCloak[2]},
    {ICON_CLOAK + 0, 20, DoGet, &s_IconCloak[0]},
};

STATE s_IconFly[] =
{
    {ICON_FLY + 0, 20, DoGet, &s_IconFly[1]},
    {ICON_FLY + 1, 20, DoGet, &s_IconFly[2]},
    {ICON_FLY + 2, 20, DoGet, &s_IconFly[3]},
    {ICON_FLY + 3, 20, DoGet, &s_IconFly[4]},
    {ICON_FLY + 4, 20, DoGet, &s_IconFly[5]},
    {ICON_FLY + 5, 20, DoGet, &s_IconFly[6]},
    {ICON_FLY + 6, 20, DoGet, &s_IconFly[7]},
    {ICON_FLY + 7, 20, DoGet, &s_IconFly[0]}
};

STATE s_IconNightVision[] =
{
    {ICON_NIGHT_VISION + 0, 20, DoGet, &s_IconNightVision[0]},
};

STATE s_IconFlag[] =
{
    {ICON_FLAG + 0, 32, DoGet, &s_IconFlag[1]},
    {ICON_FLAG + 1, 32, DoGet, &s_IconFlag[2]},
    {ICON_FLAG + 2, 32, DoGet, &s_IconFlag[0]}
};

void SetOwner(DSWActor* ownr, DSWActor* child, bool flag)
{
    SPRITEp cp = &child->s();

    if (flag && ownr != nullptr && ownr->hasU())
    {
        SET(ownr->u()->Flags2, SPR2_CHILDREN);
    }

    cp->owner = ownr? ownr->GetSpriteIndex() : -1;
}


DSWActor* GetOwner(DSWActor* child)
{
    if (!child || child->s().owner < 0) return nullptr;
    return &swActors[child->s().owner];
}

void ClearOwner(DSWActor* child)
{
    if (child) child->s().owner = -1;
}

void SetAttach(DSWActor* ownr, DSWActor* child)
{
    if (child && child->hasU() && ownr->hasU())
    {
        USERp cu = child->u();
        SET(ownr->u()->Flags2, SPR2_CHILDREN);
        cu->attachActor = ownr;
    }
}

void KillActor(DSWActor* actor)
{
    SPRITEp sp = &actor->s();
    USERp u = actor->u();
    int i;
    unsigned stat;
    short statnum,sectnum;
    //extern short Zombies;

    ASSERT(!Prediction);

    ASSERT(sp->statnum < MAXSTATUS);

    //////////////////////////////////////////////
    //    Check sounds list to kill attached sounds
    DeleteNoSoundOwner(actor);
    DeleteNoFollowSoundOwner(actor);
    //////////////////////////////////////////////

    if (u)
    {
        PLAYERp pp;
        short pnum;

        // doing a MissileSetPos - don't allow killing
        if (TEST(u->Flags, SPR_SET_POS_DONT_KILL))
            return;

        // for attached sprites that are getable make sure they don't have
        // any Anims attached
        AnimDelete(ANIM_Userz, 0, actor);
        AnimDelete(ANIM_Spritez, 0, actor);
        StopInterpolation(actor->GetSpriteIndex(), Interp_Sprite_Z);

        //if (TEST(u->Flags2, SPR2_DONT_TARGET_OWNER))
        //    Zombies--;

        // adjust sprites attached to sector objects
        if (TEST(u->Flags, SPR_SO_ATTACHED))
        {
            SECTOR_OBJECTp sop;
            short sn, FoundSpriteNdx = -1;

            for (sop = SectorObject; sop < &SectorObject[MAX_SECTOR_OBJECTS]; sop++)
            {
                for (sn = 0; sop->so_actors[sn] != nullptr; sn++)
                {
                    if (sop->so_actors[sn] == actor)
                    {
                        FoundSpriteNdx = sn;
                    }
                }

                if (FoundSpriteNdx >= 0)
                {
                    // back up sn so it points to the last valid sprite num
                    sn--;
                    ASSERT(sop->so_actors[sn] != nullptr);

                    so_stopspriteinterpolation(sop, actor);
                    // replace the one to be deleted with the last ndx
                    sop->so_actors[FoundSpriteNdx] = sop->so_actors[sn];
                    // the last ndx is not -1
                    sop->so_actors[sn] = nullptr;

                    break;
                }
            }

            ASSERT(FoundSpriteNdx >= 0);
        }

        // if a player is dead and watching this sprite
        // reset it.
        TRAVERSE_CONNECT(pnum)
        {
            pp = Player + pnum;

            if (pp->KillerActor != nullptr)
            {
                if (pp->KillerActor == actor)
                {
                    pp->KillerActor = nullptr;;
                }
            }
        }

        // if on a track and died reset the track to non-occupied
        if (sp->statnum == STAT_ENEMY)
        {
            if (u->track != -1)
            {
                if (Track[u->track].flags)
                    RESET(Track[u->track].flags, TF_TRACK_OCCUPIED);
            }
        }

        // if missile is heading for the sprite, the missile need to know
        // that it is already dead
        if (TEST(sp->extra, SPRX_PLAYER_OR_ENEMY))
        {
            USERp mu;
            static short MissileStats[] = {STAT_MISSILE, STAT_MISSILE_SKIP4};

            for (stat = 0; stat < SIZ(MissileStats); stat++)
            {
                SWStatIterator it(MissileStats[stat]);
                while (auto actor = it.Next())
                {
                    if (!actor->hasU()) continue;
                    mu = actor->u();
                    if (mu->WpnGoalActor == actor)
                    {
                        mu->WpnGoalActor = nullptr;
                    }
                }
            }
        }

        // much faster
        if (TEST(u->Flags2, SPR2_CHILDREN))
        //if (TEST(sp->extra, SPRX_CHILDREN))
        {
            // check for children and alert them that the Owner is dead
            // don't bother th check if you've never had children
            for (stat = 0; stat < STAT_DONT_DRAW; stat++)
            {
                SWStatIterator it(stat);
                while (auto itActor = it.Next())
                {
                    if (GetOwner(itActor) == actor)
                    {
                        ClearOwner(itActor);
                    }

                    
                    if (itActor->hasU() && itActor->u()->attachActor == actor)
                    {
                        itActor->u()->attachActor = nullptr;
                    }
                }
            }
        }

        if (sp->statnum == STAT_ENEMY)
        {
            SWStatIterator it(STAT_ENEMY);
            while (auto itActor = it.Next())
            {
                if (itActor->hasU() && itActor->u()->targetActor == actor)
                {
                    DoActorPickClosePlayer(itActor);
                }
            }
        }

        if (u->flameActor != nullptr)
        {
            SetSuicide(u->flameActor);
        }
        actor->clearUser();
    }

    FVector3 pos = GetSoundPos(&actor->s().pos);
    soundEngine->RelinkSound(SOURCE_Actor, &actor->s(), nullptr, &pos);

    deletesprite(actor->GetSpriteIndex());
    // shred your garbage - but not statnum
    statnum = sp->statnum;
    sectnum = sp->sectnum;
    memset(sp, 0xCC, sizeof(SPRITE));
    sp->statnum = statnum;
    sp->sectnum = sectnum;

    // Kill references in all users - slow but unavoidable if we don't want the game to crash on stale pointers.
    for (auto& u : User)
    {
        if (u.Data())
        {
            if (u->highActor == actor) u->highActor = nullptr;
            if (u->lowActor == actor) u->lowActor = nullptr;
            if (u->targetActor == actor) u->targetActor = nullptr;
        }
    }
}

void ChangeState(DSWActor* actor, STATEp statep)
{
    if (!actor->hasU())
        return;

    USERp u = actor->u();
    u->Tics = 0;
    u->State = u->StateStart = statep;
    // Just in case
    PicAnimOff(u->State->Pic);
}

void change_actor_stat(DSWActor* actor, int stat, bool quick)
{

    changespritestat(actor->GetSpriteIndex(), stat);

    if (actor->hasU() && !quick)
    {
        USERp u = actor->u();
        RESET(u->Flags, SPR_SKIP2|SPR_SKIP4);

        if (stat >= STAT_SKIP4_START && stat <= STAT_SKIP4_END)
            SET(u->Flags, SPR_SKIP4);

        if (stat >= STAT_SKIP2_START && stat <= STAT_SKIP2_END)
            SET(u->Flags, SPR_SKIP2);

        switch (stat)
        {
            //case STAT_DEAD_ACTOR:
            //case STAT_ITEM:
            //case STAT_SKIP4:
#if 0
        case STAT_PLAYER0:
        case STAT_PLAYER1:
        case STAT_PLAYER2:
        case STAT_PLAYER3:
        case STAT_PLAYER4:
        case STAT_PLAYER5:
        case STAT_PLAYER6:
        case STAT_PLAYER7:
#endif
        case STAT_ENEMY_SKIP4:
        case STAT_ENEMY:
            // for enemys - create offsets so all enemys don't call FAFcansee at once
            wait_active_check_offset += ACTORMOVETICS*3;
            if (wait_active_check_offset > ACTIVE_CHECK_TIME)
                wait_active_check_offset = 0;
            u->wait_active_check = wait_active_check_offset;
            // don't do a break here
            SET(u->Flags, SPR_SHADOW);
            break;
        }

    }
}

USERp SpawnUser(DSWActor* actor, short id, STATEp state)
{
    SPRITEp sp = &actor->s();

    USERp u;

    ASSERT(!Prediction);

    actor->clearUser();    // make sure to delete old, stale content first!
    actor->allocUser();
    u = actor->u();

    PRODUCTION_ASSERT(u != nullptr);

    // be careful State can be nullptr
    u->State = u->StateStart = state;

    change_actor_stat(actor, sp->statnum);

    u->ID = id;
    u->Health = 100;
    u->WpnGoalActor = nullptr;
    u->attachActor = nullptr;
    u->track = -1;
    u->targetActor = Player[0].Actor();
    u->Radius = 220;
    u->Sibling = -1;
    u->SpriteNum = actor->GetSpriteIndex();
    u->WaitTics = 0;
    u->OverlapZ = Z(4);
    u->bounce = 0;

    u->motion_blur_num = 0;
    u->motion_blur_dist = 256;

    sp->backuppos();
    u->oz = sp->oz;

    u->active_range = MIN_ACTIVE_RANGE;

    // default

    // based on clipmove z of 48 pixels off the floor
    u->floor_dist = Z(48) - Z(28);
    u->ceiling_dist = Z(8);

    // Problem with sprites spawned really close to white sector walls
    // cant do a getzrange there
    // Just put in some valid starting values
    u->loz = sector[sp->sectnum].floorz;
    u->hiz = sector[sp->sectnum].ceilingz;
    u->lowActor = nullptr;
    u->highActor = nullptr;
    u->lo_sectp = &sector[sp->sectnum];
    u->hi_sectp = &sector[sp->sectnum];

    return u;
}

SECT_USERp GetSectUser(short sectnum)
{
    SECT_USERp sectu;

    if (SectUser[sectnum].Data())
        return SectUser[sectnum].Data();

    SectUser[sectnum].Alloc();
    sectu = SectUser[sectnum].Data();

    ASSERT(sectu != nullptr);

    return sectu;
}


DSWActor* SpawnActor(short stat, short id, STATEp state, short sectnum, int x, int y, int z, int init_ang, int vel)
{
    SPRITEp sp;
    USERp u;

    if (sectnum < 0)
        return nullptr;

    ASSERT(!Prediction);

    auto spawnedActor = InsertActor(sectnum, stat);

    sp = &spawnedActor->s();

    sp->pal = 0;
    sp->x = x;
    sp->y = y;
    sp->z = z;
    sp->cstat = 0;

    u = SpawnUser(spawnedActor, id, state);

    // be careful State can be nullptr
    if (u->State)
    {
        sp->picnum = u->State->Pic;
        PicAnimOff(sp->picnum);
    }

    sp->shade = 0;
    sp->xrepeat = 64;
    sp->yrepeat = 64;
    sp->ang = NORM_ANGLE(init_ang);

    sp->xvel = vel;
    sp->zvel = 0;
    sp->lotag = 0L;
    sp->hitag = 0L;
    sp->extra = 0;
    sp->xoffset = 0;
    sp->yoffset = 0;
    sp->clipdist = 0;

    return spawnedActor;
}

void PicAnimOff(short picnum)
{
    short anim_type = TEST(picanm[picnum].sf, PICANM_ANIMTYPE_MASK) >> PICANM_ANIMTYPE_SHIFT;

    ASSERT(picnum >= 0 && picnum < MAXTILES);

    if (!anim_type)
        return;

    RESET(picanm[picnum].sf, PICANM_ANIMTYPE_MASK);
}

bool IconSpawn(SPRITEp sp)
{
    // if multi item and not a modem game
    if (TEST(sp->extra, SPRX_MULTI_ITEM))
    {
        if (numplayers <= 1 || gNet.MultiGameType == MULTI_GAME_COOPERATIVE)
            return false;
    }
    sp->cstat &= ~(CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_BLOCK_HITSCAN | CSTAT_SPRITE_ONE_SIDED | CSTAT_SPRITE_ALIGNMENT_FLOOR);
    return true;
}

bool ActorTestSpawn(DSWActor* actor)
{
    auto sp = &actor->s();
    if (sp->statnum == STAT_DEFAULT && sp->lotag == TAG_SPAWN_ACTOR)
    {
        auto actorNew = InsertActor(sp->sectnum, STAT_DEFAULT);
        int t = actorNew->s().time;  // must be preserved!
        actorNew->s() = *sp;
        actorNew->s().time = t;
        change_actor_stat(actorNew, STAT_SPAWN_TRIGGER);
        RESET(actorNew->s().cstat, CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN);
        return false;
    }

    // Countermeasure for mods that leave the lower skills unpopulated.
    int spawnskill = TEST(sp->extra, SPRX_SKILL);
    if (MinEnemySkill > 0 && spawnskill == MinEnemySkill) spawnskill = 0;

    // Skill ranges from -1 (No Monsters) to 3.
    if (spawnskill > Skill)
    {
        // JBF: hack to fix Wanton Destruction's missing Sumos, Serpents, and Zilla on Skill < 2
        if (((sp->picnum == SERP_RUN_R0 || sp->picnum == SUMO_RUN_R0) && sp->lotag > 0 &&
             sp->lotag != TAG_SPAWN_ACTOR && sp->extra > 0) || sp->picnum == ZILLA_RUN_R0)
        {
            const char *c;

            // NOTE: Wanton's $boat.map has two sumos, neither of which actually activate
            // anything but are spawned in, and one of them has a skill level 2 mask. This
            // hack however forces both sumos to appear on the easy levels. Bummer.
            switch (sp->picnum)
            {
            case SERP_RUN_R0: c = "serpent"; break;
            case SUMO_RUN_R0: c = "sumo"; break;
            case ZILLA_RUN_R0: c = "zilla"; break;
            default: c = "?"; break;
            }
            Printf("WARNING: skill-masked %s at %d,%d,%d not being killed because it "
                        "activates something\n", c, sp->x, sp->y, sp->z);
            return true;
        }
        //always spawn girls in addons
        if ((sp->picnum == TOILETGIRL_R0 ||
            sp->picnum == WASHGIRL_R0 ||
            sp->picnum == MECHANICGIRL_R0 ||
            sp->picnum == CARGIRL_R0 ||
            sp->picnum == PRUNEGIRL_R0 ||
            sp->picnum == SAILORGIRL_R0) && (g_gameType & GAMEFLAG_ADDON)) return true;

        // spawn Bouncing Betty (mine) in TD map 09 Warehouse 
        if (sp->picnum == 817 && (currentLevel->flags & LEVEL_SW_SPAWNMINES))
            return true;

        return false;
    }

    return true;
}


int EnemyCheckSkill()
{
    SWStatIterator it(STAT_DEFAULT);
    int maxskill = INT_MAX;
    while (auto actor = it.Next())
    {
        auto sp = &actor->s();

        switch (sp->picnum)
        {
        case COOLIE_RUN_R0:
        case NINJA_RUN_R0:
        case NINJA_CRAWL_R0:
        case GORO_RUN_R0:
        case 1441:
        case COOLG_RUN_R0:
        case EEL_RUN_R0:
        case SUMO_RUN_R0:
        case ZILLA_RUN_R0:
        case RIPPER_RUN_R0:
        case RIPPER2_RUN_R0:
        case SERP_RUN_R0:
        case LAVA_RUN_R0:
        case SKEL_RUN_R0:
        case HORNET_RUN_R0:
        case SKULL_R0:
        case BETTY_R0:
        case GIRLNINJA_RUN_R0:
        {
            int myskill = sp->extra & SPRX_SKILL;
            if (myskill < maxskill) maxskill = myskill;
        }
        }
    }
    if (maxskill < 0 || maxskill == INT_MAX) maxskill = 0;
    return maxskill;
}

bool ActorSpawn(DSWActor* actor)
{
    bool ret = true;
    int picnum = actor->s().picnum;
    switch (picnum)
    {
    case COOLIE_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupCoolie(actor);

        break;
    }

    case NINJA_RUN_R0:
    case NINJA_CRAWL_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupNinja(actor);

        break;
    }

    case GORO_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupGoro(actor);
        break;
    }

    case 1441:
    case COOLG_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupCoolg(actor);
        break;
    }

    case EEL_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupEel(actor);
        break;
    }

    case SUMO_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupSumo(actor);

        break;
    }

    case ZILLA_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupZilla(actor);

        break;
    }

    case TOILETGIRL_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupToiletGirl(actor);

        break;
    }

    case WASHGIRL_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupWashGirl(actor);

        break;
    }

    case CARGIRL_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupCarGirl(actor);

        break;
    }

    case MECHANICGIRL_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupMechanicGirl(actor);

        break;
    }

    case SAILORGIRL_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupSailorGirl(actor);

        break;
    }

    case PRUNEGIRL_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupPruneGirl(actor);

        break;
    }

    case TRASHCAN:
    {
        PicAnimOff(picnum);
        SetupTrashCan(actor);
        break;
    }

    case BUNNY_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupBunny(actor);
        break;
    }

    case RIPPER_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupRipper(actor);
        break;
    }

    case RIPPER2_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupRipper2(actor);
        break;
    }

    case SERP_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupSerp(actor);
        break;
    }

    case LAVA_RUN_R0:
    {

        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupLava(actor);
        break;
    }

    case SKEL_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupSkel(actor);
        break;
    }

    case HORNET_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupHornet(actor);
        break;
    }

    case SKULL_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupSkull(actor);
        break;
    }

    case BETTY_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupBetty(actor);
        break;
    }

    case 623:   // Pachinko win light
    {
        PicAnimOff(picnum);
        SetupPachinkoLight(actor);
        break;
    }

    case PACHINKO1:
    {
        PicAnimOff(picnum);
        SetupPachinko1(actor);
        break;
    }

    case PACHINKO2:
    {
        PicAnimOff(picnum);
        SetupPachinko2(actor);
        break;
    }

    case PACHINKO3:
    {
        PicAnimOff(picnum);
        SetupPachinko3(actor);
        break;
    }

    case PACHINKO4:
    {
        PicAnimOff(picnum);
        SetupPachinko4(actor);
        break;
    }

    case GIRLNINJA_RUN_R0:
    {
        if (!ActorTestSpawn(actor))
        {
            KillActor(actor);
            return false;
        }

        PicAnimOff(picnum);
        SetupGirlNinja(actor);

        break;
    }

    default:
        ret = false;
        break;
    }

    return ret;
}


void IconDefault(DSWActor* actor)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();

    //if (sp->statnum == STAT_ITEM)
    change_actor_stat(actor, STAT_ITEM);

    RESET(sp->cstat, CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_BLOCK_HITSCAN);
    u->Radius = 650;

    DoActorZrange(actor);
}

void PreMapCombineFloors(void)
{
    const int MAX_FLOORS = 32;
    SPRITEp sp;
    int i, j, k;
    int base_offset;
    int dx, dy;
    int dasect, startwall, endwall, nextsector;
    short pnum;

    typedef struct
    {
        SPRITEp offset;
    } BOUND_LIST;

    BOUND_LIST BoundList[MAX_FLOORS];

    memset(BoundList, 0, MAX_FLOORS * sizeof(BOUND_LIST));

    SWStatIterator it(0);
    while (auto actor = it.Next())
    {
        USERp u = actor->u();
        SPRITEp sp = &actor->s();

        if (sp->picnum != ST1)
            continue;

        if (sp->hitag == BOUND_FLOOR_OFFSET || sp->hitag == BOUND_FLOOR_BASE_OFFSET)
        {
            ASSERT(sp->lotag < MAX_FLOORS);
            BoundList[sp->lotag].offset = sp;
            change_actor_stat(actor, STAT_FAF);
        }
    }

    for (i = base_offset = 0; i < MAX_FLOORS; i++)
    {
        // blank so continue
        if (!BoundList[i].offset)
            continue;

        if (BoundList[i].offset->hitag == BOUND_FLOOR_BASE_OFFSET)
        {
            base_offset = i;
            continue;
        }

        dx = BoundList[base_offset].offset->x - BoundList[i].offset->x;
        dy = BoundList[base_offset].offset->y - BoundList[i].offset->y;

        BFSSearch search(numsectors, BoundList[i].offset->sectnum);
        for (unsigned dasect; (dasect = search.GetNext()) != BFSSearch::EOL;)
        {
            SWSectIterator it(dasect);
            while (auto jActor = it.Next())
            {
                jActor->s().x += dx;
                jActor->s().y += dy;
            }

            for (auto& wal : wallsofsector(dasect))
            {
                wal.x += dx;
                wal.y += dy;

                nextsector = wal.nextsector;
                if (nextsector >= 0)
                    search.Add(nextsector);
            }
        }

        TRAVERSE_CONNECT(pnum)
        {
            PLAYERp pp = &Player[pnum];
            dasect = pp->cursectnum;
            search.Rewind();
            for (unsigned itsect; (itsect = search.GetNext()) != BFSSearch::EOL;)
            {
                if (itsect == dasect)
                {
                    pp->posx += dx;
                    pp->posy += dy;
                    pp->oposx = pp->oldposx = pp->posx;
                    pp->oposy = pp->oldposy = pp->posy;
                    break;
                }
            }
        }

    }

    // get rid of the sprites used
    it.Reset(STAT_FAF);
    while (auto actor = it.Next())
    {
        KillActor(actor);
    }
}

#if 0
// example of way to traverse through sectors from closest to farthest
void TraverseSectors(short start_sect)
{
    int i, j, k;
    short sectlist[M AXSECTORS];
    short sectlistplc, sectlistend, sect, startwall, endwall, nextsector;

    sectlist[0] = start_sect;
    sectlistplc = 0; sectlistend = 1;
    while (sectlistplc < sectlistend)
    {
        sect = sectlist[sectlistplc++];

        startwall = sector[sect].wallptr;
        endwall = startwall + sector[sect].wallnum;

        for (j=startwall; j<endwall; j++)
        {
            nextsector = wall[j].nextsector;

            if (nextsector < 0)
                continue;

            // make sure its not on the list
            for (k = sectlistend-1; k >= 0; k--)
            {
                if (sectlist[k] == nextsector)
                    break;
            }

            // if its not on the list add it to the end
            if (k < 0)
                sectlist[sectlistend++] = nextsector;
        }

    }

    // list is finished - can now traverse it
#if 0
    sect = pp->cursectnum;
    for (j=0; j<sectlistend; j++)
    {
        if (sectlist[j] == sect)
        {
            break;
        }
    }
#endif
}
#endif


void SpriteSetupPost(void)
{
    SPRITEp ds;
    USERp u;
    int cz,fz;

    // Post processing of some sprites after gone through the main SpriteSetup()
    // routine

    SWStatIterator it(STAT_FLOOR_PAN);
    while (auto iActor = it.Next())
    {
        SWSectIterator it2(iActor->s().sectnum);
        while (auto jActor = it.Next())
        {
            ds = &jActor->s();

            if (ds->picnum == ST1)
                continue;

            if (TEST(ds->cstat, CSTAT_SPRITE_ALIGNMENT_WALL|CSTAT_SPRITE_ALIGNMENT_FLOOR))
                continue;

            if (jActor->hasU())
                continue;

            getzsofslope(ds->sectnum, ds->x, ds->y, &cz, &fz);
            if (labs(ds->z - fz) > Z(4))
                continue;

            u = SpawnUser(jActor, 0, nullptr);
            change_actor_stat(jActor, STAT_NO_STATE);
            u->ceiling_dist = Z(4);
            u->floor_dist = -Z(2);

            u->ActorActionFunc = DoActorDebris;

            SET(ds->cstat, CSTAT_SPRITE_BREAKABLE);
            SET(ds->extra, SPRX_BREAKABLE);
        }
    }
}


void SpriteSetup(void)
{
    short num;
    int cz,fz;

    MinEnemySkill = EnemyCheckSkill();

    // special case for player
    PicAnimOff(PLAYER_NINJA_RUN_R0);

    // Clear Sprite Extension structure

    // Clear all extra bits - they are set by sprites
    for (int i = 0; i < numsectors; i++)
    {
        SectUser[i].Clear();
        sector[i].extra = 0;
    }

    // Clear PARALLAX_LEVEL overrides
    parallaxyscale_override = 0;
    pskybits_override = -1;

    // Call my little sprite setup routine first
    JS_SpriteSetup();

    int minEnemySkill = EnemyCheckSkill();

    SWStatIterator it(STAT_DEFAULT);
    while (auto actor = it.Next())
    {
        USERp u = actor->u();
        SPRITEp sp = &actor->s();

        // not used yetv
        getzsofslope(sp->sectnum, sp->x, sp->y, &cz, &fz);
        if (sp->z > DIV2(cz + fz))
        {
            // closer to a floor
            SET(sp->cstat, CSTAT_SPRITE_CLOSE_FLOOR);
        }

        // CSTAT_SPIN is insupported - get rid of it
        if (TEST(sp->cstat, CSTAT_SPRITE_ALIGNMENT) == CSTAT_SPRITE_ALIGNMENT_SLAB)
            RESET(sp->cstat, CSTAT_SPRITE_ALIGNMENT_SLAB);

        // if BLOCK is set set BLOCK_HITSCAN
        // Hope this doesn't screw up anything
        if (TEST(sp->cstat, CSTAT_SPRITE_BLOCK))
            SET(sp->cstat, CSTAT_SPRITE_BLOCK_HITSCAN);

        ////////////////////////////////////////////
        //
        // BREAKABLE CHECK
        //
        ////////////////////////////////////////////

        // USER SETUP - TAGGED BY USER
        // Non ST1 sprites that are tagged like them
        if (TEST_BOOL1(sp) && sp->picnum != ST1)
        {
            RESET(sp->extra,
                  SPRX_BOOL4|
                  SPRX_BOOL5|
                  SPRX_BOOL6|
                  SPRX_BOOL7|
                  SPRX_BOOL8|
                  SPRX_BOOL9|
                  SPRX_BOOL10);

            switch (sp->hitag)
            {
            case BREAKABLE:
                // need something that tells missiles to hit them
                // but allows actors to move through them
                sp->clipdist = SPRITEp_SIZE_X(sp);
                SET(sp->extra, SPRX_BREAKABLE);
                SET(sp->cstat, CSTAT_SPRITE_BREAKABLE);
                break;
            }
        }
        else
        {
            // BREAK SETUP TABLE AUTOMATED
            SetupSpriteForBreak(actor);
        }

        if (sp->lotag == TAG_SPRITE_HIT_MATCH)
        {
            // if multi item and not a modem game
            if (TEST(sp->extra, SPRX_MULTI_ITEM))
            {
                if (numplayers <= 1 || gNet.MultiGameType == MULTI_GAME_COOPERATIVE)
                {
                    KillActor(actor);
                    continue;
                }
            }


            // crack sprite
            if (sp->picnum == 80)
            {
                RESET(sp->cstat, CSTAT_SPRITE_BLOCK);
                SET(sp->cstat, CSTAT_SPRITE_BLOCK_HITSCAN|CSTAT_SPRITE_BLOCK_MISSILE);;
            }
            else
            {
                RESET(sp->cstat, CSTAT_SPRITE_BLOCK);
                SET(sp->cstat, CSTAT_SPRITE_BLOCK_HITSCAN|CSTAT_SPRITE_BLOCK_MISSILE);;
                SET(sp->cstat, CSTAT_SPRITE_INVISIBLE);;
            }

            if (TEST(SP_TAG8(sp), BIT(0)))
                SET(sp->cstat, CSTAT_SPRITE_INVISIBLE); ;

            if (TEST(SP_TAG8(sp), BIT(1)))
                RESET(sp->cstat, CSTAT_SPRITE_INVISIBLE);

            change_actor_stat(actor, STAT_SPRITE_HIT_MATCH);
            continue;
        }

        if (sp->picnum >= TRACK_SPRITE &&
            sp->picnum <= TRACK_SPRITE + MAX_TRACKS)
        {
            short track_num;

            // skip this sprite, just for numbering walls/sectors
            if (TEST(sp->cstat, CSTAT_SPRITE_ALIGNMENT_WALL))
                continue;

            track_num = sp->picnum - TRACK_SPRITE + 0;

            change_actor_stat(actor, STAT_TRACK + track_num);

            continue;
        }

        if (ActorSpawn(actor))
            continue;

        switch (sp->picnum)
        {
        case ST_QUICK_JUMP:
            change_actor_stat(actor, STAT_QUICK_JUMP);
            break;
        case ST_QUICK_JUMP_DOWN:
            change_actor_stat(actor, STAT_QUICK_JUMP_DOWN);
            break;
        case ST_QUICK_SUPER_JUMP:
            change_actor_stat(actor, STAT_QUICK_SUPER_JUMP);
            break;
        case ST_QUICK_SCAN:
            change_actor_stat(actor, STAT_QUICK_SCAN);
            break;
        case ST_QUICK_EXIT:
            change_actor_stat(actor, STAT_QUICK_EXIT);
            break;
        case ST_QUICK_OPERATE:
            change_actor_stat(actor, STAT_QUICK_OPERATE);
            break;
        case ST_QUICK_DUCK:
            change_actor_stat(actor, STAT_QUICK_DUCK);
            break;
        case ST_QUICK_DEFEND:
            change_actor_stat(actor, STAT_QUICK_DEFEND);
            break;

        case ST1:
        {
            SECT_USERp sectu;
            short tag;
            short bit;

            // get rid of defaults
            if (SP_TAG3(sp) == 32)
                SP_TAG3(sp) = 0;

            tag = sp->hitag;

            RESET(sp->cstat, CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN);
            SET(sp->cstat, CSTAT_SPRITE_INVISIBLE);

            // for bounding sector objects
            if ((tag >= 500 && tag < 600) || tag == SECT_SO_CENTER)
            {
                // NOTE: These will get deleted by the sector object
                // setup code
                change_actor_stat(actor, STAT_ST1);
                break;
            }

            if (tag < 16)
            {
                bit = 1 << (tag);

                SET(sector[sp->sectnum].extra, bit);

                if (TEST(bit, SECTFX_SINK))
                {
                    sectu = GetSectUser(sp->sectnum);
                    sectu->depth_fixed = IntToFixed(sp->lotag);
                    KillActor(actor);
                }
                else if (TEST(bit, SECTFX_OPERATIONAL))
                {
                    KillActor(actor);
                }
                else if (TEST(bit, SECTFX_CURRENT))
                {
                    sectu = GetSectUser(sp->sectnum);
                    sectu->speed = sp->lotag;
                    sectu->ang = sp->ang;
                    KillActor(actor);
                }
                else if (TEST(bit, SECTFX_NO_RIDE))
                {
                    change_actor_stat(actor, STAT_NO_RIDE);
                }
                else if (TEST(bit, SECTFX_DIVE_AREA))
                {
                    sectu = GetSectUser(sp->sectnum);
                    sectu->number = sp->lotag;
                    change_actor_stat(actor, STAT_DIVE_AREA);
                }
                else if (TEST(bit, SECTFX_UNDERWATER))
                {
                    sectu = GetSectUser(sp->sectnum);
                    sectu->number = sp->lotag;
                    change_actor_stat(actor, STAT_UNDERWATER);
                }
                else if (TEST(bit, SECTFX_UNDERWATER2))
                {
                    sectu = GetSectUser(sp->sectnum);
                    sectu->number = sp->lotag;
                    if (sp->clipdist == 1)
                        SET(sectu->flags, SECTFU_CANT_SURFACE);
                    change_actor_stat(actor, STAT_UNDERWATER2);
                }
            }
            else
            {
                switch (tag)
                {
#if 0
                case MULTI_PLAYER_START:
                    change_actor_stat(actor, STAT_MULTI_START + sp->lotag);
                    break;
                case MULTI_COOPERATIVE_START:
                    change_actor_stat(actor, STAT_CO_OP_START + sp->lotag);
                    break;
#endif

                case SECT_MATCH:
                    sectu = GetSectUser(sp->sectnum);

                    sectu->number = sp->lotag;

                    KillActor(actor);
                    break;

                case SLIDE_SECTOR:
                    sectu = GetSectUser(sp->sectnum);
                    SET(sectu->flags, SECTFU_SLIDE_SECTOR);
                    sectu->speed = SP_TAG2(sp);
                    KillActor(actor);
                    break;

                case SECT_DAMAGE:
                {
                    sectu = GetSectUser(sp->sectnum);
                    if (TEST_BOOL1(sp))
                        SET(sectu->flags, SECTFU_DAMAGE_ABOVE_SECTOR);
                    sectu->damage = sp->lotag;
                    KillActor(actor);
                    break;
                }

                case PARALLAX_LEVEL:
                {
                    parallaxyscale_override = 8192;
                    pskybits_override = sp->lotag;
                    if (SP_TAG4(sp) > 2048)
                        parallaxyscale_override = SP_TAG4(sp);
                    KillActor(actor);
                    break;
                }

                case BREAKABLE:
                    // used for wall info
                    change_actor_stat(actor, STAT_BREAKABLE);
                    break;

                case SECT_DONT_COPY_PALETTE:
                {
                    sectu = GetSectUser(sp->sectnum);

                    SET(sectu->flags, SECTFU_DONT_COPY_PALETTE);
                    KillActor(actor);
                    break;
                }

                case SECT_FLOOR_PAN:
                {
                    // if moves with SO
                    if (TEST_BOOL1(sp))
                        sp->xvel = 0;
                    else
                        sp->xvel = sp->lotag;

                    StartInterpolation(sp->sectnum, Interp_Sect_FloorPanX);
                    StartInterpolation(sp->sectnum, Interp_Sect_FloorPanY);
                    change_actor_stat(actor, STAT_FLOOR_PAN);
                    break;
                }

                case SECT_CEILING_PAN:
                {
                    // if moves with SO
                    if (TEST_BOOL1(sp))
                        sp->xvel = 0;
                    else
                        sp->xvel = sp->lotag;
                    StartInterpolation(sp->sectnum, Interp_Sect_CeilingPanX);
                    StartInterpolation(sp->sectnum, Interp_Sect_CeilingPanY);
                    change_actor_stat(actor, STAT_CEILING_PAN);
                    break;
                }

                case SECT_WALL_PAN_SPEED:
                {
                    vec3_t hit_pos = { sp->x, sp->y, sp->z - Z(8) };
                    hitdata_t hitinfo;

                    hitscan(&hit_pos, sp->sectnum,    // Start position
                            bcos(sp->ang),    // X vector of 3D ang
                            bsin(sp->ang),    // Y vector of 3D ang
                            0,                // Z vector of 3D ang
                            &hitinfo, CLIPMASK_MISSILE);

                    if (hitinfo.wall == -1)
                    {
                        KillActor(actor);
                        break;
                    }

                    sp->owner = hitinfo.wall;
                    // if moves with SO
                    if (TEST_BOOL1(sp))
                        sp->xvel = 0;
                    else
                        sp->xvel = sp->lotag;
                    sp->ang = SP_TAG6(sp);
                    // attach to the sector that contains the wall
                    ChangeActorSect(actor, hitinfo.sect);
                    StartInterpolation(hitinfo.wall, Interp_Wall_PanX);
                    StartInterpolation(hitinfo.wall, Interp_Wall_PanY);
                    change_actor_stat(actor, STAT_WALL_PAN);
                    break;
                }

                case WALL_DONT_STICK:
                {
                    vec3_t hit_pos = { sp->x, sp->y, sp->z - Z(8) };
                    hitdata_t hitinfo;

                    hitscan(&hit_pos, sp->sectnum,    // Start position
                            bcos(sp->ang),    // X vector of 3D ang
                            bcos(sp->ang),    // Y vector of 3D ang
                            0,                // Z vector of 3D ang
                            &hitinfo, CLIPMASK_MISSILE);

                    if (hitinfo.wall == -1)
                    {
                        KillActor(actor);
                        break;
                    }

                    SET(wall[hitinfo.wall].extra, WALLFX_DONT_STICK);
                    KillActor(actor);
                    break;
                }

                case TRIGGER_SECTOR:
                {
                    SET(sector[sp->sectnum].extra, SECTFX_TRIGGER);
                    change_actor_stat(actor, STAT_TRIGGER);
                    break;
                }

                case DELETE_SPRITE:
                {
                    change_actor_stat(actor, STAT_DELETE_SPRITE);
                    break;
                }

                case SPAWN_ITEMS:
                {
                    if (TEST(sp->extra, SPRX_MULTI_ITEM))
                    {
                        if (numplayers <= 1 || gNet.MultiGameType == MULTI_GAME_COOPERATIVE)
                        {
                            KillActor(actor);
                            break;
                        }
                    }


                    change_actor_stat(actor, STAT_SPAWN_ITEMS);
                    break;
                }

                case CEILING_FLOOR_PIC_OVERRIDE:
                {
                    // block hitscans depending on translucency
                    if (SP_TAG7(sp) == 0 || SP_TAG7(sp) == 1)
                    {
                        if (SP_TAG3(sp) == 0)
                            SET(sector[sp->sectnum].ceilingstat, CEILING_STAT_FAF_BLOCK_HITSCAN);
                        else
                            SET(sector[sp->sectnum].floorstat, FLOOR_STAT_FAF_BLOCK_HITSCAN);
                    }
                    else if (TEST_BOOL1(sp))
                    {
                        if (SP_TAG3(sp) == 0)
                            SET(sector[sp->sectnum].ceilingstat, CEILING_STAT_FAF_BLOCK_HITSCAN);
                        else
                            SET(sector[sp->sectnum].floorstat, FLOOR_STAT_FAF_BLOCK_HITSCAN);
                    }

                    // copy tag 7 to tag 6 and pre-shift it
                    SP_TAG6(sp) = SP_TAG7(sp);
                    SP_TAG6(sp) <<= 7;
                    change_actor_stat(actor, STAT_CEILING_FLOOR_PIC_OVERRIDE);
                    break;
                }

                case QUAKE_SPOT:
                {
                    change_actor_stat(actor, STAT_QUAKE_SPOT);
                    //SP_TAG13(sp) = (SP_TAG6(sp)*10L) * 120L;
                    SET_SP_TAG13(sp, ((SP_TAG6(sp)*10L) * 120L));
                    break;
                }

                case SECT_CHANGOR:
                {
                    change_actor_stat(actor, STAT_CHANGOR);
                    break;
                }

#if 0
                case SECT_DEBRIS_SEWER:
                {
                    ANIMATOR DoGenerateSewerDebris;

                    u = SpawnUser(actor, 0, nullptr);

                    ASSERT(u != nullptr);
                    u->RotNum = 0;
                    u->WaitTics = sp->lotag * 120;

                    u->ActorActionFunc = DoGenerateSewerDebris;

                    change_actor_stat(actor, STAT_NO_STATE);
                    break;
                }
#endif

                case SECT_VATOR:
                {
                    SECTORp sectp = &sector[sp->sectnum];
                    SECT_USERp sectu;
                    short speed,vel,time,type,start_on,floor_vator;
                    u = SpawnUser(actor, 0, nullptr);

                    // vator already set - ceiling AND floor vator
                    if (TEST(sectp->extra, SECTFX_VATOR))
                    {
                        sectu = GetSectUser(sp->sectnum);
                        SET(sectu->flags, SECTFU_VATOR_BOTH);
                    }
                    SET(sectp->extra, SECTFX_VATOR);
                    SetSectorWallBits(sp->sectnum, WALLFX_DONT_STICK, true, true);
                    SET(sector[sp->sectnum].extra, SECTFX_DYNAMIC_AREA);

                    // don't step on toes of other sector settings
                    if (sectp->lotag == 0 && sectp->hitag == 0)
                        sectp->lotag = TAG_VATOR;

                    type = SP_TAG3(sp);
                    speed = SP_TAG4(sp);
                    vel = SP_TAG5(sp);
                    time = SP_TAG9(sp);
                    start_on = !!TEST_BOOL1(sp);
                    floor_vator = true;
                    if (TEST(sp->cstat, CSTAT_SPRITE_YFLIP))
                        floor_vator = false;

                    u->jump_speed = u->vel_tgt = speed;
                    u->vel_rate = vel;
                    u->WaitTics = time*15; // 1/8 of a sec
                    u->Tics = 0;

                    SET(u->Flags, SPR_ACTIVE);

                    switch (type)
                    {
                    case 0:
                        RESET(u->Flags, SPR_ACTIVE);
                        u->ActorActionFunc = DoVator;
                        break;
                    case 1:
                        RESET(u->Flags, SPR_ACTIVE);
                        u->ActorActionFunc = DoVator;
                        break;
                    case 2:
                        u->ActorActionFunc = DoVatorAuto;
                        break;
                    case 3:
                        RESET(u->Flags, SPR_ACTIVE);
                        u->ActorActionFunc = DoVatorAuto;
                        break;
                    }

                    if (floor_vator)
                    {
                        // start off
                        u->sz = sectp->floorz;
                        u->z_tgt = sp->z;
                        if (start_on)
                        {
                            int amt;
                            amt = sp->z - sectp->floorz;

                            // start in the on position
                            //sectp->floorz = sp->z;
                            sectp->floorz += amt;
                            u->z_tgt = u->sz;

                            MoveSpritesWithSector(sp->sectnum, amt, false); // floor
                        }

                        // set orig z
                        u->oz = sp->oz = sectp->floorz;
                    }
                    else
                    {
                        // start off
                        u->sz = sectp->ceilingz;
                        u->z_tgt = sp->z;
                        if (start_on)
                        {
                            int amt;
                            amt = sp->z - sectp->ceilingz;

                            // starting in the on position
                            //sectp->ceilingz = sp->z;
                            sectp->ceilingz += amt;
                            u->z_tgt = u->sz;

                            MoveSpritesWithSector(sp->sectnum, amt, true); // ceiling
                        }

                        // set orig z
                        u->oz = sp->oz = sectp->ceilingz;
                    }


                    change_actor_stat(actor, STAT_VATOR);
                    break;
                }

                case SECT_ROTATOR_PIVOT:
                {
                    change_actor_stat(actor, STAT_ROTATOR_PIVOT);
                    break;
                }

                case SECT_ROTATOR:
                {
					SECTORp sectp = &sector[sp->sectnum];
                    short time,type;
                    short wallcount,startwall,endwall,w;
                    u = SpawnUser(actor, 0, nullptr);

                    SetSectorWallBits(sp->sectnum, WALLFX_DONT_STICK, true, true);

                    // need something for this
                    sectp->lotag = TAG_ROTATOR;
                    sectp->hitag = sp->lotag;

                    type = SP_TAG3(sp);
                    time = SP_TAG9(sp);

                    u->WaitTics = time*15; // 1/8 of a sec
                    u->Tics = 0;

                    startwall = sector[sp->sectnum].wallptr;
                    endwall = startwall + sector[sp->sectnum].wallnum - 1;

                    // count walls of sector
                    for (w = startwall, wallcount = 0; w <= endwall; w++)
                        wallcount++;

                    u->rotator.Alloc();
                    u->rotator->open_dest = SP_TAG5(sp);
                    u->rotator->speed = SP_TAG7(sp);
                    u->rotator->vel = SP_TAG8(sp);
                    u->rotator->pos = 0; // closed
                    u->rotator->tgt = u->rotator->open_dest; // closed
                    u->rotator->SetNumWalls(wallcount);

                    u->rotator->orig_speed = u->rotator->speed;

                    for (w = startwall, wallcount = 0; w <= endwall; w++)
                    {
                        u->rotator->origX[wallcount] = wall[w].x;
                        u->rotator->origY[wallcount] = wall[w].y;
                        wallcount++;
                    }

                    SET(u->Flags, SPR_ACTIVE);

                    switch (type)
                    {
                    case 0:
                        RESET(u->Flags, SPR_ACTIVE);
                        u->ActorActionFunc = DoRotator;
                        break;
                    case 1:
                        RESET(u->Flags, SPR_ACTIVE);
                        u->ActorActionFunc = DoRotator;
                        break;
                    }

                    change_actor_stat(actor, STAT_ROTATOR);
                    break;
                }

                case SECT_SLIDOR:
                {
                    SECTORp sectp = &sector[sp->sectnum];
                    short time,type;

                    u = SpawnUser(actor, 0, nullptr);

                    SetSectorWallBits(sp->sectnum, WALLFX_DONT_STICK, true, true);

                    // need something for this
                    sectp->lotag = TAG_SLIDOR;
                    sectp->hitag = sp->lotag;

                    type = SP_TAG3(sp);
                    time = SP_TAG9(sp);

                    u->WaitTics = time*15; // 1/8 of a sec
                    u->Tics = 0;

                    u->rotator.Alloc();
                    u->rotator->open_dest = SP_TAG5(sp);
                    u->rotator->speed = SP_TAG7(sp);
                    u->rotator->vel = SP_TAG8(sp);
                    u->rotator->pos = 0; // closed
                    u->rotator->tgt = u->rotator->open_dest; // closed
                    u->rotator->ClearWalls();
                    u->rotator->orig_speed = u->rotator->speed;

                    SET(u->Flags, SPR_ACTIVE);

                    switch (type)
                    {
                    case 0:
                        RESET(u->Flags, SPR_ACTIVE);
                        u->ActorActionFunc = DoSlidor;
                        break;
                    case 1:
                        RESET(u->Flags, SPR_ACTIVE);
                        u->ActorActionFunc = DoSlidor;
                        break;
                    }


                    if (TEST_BOOL5(sp))
                    {
                        DoSlidorInstantClose(actor);
                    }

                    change_actor_stat(actor, STAT_SLIDOR);
                    break;
                }

                case SECT_SPIKE:
                {
                    short speed,vel,time,type,start_on,floor_vator;
                    int floorz,ceilingz;
                    Collision trash;
                    u = SpawnUser(actor, 0, nullptr);

                    SetSectorWallBits(sp->sectnum, WALLFX_DONT_STICK, false, true);
                    SET(sector[sp->sectnum].extra, SECTFX_DYNAMIC_AREA);

                    type = SP_TAG3(sp);
                    speed = SP_TAG4(sp);
                    vel = SP_TAG5(sp);
                    time = SP_TAG9(sp);
                    start_on = !!TEST_BOOL1(sp);
                    floor_vator = true;
                    if (TEST(sp->cstat, CSTAT_SPRITE_YFLIP))
                        floor_vator = false;

                    u->jump_speed = u->vel_tgt = speed;
                    u->vel_rate = vel;
                    u->WaitTics = time*15; // 1/8 of a sec
                    u->Tics = 0;

                    SET(u->Flags, SPR_ACTIVE);

                    switch (type)
                    {
                    case 0:
                        RESET(u->Flags, SPR_ACTIVE);
                        u->ActorActionFunc = DoSpike;
                        break;
                    case 1:
                        RESET(u->Flags, SPR_ACTIVE);
                        u->ActorActionFunc = DoSpike;
                        break;
                    case 2:
                        u->ActorActionFunc = DoSpikeAuto;
                        break;
                    case 3:
                        RESET(u->Flags, SPR_ACTIVE);
                        u->ActorActionFunc = DoSpikeAuto;
                        break;
                    }

                    getzrangepoint(sp->x, sp->y, sp->z, sp->sectnum, &ceilingz, &trash, &floorz, &trash);

                    if (floor_vator)
                    {
                        u->zclip = floorz;

                        // start off
                        u->sz = u->zclip;
                        u->z_tgt = sp->z;
                        if (start_on)
                        {
                            // start in the on position
                            u->zclip = sp->z;
                            u->z_tgt = u->sz;
                            SpikeAlign(actor);
                        }

                        // set orig z
                        u->oz = sp->oz = u->zclip;
                    }
                    else
                    {
                        u->zclip = ceilingz;

                        // start off
                        u->sz = u->zclip;
                        u->z_tgt = sp->z;
                        if (start_on)
                        {
                            // starting in the on position
                            u->zclip = sp->z;
                            u->z_tgt = u->sz;
                            SpikeAlign(actor);
                        }

                        // set orig z
                        u->oz = sp->oz = u->zclip;
                    }

                    change_actor_stat(actor, STAT_SPIKE);
                    break;
                }

                case LIGHTING:
                {
                    short w, startwall, endwall;
                    short wallcount;
                    int8_t* wall_shade;
                    USERp u;

                    LIGHT_Tics(sp) = 0;

                    if (LIGHT_ShadeInc(sp) == 0)
                        LIGHT_ShadeInc(sp) = 1;

                    // save off original floor and ceil shades
                    LIGHT_FloorShade(sp) = sector[sp->sectnum].floorshade;
                    LIGHT_CeilingShade(sp) = sector[sp->sectnum].ceilingshade;

                    startwall = sector[sp->sectnum].wallptr;
                    endwall = startwall + sector[sp->sectnum].wallnum - 1;

                    // count walls of sector
                    for (w = startwall, wallcount = 0; w <= endwall; w++)
                    {
                        wallcount++;
                        if (TEST_BOOL5(sp))
                        {
                            if (wall[w].nextwall >= 0)
                                wallcount++;
                        }
                    }

                    u = SpawnUser(actor, 0, nullptr);
                    u->WallShade.Resize(wallcount);
                    wall_shade = u->WallShade.Data();

                    // save off original wall shades
                    for (w = startwall, wallcount = 0; w <= endwall; w++)
                    {
                        wall_shade[wallcount] =  wall[w].shade;
                        wallcount++;
                        if (TEST_BOOL5(sp))
                        {
                            uint16_t const nextwall = wall[w].nextwall;
                            if (validWallIndex(nextwall))
                            {
                                wall_shade[wallcount] = wall[wall[w].nextwall].shade;
                                wallcount++;
                            }
                        }
                    }

                    u->spal = sp->pal;

                    // DON'T USE COVER function
                    change_actor_stat(actor, STAT_LIGHTING, true);
                    break;
                }

                case LIGHTING_DIFFUSE:
                {
                    short w, startwall, endwall;
                    short wallcount;
                    int8_t* wall_shade;
                    USERp u;

                    LIGHT_Tics(sp) = 0;

                    // save off original floor and ceil shades
                    LIGHT_FloorShade(sp) = sector[sp->sectnum].floorshade;
                    LIGHT_CeilingShade(sp) = sector[sp->sectnum].ceilingshade;

                    startwall = sector[sp->sectnum].wallptr;
                    endwall = startwall + sector[sp->sectnum].wallnum - 1;

                    // count walls of sector
                    for (w = startwall, wallcount = 0; w <= endwall; w++)
                    {
                        wallcount++;
                        if (TEST_BOOL5(sp))
                        {
                            if (wall[w].nextwall >= 0)
                                wallcount++;
                        }
                    }

                    // !LIGHT
                    // make an wall_shade array and put it in User
                    u = SpawnUser(actor, 0, nullptr);
                    u->WallShade.Resize(wallcount);
                    wall_shade = u->WallShade.Data();

                    // save off original wall shades
                    for (w = startwall, wallcount = 0; w <= endwall; w++)
                    {
                        wall_shade[wallcount] = wall[w].shade;
                        wallcount++;
                        if (TEST_BOOL5(sp))
                        {
                            uint16_t const nextwall = wall[w].nextwall;
                            if (validWallIndex(nextwall))
                            {
                                wall_shade[wallcount] = wall[wall[w].nextwall].shade;
                                wallcount++;
                            }
                        }
                    }

                    // DON'T USE COVER function
                    change_actor_stat(actor, STAT_LIGHTING_DIFFUSE, true);
                    break;
                }

                case SECT_VATOR_DEST:
                    change_actor_stat(actor, STAT_VATOR);
                    break;

                case SO_WALL_DONT_MOVE_UPPER:
                    change_actor_stat(actor, STAT_WALL_DONT_MOVE_UPPER);
                    break;

                case SO_WALL_DONT_MOVE_LOWER:
                    change_actor_stat(actor, STAT_WALL_DONT_MOVE_LOWER);
                    break;

                case FLOOR_SLOPE_DONT_DRAW:
                    change_actor_stat(actor, STAT_FLOOR_SLOPE_DONT_DRAW);
                    break;

                case DEMO_CAMERA:
                    sp->yvel = sp->zvel = 100; //attempt horiz control
                    change_actor_stat(actor, STAT_DEMO_CAMERA);
                    break;

                case LAVA_ERUPT:
                {

                    u = SpawnUser(actor, ST1, nullptr);

                    change_actor_stat(actor, STAT_NO_STATE);
                    u->ActorActionFunc = DoLavaErupt;

                    // interval between erupts
                    if (SP_TAG10(sp) == 0)
                        SP_TAG10(sp) = 20;

                    // interval in seconds
                    u->WaitTics = RandomRange(SP_TAG10(sp)) * 120;

                    // time to erupt
                    if (SP_TAG9(sp) == 0)
                        SP_TAG9(sp) = 10;

                    sp->z += Z(30);

                    break;
                }


                case SECT_EXPLODING_CEIL_FLOOR:
                {
                    SECTORp sectp = &sector[sp->sectnum];

                    SetSectorWallBits(sp->sectnum, WALLFX_DONT_STICK, false, true);

                    if (TEST(sectp->floorstat, FLOOR_STAT_SLOPE))
                    {
                        SP_TAG5(sp) = sectp->floorheinum;
                        RESET(sectp->floorstat, FLOOR_STAT_SLOPE);
                        sectp->floorheinum = 0;
                    }

                    if (TEST(sectp->ceilingstat, CEILING_STAT_SLOPE))
                    {
                        SP_TAG6(sp) = sectp->ceilingheinum;
                        RESET(sectp->ceilingstat, CEILING_STAT_SLOPE);
                        sectp->ceilingheinum = 0;
                    }

                    SP_TAG4(sp) = abs(sectp->ceilingz - sectp->floorz)>>8;

                    sectp->ceilingz = sectp->floorz;

                    change_actor_stat(actor, STAT_EXPLODING_CEIL_FLOOR);
                    break;
                }

                case SECT_COPY_SOURCE:
                    change_actor_stat(actor, STAT_COPY_SOURCE);
                    break;

                case SECT_COPY_DEST:
                {
                    SetSectorWallBits(sp->sectnum, WALLFX_DONT_STICK, false, true);
                    change_actor_stat(actor, STAT_COPY_DEST);
                    break;
                }


                case SECT_WALL_MOVE:
                    change_actor_stat(actor, STAT_WALL_MOVE);
                    break;
                case SECT_WALL_MOVE_CANSEE:
                    change_actor_stat(actor, STAT_WALL_MOVE_CANSEE);
                    break;

                case SPRI_CLIMB_MARKER:
                {
                    // setup climb marker
                    change_actor_stat(actor, STAT_CLIMB_MARKER);

                    // make a QUICK_LADDER sprite automatically
                    auto ns = InsertActor(sp->sectnum, STAT_QUICK_LADDER);
                    auto np = &ns->s();

                    np->cstat = 0;
                    np->extra = 0;
                    np->x = sp->x;
                    np->y = sp->y;
                    np->z = sp->z;
                    np->ang = NORM_ANGLE(sp->ang + 1024);
                    np->picnum = sp->picnum;

                    np->x += MOVEx(256+128, sp->ang);
                    np->y += MOVEy(256+128, sp->ang);

                    break;
                }

                case SO_AUTO_TURRET:
#if 0
                    switch (gNet.MultiGameType)
                    {
                    case MULTI_GAME_NONE:
                        change_actor_stat(actor, STAT_ST1);
                        break;
                    case MULTI_GAME_COMMBAT:
                        KillActor(actor);
                        break;
                    case MULTI_GAME_COOPERATIVE:
                        change_actor_stat(actor, STAT_ST1);
                        break;
                    }
#else
                    change_actor_stat(actor, STAT_ST1);
#endif
                    break;

                case SO_DRIVABLE_ATTRIB:
                case SO_SCALE_XY_MULT:
                case SO_SCALE_INFO:
                case SO_SCALE_POINT_INFO:
                case SO_TORNADO:
                case SO_FLOOR_MORPH:
                case SO_AMOEBA:
                case SO_SET_SPEED:
                case SO_ANGLE:
                case SO_SPIN:
                case SO_SPIN_REVERSE:
                case SO_BOB_START:
                case SO_BOB_SPEED:
                case SO_TURN_SPEED:
                case SO_SYNC1:
                case SO_SYNC2:
                case SO_LIMIT_TURN:
                case SO_MATCH_EVENT:
                case SO_MAX_DAMAGE:
                case SO_RAM_DAMAGE:
                case SO_SLIDE:
                case SO_KILLABLE:
                case SECT_SO_SPRITE_OBJ:
                case SECT_SO_DONT_ROTATE:
                case SECT_SO_CLIP_DIST:
                {
                    // NOTE: These will get deleted by the sector
                    // object
                    // setup code

                    change_actor_stat(actor, STAT_ST1);
                    break;
                }

                case SOUND_SPOT:
                    //SP_TAG13(sp) = SP_TAG4(sp);
                    SET_SP_TAG13(sp, SP_TAG4(sp));
                    change_actor_stat(actor, STAT_SOUND_SPOT);
                    break;

                case STOP_SOUND_SPOT:
                    change_actor_stat(actor, STAT_STOP_SOUND_SPOT);
                    break;

                case SPAWN_SPOT:
                    if (!actor->hasU())
                        u = SpawnUser(actor, ST1, nullptr);

                    if (SP_TAG14(sp) == ((64<<8)|64))
                        //SP_TAG14(sp) = 0;
                        SET_SP_TAG14(sp, 0);

                    change_actor_stat(actor, STAT_SPAWN_SPOT);
                    break;

                case VIEW_THRU_CEILING:
                case VIEW_THRU_FLOOR:
                {
                    // make sure there is only one set per level of these
                    SWStatIterator it(STAT_FAF);
                    while (auto itActor = it.Next())
                    {
                        auto ispr = &itActor->s();
                        if (ispr->hitag == sp->hitag && ispr->lotag == sp->lotag)
                        {
                            I_Error("Two VIEW_THRU_ tags with same match found on level\n1: x %d, y %d \n2: x %d, y %d", sp->x, sp->y, ispr->x, ispr->y);
                        }
                    }
                    change_actor_stat(actor, STAT_FAF);
                    break;
                }

                case VIEW_LEVEL1:
                case VIEW_LEVEL2:
                case VIEW_LEVEL3:
                case VIEW_LEVEL4:
                case VIEW_LEVEL5:
                case VIEW_LEVEL6:
                {
                    change_actor_stat(actor, STAT_FAF);
                    break;
                }

                case PLAX_GLOB_Z_ADJUST:
                {
                    SET(sector[sp->sectnum].extra, SECTFX_Z_ADJUST);
                    PlaxCeilGlobZadjust = SP_TAG2(sp);
                    PlaxFloorGlobZadjust = SP_TAG3(sp);
                    KillActor(actor);
                    break;
                }

                case CEILING_Z_ADJUST:
                {
                    //SET(sector[sp->sectnum].ceilingstat, CEILING_STAT_FAF_BLOCK_HITSCAN);
                    SET(sector[sp->sectnum].extra, SECTFX_Z_ADJUST);
                    change_actor_stat(actor, STAT_ST1);
                    break;
                }

                case FLOOR_Z_ADJUST:
                {
                    //SET(sector[sp->sectnum].floorstat, FLOOR_STAT_FAF_BLOCK_HITSCAN);
                    SET(sector[sp->sectnum].extra, SECTFX_Z_ADJUST);
                    change_actor_stat(actor, STAT_ST1);
                    break;
                }

                case WARP_TELEPORTER:
                {
                    short start_wall, wall_num;
                    short sectnum = sp->sectnum;

                    SET(sp->cstat, CSTAT_SPRITE_INVISIBLE);
                    SET(sector[sp->sectnum].extra, SECTFX_WARP_SECTOR);
                    change_actor_stat(actor, STAT_WARP);

                    // if just a destination teleporter
                    // don't set up flags
                    if (SP_TAG10(sp) == 1)
                        break;

                    // move the the next wall
                    wall_num = start_wall = sector[sectnum].wallptr;

                    // Travel all the way around loop setting wall bits
                    do
                    {
                        // DO NOT TAG WHITE WALLS!
                        if (validWallIndex(wall[wall_num].nextwall))
                        {
                            SET(wall[wall_num].cstat, CSTAT_WALL_WARP_HITSCAN);
                        }

                        wall_num = wall[wall_num].point2;
                    }
                    while (wall_num != start_wall);

                    break;
                }

                case WARP_CEILING_PLANE:
                case WARP_FLOOR_PLANE:
                {
                    SET(sp->cstat, CSTAT_SPRITE_INVISIBLE);
                    SET(sector[sp->sectnum].extra, SECTFX_WARP_SECTOR);
                    change_actor_stat(actor, STAT_WARP);
                    break;
                }

                case WARP_COPY_SPRITE1:
                    SET(sp->cstat, CSTAT_SPRITE_INVISIBLE);
                    SET(sector[sp->sectnum].extra, SECTFX_WARP_SECTOR);
                    change_actor_stat(actor, STAT_WARP_COPY_SPRITE1);
                    break;
                case WARP_COPY_SPRITE2:
                    SET(sp->cstat, CSTAT_SPRITE_INVISIBLE);
                    SET(sector[sp->sectnum].extra, SECTFX_WARP_SECTOR);
                    change_actor_stat(actor, STAT_WARP_COPY_SPRITE2);
                    break;

                case FIREBALL_TRAP:
                case BOLT_TRAP:
                case SPEAR_TRAP:
                {
                    u = SpawnUser(actor, 0, nullptr);
                    ClearOwner(actor);
                    change_actor_stat(actor, STAT_TRAP);
                    break;
                }

                case SECT_SO_DONT_BOB:
                {
                    sectu = GetSectUser(sp->sectnum);
                    SET(sectu->flags, SECTFU_SO_DONT_BOB);
                    KillActor(actor);
                    break;
                }

                case SECT_LOCK_DOOR:
                {
                    sectu = GetSectUser(sp->sectnum);
                    sectu->number = sp->lotag;
                    sectu->stag = SECT_LOCK_DOOR;
                    KillActor(actor);
                    break;
                }

                case SECT_SO_SINK_DEST:
                {
                    sectu = GetSectUser(sp->sectnum);
                    SET(sectu->flags, SECTFU_SO_SINK_DEST);
                    sectu->number = sp->lotag;  // acually the offset Z
                    // value
                    KillActor(actor);
                    break;
                }

                case SECT_SO_DONT_SINK:
                {
                    sectu = GetSectUser(sp->sectnum);
                    SET(sectu->flags, SECTFU_SO_DONT_SINK);
                    KillActor(actor);
                    break;
                }

                case SO_SLOPE_FLOOR_TO_POINT:
                {
                    sectu = GetSectUser(sp->sectnum);
                    SET(sectu->flags, SECTFU_SO_SLOPE_FLOOR_TO_POINT);
                    SET(sector[sp->sectnum].extra, SECTFX_DYNAMIC_AREA);
                    KillActor(actor);
                    break;
                }

                case SO_SLOPE_CEILING_TO_POINT:
                {
                    sectu = GetSectUser(sp->sectnum);
                    SET(sectu->flags, SECTFU_SO_SLOPE_CEILING_TO_POINT);
                    SET(sector[sp->sectnum].extra, SECTFX_DYNAMIC_AREA);
                    KillActor(actor);
                    break;
                }
                case SECT_SO_FORM_WHIRLPOOL:
                {
                    sectu = GetSectUser(sp->sectnum);
                    sectu->stag = SECT_SO_FORM_WHIRLPOOL;
                    sectu->height = sp->lotag;
                    KillActor(actor);
                    break;
                }

                case SECT_ACTOR_BLOCK:
                {
                    short start_wall, wall_num;
                    short sectnum = sp->sectnum;

                    // move the the next wall
                    wall_num = start_wall = sector[sectnum].wallptr;

                    // Travel all the way around loop setting wall bits
                    do
                    {
                        SET(wall[wall_num].cstat, CSTAT_WALL_BLOCK_ACTOR);
                        uint16_t const nextwall = wall[wall_num].nextwall;
                        if (validWallIndex(nextwall))
                            SET(wall[nextwall].cstat, CSTAT_WALL_BLOCK_ACTOR);
                        wall_num = wall[wall_num].point2;
                    }
                    while (wall_num != start_wall);

                    KillActor(actor);
                    break;
                }
                }
            }
        }
        break;

        case RED_CARD:
            num = 4;
            goto KeyMain;
        case RED_KEY:
            num = 0;
            goto KeyMain;
        case BLUE_CARD:
            num = 5;
            goto KeyMain;
        case BLUE_KEY:
            num = 1;
            goto KeyMain;
        case GREEN_CARD:
            num = 6;
            goto KeyMain;
        case GREEN_KEY:
            num = 2;
            goto KeyMain;
        case YELLOW_CARD:
            num = 7;
            goto KeyMain;
        case YELLOW_KEY:
            num = 3;
            goto KeyMain;
        case GOLD_SKELKEY:
            num = 8;
            goto KeyMain;
        case SILVER_SKELKEY:
            num = 9;
            goto KeyMain;
        case BRONZE_SKELKEY:
            num = 10;
            goto KeyMain;
        case RED_SKELKEY:
            num = 11;
KeyMain:
            {

                if (gNet.MultiGameType == MULTI_GAME_COMMBAT || gNet.MultiGameType == MULTI_GAME_AI_BOTS)
                {
                    KillActor(actor);
                    break;
                }

                u = SpawnUser(actor, 0, nullptr);

                ASSERT(u != nullptr);
                sp->picnum = u->ID = sp->picnum;

                u->spal = sp->pal; // Set the palette from build

                //SET(sp->cstat, CSTAT_SPRITE_ALIGNMENT_WALL);

                ChangeState(actor, s_Key[num]);

                RESET(picanm[sp->picnum].sf, PICANM_ANIMTYPE_MASK);
                RESET(picanm[sp->picnum + 1].sf, PICANM_ANIMTYPE_MASK);
                change_actor_stat(actor, STAT_ITEM);
                RESET(sp->cstat, CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_BLOCK_HITSCAN | CSTAT_SPRITE_ONE_SIDED);
                u->Radius = 500;
                sp->hitag = LUMINOUS; //Set so keys over ride colored lighting

                DoActorZrange(actor);
            }

            break;


        // Used for multiplayer locks
        case 1846:
        case 1850:
        case 1852:
        case 2470:

            if (TEST(sp->extra, SPRX_MULTI_ITEM))
                if (numplayers <= 1 || gNet.MultiGameType == MULTI_GAME_COOPERATIVE)
                {
                    KillActor(actor);
                }

            break;

        case FIRE_FLY0:

            /*
             * u = SpawnUser(actor, FIRE_FLY0, nullptr);
             *
             * u->State = u->StateStart = &s_FireFly[0]; u->RotNum = 0;
             *
             * sp->ang = 0; sp->xvel = 4;
             *
             * if (labs(sp->z - sector[sp->sectnum].floorz) < Z(32)) sp->z =
             * sector[sp->sectnum].floorz - Z(32);
             *
             * u->sz = sp->z;
             *
             * change_actor_stat(actor, STAT_MISC);
             */

            break;

        case ICON_REPAIR_KIT:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_REPAIR_KIT, s_RepairKit);

            IconDefault(actor);
            break;

        case ICON_STAR:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_STAR, s_IconStar);

            IconDefault(actor);
            break;

        case ICON_LG_MINE:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_LG_MINE, s_IconLgMine);
            IconDefault(actor);
            break;


        case ICON_MICRO_GUN:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_MICRO_GUN, s_IconMicroGun);

            IconDefault(actor);
            break;

        case ICON_MICRO_BATTERY:

NUKE_REPLACEMENT:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_MICRO_BATTERY, s_IconMicroBattery);

            IconDefault(actor);
            break;

        case ICON_UZI:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_UZI, s_IconUzi);
            IconDefault(actor);
            break;

        case ICON_UZIFLOOR:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_UZIFLOOR, s_IconUziFloor);
            IconDefault(actor);
            break;

        case ICON_LG_UZI_AMMO:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_LG_UZI_AMMO, s_IconLgUziAmmo);
            IconDefault(actor);
            break;

        case ICON_GRENADE_LAUNCHER:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_GRENADE_LAUNCHER, s_IconGrenadeLauncher);

            IconDefault(actor);
            break;

        case ICON_LG_GRENADE:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_LG_GRENADE, s_IconLgGrenade);
            IconDefault(actor);
            break;

        case ICON_RAIL_GUN:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_RAIL_GUN, s_IconRailGun);
            IconDefault(actor);
            break;

        case ICON_RAIL_AMMO:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_RAIL_AMMO, s_IconRailAmmo);
            IconDefault(actor);
            break;


        case ICON_ROCKET:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_ROCKET, s_IconRocket);
            IconDefault(actor);
            break;

        case ICON_LG_ROCKET:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_LG_ROCKET, s_IconLgRocket);
            IconDefault(actor);
            break;

        case ICON_SHOTGUN:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_SHOTGUN, s_IconShotgun);

            u->Radius = 350; // Shotgun is hard to pick up for some reason.

            IconDefault(actor);
            break;

        case ICON_LG_SHOTSHELL:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_LG_SHOTSHELL, s_IconLgShotshell);
            IconDefault(actor);
            break;

        case ICON_AUTORIOT:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_AUTORIOT, s_IconAutoRiot);
            IconDefault(actor);
            break;


        case ICON_GUARD_HEAD:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_GUARD_HEAD, s_IconGuardHead);
            IconDefault(actor);
            break;

        case ICON_FIREBALL_LG_AMMO:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_FIREBALL_LG_AMMO, s_IconFireballLgAmmo);
            IconDefault(actor);
            break;

        case ICON_HEART:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_HEART, s_IconHeart);
            IconDefault(actor);
            break;

        case ICON_HEART_LG_AMMO:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_HEART_LG_AMMO, s_IconHeartLgAmmo);
            IconDefault(actor);
            break;

#if 0
        case ICON_ELECTRO:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_ELECTRO, s_IconElectro);
            IconDefault(actor);
            break;
#endif

        case ICON_SPELL:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_SPELL, s_IconSpell);
            IconDefault(actor);

            PicAnimOff(sp->picnum);
            break;

        case ICON_ARMOR:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_ARMOR, s_IconArmor);
            if (sp->pal != PALETTE_PLAYER3)
                sp->pal = u->spal = PALETTE_PLAYER1;
            else
                sp->pal = u->spal = PALETTE_PLAYER3;
            IconDefault(actor);
            break;

        case ICON_MEDKIT:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_MEDKIT, s_IconMedkit);
            IconDefault(actor);
            break;

        case ICON_SM_MEDKIT:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_SM_MEDKIT, s_IconSmMedkit);
            IconDefault(actor);
            break;

        case ICON_CHEMBOMB:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_CHEMBOMB, s_IconChemBomb);
            IconDefault(actor);
            break;

        case ICON_FLASHBOMB:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_FLASHBOMB, s_IconFlashBomb);
            IconDefault(actor);
            break;

        case ICON_NUKE:

            if (gNet.MultiGameType)
            {
                if (!gNet.Nuke)
                {
                    goto NUKE_REPLACEMENT;
                    break;
                }
            }

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_NUKE, s_IconNuke);
            IconDefault(actor);
            break;


        case ICON_CALTROPS:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_CALTROPS, s_IconCaltrops);
            IconDefault(actor);
            break;

        case ICON_BOOSTER:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_BOOSTER, s_IconBooster);
            IconDefault(actor);
            break;

        case ICON_HEAT_CARD:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_HEAT_CARD, s_IconHeatCard);
            IconDefault(actor);
            break;

#if 0
        case ICON_ENVIRON_SUIT:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_ENVIRON_SUIT, s_IconEnvironSuit);
            IconDefault(actor);
            PicAnimOff(sp->picnum);
            break;
#endif

        case ICON_CLOAK:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_CLOAK, s_IconCloak);
            IconDefault(actor);
            PicAnimOff(sp->picnum);
            break;

        case ICON_FLY:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_FLY, s_IconFly);
            IconDefault(actor);
            PicAnimOff(sp->picnum);
            break;

        case ICON_NIGHT_VISION:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_NIGHT_VISION, s_IconNightVision);
            IconDefault(actor);
            PicAnimOff(sp->picnum);
            break;

        case ICON_FLAG:

            if (!IconSpawn(sp))
            {
                KillActor(actor);
                break;
            }

            u = SpawnUser(actor, ICON_FLAG, s_IconFlag);
            u->spal = sp->pal;
            sector[sp->sectnum].hitag = 9000;       // Put flag's color in sect containing it
            sector[sp->sectnum].lotag = u->spal;
            IconDefault(actor);
            PicAnimOff(sp->picnum);
            break;

#if 0
        case 380:
        case 396:
        case 430:
        case 512:
        case 521:
        case 541:
        case 2720:
#endif
        case 3143:
        case 3157:
        {
            u = SpawnUser(actor, sp->picnum, nullptr);

            change_actor_stat(actor, STAT_STATIC_FIRE);

            u->ID = FIREBALL_FLAMES;
            u->Radius = 200;
            RESET(sp->cstat, CSTAT_SPRITE_BLOCK);
            RESET(sp->cstat, CSTAT_SPRITE_BLOCK_HITSCAN);

            sp->hitag = LUMINOUS; //Always full brightness
            sp->shade = -40;

            break;
        }

        // blades
        case BLADE1:
        case BLADE2:
        case BLADE3:
        case 5011:
        {
            u = SpawnUser(actor, sp->picnum, nullptr);

            change_actor_stat(actor, STAT_DEFAULT);

            RESET(sp->cstat, CSTAT_SPRITE_BLOCK);
            SET(sp->cstat, CSTAT_SPRITE_BLOCK_HITSCAN);
            SET(sp->extra, SPRX_BLADE);

            break;
        }

        case BREAK_LIGHT:
        case BREAK_BARREL:
        case BREAK_PEDISTAL:
        case BREAK_BOTTLE1:
        case BREAK_BOTTLE2:
        case BREAK_MUSHROOM:

            //if (TEST(sp->extra, SPRX_BREAKABLE))
            //    break;

            u = SpawnUser(actor, sp->picnum, nullptr);

            sp->clipdist = SPRITEp_SIZE_X(sp);
            SET(sp->cstat, CSTAT_SPRITE_BREAKABLE);
            SET(sp->extra, SPRX_BREAKABLE);
            break;

        // switches
        case 581:
        case 582:
        case 558:
        case 559:
        case 560:
        case 561:
        case 562:
        case 563:
        case 564:
        case 565:
        case 566:
        case 567:
        case 568:
        case 569:
        case 570:
        case 571:
        case 572:
        case 573:
        case 574:

        case 551:
        case 552:
        case 575:
        case 576:
        case 577:
        case 578:
        case 579:
        case 589:
        case 583:
        case 584:

        case 553:
        case 554:
        {
            if (TEST(sp->extra, SPRX_MULTI_ITEM))
            {
                if (numplayers <= 1 || gNet.MultiGameType == MULTI_GAME_COOPERATIVE)
                {
                    KillActor(actor);
                    break;
                }
            }


            SET(sp->cstat, CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_BLOCK_HITSCAN);
            break;
        }

        }
    }
}

bool ItemSpotClear(DSWActor* actor, short statnum, short id)
{
    bool found = false;
    int i;
    auto sip = &actor->s();

    if (TEST_BOOL2(sip))
    {
        SWSectIterator it(sip->sectnum);
        while (auto itActor = it.Next())
        {
            if (itActor->s().statnum == statnum && itActor->u()->ID == id)
            {
                found = true;
                break;
            }
        }
    }

    return !found;
}

void SetupItemForJump(DSWActor* spawner, DSWActor* actor)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();
    auto sip = &spawner->s();

    // setup item for jumping
    if (SP_TAG7(sip))
    {
        change_actor_stat(actor, STAT_SKIP4);
        u->ceiling_dist = Z(6);
        u->floor_dist = Z(0);
        u->Counter = 0;

        sp->xvel = (int)SP_TAG7(sip)<<2;
        sp->zvel = -(((int)SP_TAG8(sip))<<5);

        u->xchange = MOVEx(sp->xvel, sp->ang);
        u->ychange = MOVEy(sp->xvel, sp->ang);
        u->zchange = sp->zvel;
    }
}

int ActorCoughItem(DSWActor* actor)
{
    SPRITEp sp = &actor->s();
    USERp u = actor->u();
    short choose;
    SPRITEp np;
    DSWActor* actorNew = nullptr;


    switch (u->ID)
    {
    case SAILORGIRL_R0:
        ASSERT(sp->sectnum >= 0);
        actorNew = InsertActor(sp->sectnum, STAT_SPAWN_ITEMS);
        np = &actorNew->s();
        np->cstat = np->extra = 0;
        np->x = sp->x;
        np->y = sp->y;
        np->z = SPRITEp_MID(sp);
        np->ang = 0;
        np->extra = 0;

        // vel
        SP_TAG7(np) = 1;
        // zvel
        SP_TAG8(np) = 40;

        choose = RANDOM_P2(1024);

        if (choose > 854)
            SP_TAG3(np) = 91; // Match number
        else if (choose > 684)
            SP_TAG3(np) = 48; // Match number
        else if (choose > 514)
            SP_TAG3(np) = 58; // Match number
        else if (choose > 344)
            SP_TAG3(np) = 60; // Match number
        else if (choose > 174)
            SP_TAG3(np) = 62; // Match number
        else
            SP_TAG3(np) = 68; // Match number

        // match
        SP_TAG2(np) = -1;
        // kill
        RESET_BOOL1(np);
        SpawnItemsMatch(-1);
        break;

    case GORO_RUN_R0:
        if (RANDOM_P2(1024) < 700)
            return 0;

        ASSERT(sp->sectnum >= 0);
        actorNew = InsertActor(sp->sectnum, STAT_SPAWN_ITEMS);
        np = &actorNew->s();
        np->cstat = np->extra = 0;
        np->x = sp->x;
        np->y = sp->y;
        np->z = SPRITEp_MID(sp);
        np->ang = 0;
        np->extra = 0;

        // vel
        SP_TAG7(np) = 1;
        // zvel
        SP_TAG8(np) = 40;

        SP_TAG3(np) = 69; // Match number

        // match
        SP_TAG2(np) = -1;
        // kill
        RESET_BOOL1(np);
        SpawnItemsMatch(-1);
        break;

    case RIPPER2_RUN_R0:
        if (RANDOM_P2(1024) < 700)
            return 0;

        ASSERT(sp->sectnum >= 0);
        actorNew = InsertActor(sp->sectnum, STAT_SPAWN_ITEMS);
        np = &actorNew->s();
        np->cstat = np->extra = 0;
        np->x = sp->x;
        np->y = sp->y;
        np->z = SPRITEp_MID(sp);
        np->ang = 0;
        np->extra = 0;

        // vel
        SP_TAG7(np) = 1;
        // zvel
        SP_TAG8(np) = 40;

        SP_TAG3(np) = 70; // Match number

        // match
        SP_TAG2(np) = -1;
        // kill
        RESET_BOOL1(np);
        SpawnItemsMatch(-1);
        break;

    case NINJA_RUN_R0:

        if (u->PlayerP)
        {
            if (RANDOM_P2(1024) > 200)
                return 0;

            ASSERT(sp->sectnum >= 0);
            actorNew = InsertActor(sp->sectnum, STAT_SPAWN_ITEMS);
            np = &actorNew->s();
            np->cstat = 0;
            np->extra = 0;
            np->x = sp->x;
            np->y = sp->y;
            np->z = SPRITEp_MID(sp);
            np->ang = 0;
            np->extra = 0;

            // vel
            SP_TAG7(np) = 1;
            // zvel
            SP_TAG8(np) = 40;

            switch (u->WeaponNum)
            {
            case WPN_UZI:
                SP_TAG3(np) = 0;
                break;
            case WPN_SHOTGUN:
                SP_TAG3(np) = 51;
                break;
            case WPN_STAR:
                if (u->PlayerP->WpnAmmo[WPN_STAR] < 9)
                    break;
                SP_TAG3(np) = 41;
                break;
            case WPN_MINE:
                if (u->PlayerP->WpnAmmo[WPN_MINE] < 5)
                    break;
                SP_TAG3(np) = 42;
                break;
            case WPN_MICRO:
            case WPN_ROCKET:
                SP_TAG3(np) = 43;
                break;
            case WPN_GRENADE:
                SP_TAG3(np) = 45;
                break;
            case WPN_RAIL:
                SP_TAG3(np) = 47;
                break;
            case WPN_HEART:
                SP_TAG3(np) = 55;
                break;
            case WPN_HOTHEAD:
                SP_TAG3(np) = 53;
                break;
            }

            // match
            SP_TAG2(np) = -1;
            // kill
            RESET_BOOL1(np);
            SpawnItemsMatch(-1);
            break;
        }

        if (RANDOM_P2(1024) < 512)
            return 0;

        ASSERT(sp->sectnum >= 0);
        actorNew = InsertActor(sp->sectnum, STAT_SPAWN_ITEMS);
        np = &actorNew->s();
        np->cstat = np->extra = 0;
        np->x = sp->x;
        np->y = sp->y;
        np->z = SPRITEp_MID(sp);
        np->ang = 0;
        np->extra = 0;

        // vel
        SP_TAG7(np) = 1;
        // zvel
        SP_TAG8(np) = 40;

        if (u->spal == PAL_XLAT_LT_TAN)
        {
            SP_TAG3(np) = 44;
        }
        else if (u->spal == PAL_XLAT_LT_GREY)
        {
            SP_TAG3(np) = 46;
        }
        else if (u->spal == PALETTE_PLAYER5) // Green Ninja
        {
            if (RANDOM_P2(1024) < 700)
                SP_TAG3(np) = 61;
            else
                SP_TAG3(np) = 60;
        }
        else if (u->spal == PALETTE_PLAYER3) // Red Ninja
        {
            // type
            if (RANDOM_P2(1024) < 800)
                SP_TAG3(np) = 68;
            else
                SP_TAG3(np) = 44;
        }
        else
        {
            if (RANDOM_P2(1024) < 512)
                SP_TAG3(np) = 41;
            else
                SP_TAG3(np) = 68;
        }

        // match
        SP_TAG2(np) = -1;
        // kill
        RESET_BOOL1(np);
        SpawnItemsMatch(-1);
        break;

    case PACHINKO1:
    case PACHINKO2:
    case PACHINKO3:
    case PACHINKO4:

        ASSERT(sp->sectnum >= 0);
        actorNew = InsertActor(sp->sectnum, STAT_SPAWN_ITEMS);
        np = &actorNew->s();
        np->cstat = np->extra = 0;
        np->x = sp->x;
        np->y = sp->y;
        np->z = SPRITEp_LOWER(sp)+Z(10);
        np->ang = sp->ang;

        // vel
        SP_TAG7(np) = 10;
        // zvel
        SP_TAG8(np) = 10;

        if (u->ID == PACHINKO1)
        {
            if (RANDOM_P2(1024) < 600)
                SP_TAG3(np) = 64; // Small MedKit
            else
                SP_TAG3(np) = 59; // Fortune Cookie
        }
        else if (u->ID == PACHINKO2)
        {
            if (RANDOM_P2(1024) < 600)
                SP_TAG3(np) = 52; // Lg Shot Shell
            else
                SP_TAG3(np) = 68; // Uzi clip
        }
        else if (u->ID == PACHINKO3)
        {
            if (RANDOM_P2(1024) < 600)
                SP_TAG3(np) = 57;
            else
                SP_TAG3(np) = 63;
        }
        else if (u->ID == PACHINKO4)
        {
            if (RANDOM_P2(1024) < 600)
                SP_TAG3(np) = 60;
            else
                SP_TAG3(np) = 61;
        }

        // match
        SP_TAG2(np) = -1;
        // kill
        RESET_BOOL1(np);
        SpawnItemsMatch(-1);
        break;
    }

    return 0;
}

int SpawnItemsMatch(short match)
{
    DSWActor* spawnedActor = nullptr;
    SPRITEp sp,sip;

    SWStatIterator it(STAT_SPAWN_ITEMS);
    while (auto itActor = it.Next())
    {
        sip = &itActor->s();

        if (SP_TAG2(sip) != match)
            continue;

        switch (SP_TAG3(sip))
        {
        case 90:
            spawnedActor = BunnyHatch2(itActor);
            sp = &spawnedActor->s();
            spawnedActor->u()->spal = sp->pal = PALETTE_PLAYER8; // Boy
            sp->ang = sip->ang;
            break;
        case 91:
            spawnedActor = BunnyHatch2(itActor);
            sp = &spawnedActor->s();
            spawnedActor->u()->spal = sp->pal = PALETTE_PLAYER0; // Girl
            sp->ang = sip->ang;
            break;
        case 92:
            spawnedActor = BunnyHatch2(itActor);
            sp = &spawnedActor->s();
            sp->ang = sip->ang;
             break;

        case 40:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_REPAIR_KIT))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_REPAIR_KIT, s_RepairKit, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 41:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_STAR))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_STAR, s_IconStar, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 42:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_LG_MINE))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_LG_MINE, s_IconLgMine, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 43:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_MICRO_GUN))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_MICRO_GUN, s_IconMicroGun, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 44:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_MICRO_BATTERY))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_MICRO_BATTERY, s_IconMicroBattery, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 45:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_GRENADE_LAUNCHER))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_GRENADE_LAUNCHER, s_IconGrenadeLauncher, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 46:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_LG_GRENADE))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_LG_GRENADE, s_IconLgGrenade, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 47:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_RAIL_GUN))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_RAIL_GUN, s_IconRailGun, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 48:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_RAIL_AMMO))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_RAIL_AMMO, s_IconRailAmmo, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 49:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_ROCKET))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_ROCKET, s_IconRocket, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 51:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_SHOTGUN))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_SHOTGUN, s_IconShotgun, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 52:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_LG_SHOTSHELL))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_LG_SHOTSHELL, s_IconLgShotshell, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 53:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_GUARD_HEAD))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_GUARD_HEAD, s_IconGuardHead, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 54:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_FIREBALL_LG_AMMO))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_FIREBALL_LG_AMMO, s_IconFireballLgAmmo, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 55:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_HEART))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_HEART, s_IconHeart, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 56:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_HEART_LG_AMMO))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_HEART_LG_AMMO, s_IconHeartLgAmmo, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 57:
        {
            USERp u;
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_ARMOR))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_ARMOR, s_IconArmor, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            sp = &spawnedActor->s();
            u = spawnedActor->u();
            SET(u->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);

            if (sp->pal != PALETTE_PLAYER3)
                sp->pal = u->spal = PALETTE_PLAYER1;
            else
                sp->pal = u->spal = PALETTE_PLAYER3;
            break;
        }

        case 58:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_MEDKIT))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_MEDKIT, s_IconMedkit, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 59:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_SM_MEDKIT))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_SM_MEDKIT, s_IconSmMedkit, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 60:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_CHEMBOMB))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_CHEMBOMB, s_IconChemBomb, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 61:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_FLASHBOMB))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_FLASHBOMB, s_IconFlashBomb, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 62:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_NUKE))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_NUKE, s_IconNuke, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 63:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_CALTROPS))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_CALTROPS, s_IconCaltrops, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 64:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_BOOSTER))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_BOOSTER, s_IconBooster, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 65:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_HEAT_CARD))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_HEAT_CARD, s_IconHeatCard, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 66:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_CLOAK))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_CLOAK, s_IconCloak, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 67:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_NIGHT_VISION))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_NIGHT_VISION, s_IconNightVision, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;


        case 68:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_LG_UZI_AMMO))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_LG_UZI_AMMO, s_IconLgUziAmmo, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 69:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_GUARD_HEAD))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_GUARD_HEAD, s_IconGuardHead, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 70:
            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_HEART))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_HEART, s_IconHeart, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 20:

            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_UZIFLOOR))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_UZIFLOOR, s_IconUziFloor, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;


        case 32:
        case 0:

            if (!ItemSpotClear(itActor, STAT_ITEM, ICON_UZI))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, ICON_UZI, s_IconUzi, sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            SET(spawnedActor->u()->Flags2, SPR2_NEVER_RESPAWN);
            IconDefault(spawnedActor);

            SetupItemForJump(itActor, spawnedActor);
            break;

        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        {
            short num;
            USERp u;

            uint8_t KeyPal[] =
            {
                PALETTE_PLAYER9,
                PALETTE_PLAYER7,
                PALETTE_PLAYER6,
                PALETTE_PLAYER4,
                PALETTE_PLAYER9,
                PALETTE_PLAYER7,
                PALETTE_PLAYER6,
                PALETTE_PLAYER4,
                PALETTE_PLAYER4,
                PALETTE_PLAYER1,
                PALETTE_PLAYER8,
                PALETTE_PLAYER9
            };

            if (gNet.MultiGameType == MULTI_GAME_COMMBAT || gNet.MultiGameType == MULTI_GAME_AI_BOTS)
                break;

            num = SP_TAG3(sip) - 1;

            if (!ItemSpotClear(itActor, STAT_ITEM, s_Key[num]->Pic))
                break;

            spawnedActor = SpawnActor(STAT_ITEM, s_Key[num]->Pic, s_Key[num], sip->sectnum, sip->x, sip->y, sip->z, sip->ang, 0);
            u = spawnedActor->u();
            sp = &spawnedActor->s();

            ASSERT(u != nullptr);
            sp->picnum = u->ID = s_Key[num]->Pic;


            // need to set the palette here - suggest table lookup
            u->spal = sp->pal = KeyPal[num];
            //SET(sp->cstat, CSTAT_SPRITE_ALIGNMENT_WALL);


            ChangeState(spawnedActor, s_Key[num]);

            RESET(picanm[sp->picnum].sf, PICANM_ANIMTYPE_MASK);
            RESET(picanm[sp->picnum + 1].sf, PICANM_ANIMTYPE_MASK);

            SetupItemForJump(itActor, spawnedActor);

            break;
        }
        }

        if (!TEST_BOOL1(sip))
            KillActor(itActor);
    }
    return 0;
}

int NewStateGroup(DSWActor* actor, STATEp StateGroup[])
{
    USERp u = actor->u();

    //if (Prediction)
    //    return;

    if (!StateGroup)
        return 0;

    ASSERT(u);

    // Kind of a goofy check, but it should catch alot of invalid states!
    // BTW, 6144 is the max tile number allowed in editart.
    if (u->State && (u->State->Pic < 0 || u->State->Pic > MAXTILES))    // JBF: verify this!
        return 0;

    u->Rot = StateGroup;
    u->State = u->StateStart = StateGroup[0];

    u->Tics = 0;

    // turn anims off because people keep setting them in the
    // art file
    RESET(picanm[actor->s().picnum].sf, PICANM_ANIMTYPE_MASK);
    return 0;
}


bool SpriteOverlap(DSWActor* actor_a, DSWActor* actor_b)
{
    SPRITEp spa = &actor_a->s(), spb = &actor_b->s();

    USERp ua = actor_a->u();
    USERp ub = actor_b->u();

    int spa_tos, spa_bos, spb_tos, spb_bos, overlap_z;

    if (!ua || !ub) return false;
    if ((unsigned)Distance(spa->x, spa->y, spb->x, spb->y) > ua->Radius + ub->Radius)
    {
        return false;
    }

    spa_tos = SPRITEp_TOS(spa);
    spa_bos = SPRITEp_BOS(spa);

    spb_tos = SPRITEp_TOS(spb);
    spb_bos = SPRITEp_BOS(spb);


    overlap_z = ua->OverlapZ + ub->OverlapZ;

    // if the top of sprite a is below the bottom of b
    if (spa_tos - overlap_z > spb_bos)
    {
        return false;
    }

    // if the top of sprite b is is below the bottom of a
    if (spb_tos - overlap_z > spa_bos)
    {
        return false;
    }

    return true;

}

bool SpriteOverlapZ(DSWActor* actor_a, DSWActor* actor_b, int z_overlap)
{
    SPRITEp spa = &actor_a->s(), spb = &actor_b->s();

    int spa_tos, spa_bos, spb_tos, spb_bos;

    spa_tos = SPRITEp_TOS(spa);
    spa_bos = SPRITEp_BOS(spa);

    spb_tos = SPRITEp_TOS(spb);
    spb_bos = SPRITEp_BOS(spb);


    // if the top of sprite a is below the bottom of b
    if (spa_tos + z_overlap > spb_bos)
    {
        return false;
    }

    // if the top of sprite b is is below the bottom of a
    if (spb_tos + z_overlap > spa_bos)
    {
        return false;
    }

    return true;

}

void getzrangepoint(int x, int y, int z, short sectnum,
               int32_t* ceilz, Collision* ceilhit, int32_t* florz, Collision* florhit)
{
    spritetype *spr;
    int j, k, l, dax, day, daz, xspan, yspan, xoff, yoff;
    int x1, y1, x2, y2, x3, y3, x4, y4, cosang, sinang, tilenum;
    short cstat;
    char clipyou;

    if (sectnum < 0)
    {
        *ceilz = 0x80000000;
        
        *florz = 0x7fffffff;
        florhit->invalidate();
        return;
    }

    // Initialize z's and hits to the current sector's top&bottom
    getzsofslope(sectnum, x, y, ceilz, florz);
    ceilhit->setSector(sectnum);
    florhit->setSector(sectnum);

    // Go through sprites of only the current sector
    SWSectIterator it(sectnum);
    while (auto itActor = it.Next())
    {
        spr = &itActor->s();
        cstat = spr->cstat;
        if ((cstat & 49) != 33)
            continue;                   // Only check blocking floor sprites

        daz = spr->z;

        // Only check if sprite's 2-sided or your on the 1-sided side
        if (((cstat & 64) != 0) && ((z > daz) == ((cstat & 8) == 0)))
            continue;

        // Calculate and store centering offset information into xoff&yoff
        tilenum = spr->picnum;
        xoff = (int)tileLeftOffset(tilenum) + (int)spr->xoffset;
        yoff = (int)tileTopOffset(tilenum) + (int)spr->yoffset;
        if (cstat & 4)
            xoff = -xoff;
        if (cstat & 8)
            yoff = -yoff;

        // Calculate all 4 points of the floor sprite.
        // (x1,y1),(x2,y2),(x3,y3),(x4,y4)
        // These points will already have (x,y) subtracted from them
        cosang = bcos(spr->ang);
        sinang = bsin(spr->ang);
        xspan = tileWidth(tilenum);
        dax = ((xspan >> 1) + xoff) * spr->xrepeat;
        yspan = tileHeight(tilenum);
        day = ((yspan >> 1) + yoff) * spr->yrepeat;
        x1 = spr->x + DMulScale(sinang, dax, cosang, day, 16) - x;
        y1 = spr->y + DMulScale(sinang, day, -cosang, dax, 16) - y;
        l = xspan * spr->xrepeat;
        x2 = x1 - MulScale(sinang, l, 16);
        y2 = y1 + MulScale(cosang, l, 16);
        l = yspan * spr->yrepeat;
        k = -MulScale(cosang, l, 16);
        x3 = x2 + k;
        x4 = x1 + k;
        k = -MulScale(sinang, l, 16);
        y3 = y2 + k;
        y4 = y1 + k;

        // Check to see if point (0,0) is inside the 4 points by seeing if
        // the number of lines crossed as a line is shot outward is odd
        clipyou = 0;
        if ((y1 ^ y2) < 0)              // If y1 and y2 have different signs
        // (- / +)
        {
            if ((x1 ^ x2) < 0)
                clipyou ^= (x1 * y2 < x2 * y1) ^ (y1 < y2);
            else if (x1 >= 0)
                clipyou ^= 1;
        }
        if ((y2 ^ y3) < 0)
        {
            if ((x2 ^ x3) < 0)
                clipyou ^= (x2 * y3 < x3 * y2) ^ (y2 < y3);
            else if (x2 >= 0)
                clipyou ^= 1;
        }
        if ((y3 ^ y4) < 0)
        {
            if ((x3 ^ x4) < 0)
                clipyou ^= (x3 * y4 < x4 * y3) ^ (y3 < y4);
            else if (x3 >= 0)
                clipyou ^= 1;
        }
        if ((y4 ^ y1) < 0)
        {
            if ((x4 ^ x1) < 0)
                clipyou ^= (x4 * y1 < x1 * y4) ^ (y4 < y1);
            else if (x4 >= 0)
                clipyou ^= 1;
        }
        if (clipyou == 0)
            continue;                   // Point is not inside, don't clip

        // Clipping time!
        if (z > daz)
        {
            if (daz > *ceilz)
            {
                *ceilz = daz;
                ceilhit->setSprite(itActor);
            }
        }
        else
        {
            if (daz < *florz)
            {
                *florz = daz;
                florhit->setSprite(itActor);
            }
        }
    }
}


void DoActorZrange(DSWActor* actor)
{
    USERp u = actor->u(), wu;
    SPRITEp sp = &actor->s(), wp;
    Collision ceilhit, florhit;
    short save_cstat;

    save_cstat = TEST(sp->cstat, CSTAT_SPRITE_BLOCK);
    RESET(sp->cstat, CSTAT_SPRITE_BLOCK);
    vec3_t pos = sp->pos;
    pos.z -= DIV2(SPRITEp_SIZE_Z(sp));
    FAFgetzrange(pos, sp->sectnum, &u->hiz, &ceilhit, &u->loz, &florhit, (((int) sp->clipdist) << 2) - GETZRANGE_CLIP_ADJ, CLIPMASK_ACTOR);
    SET(sp->cstat, save_cstat);

    u->lo_sectp = u->hi_sectp = nullptr;
    u->highActor = nullptr;
    u->lowActor = nullptr;

    switch (ceilhit.type)
    {
    case kHitSprite:
        u->highActor = ceilhit.actor;
        break;
    case kHitSector:
        u->hi_sectp = &sector[ceilhit.index];
        break;
    default:
        ASSERT(true==false);
        break;
    }

    switch (florhit.type)
    {
    case kHitSprite:
        u->lowActor = florhit.actor;
        break;
    case kHitSector:
        u->lo_sectp = &sector[florhit.index];
        break;
    default:
        ASSERT(true==false);
        break;
    }
}

// !AIC - puts getzrange results into USER varaible u->loz, u->hiz, u->lo_sectp, u->hi_sectp, etc.
// The loz and hiz are used a lot.

int DoActorGlobZ(DSWActor* actor)
{
    USERp u = actor->u();

    u->loz = globloz;
    u->hiz = globhiz;

    u->lo_sectp = u->hi_sectp = nullptr;
    u->highActor = nullptr;
    u->lowActor = nullptr;

    switch (globhihit.type)
    {
    case kHitSprite:
        u->highActor = globhihit.actor;
        break;
    default:
        u->hi_sectp = &sector[globhihit.index];
        break;
    }

    switch (globlohit.type)
    {
    case kHitSprite:
        u->lowActor = globlohit.actor;
        break;
    default:
        u->lo_sectp = &sector[globlohit.index];
        break;
    }

    return 0;
}


bool ActorDrop(DSWActor* actor, int x, int y, int z, short new_sector, short min_height)
{
    SPRITEp sp = &actor->s();
    int hiz, loz;
    Collision ceilhit, florhit;
    short save_cstat;

    // look only at the center point for a floor sprite
    save_cstat = TEST(sp->cstat, CSTAT_SPRITE_BLOCK);
    RESET(sp->cstat, CSTAT_SPRITE_BLOCK);
    FAFgetzrangepoint(x, y, z - DIV2(SPRITEp_SIZE_Z(sp)), new_sector, &hiz, &ceilhit, &loz, &florhit);
    SET(sp->cstat, save_cstat);

    if (florhit.type < 0 || ceilhit.type < 0)
    {
        return true;
    }

    switch (florhit.type)
    {
    case kHitSprite:
    {
        SPRITEp hsp = &florhit.actor->s();

        // if its a floor sprite and not too far down
        if (TEST(hsp->cstat, CSTAT_SPRITE_ALIGNMENT_FLOOR) &&
            (labs(loz - z) <= min_height))
        {
            return false;
        }

        break;
    }

    case kHitSector:
    {
        if (labs(loz - z) <= min_height)
        {
            return false;
        }

        break;
    }
    default:
        ASSERT(true == false);
        break;
    }

    return true;
}

// Primarily used in ai.c for now - need to get rid of
bool
DropAhead(DSWActor* actor, short min_height)
{
    SPRITEp sp = &actor->s();
    int dax, day;
    int newsector;

    // dax = sp->x + MOVEx(128, sp->ang);
    // day = sp->y + MOVEy(128, sp->ang);

    dax = sp->x + MOVEx(256, sp->ang);
    day = sp->y + MOVEy(256, sp->ang);

    newsector = sp->sectnum;
    updatesector(dax, day, &newsector);

    // look straight down for a drop
    if (ActorDrop(actor, dax, day, sp->z, newsector, min_height))
        return true;

    return false;
}

/*

  !AIC KEY - Called by ai.c routines.  Calls move_sprite which calls clipmove.
  This incapulates move_sprite and makes sure that actors don't walk off of
  ledges.  If it finds itself in mid air then it restores the last good
  position.  This is a hack because Ken had no good way of doing this from his
  code.  ActorDrop() is called from here.

*/

int move_actor(DSWActor* actor, int xchange, int ychange, int zchange)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    int x, y, z, loz, hiz;
    DSWActor* highActor;
    DSWActor* lowActor;
    SECTORp lo_sectp, hi_sectp;
    short sectnum;
    short dist;
    int cliptype = CLIPMASK_ACTOR;


    if (TEST(u->Flags, SPR_NO_SCAREDZ))
    {
        // For COOLG & HORNETS
        // set to actual z before you move
        sp->z = u->sz;
    }

    // save off x,y values
    x = sp->x;
    y = sp->y;
    z = sp->z;
    loz = u->loz;
    hiz = u->hiz;
    lowActor = u->lowActor;
    highActor = u->highActor;
    lo_sectp = u->lo_sectp;
    hi_sectp = u->hi_sectp;
    sectnum = sp->sectnum;

    u->coll = move_sprite(actor, xchange, ychange, zchange,
                         u->ceiling_dist, u->floor_dist, cliptype, ACTORMOVETICS);

    ASSERT(sp->sectnum >= 0);

    // try and determine whether you moved > lo_step in the z direction
    if (!TEST(u->Flags, SPR_NO_SCAREDZ | SPR_JUMPING | SPR_CLIMBING | SPR_FALLING | SPR_DEAD | SPR_SWIMMING))
    {
        if (labs(sp->z - globloz) > u->lo_step)
        {
            // cancel move
            sp->x = x;
            sp->y = y;
            sp->z = z;
            //sp->z = u->loz;             // place on ground in case you are in the air
            u->loz = loz;
            u->hiz = hiz;
            u->lowActor = lowActor;
            u->highActor = highActor;
            u->lo_sectp = lo_sectp;
            u->hi_sectp = hi_sectp;
            u->coll.invalidate();
            ChangeActorSect(actor, sectnum);
            return false;
        }

        if (ActorDrop(actor, sp->x, sp->y, sp->z, sp->sectnum, u->lo_step))
        {
            // cancel move
            sp->x = x;
            sp->y = y;
            sp->z = z;
            //sp->z = u->loz;             // place on ground in case you are in the air
            u->loz = loz;
            u->hiz = hiz;
            u->lowActor = lowActor;
            u->highActor = highActor;
            u->lo_sectp = lo_sectp;
            u->hi_sectp = hi_sectp;
            u->coll.invalidate();
            ChangeActorSect(actor, sectnum);
            return false;
        }
    }

    SET(u->Flags, SPR_MOVED);

    if (u->coll.type == kHitNone)
    {
        // Keep track of how far sprite has moved
        dist = Distance(x, y, sp->x, sp->y);
        u->TargetDist -= dist;
        u->Dist += dist;
        u->DistCheck += dist;
        return true;
    }
    else
    {
        return false;
    }

}

int DoStayOnFloor(DSWActor* actor)
{
    actor->s().z = sector[actor->s().sectnum].floorz;
    return 0;
}

int
DoGrating(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();
    int dir;
    const int GRATE_FACTOR = 3;

    // reduce to 0 to 3 value
    dir = sp->ang >> 9;

    if ((dir & 1) == 0)
    {
        if (dir == 0)
            sp->x += 2 * GRATE_FACTOR;
        else
            sp->x -= 2 * GRATE_FACTOR;
    }
    else
    {
        if (dir == 1)
            sp->y += 2 * GRATE_FACTOR;
        else
            sp->y -= 2 * GRATE_FACTOR;
    }

    sp->hitag -= GRATE_FACTOR;

    if (sp->hitag <= 0)
    {
        change_actor_stat(actor, STAT_DEFAULT);
        actor->clearUser();
    }

    SetActorZ(actor, &sp->pos);

    return 0;
}



int DoKey(DSWActor* actor)
{
    USER* u = actor->u();
    SPRITEp sp = &actor->s();

    sp->ang = NORM_ANGLE(sp->ang + (14 * ACTORMOVETICS));

    DoGet(actor);
    return 0;
}

int DoCoin(DSWActor* actor)
{
    USER* u = actor->u();
    int offset;

    u->WaitTics -= ACTORMOVETICS * 2;

    if (u->WaitTics <= 0)
    {
        KillActor(actor);
        return 0;
    }

    if (u->WaitTics < 10*120)
    {
        if (u->StateStart != s_GreenCoin)
        {
            offset = int(u->State - u->StateStart);
            ChangeState(actor, s_GreenCoin);
            u->State = u->StateStart + offset;
        }
    }
    else if (u->WaitTics < 20*120)
    {
        if (u->StateStart != s_YellowCoin)
        {
            offset = int(u->State - u->StateStart);
            ChangeState(actor, s_YellowCoin);
            u->State = u->StateStart + offset;
        }
    }

    return 0;
}

int KillGet(DSWActor* actor)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();

    USERp nu;
    SPRITEp np;

    switch (gNet.MultiGameType)
    {
    case MULTI_GAME_NONE:
    case MULTI_GAME_COOPERATIVE:
        KillActor(actor);
        break;
    case MULTI_GAME_COMMBAT:
    case MULTI_GAME_AI_BOTS:

        if (TEST(u->Flags2, SPR2_NEVER_RESPAWN))
        {
            KillActor(actor);
            break;
        }

        u->WaitTics = 30*120;
        SET(sp->cstat, CSTAT_SPRITE_INVISIBLE);

        // respawn markers
        if (!gNet.SpawnMarkers || sp->hitag == TAG_NORESPAWN_FLAG)  // No coin if it's a special flag
            break;

        auto actorNew = SpawnActor(STAT_ITEM, Red_COIN, s_RedCoin, sp->sectnum,
                          sp->x, sp->y, sp->z, 0, 0);

        np = &actorNew->s();
        nu = actorNew->u();

        np->shade = -20;
        nu->WaitTics = u->WaitTics - 12;

        break;
    }
    return 0;
}

int KillGetAmmo(DSWActor* actor)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();

    USERp nu;
    SPRITEp np;

    switch (gNet.MultiGameType)
    {
    case MULTI_GAME_NONE:
    case MULTI_GAME_COOPERATIVE:
        KillActor(actor);
        break;

    case MULTI_GAME_COMMBAT:
    case MULTI_GAME_AI_BOTS:

        if (TEST(u->Flags2, SPR2_NEVER_RESPAWN))
        {
            KillActor(actor);
            break;
        }

        // No Respawn mode - all ammo goes away
        if (gNet.NoRespawn)
        {
            KillActor(actor);
            break;
        }

        u->WaitTics = 30*120;
        SET(sp->cstat, CSTAT_SPRITE_INVISIBLE);

        // respawn markers
        if (!gNet.SpawnMarkers)
            break;

        auto actorNew = SpawnActor(STAT_ITEM, Red_COIN, s_RedCoin, sp->sectnum,
                          sp->x, sp->y, sp->z, 0, 0);

        np = &actorNew->s();
        nu = actorNew->u();

        np->shade = -20;
        nu->WaitTics = u->WaitTics - 12;

        break;
    }
    return 0;
}

int KillGetWeapon(DSWActor* actor)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();

    USERp nu;
    SPRITEp np;

    switch (gNet.MultiGameType)
    {
    case MULTI_GAME_NONE:
        KillActor(actor);
        break;

    case MULTI_GAME_COOPERATIVE:
        // don't kill weapons in coop

        // unless told too :)
        if (TEST(u->Flags2, SPR2_NEVER_RESPAWN))
        {
            KillActor(actor);
            break;
        }
        break;

    case MULTI_GAME_COMMBAT:
    case MULTI_GAME_AI_BOTS:

        if (TEST(u->Flags2, SPR2_NEVER_RESPAWN))
        {
            KillActor(actor);
            break;
        }

        // No Respawn mode - all weapons stay
        // but can only get once
        if (gNet.NoRespawn)
            break;

        u->WaitTics = 30*120;
        SET(sp->cstat, CSTAT_SPRITE_INVISIBLE);

        // respawn markers
        if (!gNet.SpawnMarkers)
            break;

        auto actorNew = SpawnActor(STAT_ITEM, Red_COIN, s_RedCoin, sp->sectnum,
                          sp->x, sp->y, sp->z, 0, 0);

        np = &actorNew->s();
        nu = actorNew->u();

        np->shade = -20;
        nu->WaitTics = u->WaitTics - 12;

        break;
    }
    return 0;
}

int
DoSpawnItemTeleporterEffect(SPRITEp sp)
{
    extern STATE s_TeleportEffect[];
    SPRITEp ep;

    auto effect = SpawnActor(STAT_MISSILE, 0, s_TeleportEffect, sp->sectnum,
                         sp->x, sp->y, sp->z - Z(12),
                         sp->ang, 0);

    ep = &effect->s();

    ep->shade = -40;
    ep->xrepeat = ep->yrepeat = 36;
    SET(ep->cstat, CSTAT_SPRITE_YCENTER);
    RESET(ep->cstat, CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN);
    return 0;
}

void ChoosePlayerGetSound(PLAYERp pp)
{
    int choose_snd=0;

    if (pp != Player+myconnectindex) return;

    choose_snd = STD_RANDOM_RANGE((MAX_GETSOUNDS-1)<<8)>>8;

    PlayerSound(PlayerGetItemVocs[choose_snd], v3df_follow|v3df_dontpan,pp);
}

bool CanGetWeapon(PLAYERp pp, DSWActor* actor, int WPN)
{
    USERp u = actor->u();

    switch (gNet.MultiGameType)
    {
    case MULTI_GAME_NONE:
        return true;

    case MULTI_GAME_COOPERATIVE:
        if (TEST(u->Flags2, SPR2_NEVER_RESPAWN))
            return true;

        if (TEST(pp->WpnGotOnceFlags, BIT(WPN)))
            return false;

        return true;

    case MULTI_GAME_COMMBAT:
    case MULTI_GAME_AI_BOTS:

        if (TEST(u->Flags2, SPR2_NEVER_RESPAWN))
            return true;

        // No Respawn - can't get a weapon again if you already got it
        if (gNet.NoRespawn && TEST(pp->WpnGotOnceFlags, BIT(WPN)))
            return false;

        return true;
    }

    return true;
}

struct InventoryDecl_t InventoryDecls[InvDecl_TOTAL] =
{
    {50  },
    {100 },
    {20  },
    {50  },
    {100 },
    {1   },
    {2   },
    {3   },
    {100 },
    {100 },
    {100 },
};

enum
{
    ITEMFLASHAMT = -8,
    ITEMFLASHCLR = 144
};
int DoGet(DSWActor* actor)
{
    USER* u = actor->u();
	USERp pu;
    SPRITEp sp = &actor->s();
    PLAYERp pp;
    short pnum, key_num;
    int dist, a,b,c;
    bool can_see;
    int cstat_bak;

    // For flag stuff
    USERp nu;
    SPRITEp np;


    // Invisiblility is only used for DeathMatch type games
    // Sprites stays invisible for a period of time and is un-gettable
    // then "Re-Spawns" by becomming visible.  Its never actually killed.
    if (TEST(sp->cstat, CSTAT_SPRITE_INVISIBLE))
    {
        u->WaitTics -= ACTORMOVETICS * 2;
        if (u->WaitTics <= 0)
        {
            PlaySound(DIGI_ITEM_SPAWN, actor, v3df_none);
            DoSpawnItemTeleporterEffect(sp);
            RESET(sp->cstat, CSTAT_SPRITE_INVISIBLE);
        }

        return 0;
    }

    if (sp->xvel)
    {
        if (!DoItemFly(actor))
        {
            sp->xvel = 0;
            change_actor_stat(actor, STAT_ITEM);
        }
    }

    TRAVERSE_CONNECT(pnum)
    {
        pp = &Player[pnum];
        pu = pp->Actor()->u();

        if (TEST(pp->Flags, PF_DEAD))
            continue;

        DISTANCE(pp->posx, pp->posy, sp->x, sp->y, dist, a,b,c);
        if ((unsigned)dist > (pu->Radius + u->Radius))
        {
            continue;
        }

        if (!SpriteOverlap(actor, pp->Actor()))
        {
            continue;
        }

        cstat_bak = sp->cstat;
        SET(sp->cstat, CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN);
        can_see = FAFcansee(sp->x, sp->y, sp->z, sp->sectnum,
                            pp->posx, pp->posy, pp->posz, pp->cursectnum);
        sp->cstat = cstat_bak;

        if (!can_see)
        {
            continue;
        }

        switch (u->ID)
        {
        //
        // Keys
        //
        case RED_CARD:
        case RED_KEY:
            key_num = 0;
            goto KeyMain;
        case BLUE_CARD:
        case BLUE_KEY:
            key_num = 1;
            goto KeyMain;
        case GREEN_CARD:
        case GREEN_KEY:
            key_num = 2;
            goto KeyMain;
        case YELLOW_CARD:
        case YELLOW_KEY:
            key_num = 3;
            goto KeyMain;
        case GOLD_SKELKEY:
            key_num = 4;
            goto KeyMain;
        case SILVER_SKELKEY:
            key_num = 5;
            goto KeyMain;
        case BRONZE_SKELKEY:
            key_num = 6;
            goto KeyMain;
        case RED_SKELKEY:
            key_num = 7;
KeyMain:

            if (pp->HasKey[key_num])
                break;

            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_KEYMSG + key_num));

            pp->HasKey[key_num] = true;
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_KEY, actor, v3df_dontpan);

            // don't kill keys in coop
            if (gNet.MultiGameType == MULTI_GAME_COOPERATIVE)
                break;

            KillActor(actor);
            break;

        case ICON_ARMOR:
            if (pp->Armor < InventoryDecls[InvDecl_Kevlar].amount)
            {
                if (u->spal == PALETTE_PLAYER3)
                {
                    PlayerUpdateArmor(pp, 1000+InventoryDecls[InvDecl_Kevlar].amount);
                    PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_INVENTORY + InvDecl_Kevlar));
                }
                else
                {
                    if (pp->Armor < InventoryDecls[InvDecl_Armor].amount)
                    {
                        PlayerUpdateArmor(pp, 1000+InventoryDecls[InvDecl_Armor].amount);
                        PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_INVENTORY + InvDecl_Armor));
                    }
                    else
                        break;
                }
                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_BIGITEM, actor, v3df_dontpan);

                // override for respawn mode
                if (gNet.MultiGameType == MULTI_GAME_COMMBAT && gNet.NoRespawn)
                {
                    KillActor(actor);
                    break;
                }

                KillGet(actor);
            }
            break;

        //
        // Health - Instant Use
        //

        case ICON_SM_MEDKIT:
            if (pu->Health < 100)
            {
                bool putbackmax=false;

                PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_INVENTORY + InvDecl_SmMedkit));

                if (pp->MaxHealth == 200)
                {
                    pp->MaxHealth = 100;
                    putbackmax = true;
                }
                PlayerUpdateHealth(pp, InventoryDecls[InvDecl_SmMedkit].amount);

                if (putbackmax) pp->MaxHealth = 200;

                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);

                // override for respawn mode
                if (gNet.MultiGameType == MULTI_GAME_COMMBAT && gNet.NoRespawn)
                {
                    KillActor(actor);
                    break;
                }

                KillGet(actor);
            }
            break;

        case ICON_BOOSTER:   // Fortune cookie
            pp->MaxHealth = 200;
            if (pu->Health < 200)
            {
                PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_INVENTORY + InvDecl_Booster));
                PlayerUpdateHealth(pp, InventoryDecls[InvDecl_Booster].amount);       // This is for health
                // over 100%
                // Say something witty
                if (pp == Player+myconnectindex)
                {
                    int cookie = STD_RANDOM_RANGE(MAX_FORTUNES);
                    // print to the console, and the user quote display.
                    FStringf msg("%s %s", GStrings("TXTS_FORTUNE"), quoteMgr.GetQuote(QUOTE_COOKIE + cookie));
                    Printf(PRINT_NONOTIFY, TEXTCOLOR_SAPPHIRE "%s\n", msg.GetChars());
                    if (hud_messages)
                    {
                        strncpy(pp->cookieQuote, msg, 255);
                        pp->cookieQuote[255] = 0;
                        pp->cookieTime = 540;
                    }
                }

                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_BIGITEM, actor, v3df_dontpan);

                // override for respawn mode
                if (gNet.MultiGameType == MULTI_GAME_COMMBAT && gNet.NoRespawn)
                {
                    KillActor(actor);
                    break;
                }

                KillGet(actor);
            }
            break;

        //
        // Inventory
        //
        case ICON_MEDKIT:

            if (!pp->InventoryAmount[INVENTORY_MEDKIT] || pp->InventoryPercent[INVENTORY_MEDKIT] < InventoryDecls[InvDecl_Medkit].amount)
            {
                PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_INVENTORY + InvDecl_Medkit));
                pp->InventoryPercent[INVENTORY_MEDKIT] = InventoryDecls[InvDecl_Medkit].amount;
                pp->InventoryAmount[INVENTORY_MEDKIT] = 1;
                PlayerUpdateInventory(pp, INVENTORY_MEDKIT);
                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);

                // override for respawn mode
                if (gNet.MultiGameType == MULTI_GAME_COMMBAT && gNet.NoRespawn)
                {
                    KillActor(actor);
                    break;
                }

                KillGet(actor);
            }
            break;

        case ICON_CHEMBOMB:

            if (pp->InventoryAmount[INVENTORY_CHEMBOMB] < InventoryDecls[InvDecl_ChemBomb].amount)
            {
                PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_INVENTORY + InvDecl_ChemBomb));
                pp->InventoryPercent[INVENTORY_CHEMBOMB] = 0;
                pp->InventoryAmount[INVENTORY_CHEMBOMB]++;
                PlayerUpdateInventory(pp, INVENTORY_CHEMBOMB);
                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);
                KillGet(actor);
            }
            break;

        case ICON_FLASHBOMB:

            if (pp->InventoryAmount[INVENTORY_FLASHBOMB] < InventoryDecls[InvDecl_FlashBomb].amount)
            {
                PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_INVENTORY + InvDecl_FlashBomb));
                pp->InventoryPercent[INVENTORY_FLASHBOMB] = 0;
                pp->InventoryAmount[INVENTORY_FLASHBOMB]++;
                PlayerUpdateInventory(pp, INVENTORY_FLASHBOMB);
                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);
                KillGet(actor);
            }
            break;

        case ICON_CALTROPS:

            if (pp->InventoryAmount[INVENTORY_CALTROPS] < InventoryDecls[InvDecl_Caltrops].amount)
            {
                PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_INVENTORY + InvDecl_Caltrops));
                pp->InventoryPercent[INVENTORY_CALTROPS] = 0;
                pp->InventoryAmount[INVENTORY_CALTROPS]+=3;
                if (pp->InventoryAmount[INVENTORY_CALTROPS] > InventoryDecls[InvDecl_Caltrops].amount)
                    pp->InventoryAmount[INVENTORY_CALTROPS] = InventoryDecls[InvDecl_Caltrops].amount;
                PlayerUpdateInventory(pp, INVENTORY_CALTROPS);
                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);
                KillGet(actor);
            }
            break;

        case ICON_NIGHT_VISION:
            if (!pp->InventoryAmount[INVENTORY_NIGHT_VISION] || pp->InventoryPercent[INVENTORY_NIGHT_VISION] < InventoryDecls[InvDecl_NightVision].amount)
            {
                PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_INVENTORY + InvDecl_NightVision));
                pp->InventoryPercent[INVENTORY_NIGHT_VISION] = InventoryDecls[InvDecl_NightVision].amount;
                pp->InventoryAmount[INVENTORY_NIGHT_VISION] = 1;
                PlayerUpdateInventory(pp, INVENTORY_NIGHT_VISION);
                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);
                KillGet(actor);
            }
            break;
        case ICON_REPAIR_KIT:
            if (!pp->InventoryAmount[INVENTORY_REPAIR_KIT] || pp->InventoryPercent[INVENTORY_REPAIR_KIT] < InventoryDecls[InvDecl_RepairKit].amount)
            {
                PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_INVENTORY + InvDecl_RepairKit));
                pp->InventoryPercent[INVENTORY_REPAIR_KIT] = InventoryDecls[InvDecl_RepairKit].amount;
                pp->InventoryAmount[INVENTORY_REPAIR_KIT] = 1;
                PlayerUpdateInventory(pp, INVENTORY_REPAIR_KIT);
                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);

                // don't kill repair kit in coop
                if (gNet.MultiGameType == MULTI_GAME_COOPERATIVE)
                    break;

                KillGet(actor);
            }
            break;
#if 0
        case ICON_ENVIRON_SUIT:
            if (!pp->InventoryAmount[INVENTORY_ENVIRON_SUIT] || pp->InventoryPercent[INVENTORY_ENVIRON_SUIT] < 100)
            {
                pp->InventoryPercent[INVENTORY_ENVIRON_SUIT] = 100;
                pp->InventoryAmount[INVENTORY_ENVIRON_SUIT] = 1;
                PlayerUpdateInventory(pp, INVENTORY_ENVIRON_SUIT);
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);
                KillGet(actor);
            }
            break;
#endif
        case ICON_CLOAK:
            if (!pp->InventoryAmount[INVENTORY_CLOAK] || pp->InventoryPercent[INVENTORY_CLOAK] < InventoryDecls[InvDecl_Cloak].amount)
            {
                PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_INVENTORY + InvDecl_Cloak));
                pp->InventoryPercent[INVENTORY_CLOAK] = InventoryDecls[InvDecl_Cloak].amount;
                pp->InventoryAmount[INVENTORY_CLOAK] = 1;
                PlayerUpdateInventory(pp, INVENTORY_CLOAK);
                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);
                KillGet(actor);
            }
            break;
        //
        // Weapon
        //
        case ICON_STAR:

            if (!CanGetWeapon(pp, actor, WPN_STAR))
                break;

            SET(pp->WpnGotOnceFlags, BIT(WPN_STAR));

            if (pp->WpnAmmo[WPN_STAR] >= DamageData[WPN_STAR].max_ammo)
                break;

            PutStringInfo(Player+pnum, sw_darts? GStrings("TXTS_DARTS") : quoteMgr.GetQuote(QUOTE_WPNSHURIKEN));
            PlayerUpdateAmmo(pp, WPN_STAR, DamageData[WPN_STAR].weapon_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            KillGetWeapon(actor);
            if (TEST(pp->WpnFlags, BIT(WPN_STAR)))
                break;
            SET(pp->WpnFlags, BIT(WPN_STAR));

            if (!cl_weaponswitch)
                break;
            if (pu->WeaponNum <= WPN_STAR && pu->WeaponNum != WPN_SWORD)
                break;
            InitWeaponStar(pp);
            break;

        case ICON_LG_MINE:

            if (!CanGetWeapon(pp, actor, WPN_MINE))
                break;

            SET(pp->WpnGotOnceFlags, BIT(WPN_MINE));

            if (pp->WpnAmmo[WPN_MINE] >= DamageData[WPN_MINE].max_ammo)
                break;
            //sprintf(ds,"Sticky Bombs");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_WPNSTICKY));
            PlayerUpdateAmmo(pp, WPN_MINE, DamageData[WPN_MINE].weapon_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            ChoosePlayerGetSound(pp);
            KillGetWeapon(actor);
            if (TEST(pp->WpnFlags, BIT(WPN_MINE)))
                break;
            SET(pp->WpnFlags, BIT(WPN_MINE));

            if (!cl_weaponswitch)
                break;
            if (pu->WeaponNum > WPN_MINE && pu->WeaponNum != WPN_SWORD)
                break;
            InitWeaponMine(pp);
            break;

        case ICON_UZI:
        case ICON_UZIFLOOR:

            if (!CanGetWeapon(pp, actor, WPN_UZI))
                break;

            SET(pp->WpnGotOnceFlags, BIT(WPN_UZI));

            if (TEST(pp->Flags, PF_TWO_UZI) && pp->WpnAmmo[WPN_UZI] >= DamageData[WPN_UZI].max_ammo)
                break;
            //sprintf(ds,"UZI Submachine Gun");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_WPNUZI));
//            pp->WpnAmmo[WPN_UZI] += 50;
            PlayerUpdateAmmo(pp, WPN_UZI, DamageData[WPN_UZI].weapon_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            KillGetWeapon(actor);

            if (TEST(pp->WpnFlags, BIT(WPN_UZI)) && TEST(pp->Flags, PF_TWO_UZI))
                break;
            // flag to help with double uzi powerup - simpler but kludgy
            SET(pp->Flags, PF_PICKED_UP_AN_UZI);
            if (TEST(pp->WpnFlags, BIT(WPN_UZI)))
            {
                SET(pp->Flags, PF_TWO_UZI);
                pp->WpnUziType = 0; // Let it come up
                if (pp == Player+myconnectindex)
                    PlayerSound(DIGI_DOUBLEUZI, v3df_dontpan|v3df_follow, pp);
            }
            else
            {
                SET(pp->WpnFlags, BIT(WPN_UZI));
                ChoosePlayerGetSound(pp);
            }

            if (!cl_weaponswitch)
                break;

            if (pu->WeaponNum > WPN_UZI && pu->WeaponNum != WPN_SWORD)
                break;

            InitWeaponUzi(pp);
            break;

        case ICON_LG_UZI_AMMO:
            if (pp->WpnAmmo[WPN_UZI] >= DamageData[WPN_UZI].max_ammo)
                break;
            //sprintf(ds,"UZI Clip");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_AMMOUZI));
            PlayerUpdateAmmo(pp, WPN_UZI, DamageData[WPN_UZI].ammo_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            KillGetAmmo(actor);
            break;

        case ICON_MICRO_GUN:

            if (!CanGetWeapon(pp, actor, WPN_MICRO))
                break;

            SET(pp->WpnGotOnceFlags, BIT(WPN_MICRO));

            if (TEST(pp->WpnFlags, BIT(WPN_MICRO)) && pp->WpnAmmo[WPN_MICRO] >= DamageData[WPN_MICRO].max_ammo)
                break;
            //sprintf(ds,"Missile Launcher");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_WPNLAUNCH));
//            pp->WpnAmmo[WPN_MICRO] += 5;
            PlayerUpdateAmmo(pp, WPN_MICRO, DamageData[WPN_MICRO].weapon_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            ChoosePlayerGetSound(pp);
            KillGetWeapon(actor);
            if (TEST(pp->WpnFlags, BIT(WPN_MICRO)))
                break;
            SET(pp->WpnFlags, BIT(WPN_MICRO));

            if (!cl_weaponswitch)
                break;
            if (pu->WeaponNum > WPN_MICRO && pu->WeaponNum != WPN_SWORD)
                break;
            InitWeaponMicro(pp);
            break;

        case ICON_MICRO_BATTERY:
            if (pp->WpnAmmo[WPN_MICRO] >= DamageData[WPN_MICRO].max_ammo)
                break;
            //sprintf(ds,"Missiles");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_AMMOLAUNCH));
            PlayerUpdateAmmo(pp, WPN_MICRO, DamageData[WPN_MICRO].ammo_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            KillGetAmmo(actor);
            break;

        case ICON_NUKE:
            if (pp->WpnRocketNuke != 1)
            {
                //sprintf(ds,"Nuclear Warhead");
                PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_WPNNUKE));
                pp->WpnRocketNuke =uint8_t(DamageData[DMG_NUCLEAR_EXP].weapon_pickup);
                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);
                if (STD_RANDOM_RANGE(1000) > 800 && pp == Player+myconnectindex)
                    PlayerSound(DIGI_ILIKENUKES, v3df_dontpan|v3df_doppler|v3df_follow,pp);
                if (pp->CurWpn == pp->Wpn[WPN_MICRO])
                {
                    if (pp->WpnRocketType != 2)
                    {
                        extern PANEL_STATE ps_MicroNukeFlash[];
                        pp->CurWpn->over[MICRO_SHOT_NUM].tics = 0;
                        pp->CurWpn->over[MICRO_SHOT_NUM].State = ps_MicroNukeFlash;
                        // Play Nuke available sound here!
                    }

                }

                KillGetAmmo(actor);
            }
            break;

        case ICON_GRENADE_LAUNCHER:
            if (!CanGetWeapon(pp, actor, WPN_GRENADE))
                break;

            SET(pp->WpnGotOnceFlags, BIT(WPN_GRENADE));

            if (TEST(pp->WpnFlags, BIT(WPN_GRENADE)) && pp->WpnAmmo[WPN_GRENADE] >= DamageData[WPN_GRENADE].max_ammo)
                break;
            //sprintf(ds,"Grenade Launcher");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_WPNGRENADE));
//            pp->WpnAmmo[WPN_GRENADE] += 6;
            PlayerUpdateAmmo(pp, WPN_GRENADE, DamageData[WPN_GRENADE].weapon_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            //ChoosePlayerGetSound(pp);
            if (STD_RANDOM_RANGE(1000) > 800 && pp == Player+myconnectindex)
                PlayerSound(DIGI_LIKEBIGWEAPONS, v3df_dontpan|v3df_doppler|v3df_follow,pp);
            KillGetWeapon(actor);
            if (TEST(pp->WpnFlags, BIT(WPN_GRENADE)))
                break;
            SET(pp->WpnFlags, BIT(WPN_GRENADE));

            if (!cl_weaponswitch)
                break;
            if (pu->WeaponNum > WPN_GRENADE && pu->WeaponNum != WPN_SWORD)
                break;
            InitWeaponGrenade(pp);
            break;

        case ICON_LG_GRENADE:
            if (pp->WpnAmmo[WPN_GRENADE] >= DamageData[WPN_GRENADE].max_ammo)
                break;
            //sprintf(ds,"Grenade Shells");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_AMMOGRENADE));
            PlayerUpdateAmmo(pp, WPN_GRENADE, DamageData[WPN_GRENADE].ammo_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            KillGetAmmo(actor);
            break;

#if 0
        case ICON_ROCKET:
            pp->WpnAmmo[WPN_ROCKET] += 15;
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            KillGet(actor);
            if (TEST(pp->WpnFlags, BIT(WPN_ROCKET)))
                break;
            SET(pp->WpnFlags, BIT(WPN_ROCKET));

            if (!cl_weaponswitch)
                break;
            InitWeaponRocket(pp);
            break;

        case ICON_LG_ROCKET:
            sprintf(ds,"20 Missiles");
            PutStringInfo(Player+pnum, ds);
            PlayerUpdateAmmo(pp, WPN_ROCKET, 20);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            KillGet(actor);
            break;
#endif

        case ICON_RAIL_GUN:
            if (SW_SHAREWARE) { KillActor(actor); break; }

            if (!CanGetWeapon(pp, actor, WPN_RAIL))
                break;

            SET(pp->WpnGotOnceFlags, BIT(WPN_RAIL));

            if (TEST(pp->WpnFlags, BIT(WPN_RAIL)) && pp->WpnAmmo[WPN_RAIL] >= DamageData[WPN_RAIL].max_ammo)
                break;
            //sprintf(ds,"Rail Gun");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_WPNRAILGUN));
            PlayerUpdateAmmo(pp, WPN_RAIL, DamageData[WPN_RAIL].weapon_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            if (pp == Player+myconnectindex)
            {
                if (STD_RANDOM_RANGE(1000) > 700)
                    PlayerSound(DIGI_LIKEBIGWEAPONS, v3df_dontpan|v3df_doppler|v3df_follow,pp);
                else
                    PlayerSound(DIGI_GOTRAILGUN, v3df_dontpan|v3df_doppler|v3df_follow,pp);
            }
            //ChoosePlayerGetSound(pp);
            KillGetWeapon(actor);
            if (TEST(pp->WpnFlags, BIT(WPN_RAIL)))
                break;
            SET(pp->WpnFlags, BIT(WPN_RAIL));

            if (!cl_weaponswitch)
                break;
            if (pu->WeaponNum > WPN_RAIL && pu->WeaponNum != WPN_SWORD)
                break;
            InitWeaponRail(pp);
            break;

        case ICON_RAIL_AMMO:
            if (SW_SHAREWARE) { KillActor(actor); break; }

            if (pp->WpnAmmo[WPN_RAIL] >= DamageData[WPN_RAIL].max_ammo)
                break;
            //sprintf(ds,"Rail Gun Rods");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_AMMORAILGUN));
            PlayerUpdateAmmo(pp, WPN_RAIL, DamageData[WPN_RAIL].ammo_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            KillGetAmmo(actor);
            break;

        case ICON_SHOTGUN:
            if (!CanGetWeapon(pp, actor, WPN_SHOTGUN))
                break;

            SET(pp->WpnGotOnceFlags, BIT(WPN_SHOTGUN));

            if (TEST(pp->WpnFlags, BIT(WPN_SHOTGUN)) && pp->WpnAmmo[WPN_SHOTGUN] >= DamageData[WPN_SHOTGUN].max_ammo)
                break;
            //sprintf(ds,"Riot Gun");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_WPNRIOT));
//            pp->WpnAmmo[WPN_SHOTGUN] += 10;
            PlayerUpdateAmmo(pp, WPN_SHOTGUN, DamageData[WPN_SHOTGUN].weapon_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            ChoosePlayerGetSound(pp);
            KillGetWeapon(actor);
            if (TEST(pp->WpnFlags, BIT(WPN_SHOTGUN)))
                break;
            SET(pp->WpnFlags, BIT(WPN_SHOTGUN));

            if (!cl_weaponswitch)
                break;
            if (pu->WeaponNum > WPN_SHOTGUN && pu->WeaponNum != WPN_SWORD)
                break;
            InitWeaponShotgun(pp);
            break;

        case ICON_LG_SHOTSHELL:
            if (pp->WpnAmmo[WPN_SHOTGUN] >= DamageData[WPN_SHOTGUN].max_ammo)
                break;
            //sprintf(ds,"Shotshells");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_AMMORIOT));
            PlayerUpdateAmmo(pp, WPN_SHOTGUN, DamageData[WPN_SHOTGUN].ammo_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            KillGetAmmo(actor);
            break;

#if 0
        case ICON_AUTORIOT:
            if (pp->WpnShotgunAuto != 50)
            {
                sprintf(ds,"Riot Gun TurboDrive, +50 12-Gauge Slugs");
                PutStringInfo(Player+pnum, ds);
                pp->WpnShotgunAuto = 50;
                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);
                KillGet(actor);
                if (pp->CurWpn == pp->Wpn[WPN_SHOTGUN])
                {
                    if (pp->WpnShotgunType != 1)
                    {
                        extern PANEL_STATE ps_ShotgunFlash[];
                        pp->CurWpn->over[SHOTGUN_AUTO_NUM].tics = 0;
                        pp->CurWpn->over[SHOTGUN_AUTO_NUM].State = ps_ShotgunFlash;
                    }

                }
            }
            break;
#endif

        case ICON_GUARD_HEAD:
            if (SW_SHAREWARE) { KillActor(actor); break; }

            if (!CanGetWeapon(pp, actor, WPN_HOTHEAD))
                break;

            SET(pp->WpnGotOnceFlags, BIT(WPN_HOTHEAD));

            if (TEST(pp->WpnFlags, BIT(WPN_HOTHEAD)) && pp->WpnAmmo[WPN_HOTHEAD] >= DamageData[WPN_HOTHEAD].max_ammo)
                break;
            //sprintf(ds,"Guardian Head");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_WPNHEAD));
            PlayerUpdateAmmo(pp, WPN_HOTHEAD, DamageData[WPN_HOTHEAD].weapon_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            //ChoosePlayerGetSound(pp);
            if (STD_RANDOM_RANGE(1000) > 800 && pp == Player+myconnectindex)
                PlayerSound(DIGI_LIKEBIGWEAPONS, v3df_dontpan|v3df_doppler|v3df_follow,pp);
            KillGetWeapon(actor);
            if (TEST(pp->WpnFlags, BIT(WPN_HOTHEAD)))
                break;
            SET(pp->WpnFlags, BIT(WPN_NAPALM) | BIT(WPN_RING) | BIT(WPN_HOTHEAD));

            if (!cl_weaponswitch)
                break;
            if (pu->WeaponNum > WPN_HOTHEAD && pu->WeaponNum != WPN_SWORD)
                break;
            InitWeaponHothead(pp);
            break;

        case ICON_FIREBALL_LG_AMMO:
            if (SW_SHAREWARE) { KillActor(actor); break; }

            if (pp->WpnAmmo[WPN_HOTHEAD] >= DamageData[WPN_HOTHEAD].max_ammo)
                break;
            //sprintf(ds,"Firebursts");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_AMMOHEAD));
            PlayerUpdateAmmo(pp, WPN_HOTHEAD, DamageData[WPN_HOTHEAD].ammo_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            KillGetAmmo(actor);
            break;

        case ICON_HEART:
            if (SW_SHAREWARE) { KillActor(actor); break; }

            if (!CanGetWeapon(pp, actor, WPN_HEART))
                break;

            SET(pp->WpnGotOnceFlags, BIT(WPN_HEART));

            if (TEST(pp->WpnFlags, BIT(WPN_HEART)) && pp->WpnAmmo[WPN_HEART] >= DamageData[WPN_HEART].max_ammo)
                break;
            //sprintf(ds,"Ripper Heart");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_WPNRIPPER));
            PlayerUpdateAmmo(pp, WPN_HEART, DamageData[WPN_HEART].weapon_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            //ChoosePlayerGetSound(pp);
            if (STD_RANDOM_RANGE(1000) > 800 && pp == Player+myconnectindex)
                PlayerSound(DIGI_LIKEBIGWEAPONS, v3df_dontpan|v3df_doppler|v3df_follow,pp);
            KillGetWeapon(actor);
            if (TEST(pp->WpnFlags, BIT(WPN_HEART)))
                break;
            SET(pp->WpnFlags, BIT(WPN_HEART));

            if (!cl_weaponswitch)
                break;

            if (pu->WeaponNum > WPN_HEART && pu->WeaponNum != WPN_SWORD)
                break;

            InitWeaponHeart(pp);
            break;

        case ICON_HEART_LG_AMMO:
            if (SW_SHAREWARE) { KillActor(actor); break; }

            if (pp->WpnAmmo[WPN_HEART] >= DamageData[WPN_HEART].max_ammo)
                break;
            //sprintf(ds,"Deathcoils");
            PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_AMMORIPPER));
            PlayerUpdateAmmo(pp, WPN_HEART, DamageData[WPN_HEART].ammo_pickup);
            SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
            if (pp == Player+myconnectindex)
                PlaySound(DIGI_ITEM, actor, v3df_dontpan);
            KillGetAmmo(actor);
            break;

        case ICON_HEAT_CARD:
            if (pp->WpnRocketHeat != 5)
            {
                //sprintf(ds,"Heat Seeker Card");
                PutStringInfo(Player+pnum, quoteMgr.GetQuote(QUOTE_AMMONUKE));
                pp->WpnRocketHeat = uint8_t(DamageData[DMG_NUCLEAR_EXP].ammo_pickup);
                SetFadeAmt(pp,ITEMFLASHAMT,ITEMFLASHCLR);  // Flash blue on item pickup
                if (pp == Player+myconnectindex)
                    PlaySound(DIGI_ITEM, actor, v3df_dontpan);
                KillGet(actor);

                if (pp->CurWpn == pp->Wpn[WPN_MICRO])
                {
                    if (pp->WpnRocketType == 0)
                    {
                        pp->WpnRocketType = 1;
                    }
                    else if (pp->WpnRocketType == 2)
                    {
                        extern PANEL_STATE ps_MicroHeatFlash[];
                        pp->CurWpn->over[MICRO_HEAT_NUM].tics = 0;
                        pp->CurWpn->over[MICRO_HEAT_NUM].State = ps_MicroHeatFlash;
                    }

                }
            }
            break;

        case ICON_FLAG:
        {
            if (sp->pal == pp->Actor()->s().pal) break; // Can't pick up your own flag!

            PlaySound(DIGI_ITEM, actor, v3df_dontpan);

            DSWActor* actorNew;
            if (sp->hitag == TAG_NORESPAWN_FLAG)
                actorNew = SpawnActor(STAT_ITEM, ICON_FLAG, s_CarryFlagNoDet, sp->sectnum,
                                  sp->x, sp->y, sp->z, 0, 0);
            else
                actorNew = SpawnActor(STAT_ITEM, ICON_FLAG, s_CarryFlag, sp->sectnum,
                                  sp->x, sp->y, sp->z, 0, 0);

            np = &actorNew->s();
            nu = actorNew->u();
            np->shade = -20;

            // Attach flag to player
            nu->Counter = 0;
            RESET(np->cstat, CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN);
            SET(np->cstat, CSTAT_SPRITE_ALIGNMENT_WALL);
            SetAttach(pp->Actor(), actorNew);
            nu->sz = SPRITEp_MID(&pp->Actor()->s());  // Set mid way up who it hit
            nu->spal = np->pal = sp->pal;   // Set the palette of the flag

            SetOwner(pp->Actor(), actorNew);  // Player now owns the flag
            nu->flagOwnerActor = actor;       // Tell carried flag who owns it
            KillGet(actor);  // Set up for flag respawning
            break;
        }
        default:
            KillActor(actor);
        }
    }

    return 0;
}

// This function mostly only adjust the active_range field

void ProcessActiveVars(DSWActor* actor)
{
    USERp u = actor->u();
    const int TIME_TILL_INACTIVE = (4 * 120);

    if (!TEST(u->Flags, SPR_ACTIVE))
    {
        // if actor has been unaware for more than a few seconds
        u->inactive_time += ACTORMOVETICS;
        if (u->inactive_time > TIME_TILL_INACTIVE)
        {
            // reset to min update range
            u->active_range = MIN_ACTIVE_RANGE;
            // keep time low so it doesn't roll over
            u->inactive_time = TIME_TILL_INACTIVE;
        }
    }

    u->wait_active_check += ACTORMOVETICS;
}

void
AdjustActiveRange(PLAYERp pp, DSWActor* actor, int dist)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();
    SPRITEp psp = &pp->Actor()->s();
    int look_height;


    // do no FAFcansee before it is time
    if (u->wait_active_check < ACTIVE_CHECK_TIME)
        return;

    u->wait_active_check = 0;

    // check aboslute max
    if (dist > MAX_ACTIVE_RANGE)
        return;

    // do not do a FAFcansee if your already active
    // Actor only becomes INACTIVE in DoActorDecision
    if (TEST(u->Flags, SPR_ACTIVE))
        return;

    //
    // From this point on Actor is INACTIVE
    //

    // if actor can still see the player
    look_height = SPRITEp_TOS(sp);
    if (FAFcansee(sp->x, sp->y, look_height, sp->sectnum, psp->x, psp->y, SPRITEp_UPPER(psp), psp->sectnum))
    {
        // Player is visible
        // adjust update range of this sprite

        // some huge distance
        u->active_range = 75000;
        // sprite is AWARE
        SET(u->Flags, SPR_ACTIVE);
        u->inactive_time = 0;
    }
}


/*

  !AIC KEY - Reads state tables for animation frame transitions and handles
  calling animators, QUICK_CALLS, etc.  This is handled for many types of
  sprites not just actors.

*/

int  StateControl(DSWActor* actor)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();
    short StateTics;

    if (!u->State)
    {
        ASSERT(u->ActorActionFunc);
        (u->ActorActionFunc)(actor);
        return 0;
    }

    if (sp->statnum >= STAT_SKIP4_START && sp->statnum <= STAT_SKIP4_END)
        u->Tics += ACTORMOVETICS * 2;
    else
        u->Tics += ACTORMOVETICS;

    // Skip states if too much time has passed
    while (u->Tics >= TEST(u->State->Tics, SF_TICS_MASK))
    {
        StateTics = TEST(u->State->Tics, SF_TICS_MASK);

        if (TEST(u->State->Tics, SF_TIC_ADJUST))
        {
            ASSERT(u->Attrib);
            ASSERT(u->speed < MAX_SPEED);
            ASSERT(StateTics > -u->Attrib->TicAdjust[u->speed]);

            StateTics += u->Attrib->TicAdjust[u->speed];
        }

        // Set Tics
        u->Tics -= StateTics;

        // Transition to the next state
        u->State = u->State->NextState;

        // Look for flags embedded into the Tics variable
        while (TEST(u->State->Tics, SF_QUICK_CALL))
        {
            // Call it once and go to the next state
            (*u->State->Animator)(actor);

            ASSERT(u); //put this in to see if actor was getting killed with in his QUICK_CALL state

            if (!u)
                break;

            // if still on the same QUICK_CALL should you
            // go to the next state.
            if (TEST(u->State->Tics, SF_QUICK_CALL))
                u->State = u->State->NextState;
        }

        if (!u)
            break;

        if (!u->State->Pic)
        {
            NewStateGroup(actor, (STATEp *) u->State->NextState);
        }
    }

    if (u)
    {
        ASSERT(u->State);
        // Set picnum to the correct pic
        if (TEST(u->State->Tics, SF_WALL_STATE))
        {
            ASSERT(u->WallP);
            u->WallP->picnum = u->State->Pic;
        }
        else
        {
            if (u->RotNum > 1)
                sp->picnum = u->Rot[0]->Pic;
            else
                sp->picnum = u->State->Pic;
        }

        // Call the correct animator
        if (u->State->Animator && u->State->Animator != NullAnimator)
            (*u->State->Animator)(actor);
    }

    return 0;
}



void SpriteControl(void)
{
    int32_t stat;
    SPRITEp sp;
    USERp u;
    short pnum, CloseToPlayer;
    PLAYERp pp;
    int tx, ty, tmin, dist;
    short StateTics;

    SWStatIterator it(STAT_MISC);
    while (auto actor = it.Next())
    {
        StateControl(actor);
    }

    // Items and skip2 things
    if (MoveSkip2 == 0)
    {
        for (stat = STAT_SKIP2_START + 1; stat <= STAT_SKIP2_END; stat++)
        {
            it.Reset(stat);
            while (auto actor = it.Next())
            {
                StateControl(actor);
            }
        }
    }

    if (MoveSkip2 == 0)                 // limit to 20 times a second
    {
        // move bad guys around
        it.Reset(STAT_ENEMY);
        while (auto actor = it.Next())
        {
            if (!actor->hasU()) continue;

            u = actor->u();
            sp = &actor->s();

            CloseToPlayer = false;

            ProcessActiveVars(actor);

            TRAVERSE_CONNECT(pnum)
            {
                pp = &Player[pnum];

                // Only update the ones closest
                DISTANCE(pp->posx, pp->posy, sp->x, sp->y, dist, tx, ty, tmin);

                AdjustActiveRange(pp, actor, dist);

                if (dist < u->active_range)
                {
                    CloseToPlayer = true;
                }
            }

            RESET(u->Flags, SPR_MOVED);

            // Only update the ones close to ANY player
            if (CloseToPlayer)
            {
                StateControl(actor);
            }
            else
            {
                // to far away to be attacked
                RESET(u->Flags, SPR_ATTACKED);
            }
        }
    }

    // Skip4 things
    if (MoveSkip4 == 0)                 // limit to 10 times a second
    {
        for (stat = STAT_SKIP4_START; stat <= STAT_SKIP4_END; stat++)
        {
            it.Reset(stat);
            while (auto actor = it.Next())
            {
				StateControl(actor);
            }
        }
    }

    it.Reset(STAT_NO_STATE);
    while (auto actor = it.Next())
    {
        auto u = actor->u();
        if (u && u->ActorActionFunc)
            u->ActorActionFunc(actor);
    }

    if (MoveSkip8 == 0)
    {
        it.Reset(STAT_STATIC_FIRE);
        while (auto itActor = it.Next())
        {
            DoStaticFlamesDamage(itActor);
        }
    }

    if (MoveSkip4 == 0)                 // limit to 10 times a second
    {
        it.Reset(STAT_WALLBLOOD_QUEUE);
        while (auto actor = it.Next())
        {
			StateControl(actor);
        }
    }

    // vator/rotator/spike/slidor all have some code to
    // prevent calling of the action func()
    it.Reset(STAT_VATOR);
    while (auto actor = it.Next())
    {
        u = actor->u();

        if (u == 0)
            continue;
        if (u->Tics)
        {
            if ((u->Tics -= synctics) <= 0)
                SetVatorActive(actor);
            else
                continue;
        }

        if (!TEST(u->Flags, SPR_ACTIVE))
            continue;

        u->ActorActionFunc(actor);
    }

    it.Reset(STAT_SPIKE);
    while (auto actor = it.Next())
    {
        u = actor->u();

        if (u->Tics)
        {
            if ((u->Tics -= synctics) <= 0)
                SetSpikeActive(actor);
            else
                continue;
        }

        if (!TEST(u->Flags, SPR_ACTIVE))
            continue;

        u->ActorActionFunc(actor);
    }

    it.Reset(STAT_ROTATOR);
    while (auto actor = it.Next())
    {
        u = actor->u();

        if (u->Tics)
        {
            if ((u->Tics -= synctics) <= 0)
                SetRotatorActive(actor);
            else
                continue;
        }

        if (!TEST(u->Flags, SPR_ACTIVE))
            continue;

        u->ActorActionFunc(actor);
    }

    it.Reset(STAT_SLIDOR);
    while (auto actor = it.Next())
    {
        u = actor->u();

        if (u->Tics)
        {
            if ((u->Tics -= synctics) <= 0)
                SetSlidorActive(actor);
            else
                continue;
        }

        if (!TEST(u->Flags, SPR_ACTIVE))
            continue;

        u->ActorActionFunc(actor);
    }

    it.Reset(STAT_SUICIDE);
    while (auto actor = it.Next())
    {
        KillActor(actor);
    }
}


//
// This moves an actor about with FAFgetzrange clip adjustment
//

/*

  !AIC KEY - calls clipmove - Look through and try to understatnd  Should
  hopefully never have to change this.  Its very delicate.

*/

Collision move_sprite(DSWActor* actor, int xchange, int ychange, int zchange, int ceildist, int flordist, uint32_t cliptype, int numtics)
{
    Collision retval(0);
    int zh;
	int dasectnum;
	short tempshort;
    SPRITEp spr = &actor->s();
    USERp u = actor->u();
    short lastsectnum;

    ASSERT(actor->hasU());

    vec3_t clippos = spr->pos;

    // Can't modify sprite sectors
    // directly becuase of linked lists
    dasectnum = lastsectnum = spr->sectnum;

    if (TEST(spr->cstat, CSTAT_SPRITE_YCENTER))
    {
        zh = 0;
    }
    else
    {
        // move the center point up for moving
        zh = u->zclip;
        clippos.z -= zh;
    }


//    ASSERT(inside(spr->x,spr->y,dasectnum));

    int cmret = clipmove(&clippos, &dasectnum,
                      ((xchange * numtics) << 11), ((ychange * numtics) << 11),
                      (((int) spr->clipdist) << 2), ceildist, flordist, cliptype, 1);

    spr->pos.vec2 = clippos.vec2;

    if (dasectnum < 0)
    {
        retval.setWall(0); // this is wrong but what the original code did.
        return retval;
    }
    retval.setFromEngine(cmret);

    if ((dasectnum != spr->sectnum) && (dasectnum >= 0))
        ChangeActorSect(actor, dasectnum);

    // Set the blocking bit to 0 temporarly so FAFgetzrange doesn't pick
    // up its own sprite
    tempshort = spr->cstat;
    spr->cstat = 0;

    // I subtracted 8 from the clipdist because actors kept going up on
    // ledges they were not supposed to go up on.  Did the same for the
    // player. Seems to work ok!
    vec3_t pos = spr->pos;
    pos.z -= zh + 1;
    FAFgetzrange(pos, spr->sectnum,
                 &globhiz, &globhihit, &globloz, &globlohit,
                 (((int) spr->clipdist) << 2) - GETZRANGE_CLIP_ADJ, cliptype);

    spr->cstat = tempshort;

    // !AIC - puts getzrange results into USER varaible u->loz, u->hiz, u->lo_sectp, u->hi_sectp, etc.
    // Takes info from global variables
    DoActorGlobZ(actor);

    clippos.z = spr->z + ((zchange * numtics) >> 3);

    // test for hitting ceiling or floor
    if ((clippos.z - zh <= globhiz) || (clippos.z - zh > globloz))
    {
        if (retval.type == kHitNone)
        {
            if (TEST(u->Flags, SPR_CLIMBING))
            {
                spr->z = clippos.z;
                return retval;
            }

            retval.setSector(dasectnum);
        }
    }
    else
    {
        spr->z = clippos.z;
    }

    // extra processing for Stacks and warping
    if (FAF_ConnectArea(spr->sectnum))
        SetActorZ(actor, &spr->pos);

    if (TEST(sector[spr->sectnum].extra, SECTFX_WARP_SECTOR))
    {
        DSWActor* sp_warp;
        if ((sp_warp = WarpPlane(&spr->x, &spr->y, &spr->z, &dasectnum)))
        {
            ActorWarpUpdatePos(actor, dasectnum);
            ActorWarpType(actor, sp_warp);
        }

        if (spr->sectnum != lastsectnum)
        {
            if ((sp_warp = Warp(&spr->x, &spr->y, &spr->z, &dasectnum)))
            {
                ActorWarpUpdatePos(actor, dasectnum);
                ActorWarpType(actor, sp_warp);
            }
        }
    }

    return retval;
}

void MissileWarpUpdatePos(DSWActor* actor, short sectnum)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();
    sp->backuppos();
    u->oz = sp->oz;
    ChangeActorSect(actor, sectnum);
    MissileZrange(actor);
}

void ActorWarpUpdatePos(DSWActor* actor, short sectnum)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();
    sp->backuppos();
    u->oz = sp->oz;
    ChangeActorSect(actor, sectnum);
    DoActorZrange(actor);
}

void MissileWarpType(DSWActor* actor, DSWActor* act_warp)
{
    auto sp_warp = &act_warp->s();
    switch (SP_TAG1(sp_warp))
    {
    case WARP_CEILING_PLANE:
    case WARP_FLOOR_PLANE:
        return;
    }

    switch (SP_TAG3(sp_warp))
    {
    case 1:
        break;
    default:
        PlaySound(DIGI_ITEM_SPAWN, actor, v3df_none);
        DoSpawnItemTeleporterEffect(&actor->s());
        break;
    }
}

void ActorWarpType(DSWActor* actor, DSWActor* act_warp)
{
    auto sp_warp = &act_warp->s();
    switch (SP_TAG3(sp_warp))
    {
    case 1:
        break;
    default:
        PlaySound(DIGI_ITEM_SPAWN, actor, v3df_none);
        DoSpawnTeleporterEffectPlace(actor);
        break;
    }
}

//
// This moves a small projectile with FAFgetzrangepoint
//

int MissileWaterAdjust(DSWActor* actor)
{
    USERp u = actor->u();

    if (u->lo_sectp)
    {
        SECT_USERp sectu = SectUser[sectnum(u->lo_sectp)].Data();
        if (sectu && FixedToInt(sectu->depth_fixed))
            u->loz -= Z(FixedToInt(sectu->depth_fixed));
    }
    return 0;
}

int
MissileZrange(DSWActor* actor)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();
    short tempshort;

    // Set the blocking bit to 0 temporarly so FAFgetzrange doesn't pick
    // up its own sprite
    tempshort = sp->cstat;
    RESET(sp->cstat, CSTAT_SPRITE_BLOCK);

    FAFgetzrangepoint(sp->x, sp->y, sp->z - 1, sp->sectnum,
                      &globhiz, &globhihit, &globloz, &globlohit);

    sp->cstat = tempshort;

    DoActorGlobZ(actor);
    return 0;
}


Collision move_missile(DSWActor* actor, int xchange, int ychange, int zchange, int ceildist, int flordist, uint32_t cliptype, int numtics)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();
    Collision retval(0);
    int zh;
    int dasectnum, tempshort;
    short lastsectnum;

    ASSERT(actor->hasU());

    vec3_t clippos = sp->pos;

    // Can't modify sprite sectors
    // directly becuase of linked lists
    dasectnum = lastsectnum = sp->sectnum;

    if (TEST(sp->cstat, CSTAT_SPRITE_YCENTER))
    {
        zh = 0;
    }
    else
    {
        zh = u->zclip;
        clippos.z -= zh;
    }


    int cmret = clipmove(&clippos, &dasectnum,
                      ((xchange * numtics) << 11), ((ychange * numtics) << 11),
                      (((int) sp->clipdist) << 2), ceildist, flordist, cliptype, 1);
    sp->pos.vec2 = clippos.vec2;

    if (dasectnum < 0)
    {
        // we've gone beyond a white wall - kill it
        retval.setSky();
        return retval;
    }
    retval.setFromEngine(cmret);

    if ((dasectnum != sp->sectnum) && (dasectnum >= 0))
        ChangeActorSect(actor, dasectnum);

    // Set the blocking bit to 0 temporarly so FAFgetzrange doesn't pick
    // up its own sprite
    tempshort = sp->cstat;
    RESET(sp->cstat, CSTAT_SPRITE_BLOCK);

    FAFgetzrangepoint(sp->x, sp->y, sp->z - 1, sp->sectnum,
                      &globhiz, &globhihit, &globloz, &globlohit);

    sp->cstat = tempshort;

    DoActorGlobZ(actor);

    // getzrangepoint moves water down
    // missiles don't need the water to be down
    MissileWaterAdjust(actor);

    clippos.z = sp->z + ((zchange * numtics) >> 3);

    // NOTE: this does not tell you when you hit a floor sprite
    // this case is currently treated like it hit a sector

    // test for hitting ceiling or floor
    if (clippos.z - zh <= u->hiz + ceildist)
    {
        // normal code
        sp->z = u->hiz + zh + ceildist;
        if (retval.type == kHitNone)
            retval.setSector(dasectnum);
    }
    else if (clippos.z - zh > u->loz - flordist)
    {
        sp->z = u->loz + zh - flordist;
        if (retval.type == kHitNone)
            retval.setSector(dasectnum);
    }
    else
    {
        sp->z = clippos.z;
    }

    if (FAF_ConnectArea(sp->sectnum))
        SetActorZ(actor, &sp->pos);

    if (TEST(sector[sp->sectnum].extra, SECTFX_WARP_SECTOR))
    {
        DSWActor* sp_warp;

        if ((sp_warp = WarpPlane(&sp->x, &sp->y, &sp->z, &dasectnum)))
        {
            MissileWarpUpdatePos(actor, dasectnum);
            MissileWarpType(actor, sp_warp);
        }

        if (sp->sectnum != lastsectnum)
        {
            if ((sp_warp = Warp(&sp->x, &sp->y, &sp->z, &dasectnum)))
            {
                MissileWarpUpdatePos(actor, dasectnum);
                MissileWarpType(actor, sp_warp);
            }
        }
    }

    if (retval.type != kHitNone && TEST(sector[sp->sectnum].ceilingstat, CEILING_STAT_PLAX))
    {
        if (sp->z < sector[sp->sectnum].ceilingz)
        {
            retval.setSky();
        }
    }

    if (retval.type != kHitNone && TEST(sector[sp->sectnum].floorstat, FLOOR_STAT_PLAX))
    {
        if (sp->z > sector[sp->sectnum].floorz)
        {
            retval.setSky();
        }
    }

    return retval;
}


Collision move_ground_missile(DSWActor* actor, int xchange, int ychange, int ceildist, int flordist, uint32_t cliptype, int numtics)
{
    USERp u = actor->u();
    SPRITEp sp = &actor->s();
    int daz;
    Collision retval(0);
    int dasectnum;
    short lastsectnum;
    int ox,oy;

    ASSERT(actor->hasU());

    // Can't modify sprite sectors
    // directly becuase of linked lists
    dasectnum = lastsectnum = sp->sectnum;

    vec3_t opos = sp->pos;
    daz = sp->z;

    // climbing a wall
    if (u->z_tgt)
    {
        if (labs(u->z_tgt - sp->z) > Z(40))
        {
            if (u->z_tgt > sp->z)
            {
                sp->z += Z(30);
                return retval;
            }
            else
            {
                sp->z -= Z(30);
                return retval;
            }
        }
        else
            u->z_tgt = 0;
    }

    sp->x += xchange/2;
    sp->y += ychange/2;

    updatesector(sp->x, sp->y, &dasectnum);

    if (dasectnum < 0)
    {
        // back up and try again
        dasectnum = lastsectnum = sp->sectnum;
        opos = sp->pos;
        opos.z = daz;
        auto cmret = clipmove(&opos, &dasectnum,
                          ((xchange * numtics) << 11), ((ychange * numtics) << 11),
                          (((int) sp->clipdist) << 2), ceildist, flordist, cliptype, 1);
        sp->pos.vec2 = opos.vec2;
        retval.setFromEngine(cmret);
    }

    if (dasectnum < 0)
    {
        // we've gone beyond a white wall - kill it
        retval.setSky();
        return retval;
    }


    if (retval.type != kHitNone)  // ran into a white wall
    {
            return retval;
        }


    u->z_tgt = 0;
    if ((dasectnum != sp->sectnum) && (dasectnum >= 0))
    {
        int new_loz,new_hiz;
        getzsofslope(dasectnum, sp->x, sp->y, &new_hiz, &new_loz);

        sp->z = new_loz;
        ChangeActorSect(actor, dasectnum);
    }

    getzsofslope(sp->sectnum, sp->x, sp->y, &u->hiz, &u->loz);

    u->hi_sectp = u->lo_sectp = &sector[sp->sectnum];
    u->highActor = nullptr; u->lowActor = nullptr;
    sp->z = u->loz - Z(8);

    if (labs(u->hiz - u->loz) < Z(12))
    {
        // we've gone into a very small place - kill it
        retval.setSky();
        return retval;
    }

    if (TEST(sector[sp->sectnum].extra, SECTFX_WARP_SECTOR))
    {
        DSWActor* sp_warp;

        if ((sp_warp = WarpPlane(&sp->x, &sp->y, &sp->z, &dasectnum)))
        {
            MissileWarpUpdatePos(actor, dasectnum);
            MissileWarpType(actor, sp_warp);
        }

        if (sp->sectnum != lastsectnum)
        {
            if ((sp_warp = Warp(&sp->x, &sp->y, &sp->z, &dasectnum)))
            {
                MissileWarpUpdatePos(actor, dasectnum);
                MissileWarpType(actor, sp_warp);
            }
        }
    }

    return retval;
}

#include "saveable.h"

static saveable_code saveable_sprite_code[] =
{
    SAVE_CODE(DoGrating),
    SAVE_CODE(DoKey),
    SAVE_CODE(DoCoin),
    SAVE_CODE(DoGet),
};

static saveable_data saveable_sprite_data[] =
{
    SAVE_DATA(Track),
    SAVE_DATA(SectorObject),

    SAVE_DATA(s_DebrisNinja),
    SAVE_DATA(s_DebrisRat),
    SAVE_DATA(s_DebrisCrab),
    SAVE_DATA(s_DebrisStarFish),
    SAVE_DATA(s_RepairKit),
    SAVE_DATA(s_GoldSkelKey),
    SAVE_DATA(s_BlueKey),
    SAVE_DATA(s_BlueCard),
    SAVE_DATA(s_SilverSkelKey),
    SAVE_DATA(s_RedKey),
    SAVE_DATA(s_RedCard),
    SAVE_DATA(s_BronzeSkelKey),
    SAVE_DATA(s_GreenKey),
    SAVE_DATA(s_GreenCard),
    SAVE_DATA(s_RedSkelKey),
    SAVE_DATA(s_YellowKey),
    SAVE_DATA(s_YellowCard),
    SAVE_DATA(s_Key),
    SAVE_DATA(s_BlueKey),
    SAVE_DATA(s_RedKey),
    SAVE_DATA(s_GreenKey),
    SAVE_DATA(s_YellowKey),
    SAVE_DATA(s_Key),
    /*
    SAVE_DATA(s_BlueKeyStatue),
    SAVE_DATA(s_RedKeyStatue),
    SAVE_DATA(s_GreenKeyStatue),
    SAVE_DATA(s_YellowKeyStatue),
    SAVE_DATA(s_KeyStatue),
    */
    SAVE_DATA(s_RedCoin),
    SAVE_DATA(s_YellowCoin),
    SAVE_DATA(s_GreenCoin),
    SAVE_DATA(s_FireFly),
    SAVE_DATA(s_IconStar),
    SAVE_DATA(s_IconUzi),
    SAVE_DATA(s_IconLgUziAmmo),
    SAVE_DATA(s_IconUziFloor),
    SAVE_DATA(s_IconRocket),
    SAVE_DATA(s_IconLgRocket),
    SAVE_DATA(s_IconShotgun),
    SAVE_DATA(s_IconLgShotshell),
    SAVE_DATA(s_IconAutoRiot),
    SAVE_DATA(s_IconGrenadeLauncher),
    SAVE_DATA(s_IconLgGrenade),
    SAVE_DATA(s_IconLgMine),
    SAVE_DATA(s_IconGuardHead),
    SAVE_DATA(s_IconFireballLgAmmo),
    SAVE_DATA(s_IconHeart),
    SAVE_DATA(s_IconHeartLgAmmo),
    SAVE_DATA(s_IconMicroGun),
    SAVE_DATA(s_IconMicroBattery),
    SAVE_DATA(s_IconRailGun),
    SAVE_DATA(s_IconRailAmmo),
    SAVE_DATA(s_IconElectro),
    SAVE_DATA(s_IconSpell),
    SAVE_DATA(s_IconArmor),
    SAVE_DATA(s_IconMedkit),
    SAVE_DATA(s_IconChemBomb),
    SAVE_DATA(s_IconFlashBomb),
    SAVE_DATA(s_IconNuke),
    SAVE_DATA(s_IconCaltrops),
    SAVE_DATA(s_IconSmMedkit),
    SAVE_DATA(s_IconBooster),
    SAVE_DATA(s_IconHeatCard),
    //SAVE_DATA(s_IconEnvironSuit),
    SAVE_DATA(s_IconCloak),
    SAVE_DATA(s_IconFly),
    SAVE_DATA(s_IconNightVision),
    SAVE_DATA(s_IconFlag),
};

saveable_module saveable_sprite =
{
    // code
    saveable_sprite_code,
    SIZ(saveable_sprite_code),

    // data
    saveable_sprite_data,
    SIZ(saveable_sprite_data)
};

END_SW_NS
