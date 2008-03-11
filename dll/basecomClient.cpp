#include <windows.h>
//#include <winuser.h> // KEYBDINPUT Structure, ...

#if 1//#ifdef MDT_DEBUG
	#include <stdio.h>

	#include "wrect.h"
	#include "cl_dll.h"
	#include "cdll_int.h"

	#include "cmdregister.h"

	extern cl_enginefuncs_s* pEngfuncs;
#endif

#include "../shared/com/basecom.h"

#include "basecomClient.h"


HWND g_hwHlaeBcCltWindow = NULL;

HWND g_HL_MainWindow = NULL;
WNDCLASSA *g_HL_WndClassA = NULL;

HLAE_BASECOM_WndRectUpdate_s g_HlaeWindowRect; // Warning: in the current implementation those values may not represent the actual server's size, since HlaeBc_AdjustViewPort will do some security clamping on them when needed.

struct
{
	int iX;
	int iY;
	struct {
		int iX;
		int iY;
	} MouseTarget;
} g_HL_MainWindow_info;

LRESULT CALLBACK Hooking_WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	// filter window unspecific messages:

	switch(uMsg)
	{
	case WM_ACTIVATE:
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		// we won't override those messages, since this would give us problems with the mousecapture if we won't allow it to deactivate
		break;
	}

	if (hWnd==NULL || hWnd != g_HL_MainWindow)
		// this is not the MainWindow we want to control
		return g_HL_WndClassA->lpfnWndProc(hWnd,uMsg,wParam,lParam);

	// filter MainWindow specific messages:

	// blah
	switch (uMsg)
	{
	case WM_MOVE:
		g_HL_MainWindow_info.iX = (int)(short) LOWORD(lParam); 
		g_HL_MainWindow_info.iY = (int)(short) HIWORD(lParam);
		break;
	}

	return g_HL_WndClassA->lpfnWndProc(hWnd,uMsg,wParam,lParam);
}

LRESULT HlaeBcCallWndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
// this function is not supported atm / it will reject all requests
// the server will inform us about specific events regarding the game window,
// this is our chance to react to them:
{
	// make sure we have the WindowProc and the H-L MainWindow handle, otherwise quit:
	if (!(g_HL_WndClassA && g_HL_WndClassA->lpfnWndProc && g_HL_MainWindow)) return FALSE;

	//
	// filtering of specific message codes:

	return FALSE; // this function is not supported atm

	// in any case we will replace the hwnd param, since the H-L winproc shall belive it originated from it's own window:


	//
	// after filtering we pass them on to the real proc (in case we didn't quit already):

	LRESULT lrRet = FALSE;

	lrRet = g_HL_WndClassA->lpfnWndProc(hwnd,uMsg,wParam,lParam);

	//
	// we could examine or modify the result here:

	// blah

	//
	// and finally we return:
	return lrRet;
}

// Handlers for events from Server's GameWindow:

LRESULT HlaeBcCl_WndRectUpdate(PVOID lpData)
{
	HLAE_BASECOM_WndRectUpdate_s *myps = (HLAE_BASECOM_WndRectUpdate_s *)lpData;
	//pEngfuncs->Con_Printf("MoveEvent: %i,%i\n",myps->iLeftGlobal,myps->iTopGlobal);
	g_HlaeWindowRect = *myps;

	return TRUE;
}

LRESULT HlaeBcCl_MouseEvent(PVOID lpData)
{
	HLAE_BASECOM_MSGCL_MouseEvent_s *myps = (HLAE_BASECOM_MSGCL_MouseEvent_s *)lpData;

	g_HL_MainWindow_info.MouseTarget.iX = myps->iX;
	g_HL_MainWindow_info.MouseTarget.iY = myps->iY;

	//pEngfuncs->Con_Printf("HlaeBcCl_MouseEvent: (%i,%i)\n",myps->iX,myps->iY);

	pEngfuncs->Con_Printf("OutPorc: 0x%08x - IsProc: 0x%08x\n",(void *)Hooking_WndProc,(void *)(GetWindowLong( g_HL_MainWindow, GWL_WNDPROC )));

	LRESULT tr = FALSE;//g_HL_WndClassA->lpfnWndProc(g_HL_MainWindow,(UINT)(myps->uMsg),(WPARAM)(myps->wParam),(LPARAM)(((myps->iY) << 16) + myps->iX));

	/*int ix,iy;
	pEngfuncs->GetMousePosition(&ix,&iy);
	int dx=g_HL_MainWindow_info.iX+myps->iX;
	int dy=g_HL_MainWindow_info.iY+myps->iX;
	pEngfuncs->Con_Printf("MouseEvent: inx=%i,iny=%i,dx=%i,dy=%i (%i,%i)\n",myps->iX,myps->iY,dx,dy,ix,iy);*/

	return tr;
}

