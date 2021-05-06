//Clownacy's implementation

#include "PLC.h"

#include "Nemesis.h"

#include "Backend/VDP.h"

#include <string.h>

//PLC constants
#define PLC_SPEED_1 9 //How many tiles are loaded per frame during a 'loading' state
#define PLC_SPEED_2 3 //How many tiles are loaded per frame while the game's running

//Level art
const uint8_t art_ghz1[] = {
	#include "Resource/Art/GHZ1.h"
	,0,
};
const uint8_t art_ghz2[] = {
	#include "Resource/Art/GHZ2.h"
	,0,
};
const uint8_t art_lz[] = {
	#include "Resource/Art/LZ.h"
	,0,
};
const uint8_t art_mz[] = {
	#include "Resource/Art/MZ.h"
	,0,
};
const uint8_t art_slz[] = {
	#include "Resource/Art/SLZ.h"
	,0,
};
const uint8_t art_syz[] = {
	#include "Resource/Art/SYZ.h"
	,0,
};
const uint8_t art_sbz[] = {
	#include "Resource/Art/SBZ.h"
	,0,
};

//Object art
static const uint8_t art_lamppost[] = {
	#include "Resource/Art/Lamppost.h"
	,0,
};
static const uint8_t art_hud[] = {
	#include "Resource/Art/HUD.h"
	,0,
};
static const uint8_t art_hud_life[] = {
	#include "Resource/Art/HUDLife.h"
	,0,
};
static const uint8_t art_ring[] = {
	#include "Resource/Art/Ring.h"
	,0,
};
static const uint8_t art_points[] = {
	#include "Resource/Art/Points.h"
	,0,
};

static const uint8_t art_monitor[] = {
	#include "Resource/Art/Monitor.h"
	,0,
};
static const uint8_t art_shield[] = {
	#include "Resource/Art/Shield.h"
	,0,
};
static const uint8_t art_invincibility[] = {
	#include "Resource/Art/Invincibility.h"
	,0,
};

static const uint8_t art_explosion[] = {
	#include "Resource/Art/Explosion.h"
	,0,
};
static const uint8_t art_game_over[] = {
	#include "Resource/Art/GameOver.h"
	,0,
};
static const uint8_t art_ghz_stalk[] = {
	#include "Resource/Art/GHZStalk.h"
	,0,
};
static const uint8_t art_ghz_rock[] = {
	#include "Resource/Art/GHZRock.h"
	,0,
};
static const uint8_t art_crabmeat[] = {
	#include "Resource/Art/Crabmeat.h"
	,0,
};
static const uint8_t art_buzz_bomber[] = {
	#include "Resource/Art/BuzzBomber.h"
	,0,
};
static const uint8_t art_chopper[] = {
	#include "Resource/Art/Chopper.h"
	,0,
};
static const uint8_t art_newtron[] = {
	#include "Resource/Art/Newtron.h"
	,0,
};
static const uint8_t art_motobug[] = {
	#include "Resource/Art/Motobug.h"
	,0,
};
static const uint8_t art_spikes[] = {
	#include "Resource/Art/Spikes.h"
	,0,
};
static const uint8_t art_springh[] = {
	#include "Resource/Art/SpringH.h"
	,0,
};
static const uint8_t art_springv[] = {
	#include "Resource/Art/SpringV.h"
	,0,
};
static const uint8_t art_ghz_swing[] = {
	#include "Resource/Art/GHZSwing.h"
	,0,
};
static const uint8_t art_ghz_bridge[] = {
	#include "Resource/Art/GHZBridge.h"
	,0,
};
static const uint8_t art_ghz_log[] = {
	#include "Resource/Art/GHZLog.h"
	,0,
};
static const uint8_t art_ghz_ball[] = {
	#include "Resource/Art/GHZBall.h"
	,0,
};
static const uint8_t art_ghz_wall1[] = {
	#include "Resource/Art/GHZWall1.h"
	,0,
};
static const uint8_t art_ghz_wall2[] = {
	#include "Resource/Art/GHZWall2.h"
	,0,
};

