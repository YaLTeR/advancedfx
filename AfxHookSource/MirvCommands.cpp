#include "stdafx.h"

// Copyright (c) advancedfx.org
//
// Last changes:
// 2017-03-15 dominik.matrixstorm.com
//
// First changes:
// 2009-09-30 by dominik.matrixstorm.com


// Hint: for now commands are registered upon the first client.dll CreateInterface() call

#include <shared/StringTools.h>

#include "RenderView.h"
#include "SourceInterfaces.h"
#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "WrpVEngineClient.h"
#include "csgo_CHudDeathNotice.h"
#include "csgo_GetPlayerName.h"
#include "csgo_SndMixTimeScalePatch.h"
#include "AfxHookSourceInput.h"
#include <shared/hooks/gameOverlayRenderer.h>
#include "AfxStreams.h"
#include "addresses.h"
#include "CampathDrawer.h"
#include "csgo_S_StartSound.h"
#include "d3d9Hooks.h"
#include "aiming.h"
#include "CommandSystem.h"
#include <shared/binutils.h>
#include "ClientTools.h"
#include "csgo_CBasePlayer.h"
#include "MirvInputMem.h"
#include "csgo_CCSGameMovement.h"
#include "csgo_vphysics.h"
#include "csgo_c_baseentity.h"
#include"csgo_c_baseanimatingoverlay.h"

#include "csgo_Stdshader_dx9_Hooks.h"

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <cctype>
#include <sstream>
#include <iomanip>

extern WrpVEngineClient * g_VEngineClient;

HMODULE g_H_EngineDll = 0;


#if AFXSTREAMS_REFTRACKER

CON_COMMAND(__mirv_streams_ref, "")
{
	Tier0_Msg("Current tracked count: %i\n", AfxStreams_RefTracker_Get());
}

#endif


float mirv_setup_add = 0;

CON_COMMAND(__mirv_test8, "")
{
	int argc = args->ArgC();

	if (2 > argc)
		return;


	int idx = atoi(args->ArgV(1));

	SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(idx);
	SOURCESDK::C_BaseEntity_csgo * be = ce ? ce->GetBaseEntity() : 0;

	if (be)
	{
		int * p = (int *)((char *)be + 0xA38);
		int * pp = (int *)((char *)be + 0xA38+0x8);

		float *fp = (float *)((char *)be + 0x25c);
		*pp = *p;

		*fp = g_Hook_VClient_RenderView.GetGlobals()->curtime_get() + g_Hook_VClient_RenderView.GetGlobals()->frametime_get();

		Tier0_Msg("%i: %i / %f\n", idx, *p, *fp);
	}
}


CON_COMMAND(__mirv_test6, "")
{
	int argc = args->ArgC();

	if (2 > argc)
		return;


	int idx = atoi(args->ArgV(1));
	
	SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(idx);
	SOURCESDK::C_BaseEntity_csgo * be = ce ? ce->GetBaseEntity() : 0;

	if (be)
	{
		static float old_m_flAnimTime = 0;
		static float old_m_flSimualtionTime = 0;
		static float old_curtime = 0;

		float m_flAnimTime = *(float *)((char *)be +0x25c);
		float m_flSimulationTime = *(float *)((char *)be + 0x264);
		float curtime = g_Hook_VClient_RenderView.GetGlobals()->curtime_get();

		float delta_m_flAnimTime = m_flAnimTime - old_m_flAnimTime;
		float delta_m_flSimualtionTime = m_flSimulationTime - old_m_flSimualtionTime;
		float delta_curtime = curtime - old_curtime;

		bool oneIsNeg = delta_m_flAnimTime < 0 || delta_m_flSimualtionTime < 0 || delta_curtime < 0;

		if (!oneIsNeg)
			Tier0_Msg(
				"%i: animTime=%f (%f), simulationTime=%f (%f) | curTime=%f (%f)\n"
				, idx, m_flAnimTime, delta_m_flAnimTime, m_flSimulationTime, delta_m_flSimualtionTime, curtime, delta_curtime
			);
		else
			Tier0_Warning(
				"%i: animTime=%f (%f), simulationTime=%f (%f) | curTime=%f (%f)\n"
				, idx, m_flAnimTime, delta_m_flAnimTime, m_flSimulationTime, delta_m_flSimualtionTime, curtime, delta_curtime
			);

		old_m_flAnimTime = m_flAnimTime;
		old_m_flSimualtionTime = m_flSimulationTime;
		old_curtime = curtime;
	}
}

std::map<void *, float> m_MirvSetupMap;

//extern HMODULE g_H_ClientDll;

void  __declspec(naked) __declspec(dllexport) mirv_setup(void)
{
	/*
	__asm mov eax, [g_H_ClientDll]
	__asm add eax, 0x1D8D97
	__asm mov tmp, eax
	__asm jmp [tmp]
	*/
}

CON_COMMAND(__mirv_ct, "")
{
	int argc = args->ArgC();

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (!_stricmp("start", cmd1))
		{

			g_ClientTools.StartRecording(L"afxGameRecord.agr");
		}
		else
		if (!_stricmp("end", cmd1))
		{
			g_ClientTools.EndRecording();
		}
		else
		if (!_stricmp("debug", cmd1) && 3 <= argc)
		{
			g_ClientTools.DebugEntIndex(atoi(args->ArgV(2)));
		}

	}
}

CON_COMMAND(__mirv_test5, "")
{
	int argc = args->ArgC();

	if (3 <= argc)
	{
		HMODULE hModule = GetModuleHandleA(args->ArgV(1));

		if (!hModule)
		{
			Tier0_Warning("Invalid module name.\n");
			return;
		}

		char const * cmd1 = args->ArgV(2);

		Afx::BinUtils::ImageSectionsReader sections(hModule);
		if (!sections.Eof())
		{
			Afx::BinUtils::MemRange search = sections.GetMemRange();

			Afx::BinUtils::MemRange result;

			int numResults = 0;

			while (!search.IsEmpty())
			{
				result = FindPatternString(search, cmd1);

				if (result.IsEmpty())
					break;

				Tier0_Msg(
					"%i: [0x%08x,0x%08x)\n",
					numResults,
					result.Start - (DWORD)hModule,
					result.End - (DWORD)hModule
				);

				search = Afx::BinUtils::MemRange(result.End, search.End);

				++numResults;

				if (100 < numResults)
				{
					Tier0_Msg("RESULT LIMIT REACHED\n");
					break;
				}
			}
		}
	}
}

CON_COMMAND(__mirv_test4, "")
{
	if (3 <= args->ArgC())
	{
		WrpConVarRef ref(args->ArgV(1));

		ref.SetValue((float)atof(args->ArgV(2)));
	}
}

CON_COMMAND(__mirv_test3, "")
{
	if (2 <= args->ArgC())
	{
		WrpConVarRef ref(args->ArgV(1));

		Tier0_Msg("%s: f:%f, i:%i\n", args->ArgV(1), ref.GetFloat(), ref.GetInt());
	}
}

CON_COMMAND(__mirv_test2, "")
{
	WrpVEngineClientDemoInfoEx * di = g_VEngineClient->GetDemoInfoEx();

	if(di && g_Hook_VClient_RenderView.GetGlobals())
	{
		Tier0_Msg(
			"GetDemoRecordingTick=%i\n"
			"GetDemoPlaybackTick=%i\n"
			"GetDemoPlaybackStartTick=%i\n"
			"GetDemoPlaybackTimeScale=%f\n"
			"GetDemoPlaybackTotalTicks=%i\n",
			di->GetDemoRecordingTick(),
			di->GetDemoPlaybackTick(),
			di->GetDemoPlaybackStartTick(),
			di->GetDemoPlaybackTimeScale(),
			di->GetDemoPlaybackTotalTicks()
		);

		double curTime = g_Hook_VClient_RenderView.GetGlobals()->curtime_get();
		int client_current_tick = di->GetDemoPlaybackTick();
		double tick_interval = g_Hook_VClient_RenderView.GetGlobals()->interval_per_tick_get();
		double interpolation_amount = g_Hook_VClient_RenderView.GetGlobals()->interpolation_amount_get();

		double result = client_current_tick * tick_interval + interpolation_amount* tick_interval;

		Tier0_Msg(
			"curtime(%f) == client_current_tick(%i) * tick_interval(%f) +interpolation_amount(%f) * tick_interval == %f\n",
			curTime,
			client_current_tick,
			tick_interval,
			interpolation_amount,
			result
		);
	}
}

bool g_bD3D9DebugPrint = false;

CON_COMMAND(__mirv_test, "")
{
	g_bD3D9DebugPrint = true;
}

CON_COMMAND(__mirv_skyboxscale, "print skyboxscale in CS:GO")
{
	if(AFXADDR_GET(csgo_pLocalPlayer) && 0 != *(unsigned char **)AFXADDR_GET(csgo_pLocalPlayer))
	{
		int skyBoxScale = *(int *)(*(unsigned char **)AFXADDR_GET(csgo_pLocalPlayer) +AFXADDR_GET(csgo_C_BasePlayer_OFS_m_skybox3d_scale));

		Tier0_Msg("skyBoxScale: %i\n", skyBoxScale);
	}
	else
		Tier0_Msg("skyBoxScale: n/a\n");
}

