#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct NemesisState
{
	const uint8_t *source; // a0
	uint8_t *dictionary;   // a1
	bool xor_mode;         // a3
	bool vram_mode;        // a3
	uint8_t *destination;  // a4
	uint16_t remaining;    // a5
	uint8_t d0;            // d0
	uint8_t d1;            // d1
	uint32_t d2;           // d2
	uint16_t d3;           // d3
	uint32_t d4;           // d4
	uint16_t d5;           // d5
	uint16_t d6;           // d6
} NemesisState;

extern uint8_t nemesis_buffer[0x200];

void NemDecPrepare(NemesisState *state);
void NemDecRun(NemesisState *state);
void NemDecSeek(size_t off);
void NemDec(const uint8_t *source);
void NemDecToRAM(const uint8_t *source, uint8_t *destination);
