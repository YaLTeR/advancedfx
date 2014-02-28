#include "stdafx.h"

#ifdef AFX_GUI

/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "GuiFileInterface.h"

#include <stdio.h>
#include "../hlaeFolder.h"

#define ROCKET_PATH "rocket\\"

GuiFileInterface g_GuiFileInterface;

GuiFileInterface::GuiFileInterface()
{
	root = GetHlaeFolder();
	root += ROCKET_PATH;
}

GuiFileInterface::~GuiFileInterface()
{
}

// Opens a file.
Rocket::Core::FileHandle GuiFileInterface::Open(const Rocket::Core::String& path)
{
	// Attempt to open the file relative to the application's root.
	FILE* fp = fopen((root + path).CString(), "rb");
	return (Rocket::Core::FileHandle) fp;
}

// Closes a previously opened file.
void GuiFileInterface::Close(Rocket::Core::FileHandle file)
{
	fclose((FILE*) file);
}

// Reads data from a previously opened file.
size_t GuiFileInterface::Read(void* buffer, size_t size, Rocket::Core::FileHandle file)
{
	return fread(buffer, 1, size, (FILE*) file);
}

// Seeks to a point in a previously opened file.
bool GuiFileInterface::Seek(Rocket::Core::FileHandle file, long offset, int origin)
{
	return fseek((FILE*) file, offset, origin) == 0;
}

// Returns the current position of the file pointer.
size_t GuiFileInterface::Tell(Rocket::Core::FileHandle file)
{
	return ftell((FILE*) file);
}

#endif // AFX_GUI