CON_COMMAND(mirv_streams, "Access to streams system.")
{
	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if(!_stricmp(cmd1, "add"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if(!_stricmp(cmd2, "normal"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add normal <name> - Add a normal stream with name <name>.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "baseFx"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddBaseFxStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add baseFx <name> - Add a baseFx stream with name <name>.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "depth"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddDepthStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add depth <name> - Add a depth stream with name <name>.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "matteWorld"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddMatteWorldStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add matteWorld <name> - Add a matte world stream with name <name>.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "depthWorld"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddDepthWorldStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add depthWorld <name> - Add a depth world stream with name <name>.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "matteEntity"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddMatteEntityStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add matteEnitity <name> - Add a matte entity stream with name <name>.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "depthEntity"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddDepthEntityStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add depthEntity <name> - Add a depth entity stream with name <name>.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "alphaMatteEntity"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddAlphaMatteEntityStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add alphaMatteEntity <name> - Add a alpha matte entity stream with name <name>.\n"
					);
					return;
				}				
				else
				if(!_stricmp(cmd2, "alphaMatte"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddAlphaMatteStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add alphaMatte <name> - Add a alpha matte stream with name <name>.\n"
					);
					return;
				}				
				else
				if(!_stricmp(cmd2, "alphaEntity"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddAlphaEntityStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add alphaEntity <name> - Add a alpha entity stream with name <name>.\n"
					);
					return;
				}				
				else
				if(!_stricmp(cmd2, "alphaWorld"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						g_AfxStreams.Console_AddAlphaWorldStream(cmd3);

						return;
					}

					Tier0_Msg(
						"mirv_streams add alphaWorld <name> - Add a alpha world stream with name <name>.\n"
					);
					return;
				}				
			}

			Tier0_Msg(
				"mirv_streams add normal [...] - Add a normal stream.\n"
				"mirv_streams add baseFx [...] - Add a stream that allows effects, but should not look different from normal stream unless you edit its settings.\n"
				"mirv_streams add depth [...] - Add a depth stream. WILL NOT WORK PROPERLY ATM!\n"
				"mirv_streams add matteWorld [...] - Add a matte world stream.\n"
				"mirv_streams add depthWorld [...] - Add a depth world stream. WILL NOT WORK PROPERLY ATM!\n"
				"mirv_streams add matteEntity [...] - Add a matte entity stream.\n"
				"mirv_streams add depthEntity [...] - Add a depth entity stream. WILL NOT WORK PROPERLY ATM!\n"
				"mirv_streams add alphaMatteEntity [...] - Add a entity stream with alpha matte combined into a single stream.\n"
				"mirv_streams add alphaWorld [...] - Add a alpha world stream.\n"
				"mirv_streams add alphaMatte [...] - Add a alpha matte stream (alpha channel of alphaMatteEntity).\n"
				"mirv_streams add alphaEntity [...] - Add a alpha entity stream (color channel of alphaMatteEntity).\n"
			);
			return;
		}
		else
		if(!_stricmp(cmd1, "edit"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				CSubWrpCommandArgs subArgs(args, 3);

				g_AfxStreams.Console_EditStream(cmd2, &subArgs);
				return;
			}

			Tier0_Msg(
				"mirv_streams edit <streamName> [...] - Edit the stream with name <streamName>, you can get the value from mirv_streams print. <streamName> can match multiple streams when using wildcards (\\* = wildcard and \\\\ = \\), however this will only do something useful when the streams matched are compatible!\n"
			);
			return;
		}
		else
		if(!_stricmp(cmd1, "remove"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);
				
				g_AfxStreams.Console_RemoveStream(cmd2);
				return;
			}

			Tier0_Msg(
				"mirv_streams remove <streamName> - Remove a stream with name <streamName>, you can get the value from mirv_streams print.\n"
			);
			return;
		}
		else
		if(!_stricmp(cmd1, "preview"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);
				
				g_AfxStreams.Console_PreviewStream(cmd2);
				return;
			}

			Tier0_Msg(
				"mirv_streams preview <streamName> - Preview the stream with name <streamName>, you can get the value from mirv_streams print. To end previewing enter \"\" (empty string) for <streamName>!\n"
			);
			return;
		}
		else
		if(!_stricmp(cmd1, "previewEnd"))
		{
			g_AfxStreams.Console_PreviewStream("");
			return;
		}

		else
		if(!_stricmp(cmd1, "print"))
		{
			g_AfxStreams.Console_PrintStreams();

			return;
		}
		else
		if(!_stricmp(cmd1, "record"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if(!_stricmp(cmd2, "name"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_RecordName_set(cmd3);
						return;
					}

					Tier0_Msg(
						"mirv_streams record name <name> - Set record name to <name>.\n"
						"Current value: %s.\n",
						g_AfxStreams.Console_RecordName_get()
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "start"))
				{
					g_AfxStreams.Console_Record_Start();
					return;
				}
				else
				if(!_stricmp(cmd2, "end"))
				{
					g_AfxStreams.Console_Record_End();
					return;
				}
				else
				if(!_stricmp(cmd2, "format"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_RecordFormat_set(cmd3);
						return;
					}

					Tier0_Msg(
						"mirv_streams record format tga|bmp - Set record format to tga or bmp.\n"
						"Current value: %s.\n",
						g_AfxStreams.Console_RecordFormat_get()
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "presentOnScreen"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_PresentRecordOnScreen_set(0 != atoi(cmd3));
						return;
					}

					Tier0_Msg(
						"mirv_streams record presentOnScreen 0|1 - Whether to show recording on screen (where possible) (1) [May cause epileptic seizures!] or not (0).\n"
						"Current value: %s.\n",
						g_AfxStreams.Console_PresentRecordOnScreen_get() ? "1" : "0"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "matForceTonemapScale"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_MatForceTonemapScale_set((float)atof(cmd3));
						return;
					}

					Tier0_Msg(
						"mirv_streams record matForceTonemapScale <fValue> - Force floating point value <fValue> for mat_force_tonemap_scale during recording.\n"
						"Current value: %f.\n",
						g_AfxStreams.Console_MatForceTonemapScale_get()
					);
					return;
				}
				else
				if (!_stricmp(cmd2, "startMovieWav"))
				{
					if (4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						g_AfxStreams.Console_StartMovieWav_set(0 != atoi(cmd3));
						return;
					}

					Tier0_Msg(
						"mirv_streams record startMovieWav 0|1 - Whether to record WAV audio (1) or not (0).\n"
						"Current value: %s.\n",
						g_AfxStreams.Console_StartMovieWav_get() ? "1" : "0"
					);
					return;
				}
				else
				if (!_stricmp(cmd2, "bvh"))
				{
					CSubWrpCommandArgs subArgs(args, 3);

					g_AfxStreams.Console_Bvh(&subArgs);
					return;
				}
				else
				if (!_stricmp(cmd2, "agr"))
				{
					CSubWrpCommandArgs subArgs(args, 3);

					g_AfxStreams.Console_GameRecording(&subArgs);
					return;
				}
			}

			Tier0_Msg(
				"mirv_streams record name [...] - Set/get record name.\n"
				"mirv_streams record start - Begin recording.\n"
				"mirv_streams record end - End recording.\n"
				"mirv_streams record format [...] - Set/get file format.\n"
				"mirv_streams record presentOnScreen [...] - Controls screen presentation during recording.\n"
				"mirv_streams record matForceTonemapScale [...] - Controls mat_force_tonemap_scale variable during recording.\n"
				"mirv_streams record startMovieWav [...] - Controls whether startmovie shall be used for automatically recording audio.\n"
				"mirv_streams record bvh [...] - Controls the HLAE/BVH Camera motion data capture output.\n"
				"mirv_streams record agr [...] - Controls afxGameRecord (.agr) game state recording [still in developement, file format will have breaking changes].\n"
			);
			return;
		}
		else
		if(!_stricmp(cmd1, "actions"))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if(!_stricmp(cmd2, "add"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						
						if(!_stricmp(cmd3, "replace"))
						{
							CSubWrpCommandArgs subArgs(args, 4);

							CAfxBaseFxStream::Console_AddReplaceAction(&subArgs);
							return;
						}
					}

					Tier0_Msg(
						"mirv_streams actions add replace [...] - Add replace action.\n"
					);
					return;
				}
				else
				if(!_stricmp(cmd2, "print"))
				{
					CAfxBaseFxStream::Console_ListActions();
					return;
				}
				else
				if(!_stricmp(cmd2, "remove"))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);
						CAfxBaseFxStream::RemoveAction(CAfxBaseFxStream::CActionKey(cmd3));
						return;
					}
				}
			}
			Tier0_Msg(
				"mirv_streams actions add [...] - Add an action.\n"
				"mirv_streams actions print - Print available actions.\n"
				"mirv_streams actions remove <actionName> - Remove an action named <actionName>.\n"
			);
			return;
		}
	}

	Tier0_Msg(
		"mirv_streams add [...]- Add a stream.\n"
		"mirv_streams edit [...]- Edit a stream.\n"
		"mirv_streams remove [...] - Remove a stream.\n"
		"mirv_streams preview [...] - Preview a stream.\n"
		"mirv_streams previewEnd - End preview.\n"
		"mirv_streams print - Print current streams.\n"
		"mirv_streams record [...] - Recording control.\n"
		"mirv_streams actions [...] - Actions control (for baseFx based streams).\n"
	);
	return;
}

CON_COMMAND(__mirv_exec, "client command execution: __mirv_exec <as you would have typed here>") {
	unsigned int len=0;
	char *ttt, *ct;

	for(int i=0; i<args->ArgC(); i++)
	{
		len += strlen(args->ArgV(i))+1;
	}

	if(len<1) len=1;

	ct = ttt = (char *)malloc(sizeof(char)*len);

	for(int i=1; i<args->ArgC(); i++) {
		char const * cur = args->ArgV(i);
		unsigned int lcur = strlen(cur);
		
		if(1<i) {
			strcpy(ct, " ");
			ct++;
		}

		strcpy(ct, cur);
		ct += lcur;
	}

	*ct = 0;

	g_VEngineClient->ExecuteClientCmd(ttt);

	delete ttt;
}

bool GetCurrentDemoTick(int &outTick)
{
	WrpVEngineClientDemoInfoEx * di = g_VEngineClient->GetDemoInfoEx();

	if(di)
	{
		outTick = di->GetDemoPlaybackTick();
		return true;
	}

	return false;
}

bool GetDemoTickFromTime(double curTime, double time, int &outTick)
{
	WrpVEngineClientDemoInfoEx * di = g_VEngineClient->GetDemoInfoEx();
	WrpGlobals * gl = g_Hook_VClient_RenderView.GetGlobals();

	if(di && gl)
	{
		int client_current_tick = di->GetDemoPlaybackTick();

		double tick_interval = gl->interval_per_tick_get();

		double interpolation_amount = gl->interpolation_amount_get();

		double demoTime = (client_current_tick +interpolation_amount) * tick_interval;

		double deltaTime = curTime -demoTime;

		time -= deltaTime;

		outTick = (int)round(time / tick_interval);

		return true;
	}

	return false;
}

bool GetDemoTimeFromTime(double curTime, double time, double &outDemoTime)
{
	WrpVEngineClientDemoInfoEx * di = g_VEngineClient->GetDemoInfoEx();
	WrpGlobals * gl = g_Hook_VClient_RenderView.GetGlobals();

	if(di && gl)
	{
		int client_current_tick = di->GetDemoPlaybackTick();

		double tick_interval = gl->interval_per_tick_get();

		double interpolation_amount = gl->interpolation_amount_get();

		double demoTime = (client_current_tick +interpolation_amount) * tick_interval;

		double deltaTime = curTime -demoTime;

		time -= deltaTime;

		outDemoTime = time;

		return true;

	}

	return false;
}

bool GetCurrentDemoTime(double &outDemoTime)
{
	WrpVEngineClientDemoInfoEx * di = g_VEngineClient->GetDemoInfoEx();
	WrpGlobals * gl = g_Hook_VClient_RenderView.GetGlobals();

	if(gl)
	{
		int client_current_tick = di->GetDemoPlaybackTick();

		double tick_interval = gl->interval_per_tick_get();

		double interpolation_amount = gl->interpolation_amount_get();

		double demoTime = (client_current_tick +interpolation_amount) * tick_interval;

		outDemoTime = demoTime;

		return true;
	}

	return false;
}

void PrintTimeFormated(double time)
{
	int seconds = (int)time % 60;

	time /= 60;
	int minutes = (int)time % 60;

	time /= 60;
	int hours = (int)time;

	std::ostringstream oss;

	oss << std::setfill('0') << std::setw(2);

	if(hours)
	{
		oss << hours << "h";
	}

	oss << minutes << "m" << seconds << "s";
	
	Tier0_Msg("%s", oss.str().c_str());
}

CON_COMMAND(mirv_skip, "for skipping trhough demos (uses demo_gototick)")
{
	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(!_stricmp(arg1, "tick"))
		{
			int curTick;
			if(!GetCurrentDemoTick(curTick))
			{
				Tier0_Warning("Error: GetCurrentDemoTick failed!\n");
				return;
			}

			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(!_stricmp(arg2, "to") && 4 <= argc)
				{
					int targetTick = atoi(args->ArgV(3));

					std::ostringstream oss;

					oss << "demo_gototick " << targetTick;

					g_VEngineClient->ExecuteClientCmd(oss.str().c_str());
					
					return;
				}

				if(3 <= argc)
				{
					int deltaTicks = atoi(arg2);
					int targetTick = curTick + deltaTicks;

					std::ostringstream oss;

					oss << "demo_gototick " << targetTick;

					g_VEngineClient->ExecuteClientCmd(oss.str().c_str());

					return;
				}
			}

			Tier0_Msg(
				"mirv_skip tick <iValue> - skip approximately integer value <iValue> ticks (negative values skip back).\n"
				"mirv_skip tick to <iValue> - go approximately to demo tick <iValue>\n"
				"Current demo tick: %i\n",
				curTick
			);
			return;
		}
		else
		if(!_stricmp(arg1, "time"))
		{
			double curTime;
			if(!GetCurrentDemoTime(curTime))
			{
				Tier0_Warning("Error: GetCurrentDemoTime failed!\n");
				return;
			}

			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(!_stricmp(arg2, "to") && 4 <= argc)
				{
					double targetTime = atof(args->ArgV(3));
					int targetTick;

					if(!GetDemoTickFromTime(curTime, targetTime, targetTick))
					{
						Tier0_Warning("Error: GetDemoTickFromTime failed!\n");
						return;
					}

					std::ostringstream oss;

					oss << "demo_gototick " << targetTick;

					g_VEngineClient->ExecuteClientCmd(oss.str().c_str());
					
					return;
				}

				if(3 <= argc)
				{
					double deltaTime = atof(arg2);
					double targetTime = curTime+deltaTime;
					int targetTick;

					if(!GetDemoTickFromTime(curTime, targetTime, targetTick))
					{
						Tier0_Warning("Error: GetDemoTickFromTime failed!\n");
						return;
					}

					std::ostringstream oss;

					oss << "demo_gototick " << targetTick;

					g_VEngineClient->ExecuteClientCmd(oss.str().c_str());
					
					return;
				}
			}

			Tier0_Msg(
				"mirv_skip time <dValue> - skip approximately time <dValue> seconds (negative values skip back).\n"
				"mirv_skip time to <dValue> - go approximately to demo time <dValue> seconds\n"
				"Current demo time in seconds: %f (",
				curTime
			);
			PrintTimeFormated(curTime);
			Tier0_Msg(")\n");

			return;
		}
	}

	Tier0_Msg(
		"mirv_skip tick [...] - skip demo ticks\n"
		"mirv_skip time [...] - skip demo time\n"
	);
	return;
}

