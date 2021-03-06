/* Common definitions used in new source */

#pragma warning ( disable: 4244 4133 4305 4761 )

// Nick
#ifdef ALREADY_HAVE_TYPEDEFS
#define INT8 unsigned char
#define INT16 unsigned short
#define INT32 unsigned long
#define INT64 unsigned __int64
#endif

#ifndef ALREADY_HAVE_TYPEDEFS

typedef unsigned char INT8;
typedef unsigned short INT16;
typedef unsigned long INT32;

#endif

typedef signed char SINT8;
typedef signed short SINT16;
typedef signed long SINT32;

#ifndef __GNUC__
#ifndef ALREADY_HAVE_TYPEDEFS		// Nick
typedef unsigned __int64 INT64;
#endif
typedef signed __int64 SINT64;
#else
typedef unsigned long long INT64;
typedef signed long long SINT64;
#endif

typedef enum
{
	RATING_HARMLESS = 0,
	RATING_MOSTLYHARMLESS,
	RATING_POOR,
	RATING_BELOWAVG,
	RATING_AVERAGE,
	RATING_ABOVEAVG,
	RATING_COMPETENT,
	RATING_DANGEROUS,
	RATING_DEADLY,
	RATING_ELITE,
} eliteranking_t;


typedef struct
{
	INT32 price;
	INT32 pricevar;
	INT32 stock;
	INT32 stockvar;
} stockitem_t;

typedef struct
{
	INT32 id;
	INT32 weight;
} laserdata_t;

typedef struct
{
	INT32 string;
	INT32 data1;
	INT32 data2;
	INT32 data3;
	INT32 data4;
	INT32 data5; // ?
} militaryMission_t;

typedef struct
{
	INT32 string;
	INT32 data1;
	INT32 data2;
	INT32 data3;
	INT32 data4;
	INT32 data5;
} bbsAdvert_t;

typedef struct
{
	SINT32 avail;
	SINT32 price;
} stockinfo_t;

typedef struct
{
	INT32 randseed1;	// from [D8956] - position dependent
	INT32 randseed2;	// from [D8957] - also total mass?
	INT32 numstars;	// from b[D8954] & 7
	INT32 systemcode;	// from [D8950]
	INT32 techLevel;	// from [D8962]
	char pName[20];	// from D8656
	INT8 junk[16];
} sysGenParams_t;

typedef enum 
{
	ST_INDUSTRY = 0,
	ST_GREEN,
	ST_JUNGLE,
	ST_ICE,
	ST_DESERT,
	ST_MAX
} worldType_t;

typedef enum 
{
	GOV_NONE = 0,
	GOV_ANARCHY,
	GOV_FUEDAL,
	GOV_LOCALCORP,
	GOV_CORPSTATE,
	GOV_FEDERAL_DEMOCRACY,
	GOV_FEDERAL_COLONY,
	GOV_DICTATORSHIP,
	GOV_COMMUNIST,
	GOV_RELIGIOUS,
	GOV_DEMOCRACY,
	GOV_IMPERIAL_RULE,
	GOV_IMPERIAL_COLONY,
	GOV_NONE_STABLE,
	GOV_MARTIAL_LAW,
	GOV_ALLIANCE_DEMOCRACY,
} government_t;

typedef enum 
{
	EF_MINING = 0,
	EF_INDUSTRY,
	EF_WET,
	EF_AGRICULTURE,
	EF_TOURISM,
	EF_LIFE,
	EF_CRIME,
	EF_MAX,
} effectType_t;

typedef struct
{
	SINT32 effects[EF_MAX];
	INT16 distFromCenter;
	worldType_t primaryWorld;	
	INT32 development;
} sysType_t;

#define NUM_SYSTEMS 64

extern INT32 g_nearbySystems[NUM_SYSTEMS];
extern INT32 g_systemWeights[NUM_SYSTEMS];
extern INT8 g_systemAllegiances[NUM_SYSTEMS];

// Thanks to John Jordan for this one...
#ifndef __GNUC__
#pragma pack(push)
#pragma pack(1)
typedef struct equipdata_s
{
	INT32 buyCost;
	INT32 sellCost;
	INT32 weight;
	INT32 three;
	INT8 type;
	INT8 eighty;
	INT8 id;
	INT8 techLevel;
	INT8 something;
	INT8 zero;
} equipdata_t;

typedef struct
{
	stockinfo_t marketData[38];
	INT8 objectIdx;
	INT8 flags;
	bbsAdvert_t adverts[18];
	INT32 numAdverts;
	INT32 shipyard[14];
	INT32 numShips;
} starport_t;

typedef struct
{
	INT32 cash;
	INT8  passengers;
	INT8  mission_idx;
	INT16 ships;
	INT32 system;
	INT32 date;
	INT32 name;
} packageEntry_t;

