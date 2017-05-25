/* 
 * $Id: TaskbarNotifier.cpp 11398 2009-03-17 11:00:27Z huby $
 * 
 *  TaskbarNotifier.cpp : implementation file
 *  By John O'Byrne
 *  11 August 2002: - Timer precision is now determined automatically
 * 		   Complete change in the way the popup is showing (thanks to this,now the popup can be always on top, it shows even while watching a movie)
 * 		   The popup doesn't steal the focus anymore (by replacing ShowWindow(SW_SHOW) by ShowWindow(SW_SHOWNOACTIVATE))
 * 		   Thanks to Daniel Lohmann, update in the way the taskbar pos is determined (more flexible now)
 *  17 July 2002: - Another big Change in the method for determining the pos of the taskbar (using the SHAppBarMessage function)
 *  16 July 2002: - Thanks to the help of Daniel Lohmann, the Show Function timings work perfectly now ;)
 *  15 July 2002: - Change in the method for determining the pos of the taskbar
 * 		 (now handles the presence of quick launch or any other bar).
 * 		 Remove the Handlers for WM_CREATE and WM_DESTROY
 * 		 SetSkin is now called SetBitmap
 *  14 July 2002: - Changed the GenerateRegion func by a new one (to correct a win98 bug)
 *  kei-kun modifications:
 *  30 October  2002: - Added event type management (TBN_*) for eMule project
 *  04 November 2002: - added skin support via .ini file
 */

#include "stdafx.h"
#include "emule.h"
#include "ini2.h"
#include "otherfunctions.h"
#include "enbitmap.h"
#include "TaskbarNotifier.h"
#include "emuledlg.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define IDT_HIDDEN			0
#define IDT_APPEARING		1
#define IDT_WAITING			2
#define IDT_DISAPPEARING	3

#define TASKBAR_X_TOLERANCE 10
#define TASKBAR_Y_TOLERANCE 10

inline bool NearlyEqual(int a, int b, int epsilon)
{
	return abs(a - b) < epsilon / 2;
}

// CTaskbarNotifier

IMPLEMENT_DYNAMIC(CTaskbarNotifier, CWnd)

BEGIN_MESSAGE_MAP(CTaskbarNotifier, CWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CTaskbarNotifier::CTaskbarNotifier()
{
	m_strCaption="";
	m_pWndParent=NULL;
	m_bMouseIsOver=FALSE;
	m_hBitmapRegion=NULL;
	m_hCursor=NULL;
	m_crNormalTextColor=RGB(133,146,181);
	m_crSelectedTextColor=RGB(10,36,106);
	m_nBitmapHeight=0;
	m_nBitmapWidth=0;
	m_dwTimeToStay=0;
	m_dwShowEvents=0;
	m_dwHideEvents=0;
	m_nCurrentPosX=0;
	m_nCurrentPosY=0;
	m_nCurrentWidth=0;
	m_nCurrentHeight=0;
	m_nIncrementShow=0;
	m_nIncrementHide=0;
	m_dwTimeToShow=500;
	m_dwTimeToStay=4000;
	m_dwTimeToHide=200;
	m_nTaskbarPlacement=ABE_BOTTOM;
	m_nAnimStatus=IDT_HIDDEN;
	m_rcText.SetRect(0,0,0,0);
	m_rcCloseBtn.SetRect(0,0,0,0);
	m_uTextFormat=DT_MODIFYSTRING | DT_WORDBREAK | DT_PATH_ELLIPSIS | DT_END_ELLIPSIS | DT_NOPREFIX; // Default Text format (see DrawText in the win32 API for the different values)
	m_hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(32649)); // System Hand cursor
	m_nHistoryPosition = 0;
	m_nActiveMessageType  = TBN_NULL;
	m_bTextSelected = FALSE;
	m_bAutoClose = TRUE;

	// If running on NT, timer precision is 10 ms, if not timer precision is 50 ms
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if (osvi.dwPlatformId==VER_PLATFORM_WIN32_NT)
		m_dwTimerPrecision=10;
	else
		m_dwTimerPrecision=50;

	SetTextDefaultFont(); // We use default GUI Font
}

CTaskbarNotifier::~CTaskbarNotifier()
{
	while (m_MessageHistory.GetCount() > 0)
	{
		CTaskbarNotifierHistory* messagePTR = (CTaskbarNotifierHistory*)m_MessageHistory.RemoveTail();
		delete messagePTR;
	}
}

LRESULT CALLBACK My_AfxWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

