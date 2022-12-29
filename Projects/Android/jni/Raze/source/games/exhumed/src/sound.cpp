//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT, sirlemonhead
This file is part of PCExhumed.
PCExhumed is free software; you can redistribute it and/or
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
#include "ns.h"
#include "compat.h"
#include "build.h"
#include "engine.h"
#include "exhumed.h"
#include "sound.h"
#include "aistuff.h"
#include "player.h"
#include "sequence.h"
#include "raze_sound.h"
#include "mapinfo.h"

BEGIN_PS_NS

const char *SoundFiles[kMaxSoundFiles] =
{
  "spl_big",
  "spl_smal",
  "bubble_l",
  "grn_drop",
  "p_click",
  "grn_roll",
  "cosprite",
  "m_chant0",
  "anu_icu",
  "item_reg",
  "item_spe", // 10
  "item_key",
  "torch_on", // 12
  "jon_bnst",
  "jon_gasp",
  "jon_land",
  "jon_gags",
  "jon_fall",
  "jon_drwn",
  "jon_air1",
  "jon_glp1", // 20
  "jon_bbwl",
  "jon_pois",
  "amb_ston",
  "cat_icu",
  "bubble_h",
  "set_land",
  "jon_hlnd",
  "jon_laf2",
  "spi_jump",
  "jon_scub", // 30
  "item_use",
  "tr_arrow",
  "swi_foot",
  "swi_ston",
  "swi_wtr1",
  "tr_fire",
  "m_skull5",
  "spi_atak",
  "anu_hit",
  "fishdies", // 40
  "scrp_icu",
  "jon_wade",
  "amb_watr",
  "tele_1",
  "wasp_stg",
  "res",
  "drum4",
  "rex_icu",
  "m_hits_u",
  "q_tail", // 50
  "vatr_mov",
  "jon_hit3",
  "jon_t_2", // 53
  "jon_t_1",
  "jon_t_5",
  "jon_t_6",
  "jon_t_8",
  "jon_t_4",
  "rasprit1",
  "jon_fdie", // 60
  "wijaf1",
  "ship_1",
  "saw_on",
  "ra_on",
  "amb_ston", // 65
  "vatr_stp", // 66
  "mana1",
  "mana2",
  "ammo",
  "pot_pc1", // 70?
  "pot_pc2",
  "weapon",
  "alarm",
  "tick1",
  "scrp_zap", // 75
  "jon_t_3",
  "jon_laf1",
  "blasted",
  "jon_air2" // 79
};

short nStopSound;
short nStoneSound;
short nSwitchSound;
short nLocalEyeSect;
short nElevSound;
short nCreepyTimer;

bool looped[kMaxSounds];

short StaticSound[kMaxSounds];
int fakesources[] = { 0, 1, 2, 3 };
int swirlysources[4]= { 0, 1, 2, 3 };
FVector3 amb, creepy;

int nLocalChan = 0;

//==========================================================================
//
//
//
//==========================================================================

class EXSoundEngine : public SoundEngine
{
    // client specific parts of the sound engine go in this class.
    void CalcPosVel(int type, const void* source, const float pt[3], int channum, int chanflags, FSoundID chanSound, FVector3* pos, FVector3* vel, FSoundChan* chan) override;
    TArray<uint8_t> ReadSound(int lumpnum) override;

public:
    EXSoundEngine()
    {
        S_Rolloff.RolloffType = ROLLOFF_Doom;
        S_Rolloff.MinDistance = 0;
        S_Rolloff.MaxDistance = 1536;
    }

    void StopChannel(FSoundChan* chan) override
    {
        if (chan && chan->SysChannel != NULL && !(chan->ChanFlags & CHANF_EVICTED) && chan->SourceType == SOURCE_Actor)
        {
            chan->Source = NULL;
            chan->SourceType = SOURCE_Unattached;
        }
        SoundEngine::StopChannel(chan);
    }

};

//==========================================================================
//
//
// 
//==========================================================================

