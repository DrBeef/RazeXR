/*
 * mathutil.c
 * Mathematical utility functions to emulate MACT
 *
 * by Jonathon Fowler
 *
 * Since we weren't given the source for MACT386.LIB so I've had to do some
 * creative interpolation here.
 *
 */
//-------------------------------------------------------------------------
/*
Duke Nukem Copyright (C) 1996, 2003 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

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
*/
//-------------------------------------------------------------------------

#include <algorithm>
#include "types.h"
#include <stdlib.h>

// This extracted from the Rise of the Triad source RT_UTIL.C :-|

int FindDistance2D(int x, int y)
{
  x= abs(x);        /* absolute values */
  y= abs(y);

  if (x<y)
     std::swap(x,y);

  int t = y + (y>>1);

  return (x - (x>>5) - (x>>7)  + (t>>2) + (t>>6));
}

double fFindDistance2D(int x, int y)
{
  x= abs(x);        /* absolute values */
  y= abs(y);

  if (x<y)
     std::swap(x,y);

  double t = y + (y * (1. / 2.));

  return (x - (x * (1. / 32.)) - (x * (1. / 128.))  + (t * (1. / 4.)) + (t * (1. / 64.)));
}


int FindDistance3D(int x, int y, int z)
{
   x= abs(x);           /* absolute values */
   y= abs(y);
   z= abs(z >> 4);

   if (x<y)
     std::swap(x,y);

   if (x<z)
     std::swap(x,z);

   int t = y + z;

   return (x - (x>>4) + (t>>2) + (t>>3));
}