CON_COMMAND(mirv_campath,"camera paths")
{
	if(!g_Hook_VClient_RenderView.IsInstalled())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * subcmd = args->ArgV(1);

		if(!_stricmp("add", subcmd) && 2 == argc)
		{
			g_Hook_VClient_RenderView.m_CamPath.Add(
				g_Hook_VClient_RenderView.GetCurTime(),
				CamPathValue(
					g_Hook_VClient_RenderView.LastCameraOrigin[0],
					g_Hook_VClient_RenderView.LastCameraOrigin[1],
					g_Hook_VClient_RenderView.LastCameraOrigin[2],
					g_Hook_VClient_RenderView.LastCameraAngles[0],
					g_Hook_VClient_RenderView.LastCameraAngles[1],
					g_Hook_VClient_RenderView.LastCameraAngles[2],
					g_Hook_VClient_RenderView.LastCameraFov
				)
			);

			return;
		}
		else if(!_stricmp("enabled", subcmd) && 3 == argc
			|| !_stricmp("enable", subcmd) && 3 == argc)
		{
			bool enable = 0 != atoi(args->ArgV(2));
			g_Hook_VClient_RenderView.m_CamPath.Enabled_set(enable);

			if(enable && !g_Hook_VClient_RenderView.m_CamPath.CanEval())
				Tier0_Warning(
					"Warning: Campath enabled but can not be evaluated yet.\n"
					"Did you add enough points?\n"
				);

			return;
		}
		else if(!_stricmp("draw", subcmd))
		{
			if(3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if(!_stricmp("enabled", cmd2))
				{
					if(4 <= argc)
					{
						char const * cmd3 = args->ArgV(3);

						bool enabled = 0 != atoi(cmd3);

						if(enabled)
						{
							Tier0_Msg("AFXINFO: Forcing mat_queue_mode 0\n");
							g_VEngineClient->ExecuteClientCmd("mat_queue_mode 0");
						}

						g_CampathDrawer.Draw_set(enabled);
						return;
					}

					Tier0_Msg(
						"mirv_campath draw enabled 0|1 - enable (1) / disable (0) drawing.\n"
						"Current value: %s\n",
						g_CampathDrawer.Draw_get() ? "1 (enabled)" : "0 (disabled)"
					);
					return;
				}
			}

			Tier0_Msg("mirv_campath draw enabled [...] - enable / disable drawing.\n");
			return;
		}
		else if(!_stricmp("clear", subcmd) && 2 == argc)
		{
			g_Hook_VClient_RenderView.m_CamPath.Clear();

			return;
		}
		else if(!_stricmp("print", subcmd) && 2 == argc)
		{
			Tier0_Msg("passed? selected? id: tick[approximate!], demoTime[approximate!], gameTime -> (x,y,z) fov (pitch,yaw,roll)\n");

			double curtime = g_Hook_VClient_RenderView.GetCurTime();
			
			int i=0;
			for(CamPathIterator it = g_Hook_VClient_RenderView.m_CamPath.GetBegin(); it != g_Hook_VClient_RenderView.m_CamPath.GetEnd(); ++it)
			{
				double vieworigin[3];
				double viewangles[3];
				double fov;

				double time = it.GetTime();
				CamPathValue val = it.GetValue();
				bool selected = val.Selected;
				QEulerAngles ang = val.R.ToQREulerAngles().ToQEulerAngles();

				vieworigin[0] = val.X;
				vieworigin[1] = val.Y;
				vieworigin[2] = val.Z;
				viewangles[0] = ang.Pitch;
				viewangles[1] = ang.Yaw;
				viewangles[2] = ang.Roll;
				fov = val.Fov;

				Tier0_Msg(
					"%s %s %i: ",
					time <= curtime ? "Y" : "n",
					selected ? "Y" : "n",
					i
				);
				
				int myTick;
				if(GetDemoTickFromTime(curtime, time, myTick))
					Tier0_Msg("%i", myTick);
				else
					Tier0_Msg("n/a");

				Tier0_Msg(", ");

				double myDemoTime;
				if(GetDemoTimeFromTime(curtime, time, myDemoTime))
					PrintTimeFormated(myDemoTime);
				else
					Tier0_Msg("n/a");

				Tier0_Msg(", %f -> (%f,%f,%f) %f (%f,%f,%f)\n",
					time,
					vieworigin[0],vieworigin[1],vieworigin[2],
					fov,
					viewangles[0],viewangles[1],viewangles[2]
				);

				i++;
			}
			
			Tier0_Msg("----\n");
			
			Tier0_Msg("Current tick: ");
			int curTick;
			bool hasCurTick;
			if(hasCurTick = GetCurrentDemoTick(curTick))
				Tier0_Msg("%i", curTick);
			else
				Tier0_Msg("n/a");
			Tier0_Msg(", Current demoTime: ");
			double curDemoTime;
			if(hasCurTick && GetCurrentDemoTime(curDemoTime))
				PrintTimeFormated(curDemoTime);
			else
				Tier0_Msg("n/a");
			Tier0_Msg(", Current gameTime: %f\n", curtime);
			
			Tier0_Msg("Current (x,y,z) fov (pitch,yaw,roll): (%f,%f,%f), %f, (%f,%f,%f)\n",
				g_Hook_VClient_RenderView.LastCameraOrigin[0],
				g_Hook_VClient_RenderView.LastCameraOrigin[1],
				g_Hook_VClient_RenderView.LastCameraOrigin[2],
				g_Hook_VClient_RenderView.LastCameraFov,
				g_Hook_VClient_RenderView.LastCameraAngles[0],
				g_Hook_VClient_RenderView.LastCameraAngles[1],
				g_Hook_VClient_RenderView.LastCameraAngles[2]
			);

			return;
		}
		else if(!_stricmp("remove", subcmd) && 3 == argc)
		{
			int idx = atoi(args->ArgV(2));
			int i=0;
			for(CamPathIterator it = g_Hook_VClient_RenderView.m_CamPath.GetBegin(); it != g_Hook_VClient_RenderView.m_CamPath.GetEnd(); ++it)
			{
				if(i == idx)
				{
					double time = it.GetTime();
					g_Hook_VClient_RenderView.m_CamPath.Remove(time);
					break;
				}
				i++;
			}

			return;
		}
		else if(!_stricmp("load", subcmd) && 3 == argc)
		{	
			std::wstring wideString;
			bool bOk = AnsiStringToWideString(args->ArgV(2), wideString)
				&& g_Hook_VClient_RenderView.m_CamPath.Load(wideString.c_str())
			;

			Tier0_Msg("Loading campath: %s.\n", bOk ? "OK" : "ERROR");

			return;
		}
		else if(!_stricmp("save", subcmd) && 3 == argc)
		{	
			std::wstring wideString;
			bool bOk = AnsiStringToWideString(args->ArgV(2), wideString)
				&& g_Hook_VClient_RenderView.m_CamPath.Save(wideString.c_str())
			;

			Tier0_Msg("Saving campath: %s.\n", bOk ? "OK" : "ERROR");

			return;
		}
		else if(!_stricmp("edit", subcmd))
		{	
			if(3 <= argc)
			{
				const char * arg2 = args->ArgV(2);
			
				if(!_stricmp("start", arg2))
				{
					if(3 == argc)
					{
						g_Hook_VClient_RenderView.m_CamPath.SetStart(
							g_Hook_VClient_RenderView.GetCurTime()
						);

						return;
					}
					else
					if(3 < argc)
					{
						const char * arg3 = args->ArgV(3);

						if(!_stricmp("abs",arg3) && 5 <= argc)
						{
							char const * arg4 = args->ArgV(4);

							g_Hook_VClient_RenderView.m_CamPath.SetStart(
								atof(arg4)
							);

							return;
						}
						else
						if(StringBeginsWith(arg3, "delta") && 4 == argc)
						{
							if(StringBeginsWith(arg3, "delta+"))
							{
								arg3 += strlen("delta+");

								g_Hook_VClient_RenderView.m_CamPath.SetStart(
									atof(arg3)
									, true
								);

								return;
							}
							else
							if(StringBeginsWith(arg3, "delta-"))
							{
								arg3 += strlen("delta-");

								g_Hook_VClient_RenderView.m_CamPath.SetStart(
									- atof(arg3)
									, true
								);

								return;
							}
						}
					}

				}
				else
				if(!_stricmp("duration", arg2) && 4 <= argc)
				{
					double duration = atof(args->ArgV(3));

					g_Hook_VClient_RenderView.m_CamPath.SetDuration(
						duration
					);

					return;
				}
				else
				if(!_stricmp(arg2, "position") && 4 <= argc)
				{
					char const * arg3 = args->ArgV(3);

					if(!_stricmp("current", arg3))
					{
						g_Hook_VClient_RenderView.m_CamPath.SetPosition(
							g_Hook_VClient_RenderView.LastCameraOrigin[0],
							g_Hook_VClient_RenderView.LastCameraOrigin[1],
							g_Hook_VClient_RenderView.LastCameraOrigin[2]
						);

						return;
					}
					else
					if(6 <= argc)
					{
						char const * arg4 = args->ArgV(4);
						char const * arg5 = args->ArgV(5);
						g_Hook_VClient_RenderView.m_CamPath.SetPosition(
							atof(arg3),
							atof(arg4),
							atof(arg5)
						);

						return;
					}
				}
				else
				if(!_stricmp(arg2, "angles") && 4 <= argc)
				{
					char const * arg3 = args->ArgV(3);

					if(!_stricmp("current", arg3))
					{
						g_Hook_VClient_RenderView.m_CamPath.SetAngles(
							g_Hook_VClient_RenderView.LastCameraAngles[0],
							g_Hook_VClient_RenderView.LastCameraAngles[1],
							g_Hook_VClient_RenderView.LastCameraAngles[2]
						);

						return;
					}
					else
					if(6 <= argc)
					{
						char const * arg4 = args->ArgV(4);
						char const * arg5 = args->ArgV(5);
						g_Hook_VClient_RenderView.m_CamPath.SetAngles(
							atof(arg3),
							atof(arg4),
							atof(arg5)
						);

						return;
					}
				}
				else
				if(!_stricmp(arg2, "fov") && 4 <= argc)
				{
					char const * arg3 = args->ArgV(3);

					if(!_stricmp("current", arg3))
					{
						g_Hook_VClient_RenderView.m_CamPath.SetFov(
							g_Hook_VClient_RenderView.LastCameraFov
						);

						return;
					}
					else
					{
						g_Hook_VClient_RenderView.m_CamPath.SetFov(
							atof(arg3)
						);

						return;
					}
				}
				else
				if(!_stricmp(arg2, "rotate") && 6 <= argc)
				{
					char const * arg3 = args->ArgV(3);
					char const * arg4 = args->ArgV(4);
					char const * arg5 = args->ArgV(5);

					g_Hook_VClient_RenderView.m_CamPath.Rotate(
						atof(arg3),
						atof(arg4),
						atof(arg5)
					);
					return;
				}
				else
				if(!_stricmp(arg2, "interp"))
				{
					if(4 <= argc)
					{
						char const * arg3 = args->ArgV(3);

						if(!_stricmp(arg3, "position"))
						{
							if(5 <= argc)
							{
								char const * arg4 = args->ArgV(4);
								CamPath::DoubleInterp value;

								if(CamPath::DoubleInterp_FromString(arg4, value))
								{
									g_Hook_VClient_RenderView.m_CamPath.PositionInterpMethod_set(value);
									return;
								}
							}


							Tier0_Msg("mirv_campath edit interp positon ");
							for(CamPath::DoubleInterp i = CamPath::DI_DEFAULT; i < CamPath::_DI_COUNT; i = (CamPath::DoubleInterp)((int)i +1))
							{
								Tier0_Msg("%s%s", i != CamPath::DI_DEFAULT ? "|": "", CamPath::DoubleInterp_ToString(i));
							}
							Tier0_Msg("\n"
								"Current value: %s\n", CamPath::DoubleInterp_ToString(g_Hook_VClient_RenderView.m_CamPath.PositionInterpMethod_get())
							);
							return;
						}
						else
						if(!_stricmp(arg3, "rotation"))
						{
							if(5 <= argc)
							{
								char const * arg4 = args->ArgV(4);
								CamPath::QuaternionInterp value;

								if(CamPath::QuaternionInterp_FromString(arg4, value))
								{
									g_Hook_VClient_RenderView.m_CamPath.RotationInterpMethod_set(value);
									return;
								}
							}


							Tier0_Msg("mirv_campath edit interp rotation ");
							for(CamPath::QuaternionInterp i = CamPath::QI_DEFAULT; i < CamPath::_QI_COUNT; i = (CamPath::QuaternionInterp)((int)i +1))
							{
								Tier0_Msg("%s%s", i != CamPath::QI_DEFAULT ? "|": "", CamPath::QuaternionInterp_ToString(i));
							}
							Tier0_Msg("\n"
								"Current value: %s\n", CamPath::QuaternionInterp_ToString(g_Hook_VClient_RenderView.m_CamPath.RotationInterpMethod_get())
							);
							return;
						}
						else
						if(!_stricmp(arg3, "fov"))
						{
							if(5 <= argc)
							{
								char const * arg4 = args->ArgV(4);
								CamPath::DoubleInterp value;

								if(CamPath::DoubleInterp_FromString(arg4, value))
								{
									g_Hook_VClient_RenderView.m_CamPath.FovInterpMethod_set(value);
									return;
								}
							}


							Tier0_Msg("mirv_campath edit interp fov ");
							for(CamPath::DoubleInterp i = CamPath::DI_DEFAULT; i < CamPath::_DI_COUNT; i = (CamPath::DoubleInterp)((int)i +1))
							{
								Tier0_Msg("%s%s", i != CamPath::DI_DEFAULT ? "|": "", CamPath::DoubleInterp_ToString(i));
							}
							Tier0_Msg("\n"
								"Current value: %s\n", CamPath::DoubleInterp_ToString(g_Hook_VClient_RenderView.m_CamPath.FovInterpMethod_get())
							);
							return;
						}
					}

					Tier0_Msg(
						"mirv_campath edit interp positon [...]\n"
						"mirv_campath edit interp rotation [...]\n"
						"mirv_campath edit interp fov [...]\n"
					);
					return;
				}
			}

			Tier0_Msg(
				"mirv_campath edit start - Sets current demotime as new start time for the path [or selected keyframes].\n"
				"mirv_campath edit start abs <dValue> - Sets an given floating point value as new start time for the path [or selected keyframes].\n"
				"mirv_campath edit start delta(+|-)<dValue> - Offsets the path [or selected keyframes] by the given <dValue> delta value (Example: \"mirv_campath edit start delta-1.5\" moves the path [or selected keyframes] 1.5 seconds back in time).\n"
				"mirv_campath edit duration <dValue> - set floating point value <dValue> as new duration for the path [or selected keyframes] (in seconds). Please see remarks in HLAE manual.\n"
				"mirv_campath edit position current|(<dX> <dY> <dZ>) - Edit position of the path [or selected keyframes]. The position is applied to the center of the bounding box (\"middle\") of all [or the selected] keyframes, meaning the keyframes are moved releative to that. Current uses the current camera position, otherwise you can give the exact position.\n"
				"mirv_campath edit angles current|(<dPitchY> <dYawZ> <dRollX>) - Edit angles of the path [or selected keyframes]. All keyframes are assigned the same angles. Current uses the current camera angles, otherwise you can give the exact angles.\n"
				"mirv_campath edit fov current|<dFov> - Similar to mirv_campath edit angles, except for field of view (fov).\n"
				"mirv_campath edit rotate <dPitchY> <dYawZ> <dRollX> - Rotate path [or selected keyframes] around the middle of their bounding box by the given angles in degrees.\n"
				"mirv_campath edit interp [...] - Edit interpolation properties.\n"
			);
			return;
		}
		else if(!_stricmp("select", subcmd))
		{	
			if(3 <= argc)
			{
				const char * cmd2 = args->ArgV(2);

				if(!_stricmp(cmd2, "all"))
				{
					g_Hook_VClient_RenderView.m_CamPath.SelectAll();
					return;
				}
				else
				if(!_stricmp(cmd2, "none"))
				{
					g_Hook_VClient_RenderView.m_CamPath.SelectNone();
					return;
				}
				else
				if(!_stricmp(cmd2, "invert"))
				{
					g_Hook_VClient_RenderView.m_CamPath.SelectInvert();
					return;
				}
				else
				{
					bool bOk = true;

					int idx = 2;
					bool add = false;
					if(!_stricmp(cmd2, "add"))
					{
						add = true;
						++idx;
					}

					bool isFromId = false;
					int fromId = 0;
					bool isFromCurrent = false;
					double fromValue = 0.0;
					if(idx < argc)
					{
						const char * fromArg = args->ArgV(idx);

						if(StringBeginsWith(fromArg, "#"))
						{
							isFromId = true;
							++fromArg;
							fromId = atoi(fromArg);
						}
						else if(!_stricmp(fromArg, "current"))
						{
							fromValue = g_Hook_VClient_RenderView.GetCurTime();
						}
						else if(StringBeginsWith(fromArg, "current+"))
						{
							fromValue = g_Hook_VClient_RenderView.GetCurTime();
							fromArg += strlen("current+");
							fromValue += atof(fromArg);
						}
						else if(StringBeginsWith(fromArg, "current-"))
						{
							fromValue = g_Hook_VClient_RenderView.GetCurTime();
							fromArg += strlen("current-");
							fromValue -= atof(fromArg);
						}
						else
						{
							fromValue = atof(fromArg);
						}

						++idx;
					}
					else
						bOk = false;

					bool isToId = false;
					int toId = 0;
					double toValue = 0.0;
					if(idx < argc)
					{
						const char * toArg = args->ArgV(idx);

						if(StringBeginsWith(toArg, "#"))
						{
							isToId = true;
							++toArg;
							toId = atoi(toArg);
						}
						else if(StringBeginsWith(toArg, "current+"))
						{
							toValue = g_Hook_VClient_RenderView.GetCurTime();
							toArg += strlen("current+");
							toValue += atof(toArg);
						}
						else if(StringBeginsWith(toArg, "current-"))
						{
							toValue = g_Hook_VClient_RenderView.GetCurTime();
							toArg += strlen("current-");
							toValue -= atof(toArg);
						}
						else
						{
							toValue = atof(toArg);
						}
						++idx;
					}
					else
						bOk = false;

					bOk = bOk && idx == argc;

					size_t selected = 0;

					if(bOk)
					{
						if(isFromId && isToId)
						{
							if(!add) g_Hook_VClient_RenderView.m_CamPath.SelectNone();
							selected = g_Hook_VClient_RenderView.m_CamPath.SelectAdd((size_t)fromId,(size_t)toId);
						}
						else
						if(!isFromId && isToId)
						{
							if(!add) g_Hook_VClient_RenderView.m_CamPath.SelectNone();
							selected = g_Hook_VClient_RenderView.m_CamPath.SelectAdd((double)fromValue,(size_t)toId);
						}
						else
						if(!isFromId && !isToId)
						{
							if(!add) g_Hook_VClient_RenderView.m_CamPath.SelectNone();
							selected = g_Hook_VClient_RenderView.m_CamPath.SelectAdd((double)fromValue,(double)toValue);
						}
						else bOk = false;
					}

					if(bOk)
					{
						Tier0_Msg("A total of %u keyframes is selected now.\n", selected);
						if(selected < 1)
							Tier0_Warning("WARNING: You have no keyframes selected, thus most operations like mirv_campath clear will think you mean all keyframes (i.e. clear all)!\n", selected);

						return;
					}
				}

			}

			Tier0_Msg(
				"mirv_campath select all - Select all points.\n"
				"mirv_campath select none - Selects no points.\n"
				"mirv_campath select invert - Invert selection.\n"
				"mirv_campath select [add] #<idBegin> #<idEnd> - Select keyframes starting at id <idBegin> and ending at id <idEnd>. If add is given, then selection is added to the current one.\n"
				"mirv_campath select [add] current[(+|-)<dOfsMin>]|<dMin> #<count> - Select keyframes starting at given time and up to <count> number of keyframes. If add is given, then selection is added to the current one.\n"
				"mirv_campath select [add] current[(+|-)<dOfsMin>]|<dMin> current[(+|-)<dOfsMax>]|<dMax> - Select keyframes betwen given star time and given end time . If add is given, then selection is added to the current one.\n"
				"Examples:\n"
				"mirv_campath select current #2 - Select two keyframes starting from current time.\n"
				"mirv_campath select add current #2 - Add two keyframes starting from current time to the current selection.\n"
				"mirv_campath select 64.5 #2 - Select two keyframes starting from time 64.5 seconds.\n"
				"mirv_campath select current-0.5 current+2.5 - Select keyframes between half a second earlier than now and 2.5 seconds later than now.\n"
				"mirv_campath select 128.0 current - Select keyframes between time 128.0 seconds and current time.\n"
				"mirv_campath select add 128.0 current+2.0 - Add keyframes between time 128.0 seconds and 2 seconds later than now to the current selection.\n"
				"Hint: All time values are in game time (in seconds).\n"
			);
			return;

		}
	}

	Tier0_Msg(
		"mirv_campath add - Adds current demotime and view as keyframe\n"
		"mirv_campath enabled 0|1 - Set whether the camera path is active or not. Please note that currently at least 4 Points are required to make it active successfully!\n"
		"mirv_campath draw [...] - Controls drawing of the camera path.\n"
		"mirv_campath clear - Removes all [or all selected] keyframes\n"
		"mirv_campath print - Prints keyframes\n"
		"mirv_campath remove <id> - Removes a keyframe\n"
		"mirv_campath load <fileName> - Loads the campath from the file (XML format)\n"
		"mirv_campath save <fileName> - Saves the campath to the file (XML format)\n"
		"mirv_campath edit [...] - Edit properties of the path [or selected keyframes]\n"
		"mirv_campath select [...] - Keyframe selection.\n"
	);
	return;
}

