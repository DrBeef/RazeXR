//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT
This file is part of PCExhumed.
PCExhumed is free software; you can redistribute it and/or
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
#include "ns.h"
#include "engine.h"
#include "exhumed.h"
#include "aistuff.h"
#include "player.h"
#include "view.h"
#include "status.h"
#include "sound.h"
#include "mapinfo.h"
#include <string.h>
#include <assert.h>


BEGIN_PS_NS

int nPushBlocks;

// TODO - moveme?
sectortype* overridesect;

enum
{
    kMaxPushBlocks = 100,
    kMaxMoveChunks = 75
};


TObjPtr<DExhumedActor*> nBodySprite[50];
TObjPtr<DExhumedActor*> nChunkSprite[kMaxMoveChunks];
BlockInfo sBlockInfo[kMaxPushBlocks];
TObjPtr<DExhumedActor*> nBodyGunSprite[50];
int nCurBodyGunNum;

int sprceiling, sprfloor;
Collision loHit, hiHit;

// think this belongs in init.c?


size_t MarkMove()
{
    GC::MarkArray(nBodySprite, 50);
    GC::MarkArray(nChunkSprite, kMaxMoveChunks);
    for(int i = 0; i < nPushBlocks; i++)
        GC::Mark(sBlockInfo[i].pActor);
    return 50 + kMaxMoveChunks + nPushBlocks;
}

FSerializer& Serialize(FSerializer& arc, const char* keyname, BlockInfo& w, BlockInfo* def)
{
    if (arc.BeginObject(keyname))
    {
        arc("at8", w.field_8)
            ("sprite", w.pActor)
            ("x", w.x)
            ("y", w.y)
            .EndObject();
    }
    return arc;
}

void SerializeMove(FSerializer& arc)
{
    if (arc.BeginObject("move"))
    {
        arc ("pushcount", nPushBlocks)
            .Array("blocks", sBlockInfo, nPushBlocks)
            ("chunkcount", nCurChunkNum)
            .Array("chunks", nChunkSprite, kMaxMoveChunks)
            ("overridesect", overridesect)
            .Array("bodysprite", nBodySprite, countof(nBodySprite))
            ("curbodygun", nCurBodyGunNum)
            .Array("bodygunsprite", nBodyGunSprite, countof(nBodyGunSprite))
            .EndObject();
    }
}

signed int lsqrt(int a1)
{
    int v1;
    int v2;
    signed int result;

    v1 = a1;
    v2 = a1 - 0x40000000;

    result = 0;

    if (v2 >= 0)
    {
        result = 32768;
        v1 = v2;
    }
    if (v1 - ((result << 15) + 0x10000000) >= 0)
    {
        v1 -= (result << 15) + 0x10000000;
        result += 16384;
    }
    if (v1 - ((result << 14) + 0x4000000) >= 0)
    {
        v1 -= (result << 14) + 0x4000000;
        result += 8192;
    }
    if (v1 - ((result << 13) + 0x1000000) >= 0)
    {
        v1 -= (result << 13) + 0x1000000;
        result += 4096;
    }
    if (v1 - ((result << 12) + 0x400000) >= 0)
    {
        v1 -= (result << 12) + 0x400000;
        result += 2048;
    }
    if (v1 - ((result << 11) + 0x100000) >= 0)
    {
        v1 -= (result << 11) + 0x100000;
        result += 1024;
    }
    if (v1 - ((result << 10) + 0x40000) >= 0)
    {
        v1 -= (result << 10) + 0x40000;
        result += 512;
    }
    if (v1 - ((result << 9) + 0x10000) >= 0)
    {
        v1 -= (result << 9) + 0x10000;
        result += 256;
    }
    if (v1 - ((result << 8) + 0x4000) >= 0)
    {
        v1 -= (result << 8) + 0x4000;
        result += 128;
    }
    if (v1 - ((result << 7) + 4096) >= 0)
    {
        v1 -= (result << 7) + 4096;
        result += 64;
    }
    if (v1 - ((result << 6) + 1024) >= 0)
    {
        v1 -= (result << 6) + 1024;
        result += 32;
    }
    if (v1 - (32 * result + 256) >= 0)
    {
        v1 -= 32 * result + 256;
        result += 16;
    }
    if (v1 - (16 * result + 64) >= 0)
    {
        v1 -= 16 * result + 64;
        result += 8;
    }
    if (v1 - (8 * result + 16) >= 0)
    {
        v1 -= 8 * result + 16;
        result += 4;
    }
    if (v1 - (4 * result + 4) >= 0)
    {
        v1 -= 4 * result + 4;
        result += 2;
    }
    if (v1 - (2 * result + 1) >= 0)
        result += 1;

    return result;
}

void MoveThings()
{
    thinktime.Reset();
    thinktime.Clock();

    UndoFlashes();
    DoLights();

    if (nFreeze)
    {
        if (nFreeze == 1 || nFreeze == 2) {
            DoSpiritHead();
        }
    }
    else
    {
        actortime.Reset();
        actortime.Clock();
        runlist_ExecObjects();
        runlist_CleanRunRecs();
        actortime.Unclock();
    }

    DoBubbleMachines();
    DoDrips();
    DoMovingSects();
    DoRegenerates();

    if (currentLevel->gameflags & LEVEL_EX_COUNTDOWN)
    {
        DoFinale();
        if (lCountDown < 1800 && nDronePitch < 2400 && !lFinaleStart)
        {
            nDronePitch += 64;
            BendAmbientSound();
        }
    }

    thinktime.Unclock();
}