typedef struct
{
	INT32 mass;			// in earth masses
	INT32 randseed;
	INT16 parentindex;
	INT16 objType;		// 0x23 for starports, < 0x20 for others...
	INT16 modelindex;
	INT32 orbradius;		// something orbital-distance related
	INT16 latitude;		// angle related
	INT32 orbvel;			// units unknown
	INT32 tempptr;		// points to PhysObj
	INT32 lightoutput;	// not used in PhysObj
	INT16 unknown;
	INT16 longitude;		// angle related
	INT16 temperature;		// in kelvin
	char pName[20];
	INT16 unknown2;
	INT16 unknown3;
	INT8 rotspeed;
	INT8 treelevel;		// copied to 0xcb of PhysObj
	INT16 unknown4;
	INT16 orientation;		// set to zero though.
} systemObject_t;

#pragma pack(pop)
#else
typedef struct equipdata_s
{
	INT32 buyCost __attribute__ ((packed));
	INT32 sellCost __attribute__ ((packed));
	INT32 weight __attribute__ ((packed));
	INT32 three __attribute__ ((packed));
	INT8 type __attribute__ ((packed));
	INT8 eighty __attribute__ ((packed));
	INT8 id __attribute__ ((packed));
	INT8 techLevel __attribute__ ((packed));
	INT8 something __attribute__ ((packed));
	INT8 zero __attribute__ ((packed));
} equipdata_t;

typedef struct
{
	stockinfo_t marketData[38] __attribute__ ((packed));
	INT8 objectIdx __attribute__ ((packed));
	INT8 flags __attribute__ ((packed));
	bbsAdvert_t adverts[18] __attribute__ ((packed));
	INT32 numAdverts __attribute__ ((packed));
	INT32 shipyard[14] __attribute__ ((packed));
	INT32 numShips;
} starport_t;

typedef struct
{
	INT32 cash __attribute__ ((packed));
	INT8  passengers __attribute__ ((packed));
	INT8  mission_idx __attribute__ ((packed));
	INT16 ships __attribute__ ((packed));
	INT32 system __attribute__ ((packed));
	INT32 date __attribute__ ((packed));
	INT32 name __attribute__ ((packed));
} packageEntry_t;

typedef struct
{
	INT32 mass __attribute__ ((packed));			// in earth masses
	INT32 randseed __attribute__ ((packed));
	INT16 parentindex __attribute__ ((packed));
	INT16 objType __attribute__ ((packed));		// 0x23 for starports, < 0x20 for others...
	INT16 modelindex __attribute__ ((packed));
	INT32 orbradius __attribute__ ((packed));		// something orbital-distance related
	INT16 latitude __attribute__ ((packed));		// angle related
	INT32 orbvel __attribute__ ((packed));			// units unknown
	INT32 tempptr __attribute__ ((packed));		// points to PhysObj
	INT32 lightoutput __attribute__ ((packed));	// not used in PhysObj
	INT16 unknown __attribute__ ((packed));
	INT16 longitude __attribute__ ((packed));		// angle related
	INT16 temperature __attribute__ ((packed));		// in kelvin
	char pName[20] __attribute__ ((packed));
	INT16 unknown2 __attribute__ ((packed));
	INT16 unknown3 __attribute__ ((packed));
	INT8 rotspeed __attribute__ ((packed));
	INT8 treelevel __attribute__ ((packed));		// copied to 0xcb of PhysObj
	INT16 unknown4 __attribute__ ((packed));
	INT16 orientation __attribute__ ((packed));		// set to zero though.
} systemObject_t;

#endif

// maximum distance out in sectors (squared)
// we can find civilized systems up to 30 sector lengths out
#define MAX_SECT_DIST 30.0

#define PI 3.14159265359

// 1 kilometer in game units (approx)
#define KM_DIST  999539.7368756

#define ALLY_INDEPENDENT 0
#define ALLY_IMPERIAL 1
#define ALLY_FEDERAL 2
#define ALLY_ALLIANCE 3

// ugly pointer definitions.
#define VEC32_AT(p) (*((Vec32*)(p)))
#define INT64_AT(p) (*((INT64*)(p)))
#define SINT64_AT(p) (*((SINT64*)(p)))
#define INT32_AT(p) (*((INT32*)(p)))
#define SINT32_AT(p) (*((SINT32*)(p)))
#define INT16_AT(p) (*((INT16*)(p)))
#define SINT16_AT(p) (*((SINT16*)(p)))
#define INT8_AT(p) (*((INT8*)(p)))
#define SINT8_AT(p) (*((SINT8*)(p)))

#define VOIDPTR_AT(p) (*((void**)(p)))
#define INT16_PTR(p) ((INT16*)(p))

#define NUM_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