int CTaskbarNotifier::Create(CWnd *pWndParent)
{
	ASSERT(pWndParent!=NULL);
	m_pWndParent=pWndParent;

	WNDCLASSEX wcx;

	wcx.cbSize = sizeof(wcx);
	// From: http://www.trigeminal.com/usenet/usenet031.asp?1033
	// Subject: If you are using MFC 6.0 or 7.0 and you want to use MSLU...
	// 
	// There is one additional problem that can occur if you are using AfxWndProc (MFC's main, shared window
	// proc wrapper) as an actual wndproc in any of your windows. You see, MFC has code in it so that if AfxWndProc 
	// is called and is told that the wndproc to follow up with is AfxWndProc, it notices that it is being asked to 
	// call itself and forwards for DefWindowProc instead.
	// 
	// Unfortunately, MSLU breaks this code by having its own proc be the one that shows up. MFC has no way of 
	// detecting this case so it calls the MSLU proc which calls AfxWndProc which calls the MSLU proc, etc., until 
	// the stack overflows. By using either DefWindowProc or your own proc yourself, you avoid the stack overflow.
	extern bool g_bUnicoWS;
	if (g_bUnicoWS)
		wcx.lpfnWndProc = My_AfxWndProc;
	else
		wcx.lpfnWndProc = AfxWndProc;
	static const TCHAR s_szClassName[] = _T("eMule_TaskbarNotifierWndClass");
	wcx.style = CS_DBLCLKS|CS_SAVEBITS;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = AfxGetInstanceHandle();
	wcx.hIcon = NULL;
	wcx.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcx.hbrBackground=::GetSysColorBrush(COLOR_WINDOW);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = s_szClassName;
	wcx.hIconSm = NULL;
	RegisterClassEx(&wcx);

	return CreateEx(WS_EX_TOOLWINDOW, s_szClassName, NULL, WS_POPUP, 0, 0, 0, 0, pWndParent->m_hWnd, NULL);
}

void CTaskbarNotifier::OnSysColorChange()
{
	CWnd::OnSysColorChange();
	LoadConfiguration(m_strConfigFilePath);
}

BOOL CTaskbarNotifier::LoadConfiguration(LPCTSTR szFileName)
{
	TCHAR szConfigDir[MAX_PATH];
	int nRed, nGreen, nBlue, sRed, sGreen, sBlue;
	int rcLeft, rcTop, rcRight, rcBottom;
	int bmpTrasparentRed, bmpTrasparentGreen, bmpTrasparentBlue;
	int fontSize;
	CString fontType, bmpFullPath, strBmpFileName;

	Hide();

	m_strConfigFilePath = szFileName;
	CIni ini(szFileName, _T("CONFIG"));
	_tcsncpy(szConfigDir, szFileName, _countof(szConfigDir));
	szConfigDir[_countof(szConfigDir)-1] = _T('\0');
	LPTSTR pszFileName = _tcsrchr(szConfigDir, _T('\\'));
	if (pszFileName == NULL)
		return FALSE;
	*(pszFileName + 1) = _T('\0');

	nRed   = ini.GetInt(_T("TextNormalRed"),255);
	nGreen = ini.GetInt(_T("TextNormalGreen"),255);
	nBlue  = ini.GetInt(_T("TextNormalBlue"),255);
	sRed   = ini.GetInt(_T("TextSelectedRed"),255);
	sGreen = ini.GetInt(_T("TextSelectedGreen"),255);
	sBlue  = ini.GetInt(_T("TextSelectedBlue"),255);
	bmpTrasparentRed   = ini.GetInt(_T("bmpTrasparentRed"),255);
	bmpTrasparentGreen = ini.GetInt(_T("bmpTrasparentGreen"),0);
	bmpTrasparentBlue  = ini.GetInt(_T("bmpTrasparentBlue"),255);
	fontType = ini.GetString(_T("FontType"), _T("MS Shell Dlg"));
	fontSize = ini.GetInt(_T("FontSize"), 8) * 10;
	m_dwTimeToStay = ini.GetInt(_T("TimeToStay"), 4000);
	m_dwTimeToShow = ini.GetInt(_T("TimeToShow"), 500); 
	m_dwTimeToHide = ini.GetInt(_T("TimeToHide"), 200);
	strBmpFileName = ini.GetString(_T("bmpFileName"), _T(""));
	if (!strBmpFileName.IsEmpty()) {
		if (PathIsRelative(strBmpFileName))
			bmpFullPath.Format(_T("%s%s"), szConfigDir, strBmpFileName);
		else
			bmpFullPath = strBmpFileName;
	}

	// get text rectangle coordinates
	rcLeft = ini.GetInt(_T("rcTextLeft"),5);
	rcTop  = ini.GetInt(_T("rcTextTop"),55);	
	rcRight  = ini.GetInt(_T("rcTextRight"),181);
	rcBottom = ini.GetInt(_T("rcTextBottom"), 97);
	if (rcLeft<=0)	  rcLeft=1;
	if (rcTop<=0)	  rcTop=1;
	if (rcRight<=0)	  rcRight=1;
	if (rcBottom<=0)  rcBottom=1;
	SetTextRect(CRect(rcLeft,rcTop,rcRight,rcBottom));

	// get close button rectangle coordinates
	rcLeft = ini.GetInt(_T("rcCloseBtnLeft"),283);
	rcTop  = ini.GetInt(_T("rcCloseBtnTop"),52); 
	rcRight  = ini.GetInt(_T("rcCloseBtnRight"), 296);
	rcBottom = ini.GetInt(_T("rcCloseBtnBottom"), 65);
	if (rcLeft<=0)	  rcLeft=1;
	if (rcTop<=0)	  rcTop=1;
	if (rcRight<=0)	  rcRight=1;
	if (rcBottom<=0)  rcBottom=1;
	SetCloseBtnRect(CRect(rcLeft,rcTop,rcRight,rcBottom));

	// get history button rectangle coordinates
	rcLeft = ini.GetInt(_T("rcHistoryBtnLeft"),261);
	rcTop  = ini.GetInt(_T("rcHistoryBtnTop"),5);	
	rcRight  = ini.GetInt(_T("rcHistoryBtnRight"), 299);
	rcBottom = ini.GetInt(_T("rcHistoryBtnBottom"), 40);
	if (rcLeft<=0)	  rcLeft=1;
	if (rcTop<=0)	  rcTop=1;
	if (rcRight<=0)	  rcRight=1;
	if (rcBottom<=0)  rcBottom=1;
	SetHistoryBtnRect(CRect(rcLeft,rcTop,rcRight,rcBottom));

	if (bmpFullPath.IsEmpty() || !SetBitmap(bmpFullPath, bmpTrasparentRed, bmpTrasparentGreen, bmpTrasparentBlue))
	{
		CEnBitmap imgTaskbar;
		if (imgTaskbar.LoadImage(IDR_TASKBAR, _T("GIF")))
		{
			if (!SetBitmap(&imgTaskbar, bmpTrasparentRed, bmpTrasparentGreen, bmpTrasparentBlue))
				return FALSE;
		}
	}

	SetTextFont(fontType, fontSize, TN_TEXT_NORMAL,TN_TEXT_UNDERLINE);
	SetTextColor(RGB(nRed, nGreen, nBlue), RGB(sRed, sGreen, sBlue));
	return TRUE;
}

