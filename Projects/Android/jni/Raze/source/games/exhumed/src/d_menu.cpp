//-------------------------------------------------------------------------
/*
Copyright (C) 2016 EDuke32 developers and contributors
Copyright (C) 2019 Christoph Oelckers

This is free software; you can redistribute it and/or
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
#include "exhumed.h"
#include "engine.h"
#include "sound.h"
#include "names.h"
#include "version.h"
#include "raze_sound.h"
#include "gamestate.h"
#include "mapinfo.h"
#include "gamecontrol.h"
#include "v_draw.h"
#include "vm.h"
#include "razemenu.h"



BEGIN_PS_NS

void GameInterface::MenuOpened()
{
	GrabPalette();
	menuDelegate->FloatVar(NAME_zoomsize) = 0;
	StopLocalSound();
}

void GameInterface::MenuSound(EMenuSounds snd)
{
	switch (snd)
	{
	case ActivateSound:
		PlayLocalSound(StaticSound[kSound31], 0, false, CHANF_UI);
		break;

	case CursorSound:
		PlayLocalSound(StaticSound[kSound35], 0, false, CHANF_UI);
		break;

	case AdvanceSound:
	case BackSound:
		PlayLocalSound(StaticSound[kSound33], 0, false, CHANF_UI);
		break;

	default:
		return;
	}
}

FSavegameInfo GameInterface::GetSaveSig()
{
	return { SAVESIG_PS, MINSAVEVER_PS, SAVEVER_PS };
}



END_PS_NS

using namespace Exhumed;

DEFINE_ACTION_FUNCTION(_ListMenuItemExhumedLogo, Draw)
{
	auto nLogoTile = GameLogo();
	DrawRel(nLogoTile, 160, 40);
	return 0;
}

