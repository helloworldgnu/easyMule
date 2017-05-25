/* 
 * $Id: SplashScreen.cpp 9073 2008-12-18 04:38:51Z dgkang $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "stdafx.h"
#include "emule.h"
#include "SplashScreen.h"
#include "OtherFunctions.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CSplashScreen, CDialog)

BEGIN_MESSAGE_MAP(CSplashScreen, CDialog)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CSplashScreen::CSplashScreen(CWnd* pParent /*=NULL*/)
	: CDialog(CSplashScreen::IDD, pParent)
{
	m_image = NULL;

	TCHAR szAppDir[MAX_PATH];       //应用程序路径
	VERIFY( GetModuleFileName(theApp.m_hInstance, szAppDir, ARRSIZE(szAppDir)));
	VERIFY( PathRemoveFileSpec(szAppDir));//去除了"/eMule.exe", 只剩路径

	TCHAR szPrefFilePath[MAX_PATH];            //SplashScreen文件路径
	PathCombine(szPrefFilePath, szAppDir, _T("Splashbg.png"));
	m_image = new CxImage(szPrefFilePath, CXIMAGE_FORMAT_PNG); // Added by thilon on 2006.08.01
}

CSplashScreen::~CSplashScreen()
{
	delete m_image;
}

void CSplashScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CSplashScreen::OnInitDialog()
{

	if (!m_image->IsValid())
		return FALSE;

#ifdef _DEBUG	//ADDED by fengwen on 2006/11/22 : 调试时不让SplashWnd永远置前，妨碍调试。
	SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#endif

	HDC hScrDC;						// 屏幕设备描述表
	hScrDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	
	int xScrn, yScrn;				// 屏幕分辨率

	xScrn = GetDeviceCaps(hScrDC, HORZRES);
	yScrn = GetDeviceCaps(hScrDC, VERTRES);

	CDialog::OnInitDialog();
	
	//InitWindowStyles(this);

	WINDOWPLACEMENT wp;
	GetWindowPlacement(&wp);
	wp.rcNormalPosition.left = 0;
	wp.rcNormalPosition.top = 0;
	wp.rcNormalPosition.right = xScrn;
	wp.rcNormalPosition.bottom = yScrn;

	SetWindowPlacement(&wp);


	int x, y;
	x = ( xScrn - m_image->GetWidth() ) / 2;
	y = ( yScrn - m_image->GetHeight() ) / 2;

	m_rtImg.SetRect(x, y, xScrn - x, yScrn - y);
	m_rtVersion.SetRect(m_rtImg.left + 198, m_rtImg.top + 293, m_rtImg.left + 253, m_rtImg.top + 311);
	m_rtText.SetRect(m_rtImg.left + 37, m_rtImg.top + 308, m_rtImg.left + 241, m_rtImg.top + 322);


	return TRUE;
}

BOOL CSplashScreen::PreTranslateMessage(MSG* pMsg)
{
	BOOL bResult = CDialog::PreTranslateMessage(pMsg);

	if ((pMsg->message == WM_KEYDOWN	   ||
		 pMsg->message == WM_SYSKEYDOWN	   ||
		 pMsg->message == WM_LBUTTONDOWN   ||
		 pMsg->message == WM_RBUTTONDOWN   ||
		 pMsg->message == WM_MBUTTONDOWN   ||
		 pMsg->message == WM_NCLBUTTONDOWN ||
		 pMsg->message == WM_NCRBUTTONDOWN ||
		 pMsg->message == WM_NCMBUTTONDOWN))
	{
		DestroyWindow();
	}

	return bResult;
}

void CSplashScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CDC dcMem;

	GetWindowRect(&rect);//获得要抓图的区域
	m_hbm = CopyScreenToBitmap(rect);//抓图
	GetObject(m_hbm, sizeof(m_bitmap),&m_bitmap);//获得抓到的图的尺寸

	if (dcMem.CreateCompatibleDC(&dc))
	{
		SelectObject(dcMem, m_hbm);
		BitBlt(dc,0,0,m_bitmap.bmWidth, m_bitmap.bmHeight, dcMem,0 ,0, SRCCOPY);
			
		//m_image->Draw(dc.m_hDC,m_bitmap.bmWidth/2 - 380/2,m_bitmap.bmHeight/2 - 350/2,380,350);
		m_image->Draw(dc.m_hDC, m_rtImg.left, m_rtImg.top, -1, -1);

		DrawText(&dc);

		//CRect rc(0, m_bitmap.bmHeight/2 + 70, m_bitmap.bmWidth ,m_bitmap.bmHeight);

		//LOGFONT lf = {0};
		//lf.lfHeight = 26;
		//lf.lfWeight = FW_BOLD;
		//lf.lfQuality = afxData.bWin95 ? NONANTIALIASED_QUALITY : ANTIALIASED_QUALITY;
		//_tcscpy(lf.lfFaceName, _T("Arial"));
		//	
		//CFont font;
		//font.CreateFontIndirect(&lf);
		//CFont* pOldFont = dc.SelectObject(&font);

		//CString strAppVersion;
		//strAppVersion.Format(_T("eMule %u.%u%c VeryCD"), CGlobalVariable::m_nVersionMjr, CGlobalVariable::m_nVersionMin, _T('a') + CGlobalVariable::m_nVersionUpd);
		//	
		//rc.top += 30;
		//rc.top += dc.DrawText(strAppVersion, &rc, DT_CENTER | DT_NOPREFIX);

		//if (pOldFont)
		//{
		//	dc.SelectObject(pOldFont);
		//}
		//font.DeleteObject();

		//rc.top += 8;

		//lf.lfHeight = 14;
		//lf.lfWeight = FW_NORMAL;
		//lf.lfQuality = afxData.bWin95 ? NONANTIALIASED_QUALITY : ANTIALIASED_QUALITY;
		//_tcscpy(lf.lfFaceName, _T("Arial"));
		//font.CreateFontIndirect(&lf);
		//pOldFont = dc.SelectObject(&font);
		//dc.DrawText(_T("Copyright (C) 2002-2006 Merkur"), &rc, DT_CENTER | DT_NOPREFIX);

		//if (pOldFont)
		//{
		//	dc.SelectObject(pOldFont);
		//}
		//font.DeleteObject();
	}
}