static const uint8_t art_signpost[] = {
	#include "Resource/Art/Signpost.h"
	,0,
};
static const uint8_t art_hidden_bonus[] = {
	#include "Resource/Art/HiddenBonus.h"
	,0,
};
static const uint8_t art_big_flash[] = {
	#include "Resource/Art/BigFlash.h"
	,0,
};

static const uint8_t art_ss_clouds[] = {
	#include "Resource/Art/SSClouds.h"
	,0,
};
static const uint8_t art_ss_back[] = {
	#include "Resource/Art/SSBack.h"
	,0,
};
static const uint8_t art_ss_wall[] = {
	#include "Resource/Art/SSWall.h"
	,0,
};
static const uint8_t art_ss_goal[] = {
	#include "Resource/Art/SSGoal.h"
	,0,
};
static const uint8_t art_ss_speed[] = {
	#include "Resource/Art/SSSpeed.h"
	,0,
};
static const uint8_t art_ss_rotate[] = {
	#include "Resource/Art/SSRotate.h"
	,0,
};
static const uint8_t art_ss_life[] = {
	#include "Resource/Art/SSLife.h"
	,0,
};
static const uint8_t art_ss_twinkle[] = {
	#include "Resource/Art/SSTwinkle.h"
	,0,
};
static const uint8_t art_ss_checker[] = {
	#include "Resource/Art/SSChecker.h"
	,0,
};
static const uint8_t art_ss_ghost[] = {
	#include "Resource/Art/SSGhost.h"
	,0,
};
static const uint8_t art_ss_warp[] = {
	#include "Resource/Art/SSWarp.h"
	,0,
};
static const uint8_t art_ss_glass[] = {
	#include "Resource/Art/SSGlass.h"
	,0,
};
static const uint8_t art_ss_emerald[] = {
	#include "Resource/Art/SSEmerald.h"
	,0,
};
static const uint8_t art_ss_zone1[] = {
	#include "Resource/Art/SSZone1.h"
	,0,
};
static const uint8_t art_ss_zone2[] = {
	#include "Resource/Art/SSZone2.h"
	,0,
};
static const uint8_t art_ss_zone3[] = {
	#include "Resource/Art/SSZone3.h"
	,0,
};
static const uint8_t art_ss_zone4[] = {
	#include "Resource/Art/SSZone4.h"
	,0,
};
static const uint8_t art_ss_zone5[] = {
	#include "Resource/Art/SSZone5.h"
	,0,
};
static const uint8_t art_ss_zone6[] = {
	#include "Resource/Art/SSZone6.h"
	,0,
};

//PLC lists
typedef struct
{
	size_t plcs;
	const PLC *plc;
} PLCList;

static const PLCList PLC_Main = {
	5,
	(const PLC[]){
		{art_lamppost, 0xF400},
		{art_hud,      0xD940},
		{art_hud_life, 0xFA80},
		{art_ring,     0xF640},
		{art_points,   0xF2E0},
	}
};

static const PLCList PLC_Main2 = {
	3,
	(const PLC[]){
		{art_monitor,       0xD000},
		{art_shield,        0xA820},
		{art_invincibility, 0xAB80},
	}
};

static const PLCList PLC_Explode = {
	1,
	(const PLC[]){
		{art_explosion, 0xB400},
	}
};

static const PLCList PLC_GameOver = {
	1,
	(const PLC[]){
		{art_game_over, 0xABC0},
	}
};

static const PLCList PLC_GHZ = {
	12,
	(const PLC[]){
		{art_ghz1, 0x0000},
		{art_ghz2, 0x39A0},
		{art_ghz_stalk, 0x6B00},
		{art_ghz_rock, 0x7A00},
		{art_crabmeat, 0x8000},
		{art_buzz_bomber, 0x8880},
		{art_chopper, 0x8F60},
		{art_newtron, 0x9360},
		{art_motobug, 0x9E00},
		{art_spikes, 0xA360},
		{art_springh, 0xA460},
		{art_springv, 0xA660},
	}
};

