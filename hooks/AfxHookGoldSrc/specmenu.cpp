// BEGIN HLSDK includes
//
// HACK: prevent cldll_int.h from messing the HSPRITE definition,
// HLSDK's HSPRITE --> MDTHACKED_HSPRITE
#pragma push_macro("HSPRITE")
#define HSPRITE MDTHACKED_HSPRITE
//
#include <hlsdk/multiplayer/cl_dll/wrect.h>
#include <hlsdk/multiplayer/cl_dll/cl_dll.h>
#include <hlsdk/multiplayer/engine/cdll_int.h>
#include <hlsdk/multiplayer/common/cvardef.h>
//
#undef HSPRITE
#pragma pop_macro("HSPRITE")
// END HLSDK includes

#include <hooks/shared/detours.h>
#include "cmdregister.h"

#include "hl_addresses.h"

extern cl_enginefuncs_s *pEngfuncs;


REGISTER_CMD_FUNC(disable_specmenu)
{
	const char *gamedir = pEngfuncs->pfnGetGameDirectory();
	unsigned short *usCheck = NULL;

	if( !strcmp("ag",gamedir) )
	{
		usCheck = (unsigned short *)((unsigned char *)(GetModuleHandle("client.dll")) + HL_ADDR_GET(UpdateSpectatorPanel_checkjmp_ag_clofs));
	}
	else if( !strcmp("tfc",gamedir) )
	{
		usCheck = (unsigned short *) HL_ADDR_GET(UpdateSpectatorPanel_checkjmp_tfc);
	}
	else if( !strcmp("ns",gamedir) )
	{
		usCheck = (unsigned short *)((unsigned char *)(GetModuleHandle("client.dll")) + HL_ADDR_GET(UpdateSpectatorPanel_checkjmp_ns_clofs));
	}
	else if( !strcmp("valve",gamedir) )
	{
		usCheck = (unsigned short *) HL_ADDR_GET(UpdateSpectatorPanel_checkjmp_valve);
	}

	pEngfuncs->Con_DPrintf("0x%08x\n",usCheck);

	if (!usCheck)
	{
		pEngfuncs->Con_Printf(
			"Sorry, your mod (%s) is not supported for this command.\n"
			"May be you can remove the spec menu by modifing a .res file like for cstrike?\n",
			gamedir
		);
	}
	else if (pEngfuncs->Cmd_Argc() == 2)
	{
		int iOn = atoi(pEngfuncs->Cmd_Argv(1));

		MdtMemBlockInfos mbis;
		MdtMemAccessBegin(usCheck, 2, &mbis);
		if (!iOn)
		{
			*usCheck = 0x840f; // original JE code
		}
		else
		{
			*usCheck = 0xE990; // NOP + JMP opcode
		}
		MdtMemAccessEnd(&mbis);

	}
	else
		pEngfuncs->Con_Printf("Usage: " PREFIX "disable_specmenu 0 (disable) / 1 (enable)\n");

}