CON_COMMAND(mirv_camexport, "controls camera motion data export") {
	if(!g_Hook_VClient_RenderView.IsInstalled())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);
		if(0 == _stricmp("stop", arg1))
		{
			g_Hook_VClient_RenderView.ExportEnd();
			return;
		}
		else
		if(0 == _stricmp("start", arg1) && 4 <= argc) {
			char const * fileName = args->ArgV(2);
			double fps = atof(args->ArgV(3));
			if(fps < 0.1f) fps = 0.1f;

			std::wstring wideFileName;
			if(
				!AnsiStringToWideString(fileName, wideFileName)
				|| !g_Hook_VClient_RenderView.ExportBegin(wideFileName.c_str(), 1.0/fps)
			)
				Tier0_Msg("Error: exporting failed.");

			return;
		}
		if(0 == _stricmp("timeinfo", arg1))
		{			
			Tier0_Msg("Current (interpolated client) time: %f\n", g_Hook_VClient_RenderView.GetCurTime());
			return;
		}

	}

	Tier0_Msg(
		"Usage:\n"
		"mirv_camexport start <filename> <fps>\n"
		"mirv_camexport stop\n"
		"mirv_camexport timeinfo\n"
	);
}

CON_COMMAND(mirv_camimport, "controls camera motion data import") {
	if(!g_Hook_VClient_RenderView.IsInstalled())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);
		if(0 == _stricmp("stop", arg1)) {
			g_Hook_VClient_RenderView.ImportEnd();
			return;
		}
		else
		if(0 == _stricmp("start", arg1) && 3 <= argc) {
			char const * fileName = args->ArgV(2);
			g_Hook_VClient_RenderView.SetImportBaseTime(g_Hook_VClient_RenderView.GetCurTime());

			std::wstring wideFileName;
			if(!AnsiStringToWideString(fileName, wideFileName)
				|| !g_Hook_VClient_RenderView.ImportBegin(wideFileName.c_str())
			)
				Tier0_Msg("Loading failed.");
			return;
		}
		else
		if(0 == _stricmp("basetime", arg1)) {
			if(3 <= argc) {
				char const * newTime = args->ArgV(2);
				if(0 == _stricmp("current", newTime))
					g_Hook_VClient_RenderView.SetImportBaseTime(g_Hook_VClient_RenderView.GetCurTime());
				else
					g_Hook_VClient_RenderView.SetImportBaseTime((float)atof(newTime));
				return;
			}

			Tier0_Msg(
				"Usage:\n"
				"mirv_camimport basetime <newTime> | current\n"
				"Current setting: %f\n",
				g_Hook_VClient_RenderView.GetImportBasteTime()
			);
			return;
		}
		else
		if(0 == _stricmp("toCamPath", arg1) && 4 <= argc)
		{
			char const * arg2 = args->ArgV(2);
			char const * arg3 = args->ArgV(3);

			if(!g_Hook_VClient_RenderView.ImportToCamPath(0 != atoi(arg2), atof(arg3)))
				Tier0_Warning("ERROR: Something went wrong when converting to mirv_campath!\n");
			return;
		}
	}
	Tier0_Msg(
		"Usage:\n"
		"mirv_camimport start <filename>\n"
		"mirv_camimport stop\n"
		"mirv_camimport basetime [...]\n"
		"mirv_camimport toCamPath 0|1 <fov> - Convert to mirv_campath, 0 = no interp adjust | 1 (recommended) = adjust campath interpolation (to linear), <fov> Field Of View value to use (90 recommended).\n"
	);
}

