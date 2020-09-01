#define ALREADY_HAVE_TYPEDEFS

#include <windows.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

extern "C" 
{
	#include "ffecfg.h"
	#include "../aniso/aniso.h"
}

#define __SHIPDATAFILE__	"shipdata.txt" 
#define __HACKSFILE__		"hacks.txt"

char *ShipNames[] =
{
	"Missle1", "Missle2", "Missile3", "Missile4",
	// Missiles 0x4-0xd
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
	// Escape Capsule and other worthless ships
	"11", "12", "13", "14", "15",
	"Osprey Interceptor",
	"Falcon Interceptor",
	"Hawk Airfighter",
	"Kestrel Airfighter",
	"Eagle long range fighter",
	"Eagle MKII",
	"Eagle MKIII",
	"Sidewinder",
	"Krait",
	"Gecko",
	"Adder",
	"Viper Defence Craft",
	"Saker MKIII",
	"Osprey 'X' Interceptor",
	"Merlin Interceptor",
	"Viper MKII",
	"Gyr Attack Fighter",
	"Cobra MKI",
	"Moray Starboat",
	"Cobra MKIII",
	"Constrictor",
	"Asp Explorer",
	"Lanner",
	"Harris Fighter",
	"Spar Interceptor",
	"Wyvern Explorer",
	"Skeet Cruiser",
	"Turner Class",
	"Lanner II",
	"Harrier",
	"U1", "U2", "U3", "U4",		// Unknowns
	"Transporter",
	"Lion Transport",
	"Tiger Trader",
	"Imperial Courier",
	"Python Trader",
	"Imperial Trader",
	"Anaconda Destroyer",
	"Puma Freighter",
	"Boa Trader",
	"Panther Clipper",
	"U5",
	"Tiercel Freighter",
	"Imperial Explorer",
	"INRA Command Ship",
	"Mantis Freighter",
	"Griffin Hauler",
	"Thargoid Transport",
	"Lynx Bulk Carrier",
	"Long-range Cruiser"
};

int GetShipIdx(const char *szName)
{
	for (int i = 0; i != (sizeof(ShipNames)/sizeof(char*)); i++ )
	{
		if ( stricmp(szName, ShipNames[i]) == 0 )
		{
			return i;
		}
	}
	return -1;
}

void SetINT16(CfgStruct *cfg, INT8 *pTemp, int pos, char *szName)
{
	int Temp;
	if ( CfgGetKeyVal (cfg, szName, &Temp) )
	{
		INT16_AT (pTemp+pos) = Temp;
	}
}


void SetINT32(CfgStruct *cfg, INT8 *pTemp, int pos, char *szName)
{
	int Temp;
	if ( CfgGetKeyVal (cfg, szName, &Temp) )
	{
		INT32_AT (pTemp+pos) = Temp;
	}
}

extern "C" DWORD Beta_Lyrae;
extern "C" BYTE Equipment;

extern "C" BYTE PlanetDetailHigh = 0;
extern "C" BYTE CanHaveExcessCrew = 0;
extern "C" BYTE NoMassMissiles = 0;
extern "C" BYTE DoubleFastMissiles = 0;
extern "C" BYTE NoTurretMovementWhilePaused = 0;
extern "C" BYTE CorrectShipWorth = 0;