static const PLCList PLC_GHZ2 = {
	6,
	(const PLC[]){
		{art_ghz_swing, 0x7000},
		{art_ghz_bridge, 0x71C0},
		{art_ghz_log, 0x7300},
		{art_ghz_ball, 0x7540},
		{art_ghz_wall1, 0xA1E0},
		{art_ghz_wall2, 0x6980},
	}
};

static const PLCList PLC_LZ = {
	1,
	(const PLC[]){
		{art_lz, 0x0000},
		//plcm	Nem_LzBlock1, $3C00	; block
		//plcm	Nem_LzBlock2, $3E00	; blocks
		//plcm	Nem_Splash, $4B20	; waterfalls and splash
		//plcm	Nem_Water, $6000	; water	surface
		//plcm	Nem_LzSpikeBall, $6200	; spiked ball
		//plcm	Nem_FlapDoor, $6500	; flapping door
		//plcm	Nem_Bubbles, $6900	; bubbles and numbers
		//plcm	Nem_LzBlock3, $7780	; block
		//plcm	Nem_LzDoor1, $7880	; vertical door
		//plcm	Nem_Harpoon, $7980	; harpoon
		//plcm	Nem_Burrobot, $94C0	; burrobot enemy
	}
};

static const PLCList PLC_LZ2 = {
	0,
	(const PLC[]){
		{NULL, 0}, //ISO C forbids empty initializer braces
		//plcm	Nem_LzPole, $7BC0	; pole that breaks
		//plcm	Nem_LzDoor2, $7CC0	; large	horizontal door
		//plcm	Nem_LzWheel, $7EC0	; wheel
		//plcm	Nem_Gargoyle, $5D20	; gargoyle head
		//if Revision=0
		//plcm	Nem_LzSonic, $8800	; Sonic	holding	his breath
		//else
		//endc
		//plcm	Nem_LzPlatfm, $89E0	; rising platform
		//plcm	Nem_Orbinaut, $8CE0	; orbinaut enemy
		//plcm	Nem_Jaws, $90C0		; jaws enemy
		//plcm	Nem_LzSwitch, $A1E0	; switch
		//plcm	Nem_Cork, $A000		; cork block
		//plcm	Nem_Spikes, $A360	; spikes
		//plcm	Nem_HSpring, $A460	; horizontal spring
		//plcm	Nem_VSpring, $A660	; vertical spring
	}
};

static const PLCList PLC_MZ = {
	1,
	(const PLC[]){
		{art_mz, 0x0000},
		//plcm	Nem_MzMetal, $6000	; metal	blocks
		//plcm	Nem_MzFire, $68A0	; fireballs
		//plcm	Nem_Swing, $7000	; swinging platform
		//plcm	Nem_MzGlass, $71C0	; green	glassy block
		//plcm	Nem_Lava, $7500		; lava
		//plcm	Nem_Buzz, $8880		; buzz bomber enemy
		//plcm	Nem_Yadrin, $8F60	; yadrin enemy
		//plcm	Nem_Basaran, $9700	; basaran enemy
		//plcm	Nem_Cater, $9FE0	; caterkiller enemy
	}
};

static const PLCList PLC_MZ2 = {
	0,
	(const PLC[]){
		{NULL, 0}, //ISO C forbids empty initializer braces
		//plcm	Nem_MzSwitch, $A260	; switch
		//plcm	Nem_Spikes, $A360	; spikes
		//plcm	Nem_HSpring, $A460	; horizontal spring
		//plcm	Nem_VSpring, $A660	; vertical spring
		//plcm	Nem_MzBlock, $5700	; green	stone block
	}
};

static const PLCList PLC_SLZ = {
	1,
	(const PLC[]){
		{art_slz, 0x0000},
		//plcm	Nem_Bomb, $8000		; bomb enemy
		//plcm	Nem_Orbinaut, $8520	; orbinaut enemy
		//plcm	Nem_MzFire, $9000	; fireballs
		//plcm	Nem_SlzBlock, $9C00	; block
		//plcm	Nem_SlzWall, $A260	; breakable wall
		//plcm	Nem_Spikes, $A360	; spikes
		//plcm	Nem_HSpring, $A460	; horizontal spring
		//plcm	Nem_VSpring, $A660	; vertical spring
	}
};