CON_COMMAND(mirv_cvar_unhide_all,"(CS:GO only) removes hidden and development only flags from all cvars.")
{
	SOURCESDK::ICvar_007 * pCvar = WrpConCommands::GetVEngineCvar007();
	if(!pCvar)
	{
		Tier0_Warning("Error: No suitable Cvar interface found.\n");
		return;
	}

	SOURCESDK::ICvar_007::Iterator iter(pCvar);

	int nUnhidden = 0;

	for(iter.SetFirst(); iter.IsValid(); iter.Next())
	{
		SOURCESDK::ConCommandBase_007 * cmd = iter.Get();

		if(cmd->IsFlagSet(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN))
			nUnhidden++;

		cmd->RemoveFlags(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
	}

	Tier0_Msg("Removed FCVAR_DEVELOPMENTONLY or FCVAR_HIDDEN from %i ConVars.\n", nUnhidden);
}

CON_COMMAND(mirv_cvar_hack, "")
{
	int argc = args->ArgC();

	if(3 <= argc)
	{
		char const * cvarName = args->ArgV(1);
		float cvarValue = (float)atof(args->ArgV(2));

		WrpConVarRef * cvar = new WrpConVarRef(cvarName);

		cvar->SetDirectHack(cvarValue);

		delete cvar;

		return;
	}

	Tier0_Msg(
		"mirv_cvar_hack <cvarName> <floatValue> - Force value directly, this will only work for true cvars (not cmds) and only for a subset of those, might have unwanted side effects - use with caution!\n"
	);
}

CON_COMMAND(mirv_deathmsg, "controls death notification options")
{
	if(!csgo_CHudDeathNotice_Install())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(0 == _stricmp("debug", arg1))
		{
			if(3 <= argc)
			{
				csgo_debug_CHudDeathNotice_FireGameEvent = atoi(args->ArgV(2));
				return;
			}
			Tier0_Msg(
				"Usage:\n"
				"mirv_deathmsg debug 0|1|2 - disbale/enable printing of ids of current death notices.\n"
				"Current setting: %i\n",
				csgo_debug_CHudDeathNotice_FireGameEvent
			);
			return;
		}
		else
		if(0 == _stricmp("block", arg1))
		{
			if(4 <= argc)
			{
				char const * uidAttacker = args->ArgV(2);
				char const * uidVictim = args->ArgV(3);
				char const * uidAssister = 5 <= argc ? args->ArgV(4) : "*";

				csgo_CHudDeathNotice_Block(uidAttacker, uidVictim, uidAssister, -1);

				return;
			}
			else
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("list", arg2))
				{
					csgo_CHudDeathNotice_Block_List();
					return;
				}
				else
				if(0 == _stricmp("clear", arg2))
				{
					csgo_CHudDeathNotice_Block_Clear();
					return;
				}
				
			}
			Tier0_Msg(
				"Usage:\n"
				"mirv_deathmsg block <uidAttacker> <uidVictim> - block these ids\n"
				"\tRemarks: * to match any uid, use !x to match any uid apart from x.\n"
				"mirv_deathmsg block <uidAttacker> <uidVictim> <uidAssister> - block these ids\n"
				"\tRemarks: * to match any uid, use !x to match any uid apart from x.\n"
				"mirv_deathmsg block list - list current blocks\n"
				"mirv_deathmsg block clear - clear current blocks\n"
				"(Use mirv_deathmsg debug 1 to get the uids.)\n"
			);
			return;
		}
		else
		if(0 == _stricmp("modTime", arg1))
		{
			if(5 <= argc)
			{
				char const * uidAttacker = args->ArgV(2);
				char const * uidVictim = args->ArgV(3);
				char const * uidAssister = 6 <= argc ? args->ArgV(4) : "*";
				float modTime = (float)atof(
					6 <= argc ? args->ArgV(5) : args->ArgV(4)
					);

				csgo_CHudDeathNotice_Block(uidAttacker, uidVictim, uidAssister, modTime);

				return;
			}
			else
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("list", arg2))
				{
					csgo_CHudDeathNotice_Block_List();
					return;
				}
				else
				if(0 == _stricmp("clear", arg2))
				{
					csgo_CHudDeathNotice_Block_Clear();
					return;
				}
				
			}
			Tier0_Msg(
				"Usage:\n"
				"mirv_deathmsg modTime <uidAttacker> <uidVictim> <modTime> - use <modTime> multiplier for matche ids\n"
				"\tRemarks: * to match any uid, use !x to match any uid apart from x.\n"
				"mirv_deathmsg modTime <uidAttacker> <uidVictim> <uidAssister> <modTime> - use <modTime> multiplier for matche ids\n"
				"\tRemarks: * to match any uid, use !x to match any uid apart from x.\n"
				"mirv_deathmsg modTime list - list current modifiers\n"
				"mirv_deathmsg modTime clear - clear current modifiers\n"
				"(Use mirv_deathmsg debug 1 to get the uids.)\n"
			);
			return;
		}
		else
		if(0 == _stricmp("highLightId", arg1))
		{
			if(3 <= argc)
			{
				csgo_CHudDeathNotice_HighLightId = atoi(args->ArgV(2));
				return;
			}
			Tier0_Msg(
				"Usage:\n"
				"mirv_deathmsg highLightId -1|0|<id> - -1 is default behaviour, 0 is never highlight, otherwise <id> is the ID (you can get it from mirv_deathmsg debug) of the player you want to highlight.\n"
				"Current setting: %i\n",
				csgo_CHudDeathNotice_HighLightId
			);
			return;
		}
		else
		if(0 == _stricmp("highLightAssists", arg1))
		{
			if(3 <= argc)
			{
				csgo_CHudDeathNotice_HighLightAssists = 0 != atoi(args->ArgV(2));
				return;
			}
			Tier0_Msg(
				"Usage:\n"
				"mirv_deathmsg highLightAssists 0|1 - 0 is don't highlight asissts, 1 is highlight assists.\n"
				"Current setting: %s\n",
				csgo_CHudDeathNotice_HighLightAssists ? "1" : "0"
			);
			return;
		}
		else
		if(0 == _stricmp("cfg", arg1))
		{
			if(4 <= argc)
			{
				char const * arg2 = args->ArgV(2);
				float arg3 = (float)atof(args->ArgV(3));

				if(0 == _stricmp("scrollInTime", arg2))
				{
					csgo_CHudDeathNotice_nScrollInTime = arg3;
					return;
				}
				else
				if(0 == _stricmp("fadeOutTime", arg2))
				{
					csgo_CHudDeathNotice_nFadeOutTime = arg3;
					return;
				}
				else
				if(0 == _stricmp("noticeLifeTime", arg2))
				{
					csgo_CHudDeathNotice_nNoticeLifeTime = arg3;
					return;
				}
				else
				if(0 == _stricmp("localPlayerLifeTimeMod", arg2))
				{
					csgo_CHudDeathNotice_nLocalPlayerLifeTimeMod = arg3;
					return;
				}
			}
			Tier0_Msg(
				"Usage:\n"
				"mirv_deathmsg cfg scrollInTime f - Current: %f\n"
				"mirv_deathmsg cfg fadeOutTime f - Current: %f\n"
				"mirv_deathmsg cfg noticeLifeTime f - This is what you want. Current: %f\n"
				"mirv_deathmsg cfg localPlayerLifeTimeMod f - Current: %f\n"
				"Where f is a floating point value in seconds. Use -1 (a negative value) to use the orginal value instead.\n",
				csgo_CHudDeathNotice_nScrollInTime,
				csgo_CHudDeathNotice_nFadeOutTime,
				csgo_CHudDeathNotice_nNoticeLifeTime,
				csgo_CHudDeathNotice_nLocalPlayerLifeTimeMod
			);
			return;
		}
		else if (0 == _stricmp("fake", arg1))
		{
			if (5 <= argc)
			{
				Console_csgo_CHudDeathNotice_Fake(
					args->ArgV(2),
					0 != atoi(args->ArgV(3)),
					0 != atoi(args->ArgV(4))
				);

				return;
			}

			Tier0_Msg(
				"mirv_deathmsg fake <htmlString> <bIsVictim> <bIsKiller> - Fake a death message (mirv_deathmsg command needs to be entered once (so hooks get installed) and after that one deathmsg has to be drawn earlier already (so it gets the object pointer) for this to work!).\n"
				"\t<htmlString> - String with HTML, i.e. \"\".\n"
				"\t<bIsVictim> - can be 0 or 1 (relevant for POV demo style / local player, for GOTV fakes you usually want 0 here)\n"
				"\t<bIsKiller> - can be 0 or 1 (relevant for POV demo style / local player, for GOTV fakes you usually want 0 here)\n"
				"Example: mirv_deathmsg fake \"<font color='#eabe54'>Cloud9 Stewie2K</font><font color='#eabe54'></font> <img src='icon-ak47.png' height='16'/><font color='#6f9ce6'>Na`Vi Zeus</font>\" 0 0\n"
			);
			return;
		}
	}
	Tier0_Msg(
		"Usage:\n"
		"mirv_deathmsg block [...] - block specific death messages.\n"
		"mirv_deathmsg cfg [...] - configure death message properties, i.e. noticeLifeTime.\n"
		"mirv_deathmsg debug [...] - enable debug message in cosnole (allows finding player IDs).\n"
		"mirv_deathmsg highLightId [...] - control highlighting.\n"
		"mirv_deathmsg highLightAssists [...] - Whether to highlight assists for highLightId.\n"
		"mirv_deathmsg modTime [...] - allows to set a display time multiplier for a message.\n"
		"mirv_deathmsg fake [...] - Fake a death message.\n"
	);
}

