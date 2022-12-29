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
#include "aistuff.h"
#include "engine.h"
#include "exhumed.h"
#include "sequence.h"
#include "sound.h"
#include <assert.h>

BEGIN_PS_NS

static actionSeq ScorpSeq[] = {
    {0, 0},
    {8, 0},
    {29, 0},
    {19, 0},
    {45, 1},
    {46, 1},
    {47, 1},
    {48, 1},
    {50, 1},
    {53, 1}
};

void BuildScorp(DExhumedActor* pActor, int x, int y, int z, int nSector, int nAngle, int nChannel)
{
    spritetype* pSprite;

    if (pActor == nullptr)
    {
        pActor = insertActor(nSector, 122);
        pSprite = &pActor->s();
    }
    else
    {
        ChangeActorStat(pActor, 122);

		pSprite = &pActor->s();
        x = pSprite->x;
        y = pSprite->y;
        z = pSprite->sector()->floorz;
        nAngle = pSprite->ang;
    }

	pSprite->x = x;
    pSprite->y = y;
    pSprite->z = z;
    pSprite->cstat = 0x101;
    pSprite->clipdist = 70;
    pSprite->shade = -12;
    pSprite->xrepeat = 80;
    pSprite->yrepeat = 80;
    pSprite->picnum = 1;
    pSprite->pal = pSprite->sector()->ceilingpal;
    pSprite->xoffset = 0;
    pSprite->yoffset = 0;
    pSprite->ang = nAngle;
    pSprite->xvel = 0;
    pSprite->yvel = 0;
    pSprite->zvel = 0;
    pSprite->lotag = runlist_HeadRun() + 1;
    pSprite->extra = -1;
    pSprite->hitag = 0;

    //	GrabTimeSlot(3);

    pActor->nHealth = 20000;
    pActor->nFrame = 0;
    pActor->nAction = 0;
    pActor->pTarget = nullptr;
    pActor->nCount = 0;
    pActor->nIndex2 = 1;
	pActor->nPhase = Counters[kCountScorp]++;

    pActor->nChannel = nChannel;

    pSprite->owner = runlist_AddRunRec(pSprite->lotag - 1, pActor, 0x220000);
    pActor->nRun = runlist_AddRunRec(NewRun, pActor, 0x220000);

    nCreaturesTotal++;
}

void AIScorp::Draw(RunListEvent* ev)
{
	auto pActor = ev->pObjActor;
	if (!pActor) return;

    short nAction = pActor->nAction;

    seq_PlotSequence(ev->nParam, SeqOffsets[kSeqScorp] + ScorpSeq[nAction].a, pActor->nFrame, ScorpSeq[nAction].b);
}

void AIScorp::RadialDamage(RunListEvent* ev)
{
	auto pActor = ev->pObjActor;
	if (!pActor) return;

    ev->nDamage = runlist_CheckRadialDamage(pActor);
    if (ev->nDamage) Damage(ev);
}


void AIScorp::Damage(RunListEvent* ev)
{
	auto pActor = ev->pObjActor;
	if (!pActor) return;

    auto pSprite = &pActor->s();


    DExhumedActor* pTarget = nullptr;

    if (pActor->nHealth <= 0) {
        return;
    }

    pActor->nHealth -= dmgAdjust(ev->nDamage);

    if (pActor->nHealth <= 0)
    {
        pActor->nHealth = 0;
        pActor->nAction = 4;
        pActor->nFrame = 0;
        pActor->nCount = 10;

        pSprite->xvel = 0;
        pSprite->yvel = 0;
        pSprite->zvel = 0;
        pSprite->cstat &= 0xFEFE;

        nCreaturesKilled++;
        return;
    }
    else
    {
        pTarget = ev->pOtherActor;

        if (pTarget)
        {
            if (pSprite->statnum == 100 || (pSprite->statnum < 199 && !RandomSize(5)))
            {
                pActor->pTarget = pTarget;
            }
        }

        if (!RandomSize(5))
        {
            pActor->nAction = RandomSize(2) + 4;
            pActor->nFrame = 0;
            return;
        }

        if (RandomSize(2)) {
            return;
        }

        D3PlayFX(StaticSound[kSound41], pActor);
        Effect(ev, pTarget, 0);
    }
}