void CTaskbarNotifier::SetTextFont(LPCTSTR szFont,int nSize,int nNormalStyle,int nSelectedStyle)
{
	LOGFONT lf;
	m_myNormalFont.DeleteObject();
	m_myNormalFont.CreatePointFont(nSize,szFont);
	m_myNormalFont.GetLogFont(&lf);

	// We  set the Font of the unselected ITEM
	if (nNormalStyle & TN_TEXT_BOLD)
		lf.lfWeight = FW_BOLD;
	else
		lf.lfWeight = FW_NORMAL;

	if (nNormalStyle & TN_TEXT_ITALIC)
		lf.lfItalic=TRUE;
	else
		lf.lfItalic=FALSE;

	if (nNormalStyle & TN_TEXT_UNDERLINE)
		lf.lfUnderline=TRUE;
	else
		lf.lfUnderline=FALSE;

	m_myNormalFont.DeleteObject();
	m_myNormalFont.CreateFontIndirect(&lf);

	// We set the Font of the selected ITEM
	if (nSelectedStyle & TN_TEXT_BOLD)
		lf.lfWeight = FW_BOLD;
	else
		lf.lfWeight = FW_NORMAL;

	if (nSelectedStyle & TN_TEXT_ITALIC)
		lf.lfItalic=TRUE;
	else
		lf.lfItalic=FALSE;

	if (nSelectedStyle & TN_TEXT_UNDERLINE)
		lf.lfUnderline=TRUE;
	else
		lf.lfUnderline=FALSE;

	m_mySelectedFont.DeleteObject();
	m_mySelectedFont.CreateFontIndirect(&lf);
}

void CTaskbarNotifier::SetTextDefaultFont()
{
	LOGFONT lf;
	CFont *pFont=CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	pFont->GetLogFont(&lf);
	m_myNormalFont.DeleteObject();
	m_mySelectedFont.DeleteObject();
	m_myNormalFont.CreateFontIndirect(&lf);
	lf.lfUnderline=TRUE;
	m_mySelectedFont.CreateFontIndirect(&lf);
}

void CTaskbarNotifier::SetTextColor(COLORREF crNormalTextColor,COLORREF crSelectedTextColor)
{
	m_crNormalTextColor=crNormalTextColor;
	m_crSelectedTextColor=crSelectedTextColor;
	RedrawWindow();
}

void CTaskbarNotifier::SetTextRect(RECT rcText)
{
	m_rcText = rcText;
}

void CTaskbarNotifier::SetCloseBtnRect(RECT rcCloseBtn)
{
	m_rcCloseBtn=rcCloseBtn;
}

void CTaskbarNotifier::SetHistoryBtnRect(RECT rcHistoryBtn)
{
	m_rcHistoryBtn=rcHistoryBtn;
}

void CTaskbarNotifier::SetTextFormat(UINT uTextFormat)
{
	m_uTextFormat=uTextFormat;
}

