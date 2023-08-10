#define INTERNAL_SRC_CODE_CODE_800A9F30_C
#include "global.h"
#include <math.h>
#include "padmgr.h"
#include "port/player/players.h"
#include "def/code_800A9F30.h"
#include "def/code_800D2E30.h"
#include "def/padmgr.h"

RumbleStruct g_Rumble;

void Rumble_Init(PadMgr* a, s32 b)
{
	Rumble_Update(&g_Rumble);
	PadMgr_RumbleSet(a, g_Rumble.rumbleOn);
}

// Used for bosses and fishing
// distance to player (unused), strength, time, decay
void Rumble_Shake2(f32 playerDistance, u8 baseStrength, u8 length, u8 decay)
{
	s32 distance_decay; // By how much should the rumble effect be lowered, based on the distance
	s32 strength;

	if(1000000.0f < playerDistance)
	{
		distance_decay = 1000;
	}
	else
	{
		distance_decay = sqrtf(playerDistance);
	}

	if((distance_decay < 1000) && (baseStrength != 0) && (decay != 0))
	{
		strength = baseStrength - (distance_decay * 255) / 1000;

		if(strength > 0)
		{
			oot::player(0).SendMotorEvent(length, strength, decay);

			g_Rumble.strength = strength;
			g_Rumble.length = length;
			g_Rumble.decay = decay;
		}
	}
}


void Rumble_Update(RumbleStruct* rumbleCtx)
{
	static u8 reset_needed = 1;
	s32 i;
	s32 temp;
	s32 index = -1; // Index into the list with the most rumble

	for(i = 0; i < 4; i++)
	{
		rumbleCtx->rumbleOn[i] = 0;
	}

	if(rumbleCtx->reset == 0)
	{
		if(reset_needed != 0)
		{
			for(i = 0; i < 4; i++)
			{
				gPadMgr.pakType[i] = 0;
			}
		}
		reset_needed = rumbleCtx->reset; // Don't reset again next frame
		return;
	}

	reset_needed = rumbleCtx->reset; // Allow resetting

	if(rumbleCtx->state == 2)
	{ // Perform full clear of all data
		for(i = 0; i < 4; ++i)
		{
			gPadMgr.pakType[i] = 0;
		}

		for(i = 0; i < 0x40; i++)
		{
			rumbleCtx->strengthList_easing[i] = 0;
			rumbleCtx->decayList[i] = 0;
			rumbleCtx->lengthList[i] = 0;
			rumbleCtx->strengthList[i] = 0;
		}
		rumbleCtx->timer1 = rumbleCtx->timer2 = rumbleCtx->strength = rumbleCtx->length = rumbleCtx->decay = rumbleCtx->strength_easing = 0;
		rumbleCtx->state = 1; // Normal operation mode
	}
	if(rumbleCtx->state != 0)
	{
		for(i = 0; i < 0x40; i++)
		{
			if(rumbleCtx->strengthList[i] != 0)
			{
				if(rumbleCtx->lengthList[i] > 0)
				{
					rumbleCtx->lengthList[i]--;
				}
				else
				{
					temp = rumbleCtx->strengthList[i] - rumbleCtx->decayList[i];
					if(temp > 0)
					{
						rumbleCtx->strengthList[i] = temp;
					}
					else
					{
						rumbleCtx->strengthList[i] = 0;
					}
				}

				temp = rumbleCtx->strengthList_easing[i] + rumbleCtx->strengthList[i];
				rumbleCtx->strengthList_easing[i] = temp;
				if(index == -1)
				{
					index = i;
					rumbleCtx->rumbleOn[0] = (temp >= 0x100);
				}
				else if(rumbleCtx->strengthList[index] < rumbleCtx->strengthList[i])
				{
					index = i;
					rumbleCtx->rumbleOn[0] = (temp >= 0x100);
				}
			}
		}
		if(rumbleCtx->strength != 0)
		{
			if(rumbleCtx->length > 0)
			{
				rumbleCtx->length--;
			}
			else
			{
				temp = rumbleCtx->strength - rumbleCtx->decay;
				if(temp > 0)
				{
					rumbleCtx->strength = temp;
				}
				else
				{
					rumbleCtx->strength = 0;
				}
			}
			temp = rumbleCtx->strength_easing + rumbleCtx->strength;
			rumbleCtx->strength_easing = temp;
			rumbleCtx->rumbleOn[0] = (temp >= 0x100);
		}
		if(rumbleCtx->strength != 0)
		{
			temp = rumbleCtx->strength;
		}
		else
		{
			// Write the largest rumble value in temp
			if(index == -1)
			{
				temp = 0;
			}
			else
			{
				temp = rumbleCtx->strengthList[index];
			}
		}
		if(temp == 0)
		{ // No rumble
			if((++rumbleCtx->timer2) >= 6)
			{
				rumbleCtx->timer1 = 0; // Clear emergency timer
				rumbleCtx->timer2 = 5;
			}
		}
		else
		{
			rumbleCtx->timer2 = 0;
			if((++rumbleCtx->timer1) >= 0x1C21)
			{			      // Has been rumbling for a very long time?
				rumbleCtx->state = 0; // Clear rumble data
			}
		}
	}
	else
	{ // Clear rumble data
		for(i = 0; i < 0x40; i++)
		{
			rumbleCtx->strengthList_easing[i] = 0;
			rumbleCtx->strengthList[i] = 0;
			rumbleCtx->lengthList[i] = 0;
			rumbleCtx->decayList[i] = 0;
		}

		rumbleCtx->timer1 = rumbleCtx->timer2 = rumbleCtx->strength = rumbleCtx->length = rumbleCtx->decay = rumbleCtx->strength_easing = 0;
	}
}