static const PLCList PLC_SLZ2 = {
	0,
	(const PLC[]){
		{NULL, 0}, //ISO C forbids empty initializer braces
		//plcm	Nem_Seesaw, $6E80	; seesaw
		//plcm	Nem_Fan, $7400		; fan
		//plcm	Nem_Pylon, $7980	; foreground pylon
		//plcm	Nem_SlzSwing, $7B80	; swinging platform
		//plcm	Nem_SlzCannon, $9B00	; fireball launcher
		//plcm	Nem_SlzSpike, $9E00	; spikeball
	}
};

static const PLCList PLC_SYZ = {
	1,
	(const PLC[]){
		{art_syz, 0x0000},
		//plcm	Nem_Crabmeat, $8000	; crabmeat enemy
		//plcm	Nem_Buzz, $8880		; buzz bomber enemy
		//plcm	Nem_Yadrin, $8F60	; yadrin enemy
		//plcm	Nem_Roller, $9700	; roller enemy
	}
};

static const PLCList PLC_SYZ2 = {
	0,
	(const PLC[]){
		{NULL, 0}, //ISO C forbids empty initializer braces
		//plcm	Nem_Bumper, $7000	; bumper
		//plcm	Nem_SyzSpike1, $72C0	; large	spikeball
		//plcm	Nem_SyzSpike2, $7740	; small	spikeball
		//plcm	Nem_Cater, $9FE0	; caterkiller enemy
		//plcm	Nem_LzSwitch, $A1E0	; switch
		//plcm	Nem_Spikes, $A360	; spikes
		//plcm	Nem_HSpring, $A460	; horizontal spring
		//plcm	Nem_VSpring, $A660	; vertical spring
	}
};

static const PLCList PLC_SBZ = {
	1,
	(const PLC[]){
		{art_sbz, 0x0000},
		//plcm	Nem_Stomper, $5800	; moving platform and stomper
		//plcm	Nem_SbzDoor1, $5D00	; door
		//plcm	Nem_Girder, $5E00	; girder
		//plcm	Nem_BallHog, $6040	; ball hog enemy
		//plcm	Nem_SbzWheel1, $6880	; spot on large	wheel
		//plcm	Nem_SbzWheel2, $6900	; wheel	that grabs Sonic
		//plcm	Nem_SyzSpike1, $7220	; large	spikeball
		//plcm	Nem_Cutter, $76A0	; pizza	cutter
		//plcm	Nem_FlamePipe, $7B20	; flaming pipe
		//plcm	Nem_SbzFloor, $7EA0	; collapsing floor
		//plcm	Nem_SbzBlock, $9860	; vanishing block
	}
};

static const PLCList PLC_SBZ2 = {
	0,
	(const PLC[]){
		{NULL, 0}, //ISO C forbids empty initializer braces
		//plcm	Nem_Cater, $5600	; caterkiller enemy
		//plcm	Nem_Bomb, $8000		; bomb enemy
		//plcm	Nem_Orbinaut, $8520	; orbinaut enemy
		//plcm	Nem_SlideFloor, $8C00	; floor	that slides away
		//plcm	Nem_SbzDoor2, $8DE0	; horizontal door
		//plcm	Nem_Electric, $8FC0	; electric orb
		//plcm	Nem_TrapDoor, $9240	; trapdoor
		//plcm	Nem_SbzFloor, $7F20	; collapsing floor
		//plcm	Nem_SpinPform, $9BE0	; small	spinning platform
		//plcm	Nem_LzSwitch, $A1E0	; switch
		//plcm	Nem_Spikes, $A360	; spikes
		//plcm	Nem_HSpring, $A460	; horizontal spring
		//plcm	Nem_VSpring, $A660	; vertical spring
	}
};

static const PLCList PLC_Signpost = {
	3,
	(const PLC[]){
		{art_signpost,     0xD000},
		{art_hidden_bonus, 0x96C0},
		{art_big_flash,    0x8C40},
	}
};