BOOL CTaskbarNotifier::SetBitmap(UINT nBitmapID, int red, int green, int blue)
{
	m_bitmapBackground.DeleteObject();
	if (!m_bitmapBackground.LoadBitmap(nBitmapID))
		return FALSE;

	BITMAP bm;
	GetObject(m_bitmapBackground.GetSafeHandle(), sizeof(bm), &bm);
	m_nBitmapWidth = bm.bmWidth;
	m_nBitmapHeight = bm.bmHeight;

	if (red!=-1 && green!=-1 && blue!=-1)
	{
		// No need to delete the HRGN,	SetWindowRgn() owns it after being called
		m_hBitmapRegion=CreateRgnFromBitmap((HBITMAP)m_bitmapBackground.GetSafeHandle(),RGB(red,green,blue));
		SetWindowRgn(m_hBitmapRegion, true);
	}

	return TRUE;
}

BOOL CTaskbarNotifier::SetBitmap(CBitmap* Bitmap, int red, int green, int blue)
{
	m_bitmapBackground.DeleteObject();
	if (!m_bitmapBackground.Attach(Bitmap->Detach()))
		return FALSE;

	BITMAP bm;
	GetObject(m_bitmapBackground.GetSafeHandle(), sizeof(bm), &bm);
	m_nBitmapWidth = bm.bmWidth;
	m_nBitmapHeight = bm.bmHeight;

	if (red!=-1 && green!=-1 && blue!=-1)
	{
		// No need to delete the HRGN,	SetWindowRgn() owns it after being called
		m_hBitmapRegion=CreateRgnFromBitmap((HBITMAP)m_bitmapBackground.GetSafeHandle(),RGB(red,green,blue));
		SetWindowRgn(m_hBitmapRegion, true);
	}

	return TRUE;
}

BOOL CTaskbarNotifier::SetBitmap(LPCTSTR szFileName, int red, int green, int blue)
{
	if (szFileName==NULL || szFileName[0]==_T('\0'))
		return FALSE;
	HBITMAP hBmp = (HBITMAP) ::LoadImage(AfxGetInstanceHandle(),szFileName,IMAGE_BITMAP,0,0, LR_LOADFROMFILE);
	if (!hBmp)
		return FALSE;

	m_bitmapBackground.DeleteObject();
	m_bitmapBackground.Attach(hBmp);

	BITMAP bm;
	GetObject(m_bitmapBackground.GetSafeHandle(), sizeof(bm), &bm);
	m_nBitmapWidth = bm.bmWidth;
	m_nBitmapHeight = bm.bmHeight;

	if (red!=-1 && green!=-1 && blue!=-1)
	{
		// No need to delete the HRGN,	SetWindowRgn() owns it after being called
		m_hBitmapRegion=CreateRgnFromBitmap((HBITMAP)m_bitmapBackground.GetSafeHandle(),RGB(red,green,blue));
		SetWindowRgn(m_hBitmapRegion, true);
	}

	return TRUE;
}

void CTaskbarNotifier::SetAutoClose(BOOL autoClose)
{
	m_bAutoClose = autoClose;
	if (autoClose == TRUE) {
		switch (m_nAnimStatus)
		{
		case IDT_APPEARING:
			KillTimer(IDT_APPEARING);
			break;
		case IDT_WAITING:
			KillTimer(IDT_WAITING);
			break;
		case IDT_DISAPPEARING:
			KillTimer(IDT_DISAPPEARING);
			break;
		}
		m_nAnimStatus = IDT_DISAPPEARING;
		SetTimer(IDT_DISAPPEARING, m_dwHideEvents, NULL);
	}
}

void CTaskbarNotifier::ShowLastHistoryMessage()
{
	if (m_MessageHistory.GetCount() > 0)
	{
		CTaskbarNotifierHistory* messagePTR = (CTaskbarNotifierHistory*)m_MessageHistory.RemoveHead();
		Show(messagePTR->m_strMessage, messagePTR->m_nMessageType, messagePTR->m_strLink);
		delete messagePTR;
	}
	else
		Show(GetResString(IDS_TBN_NOMESSAGEHISTORY), TBN_NULL, NULL);
}

