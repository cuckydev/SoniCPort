//Clownacy's implementation

#include "PLC.h"

#include "Nemesis.h"

#include <string.h>

//Level art
const uint8_t art_ghz1[] = {
	#include <Resource/Art/GHZ1.h>
};
const uint8_t art_ghz2[] = {
	#include <Resource/Art/GHZ2.h>
};

//PLC lists
typedef struct
{
	size_t plcs;
	const PLC *plc;
} PLCList;

static const PLCList PLC_Main = {
	0,
	(const PLC[]){
		{NULL, 0}, //ISO C forbids empty initializer braces
		//plcm	Nem_Lamp, $F400		; lamppost
		//plcm	Nem_Hud, $D940		; HUD
		//plcm	Nem_Lives, $FA80	; lives	counter
		//plcm	Nem_Ring, $F640 	; rings
		//plcm	Nem_Points, $F2E0	; points from enemy
	}
};

static const PLCList PLC_Main2 = {
	0,
	(const PLC[]){
		{NULL, 0}, //ISO C forbids empty initializer braces
		//plcm	Nem_Monitors, $D000	; monitors
		//plcm	Nem_Shield, $A820	; shield
		//plcm	Nem_Stars, $AB80	; invincibility	stars
	}
};

static const PLCList PLC_Explode = {
	0,
	(const PLC[]){
		{NULL, 0}, //ISO C forbids empty initializer braces
		//plcm	Nem_Explode, $B400	; explosion
	}
};

static const PLCList PLC_GameOver = {
	0,
	(const PLC[]){
		{NULL, 0}, //ISO C forbids empty initializer braces
		//plcm	Nem_GameOver, $ABC0	; game/time over
	}
};

static const PLCList PLC_GHZ = {
	2,
	(const PLC[]){
		{art_ghz1, 0x0000},
		{art_ghz2, 0x39A0},
		//plcm	Nem_Stalk, $6B00	; flower stalk
		//plcm	Nem_PplRock, $7A00	; purple rock
		//plcm	Nem_Crabmeat, $8000	; crabmeat enemy
		//plcm	Nem_Buzz, $8880		; buzz bomber enemy
		//plcm	Nem_Chopper, $8F60	; chopper enemy
		//plcm	Nem_Newtron, $9360	; newtron enemy
		//plcm	Nem_Motobug, $9E00	; motobug enemy
		//plcm	Nem_Spikes, $A360	; spikes
		//plcm	Nem_HSpring, $A460	; horizontal spring
		//plcm	Nem_VSpring, $A660	; vertical spring
	}
};

static const PLCList PLC_GHZ2 = {
	0,
	(const PLC[]){
		{NULL, 0}, //ISO C forbids empty initializer braces
		//plcm	Nem_Swing, $7000	; swinging platform
		//plcm	Nem_Bridge, $71C0	; bridge
		//plcm	Nem_SpikePole, $7300	; spiked pole
		//plcm	Nem_Ball, $7540		; giant	ball
		//plcm	Nem_GhzWall1, $A1E0	; breakable wall
		//plcm	Nem_GhzWall2, $6980	; normal wall
	}
};

static const PLCList *plcs[PlcId_Num] = {
	/* PlcId_Main        */ &PLC_Main,
	/* PlcId_Main2       */ &PLC_Main2,
	/* PlcId_Explode     */ &PLC_Explode,
	/* PlcId_GameOver    */ &PLC_GameOver,
	/* PlcId_GHZ         */ &PLC_GHZ,
	/* PlcId_GHZ2        */ &PLC_GHZ2,
	/* PlcId_LZ          */ NULL,
	/* PlcId_LZ2         */ NULL,
	/* PlcId_MZ          */ NULL,
	/* PlcId_MZ2         */ NULL,
	/* PlcId_SLZ         */ NULL,
	/* PlcId_SLZ2        */ NULL,
	/* PlcId_SYZ         */ NULL,
	/* PlcId_SYZ2        */ NULL,
	/* PlcId_SBZ         */ NULL,
	/* PlcId_SBZ2        */ NULL,
	/* PlcId_TitleCard   */ NULL,
	/* PlcId_Boss        */ NULL,
	/* PlcId_Signpost    */ NULL,
	/* PlcId_Warp        */ NULL,
	/* PlcId_SpecialStage*/ NULL,
	/* PlcId_GHZAnimals  */ NULL,
	/* PlcId_LZAnimals   */ NULL,
	/* PlcId_MZAnimals   */ NULL,
	/* PlcId_SLZAnimals  */ NULL,
	/* PlcId_SYZAnimals  */ NULL,
	/* PlcId_SBZAnimals  */ NULL,
	/* PlcId_SSResult    */ NULL,
	/* PlcId_Ending      */ NULL,
	/* PlcId_TryAgain    */ NULL,
	/* PlcId_EggmanSBZ2  */ NULL,
	/* PlcId_FZBoss      */ NULL,
};