LRESULT HlaeBcCl_KeyBoardEvent(PVOID lpData)
{
	HLAE_BASECOM_MSGCL_KeyBoardEvent_s *myps = (HLAE_BASECOM_MSGCL_KeyBoardEvent_s *)lpData;

	// pEngfuncs->Con_Printf("KeyEvent: %i %i 0x%08x\n",myps->uMsg,myps->uKeyCode,myps->uKeyFlags);

	return g_HL_WndClassA->lpfnWndProc(g_HL_MainWindow,(UINT)(myps->uMsg),(WPARAM)(myps->uKeyCode),(LPARAM)(myps->uKeyFlags));

}


// In order to understand the source that follows the Microsoft Documentation of WM_COPYDATA might be useful.

void *g_pHlaeBcResultTarget=NULL;

LRESULT CALLBACK HlaeBcCltWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{ 
    switch (uMsg) 
    { 
        case WM_CREATE: 
            // Initialize the window.
			//MessageBoxW(hwnd,L"WM_CREATE",HLAE_BASECOM_CLIENT_ID,MB_OK);
            return 0; 
 
        case WM_PAINT: 
            // Paint the window's client area. 
			return 0; 
 
        case WM_SIZE: 
            // Set the size and position of the window. 
            return 0; 
 
        case WM_DESTROY: 
            // Clean up window-specific data objects. 
			//MessageBoxW(hwnd,L"WM_DESTROY",HLAE_BASECOM_CLIENT_ID,MB_OK); 
            return 0; 

		case WM_COPYDATA:
			PCOPYDATASTRUCT pMyCDS;
			pMyCDS = (PCOPYDATASTRUCT) lParam;

			switch(pMyCDS->dwData)
			{
			case HLAE_BASECOM_MSG_TESTDUMMY:
				MessageBoxW(hwnd,L"Got empty test data.",HLAE_BASECOM_CLIENT_ID,MB_OK);
				return TRUE;

			case HLAE_BASECOM_MSGCL_RET_CreateWindowExA:
				//MessageBox(0,"TEST","Hello",MB_OK);
				if (g_pHlaeBcResultTarget) memcpy(g_pHlaeBcResultTarget,pMyCDS->lpData,sizeof(HLAE_BASECOM_RET_CreateWindowExA_s));
				if(pMyCDS->lpData == NULL) MessageBox(0,"NULL","0000",MB_OK);
				return TRUE;

			case HLAE_BASECOM_MSGCL_RET_RegisterClassA:
				if (g_pHlaeBcResultTarget) memcpy(g_pHlaeBcResultTarget,pMyCDS->lpData,sizeof(HLAE_BASECOM_RET_RegisterClassA_s));
				return TRUE;

			case HLAE_BASECOM_MSGCL_RET_DestroyWindow:
				if (g_pHlaeBcResultTarget) memcpy(g_pHlaeBcResultTarget,pMyCDS->lpData,sizeof(HLAE_BASECOM_RET_DestroyWindow_s));
				return TRUE;

			case HLAE_BASECOM_MSGCL_CallWndProc_s:
				// it's a bit risky, but whatev0r:
				return HlaeBcCallWndProc(((HLAE_BASECOM_CallWndProc_s *)(pMyCDS->lpData))->hwnd,((HLAE_BASECOM_CallWndProc_s *)(pMyCDS->lpData))->uMsg,((HLAE_BASECOM_CallWndProc_s *)(pMyCDS->lpData))->wParam,((HLAE_BASECOM_CallWndProc_s *)(pMyCDS->lpData))->lParam);

			case HLAE_BASECOM_MSGCL_RET_GameWndGetDC:
				if (g_pHlaeBcResultTarget) memcpy(g_pHlaeBcResultTarget,pMyCDS->lpData,sizeof(HLAE_BASECOM_RET_GameWndGetDC_s));
				return TRUE;

			case HLAE_BASECOM_MSGCL_WndRectUpdate:
				return HlaeBcCl_WndRectUpdate(pMyCDS->lpData);

			case HLAE_BASECOM_MSGCL_MouseEvent:
				return HlaeBcCl_MouseEvent(pMyCDS->lpData);

			case HLAE_BASECOM_MSGCL_KeyBoardEvent:
				return HlaeBcCl_KeyBoardEvent(pMyCDS->lpData);

			case HLAE_BASECOM_MSGCL_RET_ChooseNVIDIA	:
				if (g_pHlaeBcResultTarget) memcpy(g_pHlaeBcResultTarget,pMyCDS->lpData,sizeof(HLAE_BASECOM_RET_ChooseNVIDIA_s));
				return TRUE;


			default:
				MessageBoxW(hwnd,L"Unexpected message.",HLAE_BASECOM_CLIENT_ID,MB_OK|MB_ICONERROR);
			}
			return FALSE;
 
        default: 
            return DefWindowProc(hwnd, uMsg, wParam, lParam); 
    } 
    return 0; 
}