void CTaskbarNotifier::Show(LPCTSTR szCaption, int nMsgType, LPCTSTR pszLink, BOOL bAutoClose)
{
	if (nMsgType == TBN_NONOTIFY)
		return;
	UINT nScreenWidth;
	UINT nScreenHeight;
	UINT nEvents;
	UINT nBitmapSize;
	CRect rcTaskbar;
	CTaskbarNotifierHistory* messagePTR;

	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	m_strCaption = szCaption;
	m_nActiveMessageType = nMsgType;
	m_strLink = pszLink;

	if (m_bAutoClose) // sets it only if already true, else wait for user action
		m_bAutoClose = bAutoClose;

	if ((nMsgType != TBN_NULL) && (nMsgType != TBN_LOG) && (nMsgType != TBN_IMPORTANTEVENT))
	{
		//Add element into string list. Max 5 elements.
		if (m_MessageHistory.GetCount() == 5) {
			messagePTR = (CTaskbarNotifierHistory*)m_MessageHistory.RemoveHead();
			delete messagePTR;
			messagePTR = NULL;
		}
		messagePTR = new CTaskbarNotifierHistory;
		messagePTR->m_strMessage = m_strCaption;
		messagePTR->m_nMessageType = nMsgType;
		messagePTR->m_strLink = m_strLink;
		m_MessageHistory.AddTail(messagePTR);
	}

	nScreenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	nScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);
	HWND hWndTaskbar = ::FindWindow(_T("Shell_TrayWnd"), 0);
	::GetWindowRect(hWndTaskbar, &rcTaskbar);

	// Daniel Lohmann: Calculate taskbar position from its window rect. However, on XP  it may be that 
	// the taskbar is slightly larger or smaller than the screen size. Therefore we allow some tolerance here.
	if (NearlyEqual(rcTaskbar.left, 0, TASKBAR_X_TOLERANCE) && NearlyEqual(rcTaskbar.right, nScreenWidth, TASKBAR_X_TOLERANCE))
	{
		// Taskbar is on top or on bottom
		m_nTaskbarPlacement = NearlyEqual(rcTaskbar.top, 0, TASKBAR_Y_TOLERANCE) ? ABE_TOP : ABE_BOTTOM;
		nBitmapSize = m_nBitmapHeight;
	}
	else
	{
		// Taskbar is on left or on right
		m_nTaskbarPlacement = NearlyEqual(rcTaskbar.left, 0, TASKBAR_X_TOLERANCE) ? ABE_LEFT : ABE_RIGHT;
		nBitmapSize = m_nBitmapWidth;
	}

	// We calculate the pixel increment and the timer value for the showing animation
	if (m_dwTimeToShow > m_dwTimerPrecision)
	{
		nEvents = min((m_dwTimeToShow / m_dwTimerPrecision) / 2, nBitmapSize); //<<-- enkeyDEV(Ottavio84) -Reduced frames of a half-
		//ADDED by VC-fengwen on 2007/10/08 <begin> : nEvents 有可能是0，则会出现异常。
		if (0 == nEvents)
			nEvents = 1;
		//ADDED by VC-fengwen on 2007/10/08 <end> : nEvents 有可能是0，则会出现异常。
		m_dwShowEvents = m_dwTimeToShow / nEvents;
		m_nIncrementShow = nBitmapSize / nEvents;
	}
	else
	{
		m_dwShowEvents = m_dwTimerPrecision;
		m_nIncrementShow = nBitmapSize;
	}

	// We calculate the pixel increment and the timer value for the hiding animation
	if (m_dwTimeToHide > m_dwTimerPrecision)
	{
		nEvents = min((m_dwTimeToHide / m_dwTimerPrecision / 2), nBitmapSize); //<<-- enkeyDEV(Ottavio84) -Reduced frames of a half-
		//ADDED by VC-fengwen on 2007/10/08 <begin> : nEvents 有可能是0，则会出现异常。
		if (0 == nEvents)
			nEvents = 1;
		//ADDED by VC-fengwen on 2007/10/08 <end> : nEvents 有可能是0，则会出现异常。
		m_dwHideEvents = m_dwTimeToHide / nEvents;
		m_nIncrementHide = nBitmapSize / nEvents;
	}
	else
	{
		m_dwShowEvents = m_dwTimerPrecision;
		m_nIncrementHide = nBitmapSize;
	}

	// Compute init values for the animation
	switch (m_nAnimStatus)
	{
		case IDT_HIDDEN:
			if (m_nTaskbarPlacement == ABE_RIGHT)
			{
				m_nCurrentPosX = rcTaskbar.left;
				m_nCurrentPosY = rcTaskbar.bottom - m_nBitmapHeight;
				m_nCurrentWidth = 0;
				m_nCurrentHeight = m_nBitmapHeight;
			}
			else if (m_nTaskbarPlacement == ABE_LEFT)
			{
				m_nCurrentPosX = rcTaskbar.right;
				m_nCurrentPosY = rcTaskbar.bottom - m_nBitmapHeight;
				m_nCurrentWidth = 0;
				m_nCurrentHeight = m_nBitmapHeight;
			}
			else if (m_nTaskbarPlacement == ABE_TOP)
			{
				m_nCurrentPosX = rcTaskbar.right - m_nBitmapWidth;
				m_nCurrentPosY = rcTaskbar.bottom;
				m_nCurrentWidth = m_nBitmapWidth;
				m_nCurrentHeight = 0;
			}
			else
			{
				// Taskbar is on the bottom or Invisible
				m_nCurrentPosX = rcTaskbar.right - m_nBitmapWidth;
				m_nCurrentPosY = rcTaskbar.top;
				m_nCurrentWidth = m_nBitmapWidth;
				m_nCurrentHeight = 0;
			}
			ShowWindow(SW_SHOWNOACTIVATE);
			SetTimer(IDT_APPEARING, m_dwShowEvents, NULL);
			break;

		case IDT_APPEARING:
			RedrawWindow();
			break;

		case IDT_WAITING:
			RedrawWindow();
			KillTimer(IDT_WAITING);
			SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
			break;

		case IDT_DISAPPEARING:
			KillTimer(IDT_DISAPPEARING);
			SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
			if (m_nTaskbarPlacement == ABE_RIGHT)
			{
				m_nCurrentPosX = rcTaskbar.left - m_nBitmapWidth;
				m_nCurrentWidth = m_nBitmapWidth;
			}
			else if (m_nTaskbarPlacement == ABE_LEFT)
			{
				m_nCurrentPosX = rcTaskbar.right;
				m_nCurrentWidth = m_nBitmapWidth;
			}
			else if (m_nTaskbarPlacement == ABE_TOP)
			{
				m_nCurrentPosY = rcTaskbar.bottom;
				m_nCurrentHeight = m_nBitmapHeight;
			}
			else
			{
				m_nCurrentPosY = rcTaskbar.top - m_nBitmapHeight;
				m_nCurrentHeight = m_nBitmapHeight;
			}

			SetWindowPos(NULL/*&wndTopMost*/, m_nCurrentPosX, m_nCurrentPosY, m_nCurrentWidth, m_nCurrentHeight, SWP_NOACTIVATE | SWP_NOZORDER);
			RedrawWindow();
			break;
	}
}