void ResetMoveFifo()
{
    movefifoend = 0;
    movefifopos = 0;
}

// not used
void clipwall()
{

}

int BelowNear(DExhumedActor* pActor, int x, int y, int walldist)
{
    auto pSector = pActor->sector();
    int z = pActor->spr.pos.Z;

    int z2;

    if (loHit.type == kHitSprite)
    {
        z2 = loHit.actor()->spr.pos.Z;
    }
    else
    {
        z2 = pSector->floorz + pSector->Depth;

        BFSSectorSearch search(pSector);

        sectortype* pTempSect = nullptr;
        while (auto pCurSector = search.GetNext())
        {
            for (auto& wal : wallsofsector(pCurSector))
            {
                if (wal.twoSided())
                {
                    if (!search.Check(wal.nextSector()))
                    {
                        vec2_t pos = { x, y };
                        if (clipinsidebox(&pos, wallnum(&wal), walldist))
                        {
                            search.Add(wal.nextSector());
                        }
                    }
                }
            }

            auto pSect2 = pCurSector;

            while (pSect2)
            {
                pTempSect = pSect2;
                pSect2 = pSect2->pBelow;
            }

            int ecx = pTempSect->floorz + pTempSect->Depth;
            int eax = ecx - z;

            if (eax < 0 && eax >= -5120)
            {
                z2 = ecx;
                pSector = pTempSect;
            }
        }
    }


    if (z2 < pActor->spr.pos.Z)
    {
        pActor->spr.pos.Z = z2;
        overridesect = pSector;
        pActor->spr.zvel = 0;

        bTouchFloor = true;

        return kHitAux2;
    }
    else
    {
        return 0;
    }
}

Collision movespritez(DExhumedActor* pActor, int z, int height, int, int clipdist)
{
    auto pSector = pActor->sector();
    assert(pSector);

    overridesect = pSector;
    auto pSect2 = pSector;

    // backup cstat
    auto cstat = pActor->spr.cstat;

    pActor->spr.cstat &= ~CSTAT_SPRITE_BLOCK;

    Collision nRet;
    nRet.setNone();

    int nSectFlags = pSector->Flag;

    if (nSectFlags & kSectUnderwater) {
        z >>= 1;
    }

    int spriteZ = pActor->spr.pos.Z;
    int floorZ = pSector->floorz;

    int ebp = spriteZ + z;
    int eax = pSector->ceilingz + (height >> 1);

    if ((nSectFlags & kSectUnderwater) && ebp < eax) {
        ebp = eax;
    }

    // loc_151E7:
    while (ebp > pActor->sector()->floorz && pActor->sector()->pBelow != nullptr)
    {
        ChangeActorSect(pActor, pActor->sector()->pBelow);
    }

    if (pSect2 != pSector)
    {
        pActor->spr.pos.Z = ebp;

        if (pSect2->Flag & kSectUnderwater)
        {
            if (pActor == PlayerList[nLocalPlayer].pActor) {
                D3PlayFX(StaticSound[kSound2], pActor);
            }

            if (pActor->spr.statnum <= 107) {
                pActor->spr.hitag = 0;
            }
        }
    }
    else
    {
        while ((ebp < pActor->sector()->ceilingz) && (pActor->sector()->pAbove != nullptr))
        {
            ChangeActorSect(pActor, pActor->sector()->pAbove);
        }
    }

    // This function will keep the player from falling off cliffs when you're too close to the edge.
    // This function finds the highest and lowest z coordinates that your clipping BOX can get to.
    vec3_t pos = pActor->spr.pos;
    pos.Z -= 256;
    getzrange(pos, pActor->sector(), &sprceiling, hiHit, &sprfloor, loHit, 128, CLIPMASK0);

    int mySprfloor = sprfloor;

    if (loHit.type != kHitSprite) {
        mySprfloor += pActor->sector()->Depth;
    }

    if (ebp > mySprfloor)
    {
        if (z > 0)
        {
            bTouchFloor = true;

            if (loHit.type == kHitSprite)
            {
                // Path A
                auto pFloorActor = loHit.actor();

                if (pActor->spr.statnum == 100 && pFloorActor->spr.statnum != 0 && pFloorActor->spr.statnum < 100)
                {
                    int nDamage = (z >> 9);
                    if (nDamage)
                    {
                        runlist_DamageEnemy(loHit.actor(), pActor, nDamage << 1);
                    }

                    pActor->spr.zvel = -z;
                }
                else
                {
                    if (pFloorActor->spr.statnum == 0 || pFloorActor->spr.statnum > 199)
                    {
                        nRet.exbits |= kHitAux2;
                    }
                    else
                    {
                        nRet = loHit;
                    }

                    pActor->spr.zvel = 0;
                }
            }
            else
            {
                // Path B
                if (pActor->sector()->pBelow == nullptr)
                {
                    nRet.exbits |= kHitAux2;

                    int nSectDamage = pActor->sector()->Damage;

                    if (nSectDamage != 0)
                    {
                        if (pActor->spr.hitag < 15)
                        {
                            IgniteSprite(pActor);
                            pActor->spr.hitag = 20;
                        }
                        nSectDamage >>= 2;
                        nSectDamage = nSectDamage - (nSectDamage>>2);
                        if (nSectDamage) {
                            runlist_DamageEnemy(pActor, nullptr, nSectDamage);
                        }
                    }

                    pActor->spr.zvel = 0;
                }
            }
        }

        // loc_1543B:
        ebp = mySprfloor;
        pActor->spr.pos.Z = mySprfloor;
    }
    else
    {
        if ((ebp - height) < sprceiling && (hiHit.type == kHitSprite || pActor->sector()->pAbove == nullptr))
        {
            ebp = sprceiling + height;
            nRet.exbits |= kHitAux1;
        }
    }

    if (spriteZ <= floorZ && ebp > floorZ)
    {
        if ((pSector->Depth != 0) || (pSect2 != pSector && (pSect2->Flag & kSectUnderwater)))
        {
            BuildSplash(pActor, pSector);
        }
    }

    pActor->spr.cstat = cstat; // restore cstat
    pActor->spr.pos.Z = ebp;

    if (pActor->spr.statnum == 100)
    {
        nRet.exbits |= BelowNear(pActor, pActor->spr.pos.X, pActor->spr.pos.Y, clipdist + (clipdist / 2));
    }

    return nRet;
}

