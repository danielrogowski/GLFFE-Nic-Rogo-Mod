#include <math.h>
#include <string.h>
#include <stdio.h>
#include "aniso.h"

void BlitClipWrapper (void *pData, int xpos,
		int ypos, int width, int height, int jump,
		void (*BlitFunc)(void *, int, int, int, int, int))
{
	int srcx, srcy, t;
	int scanw = width + jump;

	if (xpos >= 0) srcx = 0;
	else { srcx = -xpos; width += xpos; xpos = 0; }
	if (ypos >= 0) srcy = 0;
	else { srcy = -ypos; height += ypos; ypos = 0; }

	if ((t = xpos + width - 640) > 0) { width -= t; }
	if ((t = ypos + height - 400) > 0) { height -= t; }

	jump = scanw - width;
	pData = (void *)((char *)pData + srcy*scanw + srcx);

	BlitFunc (pData, xpos, ypos, width, height, jump);
}	




// used to set breakpoints in ASM code.
void DebugBreak()
{
	return;
}

// used to test strings in ASM code.
void StringBreak(char* c)
{
	if (strstr(c, "Cegreeth D") != 0)
	{
		c = 0;
	}

	return;
}

void MakeCargoString(INT8 *cargoObj)
{
	char *nameStr, newStr[40];
	char *inStr, oldStr[40];

	nameStr = cargoObj+0x124;

	inStr = strchr(nameStr, '(');

	if (inStr)
	{
		inStr[0] = 0;	// null-terminate

		strcpy(oldStr, nameStr); // get the beginning

		sprintf(newStr, "%s(%d)", oldStr, INT16_AT(cargoObj+0x116));
	}
	else
		sprintf(newStr, "%s (%d)", nameStr, INT16_AT(cargoObj+0x116));

	newStr[19] = 0;
	strncpy(nameStr, newStr, 20);

}

// determines whether a bribe is successful.
INT32 CanBribe(INT32 maxFineAmt, INT32 fineAmt, INT32 policeIntegrity)
{
	float bribeFactor, allowedFactor;

	bribeFactor = fineAmt/(float)maxFineAmt;

	// can bribe up to maxFineAmt, depending
	allowedFactor = 1 - sin(policeIntegrity * (PI / 52000.0));
	allowedFactor *= 0.9 + 0.2*FloatRandom();

	if (bribeFactor < allowedFactor)
		return 1;

	return 0;
}

INT32 IsStarportLocked(INT8 *starport)
{
	INT8 i, *base, baseIdx, specialID;

	// find a photography mission for this system.
	for (i = 0; i < DATA_NumContracts; i++)
	{
		specialID = DATA_ContractArray[i*52+4] & 0xf;

		if (specialID >= 7)
			continue;
		if (specialID < 3)
			continue;
		if (INT32_AT(DATA_ContractArray+(i*52)+6) != DATA_CurrentSystem)
			continue;

		baseIdx = DATA_ContractArray[i*52+0x1a];
		base = DATA_GetObjectFunc(baseIdx, DATA_ObjectArray);

		// starports on the same planet as the base are locked down
		if (INT8_AT(base+0x56) == INT8_AT(starport+0x56))
			return 1;
	}

	return 0;
}

INT8 *IsCloseToStarport(INT32 offenseIdx)
{
	SINT8 i;
	INT8 *starportObj, starportIdx, dist, nearestDist, *nearestStarport;

	nearestStarport = 0;
	nearestDist = 255;

	for (i = (DATA_NumStarports-1); i >= 0; i--)
	{
		starportIdx = DATA_StarportArray[i].objectIdx;
		starportObj = DATA_GetObjectFunc(starportIdx, DATA_ObjectArray);

		dist = starportObj[0x88];

		if (dist <= nearestDist && !IsStarportLocked(starportObj))
		{
			nearestDist = dist;
			nearestStarport = starportObj;
		}
	}

	if (nearestDist < 0x13)
	{
		// unlawful discharge/piracy charge only once each per system
		if (offenseIdx == 3)
		{
			if (DATA_PlayerFlags & 0x4)
				return 0;
		}
/*		else if (offenseIdx == 10)
		{
			if (DATA_PlayerFlags & 0x200)
				return 0;
		}*/
		
		return nearestStarport;
	}
	// unlawful radioactive dumping can get you in trouble anyway
	if (offenseIdx == 0x8 && DATA_LastJettisonedCargoIndex == 0x1d && BoundRandom(10 + DATA_CurrentPirates*5) == 0)
		return nearestStarport;

	return 0;
}

INT32 ModifyEquipmentPrice(INT32 in)
{
	INT32 mult;

	mult = 9 + 9 * ((0xff - DATA_SystemTechLevel) / 256.0);
	
	return in*mult;
}