CON_COMMAND(mirv_fov,"allows overriding FOV (Filed Of View) of the camera")
{
	if(!g_Hook_VClient_RenderView.IsInstalled())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(!_stricmp(arg1,"default"))
		{
			g_Hook_VClient_RenderView.FovDefault();
			return;
		}
		else
		if(!_stricmp(arg1,"handleZoom"))
		{
			if(3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if(!_stricmp(arg2, "enabled"))
				{
					if(4 <= argc)
					{
						const char * arg3 = args->ArgV(3);

						g_Hook_VClient_RenderView.handleZoomEnabled = 0 != atoi(arg3);
						return;
					}

					Tier0_Msg(
						"Usage:\n"
						"mirv_fov handleZoom enabled 0|1 - Enable (1), disable (0).\n"
						"Current value: %s\n",
						g_Hook_VClient_RenderView.handleZoomEnabled ? "1" : "0"
					);
					return;
				}
				else
				if(!_stricmp(arg2, "minUnzoomedFov"))
				{
					if(4 <= argc)
					{
						const char * arg3 = args->ArgV(3);

						if(!_stricmp(arg3, "current"))
						{
							g_Hook_VClient_RenderView.handleZoomMinUnzoomedFov = g_Hook_VClient_RenderView.LastCameraFov;
							return;
						}
						else
						if(StringIsEmpty(arg3) || !StringIsAlphas(arg3))
						{
							g_Hook_VClient_RenderView.handleZoomMinUnzoomedFov = atof(arg3);
							return;
						}
					}

					Tier0_Msg(
						"Usage:\n"
						"mirv_fov handleZoom minUnzoomedFov current - Set current fov as threshold.\n"
						"mirv_fov handleZoom minUnzoomedFov <f> - Set floating point value <f> as threshold.\n"
						"Current value: %f\n",
						g_Hook_VClient_RenderView.handleZoomMinUnzoomedFov
					);
					return;
				}
			}

			Tier0_Msg(
				"Usage:\n"
				"mirv_fov handleZoom enabled [...] - Whether to enable zoom handling (if enabled mirv_fov is only active if it's not bellow minUnzoomedFov (not zoomed)).\n"
				"mirv_fov handleZoom minUnzoomedFov [...] - Zoom detection threshold.\n"
			);
			return;
		}
		else
		if(StringIsEmpty(arg1) || !StringIsAlphas(arg1))
		{
			g_Hook_VClient_RenderView.FovOverride(atof(arg1));
			return;
		}
	}

	Tier0_Msg(
		"Usage:\n"
		"mirv_fov <f> - Override fov with given floating point value <f>.\n"
		"mirv_fov default - Revert to the game's default behaviour.\n"
		"mirv_fov handleZoom [...] - Handle zooming (i.e. AWP in CS:GO).\n"
	);
	{
		Tier0_Msg("Current value: ");

		double fovValue;
		if(!g_Hook_VClient_RenderView.GetFovOverride(fovValue))
			Tier0_Msg("default (currently: %f)\n", g_Hook_VClient_RenderView.LastCameraFov);
		else
			Tier0_Msg("%f\n", fovValue);
	}
}

CON_COMMAND(mirv_replace_name, "allows replacing player names")
{
	if(!csgo_GetPlayerName_Install())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(0 == _stricmp("list", arg1))
		{
			csgo_GetPlayerName_Replace_List();
			return;
		}
		else
		if(0 == _stricmp("clear", arg1))
		{
			csgo_GetPlayerName_Replace_Clear();
			return;
		}

		if(3 <= argc)
		{
			const char * arg2 = args->ArgV(2);

			if(0 == _stricmp("debug", arg1))
			{
				csgo_debug_GetPlayerName = 0 != atoi(arg2);
				return;
			}
			else
			if(0 == _stricmp("delete", arg1))
			{
				csgo_GetPlayerName_Replace_Delete(arg2);
				return;
			}

			if(1 <= strlen(arg1) && isdigit(arg1[0]))
			{
				csgo_GetPlayerName_Replace(arg1, arg2);
				return;
			}
		}
	}

	Tier0_Msg(
		"Usage:\n"
		"mirv_replace_name <playerId> <name> - replace <playerId> with given <name>.\n"
		"mirv_replace_name debug 0|1 - print <playerId> -> <name> pairs into console as they get queried by the game.\n"
		"mirv_replace_name delete <playerId> - delete replacement for <playerId>.\n"
		"mirv_replace_name list - list all <playerId> -> <name> replacements currently active.\n"
		"mirv_replace_name clear - clear all replacements.\n"
	);
}