int GetActorHeight(DExhumedActor* actor)
{
    return tileHeight(actor->spr.picnum) * actor->spr.yrepeat * 4;
}

DExhumedActor* insertActor(sectortype* s, int st)
{
    return static_cast<DExhumedActor*>(::InsertActor(RUNTIME_CLASS(DExhumedActor), s, st));
}


Collision movesprite(DExhumedActor* pActor, int dx, int dy, int dz, int ceildist, int flordist, unsigned int clipmask)
{
    bTouchFloor = false;

    int x = pActor->spr.pos.X;
    int y = pActor->spr.pos.Y;
    int z = pActor->spr.pos.Z;

    int nSpriteHeight = GetActorHeight(pActor);

    int nClipDist = (int8_t)pActor->spr.clipdist << 2;

    auto pSector = pActor->sector();
    assert(pSector);

    int floorZ = pSector->floorz;

    if ((pSector->Flag & kSectUnderwater) || (floorZ < z))
    {
        dx >>= 1;
        dy >>= 1;
    }

    Collision nRet = movespritez(pActor, dz, nSpriteHeight, flordist, nClipDist);

    pSector = pActor->sector(); // modified in movespritez so re-grab this variable

    if (pActor->spr.statnum == 100)
    {
        int nPlayer = GetPlayerFromActor(pActor);

        int varA = 0;
        int varB = 0;

        CheckSectorFloor(overridesect, pActor->spr.pos.Z, &varB, &varA);

        if (varB || varA)
        {
            PlayerList[nPlayer].nDamage.X = varB;
            PlayerList[nPlayer].nDamage.Y = varA;
        }

        dx += PlayerList[nPlayer].nDamage.X;
        dy += PlayerList[nPlayer].nDamage.Y;
    }
    else
    {
        CheckSectorFloor(overridesect, pActor->spr.pos.Z, &dx, &dy);
    }

    Collision coll;
    clipmove(pActor->spr.pos, &pSector, dx, dy, nClipDist, nSpriteHeight, flordist, clipmask, coll);
    if (coll.type != kHitNone) // originally this or'ed the two values which can create unpredictable bad values in some edge cases.
    {
        coll.exbits = nRet.exbits;
        nRet = coll;
    }

    if ((pSector != pActor->sector()) && pSector != nullptr)
    {
        if (nRet.exbits & kHitAux2) {
            dz = 0;
        }

        if ((pSector->floorz - z) < (dz + flordist))
        {
            pActor->spr.pos.X = x;
            pActor->spr.pos.Y = y;
        }
        else
        {
            ChangeActorSect(pActor, pSector);

            if (pActor->spr.pal < 5 && !pActor->spr.hitag)
            {
                pActor->spr.pal = pActor->sector()->ceilingpal;
            }
        }
    }

    return nRet;
}

void Gravity(DExhumedActor* pActor)
{
    if (pActor->sector()->Flag & kSectUnderwater)
    {
        if (pActor->spr.statnum != 100)
        {
            if (pActor->spr.zvel <= 1024)
            {
                if (pActor->spr.zvel < 2048) {
                    pActor->spr.zvel += 512;
                }
            }
            else
            {
                pActor->spr.zvel -= 64;
            }
        }
        else
        {
            if (pActor->spr.zvel > 0)
            {
                pActor->spr.zvel -= 64;
                if (pActor->spr.zvel < 0) {
                    pActor->spr.zvel = 0;
                }
            }
            else if (pActor->spr.zvel < 0)
            {
                pActor->spr.zvel += 64;
                if (pActor->spr.zvel > 0) {
                    pActor->spr.zvel = 0;
                }
            }
        }
    }
    else
    {
        pActor->spr.zvel += 512;
        if (pActor->spr.zvel > 16384) {
            pActor->spr.zvel = 16384;
        }
    }
}

