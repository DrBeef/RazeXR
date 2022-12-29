/*
** hw_bunchdrawer.cpp
**
**---------------------------------------------------------------------------
** Copyright 2008-2021 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/ 

#include "hw_drawinfo.h"
#include "hw_bunchdrawer.h"
#include "hw_clipper.h"
#include "hw_clock.h"
#include "hw_drawstructs.h"
#include "automap.h"
#include "gamefuncs.h"
#include "hw_portal.h"
#include "gamestruct.h"
#include "hw_voxels.h"
#include "mapinfo.h"
#include "gamecontrol.h"
#include "hw_sections.h"

extern TArray<int> blockingpairs[MAXWALLS];

//==========================================================================
//
//
//
//==========================================================================

void BunchDrawer::Init(HWDrawInfo *_di, Clipper* c, vec2_t& view, binangle a1, binangle a2)
{
	ang1 = a1;
	ang2 = a2;
	angrange = ang2 - ang1;
	di = _di;
	clipper = c;
	viewx = view.x * (1/ 16.f);
	viewy = view.y * -(1/ 16.f);
	iview = view;
	StartScene();
	clipper->SetViewpoint(view);

	gcosang = bamang(di->Viewpoint.RotAngle).fcos();
	gsinang = bamang(di->Viewpoint.RotAngle).fsin();

	for (int i = 0; i < numwalls; i++)
	{
		// Precalculate the clip angles to avoid doing this repeatedly during level traversal.
		wall[i].clipangle = clipper->PointToAngle(wall[i].pos);
	}
	memset(sectionstartang, -1, sizeof(sectionstartang));
	memset(sectionendang, -1, sizeof(sectionendang));
}

//==========================================================================
//
//
//
//==========================================================================

void BunchDrawer::StartScene()
{
	LastBunch = 0;
	StartTime = I_msTime();
	Bunches.Clear();
	CompareData.Clear();
	gotsector.Zero();
	gotsection2.Zero();
	gotwall.Zero();
	blockwall.Zero();
}

//==========================================================================
//
//
//
//==========================================================================

bool BunchDrawer::StartBunch(int sectnum, int linenum, binangle startan, binangle endan, bool portal)
{
	FBunch* bunch = &Bunches[LastBunch = Bunches.Reserve(1)];

	bunch->sectnum = sectnum;
	bunch->startline = bunch->endline = linenum;
	bunch->startangle = startan;
	bunch->endangle = endan;
	bunch->portal = portal;
	assert(bunch->endangle.asbam() > bunch->startangle.asbam());
	return bunch->endangle != angrange;
}

//==========================================================================
//
//
//
//==========================================================================

bool BunchDrawer::AddLineToBunch(int line, binangle newan)
{
	Bunches[LastBunch].endline++;
	assert(newan.asbam() > Bunches[LastBunch].endangle.asbam());
	Bunches[LastBunch].endangle = newan;
	assert(Bunches[LastBunch].endangle.asbam() > Bunches[LastBunch].startangle.asbam());
	return Bunches[LastBunch].endangle != angrange;
}

//==========================================================================
//
//
//
//==========================================================================

void BunchDrawer::DeleteBunch(int index)
{
	Bunches[index] = Bunches.Last();
	Bunches.Pop();
}

bool BunchDrawer::CheckClip(walltype* wal)
{
	auto pt2 = &wall[wal->point2];
	sectortype* backsector = &sector[wal->nextsector];
	sectortype* frontsector = &sector[wall[wal->nextwall].nextsector];

	// if one plane is sky on both sides, the line must not clip.
	if (frontsector->ceilingstat & backsector->ceilingstat & CSTAT_SECTOR_SKY) return false;
	if (frontsector->floorstat & backsector->floorstat & CSTAT_SECTOR_SKY) return false;

	float bs_floorheight1;
	float bs_floorheight2;
	float bs_ceilingheight1;
	float bs_ceilingheight2;
	float fs_floorheight1;
	float fs_floorheight2;
	float fs_ceilingheight1;
	float fs_ceilingheight2;

	// Mirrors and horizons always block the view
	//if (linedef->special==Line_Mirror || linedef->special==Line_Horizon) return true;

	PlanesAtPoint(frontsector, wal->x, wal->y, &fs_ceilingheight1, &fs_floorheight1);
	PlanesAtPoint(frontsector, pt2->x, pt2->y, &fs_ceilingheight2, &fs_floorheight2);

	PlanesAtPoint(backsector, wal->x, wal->y, &bs_ceilingheight1, &bs_floorheight1);
	PlanesAtPoint(backsector, pt2->x, pt2->y, &bs_ceilingheight2, &bs_floorheight2);

	// now check for closed sectors! No idea if we really need the sky checks. We'll see.
	if (bs_ceilingheight1 <= fs_floorheight1 && bs_ceilingheight2 <= fs_floorheight2)
	{
		// backsector's ceiling is below frontsector's floor.
		return true;
	}

	if (fs_ceilingheight1 <= bs_floorheight1 && fs_ceilingheight2 <= bs_floorheight2) 
	{
		// backsector's floor is above frontsector's ceiling
		return true;
	}

	if (bs_ceilingheight1 <= bs_floorheight1 && bs_ceilingheight2 <= bs_floorheight2) 
	{
		// backsector is closed
		return true;
	}

	return false;
}

//==========================================================================
//
// ClipLine
// Clips the given segment
//
//==========================================================================

int BunchDrawer::ClipLine(int aline, bool portal)
{
	auto cline = &sectionLines[aline];
	int section = cline->section;
	int line = cline->wall;

	auto startAngleBam = ClipAngle(cline->startpoint);
	auto endAngleBam = ClipAngle(cline->endpoint);

	// Back side, i.e. backface culling	- read: endAngle <= startAngle!
	if (startAngleBam.asbam() - endAngleBam.asbam() < ANGLE_180)
	{
		return CL_Skip;
	}
	if (line >= 0 && blockwall[line]) return CL_Draw;
	auto wal = &wall[line];

	// convert to clipper coordinates and clamp to valid range.
	int startAngle = startAngleBam.asbam();
	int endAngle = endAngleBam.asbam();
	if (startAngle < 0) startAngle = 0;
	if (endAngle < 0 || endAngle > (int)angrange.asbam()) endAngle = angrange.asbam();

	// since these values are derived from previous calls of this function they cannot be out of range.
	int sectStartAngle = sectionstartang[section];
	auto sectEndAngle = sectionendang[section];

	// check against the maximum possible viewing range of the sector.
	// Todo: check if this is sufficient or if we really have to do a more costly check against the single visible segments.
	// Note: These walls may be excluded from the clipper, but not from being drawn!
	bool dontclip = false;
	if (sectStartAngle != -1)
	{
		if (sectStartAngle > endAngle || sectEndAngle < startAngle)
		{
			dontclip = true;
		}
		else
		{
			if (sectStartAngle > startAngle) startAngle = sectStartAngle;
			if (sectEndAngle < endAngle) endAngle = sectEndAngle;
			if (endAngle <= startAngle) return CL_Skip; // can this even happen?
		}
	}

	if (!portal && !clipper->IsRangeVisible(startAngle, endAngle))
	{
		return CL_Skip;
	}

	if (cline->partner == -1 || (wal->cstat & CSTAT_WALL_1WAY) || CheckClip(wal))
	{
		// one-sided
		if (!portal && !dontclip) clipper->AddClipRange(startAngle, endAngle);
		return CL_Draw;
	}
	else
	{
		if (portal) clipper->RemoveClipRange(startAngle, endAngle);

		// set potentially visible viewing range for this line's back sector.
		int nsection = cline->partnersection;
		if (sectionstartang[nsection] == -1)
		{
			sectionstartang[nsection] = startAngle;
			sectionendang[nsection] = endAngle;
		}
		else
		{
			if (startAngle < sectionstartang[nsection]) sectionstartang[nsection] = startAngle;
			if (endAngle > sectionendang[nsection]) sectionendang[nsection] = endAngle;
		}

		return CL_Draw | CL_Pass;
	}
}

//==========================================================================
//
//
//
//==========================================================================

void BunchDrawer::ProcessBunch(int bnch)
{
	FBunch* bunch = &Bunches[bnch];
	int start = bunch->startline;
	int end = bunch->endline;

	ClipWall.Clock();
	for (int i = start; i <= end; i++)
	{
		bunch = &Bunches[bnch];	// re-get the pointer in case of reallocation.
		int clipped = ClipLine(i, bunch->portal);

		if (clipped & CL_Draw)
		{
			int ww = sectionLines[i].wall;
			if (ww != -1)
			{
				for (auto p : blockingpairs[ww]) blockwall.Set(sectionLines[p].wall);
				show2dwall.Set(ww);

				if (!gotwall[i])
				{
					gotwall.Set(i);
					ClipWall.Unclock();
					Bsp.Unclock();
					SetupWall.Clock();

					HWWall hwwall;
					hwwall.Process(di, &wall[ww], &sector[bunch->sectnum], wall[ww].nextsector < 0 ? nullptr : &sector[wall[ww].nextsector]);
					rendered_lines++;

					SetupWall.Unclock();
					Bsp.Clock();
					ClipWall.Clock();
				}
			}
		}

		if (clipped & CL_Pass)
		{
			ClipWall.Unclock();
			ProcessSection(sectionLines[i].partnersection, false);
			ClipWall.Clock();
		}
	}
	ClipWall.Unclock();
}

//==========================================================================
//
// 
//
//==========================================================================

int BunchDrawer::WallInFront(int line1, int line2)
{
	int wall1s = sectionLines[line1].startpoint;
	int wall1e = sectionLines[line1].endpoint;
	int wall2s = sectionLines[line2].startpoint;
	int wall2e = sectionLines[line2].endpoint;

	double x1s = WallStartX(wall1s);
	double y1s = WallStartY(wall1s);
	double x1e = WallStartX(wall1e);
	double y1e = WallStartY(wall1e);
	double x2s = WallStartX(wall2s);
	double y2s = WallStartY(wall2s);
	double x2e = WallStartX(wall2e);
	double y2e = WallStartY(wall2e);

	double dx = x1e - x1s;
	double dy = y1e - y1s;

	double t1 = PointOnLineSide(x2s, y2s, x1s, y1s, dx, dy);
	double t2 = PointOnLineSide(x2e, y2e, x1s, y1s, dx, dy);
	if (t1 == 0)
	{
		if (t2 == 0) return(-1);
		t1 = t2;
	}
	if (t2 == 0) t2 = t1;

	if ((t1 * t2) >= 0)
	{
		t2 = PointOnLineSide(viewx, viewy, x1s, y1s, dx, dy);
		return((t2 * t1) <= 0);
	}

	dx = x2e - x2s;
	dy = y2e - y2s;
	t1 = PointOnLineSide(x1s, y1s, x2s, y2s, dx, dy);
	t2 = PointOnLineSide(x1e, y1e, x2s, y2s, dx, dy);
	if (t1 == 0)
	{
		if (t2 == 0) return(-1);
		t1 = t2;
	}
	if (t2 == 0) t2 = t1;
	if ((t1 * t2) >= 0)
	{
		t2 = PointOnLineSide(viewx, viewy, x2s, y2s, dx, dy);
		return((t2 * t1) > 0);
	}
	return(-2);
}

//==========================================================================
//
// Rules:
// 1. Any bunch can span at most 180°.
// 2. 2 bunches can never overlap at both ends
// 3. if there is an overlap one of the 2 starting points must be in the
//    overlapping area.
//
//==========================================================================

int BunchDrawer::ColinearBunchInFront(FBunch* b1, FBunch* b2)
{
	// Unable to determine the order. The only option left is to see if the sectors within the bunch can be ordered.
	for (int i = b1->startline; i <= b1->endline; i++)
	{
		int wall1s = sectionLines[i].wall;
		if (wall1s == -1) continue;
		int sect1 = wall[wall1s].sector;
		int nsect1 = wall[wall1s].nextsector;
		if (nsect1 < 0) continue;
		for (int j = b2->startline; j <= b2->endline; j++)
		{
			int wall2s = sectionLines[j].wall;
			if (wall2s == -1) continue;
			int sect2 = wall[wall2s].sector;
			int nsect2 = wall[wall2s].nextsector;
			if (nsect2 < 0) continue;
			if (sect1 == nsect2) return 1; // bunch 2 is in front
			if (sect2 == nsect1) return 0; // bunch 1 is in front
		}
	}
	return -1;
}

int BunchDrawer::BunchInFront(FBunch* b1, FBunch* b2)
{
	binangle anglecheck, endang;
	bool colinear = false;

	if (b2->startangle.asbam() >= b1->startangle.asbam() && b2->startangle.asbam() < b1->endangle.asbam())
	{
		// we have an overlap at b2->startangle
		anglecheck = b2->startangle;

		// Find the wall in b1 that overlaps b2->startangle
		for (int i = b1->startline; i <= b1->endline; i++)
		{
			endang = ClipAngle(wall[i].point2);
			if (endang.asbam() > anglecheck.asbam())
			{
				// found a line
				int ret = WallInFront(b2->startline, i);
				if (ret == -1)
				{
					ret = ColinearBunchInFront(b1, b2);
					if (ret == -1)
					{
						colinear = true;
						continue;
					}
				}
				return ret;
			}
		}
	}
	else if (b1->startangle.asbam() >= b2->startangle.asbam() && b1->startangle.asbam() < b2->endangle.asbam())
	{
		// we have an overlap at b1->startangle
		anglecheck = b1->startangle;

		// Find the wall in b2 that overlaps b1->startangle
		for (int i = b2->startline; i <= b2->endline; i++)
		{
			endang = ClipAngle(wall[i].point2);
			if (endang.asbam() > anglecheck.asbam())
			{
				// found a line
				int ret = WallInFront(i, b1->startline);
				if (ret == -1)
				{
					ret = ColinearBunchInFront(b1, b2);
					if (ret == -1)
					{
						colinear = true;
						continue;
					}
				}
				return ret;
			}
		}
	}
	if (colinear)
	{
		// This should never happen.
		assert(true);
	}
	// we have no overlap
	return -1;
}

//==========================================================================
//
//
//
//==========================================================================

int BunchDrawer::FindClosestBunch()
{
	int closest = 0;              //Almost works, but not quite :(

	CompareData.Clear();
	for (unsigned i = 1; i < Bunches.Size(); i++)
	{
		switch (BunchInFront(&Bunches[i], &Bunches[closest]))
		{
		case 0:		// i is in front
			closest = i;
			continue;

		case 1:	// i is behind
			continue;

		default:		// can't determine
			CompareData.Push(i);	// mark for later comparison
			continue;
		}
	}

	// we need to do a second pass to see how the marked bunches relate to the currently closest one.
	for (unsigned i = 0; i < CompareData.Size(); i++)
	{
		switch (BunchInFront(&Bunches[CompareData[i]], &Bunches[closest]))
		{
		case 0:		// is in front
			closest = CompareData[i];
			CompareData[i] = CompareData.Last();
			CompareData.Pop();
			i = -1;	// we need to recheck everything that's still marked. -1 because this will get incremented before being used.
			continue;

		case 1:	// is behind
			CompareData[i] = CompareData.Last();
			CompareData.Pop();
			i--;
			continue;

		default:
			continue;

		}
	}
	//Printf("picked bunch starting at %d\n", Bunches[closest].startline);
	return closest;
}

//==========================================================================
//
//
//
//==========================================================================

void BunchDrawer::ProcessSection(int sectionnum, bool portal)
{
	if (gotsection2[sectionnum]) return;
	gotsection2.Set(sectionnum);

	bool inbunch;

	SetupSprite.Clock();

	int z;
	int sectnum = sections[sectionnum].sector;
	if (!gotsector[sectnum])
	{
		gotsector.Set(sectnum);
		SectIterator it(sectnum);
		while ((z = it.NextIndex()) >= 0)
		{
			auto const spr = (uspriteptr_t)&sprite[z];

			if ((spr->cstat & CSTAT_SPRITE_INVISIBLE) || spr->xrepeat == 0 || spr->yrepeat == 0) // skip invisible sprites
				continue;

			int sx = spr->x - iview.x, sy = spr->y - int(iview.y);

			// this checks if the sprite is it behind the camera, which will not work if the pitch is high enough to necessitate a FOV of more than 180°.
			//if ((spr->cstat & CSTAT_SPRITE_ALIGNMENT_MASK) || (hw_models && tile2model[spr->picnum].modelid >= 0) || ((sx * gcosang) + (sy * gsinang) > 0)) 
			{
				if ((spr->cstat & (CSTAT_SPRITE_ONE_SIDED | CSTAT_SPRITE_ALIGNMENT_MASK)) != (CSTAT_SPRITE_ONE_SIDED | CSTAT_SPRITE_ALIGNMENT_WALL) ||
					(r_voxels && tiletovox[spr->picnum] >= 0 && voxmodels[tiletovox[spr->picnum]]) ||
					(r_voxels && gi->Voxelize(spr->picnum) > -1) ||
					DMulScale(bcos(spr->ang), -sx, bsin(spr->ang), -sy, 6) > 0)
					if (renderAddTsprite(di->tsprite, di->spritesortcnt, z, sectnum))
						break;
			}
		}
		SetupSprite.Unclock();
	}

	if (automapping)
		show2dsector.Set(sectnum);

	SetupFlat.Clock();
	HWFlat flat;
	flat.ProcessSector(di, &sector[sectnum], sectionnum);
	SetupFlat.Unclock();

	//Todo: process subsectors
	inbunch = false;
	auto section = &sections[sectionnum];
	for (unsigned i = 0; i < section->lines.Size(); i++)
	{
		auto thisline = &sectionLines[section->lines[i]];

		binangle walang1 = ClipAngle(thisline->startpoint);
		binangle walang2 = ClipAngle(thisline->endpoint);

		// outside the visible area or seen from the backside.
		if ((walang1.asbam() > angrange.asbam() && walang2.asbam() > angrange.asbam() && walang1.asbam() < walang2.asbam()) ||
			(walang1.asbam() - walang2.asbam() < ANGLE_180))
		{
			inbunch = false;
		}
		else
		{
			if (walang1.asbam() >= angrange.asbam()) { walang1 = bamang(0); inbunch = false; }
			if (walang2.asbam() >= angrange.asbam()) walang2 = angrange;
			if (!inbunch)
			{
				//Printf("Starting bunch:\n\tWall %d\n", section->lines[i]);
				inbunch = StartBunch(sectnum, section->lines[i], walang1, walang2, portal);
			}
			else
			{
				//Printf("\tWall %d\n", section->lines[i]);
				inbunch = AddLineToBunch(section->lines[i], walang2);
			}
		}
		if (thisline->endpoint != section->lines[i] + 1) inbunch = false;
	}
}

//==========================================================================
//
//
//
//==========================================================================

void BunchDrawer::RenderScene(const int* viewsectors, unsigned sectcount, bool portal)
{
	//Printf("----------------------------------------- \nstart at sector %d\n", viewsectors[0]);
	auto process = [&]()
	{
		clipper->Clear(ang1);

		for (unsigned i = 0; i < sectcount; i++)
		{
			for (auto j : sectionspersector[viewsectors[i]])
			{
				sectionstartang[j] = 0;
				sectionendang[j] = int(angrange.asbam());
			}
		}
		for (unsigned i = 0; i < sectcount; i++)
		{
			for (auto j : sectionspersector[viewsectors[i]])
			{
				ProcessSection(j, portal);
			}
		}
		while (Bunches.Size() > 0)
		{
			int closest = FindClosestBunch();
			ProcessBunch(closest);
			DeleteBunch(closest);
		}
	};

	Bsp.Clock();
	if (ang1.asbam() != 0 || ang2.asbam() != 0)
	{
		process();
	}
	else
	{
		// with a 360° field of view we need to split the scene into two halves. 
		// The BunchInFront check can fail with angles that may wrap around.
		auto rotang = di->Viewpoint.RotAngle;
		ang1 = bamang(rotang - ANGLE_90);
		ang2 = bamang(rotang + ANGLE_90 - 1);
		angrange = ang2 - ang1;
		process();
		gotsection2.Zero();
		ang1 = bamang(rotang + ANGLE_90);
		ang2 = bamang(rotang - ANGLE_90 - 1);
		angrange = ang2 - ang1;
		process();
	}
	Bsp.Unclock();
}