CON_COMMAND(mirv_input, "Input mode configuration.")
{
	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(0 == _stricmp("camera", arg1))
		{
			g_AfxHookSourceInput.SetCameraControlMode(true);
			return;
		}
		else
		if(0 == _stricmp("cfg", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("msens", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.SetMouseSensitivity(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.GetMouseSensitivty());
					return;
				}
				else
				if(0 == _stricmp("ksens", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.SetKeyboardSensitivity(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.GetKeyboardSensitivty());
					return;
				}
				else
				if(0 == _stricmp("kForwardSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardForwardSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardForwardSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kBackwardSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardBackwardSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardBackwardSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kLeftSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardLeftSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardLeftSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kRightSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardRightSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardRightSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kUpSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardUpSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardUpSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kDownSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardDownSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardDownSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kPitchPositiveSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardPitchPositiveSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardPitchPositiveSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kPitchNegativeSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardPitchNegativeSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardPitchNegativeSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kYawPositiveSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardYawPositiveSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardYawPositiveSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kYawNegativeSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardYawNegativeSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardYawNegativeSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kRollPositiveSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardRollPositiveSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardRollPositiveSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kRollNegativeSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardRollNegativeSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardRollNegativeSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kFovPositiveSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardFovPositiveSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardFovPositiveSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("kFovNegativeSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.KeyboardFovNegativeSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.KeyboardFovNegativeSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("mYawSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.MouseYawSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.MouseYawSpeed_get());
					return;
				}
				else
				if(0 == _stricmp("mPitchSpeed", arg2))
				{
					if(4 <= argc)
					{
						double value = atof(args->ArgV(3));
						g_AfxHookSourceInput.MousePitchSpeed_set(value);
						return;
					}
					Tier0_Msg("Value: %f", g_AfxHookSourceInput.MousePitchSpeed_get());
					return;
				}
			}

			Tier0_Msg(
				"Usage:\n"
				"mirv_input cfg msens - Get mouse sensitiviy.\n"
				"mirv_input cfg msens <dValue> - Set mouse sensitiviy.\n"
				"mirv_input cfg ksens - Get keyboard sensitivity.\n"
				"mirv_input cfg ksens <dValue> - Set keyboard sensitivity.\n"
				"mirv_input cfg kForwardSpeed - Get value.\n"
				"mirv_input cfg kForwardSpeed <dValue> - Set value.\n"
				"mirv_input cfg kBackwardSpeed - Get value.\n"
				"mirv_input cfg kBackwardSpeed <dValue> - Set value.\n"
				"mirv_input cfg kLeftSpeed - Get value.\n"
				"mirv_input cfg kLeftSpeed <dValue> - Set value.\n"
				"mirv_input cfg kRightSpeed - Get value.\n"
				"mirv_input cfg kRightSpeed <dValue> - Set value.\n"
				"mirv_input cfg kUpSpeed - Get value.\n"
				"mirv_input cfg kUpSpeed <dValue> - Set value.\n"
				"mirv_input cfg kDownSpeed - Get value.\n"
				"mirv_input cfg kDownSpeed <dValue> - Set value.\n"
				"mirv_input cfg kPitchPositiveSpeed - Get value.\n"
				"mirv_input cfg kPitchPositiveSpeed <dValue> - Set value.\n"
				"mirv_input cfg kPitchNegativeSpeed - Get value.\n"
				"mirv_input cfg kPitchNegativeSpeed <dValue> - Set value.\n"
				"mirv_input cfg kYawPositiveSpeed - Get value.\n"
				"mirv_input cfg kYawPositiveSpeed <dValue> - Set value.\n"
				"mirv_input cfg kYawNegativeSpeed - Get value.\n"
				"mirv_input cfg kYawNegativeSpeed <dValue> - Set value.\n"
				"mirv_input cfg kRollPositiveSpeed - Get value.\n"
				"mirv_input cfg kRollPositiveSpeed <dValue> - Set value.\n"
				"mirv_input cfg kRollNegativeSpeed - Get value.\n"
				"mirv_input cfg kRollNegativeSpeed <dValue> - Set value.\n"
				"mirv_input cfg kFovPositiveSpeed - Get value.\n"
				"mirv_input cfg kFovPositiveSpeed <dValue> - Set value.\n"
				"mirv_input cfg kFovNegativeSpeed - Get value.\n"
				"mirv_input cfg kFovNegativeSpeed <dValue> - Set value.\n"
				"mirv_input cfg mYawSpeed - Get value.\n"
				"mirv_input cfg mYawSpeed <dValue> - Set value.\n"
				"mirv_input cfg mPitchSpeed - Get value.\n"
				"mirv_input cfg mPitchSpeed <dValue> - Set value.\n"
			);
			return;
		}
		else
		if(0 == _stricmp("end", arg1))
		{
			g_AfxHookSourceInput.SetCameraControlMode(false);
			return;
		}
		else
		if(0 == _stricmp("position", arg1))
		{
			if(5 == argc)
			{
				char const * arg2 = args->ArgV(2);
				char const * arg3 = args->ArgV(3);
				char const * arg4 = args->ArgV(4);
	
				g_Hook_VClient_RenderView.LastCameraOrigin[0] = atof(arg2);
				g_Hook_VClient_RenderView.LastCameraOrigin[1] = atof(arg3);
				g_Hook_VClient_RenderView.LastCameraOrigin[2] = atof(arg4);
				return;
			}

			Tier0_Msg(
				"mirv_input positon <x> <y> <z> - Set new position (only useful in camera input mode).\n"
				"Current value: %f %f %f\n"
				, g_Hook_VClient_RenderView.LastCameraOrigin[0]
				, g_Hook_VClient_RenderView.LastCameraOrigin[1]
				, g_Hook_VClient_RenderView.LastCameraOrigin[2]
			);
			return;
		}
		else
		if(0 == _stricmp("angles", arg1))
		{
			if(5 == argc)
			{
				char const * arg2 = args->ArgV(2);
				char const * arg3 = args->ArgV(3);
				char const * arg4 = args->ArgV(4);
	
				g_Hook_VClient_RenderView.LastCameraAngles[0] = atof(arg2);
				g_Hook_VClient_RenderView.LastCameraAngles[1] = atof(arg3);
				g_Hook_VClient_RenderView.LastCameraAngles[2] = atof(arg4);
				return;
			}

			Tier0_Msg(
				"mirv_input angles <yPitch> <xRoll> <zYaw> - Set new angles (only useful in camera input mode).\n"
				"Current value: %f %f %f\n"
				, g_Hook_VClient_RenderView.LastCameraAngles[0]
				, g_Hook_VClient_RenderView.LastCameraAngles[1]
				, g_Hook_VClient_RenderView.LastCameraAngles[2]
			);
			return;
		}
		else
		if(0 == _stricmp("fov", arg1))
		{
			if(3 == argc)
			{
				char const * arg2 = args->ArgV(2);
	
				g_Hook_VClient_RenderView.LastCameraFov = atof(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_input fov <fov> - Set new fov (only useful in camera input mode).\n"
				"Current value: %f\n"
				, g_Hook_VClient_RenderView.LastCameraFov
			);
			return;
		}
		else if (0 == _stricmp("mem", arg1))
		{
			CSubWrpCommandArgs subArgs(args, 2);

			g_MirvInputMem.Console(&subArgs);

			return;
		}
	}

	Tier0_Msg(
		"Usage:\n"
		"mirv_input camera - Enable camera input mode, see HLAE manual for keys etc.\n"
		"mirv_input cfg [...] - Control input mode configuration.\n"
		"mirv_input end - End input mode(s).\n"
		"mirv_input positon [...]\n"
		"mirv_input angles [...]\n"
		"mirv_input fov [...]\n"
		"mirv_input mem [...] - Store, use, save and load mirv_input view states.\n"
	);
}

CON_COMMAND(mirv_snd_timescale, "(CS:GO only) allows to override host_timescale value for sound system.")
{
	if(!Hook_csgo_SndMixTimeScalePatch())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if(0 == _stricmp("default", arg1))
		{
			csgo_SndMixTimeScalePatch_enable = false;
			return;
		}
		else
		{
			csgo_SndMixTimeScalePatch_enable = true;
			csgo_SndMixTimeScalePatch_value = (float)atof(arg1);
			return;
		}
	}
	Tier0_Msg(
		"Usage:\n"
		"mirv_snd_timescale <fValue> - override sound system host_timescale value with floating point value <fValue>.\n"
		"mirv_snd_timescale default - don't override.\n"
	);
}

CON_COMMAND(mirv_gameoverlay, "GameOverlayRenderer control.")
{
	int argc = args->ArgC();

	if(3 <= argc)
	{
		const char * arg1 = args->ArgV(1);

		if( 0 == _stricmp("enabled", arg1)
			|| 0 == _stricmp("enable", arg1))
		{
			bool value = 0 != atoi(args->ArgV(2));
			Tier0_Msg(
				"%s %s.\n",
				value ? "Enable" : "Disable",
				GameOverlay_Enable(value) ? "OK" : "FAILED"
			);
			return;
		}
	}

	Tier0_Msg(
		"Usage:\n"
		"mirv_gameoverlay enabled 0|1 - Disable/Enable the GameOverlay (will only do s.th. useful when it was enabled initally).\n"
	);
}

CON_COMMAND(mirv_snd_filter, "Sound control (i.e. blocking sounds).")
{
	if(!csgo_S_StartSound_Install())
	{
		Tier0_Warning("Error: Hook not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(0 == _stricmp("block", arg1))
		{
			if(3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				csgo_S_StartSound_Block_Add(arg2);
				return;
			}
		}
		else
		if(0 == _stricmp("print", arg1))
		{
			csgo_S_StartSound_Block_Print();
			return;
		}
		else
		if(0 == _stricmp("remove", arg1))
		{
			if(3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				csgo_S_StartSound_Block_Remove(atoi(arg2));
				return;
			}
		}
		else
		if(0 == _stricmp("clear", arg1))
		{
			csgo_S_StartSound_Block_Clear();
			return;
		}
		else
		if(0 == _stricmp("debug", arg1))
		{
			if(3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				g_csgo_S_StartSound_Debug = 0 != atoi(arg2);
				return;
			}
			
			Tier0_Msg("Current value: %s\n", g_csgo_S_StartSound_Debug ? "1" : "0");
			return;
		}
	}

	Tier0_Msg(
		"Usage:\n"
		"mirv_snd_filter block <mask> - Blocks given <mask> string (for format see bellow).\n"
		"mirv_snd_filter print - Prints current blocks.\n"
		"mirv_snd_filter remove <index> - Removes the block with index <index> (You can get that from the print sub-command).\n"
		"mirv_snd_filter clear - Clears all blocks.\n"
		"mirv_snd_filter debug 0|1 - Print sounds played into console.\n"
		"<mask> - string to match, where \\* = wildcard and \\\\ = \\\n"
	);
}

CON_COMMAND(mirv_listentities, "Print info about currently active entites. (CS:GO only)")
{
	if(!SOURCESDK::g_Entitylist_csgo)
	{
		Tier0_Warning("Not supported for your engine!\n");
		return;
	}

	Tier0_Msg(
		"index (distance): className::enitityName playerName :entityHandle\n"
	);

	Vector3 cameraOrigin(
		g_Hook_VClient_RenderView.LastCameraOrigin[0],
		g_Hook_VClient_RenderView.LastCameraOrigin[1],
		g_Hook_VClient_RenderView.LastCameraOrigin[2]
	);

	int imax = SOURCESDK::g_Entitylist_csgo->GetHighestEntityIndex();

	for(int i=0; i<= imax; ++i)
	{
		SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(i);
		SOURCESDK::C_BaseEntity_csgo * be = ce ? ce->GetBaseEntity() : 0;

		if(be)
		{
			SOURCESDK::Vector vEntOrigin = be->GetAbsOrigin();
			Vector3 entOrigin(vEntOrigin.x, vEntOrigin.y, vEntOrigin.z);

			double dist = (entOrigin -cameraOrigin).Length();

			// This won't work, because Valve doesn't overload the const function:
			//char const * playerName = be->GetPlayerName();
			//if(!playerName) playerName = "(null)";

			char const * playerName = "[n/a]";

			Tier0_Msg(
				"%i (%f): %s::%s %s :%i\n"
				, i
				, dist
				, be->GetClassname()
				, be->GetEntityName()
				, playerName
				, be->GetRefEHandle().ToInt()
				);
		}
	}
}

CON_COMMAND(mirv_aim, "Aiming system control.")
{
	if(!g_Hook_VClient_RenderView.GetGlobals())
	{
		Tier0_Warning("Error: Required hooks missing.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if(0 == _stricmp("active", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.Active = 0 != atoi(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim active 0|1 - Whether aiming is active (1) or not (0).\n"
				"Current value: %s\n",
				g_Aiming.Active ? "1" : "0"
			);
			return;
		}
		else
		if(0 == _stricmp("softDeactivate", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.SoftDeactivate = 0 != atoi(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim softDeactivate 0|1 - Wheter to support soft deactivation (1) or not (0).\n"
				"Current value: %s\n",
				g_Aiming.SoftDeactivate ? "1" : "0"
			);
			return;
		}
		else
		if(0 == _stricmp("entityIndex", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.EntityIndex = atoi(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim entityIndex <n> - Entity index to aim after (use mirv_listentities to get one). Use invalid index (i.e. -1) to deactivate re-targeting.\n"
				"Current value: %i\n",
				g_Aiming.EntityIndex
			);
			return;
		}
		else
		if(0 == _stricmp("point", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("cam", arg2))
				{
					Vector3 offset(0, 0, 0);

					if(6 <= argc)
					{
						char const * arg3 = args->ArgV(3);
						char const * arg4 = args->ArgV(4);
						char const * arg5 = args->ArgV(5);

						offset.X = atof(arg3);
						offset.Y = atof(arg4);
						offset.Z = atof(arg5);
					}

					g_Aiming.TargetPoint(
						Vector3(g_Hook_VClient_RenderView.LastCameraOrigin[0],
							g_Hook_VClient_RenderView.LastCameraOrigin[1],
							g_Hook_VClient_RenderView.LastCameraOrigin[2])
						+ offset);
					return;
				}
				else
				if(0 == _stricmp("last", arg2))
				{
					Vector3 offset(0, 0, 0);

					if(6 <= argc)
					{
						char const * arg3 = args->ArgV(3);
						char const * arg4 = args->ArgV(4);
						char const * arg5 = args->ArgV(5);

						offset.X = atof(arg3);
						offset.Y = atof(arg4);
						offset.Z = atof(arg5);
					}

					g_Aiming.TargetPointFromLast(offset);
					return;
				}
				else
				if(0 == _stricmp("abs", arg2) && 6 <= argc)
				{
					Vector3 offset(0, 0, 0);

					{
						char const * arg3 = args->ArgV(3);
						char const * arg4 = args->ArgV(4);
						char const * arg5 = args->ArgV(5);

						offset.X = atof(arg3);
						offset.Y = atof(arg4);
						offset.Z = atof(arg5);
					}

					g_Aiming.TargetPoint(offset);
					return;
				}
			}

			Tier0_Msg(
				"mirv_aim point abs <fX> <fY> <fz> - Absolute point in world coordinates.\n"
				"mirv_aim point cam [<fOffsetX> <fOffsetY> <fOffsetZ>] - Aim at current camera position with optional offset in world coordinates.\n"
				"mirv_aim point last [<fOffsetX> <fOffsetY> <fOffsetZ>] - Aim at last target position with optional offset in world coordinates.\n"
				"Current value: %i\n",
				g_Aiming.EntityIndex
			);
			return;
		}
		else
		if(0 == _stricmp("offset", arg1))
		{
			if(5 <= argc)
			{
				char const * arg2 = args->ArgV(2);
				char const * arg3 = args->ArgV(3);
				char const * arg4 = args->ArgV(4);

				g_Aiming.OffSet = Vector3(
					atof(arg2), atof(arg3), atof(arg4)
				);
				return;
			}

			Tier0_Msg(
				"mirv_aim offset <x> <y> <z> - Offset in target local space to aim at (3 floating point values).\n"
				"Current value: %f %f %f\n",
				g_Aiming.OffSet.X, g_Aiming.OffSet.Y, g_Aiming.OffSet.Z
			);
			return;
		}
		else
		if(0 == _stricmp("snapTo", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.SnapTo = 0 != atoi(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim snapTo 0|1 - Wheter to aim non-soft (1) or not (0)."
				"Current value: %s\n",
				g_Aiming.SnapTo ? "1" : "0"
			);
			return;
		}
		else
		if(0 == _stricmp("velLimit", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.LimitVelocity = atof(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim velLimit <fValue> - Max velocity (in angle degrees per second^1) possible (floating point value).\n"
				"Current value: %f\n",
				g_Aiming.LimitVelocity
			);
			return;
		}
		else
		if(0 == _stricmp("accelLimit", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.LimitAcceleration = atof(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim accelLimit <fValue> - Max acceleration (in angle degrees per second^2) possible (floating point value).\n"
				"Current value: %f\n",
				g_Aiming.LimitAcceleration
			);
			return;
		}
/*
		else
		if(0 == _stricmp("jerkLimit", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				g_Aiming.LimitJerk = atof(arg2);
				return;
			}

			Tier0_Msg(
				"mirv_aim jerkLimit <fValue> - Max jerk (in angle degrees per second^3) possible (floating point value).\n"
				"Current value: %f\n",
				g_Aiming.LimitJerk
			);
			return;
		}
*/
		else
		if(0 == _stricmp("origin", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("net", arg2))
				{
					g_Aiming.Origin = Aiming::O_Net;
					return;
				}
				else
				if(0 == _stricmp("view", arg2))
				{
					g_Aiming.Origin = Aiming::O_View;
					return;
				}
			}

			char const * curValue = "[unknown]";
			switch(g_Aiming.Origin)
			{
			case Aiming::O_Net:
				curValue = "net";
				break;
			case Aiming::O_View:
				curValue = "view";
				break;
			}

			Tier0_Msg(
				"mirv_aim origin net|view - Target origin to use.\n"
				"Current value: %s\n",
				curValue
			);
			return;
		}
		else
		if(0 == _stricmp("angles", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("net", arg2))
				{
					g_Aiming.Angles = Aiming::A_Net;
					return;
				}
				else
				if(0 == _stricmp("view", arg2))
				{
					g_Aiming.Angles = Aiming::A_View;
					return;
				}
			}

			char const * curValue = "[unknown]";
			switch(g_Aiming.Angles)
			{
			case Aiming::A_Net:
				curValue = "net";
				break;
			case Aiming::A_View:
				curValue = "view";
				break;
			}

			Tier0_Msg(
				"mirv_aim angles net|view - Target angles to use.\n"
				"Current value: %s\n",
				curValue
			);
			return;
		}
		else
		if(0 == _stricmp("up", arg1))
		{
			if(3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if(0 == _stricmp("input", arg2))
				{
					g_Aiming.Up = Aiming::U_Input;
					return;
				}
				else
				if(0 == _stricmp("world", arg2))
				{
					g_Aiming.Up = Aiming::U_World;
					return;
				}
			}

			char const * curValue = "[unknown]";
			switch(g_Aiming.Up)
			{
			case Aiming::U_Input:
				curValue = "input";
				break;
			case Aiming::U_World:
				curValue = "world";
				break;
			}

			Tier0_Msg(
				"mirv_aim up input|world - Camera up direction to use.\n"
				"Current value: %s\n",
				curValue
			);
			return;
		}
	}

	Tier0_Msg(
		"mirv_aim active [...] - Whether aiming is active.\n"
		"mirv_aim softDeactivate [...] - Wheter to support soft deactivation (for snapTo 0).\n"
		"mirv_aim entityIndex [...] - Entity index to aim after (use mirv_listentities to get one).\n"
		"mirv_aim point [...] - Point to aim after.\n"
		"mirv_aim origin [...] - Target origin to use.\n"
		"mirv_aim angles [...] - Target angles to use.\n"
		"mirv_aim offset [...] - Offset in target space to aim at.\n"
		"mirv_aim up [...] - How to determine the camera up direction.\n"
		"mirv_aim snapTo [...] - Wheter to aim non-soft or soft.\n"
		"mirv_aim velLimit [...] - Max velocity possible (for snapTo 0).\n"
		"mirv_aim accelLimit [...] - Max acceleration possible (for snapTo 0).\n"
		// does not work atm // "mirv_aim jerkLimit [...] - Max jerk possible (for snapTo 0).\n"
	);
}

CON_COMMAND(mirv_cmd, "Command system (for scheduling commands).")
{
	if(!g_Hook_VClient_RenderView.IsInstalled() || !g_VEngineClient)
	{
		Tier0_Warning("Error: Required hooks not installed.\n");
		return;
	}

	int argc = args->ArgC();

	if(2 <= argc)
	{
		char const * subcmd = args->ArgV(1);

		if(!_stricmp("add", subcmd))
		{
			std::string cmds("");

			for(int i = 2; i < args->ArgC(); ++i)
			{
				if(2 < i) cmds.append(" ");
				
				cmds.append(args->ArgV(i));
			}

			g_CommandSystem.Add(
				g_Hook_VClient_RenderView.GetCurTime()
				, cmds.c_str()
			);
			return;
		}
		else if(!_stricmp("enabled", subcmd))
		{
			if(3 < argc)
			{
				g_CommandSystem.Enabled = 0 != atof(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"mirv_cmd enabled 0|1 - Disable / enable command system.\n"
				"Current value: %s\n"
				, g_CommandSystem.Enabled ? "1" : "0"
			);
			return;
		}
		else if(!_stricmp("clear", subcmd) && 2 == argc)
		{
			g_CommandSystem.Clear();

			return;
		}
		else if(!_stricmp("print", subcmd) && 2 == argc)
		{
			g_CommandSystem.Console_List();

			if(g_CommandSystem.Enabled)
				Tier0_Msg("Command system is enabled (mirv_cmd enabled is 1).\n");
			else
				Tier0_Msg("COMMAND SYSTEM IS NOT ENABLED (mirv_cmd enabled is 0).\n");

			return;
		}
		else if(!_stricmp("remove", subcmd) && 3 <= argc)
		{
			g_CommandSystem.Remove(atoi(args->ArgV(2)));
			return;
		}
		else if(!_stricmp("load", subcmd) && 3 == argc)
		{	
			std::wstring wideString;
			bool bOk = AnsiStringToWideString(args->ArgV(2), wideString)
				&& g_CommandSystem.Load(wideString.c_str())
			;

			Tier0_Msg("Loading command system: %s.\n", bOk ? "OK" : "ERROR");

			return;
		}
		else if(!_stricmp("save", subcmd) && 3 == argc)
		{	
			std::wstring wideString;
			bool bOk = AnsiStringToWideString(args->ArgV(2), wideString)
				&& g_CommandSystem.Save(wideString.c_str())
			;

			Tier0_Msg("Saving command system: %s.\n", bOk ? "OK" : "ERROR");

			return;
		}
	}

	Tier0_Msg(
		"mirv_cmd enabled [...] - Control if command system is enabled (by default it is).\n"
		"mirv_cmd add [command1] [command2] ... [commandN] - Adds/appends one or multiple commands at the current time.\n"
		"mirv_cmd clear - Removes all commands.\n"
		"mirv_cmd print - Prints commands / state.\n"
		"mirv_cmd remove <index> - Removes a command by it's index.\n"
		"mirv_cmd load <fileName> - loads commands from the file (XML format)\n"
		"mirv_cmd save <fileName> - saves commands to the file (XML format)\n"
	);
	return;
}

CON_COMMAND(mirv_fix, "Various fixes")
{
	int argc = args->ArgC();

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (!_stricmp("physicsMaxFps", cmd1))
		{
			if (!Hook_csgo_vphsyics_frametime_lowerlimit())
			{
				Tier0_Warning("Error: Required hooks not installed.\n");
				return;
			}

			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				csgo_vphysics_SetMaxFps(atof(cmd2));
				return;
			}

			Tier0_Msg(
				"mirv_fix physicsMaxFps <floatFPSval> - Set the FPS limit for physics.\n"
				"Current value: %f\n",
				csgo_vphysics_GetMaxFps()
			);
			return;
		}
		else
			if (!_stricmp("blockObserverTarget", cmd1))
		{
			if (!Hook_csgo_C_BasePlayer_RecvProxy_ObserverTarget())
			{
				Tier0_Warning("Error: Required hooks not installed.\n");
				return;
			}

			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				g_csgo_Block_C_BasePlayer_RecvProxy_ObserverTarget = 0 != atoi(cmd2);
				return;
			}

			Tier0_Msg(
				"mirv_fix blockObserverTarget 0|1 - Fixes unwanted player switching i.e. upon bomb plant (blocks C_BasePlayer::RecvProxy_ObserverTarget).\n"
				"Current value: %i\n",
				g_csgo_Block_C_BasePlayer_RecvProxy_ObserverTarget ? 1 : 0
			);
			return;
		}
		else
		if (!_stricmp("oldDuckFix", cmd1))
		{
			if (!Hook_csgo_CCSGameMovement_DuckFix())
			{
				Tier0_Warning("Error: Required hooks not installed.\n");
				return;
			}

			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				g_Enable_csgo_CCSGameMovement_DuckFix = 0 != atoi(cmd2);
				return;
			}

			Tier0_Msg(
				"mirv_fix oldDuckFix 0|1 - Can fix player stuck in duck for old demos.\n"
				"Current value: %i\n",
				g_Enable_csgo_CCSGameMovement_DuckFix ? 1 : 0
			);
			return;
		}
		else
		if (!_stricmp("playerAnimState", cmd1))
		{
			if (!Hook_csgo_PlayerAnimStateFix())
			{
				Tier0_Warning("Error: Required hooks not installed.\n");
				return;
			}

			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				Enable_csgo_PlayerAnimStateFix_set(atoi(cmd2));

				if(args->ArgC() >= 4)
					g_csgo_mystique_annimation_factor = (float)atof(args->ArgV(3));

				return;
			}

			Tier0_Msg(
				"mirv_fix playerAnimState 0|1|2|3- Fixes twitching of player arms, see https://github.com/ripieces/advancedfx/wiki/Source%3ASmoother-Demos , 0 - disabled, 1 - enabled, 2/3 - debug.\n"
				"Current value: %i\n",
				Enable_csgo_PlayerAnimStateFix_get()
			);
			return;
		}
	}

	Tier0_Msg(
		"mirv_fix physicsMaxFps [...] - Can raise the FPS limit for physics (i.e. rag dolls, so they don't freeze upon high host_framerate).\n"
		"mirv_fix blockObserverTarget [...] - Fixes unwanted player switching i.e. upon bomb plant (blocks C_BasePlayer::RecvProxy_ObserverTarget).\n"
		"mirv_fix oldDuckFix [...] - Can fix player stuck in duck for old demos.\n"
		"mirv_fix playerAnimState [...] - Fixes twitching of player arms/legs, see https://github.com/ripieces/advancedfx/wiki/Source%3ASmoother-Demos ."
	);
	return;
}

