#pragma once

#include "UpDownClient.h"

class CEd2kUpDownClient : public CUpDownClient
{
	DECLARE_DYNAMIC(CEd2kUpDownClient)

public:
	CEd2kUpDownClient(CClientReqSocket* sender = 0);
	CEd2kUpDownClient(CPartFile* in_reqfile, uint16 in_port, uint32 in_userid, uint32 in_serverip, uint16 in_serverport, bool ed2kID = false);
	virtual ~CEd2kUpDownClient();

	//Xman Anti-Leecher: simple Anti-Thief
	float	GetXtremeVersion(CString modversion) const;
	static const CString str_ANTAddOn;
	static const CString GetANTAddOn()
	{
		CString nick=_T("[");
		srand( (unsigned)time( NULL ) );
		for(uint8 i = 0; i < 4; ++i)
		{
				nick.AppendFormat(_T("%c"), 32+rand()%95);
		}
		nick += _T(']');

		return nick;
	}

	TCHAR*	m_pszFunnyNick;
	//Xman end

	bool	ProcessHelloTypePacket(CSafeMemFile* data, bool isHelloPacket);

	void			ClearHelloProperties();
	bool			ProcessHelloAnswer(const uchar* pachPacket, UINT nSize);
	bool			ProcessHelloPacket(const uchar* pachPacket, UINT nSize);
	void			SendHelloAnswer();
	void			SendMuleInfoPacket(bool bAnswer);
	void			ProcessMuleInfoPacket(const uchar* pachPacket, UINT nSize);
	void			ProcessMuleCommentPacket(const uchar* pachPacket, UINT nSize);
	void			ProcessEmuleQueueRank(const uchar* packet, UINT size);
	void			ProcessEdonkeyQueueRank(const uchar* packet, UINT size);
	void			CheckQueueRankFlood();
	void			SetCommentDirty(bool bDirty = true)				{ m_bCommentDirty = bDirty; }
	void			ProcessPublicIPAnswer(const BYTE* pbyData, UINT uSize);
	bool			SendBuddyPingPong()								{ return m_dwLastBuddyPingPongTime < ::GetTickCount(); }
	bool			AllowIncomeingBuddyPingPong()					{ return m_dwLastBuddyPingPongTime < (::GetTickCount()-(3*60*1000)); }
	void			SetLastBuddyPingPongTime()						{ m_dwLastBuddyPingPongTime = (::GetTickCount()+(10*60*1000)); }
	void			ProcessFirewallCheckUDPRequest(CSafeMemFile* data);
	// secure ident
	void			SendPublicKeyPacket();
	void			SendSignaturePacket();
	void			ProcessPublicKeyPacket(const uchar* pachPacket, UINT nSize);
	void			ProcessSignaturePacket(const uchar* pachPacket, UINT nSize);
	uint8			GetSecureIdentState() const						{ return (uint8)m_SecureIdentState; }
	void			SendSecIdentStatePacket();
	void			ProcessSecIdentStatePacket(const uchar* pachPacket, UINT nSize);
	uint8			GetInfoPacketsReceived() const					{ return m_byInfopacketsReceived; }
	void			InfoPacketsReceived();
	// preview
	void			SendPreviewRequest(const CAbstractFile* pForFile);
	void			SendPreviewAnswer(const CKnownFile* pForFile, CxImage** imgFrames, uint8 nCount);
	void			ProcessPreviewReq(const uchar* pachPacket, UINT nSize);
	void			ProcessPreviewAnswer(const uchar* pachPacket, UINT nSize);
	bool			GetPreviewSupport() const						{ return m_fSupportsPreview && GetViewSharedFilesSupport(); }
	bool			GetViewSharedFilesSupport() const				{ return m_fNoViewSharedFiles==0; }

	//Xman Anti-Leecher
	void			TestLeecher();
	void			BanLeecher(LPCTSTR pszReason , uint8 leechercategory); 
	bool			ProcessUnknownHelloTag(CTag *tag, CString &pszReason);
	void			ProcessUnknownInfoTag(CTag *tag, CString &pszReason);
	void			ProcessBanMessage();
	CString			GetBanMessageString() const {return strBanReason_permament;}

	void			UDPReaskForDownload();

	//>>> Anti-XS-Exploit (Xman)
	void IncXSAnswer()  {m_uiXSAnswer++;}
	void IncXSReqs()  {m_uiXSReqs++;}
	UINT GetXSReqs() const { return m_uiXSReqs;}
	UINT GetXSAnswers() const {return m_uiXSAnswer;}
	bool IsXSExploiter() const { return m_uiXSReqs>2 && ((float)(m_uiXSAnswer+1))/m_uiXSReqs<0.5f;}
	//<<< Anti-XS-Exploit

	//Xman end


	void init();
};
