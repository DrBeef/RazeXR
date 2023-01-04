BEGIN_DUKE_NS

void SE40_Draw(int tag, spritetype *spr, int x, int y, int z, binangle a, fixedhoriz h, int smoothratio)
{
	int i, k = 0;
	int ok = 0, fofmode = 0;
	int offx, offy;
	spritetype* floor1, *floor2 = nullptr;

	if (spr->ang != 512) return;

	i = FOF; //Effect TILE
	tileDelete(FOF);
	if (!testgotpic(FOF, true)) return;

	floor1 = spr;

	if (spr->lotag == tag + 2) fofmode = tag + 0;
	if (spr->lotag == tag + 3) fofmode = tag + 1;
	if (spr->lotag == tag + 4) fofmode = tag + 0;
	if (spr->lotag == tag + 5) fofmode = tag + 1;

	ok++;

	DukeStatIterator it(STAT_RAROR);
	while (auto act = it.Next())
	{
		if (
			act->spr.picnum == SECTOREFFECTOR &&
			act->spr.lotag == fofmode &&
			act->spr.hitag == floor1->hitag
			) 
		{
			floor1 = spr; 
			fofmode = act->spr.lotag; 
			ok++; 
			break;
		}
	}
	// if(ok==1) { Message("no floor1",RED); return; }

	if (fofmode == tag + 0) k = tag + 1; else k = tag + 0;

	it.Reset(STAT_RAROR);
	while (auto act = it.Next())
	{
		if (
			act->spr.picnum == SECTOREFFECTOR &&
			act->spr.lotag == k &&
			act->spr.hitag == floor1->hitag
			) 
		{
			floor2 = spr; 
			ok++; 
			break;
		}
	}

	// if(ok==2) { Message("no floor2",RED); return; }

	it.Reset(STAT_RAROR);
	while (auto act = it.Next())
	{
		if (act->spr.picnum == SECTOREFFECTOR &&
			act->spr.lotag == k + 2 &&
			act->spr.hitag == floor1->hitag
			)
		{
			auto sect = act->sector();
			// repurpose otherwise unused fields in sectortype as temporary storage.
			if (k == tag + 0)
			{
				sect->Flag = sect->floorz;
				sect->addfloorz((((z - sect->floorz) / 32768) + 1) * 32768, true);
				sect->Damage = sect->floorpicnum;
				sect->floorpicnum = 13;
			}
			if (k == tag + 1)
			{
				sect->Flag = sect->ceilingz;
				sect->addceilingz((((z - sect->ceilingz) / 32768) - 1) * 32768, true);
				sect->Damage = sect->ceilingpicnum;
				sect->ceilingpicnum = 13;
			}
		}
	}

	offx = x - floor1->pos.X;
	offy = y - floor1->pos.Y;

	renderDrawRoomsQ16(floor2->pos.X + offx, floor2->pos.Y + offy, z, a.asq16(), h.asq16(), floor2->sectp, false);
	fi.animatesprites(pm_tsprite, pm_spritesortcnt, offx + floor2->pos.X, offy + floor2->pos.Y, a.asbuild(), smoothratio);
	renderDrawMasks();

	it.Reset(STAT_RAROR);
	while (auto act = it.Next())
	{
		if (act->spr.picnum == 1 &&
			act->spr.lotag == k + 2 &&
			act->spr.hitag == floor1->hitag
			)
		{
			auto sect = act->sector();
			if (k == tag + 0)
			{
				sect->setfloorz(sect->Flag, true);
				sect->floorpicnum = sect->Damage;
			}
			if (k == tag + 1)
			{
				sect->setceilingz(sect->Flag, true);
				sect->ceilingpicnum = sect->Damage;
			}
		}// end if
	}// end for

} // end SE40


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void se40code(int x, int y, int z, binangle a, fixedhoriz h, int smoothratio)
{
	int tag;
	if (!isRR()) tag = 40;
	else if (isRRRA()) tag = 150;
	else return;

	DukeStatIterator it(STAT_RAROR);
	while (auto act = it.Next())
	{
		switch (act->spr.lotag - tag + 40)
		{
			//case 40:
			//case 41:
			//	SE40_Draw(i,x,y,a,smoothratio);
			//	break;
		case 42:
		case 43:
		case 44:
		case 45:
			if (ps[screenpeek].cursector == act->sector())
				SE40_Draw(tag, &act->spr, x, y, z, a, h, smoothratio);
			break;
		}
	}
}


//---------------------------------------------------------------------------
//
// split out so it can also be applied to camera views
//
//---------------------------------------------------------------------------