static const PLCList PLC_SpecialStage = {
	16,
	(const PLC[]){
		{art_ss_clouds,  0x0000},
		{art_ss_back,    0x0A20},
		{art_ss_wall,    0x2840},
		//Nem_Bumper, $4760	; bumper
		{art_ss_goal,    0x4A20},
		{art_ss_speed,   0x4C60},
		{art_ss_rotate,  0x5E00},
		{art_ss_life,    0x6E00},
		{art_ss_twinkle, 0x7E00},
		{art_ss_checker, 0x8E00},
		{art_ss_ghost,   0x9E00},
		{art_ss_warp,    0xAE00},
		{art_ss_glass,   0xBE00},
		{art_ss_emerald, 0xEE00},
		{art_ss_zone1,   0xF2E0},
		{art_ss_zone2,   0xF400},
		{art_ss_zone3,   0xF520},
		//These last 3 are unused
		{art_ss_zone4,   0xF2E0},
		{art_ss_zone5,   0xF400},
		{art_ss_zone6,   0xF520},
	}
};

//PLC list
static const PLCList *plcs[PlcId_Num] = {
	/* PlcId_Main        */ &PLC_Main,
	/* PlcId_Main2       */ &PLC_Main2,
	/* PlcId_Explode     */ &PLC_Explode,
	/* PlcId_GameOver    */ &PLC_GameOver,
	/* PlcId_GHZ         */ &PLC_GHZ,
	/* PlcId_GHZ2        */ &PLC_GHZ2,
	/* PlcId_LZ          */ &PLC_LZ,
	/* PlcId_LZ2         */ &PLC_LZ2,
	/* PlcId_MZ          */ &PLC_MZ,
	/* PlcId_MZ2         */ &PLC_MZ2,
	/* PlcId_SLZ         */ &PLC_SLZ,
	/* PlcId_SLZ2        */ &PLC_SLZ2,
	/* PlcId_SYZ         */ &PLC_SYZ,
	/* PlcId_SYZ2        */ &PLC_SYZ2,
	/* PlcId_SBZ         */ &PLC_SBZ,
	/* PlcId_SBZ2        */ &PLC_SBZ2,
	/* PlcId_TitleCard   */ NULL,
	/* PlcId_Boss        */ NULL,
	/* PlcId_Signpost    */ &PLC_Signpost,
	/* PlcId_Warp        */ NULL,
	/* PlcId_SpecialStage*/ &PLC_SpecialStage,
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
	if (list == NULL)
		return;
	
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
	if (list == NULL)
		return;
	
	//Clear previous PLCs
	ClearPLC();
	
	//Push PLCs to buffer
	for (size_t i = 0; i < list->plcs; i++)
		plc_buffer[i] = list->plc[i];
}

void ClearPLC()
{
	//Clear PLC buffer
	plc_buffer_reg18 = 0;
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
	VDP_SeekVRAM(off);
	
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

void ProcessDPLC()
{
	if (plc_buffer_reg18 != 0)
	{
		plc_buffer_reg1A = PLC_SPEED_1; //Process PLC_SPEED_1 tiles
		
		size_t off = plc_buffer[0].off;
		plc_buffer[0].off += PLC_SPEED_1 * 0x20;
		
		ProcessDPLC_Main(off);
	}
}

void ProcessDPLC2()
{
	if (plc_buffer_reg18 != 0)
	{
		plc_buffer_reg1A = PLC_SPEED_2; //Process PLC_SPEED_2 tiles
		
		size_t off = plc_buffer[0].off;
		plc_buffer[0].off += PLC_SPEED_2 * 0x20;
		
		ProcessDPLC_Main(off);
	}
}

void QuickPLC(PlcId plc)
{
	//Get PLC list to load and decompress immediately
	const PLCList *list = plcs[plc];
	if (list == NULL)
		return;
	for (size_t i = 0; i < list->plcs; i++)
	{
		VDP_SeekVRAM(list->plc[i].off);
		NemDec(list->plc[i].art);
	}
}