bool HlaeBcCltStart()
// this starts the client server for being able to recieve
// return messages and other messages form the server
{
	HINSTANCE hInstance = (HINSTANCE)GetCurrentProcessId();

	if (g_hwHlaeBcCltWindow)
		return false; // if already running quit

	static bool bRegistered=false;
	static WNDCLASSW wc;

	if (!bRegistered)
	{
 
		// Register the main window class. 
		wc.style = NULL; 
		wc.lpfnWndProc = (WNDPROC) HlaeBcCltWndProc; 
		wc.cbClsExtra = 0; 
		wc.cbWndExtra = 0; 
		wc.hInstance = hInstance; 
		wc.hIcon = NULL; 
		wc.hCursor = NULL; 
		wc.hbrBackground = NULL; 
		wc.lpszMenuName =  NULL; 
		wc.lpszClassName = HLAE_BASECOM_CLASSNAME;

		if (!RegisterClassW(&wc))
			return false;

		bRegistered = true;
	 }

	if (!(g_hwHlaeBcCltWindow = CreateWindowW(wc.lpszClassName,HLAE_BASECOM_CLIENT_ID,WS_DISABLED|WS_POPUP,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,hInstance,NULL)))
		return false;

	return true;
}

bool HlaeBcCltStop()
// this stops the client server
{
	if (!g_hwHlaeBcCltWindow) return true;

	if (!DestroyWindow(g_hwHlaeBcCltWindow)) return false;

	g_hwHlaeBcCltWindow = NULL;
	return true;
}

bool HlaeBcCltSendMessage(DWORD dwId,DWORD cbSize,PVOID lpData)
// use this for one-way messages to the server
// this function could be optimized by reducing expensive FindWindowW calls
{
	HWND hwServerWindow;
	
	// curently wastes time:
	if(!(hwServerWindow=FindWindowW(HLAE_BASECOM_CLASSNAME,HLAE_BASECOM_SERVER_ID)))
		return false;

	COPYDATASTRUCT myCopyData;
	
	myCopyData.dwData=dwId;
	myCopyData.cbData=cbSize;
	myCopyData.lpData=lpData;

	return TRUE==SendMessageW(
		hwServerWindow, // the Server is the target
		WM_COPYDATA,
		(WPARAM)g_hwHlaeBcCltWindow, // if we have a window running (!=NULL), then inform the server, so it can identifiy us
		(LPARAM)&myCopyData
	);
}

bool HlaeBcCltSendMessageRet(DWORD dwId,DWORD cbSize,PVOID lpData,void *pResultTarget)
// use this for messages to the server to which the server will reply with a return (RET) message
{
	bool bRes;

	if (!g_hwHlaeBcCltWindow) return false;

	g_pHlaeBcResultTarget = pResultTarget;
	bRes=HlaeBcCltSendMessage(dwId,cbSize,lpData);
	g_pHlaeBcResultTarget = NULL;

	return bRes;
}

//
// Higher level interaction with the server:
//

// Please be aware that the Recieving Window Proc has to know about the return
// codes and act accordingly!
// So if you modifiy s.th. regarding the returns, you might need to modifiy it
// too.