TArray<uint8_t> EXSoundEngine::ReadSound(int lumpnum)
{
    auto wlump = fileSystem.OpenFileReader(lumpnum);
    return wlump.Read();
}


//==========================================================================
//
//
//
//==========================================================================

int LoadSound(const char* name)
{
    FString nname(name, 8);
    int sndid = soundEngine->FindSoundNoHash(nname.GetChars());
    if (sndid > 0) return sndid - 1;

    FStringf filename("%s.voc", nname.GetChars());
    auto lump = S_LookupSound(filename);
    if (lump > 0)
    {
        auto check = fileSystem.GetFileData(lump);
		bool loops = false;
        if (check.Size() > 26 && check[26] == 6 && !memcmp("Creative Voice File", check.Data(), 19))
        {
            // This game uses the actual loop point information in the sound data as its only means to check if a sound is looped.
            loops = true;
        }
		int retval = soundEngine->AddSoundLump(nname, lump, 0, -1, 6);
        soundEngine->CacheSound(retval);
		looped[retval-1] = loops;
        return retval - 1;
    }
    else if (!isShareware())  // demo tries to load sound files it doesn't have
    {
        Printf("Unable to open sound '%s'!\n", filename.GetChars());
    }
    return -1;
}
//==========================================================================
//
//
//
//==========================================================================

void InitFX(void)
{
    if (soundEngine) return; // just in case.
    soundEngine = new EXSoundEngine;

    auto& S_sfx = soundEngine->GetSounds();
    S_sfx.Resize(1);
    for (size_t i = 0; i < kMaxSoundFiles; i++)
    {
        StaticSound[i] = LoadSound(SoundFiles[i]);
    }
    soundEngine->HashSounds();
    nCreepyTimer = kCreepyCount;
}


//==========================================================================
//
//
//
//==========================================================================

void GetSpriteSoundPitch(int* pVolume, int* pPitch)
{
    int nSoundSect = PlayerList[nLocalPlayer].nPlayerViewSect;
    int nLocalSectFlags = SectFlag[nSoundSect];
    if (nLocalSectFlags & kSectUnderwater)
    {
		if (*pVolume == 255)
		{
			*pVolume >>= 1;
			*pPitch -= 1200;
		}
	}
	else
	{
		if (*pVolume < 255)
		{
			*pVolume = 255;
			*pPitch += 1200;
		}
    }
}

//==========================================================================
//
//
//
//==========================================================================

void BendAmbientSound(void)
{
    soundEngine->EnumerateChannels([](FSoundChan* chan)
        {
            if (chan->SourceType == SOURCE_Ambient && chan->Source == &amb)
            {
                soundEngine->SetPitch(chan, (nDronePitch + 11800) / 11025.f);
            }
            return 1;
        });
}

//==========================================================================
//
//
//
//==========================================================================

void PlayLocalSound(short nSound, short nRate, bool unattached, EChanFlags cflags)
{
    if (!SoundEnabled()) return;
    if (nSound < 0 || nSound >= kMaxSounds || !soundEngine->isValidSoundId(nSound + 1))
    {
        Printf("PlayLocalSound: Invalid sound nSound == %i, nRate == %i\n", nSound, nRate);
        return;
    }
    if (looped[nSound]) cflags |= CHANF_LOOP;

    FSoundChan* chan;
    if (!unattached)
    {
        soundEngine->StopSound(SOURCE_None, nullptr, CHAN_BODY);
        chan = soundEngine->StartSound(SOURCE_None, nullptr, nullptr, CHAN_BODY, cflags, nSound + 1, 1.f, ATTN_NONE, nullptr);
    }
    else
    {
        chan = soundEngine->StartSound(SOURCE_None, nullptr, nullptr, CHAN_VOICE, CHANF_OVERLAP|cflags, nSound + 1, 1.f, ATTN_NONE, nullptr);
    }

    if (nRate && chan)
    {
        float ratefac = (11025 + nRate) / 11025.f;
        soundEngine->SetPitch(chan, ratefac);
    }
}

