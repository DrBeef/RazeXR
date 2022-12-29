//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) NoOne

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


////////////////////////////////////////////////////////////////////////////////////
// This file provides modern features for mappers.
// For full documentation please visit http://cruo.bloodgame.ru/xxsystem
/////////////////////////////////////////////////////////////////////////


#pragma once
#include "common_game.h"
#ifdef NOONE_EXTENSIONS
#include "eventq.h"
#include "qav.h"
#include "actor.h"
#include "dude.h"
#include "player.h"

BEGIN_BLD_NS

enum
{
    // CONSTANTS
    // additional non-thing proximity, sight and physics sprites 
    kMaxSuperXSprites = 512,
    kMaxTrackingConditions = 64,
    kMaxTracedObjects = 32, // per one tracking condition

    // additional physics attributes for debris sprites
    kPhysDebrisFloat = 0x0008, // *debris* slowly goes up and down from it's position
    kPhysDebrisFly = 0x0010, // *debris* affected by negative gravity (fly instead of falling)
    kPhysDebrisSwim = 0x0020, // *debris* can swim underwater (instead of drowning)
    kPhysDebrisTouch = 0x0040, // *debris* can be moved via touch
    kPhysDebrisVector = 0x0400, // *debris* can be affected by vector weapons
    kPhysDebrisExplode = 0x0800, // *debris* can be affected by explosions

    // *modern types only hitag*
    kModernTypeFlag0 = 0x0000,
    kModernTypeFlag1 = 0x0001,
    kModernTypeFlag2 = 0x0002,
    kModernTypeFlag3 = 0x0003,
    kModernTypeFlag4 = 0x0004,
    kModernTypeFlag8 = 0x0008,
    kModernTypeFlag16 = 0x0010,

    kMaxRandomizeRetries = 16,
    kPercFull = 100,
    kCondRange = 100,
};

enum
{
    kPatrolStateSize = 42,
    kPatrolAlarmSeeDist = 10000,
    kPatrolAlarmHearDist = 10000,
    kMaxPatrolVelocity = 500000,
    kMaxPatrolCrouchVelocity = (kMaxPatrolVelocity >> 1),
    kMaxPatrolSpotValue = 500,
    kMinPatrolTurnDelay = 8,
    kPatrolTurnDelayRange = 20,

    kDudeFlagStealth    = 0x0001,
    kDudeFlagCrouch     = 0x0002,

    kSlopeDist = 0x20,
    kEffectGenCallbackBase = 200,
    kTriggerSpriteScreen = 0x0001,
    kTriggerSpriteAim    = 0x0002,

    kMinAllowedPowerup = kPwUpFeatherFall,
    kMaxAllowedPowerup = kMaxPowerUps
};

// modern statnums
enum {
kStatModernBase                     = 20,
kStatModernDudeTargetChanger        = kStatModernBase,
kStatModernCondition                = 21,
kStatModernEventRedirector          = 22,
kStatModernPlayerLinker             = 23,
kStatModernBrokenDudeLeech          = 24,
kStatModernQavScene                 = 25,
kStatModernWindGen                  = 26,
kStatModernStealthRegion            = 27,
kStatModernTmp                      = 39,
kStatModernMax                      = 40,
};

// modern sprite types
enum {
kModernStealthRegion                = 16,
kModernCustomDudeSpawn              = 24,
kModernRandomTX                     = 25,
kModernSequentialTX                 = 26,
kModernSeqSpawner                   = 27,
kModernObjPropertiesChanger         = 28,
kModernObjPicnumChanger             = 29,
kModernObjSizeChanger               = 31,
kModernDudeTargetChanger            = 33,
kModernSectorFXChanger              = 34,
kModernObjDataChanger               = 35,
kModernSpriteDamager                = 36,
kModernObjDataAccumulator           = 37,
kModernEffectSpawner                = 38,
kModernWindGenerator                = 39,
kModernRandom                       = 40,
kModernRandom2                      = 80,
kItemShroomGrow                     = 129,
kItemShroomShrink                   = 130,
kItemModernMapLevel                 = 150,  // once picked up, draws whole minimap
kDudeModernCustom                   = kDudeVanillaMax,
kDudeModernCustomBurning            = 255,
kModernThingTNTProx                 = 433, // detects only players
kModernThingThrowableRock           = 434, // does small damage if hits target
kModernThingEnemyLifeLeech          = 435, // the same as normal, except it aims in specified target only
kModernPlayerControl                = 500, /// WIP
kModernCondition                    = 501, /// WIP, sends command only if specified conditions == true
kModernConditionFalse               = 502, /// WIP, sends command only if specified conditions != true
kModernSlopeChanger                 = 504,
kGenModernMissileUniversal          = 704,
kGenModernSound                     = 708,
};

