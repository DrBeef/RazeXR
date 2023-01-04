//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment
Copyright (C) 2000, 2003 - Matt Saettler (EDuke Enhancements)
Copyright (C) 2020 - Christoph Oelckers

This file is part of Enhanced Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms

EDuke enhancements integrated: 04/13/2003 - Matt Saettler

Note: EDuke source was in transition.  Changes are in-progress in the
source as it is released.

*/
//-------------------------------------------------------------------------
#include "ns.h"
#include "concmd.h"
#include "duke3d.h"
#include "gamevar.h"
#include "mapinfo.h"
#include "gamestate.h"
#include "conlabel.h"
#include "automap.h"
#include "dukeactor.h"

BEGIN_DUKE_NS

// Player Actions - used by ifp instruction.
enum playeraction_t {
	pstanding    = 0x00000001,
	pwalking     = 0x00000002,
	prunning     = 0x00000004,
	pducking     = 0x00000008,
	pfalling     = 0x00000010,
	pjumping     = 0x00000020,
	phigher      = 0x00000040,
	pwalkingback = 0x00000080,
	prunningback = 0x00000100,
	pkicking     = 0x00000200,
	pshrunk      = 0x00000400,
	pjetpack     = 0x00000800,
	ponsteroids  = 0x00001000,
	ponground    = 0x00002000,
	palive       = 0x00004000,
	pdead        = 0x00008000,
	pfacing      = 0x00010000
};



struct ParseState
{
	int g_p;
	int g_x;
	int* g_t;
	uint8_t killit_flag;
	DDukeActor *g_ac;
	int* insptr;
	Collision coll;

	int parse(void);
	void parseifelse(int condition);
};

int furthestcanseepoint(DDukeActor* i, DDukeActor* ts, int* dax, int* day);
bool ifsquished(DDukeActor* i, int p);
void fakebubbaspawn(DDukeActor* actor, int g_p);
void tearitup(sectortype* sect);
void destroyit(DDukeActor* actor);
void mamaspawn(DDukeActor* actor);
void forceplayerangle(int snum);

bool killthesprite = false;

void addspritetodelete(int spnum)
{
	killthesprite = true;
}

sectortype* toSect(int index)
{
	return validSectorIndex(index) ? &sector[index] : nullptr;
}

int fromSect(sectortype* sect)
{
	return sect ? sectnum(sect) : -1;
}

walltype* toWall(int index)
{
	return validWallIndex(index) ? &wall[index] : nullptr;
}

int fromWall(walltype* sect)
{
	return sect ? wallnum(sect) : -1;
}

static void DoUserDef(bool bSet, int lVar1, int lLabelID, int lVar2, DDukeActor* sActor, int sPlayer, int lParm2)
{
	auto vValue = GetGameVarID(lVar2, sActor, sPlayer);
	auto lValue = vValue.safeValue();

	// most settings have been removed because they are either pointless, no longer existent or simply too dangerous to access.
	// Others have been made read-only.
	switch (lLabelID)
	{
	case USERDEFS_GOD: // redid this so that the script won't be able to disable user-set god mode.
		if (bSet) ud.god = (ud.god & ~2) | (lValue? 2:0);
		else SetGameVarID(lVar2, !!ud.god, sActor, sPlayer);
		break;

	case USERDEFS_CASHMAN:
		if (bSet) ud.cashman = lValue;
		else SetGameVarID(lVar2, ud.cashman, sActor, sPlayer);
		break;

	case USERDEFS_EOG:
		if (bSet) ud.eog = lValue;
		else SetGameVarID(lVar2, ud.eog, sActor, sPlayer);
		break;

	case USERDEFS_SHOWALLMAP:
		if (bSet) gFullMap = lValue;
		else SetGameVarID(lVar2, gFullMap, sActor, sPlayer);
		break;

	case USERDEFS_SHOWWEAPONS:
		// Read-only user state.
		if (!bSet) SetGameVarID(lVar2, cl_showweapon, sActor, sPlayer);
		break;

	case USERDEFS_CAMERASPRITE:
		if (bSet) ud.cameraactor = vValue.safeActor();
		else SetGameVarID(lVar2, ud.cameraactor, sActor, sPlayer);
		break;

	case USERDEFS_LAST_CAMSPRITE:
		if (!bSet) SetGameVarID(lVar2, -1, sActor, sPlayer);
		break;

	case USERDEFS_LAST_LEVEL:
		if (bSet) ud.last_level = lValue;
		else SetGameVarID(lVar2, ud.last_level, sActor, sPlayer);
		break;

	case USERDEFS_SECRETLEVEL:
		if (bSet) ud.secretlevel = lValue;
		else SetGameVarID(lVar2, ud.secretlevel, sActor, sPlayer);
		break;

	case USERDEFS_CONST_VISIBILITY:
		if (bSet) ud.const_visibility = lValue;
		else SetGameVarID(lVar2, ud.const_visibility, sActor, sPlayer);
		break;

	case USERDEFS_SHADOWS:
		if (bSet) ud.shadows = lValue;
		else SetGameVarID(lVar2, ud.shadows, sActor, sPlayer);
		break;
	case USERDEFS_M_COOP:
		if (!bSet) SetGameVarID(lVar2, ud.m_coop, sActor, sPlayer);
		break;

	case USERDEFS_COOP:
		if (!bSet) SetGameVarID(lVar2, ud.coop, sActor, sPlayer);
		break;

	case USERDEFS_RESPAWN_MONSTERS:
		if (bSet) ud.respawn_monsters = lValue;
		else SetGameVarID(lVar2, ud.respawn_monsters, sActor, sPlayer);
		break;

	case USERDEFS_RESPAWN_ITEMS:
		if (bSet) ud.respawn_items = lValue;
		else SetGameVarID(lVar2, ud.respawn_items, sActor, sPlayer);
		break;

	case USERDEFS_RESPAWN_INVENTORY:
		if (bSet) ud.respawn_inventory = lValue;
		else SetGameVarID(lVar2, ud.respawn_inventory, sActor, sPlayer);
		break;

	case USERDEFS_RECSTAT:
		if (!bSet) SetGameVarID(lVar2, ud.recstat, sActor, sPlayer);
		break;

	case USERDEFS_MONSTERS_OFF:
		if (bSet) ud.monsters_off = lValue;
		else SetGameVarID(lVar2, ud.monsters_off, sActor, sPlayer);
		break;

	case USERDEFS_BRIGHTNESS:
		if (bSet) ud.brightness = lValue;
		else SetGameVarID(lVar2, ud.brightness, sActor, sPlayer);
		break;

	case USERDEFS_M_RESPAWN_ITEMS:
		if (bSet) ud.m_respawn_items = lValue;
		else SetGameVarID(lVar2, ud.m_respawn_items, sActor, sPlayer);
		break;

	case USERDEFS_M_RESPAWN_MONSTERS:
		if (bSet) ud.m_respawn_monsters = lValue;
		else SetGameVarID(lVar2, ud.m_respawn_monsters, sActor, sPlayer);
		break;

	case USERDEFS_M_RESPAWN_INVENTORY:
		if (bSet) ud.m_respawn_inventory = lValue;
		else SetGameVarID(lVar2, ud.m_respawn_inventory, sActor, sPlayer);
		break;

	case USERDEFS_M_MONSTERS_OFF:
		if (bSet) ud.m_monsters_off = lValue;
		else SetGameVarID(lVar2, ud.m_monsters_off, sActor, sPlayer);
		break;

	case USERDEFS_M_FFIRE:
		if (bSet) ud.m_ffire = lValue;
		else SetGameVarID(lVar2, ud.m_ffire, sActor, sPlayer);
		break;

	case USERDEFS_FFIRE:
		if (bSet) ud.ffire = lValue;
		else SetGameVarID(lVar2, ud.ffire, sActor, sPlayer);
		break;

	case USERDEFS_MULTIMODE:
		if (!bSet) SetGameVarID(lVar2, ud.multimode, sActor, sPlayer);
		break;

	case USERDEFS_PLAYER_SKILL:
		if (bSet) ud.player_skill = lValue;
		else SetGameVarID(lVar2, ud.player_skill, sActor, sPlayer);
		break;

	case USERDEFS_MARKER:
		if (bSet) ud.marker = lValue;
		else SetGameVarID(lVar2, ud.marker, sActor, sPlayer);
		break;

	default:
		// This will also catch all deleted cases.
		// make sure that the return value is always defined.
		if (!bSet) SetGameVarID(lVar2, 0, sActor, sPlayer);
		break;
	}
	return;
}

