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
#include "ns.h"	// Must come before everything else!

#include <stdlib.h>
#include <string.h>
#include "automap.h"
#include "build.h"

#include "blood.h"
#include "gstrings.h"
#include "gamestate.h"
#include "automap.h"

BEGIN_BLD_NS

PLAYER gPlayer[kMaxPlayers];
PLAYER* gMe, * gView;

bool gBlueFlagDropped = false;
bool gRedFlagDropped = false;

// V = has effect in game, X = no effect in game
POWERUPINFO gPowerUpInfo[kMaxPowerUps] = {
	{ -1, 1, 1, 1 },            // 00: V keys
	{ -1, 1, 1, 1 },            // 01: V keys
	{ -1, 1, 1, 1 },            // 02: V keys
	{ -1, 1, 1, 1 },            // 03: V keys
	{ -1, 1, 1, 1 },            // 04: V keys
	{ -1, 1, 1, 1 },            // 05: V keys
	{ -1, 1, 1, 1 },            // 06: V keys
	{ -1, 0, 100, 100 },        // 07: V doctor's bag
	{ -1, 0, 50, 100 },         // 08: V medicine pouch
	{ -1, 0, 20, 100 },         // 09: V life essense
	{ -1, 0, 100, 200 },        // 10: V life seed
	{ -1, 0, 2, 200 },          // 11: V red potion
	{ 783, 0, 3600, 432000 },   // 12: V feather fall
	{ 896, 0, 3600, 432000 },   // 13: V cloak of invisibility
	{ 825, 1, 3600, 432000 },   // 14: V death mask (invulnerability)
	{ 827, 0, 3600, 432000 },   // 15: V jump boots
	{ 828, 0, 3600, 432000 },   // 16: X raven flight
	{ 829, 0, 3600, 1728000 },  // 17: V guns akimbo
	{ 830, 0, 3600, 432000 },   // 18: V diving suit
	{ 831, 0, 3600, 432000 },   // 19: V gas mask
	{ -1, 0, 3600, 432000 },    // 20: X clone
	{ 2566, 0, 3600, 432000 },  // 21: V crystal ball
	{ 836, 0, 3600, 432000 },   // 22: X decoy
	{ 853, 0, 3600, 432000 },   // 23: V doppleganger
	{ 2428, 0, 3600, 432000 },  // 24: V reflective shots
	{ 839, 0, 3600, 432000 },   // 25: V beast vision
	{ 768, 0, 3600, 432000 },   // 26: X cloak of shadow (useless)
	{ 840, 0, 3600, 432000 },   // 27: X rage shroom
	{ 841, 0, 900, 432000 },    // 28: V delirium shroom
	{ 842, 0, 3600, 432000 },   // 29: V grow shroom (gModernMap only)
	{ 843, 0, 3600, 432000 },   // 30: V shrink shroom (gModernMap only)
	{ -1, 0, 3600, 432000 },    // 31: X death mask (useless)
	{ -1, 0, 3600, 432000 },    // 32: X wine goblet
	{ -1, 0, 3600, 432000 },    // 33: X wine bottle
	{ -1, 0, 3600, 432000 },    // 34: X skull grail
	{ -1, 0, 3600, 432000 },    // 35: X silver grail
	{ -1, 0, 3600, 432000 },    // 36: X tome
	{ -1, 0, 3600, 432000 },    // 37: X black chest
	{ -1, 0, 3600, 432000 },    // 38: X wooden chest
	{ 837, 1, 3600, 432000 },   // 39: V asbestos armor
	{ -1, 0, 1, 432000 },       // 40: V basic armor
	{ -1, 0, 1, 432000 },       // 41: V body armor
	{ -1, 0, 1, 432000 },       // 42: V fire armor
	{ -1, 0, 1, 432000 },       // 43: V spirit armor
	{ -1, 0, 1, 432000 },       // 44: V super armor
	{ 0, 0, 0, 0 },             // 45: ? unknown
	{ 0, 0, 0, 0 },             // 46: ? unknown
	{ 0, 0, 0, 0 },             // 47: ? unknown
	{ 0, 0, 0, 0 },             // 48: ? unknown
	{ 0, 0, 0, 0 },             // 49: X dummy
	{ 833, 1, 1, 1 }            // 50: V kModernItemLevelMap (gModernMap only)
};

int Handicap[] = {
	144, 208, 256, 304, 368
};

POSTURE gPostureDefaults[kModeMax][kPostureMax] = {

	// normal human
	{
		{ 0x4000, 0x4000, 0x4000, 14, 17, 24, 16, 32, 80, 0x1600, 0x1200, 0xc00, 0x90, -0xbaaaa, -0x175555 },
		{ 0x1200, 0x1200, 0x1200, 14, 17, 24, 16, 32, 80, 0x1400, 0x1000, -0x600, 0xb0, 0x5b05, 0 },
		{ 0x2000, 0x2000, 0x2000, 22, 28, 24, 16, 16, 40, 0x800, 0x600, -0x600, 0xb0, 0, 0 },
	},

	// normal beast
	{
		{ 0x4000, 0x4000, 0x4000, 14, 17, 24, 16, 32, 80, 0x1600, 0x1200, 0xc00, 0x90, -0xbaaaa, -0x175555 },
		{ 0x1200, 0x1200, 0x1200, 14, 17, 24, 16, 32, 80, 0x1400, 0x1000, -0x600, 0xb0, 0x5b05, 0 },
		{ 0x2000, 0x2000, 0x2000, 22, 28, 24, 16, 16, 40, 0x800, 0x600, -0x600, 0xb0, 0, 0 },
	},

	// shrink human
	{
		{ 10384, 10384, 10384, 14, 17, 24, 16, 32, 80, 5632, 4608, 3072, 144, -564586, -1329173 },
		{ 2108, 2108, 2108, 14, 17, 24, 16, 32, 80, 5120, 4096, -1536, 176, 0x5b05, 0 },
		{ 2192, 2192, 2192, 22, 28, 24, 16, 16, 40, 2048, 1536, -1536, 176, 0, 0 },
	},

	// grown human
	{
		{ 19384, 19384, 19384, 14, 17, 24, 16, 32, 80, 5632, 4608, 3072, 144, -1014586, -1779173 },
		{ 5608, 5608, 5608, 14, 17, 24, 16, 32, 80, 5120, 4096, -1536, 176, 0x5b05, 0 },
		{ 11192, 11192, 11192, 22, 28, 24, 16, 16, 40, 2048, 1536, -1536, 176, 0, 0 },
	},
};

AMMOINFO gAmmoInfo[] = {
	{ 0, -1 },
	{ 100, -1 },
	{ 100, 4 },
	{ 500, 5 },
	{ 100, -1 },
	{ 50, -1 },
	{ 2880, -1 },
	{ 250, -1 },
	{ 100, -1 },
	{ 100, -1 },
	{ 50, -1 },
	{ 50, -1 },
};

struct ARMORDATA {
	int armor0;
	int armor0max;
	int armor1;
	int armor1max;
	int armor2;
	int armor2max;
};
ARMORDATA armorData[5] = {
	{ 0x320, 0x640, 0x320, 0x640, 0x320, 0x640 },
	{ 0x640, 0x640, 0, 0x640, 0, 0x640 },
	{ 0, 0x640, 0x640, 0x640, 0, 0x640 },
	{ 0, 0x640, 0, 0x640, 0x640, 0x640 },
	{ 0xc80, 0xc80, 0xc80, 0xc80, 0xc80, 0xc80 }
};



struct VICTORY {
	const char* message;
	int Kills;
};

VICTORY gVictory[] = {
	{ "%s boned %s like a fish", 4100 },
	{ "%s castrated %s", 4101 },
	{ "%s creamed %s", 4102 },
	{ "%s destroyed %s", 4103 },
	{ "%s diced %s", 4104 },
	{ "%s disemboweled %s", 4105 },
	{ "%s flattened %s", 4106 },
	{ "%s gave %s Anal Justice", 4107 },
	{ "%s gave AnAl MaDnEsS to %s", 4108 },
	{ "%s hurt %s real bad", 4109 },
	{ "%s killed %s", 4110 },
	{ "%s made mincemeat out of %s", 4111 },
	{ "%s massacred %s", 4112 },
	{ "%s mutilated %s", 4113 },
	{ "%s reamed %s", 4114 },
	{ "%s ripped %s a new orifice", 4115 },
	{ "%s slaughtered %s", 4116 },
	{ "%s sliced %s", 4117 },
	{ "%s smashed %s", 4118 },
	{ "%s sodomized %s", 4119 },
	{ "%s splattered %s", 4120 },
	{ "%s squashed %s", 4121 },
	{ "%s throttled %s", 4122 },
	{ "%s wasted %s", 4123 },
	{ "%s body bagged %s", 4124 },
};

struct SUICIDE {
	const char* message;
	int Kills;
};

SUICIDE gSuicide[] = {
	{ "%s is excrement", 4202 },
	{ "%s is hamburger", 4203 },
	{ "%s suffered scrotum separation", 4204 },
	{ "%s volunteered for population control", 4206 },
	{ "%s has suicided", 4207 },
};

struct DAMAGEINFO {
	int armorType;
	int Kills[3];
	int at10[3];
};

DAMAGEINFO damageInfo[7] = {
	{ -1, 731, 732, 733, 710, 710, 710 },
	{ 1, 742, 743, 744, 711, 711, 711 },
	{ 0, 731, 732, 733, 712, 712, 712 },
	{ 1, 731, 732, 733, 713, 713, 713 },
	{ -1, 724, 724, 724, 714, 714, 714 },
	{ 2, 731, 732, 733, 715, 715, 715 },
	{ 0, 0, 0, 0, 0, 0, 0 }
};

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

inline bool IsTargetTeammate(PLAYER* pSourcePlayer, DBloodActor* target)
{
	if (pSourcePlayer == nullptr)
		return false;
	if (!target->IsPlayerActor())
		return false;
	if (gGameOptions.nGameType == 1 || gGameOptions.nGameType == 3)
	{
		PLAYER* pTargetPlayer = &gPlayer[target->spr.type - kDudePlayer1];
		if (pSourcePlayer != pTargetPlayer)
		{
			if (gGameOptions.nGameType == 1)
				return true;
			if (gGameOptions.nGameType == 3 && (pSourcePlayer->teamId & 3) == (pTargetPlayer->teamId & 3))
				return true;
		}
	}

	return false;
}


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int powerupCheck(PLAYER* pPlayer, int nPowerUp)
{
	assert(pPlayer != NULL);
	assert(nPowerUp >= 0 && nPowerUp < kMaxPowerUps);
	int nPack = powerupToPackItem(nPowerUp);
	if (nPack >= 0 && !packItemActive(pPlayer, nPack))
		return 0;
	return pPlayer->pwUpTime[nPowerUp];
}


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

