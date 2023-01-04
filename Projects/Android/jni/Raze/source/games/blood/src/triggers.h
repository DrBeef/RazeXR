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
#include "build.h"
#include "common_game.h"

#include "db.h"
#include "eventq.h"
#include "dude.h"
#include "player.h"

BEGIN_BLD_NS

enum BUSYID {
	BUSYID_0 = 0,
	BUSYID_1,
	BUSYID_2,
	BUSYID_3,
	BUSYID_4,
	BUSYID_5,
	BUSYID_6,
	BUSYID_7,
};

struct BUSY {
	sectortype* sect;
	int delta;
	int busy;
	int/*BUSYID*/ type;
};

extern TArray<BUSY> gBusy;

void trTriggerSector(sectortype* pSector, int command);
void trMessageSector(sectortype* pSector, EVENT event);
void trTriggerWall(walltype*, int command);
void trMessageWall(walltype* pWall, EVENT& event);
void trTriggerSprite(DBloodActor* actor, int command);
void trMessageSprite(DBloodActor* actor, EVENT event);
void trProcessBusy(void);
void trInit(TArray<DBloodActor*>& actors);
void trTextOver(int nId);
bool SetSpriteState(DBloodActor* actor, int nState);
bool SetWallState(walltype* pWall, int nState);
bool SetSectorState(sectortype* pSector, int nState);
void TeleFrag(DBloodActor* killer, sectortype* pSector);
void SectorStartSound(sectortype* pSector, int nState);
void SectorEndSound(sectortype* pSector, int nState);

END_BLD_NS
