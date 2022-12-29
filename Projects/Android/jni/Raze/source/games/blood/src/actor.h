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

BEGIN_BLD_NS

class DBloodActor;

enum DAMAGE_TYPE {
    kDamageFall = 0,
    kDamageBurn,
    kDamageBullet,
    kDamageExplode,
    kDamageDrown,
    kDamageSpirit,
    kDamageTesla,
    kDamageMax = 7,
};

enum VECTOR_TYPE {
    kVectorTine = 0,
    kVectorShell,
    kVectorBullet,
    kVectorTommyAP,
    kVectorShellAP,
    kVectorTommyRegular,
    kVectorBatBite,
    kVectorBoneelBite,
    kVectorGillBite,
    kVectorBeastSlash,
    kVectorAxe,
    kVectorCleaver,
    kVectorGhost,
    kVectorGargSlash,
    kVectorCerberusHack,
    kVectorHoundBite,
    kVectorRatBite,
    kVectorSpiderBite,
    VECTOR_TYPE_18,
    VECTOR_TYPE_19,
    kVectorTchernobogBurn,
    kVectorVoodoo10,
    #ifdef NOONE_EXTENSIONS
    kVectorGenDudePunch,
    #endif
    kVectorMax,
};

struct THINGINFO
{
    int16_t startHealth;
    int16_t mass;
    uint8_t clipdist;
    int16_t flags;
    int32_t elastic; // elasticity
    int32_t dmgResist; // damage resistance
    int16_t cstat;
    int16_t picnum;
    int8_t shade;
    uint8_t pal;
    uint8_t xrepeat; // xrepeat
    uint8_t yrepeat; // yrepeat
    int dmgControl[kDamageMax]; // damage
};

struct AMMOITEMDATA
{
    int16_t cstat;
    int16_t picnum;
    int8_t shade;
    uint8_t pal;
    uint8_t xrepeat;
    uint8_t yrepeat;
    int16_t count;
    uint8_t type;
    uint8_t weaponType;
};

struct WEAPONITEMDATA
{
    int16_t cstat;
    int16_t picnum;
    int8_t shade;
    uint8_t pal;
    uint8_t xrepeat;
    uint8_t yrepeat;
    int16_t type;
    int16_t ammoType;
    int16_t count;
};

struct ITEMDATA
{
    int16_t cstat;
    int16_t picnum;
    int8_t shade;
    uint8_t pal;
    uint8_t xrepeat;
    uint8_t yrepeat;
    int16_t packSlot;
};

struct MissileType
{
    int16_t picnum;
    int velocity;
    int angleOfs;
    uint8_t xrepeat;
    uint8_t yrepeat;
    int8_t shade;
    uint8_t clipDist;
};

struct EXPLOSION
{
    uint8_t repeat;
    uint8_t dmg;
    uint8_t dmgRng;
    int radius;
    int dmgType;
    int burnTime;
    int ticks;
    int quakeEffect;
    int flashEffect;
};

struct SURFHIT {
    FX_ID fx1;
    FX_ID fx2;
    FX_ID fx3;
    int fxSnd;
};

struct VECTORDATA {
    DAMAGE_TYPE dmgType;
    int dmg; // damage
    int impulse;
    int maxDist;
    int fxChance;
    int burnTime; // burn
    int bloodSplats; // blood splats
    int splatChance; // blood splat chance
    SURFHIT surfHit[15];
};

extern const AMMOITEMDATA gAmmoItemData[];
extern const WEAPONITEMDATA gWeaponItemData[];
extern const ITEMDATA gItemData[];
extern const MissileType missileInfo[];
extern const EXPLOSION explodeInfo[];
extern const THINGINFO thingInfo[];
extern VECTORDATA gVectorData[];

const int gDudeDrag = 0x2a00;

template<typename T> bool IsPlayerSprite(T const * const pSprite)
{
    return pSprite->type >= kDudePlayer1 && pSprite->type <= kDudePlayer8;
}

template<typename T> bool IsDudeSprite(T const * const pSprite)
{
    return pSprite->type >= kDudeBase && pSprite->type < kDudeMax;
}

template<typename T> bool IsItemSprite(T const * const pSprite)
{
    return pSprite->type >= kItemBase && pSprite->type < kItemMax;
}

template<typename T> bool IsWeaponSprite(T const * const pSprite)
{
    return pSprite->type >= kItemWeaponBase && pSprite->type < kItemWeaponMax;
}

template<typename T> bool IsAmmoSprite(T const * const pSprite)
{
    return pSprite->type >= kItemAmmoBase && pSprite->type < kItemAmmoMax;
}


#ifdef POLYMER
void actAddGameLight(int lightRadius, int spriteNum, int zOffset, int lightRange, int lightColor, int lightPrio);
void actDoLight(int spriteNum);
#endif

void FireballSeqCallback(int, int);
void sub_38938(int, int);
void NapalmSeqCallback(int, int);
void sub_3888C(int, int);
void TreeToGibCallback(int, int);

bool IsUnderwaterSector(int nSector);
void actInit();
int actWallBounceVector(int *x, int *y, int nWall, int a4);
int actFloorBounceVector(int *x, int *y, int *z, int nSector, int a5);
void actRadiusDamage(DBloodActor* source, int x, int y, int z, int nSector, int nDist, int a7, int a8, DAMAGE_TYPE a9, int a10, int a11);
DBloodActor *actDropObject(DBloodActor *pSprite, int nType);
bool actHealDude(DBloodActor* pXDude, int a2, int a3);
void actKillDude(DBloodActor* a1, DBloodActor* pSprite, DAMAGE_TYPE a3, int a4);
int actDamageSprite(DBloodActor* pSource, DBloodActor* pTarget, DAMAGE_TYPE damageType, int damage);
void actHitcodeToData(int a1, HITINFO *pHitInfo, DBloodActor **actor, walltype **a7 = nullptr);
void actAirDrag(DBloodActor *pSprite, int a2);
void actExplodeSprite(DBloodActor *pSprite);
void actActivateGibObject(DBloodActor *actor);
void actProcessSprites(void);
DBloodActor* actSpawnSprite(int nSector, int x, int y, int z, int nStat, bool a6);
DBloodActor* actSpawnDude(DBloodActor* pSource, int nType, int a3, int a4);
DBloodActor * actSpawnSprite(DBloodActor *pSource, int nStat);
DBloodActor * actSpawnThing(int nSector, int x, int y, int z, int nThingType);
DBloodActor* actFireThing(DBloodActor* pSprite, int a2, int a3, int a4, int thingType, int a6);
DBloodActor* actFireMissile(DBloodActor *pSprite, int a2, int a3, int a4, int a5, int a6, int nType);

void actBurnSprite(DBloodActor* pSource, DBloodActor* pTarget, int nTime);

int actGetRespawnTime(DBloodActor *pSprite);
bool actCheckRespawn(DBloodActor *pSprite);
bool actCanSplatWall(int nWall);
void actFireVector(DBloodActor *pShooter, int a2, int a3, int a4, int a5, int a6, VECTOR_TYPE vectorType);
void actPostSprite(DBloodActor* actor, int status);
void actPostProcess(void);
void MakeSplash(DBloodActor *actor);
void actBuildMissile(DBloodActor* spawned, DBloodActor* actor);

extern const int DudeDifficulty[];

END_BLD_NS