bool HlaeBcClt_GameWndPrepare(int nWidth, int nHeight)
// Query the server to prepare the game window
// nWidth - desired width
// nHeight - desired height
// Returns true if the query was successfull, otherwise false
{
	bool bResult=false;
	HLAE_BASECOM_GameWndPrepare_s *mycws = new HLAE_BASECOM_GameWndPrepare_s;

	mycws->nWidth = nWidth;
	mycws->nHeight = nHeight;

	bResult=HlaeBcCltSendMessage(HLAE_BASECOM_MSGSV_GameWndPrepare,sizeof(HLAE_BASECOM_GameWndPrepare_s),(PVOID)mycws);

	delete mycws;
	return bResult;
}

bool HlaeBcClt_GameWndGetDC(HWND *pretHWND)
// Query the HDC from the server
// pretHWND - pointer where the resulting HWND shall be saved
//            if pretHWND is NULL, the result will be discarded
//            this returns the DC's HWND, not the DC (because the
//            HDC is not allowed to be shared among threads)
// Returns true if the query was successfull, otherwise false
{
	bool bResult=false;

	HLAE_BASECOM_RET_GameWndGetDC_s *mycwret = new HLAE_BASECOM_RET_GameWndGetDC_s;
	HLAE_BASECOM_GameWndGetDC_s *mycws = new HLAE_BASECOM_GameWndGetDC_s;
	
	if(HlaeBcCltSendMessageRet(HLAE_BASECOM_MSGSV_GameWndGetDC,sizeof(HLAE_BASECOM_GameWndGetDC_s),(PVOID)mycws,mycwret))
	{
		if(pretHWND) *pretHWND = mycwret->retResult;
		bResult=true;
	}

	delete mycws;
	delete mycwret;

	return bResult;
}

bool HlaeBcClt_GameWndRelease()
// Query the server to inform it, that we won't need the GameWindow anymore
// Returns true if the query was successfull, otherwise false
{
	bool bResult=false;
	HLAE_BASECOM_GameWndRelease_s *mycws = new HLAE_BASECOM_GameWndRelease_s;

	bResult=HlaeBcCltSendMessage(HLAE_BASECOM_MSGSV_GameWndRelease,sizeof(HLAE_BASECOM_GameWndRelease_s),(PVOID)mycws);

	delete mycws;
	return bResult;
}

bool HlaeBcClt_FormatNVIDIA(int iPixelFormat, CONST PIXELFORMATDESCRIPTOR * ppfd)
{
	bool bResult=false;
	HLAE_BASECOM_FormatNVIDIA_s *mycws = new HLAE_BASECOM_FormatNVIDIA_s;

	mycws->iPixelFormat = iPixelFormat;
	mycws->pfd = *ppfd;

	bResult=HlaeBcCltSendMessage(HLAE_BASECOM_MSGSV_FormatNVIDIA,sizeof(HLAE_BASECOM_FormatNVIDIA_s),(PVOID)mycws);

	delete mycws;
	return bResult;
}

bool HlaeBcClt_ChooseNVIDIA(const PIXELFORMATDESCRIPTOR *ppfd,int *piRet)
{
	bool bResult=false;

	HLAE_BASECOM_RET_ChooseNVIDIA_s *mycwret = new HLAE_BASECOM_RET_ChooseNVIDIA_s;
	HLAE_BASECOM_ChooseNVIDIA_s *mycws = new HLAE_BASECOM_ChooseNVIDIA_s;

	mycws->pfd=*ppfd;
	
	if(HlaeBcCltSendMessageRet(HLAE_BASECOM_MSGSV_ChooseNVIDIA,sizeof(HLAE_BASECOM_ChooseNVIDIA_s),(PVOID)mycws,mycwret))
	{
		if(piRet) *piRet = mycwret->retResult;
		bResult=true;
	}

	delete mycws;
	delete mycwret;

	return bResult;
}

//
// WinAPI hooks for export in the basecomClient.h:
//

