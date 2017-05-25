#include "stdafx.h"
#include "AntiVirusIDs.h"
#include "Preferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HINSTANCE _hSecurityDLL = NULL;

struct SSecurity
{
	ANTIVIRUSID antivirusid;		//杀毒软件ID号
	LPCTSTR		pszSoftName;		//杀毒软件名称
	BOOL		bSupported;			//是否可用
	LPCTSTR		pszFolderName;		//链接库所在的文件夹名
	LPCTSTR		pszDLLName;			//链接库的文件名，不包括扩展名
	LPCTSTR		pszModelName;		//杀毒软件厂商提供的杀毒中间件
};


static SSecurity _Security[] =
{
	{ANTIVIRUS_RISING, _T("瑞星杀毒模块"), FALSE, _T("RAV"), _T("RAV"), _T("rsscan.dll")},
	{0, NULL, 0, NULL, NULL, NULL}
};

static bool IsRisingExist()
{
	HKEY hKey = NULL;
	TCHAR szVersion[10];

	DWORD dwType = REG_SZ; 
	DWORD SizeVersion = sizeof(szVersion);

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\rising\\Rav"), 0, KEY_READ,&hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return false;
	}

	*szVersion = 0;
	if (RegQueryValueEx(hKey, _T("Version"), NULL, &dwType, (LPBYTE)szVersion, &SizeVersion) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return false;
	}

	CString strVersion = CString(szVersion);

	strVersion = strVersion.Left(2);

	if (strVersion.Compare(_T("20")) == 0)
	{
		return true;
	}

	return false;
}

//将找到的dll标记为可用,如果是瑞星软件的话，判断瑞星杀毒软件是否已经安装
static void InitSecurity(const CString& strSecurityDir, bool bReInit = false)
{
	static BOOL _bInitialized = FALSE;
	if (_bInitialized && !bReInit)
	{
		return;
	}

	_bInitialized = TRUE;

	CFileFind ff;
	SSecurity* pSecurity = _Security;
	if (pSecurity)
	{
		while (pSecurity->antivirusid)
		{
			if(ff.FindFile(strSecurityDir + pSecurity->pszFolderName + _T("\\") + pSecurity->pszDLLName + _T(".dll"), 0))
			{
				if (ff.FindFile(strSecurityDir + pSecurity->pszFolderName + _T("\\") + pSecurity->pszModelName, 0))
				{
					pSecurity->bSupported = TRUE;
				}
			}

			if (pSecurity->antivirusid == ANTIVIRUS_RISING && !IsRisingExist())
			{
				pSecurity->bSupported = FALSE;
			}
			pSecurity++;
		}
	}

	ff.Close();
}

HINSTANCE GetSecurityDLL()
{
	return _hSecurityDLL;
}

void CPreferences::FreeSecurityDLL()
{
	if (_hSecurityDLL != NULL && _hSecurityDLL != GetModuleHandle(NULL))
	{
		VERIFY( FreeLibrary(_hSecurityDLL) );
		_hSecurityDLL = NULL;
	}
}

static bool LoadSecurityLib(const CString& strSecurityDir, ANTIVIRUSID antivirusid)
{
	SSecurity* pSecurity = _Security;

	if (pSecurity)
	{
		while (pSecurity->antivirusid)
		{
			if (pSecurity->bSupported && pSecurity->antivirusid == antivirusid)
			{
				thePrefs.FreeSecurityDLL();

				CString strSecurityDLL = strSecurityDir;
				strSecurityDLL += pSecurity->pszFolderName;
				strSecurityDLL += _T("\\");
				strSecurityDLL += pSecurity->pszDLLName;
				strSecurityDLL += _T(".dll");

				_hSecurityDLL = LoadLibrary(strSecurityDLL);
				if (_hSecurityDLL)
				{
					return true;
				}
				break;
			 }
			 pSecurity++;
		}
	}

	return false;
}

//将标记为TRUE的杀毒软件信息放入AntiVirusIDs数组中
void CPreferences::GetAntiVirusProgs(CWordArray& AntiVirusIDs)
{
	const SSecurity* pSecurity = _Security;
	while (pSecurity->antivirusid && pSecurity->bSupported == TRUE)
	{
		AntiVirusIDs.Add(pSecurity->antivirusid);
		pSecurity++;
	}
}

void CPreferences::SetAntiVirusID(WORD antivirusid)
{
	m_wAntiVirusID = antivirusid;
}

//{begin} VC-dgkang 2008年8月7日
ANTIVIRUSID CPreferences::HasSecuritySoft(ANTIVIRUSID id /* = 0 */)
{	
	InitSecurity(thePrefs.GetMuleDirectory(EMULE_SECURITYDIR));

	const SSecurity* pSecurity = _Security;
	ANTIVIRUSID retID = 0;
	for(;pSecurity->antivirusid; pSecurity++)
	{
		if (id && pSecurity->bSupported && pSecurity->antivirusid == id)
			return id;

		if (!retID && pSecurity->bSupported)
			retID = pSecurity->antivirusid;
	}
	return retID;
}
//{end}

bool CPreferences::SetSecurity()
{
	InitSecurity(thePrefs.GetMuleDirectory(EMULE_SECURITYDIR));

	if (m_wAntiVirusID)
	{
		return LoadSecurityLib(thePrefs.GetMuleDirectory(EMULE_SECURITYDIR), m_wAntiVirusID);
	}
	return false;
}

CString CPreferences::GetAntiVirusNameByID(ANTIVIRUSID antivirusid)
{
	const SSecurity* pSecurity = _Security;
	for (;pSecurity->antivirusid;pSecurity++)
	{
		if (pSecurity->antivirusid == antivirusid)
		{
			return CString(pSecurity->pszSoftName);
		}
	}
	ASSERT ( false );
	return CString(_T(""));
}

CString CPreferences::GetAntiVirusDLLNameByID(ANTIVIRUSID antivirusid)
{
	const SSecurity* pSecurity = _Security;
	for (;pSecurity->antivirusid;pSecurity++)
	{
		if (pSecurity->antivirusid == antivirusid)
		{
			return CString(pSecurity->pszDLLName) + _T(".dll");
		}
	}
	ASSERT ( false );
	return CString(_T(""));
}

WORD CPreferences::GetAntiVirusID()
{
	return m_wAntiVirusID;
}