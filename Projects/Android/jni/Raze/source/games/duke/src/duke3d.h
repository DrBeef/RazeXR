#pragma once

#include "build.h"

#include "compat.h"


#include "polymost.h"
#include "gamecvars.h"
#include "razemenu.h"
#include "gamecontrol.h"
#include "gamevar.h"
#include "global.h"
#include "funct.h"
#include "names.h"
#include "quotemgr.h"
#include "rts.h"
#include "sounds.h"
#include "soundefs.h"
#include "gamestruct.h"
#include "v_draw.h"
#include "gamefuncs.h"

BEGIN_DUKE_NS

struct GameInterface : public ::GameInterface
{
	const char* Name() override { return "Duke"; }
	void app_init() override;
	void loadPalette();
	void clearlocalinputstate() override;
	bool GenerateSavePic() override;
	void PlayHudSound() override;
	GameStats getStats() override;
	void MenuOpened() override;
	void MenuSound(EMenuSounds snd) override;
	bool CanSave() override;
	bool StartGame(FNewGameStartup& gs) override;
	FSavegameInfo GetSaveSig() override;
	double SmallFontScale() override { return isRR() ? 0.5 : 1.; }
	void SerializeGameState(FSerializer& arc) override;
	FString GetCoordString() override;
	void ExitFromMenu() override;
	ReservedSpace GetReservedScreenSpace(int viewsize) override;
	void DrawPlayerSprite(const DVector2& origin, bool onteam) override;
	void GetInput(ControlInfo* const hidInput, double const scaleAdjust, InputPacket* packet = nullptr) override;
	void UpdateSounds() override;
	void Startup() override;
	void DrawBackground() override;
	void Render() override;
	void Ticker() override;
	const char* GenericCheat(int player, int cheat) override;
	const char* CheckCheatMode() override;
	void NextLevel(MapRecord* map, int skill) override;
	void NewGame(MapRecord* map, int skill, bool) override;
	void LevelCompleted(MapRecord* map, int skill) override;
	bool DrawAutomapPlayer(int mx, int my, int x, int y, int z, int a, double const smoothratio) override;
	int playerKeyMove() override { return 40; }
	void WarpToCoords(int x, int y, int z, int a, int h) override;
	void ToggleThirdPerson() override;
	void SwitchCoopView() override;
	void ToggleShowWeapon() override;
	int chaseCamX(binangle ang) { return -ang.bcos(-4); }
	int chaseCamY(binangle ang) { return -ang.bsin(-4); }
	int chaseCamZ(fixedhoriz horiz) { return horiz.asq16() >> 9; }
	void processSprites(spritetype* tsprite, int& spritesortcnt, int viewx, int viewy, int viewz, binangle viewang, double smoothRatio) override;
	void UpdateCameras(double smoothratio) override;
	void EnterPortal(spritetype* viewer, int type) override;
	void LeavePortal(spritetype* viewer, int type) override;
	bool GetGeoEffect(GeoEffect* eff, sectortype* viewsector) override;
	void AddExcludedEpisode(const FString& episode) override;
	int GetCurrentSkill() override;

};

struct Dispatcher
{
	// sectors_?.cpp
	void (*think)();
	void (*initactorflags)();
	bool (*isadoorwall)(int dapic);
	void (*animatewalls)();
	void (*operaterespawns)(int low);
	void (*operateforcefields)(DDukeActor* act, int low);
	bool (*checkhitswitch)(int snum, walltype* w, DDukeActor* act);
	void (*activatebysector)(sectortype* sect, DDukeActor* j);
	void (*checkhitwall)(DDukeActor* spr, walltype* dawall, int x, int y, int z, int atwith);
	bool (*checkhitceiling)(sectortype* sn);
	void (*checkhitsprite)(DDukeActor* i, DDukeActor* sn);
	void (*checksectors)(int low);
	DDukeActor* (*spawninit)(DDukeActor* actj, DDukeActor* act);

	bool (*ceilingspace)(sectortype* sectp);
	bool (*floorspace)(sectortype* sectp);
	void (*addweapon)(struct player_struct *p, int weapon);
	void (*hitradius)(DDukeActor* i, int  r, int  hp1, int  hp2, int  hp3, int  hp4);
	void (*lotsofmoney)(DDukeActor *s, int n);
	void (*lotsofmail)(DDukeActor *s, int n);
	void (*lotsofpaper)(DDukeActor *s, int n);
	void (*guts)(DDukeActor* s, int gtype, int n, int p);
	DDukeActor* (*ifhitsectors)(int sectnum);
	int  (*ifhitbyweapon)(DDukeActor* sectnum);
	void (*fall)(DDukeActor* actor, int g_p);
	bool (*spawnweapondebris)(int picnum, int dnum);
	void (*respawnhitag)(DDukeActor* g_sp);
	void (*checktimetosleep)(DDukeActor* actor);
	void (*move)(DDukeActor* i, int g_p, int g_x);

	// player
	void (*incur_damage)(struct player_struct* p);
	void (*shoot)(DDukeActor*, int);
	void (*selectweapon)(int snum, int j);
	int (*doincrements)(struct player_struct* p);
	void (*checkweapons)(struct player_struct* p);
	void (*processinput)(int snum);
	void (*displayweapon)(int snum, double smoothratio);
	void (*displaymasks)(int snum, int p, double smoothratio);

	void (*animatesprites)(spritetype* tsprite, int& spritesortcnt, int x, int y, int a, int smoothratio);


};

extern Dispatcher fi;

END_DUKE_NS