// type of random
enum {
kRandomizeItem                      = 0,
kRandomizeDude                      = 1,
kRandomizeTX                        = 2,
};

// type of object
enum {
OBJ_WALL                            = 0,
OBJ_SPRITE                          = 3,
OBJ_SECTOR                          = 6,
};

enum {
kCondGameBase                       = 0,
kCondGameMax                        = 50,
kCondMixedBase                      = 100,
kCondMixedMax                       = 200,
kCondWallBase                       = 200,
kCondWallMax                        = 300,
kCondSectorBase                     = 300,
kCondSectorMax                      = 400,
kCondPlayerBase                     = 400,
kCondPlayerMax                      = 450,
kCondDudeBase                       = 450,
kCondDudeMax                        = 500,
kCondSpriteBase                     = 500,
kCondSpriteMax                      = 600,
};

enum {
kCondSerialSector                   = 100000,
kCondSerialWall                     = 200000,
kCondSerialSprite                   = 300000,
kCondSerialMax                      = 400000,
};

enum {
kPatrolMoveForward                  = 0,
kPatrolMoveBackward                 = 1,
};

// - STRUCTS ------------------------------------------------------------------
struct SPRITEMASS { // sprite mass info for getSpriteMassBySize();
    int seqId;
    int16_t picnum; // mainly needs for moving debris
    int16_t xrepeat;
    int16_t yrepeat;
    int16_t clipdist; // mass multiplier
    int mass;
    int16_t airVel; // mainly needs for moving debris
    int fraction; // mainly needs for moving debris
};

struct QAVSCENE { // this one stores qavs anims that can be played by trigger
    DBloodActor* initiator = nullptr;  // index of sprite which triggered qav scene
    QAV* qavResrc = nullptr;
    short dummy = -1;
};

struct THINGINFO_EXTRA {
    bool allowThrow; // indicates if kDudeModernCustom can throw it
};

struct VECTORINFO_EXTRA {
    int fireSound[2]; // predefined fire sounds. used by kDudeModernCustom, but can be used for something else.
};

struct MISSILEINFO_EXTRA {
    int fireSound[2]; // predefined fire sounds. used by kDudeModernCustom, but can be used for something else.
    bool dmgType[kDamageMax]; // list of damages types missile can use
    bool allowImpact; // allow to trigger object with Impact flag enabled with this missile
};

struct DUDEINFO_EXTRA {
    bool flying;    // used by kModernDudeTargetChanger (ai fight)
    bool melee;     // used by kModernDudeTargetChanger (ai fight)
    int idlgseqofs : 6;             // used for patrol
    int mvegseqofs : 6;             // used for patrol
    int idlwseqofs : 6;             // used for patrol
    int mvewseqofs : 6;             // used for patrol
    int idlcseqofs : 6;             // used for patrol
    int mvecseqofs : 6;             // used for patrol
    
};

struct TRPLAYERCTRL { // this one for controlling the player using triggers (movement speed, jumps and other stuff)
    QAVSCENE qavScene;
};

struct OBJECTS_TO_TRACK {
    int8_t type;
    uint8_t cmd;
    unsigned int index_;
    DBloodActor* actor;
};

struct TRCONDITION {
    DBloodActor* actor;
    uint8_t length;
    OBJECTS_TO_TRACK obj[kMaxTracedObjects];
};

struct PATROL_FOUND_SOUNDS {

    int snd;
    int max;
    int cur;

};

struct CONDITION_TYPE_NAMES {

    int rng1;
    int rng2;
    char name[32];

};

// - VARIABLES ------------------------------------------------------------------
extern bool gModernMap;
extern bool gTeamsSpawnUsed;
extern bool gEventRedirectsUsed;
extern ZONE gStartZoneTeam1[kMaxPlayers];
extern ZONE gStartZoneTeam2[kMaxPlayers];
extern const THINGINFO_EXTRA gThingInfoExtra[kThingMax];
extern const VECTORINFO_EXTRA gVectorInfoExtra[kVectorMax];
extern const MISSILEINFO_EXTRA gMissileInfoExtra[kMissileMax];
extern const DUDEINFO_EXTRA gDudeInfoExtra[kDudeMax];
extern TRPLAYERCTRL gPlayerCtrl[kMaxPlayers];
extern TRCONDITION gCondition[kMaxTrackingConditions];
extern DBloodActor* gProxySpritesList[kMaxSuperXSprites];
extern DBloodActor* gSightSpritesList[kMaxSuperXSprites];
extern DBloodActor* gPhysSpritesList[kMaxSuperXSprites];
extern DBloodActor* gImpactSpritesList[kMaxSuperXSprites];
extern int gProxySpritesCount;
extern int gSightSpritesCount;
extern int gPhysSpritesCount;
extern int gImpactSpritesCount;
extern int gTrackingCondsCount;
extern AISTATE genPatrolStates[kPatrolStateSize];