// returns 0 if any missiles are in flight
// also disallows acceleration with enemy ships nearby
INT32 ShouldAllowAcceleration(INT32 accel)
{
	SINT8 i;
	INT8 *obj;

	// OK to play or pause
	if (accel <= 0)
		return 1;

	for (i = 0x72; i >= 0; i--)
	{
		if (INT8_AT(DATA_ObjectArray+i) == 0)
			continue;

		obj = DATA_GetObjectFunc(i, DATA_ObjectArray);

		if (INT8_AT(obj+0xff) == 0x13 || (INT8_AT(obj+0x88) <= 14 && INT8_AT(obj+0xfe) == DATA_PlayerIndex && INT8_AT(obj+0xff) > 0 && INT8_AT(obj+0xff) <= 5))
			return 0;
	}

	return 1;
}

INT32 g_nearbySystems[NUM_SYSTEMS];
INT32 g_systemWeights[NUM_SYSTEMS];
INT32 g_systemWeightSum, g_opposingWeightSum, g_friendlyWeightSum;
SINT8 g_systemOpposing[NUM_SYSTEMS];

INT32 FindDestID(SINT16 sectX, SINT16 sectY, SINT16 sectDist)
{
	SINT16 destX, destY, numStars;
	INT8 randStarnum;
	float randAngle;

	if (sectDist <= 0)
	{
		destX = sectX;
		destY = sectY;
	}
	else
	{
		randAngle = FloatRandom()*2*PI;
		destX = sectX + sectDist*cos(randAngle);
		destY = sectY + sectDist*sin(randAngle);
	}

	numStars = (FUNC_000853_GetNumstars(destX, destY, 0) >> 10);

	if (numStars == 0)
		return 0;

	randStarnum = DATA_RandomizerFunc() % numStars;
	return ((destX & 0x1fff) | (((INT32)destY & 0x1fff) << 0xd) | (((INT32)randStarnum & 0x3f) << 0x1a));
}

// fill a global array that can be used	throughout a loop
void FillSystemData()
{
	INT32 population, tourism, allegiance, destID;
	SINT16 sectX, sectY, sectDist;
	SINT16 i, j;
	INT32 numSystems;
	INT8 opposingAllegiance;

	sectX = DATA_CurrentSystem & 0x1fff;
	sectY = (DATA_CurrentSystem >> 0xd) & 0x1fff;

	g_systemWeightSum = g_opposingWeightSum = g_friendlyWeightSum = 0;	
	numSystems = 0;
	
	memset(g_nearbySystems, 0, sizeof(g_nearbySystems));
	memset(g_systemOpposing, 0, sizeof(g_systemOpposing));
	memset(g_systemWeights, 0, sizeof(g_systemWeights));

	if (DATA_CurrentAllegiance == ALLY_FEDERAL)
		opposingAllegiance = ALLY_IMPERIAL;
	else if (DATA_CurrentAllegiance == ALLY_IMPERIAL)
		opposingAllegiance = ALLY_FEDERAL;
	else
		opposingAllegiance = 0;

	for (i = 0; i < NUM_SYSTEMS; i++)
	{
		sectDist = i/8 + BoundRandom(3) - 1;

		destID = FindDestID(sectX, sectY, sectDist);

		if (destID == DATA_CurrentSystem || destID == 0)
			continue;
		
		// duplicates have 0 weight
		for (j = numSystems-1; j >= 0; j--)
		{
			if (destID == g_nearbySystems[j])
				break;
		}

		if (j >= 0)
			continue;	// duplicate

		// special time-saving signal by setting p2 to 0 (hacky but useful)
		FUNC_000870_GetSystemDataExt(destID, 0, &population, &tourism, &allegiance);
		if (population < 3)
			continue;

		g_nearbySystems[numSystems] = destID;
		if (sectDist <= 0)
			sectDist = 4;
		else
			sectDist += 4;

		g_systemWeightSum += g_systemWeights[numSystems] = (16000*(population + tourism)) / (sectDist*sectDist);
		
		if (opposingAllegiance != 0 && allegiance == opposingAllegiance)
		{
			g_opposingWeightSum += g_systemWeights[numSystems];
			g_systemOpposing[numSystems] = 1;
		}
		else if (allegiance == DATA_CurrentAllegiance)
		{
			g_friendlyWeightSum += g_systemWeights[numSystems];
			g_systemOpposing[numSystems] = -1;
		}

		numSystems++;
	}

	if (DATA_CurrentAllegiance != ALLY_FEDERAL && DATA_CurrentAllegiance != ALLY_IMPERIAL)
		return;

	sectX = CoreX[opposingAllegiance];
	sectY = CoreY[opposingAllegiance];

	// for military systems, find more opposing systems starting from the enemy core
	for (i = 0; i < NUM_SYSTEMS && numSystems < NUM_SYSTEMS; i++)
	{
		sectDist = i/16 + BoundRandom(3);

		destID = FindDestID(sectX, sectY, sectDist);

		if (destID == DATA_CurrentSystem || destID == 0)
			continue;
		
		// duplicates have 0 weight
		for (j = numSystems-1; j >= 0; j--)
		{
			if (destID == g_nearbySystems[j])
				break;
		}

		if (j >= 0)
			continue;	// duplicate

		// special time-saving signal by setting p2 to 0 (hacky but useful)
		FUNC_000870_GetSystemDataExt(destID, 0, &population, &tourism, &allegiance);
		
		if (allegiance != opposingAllegiance)
			continue;
		if (population < 3)
			continue;

		g_nearbySystems[numSystems] = destID;
		if (sectDist <= 0)
			sectDist = 4;
		else
			sectDist += 4;

		g_opposingWeightSum += g_systemWeights[numSystems] = 500 / sectDist;
		g_systemOpposing[numSystems] = 2;

		numSystems++;
	}
		
}