Collision MoveCreature(DExhumedActor* pActor)
{
    return movesprite(pActor, pActor->spr.xvel << 8, pActor->spr.yvel << 8, pActor->spr.zvel, 15360, -5120, CLIPMASK0);
}

Collision MoveCreatureWithCaution(DExhumedActor* pActor)
{
    int x = pActor->spr.pos.X;
    int y = pActor->spr.pos.Y;
    int z = pActor->spr.pos.Z;
    auto pSectorPre = pActor->sector();

    auto ecx = MoveCreature(pActor);

    auto pSector =pActor->sector();

    if (pSector != pSectorPre)
    {
        int zDiff = pSectorPre->floorz - pSector->floorz;
        if (zDiff < 0) {
            zDiff = -zDiff;
        }

        if (zDiff > 15360 || (pSector->Flag & kSectUnderwater) || (pSector->pBelow != nullptr && pSector->pBelow->Flag) || pSector->Damage)
        {
            pActor->spr.pos.X = x;
            pActor->spr.pos.Y = y;
            pActor->spr.pos.Z = z;

            ChangeActorSect(pActor, pSectorPre);

            pActor->spr.ang = (pActor->spr.ang + 256) & kAngleMask;
            pActor->spr.xvel = bcos(pActor->spr.ang, -2);
            pActor->spr.yvel = bsin(pActor->spr.ang, -2);
            Collision c;
            c.setNone();
            return c;
        }
    }

    return ecx;
}

int GetAngleToSprite(DExhumedActor* a1, DExhumedActor* a2)
{
    if (!a1 || !a2)
        return -1;

    return GetMyAngle(a2->spr.pos.X - a1->spr.pos.X, a2->spr.pos.Y - a1->spr.pos.Y);
}

int PlotCourseToSprite(DExhumedActor* pActor1, DExhumedActor* pActor2)
{
    if (pActor1 == nullptr || pActor2 == nullptr)
        return -1;

    int x = pActor2->spr.pos.X - pActor1->spr.pos.X;
    int y = pActor2->spr.pos.Y - pActor1->spr.pos.Y;

    pActor1->spr.ang = GetMyAngle(x, y);

    uint32_t x2 = abs(x);
    uint32_t y2 = abs(y);

    uint32_t diff = x2 * x2 + y2 * y2;

    if (diff > INT_MAX)
    {
        DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
        diff = INT_MAX;
    }

    return ksqrt(diff);
}

DExhumedActor* FindPlayer(DExhumedActor* pActor, int nDistance, bool dontengage)
{
    int var_18 = !dontengage;

    if (nDistance < 0)
        nDistance = 100;

    int x = pActor->spr.pos.X;
    int y = pActor->spr.pos.Y;
    auto pSector =pActor->sector();

    int z = pActor->spr.pos.Z - GetActorHeight(pActor);

    nDistance <<= 8;

    DExhumedActor* pPlayerActor = nullptr;
    int i = 0;

    while (1)
    {
        if (i >= nTotalPlayers)
            return nullptr;

        pPlayerActor = PlayerList[i].pActor;

        if ((pPlayerActor->spr.cstat & CSTAT_SPRITE_BLOCK_ALL) && (!(pPlayerActor->spr.cstat & CSTAT_SPRITE_INVISIBLE)))
        {
            int v9 = abs(pPlayerActor->spr.pos.X - x);

            if (v9 < nDistance)
            {
                int v10 = abs(pPlayerActor->spr.pos.Y - y);

                if (v10 < nDistance && cansee(pPlayerActor->spr.pos.X, pPlayerActor->spr.pos.Y, pPlayerActor->spr.pos.Z - 7680, pPlayerActor->sector(), x, y, z, pSector))
                {
                    break;
                }
            }
        }

        i++;
    }

    if (var_18) {
        PlotCourseToSprite(pActor, pPlayerActor);
    }

    return pPlayerActor;
}

void CheckSectorFloor(sectortype* pSector, int z, int *x, int *y)
{
    int nSpeed = pSector->Speed;

    if (!nSpeed) {
        return;
    }

    int nFlag = pSector->Flag;
    int nAng = nFlag & kAngleMask;

    if (z >= pSector->floorz)
    {
        *x += bcos(nAng, 3) * nSpeed;
        *y += bsin(nAng, 3) * nSpeed;
    }
    else if (nFlag & 0x800)
    {
        *x += bcos(nAng, 4) * nSpeed;
        *y += bsin(nAng, 4) * nSpeed;
    }
}

int GetUpAngle(DExhumedActor* pActor1, int nVal, DExhumedActor* pActor2, int ecx)
{
    int x = pActor2->spr.pos.X - pActor1->spr.pos.X;
    int y = pActor2->spr.pos.Y - pActor1->spr.pos.Y;

    int ebx = (pActor2->spr.pos.Z + ecx) - (pActor1->spr.pos.Z + nVal);
    int edx = (pActor2->spr.pos.Z + ecx) - (pActor1->spr.pos.Z + nVal);

    ebx >>= 4;
    edx >>= 8;

    ebx = -ebx;

    ebx -= edx;

    int nSqrt = lsqrt(x * x + y * y);

    return GetMyAngle(nSqrt, ebx);
}

