/*
 * $Id: MenuXP.cpp 4483 2008-01-02 09:19:06Z soarchin $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2008 VeryCD Dev Team ( strEmail.Format("%s@%s", "emuledev", "verycd.com") / http: * www.easymule.org )
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
// MenuXP.cpp: implementation of the CMenuXP class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MenuXP.h"
#include "emule.h"
#include "CIF.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CM_DISABLEDBLEND ILD_BLEND25
#define CM_ICONWIDTH		16
#define CM_ICONHEIGHT		16
//#define ID_SEPARATOR       0


//IMPLEMENT_DYNAMIC( CMenuXP, CMenu )
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMenuXP MenuXP;

CMenuXP::CMenuXP()
{
		cif.CreateFonts();
		m_nCheckIcon	= 0;
		m_bEnable		= TRUE;
		m_bUnhook		= FALSE;
}

CMenuXP::~CMenuXP()
{
		SetWatermark( NULL );
		if ( m_bUnhook ) EnableHook( FALSE );
}

/****************************************************************************
                          
函数名:
       MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)

函数功能:
      	获得菜单项的详细尺寸,同时定义了分隔条和菜单项的长和宽

被本函数调用的函数清单:
      		
调用本函数的函数清单:
      
参数:
     LPMEASUREITEMSTRUCT lpMeasureItemStruct 指向MEASUREITEMSTRUCT结构的指针

	typedef struct tagMEASUREITEMSTRUCT {   // mis 
				UINT  CtlType;      // type of control 
				UINT  CtlID;        // combo box, list box, or button identifier 
				UINT  itemID;       // menu item, variable-height list box, or combo box identifier
				UINT  itemWidth;    // width of menu item, in pixels 
				UINT  itemHeight;   // height of single item in list box menu, in pixels 
				DWORD itemData;     // application-defined 32-bit value 
	} MEASUREITEMSTRUCT;

返回值:
	    void
内容:
  
****************************************************************************/
void CMenuXP::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
		if(lpMeasureItemStruct->itemID == ID_SEPARATOR)    //#define ID_SEPARATOR     0
		{
				lpMeasureItemStruct->itemWidth	= 16;    //分隔条宽度
				lpMeasureItemStruct->itemHeight	= 2;     //分隔条高度
		}
		else
        {
				CString		strText;
				CDC         dc;

				m_pStrings.Lookup(static_cast<DWORD>(lpMeasureItemStruct->itemData), strText);

				dc.Attach(GetDC(0));

				CFont* pOld = (CFont*)dc.SelectObject( &cif.m_fntUP );
				CSize sz = dc.GetTextExtent( strText );
				dc.SelectObject( pOld );
		
				ReleaseDC( 0, dc.Detach() );
		
				lpMeasureItemStruct->itemWidth	= sz.cx + 32;
				lpMeasureItemStruct->itemHeight	= 23;
		}

}