void CTaskbarNotifier::Hide()
{
	switch (m_nAnimStatus)
	{
		case IDT_APPEARING:
			KillTimer(IDT_APPEARING);
			break;
		case IDT_WAITING:
			KillTimer(IDT_WAITING);
			break;
		case IDT_DISAPPEARING:
			KillTimer(IDT_DISAPPEARING);
			break;
	}
	SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOACTIVATE);
	//MoveWindow(0, 0, 0, 0);
	ShowWindow(SW_HIDE);
	m_nAnimStatus = IDT_HIDDEN;
	m_nActiveMessageType = TBN_NULL;
}

HRGN CTaskbarNotifier::CreateRgnFromBitmap(HBITMAP hBmp, COLORREF color)
{
	if (!hBmp)
		return NULL;

	CDC* pDC = GetDC();
	if (!pDC)
		return NULL;

	BITMAP bm;
	GetObject( hBmp, sizeof(BITMAP), &bm ); // get bitmap attributes

	CDC dcBmp;
	dcBmp.CreateCompatibleDC(pDC);	//Creates a memory device context for the bitmap
	HGDIOBJ hOldBmp = dcBmp.SelectObject(hBmp);			//selects the bitmap in the device context

	const DWORD RDHDR = sizeof(RGNDATAHEADER);
	const DWORD MAXBUF = 40;		// size of one block in RECTs
									// (i.e. MAXBUF*sizeof(RECT) in bytes)
	LPRECT	pRects;
	DWORD	cBlocks = 0;			// number of allocated blocks

	INT		i, j;					// current position in mask image
	INT		first = 0;				// left position of current scan line
									// where mask was found
	bool	wasfirst = false;		// set when if mask was found in current scan line
	bool	ismask;					// set when current color is mask color

	// allocate memory for region data
	RGNDATAHEADER* pRgnData = (RGNDATAHEADER*)new BYTE[ RDHDR + ++cBlocks * MAXBUF * sizeof(RECT) ];
	memset( pRgnData, 0, RDHDR + cBlocks * MAXBUF * sizeof(RECT) );
	// fill it by default
	pRgnData->dwSize	= RDHDR;
	pRgnData->iType		= RDH_RECTANGLES;
	pRgnData->nCount	= 0;
	for ( i = 0; i < bm.bmHeight; i++ )
		for ( j = 0; j < bm.bmWidth; j++ ){
			// get color
			ismask=(dcBmp.GetPixel(j,bm.bmHeight-i-1)!=color);
			// place part of scan line as RECT region if transparent color found after mask color or
			// mask color found at the end of mask image
			if (wasfirst && ((ismask && (j==(bm.bmWidth-1)))||(ismask ^ (j<bm.bmWidth)))){
				// get offset to RECT array if RGNDATA buffer
				pRects = (LPRECT)((LPBYTE)pRgnData + RDHDR);
				// save current RECT
				pRects[ pRgnData->nCount++ ] = CRect( first, bm.bmHeight - i - 1, j+(j==(bm.bmWidth-1)), bm.bmHeight - i );
				// if buffer full reallocate it
				if ( pRgnData->nCount >= cBlocks * MAXBUF ){
					LPBYTE pRgnDataNew = new BYTE[ RDHDR + ++cBlocks * MAXBUF * sizeof(RECT) ];
					memcpy( pRgnDataNew, pRgnData, RDHDR + (cBlocks - 1) * MAXBUF * sizeof(RECT) );
					delete[] pRgnData;
					pRgnData = (RGNDATAHEADER*)pRgnDataNew;
				}
				wasfirst = false;
			} else if ( !wasfirst && ismask ){		// set wasfirst when mask is found
				first = j;
				wasfirst = true;
			}
		}

	dcBmp.SelectObject(hOldBmp);
	dcBmp.DeleteDC();	//release the bitmap
	// create region
	/*	Under WinNT the ExtCreateRegion returns NULL (by Fable@aramszu.net) */
	//	HRGN hRgn = ExtCreateRegion( NULL, RDHDR + pRgnData->nCount * sizeof(RECT), (LPRGNDATA)pRgnData );
	/* ExtCreateRegion replacement { */
	HRGN hRgn=CreateRectRgn(0, 0, 0, 0);
	ASSERT( hRgn!=NULL );
	pRects = (LPRECT)((LPBYTE)pRgnData + RDHDR);
	for(i=0;i<(int)pRgnData->nCount;i++)
	{
		HRGN hr=CreateRectRgn(pRects[i].left, pRects[i].top, pRects[i].right, pRects[i].bottom);
		VERIFY(CombineRgn(hRgn, hRgn, hr, RGN_OR)!=ERROR);
		if (hr) DeleteObject(hr);
	}
	ASSERT( hRgn!=NULL );
	/* } ExtCreateRegion replacement */

	delete[] pRgnData;
	ReleaseDC(pDC);
	return hRgn;
}