extern INT32 (*DATA_RandomizerFunc)();	// randomizer
extern INT8* DATA_PlayerObject;
extern stockitem_t DATA_StockData[0x20];
extern INT8 DATA_CargoFromShip[0x20];
extern laserdata_t DATA_AILasers[0x9];
extern INT32 DATA_DriveMasses[];
extern INT32 DATA_ECM_Weight;
extern INT32 DATA_NECM_Weight;
extern INT8 DATA_AIMissiles[0x10];
extern INT8 *DATA_ObjectArray;
extern INT8 DATA_FederalShips[16];
extern INT8 DATA_ImperialShips[16];
extern INT16 DATA_CurrentAllegiance;
extern INT8 DATA_PlayerIndex;
extern equipdata_t DATA_EquipmentData[64];
extern INT16 DATA_DriveFuelConsumption[];
extern INT32 DATA_FrameTime;
extern INT32 DATA_008886_Unknown;
extern INT16 DATA_PlayerCargo[32];
extern INT32 DATA_PlayerCargoSpace;
extern INT8  DATA_LastJettisonedCargoIndex;
extern INT8  DATA_ExtendedUniverse;
extern void* (*DATA_GetStaticDataFunc)(int modelidx);	// finds ship static data
extern INT32* (*DATA_HasEquipmentFunc)(int unknown, INT8 *equip);	// finds ship static data
extern INT32 (*DATA_FixedRandomFunc)(int num, int *seed1, int *seed2); 
extern void (*DATA_DrawStringWrapShadowFunc)(INT32 ffcode, void *vars, INT32 col, INT32 xpos, INT32 ypos, INT32 wrapWidth);

extern void FUNC_001907_SoundPlaySampleLogVol(INT32 soundidx, INT32 logVol);
extern void FUNC_001908_SoundPlaySampleLinVol(INT32 soundidx, INT32 volume);		

extern INT32 FUNC_000952_DestroyEquip(INT8 *ship);
extern INT32 FUNC_000953_TakeDamage(INT8 *victim, INT32 damage, INT8 inflictorIdx, INT32 bContinuous);

extern void FUNC_000148_Unknown(INT32, INT32);
extern void FUNC_000048_Unknown(INT32, INT32, INT32);
extern void FUNC_000034_Unknown(INT32, INT32);
extern INT32 FUNC_000035_GetSpecialShips(INT32);

extern INT8* (*DATA_GetObjectFunc)(int objidx, INT8 *objectArray);	// finds object data

// this function is unreliable, replace
//extern INT16 (*DATA_BoundRandom)(INT16 max);	// returns a number between 0 and max
#define BoundRandom(m) (m == 0 ? 0 : (DATA_RandomizerFunc() % m))

extern INT16 DATA_CurrentAllegiance;
extern INT8 *DATA_CustomShipList;
extern INT32 DATA_CustomShipIndex;
extern starport_t DATA_StarportArray[18]; // 18 starport slots
extern INT32 DATA_NumStarports;
extern INT16 DATA_CurrentPopulation;
extern INT8 DATA_CurrentPirates;
extern INT32 DATA_GameTics;
extern INT32 DATA_GameDays;
extern INT32 DATA_PlayerSpaceAvail;
extern SINT32 DATA_NumContracts;
extern INT8  DATA_ContractArray[312];
extern INT32 DATA_CurrentSystem;
extern INT32 DATA_NumObjects;
extern INT64 DATA_GroupingVector[];
extern INT8  DATA_StockFlags[38];
extern INT32 DATA_RandSeed1;
extern INT32 DATA_RandSeed2;
extern INT8 DATA_SystemMilitaryActivity;
extern INT32 DATA_MilRankBase;
extern INT32 DATA_MilRankSub;
extern INT32 DATA_PoliceLevel;
extern INT8 DATA_CurrentDanger;
extern INT8 DATA_CurrentTraders;
extern INT32 DATA_StarportRand;
extern INT8 DATA_SystemTechLevel;
extern INT8 DATA_LastAttackedIndex;
extern INT32 DATA_NumPackages;
extern INT8 DATA_JettisonedCargoTypes[33];
extern INT8 DATA_PlasmaMount[];
extern INT16 DATA_PlayerFlags;
extern INT32 DATA_PlayerCash;
extern INT8 DATA_PlayerState;
extern INT32 DATA_FederalRank;
extern INT32 DATA_ImperialRank;
extern INT16 DATA_HostileTimer;

extern char DATA_009148_StringArray[612];

extern militaryMission_t DATA_MilitaryMissions[26];
extern packageEntry_t DATA_PackageArray[60];

// cargotype = 0x8e00 + type
// returns pointer to cargo physobj.
extern INT8* FUNC_000926_SpawnCargo(INT32 cargotype, INT8 *ship, INT32 cargoAmount);
extern void FUNC_000924_DestroyObject(INT8 *object, INT8 inflictorIdx);