ATOM APIENTRY HlaeBcClt_RegisterClassA(CONST WNDCLASSA *lpWndClass)
{

#ifdef MDT_DEBUG
	char sztemp[1000];
	if (HIWORD(lpWndClass->lpszClassName))
	{
		_snprintf(sztemp,sizeof(sztemp),"ClassName: %s",lpWndClass->lpszClassName);
		MessageBox(0,sztemp,"HlaeBcClt_RegisterClassA",MB_OK);
	}
#endif

	// quit if it's not the class we want:
	if (!HIWORD(lpWndClass->lpszClassName) || lstrcmp(lpWndClass->lpszClassName,"Valve001"))
		return RegisterClassA(lpWndClass);

	ATOM tResult = NULL;
	WNDCLASSA *tWndClass= new WNDCLASSA;

	if(!g_HL_WndClassA) g_HL_WndClassA = new WNDCLASSA;

	memcpy(g_HL_WndClassA,lpWndClass,sizeof(WNDCLASSA)); // get us a global copy of the original class, we will need it to access the original lpfnWindowProc i.e.
	memcpy(tWndClass,lpWndClass,sizeof(WNDCLASSA)); // we want to register a modified copy

	// modifiy the copy to fit our needs:

	tWndClass->lpfnWndProc = (WNDPROC)Hooking_WndProc; // we want to hook the WindowProc, so we can control the message flow

	// register our modified copy of the class instead:
	tResult = RegisterClassA(tWndClass);
	delete tWndClass;

	// start client server (if already started this will be ignored):
	HlaeBcCltStart();

	return tResult;
}

HWND APIENTRY HlaeBcClt_CreateWindowExA(DWORD dwExStyle,LPCTSTR lpClassName,LPCTSTR lpWindowName,DWORD dwStyle,int x,int y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam)
{

#ifdef MDT_DEBUG
	char sbuff[1000];
	_snprintf(sbuff,sizeof(sbuff),"dwExStyle: 0x%08x\nlpClassName: %s\nlpWindowName: %s\ndwStyle: 0x%08x\nx: %i\ny: %i\nWidth: %i\nnHeight: %i\nhWndParent: %u\nhMenu: %u\nhInstance: %u\nlpParam: 0x%08x",dwExStyle,lpClassName,lpWindowName,dwStyle,x,y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam);
	MessageBoxA(NULL,sbuff,"MDT CreateWindowEx",MB_OK|MB_ICONINFORMATION);
#endif

	// quit if it's not the window we want:
	if (hWndParent!=NULL)
		return CreateWindowExA(dwExStyle,lpClassName,lpWindowName,dwStyle,x,y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam);

	// save postion:
	g_HL_MainWindow_info.iX = x;
	g_HL_MainWindow_info.iY = y;

	// request the server to prepare the gamewindow and stuff according to our needs:
	HlaeBcClt_GameWndPrepare(nWidth,nHeight);

	g_HL_MainWindow = CreateWindowExA(dwExStyle,lpClassName,lpWindowName,dwStyle,x,y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam);

	return g_HL_MainWindow;

}

BOOL APIENTRY HlaeBcClt_DestroyWindow(HWND hWnd)
{
	// quit if it's not the window we want:
	if (hWnd==NULL || hWnd != g_HL_MainWindow)
		return DestroyWindow(hWnd);

#ifdef MDT_DEBUG
	MessageBoxA(NULL,"I am about to destroy the main window ...","HlaeBcClt_DestroyWindow",MB_OK|MB_ICONINFORMATION);
#endif

	bool bResult;

	bResult = TRUE==DestroyWindow(hWnd);

	// inform server that we won't use it's window anymore:
	HlaeBcClt_GameWndRelease();

	// halt client server:
	HlaeBcCltStop();

	// clean up globals:
	g_HL_MainWindow = NULL;
	if(g_HL_WndClassA) delete g_HL_WndClassA; // I asume we won't use it anymore, this might be wrong in some rare cases.

	return bResult;
}

bool g_bHlaeAsumeServerDCPresent=false;
HWND g_hHlaeServerWnd=NULL;

HDC g_HL_HDC=NULL;