int CTaskbarNotifier::GetMessageType()
{
	return m_nActiveMessageType;
}

void CTaskbarNotifier::OnMouseMove(UINT nFlags, CPoint point)
{
	TRACKMOUSEEVENT t_MouseEvent;
	t_MouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
	t_MouseEvent.dwFlags = TME_LEAVE | TME_HOVER;
	t_MouseEvent.hwndTrack = m_hWnd;
	t_MouseEvent.dwHoverTime = 1;

	// We Tell Windows we want to receive WM_MOUSEHOVER and WM_MOUSELEAVE
	::_TrackMouseEvent(&t_MouseEvent);

	CWnd::OnMouseMove(nFlags, point);
}

void CTaskbarNotifier::OnLButtonUp(UINT nFlags, CPoint point)
{
	// close button clicked
	if (m_rcCloseBtn.PtInRect(point))
	{
		m_bAutoClose = TRUE;	// set true so next time arrive an autoclose event the popup will autoclose
								// (when m_bAutoClose is false a "true" event will be ignored until the user
								// manually close the windows)
		switch (m_nAnimStatus)
		{
		case IDT_APPEARING:
			KillTimer(IDT_APPEARING);
			break;
		case IDT_WAITING:
			KillTimer(IDT_WAITING);
			break;
		case IDT_DISAPPEARING:
			KillTimer(IDT_DISAPPEARING);
			break;
		}
		m_nAnimStatus = IDT_DISAPPEARING;
		SetTimer(IDT_DISAPPEARING, m_dwHideEvents, NULL);
		//Hide();
	}

	// cycle history button clicked
	if (m_rcHistoryBtn.PtInRect(point))
	{
		if (m_MessageHistory.GetCount() > 0)
		{
			CTaskbarNotifierHistory* messagePTR = (CTaskbarNotifierHistory*)m_MessageHistory.RemoveHead();
			Show(messagePTR->m_strMessage, messagePTR->m_nMessageType, messagePTR->m_strLink);
			delete messagePTR;
		}
	}

	// message clicked
	if (m_rcText.PtInRect(point))
	{
		// Notify the parent window that the Notifier popup was clicked
		LPCTSTR pszLink = m_strLink.IsEmpty() ? NULL : _tcsdup(m_strLink);
		theApp.emuledlg->PostMessage(UM_TASKBARNOTIFIERCLICKED, 0, (LPARAM)pszLink);
	}

	CWnd::OnLButtonUp(nFlags, point);
}

LRESULT CTaskbarNotifier::OnMouseHover(WPARAM /*wParam*/, LPARAM lParam)
{
	if (m_nAnimStatus == IDT_WAITING)
		KillTimer(IDT_WAITING);

	POINTS mp;
	mp = MAKEPOINTS(lParam);
	m_ptMousePosition.x = mp.x;
	m_ptMousePosition.y = mp.y;

	if (m_bMouseIsOver == FALSE)
	{
		m_bMouseIsOver = TRUE;
		RedrawWindow();
	}
	else if ((m_ptMousePosition.x >= m_rcText.left) && (m_ptMousePosition.x <= m_rcText.right)
			 && (m_ptMousePosition.y >= m_rcText.top) && (m_ptMousePosition.y <= m_rcText.bottom))
	{
		if (!m_bTextSelected)
			RedrawWindow();
	}
	else
	{
		if (m_bTextSelected)
			RedrawWindow();
	}

	return 0;
}

LRESULT CTaskbarNotifier::OnMouseLeave(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_bMouseIsOver == TRUE)
	{
		m_bMouseIsOver = FALSE;
		RedrawWindow();
		if (m_nAnimStatus == IDT_WAITING)
			SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
	}
	return 0;
}

BOOL CTaskbarNotifier::OnEraseBkgnd(CDC* pDC)
{
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap *pOldBitmap=memDC.SelectObject(&m_bitmapBackground);
	pDC->BitBlt(0, 0, m_nCurrentWidth, m_nCurrentHeight, &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBitmap);
	return TRUE;
}