// called on hyperjump for new system
void CreateSystemData(INT32 systemID)
{
	INT8 i, *stockFlagsPtr;
	INT32 d1, d2, techLevel, pirates, policeLevel, traders, d7, government;
	INT32 population, danger, c4;
	INT32 b1, b2, milActivity, b4;

	// get and store info on the new system.
	FUNC_000869_GetSystemData(DATA_CurrentSystem, &d1, &d2, &techLevel, &pirates, &policeLevel, &traders, &d7, &government);
	FUNC_000870_GetSystemDataExt(DATA_CurrentSystem, &stockFlagsPtr, &population, &danger, &c4);
	FUNC_000871_GetSystemDataExt2(DATA_CurrentSystem, &b1, &b2, &milActivity, &b4, &DATA_MilRankSub, &DATA_MilRankBase);

	for (i = 0; i < 38; i++)
		DATA_StockFlags[i] = stockFlagsPtr[i];

	DATA_CurrentPopulation = population;
	DATA_CurrentDanger = danger;
	DATA_SystemTechLevel = techLevel;
	DATA_CurrentPirates = pirates;
	DATA_PoliceLevel = policeLevel;
	DATA_CurrentTraders = traders;
	
	DATA_SystemMilitaryActivity = milActivity + population + 7;

	DATA_CurrentAllegiance = government >> 6;
	
	FillSystemData();

	for (i = 0; i < DATA_NumStarports; i++)
	{
		CreateBBSData(DATA_StarportArray+i);
		CreateMarketData(DATA_StarportArray+i);
		CreateShipyardData(DATA_StarportArray+i);
		FUNC_000034_Unknown(0x10, DATA_StarportArray[i].objectIdx);
		FUNC_000034_Unknown(0x10, 0);
	}

	if (DATA_CurrentAllegiance == ALLY_FEDERAL || DATA_CurrentAllegiance == ALLY_IMPERIAL)
		CreateMilitaryData();
}

// called on every new day
void RefreshSystemData()
{
	INT8 i;

	FillSystemData();

	for (i = 0; i < DATA_NumStarports; i++)
	{
		RefreshBBSData(DATA_StarportArray+i);
		RefreshMarketData(DATA_StarportArray+i);
		RefreshShipyardData(DATA_StarportArray+i);
		FUNC_000034_Unknown(0x10, DATA_StarportArray[i].objectIdx);
		FUNC_000034_Unknown(0x10, 0);
	}

	if (DATA_CurrentAllegiance == ALLY_FEDERAL || DATA_CurrentAllegiance == ALLY_IMPERIAL)
		RefreshMilitaryData();

	RefreshShips();
}

float HostileSnapAccum = 0.0;

#define MAX_HOSTILE_DELAY 2863311530	// approx. 16 hours in gametics

void SystemTick()
{
	INT32 hostileSub;

	if (DATA_PlayerState == 0x2a || DATA_PlayerState == 0x30)
		return;

	HostileSnapAccum += DATA_FrameTime*(65536.0/MAX_HOSTILE_DELAY);
	hostileSub = (INT32)HostileSnapAccum;

	if (hostileSub > DATA_HostileTimer)
	{
		DATA_HostileTimer = 0;
		HostileSnapAccum = 0.0;
	}
	else
	{
		DATA_HostileTimer -= hostileSub;
		HostileSnapAccum -= hostileSub;
	}
}

INT32 GetNearbySystem(SINT8 bGetOpposing)
{
	INT32 rnd, i, weight;

	rnd = ((INT32)DATA_RandomizerFunc() << 16) | DATA_RandomizerFunc();
	
	if (bGetOpposing == 1)
		rnd %= g_opposingWeightSum + 1;
	else if (bGetOpposing == -1)
		rnd %= g_friendlyWeightSum + 1;
	else
		rnd %= g_systemWeightSum + 1;

	weight = 0;
	for (i = 0; i < NUM_SYSTEMS && g_nearbySystems[i] != 0; i++)
	{
		if (g_systemOpposing[i] == 2)
		{
			if (bGetOpposing != 1)
				continue;
		}
		else if (bGetOpposing != 0 && bGetOpposing != g_systemOpposing[i])
			continue;

		weight += g_systemWeights[i];
		if (weight > rnd)
			return g_nearbySystems[i];
	}

	return 0;
}