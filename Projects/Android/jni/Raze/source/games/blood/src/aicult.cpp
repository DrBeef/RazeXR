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

#include "build.h"

#include "blood.h"

BEGIN_BLD_NS

static void cultThinkSearch(DBloodActor*);
static void cultThinkGoto(DBloodActor*);
static void cultThinkChase(DBloodActor*);

AISTATE cultistIdle = { kAiStateIdle, 0, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE cultistProneIdle = { kAiStateIdle, 17, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE fanaticProneIdle = { kAiStateIdle, 17, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE cultistProneIdle3 = { kAiStateIdle, 17, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE cultistChase = { kAiStateChase, 9, -1, 0, NULL, aiMoveForward, cultThinkChase, NULL };
AISTATE fanaticChase = { kAiStateChase, 0, -1, 0, NULL, aiMoveTurn, cultThinkChase, NULL };
AISTATE cultistDodge = { kAiStateMove, 9, -1, 90, NULL, aiMoveDodge, NULL, &cultistChase };
AISTATE cultistGoto = { kAiStateMove, 9, -1, 600, NULL, aiMoveForward, cultThinkGoto, &cultistIdle };
AISTATE cultistProneChase = { kAiStateChase, 14, -1, 0, NULL, aiMoveForward, cultThinkChase, NULL };
AISTATE cultistProneDodge = { kAiStateMove, 14, -1, 90, NULL, aiMoveDodge, NULL, &cultistProneChase };
AISTATE cultistTThrow = { kAiStateChase, 7, nThrowClient, 120, NULL, NULL, NULL, &cultistTFire };
AISTATE cultistSThrow = { kAiStateChase, 7, nThrowClient, 120, NULL, NULL, NULL, &cultistSFire };
AISTATE cultistTsThrow = { kAiStateChase, 7, nThrowClient, 120, NULL, NULL, NULL, &cultistTsFire };
AISTATE cultistDThrow = { kAiStateChase, 7, nThrowClient, 120, NULL, NULL, NULL, &cultistChase };
AISTATE cultist139A78 = { kAiStateChase, 7, n68170Client, 120, NULL, NULL, NULL, &cultistChase };
AISTATE cultist139A94 = { kAiStateChase, 7, n68230Client, 120, NULL, NULL, NULL, &cultistIdle };
AISTATE cultist139AB0 = { kAiStateChase, 7, n68230Client, 120, NULL, NULL, cultThinkSearch, &cultist139A94 };
AISTATE cultist139ACC = { kAiStateChase, 7, n68230Client, 120, NULL, NULL, cultThinkSearch, &cultist139AB0 };
AISTATE cultist139AE8 = { kAiStateChase, 7, n68230Client, 120, NULL, NULL, cultThinkSearch, &cultist139AE8 };
AISTATE cultistSearch = { kAiStateSearch, 9, -1, 1800, NULL, aiMoveForward, cultThinkSearch, &cultistIdle };
AISTATE cultistSFire = { kAiStateChase, 6, nShotClient, 60, NULL, NULL, NULL, &cultistChase };
AISTATE cultistTFire = { kAiStateChase, 6, nTommyClient, 0, NULL, aiMoveTurn, cultThinkChase, &cultistTFire };
AISTATE cultistTsFire = { kAiStateChase, 6, nTeslaClient, 0, NULL, aiMoveTurn, cultThinkChase, &cultistChase };
AISTATE cultistSProneFire = { kAiStateChase, 8, nShotClient, 60, NULL, NULL, NULL, &cultistProneChase };
AISTATE cultistTProneFire = { kAiStateChase, 8, nTommyClient, 0, NULL, aiMoveTurn, cultThinkChase, &cultistTProneFire };
AISTATE cultistTsProneFire = { kAiStateChase, 8, nTeslaClient, 0, NULL, aiMoveTurn, NULL, &cultistTsProneFire }; // vanilla, broken
AISTATE cultistTsProneFireFixed = { kAiStateChase, 8, nTeslaClient, 0, NULL, aiMoveTurn, cultThinkChase, &cultistTsProneFireFixed };
AISTATE cultistRecoil = { kAiStateRecoil, 5, -1, 0, NULL, NULL, NULL, &cultistDodge };
AISTATE cultistProneRecoil = { kAiStateRecoil, 5, -1, 0, NULL, NULL, NULL, &cultistProneDodge };
AISTATE cultistTeslaRecoil = { kAiStateRecoil, 4, -1, 0, NULL, NULL, NULL, &cultistDodge };
AISTATE cultistSwimIdle = { kAiStateIdle, 13, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE cultistSwimChase = { kAiStateChase, 13, -1, 0, NULL, aiMoveForward, cultThinkChase, NULL };
AISTATE cultistSwimDodge = { kAiStateMove, 13, -1, 90, NULL, aiMoveDodge, NULL, &cultistSwimChase };
AISTATE cultistSwimGoto = { kAiStateMove, 13, -1, 600, NULL, aiMoveForward, cultThinkGoto, &cultistSwimIdle };
AISTATE cultistSwimSearch = { kAiStateSearch, 13, -1, 1800, NULL, aiMoveForward, cultThinkSearch, &cultistSwimIdle };
AISTATE cultistSSwimFire = { kAiStateChase, 8, nShotClient, 60, NULL, NULL, NULL, &cultistSwimChase };
AISTATE cultistTSwimFire = { kAiStateChase, 8, nTommyClient, 0, NULL, aiMoveTurn, cultThinkChase, &cultistTSwimFire };
AISTATE cultistTsSwimFire = { kAiStateChase, 8, nTeslaClient, 0, NULL, aiMoveTurn, cultThinkChase, &cultistTsSwimFire };
AISTATE cultistSwimRecoil = { kAiStateRecoil, 5, -1, 0, NULL, NULL, NULL, &cultistSwimDodge };

void TommySeqCallback(int, DBloodActor* actor)
{
	int dx = bcos(actor->spr.ang);
	int dy = bsin(actor->spr.ang);
	int dz = actor->dudeSlope;
	dx += Random3((5 - gGameOptions.nDifficulty) * 1000);
	dy += Random3((5 - gGameOptions.nDifficulty) * 1000);
	dz += Random3((5 - gGameOptions.nDifficulty) * 500);
	actFireVector(actor, 0, 0, dx, dy, dz, kVectorBullet);
	sfxPlay3DSound(actor, 4001, -1, 0);
}

void TeslaSeqCallback(int, DBloodActor* actor)
{
	if (Chance(gCultTeslaFireChance[gGameOptions.nDifficulty]))
	{
		int dx = bcos(actor->spr.ang);
		int dy = bsin(actor->spr.ang);
		int dz = actor->dudeSlope;
		dx += Random3((5 - gGameOptions.nDifficulty) * 1000);
		dy += Random3((5 - gGameOptions.nDifficulty) * 1000);
		dz += Random3((5 - gGameOptions.nDifficulty) * 500);
		actFireMissile(actor, 0, 0, dx, dy, dz, kMissileTeslaRegular);
		sfxPlay3DSound(actor, 470, -1, 0);
	}
}

void ShotSeqCallback(int, DBloodActor* actor)
{
	int dx = bcos(actor->spr.ang);
	int dy = bsin(actor->spr.ang);
	int dz = actor->dudeSlope;
	dx += Random2((5 - gGameOptions.nDifficulty) * 1000 - 500);
	dy += Random2((5 - gGameOptions.nDifficulty) * 1000 - 500);
	dz += Random2((5 - gGameOptions.nDifficulty) * 500);
	for (int i = 0; i < 8; i++)
	{
		int r1 = Random3(500);
		int r2 = Random3(1000);
		int r3 = Random3(1000);
		actFireVector(actor, 0, 0, dx + r3, dy + r2, dz + r1, kVectorShell);
	}
	if (Chance(0x8000))
		sfxPlay3DSound(actor, 1001, -1, 0);
	else
		sfxPlay3DSound(actor, 1002, -1, 0);
}

void cultThrowSeqCallback(int, DBloodActor* actor)
{
	int nMissile = kThingArmedTNTStick;
	if (gGameOptions.nDifficulty > 2)
		nMissile = kThingArmedTNTBundle;
	uint8_t v4 = Chance(0x6000);
	sfxPlay3DSound(actor, 455, -1, 0);
	if (!actor->ValidateTarget(__FUNCTION__)) return;
	auto target = actor->GetTarget();
	assert(actor->spr.type >= kDudeBase && actor->spr.type < kDudeMax);
	int dx = target->spr.pos.X - actor->spr.pos.X;
	int dy = target->spr.pos.Y - actor->spr.pos.Y;
	int dz = target->spr.pos.Z - actor->spr.pos.Z;
	int nDist = approxDist(dx, dy);
	int nDist2 = nDist / 540;
	if (nDist > 0x1e00)
		v4 = 0;
	auto* pMissile = actFireThing(actor, 0, 0, dz / 128 - 14500, nMissile, (nDist2 << 23) / 120);
	if (v4)
		pMissile->xspr.Impact = 1;
	else
		evPostActor(pMissile, 120 * (1 + Random(2)), kCmdOn);
}

void sub_68170(int, DBloodActor* actor)
{
	int nMissile = kThingArmedTNTStick;
	if (gGameOptions.nDifficulty > 2)
		nMissile = kThingArmedTNTBundle;
	sfxPlay3DSound(actor, 455, -1, 0);
	auto pMissile = actFireThing(actor, 0, 0, actor->dudeSlope - 9460, nMissile, 0x133333);
	evPostActor(pMissile, 120 * (2 + Random(2)), kCmdOn);
}

void sub_68230(int, DBloodActor* actor)
{
	int nMissile = kThingArmedTNTStick;
	if (gGameOptions.nDifficulty > 2)
		nMissile = kThingArmedTNTBundle;
	sfxPlay3DSound(actor, 455, -1, 0);
	if (!actor->ValidateTarget(__FUNCTION__)) return;
	auto target = actor->GetTarget();
	assert(actor->spr.type >= kDudeBase && actor->spr.type < kDudeMax);
	int dx = target->spr.pos.X - actor->spr.pos.X;
	int dy = target->spr.pos.Y - actor->spr.pos.Y;
	int dz = target->spr.pos.Z - actor->spr.pos.Z;
	int nDist = approxDist(dx, dy);
	int nDist2 = nDist / 540;
	auto pMissile = actFireThing(actor, 0, 0, dz / 128 - 14500, nMissile, (nDist2 << 17) / 120);
	pMissile->xspr.Impact = 1;
}

static bool TargetNearExplosion(sectortype* sector)
{
	BloodSectIterator it(sector);
	while (auto actor = it.Next())
	{
		if (actor->spr.type == kThingArmedTNTStick || actor->spr.statnum == kStatExplosion)
			return true;
	}
	return false;
}

static void cultThinkSearch(DBloodActor* actor)
{
	aiChooseDirection(actor, actor->xspr.goalAng);
	aiLookForTarget(actor);
}

static void cultThinkGoto(DBloodActor* actor)
{
	assert(actor->spr.type >= kDudeBase && actor->spr.type < kDudeMax);
	DUDEINFO* pDudeInfo = getDudeInfo(actor->spr.type);
	int dx = actor->xspr.TargetPos.X - actor->spr.pos.X;
	int dy = actor->xspr.TargetPos.Y - actor->spr.pos.Y;
	int nAngle = getangle(dx, dy);
	int nDist = approxDist(dx, dy);
	aiChooseDirection(actor, nAngle);
	if (nDist < 5120 && abs(actor->spr.ang - nAngle) < pDudeInfo->periphery)
	{
		switch (actor->xspr.medium)
		{
		case kMediumNormal:
			aiNewState(actor, &cultistSearch);
			break;
		case kMediumWater:
		case kMediumGoo:
			aiNewState(actor, &cultistSwimSearch);
			break;
		}
	}
	aiThinkTarget(actor);
}

static void cultThinkChase(DBloodActor* actor)
{
	if (actor->GetTarget() == nullptr)
	{
		switch (actor->xspr.medium)
		{
		case kMediumNormal:
			aiNewState(actor, &cultistGoto);
			break;
		case kMediumWater:
		case kMediumGoo:
			aiNewState(actor, &cultistSwimGoto);
			break;
		}
		return;
	}
	assert(actor->spr.type >= kDudeBase && actor->spr.type < kDudeMax);
	DUDEINFO* pDudeInfo = getDudeInfo(actor->spr.type);
	auto target = actor->GetTarget();

	int dx = target->spr.pos.X - actor->spr.pos.X;
	int dy = target->spr.pos.Y - actor->spr.pos.Y;
	aiChooseDirection(actor, getangle(dx, dy));
	if (target->xspr.health == 0)
	{
		switch (actor->xspr.medium)
		{
		case kMediumNormal:
			aiNewState(actor, &cultistSearch);
			if (actor->spr.type == kDudeCultistTommy)
				aiPlay3DSound(actor, 4021 + Random(4), AI_SFX_PRIORITY_1, -1);
			else
				aiPlay3DSound(actor, 1021 + Random(4), AI_SFX_PRIORITY_1, -1);
			break;
		case kMediumWater:
		case kMediumGoo:
			aiNewState(actor, &cultistSwimSearch);
			break;
		}
		return;
	}
	if (target->IsPlayerActor() && powerupCheck(&gPlayer[target->spr.type - kDudePlayer1], kPwUpShadowCloak) > 0)
	{
		switch (actor->xspr.medium)
		{
		case kMediumNormal:
			aiNewState(actor, &cultistSearch);
			break;
		case kMediumWater:
		case kMediumGoo:
			aiNewState(actor, &cultistSwimSearch);
			break;
		}
		return;
	}
	int nDist = approxDist(dx, dy);
	if (nDist > 0 && nDist <= pDudeInfo->seeDist)
	{
		int nDeltaAngle = ((getangle(dx, dy) + 1024 - actor->spr.ang) & 2047) - 1024;
		int height = (pDudeInfo->eyeHeight * actor->spr.yrepeat) << 2;
		if (cansee(target->spr.pos.X, target->spr.pos.Y, target->spr.pos.Z, target->sector(), actor->spr.pos.X, actor->spr.pos.Y, actor->spr.pos.Z - height, actor->sector()))
		{
			if (nDist < pDudeInfo->seeDist && abs(nDeltaAngle) <= pDudeInfo->periphery)
			{
				aiSetTarget(actor, actor->GetTarget());
				actor->dudeSlope = nDist == 0 ? 0 : DivScale(target->spr.pos.Z - actor->spr.pos.Z, nDist, 10);
				switch (actor->spr.type) {
				case kDudeCultistTommy:
					if (nDist < 0x1e00 && nDist > 0xe00 && abs(nDeltaAngle) < 85 && !TargetNearExplosion(target->sector())
						&& (target->spr.flags & 2) && gGameOptions.nDifficulty > 2 && target->IsPlayerActor() && gPlayer[target->spr.type - kDudePlayer1].isRunning
						&& Chance(0x8000))
					{
						int hit = HitScan(actor, actor->spr.pos.Z, dx, dy, 0, CLIPMASK1, 0);
						switch (hit)
						{
						case -1:
							if (actor->xspr.medium != kMediumWater && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultistTThrow);
							break;
						case 0:
						case 4:
							break;
						case 3:
							if (actor->spr.type != gHitInfo.actor()->spr.type && gHitInfo.actor()->spr.type != kDudeCultistShotgun && actor->xspr.medium != kMediumWater && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultistTThrow);
							break;
						default:
							aiNewState(actor, &cultistTThrow);
							break;
						}
					}
					else if (nDist < 0x4600 && abs(nDeltaAngle) < 28)
					{
						int hit = HitScan(actor, actor->spr.pos.Z, dx, dy, 0, CLIPMASK1, 0);
						switch (hit)
						{
						case -1:
							if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistTFire);
							else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistTProneFire);
							else if (dudeIsPlayingSeq(actor, 13) && (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo))
								aiNewState(actor, &cultistTSwimFire);
							break;
						case 3:
							if (actor->spr.type != gHitInfo.actor()->spr.type && gHitInfo.actor()->spr.type != kDudeCultistShotgun)
							{
								if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistTFire);
								else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistTProneFire);
								else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
									aiNewState(actor, &cultistTSwimFire);
							}
							else
							{
								if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistDodge);
								else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistProneDodge);
								else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
									aiNewState(actor, &cultistSwimDodge);
							}
							break;
						default:
							if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistTFire);
							else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistTProneFire);
							else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
								aiNewState(actor, &cultistTSwimFire);
							break;
						}
					}
					break;
				case kDudeCultistShotgun:
					if (nDist < 0x2c00 && nDist > 0x1400 && !TargetNearExplosion(target->sector())
						&& (target->spr.flags & 2) && gGameOptions.nDifficulty >= 2 && target->IsPlayerActor() && !gPlayer[target->spr.type - kDudePlayer1].isRunning
						&& Chance(0x8000))
					{
						int hit = HitScan(actor, actor->spr.pos.Z, dx, dy, 0, CLIPMASK1, 0);
						switch (hit)
						{
						case -1:
							if (actor->xspr.medium != kMediumWater && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultistSThrow);
							break;
						case 0:
						case 4:
							break;
						case 3:
							if (actor->spr.type != gHitInfo.actor()->spr.type && gHitInfo.actor()->spr.type != kDudeCultistShotgun && actor->xspr.medium != kMediumWater && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultistSThrow);
							break;
						default:
							aiNewState(actor, &cultistSThrow);
							break;
						}
					}
					else if (nDist < 0x3200 && abs(nDeltaAngle) < 28)
					{
						int hit = HitScan(actor, actor->spr.pos.Z, dx, dy, 0, CLIPMASK1, 0);
						switch (hit)
						{
						case -1:
							if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistSFire);
							else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistSProneFire);
							else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
								aiNewState(actor, &cultistSSwimFire);
							break;
						case 3:
							if (actor->spr.type != gHitInfo.actor()->spr.type && gHitInfo.actor()->spr.type != kDudeCultistTommy)
							{
								if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistSFire);
								else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistSProneFire);
								else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
									aiNewState(actor, &cultistSSwimFire);
							}
							else
							{
								if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistDodge);
								else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistProneDodge);
								else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
									aiNewState(actor, &cultistSwimDodge);
							}
							break;
						default:
							if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistSFire);
							else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistSProneFire);
							else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
								aiNewState(actor, &cultistSSwimFire);
							break;
						}
					}
					break;
				case kDudeCultistTesla:
					if (nDist < 0x1e00 && nDist > 0xe00 && !TargetNearExplosion(target->sector())
						&& (target->spr.flags & 2) && gGameOptions.nDifficulty > 2 && target->IsPlayerActor() && gPlayer[target->spr.type - kDudePlayer1].isRunning
						&& Chance(0x8000))
					{
						int hit = HitScan(actor, actor->spr.pos.Z, dx, dy, 0, CLIPMASK1, 0);
						switch (hit)
						{
						case -1:
							if (actor->xspr.medium != kMediumWater && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultistTsThrow);
							break;
						case 0:
						case 4:
							break;
						case 3:
							if (actor->spr.type != gHitInfo.actor()->spr.type && gHitInfo.actor()->spr.type != kDudeCultistShotgun && actor->xspr.medium != kMediumWater && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultistTsThrow);
							break;
						default:
							aiNewState(actor, &cultistTsThrow);
							break;
						}
					}
					else if (nDist < 0x3200 && abs(nDeltaAngle) < 28)
					{
						AISTATE *pCultistTsProneFire = !cl_bloodvanillaenemies && !VanillaMode() ? &cultistTsProneFireFixed : &cultistTsProneFire;
						int hit = HitScan(actor, actor->spr.pos.Z, dx, dy, 0, CLIPMASK1, 0);
						switch (hit)
						{
						case -1:
							if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistTsFire);
							else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, pCultistTsProneFire);
							else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
								aiNewState(actor, &cultistTsSwimFire);
							break;
						case 3:
							if (actor->spr.type != gHitInfo.actor()->spr.type && gHitInfo.actor()->spr.type != kDudeCultistTommy)
							{
								if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistTsFire);
								else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, pCultistTsProneFire);
								else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
									aiNewState(actor, &cultistTsSwimFire);
							}
							else
							{
								if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistDodge);
								else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistProneDodge);
								else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
									aiNewState(actor, &cultistSwimDodge);
							}
							break;
						default:
							if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistTsFire);
							else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, pCultistTsProneFire);
							else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
								aiNewState(actor, &cultistTsSwimFire);
							break;
						}
					}
					break;
				case kDudeCultistTNT:
					if (nDist < 0x2c00 && nDist > 0x1400 && abs(nDeltaAngle) < 85
						&& (target->spr.flags & 2) && target->IsPlayerActor())
					{
						int hit = HitScan(actor, actor->spr.pos.Z, dx, dy, 0, CLIPMASK1, 0);
						switch (hit)
						{
						case -1:
							if (actor->xspr.medium != kMediumWater && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultistDThrow);
							break;
						case 4:
							break;
						case 3:
							if (actor->spr.type != gHitInfo.actor()->spr.type && gHitInfo.actor()->spr.type != kDudeCultistShotgun && actor->xspr.medium != kMediumWater && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultistDThrow);
							break;
						default:
							aiNewState(actor, &cultistDThrow);
							break;
						}
					}
					else if (nDist < 0x1400 && abs(nDeltaAngle) < 85
						&& (target->spr.flags & 2) && target->IsPlayerActor())
					{
						int hit = HitScan(actor, actor->spr.pos.Z, dx, dy, 0, CLIPMASK1, 0);
						switch (hit)
						{
						case -1:
							if (actor->xspr.medium != 1 && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultist139A78);
							break;
						case 4:
							break;
						case 3:
							if (actor->spr.type != gHitInfo.actor()->spr.type && gHitInfo.actor()->spr.type != kDudeCultistShotgun && actor->xspr.medium != kMediumWater && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultist139A78);
							break;
						default:
							aiNewState(actor, &cultist139A78);
							break;
						}
					}
					break;
				case kDudeCultistBeast:
					if (nDist < 0x1e00 && nDist > 0xe00 && !TargetNearExplosion(target->sector())
						&& (target->spr.flags & 2) && gGameOptions.nDifficulty > 2 && target->IsPlayerActor() && gPlayer[target->spr.type - kDudePlayer1].isRunning
						&& Chance(0x8000))
					{
						int hit = HitScan(actor, actor->spr.pos.Z, dx, dy, 0, CLIPMASK1, 0);
						switch (hit)
						{
						case -1:
							if (actor->xspr.medium != kMediumWater && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultistSThrow);
							break;
						case 0:
						case 4:
							break;
						case 3:
							if (actor->spr.type != gHitInfo.actor()->spr.type && gHitInfo.actor()->spr.type != kDudeCultistShotgun && actor->xspr.medium != kMediumWater && actor->xspr.medium != kMediumGoo)
								aiNewState(actor, &cultistSThrow);
							break;
						default:
							aiNewState(actor, &cultistSThrow);
							break;
						}
					}
					else if (nDist < 0x3200 && abs(nDeltaAngle) < 28)
					{
						int hit = HitScan(actor, actor->spr.pos.Z, dx, dy, 0, CLIPMASK1, 0);
						switch (hit)
						{
						case -1:
							if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistSFire);
							else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistSProneFire);
							else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
								aiNewState(actor, &cultistSSwimFire);
							break;
						case 3:
							if (actor->spr.type != gHitInfo.actor()->spr.type && gHitInfo.actor()->spr.type != kDudeCultistTommy)
							{
								if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistSFire);
								else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistSProneFire);
								else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
									aiNewState(actor, &cultistSSwimFire);
							}
							else
							{
								if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistDodge);
								else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
									aiNewState(actor, &cultistProneDodge);
								else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
									aiNewState(actor, &cultistSwimDodge);
							}
							break;
						default:
							if (!dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistSFire);
							else if (dudeIsPlayingSeq(actor, 14) && actor->xspr.medium == kMediumNormal)
								aiNewState(actor, &cultistSProneFire);
							else if (actor->xspr.medium == kMediumWater || actor->xspr.medium == kMediumGoo)
								aiNewState(actor, &cultistSSwimFire);
							break;
						}
					}
					break;
				}
				return;
			}
		}
	}
	switch (actor->xspr.medium)
	{
	case kMediumNormal:
		aiNewState(actor, &cultistGoto);
		break;
	case kMediumWater:
	case kMediumGoo:
		aiNewState(actor, &cultistSwimGoto);
		break;
	}
	actor->SetTarget(nullptr);
}

END_BLD_NS