//PLC state
PLC plc_buffer[16];

static NemesisState plc_buffer_regs;
static uint16_t plc_buffer_reg18;
static uint16_t plc_buffer_reg1A;

//PLC interface
void AddPLC(PlcId plc)
{
	//Get PLC list to load
	const PLCList *list = plcs[plc];
	
	//Find empty PLC slot
	PLC *plc_free = plc_buffer;
	while (plc_free->art != NULL)
		plc_free++;
	
	//Push PLCs to buffer
	for (size_t i = 0; i < list->plcs; i++)
		plc_free[i] = list->plc[i];
}

void NewPLC(PlcId plc)
{
	//Get PLC list to load
	const PLCList *list = plcs[plc];
	
	//Clear previous PLCs
	ClearPLC();
	
	//Push PLCs to buffer
	for (size_t i = 0; i < list->plcs; i++)
		plc_buffer[i] = list->plc[i];
}

void ClearPLC()
{
	//Clear PLC buffer
	memset(plc_buffer, 0, sizeof(plc_buffer));
}

void RunPLC()
{
	if (plc_buffer[0].art != NULL && plc_buffer_reg18 == 0)
	{
		plc_buffer_regs.source = plc_buffer[0].art;
		plc_buffer_regs.vram_mode = true;
		plc_buffer_regs.dictionary = nemesis_buffer;
		
		uint16_t header = (plc_buffer_regs.source[0] << 8) | plc_buffer_regs.source[1];
		
		plc_buffer_regs.source += 2;
		plc_buffer_regs.xor_mode = header & 0x8000;
		plc_buffer_reg18 = header & 0x7FFF;
		
		NemDecPrepare(&plc_buffer_regs);
		
		plc_buffer_regs.d5 = (plc_buffer_regs.source[0] << 8) | plc_buffer_regs.source[1];
		plc_buffer_regs.source += 2;
		
		plc_buffer_regs.d0 = 0;
		plc_buffer_regs.d1 = 0;
		plc_buffer_regs.d2 = 0;
		plc_buffer_regs.d6 = 0x10;
	}
}

static void ProcessDPLC_Main(size_t off)
{
	NemDecSeek(off);
	
	do
	{
		plc_buffer_regs.remaining = 8;
		
		//Inlined NemDec_WriteIter
		plc_buffer_regs.d3 = 8;
		plc_buffer_regs.d4 = 0;
		
		NemDecRun(&plc_buffer_regs);
		
		if (--plc_buffer_reg18 == 0)
		{
			//Pop one request off the buffer so that the next one can be filled
			for (size_t i = 0; i < sizeof(plc_buffer) / sizeof(*plc_buffer) - 1; i++)
				plc_buffer[i] = plc_buffer[i + 1];
			return;
		}
	} while (--plc_buffer_reg1A != 0);
}

void RunPLC_VBlank()
{
	if (plc_buffer_reg18 != 0)
	{
		plc_buffer_reg1A = 9; //Process 9 tiles
		
		size_t off = plc_buffer[0].off;
		plc_buffer[0].off += 9 * 0x20;
		
		ProcessDPLC_Main(off);
	}
}

void ProcessDPLC2()
{
	if (plc_buffer_reg18 != 0)
	{
		plc_buffer_reg1A = 3; //Process 3 tiles
		
		size_t off = plc_buffer[0].off;
		plc_buffer[0].off += 3 * 0x20;
		
		ProcessDPLC_Main(off);
	}
}

void QuickPLC(PlcId plc)
{
	//Get PLC list to load and decompress immediately
	const PLCList *list = plcs[plc];
	for (size_t i = 0; i < list->plcs; i++)
		NemDec(list->plc[i].off, list->plc[i].art);
}