/****************************************************************************
                          
函数名:
       DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)

函数功能:
      	绘制菜单项

被本函数调用的函数清单:
      		
调用本函数的函数清单:
      
参数:
     LPDRAWITEMSTRUCT lpDrawItemStruct 指向DRAWITEMSTRUCT结构的指针

	typedef struct tagDRAWITEMSTRUCT {  // dis 
				UINT  CtlType; 
				UINT  CtlID; 
				UINT  itemID; 
				UINT  itemAction; 
				UINT  itemState; 
				HWND  hwndItem; 
				HDC   hDC; 
				RECT  rcItem; 
				DWORD itemData; 
	} DRAWITEMSTRUCT; 


返回值:
	    void
内容:
  
****************************************************************************/
void CMenuXP::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	
		CRect rcItem, rcText;
		CString strText;
		int nIcon = -1;
		CDC dc;
		
	    //菜单的几种状态
		BOOL	bSelected	= lpDrawItemStruct->itemState & ODS_SELECTED;//选中
		BOOL	bChecked	= lpDrawItemStruct->itemState & ODS_CHECKED;//复选
		BOOL	bDisabled	= lpDrawItemStruct->itemState & ODS_GRAYED;//不可用
		BOOL	bKeyboard	= FALSE;//键盘
		BOOL	bEdge		= TRUE;
	
		dc.Attach( lpDrawItemStruct->hDC );//得到要绘制的菜单

		if (CWnd* pWnd = dc.GetWindow())
		{
				CRect rcScreen( &lpDrawItemStruct->rcItem );
				CPoint ptCursor;
		
				GetCursorPos( &ptCursor );
				pWnd->ClientToScreen( &rcScreen );
		}
		
		rcItem.CopyRect( &lpDrawItemStruct->rcItem );
		rcItem.OffsetRect( -rcItem.left, -rcItem.top );

		rcText.CopyRect( &rcItem );
		rcText.left += 32;//菜单项中文本的位置
		rcText.right -= 2;

		CDC* pDC = cif.GetBuffer( dc, rcItem.Size() );

		if ( m_bmWatermark.m_hObject != NULL )
		{
				DrawWatermark( pDC, &rcItem, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top );
		}
		else
		{
				pDC->FillSolidRect( rcItem.left, rcItem.top, 24, rcItem.Height(), cif.m_crMargin );
				pDC->FillSolidRect( rcItem.left + 24, rcItem.top, rcItem.Width() - 24, rcItem.Height(), cif.m_crBackNormal );
		}
		//判断分隔条(若菜单项数据不是相应字符串则认为是分隔条)
		if ( m_pStrings.Lookup( static_cast<DWORD>(lpDrawItemStruct->itemData), strText ) == FALSE )
		{
				int nMiddle = rcText.top + 1;//分割条的位置: + 水平向上移动; - 水平向下移动
		
				pDC->FillSolidRect( rcText.left, nMiddle, rcText.Width() + 2, 1, cif.m_crDisabled );//绘制分割条
		
				dc.BitBlt( lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top,rcItem.Width(), rcItem.Height(), pDC, 0, 0, SRCCOPY );
				dc.Detach();
		
				return;
		}

		if ( bSelected )//选中后是否可用
		{
				if ( ! bDisabled )
				{       //绘制边框
						pDC->Draw3dRect(
								rcItem.left + 1, //距离左边的位置
								rcItem.top + 1,
								rcItem.Width() - 2, 
								rcItem.Height() - 1,
								cif.m_crBorder,
								cif.m_crBorder
						);
						//填充背景色
						pDC->FillSolidRect(
								rcItem.left + 2,
								rcItem.top + 2,
								rcItem.Width() - 4,
								rcItem.Height() - 2 - bEdge,
								cif.m_crBackSel
						);
			
						pDC->SetBkColor( cif.m_crBackSel );
				}
				else if ( bKeyboard )
				{
						pDC->Draw3dRect( rcItem.left + 1, rcItem.top + 1,
						rcItem.Width() - 2, rcItem.Height() - 1 - bEdge,
						cif.m_crBorder, cif.m_crBorder );
						pDC->FillSolidRect( rcItem.left + 2, rcItem.top + 2,
						rcItem.Width() - 4, rcItem.Height() - 3 - bEdge,
						cif.m_crBackNormal );
			
						pDC->SetBkColor( cif.m_crBackNormal );
				}
		}
		else
		{
				pDC->SetBkColor( cif.m_crBackNormal );
		}
	
		if ( bChecked )
		{
				pDC->Draw3dRect( rcItem.left + 2, rcItem.top + 2, 20, rcItem.Height() - 2 - bEdge, cif.m_crBorder, cif.m_crBorder );
				pDC->FillSolidRect( rcItem.left + 3, rcItem.top + 3, 18, rcItem.Height() - 4 - bEdge, ( bSelected && !bDisabled ) ? cif.m_crBackCheckSel : cif.m_crBackCheck );
		}
	
		//绘制菜单图标

		nIcon = cif.ImageForID( (DWORD)lpDrawItemStruct->itemID );//=0

		/*if ( bChecked && nIcon < 0 ) nIcon = m_nCheckIcon;*///原来的格式, 现修改成如下格式
		
		if ( bChecked && nIcon < 0 ) 
		{
			nIcon = m_nCheckIcon;
		}
	
		if ( nIcon >= 0 )
		{
			CPoint pt( rcItem.left + 4, rcItem.top + 4 );//图标位置
		
			if ( bDisabled )//当菜单项不可用时
			{
				ImageList_DrawEx( cif.m_GrayImageList.m_hImageList, cif.GrayImageForID((DWORD)lpDrawItemStruct->itemID), pDC->GetSafeHdc(), pt.x, pt.y, 0, 0, CLR_NONE, cif.m_crDisabled, CM_DISABLEDBLEND );
			}
			else
			{
				if ( bChecked )//当菜单项是复选时
				{
					cif.m_ImageList.Draw( pDC, nIcon, pt, ILD_NORMAL );
				}
				else
				{
					if ( bSelected )//当菜单项选中时
					{
						pt.Offset( 1, 1 );
						pDC->SetTextColor( cif.m_crShadow );
						cif.m_ImageList.Draw( pDC, nIcon, pt, ILD_MASK );//先绘制图标的阴影
						pt.Offset( -2, -2 );//向左上方移动2个像素
						cif.m_ImageList.Draw( pDC, nIcon, pt, ILD_NORMAL );//再绘制图标
					}
					else
					{
						ImageList_DrawEx( cif.m_ImageList.m_hImageList, nIcon, pDC->GetSafeHdc(),pt.x, pt.y, 0, 0, CLR_NONE, cif.m_crMargin, ILD_BLEND25 );
					}
				}
			}
		}
	
		CFont* pOld = (CFont*)pDC->SelectObject(( lpDrawItemStruct->itemState & ODS_DEFAULT ) && ! bDisabled ? &cif.m_fntBold : &cif.m_fntUP );
	
		pDC->SetBkMode( TRANSPARENT );
		pDC->SetTextColor( bDisabled ? cif.m_crDisabled :( bSelected ? cif.m_crCmdTextSel : cif.m_crCmdText ) );
		DrawMenuText( pDC, &rcText, strText );
	
		pDC->SelectObject( pOld );
	
		dc.BitBlt( lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top,
		rcItem.Width(), rcItem.Height(), pDC, 0, 0, SRCCOPY );

		dc.Detach();
}