// - FUNCTIONS ------------------------------------------------------------------
bool nnExtEraseModernStuff(DBloodActor* actor);
void nnExtInitModernStuff();
void nnExtProcessSuperSprites(void);
bool nnExtIsImmune(DBloodActor* pSprite, int dmgType, int minScale = 16);
int nnExtRandom(int a, int b);
void nnExtResetGlobals();
//  -------------------------------------------------------------------------   //
void sfxPlayMissileSound(DBloodActor* pSprite, int missileId);
void sfxPlayVectorSound(DBloodActor* pSprite, int vectorId);
//  -------------------------------------------------------------------------   //
int debrisGetFreeIndex(void);
void debrisBubble(DBloodActor* nSprite);
void debrisMove(int listIndex);
void debrisConcuss(DBloodActor* nOwner, int listIndex, int x, int y, int z, int dmg);
//  -------------------------------------------------------------------------   //
void aiSetGenIdleState(DBloodActor*);

// triggers related
//  -------------------------------------------------------------------------   //
int aiFightGetTargetDist(DBloodActor* pSprite, DUDEINFO* pDudeInfo, DBloodActor* pTarget);
int aiFightGetFineTargetDist(DBloodActor* actor, DBloodActor* target);
bool aiFightDudeCanSeeTarget(DBloodActor* pXDude, DUDEINFO* pDudeInfo, DBloodActor* pTarget);
bool aiFightDudeIsAffected(DBloodActor* pXDude);
bool aiFightMatesHaveSameTarget(DBloodActor* leaderactor, DBloodActor* targetactor, int allow);
void aiFightActivateDudes(int rx);
//  -------------------------------------------------------------------------   //
void useSlopeChanger(DBloodActor* sourceactor, int objType, int objIndex, DBloodActor* objActor);
void damageSprites(DBloodActor* pXSource, DBloodActor* pSprite);
void useRandomItemGen(DBloodActor* pSource);
void useUniMissileGen(DBloodActor* sourceactor, DBloodActor* actor);
void useSoundGen(DBloodActor* sourceactor, DBloodActor* actor);
void useIncDecGen(DBloodActor* sourceactor, short objType, int objIndex, DBloodActor* objactor);
void useDataChanger(DBloodActor* sourceactor, int objType, int objIndex, DBloodActor* objActor);
void useSectorLigthChanger(DBloodActor* pXSource, XSECTOR* pXSector);
void useTargetChanger(DBloodActor* sourceactor, DBloodActor* actor);
void usePictureChanger(DBloodActor* sourceactor, int objType, int objIndex, DBloodActor* objActor);
void useSequentialTx(DBloodActor* pXSource, COMMAND_ID cmd, bool setState);
void useRandomTx(DBloodActor* sourceactor, COMMAND_ID cmd, bool setState);
void useDudeSpawn(DBloodActor* pXSource, DBloodActor* pSprite);
void useCustomDudeSpawn(DBloodActor* pXSource, DBloodActor* pSprite);
void seqTxSendCmdAll(DBloodActor* pXSource, DBloodActor* nIndex, COMMAND_ID cmd, bool modernSend);
//  -------------------------------------------------------------------------   //
void trPlayerCtrlLink(DBloodActor* pXSource, PLAYER* pPlayer, bool checkCondition);
void trPlayerCtrlStopScene(PLAYER* pPlayer);
//  -------------------------------------------------------------------------   //
void modernTypeTrigger(int type, int nDest, DBloodActor* actor, const EVENT& event);
bool modernTypeOperateSector(int nSector, sectortype* pSector, XSECTOR* pXSector, const EVENT& event);
bool modernTypeOperateSprite(DBloodActor*, EVENT event);
bool modernTypeOperateWall(int nWall, walltype* pWall, XWALL* pXWall, EVENT event);
void modernTypeSendCommand(DBloodActor* nSprite, int channel, COMMAND_ID command);
//  -------------------------------------------------------------------------   //
bool playerSizeShrink(PLAYER* pPlayer, int divider);
bool playerSizeGrow(PLAYER* pPlayer, int multiplier);
bool playerSizeReset(PLAYER* pPlayer);
void playerDeactivateShrooms(PLAYER* pPlayer);
//  -------------------------------------------------------------------------   //
QAV* playerQavSceneLoad(int qavId);
void playerQavSceneProcess(PLAYER* pPlayer, QAVSCENE* pQavScene);
void playerQavScenePlay(PLAYER* pPlayer);
void playerQavSceneDraw(PLAYER* pPlayer, int a2, double a3, double a4, int a5);
void playerQavSceneReset(PLAYER* pPlayer);
//  -------------------------------------------------------------------------   //
void callbackUniMissileBurst(DBloodActor*actor, int nSprite);
void callbackMakeMissileBlocking(DBloodActor* actor, int nSprite);
void callbackGenDudeUpdate(DBloodActor* actor, int nSprite);
//  -------------------------------------------------------------------------   //
PLAYER* getPlayerById(int id);
bool isGrown(DBloodActor* pSprite);
bool isShrinked(DBloodActor* pSprite);
bool IsBurningDude(DBloodActor* pSprite);
bool IsKillableDude(DBloodActor* pSprite);
bool isActive(DBloodActor* nSprite);
int getDataFieldOfObject(int objType, int objIndex, DBloodActor* objActor, int dataIndex);
bool setDataValueOfObject(int objType, int objIndex, DBloodActor* objActor, int dataIndex, int value);
bool incDecGoalValueIsReached(DBloodActor* actor);
int getSpriteMassBySize(DBloodActor* pSprite);
bool ceilIsTooLow(DBloodActor* pSprite);
void levelEndLevelCustom(int nLevel);
int useCondition(DBloodActor*, const EVENT& event);
bool condCmp(int val, int arg1, int arg2, int comOp);
bool condCmpne(int arg1, int arg2, int comOp);
void condError(DBloodActor* pXCond, const char* pzFormat, ...);
void condUpdateObjectIndex(DBloodActor* oldplayer, DBloodActor* newplayer);
DBloodActor* evrListRedirectors(int objType, int objXIndex, DBloodActor* objActor, DBloodActor* pXRedir, int* tx);
void seqSpawnerOffSameTx(DBloodActor* actor);
//  -------------------------------------------------------------------------   //
void aiPatrolSetMarker(DBloodActor* actor);
void aiPatrolThink(DBloodActor* actor);
void aiPatrolStop(DBloodActor* actor, DBloodActor* targetactor, bool alarm = false);
void aiPatrolAlarmFull(DBloodActor* actor, DBloodActor* targetactor, bool chain);
void aiPatrolAlarmLite(DBloodActor* actor, DBloodActor* targetactor);
void aiPatrolState(DBloodActor* pSprite, int state);
void aiPatrolMove(DBloodActor* actor);
DBloodActor* aiPatrolMarkerBusy(DBloodActor* except, DBloodActor* marker);
bool aiPatrolMarkerReached(DBloodActor*);
bool aiPatrolGetPathDir(DBloodActor* actor, DBloodActor* marker);
void aiPatrolFlagsMgr(DBloodActor* sourceactor, DBloodActor* destactor, bool copy, bool init);
void aiPatrolRandGoalAng(DBloodActor* actor);
void aiPatrolTurn(DBloodActor* actor);
inline int aiPatrolGetVelocity(int speed, int value) {
    return (value > 0) ? ClipRange((speed / 3) + (2500 * value), 0, 0x47956) : speed;
}