///////////////////////////////////////////
void DoPlayer(bool bSet, int lVar1, int lLabelID, int lVar2, DDukeActor* sActor, int sPlayer, int lParm2)
{
	auto vValue = GetGameVarID(lVar2, sActor, sPlayer);
	auto lValue = vValue.safeValue();

	int iPlayer;
	int lTemp;

	if (lVar1 == g_iThisActorID)
	{
		// if they've asked for 'this', then use 'this player'...
		iPlayer = sPlayer;
	}
	else
	{
		iPlayer = GetGameVarID(lVar1, sActor, sPlayer).safeValue();
	}

	if (iPlayer < 0 || iPlayer >= MAXPLAYERS)
		return;

	switch (lLabelID)
	{
	case PLAYER_ZOOM:
		SetGameVarID(lVar2, 768, sActor, sPlayer);	//return default for AM zoom.
		break;

	case PLAYER_EXITX:
		if (bSet) ps[iPlayer].exit.X = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].exit.X, sActor, sPlayer);
		break;

	case PLAYER_EXITY:
		if (bSet) ps[iPlayer].exit.Y = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].exit.Y, sActor, sPlayer);
		break;

	case PLAYER_LOOGIEX:
		if (bSet) ps[iPlayer].loogie[lParm2].X = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].loogie[lParm2].X, sActor, sPlayer);
		break;

	case PLAYER_LOOGIEY:
		if (bSet) ps[iPlayer].loogie[lParm2].Y = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].loogie[lParm2].Y, sActor, sPlayer);
		break;

	case PLAYER_NUMLOOGS:
		if (bSet) ps[iPlayer].numloogs = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].numloogs, sActor, sPlayer);
		break;

	case PLAYER_LOOGCNT:
		if (bSet) ps[iPlayer].oloogcnt = ps[iPlayer].loogcnt = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].loogcnt, sActor, sPlayer);
		break;

	case PLAYER_POSX: // oh, my... :( Writing to these has been disabled until I know how to do it without the engine shitting all over itself.
		if (!bSet) SetGameVarID(lVar2, ps[iPlayer].pos.X, sActor, sPlayer);
		break;

	case PLAYER_POSY:
		if (!bSet) SetGameVarID(lVar2, ps[iPlayer].pos.Y, sActor, sPlayer);
		break;

	case PLAYER_POSZ:
		if (!bSet) SetGameVarID(lVar2, ps[iPlayer].pos.Z, sActor, sPlayer);
		break;

	case PLAYER_HORIZ:
		if (bSet)
		{	
			if (ps[iPlayer].sync.actions & SB_CENTERVIEW)
			{
				ps[iPlayer].sync.actions &= ~SB_CENTERVIEW;
			}
			ps[iPlayer].horizon.settarget(buildhoriz(lValue));
		}
		else SetGameVarID(lVar2, ps[iPlayer].horizon.horiz.asbuild(), sActor, sPlayer);
		break;

	case PLAYER_OHORIZ:
		if (!bSet) SetGameVarID(lVar2, ps[iPlayer].horizon.ohoriz.asbuild(), sActor, sPlayer);
		break;

	case PLAYER_HORIZOFF:
		if (bSet) ps[iPlayer].horizon.horizoff = buildhoriz(lValue);
		else SetGameVarID(lVar2, ps[iPlayer].horizon.horizoff.asbuild(), sActor, sPlayer);
		break;

	case PLAYER_OHORIZOFF:
		if (!bSet) SetGameVarID(lVar2, ps[iPlayer].horizon.ohorizoff.asbuild(), sActor, sPlayer);
		break;

	case PLAYER_INVDISPTIME:
		if (bSet) ps[iPlayer].invdisptime = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].invdisptime, sActor, sPlayer);
		break;

	case PLAYER_BOBPOSX:
		if (bSet) ps[iPlayer].bobpos.X = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].bobpos.X, sActor, sPlayer);
		break;

	case PLAYER_BOBPOSY:
		if (bSet) ps[iPlayer].bobpos.Y = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].bobpos.Y, sActor, sPlayer);
		break;

	case PLAYER_OPOSX:
		if (bSet) ps[iPlayer].opos.X = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].opos.X, sActor, sPlayer);
		break;

	case PLAYER_OPOSY:
		if (bSet) ps[iPlayer].opos.Y = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].opos.Y, sActor, sPlayer);
		break;

	case PLAYER_OPOSZ:
		if (bSet) ps[iPlayer].opos.Z = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].opos.Z, sActor, sPlayer);
		break;

	case PLAYER_PYOFF:
		if (bSet) ps[iPlayer].pyoff = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].pyoff, sActor, sPlayer);
		break;

	case PLAYER_OPYOFF:
		if (bSet) ps[iPlayer].opyoff = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].opyoff, sActor, sPlayer);
		break;

	case PLAYER_POSXV:
		if (bSet) ps[iPlayer].vel.X = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].vel.X, sActor, sPlayer);
		break;

	case PLAYER_POSYV:
		if (bSet) ps[iPlayer].vel.Y = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].vel.Y, sActor, sPlayer);
		break;

	case PLAYER_POSZV:
		if (bSet) ps[iPlayer].vel.Z = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].vel.Z, sActor, sPlayer);
		break;

	case PLAYER_LAST_PISSED_TIME:
		if (bSet) ps[iPlayer].last_pissed_time = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].last_pissed_time, sActor, sPlayer);
		break;

	case PLAYER_TRUEFZ:
		if (bSet) ps[iPlayer].truefz = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].truefz, sActor, sPlayer);
		break;

	case PLAYER_TRUECZ:
 		if (bSet) ps[iPlayer].truecz = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].truecz, sActor, sPlayer);
		break;

	case PLAYER_PLAYER_PAR:
		if (bSet) ps[iPlayer].player_par = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].player_par, sActor, sPlayer);
		break;

	case PLAYER_VISIBILITY:
		if (bSet) ps[iPlayer].visibility = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].visibility, sActor, sPlayer);
		break;

	case PLAYER_BOBCOUNTER:
		if (bSet) ps[iPlayer].bobcounter = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].bobcounter, sActor, sPlayer);
		break;

	case PLAYER_WEAPON_SWAY:
		if (bSet) ps[iPlayer].oweapon_sway = ps[iPlayer].weapon_sway = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].weapon_sway, sActor, sPlayer);
		break;

	case PLAYER_PALS_TIME:
		if (bSet) ps[iPlayer].pals.a = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].pals.a, sActor, sPlayer);
		break;

	case PLAYER_RANDOMFLAMEX:
		if (bSet) ps[iPlayer].randomflamex = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].randomflamex, sActor, sPlayer);
		break;

	case PLAYER_CRACK_TIME:
		if (bSet) ps[iPlayer].crack_time = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].crack_time, sActor, sPlayer);
		break;

	case PLAYER_AIM_MODE: // game has no business enforcing this in any way.
		if (!bSet) SetGameVarID(lVar2, ps[iPlayer].aim_mode, sActor, sPlayer);
		break;

	case PLAYER_ANG:
		if (bSet) ps[iPlayer].angle.settarget(buildang(lValue));
		else SetGameVarID(lVar2, ps[iPlayer].angle.ang.asbuild(), sActor, sPlayer);
		break;

	case PLAYER_OANG:
		if (!bSet) SetGameVarID(lVar2, ps[iPlayer].angle.oang.asbuild(), sActor, sPlayer);
		break;

	case PLAYER_ANGVEL: // This no longer exists.
		if (!bSet) SetGameVarID(lVar2, 0, sActor, sPlayer);
		break;

	case PLAYER_CURSECTNUM:
		if (bSet) ps[iPlayer].cursector = toSect(lValue);
		else SetGameVarID(lVar2, fromSect(ps[iPlayer].cursector), sActor, sPlayer);
		break;

	case PLAYER_LOOK_ANG:
		if (bSet) ps[iPlayer].angle.look_ang = buildang(lValue);
		else SetGameVarID(lVar2, ps[iPlayer].angle.look_ang.asbuild(), sActor, sPlayer);
		break;

	case PLAYER_LAST_EXTRA:
		if (bSet) ps[iPlayer].last_extra = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].last_extra, sActor, sPlayer);
		break;

	case PLAYER_SUBWEAPON:
		if (bSet) ps[iPlayer].subweapon = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].subweapon, sActor, sPlayer);
		break;

	case PLAYER_AMMO_AMOUNT:
		lTemp = GetGameVarID(lParm2, sActor, sPlayer).safeValue();
		if (bSet) ps[iPlayer].ammo_amount[lTemp] = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].ammo_amount[lTemp], sActor, sPlayer);
		break;

	case PLAYER_WACKEDBYACTOR:
		if (bSet) ps[iPlayer].wackedbyactor = vValue.safeActor();
		else SetGameVarID(lVar2, ps[iPlayer].wackedbyactor, sActor, sPlayer);
		break;

	case PLAYER_FRAG:
		if (bSet) ps[iPlayer].frag = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].frag, sActor, sPlayer);
		break;

	case PLAYER_FRAGGEDSELF:
		if (bSet) ps[iPlayer].fraggedself = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].fraggedself, sActor, sPlayer);
		break;

	case PLAYER_CURR_WEAPON:
		if (bSet) ps[iPlayer].curr_weapon = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].curr_weapon, sActor, sPlayer);
		break;

	case PLAYER_LAST_WEAPON:
		if (bSet) ps[iPlayer].last_weapon = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].last_weapon, sActor, sPlayer);
		break;

	case PLAYER_TIPINCS:
		if (bSet) ps[iPlayer].otipincs = ps[iPlayer].tipincs = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].tipincs, sActor, sPlayer);
		break;

	case PLAYER_WANTWEAPONFIRE:
		if (bSet) ps[iPlayer].wantweaponfire = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].wantweaponfire, sActor, sPlayer);
		break;

	case PLAYER_HOLODUKE_AMOUNT:
		if (bSet) ps[iPlayer].holoduke_amount = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].holoduke_amount, sActor, sPlayer);
		break;

	case PLAYER_NEWOWNER:
		if (bSet) ps[iPlayer].newOwner = vValue.safeActor();
		else SetGameVarID(lVar2, ps[iPlayer].newOwner, sActor, sPlayer);
		break;

	case PLAYER_HURT_DELAY:
		if (bSet) ps[iPlayer].hurt_delay = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].hurt_delay, sActor, sPlayer);
		break;

	case PLAYER_HBOMB_HOLD_DELAY:
		if (bSet) ps[iPlayer].hbomb_hold_delay = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].hbomb_hold_delay, sActor, sPlayer);
		break;

	case PLAYER_JUMPING_COUNTER:
		if (bSet) ps[iPlayer].jumping_counter = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].jumping_counter, sActor, sPlayer);
		break;

	case PLAYER_AIRLEFT:
		if (bSet) ps[iPlayer].airleft = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].airleft, sActor, sPlayer);
		break;

	case PLAYER_KNEE_INCS:
		if (bSet) ps[iPlayer].oknee_incs = ps[iPlayer].knee_incs = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].knee_incs, sActor, sPlayer);
		break;

	case PLAYER_ACCESS_INCS:
		if (bSet) ps[iPlayer].oaccess_incs = ps[iPlayer].access_incs = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].access_incs, sActor, sPlayer);
		break;

	case PLAYER_ACCESS_WALLNUM:
		if (bSet) ps[iPlayer].access_wall = toWall(lValue);
		else SetGameVarID(lVar2, fromWall(ps[iPlayer].access_wall), sActor, sPlayer);
		break;

	case PLAYER_ACCESS_SPRITENUM:
		if (bSet) ps[iPlayer].access_spritenum = vValue.safeActor();
		else SetGameVarID(lVar2, ps[iPlayer].access_spritenum, sActor, sPlayer);
		break;

	case PLAYER_KICKBACK_PIC:
		if (bSet) ps[iPlayer].okickback_pic = ps[iPlayer].kickback_pic = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].kickback_pic, sActor, sPlayer);
		break;

	case PLAYER_GOT_ACCESS:
		if (bSet) ps[iPlayer].got_access = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].got_access, sActor, sPlayer);
		break;

	case PLAYER_WEAPON_ANG:
		if (bSet) ps[iPlayer].weapon_ang = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].weapon_ang, sActor, sPlayer);
		break;

	case PLAYER_FIRSTAID_AMOUNT:
		if (bSet) ps[iPlayer].firstaid_amount = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].firstaid_amount, sActor, sPlayer);
		break;

	case PLAYER_SOMETHINGONPLAYER:
		if (bSet) ps[iPlayer].somethingonplayer = vValue.safeActor();
		else SetGameVarID(lVar2, (ps[iPlayer].somethingonplayer), sActor, sPlayer);
		break;

	case PLAYER_ON_CRANE:
		if (bSet) ps[iPlayer].on_crane = vValue.safeActor();
		else SetGameVarID(lVar2, (ps[iPlayer].on_crane), sActor, sPlayer);
		break;

	case PLAYER_I:	// Read only, because this is very dangerous.
		if (!bSet) SetGameVarID(lVar2, ps[iPlayer].actor, sActor, sPlayer);
		break;

	case PLAYER_ONE_PARALLAX_SECTNUM:
		if (bSet) ps[iPlayer].one_parallax_sectnum = toSect(lValue);
		else SetGameVarID(lVar2, fromSect(ps[iPlayer].one_parallax_sectnum), sActor, sPlayer);
		break;

	case PLAYER_OVER_SHOULDER_ON:
		if (bSet) ps[iPlayer].over_shoulder_on = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].over_shoulder_on, sActor, sPlayer);
		break;

	case PLAYER_RANDOM_CLUB_FRAME:
		if (bSet) ps[iPlayer].orandom_club_frame = ps[iPlayer].random_club_frame = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].random_club_frame, sActor, sPlayer);
		break;

	case PLAYER_FIST_INCS:
		if (bSet) ps[iPlayer].ofist_incs = ps[iPlayer].fist_incs = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].fist_incs, sActor, sPlayer);
		break;

	case PLAYER_ONE_EIGHTY_COUNT:
		if (bSet) ps[iPlayer].angle.spin = lValue;
		else SetGameVarID(lVar2, int(ps[iPlayer].angle.spin), sActor, sPlayer);
		break;

	case PLAYER_CHEAT_PHASE:
		if (bSet) ps[iPlayer].cheat_phase = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].cheat_phase, sActor, sPlayer);
		break;

	case PLAYER_DUMMYPLAYERSPRITE:
		if (bSet) ps[iPlayer].dummyplayersprite = vValue.safeActor();
		else SetGameVarID(lVar2, (ps[iPlayer].dummyplayersprite), sActor, sPlayer);
		break;

	case PLAYER_EXTRA_EXTRA8:
		if (bSet) ps[iPlayer].extra_extra8 = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].extra_extra8, sActor, sPlayer);
		break;

	case PLAYER_QUICK_KICK:
		if (bSet) ps[iPlayer].quick_kick = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].quick_kick, sActor, sPlayer);
		break;

	case PLAYER_HEAT_AMOUNT:
		if (bSet) ps[iPlayer].heat_amount = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].heat_amount, sActor, sPlayer);
		break;

	case PLAYER_ACTORSQU:
		if (bSet) ps[iPlayer].actorsqu = vValue.safeActor();
		else SetGameVarID(lVar2, (ps[iPlayer].actorsqu), sActor, sPlayer);
		break;

	case PLAYER_TIMEBEFOREEXIT:
		if (bSet) ps[iPlayer].timebeforeexit = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].timebeforeexit, sActor, sPlayer);
		break;

	case PLAYER_CUSTOMEXITSOUND:
		if (bSet) ps[iPlayer].customexitsound = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].customexitsound, sActor, sPlayer);
		break;

	case PLAYER_WEAPRECS:
		if (bSet) ps[iPlayer].weaprecs[lParm2] = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].weaprecs[lParm2], sActor, sPlayer);
		break;

	case PLAYER_WEAPRECCNT:
		if (bSet) ps[iPlayer].weapreccnt = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].weapreccnt, sActor, sPlayer);
		break;

	case PLAYER_INTERFACE_TOGGLE_FLAG:
		if (bSet) ps[iPlayer].interface_toggle_flag = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].interface_toggle_flag, sActor, sPlayer);
		break;

	case PLAYER_ROTSCRNANG:
		if (bSet)
		{
			ps[iPlayer].angle.orotscrnang = ps[iPlayer].angle.rotscrnang;
			ps[iPlayer].angle.rotscrnang = buildang(lValue);
		}
		else SetGameVarID(lVar2, ps[iPlayer].angle.rotscrnang.asbuild(), sActor, sPlayer);
		break;

	case PLAYER_DEAD_FLAG:
		if (bSet) ps[iPlayer].dead_flag = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].dead_flag, sActor, sPlayer);
		break;

	case PLAYER_SHOW_EMPTY_WEAPON:
		if (bSet) ps[iPlayer].show_empty_weapon = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].show_empty_weapon, sActor, sPlayer);
		break;

	case PLAYER_SCUBA_AMOUNT:
		if (bSet) ps[iPlayer].scuba_amount = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].scuba_amount, sActor, sPlayer);
		break;

	case PLAYER_JETPACK_AMOUNT:
		if (bSet) ps[iPlayer].jetpack_amount = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].jetpack_amount, sActor, sPlayer);
		break;

	case PLAYER_STEROIDS_AMOUNT:
		if (bSet) ps[iPlayer].steroids_amount = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].steroids_amount, sActor, sPlayer);
		break;

	case PLAYER_SHIELD_AMOUNT:
		if (bSet) ps[iPlayer].shield_amount = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].shield_amount, sActor, sPlayer);
		break;

	case PLAYER_HOLODUKE_ON:
		if (bSet) ps[iPlayer].holoduke_on = vValue.safeActor();
		else SetGameVarID(lVar2, (ps[iPlayer].holoduke_on), sActor, sPlayer);
		break;

	case PLAYER_PYCOUNT:
		if (bSet) ps[iPlayer].pycount = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].pycount, sActor, sPlayer);
		break;

	case PLAYER_WEAPON_POS:
		if (bSet) ps[iPlayer].oweapon_pos = ps[iPlayer].weapon_pos = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].weapon_pos, sActor, sPlayer);
		break;

	case PLAYER_FRAG_PS:
		if (bSet) 	ps[iPlayer].frag_ps = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].frag_ps, sActor, sPlayer);
		break;

	case PLAYER_TRANSPORTER_HOLD:
		if (bSet) ps[iPlayer].transporter_hold = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].transporter_hold, sActor, sPlayer);
		break;

	case PLAYER_LAST_FULL_WEAPON:
		if (bSet) ps[iPlayer].last_full_weapon = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].last_full_weapon, sActor, sPlayer);
		break;

	case PLAYER_FOOTPRINTSHADE:
		if (bSet) ps[iPlayer].footprintshade = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].footprintshade, sActor, sPlayer);
		break;

	case PLAYER_BOOT_AMOUNT:
		if (bSet) ps[iPlayer].boot_amount = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].boot_amount, sActor, sPlayer);
		break;

	case PLAYER_GM:
		if (!bSet) SetGameVarID(lVar2, MODE_GAME, sActor, sPlayer);
		break;

	case PLAYER_ON_WARPING_SECTOR:
		if (bSet) ps[iPlayer].on_warping_sector = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].on_warping_sector, sActor, sPlayer);
		break;

	case PLAYER_FOOTPRINTCOUNT:
		if (bSet) ps[iPlayer].footprintcount = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].footprintcount, sActor, sPlayer);
		break;

	case PLAYER_HBOMB_ON:
		if (bSet) ps[iPlayer].hbomb_on = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].hbomb_on, sActor, sPlayer);
		break;

	case PLAYER_JUMPING_TOGGLE:
		if (bSet) ps[iPlayer].jumping_toggle = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].jumping_toggle, sActor, sPlayer);
		break;

	case PLAYER_RAPID_FIRE_HOLD:
		if (bSet) ps[iPlayer].rapid_fire_hold = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].rapid_fire_hold, sActor, sPlayer);
		break;

	case PLAYER_ON_GROUND:
		if (bSet) ps[iPlayer].on_ground = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].on_ground, sActor, sPlayer);
		break;

	case PLAYER_INVEN_ICON:
		if (bSet) ps[iPlayer].inven_icon = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].inven_icon, sActor, sPlayer);
		break;

	case PLAYER_BUTTONPALETTE:
		if (bSet) ps[iPlayer].buttonpalette = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].buttonpalette, sActor, sPlayer);
		break;

	case PLAYER_JETPACK_ON:
		if (bSet) ps[iPlayer].jetpack_on = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].jetpack_on, sActor, sPlayer);
		break;

	case PLAYER_SPRITEBRIDGE:
		if (bSet) ps[iPlayer].spritebridge = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].spritebridge, sActor, sPlayer);
		break;

	case PLAYER_LASTRANDOMSPOT:
		if (bSet) ps[iPlayer].lastrandomspot = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].lastrandomspot, sActor, sPlayer);
		break;

	case PLAYER_SCUBA_ON:
		if (bSet) ps[iPlayer].scuba_on = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].scuba_on, sActor, sPlayer);
		break;

	case PLAYER_FOOTPRINTPAL:
		if (bSet) ps[iPlayer].footprintpal = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].footprintpal, sActor, sPlayer);
		break;

	case PLAYER_HEAT_ON:
		if (bSet) ps[iPlayer].heat_on = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].heat_on, sActor, sPlayer);
		break;

	case PLAYER_HOLSTER_WEAPON:
		if (bSet) ps[iPlayer].holster_weapon = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].holster_weapon, sActor, sPlayer);
		break;

	case PLAYER_FALLING_COUNTER:
		if (bSet) ps[iPlayer].falling_counter = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].falling_counter, sActor, sPlayer);
		break;

	case PLAYER_GOTWEAPON:
		if (bSet) ps[iPlayer].gotweapon[lParm2] = !!lValue;
		else SetGameVarID(lVar2, ps[iPlayer].gotweapon[lParm2], sActor, sPlayer);
		break;

	case PLAYER_REFRESH_INVENTORY:
		if (bSet) ps[iPlayer].refresh_inventory = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].refresh_inventory, sActor, sPlayer);
		break;

	case PLAYER_TOGGLE_KEY_FLAG:
		if (bSet) ps[iPlayer].toggle_key_flag = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].toggle_key_flag, sActor, sPlayer);
		break;

	case PLAYER_KNUCKLE_INCS:
		if (bSet) ps[iPlayer].knuckle_incs = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].knuckle_incs, sActor, sPlayer);
		break;

	case PLAYER_WALKING_SND_TOGGLE:
		if (bSet) ps[iPlayer].walking_snd_toggle = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].walking_snd_toggle, sActor, sPlayer);
		break;

	case PLAYER_PALOOKUP:
		if (bSet) ps[iPlayer].palookup = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].palookup, sActor, sPlayer);
		break;

	case PLAYER_HARD_LANDING:
		if (bSet) ps[iPlayer].ohard_landing = ps[iPlayer].hard_landing = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].hard_landing, sActor, sPlayer);
		break;

	case PLAYER_MAX_SECRET_ROOMS:
		if (bSet) ps[iPlayer].max_secret_rooms = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].max_secret_rooms, sActor, sPlayer);
		break;

	case PLAYER_SECRET_ROOMS:
		if (bSet) ps[iPlayer].secret_rooms = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].secret_rooms, sActor, sPlayer);
		break;

	case PLAYER_MAX_ACTORS_KILLED:
		if (bSet) ps[iPlayer].max_actors_killed = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].max_actors_killed, sActor, sPlayer);
		break;

	case PLAYER_ACTORS_KILLED:
		if (bSet) ps[iPlayer].actors_killed = lValue;
		else SetGameVarID(lVar2, ps[iPlayer].actors_killed, sActor, sPlayer);
		break;

	case PLAYER_RETURN_TO_CENTER:
		if (bSet)
		{
			ps[iPlayer].sync.actions |= SB_CENTERVIEW;
		}
		else
		{
			auto center = ps[iPlayer].sync.actions & SB_CENTERVIEW ? abs(xs_CRoundToInt(ps[iPlayer].horizon.horiz.asq16() * (9. / gi->playerHorizMax()))) : 0;
			SetGameVarID(lVar2, center, sActor, sPlayer);
		}
		break;

	default:
		if (!bSet) SetGameVarID(lVar2, 0, sActor, sPlayer);
		break;
	}
	return;
}

