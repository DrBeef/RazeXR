//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------
#pragma once
#include "actor.h"
#include "build.h"
#include "common_game.h"
#include "globals.h"
#include "db.h"
#include "dude.h"
#include "levels.h"
#include "qav.h"
#include "gameinput.h"

BEGIN_BLD_NS

// life modes of the player
enum
{
	kModeHuman = 0,
	kModeBeast = 1,
	kModeHumanShrink = 2,
	kModeHumanGrown = 3,
	kModeMax = 4,
};

// postures
enum
{
	kPostureStand = 0,
	kPostureSwim = 1,
	kPostureCrouch = 2,
	kPostureMax = 3,
};

struct PACKINFO
{
	bool isActive; // is active (0/1)
	int curAmount = 0; // remaining percent
};

struct POSTURE
{
	int frontAccel;
	int sideAccel;
	int backAccel;
	int pace[2];
	int bobV;
	int bobH;
	int swayV;
	int swayH;
	int eyeAboveZ;
	int weaponAboveZ;
	int xOffset;
	int zOffset;
	int normalJumpZ;
	int pwupJumpZ;
};

extern POSTURE gPostureDefaults[kModeMax][kPostureMax];

struct PLAYER
{
	DBloodActor* actor;
	DUDEINFO* pDudeInfo;
	InputPacket         input;
	PlayerHorizon       horizon;
	PlayerAngle         angle;
	uint8_t             newWeapon;
	int                 used1;  // something related to game checksum
	int                 weaponQav;
	int                 qavCallback;
	bool                isRunning;
	int                 posture;   // stand, crouch, swim
	int                 sceneQav;  // by NoOne: used to keep qav id
	int                 bobPhase;
	int                 bobAmp;
	int                 bobHeight;
	int                 bobWidth;
	int                 swayPhase;
	int                 swayAmp;
	int                 swayHeight;
	int                 swayWidth;
	int                 nPlayer;  // Connect id
	int                 lifeMode;
	int                 bloodlust;  // ---> useless
	int                 zView;
	int                 zViewVel;
	int                 zWeapon;
	int                 zWeaponVel;
	int                 slope;
	bool                isUnderwater;
	bool                hasKey[8];
	int8_t              hasFlag;
	TObjPtr<DBloodActor*>        ctfFlagState[2];
	int                 damageControl[7];
	int8_t              curWeapon;
	int8_t              nextWeapon;
	int                 weaponTimer;
	int                 weaponState;
	int                 weaponAmmo;  //rename
	bool                hasWeapon[kWeapMax];
	int                 weaponMode[kWeapMax];
	int                 weaponOrder[2][kWeapMax];
	//int               at149[14];
	int                 ammoCount[12];
	bool                qavLoop;
	int                 qavLastTick;
	int                 qavTimer;
	int                 fuseTime;
	int                 throwTime;
	int                 throwPower;
	Aim                 aim;  // world
	//int               at1c6;
	Aim                 relAim;  // relative
	//int               relAim;
	//int               at1ce;
	//int               at1d2;
	TObjPtr<DBloodActor*>        aimTarget;  // aim target sprite
	int                 aimTargetsCount;
	TObjPtr<DBloodActor*>        aimTargets[16];
	int                 deathTime;
	int                 pwUpTime[kMaxPowerUps];
	int                 fragCount;
	int                 fragInfo[8];
	int                 teamId;
	TObjPtr<DBloodActor*>        fragger;
	int                 underwaterTime;
	int                 bubbleTime;
	int                 restTime;
	int                 kickPower;
	int                 laughCount;
	bool                godMode;
	bool                fallScream;
	bool                cantJump;
	int                 packItemTime;  // pack timer
	int                 packItemId;    // pack id 1: diving suit, 2: crystal ball, 3: beast vision 4: jump boots
	PACKINFO            packSlots[5];  // at325 [1]: diving suit, [2]: crystal ball, [3]: beast vision [4]: jump boots
	int                 armor[3];      // armor
	//int               at342;
	//int               at346;
	TObjPtr<DBloodActor*>        voodooTarget;
	int                 voodooTargets;  // --> useless
	int                 voodooVar1;     // --> useless
	int                 vodooVar2;      // --> useless
	int                 flickerEffect;
	int                 tiltEffect;
	int                 visibility;
	int                 painEffect;
	int                 blindEffect;
	int                 chokeEffect;
	int                 handTime;
	bool                hand;  // if true, there is hand start choking the player
	int                 pickupEffect;
	bool                flashEffect;  // if true, reduce pPlayer->visibility counter
	int                 quakeEffect;
	int                 player_par;
	int                 nWaterPal;
	POSTURE             pPosture[kModeMax][kPostureMax];
};