inline bool aiPatrolWaiting(AISTATE* pAiState) {
    return (pAiState && pAiState->stateType >= kAiStatePatrolWaitL && pAiState->stateType <= kAiStatePatrolWaitW);
}

inline bool aiPatrolMoving(AISTATE* pAiState) {
    return (pAiState && pAiState->stateType >= kAiStatePatrolMoveL && pAiState->stateType <= kAiStatePatrolMoveW);
}

inline bool aiPatrolTurning(AISTATE* pAiState) {
    return (pAiState && pAiState->stateType >= kAiStatePatrolTurnL && pAiState->stateType <= kAiStatePatrolTurnW);
}

inline bool aiInPatrolState(AISTATE* pAiState) {
    return (pAiState && pAiState->stateType >= kAiStatePatrolBase && pAiState->stateType < kAiStatePatrolMax);
}

inline bool aiInPatrolState(int nAiStateType) {
    return (nAiStateType >= kAiStatePatrolBase && nAiStateType < kAiStatePatrolMax);
}
//  -------------------------------------------------------------------------   //
bool readyForCrit(DBloodActor* pHunter, DBloodActor* pVictim);
int sectorInMotion(int nSector);
void clampSprite(DBloodActor* actor, int which = 3);
#endif

inline bool valueIsBetween(int val, int min, int max)
{
    return (val > min && val < max);
}


////////////////////////////////////////////////////////////////////////
// This file provides modern features for mappers.
// For full documentation please visit http://cruo.bloodgame.ru/xxsystem
////////////////////////////////////////////////////////////////////////////////////
END_BLD_NS