HDC APIENTRY HlaeBcClt_GetDC( HWND hWnd )
{
	// quit if it's not the window we want:
	if (hWnd==NULL || hWnd != g_HL_MainWindow)
		return GetDC(hWnd);

#ifdef MDT_DEBUG
	MessageBoxA(hWnd,"GetDC for MainWindow requested","HlaeBcClt_GetDC",MB_OK|MB_ICONINFORMATION);
#endif

	HDC hdcResult;
	HWND hwndResult;

	if((g_bHlaeAsumeServerDCPresent=HlaeBcClt_GameWndGetDC(&hwndResult)))
	{
		g_hHlaeServerWnd=hwndResult; // save handle for HlaeBcClt_ReleaseDC
		hdcResult=GetDC(hwndResult); // trick it into the server's window
	} else
		hdcResult=GetDC(hWnd);

	/*if (!DuplicateHandle(GetCurrentThread(),hdcResult,GetCurrentProcess(),(HANDLE *)&g_hMappedDC,0,true,DUPLICATE_SAME_ACCESS))
	{
		char ptemp[150];
		int iLastError=GetLastError();
		_snprintf(ptemp,149,"Duplicate handle failed ( 0x%08x ).\nGetLastError: %u (0x%08x)",hdcResult,iLastError,iLastError);
		ptemp[149]=0;
		MessageBoxA(0,ptemp,"HlaeBcClt_SetPixelFormat error:",MB_ICONERROR|MB_OK);
	}*/

	//if (g_hMappedDC) return g_hMappedDC;

	g_HL_HDC = hdcResult;
	return hdcResult;
}

int APIENTRY HlaeBcClt_ReleaseDC( HWND hWnd, HDC hDC )
{
	// quit if it's not the window we want:
	if (hWnd==NULL || hWnd != g_HL_MainWindow || !g_bHlaeAsumeServerDCPresent)
		return ReleaseDC(hWnd,hDC);

#ifdef MDT_DEBUG
	MessageBoxA(hWnd,"ReleaseDC for MainWindow requested","HlaeBcClt_ReleaseDC",MB_OK|MB_ICONINFORMATION);
#endif

	g_bHlaeAsumeServerDCPresent=false;
	//if (g_hMappedDC) CloseHandle(g_hMappedDC);
	return ReleaseDC(g_hHlaeServerWnd,hDC); // of course we trick it into the server's window again
}


void HlaeBcCl_AdjustViewPort(int &x, int &y, int width, int height)
// this will adjust the incoming params accroding to the gamewindow (in case it's DC can be asumed to be present)
{
	if(!g_hHlaeServerWnd) return;

	static int iMyLastWidth=-1;
	static int iMyLastHeight=-1;

	// firt check if we need to inform the server about changed coords,
	// this has to be done carefully to avoid pingpongs in case the Server's Window is oversized i.e.
	// we also prevent updates for 
	if ((iMyLastWidth!=width || iMyLastHeight != height ) && width > 0 && height > 0)
	{
		pEngfuncs->Con_DPrintf("HlaeBcCl_AdjustViewPort: Cached values (%ix%i) outdated, forcing update (%ix%i).\n",iMyLastWidth,iMyLastHeight,width,height);

		iMyLastWidth=width;
		iMyLastHeight=height;
		// update the server!:
		HlaeBcClt_GameWndPrepare(width,height);
	}

	// now we will apply some clamping to avoid problems
	// in case the server's window is oversized for some reason:
	
	if (g_HlaeWindowRect.iHeightTotal >  height) g_HlaeWindowRect.iHeightTotal = height;
	if (g_HlaeWindowRect.iWidthTotal >  width) g_HlaeWindowRect.iWidthTotal = width;

	if (g_HlaeWindowRect.iWidthVisible > g_HlaeWindowRect.iWidthTotal) g_HlaeWindowRect.iWidthVisible =  g_HlaeWindowRect.iWidthTotal;
	if (g_HlaeWindowRect.iHeightVisible >  g_HlaeWindowRect.iWidthTotal) g_HlaeWindowRect.iHeightVisible = g_HlaeWindowRect.iHeightTotal;


	// finally we will offset the x and y values for the glViewPort
	// in order to simulate scrolling:
	x=x-g_HlaeWindowRect.iLeft;
	y=y-g_HlaeWindowRect.iHeightTotal+g_HlaeWindowRect.iHeightVisible+g_HlaeWindowRect.iTop;

}

BOOL APIENTRY HlaeBcCl_GetCursorPos(LPPOINT lpPoint)
{
	BOOL bRet = GetCursorPos(lpPoint);
	if(!g_hHlaeServerWnd) return bRet;

	POINT dp = *lpPoint;

	// translate mouse into H-L's coords:
	lpPoint->x=g_HL_MainWindow_info.iX + g_HL_MainWindow_info.MouseTarget.iX;
	lpPoint->y=g_HL_MainWindow_info.iY + g_HL_MainWindow_info.MouseTarget.iY;

	pEngfuncs->Con_Printf("Mouse (%i%i,) -> (%i,%i)\n",dp.x,dp.y,lpPoint->x,lpPoint->y);

	return bRet;
}