// distance to player, strength, time, decay?
void Rumble_Shake(f32 playerDistance, u8 baseStrength, u8 length, u8 decay)
{
	s32 distance_decay; // By how much should the rumble effect be lowered, based on the distance
	s32 strength;
	s32 i;

	if(1000000.0f < playerDistance)
	{
		distance_decay = 1000;
	}
	else
	{
		distance_decay = sqrtf(playerDistance);
	}

	if(distance_decay < 1000 && baseStrength != 0 && decay != 0)
	{
		strength = baseStrength - (distance_decay * 255) / 1000;

		oot::player(0).SendMotorEvent(length, strength, decay);

		for(i = 0; i < 0x40; i++)
		{
			if(g_Rumble.strengthList[i] == 0)
			{
				if(strength > 0)
				{
					g_Rumble.strengthList[i] = strength;
					g_Rumble.lengthList[i] = length;
					g_Rumble.decayList[i] = decay;
				}
				break;
			}
		}
	}
}

void Rumble_Reset(void)
{		   // called on GameState_Init
#ifdef N64_VERSION // Don't clear our callback function
	bzero(&g_Rumble, sizeof(UnkRumbleStruct));
#endif

	g_Rumble.state = 2;
	g_Rumble.reset = 1;

	gPadMgr.retraceCallback = Rumble_Init;
	gPadMgr.retraceCallbackValue = 0;

	if(1)
	{
	}
}

void Rumble_Destroy(void)
{ // called on GameState_Destroy
	PadMgr* padmgr = &gPadMgr;

	if((padmgr->retraceCallback == Rumble_Init) && (padmgr->retraceCallbackValue == 0))
	{
		padmgr->retraceCallback = NULL;
		padmgr->retraceCallbackValue = 0;
	}

#ifdef N64_VERSION
	bzero(&g_Rumble, sizeof(UnkRumbleStruct));
#endif
}

u32 Rumble_IsEnabled(void)
{
	return gPadMgr.pakType[0] == 1;
}

void Rumble_Stop(void)
{ // called on Environment_Init and game over
	g_Rumble.state = 2;
}

void Rumble_Clear(void)
{ // called per frame specific gSaveContext.gameMode
	g_Rumble.state = 0;
}

void Rumble_Enable(u32 a)
{
	g_Rumble.reset = !!a;
}