//==========================================================================
//
//
//
//==========================================================================

int LocalSoundPlaying(void)
{
    return soundEngine->EnumerateChannels([](FSoundChan* chan)
        {
            return chan->SourceType == SOURCE_None;
        });
}

//==========================================================================
//
//
//
//==========================================================================

void StopLocalSound(void)
{
    soundEngine->StopSound(SOURCE_None, nullptr, -1);
}

//==========================================================================
//
//
//
//==========================================================================
int nNextFreq;
int nSwirlyFrames;

void StartSwirly(int nActiveSound)
{
    if (!SoundEnabled()) return;
    auto &swirly = swirlysources[nActiveSound];

    short nPitch = nNextFreq - RandomSize(9);
    nNextFreq = 25000 - RandomSize(10) * 6;
    if (nNextFreq > 32000)
        nNextFreq = 32000;

    int nVolume = nSwirlyFrames + 1;
    if (nVolume >= 220)
        nVolume = 220;

    soundEngine->StopSound(SOURCE_Swirly, &swirly, -1);
    soundEngine->StartSound(SOURCE_Swirly, &swirly, nullptr, CHAN_BODY, CHANF_TRANSIENT, StaticSound[kSoundMana1]+1, nVolume / 255.f, ATTN_NONE, nullptr, nPitch / 11025.f);
}

//==========================================================================
//
//
//
//==========================================================================

void StartSwirlies()
{
    StopAllSounds();

    nNextFreq = 19000;
    nSwirlyFrames = 0;

    for (int i = 0; i <= 4; i++)
        StartSwirly(i);
}

//==========================================================================
//
//
//
//==========================================================================

void UpdateSwirlies()
{
    nSwirlyFrames++;
    for (int i = 0; i <= 4; i++)
    {
        if (!soundEngine->IsSourcePlayingSomething(SOURCE_Swirly, &swirlysources[i], -1))
            StartSwirly(i);
    }
}

//==========================================================================
//
//
//
//==========================================================================

void SoundBigEntrance(void)
{
    if (!SoundEnabled()) return;
    StopAllSounds();
    for (int i = 0; i < 4; i++)
    {
        short nPitch = 11025 + (i * 512 - 1200);
        //pASound->snd_pitch = nPitch;
        soundEngine->StopSound(SOURCE_EXBoss, &fakesources[i], -1);
        soundEngine->StartSound(SOURCE_EXBoss, &fakesources[i], nullptr, CHAN_BODY, CHANF_TRANSIENT, StaticSound[kSoundTorchOn]+1, 200 / 255.f, ATTN_NONE, nullptr, nPitch / 11025.f);
    }
}


//==========================================================================
//
//
//
//==========================================================================

void EXSoundEngine::CalcPosVel(int type, const void* source, const float pt[3], int channum, int chanflags, FSoundID chanSound, FVector3* pos, FVector3* vel, FSoundChan* chan)
{
    if (pos != nullptr)
    {
        vec3_t campos;
        if (nSnakeCam > -1)
        {
            Snake* pSnake = &SnakeList[nSnakeCam];
            spritetype* pSnakeSprite = &pSnake->pSprites[0]->s();
            campos.x = pSnakeSprite->x;
            campos.y = pSnakeSprite->y;
            campos.z = pSnakeSprite->z;
        }
        else
        {
            campos = { initx, inity, initz };
        }
        auto fcampos = GetSoundPos(&campos);

        if (vel) vel->Zero();

        if (type == SOURCE_Ambient)
        {
            *pos = *(FVector3*)source;
        }
        else if (type == SOURCE_Unattached)
        {
            pos->X = pt[0];
            pos->Y = pt[1];
            pos->Z = pt[2];
        }
        // Do some angular magic. The original was just 2D panning which in a 3D sound field is not sufficient.
        else if (type == SOURCE_Swirly)
        {
            int which = *(int*)source;
            double phase = (PlayClock << (4 + which)) * BAngRadian;
            pos->X = fcampos.X + float(256 * cos(phase));
            pos->Z = fcampos.Z + float(256 * sin(phase));
        }
        else  if (type == SOURCE_EXBoss)
        {
            int which = *(int*)source;
            *pos = fcampos;
            // Should be positioned in 90� intervals.
            switch (which)
            {
            default:
            case 0: pos->X -= 256; break;
            case 1: pos->Z -= 256; break;
            case 2: pos->X += 256; break;
            case 3: pos->Z += 256; break;
            }
        }
        else if (type == SOURCE_Actor)
        {
            auto actor = (spritetype*)source;
            assert(actor != nullptr);
            if (actor != nullptr)
            {
                *pos = GetSoundPos(&actor->pos);
            }
        }
        if ((chanflags & CHANF_LISTENERZ) && type != SOURCE_None)
        {
            pos->Y = fcampos.Y;
        }
    }
}