void InitPushBlocks()
{
    nPushBlocks = 0;
    memset(sBlockInfo, 0, sizeof(sBlockInfo));
}

int GrabPushBlock()
{
    if (nPushBlocks >= kMaxPushBlocks) {
        return -1;
    }

    return nPushBlocks++;
}

void CreatePushBlock(sectortype* pSector)
{
    int nBlock = GrabPushBlock();

    int xSum = 0;
    int ySum = 0;

    for (auto& wal : wallsofsector(pSector))
    {
        xSum += wal.wall_int_pos().X;
        ySum += wal.wall_int_pos().Y;
    }

    int xAvg = xSum / pSector->wallnum;
    int yAvg = ySum / pSector->wallnum;

    sBlockInfo[nBlock].x = xAvg;
    sBlockInfo[nBlock].y = yAvg;

    auto pActor = insertActor(pSector, 0);

    sBlockInfo[nBlock].pActor = pActor;

    pActor->spr.pos.X = xAvg;
    pActor->spr.pos.Y = yAvg;
    pActor->spr.pos.Z = pSector->floorz - 256;
    pActor->spr.cstat = CSTAT_SPRITE_INVISIBLE;

    int var_28 = 0;

	for (auto& wal : wallsofsector(pSector))
    {
        uint32_t xDiff = abs(xAvg - wal.wall_int_pos().X);
        uint32_t yDiff = abs(yAvg - wal.wall_int_pos().Y);

        uint32_t sqrtNum = xDiff * xDiff + yDiff * yDiff;

        if (sqrtNum > INT_MAX)
        {
            DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
            sqrtNum = INT_MAX;
        }

        int nSqrt = ksqrt(sqrtNum);
        if (nSqrt > var_28) {
            var_28 = nSqrt;
        }
    }

    sBlockInfo[nBlock].field_8 = var_28;

    pActor->spr.clipdist = (var_28 & 0xFF) << 2;
    pSector->extra = nBlock;
}

