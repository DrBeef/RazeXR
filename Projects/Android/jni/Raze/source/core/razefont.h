#pragma once
#include "v_font.h"
#include "c_cvars.h"

EXTERN_CVAR(Int, duke_menufont)

extern FFont* IndexFont;
extern FFont* DigiFont;

void InitFont();
void SetupFontSubstitution();
FFont* PickBigFont(const char* txt = nullptr);
FFont* PickSmallFont(const char* txt = nullptr);