bool powerupActivate(PLAYER* pPlayer, int nPowerUp)
{
	if (powerupCheck(pPlayer, nPowerUp) > 0 && gPowerUpInfo[nPowerUp].pickupOnce)
		return 0;
	if (!pPlayer->pwUpTime[nPowerUp])
		pPlayer->pwUpTime[nPowerUp] = gPowerUpInfo[nPowerUp].bonusTime;
	int nPack = powerupToPackItem(nPowerUp);
	if (nPack >= 0)
		pPlayer->packSlots[nPack].isActive = 1;

	switch (nPowerUp + kItemBase) {
#ifdef NOONE_EXTENSIONS
	case kItemModernMapLevel:
		if (gModernMap) gFullMap = true;
		break;
	case kItemShroomShrink:
		if (!gModernMap) break;
		else if (isGrown(pPlayer->actor)) playerDeactivateShrooms(pPlayer);
		else playerSizeShrink(pPlayer, 2);
		break;
	case kItemShroomGrow:
		if (!gModernMap) break;
		else if (isShrinked(pPlayer->actor)) playerDeactivateShrooms(pPlayer);
		else {
			playerSizeGrow(pPlayer, 2);
			if (powerupCheck(&gPlayer[pPlayer->actor->spr.type - kDudePlayer1], kPwUpShadowCloak) > 0) {
				powerupDeactivate(pPlayer, kPwUpShadowCloak);
				pPlayer->pwUpTime[kPwUpShadowCloak] = 0;
			}

			if (ceilIsTooLow(pPlayer->actor))
				actDamageSprite(pPlayer->actor, pPlayer->actor, kDamageExplode, 65535);
		}
		break;
#endif
	case kItemFeatherFall:
	case kItemJumpBoots:
		pPlayer->damageControl[0]++;
		break;
	case kItemReflectShots: // reflective shots
		if (pPlayer == gMe && gGameOptions.nGameType == 0)
			sfxSetReverb2(1);
		break;
	case kItemDeathMask:
		for (int i = 0; i < 7; i++)
			pPlayer->damageControl[i]++;
		break;
	case kItemDivingSuit: // diving suit
		pPlayer->damageControl[4]++;
		if (pPlayer == gMe && gGameOptions.nGameType == 0)
			sfxSetReverb(1);
		break;
	case kItemGasMask:
		pPlayer->damageControl[4]++;
		break;
	case kItemArmorAsbest:
		pPlayer->damageControl[1]++;
		break;
	case kItemTwoGuns:
		pPlayer->newWeapon = pPlayer->curWeapon;
		WeaponRaise(pPlayer);
		break;
	}
	sfxPlay3DSound(pPlayer->actor, 776, -1, 0);
	return 1;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void powerupDeactivate(PLAYER* pPlayer, int nPowerUp)
{
	int nPack = powerupToPackItem(nPowerUp);
	if (nPack >= 0)
		pPlayer->packSlots[nPack].isActive = 0;

	switch (nPowerUp + kItemBase) {
#ifdef NOONE_EXTENSIONS
	case kItemShroomShrink:
		if (gModernMap) {
			playerSizeReset(pPlayer);
			if (ceilIsTooLow(pPlayer->actor))
				actDamageSprite(pPlayer->actor, pPlayer->actor, kDamageExplode, 65535);
		}
		break;
	case kItemShroomGrow:
		if (gModernMap) playerSizeReset(pPlayer);
		break;
#endif
	case kItemFeatherFall:
	case kItemJumpBoots:
		pPlayer->damageControl[0]--;
		break;
	case kItemDeathMask:
		for (int i = 0; i < 7; i++)
			pPlayer->damageControl[i]--;
		break;
	case kItemDivingSuit:
		pPlayer->damageControl[4]--;
		if (pPlayer == gMe && VanillaMode() ? true : pPlayer->pwUpTime[24] == 0)
			sfxSetReverb(0);
		break;
	case kItemReflectShots:
		if (pPlayer == gMe && VanillaMode() ? true : pPlayer->packSlots[1].isActive == 0)
			sfxSetReverb(0);
		break;
	case kItemGasMask:
		pPlayer->damageControl[4]--;
		break;
	case kItemArmorAsbest:
		pPlayer->damageControl[1]--;
		break;
	case kItemTwoGuns:
		pPlayer->newWeapon = pPlayer->curWeapon;
		WeaponRaise(pPlayer);
		break;
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void powerupSetState(PLAYER* pPlayer, int nPowerUp, bool bState)
{
	if (!bState)
		powerupActivate(pPlayer, nPowerUp);
	else
		powerupDeactivate(pPlayer, nPowerUp);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void powerupProcess(PLAYER* pPlayer)
{
	pPlayer->packItemTime = ClipLow(pPlayer->packItemTime - 4, 0);
	for (int i = kMaxPowerUps - 1; i >= 0; i--)
	{
		int nPack = powerupToPackItem(i);
		if (nPack >= 0)
		{
			if (pPlayer->packSlots[nPack].isActive)
			{
				pPlayer->pwUpTime[i] = ClipLow(pPlayer->pwUpTime[i] - 4, 0);
				if (pPlayer->pwUpTime[i])
					pPlayer->packSlots[nPack].curAmount = (100 * pPlayer->pwUpTime[i]) / gPowerUpInfo[i].bonusTime;
				else
				{
					powerupDeactivate(pPlayer, i);
					if (pPlayer->packItemId == nPack)
						pPlayer->packItemId = 0;
				}
			}
		}
		else if (pPlayer->pwUpTime[i] > 0)
		{
			pPlayer->pwUpTime[i] = ClipLow(pPlayer->pwUpTime[i] - 4, 0);
			if (!pPlayer->pwUpTime[i])
				powerupDeactivate(pPlayer, i);
		}
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void powerupClear(PLAYER* pPlayer)
{
	for (int i = kMaxPowerUps - 1; i >= 0; i--)
	{
		pPlayer->pwUpTime[i] = 0;
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int packItemToPowerup(int nPack)
{
	int nPowerUp = -1;
	switch (nPack) {
	case 0:
		break;
	case 1:
		nPowerUp = kPwUpDivingSuit;
		break;
	case 2:
		nPowerUp = kPwUpCrystalBall;
		break;
	case 3:
		nPowerUp = kPwUpBeastVision;
		break;
	case 4:
		nPowerUp = kPwUpJumpBoots;
		break;
	default:
		I_Error("Unhandled pack item %d", nPack);
		break;
	}
	return nPowerUp;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int powerupToPackItem(int nPowerUp)
{
	switch (nPowerUp) {
	case kPwUpDivingSuit:
		return 1;
	case kPwUpCrystalBall:
		return 2;
	case kPwUpBeastVision:
		return 3;
	case kPwUpJumpBoots:
		return 4;
	}
	return -1;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

bool packAddItem(PLAYER* pPlayer, unsigned int nPack)
{
	if (nPack <= 4)
	{
		if (pPlayer->packSlots[nPack].curAmount >= 100)
			return 0;
		pPlayer->packSlots[nPack].curAmount = 100;
		int nPowerUp = packItemToPowerup(nPack);
		if (nPowerUp >= 0)
			pPlayer->pwUpTime[nPowerUp] = gPowerUpInfo[nPowerUp].bonusTime;
		if (pPlayer->packItemId == -1)
			pPlayer->packItemId = nPack;
		if (!pPlayer->packSlots[pPlayer->packItemId].curAmount)
			pPlayer->packItemId = nPack;
	}
	else
		I_Error("Unhandled pack item %d", nPack);
	return 1;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int packCheckItem(PLAYER* pPlayer, int nPack)
{
	return pPlayer->packSlots[nPack].curAmount;
}

bool packItemActive(PLAYER* pPlayer, int nPack)
{
	return pPlayer->packSlots[nPack].isActive;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void packUseItem(PLAYER* pPlayer, int nPack)
{
	bool v4 = 0;
	int nPowerUp = -1;
	if (pPlayer->packSlots[nPack].curAmount > 0)
	{
		pPlayer->packItemId = nPack;

		switch (nPack)
		{
		case 0:
		{
			unsigned int health = pPlayer->actor->xspr.health >> 4;
			if (health < 100)
			{
				int heal = ClipHigh(100 - health, pPlayer->packSlots[0].curAmount);
				actHealDude(pPlayer->actor, heal, 100);
				pPlayer->packSlots[0].curAmount -= heal;
			}
			break;
		}
		case 1:
			v4 = 1;
			nPowerUp = kPwUpDivingSuit;
			break;
		case 2:
			v4 = 1;
			nPowerUp = kPwUpCrystalBall;
			break;
		case 3:
			v4 = 1;
			nPowerUp = kPwUpBeastVision;
			break;
		case 4:
			v4 = 1;
			nPowerUp = kPwUpJumpBoots;
			break;
		default:
			I_Error("Unhandled pack item %d", nPack);
			return;
		}
	}
	pPlayer->packItemTime = 0;
	if (v4)
		powerupSetState(pPlayer, nPowerUp, pPlayer->packSlots[nPack].isActive);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void packPrevItem(PLAYER* pPlayer)
{
	if (pPlayer->packItemTime > 0)
	{
		for (int i = 0; i < 2; i++)
		{
			for (int nPrev = pPlayer->packItemId - 1; nPrev >= 0; nPrev--)
			{
				if (pPlayer->packSlots[nPrev].curAmount)
				{
					pPlayer->packItemId = nPrev;
					pPlayer->packItemTime = 600;
					return;
				}
			}
			pPlayer->packItemId = 4;
			if (pPlayer->packSlots[4].curAmount) break;
		}
	}

	pPlayer->packItemTime = 600;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void packNextItem(PLAYER* pPlayer)
{
	if (pPlayer->packItemTime > 0)
	{
		for (int i = 0; i < 2; i++)
		{
			for (int nNext = pPlayer->packItemId + 1; nNext < 5; nNext++)
			{
				if (pPlayer->packSlots[nNext].curAmount)
				{
					pPlayer->packItemId = nNext;
					pPlayer->packItemTime = 600;
					return;
				}
			}
			pPlayer->packItemId = 0;
			if (pPlayer->packSlots[0].curAmount) break;
		}
	}
	pPlayer->packItemTime = 600;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

bool playerSeqPlaying(PLAYER* pPlayer, int nSeq)
{
	int nCurSeq = seqGetID(pPlayer->actor);
	if (pPlayer->pDudeInfo->seqStartID + nSeq == nCurSeq && seqGetStatus(pPlayer->actor) >= 0)
		return 1;
	return 0;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void playerSetRace(PLAYER* pPlayer, int nLifeMode)
{
	assert(nLifeMode >= kModeHuman && nLifeMode <= kModeHumanGrown);
	DUDEINFO* pDudeInfo = pPlayer->pDudeInfo;
	*pDudeInfo = gPlayerTemplate[nLifeMode];
	pPlayer->lifeMode = nLifeMode;

	// By NoOne: don't forget to change clipdist for grow and shrink modes
	pPlayer->actor->spr.clipdist = pDudeInfo->clipdist;

	for (int i = 0; i < 7; i++)
		pDudeInfo->damageVal[i] = MulScale(Handicap[gSkill], pDudeInfo->startDamage[i], 8);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void playerSetGodMode(PLAYER* pPlayer, bool bGodMode)
{
	pPlayer->godMode = bGodMode;
}

void playerResetInertia(PLAYER* pPlayer)
{
	POSTURE* pPosture = &pPlayer->pPosture[pPlayer->lifeMode][pPlayer->posture];
	pPlayer->zView = pPlayer->actor->spr.pos.Z - pPosture->eyeAboveZ;
	pPlayer->zWeapon = pPlayer->actor->spr.pos.Z - pPosture->weaponAboveZ;
	viewBackupView(pPlayer->nPlayer);
}

void playerCorrectInertia(PLAYER* pPlayer, vec3_t const* oldpos)
{
	pPlayer->zView += pPlayer->actor->spr.pos.Z - oldpos->Z;
	pPlayer->zWeapon += pPlayer->actor->spr.pos.Z - oldpos->Z;
	viewCorrectViewOffsets(pPlayer->nPlayer, oldpos);
}

void playerResetPowerUps(PLAYER* pPlayer)
{
	for (int i = 0; i < kMaxPowerUps; i++) {
		if (!VanillaMode() && (i == kPwUpJumpBoots || i == kPwUpDivingSuit || i == kPwUpCrystalBall || i == kPwUpBeastVision))
			continue;
		pPlayer->pwUpTime[i] = 0;
	}
}

void playerResetPosture(PLAYER* pPlayer) {
	memcpy(pPlayer->pPosture, gPostureDefaults, sizeof(gPostureDefaults));
	if (!VanillaMode()) {
		pPlayer->bobPhase = 0;
		pPlayer->bobAmp = 0;
		pPlayer->swayAmp = 0;
		pPlayer->bobHeight = 0;
		pPlayer->bobWidth = 0;
		pPlayer->swayHeight = 0;
		pPlayer->swayWidth = 0;
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void playerStart(int nPlayer, int bNewLevel)
{
	PLAYER* pPlayer = &gPlayer[nPlayer];
	InputPacket* pInput = &pPlayer->input;
	ZONE* pStartZone = NULL;

	// normal start position
	if (gGameOptions.nGameType <= 1)
		pStartZone = &gStartZone[nPlayer];

#ifdef NOONE_EXTENSIONS
	// let's check if there is positions of teams is specified
	// if no, pick position randomly, just like it works in vanilla.
	else if (gModernMap && gGameOptions.nGameType == 3 && gTeamsSpawnUsed == true) {
		int maxRetries = 5;
		while (maxRetries-- > 0) {
			if (pPlayer->teamId == 0) pStartZone = &gStartZoneTeam1[Random(3)];
			else pStartZone = &gStartZoneTeam2[Random(3)];

			if (maxRetries != 0) {
				// check if there is no spawned player in selected zone
				BloodSectIterator it(pStartZone->sector);
				while (auto act = it.Next())
				{
					if (pStartZone->x == act->spr.pos.X && pStartZone->y == act->spr.pos.Y && act->IsPlayerActor()) {
						pStartZone = NULL;
						break;
					}
				}
			}

			if (pStartZone != NULL)
				break;
		}

	}
#endif
	else {
		pStartZone = &gStartZone[Random(8)];
	}

	auto actor = actSpawnSprite(pStartZone->sector, pStartZone->x, pStartZone->y, pStartZone->z, 6, 1);
	assert(actor->hasX());
	pPlayer->actor = actor;
	DUDEINFO* pDudeInfo = &dudeInfo[kDudePlayer1 + nPlayer - kDudeBase];
	pPlayer->pDudeInfo = pDudeInfo;
	playerSetRace(pPlayer, kModeHuman);
	playerResetPosture(pPlayer);
	seqSpawn(pDudeInfo->seqStartID, actor, -1);
	if (pPlayer == gMe)
		actor->spr.cstat2 |= CSTAT2_SPRITE_MAPPED;
	int top, bottom;
	GetActorExtents(actor, &top, &bottom);
	actor->spr.pos.Z -= bottom - actor->spr.pos.Z;
	actor->spr.pal = 11 + (pPlayer->teamId & 3);
	actor->spr.ang = pStartZone->ang;
	pPlayer->angle.ang = buildang(actor->spr.ang);
	actor->spr.type = kDudePlayer1 + nPlayer;
	actor->spr.clipdist = pDudeInfo->clipdist;
	actor->spr.flags = 15;
	actor->xspr.burnTime = 0;
	actor->SetBurnSource(nullptr);
	pPlayer->actor->xspr.health = pDudeInfo->startHealth << 4;
	pPlayer->actor->spr.cstat &= ~CSTAT_SPRITE_INVISIBLE;
	pPlayer->bloodlust = 0;
	pPlayer->horizon.horiz = pPlayer->horizon.horizoff = q16horiz(0);
	pPlayer->slope = 0;
	pPlayer->fragger = nullptr;
	pPlayer->underwaterTime = 1200;
	pPlayer->bubbleTime = 0;
	pPlayer->restTime = 0;
	pPlayer->kickPower = 0;
	pPlayer->laughCount = 0;
	pPlayer->angle.spin = 0;
	pPlayer->posture = 0;
	pPlayer->voodooTarget = nullptr;
	pPlayer->voodooTargets = 0;
	pPlayer->voodooVar1 = 0;
	pPlayer->vodooVar2 = 0;
	playerResetInertia(pPlayer);
	pPlayer->zWeaponVel = 0;
	pPlayer->relAim.dx = 0x4000;
	pPlayer->relAim.dy = 0;
	pPlayer->relAim.dz = 0;
	pPlayer->aimTarget = nullptr;
	pPlayer->zViewVel = pPlayer->zWeaponVel;
	if (!(gGameOptions.nGameType == 1 && gGameOptions.bKeepKeysOnRespawn && !bNewLevel))
		for (int i = 0; i < 8; i++)
			pPlayer->hasKey[i] = gGameOptions.nGameType >= 2;
	pPlayer->hasFlag = 0;
	for (int i = 0; i < 2; i++)
		pPlayer->ctfFlagState[i] = nullptr;
	for (int i = 0; i < 7; i++)
		pPlayer->damageControl[i] = 0;
	if (pPlayer->godMode)
		playerSetGodMode(pPlayer, 1);
	gInfiniteAmmo = 0;
	gFullMap = 0;
	pPlayer->throwPower = 0;
	pPlayer->deathTime = 0;
	pPlayer->nextWeapon = kWeapNone;
	actor->vel.X = actor->vel.Y = actor->vel.Z = 0;
	pInput->avel = 0;
	pInput->actions = 0;
	pInput->fvel = 0;
	pInput->svel = 0;
	pInput->horz = 0;
	pPlayer->flickerEffect = 0;
	pPlayer->quakeEffect = 0;
	pPlayer->tiltEffect = 0;
	pPlayer->visibility = 0;
	pPlayer->painEffect = 0;
	pPlayer->blindEffect = 0;
	pPlayer->chokeEffect = 0;
	pPlayer->handTime = 0;
	pPlayer->weaponTimer = 0;
	pPlayer->weaponState = 0;
	pPlayer->weaponQav = kQAVNone;
	pPlayer->qavLastTick = 0;
	pPlayer->qavTimer = 0;
#ifdef NOONE_EXTENSIONS
	playerQavSceneReset(pPlayer); // reset qav scene

	// assign or update player's sprite index for conditions
	if (gModernMap) {

		BloodStatIterator it(kStatModernPlayerLinker);
		while (auto iactor = it.Next())
		{
			if (iactor->xspr.data1 == pPlayer->nPlayer + 1)
			{
				DBloodActor* SpriteOld = iactor->prevmarker;
				trPlayerCtrlLink(iactor, pPlayer, (SpriteOld == nullptr)); // this modifies iactor's prevmarker field!
				if (SpriteOld)
					condUpdateObjectIndex(SpriteOld, iactor->prevmarker);
			}
		}

	}

#endif
	pPlayer->hand = 0;
	pPlayer->nWaterPal = 0;
	playerResetPowerUps(pPlayer);

	if (pPlayer == gMe)
	{
		viewInitializePrediction();
	}
	if (IsUnderwaterSector(actor->sector()))
	{
		pPlayer->posture = 1;
		pPlayer->actor->xspr.medium = kMediumWater;
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void playerReset(PLAYER* pPlayer)
{
	static int dword_136400[] = {
		3, 4, 2, 8, 9, 10, 7, 1, 1, 1, 1, 1, 1, 1
	};
	static int dword_136438[] = {
		3, 4, 2, 8, 9, 10, 7, 1, 1, 1, 1, 1, 1, 1
	};
	assert(pPlayer != NULL);
	for (int i = 0; i < 14; i++)
	{
		pPlayer->hasWeapon[i] = gInfiniteAmmo;
		pPlayer->weaponMode[i] = 0;
	}
	pPlayer->hasWeapon[kWeapPitchFork] = 1;
	pPlayer->curWeapon = kWeapNone;
	pPlayer->qavCallback = -1;
	pPlayer->newWeapon = kWeapPitchFork;
	for (int i = 0; i < 14; i++)
	{
		pPlayer->weaponOrder[0][i] = dword_136400[i];
		pPlayer->weaponOrder[1][i] = dword_136438[i];
	}
	for (int i = 0; i < 12; i++)
	{
		if (gInfiniteAmmo)
			pPlayer->ammoCount[i] = gAmmoInfo[i].max;
		else
			pPlayer->ammoCount[i] = 0;
	}
	for (int i = 0; i < 3; i++)
		pPlayer->armor[i] = 0;
	pPlayer->weaponTimer = 0;
	pPlayer->weaponState = 0;
	pPlayer->weaponQav = kQAVNone;
	pPlayer->qavLoop = 0;
	pPlayer->qavLastTick = 0;
	pPlayer->qavTimer = 0;
	pPlayer->packItemId = -1;

	for (int i = 0; i < 5; i++) {
		pPlayer->packSlots[i].isActive = 0;
		pPlayer->packSlots[i].curAmount = 0;
	}
#ifdef NOONE_EXTENSIONS
	playerQavSceneReset(pPlayer);
#endif
	// reset posture (mainly required for resetting movement speed and jump height)
	playerResetPosture(pPlayer);

}

int team_score[8];
int team_ticker[8];

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void playerInit(int nPlayer, unsigned int a2)
{
	if (!(a2 & 1))
		gPlayer[nPlayer] = {};

	PLAYER* pPlayer = &gPlayer[nPlayer];
	pPlayer->nPlayer = nPlayer;
	pPlayer->teamId = nPlayer;
	if (gGameOptions.nGameType == 3)
		pPlayer->teamId = nPlayer & 1;
	pPlayer->fragCount = 0;
	memset(team_score, 0, sizeof(team_score));
	memset(team_ticker, 0, sizeof(team_ticker));
	memset(pPlayer->fragInfo, 0, sizeof(pPlayer->fragInfo));

	if (!(a2 & 1))
		playerReset(pPlayer);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

bool findDroppedLeech(PLAYER* a1, DBloodActor* a2)
{
	BloodStatIterator it(kStatThing);
	while (auto actor = it.Next())
	{
		if (a2 == actor)
			continue;
		if (actor->spr.type == kThingDroppedLifeLeech && actor->GetOwner() == a1->actor)
			return 1;
	}
	return 0;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

bool PickupItem(PLAYER* pPlayer, DBloodActor* itemactor)
{
	char buffer[80];
	int pickupSnd = 775;
	int nType = itemactor->spr.type - kItemBase;
	auto plActor = pPlayer->actor;

	switch (itemactor->spr.type) {
	case kItemShadowCloak:
#ifdef NOONE_EXTENSIONS
		if (isGrown(pPlayer->actor) || !powerupActivate(pPlayer, nType)) return false;
#else
		if (!powerupActivate(pPlayer, nType)) return false;
#endif
		break;
#ifdef NOONE_EXTENSIONS
	case kItemShroomShrink:
	case kItemShroomGrow:

		if (gModernMap) {
			switch (itemactor->spr.type) {
			case kItemShroomShrink:
				if (isShrinked(pPlayer->actor)) return false;
				break;
			case kItemShroomGrow:
				if (isGrown(pPlayer->actor)) return false;
				break;
			}

			powerupActivate(pPlayer, nType);
		}

		break;
#endif
	case kItemFlagABase:
	case kItemFlagBBase: {
		if (gGameOptions.nGameType != 3 || !itemactor->hasX()) return 0;
		if (itemactor->spr.type == kItemFlagABase) {
			if (pPlayer->teamId == 1) {
				if ((pPlayer->hasFlag & 1) == 0 && itemactor->xspr.state) {
					pPlayer->hasFlag |= 1;
					pPlayer->ctfFlagState[0] = itemactor;
					trTriggerSprite(itemactor, kCmdOff);
					sprintf(buffer, "%s stole Blue Flag", PlayerName(pPlayer->nPlayer));
					sndStartSample(8007, 255, 2, 0);
					viewSetMessage(buffer);
				}
			}

			if (pPlayer->teamId == 0) {

				if ((pPlayer->hasFlag & 1) != 0 && !itemactor->xspr.state) {
					pPlayer->hasFlag &= ~1;
					pPlayer->ctfFlagState[0] = nullptr;
					trTriggerSprite(itemactor, kCmdOn);
					sprintf(buffer, "%s returned Blue Flag", PlayerName(pPlayer->nPlayer));
					sndStartSample(8003, 255, 2, 0);
					viewSetMessage(buffer);
				}

				if ((pPlayer->hasFlag & 2) != 0 && itemactor->xspr.state) {
					pPlayer->hasFlag &= ~2;
					pPlayer->ctfFlagState[1] = nullptr;
					team_score[pPlayer->teamId] += 10;
					team_ticker[pPlayer->teamId] += 240;
					evSendGame(81, kCmdOn);
					sprintf(buffer, "%s captured Red Flag!", PlayerName(pPlayer->nPlayer));
					sndStartSample(8001, 255, 2, 0);
					viewSetMessage(buffer);
				}
			}

		}
		else if (itemactor->spr.type == kItemFlagBBase) {

			if (pPlayer->teamId == 0) {
				if ((pPlayer->hasFlag & 2) == 0 && itemactor->xspr.state) {
					pPlayer->hasFlag |= 2;
					pPlayer->ctfFlagState[1] = itemactor;
					trTriggerSprite(itemactor, kCmdOff);
					sprintf(buffer, "%s stole Red Flag", PlayerName(pPlayer->nPlayer));
					sndStartSample(8006, 255, 2, 0);
					viewSetMessage(buffer);
				}
			}

			if (pPlayer->teamId == 1) {
				if ((pPlayer->hasFlag & 2) != 0 && !itemactor->xspr.state)
				{
					pPlayer->hasFlag &= ~2;
					pPlayer->ctfFlagState[1] = nullptr;
					trTriggerSprite(itemactor, kCmdOn);
					sprintf(buffer, "%s returned Red Flag", PlayerName(pPlayer->nPlayer));
					sndStartSample(8002, 255, 2, 0);
					viewSetMessage(buffer);
				}
				if ((pPlayer->hasFlag & 1) != 0 && itemactor->xspr.state)
				{
					pPlayer->hasFlag &= ~1;
					pPlayer->ctfFlagState[0] = nullptr;
					team_score[pPlayer->teamId] += 10;
					team_ticker[pPlayer->teamId] += 240;
					evSendGame(80, kCmdOn);
					sprintf(buffer, "%s captured Blue Flag!", PlayerName(pPlayer->nPlayer));
					sndStartSample(8000, 255, 2, 0);
					viewSetMessage(buffer);
				}
			}
		}
	}
					   return 0;
	case kItemFlagA: {
		if (gGameOptions.nGameType != 3) return 0;
		gBlueFlagDropped = false;
		const bool enemyTeam = (pPlayer->teamId & 1) == 1;
		if (!enemyTeam && itemactor->GetOwner())
		{
			pPlayer->hasFlag &= ~1;
			pPlayer->ctfFlagState[0] = nullptr;
			trTriggerSprite(itemactor->GetOwner(), kCmdOn);
			sprintf(buffer, "%s returned Blue Flag", PlayerName(pPlayer->nPlayer));
			sndStartSample(8003, 255, 2, 0);
			viewSetMessage(buffer);
			break;
		}
		pPlayer->hasFlag |= 1;
		pPlayer->ctfFlagState[0] = itemactor->GetOwner();
		if (enemyTeam)
		{
			sprintf(buffer, "%s stole Blue Flag", PlayerName(pPlayer->nPlayer));
			sndStartSample(8007, 255, 2, 0);
			viewSetMessage(buffer);
		}
		break;
	}
	case kItemFlagB: {
		if (gGameOptions.nGameType != 3) return 0;
		gRedFlagDropped = false;
		const bool enemyTeam = (pPlayer->teamId & 1) == 0;
		if (!enemyTeam && itemactor->GetOwner())
		{
			pPlayer->hasFlag &= ~2;
			pPlayer->ctfFlagState[1] = nullptr;
			trTriggerSprite(itemactor->GetOwner(), kCmdOn);
			sprintf(buffer, "%s returned Red Flag", PlayerName(pPlayer->nPlayer));
			sndStartSample(8002, 255, 2, 0);
			viewSetMessage(buffer);
			break;
		}
		pPlayer->hasFlag |= 2;
		pPlayer->ctfFlagState[1] = itemactor->GetOwner();
		if (enemyTeam)
		{
			sprintf(buffer, "%s stole Red Flag", PlayerName(pPlayer->nPlayer));
			sndStartSample(8006, 255, 2, 0);
			viewSetMessage(buffer);
		}
		break;
	}
	case kItemArmorBasic:
	case kItemArmorBody:
	case kItemArmorFire:
	case kItemArmorSpirit:
	case kItemArmorSuper: {
		ARMORDATA* pArmorData = &armorData[itemactor->spr.type - kItemArmorBasic]; bool pickedUp = false;
		if (pPlayer->armor[1] < pArmorData->armor1max) {
			pPlayer->armor[1] = ClipHigh(pPlayer->armor[1] + pArmorData->armor1, pArmorData->armor1max);
			pickedUp = true;
		}

		if (pPlayer->armor[0] < pArmorData->armor0max) {
			pPlayer->armor[0] = ClipHigh(pPlayer->armor[0] + pArmorData->armor0, pArmorData->armor0max);
			pickedUp = true;
		}

		if (pPlayer->armor[2] < pArmorData->armor2max) {
			pPlayer->armor[2] = ClipHigh(pPlayer->armor[2] + pArmorData->armor2, pArmorData->armor2max);
			pickedUp = true;
		}

		if (!pickedUp) return 0;
		pickupSnd = 779;
		break;
	}
	case kItemCrystalBall:
		if (gGameOptions.nGameType == 0 || !packAddItem(pPlayer, gItemData[nType].packSlot)) return 0;
		break;
	case kItemKeySkull:
	case kItemKeyEye:
	case kItemKeyFire:
	case kItemKeyDagger:
	case kItemKeySpider:
	case kItemKeyMoon:
	case kItemKeyKey7:
		if (pPlayer->hasKey[itemactor->spr.type - 99]) return 0;
		pPlayer->hasKey[itemactor->spr.type - 99] = 1;
		pickupSnd = 781;
		break;
	case kItemHealthMedPouch:
	case kItemHealthLifeEssense:
	case kItemHealthLifeSeed:
	case kItemHealthRedPotion: {
		int addPower = gPowerUpInfo[nType].bonusTime;
#ifdef NOONE_EXTENSIONS
		// allow custom amount for item
		if (gModernMap && itemactor->hasX() && itemactor->xspr.data1 > 0)
			addPower = itemactor->xspr.data1;
#endif

		if (!actHealDude(pPlayer->actor, addPower, gPowerUpInfo[nType].maxTime)) return 0;
		return 1;
	}
	case kItemHealthDoctorBag:
	case kItemJumpBoots:
	case kItemDivingSuit:
	case kItemBeastVision:
		if (!packAddItem(pPlayer, gItemData[nType].packSlot)) return 0;
		break;
	default:
		if (!powerupActivate(pPlayer, nType)) return 0;
		return 1;
	}

	sfxPlay3DSound(plActor->spr.pos.X, plActor->spr.pos.Y, plActor->spr.pos.Z, pickupSnd, plActor->sector());
	return 1;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

bool PickupAmmo(PLAYER* pPlayer, DBloodActor* ammoactor)
{
	const AMMOITEMDATA* pAmmoItemData = &gAmmoItemData[ammoactor->spr.type - kItemAmmoBase];
	int nAmmoType = pAmmoItemData->type;

	if (pPlayer->ammoCount[nAmmoType] >= gAmmoInfo[nAmmoType].max) return 0;
#ifdef NOONE_EXTENSIONS
	else if (gModernMap && ammoactor->hasX() && ammoactor->xspr.data1 > 0) // allow custom amount for item
		pPlayer->ammoCount[nAmmoType] = ClipHigh(pPlayer->ammoCount[nAmmoType] + ammoactor->xspr.data1, gAmmoInfo[nAmmoType].max);
#endif
	else
		pPlayer->ammoCount[nAmmoType] = ClipHigh(pPlayer->ammoCount[nAmmoType] + pAmmoItemData->count, gAmmoInfo[nAmmoType].max);

	if (pAmmoItemData->weaponType)  pPlayer->hasWeapon[pAmmoItemData->weaponType] = 1;
	sfxPlay3DSound(pPlayer->actor, 782, -1, 0);
	return 1;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

bool PickupWeapon(PLAYER* pPlayer, DBloodActor* weaponactor)
{
	const WEAPONITEMDATA* pWeaponItemData = &gWeaponItemData[weaponactor->spr.type - kItemWeaponBase];
	int nWeaponType = pWeaponItemData->type;
	int nAmmoType = pWeaponItemData->ammoType;
	if (!pPlayer->hasWeapon[nWeaponType] || gGameOptions.nWeaponSettings == 2 || gGameOptions.nWeaponSettings == 3) {
		if (weaponactor->spr.type == kItemWeaponLifeLeech && gGameOptions.nGameType > 1 && findDroppedLeech(pPlayer, NULL))
			return 0;
		pPlayer->hasWeapon[nWeaponType] = 1;
		if (nAmmoType == -1) return 0;
		// allow to set custom ammo count for weapon pickups
#ifdef NOONE_EXTENSIONS
		else if (gModernMap && weaponactor->hasX() && weaponactor->xspr.data1 > 0)
			pPlayer->ammoCount[nAmmoType] = ClipHigh(pPlayer->ammoCount[nAmmoType] + weaponactor->xspr.data1, gAmmoInfo[nAmmoType].max);
#endif
		else
			pPlayer->ammoCount[nAmmoType] = ClipHigh(pPlayer->ammoCount[nAmmoType] + pWeaponItemData->count, gAmmoInfo[nAmmoType].max);

		int nNewWeapon = WeaponUpgrade(pPlayer, nWeaponType);
		if (nNewWeapon != pPlayer->curWeapon) {
			pPlayer->weaponState = 0;
			pPlayer->nextWeapon = nNewWeapon;
		}
		sfxPlay3DSound(pPlayer->actor, 777, -1, 0);
		return 1;
	}

	if (!actGetRespawnTime(weaponactor) || nAmmoType == -1 || pPlayer->ammoCount[nAmmoType] >= gAmmoInfo[nAmmoType].max) return 0;
#ifdef NOONE_EXTENSIONS
	else if (gModernMap && weaponactor->hasX() && weaponactor->xspr.data1 > 0)
		pPlayer->ammoCount[nAmmoType] = ClipHigh(pPlayer->ammoCount[nAmmoType] + weaponactor->xspr.data1, gAmmoInfo[nAmmoType].max);
#endif
	else
		pPlayer->ammoCount[nAmmoType] = ClipHigh(pPlayer->ammoCount[nAmmoType] + pWeaponItemData->count, gAmmoInfo[nAmmoType].max);

	sfxPlay3DSound(pPlayer->actor, 777, -1, 0);
	return 1;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void PickUp(PLAYER* pPlayer, DBloodActor* actor)
{
	const char* msg = nullptr;
	int nType = actor->spr.type;
	bool pickedUp = 0;
	int customMsg = -1;
#ifdef NOONE_EXTENSIONS
	if (gModernMap && actor->hasX()) { // allow custom INI message instead "Picked up"
		if (actor->xspr.txID != 3 && actor->xspr.lockMsg > 0)
			customMsg = actor->xspr.lockMsg;
	}
#endif

	if (nType >= kItemBase && nType <= kItemMax) {
		pickedUp = PickupItem(pPlayer, actor);
		if (pickedUp && customMsg == -1) msg = GStrings(FStringf("TXTB_ITEM%02d", int(nType - kItemBase + 1)));

	}
	else if (nType >= kItemAmmoBase && nType < kItemAmmoMax) {
		pickedUp = PickupAmmo(pPlayer, actor);
		if (pickedUp && customMsg == -1) msg = GStrings(FStringf("TXTB_AMMO%02d", int(nType - kItemAmmoBase + 1)));

	}
	else if (nType >= kItemWeaponBase && nType < kItemWeaponMax) {
		pickedUp = PickupWeapon(pPlayer, actor);
		if (pickedUp && customMsg == -1) msg = GStrings(FStringf("TXTB_WPN%02d", int(nType - kItemWeaponBase + 1)));
	}

	if (!pickedUp) return;
	else if (actor->hasX())
	{
		if (actor->xspr.Pickup)
			trTriggerSprite(actor, kCmdSpritePickup);
	}

	if (!actCheckRespawn(actor))
		actPostSprite(actor, kStatFree);

	pPlayer->pickupEffect = 30;
	if (pPlayer == gMe) {
		if (customMsg > 0) trTextOver(customMsg - 1);
		else if (msg) viewSetMessage(msg, nullptr, MESSAGE_PRIORITY_PICKUP);
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void CheckPickUp(PLAYER* pPlayer)
{
	auto plActor = pPlayer->actor;
	int x = plActor->spr.pos.X;
	int y = plActor->spr.pos.Y;
	int z = plActor->spr.pos.Z;
	auto pSector = plActor->sector();
	BloodStatIterator it(kStatItem);
	while (auto itemactor = it.Next())
	{
		if (itemactor->spr.flags & 32)
			continue;
		int dx = abs(x - itemactor->spr.pos.X) >> 4;
		if (dx > 48)
			continue;
		int dy = abs(y - itemactor->spr.pos.Y) >> 4;
		if (dy > 48)
			continue;
		int top, bottom;
		GetActorExtents(plActor, &top, &bottom);
		int vb = 0;
		if (itemactor->spr.pos.Z < top)
			vb = (top - itemactor->spr.pos.Z) >> 8;
		else if (itemactor->spr.pos.Z > bottom)
			vb = (itemactor->spr.pos.Z - bottom) >> 8;
		if (vb > 32)
			continue;
		if (approxDist(dx, dy) > 48)
			continue;
		GetActorExtents(itemactor, &top, &bottom);
		if (cansee(x, y, z, pSector, itemactor->spr.pos.X, itemactor->spr.pos.Y, itemactor->spr.pos.Z, itemactor->sector())
			|| cansee(x, y, z, pSector, itemactor->spr.pos.X, itemactor->spr.pos.Y, top, itemactor->sector())
			|| cansee(x, y, z, pSector, itemactor->spr.pos.X, itemactor->spr.pos.Y, bottom, itemactor->sector()))
			PickUp(pPlayer, itemactor);
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int ActionScan(PLAYER* pPlayer, HitInfo* out)
{
	auto plActor = pPlayer->actor;
	*out = {};
	int x = bcos(plActor->spr.ang);
	int y = bsin(plActor->spr.ang);
	int z = pPlayer->slope;
	int hit = HitScan(pPlayer->actor, pPlayer->zView, x, y, z, 0x10000040, 128);
	int hitDist = approxDist(plActor->spr.pos.X - gHitInfo.hitpos.X, plActor->spr.pos.Y - gHitInfo.hitpos.Y) >> 4;
	if (hitDist < 64)
	{
		switch (hit)
		{
		case 3:
		{
			auto hitactor = gHitInfo.actor();
			if (!hitactor || !hitactor->hasX()) return -1;
			out->hitActor = hitactor;
			if (hitactor->spr.statnum == kStatThing)
			{
				if (hitactor->spr.type == kThingDroppedLifeLeech)
				{
					if (gGameOptions.nGameType > 1 && findDroppedLeech(pPlayer, hitactor))
						return -1;
					hitactor->xspr.data4 = pPlayer->nPlayer;
					hitactor->xspr.isTriggered = 0;
				}
			}
			if (hitactor->xspr.Push)
				return 3;
			if (hitactor->spr.statnum == kStatDude)
			{
				int nMass = getDudeInfo(hitactor->spr.type)->mass;
				if (nMass)
				{
					int t2 = DivScale(0xccccc, nMass, 8);
					hitactor->vel.X += MulScale(x, t2, 16);
					hitactor->vel.Y += MulScale(y, t2, 16);
					hitactor->vel.Z += MulScale(z, t2, 16);
				}
				if (hitactor->xspr.Push && !hitactor->xspr.state && !hitactor->xspr.isTriggered)
					trTriggerSprite(hitactor, kCmdSpritePush);
			}
			break;
		}
		case 0:
		case 4:
		{
			auto pWall = gHitInfo.hitWall;
			out->hitWall = gHitInfo.hitWall;
			if (pWall->hasX() && pWall->xw().triggerPush)
				return 0;
			if (pWall->twoSided())
			{
				auto sect = pWall->nextSector();
				out->hitWall = nullptr;
				out->hitSector = sect;
				if (sect->hasX() && sect->xs().Wallpush)
					return 6;
			}
			break;
		}
		case 1:
		case 2:
		{
			auto pSector = gHitInfo.hitSector;
			out->hitSector = gHitInfo.hitSector;
			if (pSector->hasX() && pSector->xs().Push)
				return 6;
			break;
		}
		}
	}
	out->hitSector = plActor->sector();
	if (plActor->sector()->hasX() && plActor->sector()->xs().Push)
		return 6;
	return -1;
}

//---------------------------------------------------------------------------
//
// Player's sprite angle function, called in ProcessInput() or from gi->GetInput() as required.
//
//---------------------------------------------------------------------------

void UpdatePlayerSpriteAngle(PLAYER* pPlayer)
{
	pPlayer->actor->spr.ang = pPlayer->angle.ang.asbuild();
}

//---------------------------------------------------------------------------
//
// Player's slope tilting wrapper function function, called in ProcessInput() or from gi->GetInput() as required.
//
//---------------------------------------------------------------------------

void doslopetilting(PLAYER* pPlayer, double const scaleAdjust = 1)
{
	auto plActor = pPlayer->actor;
	int const florhit = pPlayer->actor->hit.florhit.type;
	bool const va = plActor->xspr.height < 16 && (florhit == kHitSector || florhit == 0) ? 1 : 0;
	pPlayer->horizon.calcviewpitch(plActor->spr.pos.vec2, buildang(plActor->spr.ang), va, plActor->sector()->floorstat & CSTAT_SECTOR_SLOPE, plActor->sector(), scaleAdjust);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void ProcessInput(PLAYER* pPlayer)
{
	enum
	{
		Item_MedKit = 0,
		Item_CrystalBall = 1,
		Item_BeastVision = 2,
		Item_JumpBoots = 3
	};

	pPlayer->horizon.resetadjustment();
	pPlayer->angle.resetadjustment();

	DBloodActor* actor = pPlayer->actor;
	POSTURE* pPosture = &pPlayer->pPosture[pPlayer->lifeMode][pPlayer->posture];
	InputPacket* pInput = &pPlayer->input;

	// Originally, this was never able to be true due to sloppy input code in the original game.
	// Allow it to become true behind a CVAR to offer an alternate playing experience if desired.
	pPlayer->isRunning = !!(pInput->actions & SB_RUN) && !cl_bloodvanillarun;

	if ((pInput->actions & SB_BUTTON_MASK) || pInput->fvel || pInput->svel || pInput->avel)
		pPlayer->restTime = 0;
	else if (pPlayer->restTime >= 0)
		pPlayer->restTime += 4;
	WeaponProcess(pPlayer);
	if (actor->xspr.health == 0)
	{
		bool bSeqStat = playerSeqPlaying(pPlayer, 16);
		DBloodActor* fragger = pPlayer->fragger;
		if (fragger)
		{
			pPlayer->angle.addadjustment(getincanglebam(pPlayer->angle.ang, bvectangbam(fragger->spr.pos.X - actor->spr.pos.X, fragger->spr.pos.Y - actor->spr.pos.Y)));
		}
		pPlayer->deathTime += 4;
		if (!bSeqStat)
			pPlayer->horizon.addadjustment(q16horiz(MulScale(0x8000 - (Cos(ClipHigh(pPlayer->deathTime << 3, 1024)) >> 15), gi->playerHorizMax(), 16) - pPlayer->horizon.horiz.asq16()));
		if (pPlayer->curWeapon)
			pInput->setNewWeapon(pPlayer->curWeapon);
		if (pInput->actions & SB_OPEN)
		{
			if (bSeqStat)
			{
				if (pPlayer->deathTime > 360)
					seqSpawn(pPlayer->pDudeInfo->seqStartID + 14, pPlayer->actor, nPlayerSurviveClient);
			}
			else if (seqGetStatus(pPlayer->actor) < 0)
			{
				if (pPlayer->actor)
					pPlayer->actor->spr.type = kThingBloodChunks;
				actPostSprite(pPlayer->actor, kStatThing);
				seqSpawn(pPlayer->pDudeInfo->seqStartID + 15, pPlayer->actor, -1);
				playerReset(pPlayer);
				if (gGameOptions.nGameType == 0 && numplayers == 1)
				{
					gameaction = ga_autoloadgame;
				}
				else
					playerStart(pPlayer->nPlayer);
			}
			pInput->actions &= ~SB_OPEN;
		}
		return;
	}
	if (pPlayer->posture == 1)
	{
		int x = Cos(actor->spr.ang);
		int y = Sin(actor->spr.ang);
		if (pInput->fvel)
		{
			int forward = pInput->fvel;
			if (forward > 0)
				forward = MulScale(pPosture->frontAccel, forward, 8);
			else
				forward = MulScale(pPosture->backAccel, forward, 8);
			actor->vel.X += MulScale(forward, x, 30);
			actor->vel.Y += MulScale(forward, y, 30);
		}
		if (pInput->svel)
		{
			int strafe = pInput->svel;
			strafe = MulScale(pPosture->sideAccel, strafe, 8);
			actor->vel.X += MulScale(strafe, y, 30);
			actor->vel.Y -= MulScale(strafe, x, 30);
		}
	}
	else if (actor->xspr.height < 256)
	{
		int speed = 0x10000;
		if (actor->xspr.height > 0)
			speed -= DivScale(actor->xspr.height, 256, 16);
		int x = Cos(actor->spr.ang);
		int y = Sin(actor->spr.ang);
		if (pInput->fvel)
		{
			int forward = pInput->fvel;
			if (forward > 0)
				forward = MulScale(pPosture->frontAccel, forward, 8);
			else
				forward = MulScale(pPosture->backAccel, forward, 8);
			if (actor->xspr.height)
				forward = MulScale(forward, speed, 16);
			actor->vel.X += MulScale(forward, x, 30);
			actor->vel.Y += MulScale(forward, y, 30);
		}
		if (pInput->svel)
		{
			int strafe = pInput->svel;
			strafe = MulScale(pPosture->sideAccel, strafe, 8);
			if (actor->xspr.height)
				strafe = MulScale(strafe, speed, 16);
			actor->vel.X += MulScale(strafe, y, 30);
			actor->vel.Y -= MulScale(strafe, x, 30);
		}
	}

	if (SyncInput())
	{
		pPlayer->angle.applyinput(pInput->avel, &pInput->actions);
	}

	// unconditionally update the player's sprite angle
	// in case game is forcing synchronised input.
	UpdatePlayerSpriteAngle(pPlayer);

	if (!(pInput->actions & SB_JUMP))
		pPlayer->cantJump = 0;

	switch (pPlayer->posture) {
	case 1:
		if (pInput->actions & SB_JUMP)
			actor->vel.Z -= pPosture->normalJumpZ;//0x5b05;
		if (pInput->actions & SB_CROUCH)
			actor->vel.Z += pPosture->normalJumpZ;//0x5b05;
		break;
	case 2:
		if (!(pInput->actions & SB_CROUCH))
			pPlayer->posture = 0;
		break;
	default:
		if (!pPlayer->cantJump && (pInput->actions & SB_JUMP) && actor->xspr.height == 0) {
#ifdef NOONE_EXTENSIONS
			if ((packItemActive(pPlayer, 4) && pPosture->pwupJumpZ != 0) || pPosture->normalJumpZ != 0)
#endif
				sfxPlay3DSound(actor, 700, 0, 0);

			if (packItemActive(pPlayer, 4)) actor->vel.Z = pPosture->pwupJumpZ; //-0x175555;
			else actor->vel.Z = pPosture->normalJumpZ; //-0xbaaaa;
			pPlayer->cantJump = 1;
		}

		if (pInput->actions & SB_CROUCH)
			pPlayer->posture = 2;
		break;
	}
	if (pInput->actions & SB_OPEN)
	{
		HitInfo result;

		int hit = ActionScan(pPlayer, &result);
		switch (hit)
		{
		case 6:
		{
			auto pSector = result.hitSector;
			auto pXSector = &pSector->xs();
			int key = pXSector->Key;
			if (pXSector->locked && pPlayer == gMe)
			{
				viewSetMessage(GStrings("TXTB_LOCKED"));
				auto snd = 3062;
				if (sndCheckPlaying(snd))
					sndStopSample(snd);
				sndStartSample(snd, 255, 2, 0);
			}
			if (!key || pPlayer->hasKey[key])
				trTriggerSector(pSector, kCmdSpritePush);
			else if (pPlayer == gMe)
			{
				viewSetMessage(GStrings("TXTB_KEY"));
				auto snd = 3063;
				if (sndCheckPlaying(snd))
					sndStopSample(snd);
				sndStartSample(snd, 255, 2, 0);
			}
			break;
		}
		case 0:
		{
			auto pWall = result.hitWall;
			auto pXWall = &pWall->xw();
			int key = pXWall->key;
			if (pXWall->locked && pPlayer == gMe)
			{
				viewSetMessage(GStrings("TXTB_LOCKED"));
				auto snd = 3062;
				if (sndCheckPlaying(snd))
					sndStopSample(snd);
				sndStartSample(snd, 255, 2, 0);
			}
			if (!key || pPlayer->hasKey[key])
				trTriggerWall(pWall, kCmdWallPush);
			else if (pPlayer == gMe)
			{
				viewSetMessage(GStrings("TXTB_KEY"));
				auto snd = 3063;
				if (sndCheckPlaying(snd))
					sndStopSample(snd);
				sndStartSample(snd, 255, 2, 0);
			}
			break;
		}
		case 3:
		{
			auto act = result.actor();
			int key = act->xspr.key;
			if (actor->xspr.locked && pPlayer == gMe && act->xspr.lockMsg)
				trTextOver(act->xspr.lockMsg);
			if (!key || pPlayer->hasKey[key])
				trTriggerSprite(act, kCmdSpritePush);
			else if (pPlayer == gMe)
			{
				viewSetMessage(GStrings("TXTB_KEY"));
				sndStartSample(3063, 255, 2, 0);
			}
			break;
		}
		}
		if (pPlayer->handTime > 0)
			pPlayer->handTime = ClipLow(pPlayer->handTime - 4 * (6 - gGameOptions.nDifficulty), 0);
		if (pPlayer->handTime <= 0 && pPlayer->hand)
		{
			DBloodActor* pactor = pPlayer->actor;
			auto spawned = actSpawnDude(pactor, kDudeHand, pPlayer->actor->spr.clipdist << 1, 0);
			if (spawned)
			{
				spawned->spr.ang = (pPlayer->actor->spr.ang + 1024) & 2047;
				int x = bcos(pPlayer->actor->spr.ang);
				int y = bsin(pPlayer->actor->spr.ang);
				spawned->vel.X = pPlayer->actor->vel.X + MulScale(0x155555, x, 14);
				spawned->vel.Y = pPlayer->actor->vel.Y + MulScale(0x155555, y, 14);
				spawned->vel.Z = pPlayer->actor->vel.Z;
			}
			pPlayer->hand = 0;
		}
		pInput->actions &= ~SB_OPEN;
	}

	if (SyncInput())
	{
		pPlayer->horizon.applyinput(pInput->horz, &pInput->actions);
		doslopetilting(pPlayer);
	}

	pPlayer->angle.unlockinput();
	pPlayer->horizon.unlockinput();

	pPlayer->slope = -pPlayer->horizon.horiz.asq16() >> 9;
	if (pInput->actions & SB_INVPREV)
	{
		pInput->actions &= ~SB_INVPREV;
		packPrevItem(pPlayer);
	}
	if (pInput->actions & SB_INVNEXT)
	{
		pInput->actions &= ~SB_INVNEXT;
		packNextItem(pPlayer);
	}
	if (pInput->actions & SB_INVUSE)
	{
		pInput->actions &= ~SB_INVUSE;
		if (pPlayer->packSlots[pPlayer->packItemId].curAmount > 0)
			packUseItem(pPlayer, pPlayer->packItemId);
	}
	if (pInput->isItemUsed(Item_BeastVision))
	{
		pInput->clearItemUsed(Item_BeastVision);
		if (pPlayer->packSlots[3].curAmount > 0)
			packUseItem(pPlayer, 3);
	}
	if (pInput->isItemUsed(Item_CrystalBall))
	{
		pInput->clearItemUsed(Item_CrystalBall);
		if (pPlayer->packSlots[2].curAmount > 0)
			packUseItem(pPlayer, 2);
	}
	if (pInput->isItemUsed(Item_JumpBoots))
	{
		pInput->clearItemUsed(Item_JumpBoots);
		if (pPlayer->packSlots[4].curAmount > 0)
			packUseItem(pPlayer, 4);
	}
	if (pInput->isItemUsed(Item_MedKit))
	{
		pInput->clearItemUsed(Item_MedKit);
		if (pPlayer->packSlots[0].curAmount > 0)
			packUseItem(pPlayer, 0);
	}
	if (pInput->actions & SB_HOLSTER)
	{
		pInput->actions &= ~SB_HOLSTER;
		if (pPlayer->curWeapon)
		{
			WeaponLower(pPlayer);
			viewSetMessage("Holstering weapon");
		}
	}
	CheckPickUp(pPlayer);
}

void playerProcess(PLAYER* pPlayer)
{
	DBloodActor* actor = pPlayer->actor;
	POSTURE* pPosture = &pPlayer->pPosture[pPlayer->lifeMode][pPlayer->posture];
	powerupProcess(pPlayer);
	int top, bottom;
	GetActorExtents(actor, &top, &bottom);
	int dzb = (bottom - actor->spr.pos.Z) / 4;
	int dzt = (actor->spr.pos.Z - top) / 4;
	int dw = actor->spr.clipdist << 2;
	if (!gNoClip)
	{
		auto pSector = actor->sector();
		if (pushmove(&actor->spr.pos, &pSector, dw, dzt, dzb, CLIPMASK0) == -1)
			actDamageSprite(actor, actor, kDamageFall, 500 << 4);
		if (actor->sector() != pSector)
		{
			if (pSector == nullptr)
			{
				pSector = actor->sector();
				actDamageSprite(actor, actor, kDamageFall, 500 << 4);
			}
			else
				ChangeActorSect(actor, pSector);
		}
	}
	ProcessInput(pPlayer);
	int nSpeed = approxDist(actor->vel.X, actor->vel.Y);
	pPlayer->zViewVel = interpolatedvalue(pPlayer->zViewVel, actor->vel.Z, 0x7000);
	int dz = pPlayer->actor->spr.pos.Z - pPosture->eyeAboveZ - pPlayer->zView;
	if (dz > 0)
		pPlayer->zViewVel += MulScale(dz << 8, 0xa000, 16);
	else
		pPlayer->zViewVel += MulScale(dz << 8, 0x1800, 16);
	pPlayer->zView += pPlayer->zViewVel >> 8;
	pPlayer->zWeaponVel = interpolatedvalue(pPlayer->zWeaponVel, actor->vel.Z, 0x5000);
	dz = pPlayer->actor->spr.pos.Z - pPosture->weaponAboveZ - pPlayer->zWeapon;
	if (dz > 0)
		pPlayer->zWeaponVel += MulScale(dz << 8, 0x8000, 16);
	else
		pPlayer->zWeaponVel += MulScale(dz << 8, 0xc00, 16);
	pPlayer->zWeapon += pPlayer->zWeaponVel >> 8;
	pPlayer->bobPhase = ClipLow(pPlayer->bobPhase - 4, 0);
	nSpeed >>= FRACBITS;
	if (pPlayer->posture == 1)
	{
		pPlayer->bobAmp = (pPlayer->bobAmp + 17) & 2047;
		pPlayer->swayAmp = (pPlayer->swayAmp + 17) & 2047;
		pPlayer->bobHeight = MulScale(pPosture->bobV * 10, Sin(pPlayer->bobAmp * 2), 30);
		pPlayer->bobWidth = MulScale(pPosture->bobH * pPlayer->bobPhase, Sin(pPlayer->bobAmp - 256), 30);
		pPlayer->swayHeight = MulScale(pPosture->swayV * pPlayer->bobPhase, Sin(pPlayer->swayAmp * 2), 30);
		pPlayer->swayWidth = MulScale(pPosture->swayH * pPlayer->bobPhase, Sin(pPlayer->swayAmp - 0x155), 30);
	}
	else
	{
		if (actor->xspr.height < 256)
		{
			bool running = pPlayer->isRunning && !cl_bloodvanillabobbing;
			pPlayer->bobAmp = (pPlayer->bobAmp + pPosture->pace[running] * 4) & 2047;
			pPlayer->swayAmp = (pPlayer->swayAmp + (pPosture->pace[running] * 4) / 2) & 2047;
			if (running)
			{
				if (pPlayer->bobPhase < 60)
					pPlayer->bobPhase = ClipHigh(pPlayer->bobPhase + nSpeed, 60);
			}
			else
			{
				if (pPlayer->bobPhase < 30)
					pPlayer->bobPhase = ClipHigh(pPlayer->bobPhase + nSpeed, 30);
			}
		}
		pPlayer->bobHeight = MulScale(pPosture->bobV * pPlayer->bobPhase, Sin(pPlayer->bobAmp * 2), 30);
		pPlayer->bobWidth = MulScale(pPosture->bobH * pPlayer->bobPhase, Sin(pPlayer->bobAmp - 256), 30);
		pPlayer->swayHeight = MulScale(pPosture->swayV * pPlayer->bobPhase, Sin(pPlayer->swayAmp * 2), 30);
		pPlayer->swayWidth = MulScale(pPosture->swayH * pPlayer->bobPhase, Sin(pPlayer->swayAmp - 0x155), 30);
	}
	pPlayer->flickerEffect = 0;
	pPlayer->quakeEffect = ClipLow(pPlayer->quakeEffect - 4, 0);
	pPlayer->tiltEffect = ClipLow(pPlayer->tiltEffect - 4, 0);
	pPlayer->visibility = ClipLow(pPlayer->visibility - 4, 0);
	pPlayer->painEffect = ClipLow(pPlayer->painEffect - 4, 0);
	pPlayer->blindEffect = ClipLow(pPlayer->blindEffect - 4, 0);
	pPlayer->pickupEffect = ClipLow(pPlayer->pickupEffect - 4, 0);
	if (pPlayer == gMe && gMe->actor->xspr.health == 0)
		pPlayer->hand = 0;
	if (!actor->xspr.health)
		return;
	pPlayer->isUnderwater = 0;
	if (pPlayer->posture == 1)
	{
		pPlayer->isUnderwater = 1;
		auto link = actor->sector()->lowerLink;
		if (link && (link->spr.type == kMarkerLowGoo || link->spr.type == kMarkerLowWater))
		{
			if (getceilzofslopeptr(actor->sector(), actor->spr.pos.X, actor->spr.pos.Y) > pPlayer->zView)
				pPlayer->isUnderwater = 0;
		}
	}
	if (!pPlayer->isUnderwater)
	{
		pPlayer->underwaterTime = 1200;
		pPlayer->chokeEffect = 0;
		if (packItemActive(pPlayer, 1))
			packUseItem(pPlayer, 1);
	}
	int nType = kDudePlayer1 - kDudeBase;
	switch (pPlayer->posture)
	{
	case 1:
		seqSpawn(dudeInfo[nType].seqStartID + 9, actor, -1);
		break;
	case 2:
		seqSpawn(dudeInfo[nType].seqStartID + 10, actor, -1);
		break;
	default:
		if (!nSpeed)
			seqSpawn(dudeInfo[nType].seqStartID, actor, -1);
		else
			seqSpawn(dudeInfo[nType].seqStartID + 8, actor, -1);
		break;
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

DBloodActor* playerFireMissile(PLAYER* pPlayer, int a2, int a3, int a4, int a5, int a6)
{
	return actFireMissile(pPlayer->actor, a2, pPlayer->zWeapon - pPlayer->actor->spr.pos.Z, a3, a4, a5, a6);
}

DBloodActor* playerFireThing(PLAYER* pPlayer, int a2, int a3, int thingType, int a5)
{
	assert(thingType >= kThingBase && thingType < kThingMax);
	return actFireThing(pPlayer->actor, a2, pPlayer->zWeapon - pPlayer->actor->spr.pos.Z, pPlayer->slope + a3, thingType, a5);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void playerFrag(PLAYER* pKiller, PLAYER* pVictim)
{
	assert(pKiller != NULL);
	assert(pVictim != NULL);

	char buffer[128] = "";
	int nKiller = pKiller->actor->spr.type - kDudePlayer1;
	assert(nKiller >= 0 && nKiller < kMaxPlayers);
	int nVictim = pVictim->actor->spr.type - kDudePlayer1;
	assert(nVictim >= 0 && nVictim < kMaxPlayers);
	if (nKiller == nVictim)
	{
		pVictim->fragger = nullptr;
		if (VanillaMode() || gGameOptions.nGameType != 1)
		{
			pVictim->fragCount--;
			pVictim->fragInfo[nVictim]--;
		}
		if (gGameOptions.nGameType == 3)
			team_score[pVictim->teamId]--;
		int nMessage = Random(5);
		int nSound = gSuicide[nMessage].Kills;
		if (pVictim == gMe && gMe->handTime <= 0)
		{
			strcpy(buffer, GStrings("TXTB_KILLSELF"));
			if (gGameOptions.nGameType > 0 && nSound >= 0)
				sndStartSample(nSound, 255, 2, 0);
		}
		else
		{
			sprintf(buffer, gSuicide[nMessage].message, PlayerName(nVictim));
		}
	}
	else
	{
		if (VanillaMode() || gGameOptions.nGameType != 1)
		{
			pKiller->fragCount++;
			pKiller->fragInfo[nKiller]++;
		}
		if (gGameOptions.nGameType == 3)
		{
			if (pKiller->teamId == pVictim->teamId)
				team_score[pKiller->teamId]--;
			else
			{
				team_score[pKiller->teamId]++;
				team_ticker[pKiller->teamId] += 120;
			}
		}
		int nMessage = Random(25);
		int nSound = gVictory[nMessage].Kills;
		const char* pzMessage = gVictory[nMessage].message;
		sprintf(buffer, pzMessage, PlayerName(nKiller), PlayerName(nVictim));
		if (gGameOptions.nGameType > 0 && nSound >= 0 && pKiller == gMe)
			sndStartSample(nSound, 255, 2, 0);
	}
	viewSetMessage(buffer);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void FragPlayer(PLAYER* pPlayer, DBloodActor* killer)
{
	if (killer && killer->IsPlayerActor())
	{
		PLAYER* pKiller = &gPlayer[killer->spr.type - kDudePlayer1];
		playerFrag(pKiller, pPlayer);
		int nTeam1 = pKiller->teamId & 1;
		int nTeam2 = pPlayer->teamId & 1;
		if (nTeam1 == 0)
		{
			if (nTeam1 != nTeam2)
				evSendGame(15, kCmdToggle);
			else
				evSendGame(16, kCmdToggle);
		}
		else
		{
			if (nTeam1 == nTeam2)
				evSendGame(16, kCmdToggle);
			else
				evSendGame(15, kCmdToggle);
		}
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int playerDamageArmor(PLAYER* pPlayer, DAMAGE_TYPE nType, int nDamage)
{
	DAMAGEINFO* pDamageInfo = &damageInfo[nType];
	int nArmorType = pDamageInfo->armorType;
	if (nArmorType >= 0 && pPlayer->armor[nArmorType])
	{
#if 0
		int vbp = (nDamage * 7) / 8 - nDamage / 4;
		int v8 = pPlayer->at33e[nArmorType];
		int t = nDamage / 4 + vbp * v8 / 3200;
		v8 -= t;
#endif
		int v8 = pPlayer->armor[nArmorType];
		int t = scale(v8, 0, 3200, nDamage / 4, (nDamage * 7) / 8);
		v8 -= t;
		nDamage -= t;
		pPlayer->armor[nArmorType] = ClipLow(v8, 0);
	}
	return nDamage;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void flagDropped(PLAYER* pPlayer, int a2)
{
	DBloodActor* playeractor = pPlayer->actor;
	DBloodActor* actor;
	char buffer[80];
	switch (a2)
	{
	case kItemFlagA:
		pPlayer->hasFlag &= ~1;
		actor = actDropObject(playeractor, kItemFlagA);
		if (actor) actor->SetOwner(pPlayer->ctfFlagState[0]);
		gBlueFlagDropped = true;
		sprintf(buffer, "%s dropped Blue Flag", PlayerName(pPlayer->nPlayer));
		sndStartSample(8005, 255, 2, 0);
		viewSetMessage(buffer);
		break;
	case kItemFlagB:
		pPlayer->hasFlag &= ~2;
		actor = actDropObject(playeractor, kItemFlagB);
		if (actor) actor->SetOwner(pPlayer->ctfFlagState[1]);
		gRedFlagDropped = true;
		sprintf(buffer, "%s dropped Red Flag", PlayerName(pPlayer->nPlayer));
		sndStartSample(8004, 255, 2, 0);
		viewSetMessage(buffer);
		break;
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int playerDamageSprite(DBloodActor* source, PLAYER* pPlayer, DAMAGE_TYPE nDamageType, int nDamage)
{
	assert(pPlayer != NULL);
	if (pPlayer->damageControl[nDamageType] || pPlayer->godMode)
		return 0;
	nDamage = playerDamageArmor(pPlayer, nDamageType, nDamage);
	pPlayer->painEffect = ClipHigh(pPlayer->painEffect + (nDamage >> 3), 600);

	DBloodActor* pActor = pPlayer->actor;
	DUDEINFO* pDudeInfo = getDudeInfo(pActor->spr.type);
	int nDeathSeqID = -1;
	int nKneelingPlayer = -1;
	bool va = playerSeqPlaying(pPlayer, 16);
	if (!pActor->xspr.health)
	{
		if (va)
		{
			switch (nDamageType)
			{
			case kDamageSpirit:
				nDeathSeqID = 18;
				sfxPlay3DSound(pPlayer->actor, 716, 0, 0);
				break;
			case kDamageExplode:
				GibSprite(pActor, GIBTYPE_7, NULL, NULL);
				GibSprite(pActor, GIBTYPE_15, NULL, NULL);
				pPlayer->actor->spr.cstat |= CSTAT_SPRITE_INVISIBLE;
				nDeathSeqID = 17;
				break;
			default:
			{
				int top, bottom;
				GetActorExtents(pActor, &top, &bottom);
				CGibPosition gibPos(pActor->spr.pos.X, pActor->spr.pos.Y, top);
				CGibVelocity gibVel(pActor->vel.X >> 1, pActor->vel.Y >> 1, -0xccccc);
				GibSprite(pActor, GIBTYPE_27, &gibPos, &gibVel);
				GibSprite(pActor, GIBTYPE_7, NULL, NULL);
				fxSpawnBlood(pActor, nDamage << 4);
				fxSpawnBlood(pActor, nDamage << 4);
				nDeathSeqID = 17;
				break;
			}
			}
		}
	}
	else
	{
		int nHealth = pActor->xspr.health - nDamage;
		pActor->xspr.health = ClipLow(nHealth, 0);
		if (pActor->xspr.health > 0 && pActor->xspr.health < 16)
		{
			nDamageType = kDamageBullet;
			pActor->xspr.health = 0;
			nHealth = -25;
		}
		if (pActor->xspr.health > 0)
		{
			DAMAGEINFO* pDamageInfo = &damageInfo[nDamageType];
			int nSound;
			if (nDamage >= (10 << 4))
				nSound = pDamageInfo->Kills[0];
			else
				nSound = pDamageInfo->Kills[Random(3)];
			if (nDamageType == kDamageDrown && pActor->xspr.medium == kMediumWater && !pPlayer->hand)
				nSound = 714;
			sfxPlay3DSound(pPlayer->actor, nSound, 0, 6);
			return nDamage;
		}
		sfxKill3DSound(pPlayer->actor, -1, 441);
		if (gGameOptions.nGameType == 3 && pPlayer->hasFlag) {
			if (pPlayer->hasFlag & 1) flagDropped(pPlayer, kItemFlagA);
			if (pPlayer->hasFlag & 2) flagDropped(pPlayer, kItemFlagB);
		}
		pPlayer->deathTime = 0;
		pPlayer->qavLoop = 0;
		pPlayer->curWeapon = kWeapNone;
		pPlayer->fragger = source;
		pPlayer->voodooTargets = 0;
		if (nDamageType == kDamageExplode && nDamage < (9 << 4))
			nDamageType = kDamageFall;
		switch (nDamageType)
		{
		case kDamageExplode:
			sfxPlay3DSound(pPlayer->actor, 717, 0, 0);
			GibSprite(pActor, GIBTYPE_7, NULL, NULL);
			GibSprite(pActor, GIBTYPE_15, NULL, NULL);
			pPlayer->actor->spr.cstat |= CSTAT_SPRITE_INVISIBLE;
			nDeathSeqID = 2;
			break;
		case kDamageBurn:
			sfxPlay3DSound(pPlayer->actor, 718, 0, 0);
			nDeathSeqID = 3;
			break;
		case kDamageDrown:
			nDeathSeqID = 1;
			break;
		default:
			if (nHealth < -20 && gGameOptions.nGameType >= 2 && Chance(0x4000))
			{
				DAMAGEINFO* pDamageInfo = &damageInfo[nDamageType];
				sfxPlay3DSound(pPlayer->actor, pDamageInfo->at10[0], 0, 2);
				nDeathSeqID = 16;
				nKneelingPlayer = nPlayerKneelClient;
				powerupActivate(pPlayer, kPwUpDeliriumShroom);
				pActor->SetTarget(source);
				evPostActor(pPlayer->actor, 15, kCallbackFinishHim);
			}
			else
			{
				sfxPlay3DSound(pPlayer->actor, 716, 0, 0);
				nDeathSeqID = 1;
			}
			break;
		}
	}
	if (nDeathSeqID < 0)
		return nDamage;
	if (nDeathSeqID != 16)
	{
		powerupClear(pPlayer);
		if (pActor->sector()->hasX() && pActor->sector()->xs().Exit)
			trTriggerSector(pActor->sector(), kCmdSectorExit);
		pActor->spr.flags |= 7;
		for (int p = connecthead; p >= 0; p = connectpoint2[p])
		{
			if (gPlayer[p].fragger == pPlayer->actor && gPlayer[p].deathTime > 0)
				gPlayer[p].fragger = nullptr;
		}
		FragPlayer(pPlayer, source);
		trTriggerSprite(pActor, kCmdOff);

#ifdef NOONE_EXTENSIONS
		// allow drop items and keys in multiplayer
		if (gModernMap && gGameOptions.nGameType != 0 && pPlayer->actor->xspr.health <= 0) {

			DBloodActor* pItem = nullptr;
			if (pPlayer->actor->xspr.dropMsg && (pItem = actDropItem(pActor, pPlayer->actor->xspr.dropMsg)) != NULL)
				evPostActor(pItem, 500, kCallbackRemove);

			if (pPlayer->actor->xspr.key) {

				int i; // if all players have this key, don't drop it
				for (i = connecthead; i >= 0; i = connectpoint2[i]) {
					if (!gPlayer[i].hasKey[pPlayer->actor->xspr.key])
						break;
				}

				if (i == 0 && (pItem = actDropKey(pActor, (pPlayer->actor->xspr.key + kItemKeyBase) - 1)) != NULL)
					evPostActor(pItem, 500, kCallbackRemove);

			}


		}
#endif

	}
	assert(getSequence(pDudeInfo->seqStartID + nDeathSeqID) != NULL);
	seqSpawn(pDudeInfo->seqStartID + nDeathSeqID, pPlayer->actor, nKneelingPlayer);
	return nDamage;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int UseAmmo(PLAYER* pPlayer, int nAmmoType, int nDec)
{
	if (gInfiniteAmmo)
		return 9999;
	if (nAmmoType == -1)
		return 9999;
	pPlayer->ammoCount[nAmmoType] = ClipLow(pPlayer->ammoCount[nAmmoType] - nDec, 0);
	return pPlayer->ammoCount[nAmmoType];
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void voodooTarget(PLAYER* pPlayer)
{
	DBloodActor* actor = pPlayer->actor;
	int v4 = pPlayer->aim.dz;
	int dz = pPlayer->zWeapon - pPlayer->actor->spr.pos.Z;
	if (UseAmmo(pPlayer, 9, 0) < 8)
	{
		pPlayer->voodooTargets = 0;
		return;
	}
	for (int i = 0; i < 4; i++)
	{
		int ang1 = (pPlayer->voodooVar1 + pPlayer->vodooVar2) & 2047;
		actFireVector(actor, 0, dz, bcos(ang1), bsin(ang1), v4, kVectorVoodoo10);
		int ang2 = (pPlayer->voodooVar1 + 2048 - pPlayer->vodooVar2) & 2047;
		actFireVector(actor, 0, dz, bcos(ang2), bsin(ang2), v4, kVectorVoodoo10);
	}
	pPlayer->voodooTargets = ClipLow(pPlayer->voodooTargets - 1, 0);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void playerLandingSound(PLAYER* pPlayer)
{
	static int surfaceSound[] = {
		-1,
		600,
		601,
		602,
		603,
		604,
		605,
		605,
		605,
		600,
		605,
		605,
		605,
		604,
		603
	};
	SPRITEHIT* pHit = &pPlayer->actor->hit;
	if (pHit->florhit.type != kHitNone)
	{
		if (!gGameOptions.bFriendlyFire && pHit->florhit.type == kHitSprite && IsTargetTeammate(pPlayer, pHit->florhit.actor()))
			return;
		int nSurf = tileGetSurfType(pHit->florhit);
		if (nSurf)
			sfxPlay3DSound(pPlayer->actor, surfaceSound[nSurf], -1, 0);
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void PlayerSurvive(int, DBloodActor* actor)
{
	char buffer[80];
	actHealDude(actor, 1, 2);
	if (gGameOptions.nGameType > 0 && numplayers > 1)
	{
		sfxPlay3DSound(actor, 3009, 0, 6);
		if (actor->IsPlayerActor())
		{
			PLAYER* pPlayer = &gPlayer[actor->spr.type - kDudePlayer1];
			if (pPlayer == gMe)
				viewSetMessage(GStrings("TXT_LIVEAGAIM"));
			else
			{
				sprintf(buffer, "%s lives again!", PlayerName(pPlayer->nPlayer));
				viewSetMessage(buffer);
			}
			pPlayer->newWeapon = kWeapPitchFork;
		}
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void PlayerKneelsOver(int, DBloodActor* actor)
{
	for (int p = connecthead; p >= 0; p = connectpoint2[p])
	{
		if (gPlayer[p].actor == actor)
		{
			PLAYER* pPlayer = &gPlayer[p];
			playerDamageSprite(pPlayer->fragger, pPlayer, kDamageSpirit, 500 << 4);
			return;
		}
	}
}


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

FSerializer& Serialize(FSerializer& arc, const char* keyname, Aim& w, Aim* def)
{
	if (arc.BeginObject(keyname))
	{
		arc("x", w.dx)
			("y", w.dx)
			("z", w.dx)
			.EndObject();
	}
	return arc;
}

FSerializer& Serialize(FSerializer& arc, const char* keyname, PACKINFO& w, PACKINFO* def)
{
	if (arc.BeginObject(keyname))
	{
		arc("isactive", w.isActive)
			("curamount", w.curAmount)
			.EndObject();
	}
	return arc;
}

FSerializer& Serialize(FSerializer& arc, const char* keyname, POSTURE& w, POSTURE* def)
{
	if (arc.BeginObject(keyname))
	{
		arc("frontaccel", w.frontAccel, def->frontAccel)
			("sideaccel", w.sideAccel, def->sideAccel)
			("backaccel", w.backAccel, def->backAccel)
			("pace0", w.pace[0], def->pace[0])
			("pace1", w.pace[1], def->pace[1])
			("bobv", w.bobV, def->bobV)
			("bobh", w.bobH, def->bobH)
			("swayv", w.swayV, def->swayV)
			("swayh", w.swayH, def->swayH)
			("eyeabovez", w.eyeAboveZ, def->eyeAboveZ)
			("weaponabovez", w.weaponAboveZ, def->weaponAboveZ)
			("xoffset", w.xOffset, def->xOffset)
			("zoffset", w.zOffset, def->zOffset)
			("normaljumpz", w.normalJumpZ, def->normalJumpZ)
			("pwupjumpz", w.pwupJumpZ, def->pwupJumpZ)
			.EndObject();
	}
	return arc;
}

FSerializer& Serialize(FSerializer& arc, const char* keyname, PLAYER& w, PLAYER* def)
{
	if (arc.isReading()) playerResetPosture(&w);
	if (arc.BeginObject(keyname))
	{
		arc("spritenum", w.actor)
			("horizon", w.horizon)
			("angle", w.angle)
			("newweapon", w.newWeapon)
			("used1", w.used1)
			("weaponqav", w.weaponQav)
			("qavcallback", w.qavCallback)
			("isrunning", w.isRunning)
			("posture", w.posture)
			("sceneqav", w.sceneQav)
			("bobphase", w.bobPhase)
			("bobamp", w.bobAmp)
			("bobheight", w.bobHeight)
			("bobwidth", w.bobWidth)
			("swayphase", w.swayPhase)
			("swayamp", w.swayAmp)
			("swayheight", w.swayHeight)
			("swaywidth", w.swayWidth)
			("nplayer", w.nPlayer)
			("lifemode", w.lifeMode)
			("bloodlust", w.bloodlust)
			("zview", w.zView)
			("zviewvel", w.zViewVel)
			("zweapon", w.zWeapon)
			("zweaponvel", w.zWeaponVel)
			("slope", w.slope)
			("underwater", w.isUnderwater)
			.Array("haskey", w.hasKey, 8)
			("hasflag", w.hasFlag)
			.Array("ctfflagstate", w.ctfFlagState, 2)
			.Array("dmgcontrol", w.damageControl, 7)
			("curweapon", w.curWeapon)
			("nextweapon", w.nextWeapon)
			("weapontimer", w.weaponTimer)
			("weaponstate", w.weaponState)
			("weaponammo", w.weaponAmmo)
			.Array("hasweapon", w.hasWeapon, countof(w.hasWeapon))
			.Array("weaponmode", w.weaponMode, countof(w.weaponMode))
			.Array("weaponorder", &w.weaponOrder[0][kWeapNone], +kWeapMax * 2)
			.Array("ammocount", w.ammoCount, countof(w.ammoCount))
			("qavloop", w.qavLoop)
			("qavlastTick", w.qavLastTick)
			("qavtimer", w.qavTimer)
			("fusetime", w.fuseTime)
			("throwtime", w.throwTime)
			("throwpower", w.throwPower)
			("aim", w.aim)
			("relaim", w.relAim)
			("aimtarget", w.aimTarget)
			("aimtargetscount", w.aimTargetsCount)
			.Array("aimtargets", w.aimTargets, countof(w.aimTargets))
			("deathtime", w.deathTime)
			.Array("pwuptime", w.pwUpTime, countof(w.pwUpTime))
			("fragcount", w.fragCount)
			.Array("fraginfo", w.fragInfo, countof(w.fragInfo))
			("teamid", w.teamId)
			("fraggerid", w.fragger)
			("undserwatertime", w.underwaterTime)
			("bubbletime", w.bubbleTime)
			("resttime", w.restTime)
			("kickpower", w.kickPower)
			("laughcount", w.laughCount)
			("godmode", w.godMode)
			("fallscream", w.fallScream)
			("cantjump", w.cantJump)
			("packitemtime", w.packItemTime)
			("packitemid", w.packItemId)
			.Array("packslots", w.packSlots, countof(w.packSlots))
			.Array("armor", w.armor, countof(w.armor))
			("voodootarget", w.voodooTarget)
			("voodootargets", w.voodooTargets)
			("voodoovar1", w.voodooVar1)
			("voodoovar2", w.vodooVar2)
			("flickereffect", w.flickerEffect)
			("tilteffect", w.tiltEffect)
			("visibility", w.visibility)
			("paineffect", w.painEffect)
			("blindeffect", w.blindEffect)
			("chokeeffect", w.chokeEffect)
			("handtime", w.handTime)
			("hand", w.hand)
			("pickupeffect", w.pickupEffect)
			("flasheffect", w.flashEffect)
			("quakeeffect", w.quakeEffect)
			("player_par", w.player_par)
			("waterpal", w.nWaterPal)
			.Array("posturedata", &w.pPosture[0][0], &gPostureDefaults[0][0], kModeMax * kPostureMax) // only save actual changes in this.
			.EndObject();
	}
	return arc;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

#ifdef NOONE_EXTENSIONS
FSerializer& Serialize(FSerializer& arc, const char* keyname, TRPLAYERCTRL& w, TRPLAYERCTRL* def)
{
	if (arc.BeginObject(keyname))
	{
		arc("index", w.qavScene.initiator)
			("dummy", w.qavScene.dummy)
			.EndObject();
	}
	if (arc.isReading()) w.qavScene.qavResrc = nullptr;
	return arc;
}
#endif


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void SerializePlayers(FSerializer& arc)
{
	if (arc.BeginObject("players"))
	{
		arc("numplayers", gNetPlayers)
			.Array("teamscore", team_score, gNetPlayers)
			.Array("players", gPlayer, gNetPlayers)
#ifdef NOONE_EXTENSIONS
			.Array("playerctrl", gPlayerCtrl, gNetPlayers)
#endif
			.EndObject();
	}

	if (arc.isReading())
	{
		for (int i = 0; i < gNetPlayers; i++)
		{
			gPlayer[i].pDudeInfo = &dudeInfo[gPlayer[i].actor->spr.type - kDudeBase];

#ifdef NOONE_EXTENSIONS
			// load qav scene
			if (gPlayer[i].sceneQav != -1)
			{
				QAV* pQav = playerQavSceneLoad(gPlayer[i].sceneQav);
				if (pQav)
				{
					gPlayerCtrl[i].qavScene.qavResrc = pQav;
					//gPlayerCtrl[i].qavScene.qavResrc->Preload();
				}
				else
				{
					gPlayer[i].sceneQav = -1;
				}
			}
#endif
		}
	}
}



DEFINE_FIELD_X(BloodPlayer, PLAYER, newWeapon)
DEFINE_FIELD_X(BloodPlayer, PLAYER, weaponQav)
DEFINE_FIELD_X(BloodPlayer, PLAYER, qavCallback)
DEFINE_FIELD_X(BloodPlayer, PLAYER, isRunning)
DEFINE_FIELD_X(BloodPlayer, PLAYER, posture)   // stand, crouch, swim
DEFINE_FIELD_X(BloodPlayer, PLAYER, sceneQav)  // by NoOne: used to keep qav id
DEFINE_FIELD_X(BloodPlayer, PLAYER, bobPhase)
DEFINE_FIELD_X(BloodPlayer, PLAYER, bobAmp)
DEFINE_FIELD_X(BloodPlayer, PLAYER, bobHeight)
DEFINE_FIELD_X(BloodPlayer, PLAYER, bobWidth)
DEFINE_FIELD_X(BloodPlayer, PLAYER, swayPhase)
DEFINE_FIELD_X(BloodPlayer, PLAYER, swayAmp)
DEFINE_FIELD_X(BloodPlayer, PLAYER, swayHeight)
DEFINE_FIELD_X(BloodPlayer, PLAYER, swayWidth)
DEFINE_FIELD_X(BloodPlayer, PLAYER, nPlayer)  // Connect id
DEFINE_FIELD_X(BloodPlayer, PLAYER, lifeMode)
DEFINE_FIELD_X(BloodPlayer, PLAYER, zView)
DEFINE_FIELD_X(BloodPlayer, PLAYER, zViewVel)
DEFINE_FIELD_X(BloodPlayer, PLAYER, zWeapon)
DEFINE_FIELD_X(BloodPlayer, PLAYER, zWeaponVel)
DEFINE_FIELD_X(BloodPlayer, PLAYER, slope)
DEFINE_FIELD_X(BloodPlayer, PLAYER, isUnderwater)
DEFINE_FIELD_X(BloodPlayer, PLAYER, hasKey)
DEFINE_FIELD_X(BloodPlayer, PLAYER, hasFlag)
DEFINE_FIELD_X(BloodPlayer, PLAYER, damageControl)
DEFINE_FIELD_X(BloodPlayer, PLAYER, curWeapon)
DEFINE_FIELD_X(BloodPlayer, PLAYER, nextWeapon)
DEFINE_FIELD_X(BloodPlayer, PLAYER, weaponTimer)
DEFINE_FIELD_X(BloodPlayer, PLAYER, weaponState)
DEFINE_FIELD_X(BloodPlayer, PLAYER, weaponAmmo)  //rename
DEFINE_FIELD_X(BloodPlayer, PLAYER, hasWeapon)
DEFINE_FIELD_X(BloodPlayer, PLAYER, weaponMode)
DEFINE_FIELD_X(BloodPlayer, PLAYER, weaponOrder)
DEFINE_FIELD_X(BloodPlayer, PLAYER, ammoCount)
DEFINE_FIELD_X(BloodPlayer, PLAYER, qavLoop)
DEFINE_FIELD_X(BloodPlayer, PLAYER, fuseTime)
DEFINE_FIELD_X(BloodPlayer, PLAYER, throwTime)
DEFINE_FIELD_X(BloodPlayer, PLAYER, throwPower)
DEFINE_FIELD_X(BloodPlayer, PLAYER, aim)  // world
DEFINE_FIELD_X(BloodPlayer, PLAYER, aimTargetsCount)
//DEFINE_FIELD_X(BloodPlayer, PLAYER, aimTargets)
DEFINE_FIELD_X(BloodPlayer, PLAYER, deathTime)
DEFINE_FIELD_X(BloodPlayer, PLAYER, pwUpTime)
DEFINE_FIELD_X(BloodPlayer, PLAYER, teamId)
DEFINE_FIELD_X(BloodPlayer, PLAYER, fragCount)
DEFINE_FIELD_X(BloodPlayer, PLAYER, fragInfo)
DEFINE_FIELD_X(BloodPlayer, PLAYER, underwaterTime)
DEFINE_FIELD_X(BloodPlayer, PLAYER, bubbleTime)
DEFINE_FIELD_X(BloodPlayer, PLAYER, restTime)
DEFINE_FIELD_X(BloodPlayer, PLAYER, kickPower)
DEFINE_FIELD_X(BloodPlayer, PLAYER, laughCount)
DEFINE_FIELD_X(BloodPlayer, PLAYER, godMode)
DEFINE_FIELD_X(BloodPlayer, PLAYER, fallScream)
DEFINE_FIELD_X(BloodPlayer, PLAYER, cantJump)
DEFINE_FIELD_X(BloodPlayer, PLAYER, packItemTime)  // pack timer
DEFINE_FIELD_X(BloodPlayer, PLAYER, packItemId)    // pack id 1: diving suit, 2: crystal ball, 3:
DEFINE_FIELD_X(BloodPlayer, PLAYER, packSlots)  // at325 1]: diving suit, [2]: crystal ball, 
DEFINE_FIELD_X(BloodPlayer, PLAYER, armor)      // armor
//DEFINE_FIELD_X(BloodPlayer, PLAYER, voodooTarget)
DEFINE_FIELD_X(BloodPlayer, PLAYER, flickerEffect)
DEFINE_FIELD_X(BloodPlayer, PLAYER, tiltEffect)
DEFINE_FIELD_X(BloodPlayer, PLAYER, visibility)
DEFINE_FIELD_X(BloodPlayer, PLAYER, painEffect)
DEFINE_FIELD_X(BloodPlayer, PLAYER, blindEffect)
DEFINE_FIELD_X(BloodPlayer, PLAYER, chokeEffect)
DEFINE_FIELD_X(BloodPlayer, PLAYER, handTime)
DEFINE_FIELD_X(BloodPlayer, PLAYER, hand)  // if true, there is hand start choking the player
DEFINE_FIELD_X(BloodPlayer, PLAYER, pickupEffect)
DEFINE_FIELD_X(BloodPlayer, PLAYER, flashEffect)  // if true, reduce pPlayer->visibility counter
DEFINE_FIELD_X(BloodPlayer, PLAYER, quakeEffect)
DEFINE_FIELD_X(BloodPlayer, PLAYER, player_par)
DEFINE_FIELD_X(BloodPlayer, PLAYER, nWaterPal)

END_BLD_NS