/****************************************************************************
                          
函数名:
       DrawMenuText(CDC *pDC, CRect *pRect, const CString &strText)

函数功能:
      	绘制菜单项文本

被本函数调用的函数清单:
      		
调用本函数的函数清单:
					  CMenuXP::DrawItem
      
参数:
     CDC				*pDC 
	 CRect				*pRect
	 const CString		&strText

返回值:
	    void
内容:
  
****************************************************************************/

void CMenuXP::DrawMenuText(CDC *pDC, CRect *pRect, const CString &strText)
{
int nPos = strText.Find( '\t' );// '/t'索引号
	
	if ( nPos >= 0 )
	{
		pRect->right -= 8;
		pDC->DrawText( strText.Left( nPos ), pRect, DT_SINGLELINE|DT_VCENTER|DT_LEFT );//绘制文本，居中，左对齐
		pDC->DrawText( strText.Mid( nPos + 1 ), pRect, DT_SINGLELINE|DT_VCENTER|DT_RIGHT );//绘制文本，右对齐
		pRect->right += 8;
	}
	else
	{
		pDC->DrawText( strText, pRect, DT_SINGLELINE|DT_VCENTER|DT_LEFT );
	}
}

BOOL CMenuXP::AddMenu(CMenu *pMenu, BOOL bChild)
{
	
	if (  !m_bEnable )
	{
			return FALSE;
	}

	for ( int i = 0 ; i < (int)pMenu->GetMenuItemCount() ; i++ )//遍历菜单项
	{
		TCHAR szBuffer[128];
		MENUITEMINFO mii;//菜单项信息表
		//填充菜单项信息表
		ZeroMemory( &mii, sizeof(mii) ); //填充零（初始化菜单项信息表）
		mii.cbSize		= sizeof(mii);//菜单项信息表大小
		
		//菜单项掩码（此处为：需要设置dwItemData、wID、fType and dwTypeData、hSubMenu项）
		mii.fMask		= MIIM_DATA|MIIM_ID|MIIM_TYPE|MIIM_SUBMENU;
		
		mii.dwTypeData	= szBuffer;//菜单项文本
		mii.cch			= 128;//菜单项文本长度
		
		//得到包装的菜单项信息
		GetMenuItemInfo( pMenu->GetSafeHmenu(), i, MF_BYPOSITION, &mii );
		
		if ( mii.fType & (MF_OWNERDRAW|MF_SEPARATOR) )
		{
			mii.fType |= MF_OWNERDRAW;
			if ( mii.fType & MF_SEPARATOR ) mii.dwItemData = 0;
			SetMenuItemInfo( pMenu->GetSafeHmenu(), i, MF_BYPOSITION, &mii );
			continue;
		}
		
		mii.fType		|= MF_OWNERDRAW;
		mii.dwItemData	 = ( (DWORD)pMenu->GetSafeHmenu() << 16 ) | ( mii.wID & 0xFFFF );
		
		CString strText = szBuffer;
		m_pStrings.SetAt(static_cast<DWORD>(mii.dwItemData), strText );
		
		if ( bChild ) SetMenuItemInfo( pMenu->GetSafeHmenu(), i, MF_BYPOSITION, &mii );
		
		if ( mii.hSubMenu != NULL ) AddMenu( pMenu->GetSubMenu( i ), TRUE );
	}
	return TRUE;
}