//==========================================================================
//
//
//
//==========================================================================

void GameInterface::UpdateSounds()
{
    if (nFreeze)
        return;

    vec3_t pos;
    short ang;
    if (nSnakeCam > -1)
    {
        Snake *pSnake = &SnakeList[nSnakeCam];
        spritetype *pSnakeSprite = &pSnake->pSprites[0]->s();
        pos = pSnakeSprite->pos;
        ang = pSnakeSprite->ang;
    }
    else
    {
        pos = { initx, inity, initz };
        ang = inita;
    }
    auto fv = GetSoundPos(&pos);
    SoundListener listener;
    listener.angle = float(-ang * BAngRadian); // Build uses a period of 2048.
    listener.velocity.Zero();
    listener.position = GetSoundPos(&pos);
    listener.underwater = false;
    // This should probably use a real environment instead of the pitch hacking in S_PlaySound3D.
    // listenactor->waterlevel == 3;
    //assert(primaryLevel->Zones.Size() > listenactor->Sector->ZoneNumber);
    listener.Environment = 0;// primaryLevel->Zones[listenactor->Sector->ZoneNumber].Environment;
    listener.valid = true;


    soundEngine->SetListener(listener);
    soundEngine->EnumerateChannels([](FSoundChan* chan)
        {
            if (!(chan->ChanFlags & (CHANF_UI|CHANF_FORGETTABLE)))
            {
                int nVolume = int(chan->Volume * 255);
                int nPitch = int(chan->Pitch * (11025.f / 128.f)) - 11025;
				int oVolume = nVolume;
                GetSpriteSoundPitch(&nVolume, &nPitch);
				if (oVolume != nVolume)
				{
					soundEngine->SetPitch(chan, (11025 + nPitch) / 11025.f);
					soundEngine->SetVolume(chan, nVolume / 255.f);
				}
			}
            return 0;
        });
}

//==========================================================================
//
//
//
//==========================================================================

int soundx, soundy, soundz;
short soundsect;

