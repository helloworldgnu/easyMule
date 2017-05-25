/* 
 * $Id: vcconfig.h 4483 2008-01-02 09:19:06Z soarchin $
 * 
 * ///////////////////////////////////////////////////
 * 
 *              VeryCD eMule Config File
 *  Include:
 *  1. Client Filter Key Words
 *  Warning:
 *  These infomation should not be opened
 *  
 *                              VeryCD Software
 *                  by kernel1983 on 2006.08.02
 * ///////////////////////////////////////////////////
 */

// Filter Key Words

enum CLIENT_FILTER_METHOD
{
	CFM_INCLUDE, // 包含屏蔽字符串
	CFM_EQUAL    // 等于屏蔽字符串
};

struct _FILTER_ENTRY
{
	CString					keyStr;
	CLIENT_FILTER_METHOD	method;
};

// 根据用户名屏蔽
#define CLIENT_FILTER_NAME_SIZE 8

static _FILTER_ENTRY filterNameList[] = {
	_T("[TUOTU]"),			CFM_INCLUDE,
	_T("WWW.XUNLEI.COM"),	CFM_INCLUDE,
	_T("VAGAA"),		CFM_INCLUDE,
	_T("EMULE-CLIENT"), CFM_INCLUDE,
	_T("POWERMULE"),	CFM_INCLUDE,
	_T("EMULE.COM.CN"), CFM_INCLUDE,
	_T("MIPHONE"),      CFM_INCLUDE,
	_T("[DOWN]"),       CFM_INCLUDE
};

// 根据版本号屏蔽
#define CLIENT_FILTER_EDTION_SIZE 1

static _FILTER_ENTRY filterEditonList[] = {
	//_T("EMULECOMPAT"), CFM_INCLUDE,
	_T("EMULE"),		CFM_EQUAL
};