void CMenuXP::DrawWatermark(CDC *pDC, CRect *pRect, int nOffX, int nOffY)
{
	for ( int nY = pRect->top - nOffY ; nY < pRect->bottom ; nY += m_czWatermark.cy )
	{
		if ( nY + m_czWatermark.cy < pRect->top ) continue;
		
		for ( int nX = pRect->left - nOffX ; nX < pRect->right ; nX += m_czWatermark.cx )
		{
			if ( nX + m_czWatermark.cx < pRect->left ) continue;
			
			pDC->BitBlt( nX, nY, m_czWatermark.cx, m_czWatermark.cy, &m_dcWatermark, 0, 0, SRCCOPY );
		}
	}
}

void CMenuXP::SetWatermark(HBITMAP hBitmap)
{
if ( m_bmWatermark.m_hObject != NULL )
	{
		m_dcWatermark.SelectObject( CBitmap::FromHandle( m_hOldMark ) );
		m_bmWatermark.DeleteObject();
		m_dcWatermark.DeleteDC();
	}
	
	if ( hBitmap != NULL )
	{
		CDC dc;		  
		dc.Attach( GetDC( 0 ) );
		m_dcWatermark.CreateCompatibleDC( &dc );
		ReleaseDC( 0, dc.Detach() );
		
		m_bmWatermark.Attach( hBitmap );
		m_hOldMark = (HBITMAP)m_dcWatermark.SelectObject( &m_bmWatermark )->GetSafeHandle();
		
		BITMAP pInfo;
		m_bmWatermark.GetBitmap( &pInfo );
		m_czWatermark.cx = pInfo.bmWidth;
		m_czWatermark.cy = pInfo.bmHeight;
	}
}

