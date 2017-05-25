/* 
 * $Id: IrcMain.h 7701 2008-10-15 07:34:41Z huby $
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

#pragma once

#if _ENABLE_NOUSE

class CIrcWnd;
class CIrcSocket;
class CIrcMain
{
	public:
		CIrcMain(void);
		~CIrcMain(void);
		void ParseMessage( CString sMessage );
		void PreParseMessage( CStringA sBuffer );
		void SendLogin();
		void Connect();
		void Disconnect( bool bIsShuttingDown = false);
		void SetConnectStatus( bool bConnected );
		void SetIRCWnd(CIrcWnd* pwndIRC);
		int SendString( CString sSend );
		void ParsePerform();
		void ProcessLink( CString sED2KLink );
		uint32 SetVerify();
		CString GetNick();
	private:
		CIrcSocket* m_pIRCSocket;
		CIrcWnd* m_pwndIRC;
		CString m_sPreParseBuffer;
		CString m_sUser;
		CString m_sNick;
		CString m_sVersion;
		uint32 m_uVerify;
};
#endif