void AIScorp::Tick(RunListEvent* ev)
{
	auto pActor = ev->pObjActor;
	if (!pActor) return;


    short nAction = pActor->nAction;
    auto pSprite = &pActor->s();

    bool bVal = false;

    DExhumedActor* pTarget = nullptr;

    if (pActor->nHealth) {
        Gravity(pActor);
    }

    int nSeq = SeqOffsets[kSeqScorp] + ScorpSeq[nAction].a;

    pSprite->picnum = seq_GetSeqPicnum2(nSeq, pActor->nFrame);
    seq_MoveSequence(pActor, nSeq, pActor->nFrame);

    pActor->nFrame++;

    if (pActor->nFrame >= SeqSize[nSeq])
    {
        pActor->nFrame = 0;
        bVal = true;
    }

    int nFlag = FrameFlag[SeqBase[nSeq] + pActor->nFrame];
    pTarget = pActor->pTarget;

    switch (nAction)
    {
    default:
        return;

    case 0:
    {
        if (pActor->nCount > 0)
        {
            pActor->nCount--;
            return;
        }

        if ((pActor->nPhase & 0x1F) == (totalmoves & 0x1F))
        {
            if (pTarget == nullptr)
            {
                pTarget = FindPlayer(pActor, 500);

                if (pTarget)
                {
                    D3PlayFX(StaticSound[kSound41], pActor);

                    pActor->nFrame = 0;
                    pSprite->xvel = bcos(pSprite->ang);
                    pSprite->yvel = bsin(pSprite->ang);

                    pActor->nAction = 1;
                    pActor->pTarget = pTarget;
                }
            }
        }

        return;
    }

    case 1:
    {
        pActor->nIndex2--;

        if (pActor->nIndex2 <= 0)
        {
            pActor->nIndex2 = RandomSize(5);
            Effect(ev, pTarget, 0);
        }
        else
        {
            auto nMov = MoveCreatureWithCaution(pActor);
            if (nMov.type == kHitSprite)
            {
                if (pTarget == nMov.actor)
                {
                    int nAngle = getangle(pTarget->s().x - pSprite->x, pTarget->s().y - pSprite->y);
                    if (AngleDiff(pSprite->ang, nAngle) < 64)
                    {
                        pActor->nAction = 2;
                        pActor->nFrame = 0;
                    }
                    Effect(ev, pTarget, 2);
                }
                else
                {
                    Effect(ev, pTarget, 0);
                }
                return;
            }
            else if (nMov.type == kHitWall)
            {
                Effect(ev, pTarget, 0);
            }
            else
            {
                Effect(ev, pTarget, 1);
            }
        }
        return;
    }

    case 2:
    {
        if (pTarget == nullptr)
        {
            pActor->nAction = 0;
            pActor->nCount = 5;
        }
        else
        {
            if (PlotCourseToSprite(pActor, pTarget) >= 768)
            {
                pActor->nAction = 1;
            }
            else if (nFlag & 0x80)
            {
                runlist_DamageEnemy(pTarget, pActor, 7);
            }
        }
        Effect(ev, pTarget, 2);
        return;
    }

    case 3:
    {
        if (bVal)
        {
            pActor->nIndex--;
            if (pActor->nIndex <= 0)
            {
                pActor->nAction = 1;

                pSprite->xvel = bcos(pSprite->ang);
                pSprite->yvel = bsin(pSprite->ang);

                pActor->nFrame = 0;
                return;
            }
        }

        if (!(nFlag & 0x80)) {
            return;
        }

        auto nBulletSprite = BuildBullet(pActor, 16, -1, pSprite->ang, pTarget, 1);
        if (nBulletSprite)
        {
            PlotCourseToSprite(nBulletSprite, pTarget);
        }

        return;
    }

    case 4:
    case 5:
    case 6:
    case 7:
    {
        if (!bVal) {
            return;
        }

        if (pActor->nHealth > 0)
        {
            pActor->nAction = 1;
            pActor->nFrame = 0;
            pActor->nCount = 0;
            return;
        }

        pActor->nCount--;
        if (pActor->nCount <= 0)
        {
            pActor->nAction = 8;
        }
        else
        {
            pActor->nAction = RandomBit() + 6;
        }

        return;
    }

    case 8:
    {
        if (bVal)
        {
            pActor->nAction++; // set to 9
            pActor->nFrame = 0;

            runlist_ChangeChannel(pActor->nChannel, 1);
            return;
        }

        auto pSpiderActor = BuildSpider(nullptr, pSprite->x, pSprite->y, pSprite->z, pSprite->sectnum, pSprite->ang);
        if (pSpiderActor)
        {
			auto pSpiderSprite = &pSpiderActor->s();
            pSpiderSprite->ang = RandomSize(11);

            int nVel = RandomSize(5) + 1;

            pSpiderSprite->xvel = bcos(pSpiderSprite->ang, -8) * nVel;
            pSpiderSprite->yvel = bsin(pSpiderSprite->ang, -8) * nVel;
            pSpiderSprite->zvel = (-(RandomSize(5) + 3)) << 8;
        }

        return;
    }

    case 9:
    {
        pSprite->cstat &= 0xFEFE;

        if (bVal)
        {
            runlist_SubRunRec(pActor->nRun);
            runlist_DoSubRunRec(pSprite->owner);
            runlist_FreeRun(pSprite->lotag - 1);

            DeleteActor(pActor);
        }

        return;
    }
    }
}

void AIScorp::Effect(RunListEvent* ev, DExhumedActor* pTarget, int mode)
{
	auto pActor = ev->pObjActor;
	if (!pActor) return;

    short nAction = pActor->nAction;
    auto pSprite = &pActor->s();

    if (mode == 0)
    {
        PlotCourseToSprite(pActor, pTarget);
        pSprite->ang += RandomSize(7) - 63;
        pSprite->ang &= kAngleMask;

        pSprite->xvel = bcos(pSprite->ang);
        pSprite->yvel = bsin(pSprite->ang);
    }
    if (mode <= 1)
    {
        if (pActor->nCount)
        {
            pActor->nCount--;
        }
        else
        {
            pActor->nCount = 45;

            if (cansee(pSprite->x, pSprite->y, pSprite->z - GetActorHeight(pActor), pSprite->sectnum,
                pTarget->s().x, pTarget->s().y, pTarget->s().z - GetActorHeight(pTarget), pTarget->s().sectnum))
            {
                pSprite->xvel = 0;
                pSprite->yvel = 0;
                pSprite->ang = GetMyAngle(pTarget->s().x - pSprite->x, pTarget->s().y - pSprite->y);

                pActor->nIndex = RandomSize(2) + RandomSize(3);

                if (!pActor->nIndex) {
                    pActor->nCount = RandomSize(5);
                }
                else
                {
                    pActor->nAction = 3;
                    pActor->nFrame = 0;
                }
            }
        }
    }

    if (!nAction || pTarget == nullptr) {
        return;
    }

    if (!(pTarget->s().cstat & 0x101))
    {
        pActor->nAction = 0;
        pActor->nFrame = 0;
        pActor->nCount = 30;
        pActor->pTarget = nullptr;

        pSprite->xvel = 0;
        pSprite->yvel = 0;
    }
}


END_PS_NS