LPCTSTR CMenuXP::wpnOldProc		= _T("BLINKCD_MenuOldWndProc");
BOOL	CMenuXP::m_bPrinted		= TRUE;
HHOOK	CMenuXP::m_hMsgHook		= NULL;
int		CMenuXP::m_nEdgeLeft	= 0;
int		CMenuXP::m_nEdgeTop		= 0;
int		CMenuXP::m_nEdgeSize	= 0;

void CMenuXP::RegisterEdge(int nLeft, int nTop, int nLength)
{
		m_nEdgeLeft	= nLeft;
		m_nEdgeTop	= nTop;
		m_nEdgeSize	= nLength;
}

LRESULT CMenuXP::MenuProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
		WNDPROC pWndProc = (WNDPROC)::GetProp( hWnd, wpnOldProc );
	
	switch ( uMsg )
	{
	case WM_NCCALCSIZE:
		{
			NCCALCSIZE_PARAMS* pCalc = (NCCALCSIZE_PARAMS*)lParam;
			pCalc->rgrc[0].left ++;
			pCalc->rgrc[0].top ++;
			pCalc->rgrc[0].right --;
			pCalc->rgrc[0].bottom --;
		}
		return 0;
		
	case WM_WINDOWPOSCHANGING:
		if ( WINDOWPOS* pWndPos = (WINDOWPOS*)lParam )
		{
			DWORD nStyle	= GetWindowLong( hWnd, GWL_STYLE );
			DWORD nExStyle	= GetWindowLong( hWnd, GWL_EXSTYLE );
			CRect rc( 0, 0, 32, 32 );
			
			AdjustWindowRectEx( &rc, nStyle, FALSE, nExStyle );
			
			pWndPos->cx -= ( rc.Width() - 34 );
			pWndPos->cy -= ( rc.Height() - 34 ) - 1;
			
			if ( pWndPos->x != m_nEdgeLeft || pWndPos->y != m_nEdgeTop )
				pWndPos->x ++;
		}
		break;
		
	case WM_PRINT:
		if ( ( lParam & PRF_CHECKVISIBLE ) && ! IsWindowVisible( hWnd ) ) return 0;
		if ( lParam & PRF_NONCLIENT )
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CDC* pDC = CDC::FromHandle( (HDC)wParam );
			CRect rc;
			
			pWnd->GetWindowRect( &rc );
			BOOL bEdge = ( rc.left == m_nEdgeLeft && rc.top == m_nEdgeTop );
			rc.OffsetRect( -rc.left, -rc.top );
			
			pDC->Draw3dRect( &rc, cif.m_crDisabled, cif.m_crDisabled );
			if ( bEdge ) pDC->FillSolidRect( rc.left + 1, rc.top, min( rc.Width(), m_nEdgeSize ) - 2, 1, cif.m_crBackNormal );
		}
		if ( lParam & PRF_CLIENT )
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CDC* pDC = CDC::FromHandle( (HDC)wParam );
			CBitmap bmBuf, *pbmOld;
			CDC dcBuf;
			CRect rc;
			
			pWnd->GetClientRect( &rc );
			dcBuf.CreateCompatibleDC( pDC );
			bmBuf.CreateCompatibleBitmap( pDC, rc.Width(), rc.Height() );
			pbmOld = (CBitmap*)dcBuf.SelectObject( &bmBuf );
			
			m_bPrinted = TRUE;
			dcBuf.FillSolidRect( &rc, GetSysColor( COLOR_MENU ) );
			SendMessage( hWnd, WM_PRINTCLIENT, (WPARAM)dcBuf.GetSafeHdc(), 0 );
			
			pDC->BitBlt( 1, 1, rc.Width(), rc.Height(), &dcBuf, 0, 0, SRCCOPY );
			dcBuf.SelectObject( pbmOld );
		}
		return 0;
		
	case WM_NCPAINT:
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CWindowDC dc( pWnd );
			CRect rc;
			
			pWnd->GetWindowRect( &rc );
			BOOL bEdge = ( rc.left == m_nEdgeLeft && rc.top == m_nEdgeTop );
			rc.OffsetRect( -rc.left, -rc.top );
			
			dc.Draw3dRect( &rc,cif.m_crDisabled, cif.m_crDisabled );
			if ( bEdge ) dc.FillSolidRect( rc.left + 1, rc.top, min( rc.Width(), m_nEdgeSize ) - 2, 1, cif.m_crBackNormal );
		}
		return 0;
		
	case WM_PAINT:
		m_bPrinted = FALSE;
		break;
		
	case WM_NCDESTROY:
		::RemoveProp( hWnd, wpnOldProc );
		break;
	}
	
	return CallWindowProc( pWndProc, hWnd, uMsg, wParam, lParam );
}

