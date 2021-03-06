#include "stdafx.h"

// Copyright (c) advancedfx.org
//
// Last changes:
// 2016-11-15 dominik.matrixstorm.com
//
// First changes:
// 2015-09-18 dominik.matrixstorm.com

#include "csgo_view.h"

#include "addresses.h"
#include "AfxStreams.h"

#include <shared/detours.h>


SOURCESDK::IViewRender_csgo * GetView_csgo(void)
{
	SOURCESDK::IViewRender_csgo ** pView = (SOURCESDK::IViewRender_csgo **)AFXADDR_GET(csgo_view);

	if(pView)
		return *pView;

	return 0;
}


typedef bool (__stdcall * csgo_CViewRender_ShouldForceNoVis_t)(
	DWORD * this_ptr);

csgo_CViewRender_ShouldForceNoVis_t detoured_csgo_CViewRender_ShouldForceNoVis;

bool g_csgo_CViewRender_ShouldForceNoVis_OverrideEnable = false;
bool g_csgo_CViewRender_ShouldForceNoVis_OverrideValue;

bool __stdcall csgo_CViewRender_ShouldForceNoVis(
	DWORD *this_ptr)
{
	bool orgValue = detoured_csgo_CViewRender_ShouldForceNoVis(this_ptr);

	return g_AfxStreams.OnViewRenderShouldForceNoVis(orgValue);
}

bool Hook_csgo_CViewRender_ShouldForceNoVis(void)
{
	static bool firstResult = false;
	static bool firstRun = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	SOURCESDK::IViewRender_csgo * view = GetView_csgo();

	if (view)
	{
		int * vtable = *(int**)view;

		detoured_csgo_CViewRender_ShouldForceNoVis = (csgo_CViewRender_ShouldForceNoVis_t)DetourIfacePtr((DWORD *)&(vtable[41]), csgo_CViewRender_ShouldForceNoVis);

		firstResult = true;
	}

	return firstResult;
}