////////////////////
void DoWall(bool bSet, int lVar1, int lLabelID, int lVar2, DDukeActor* sActor, int sPlayer, int lParm2)
{
	auto lValue = GetGameVarID(lVar2, sActor, sPlayer).safeValue();
	auto vWall = GetGameVarID(lVar1, sActor, sPlayer);
	auto iWall = vWall.safeValue();

	if (iWall < 0 || iWall >= (int)wall.Size() || vWall.isActor())
	{
		if (!bSet) SetGameVarID(lVar2, 0, sActor, sPlayer);
		return;
	}
	auto wallp = &wall[iWall];

	// All fields affecting map geometry have been made read-only!
	switch (lLabelID)
	{
	case WALL_X:
		if (!bSet) SetGameVarID(lVar2, wallp->wall_int_pos().X, sActor, sPlayer);
		break;
	case WALL_Y:
		if (bSet) SetGameVarID(lVar2, wallp->wall_int_pos().Y, sActor, sPlayer);
		break;
	case WALL_POINT2:
		if (!bSet) SetGameVarID(lVar2, wallp->point2, sActor, sPlayer);
		break;
	case WALL_NEXTWALL:
		if (!bSet) SetGameVarID(lVar2, wallp->nextwall, sActor, sPlayer);
		break;
	case WALL_NEXTSECTOR:
		if (!bSet) SetGameVarID(lVar2, wallp->nextsector, sActor, sPlayer);
		break;
	case WALL_CSTAT:
		if (bSet) wallp->cstat = EWallFlags::FromInt(lValue);
		else SetGameVarID(lVar2, wallp->cstat, sActor, sPlayer);
		break;
	case WALL_PICNUM:
		if (bSet) wallp->picnum = lValue;
		else SetGameVarID(lVar2, wallp->picnum, sActor, sPlayer);
		break;
	case WALL_OVERPICNUM:
		if (bSet) wallp->overpicnum = lValue;
		else SetGameVarID(lVar2, wallp->overpicnum, sActor, sPlayer);
		break;
	case WALL_SHADE:
		if (bSet) wallp->shade = lValue;
		else SetGameVarID(lVar2, wallp->shade, sActor, sPlayer);
		break;
	case WALL_PAL:
		if (bSet) wallp->pal = lValue;
		else SetGameVarID(lVar2, wallp->pal, sActor, sPlayer);
		break;
	case WALL_XREPEAT:
		if (bSet) wallp->xrepeat = lValue;
		else SetGameVarID(lVar2, wallp->xrepeat, sActor, sPlayer);
		break;
	case WALL_YREPEAT:
		if (bSet) wallp->yrepeat = lValue;
		else SetGameVarID(lVar2, wallp->yrepeat, sActor, sPlayer);
		break;
	case WALL_XPANNING:
		if (bSet) wallp->xpan_ = (float)(lValue & 255);
		else SetGameVarID(lVar2, wallp->xpan(), sActor, sPlayer);
		break;
	case WALL_YPANNING:
		if (bSet) wallp->ypan_ = (float)(lValue & 255);
		else SetGameVarID(lVar2, wallp->ypan(), sActor, sPlayer);
		break;
	case WALL_LOTAG:
		if (bSet) wallp->lotag = lValue;
		else SetGameVarID(lVar2, wallp->lotag, sActor, sPlayer);
		break;
	case WALL_HITAG:
		if (bSet) wallp->hitag = lValue;
		else SetGameVarID(lVar2, wallp->hitag, sActor, sPlayer);
		break;
	case WALL_EXTRA:
		if (bSet) wallp->extra = lValue;
		else SetGameVarID(lVar2, wallp->extra, sActor, sPlayer);
		break;
	default:
		break;
	}
	return;
}

void DoSector(bool bSet, int lVar1, int lLabelID, int lVar2, DDukeActor* sActor, int sPlayer, int lParm2)
{
	int iSector;
	int lValue;
	bool no = false;

	if (lVar1 == g_iThisActorID)
	{
		// if they've asked for 'this', then use 'this'...
		iSector = sActor->sectno();
	}
	else
	{
		auto vv = GetGameVarID(lVar1, sActor, sPlayer);
		no = vv.isActor();
		iSector = vv.safeValue();
	}

	if (iSector < 0 || iSector >= (int)sector.Size() || no)
	{
		if (!bSet) SetGameVarID(lVar2, 0, sActor, sPlayer);
		return;
	}

	lValue = GetGameVarID(lVar2, sActor, sPlayer).safeValue();
	auto sectp = &sector[iSector];

	// All fields affecting map geometry have been made read-only!
	switch (lLabelID)
	{
	case SECTOR_WALLPTR:
		if (!bSet) SetGameVarID(lVar2, sectp->wallptr, sActor, sPlayer);
		break;
	case SECTOR_WALLNUM:
		if (!bSet) SetGameVarID(lVar2, sectp->wallnum, sActor, sPlayer);
		break;
	case SECTOR_CEILINGZ:
		if (bSet) sectp->setceilingz(lValue);
		else SetGameVarID(lVar2, sectp->ceilingz, sActor, sPlayer);
		break;
	case SECTOR_FLOORZ:
		if (bSet) sectp->setfloorz(lValue);
		else SetGameVarID(lVar2, sectp->floorz, sActor, sPlayer);
		break;
	case SECTOR_CEILINGSTAT:
		if (bSet) sectp->ceilingstat = ESectorFlags::FromInt(lValue);
		else SetGameVarID(lVar2, sectp->ceilingstat, sActor, sPlayer);
		break;
	case SECTOR_FLOORSTAT:
		if (bSet) sectp->floorstat = ESectorFlags::FromInt(lValue);
		else SetGameVarID(lVar2, sectp->floorstat, sActor, sPlayer);
		break;
	case SECTOR_CEILINGPICNUM:
		if (bSet) sectp->ceilingpicnum = lValue;
		else SetGameVarID(lVar2, sectp->ceilingpicnum, sActor, sPlayer);
		break;
	case SECTOR_CEILINGSLOPE:
		if (bSet) sectp->setceilingslope(lValue);
		else SetGameVarID(lVar2, sectp->ceilingheinum, sActor, sPlayer);
		break;
	case SECTOR_CEILINGSHADE:
		if (bSet) sectp->ceilingshade = lValue;
		else SetGameVarID(lVar2, sectp->ceilingshade, sActor, sPlayer);
		break;
	case SECTOR_CEILINGPAL:
		if (bSet) sectp->ceilingpal = lValue;
		else SetGameVarID(lVar2, sectp->ceilingpal, sActor, sPlayer);
		break;
	case SECTOR_CEILINGXPANNING:
		if (bSet) sectp->ceilingxpan_ = (float)(lValue & 255);
		else SetGameVarID(lVar2, sectp->ceilingxpan(), sActor, sPlayer);
		break;
	case SECTOR_CEILINGYPANNING:
		if (bSet) sectp->ceilingypan_ = (float)(lValue & 255);
		else SetGameVarID(lVar2, sectp->ceilingypan(), sActor, sPlayer);
		break;
	case SECTOR_FLOORPICNUM:
		if (bSet) sectp->floorpicnum = lValue;
		else SetGameVarID(lVar2, sectp->floorpicnum, sActor, sPlayer);
		break;
	case SECTOR_FLOORSLOPE:
		if (bSet) sectp->setfloorslope(lValue);
		else SetGameVarID(lVar2, sectp->floorheinum, sActor, sPlayer);
		break;
	case SECTOR_FLOORSHADE:
		if (bSet) sectp->floorshade = lValue;
		else SetGameVarID(lVar2, sectp->floorshade, sActor, sPlayer);
		break;
	case SECTOR_FLOORPAL:
		if (bSet) sectp->floorpal = lValue;
		else SetGameVarID(lVar2, sectp->floorpal, sActor, sPlayer);
		break;
	case SECTOR_FLOORXPANNING:
		if (bSet) sectp->floorxpan_ = (float)(lValue & 255);
		else SetGameVarID(lVar2, sectp->floorxpan(), sActor, sPlayer);
		break;
	case SECTOR_FLOORYPANNING:
		if (bSet) sectp->floorypan_ = (float)(lValue & 255);
		else SetGameVarID(lVar2, sectp->floorypan(), sActor, sPlayer);
		break;
	case SECTOR_VISIBILITY:
		if (bSet) sectp->visibility = lValue;
		else SetGameVarID(lVar2, sectp->visibility, sActor, sPlayer);
		break;
	case SECTOR_LOTAG:
		if (bSet) sectp->lotag = lValue;
		else SetGameVarID(lVar2, sectp->lotag, sActor, sPlayer);
		break;
	case SECTOR_HITAG:
		if (bSet) sectp->hitag = lValue;
		else SetGameVarID(lVar2, sectp->hitag, sActor, sPlayer);
		break;
	case SECTOR_EXTRA:
		if (bSet) sectp->extra = lValue;
		else SetGameVarID(lVar2, sectp->extra, sActor, sPlayer);
		break;
	default:
		break;

	}
	return;
}