void MoveSector(sectortype* pSector, int nAngle, int *nXVel, int *nYVel)
{
    if (pSector == nullptr) {
        return;
    }

    int nXVect, nYVect;

    if (nAngle < 0)
    {
        nXVect = *nXVel;
        nYVect = *nYVel;
        nAngle = GetMyAngle(nXVect, nYVect);
    }
    else
    {
        nXVect = bcos(nAngle, 6);
        nYVect = bsin(nAngle, 6);
    }

    int nBlock = pSector->extra;
    int nSectFlag = pSector->Flag;

    int nFloorZ = pSector->floorz;

    walltype *pStartWall = pSector->firstWall();
    sectortype* pNextSector = pStartWall->nextSector();

    BlockInfo *pBlockInfo = &sBlockInfo[nBlock];

    vec3_t pos;

    pos.X = sBlockInfo[nBlock].x;
    int x_b = sBlockInfo[nBlock].x;

    pos.Y = sBlockInfo[nBlock].y;
    int y_b = sBlockInfo[nBlock].y;


    int nZVal;

    int bUnderwater = nSectFlag & kSectUnderwater;

    if (nSectFlag & kSectUnderwater)
    {
        nZVal = pSector->ceilingz;
        pos.Z = pNextSector->ceilingz + 256;

        pSector->setceilingz(pNextSector->ceilingz);
    }
    else
    {
        nZVal = pSector->floorz;
        pos.Z = pNextSector->floorz - 256;

        pSector->setfloorz(pNextSector->floorz);
    }

    auto pSectorB = pSector;
    Collision scratch;
    clipmove(pos, &pSectorB, nXVect, nYVect, pBlockInfo->field_8, 0, 0, CLIPMASK1, scratch);

    int yvect = pos.Y - y_b;
    int xvect = pos.X - x_b;

    if (pSectorB != pNextSector && pSectorB != pSector)
    {
        yvect = 0;
        xvect = 0;
    }
    else
    {
        if (!bUnderwater)
        {
            pos = { x_b, y_b, nZVal };

            clipmove(pos, &pSectorB, nXVect, nYVect, pBlockInfo->field_8, 0, 0, CLIPMASK1, scratch);

            int ebx = pos.X;
            int ecx = x_b;
            int edx = pos.Y;
            int eax = xvect;
            int esi = y_b;

            if (eax < 0) {
                eax = -eax;
            }

            ebx -= ecx;
            ecx = eax;
            eax = ebx;
            edx -= esi;

            if (eax < 0) {
                eax = -eax;
            }

            if (ecx > eax)
            {
                xvect = ebx;
            }

            eax = yvect;
            if (eax < 0) {
                eax = -eax;
            }

            ebx = eax;
            eax = edx;

            if (eax < 0) {
                eax = -eax;
            }

            if (ebx > eax) {
                yvect = edx;
            }
        }
    }

    // GREEN
    if (yvect || xvect)
    {
        ExhumedSectIterator it(pSector);
        while (auto pActor = it.Next())
        {
            if (pActor->spr.statnum < 99)
            {
                pActor->spr.pos.X += xvect;
                pActor->spr.pos.Y += yvect;
            }
            else
            {
                pos.Z = pActor->spr.pos.Z;

                if ((nSectFlag & kSectUnderwater) || pos.Z != nZVal || pActor->spr.cstat & CSTAT_SPRITE_INVISIBLE)
                {
                    pos.X = pActor->spr.pos.X;
                    pos.Y = pActor->spr.pos.Y;
                    pSectorB = pSector;

                    clipmove(pos, &pSectorB, -xvect, -yvect, 4 * pActor->spr.clipdist, 0, 0, CLIPMASK0, scratch);

                    if (pSectorB) {
                        ChangeActorSect(pActor, pSectorB);
                    }
                }
            }
        }
        it.Reset(pNextSector);
        while (auto pActor = it.Next())
        {
            if (pActor->spr.statnum >= 99)
            {
                pos = pActor->spr.pos;
                pSectorB = pNextSector;

                clipmove(pos, &pSectorB,
                    -xvect - (bcos(nAngle) * (4 * pActor->spr.clipdist)),
                    -yvect - (bsin(nAngle) * (4 * pActor->spr.clipdist)),
                    4 * pActor->spr.clipdist, 0, 0, CLIPMASK0, scratch);


                if (pSectorB != pNextSector && (pSectorB == pSector || pNextSector == pSector))
                {
                    if (pSectorB != pSector || nFloorZ >= pActor->spr.pos.Z)
                    {
                        if (pSectorB) {
                            ChangeActorSect(pActor, pSectorB);
                        }
                    }
                    else
                    {
                        movesprite(pActor,
                            (xvect << 14) + bcos(nAngle) * pActor->spr.clipdist,
                            (yvect << 14) + bsin(nAngle) * pActor->spr.clipdist,
                            0, 0, 0, CLIPMASK0);
                    }
                }
            }
        }

		for(auto& wal : wallsofsector(pSector))
        {
            dragpoint(&wal, xvect + wal.wall_int_pos().X, yvect + wal.wall_int_pos().Y);
        }

        pBlockInfo->x += xvect;
        pBlockInfo->y += yvect;
    }

    // loc_163DD
    xvect <<= 14;
    yvect <<= 14;

    if (!(nSectFlag & kSectUnderwater))
    {
        ExhumedSectIterator it(pSector);
        while (auto pActor = it.Next())
        {
            if (pActor->spr.statnum >= 99 && nZVal == pActor->spr.pos.Z && !(pActor->spr.cstat & CSTAT_SPRITE_INVISIBLE))
            {
                pSectorB = pSector;
                clipmove(pActor->spr.pos, &pSectorB, xvect, yvect, 4 * pActor->spr.clipdist, 5120, -5120, CLIPMASK0, scratch);
            }
        }
    }

    if (nSectFlag & kSectUnderwater) {
        pSector->setceilingz(nZVal);
    }
    else {
        pSector->setfloorz(nZVal);
    }

    *nXVel = xvect;
    *nYVel = yvect;

    /* 
        Update player position variables, in case the player sprite was moved by a sector,
        Otherwise these can be out of sync when used in sound code (before being updated in PlayerFunc()). 
        Can cause local player sounds to play off-centre.
        TODO: Might need to be done elsewhere too?
    */
    auto pActor = PlayerList[nLocalPlayer].pActor;
    initx = pActor->spr.pos.X;
    inity = pActor->spr.pos.Y;
    initz = pActor->spr.pos.Z;
    inita = pActor->spr.ang;
    initsectp = pActor->sector();
}

void SetQuake(DExhumedActor* pActor, int nVal)
{
    int x = pActor->spr.pos.X;
    int y = pActor->spr.pos.Y;

    nVal *= 256;

    for (int i = 0; i < nTotalPlayers; i++)
    {
        auto pPlayerActor = PlayerList[i].pActor;


        uint32_t xDiff = abs((int32_t)((pPlayerActor->spr.pos.X - x) >> 8));
        uint32_t yDiff = abs((int32_t)((pPlayerActor->spr.pos.Y - y) >> 8));

        uint32_t sqrtNum = xDiff * xDiff + yDiff * yDiff;

        if (sqrtNum > INT_MAX)
        {
            DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
            sqrtNum = INT_MAX;
        }

        int nSqrt = ksqrt(sqrtNum);

        int eax = nVal;

        if (nSqrt)
        {
            eax = eax / nSqrt;

            if (eax >= 256)
            {
                if (eax > 3840) {
                    eax = 3840;
                }
            }
            else
            {
                eax = 0;
            }
        }

        if (eax > nQuake[i]) {
            nQuake[i] = eax;
        }
    }
}