/****************************************************************************
                          
函数名:
       CopyScreenToBitmap(CRect& rect)

函数功能:
      	抓取屏幕并转换成位图

被本函数调用的函数清单:
      		
调用本函数的函数清单:
		void CSplashScreen::OnPaint()
      
参数:
      CRect&				rect 

返回值:
	  HBITMAP
描述:
  
****************************************************************************/

HBITMAP CSplashScreen::CopyScreenToBitmap(CRect& rect/*lpRect 代表选定区域*/)
{
	HDC hScrDC;						// 屏幕设备描述表
	HDC	hMemDC;						// 内存设备描述表

	HBITMAP hBitmap, hOldBitmap;	// 位图句柄

	int nX, nY, nX2, nY2;           // 选定区域坐标
	
	int nWidth, nHeight;			// 位图宽度和高度

	int xScrn, yScrn;				// 屏幕分辨率

	// 确保选定区域不为空矩形
	//if (IsRectEmpty(lpRect))
	//{
	//	return NULL;
	//}

	//为屏幕创建设备描述表
	hScrDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	//为屏幕设备描述表创建兼容的内存设备描述表
	hMemDC = CreateCompatibleDC(hScrDC);
	
	// 获得选定区域坐标
	//nX = lpRect -> left;
	//nY = lpRect -> top;
	//nX2 = lpRect -> right;
	//nY2 = lpRect -> bottom;

	nX = rect.left;
	nY = rect.top;
	nX2 = rect.right;
	nY2 = rect.bottom;

	// 获得屏幕分辨率
	xScrn = GetDeviceCaps(hScrDC, HORZRES);
	yScrn = GetDeviceCaps(hScrDC, VERTRES);
	
	//nX  = (xScrn - rect.Width())/2;
	//nY  = (yScrn - rect.Height())/2;
	//nX2 = xScrn - nX + rect.Width();
	//nY2 = yScrn - nY + rect.Height();
	//确保选定区域是可见的

	if (nX<0)
		nX = 0;
	if (nY<0)
		nY = 0;
	if (nX2 > xScrn)
		nX2 = xScrn;
	if (nY2 > yScrn)
		nY2 = yScrn;
	nWidth = nX2 - nX;
	nHeight = nY2 - nY;
	
	// 创建一个与屏幕设备描述表兼容的位图
	hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);
	// 把新位图选到内存设备描述表中
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	// 把屏幕设备描述表拷贝到内存设备描述表中
	BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY);
	//得到屏幕位图的句柄
	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);
	//清除
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);
	 //返回位图句柄
	return hBitmap;
}
BOOL CSplashScreen::OnEraseBkgnd(CDC* /*pDC*/)
{
	return true;
}

void CSplashScreen::DrawText(CDC *pDC)
{
	LOGFONT lf = {0};
	CFont font;
	CFont* pOldFont;
	CTextDC	textDC(pDC->GetSafeHdc(), RGB(255, 255, 255));

	CString strVer;
	strVer.Format(_T("V%u.%u.%u"), CGlobalVariable::m_nEasyMuleMjr, CGlobalVariable::m_nEasyMuleMin, CGlobalVariable::m_nEasyMuleUpd);
	
	lf.lfHeight = 18;
	lf.lfWeight = FW_BOLD;
	lf.lfQuality = ANTIALIASED_QUALITY;
	_tcscpy(lf.lfFaceName, _T("Arial"));
	font.CreateFontIndirect(&lf);
	pOldFont = pDC->SelectObject(&font);
	pDC->DrawText(strVer, &m_rtVersion, DT_CENTER | DT_NOPREFIX);
	pDC->SelectObject(pOldFont);
	font.DeleteObject();

	lf.lfHeight = 12;
	lf.lfWeight = FW_NORMAL;
	lf.lfQuality = ANTIALIASED_QUALITY;
	_tcscpy(lf.lfFaceName, _T("Arial"));
	font.CreateFontIndirect(&lf);
	pOldFont = pDC->SelectObject(&font);
	pDC->DrawText(_T("Copyright (C) 2002-2008 VeryCD"), &m_rtText, DT_CENTER | DT_NOPREFIX);
	pDC->SelectObject(pOldFont);
	font.DeleteObject();
}
