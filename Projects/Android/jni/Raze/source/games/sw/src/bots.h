//-------------------------------------------------------------------------
/*
Copyright (C) 1997, 2005 - 3D Realms Entertainment

This file is part of Shadow Warrior version 1.2

Shadow Warrior is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

Original Source: 1997 - Frank Maddin and Jim Norwood
Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

// BOTS.H
// Contains useful header information for bot creation

#ifndef BOTS_H
#define BOTS_H

BEGIN_SW_NS

// BOT DEFINITIONS AND STRUCTURES

enum BOT_Actions
{
    BOTstand, BOThide, BOTrun, BOTduck, BOTjump, BOTstrafe, BOTshoot, BOTuseinv,
    BOTopen, BOTswimup, BOTswimdown, BOTturn, BOTuserts
};

// Linked lists containing node trees that are chosen based on desired actions

struct NODE
{
    NODE* p,* l,* r;              // Pointers to tree nodes
    int goalx, goaly, goalz;   // x,y,z point bot wants to get to
    BOT_Actions action;         // Action to take if this node is reached
    int tics;                  // Optionally stay in this node for x tics.
};

struct NODETREE
{
    NODE* tree;             // This is the node tree used to navigate to goal
    bool Locked;            // If list is locked, a bot is using/modifying it and
    // other bots cannot modify it while it's locked
};

// Bots main action variables
struct BOT_BRAIN
{
    int16_t tgt_inv;      // Inventory item it wants to use
    int16_t tgt_weapon;   // weapon in wants to activate and use
    int16_t tgt_enemy;    // Enemy it wants to kill
    int16_t tgt_sprite;   // Sprite it wants to pickup or operate
    int16_t tgt_sector;   // Sector it wants to get to
    int16_t tgt_wall;     // Wall it wants to touch
    BOT_Actions action; // Bot's current action
};

// NOTE:
// The following arrays should be saved off with save games!

// 0  = Item not accessible, no item of type was found
// 1  = Shuriken
// 3  = Caltrops
// 4  = Gas Bomb
// 5  = Flash Bomb
// 6  = Uzi Ammo
// 7  = Shotgun Ammo
// 8  = Rocket Ammo
// 9  = 40mm Ammo
// 10 = Sticky Bombs
// 11 = Rail Ammo
// 12 = Head Ammo
// 13 = Heart Ammo
// 14 = Uzi
// 15 = Shotgun
// 16 = Rocket Launcher
// 17 = 40mm Launcher
// 18 = Rail Gun
// 19 = Head
// 20 = Heart
// 21 = MedKit
// 22 = Armor
// 23 = Big Armor
// 24 = Portable MedKit
// 25 = Fortune Cookie
////////////////////////
extern NODETREE BOT_TREELIST[25][50]; // There can be up to 50 of each item
// with a cooresponding search tree for each

END_SW_NS

#endif