/*
struct ShipSpecs {
	word fthrust;
	word rthrust;
	word gunmount;
	word mass;
	word intcap;		; 0x8
	word price;
	word zoomf;
	word ID;
	word crew;			; 0x10
	word missiles;
	word drive;			; top byte potentially upgrade-drive
	word elitepoints;
	word view_zoff;		; 0x18
	word view_yoff;
	word unknown;
	word unknown;
	word fgun_zoff;		; 0x20
	word fgun_yoff;
	word rgun_zoff;
	word rgun_yoff;
	word ttgun_zoff;	; 0x24		// DWORDs ?
	word ttgun_yoff;
	word btgun_zoff;
	word btgun_yoff;
};
*/
void DumpShipData()
{
	INT8 *pTemp;
	FILE *fout = fopen("realdata.txt", "wt");

	if ( fout )
	{
		for (int i = 1; i != (sizeof(ShipNames)/sizeof(char*)); i++ )
		{
			pTemp = (INT8*)DATA_GetStaticDataFunc(i);
			pTemp = (INT8*)VOIDPTR_AT (pTemp+0x38);
			if ( pTemp )
			{
				fprintf(fout, "[%s]\n", ShipNames[i]);
				fprintf(fout, "ForwardThrust = %d\n", INT16_AT(pTemp+0));
				fprintf(fout, "ReverseThrust = %d\n", INT16_AT(pTemp+2));
				fprintf(fout, "Cost = %d\n", INT16_AT(pTemp+0xa)*1000);
				fprintf(fout, "HullMass = %d\n", INT16_AT(pTemp+6));
				fprintf(fout, "SpaceAvailable = %d\n", INT16_AT(pTemp+8));
				fprintf(fout, "Shields = %d\n", INT16_AT(pTemp+0xe0));
				fprintf(fout, "MaxShields = %d\n", INT16_AT(pTemp+0xe2));
				fprintf(fout, "Crew = %d\n", INT16_AT(pTemp+0x10));
				fprintf(fout, "MissilePylons = %d\n", INT16_AT(pTemp+0x12));
				fprintf(fout, "GunMountings = %d\n", INT16_AT(pTemp+0x4));

				fprintf(fout, "HyperDrive = %d\n", INT16_AT(pTemp+0x14));
				
				// DWORDs ?
				fprintf(fout, "TopTurretZ = %d\n", INT16_AT(pTemp+0x24+0));
				fprintf(fout, "TopTurretY = %d\n", INT16_AT(pTemp+0x24+2));

				fprintf(fout, "BottomTurretZ = %d\n", INT16_AT(pTemp+0x24+4));
				fprintf(fout, "BottomTurretY = %d\n", INT16_AT(pTemp+0x24+6));

				fprintf(fout, "TopTurretGunPoint = %d\n", INT16_AT(pTemp+0x24+8));
				fprintf(fout, "BottomTurretGunPoint = %d\n", INT16_AT(pTemp+0x24+10));

				fprintf(fout, "\n");
			}
		}
		fclose(fout);
	}
}

/*
; Nick
push ecx
push eax
call _NickDebug
pop eax
pop ecx
*/

extern "C" void NickDebug()
{
}

// see aniso/market.c
extern "C" {
void change_market_prices();
}

extern "C" void NickMain()
{
	INT8 *pTemp;
	int idx = 0;

//	DumpShipData();

	change_market_prices();

	// Load Ship Data
	CfgStruct cfg;

	// Load Hacks file
	if ( CfgOpen (&cfg, __HACKSFILE__) )
	{
		if ( CfgFindSection(&cfg, "HACKS") )
		{
			int Temp = 0;

			// BetaLyraeFix
			if ( CfgGetKeyVal (&cfg, "BetaLyraeFix", &Temp) && Temp == 1 )
				*(&Beta_Lyrae+22) = 0;

			// Priority
			if ( CfgGetKeyVal (&cfg, "ProcessPriority", &Temp) )
			{
				switch ( Temp )
				{
				case 0:
					SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
					break;
				case 1:
					SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
					break;
				case 2:
					SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
					break;
				}
			}

			// PlanetDetailHack
			if ( CfgGetKeyVal (&cfg, "PlanetDetailHack", &Temp) && Temp == 1 )
				PlanetDetailHigh = 1;
			else
				PlanetDetailHigh = 0;

			// CanHaveExcessCrew
			if ( CfgGetKeyVal (&cfg, "CanHaveExcessCrew", &Temp) && Temp == 1 )
				CanHaveExcessCrew = 1;
			else
				CanHaveExcessCrew = 0;

			// NoMassMissiles
			if ( CfgGetKeyVal (&cfg, "NoMassMissiles", &Temp) && Temp == 1 )
			{
				BYTE *pEquipment = &Equipment+3+8;

				// pEquipment[0] = 0;		// Proximity Mine
				pEquipment[22*1] = 0;
				pEquipment[22*2] = 0;
				pEquipment[22*3] = 0;
				pEquipment[22*4] = 0;
				pEquipment[22*5] = 0;
				pEquipment[22*6] = 0;
				pEquipment[22*7] = 0;
				pEquipment[22*8] = 0;

				NoMassMissiles = 1;
			}
			else
				NoMassMissiles = 0;

			// DoubleFastMissiles
			if ( CfgGetKeyVal (&cfg, "DoubleFastMissiles", &Temp) && Temp == 1 )
				DoubleFastMissiles = 1;
			else
				DoubleFastMissiles = 0;

			// NoTurretMovementWhilePaused
			if ( CfgGetKeyVal (&cfg, "NoTurretMovementWhilePaused", &Temp) && Temp == 1 )
				NoTurretMovementWhilePaused = 1;
			else
				NoTurretMovementWhilePaused = 0;


			// CorrectShipWorth
			if ( CfgGetKeyVal (&cfg, "CorrectShipWorth", &Temp) && Temp == 1 )
				CorrectShipWorth = 1;
			else
				CorrectShipWorth = 0;
		}

		// Close
		CfgClose(&cfg);
	}

	// Load Ships file
	if ( CfgOpen (&cfg, __SHIPDATAFILE__) )
	{
		for (int i = 0; i != (sizeof(ShipNames)/sizeof(char*)); i++ )
		{
			if ( CfgFindSection(&cfg, ShipNames[i]) )
			{
				int Temp;

				pTemp = (INT8*)DATA_GetStaticDataFunc(GetShipIdx(ShipNames[i]));
				pTemp = (INT8*)VOIDPTR_AT (pTemp+0x38);

				SetINT16(&cfg, pTemp, 0, "ForwardThrust");
				SetINT16(&cfg, pTemp, 2, "ReverseThrust");

				// Cost (Special as /1000)
				if ( CfgGetKeyVal (&cfg, "Cost", &Temp) )
					INT16_AT(pTemp+0xa) = Temp/1000;
				
				SetINT16(&cfg, pTemp, 6, "HullMass");
				SetINT16(&cfg, pTemp, 8, "SpaceAvailable");

				SetINT16(&cfg, pTemp, 0xe0, "Shields");
				SetINT16(&cfg, pTemp, 0xe2, "MaxShields");
			
				SetINT16(&cfg, pTemp, 0x10, "Crew");

				SetINT16(&cfg, pTemp, 0x12, "MissilePylons");
				SetINT16(&cfg, pTemp, 0x4,  "GunMountings");
				
				SetINT16(&cfg, pTemp, 0x14, "HyperDrive");
				
				if ( stricmp(ShipNames[i], "Griffin Hauler")==0 )
				{
					__asm nop;
				}

				// DWORDs ?
				SetINT16(&cfg, pTemp, 0x24+0, "TopTurretZ");
				SetINT16(&cfg, pTemp, 0x24+2, "TopTurretY");
				
				SetINT16(&cfg, pTemp, 0x24+4, "BottomTurretZ");
				SetINT16(&cfg, pTemp, 0x24+6, "BottomTurretY");

				SetINT16(&cfg, pTemp, 0x24+8, "TopTurretGunPoint");
				SetINT16(&cfg, pTemp, 0x24+10, "BottomTurretGunPoint");
			}
		}

		// Close
		CfgClose(&cfg);
	}

}