struct AMMOINFO
{
	int max;
	int8_t vectorType;
};

struct POWERUPINFO
{
	int16_t picnum;
	bool pickupOnce;
	int bonusTime;
	int maxTime;
};

void playerResetPosture(PLAYER* pPlayer);

extern PLAYER gPlayer[kMaxPlayers];
extern PLAYER* gMe, * gView;

extern bool gBlueFlagDropped;
extern bool gRedFlagDropped;

extern int team_score[kMaxPlayers];
extern int team_ticker[kMaxPlayers];
extern AMMOINFO gAmmoInfo[];
extern POWERUPINFO gPowerUpInfo[kMaxPowerUps];

bool IsTargetTeammate(PLAYER* pSourcePlayer, DBloodActor* target);
int powerupCheck(PLAYER* pPlayer, int nPowerUp);
bool powerupActivate(PLAYER* pPlayer, int nPowerUp);
void powerupDeactivate(PLAYER* pPlayer, int nPowerUp);
void powerupSetState(PLAYER* pPlayer, int nPowerUp, bool bState);
void powerupProcess(PLAYER* pPlayer);
void powerupClear(PLAYER* pPlayer);
int packItemToPowerup(int nPack);
int powerupToPackItem(int nPowerUp);
bool packAddItem(PLAYER* pPlayer, unsigned int nPack);
int packCheckItem(PLAYER* pPlayer, int nPack);
bool packItemActive(PLAYER* pPlayer, int nPack);
void packUseItem(PLAYER* pPlayer, int nPack);
void packPrevItem(PLAYER* pPlayer);
void packNextItem(PLAYER* pPlayer);
bool        playerSeqPlaying(PLAYER* pPlayer, int nSeq);
void playerSetRace(PLAYER* pPlayer, int nLifeMode);
void playerSetGodMode(PLAYER* pPlayer, bool bGodMode);
void playerResetInertia(PLAYER* pPlayer);
void        playerCorrectInertia(PLAYER* pPlayer, vec3_t const* oldpos);
void        playerStart(int nPlayer, int bNewLevel = 0);
void playerReset(PLAYER* pPlayer);
void playerInit(int nPlayer, unsigned int a2);
void CheckPickUp(PLAYER* pPlayer);
void ProcessInput(PLAYER* pPlayer);
void playerProcess(PLAYER* pPlayer);
DBloodActor* playerFireMissile(PLAYER* pPlayer, int a2, int a3, int a4, int a5, int a6);
DBloodActor* playerFireThing(PLAYER* pPlayer, int a2, int a3, int thingType, int a5);
void playerFrag(PLAYER* pKiller, PLAYER* pVictim);
int playerDamageArmor(PLAYER* pPlayer, DAMAGE_TYPE nType, int nDamage);
int playerDamageSprite(DBloodActor* nSource, PLAYER* pPlayer, DAMAGE_TYPE nDamageType, int nDamage);
int UseAmmo(PLAYER* pPlayer, int nAmmoType, int nDec);
void voodooTarget(PLAYER* pPlayer);
void playerLandingSound(PLAYER* pPlayer);
void PlayerSurvive(int, DBloodActor*);

END_BLD_NS