extern SOURCESDK::CSGO::vgui::IPanel * g_pVGuiPanel_csgo;
extern SOURCESDK::CSGO::vgui::ISurface *g_pVGuiSurface_csgo;

bool MirvFindVPanel(SOURCESDK::CSGO::vgui::VPANEL panel, char const * panelName, SOURCESDK::CSGO::vgui::VPANEL * outPanel)
{
	if (!outPanel)
		return false;

	if (!strcmp(panelName, g_pVGuiPanel_csgo->GetName(panel)))
	{
		*outPanel = panel;
		return true;
	}			

	for (int i = 0; i < g_pVGuiPanel_csgo->GetChildCount(panel); ++i)
	{
		if (MirvFindVPanel(g_pVGuiPanel_csgo->GetChild(panel, i), panelName, outPanel))
			return true;
	}

	return false;
}

bool MirvVPanelSetVisible(char const * panelName, bool visible)
{
	SOURCESDK::CSGO::vgui::VPANEL panel;

	if (MirvFindVPanel(g_pVGuiSurface_csgo->GetEmbeddedPanel(), panelName, &panel))
	{
		g_pVGuiPanel_csgo->SetVisible(panel, visible);
		return true;
	}

	return false;
}

CON_COMMAND(mirv_vpanel, "VGUI Panel access")
{
	if (!(g_pVGuiSurface_csgo && g_pVGuiPanel_csgo))
	{
		Tier0_Warning("Errror: Missing dependencies.\n");
		return;
	}

	int argc = args->ArgC();

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);


		if (!_stricmp("hide", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if (!MirvVPanelSetVisible(cmd2, false))
				{
					Tier0_Warning("Error: Invalid panel name %s\n", cmd2);
				}		
				return;
			}

			Tier0_Msg(
				"mirv_vpanel hide <panelName>\n"
			);
			return;
		}
		else
		if (!_stricmp("show", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if (!MirvVPanelSetVisible(cmd2, true))
				{
					Tier0_Warning("Error: Invalid panel name %s\n", cmd2);
				}
				return;
			}

			Tier0_Msg(
				"mirv_vpanel show <panelName>\n"
			);
			return;
		}
	}

	Tier0_Msg(
		"mirv_vpanel hide [...]\n"
		"mirv_vpanel show [...]\n"
		"Hint: To find panel names use vgui_drawtree 1.\n"
	);
}