void DoActor(bool bSet, int lVar1, int lLabelID, int lVar2, DDukeActor* sActor, int sPlayer, int lParm2)
{
	auto vValue = GetGameVarID(lVar2, sActor, sPlayer);
	auto lValue = vValue.safeValue();

	DDukeActor* act;
	if (lVar1 == g_iThisActorID)
	{
		// if they've asked for 'this', then use 'this'...
		act = sActor;
	}
	else
	{
		act = GetGameVarID(lVar1, sActor, sPlayer).safeActor();
	}
	if (!act)
	{
		if (!bSet) SetGameVarID(lVar2, 0, sActor, sPlayer);
		return;
	}

	switch (lLabelID)
	{
	case ACTOR_X:
		if (bSet) act->spr.pos.X = lValue;
		else SetGameVarID(lVar2, act->spr.pos.X, sActor, sPlayer);
		break;
	case ACTOR_Y:
		if (bSet) act->spr.pos.Y = lValue;
		else SetGameVarID(lVar2, act->spr.pos.Y, sActor, sPlayer);
		break;
	case ACTOR_Z:
		if (bSet) act->spr.pos.Z = lValue;
		else SetGameVarID(lVar2, act->spr.pos.Z, sActor, sPlayer);
		break;
	case ACTOR_CSTAT:
		if (bSet) act->spr.cstat = ESpriteFlags::FromInt(lValue);
		else SetGameVarID(lVar2, act->spr.cstat, sActor, sPlayer);
		break;
	case ACTOR_PICNUM:
		if (bSet) act->spr.picnum = lValue;
		else SetGameVarID(lVar2, act->spr.picnum, sActor, sPlayer);
		break;
	case ACTOR_SHADE:
		if (bSet) act->spr.shade = lValue;
		else SetGameVarID(lVar2, act->spr.shade, sActor, sPlayer);
		break;
	case ACTOR_PAL:
		if (bSet) act->spr.pal = lValue;
		else SetGameVarID(lVar2, act->spr.pal, sActor, sPlayer);
		break;
	case ACTOR_CLIPDIST:
		if (bSet) act->spr.clipdist = lValue;
		else SetGameVarID(lVar2, act->spr.clipdist, sActor, sPlayer);
		break;
	case ACTOR_DETAIL:
		if (bSet) act->spriteextra = lValue;
		else SetGameVarID(lVar2, act->spriteextra, sActor, sPlayer);
		break;
	case ACTOR_XREPEAT:
		if (bSet) act->spr.xrepeat = lValue;
		else SetGameVarID(lVar2, act->spr.xrepeat, sActor, sPlayer);
		break;
	case ACTOR_YREPEAT:
		if (bSet) act->spr.yrepeat = lValue;
		else SetGameVarID(lVar2, act->spr.yrepeat, sActor, sPlayer);
		break;
	case ACTOR_XOFFSET:
		if (bSet) act->spr.xoffset = lValue;
		else SetGameVarID(lVar2, act->spr.xoffset, sActor, sPlayer);
		break;
	case ACTOR_YOFFSET:
		if (bSet) act->spr.yoffset = lValue;
		else SetGameVarID(lVar2, act->spr.yoffset, sActor, sPlayer);
		break;
	case ACTOR_SECTNUM: // made read only because this is not safe.
		if (!bSet) /*changespritesect(iActor, lValue);
		else*/ SetGameVarID(lVar2, act->sectno(), sActor, sPlayer);
		break;
	case ACTOR_STATNUM: 
		if (!bSet) /*changespritestat(iActor, lValue);
		else*/ SetGameVarID(lVar2, act->spr.statnum, sActor, sPlayer);
		break;
	case ACTOR_ANG:
		if (bSet) act->spr.ang = lValue;
		else SetGameVarID(lVar2, act->spr.ang, sActor, sPlayer);
		break;
	case ACTOR_OWNER:
		// there is no way to handle this well because we do not know whether this is an actor or not. Pity.
		if (bSet) act->spr.intowner = lValue;
		else SetGameVarID(lVar2, act->spr.intowner, sActor, sPlayer);
		break;
	case ACTOR_XVEL:
		if (bSet) act->spr.xvel = lValue;
		else SetGameVarID(lVar2, act->spr.xvel, sActor, sPlayer);
		break;
	case ACTOR_YVEL:
		if (bSet) act->spr.yvel = lValue;
		else SetGameVarID(lVar2, act->spr.yvel, sActor, sPlayer);
		break;
	case ACTOR_ZVEL:
		if (bSet) act->spr.zvel = lValue;
		else SetGameVarID(lVar2, act->spr.zvel, sActor, sPlayer);
		break;
	case ACTOR_LOTAG:
		if (bSet) act->spr.lotag = lValue;
		else SetGameVarID(lVar2, act->spr.lotag, sActor, sPlayer);
		break;
	case ACTOR_HITAG:
		if (bSet) act->spr.hitag = lValue;
		else SetGameVarID(lVar2, act->spr.hitag, sActor, sPlayer);
		break;
	case ACTOR_EXTRA:
		if (bSet) act->spr.extra = lValue;
		else SetGameVarID(lVar2, act->spr.extra, sActor, sPlayer);
		break;

	case ACTOR_HTCGG:
		if (bSet) act->cgg = lValue;
		else SetGameVarID(lVar2, act->cgg, sActor, sPlayer);
		break;
	case ACTOR_HTPICNUM:
		if (bSet) act->attackertype = lValue;
		else SetGameVarID(lVar2, act->attackertype, sActor, sPlayer);
		break;
	case ACTOR_HTANG:
		if (bSet) act->hitang = lValue;
		else SetGameVarID(lVar2, act->hitang, sActor, sPlayer);
		break;
	case ACTOR_HTEXTRA:
		if (bSet) act->hitextra = lValue;
		else SetGameVarID(lVar2, act->hitextra, sActor, sPlayer);
		break;
	case ACTOR_HTOWNER:
		if (bSet) act->hitOwnerActor = vValue.safeActor();
		else SetGameVarID(lVar2, act->hitOwnerActor, sActor, sPlayer);
		break;
	case ACTOR_HTMOVFLAG:
		if (bSet) act->movflag = lValue;
		else SetGameVarID(lVar2, act->movflag, sActor, sPlayer);
		break;
	case ACTOR_HTTEMPANG:
		if (bSet) act->tempang = lValue;
		else SetGameVarID(lVar2, act->tempang, sActor, sPlayer);
		break;
	case ACTOR_HTACTORSTAYPUT:
		if (bSet) act->actorstayput = toSect(lValue);
		else SetGameVarID(lVar2, fromSect(act->actorstayput), sActor, sPlayer);
		break;
	case ACTOR_HTDISPICNUM:
		if (bSet) act->dispicnum = lValue;
		else SetGameVarID(lVar2, act->dispicnum, sActor, sPlayer);
		break;
	case ACTOR_HTTIMETOSLEEP:
		if (bSet) act->timetosleep = lValue;
		else SetGameVarID(lVar2, act->timetosleep, sActor, sPlayer);
		break;
	case ACTOR_HTFLOORZ:
		if (bSet) act->floorz = lValue;
		else SetGameVarID(lVar2, act->floorz, sActor, sPlayer);
		break;
	case ACTOR_HTCEILINGZ:
		if (bSet) act->ceilingz = lValue;
		else SetGameVarID(lVar2, act->ceilingz, sActor, sPlayer);
		break;
	case ACTOR_HTLASTVX:
		if (bSet) act->ovel.X = lValue;
		else SetGameVarID(lVar2, act->ovel.X, sActor, sPlayer);
		break;
	case ACTOR_HTLASTVY:
		if (bSet) act->ovel.Y = lValue;
		else SetGameVarID(lVar2, act->ovel.Y, sActor, sPlayer);
		break;
	case ACTOR_HTBPOSX:
		if (bSet) act->opos.X = lValue;
		else SetGameVarID(lVar2, act->opos.X, sActor, sPlayer);
		break;
	case ACTOR_HTBPOSY:
		if (bSet) act->opos.Y = lValue;
		else SetGameVarID(lVar2, act->opos.Y, sActor, sPlayer);
		break;
	case ACTOR_HTBPOSZ:
		if (bSet) act->opos.Z = lValue;
		else SetGameVarID(lVar2, act->opos.Z, sActor, sPlayer);
		break;
	case ACTOR_HTG_T0:
		if (bSet) act->temp_data[0] = lValue;
		else SetGameVarID(lVar2, act->temp_data[0], sActor, sPlayer);
		break;
	case ACTOR_HTG_T1:
		if (bSet) act->temp_data[1] = lValue;
		else SetGameVarID(lVar2, act->temp_data[1], sActor, sPlayer);
		break;
	case ACTOR_HTG_T2:
		if (bSet) act->temp_data[2] = lValue;
		else SetGameVarID(lVar2, act->temp_data[2], sActor, sPlayer);
		break;
	case ACTOR_HTG_T3:
		if (bSet) act->temp_data[3] = lValue;
		else SetGameVarID(lVar2, act->temp_data[3], sActor, sPlayer);
		break;
	case ACTOR_HTG_T4:
		if (bSet) act->temp_data[4] = lValue;
		else SetGameVarID(lVar2, act->temp_data[4], sActor, sPlayer);
		break;
	case ACTOR_HTG_T5:
		if (bSet) act->temp_data[5] = lValue;
		else SetGameVarID(lVar2, act->temp_data[5], sActor, sPlayer);
		break;

	default:
		break;
	}
	return;
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void ParseState::parseifelse(int condition)
{
	if( condition )
	{
		// skip 'else' pointer.. and...
		insptr+=2;
		parse();
	}
	else
	{
		insptr = &ScriptCode[*(insptr+1)];
		if(*insptr == 10)
		{
			// else...

			// skip 'else' and...
			insptr+=2;
			parse();
		}
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static int ifcanshoottarget(DDukeActor *actor, int g_p, int g_x)
{
	int j;
	if (g_x > 1024)
	{
		int sclip, angdif;

		if (badguy(actor) && actor->spr.xrepeat > 56)
		{
			sclip = 3084;
			angdif = 48;
		}
		else
		{
			sclip = 768;
			angdif = 16;
		}

		DDukeActor* hit;
		j = hitasprite(actor, &hit);
		if (j == (1 << 30))
		{
			return 1;
		}
		if (j > sclip)
		{
			if (hit != nullptr && hit->spr.picnum == actor->spr.picnum)
				j = 0;
			else
			{
				actor->spr.ang += angdif; j = hitasprite(actor, &hit); actor->spr.ang -= angdif;
				if (j > sclip)
				{
					if (hit != nullptr && hit->spr.picnum == actor->spr.picnum)
						j = 0;
					else
					{
						actor->spr.ang -= angdif; j = hitasprite(actor, &hit); actor->spr.ang += angdif;
						if (j > 768)
						{
							if (hit != nullptr && hit->spr.picnum == actor->spr.picnum)
								j = 0;
							else j = 1;
						}
						else j = 0;
					}
				}
				else j = 0;
			}
		}
		else j = 0;
	}
	else j = 1;
	return j;
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static bool ifcansee(DDukeActor* actor, int pnum)
{
	int j;
	DDukeActor* tosee;

	// select sprite for monster to target
	// if holoduke is on, let them target holoduke first.
	// 
	if (ps[pnum].holoduke_on != nullptr && !isRR())
	{
		tosee = ps[pnum].holoduke_on;
		j = cansee(actor->spr.pos.X, actor->spr.pos.Y, actor->spr.pos.Z - (krand() & ((32 << 8) - 1)), actor->sector(), tosee->spr.pos.X, tosee->spr.pos.Y, tosee->spr.pos.Z, tosee->sector());

		if (j == 0)
		{
			// they can't see player's holoduke
			// check for player..
			tosee = ps[pnum].GetActor();
		}
	}
	else tosee = ps[pnum].GetActor();	// holoduke not on. look for player

	// can they see player, (or player's holoduke)
	j = cansee(actor->spr.pos.X, actor->spr.pos.Y, actor->spr.pos.Z - (krand() & ((47 << 8))), actor->sector(), tosee->spr.pos.X, tosee->spr.pos.Y, tosee->spr.pos.Z - ((isRR()? 28 : 24) << 8), tosee->sector());

	if (j == 0)
	{
		// search around for target player
		// also modifies 'target' x&y if found.
		j = furthestcanseepoint(actor, tosee, &actor->ovel.X, &actor->ovel.Y);
	}
	else
	{
		// else, they did see it.
		// save where we were looking..
		actor->ovel.X = tosee->spr.pos.X;
		actor->ovel.Y = tosee->spr.pos.Y;
	}

	if (j == 1 && (actor->spr.statnum == STAT_ACTOR || actor->spr.statnum == STAT_STANDABLE))
		actor->timetosleep = SLEEPTIME;

	return j == 1;
}


// int *it = 0x00589a04;

int ParseState::parse(void)
{
	int j, l, s;

	if(killit_flag) return 1;

	switch (*insptr)
	{
	case concmd_ifrnd:
	{
		insptr++;
		// HACK ALERT! The fire animation uses a broken ifrnd setup to delay its start because original CON has no variables.
		// But the chosen random value of 16/255 is too low and can cause delays of a second or more.
		int spnum = g_ac->spr.picnum;
		if (spnum == TILE_FIRE && g_t[4] == 0 && *insptr == 16)
		{
			parseifelse(rnd(64));
			break;
		}
		parseifelse(rnd(*insptr));
		break;
	}
	case concmd_ifcanshoottarget:
		parseifelse(ifcanshoottarget(g_ac, g_p, g_x));
		break;
	case concmd_ifcanseetarget:
		j = cansee(g_ac->spr.pos.X, g_ac->spr.pos.Y, g_ac->spr.pos.Z - ((krand() & 41) << 8), g_ac->sector(), ps[g_p].pos.X, ps[g_p].pos.Y, ps[g_p].pos.Z/*-((krand()&41)<<8)*/, ps[g_p].GetActor()->sector());
		parseifelse(j);
		if (j) g_ac->timetosleep = SLEEPTIME;
		break;
	case concmd_ifnocover:
		j = cansee(g_ac->spr.pos.X, g_ac->spr.pos.Y, g_ac->spr.pos.Z, g_ac->sector(), ps[g_p].pos.X, ps[g_p].pos.Y, ps[g_p].pos.Z, ps[g_p].GetActor()->sector());
		parseifelse(j);
		if (j) g_ac->timetosleep = SLEEPTIME;
		break;

	case concmd_ifactornotstayput:
		parseifelse(g_ac->actorstayput == nullptr);
		break;
	case concmd_ifcansee:
		parseifelse(ifcansee(g_ac, g_p));
		break;

	case concmd_ifhitweapon:
		parseifelse(fi.ifhitbyweapon(g_ac) >= 0);
		break;
	case concmd_ifsquished:
		parseifelse(ifsquished(g_ac, g_p) == 1);
		break;
	case concmd_ifdead:
	{
		j = g_ac->spr.extra;
		if (g_ac->isPlayer())
			j--;
		parseifelse(j < 0);
	}
	break;
	case concmd_ai:
		insptr++;
		g_t[5] = *insptr;
		g_t[4] = ScriptCode[g_t[5]];		  // Action
		g_t[1] = ScriptCode[g_t[5] + 1];		// move
		g_ac->spr.hitag = ScriptCode[g_t[5] + 2];	  // Ai
		g_t[0] = g_t[2] = g_t[3] = 0;
		if (g_ac->spr.hitag & random_angle)
			g_ac->spr.ang = krand() & 2047;
		insptr++;
		break;
	case concmd_action:
		insptr++;
		g_t[2] = 0;
		g_t[3] = 0;
		g_t[4] = *insptr;
		insptr++;
		break;

	case concmd_ifpdistl:
		insptr++;
		parseifelse(g_x < *insptr);
		if (g_x > MAXSLEEPDIST && g_ac->timetosleep == 0)
			g_ac->timetosleep = SLEEPTIME;
		break;
	case concmd_ifpdistg:
		insptr++;
		parseifelse(g_x > * insptr);
		if (g_x > MAXSLEEPDIST && g_ac->timetosleep == 0)
			g_ac->timetosleep = SLEEPTIME;
		break;
	case concmd_else:
		insptr = &ScriptCode[*(insptr + 1)];
		break;
	case concmd_addstrength:
		insptr++;
		g_ac->spr.extra += *insptr;
		insptr++;
		break;
	case concmd_strength:
		insptr++;
		g_ac->spr.extra = *insptr;
		insptr++;
		break;
	case concmd_smacksprite:
		switch (krand() & 1)
		{
		case 0:
			g_ac->spr.ang = (+512 + g_ac->spr.ang + (krand() & 511)) & 2047;
			break;
		case 1:
			g_ac->spr.ang = (-512 + g_ac->spr.ang - (krand() & 511)) & 2047;
			break;
		}
		insptr++;
		break;
	case concmd_fakebubba:
		insptr++;
		fakebubbaspawn(g_ac, g_p);
		break;

	case concmd_rndmove:
		g_ac->spr.ang = krand() & 2047;
		g_ac->spr.xvel = 25;
		insptr++;
		break;
	case concmd_mamatrigger:
		operateactivators(667, g_p);
		insptr++;
		break;
	case concmd_mamaspawn:
		mamaspawn(g_ac);
		insptr++;
		break;
	case concmd_mamaquake:
		if (g_ac->spr.pal == 31)
			earthquaketime = 4;
		else if (g_ac->spr.pal == 32)
			earthquaketime = 6;
		insptr++;
		break;
	case concmd_garybanjo:
		if (banjosound == 0)
		{
			int rnum = (krand() & 3) + 1;
			if (rnum == 4)
			{
				banjosound = 262;
			}
			else if (rnum == 1)
			{
				banjosound = 272;
			}
			else if (rnum == 2)
			{
				banjosound = 273;
			}
			else
			{
				banjosound = 273;
			}
			S_PlayActorSound(banjosound, g_ac, CHAN_WEAPON);
		}
		else if (!S_CheckActorSoundPlaying(g_ac, banjosound))
			S_PlayActorSound(banjosound, g_ac, CHAN_WEAPON);
		insptr++;
		break;
	case concmd_motoloopsnd:
		if (!S_CheckActorSoundPlaying(g_ac, 411))
			S_PlayActorSound(411, g_ac, CHAN_VOICE);
		insptr++;
		break;
	case concmd_ifgotweaponce:
		insptr++;

		if (ud.coop >= 1 && ud.multimode > 1)
		{
			if (*insptr == 0)
			{
				for (j = 0; j < ps[g_p].weapreccnt; j++)
					if (ps[g_p].weaprecs[j] == g_ac->spr.picnum)
						break;

				parseifelse(j < ps[g_p].weapreccnt&& g_ac->GetOwner() == g_ac);
			}
			else if (ps[g_p].weapreccnt < 16)
			{
				ps[g_p].weaprecs[ps[g_p].weapreccnt++] = g_ac->spr.picnum;
				parseifelse(g_ac->GetOwner() == g_ac);
			}
		}
		else parseifelse(0);
		break;
	case concmd_getlastpal:
		insptr++;
		if (g_ac->isPlayer())
			g_ac->spr.pal = ps[g_ac->spr.yvel].palookup;
		else
		{
			// Copied from DukeGDX.
			if (g_ac->spr.picnum == TILE_EGG && g_ac->temp_data[5] == TILE_EGG + 2 && g_ac->spr.pal == 1) 
			{
				ps[connecthead].max_actors_killed++; //revive the egg
				g_ac->temp_data[5] = 0;
			}
			g_ac->spr.pal = (uint8_t)g_ac->tempang;
		}
		g_ac->tempang = 0;
		break;
	case concmd_tossweapon:
		insptr++;
		fi.checkweapons(&ps[g_ac->spr.yvel]);
		break;
	case concmd_nullop:
		insptr++;
		break;
	case concmd_mikesnd:
		insptr++;
		if (!S_CheckActorSoundPlaying(g_ac, g_ac->spr.yvel))
			S_PlayActorSound(g_ac->spr.yvel, g_ac, CHAN_VOICE);
		break;
	case concmd_pkick:
		insptr++;

		if (ud.multimode > 1 && g_ac->isPlayer())
		{
			if (ps[otherp].quick_kick == 0)
				ps[otherp].quick_kick = 14;
		}
		else if (!g_ac->isPlayer() && ps[g_p].quick_kick == 0)
			ps[g_p].quick_kick = 14;
		break;
	case concmd_sizeto:
		insptr++;

		// JBF 20030805: As I understand it, if xrepeat becomes 0 it basically kills the
		// sprite, which is why the "sizeto 0 41" calls in 1.3d became "sizeto 4 41" in
		// 1.4, so instead of patching the CONs I'll surruptitiously patch the code here
		//if (!isPlutoPak() && *insptr == 0) *insptr = 4;

		j = ((*insptr) - g_ac->spr.xrepeat) << 1;
		g_ac->spr.xrepeat += Sgn(j);

		insptr++;

		if ((g_ac->isPlayer() && g_ac->spr.yrepeat < 36) || *insptr < g_ac->spr.yrepeat || ((g_ac->spr.yrepeat * (tileHeight(g_ac->spr.picnum) + 8)) << 2) < (g_ac->floorz - g_ac->ceilingz))
		{
			j = ((*insptr) - g_ac->spr.yrepeat) << 1;
			if (abs(j)) g_ac->spr.yrepeat += Sgn(j);
		}

		insptr++;

		break;
	case concmd_sizeat:
		insptr++;
		g_ac->spr.xrepeat = (uint8_t)*insptr;
		insptr++;
		g_ac->spr.yrepeat = (uint8_t)*insptr;
		insptr++;
		break;
	case concmd_shoot:
		insptr++;
		fi.shoot(g_ac, (short)*insptr);
		insptr++;
		break;
	case concmd_ifsoundid:
		insptr++;
		parseifelse((short)*insptr == ambientlotag[g_ac->spr.ang]);
		break;
	case concmd_ifsounddist:
		insptr++;
		if (*insptr == 0)
			parseifelse(ambienthitag[g_ac->spr.ang] > g_x);
		else if (*insptr == 1)
			parseifelse(ambienthitag[g_ac->spr.ang] < g_x);
		break;
	case concmd_soundtag:
		insptr++;
		S_PlayActorSound(ambientlotag[g_ac->spr.ang], g_ac);
		break;
	case concmd_soundtagonce:
		insptr++;
		if (!S_CheckActorSoundPlaying(g_ac, ambientlotag[g_ac->spr.ang]))
			S_PlayActorSound(ambientlotag[g_ac->spr.ang], g_ac);
		break;
	case concmd_soundonce:
		insptr++;
		if (!S_CheckSoundPlaying(*insptr++))
			S_PlayActorSound(*(insptr - 1), g_ac);
		break;
	case concmd_stopsound:
		insptr++;
		if (S_CheckSoundPlaying(*insptr))
			S_StopSound(*insptr);
		insptr++;
		break;
	case concmd_globalsound:
		insptr++;
		if (g_p == screenpeek || ud.coop == 1)
			S_PlayActorSound(*insptr, ps[screenpeek].GetActor());
		insptr++;
		break;
	case concmd_smackbubba:
		insptr++;
		if (!isRRRA() || g_ac->spr.pal != 105)
		{
			setnextmap(false);
		}
		break;
	case concmd_mamaend:
		insptr++;
		ps[myconnectindex].MamaEnd = 150;
		break;

	case concmd_ifactorhealthg:
		insptr++;
		parseifelse(g_ac->spr.extra > (short)*insptr);
		break;
	case concmd_ifactorhealthl:
		insptr++;
		parseifelse(g_ac->spr.extra < (short)*insptr);
		break;
	case concmd_sound:
		insptr++;
		S_PlayActorSound((short) *insptr,g_ac);
		insptr++;
		break;
	case concmd_tip:
		insptr++;
		ps[g_p].tipincs = 26;
		break;
	case concmd_iftipcow:
	case concmd_ifhittruck: // both have the same code.
		if (g_ac->spriteextra == 1) // 
		{
			j = 1;
			g_ac->spriteextra++;
		}
		else
			j = 0;
		parseifelse(j > 0);
		break;
	case concmd_tearitup:
		insptr++;
		tearitup(g_ac->sector());
		break;
	case concmd_fall:
		insptr++;
		g_ac->spr.xoffset = 0;
		g_ac->spr.yoffset = 0;
		fi.fall(g_ac, g_p);
		break;
	case concmd_enda:
	case concmd_break:
	case concmd_ends:
	case concmd_endevent:
		return 1;
	case concmd_rightbrace:
		insptr++;
		return 1;
	case concmd_addammo:
		insptr++;
		if( ps[g_p].ammo_amount[*insptr] >= gs.max_ammo_amount[*insptr] )
		{
			killit_flag = 2;
			break;
		}
		addammo( *insptr, &ps[g_p], *(insptr+1) );
		if(ps[g_p].curr_weapon == KNEE_WEAPON)
			if( ps[g_p].gotweapon[*insptr] )
				fi.addweapon( &ps[g_p], *insptr );
		insptr += 2;
		break;
	case concmd_money:
		insptr++;
		fi.lotsofmoney(g_ac,*insptr);
		insptr++;
		break;
	case concmd_mail:
		insptr++;
		fi.lotsofmail(g_ac,*insptr);
		insptr++;
		break;
	case concmd_sleeptime:
		insptr++;
		g_ac->timetosleep = (short)*insptr;
		insptr++;
		break;
	case concmd_paper:
		insptr++;
		fi.lotsofpaper(g_ac,*insptr);
		insptr++;
		break;
	case concmd_addkills:
		insptr++;
		if (isRR())
		{
			if (g_ac->spriteextra < 1 || g_ac->spriteextra == 128)
			{
				if (actorfella(g_ac))
					ps[g_p].actors_killed += *insptr;
			}
		}
		else ps[g_p].actors_killed += *insptr;
		g_ac->actorstayput = nullptr;
		insptr++;
		break;
	case concmd_lotsofglass:
		insptr++;
		spriteglass(g_ac, *insptr);
		insptr++;
		break;
	case concmd_killit:
		insptr++;
		killit_flag = 1;
		break;
	case concmd_addweapon:
		insptr++;
		if( ps[g_p].gotweapon[*insptr] == 0 ) fi.addweapon( &ps[g_p], *insptr );
		else if( ps[g_p].ammo_amount[*insptr] >= gs.max_ammo_amount[*insptr] )
		{
				killit_flag = 2;
				break;
		}
		addammo( *insptr, &ps[g_p], *(insptr+1) );
		if(ps[g_p].curr_weapon == KNEE_WEAPON)
			if( ps[g_p].gotweapon[*insptr] )
				fi.addweapon( &ps[g_p], *insptr );
		insptr+=2;
		break;
	case concmd_debug:
		insptr++;
		Printf("%d\n",*insptr);
		insptr++;
		break;
	case concmd_endofgame:
		insptr++;
		ps[g_p].timebeforeexit = *insptr;
		ps[g_p].customexitsound = -1;
		ud.eog = true;
		insptr++;
		break;

	case concmd_isdrunk: // todo: move out to player_r.
		insptr++;
		ps[g_p].drink_amt += *insptr;
		j = ps[g_p].GetActor()->spr.extra;
		if (j > 0)
			j += *insptr;
		if (j > gs.max_player_health * 2)
			j = gs.max_player_health * 2;
		if (j < 0)
			j = 0;

		if (ud.god == 0)
		{
			if (*insptr > 0)
			{
				if ((j - *insptr) < (gs.max_player_health >> 2) &&
					j >= (gs.max_player_health >> 2))
					S_PlayActorSound(DUKE_GOTHEALTHATLOW, ps[g_p].GetActor());

				ps[g_p].last_extra = j;
			}

			ps[g_p].GetActor()->spr.extra = j;
		}
		if (ps[g_p].drink_amt > 100)
			ps[g_p].drink_amt = 100;

		if (ps[g_p].GetActor()->spr.extra >= gs.max_player_health)
		{
			ps[g_p].GetActor()->spr.extra = gs.max_player_health;
			ps[g_p].last_extra = gs.max_player_health;
		}
		insptr++;
		break;
	case concmd_strafeleft:
		insptr++;
		movesprite_ex(g_ac, -bsin(g_ac->spr.ang, -10), bcos(g_ac->spr.ang, -10), g_ac->spr.zvel, CLIPMASK0, coll);
		break;
	case concmd_straferight:
		insptr++;
		movesprite_ex(g_ac, bsin(g_ac->spr.ang, -10), -bcos(g_ac->spr.ang, -10), g_ac->spr.zvel, CLIPMASK0, coll);
		break;
	case concmd_larrybird:
		insptr++;
		ps[g_p].pos.Z = ps[g_p].GetActor()->sector()->ceilingz;
		ps[g_p].GetActor()->spr.pos.Z = ps[g_p].pos.Z;
		break;
	case concmd_destroyit:
		insptr++;
		destroyit(g_ac);
		break;
	case concmd_iseat: // move out to player_r.
		insptr++;
		ps[g_p].eat += *insptr;
		if (ps[g_p].eat > 100)
		{
			ps[g_p].eat = 100;
		}
		ps[g_p].drink_amt -= *insptr;
		if (ps[g_p].drink_amt < 0)
			ps[g_p].drink_amt = 0;
		j = ps[g_p].GetActor()->spr.extra;
		if (g_ac->spr.picnum != TILE_ATOMICHEALTH)
		{
			if (j > gs.max_player_health && *insptr > 0)
			{
				insptr++;
				break;
			}
			else
			{
				if (j > 0)
					j += (*insptr) * 3;
				if (j > gs.max_player_health && *insptr > 0)
					j = gs.max_player_health;
			}
		}
		else
		{
			if (j > 0)
				j += *insptr;
			if (j > (gs.max_player_health << 1))
				j = (gs.max_player_health << 1);
		}

		if (j < 0) j = 0;

		if (ud.god == 0)
		{
			if (*insptr > 0)
			{
				if ((j - *insptr) < (gs.max_player_health >> 2) &&
					j >= (gs.max_player_health >> 2))
					S_PlayActorSound(229, ps[g_p].GetActor());

				ps[g_p].last_extra = j;
			}

			ps[g_p].GetActor()->spr.extra = j;
		}

		insptr++;
		break;

	case concmd_addphealth: // todo: move out to player.
		insptr++;

		if(!isRR() && ps[g_p].newOwner != nullptr)
		{
			ps[g_p].newOwner = nullptr;
			ps[g_p].pos.X = ps[g_p].opos.X;
			ps[g_p].pos.Y = ps[g_p].opos.Y;
			ps[g_p].pos.Z = ps[g_p].opos.Z;
			ps[g_p].angle.restore();
			updatesector(ps[g_p].pos.X,ps[g_p].pos.Y,&ps[g_p].cursector);

			DukeStatIterator it(STAT_ACTOR);
			while (auto actj = it.Next())
			{
				if (actorflag(actj, SFLAG2_CAMERA))
					actj->spr.yvel = 0;
			}
		}

		j = ps[g_p].GetActor()->spr.extra;

		if(g_ac->spr.picnum != TILE_ATOMICHEALTH)
		{
			if( j > gs.max_player_health && *insptr > 0 )
			{
				insptr++;
				break;
			}
			else
			{
				if(j > 0)
					j += *insptr;
				if ( j > gs.max_player_health && *insptr > 0 )
					j = gs.max_player_health;
			}
		}
		else
		{
			if( j > 0 )
				j += *insptr;
			if ( j > (gs.max_player_health<<1) )
				j = (gs.max_player_health<<1);
		}

		if(j < 0) j = 0;

		if(ud.god == 0)
		{
			if(*insptr > 0)
			{
				if( ( j - *insptr ) < (gs.max_player_health>>2) &&
					j >= (gs.max_player_health>>2) )
						S_PlayActorSound(isRR()? 229 : DUKE_GOTHEALTHATLOW,ps[g_p].GetActor());

				ps[g_p].last_extra = j;
			}

			ps[g_p].GetActor()->spr.extra = j;
		}

		insptr++;
		break;

	case concmd_state:
		{
			auto tempscrptr = insptr + 2;
			insptr = &ScriptCode[*(insptr + 1)];
			while (1) if (parse()) break;
			insptr = tempscrptr;
		}
		break;
	case concmd_leftbrace:
		insptr++;
		while (1) if (parse()) break;
		break;
	case concmd_move:
		g_t[0]=0;
		insptr++;
		g_t[1] = *insptr;
		insptr++;
		g_ac->spr.hitag = *insptr;
		insptr++;
		if(g_ac->spr.hitag&random_angle)
			g_ac->spr.ang = krand()&2047;
		break;
	case concmd_spawn:
		insptr++;
		if(g_ac->insector())
			spawn(g_ac,*insptr);
		insptr++;
		break;
	case concmd_ifwasweapon:
	case concmd_ifspawnedby:	// these two are the same
		insptr++;
		parseifelse( g_ac->attackertype == *insptr);
		break;
	case concmd_ifai:
		insptr++;
		parseifelse(g_t[5] == *insptr);
		break;
	case concmd_ifaction:
		insptr++;
		parseifelse(g_t[4] == *insptr);
		break;
	case concmd_ifactioncount:
		insptr++;
		parseifelse(g_t[2] >= *insptr);
		break;
	case concmd_resetactioncount:
		insptr++;
		g_t[2] = 0;
		break;
	case concmd_debris:
	{
			int dnum;

			insptr++;
			dnum = *insptr;
			insptr++;
			bool weap = fi.spawnweapondebris(g_ac->spr.picnum, dnum);

			if(g_ac->insector())
				for(j=(*insptr)-1;j>=0;j--)
			{
				if(weap)
					s = 0;
				else s = (krand()%3);

				auto spawned = EGS(g_ac->sector(),
					g_ac->spr.pos.X + (krand() & 255) - 128, g_ac->spr.pos.Y + (krand() & 255) - 128, g_ac->spr.pos.Z - (8 << 8) - (krand() & 8191),
					dnum + s, g_ac->spr.shade, 32 + (krand() & 15), 32 + (krand() & 15),
					krand() & 2047, (krand() & 127) + 32, -(krand() & 2047), g_ac, 5);
				if (spawned)
				{
					if (weap)
						spawned->spr.yvel = gs.weaponsandammosprites[j % 14];
					else spawned->spr.yvel = -1;
					spawned->spr.pal = g_ac->spr.pal;
				}
			}
			insptr++;
		}
		break;
	case concmd_count:
		insptr++;
		g_t[0] = (short) *insptr;
		insptr++;
		break;
	case concmd_cstator:
		insptr++;
		g_ac->spr.cstat |= ESpriteFlags::FromInt(*insptr);
		insptr++;
		break;
	case concmd_clipdist:
		insptr++;
		g_ac->spr.clipdist = (uint8_t) *insptr;
		insptr++;
		break;
	case concmd_cstat:
		insptr++;
		g_ac->spr.cstat = ESpriteFlags::FromInt(*insptr);
		insptr++;
		break;
	case concmd_newpic:
		insptr++;
		g_ac->spr.picnum = (short)*insptr;
		insptr++;
		break;
	case concmd_ifmove:
		insptr++;
		parseifelse(g_t[1] == *insptr);
		break;
	case concmd_resetplayer:
		insptr++;

		if(ud.multimode < 2)
		{
			gameaction = ga_autoloadgame;
			killit_flag = 2;
		}
		else
		{
			// I am not convinced this is even remotely smart to be executed from here..
			pickrandomspot(g_p);
			g_ac->spr.pos.X = ps[g_p].bobpos.X = ps[g_p].opos.X = ps[g_p].pos.X;
			g_ac->spr.pos.Y = ps[g_p].bobpos.Y = ps[g_p].opos.Y = ps[g_p].pos.Y;
			g_ac->spr.pos.Z = ps[g_p].opos.Z = ps[g_p].pos.Z;
			g_ac->backuppos();
			updatesector(ps[g_p].pos.X, ps[g_p].pos.Y, &ps[g_p].cursector);
			SetActor(ps[g_p].GetActor(), { ps[g_p].pos.X, ps[g_p].pos.Y, ps[g_p].pos.Z + gs.playerheight });
			g_ac->spr.cstat = CSTAT_SPRITE_BLOCK_ALL;

			g_ac->spr.shade = -12;
			g_ac->spr.clipdist = 64;
			g_ac->spr.xrepeat = 42;
			g_ac->spr.yrepeat = 36;
			g_ac->SetOwner(g_ac);
			g_ac->spr.xoffset = 0;
			g_ac->spr.pal = ps[g_p].palookup;

			ps[g_p].last_extra = g_ac->spr.extra = gs.max_player_health;
			ps[g_p].wantweaponfire = -1;
			ps[g_p].horizon.ohoriz = ps[g_p].horizon.horiz = q16horiz(0);
			ps[g_p].on_crane = nullptr;
			ps[g_p].frag_ps = g_p;
			ps[g_p].horizon.ohorizoff = ps[g_p].horizon.horizoff = q16horiz(0);
			ps[g_p].opyoff = 0;
			ps[g_p].wackedbyactor = nullptr;
			ps[g_p].shield_amount = gs.max_armour_amount;
			ps[g_p].dead_flag = 0;
			ps[g_p].resurrected = false;
			ps[g_p].pals.a = 0;
			ps[g_p].footprintcount = 0;
			ps[g_p].weapreccnt = 0;
			ps[g_p].ftq = 0;
			ps[g_p].vel.X = ps[g_p].vel.Y = 0;
			if (!isRR()) ps[g_p].angle.orotscrnang = ps[g_p].angle.rotscrnang = buildang(0);

			ps[g_p].falling_counter = 0;

			g_ac->hitextra = -1;

			g_ac->cgg = 0;
			g_ac->movflag = 0;
			g_ac->tempang = 0;
			g_ac->actorstayput = nullptr;
			g_ac->dispicnum = 0;
			g_ac->SetHitOwner(ps[g_p].GetActor());
			g_ac->temp_data[4] = 0;

			resetinventory(g_p);
			resetweapons(g_p);
		}
		break;
	case concmd_ifcoop:
		parseifelse(ud.coop || numplayers > 2);
		break;
	case concmd_ifonmud:
		parseifelse(abs(g_ac->spr.pos.Z - g_ac->sector()->floorz) < (32 << 8) && g_ac->sector()->floorpicnum == 3073); // eew, hard coded tile numbers.. :?
		break;
	case concmd_ifonwater:
		parseifelse( abs(g_ac->spr.pos.Z-g_ac->sector()->floorz) < (32<<8) && g_ac->sector()->lotag == ST_1_ABOVE_WATER);
		break;
	case concmd_ifmotofast:
		parseifelse(ps[g_p].MotoSpeed > 60);
		break;
	case concmd_ifonmoto:
		parseifelse(ps[g_p].OnMotorcycle == 1);
		break;
	case concmd_ifonboat:
		parseifelse(ps[g_p].OnBoat == 1);
		break;
	case concmd_ifsizedown:
		g_ac->spr.xrepeat--;
		g_ac->spr.yrepeat--;
		parseifelse(g_ac->spr.xrepeat <= 5);
		break;
	case concmd_ifwind:
		parseifelse(WindTime > 0);
		break;

	case concmd_ifinwater:
		parseifelse( g_ac->sector()->lotag == 2);
		break;
	case concmd_ifcount:
		insptr++;
		parseifelse(g_t[0] >= *insptr);
		break;
	case concmd_ifactor:
		insptr++;
		parseifelse(g_ac->spr.picnum == *insptr);
		break;
	case concmd_resetcount:
		insptr++;
		g_t[0] = 0;
		break;
	case concmd_addinventory:
		insptr+=2;
		switch(*(insptr-1))
		{
			case 0:
				ps[g_p].steroids_amount = *insptr;
				ps[g_p].inven_icon = 2;
				break;
			case 1:
				ps[g_p].shield_amount +=		  *insptr;// 100;
				if(ps[g_p].shield_amount > gs.max_player_health)
					ps[g_p].shield_amount = gs.max_player_health;
				break;
			case 2:
				ps[g_p].scuba_amount =			   *insptr;// 1600;
				ps[g_p].inven_icon = 6;
				break;
			case 3:
				ps[g_p].holoduke_amount =		   *insptr;// 1600;
				ps[g_p].inven_icon = 3;
				break;
			case 4:
				ps[g_p].jetpack_amount =		   *insptr;// 1600;
				ps[g_p].inven_icon = 4;
				break;
			case 6:
				if (isRR())
				{
					switch (g_ac->spr.lotag)
					{
					case 100: ps[g_p].keys[1] = 1; break;
					case 101: ps[g_p].keys[2] = 1; break;
					case 102: ps[g_p].keys[3] = 1; break;
					case 103: ps[g_p].keys[4] = 1; break;
					}
				}
				else
				{
					switch (g_ac->spr.pal)
					{
					case  0: ps[g_p].got_access |= 1; break;
					case 21: ps[g_p].got_access |= 2; break;
					case 23: ps[g_p].got_access |= 4; break;
					}
				}
				break;
			case 7:
				ps[g_p].heat_amount = *insptr;
				ps[g_p].inven_icon = 5;
				break;
			case 9:
				ps[g_p].inven_icon = 1;
				ps[g_p].firstaid_amount = *insptr;
				break;
			case 10:
				ps[g_p].inven_icon = 7;
				ps[g_p].boot_amount = *insptr;
				break;
		}
		insptr++;
		break;
	case concmd_hitradius:
		fi.hitradius(g_ac, *(insptr + 1), *(insptr + 2), *(insptr + 3), *(insptr + 4), *(insptr + 5));
		insptr+=6;
		break;
	case concmd_ifp:
	{
			insptr++;

			l = *insptr;
			j = 0;

			s = g_ac->spr.xvel;

			// sigh.. this was yet another place where number literals were used as bit masks for every single value, making the code totally unreadable.
			if( (l& pducking) && ps[g_p].on_ground && PlayerInput(g_p, SB_CROUCH))
					j = 1;
			else if( (l& pfalling) && ps[g_p].jumping_counter == 0 && !ps[g_p].on_ground &&	ps[g_p].vel.Z > 2048 )
					j = 1;
			else if( (l& pjumping) && ps[g_p].jumping_counter > 348 )
					j = 1;
			else if( (l& pstanding) && s >= 0 && s < 8)
					j = 1;
			else if( (l& pwalking) && s >= 8 && !(PlayerInput(g_p, SB_RUN)) )
					j = 1;
			else if( (l& prunning) && s >= 8 && PlayerInput(g_p, SB_RUN) )
					j = 1;
			else if( (l& phigher) && ps[g_p].pos.Z < (g_ac->spr.pos.Z-(48<<8)) )
					j = 1;
			else if( (l& pwalkingback) && s <= -8 && !(PlayerInput(g_p, SB_RUN)) )
					j = 1;
			else if( (l& prunningback) && s <= -8 && (PlayerInput(g_p, SB_RUN)) )
					j = 1;
			else if( (l& pkicking) && ( ps[g_p].quick_kick > 0 || ( ps[g_p].curr_weapon == KNEE_WEAPON && ps[g_p].kickback_pic > 0 ) ) )
					j = 1;
			else if( (l& pshrunk) && ps[g_p].GetActor()->spr.xrepeat < (isRR() ? 8 : 32))
					j = 1;
			else if( (l& pjetpack) && ps[g_p].jetpack_on )
					j = 1;
			else if( (l& ponsteroids) && ps[g_p].steroids_amount > 0 && ps[g_p].steroids_amount < 400 )
					j = 1;
			else if( (l& ponground) && ps[g_p].on_ground)
					j = 1;
			else if( (l& palive) && ps[g_p].GetActor()->spr.xrepeat > (isRR() ? 8 : 32) && ps[g_p].GetActor()->spr.extra > 0 && ps[g_p].timebeforeexit == 0 )
					j = 1;
			else if( (l& pdead) && ps[g_p].GetActor()->spr.extra <= 0)
					j = 1;
			else if( (l& pfacing) )
			{
				if (g_ac->isPlayer() && ud.multimode > 1)
					j = getincangle(ps[otherp].angle.ang.asbuild(), getangle(ps[g_p].pos.X - ps[otherp].pos.X, ps[g_p].pos.Y - ps[otherp].pos.Y));
				else
					j = getincangle(ps[g_p].angle.ang.asbuild(), getangle(g_ac->spr.pos.X - ps[g_p].pos.X, g_ac->spr.pos.Y - ps[g_p].pos.Y));

				if( j > -128 && j < 128 )
					j = 1;
				else
					j = 0;
			}

			parseifelse( j);

		}
		break;
	case concmd_ifstrength:
		insptr++;
		parseifelse(g_ac->spr.extra <= *insptr);
		break;
	case concmd_guts:
		insptr += 2;
		fi.guts(g_ac,*(insptr-1),*insptr,g_p);
		insptr++;
		break;
	case concmd_slapplayer:
		insptr++;
		forceplayerangle(g_p);
		ps[g_p].vel.X -= ps[g_p].angle.ang.bcos(7);
		ps[g_p].vel.Y -= ps[g_p].angle.ang.bsin(7);
		return 0;
	case concmd_wackplayer:
		insptr++;
		if (!isRR())
			forceplayerangle(g_p);
		else
		{
			ps[g_p].vel.X -= ps[g_p].angle.ang.bcos(10);
			ps[g_p].vel.Y -= ps[g_p].angle.ang.bsin(10);
			ps[g_p].jumping_counter = 767;
			ps[g_p].jumping_toggle = 1;
		}
		return 0;
	case concmd_ifgapzl:
		insptr++;
		parseifelse( (( g_ac->floorz - g_ac->ceilingz ) >> 8 ) < *insptr);
		break;
	case concmd_ifhitspace:
		parseifelse(PlayerInput(g_p, SB_OPEN));
		break;
	case concmd_ifoutside:
		parseifelse(g_ac->sector()->ceilingstat & CSTAT_SECTOR_SKY);
		break;
	case concmd_ifmultiplayer:
		parseifelse(ud.multimode > 1);
		break;
	case concmd_operate:
		insptr++;
		if( g_ac->sector()->lotag == 0 )
		{
			HitInfo hit{};
			neartag({ g_ac->spr.pos.X, g_ac->spr.pos.Y, g_ac->spr.pos.Z - (32 << 8) }, g_ac->sector(), g_ac->spr.ang, hit, 768, 1);
			auto sectp = hit.hitSector;
			if (sectp)
			{
				if (isanearoperator(sectp->lotag))
					if ((sectp->lotag & 0xff) == ST_23_SWINGING_DOOR || sectp->floorz == sectp->ceilingz)
						if ((sectp->lotag & 16384) == 0 && (sectp->lotag & 32768) == 0)
						{
							DukeSectIterator it(sectp);
							DDukeActor* a2;
							while ((a2 = it.Next()))
							{
								if (a2->spr.picnum == ACTIVATOR)
									break;
							}
							if (a2 == nullptr)
								operatesectors(sectp, g_ac);
						}
			}
		}
		break;
	case concmd_ifinspace:
		parseifelse(fi.ceilingspace(g_ac->sector()));
		break;

	case concmd_spritepal:
		insptr++;
		if(!g_ac->isPlayer())
			g_ac->tempang = g_ac->spr.pal;
		g_ac->spr.pal = *insptr;
		insptr++;
		break;

	case concmd_cactor:
		insptr++;
		g_ac->spr.picnum = *insptr;
		insptr++;
		break;

	case concmd_ifbulletnear:
		parseifelse( dodge(g_ac) == 1);
		break;
	case concmd_ifrespawn:
		if( badguy(g_ac) )
			parseifelse( ud.respawn_monsters );
		else if( inventory(g_ac) )
			parseifelse( ud.respawn_inventory );
		else
			parseifelse( ud.respawn_items );
		break;
	case concmd_iffloordistl:
		insptr++;
		parseifelse( (g_ac->floorz - g_ac->spr.pos.Z) <= ((*insptr)<<8));
		break;
	case concmd_ifceilingdistl:
		insptr++;
		parseifelse( ( g_ac->spr.pos.Z - g_ac->ceilingz ) <= ((*insptr)<<8));
		break;
	case concmd_palfrom:
		insptr++;
		SetPlayerPal(&ps[g_p], PalEntry(insptr[0], insptr[1], insptr[2], insptr[3]));
		insptr += 4;
		break;

/*		  case 74:
		insptr++;
		getglobalz(g_ac);
		parseifelse( (( g_ac->floorz - g_ac->ceilingz ) >> 8 ) >= *insptr);
		break;
*/
	case concmd_addlog:
	{	int instr;
		int lFile;
		insptr++;
		lFile=*(insptr++);	// file
		instr=*(insptr++);	// line
		// this was only printing file name and line number as debug output.
		break;
	}
	case concmd_addlogvar:
	{	int instr;
		int lFile;
		insptr++;
		lFile=*(insptr++);	// file
		instr=*(insptr++);	// l=Line number, *instpr=varID
		if( (*insptr >= iGameVarCount)
			|| *insptr < 0
			)
		{
			// invalid varID
			insptr++;
			break;	// out of switch
		}
		DPrintf(DMSG_NOTIFY, "ADDLOGVAR: ");

		if( aGameVars[*insptr].dwFlags & GAMEVAR_FLAG_READONLY)
		{
			DPrintf(DMSG_NOTIFY, " (read-only)");
		}
		if( aGameVars[*insptr].dwFlags & GAMEVAR_FLAG_PERPLAYER)
		{
			DPrintf(DMSG_NOTIFY, " (Per Player. Player=%d)",g_p);
		}
		else if( aGameVars[*insptr].dwFlags & GAMEVAR_FLAG_PERACTOR)
		{
			DPrintf(DMSG_NOTIFY, " (Per Actor. Actor=%p)",g_ac);
		}
		else
		{
			DPrintf(DMSG_NOTIFY, " (Global)");
		}
		DPrintf(DMSG_NOTIFY, " =%d",	GetGameVarID(*insptr, g_ac, g_p).safeValue());
		insptr++;
		break;
	}
	case concmd_setvar:
	{	int i;
		insptr++;
		i=*(insptr++);	// ID of def
		SetGameVarID(i, *insptr, g_ac, g_p );
		insptr++;
		break;
	}
	case concmd_setvarvar:
	{	int i;
		insptr++;
		i=*(insptr++);	// ID of def
		SetGameVarID(i, GetGameVarID(*insptr, g_ac, g_p), g_ac, g_p );
//			aGameVars[i].lValue = aGameVars[*insptr].lValue;
		insptr++;
		break;
	}
	case concmd_addvar:
	{	int i;		
		insptr++;
		i=*(insptr++);	// ID of def
		SetGameVarID(i, GetGameVarID(i, g_ac, g_p).safeValue() + *insptr, g_ac, g_p );
		insptr++;
		break;
	}

	case concmd_addvarvar:
	{	int i;
		insptr++;
		i=*(insptr++);	// ID of def
		SetGameVarID(i, GetGameVarID(i, g_ac, g_p).safeValue() + GetGameVarID(*insptr, g_ac, g_p).safeValue(), g_ac, g_p );
		insptr++;
		break;
	}
	case concmd_ifvarvare:
	{
		int i;
		insptr++;
		i=*(insptr++);	// ID of def
		j=0;
		if(GetGameVarID(i, g_ac, g_p).safeValue() == GetGameVarID(*(insptr), g_ac, g_p).safeValue())
		{
			j=1;
		}
		parseifelse( j );
		break;
	}
	case concmd_ifvarvarg:
	{
		int i;
		insptr++;
		i=*(insptr++);	// ID of def
		j=0;
		if(GetGameVarID(i, g_ac, g_p).safeValue() > GetGameVarID(*(insptr), g_ac, g_p).safeValue())
		{
			j=1;
		}
		parseifelse( j );
		break;
	}
	case concmd_ifvarvarl:
	{
		int i;
		insptr++;
		i=*(insptr++);	// ID of def
		j=0;
		if(GetGameVarID(i, g_ac, g_p).safeValue() < GetGameVarID(*(insptr), g_ac, g_p).safeValue())
		{
			j=1;
		}
		parseifelse( j );
		break;
	}
	case concmd_ifvare:
	{
		int i;
		insptr++;
		i=*(insptr++);	// ID of def
		j=0;
		if(GetGameVarID(i, g_ac, g_p).safeValue() == *insptr)
		{
			j=1;
		}
		parseifelse( j );
		break;
	}
	case concmd_ifvarg:
	{
		int i;
		insptr++;
		i=*(insptr++);	// ID of def
		j=0;
		if(GetGameVarID(i, g_ac, g_p).safeValue() > *insptr)
		{
			j=1;
		}
		parseifelse( j );
		break;
	}
	case concmd_ifvarl:
	{
		int i;
		insptr++;
		i=*(insptr++);	// ID of def
		j=0;
		if(GetGameVarID(i, g_ac, g_p).safeValue() < *insptr)
		{
			j=1;
		}
		parseifelse( j );
		break;
	}
	case concmd_ifphealthl:
		insptr++;
		parseifelse( ps[g_p].GetActor()->spr.extra < *insptr);
		break;

	case concmd_ifpinventory:
	{
			insptr++;
			j = 0;
			switch(*(insptr++))
			{
				case 0:
					if( ps[g_p].steroids_amount != *insptr)
						j = 1;
					break;
				case 1:
					if(ps[g_p].shield_amount != gs.max_player_health )
						j = 1;
					break;
				case 2:
					if(ps[g_p].scuba_amount != *insptr) j = 1;
					break;
				case 3:
					if(ps[g_p].holoduke_amount != *insptr) j = 1;
					break;
				case 4:
					if(ps[g_p].jetpack_amount != *insptr) j = 1;
					break;
				case 6:
					if (isRR())
					{
						switch (g_ac->spr.lotag)
						{
						case 100: 
							if (ps[g_p].keys[1]) j = 1; 
							break;
						case 101: 
							if (ps[g_p].keys[2]) j = 1; 
							break;
						case 102: 
							if (ps[g_p].keys[3]) j = 1; 
							break;
						case 103: 
							if (ps[g_p].keys[4]) j = 1; 
							break;
						}
					}
					else
					{
						switch (g_ac->spr.pal)
						{
						case  0: 
							if (ps[g_p].got_access & 1) j = 1; 
							break;
						case 21: 
							if (ps[g_p].got_access & 2) j = 1; 
							break;
						case 23: 
							if (ps[g_p].got_access & 4) j = 1; 
							break;
						}
					}
					break;
				case 7:
					if(ps[g_p].heat_amount != *insptr) j = 1;
					break;
				case 9:
					if(ps[g_p].firstaid_amount != *insptr) j = 1;
					break;
				case 10:
					if(ps[g_p].boot_amount != *insptr) j = 1;
					break;
			}

			parseifelse(j);
			break;
		}
	case concmd_pstomp:
		insptr++;
		if( ps[g_p].knee_incs == 0 && ps[g_p].GetActor()->spr.xrepeat >= (isRR()? 9: 40) )
			if( cansee(g_ac->spr.pos.X,g_ac->spr.pos.Y,g_ac->spr.pos.Z-(4<<8),g_ac->sector(),ps[g_p].pos.X,ps[g_p].pos.Y,ps[g_p].pos.Z+(16<<8),ps[g_p].GetActor()->sector()) )
		{
			ps[g_p].knee_incs = 1;
			if(ps[g_p].weapon_pos == 0)
				ps[g_p].weapon_pos = -1;
			ps[g_p].actorsqu = g_ac;
		}
		break;
	case concmd_ifawayfromwall:
	{
		auto s1 = g_ac->sector();

		j = 0;

		updatesector(g_ac->spr.pos.X + 108, g_ac->spr.pos.Y + 108, &s1);
		if (s1 == g_ac->sector())
		{
			updatesector(g_ac->spr.pos.X - 108, g_ac->spr.pos.Y - 108, &s1);
			if (s1 == g_ac->sector())
			{
				updatesector(g_ac->spr.pos.X + 108, g_ac->spr.pos.Y - 108, &s1);
				if (s1 == g_ac->sector())
				{
					updatesector(g_ac->spr.pos.X - 108, g_ac->spr.pos.Y + 108, &s1);
					if (s1 == g_ac->sector())
						j = 1;
				}
			}
		}
		parseifelse(j);
		break;
	}

	case concmd_quote:
		insptr++;
		FTA(*insptr,&ps[g_p]);
		insptr++;
		break;
	case concmd_ifinouterspace:
		parseifelse( fi.floorspace(g_ac->sector()));
		break;
	case concmd_ifnotmoving:
		parseifelse( (g_ac->movflag&kHitTypeMask) > kHitSector );
		break;
	case concmd_respawnhitag:
		insptr++;
		fi.respawnhitag(g_ac);
		break;
	case concmd_ifspritepal:
		insptr++;
		parseifelse( g_ac->spr.pal == *insptr);
		break;

	case concmd_ifangdiffl:
		insptr++;
		j = abs(getincangle(ps[g_p].angle.ang.asbuild(),g_ac->spr.ang));
		parseifelse( j <= *insptr);
		break;

	case concmd_ifnosounds:
		parseifelse(!S_CheckAnyActorSoundPlaying(g_ac));
		break;

	case concmd_ifplaybackon: //Twentieth Anniversary World Tour
		parseifelse(false);
		break;

	case concmd_espawnvar:
	{
		DDukeActor* lReturn = nullptr;
		int lIn;
		insptr++;

		lIn = *insptr++;
		lIn = GetGameVarID(lIn, g_ac, g_p).safeValue();
		if(g_ac->insector())
			lReturn = spawn(g_ac, lIn);

		SetGameVarID(g_iReturnVarID, (lReturn), g_ac, g_p);
		break;
	}
	case concmd_espawn:
	{
		DDukeActor* lReturn = nullptr;
		insptr++;
		if(g_ac->insector())
			lReturn = spawn(g_ac, *insptr);
		insptr++;
		SetGameVarID(g_iReturnVarID, (lReturn), g_ac, g_p);
		break;
	}
	case concmd_setsector:
	case concmd_getsector:
	{
		// syntax [gs]etsector[<var>].x <VAR>
		// <varid> <xxxid> <varid>
		int lLabelID;
		int lVar1, lVar2;
		int lWhat;
		int lParm2;

		lWhat = *(insptr++);
		lVar1 = *(insptr++);
		lLabelID = *(insptr++);
		lParm2 = *(insptr++);
		lVar2 = *(insptr++);
		DoSector(lWhat == concmd_setsector, lVar1, lLabelID, lVar2, g_ac, g_p, lParm2);
		break;
	}
	case concmd_sqrt:
	{
		// syntax sqrt <invar> <outvar>

		int lInVarID;
		int lOutVarID;
		int lIn;

		insptr++;
		lInVarID = *(insptr++);
		lOutVarID = *(insptr++);
		lIn = GetGameVarID(lInVarID, g_ac, g_p).safeValue();
		SetGameVarID(lOutVarID, ksqrt(lIn), g_ac, g_p);
		break;
	}
	case concmd_findnearactor:
	{
		// syntax findnearactorvar <type> <maxdist> <getvar>
		// gets the sprite ID of the nearest actor within max dist
		// that is of <type> into <getvar>
		// -1 for none found
		// <type> <maxdist> <varid>
		int lType;
		int lMaxDist;
		int lVarID;
		int lTemp;
		int lDist;

		insptr++;

		lType = *(insptr++);
		lMaxDist = *(insptr++);
		lVarID = *(insptr++);

		DDukeActor* lFound = nullptr;
		lDist = 32767;	// big number

		DukeStatIterator it(STAT_ACTOR);
		while (auto actj = it.Next())
		{
			if (actj->spr.picnum == lType)
			{
				lTemp = ldist(g_ac, actj);
				if (lTemp < lMaxDist)
				{
					if (lTemp < lDist)
					{
						lFound = actj;
					}
				}

			}
		}
		SetGameVarID(lVarID, (lFound), g_ac, g_p);

		break;
	}
	case concmd_findnearactorvar:
	{
		// syntax findnearactorvar <type> <maxdistvar> <getvar>
		// gets the sprite ID of the nearest actor within max dist
		// that is of <type> into <getvar>
		// -1 for none found
		// <type> <maxdistvarid> <varid>
		int lType;
		int lMaxDistVar;
		int lMaxDist;
		int lVarID;
		int lTemp;
		int lDist;

		insptr++;

		lType = *(insptr++);
		lMaxDistVar = *(insptr++);
		lVarID = *(insptr++);
		lMaxDist = GetGameVarID(lMaxDistVar, g_ac, g_p).safeValue();
		DDukeActor* lFound = nullptr;
		lDist = 32767;	// big number

		DukeStatIterator it(STAT_ACTOR);
		while (auto actj = it.Next())
		{
			if (actj->spr.picnum == lType)
			{
				lTemp = ldist(g_ac, actj);
				if (lTemp < lMaxDist)
				{
					if (lTemp < lDist)
					{
						lFound = actj;
					}
				}

			}
		}
		SetGameVarID(lVarID, (lFound), g_ac, g_p);

		break;
	}
	case concmd_setplayer:
	case concmd_getplayer:
	{
		// syntax [gs]etplayer[<var>].x <VAR>
		// <varid> <xxxid> <varid>
		int lLabelID;
		int lVar1, lVar2;
		int lWhat;
		int lParm2;

		lWhat = *(insptr++);
		lVar1 = *(insptr++);
		lLabelID = *(insptr++);
		lParm2 = *(insptr++);
		lVar2 = *(insptr++);

		DoPlayer(lWhat == concmd_setplayer, lVar1, lLabelID, lVar2, g_ac, g_p, lParm2);
		break;
	}
	case concmd_getuserdef:
	case concmd_setuserdef:
	{
		// syntax [gs]etuserdef.xxx <VAR>
		//  <xxxid> <varid>
		int lLabelID;
		int lVar1, lVar2;
		int lWhat;
		int lParm2;

		lWhat = *(insptr++);
		lVar1 = -1;
		lLabelID = *(insptr++);
		lParm2 = *(insptr++);
		lVar2 = *(insptr++);

		DoUserDef(lWhat == concmd_setuserdef, lVar1, lLabelID, lVar2, g_ac, g_p, lParm2);
		break;
	}
	case concmd_setwall:
	case concmd_getwall:
	{
		// syntax [gs]etwall[<var>].x <VAR>
		// <varid> <xxxid> <varid>
		int lLabelID;
		int lVar1, lVar2;
		int lWhat;
		int lParm2;

		lWhat = *(insptr++);
		lVar1 = *(insptr++);
		lLabelID = *(insptr++);
		lParm2 = *(insptr++);
		lVar2 = *(insptr++);

		DoWall(lWhat == concmd_setwall, lVar1, lLabelID, lVar2, g_ac, g_p, lParm2);
		break;
	}
	case concmd_setactorvar:
	{
		// syntax [gs]etactorvar[<var>].<varx> <VAR>
		// gets the value of the per-actor variable varx into VAR
		// <var> <varx> <VAR>
		int lVar1, lVar2, lVar3;

		insptr++;

		lVar1 = *(insptr++);
		lVar2 = *(insptr++);
		lVar3 = *(insptr++);

		auto lSprite = GetGameVarID(lVar1, g_ac, g_p);
		if (lSprite.isActor())
		{
			auto lTemp = GetGameVarID(lVar3, g_ac, g_p);
			SetGameVarID(lVar2, lTemp, lSprite.actor(), g_p);
		}

		break;
	}
	case concmd_getactorvar:
	{
		// syntax [gs]etactorvar[<var>].<varx> <VAR>
		// gets the value of the per-actor variable varx into VAR
		// <var> <varx> <VAR>
		int lVar1, lVar2, lVar3;

		insptr++;

		lVar1 = *(insptr++);
		lVar2 = *(insptr++);
		lVar3 = *(insptr++);

		auto lSprite = GetGameVarID(lVar1, g_ac, g_p);
		if (lSprite.isActor())
		{
			auto lTemp = GetGameVarID(lVar2, lSprite.actor(), g_p);
			SetGameVarID(lVar3, lTemp, g_ac, g_p);
		}

		break;
	}
	case concmd_setactor:
	case concmd_getactor:
	{
		// syntax [gs]etactor[<var>].x <VAR>
		// <varid> <xxxid> <varid>
		int lLabelID;
		int lVar1, lVar2;
		int lWhat;
		int lParm2;

		lWhat = *(insptr++);
		lVar1 = *(insptr++);
		lLabelID = *(insptr++);
		lParm2 = *(insptr++);
		lVar2 = *(insptr++);

		DoActor(lWhat == concmd_setactor, lVar1, lLabelID, lVar2, g_ac, g_p, lParm2);
		break;
	}
	case concmd_getangletotarget:
	{
		int i;
		int ang;

		insptr++;
		i = *(insptr++);	// ID of def

		// g_ac->lastvx and lastvy are last known location of target.
		ang = getangle(g_ac->ovel.X - g_ac->spr.pos.X, g_ac->ovel.Y - g_ac->spr.pos.Y);
		SetGameVarID(i, ang, g_ac, g_p);
		break;
	}
	case concmd_lockplayer:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		ps[g_p].transporter_hold = GetGameVarID(i, g_ac, g_p).safeValue();
		break;
	}
	case concmd_getplayerangle:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		SetGameVarID(i, ps[g_p].angle.ang.asbuild(), g_ac, g_p);
		break;
	}
	case concmd_setplayerangle:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		ps[g_p].angle.ang = buildang(GetGameVarID(i, g_ac, g_p).safeValue() & 2047);
		break;
	}
	case concmd_getactorangle:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		SetGameVarID(i, g_ac->spr.ang, g_ac, g_p);
		break;
	}
	case concmd_setactorangle:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		g_ac->spr.ang = GetGameVarID(i, g_ac, g_p).safeValue();
		g_ac->spr.ang &= 2047;
		break;
	}
	case concmd_randvar:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		SetGameVarID(i, MulScale(rand(), *insptr, 15), g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_mulvar:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		SetGameVarID(i, GetGameVarID(i, g_ac, g_p).safeValue()* (*insptr), g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_divvar:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		if ((*insptr) == 0)
		{
			I_Error("Divide by Zero in CON.");
		}
		SetGameVarID(i, GetGameVarID(i, g_ac, g_p).safeValue() / (*insptr), g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_modvar:
	{
		int i;
		int instr;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		instr = (*insptr);
		if (instr == 0)
		{
			I_Error("Divide by Zero in CON");
		}
		lResult = GetGameVarID(i, g_ac, g_p).safeValue() % instr;
		SetGameVarID(i, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_andvar:
	{
		int i;
		int instr;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		instr = (*insptr);
		lResult = GetGameVarID(i, g_ac, g_p).safeValue() & instr;
		SetGameVarID(i, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_xorvar:
	{
		int i;
		int instr;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		instr = (*insptr);
		lResult = GetGameVarID(i, g_ac, g_p).safeValue() ^ instr;
		SetGameVarID(i, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_orvar:
	{
		int i;
		int instr;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		instr = (*insptr);
		lResult = GetGameVarID(i, g_ac, g_p).safeValue() | instr;
		SetGameVarID(i, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_randvarvar:
	{
		int i;
		int l1, l2;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		l1 = GetGameVarID(i, g_ac, g_p).safeValue(); // not used for this command
		l2 = GetGameVarID(*insptr, g_ac, g_p).safeValue();
		lResult = MulScale(rand(), l2, 15);
		SetGameVarID(i, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_gmaxammo:
	{
		int i;
		int l1, l2;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		l1 = GetGameVarID(i, g_ac, g_p).safeValue();
		l2 = GetGameVarID(*insptr, g_ac, g_p).safeValue(); // l2 not used in this one
		lResult = gs.max_ammo_amount[l1];
		SetGameVarID(*insptr, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_smaxammo:
	{
		int i;
		int l1, l2;
		insptr++;
		i = *(insptr++);	// ID of def
		l1 = GetGameVarID(i, g_ac, g_p).safeValue();
		l2 = GetGameVarID(*insptr, g_ac, g_p).safeValue();
		gs.max_ammo_amount[l1] = l2;

		insptr++;
		break;
	}
	case concmd_mulvarvar:
	{
		int i;
		int l1, l2;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		l1 = GetGameVarID(i, g_ac, g_p).safeValue();
		l2 = GetGameVarID(*insptr, g_ac, g_p).safeValue();
		lResult = l1 * l2;
		SetGameVarID(i, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_divvarvar:
	{
		int i;
		int l1, l2;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		l1 = GetGameVarID(i, g_ac, g_p).safeValue();
		l2 = GetGameVarID(*insptr, g_ac, g_p).safeValue();
		if (l2 == 0)
		{
			I_Error("Divide by Zero in CON");
		}
		lResult = l1 / l2;
		SetGameVarID(i, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_modvarvar:
	{
		int i;
		int l1, l2;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		l1 = GetGameVarID(i, g_ac, g_p).safeValue();
		l2 = GetGameVarID(*insptr, g_ac, g_p).safeValue();
		if (l2 == 0)
		{
			I_Error("Mod by Zero in CON");
		}
		lResult = l1 % l2;
		SetGameVarID(i, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_andvarvar:
	{
		int i;
		int l1, l2;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		l1 = GetGameVarID(i, g_ac, g_p).safeValue();
		l2 = GetGameVarID(*insptr, g_ac, g_p).safeValue();
		lResult = l1 & l2;
		SetGameVarID(i, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_xorvarvar:
	{
		int i;
		int l1, l2;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		l1 = GetGameVarID(i, g_ac, g_p).safeValue();
		l2 = GetGameVarID(*insptr, g_ac, g_p).safeValue();
		lResult = l1 ^ l2;
		SetGameVarID(i, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_orvarvar:
	{
		int i;
		int l1, l2;
		int lResult;
		insptr++;
		i = *(insptr++);	// ID of def
		l1 = GetGameVarID(i, g_ac, g_p).safeValue();
		l2 = GetGameVarID(*insptr, g_ac, g_p).safeValue();
		lResult = l1 | l2;
		SetGameVarID(i, lResult, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_subvar:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		SetGameVarID(i, GetGameVarID(i, g_ac, g_p).safeValue() - *insptr, g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_subvarvar:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		SetGameVarID(i, GetGameVarID(i, g_ac, g_p).safeValue() - GetGameVarID(*insptr, g_ac, g_p).safeValue(), g_ac, g_p);
		insptr++;
		break;
	}
	case concmd_sin:
	{
		int i;
		int lValue;
		insptr++;
		i = *(insptr++);	// ID of def
		lValue = bsin(GetGameVarID(*insptr, g_ac, g_p).safeValue());
		SetGameVarID(i, lValue, g_ac, g_p);
		insptr++;
		break;
	}

	case concmd_spgetlotag:
	{
		insptr++;
		SetGameVarID(g_iLoTagID, g_ac->spr.lotag, g_ac, g_p);
		break;
	}
	case concmd_spgethitag:
	{
		insptr++;
		SetGameVarID(g_iHiTagID, g_ac->spr.hitag, g_ac, g_p);
		break;
	}
	case concmd_sectgetlotag:
	{
		insptr++;
		SetGameVarID(g_iLoTagID, g_ac->sector()->lotag, g_ac, g_p);
		break;
	}
	case concmd_sectgethitag:
	{
		insptr++;
		SetGameVarID(g_iHiTagID, g_ac->sector()->hitag, g_ac, g_p);
		break;
	}
	case concmd_gettexturefloor:
	{
		insptr++;
		SetGameVarID(g_iTextureID, g_ac->sector()->floorpicnum, g_ac, g_p);
		break;
	}

	case concmd_startlevel:
	{
		// from 'level' cheat in game.c (about line 6250)
		int volnume;
		int levnume;

		insptr++; // skip command
		volnume = GetGameVarID(*insptr++, g_ac, g_p).safeValue();
		levnume = GetGameVarID(*insptr++, g_ac, g_p).safeValue();
		auto level = FindMapByIndex(volnume, levnume);
		if (level != nullptr)
			ChangeLevel(level, g_nextskill);
		break;
	}
	case concmd_myosx:
	case concmd_myospalx:

	case concmd_myos:
	case concmd_myospal:
	{
		int x, y;
		int tilenum;
		int shade;
		int orientation;
		int pal;
		int tw = *insptr++;
		x = GetGameVarID(*insptr++, g_ac, g_p).safeValue();
		y = GetGameVarID(*insptr++, g_ac, g_p).safeValue();
		tilenum = GetGameVarID(*insptr++, g_ac, g_p).safeValue();
		shade = GetGameVarID(*insptr++, g_ac, g_p).safeValue();
		orientation = GetGameVarID(*insptr++, g_ac, g_p).safeValue();
		if (tw == concmd_myospal)
		{
			pal = GetGameVarID(*insptr++, g_ac, g_p).safeValue();
			//myospal(x, y, tilenum, shade, orientation, pal);
		}
		else if (tw == concmd_myos)
		{
			//myos(x, y, tilenum, shade, orientation);
		}
		else if (tw == concmd_myosx)
		{
			//myos640(x, y, tilenum, shade, orientation);
		}
		else if (tw == concmd_myospalx)
		{
			pal = GetGameVarID(*insptr++, g_ac, g_p).safeValue();
			//myospal640(x, y, tilenum, shade, orientation, pal);
		}
		break;
	}

	case concmd_displayrand:
	{
		int i;
		insptr++;

		i = *(insptr++);	// ID of def
		SetGameVarID(i, rand(), g_ac, g_p);
		break;
	}
	case concmd_switch:
	{
		int lVarID;
		int lValue;
		int* lpDefault;
		int* lpCases;
		int lCases;
		int lEnd;
		int lCheckCase;
		bool bMatched;
		int* lTempInsPtr;

		// command format:
		// variable ID to check
		// script offset to 'end'
		// count of case statements
		// script offset to default case (null if none)
		// For each case: value, ptr to code
		insptr++; // p-code
		lVarID = *insptr++;
		lValue = GetGameVarID(lVarID, g_ac, g_p).safeValue();
		lEnd = *insptr++;
		lCases = *insptr++;
		lpDefault = insptr++;
		lpCases = insptr;
		insptr += lCases * 2;
		bMatched = false;
		lTempInsPtr = insptr;
		for (lCheckCase = 0; lCheckCase < lCases && !bMatched; lCheckCase++)
		{
			if (lpCases[lCheckCase * 2] == lValue)
			{
				insptr = &ScriptCode[lpCases[lCheckCase * 2 + 1]];
				while (1)
				{
					if (parse())
						break;
				}
				bMatched = true;
			}
		}
		if (!bMatched)
		{
			if (*lpDefault)
			{
				insptr = &ScriptCode[*lpDefault];
				while (1) if (parse()) break;
			}
			else
			{
				//AddLog("No Matching Case: No Default to use");
			}
		}
		insptr = &ScriptCode[lEnd];
		break;
	}
	case concmd_endswitch:
		insptr++;
		return 1;
		break;

	case concmd_starttrack:
	{
		insptr++;
		int music_select = *insptr++;
 		auto level = FindMapByIndex(currentLevel->cluster, music_select+1); // this was 0-based in EDuke 2.0...
		if (level) S_PlayLevelMusic(level);
		break;
	}
	case concmd_gettextureceiling:
	{
		insptr++;
		SetGameVarID(g_iTextureID, g_ac->sector()->ceilingpicnum, g_ac, g_p);
		break;
	}
	case concmd_ifvarvarand:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		j = 0;
		if (GetGameVarID(i, g_ac, g_p).safeValue() & GetGameVarID(*(insptr), g_ac, g_p).safeValue())
		{
			j = 1;
		}
		parseifelse(j);
		break;
	}
	case concmd_ifvarvarn:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		j = 0;
		if (GetGameVarID(i, g_ac, g_p).safeValue() != GetGameVarID(*(insptr), g_ac, g_p).safeValue())
		{
			j = 1;
		}
		parseifelse(j);
		break;
	}
	case concmd_ifvarn:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		j = 0;
		if (GetGameVarID(i, g_ac, g_p).safeValue() != *insptr)
		{
			j = 1;
		}
		parseifelse(j);
		break;
	}
	case concmd_ifvarand:
	{
		int i;
		insptr++;
		i = *(insptr++);	// ID of def
		j = 0;
		if (GetGameVarID(i, g_ac, g_p).safeValue() & *insptr)
		{
			j = 1;
		}
		parseifelse(j);
		break;
	}

	default:
		Printf(TEXTCOLOR_RED "Unrecognized PCode of %d  in parse.  Killing current sprite.\n",*insptr);
		Printf(TEXTCOLOR_RED "Offset=%0X\n",int(insptr-ScriptCode.Data()));
		killit_flag = 1;
		break;
	}
	return 0;
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void LoadActor(DDukeActor *actor, int p, int x)
{
	int done;

	ParseState s;
	s.g_p = p;	// Player ID
	s.g_x = x;	// ??
	s.g_ac = actor;
	s.g_t = &s.g_ac->temp_data[0];	// Sprite's 'extra' data

	auto addr = gs.tileinfo[actor->spr.picnum].loadeventscriptptr;
	if (addr == 0) return;

	s.killit_flag = 0;

	if(!actor->insector())
	{
		deletesprite(actor);
		return;
	}
	do
		done = s.parse();
	while (done == 0);

	if (s.killit_flag == 1)
	{
		// if player was set to squish, first stop that..
		if (p >= 0)
		{
			if (ps[p].actorsqu == actor)
				ps[p].actorsqu = nullptr;
		}
		deletesprite(actor);
	}
	else
	{
		fi.move(actor, p, x);

		if (actor->spr.statnum == STAT_ACTOR)
		{
			if (badguy(actor))
			{
				if (actor->spr.xrepeat > 60) return;
				if (ud.respawn_monsters == 1 && actor->spr.extra <= 0) return;
			}
			else if (ud.respawn_items == 1 && (actor->spr.cstat & CSTAT_SPRITE_INVISIBLE)) return;

			if (actor->timetosleep > 1)
				actor->timetosleep--;
			else if (actor->timetosleep == 1)
				ChangeActorStat(actor, STAT_ZOMBIEACTOR);
		}
	}

}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void execute(DDukeActor *actor,int p,int x)
{
	if (gs.actorinfo[actor->spr.picnum].scriptaddress == 0) return;

	int done;

	ParseState s;
	s.g_p = p;	// Player ID
	s.g_x = x;	// ??
	s.g_ac = actor;
	s.g_t = &actor->temp_data[0];	// Sprite's 'extra' data

	if (gs.actorinfo[actor->spr.picnum].scriptaddress == 0) return;
	s.insptr = &ScriptCode[4 + (gs.actorinfo[actor->spr.picnum].scriptaddress)];

	s.killit_flag = 0;

	if(!actor->insector())
	{
		if(badguy(actor))
			ps[p].actors_killed++;
		deletesprite(actor);
		return;
	}

	if (s.g_t[4])
	{
		// This code was utterly cryptic in the original source.
		auto ptr = &ScriptCode[s.g_t[4]];
		int numframes = ptr[1];
		int increment = ptr[3];
		int delay =  ptr[4];

		actor->spr.lotag += TICSPERFRAME;
		if (actor->spr.lotag > delay)
		{
			s.g_t[2]++;
			actor->spr.lotag = 0;
			s.g_t[3] += increment;
		}
		if (abs(s.g_t[3]) >= abs(numframes * increment))
			s.g_t[3] = 0;
	}

	do
		done = s.parse();
	while( done == 0 );

	if(s.killit_flag == 1)
	{
		// if player was set to squish, first stop that..
		if(ps[p].actorsqu == actor)
			ps[p].actorsqu = nullptr;
		killthesprite = true;
	}
	else
	{
		fi.move(actor, p, x);

		if (actor->spr.statnum == STAT_ACTOR)
		{
			if (badguy(actor))
			{
				if (actor->spr.xrepeat > 60) goto quit;
				if (ud.respawn_monsters == 1 && actor->spr.extra <= 0) goto quit;
			}
			else if (ud.respawn_items == 1 && (actor->spr.cstat & CSTAT_SPRITE_INVISIBLE)) goto quit;
		}

		if (actor->spr.statnum == STAT_ACTOR || (actor->spr.statnum == STAT_STANDABLE && actorflag(actor, SFLAG_CHECKSLEEP)))
		{
			if (actor->timetosleep > 1)
				actor->timetosleep--;
			else if (actor->timetosleep == 1)
				ChangeActorStat(actor, STAT_ZOMBIEACTOR);
		}
	}
quit:
	if (killthesprite) deletesprite(actor);
	killthesprite = false;
}


//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void OnEvent(int iEventID, int p, DDukeActor *actor, int x)
{
	int done;

	if (iEventID >= MAXGAMEEVENTS)
	{
		Printf("Invalid Event ID\n");
		return;
	}
	if (apScriptGameEvent[iEventID] == 0)
	{
		return;
	}

	ParseState s;
	s.g_p = p;	/// current player ID
	s.g_x = x;	// ?
	s.g_ac = actor;
	s.g_t = actor->temp_data;

	s.insptr = &ScriptCode[apScriptGameEvent[iEventID]];

	s.killit_flag = 0;
	do
		done = s.parse();
	while (done == 0);
}

END_DUKE_NS