LRESULT CALLBACK CMenuXP::MsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT* pCWP = (CWPSTRUCT*)lParam;
	
	while ( nCode == HC_ACTION )
	{
		if ( pCWP->message != WM_CREATE && pCWP->message != 0x01E2 ) break;
		
		TCHAR szClassName[16];
		int nClassName = GetClassName( pCWP->hwnd, szClassName, 16 );
		if ( nClassName != 6 || _tcscmp( szClassName, _T("#32768") ) != 0 ) break;
		
		if ( ::GetProp( pCWP->hwnd, wpnOldProc ) != NULL ) break;
		
		HWND hWndFore = GetForegroundWindow();
		if ( hWndFore != NULL && CWnd::FromHandlePermanent( hWndFore ) == NULL ) break;
		
		WNDPROC pWndProc = (WNDPROC)::GetWindowLong( pCWP->hwnd, GWL_WNDPROC );
		if ( pWndProc == NULL ) break;
		ASSERT( pWndProc != MenuProc );
		
		if ( ! SetProp( pCWP->hwnd, wpnOldProc, pWndProc ) ) break;
		
		if ( ! SetWindowLong( pCWP->hwnd, GWL_WNDPROC, (DWORD)MenuProc ) )
		{
			::RemoveProp( pCWP->hwnd, wpnOldProc );
			break;
		}
		
		break;
	}
	
	return CallNextHookEx( CMenuXP::m_hMsgHook, nCode, wParam, lParam );

}

void CMenuXP::EnableHook(BOOL bEnable)
{
if ( bEnable == ( m_hMsgHook != NULL ) ) return;
	
	if ( bEnable )
	{
		m_hMsgHook = SetWindowsHookEx( 
			WH_CALLWNDPROC,				//系统将消息发送到指定窗口之前的"钩子"
			MsgHook,					//钩子处理函数
			AfxGetInstanceHandle(),		//钩子处理函数所处模块的句柄
			GetCurrentThreadId()        //指定被监视的线程
		);
	}
	else
	{
		UnhookWindowsHookEx( m_hMsgHook );
		m_hMsgHook = NULL;
	}
}

void CMenuXP::EnableHook()
{
	ASSERT( m_hMsgHook == NULL );
	ASSERT( m_bUnhook == FALSE );
	m_bEnable		= TRUE;
	
	m_bUnhook = TRUE;
	EnableHook( TRUE );
}

UINT CMenuXP::TrackPopupMenu(CMenu* pszMenu, const CPoint& point, UINT nDefaultID, UINT nFlags, CWnd* pWnd)
{
	CMenu* pPopup = pszMenu;
	if ( pPopup == NULL ) return 0;
	
	if ( nDefaultID != 0 )
	{
		MENUITEMINFO pInfo;
		pInfo.cbSize	= sizeof(pInfo);
		pInfo.fMask		= MIIM_STATE;
		GetMenuItemInfo( pPopup->GetSafeHmenu(), nDefaultID, FALSE, &pInfo );
		pInfo.fState	|= MFS_DEFAULT;
		SetMenuItemInfo( pPopup->GetSafeHmenu(), nDefaultID, FALSE, &pInfo );
	}
	
	return pPopup->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|nFlags, point.x, point.y, pWnd);
}
