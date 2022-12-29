#pragma once
#include "build.h"

class FSerializer;
struct IntRect;

void render_drawrooms(spritetype* playersprite, const vec3_t& position, int sectnum, binangle angle, fixedhoriz horizon, binangle rollang, double smoothratio);
void render_camtex(spritetype* playersprite, const vec3_t& position, int sectnum, binangle angle, fixedhoriz horizon, binangle rollang, FGameTexture* camtex, IntRect& rect, double smoothratio);

struct PortalDesc
{
	int type;
	int dx, dy, dz;
	TArray<int> targets;
};

FSerializer& Serialize(FSerializer& arc, const char* key, PortalDesc& obj, PortalDesc* defval);


extern TArray<PortalDesc> allPortals;

inline void portalClear()
{
	allPortals.Clear();
}

inline int portalAdd(int type, int target, int dx = 0, int dy = 0, int dz = 0)
{
	auto& pt = allPortals[allPortals.Reserve(1)];
	pt.type = type;
	if (target >= 0) pt.targets.Push(target);
	pt.dx = dx;
	pt.dy = dy;
	pt.dz = dz;
	return allPortals.Size() - 1;
}

// merges portals in adjoining sectors.
inline void mergePortals()
{
	//Printf("Have %d portals\n", allPortals.Size());
	bool didsomething = true;
	while (didsomething)
	{
		didsomething = false;
		for (unsigned i = 0; i < allPortals.Size(); i++)
		{
			auto& pt1 = allPortals[i];
			if (pt1.type == PORTAL_SECTOR_CEILING || pt1.type == PORTAL_SECTOR_FLOOR)
			{
				for (unsigned j = i + 1; j < allPortals.Size(); j++)
				{
					auto& pt2 = allPortals[j];
					if (pt1.type != pt2.type || pt1.dx != pt2.dx || pt1.dy != pt2.dy || pt1.dz != pt2.dz) continue;
					for (unsigned s = 0; s < pt1.targets.Size() && pt2.targets.Size(); s++)
					{
						for (unsigned t = 0; t < pt2.targets.Size(); t++)
						{
							if (findwallbetweensectors(pt1.targets[s], pt2.targets[t]) >= 0)
							{
								pt1.targets.Append(pt2.targets);
								pt2.targets.Reset();
								pt2.type = -1;
								for (int n = 0; n < numsectors; n++)
								{
									//Printf("Merged %d and %d\n", i, j);
									if (sector[n].portalnum == (int)j) sector[n].portalnum = i;
								}
								didsomething = true;
								break;
							}
						}
					}
				}
			}
		}
	}
}