HWND WINAPI HlaeBcClt_SetCapture( HWND hWnd)
{
	if(!g_hHlaeServerWnd)
		return SetCapture(hWnd);

	return NULL;
}

BOOL WINAPI HlaeBcClt_ReleaseCapture( VOID )
{
	if(!g_hHlaeServerWnd)
		return ReleaseCapture();

	return TRUE;
}

int WINAPI HlaeBcClt_ChoosePixelFormat( HDC  hdc, CONST PIXELFORMATDESCRIPTOR *  ppfd )
{
	if(!g_bHlaeAsumeServerDCPresent)
		return ChoosePixelFormat(hdc,ppfd);

	int iRet=0;

	HlaeBcClt_ChooseNVIDIA(ppfd,&iRet);

	return iRet;
}

BOOL WINAPI HlaeBcClt_SetPixelFormat(  HDC  hdc,  int  iPixelFormat, CONST PIXELFORMATDESCRIPTOR *  ppfd )
{
	BOOL bRet;
	
	if(!g_bHlaeAsumeServerDCPresent)//ppfd->dwFlags&PFD_SUPPORT_OPENGL)
	{
		bRet = SetPixelFormat(hdc, iPixelFormat, ppfd);

		if (bRet==FALSE)
		{
			char ptemp[150];
			int iLastErr = GetLastError();
			_snprintf(ptemp,149,"SetPixelFormat request failed.\nGetLastError: %u, 0x%08x",iLastErr,iLastErr);
			ptemp[149]=0;
			MessageBoxA(0,ptemp,"HLAE Client error:",MB_ICONERROR|MB_OK);
		}
	} else {
		//pEngfuncs->Con_DPrintf("Marshalling SetPixelFormat to HLAE GUI in order to avoid running into NVIDIA OGL driver bugs ...\n");
		bRet=HlaeBcClt_FormatNVIDIA(iPixelFormat,ppfd);

		if (bRet==FALSE)
			MessageBoxA(0,"HLAE GUI Server - SetPixelFormat Failed","HLAE Client error:",MB_ICONERROR|MB_OK);
	}

	return bRet;
}

HGLRC WINAPI HlaeBcClt_wglCreateContext(HDC  hdc)
{
	HGLRC hRet;

	// this is all crap, instead we need to build our own context and then try wglMakeCurrent
	
	if(g_bHlaeAsumeServerDCPresent && hdc == g_HL_HDC)
	{
		ReleaseDC( g_hHlaeServerWnd, hdc );
		hdc = GetDC( g_hHlaeServerWnd );

		//MessageBoxA(0,"CreateContext","lobob",MB_OK);
		hRet= wglCreateContext(hdc);
	}
	else
		hRet= wglCreateContext(hdc);

	if (!hRet)
	{
		char ptemp[200];
		int iLastErr = GetLastError();
		_snprintf(ptemp,199,"wglCreateContext request failed.\nGetLastError: %u (0x%08x)\ng_hHlaeServerWnd=0x%08x, hdc =  0x%08x",iLastErr,iLastErr,g_hHlaeServerWnd,hdc);
		ptemp[199]=0;
		MessageBoxA(0,ptemp,"HLAE Client error:",MB_ICONERROR|MB_OK);
	}

	return hRet;
}

//
// debug helper:
//

#if 1
//#ifdef MDT_DEBUG

