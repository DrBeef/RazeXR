/*
** maploader.cpp
**
** Map loader for non-Blood maps
**
**---------------------------------------------------------------------------
** Copyright 2020 Christoph Oelckers
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

#include <stdint.h>
#include "build.h"
#include "files.h"
#include "automap.h"
#include "printf.h"
#include "inputstate.h"
#include "md4.h"
#include "gamecontrol.h"
#include "gamefuncs.h"
#include "sectorgeometry.h"
#include "render.h"
#include "hw_sections.h"
#include "interpolate.h"

// needed for skipping over to get the map size first.
enum
{
	sectorsize5 = 37,
	sectorsize6 = 37,
	sectorsize7 = 40,
	wallsize5 = 35,
	wallsize6 = 32,
	wallsize7 = 32,
};


static void ReadSectorV7(FileReader& fr, sectortype& sect)
{
	sect.wallptr = fr.ReadInt16();
	sect.wallnum = fr.ReadInt16();
	sect.ceilingz = fr.ReadInt32();
	sect.floorz = fr.ReadInt32();
	sect.ceilingstat = fr.ReadUInt16();
	sect.floorstat = fr.ReadUInt16();
	sect.ceilingpicnum = fr.ReadUInt16();
	sect.ceilingheinum = fr.ReadInt16();
	sect.ceilingshade = fr.ReadInt8();
	sect.ceilingpal = fr.ReadUInt8();
	sect.ceilingxpan_ = fr.ReadUInt8();
	sect.ceilingypan_ = fr.ReadUInt8();
	sect.floorpicnum = fr.ReadUInt16();
	sect.floorheinum = fr.ReadInt16();
	sect.floorshade = fr.ReadInt8();
	sect.floorpal = fr.ReadUInt8();
	sect.floorxpan_ = fr.ReadUInt8();
	sect.floorypan_ = fr.ReadUInt8();
	sect.visibility = fr.ReadUInt8();
	sect.fogpal = fr.ReadUInt8(); // note: currently unused, except for Blood.
	sect.lotag = fr.ReadInt16();
	sect.hitag = fr.ReadInt16();
	sect.extra = fr.ReadInt16();
}

static void ReadSectorV6(FileReader& fr, sectortype& sect)
{
	sect.wallptr = fr.ReadUInt16();
	sect.wallnum = fr.ReadUInt16();
	sect.ceilingpicnum = fr.ReadUInt16();
	sect.floorpicnum = fr.ReadUInt16();
	sect.ceilingheinum = clamp(fr.ReadInt16() << 5, -32768, 32767);
	sect.floorheinum = clamp(fr.ReadInt16() << 5, -32768, 32767);
	sect.ceilingz = fr.ReadInt32();
	sect.floorz = fr.ReadInt32();
	sect.ceilingshade = fr.ReadInt8();
	sect.floorshade = fr.ReadInt8();
	sect.ceilingxpan_ = fr.ReadUInt8();
	sect.floorxpan_ = fr.ReadUInt8();
	sect.ceilingypan_ = fr.ReadUInt8();
	sect.floorypan_ = fr.ReadUInt8();
	sect.ceilingstat = fr.ReadUInt8();
	sect.floorstat = fr.ReadUInt8();
	sect.ceilingpal = fr.ReadUInt8();
	sect.floorpal = fr.ReadUInt8();
	sect.visibility = fr.ReadUInt8();
	sect.lotag = fr.ReadInt16();
	sect.hitag = fr.ReadInt16();
	sect.extra = fr.ReadInt16();
}


static void ReadSectorV5(FileReader& fr, sectortype& sect)
{
	sect.wallptr = fr.ReadInt16();
	sect.wallnum = fr.ReadInt16();
	sect.ceilingpicnum = fr.ReadUInt16();
	sect.floorpicnum = fr.ReadUInt16();
	sect.ceilingheinum = clamp(fr.ReadInt16() << 5, -32768, 32767);
	sect.floorheinum = clamp(fr.ReadInt16() << 5, -32768, 32767);
	sect.ceilingz = fr.ReadInt32();
	sect.floorz = fr.ReadInt32();
	sect.ceilingshade = fr.ReadInt8();
	sect.floorshade = fr.ReadInt8();
	sect.ceilingxpan_ = fr.ReadUInt8();
	sect.floorxpan_ = fr.ReadUInt8();
	sect.ceilingypan_ = fr.ReadUInt8();
	sect.floorypan_ = fr.ReadUInt8();
	sect.ceilingstat = fr.ReadUInt8();
	sect.floorstat = fr.ReadUInt8();
	sect.ceilingpal = fr.ReadUInt8();
	sect.floorpal = fr.ReadUInt8();
	sect.visibility = fr.ReadUInt8();
	sect.lotag = fr.ReadInt16();
	sect.hitag = fr.ReadInt16();
	sect.extra = fr.ReadInt16();
	if ((sect.ceilingstat & 2) == 0) sect.ceilingheinum = 0;
	if ((sect.floorstat & 2) == 0) sect.floorheinum = 0;
}

static void ReadWallV7(FileReader& fr, walltype& wall)
{
	wall.pos.x = fr.ReadInt32();
	wall.pos.y = fr.ReadInt32();
	wall.point2 = fr.ReadInt16();
	wall.nextwall = fr.ReadInt16();
	wall.nextsector = fr.ReadInt16();
	wall.cstat = fr.ReadUInt16();
	wall.picnum = fr.ReadInt16();
	wall.overpicnum = fr.ReadInt16();
	wall.shade = fr.ReadInt8();
	wall.pal = fr.ReadUInt8();
	wall.xrepeat = fr.ReadUInt8();
	wall.yrepeat = fr.ReadUInt8();
	wall.xpan_ = fr.ReadUInt8();
	wall.ypan_ = fr.ReadUInt8();
	wall.lotag = fr.ReadInt16();
	wall.hitag = fr.ReadInt16();
	wall.extra = fr.ReadInt16();
}

static void ReadWallV6(FileReader& fr, walltype& wall)
{
	wall.pos.x = fr.ReadInt32();
	wall.pos.y = fr.ReadInt32();
	wall.point2 = fr.ReadInt16();
	wall.nextsector = fr.ReadInt16();
	wall.nextwall = fr.ReadInt16();
	wall.picnum = fr.ReadInt16();
	wall.overpicnum = fr.ReadInt16();
	wall.shade = fr.ReadInt8();
	wall.pal = fr.ReadUInt8();
	wall.cstat = fr.ReadUInt16();
	wall.xrepeat = fr.ReadUInt8();
	wall.yrepeat = fr.ReadUInt8();
	wall.xpan_ = fr.ReadUInt8();
	wall.ypan_ = fr.ReadUInt8();
	wall.lotag = fr.ReadInt16();
	wall.hitag = fr.ReadInt16();
	wall.extra = fr.ReadInt16();
}

static void ReadWallV5(FileReader& fr, walltype& wall)
{
	wall.pos.x = fr.ReadInt32();
	wall.pos.y = fr.ReadInt32();
	wall.point2 = fr.ReadInt16();
	wall.picnum = fr.ReadInt16();
	wall.overpicnum = fr.ReadInt16();
	wall.shade = fr.ReadInt8();
	wall.cstat = fr.ReadUInt16();
	wall.xrepeat = fr.ReadUInt8();
	wall.yrepeat = fr.ReadUInt8();
	wall.xpan_ = fr.ReadUInt8();
	wall.ypan_ = fr.ReadUInt8();

	wall.nextsector = fr.ReadInt16();
	wall.nextwall = fr.ReadInt16();
	fr.Seek(4, FileReader::SeekSet); // skip over 2 unused 16 bit values

	wall.lotag = fr.ReadInt16();
	wall.hitag = fr.ReadInt16();
	wall.extra = fr.ReadInt16();
}

static void SetWallPalV5()
{
	for (int i = 0; i < numsectors; i++)
	{
		int startwall = sector[i].wallptr;
		int endwall = startwall + sector[i].wallnum;
		for (int w = startwall; w < endwall; w++)
		{
			wall[w].pal = sector[i].floorpal;
		}
	}
}

void ValidateSprite(spritetype& spr)
{
	int index = int(&spr - sprite);
	bool bugged = false;
	if ((unsigned)spr.statnum >= MAXSTATUS)
	{
		Printf("Sprite #%d (%d,%d) has invalid statnum %d.\n", index, spr.x, spr.y, spr.statnum);
		bugged = true;
	}
	else if ((unsigned)spr.picnum >= MAXTILES)
	{
		Printf("Sprite #%d (%d,%d) has invalid picnum %d.\n", index, spr.x, spr.y, spr.picnum);
		bugged = true;
	}
	else if ((unsigned)spr.sectnum >= (unsigned)numsectors)
	{
		int sectnum = -1;
		updatesector(spr.x, spr.y, &sectnum);

		if (!DPrintf(DMSG_WARNING, "Sprite #%d (%d,%d) with invalid sector %d was corrected to sector %d\n", index, spr.x, spr.y, spr.sectnum, sectnum))
		{
			bugged = sectnum < 0;
			if (bugged) Printf("Sprite #%d (%d,%d) with invalid sector %d\n", index, spr.x, spr.y, spr.sectnum);
		}
		spr.sectnum = sectnum;
	}
	if (bugged)
	{
		spr.clear();
		spr.statnum = MAXSTATUS;
		spr.sectnum = MAXSECTORS;
	}
}

static void ReadSpriteV7(FileReader& fr, spritetype& spr)
{
	spr.pos.x = fr.ReadInt32();
	spr.pos.y = fr.ReadInt32();
	spr.pos.z = fr.ReadInt32();
	spr.cstat = fr.ReadUInt16();
	spr.picnum = fr.ReadInt16();
	spr.shade = fr.ReadInt8();
	spr.pal = fr.ReadUInt8();
	spr.clipdist = fr.ReadUInt8();
	spr.blend = fr.ReadUInt8();
	spr.xrepeat = fr.ReadUInt8();
	spr.yrepeat = fr.ReadUInt8();
	spr.xoffset = fr.ReadInt8();
	spr.yoffset = fr.ReadInt8();
	spr.sectnum = fr.ReadInt16();
	spr.statnum = fr.ReadInt16();
	spr.ang = fr.ReadInt16();
	spr.owner = fr.ReadInt16();
	spr.xvel = fr.ReadInt16();
	spr.yvel = fr.ReadInt16();
	spr.zvel = fr.ReadInt16();
	spr.lotag = fr.ReadInt16();
	spr.hitag = fr.ReadInt16();
	spr.extra = fr.ReadInt16();
	spr.detail = 0;
	ValidateSprite(spr);
}

static void ReadSpriteV6(FileReader& fr, spritetype& spr)
{
	spr.pos.x = fr.ReadInt32();
	spr.pos.y = fr.ReadInt32();
	spr.pos.z = fr.ReadInt32();
	spr.cstat = fr.ReadUInt16();
	spr.shade = fr.ReadInt8();
	spr.pal = fr.ReadUInt8();
	spr.clipdist = fr.ReadUInt8();
	spr.xrepeat = fr.ReadUInt8();
	spr.yrepeat = fr.ReadUInt8();
	spr.xoffset = fr.ReadInt8();
	spr.yoffset = fr.ReadInt8();
	spr.picnum = fr.ReadInt16();
	spr.ang = fr.ReadInt16();
	spr.xvel = fr.ReadInt16();
	spr.yvel = fr.ReadInt16();
	spr.zvel = fr.ReadInt16();
	spr.owner = fr.ReadInt16();
	spr.sectnum = fr.ReadInt16();
	spr.statnum = fr.ReadInt16();
	spr.lotag = fr.ReadInt16();
	spr.hitag = fr.ReadInt16();
	spr.extra = fr.ReadInt16();
	spr.blend = 0;
	spr.detail = 0;
	ValidateSprite(spr);
}

static void ReadSpriteV5(FileReader& fr, spritetype& spr)
{
	spr.pos.x = fr.ReadInt32();
	spr.pos.y = fr.ReadInt32();
	spr.pos.z = fr.ReadInt32();
	spr.cstat = fr.ReadUInt16();
	spr.shade = fr.ReadInt8();
	spr.xrepeat = fr.ReadUInt8();
	spr.yrepeat = fr.ReadUInt8();
	spr.picnum = fr.ReadInt16();
	spr.ang = fr.ReadInt16();
	spr.xvel = fr.ReadInt16();
	spr.yvel = fr.ReadInt16();
	spr.zvel = fr.ReadInt16();
	spr.owner = fr.ReadInt16();
	spr.sectnum = fr.ReadInt16();
	spr.statnum = fr.ReadInt16();
	spr.lotag = fr.ReadInt16();
	spr.hitag = fr.ReadInt16();
	spr.extra = fr.ReadInt16();

	int sec = spr.sectnum;
	if ((sector[sec].ceilingstat & 1) > 0)
		spr.pal = sector[sec].ceilingpal;
	else
		spr.pal = sector[sec].floorpal;

	spr.blend = 0;
	spr.clipdist = 32;
	spr.xoffset = 0;
	spr.yoffset = 0;
	spr.detail = 0;
	ValidateSprite(spr);
}


static void insertAllSprites(const char* filename, const vec3_t* pos, int* cursectnum, int16_t numsprites)
{
	// This function is stupid because it exploits side effects of insertsprite and should be redone by only inserting the valid sprites.
	int i, realnumsprites = numsprites;

	for (i = 0; i < numsprites; i++)
	{
		bool removeit = false;
		auto& spr = sprite[i];

		if (spr.statnum == MAXSTATUS)
		{
			spr.statnum = spr.sectnum = 0;
			removeit = true;
		}

		insertsprite(spr.sectnum, spr.statnum);

		if (removeit)
		{
			sprite[i].statnum = MAXSTATUS;
			realnumsprites--;
		}
	}

	if (numsprites != realnumsprites)
	{
		for (i = 0; i < numsprites; i++)
			if (sprite[i].statnum == MAXSTATUS)
			{
				// Now remove it for real!
				sprite[i].statnum = 0;
				deletesprite(i);
			}
	}

	assert(realnumsprites == Numsprites);
}

void addBlockingPairs();

// allocates global map storage. Blood will also call this.
void allocateMapArrays(int numsprites)
{
	ClearInterpolations();

	memset(sector, 0, sizeof(*sector) * MAXSECTORS);
	memset(wall, 0, sizeof(*wall) * MAXWALLS);
	memset(sprite, 0, sizeof(*sector) * MAXSPRITES);
	memset(spriteext, 0, sizeof(spriteext_t) * MAXSPRITES);
	memset(spritesmooth, 0, sizeof(spritesmooth_t) * (MAXSPRITES + MAXUNIQHUDID));

	ClearAutomap();
	Polymost::Polymost_prepare_loadboard();
}

void engineLoadBoard(const char* filename, int flags, vec3_t* pos, int16_t* ang, int* cursectnum)
{
	inputState.ClearAllInput();

	FileReader fr = fileSystem.OpenFileReader(filename);
	if (!fr.isOpen()) I_Error("Unable to open map %s", filename);
	int mapversion = fr.ReadInt32();
	if (mapversion < 5 || mapversion > 9) // 9 is most likely useless but let's try anyway.
	{
		I_Error("%s: Invalid map format, expcted 5-9, got %d", filename, mapversion);
	}

	pos->x = fr.ReadInt32();
	pos->y = fr.ReadInt32();
	pos->z = fr.ReadInt32();
	*ang = fr.ReadInt16() & 2047;
	*cursectnum = fr.ReadUInt16();

	// Get the basics out before loading the data so that we can set up the global storage.
	numsectors = fr.ReadUInt16();
	if ((unsigned)numsectors > MAXSECTORS) I_Error("%s: Invalid map, too many sectors", filename);
	auto sectorpos = fr.Tell();
	fr.Seek((mapversion == 5 ? sectorsize5 : mapversion == 6 ? sectorsize6 : sectorsize7) * numsectors, FileReader::SeekCur);
	numwalls = fr.ReadUInt16();
	if ((unsigned)numwalls > MAXWALLS) I_Error("%s: Invalid map, too many walls", filename);
	auto wallpos = fr.Tell();
	fr.Seek((mapversion == 5 ? wallsize5 : mapversion == 6 ? wallsize6 : wallsize7)* numwalls, FileReader::SeekCur);
	int numsprites = fr.ReadUInt16();
	if ((unsigned)numsprites > MAXSPRITES) I_Error("%s: Invalid map, too many sprites", filename);
	auto spritepos = fr.Tell();

	// Now that we know the map's size, set up the globals.
	allocateMapArrays(numsprites);
	initspritelists(); // may not be used in Blood!

	// Now load the actual data.
	fr.Seek(sectorpos, FileReader::SeekSet);
	for (int i = 0; i < numsectors; i++)
	{
		switch (mapversion)
		{
		case 5: ReadSectorV5(fr, sector[i]); break;
		case 6: ReadSectorV6(fr, sector[i]); break;
		default: ReadSectorV7(fr, sector[i]); break;
		}
	}

	fr.Seek(wallpos, FileReader::SeekSet);
	for (int i = 0; i < numwalls; i++)
	{
		switch (mapversion)
		{
		case 5: ReadWallV5(fr, wall[i]); break;
		case 6: ReadWallV6(fr, wall[i]); break;
		default: ReadWallV7(fr, wall[i]); break;
		}
	}

	fr.Seek(spritepos, FileReader::SeekSet);
	for (int i = 0; i < numsprites; i++)
	{
		switch (mapversion)
		{
		case 5: ReadSpriteV5(fr, sprite[i]); break;
		case 6: ReadSpriteV6(fr, sprite[i]); break;
		default: ReadSpriteV7(fr, sprite[i]); break;
		}
	}

	artSetupMapArt(filename);
	insertAllSprites(filename, pos, cursectnum, numsprites);

	for (int i = 0; i < numsprites; i++)
	{
		if ((sprite[i].cstat & 48) == 48) sprite[i].cstat &= ~48;
	}
	//Must be last.
	updatesector(pos->x, pos->y, cursectnum);
	guniqhudid = 0;
	fr.Seek(0, FileReader::SeekSet);
	auto buffer = fr.Read();
	unsigned char md4[16];
	md4once(buffer.Data(), buffer.Size(), md4);
	G_LoadMapHack(filename, md4);
	setWallSectors();
	hw_BuildSections();
	sectorGeometry.SetSize(numsections);


	memcpy(wallbackup, wall, sizeof(wallbackup));
	memcpy(sectorbackup, sector, sizeof(sectorbackup));
}


void qloadboard(const char* filename, char flags, vec3_t* dapos, int16_t* daang, int* dacursectnum);


// loads a map into the backup buffer.
void loadMapBackup(const char* filename)
{
	vec3_t pos;
	int16_t scratch;
	int scratch2;

	if (isBlood())
	{
		qloadboard(filename, 0, &pos, &scratch, &scratch2);
	}
	else
	{
		engineLoadBoard(filename, 0, &pos, &scratch, &scratch2);
		initspritelists();
	}
}

// Sets the sector reference for each wall. We need this for the triangulation cache.
void setWallSectors()
{
	for (int i = 0; i < numsectors; i++)
	{
		sector[i].dirty = 255;
		sector[i].exflags = 0;
		for (int w = 0; w < sector[i].wallnum; w++)
		{
			wall[sector[i].wallptr + w].sector = i;
		}
	}

	// validate 'nextsector' fields. Some maps have these wrong which can cause render glitches and occasionally even crashes.
	for (int i = 0; i < numwalls; i++)
	{
		if (wall[i].nextwall != -1)
		{
			if (wall[i].nextsector != wall[wall[i].nextwall].sector)
			{
				DPrintf(DMSG_ERROR, "Bad 'nextsector' reference %d on wall %d\n", wall[i].nextsector, i);
				wall[i].nextsector = wall[wall[i].nextwall].sector;
			}
		}
	}

}