// gets amount of specified equipment type owned by ship
extern INT32 FUNC_000392_GetEquipmentAmount(char *ship, INT32 equipIdx);
extern void FUNC_000349_ShowCommMessage(INT8 *msgData);
extern void FUNC_000304_AddFederalRank(INT32 addition, INT8 *data, INT32 medal);
extern void FUNC_000305_AddImperialRank(INT32 addition, INT8 *data, INT32 medal);

// gets data on a specified system (danger, pop, etc)
extern void FUNC_000869_GetSystemData(INT32 id, INT32 *d1, INT32 *d2, INT32 *techLevel, INT32 *pirates, INT32 *policeLevel, INT32 *traders, INT32 *d7, INT32 *government);
extern void FUNC_000869_NoOverride(INT32 id, INT32 *d1, INT32 *d2, INT32 *techLevel, INT32 *pirates, INT32 *policeLevel, INT32 *traders, INT32 *d7, INT32 *government);

extern void FUNC_000870_GetSystemDataExt(INT32 id, INT8 **stockFlags, INT32 *population, INT32 *danger, INT32 *d4);
extern void FUNC_000871_GetSystemDataExt2(INT32 id, INT32 *d1, INT32 *d2, INT32 *milActivity, INT32 *d4, INT32 *milRankSub, INT32 *milRankBase);

extern INT32 FUNC_000853_GetNumstars(SINT16 sectX, SINT16 sectY, INT32 p3);

extern INT32 FUNC_000857_GetNeighborSystem(INT32 centerSystem, INT32 unknown);
extern void FUNC_000878_GetSysObjectData(systemObject_t *p1, sysGenParams_t *p2, INT32 useAllegiance);
extern void FUNC_GetSysGenParams(sysGenParams_t *genParams, INT32 systemcode);

// gets a system of the opposing military power.
// returns pop. level in eax, return value of F857 in ret
extern INT32 FUNC_000625_FindOppositionSystem(INT32 *ret);
extern INT8 *FUNC_000772_AIShipSpawn(INT32 type);
extern void FUNC_000697_GetBounty(INT8 *ship, INT32 bountyLevel);
extern void FUNC_000702_Unknown(INT8 *ship, INT32 v);
extern void FUNC_000634_AddToShipyard(INT32 type, starport_t *starport, INT32 **shipyardPtr);
extern void FUNC_000688_SpawnTraders();

extern void FUNC_000689_SpawnHSTrader();
extern void FUNC_000690_SpawnHSTrader2();
extern void FUNC_000691_SpawnDockedTrader();
extern void FUNC_000699_SpawnAuxTrader();

extern void FUNC_001661_Vec64Add(INT64 *v1, INT64 *v2);
extern void FUNC_001662_Vec64Sub(INT64 *v1, INT64 *v2);
extern void FUNC_001344_StringExpandFFCode(char *dest, int ffcode, INT8 *vars);
extern INT32 GetStarRange(INT32 systemcode);

extern INT16 CoreX[];
extern INT16 CoreY[];

#define FloatRandom() (DATA_RandomizerFunc() / 65536.0)

INT8 SpawnHostileGroup(INT8 ships, INT8 *shipArray, INT32 targetName, INT8 shipType, INT8 shipIDByte);
int _finite(double);

void CreateMarketData(starport_t *starport);
void CreateBBSData(starport_t *starport);
void CreateMilitaryData();
void CreateShipyardData(starport_t *starport);

void RefreshMilitaryData();
void RefreshBBSData(starport_t *starport);
void RefreshMarketData(starport_t *starport);
void RefreshShipyardData(starport_t *starport);

INT32 GetNearbySystem(SINT8 bGetOpposing);

void DeleteBBSAdvert(INT32 idx, starport_t *starport);
INT32 GetEmptyAdSlot(starport_t *starport);

INT32 BBSCreateRandom(starport_t *starport, bbsAdvert_t *slot);

void CreateBBSAdvert(starport_t *starport, bbsAdvert_t *slot, INT32 string, INT32 d1, INT32 d2, INT32 d3, INT32 d4, INT32 d5);
void CreateMilitaryEntry(INT32 string, INT32 data1, INT32 data2, INT32 data3, INT32 data4, INT32 data5);

void SpawnPirates(INT32 pirateLevel, INT32 bInitial);
void SpawnAssassins(INT32 ships, INT32 missionIdx, INT32 name);

void CreateShips();
void RefreshShips();
INT32 GetInitialFuel(INT8 *ship, INT8 driveType);
INT32 GetNumFreeSlots();
INT32 IsStarportLocked(INT8 *starport);
void GetStarportSupply(starport_t *starport, INT8 *supply);