Collision AngleChase(DExhumedActor* pActor, DExhumedActor* pActor2, int ebx, int ecx, int push1) 
{
    int nClipType = pActor->spr.statnum != 107;

    /* bjd - need to handle cliptype to clipmask change that occured in later build engine version */
    if (nClipType == 1) {
        nClipType = CLIPMASK1;
    }
    else {
        nClipType = CLIPMASK0;
    }

    int nAngle;

    if (pActor2 == nullptr)
    {
        pActor->spr.zvel = 0;
        nAngle = pActor->spr.ang;
    }
    else
    {
        int nHeight = tileHeight(pActor2->spr.picnum) * pActor2->spr.yrepeat * 2;

        int nMyAngle = GetMyAngle(pActor2->spr.pos.X - pActor->spr.pos.X, pActor2->spr.pos.Y - pActor->spr.pos.Y);

        uint32_t xDiff = abs(pActor2->spr.pos.X - pActor->spr.pos.X);
        uint32_t yDiff = abs(pActor2->spr.pos.Y - pActor->spr.pos.Y);

        uint32_t sqrtNum = xDiff * xDiff + yDiff * yDiff;

        if (sqrtNum > INT_MAX)
        {
            DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
            sqrtNum = INT_MAX;
        }

        int nSqrt = ksqrt(sqrtNum);

        int var_18 = GetMyAngle(nSqrt, ((pActor2->spr.pos.Z - nHeight) - pActor->spr.pos.Z) >> 8);

        int nAngDelta = AngleDelta(pActor->spr.ang, nMyAngle, 1024);
        int nAngDelta2 = abs(nAngDelta);

        if (nAngDelta2 > 63)
        {
            nAngDelta2 = abs(nAngDelta >> 6);

            ebx /= nAngDelta2;

            if (ebx < 5) {
                ebx = 5;
            }
        }

        int nAngDeltaC = abs(nAngDelta);

        if (nAngDeltaC > push1)
        {
            if (nAngDelta >= 0)
                nAngDelta = push1;
            else
                nAngDelta = -push1;
        }

        nAngle = (nAngDelta + pActor->spr.ang) & kAngleMask;
        int nAngDeltaD = AngleDelta(pActor->spr.zvel, var_18, 24);

        pActor->spr.zvel = (pActor->spr.zvel + nAngDeltaD) & kAngleMask;
    }

    pActor->spr.ang = nAngle;

    int eax = abs(bcos(pActor->spr.zvel));

    int x = ((bcos(nAngle) * ebx) >> 14) * eax;
    int y = ((bsin(nAngle) * ebx) >> 14) * eax;

    int xshift = x >> 8;
    int yshift = y >> 8;

    uint32_t sqrtNum = xshift * xshift + yshift * yshift;

    if (sqrtNum > INT_MAX)
    {
        DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
        sqrtNum = INT_MAX;
    }

    int z = bsin(pActor->spr.zvel) * ksqrt(sqrtNum);

    return movesprite(pActor, x >> 2, y >> 2, (z >> 13) + bsin(ecx, -5), 0, 0, nClipType);
}

int GetWallNormal(walltype* pWall)
{
	auto delta = pWall->delta();

    int nAngle = GetMyAngle(delta.X, delta.Y);
    return (nAngle + 512) & kAngleMask;
}

void WheresMyMouth(int nPlayer, vec3_t* pos, sectortype **sectnum)
{
    auto pActor = PlayerList[nPlayer].pActor;
    int height = GetActorHeight(pActor) >> 1;

    *sectnum = pActor->sector();
    *pos = pActor->spr.pos;
    pos->Z -= height;

    Collision scratch;
    clipmove(*pos, sectnum,
        bcos(pActor->spr.ang, 7),
        bsin(pActor->spr.ang, 7),
        5120, 1280, 1280, CLIPMASK1, scratch);
}

void InitChunks()
{
    nCurChunkNum = 0;
    memset(nChunkSprite,   0, sizeof(nChunkSprite));
    memset(nBodyGunSprite, 0, sizeof(nBodyGunSprite));
    memset(nBodySprite,    0, sizeof(nBodySprite));
    nCurBodyNum    = 0;
    nCurBodyGunNum = 0;
    nBodyTotal  = 0;
    nChunkTotal = 0;
}

DExhumedActor* GrabBodyGunSprite()
{
    DExhumedActor* pActor = nBodyGunSprite[nCurBodyGunNum];
    if (pActor == nullptr)
    {
        pActor = insertActor(0, 899);
        nBodyGunSprite[nCurBodyGunNum] = pActor;

        pActor->spr.lotag = -1;
        pActor->spr.intowner = -1;
    }
    else
    {
        DestroyAnim(pActor);

        pActor->spr.lotag = -1;
        pActor->spr.intowner = -1;
    }

    nCurBodyGunNum++;
    if (nCurBodyGunNum >= 50) { // TODO - enum/define
        nCurBodyGunNum = 0;
    }

    pActor->spr.cstat = 0;

    return pActor;
}

DExhumedActor* GrabBody()
{
	DExhumedActor* pActor = nullptr;
    do
    {
        pActor = nBodySprite[nCurBodyNum];

        if (pActor == nullptr)
        {
            pActor = insertActor(0, 899);
            nBodySprite[nCurBodyNum] = pActor;
            pActor->spr.cstat = CSTAT_SPRITE_INVISIBLE;
        }


        nCurBodyNum++;
        if (nCurBodyNum >= 50) {
            nCurBodyNum = 0;
        }
    } while (pActor->spr.cstat & CSTAT_SPRITE_BLOCK_ALL);

    if (nBodyTotal < 50) {
        nBodyTotal++;
    }

    pActor->spr.cstat = 0;
    return pActor;
}