void renderMirror(int cposx, int cposy, int cposz, binangle cang, fixedhoriz choriz, int smoothratio)
{
	if (mirrorcnt > 0 && testgotpic(TILE_MIRROR, true))
	{
		int dst = 0x7fffffff, i = 0;
		for (int k = 0; k < mirrorcnt; k++)
		{
			int j = abs(mirrorwall[k]->wall_int_pos().X - cposx) + abs(mirrorwall[k]->wall_int_pos().Y - cposy);
			if (j < dst) dst = j, i = k;
		}

		if (mirrorwall[i] && mirrorwall[i]->overpicnum == TILE_MIRROR)
		{
			int tposx, tposy;
			fixed_t tang;

			renderPrepareMirror(cposx, cposy, cposz, cang.asq16(), choriz.asq16(), wallnum(mirrorwall[i]), &tposx, &tposy, &tang);

			int j = g_visibility;
			g_visibility = (j >> 1) + (j >> 2);

			renderDrawRoomsQ16(tposx, tposy, cposz, tang, choriz.asq16(), sectnum(mirrorsector[i]), true);

			display_mirror = 1;
			fi.animatesprites(pm_tsprite, pm_spritesortcnt, tposx, tposy, tang, smoothratio);
			display_mirror = 0;

			renderDrawMasks();
			renderCompleteMirror();   //Reverse screen x-wise in this function
			g_visibility = j;
		}
	}
}

//---------------------------------------------------------------------------
//
// used by RR to inject some external geometry into a scene. 
//
//---------------------------------------------------------------------------

static void geometryEffect(int cposx, int cposy, int cposz, binangle cang, fixedhoriz choriz, int sect, int smoothratio)
{
	auto sectp = &sector[sect];
	int gs, geoid = 0;
	sectortype* tgsect, *geosect;
	renderDrawRoomsQ16(cposx, cposy, cposz, cang.asq16(), choriz.asq16(), sect, false);
	fi.animatesprites(pm_tsprite, pm_spritesortcnt, cposx, cposy, cang.asbuild(), smoothratio);
	renderDrawMasks();
	for (gs = 0; gs < geocnt; gs++)
	{
		tgsect = geosector[gs];

		DukeSectIterator it(tgsect);
		while (auto act = it.Next())
		{
			ChangeActorSect(act, geosectorwarp[gs]);
			SetActor(act, { act->spr.pos.X -= int(geox[gs] * worldtoint), act->spr.pos.Y -= int(geoy[gs] * worldtoint), act->spr.pos.Z });
		}
		if (geosector[gs] == sectp)
		{
			geosect = geosectorwarp[gs];
			geoid = gs;
		}
	}
	cposx -= int(geox[geoid] * worldtoint);
	cposy -= int(geoy[geoid] * worldtoint);
	renderDrawRoomsQ16(cposx, cposy, cposz, cang.asq16(), choriz.asq16(), sect, false);
	cposx += int(geox[geoid] * worldtoint);
	cposy += int(geoy[geoid] * worldtoint);
	for (gs = 0; gs < geocnt; gs++)
	{
		tgsect = geosectorwarp[gs];
		DukeSectIterator it(tgsect);
		while (auto act = it.Next())
		{
			ChangeActorSect(act, geosector[gs]);
			SetActor(act, { act->spr.pos.X += int(geox[gs] * worldtoint), act->spr.pos.Y += int(geoy[gs] * worldtoint), act->spr.pos.Z });
		}
	}
	fi.animatesprites(pm_tsprite, pm_spritesortcnt, cposx, cposy, cang.asbuild(), smoothratio);
	renderDrawMasks();
	for (gs = 0; gs < geocnt; gs++)
	{
		tgsect = geosector[gs];
		DukeSectIterator it(tgsect);
		while (auto act = it.Next())
		{
			ChangeActorSect(act, geosectorwarp2[gs]);
			SetActor(act, { act->spr.pos.X -= int(geox2[gs] * worldtoint), act->spr.pos.Y -= int(geoy2[gs] * worldtoint), act->spr.pos.Z });
		}
		if (geosector[gs] == sectp)
		{
			geosect = geosectorwarp2[gs];
			geoid = gs;
		}
	}
	cposx -= int(geox2[geoid] * worldtoint);
	cposy -= int(geoy2[geoid] * worldtoint);
	renderDrawRoomsQ16(cposx, cposy, cposz, cang.asq16(), choriz.asq16(), sect, false);
	cposx += int(geox2[geoid] * worldtoint);
	cposy += int(geoy2[geoid] * worldtoint);
	for (gs = 0; gs < geocnt; gs++)
	{
		tgsect = geosectorwarp2[gs];
		DukeSectIterator it(tgsect);
		while (auto act = it.Next())
		{
			ChangeActorSect(act, geosector[gs]);
			SetActor(act, { act->spr.pos.X += int(geox2[gs] * worldtoint), act->spr.pos.Y += int(geoy2[gs] * worldtoint), act->spr.pos.Z });
		}
	}
	fi.animatesprites(pm_tsprite, pm_spritesortcnt, cposx, cposy, cang.asbuild(), smoothratio);
	renderDrawMasks();
}
END_DUKE_NS