REGISTER_DEBUGCMD_FUNC(info_devicecontext)
{
	if (!g_bHlaeAsumeServerDCPresent)
	{
		HDC hdcResult;
	
		hdcResult=GetDC(g_HL_MainWindow);

		PIXELFORMATDESCRIPTOR *pPfd = new PIXELFORMATDESCRIPTOR;

		char sztemp[2000];

		if (DescribePixelFormat( hdcResult, GetPixelFormat(hdcResult),sizeof(PIXELFORMATDESCRIPTOR),pPfd))
		{

			_snprintf(sztemp,sizeof(sztemp),"DescribePixelFormat says:\nnSize:%u\nnVersion:%u\ndwFlags:0x%08x\niPixelType:%i\ncColorBits:%u\ncRedBits:%u\ncRedShift:%u\ncGreenBits:%u\ncGreenShift:%u\ncBlueBits:%u\ncBlueShift:%u\ncAlphaBits:%u\ncAlphaShift:%u\ncAccumBits:%u\ncAccumRedBits:%u\ncAccumGreenBits:%u\ncAccumBlueBits:%u\ncAccumAlphaBits:%u\ncDepthBits:%u\ncStencilBits:%u\ncAuxBuffers:%u\niLayerType:%i\nbReserved:%u\ndwLayerMask:%u\ndwVisibleMask:%u\ndwDamageMask:%u",
				pPfd->nSize, pPfd->nVersion, pPfd->dwFlags, pPfd->iPixelType,
				pPfd->cColorBits, pPfd->cRedBits, pPfd->cRedShift, pPfd->cGreenBits,
				pPfd->cGreenShift, pPfd->cBlueBits, pPfd->cBlueShift, pPfd->cAlphaBits,
				pPfd->cAlphaShift, pPfd->cAccumBits, pPfd->cAccumRedBits, pPfd->cAccumGreenBits,
				pPfd->cAccumBlueBits, pPfd->cAccumAlphaBits, pPfd->cDepthBits, pPfd->cStencilBits,
				pPfd->cAuxBuffers, pPfd->iLayerType, pPfd->bReserved, pPfd->dwLayerMask,
				pPfd->dwVisibleMask, pPfd->dwDamageMask);

			MessageBoxA(NULL,sztemp,"Client - info_pxformat",MB_OK|MB_ICONINFORMATION);

		} else 	MessageBoxA(NULL,"DescribePixelFormat failed","Client - info_pxformat",MB_OK|MB_ICONERROR);

		delete pPfd;

		ReleaseDC(g_HL_MainWindow,hdcResult);
	} else pEngfuncs->Con_Printf("HLAE Server DC is used.\nInfo command not available in this Case!\n");
}
#endif

// I'll leave this old function source in here for now (may be for historical reasons, don't know):
// may be leave it in, might be a target for remembering how to do some tricky stuff by design
// (it also shows how to piggy back strings and stuff)
/*
HWND Old_HlaeBcClt_CreateWindowExA(DWORD dwExStyle,LPCTSTR lpClassName,LPCTSTR lpWindowName,DWORD dwStyle,int x,int y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam)
{
	HWND hwRetWin=NULL;

	HLAE_BASECOM_RET_CreateWindowExA_s *mycwret = new HLAE_BASECOM_RET_CreateWindowExA_s;
	HLAE_BASECOM_CreateWindowExA_s *mycws;

	size_t cbBase=sizeof(HLAE_BASECOM_CreateWindowExA_s);
	size_t cbClassName=HIWORD(lpClassName) ? strlen(lpClassName)+1 : 0;
	size_t cbWindowName=HIWORD(lpWindowName) ? strlen(lpWindowName)+1 : 0;
	size_t cbPiggyBack=cbClassName+cbWindowName;

	mycws=(HLAE_BASECOM_CreateWindowExA_s *)malloc(cbBase+cbPiggyBack);

	mycws->dwExStyle = dwExStyle;

	if(cbClassName>0)
	{
		mycws->lpClassName = (LPCTSTR)cbBase; memcpy((char *)mycws + cbBase,lpClassName,cbClassName);
	} else
		mycws->lpClassName = NULL;

	if (cbWindowName>0)
	{
		mycws->lpWindowName = (LPCTSTR)(cbBase + cbClassName); memcpy((char *)mycws + cbBase + cbClassName,lpWindowName,cbWindowName);
	} else
		mycws->lpWindowName = NULL;

	mycws->dwStyle = dwStyle;
	mycws->x = x;
	mycws->y = y ;
	mycws->nWidth = nWidth;
	mycws->nHeight = nHeight;
	mycws->hWndParent = hWndParent;
	mycws->hMenu = hMenu;
	mycws->hInstance = hInstance;
	mycws->lpParam = lpParam;

	if(HlaeBcCltSendMessageRet(HLAE_BASECOM_MSGSV_CreateWindowExA,cbBase+cbPiggyBack,(PVOID)mycws,mycwret))
		hwRetWin = mycwret->retResult;

	free(mycws);
	delete mycwret;

	if (hwRetWin)
		return hwRetWin;
	else
		// s.th. failed, create window as desired within engine:
		return CreateWindowExA(dwExStyle,lpClassName,lpWindowName,dwStyle,x,y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam);

}
*/