#pragma once

#include <stdint.h>
#include <stddef.h>

//PLC structure
typedef struct
{
	const uint8_t *art;
	size_t off;
} PLC;

//PLC buffer
extern PLC plc_buffer[16];

//PLC IDs
typedef enum
{
	PlcId_Main,
	PlcId_Main2,
	PlcId_Explode,
	PlcId_GameOver,
	PlcId_GHZ,
	PlcId_GHZ2,
	PlcId_LZ,
	PlcId_LZ2,
	PlcId_MZ,
	PlcId_MZ2,
	PlcId_SLZ,
	PlcId_SLZ2,
	PlcId_SYZ,
	PlcId_SYZ2,
	PlcId_SBZ,
	PlcId_SBZ2,
	PlcId_TitleCard,
	PlcId_Boss,
	PlcId_Signpost,
	PlcId_Warp,
	PlcId_SpecialStage,
	PlcId_GHZAnimals,
	PlcId_LZAnimals,
	PlcId_MZAnimals,
	PlcId_SLZAnimals,
	PlcId_SYZAnimals,
	PlcId_SBZAnimals,
	PlcId_SSResult,
	PlcId_Ending,
	PlcId_TryAgain,
	PlcId_EggmanSBZ2,
	PlcId_FZBoss,
	PlcId_Num,
} PlcId;

//Level art
extern const uint8_t art_ghz1[];
extern const uint8_t art_ghz2[];
extern const uint8_t art_lz[];
extern const uint8_t art_mz[];
extern const uint8_t art_slz[];
extern const uint8_t art_syz[];
extern const uint8_t art_sbz[];

//PLC interface
void AddPLC(PlcId plc);
void ClearPLC();
void RunPLC();
void ProcessDPLC();
void ProcessDPLC2();
void QuickPLC(PlcId plc);
