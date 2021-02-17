#include "MegaDrive.h"

#include "VDP.h"

//System backend interface
int System_Init(const MD_Header *header);
void System_Quit();

//MegaDrive interface
int MegaDrive_Start(const MD_Header *header)
{
	int result = 0;
	
	//Initialize MegaDrive subsystems
	if (((result = System_Init(header)) ||
	     (result = VDP_Init(header))) == 0)
	{
		//Run entry point
		header->entry_point();
	}
	
	//Quit MegaDrive subsystems
	MegaDrive_Quit();
	return result;
}

void MegaDrive_Quit()
{
	//Quit MegaDrive subsystems
	VDP_Quit();
	System_Quit();
}