DExhumedActor* GrabChunkSprite()
{
    DExhumedActor* pActor = nChunkSprite[nCurChunkNum];

    if (pActor == nullptr)
    {
        pActor = insertActor(0, 899);
		nChunkSprite[nCurChunkNum] = pActor;
    }
    else if (pActor->spr.statnum)
    {
// TODO	MonoOut("too many chunks being used at once!\n");
        return nullptr;
    }

    ChangeActorStat(pActor, 899);

    nCurChunkNum++;
    if (nCurChunkNum >= kMaxMoveChunks)
        nCurChunkNum = 0;

    if (nChunkTotal < kMaxMoveChunks)
        nChunkTotal++;

    pActor->spr.cstat = CSTAT_SPRITE_YCENTER;

    return pActor;
}

DExhumedActor* BuildCreatureChunk(DExhumedActor* pSrc, int nPic, bool bSpecial)
{
    auto pActor = GrabChunkSprite();

    if (pActor == nullptr) {
        return nullptr;
    }
    pActor->spr.pos = pSrc->spr.pos;

    ChangeActorSect(pActor, pSrc->sector());

    pActor->spr.cstat = CSTAT_SPRITE_YCENTER;
    pActor->spr.shade = -12;
    pActor->spr.pal = 0;

    pActor->spr.xvel = (RandomSize(5) - 16) << 7;
    pActor->spr.yvel = (RandomSize(5) - 16) << 7;
    pActor->spr.zvel = (-(RandomSize(8) + 512)) << 3;

    if (bSpecial)
    {
        pActor->spr.xvel *= 4;
        pActor->spr.yvel *= 4;
        pActor->spr.zvel *= 2;
    }

    pActor->spr.xrepeat = 64;
    pActor->spr.yrepeat = 64;
    pActor->spr.xoffset = 0;
    pActor->spr.yoffset = 0;
    pActor->spr.picnum = nPic;
    pActor->spr.lotag = runlist_HeadRun() + 1;
    pActor->spr.clipdist = 40;

//	GrabTimeSlot(3);

    pActor->spr.extra = -1;
    pActor->spr.intowner = runlist_AddRunRec(pActor->spr.lotag - 1, pActor, 0xD0000);
    pActor->spr.hitag = runlist_AddRunRec(NewRun, pActor, 0xD0000);

    return pActor;
}

void AICreatureChunk::Tick(RunListEvent* ev)
{
    auto pActor = ev->pObjActor;
    if (!pActor) return;

    Gravity(pActor);

    auto pSector = pActor->sector();
    pActor->spr.pal = pSector->ceilingpal;

    auto nVal = movesprite(pActor, pActor->spr.xvel << 10, pActor->spr.yvel << 10, pActor->spr.zvel, 2560, -2560, CLIPMASK1);

    if (pActor->spr.pos.Z >= pSector->floorz)
    {
        // re-grab this variable as it may have changed in movesprite(). Note the check above is against the value *before* movesprite so don't change it.
        pSector = pActor->sector();

        pActor->spr.xvel = 0;
        pActor->spr.yvel = 0;
        pActor->spr.zvel = 0;
        pActor->spr.pos.Z = pSector->floorz;
    }
    else
    {
        if (!nVal.type && !nVal.exbits)
            return;

        int nAngle;

        if (nVal.exbits & kHitAux2)
        {
            pActor->spr.cstat = CSTAT_SPRITE_INVISIBLE;
        }
        else
        {
            if (nVal.exbits & kHitAux1)
            {
                pActor->spr.xvel >>= 1;
                pActor->spr.yvel >>= 1;
                pActor->spr.zvel = -pActor->spr.zvel;
                return;
            }
            else if (nVal.type == kHitSprite)
            {
                nAngle = nVal.actor()->spr.ang;
            }
            else if (nVal.type == kHitWall)
            {
                nAngle = GetWallNormal(nVal.hitWall);
            }
            else
            {
                return;
            }

            // loc_16E0C
            int nSqrt = lsqrt(((pActor->spr.yvel >> 10) * (pActor->spr.yvel >> 10)
                + (pActor->spr.xvel >> 10) * (pActor->spr.xvel >> 10)) >> 8);

            pActor->spr.xvel = bcos(nAngle) * (nSqrt >> 1);
            pActor->spr.yvel = bsin(nAngle) * (nSqrt >> 1);
            return;
        }
    }

    runlist_DoSubRunRec(pActor->spr.intowner);
    runlist_FreeRun(pActor->spr.lotag - 1);
    runlist_SubRunRec(pActor->spr.hitag);

    ChangeActorStat(pActor, 0);
    pActor->spr.hitag = 0;
    pActor->spr.lotag = 0;
}

DExhumedActor* UpdateEnemy(DExhumedActor** ppEnemy)
{
    if (*ppEnemy)
    {
        if (!((*ppEnemy)->spr.cstat & CSTAT_SPRITE_BLOCK_ALL)) {
            *ppEnemy = nullptr;
        }
    }

    return *ppEnemy;
}
END_PS_NS
