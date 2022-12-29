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

#pragma once 

#include "compat.h"
#include "build.h"

BEGIN_PS_NS

enum
{
	kStatIgnited = 404,
	kMaxSprites = 4096,
	kMaxSectors = 1024,
	kMaxWalls   = 8192,
	kMaxVoxels	= 4096,
	kMaxPalookups = 256,
	kMaxStatus   = 1024,
	kMap20	= 20,
	kAngleMask	= 0x7FF
};


Collision movesprite(DExhumedActor* spritenum, int dx, int dy, int dz, int ceildist, int flordist, unsigned int clipmask);

void precache();
void resettiming();

// cd

bool playCDtrack(int nTrack, bool bLoop);
int StepFadeCDaudio();
bool CDplaying();
void StopCD();

// init

enum {
    kSectUnderwater = 0x2000,
    kSectLava = 0x4000,
};

extern int initx;
extern int inity;
extern int initz;
extern short inita;
extern int initsect;

extern short nCurChunkNum;
extern DExhumedActor* nBodyGunSprite[50];
extern int movefifoend;
extern int movefifopos;
extern short nCurBodyGunNum;

// all static counters combined in an array for easier maintenance.
enum ECounter
{
	kCountAnubis,
	kCountAnubisDrum,
	kCountLava,
	kCountLion,
	kCountMummy,
	kCountRex,
	kCountRoach,
	kCountScorp,
	kCountSet,
	kCountSoul,
	kCountSpider,
	kCountWasp,

	kNumCounters
};
extern int Counters[kNumCounters];

void SnapSectors(int nSectorA, int nSectorB, int b);

extern short SectSound[];
extern short SectDamage[];
extern short SectSpeed[];
extern int SectBelow[];
extern short SectFlag[];
extern int SectDepth[];
extern int SectSoundSect[];
extern int SectAbove[];

void LoadObjects();

// light

int LoadPaletteLookups();
void SetGreenPal();
void RestoreGreenPal();
void FixPalette();
int HavePLURemap();
uint8_t RemapPLU(uint8_t pal);

extern short overscanindex;

extern char *origpalookup[];

extern short nPalDiff;

// map

extern short bShowTowers;

void GrabMap();
void UpdateMap();
void DrawMap(double const smoothratio);

// random

void InitRandom();
int RandomBit();
uint8_t RandomByte();
uint16_t RandomWord();
int RandomLong();
int RandomSize(int nSize);

// record

// save

// trigdat


int GetMyAngle(int x, int y);

int AngleDiff(short a, short b);
int AngleDelta(int a, int b, int c);

END_PS_NS