void PlayFX2(unsigned short nSound, DExhumedActor* pActor, int sectf, EChanFlags chanflags, int sprflags)
{
    if (!SoundEnabled()) return;
    if ((nSound&0x1ff) >= kMaxSounds || !soundEngine->isValidSoundId((nSound & 0x1ff)+1))
    {
        Printf("PlayFX2: Invalid sound nSound == %i\n", nSound);
        return;
    }

    bool fullvol = false, hiprio = false;
    if (pActor)
    {
        fullvol = (sprflags & 0x2000) != 0;
        hiprio = (sprflags & 0x4000) != 0;
        soundx = pActor->s().x;
        soundy = pActor->s().y;
        soundz = pActor->s().z;
    }

    int nVolume = 255;

    bool allowMultiple = (nSound & 0x2000) != 0;
    bool forcePlay = (nSound & 0x1000) != 0;
    bool midprio = (nSound & 0x4000) != 0;

    int prio = 0;
    if (forcePlay || midprio) prio = 1000;
    else if (pActor != nullptr && hiprio) prio = 2000;

    short v10 = (nSound&0xe00)>>9;
    nSound &= 0x1ff;

    int nPitch = 0;
    if (v10) nPitch = -(totalmoves&((1<<v10)-1))*16;

    GetSpriteSoundPitch(&nVolume, &nPitch);

    vec3_t v = { soundx, soundy, soundz };
    FVector3 vv = GetSoundPos(&v);

    // Check if this sound is allowed to play or if it must stop some other sound.
    if (!forcePlay)
    {
        bool res = soundEngine->EnumerateChannels([=](FSoundChan* chan)
            {
                if (chan->SourceType == SOURCE_Actor && pActor != nullptr)
                {
                    if (prio >= chan->UserData)
                    {
                        if (chan->SoundID == nSound + 1)
                        {
                            if (!allowMultiple && &pActor->s() == chan->Source)
                                return 1;
                        }
                        else if (&pActor->s() == chan->Source)
                        {
                            soundEngine->StopChannel(chan);
                            return -1;
                        }
                    }

                }
                else if (chan->SourceType == SOURCE_Unattached && pActor != nullptr)
                {
                    if (chan->SoundID == nSound + 1)
                    {
                        if (vv.X == chan->Point[0] && vv.Y == chan->Point[1] && vv.Z == chan->Point[2])
                            return 1;
                    }

                }
                return 0;
            });
        if (res) return;
    }
    FSoundChan* chan = nullptr;
    if (pActor != nullptr)
    {
        chan = soundEngine->StartSound(SOURCE_Actor, &pActor->s(), nullptr, CHAN_BODY, chanflags| CHANF_OVERLAP, nSound+1, nVolume / 255.f,fullvol? 0.5f : ATTN_NORM, nullptr, (11025 + nPitch) / 11025.f);
    }
    else
    {
        chan = soundEngine->StartSound(SOURCE_Unattached, nullptr, &vv, CHAN_BODY, chanflags | CHANF_OVERLAP, nSound+1, nVolume / 255.f, ATTN_NORM, nullptr, (11025 + nPitch) / 11025.f);
    }
    if (chan)
    {
        if (sectf) chan->UserData = 2000;
        else chan->UserData = prio;
    }

    // Nuke: added nSprite >= 0 check
    if (pActor != PlayerList[nLocalPlayer].Actor() && pActor != nullptr && (pActor->s().cstat&257))
        nCreepyTimer = kCreepyCount;
}

//==========================================================================
//
//
//
//==========================================================================

void PlayFXAtXYZ(unsigned short ax, int x, int y, int z, int nSector, EChanFlags chanflags, int sectf)
{
    soundx = x;
    soundy = y;
    soundz = z;
    soundsect = nSector;
    PlayFX2(ax, nullptr, sectf, chanflags);
}

//==========================================================================
//
//
//
//==========================================================================

void CheckAmbience(int nSector)
{
    if (!SoundEnabled()) return;
    if (SectSound[nSector] != -1)
    {
        int nSector2 = SectSoundSect[nSector];
        walltype* pWall = &wall[sector[nSector2].wallptr];
        if (!soundEngine->IsSourcePlayingSomething(SOURCE_Ambient, &amb, 0))
        {
            vec3_t v = { pWall->x, pWall->y, sector[nSector2].floorz };
            amb = GetSoundPos(&v);
            soundEngine->StartSound(SOURCE_Ambient, &amb, nullptr, CHAN_BODY, CHANF_TRANSIENT, SectSound[nSector] + 1, 1.f, ATTN_NORM);
            return;
        }
        soundEngine->EnumerateChannels([=](FSoundChan* chan)
            {
                if (chan->SourceType == SOURCE_Ambient)
                {
                    if (nSector == nSector2)
                    {
                        spritetype* pSprite = &PlayerList[0].Actor()->s();
                        amb = GetSoundPos(&pSprite->pos);
                    }
                    else
                    {
                        vec3_t v = { pWall->x, pWall->y, sector[nSector2].floorz };
                        amb = GetSoundPos(&v);
                    }
                    return 1;
                }
                return 0;
            });

    }
    else
    {
        soundEngine->StopSound(SOURCE_Ambient, &amb, -1);
    }
}


