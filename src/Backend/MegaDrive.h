#pragma once

#include <stddef.h>
#include <stdint.h>

//General types
typedef void(*MD_Vector)();

enum MD_Region
{
	Region_J,
	Region_U,
	Region_E,
};

//Program header
typedef struct
{
	//Vectors
	MD_Vector entry_point; //Start of program
	MD_Vector h_interrupt; //Horizontal interrupt
	MD_Vector v_interrupt; //Vertical interrupt
	
	//Game information
	const char *title;     //Game title
} MD_Header;

//MegaDrive interface
int MegaDrive_Start(const MD_Header *header);