// Nick - Misc Functions - ANISO
// returns the exchange rate of a ship (unit: $1000cr).
extern INT32 FUNC_000392_GetEquipmentAmount(char *ship, INT32 equipIdx);
extern "C" INT32 Nic_GetShipWorth(INT8 *ship)
{
	SINT32 shipCost, i, equipAmt;
	INT8 driveType;
	INT16 hullMass, intactMass;
	
	INT8 *pTemp;

	pTemp = (INT8*)DATA_GetStaticDataFunc( INT32_AT(ship+0x82) );
	pTemp = (INT8*)VOIDPTR_AT (pTemp+0x38);
	shipCost = INT16_AT(pTemp+0xa) * 900;	// used ship

	driveType = INT8_AT(pTemp+0x14);
	
	for (i = 0; i < 64; i++)
	{
		if (DATA_EquipmentData[i].id == driveType)
			shipCost -= DATA_EquipmentData[i].buyCost * 9;
	}

	for (i = 0; i < 64; i++)
	{
		equipAmt = FUNC_000392_GetEquipmentAmount((char*)ship, i);

		// get nothing for integrated drives, or unsellable items
		if (DATA_EquipmentData[i].techLevel < 0xff)
			shipCost += equipAmt * DATA_EquipmentData[i].sellCost * 9;
	}
	
// subtract any hull repair costs

	hullMass = INT16_AT(pTemp+0x6) * 4;
	intactMass = INT16_AT(ship+0xe4);

	shipCost -= (hullMass - intactMass) * 5;

//	shipCost /= 1000;
//	shipCost *= 1000;

	return shipCost;
}

// following code only compilable with the glffe project, not the glffe_aniso project
#ifdef _PROJECT_GLFFE_

extern "C" void FUNC_001418_TimerAdvance();
extern "C" void FUNC_001637_FlipScreen();
extern "C" char FUNC_001414_GuiGetLastAction();

/* 
 ** FUNC_000172_CdrFileConfirmYN() - RMG
 ** 
 */
extern "C" int CdrFileConfirmYN() 
{ 
        char c; 
        do 
		{ 
                FUNC_001418_TimerAdvance(); 
                FUNC_001637_FlipScreen(); 
                c = FUNC_001414_GuiGetLastAction(); 
        } while (c == '\0'); 
        
		return ((c == 'Y') || (c == 'y')) ? 1 : 0; 
} 

#endif // _PROJECT_GLFFE_