//==========================================================================
//
//
//
//==========================================================================

void UpdateCreepySounds()
{
    if ((currentLevel->gameflags & LEVEL_EX_COUNTDOWN) || nFreeze || !SoundEnabled())
        return;
    spritetype* pSprite = &PlayerList[nLocalPlayer].Actor()->s();
    nCreepyTimer--;
    if (nCreepyTimer <= 0)
    {
        if (nCreaturesKilled < nCreaturesTotal && !(SectFlag[PlayerList[nLocalPlayer].nPlayerViewSect] & 0x2000))
        {
            int vsi = seq_GetFrameSound(SeqOffsets[kSeqCreepy], totalmoves % SeqSize[SeqOffsets[kSeqCreepy]]);
            if (vsi >= 0 && (vsi & 0x1ff) < kMaxSounds)
            {
                int vdx = (totalmoves + 32) & 31;
                if (totalmoves & 1)
                    vdx = -vdx;
                int vax = (totalmoves + 32) & 63;
                if (totalmoves & 2)
                    vax = -vax;

                vec3_t sp = { pSprite->x + vdx, pSprite->y + vax, pSprite->z };
                creepy = GetSoundPos(&sp);

                if ((vsi & 0x1ff) >= kMaxSounds || !soundEngine->isValidSoundId((vsi & 0x1ff) + 1))
                {
                    return;
                }

                int nVolume = 255;
                short v10 = (vsi & 0xe00) >> 9;
                vsi &= 0x1ff;

                int nPitch = 0;
                if (v10) nPitch = -(totalmoves & ((1 << v10) - 1)) * 16;

                GetSpriteSoundPitch(&nVolume, &nPitch);
                soundEngine->StopSound(SOURCE_Ambient, &creepy, CHAN_BODY);
                soundEngine->StartSound(SOURCE_Ambient, &creepy, nullptr, CHAN_BODY, CHANF_TRANSIENT, vsi + 1, nVolume / 255.f, ATTN_NONE, nullptr, (11025 + nPitch) / 11025.f);
            }
        }
        nCreepyTimer = kCreepyCount;
    }
}


//==========================================================================
//
//
//
//==========================================================================

void StopActorSound(DExhumedActor *pActor)
{
    if (pActor)
        soundEngine->StopSound(SOURCE_Actor, &pActor->s(), -1);
}

void StopAllSounds(void)
{
    soundEngine->StopAllChannels();
}

//==========================================================================
//
//
//
//==========================================================================

void PlayTitleSound(void)
{
    PlayLocalSound(StaticSound[kSoundItemSpecial], 0, false, CHANF_UI);
}

void PlayGameOverSound(void)
{
    PlayLocalSound(StaticSound[kSoundJonLaugh2], 0, false, CHANF_UI);
}

DEFINE_ACTION_FUNCTION(_Exhumed, PlayLocalSound)
{
    PARAM_PROLOGUE;
    PARAM_INT(snd);
    PARAM_INT(pitch);
    PARAM_BOOL(unatt);
    PARAM_INT(flags);
    PlayLocalSound(StaticSound[snd], pitch, unatt, EChanFlags::FromInt(flags));
    return 0;
}

DEFINE_ACTION_FUNCTION_NATIVE(_Exhumed, StopLocalSound, StopLocalSound)
{
    StopLocalSound();
    return 0;
}

DEFINE_ACTION_FUNCTION(_Exhumed, LocalSoundPlaying)
{
    ACTION_RETURN_BOOL(soundEngine->IsSourcePlayingSomething(SOURCE_None, nullptr, CHAN_AUTO, -1));
}

DEFINE_ACTION_FUNCTION(_Exhumed, PlayCDTrack)
{
    PARAM_PROLOGUE;
    PARAM_INT(track);
    PARAM_BOOL(loop);
    playCDtrack(track, loop);
    return 0;
}

END_PS_NS
