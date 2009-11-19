#pragma once

// Copyright (c) by advancedfx.org
//
// Last changes:
// 2009-11-19 dominik.matrixstorm.com
//
// First changes
// 2009-11-19 dominik.matrixstorm.com

#include <windows.h>
#include <gl\gl.h>


// FxHide /////////////////////////////////////////////////////////////////////

class FxHide {
public:
	FxHide();

	// returns false if to block
	bool OnGlBegin();

	//
	// Properties:

	bool Enabled_get() { return m_Enabled; }
	void Enabled_set(bool value) { m_Enabled = value; }


private:
	bool m_Enabled;
};

extern FxHide g_FxHide;