void CTaskbarNotifier::OnPaint()
{
	CPaintDC dc(this);
	CFont* pOldFont;
	if (m_bMouseIsOver)
	{
		if ((m_ptMousePosition.x >= m_rcText.left) && (m_ptMousePosition.x <= m_rcText.right)
			&& (m_ptMousePosition.y >= m_rcText.top) && (m_ptMousePosition.y <= m_rcText.bottom))
		{
			m_bTextSelected = TRUE;
			dc.SetTextColor(m_crSelectedTextColor);
			pOldFont = dc.SelectObject(&m_mySelectedFont);
		}
		else
		{
			m_bTextSelected = FALSE;
			dc.SetTextColor(m_crNormalTextColor);
			pOldFont = dc.SelectObject(&m_myNormalFont);
		}
	}
	else
	{
		dc.SetTextColor(m_crNormalTextColor);
		pOldFont = dc.SelectObject(&m_myNormalFont);
	}

	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(m_strCaption, m_strCaption.GetLength(), m_rcText, m_uTextFormat);

	dc.SelectObject(pOldFont);
}

BOOL CTaskbarNotifier::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT)
	{
		if (m_rcCloseBtn.PtInRect(m_ptMousePosition) ||
			m_rcHistoryBtn.PtInRect(m_ptMousePosition) ||
			m_rcText.PtInRect(m_ptMousePosition))
		{
			::SetCursor(m_hCursor);
			return TRUE;
		}
	}
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CTaskbarNotifier::OnTimer(UINT nIDEvent)
{
	switch (nIDEvent)
	{
		case IDT_APPEARING:
			m_nAnimStatus = IDT_APPEARING;
			switch (m_nTaskbarPlacement)
			{
				case ABE_BOTTOM:
					if (m_nCurrentHeight < m_nBitmapHeight)
					{
						m_nCurrentPosY -= m_nIncrementShow;
						m_nCurrentHeight += m_nIncrementShow;
					}
					else
					{
						KillTimer(IDT_APPEARING);
						SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
						m_nAnimStatus = IDT_WAITING;
					}
					break;

				case ABE_TOP:
					if (m_nCurrentHeight < m_nBitmapHeight)
						m_nCurrentHeight += m_nIncrementShow;
					else
					{
						KillTimer(IDT_APPEARING);
						SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
						m_nAnimStatus = IDT_WAITING;
					}
					break;

				case ABE_LEFT:
					if (m_nCurrentWidth < m_nBitmapWidth)
						m_nCurrentWidth += m_nIncrementShow;
					else
					{
						KillTimer(IDT_APPEARING);
						SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
						m_nAnimStatus = IDT_WAITING;
					}
					break;

				case ABE_RIGHT:
					if (m_nCurrentWidth < m_nBitmapWidth)
					{
						m_nCurrentPosX -= m_nIncrementShow;
						m_nCurrentWidth += m_nIncrementShow;
					}
					else
					{
						KillTimer(IDT_APPEARING);
						SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
						m_nAnimStatus = IDT_WAITING;
					}
					break;
			}
			SetWindowPos(NULL/*&wndTopMost*/, m_nCurrentPosX, m_nCurrentPosY, m_nCurrentWidth, m_nCurrentHeight, SWP_NOACTIVATE | SWP_NOZORDER);
			break;

		case IDT_WAITING:
			KillTimer(IDT_WAITING);
			if (m_bAutoClose)
				SetTimer(IDT_DISAPPEARING, m_dwHideEvents, NULL);
			break;

		case IDT_DISAPPEARING:
			m_nAnimStatus = IDT_DISAPPEARING;
			switch (m_nTaskbarPlacement)
			{
				case ABE_BOTTOM:
					if (m_nCurrentHeight > 0)
					{
						m_nCurrentPosY += m_nIncrementHide;
						m_nCurrentHeight -= m_nIncrementHide;
					}
					else
					{
						KillTimer(IDT_DISAPPEARING);
						Hide();						
					}
					break;

				case ABE_TOP:
					if (m_nCurrentHeight > 0)
						m_nCurrentHeight -= m_nIncrementHide;
					else
					{
						KillTimer(IDT_DISAPPEARING);
						Hide();
					}
					break;

				case ABE_LEFT:
					if (m_nCurrentWidth > 0)
						m_nCurrentWidth -= m_nIncrementHide;
					else
					{
						KillTimer(IDT_DISAPPEARING);
						Hide();
					}
					break;

				case ABE_RIGHT:
					if (m_nCurrentWidth > 0)
					{					 
						m_nCurrentPosX += m_nIncrementHide;
						m_nCurrentWidth -= m_nIncrementHide;
					}
					else
					{
						KillTimer(IDT_DISAPPEARING);
						Hide();
					}
					break;
			}
			SetWindowPos(NULL/*&wndTopMost*/, m_nCurrentPosX, m_nCurrentPosY, m_nCurrentWidth, m_nCurrentHeight, SWP_NOACTIVATE | SWP_NOZORDER);
			break;
	}

	CWnd::OnTimer(nIDEvent);
}
