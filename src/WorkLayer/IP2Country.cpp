/*
 * $Id: IP2Country.cpp 11398 2009-03-17 11:00:27Z huby $
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
//EastShare Start - added by AndCycle, IP to Country

/*
the IP to country data is provided by http://ip-to-country.webhosting.info/

"IP2Country uses the IP-to-Country Database
 provided by WebHosting.Info (http://www.webhosting.info),
 available from http://ip-to-country.webhosting.info."

 */

/*

flags are from http://sf.net/projects/flags/

*/

// by Superlexx, based on IPFilter by Bouc7

#include "StdAfx.h"
#include "IP2Country.h"
#include "emule.h"
#include "otherfunctions.h"
#include <flag/resource.h>

//refresh list
#include "serverlist.h"
#include "clientlist.h"

//refresh server list ctrl
#include "emuledlg.h"
#include "serverwnd.h"
#include "serverlistctrl.h"

#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// N/A flag is the first Res, so it should at index zero
#define NO_FLAG 0

CString FirstCharCap(CString target){

	target.TrimRight();//clean out the space at the end, prevent exception for index++
	if(target.IsEmpty()) return _T("");
	target.MakeLower();
	if(isascii(target[0])) //Modified by GGSoSo
        target.SetAt(0, target.Left(1).MakeUpper().GetAt(0));
	for(int index = target.Find(' '); index != -1; index = target.Find(' ', index)){
		index++;//set the character next to space be Upper
		target.SetAt(index, target.Mid(index, 1).MakeUpper().GetAt(0));
	}
	return target;
}

CIP2CountryOld::CIP2CountryOld()
{
	m_bRunning = false;

	defaultCountry.ShortCountryName = "N/A";
	defaultCountry.MidCountryName = "N/A";
	defaultCountry.LongCountryName = "N/A";
	defaultCountry.FlagIndex = NO_FLAG;

	EnableIP2Country = false;
	EnableCountryFlag = false;

	
	//Load();		//COMMENTED by fengwen on 2007/01/16 : 外部显示调用Load();
	
	//AddLogLine(false, "IP2Country uses the IP-to-Country Database provided by WebHosting.Info (http://www.webhosting.info), available from http://ip-to-country.webhosting.info.");

	m_bRunning = true;
}

CIP2CountryOld::~CIP2CountryOld(){

	m_bRunning = false;	
	Unload();
}

void CIP2CountryOld::Load()
{
	bool isModified = false;
	
	//ADDED by fengwen on 2007/01/16	<begin> :
	bool bIsIP2CountryShowFlag = m_bInMainThread ? thePrefs.IsIP2CountryShowFlag() : m_tp_bIsIP2CountryShowFlag;
	int iIP2CountryNameMode = m_bInMainThread ? thePrefs.m_iIP2CountryNameMode : m_tp_iIP2CountryNameMode;
	//ADDED by fengwen on 2007/01/16	<end> :

	//MODIFIED by fengwen on 2007/01/16	<begin> :
	//if(thePrefs.IsIP2CountryShowFlag()) {
	if(bIsIP2CountryShowFlag) {
	//MODIFIED by fengwen on 2007/01/16	<end> :
		if (!EnableCountryFlag) {
			EnableCountryFlag = LoadCountryFlagLib();//flag lib first, so ip range can map to flag
			if (EnableCountryFlag) {
				isModified = true;
				EnableIP2Country = false;
				RemoveAllIPs();
			}
		}
	} else if (EnableCountryFlag) {
		EnableCountryFlag = false;
		RemoveAllFlags();
		isModified = true;
	}

	//MODIFIED by fengwen on 2007/01/16	<begin> :
	//if (thePrefs.IsIP2CountryShowFlag() || thePrefs.m_iIP2CountryNameMode!=IP2CountryName_DISABLE) {
	if (bIsIP2CountryShowFlag || iIP2CountryNameMode != IP2CountryName_DISABLE) {
	//MODIFIED by fengwen on 2007/01/16	<end> :
		if (!EnableIP2Country) {
			EnableIP2Country = LoadFromFile();
			if (EnableIP2Country) {
				isModified = true;
			}
		}
	} else if (EnableIP2Country) {
		EnableIP2Country = false;
		RemoveAllIPs();
		isModified = true;
	}
	

	if (!isModified) return;

	if(m_bRunning) Reset();
	
}

void CIP2CountryOld::Unload(){

	EnableIP2Country = false;
	EnableCountryFlag = false;

	if(m_bRunning) Reset();

	RemoveAllIPs();
	RemoveAllFlags();

	AddDebugLogLine(false, _T("IP2Country unloaded"));
}

void CIP2CountryOld::Reset(){
	if (m_bInMainThread)	//ADDED by fengwen on 2007/01/16	:
	{
		CGlobalVariable::serverlist->ResetIP2Country();
		CGlobalVariable::clientlist->ResetIP2Country();
	}
}

void CIP2CountryOld::Refresh()
{
	if (m_bInMainThread)	//ADDED by fengwen on 2007/01/16	:
		theApp.emuledlg->serverwnd->serverlistctrl.RefreshAllServer();
}

bool CIP2CountryOld::LoadFromFile(){

	char buffer[1024];
	int	lenBuf = 1024;
//new 2004-12-04
#ifdef _UNICODE
	WCHAR bufferunicode[1024];
	bool IsUnicode=false;
#endif
//new 2004-12-04

	//MODIFIED by fengwen on 2007/01/16	<begin> :
	//CString ip2countryCSVfile = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)+_T("ip-to-country.csv");
	CString strConfigDir = m_bInMainThread ? thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) : m_tp_strConfigDir;
	CString ip2countryCSVfile = strConfigDir + _T("ip-to-country.csv");
	//MODIFIED by fengwen on 2007/01/16	<end> :

	//FILE* readFile = fopen(CStringA(ip2countryCSVfile), "r");
	FILE* readFile = _tfsopen(CString(ip2countryCSVfile), _T("rb"), 0x20);//new 2004-12-03
	try{
		if (readFile != NULL) {

		//new 2004-12-04
			WORD wBOM = fgetwc(readFile);
			if (wBOM == 0xFEFF)
		#ifdef _UNICODE
				IsUnicode=true;
		#else
				throw CString(_T("File is Unicode, Can't to load in"));
		#endif

		#ifdef _UNICODE
			fseek(readFile, 0, SEEK_SET);
		#endif
		//new 2004-12-04
			int count = 0;

			while (!feof(readFile)) {

				CString sbuffer = NULL;
		#ifdef _UNICODE
				if(IsUnicode)
				{
					if (_fgetts(bufferunicode,lenBuf,readFile)==0) break;
					sbuffer = bufferunicode;
				}
				else
		#endif
				{
					if (fgets(buffer,lenBuf,readFile)==0) break;
					sbuffer = buffer;
				}
				//new 2004-12-04
				/*
					http://ip-to-country.webhosting.info/node/view/54

					This is a sample of how the CSV file is structured:

					"0033996344","0033996351","GB","GBR","UNITED KINGDOM"
					"0050331648","0083886079","US","USA","UNITED STATES"
					"0094585424","0094585439","SE","SWE","SWEDEN"

					FIELD  			DATA TYPE		  	FIELD DESCRIPTION
					IP_FROM 		NUMERICAL (DOUBLE) 	Beginning of IP address range.
					IP_TO			NUMERICAL (DOUBLE) 	Ending of IP address range.
					COUNTRY_CODE2 	CHAR(2)				Two-character country code based on ISO 3166.
					COUNTRY_CODE3 	CHAR(3)				Three-character country code based on ISO 3166.
					COUNTRY_NAME 	VARCHAR(50) 		Country name based on ISO 3166
				*/
				// we assume that the ip-to-country.csv is valid and doesn't cause any troubles
				// get & process IP range
				sbuffer.Remove('"'); // get rid of the " signs

				CString tempStr[5] = {NULL};

				int curPos;
				curPos = 0;

				for(int forCount = 0; forCount !=  5; forCount++){
					tempStr[forCount] = sbuffer.Tokenize(_T(","), curPos);
					if(tempStr[forCount].IsEmpty()) {
						TRACE(_T("found an empty item in ip2country.csv\n"));
						continue;
					}
				}
				
				//tempStr[4] is full country name, capitalize country name from rayita
				//tempStr[4] = FirstCharCap(tempStr[4]); // 首字母大写对中文支持有问题 暂时取消 --GGSoSo

				count++;
				AddIPRange(_tstoi(tempStr[0]),_tstoi(tempStr[1]), tempStr[2], tempStr[3], tempStr[4]);

				//ADDED by fengwen on 2007/01/19	<begin> : 使CPU不致于100%
				static WORD wCount = 1;
				if (0 == (wCount % 0x1000))
				{
					Sleep(10);
					wCount++;
				}
				//ADDED by fengwen on 2007/01/19	<end> : 使CPU不致于100%
			}
			fclose(readFile);
		}
		else{
			throw CString(_T("Failed to load in"));
		}
	}
	catch(CString error){

		AddLogLine(false, _T("%s %s"), error, ip2countryCSVfile);
		RemoveAllIPs();
		AddDebugLogLine(false, _T("IP2Countryfile load failed"));
		return false;
	}
	AddDebugLogLine(false, _T("IP2Countryfile has been loaded"));
	return true;

}

bool CIP2CountryOld::LoadCountryFlagLib(){

	CString ip2countryCountryFlag;
	try{
		//MODIFIED by fengwen on 2007/01/16	<begin> :
		//ip2countryCountryFlag = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)+_T("countryflag.dll");
		CString strConfigDir = m_bInMainThread ?  thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) : m_tp_strConfigDir;
		ip2countryCountryFlag = strConfigDir + _T("countryflag.dll");
		//MODIFIED by fengwen on 2007/01/16	<end> :

		_hCountryFlagDll = LoadLibrary(ip2countryCountryFlag); 
		if (_hCountryFlagDll == NULL) 
		{ 
			throw CString(_T("CountryFlag Disabled, failed to load"));
		} 

		uint16	resID[] = {
			IDI_COUNTRY_FLAG_NOFLAG,//first res in image list should be N/A

			IDI_COUNTRY_FLAG_AD, IDI_COUNTRY_FLAG_AE, IDI_COUNTRY_FLAG_AF, IDI_COUNTRY_FLAG_AG, 
			IDI_COUNTRY_FLAG_AI, IDI_COUNTRY_FLAG_AL, IDI_COUNTRY_FLAG_AM, IDI_COUNTRY_FLAG_AN, 
			IDI_COUNTRY_FLAG_AO, IDI_COUNTRY_FLAG_AR, IDI_COUNTRY_FLAG_AS, IDI_COUNTRY_FLAG_AT, 
			IDI_COUNTRY_FLAG_AU, IDI_COUNTRY_FLAG_AW, IDI_COUNTRY_FLAG_AZ, IDI_COUNTRY_FLAG_BA, 
			IDI_COUNTRY_FLAG_BB, IDI_COUNTRY_FLAG_BD, IDI_COUNTRY_FLAG_BE, IDI_COUNTRY_FLAG_BF, 
			IDI_COUNTRY_FLAG_BG, IDI_COUNTRY_FLAG_BH, IDI_COUNTRY_FLAG_BI, IDI_COUNTRY_FLAG_BJ, 
			IDI_COUNTRY_FLAG_BM, IDI_COUNTRY_FLAG_BN, IDI_COUNTRY_FLAG_BO, IDI_COUNTRY_FLAG_BR, 
			IDI_COUNTRY_FLAG_BS, IDI_COUNTRY_FLAG_BT, IDI_COUNTRY_FLAG_BW, IDI_COUNTRY_FLAG_BY, 
			IDI_COUNTRY_FLAG_BZ, IDI_COUNTRY_FLAG_CA, IDI_COUNTRY_FLAG_CC, IDI_COUNTRY_FLAG_CD, 
			IDI_COUNTRY_FLAG_CF, IDI_COUNTRY_FLAG_CG, IDI_COUNTRY_FLAG_CH, IDI_COUNTRY_FLAG_CI, 
			IDI_COUNTRY_FLAG_CK, IDI_COUNTRY_FLAG_CL, IDI_COUNTRY_FLAG_CM, IDI_COUNTRY_FLAG_CN, 
			IDI_COUNTRY_FLAG_CO, IDI_COUNTRY_FLAG_CR, IDI_COUNTRY_FLAG_CU, IDI_COUNTRY_FLAG_CV, 
			IDI_COUNTRY_FLAG_CX, IDI_COUNTRY_FLAG_CY, IDI_COUNTRY_FLAG_CZ, IDI_COUNTRY_FLAG_DE, 
			IDI_COUNTRY_FLAG_DJ, IDI_COUNTRY_FLAG_DK, IDI_COUNTRY_FLAG_DM, IDI_COUNTRY_FLAG_DO, 
			IDI_COUNTRY_FLAG_DZ, IDI_COUNTRY_FLAG_EC, IDI_COUNTRY_FLAG_EE, IDI_COUNTRY_FLAG_EG, 
			IDI_COUNTRY_FLAG_EH, IDI_COUNTRY_FLAG_ER, IDI_COUNTRY_FLAG_ES, IDI_COUNTRY_FLAG_ET, 
			IDI_COUNTRY_FLAG_FI, IDI_COUNTRY_FLAG_FJ, IDI_COUNTRY_FLAG_FK, IDI_COUNTRY_FLAG_FM, 
			IDI_COUNTRY_FLAG_FO, IDI_COUNTRY_FLAG_FR, IDI_COUNTRY_FLAG_GA, IDI_COUNTRY_FLAG_GB, 
			IDI_COUNTRY_FLAG_GD, IDI_COUNTRY_FLAG_GE, IDI_COUNTRY_FLAG_GG, IDI_COUNTRY_FLAG_GH, 
			IDI_COUNTRY_FLAG_GI, IDI_COUNTRY_FLAG_GK, IDI_COUNTRY_FLAG_GL, IDI_COUNTRY_FLAG_GM, 
			IDI_COUNTRY_FLAG_GN, IDI_COUNTRY_FLAG_GP, IDI_COUNTRY_FLAG_GQ, IDI_COUNTRY_FLAG_GR, 
			IDI_COUNTRY_FLAG_GS, IDI_COUNTRY_FLAG_GT, IDI_COUNTRY_FLAG_GU, IDI_COUNTRY_FLAG_GW, 
			IDI_COUNTRY_FLAG_GY, IDI_COUNTRY_FLAG_HK, IDI_COUNTRY_FLAG_HN, IDI_COUNTRY_FLAG_HR, 
			IDI_COUNTRY_FLAG_HT, IDI_COUNTRY_FLAG_HU, IDI_COUNTRY_FLAG_ID, IDI_COUNTRY_FLAG_IE, 
			IDI_COUNTRY_FLAG_IL, IDI_COUNTRY_FLAG_IM, IDI_COUNTRY_FLAG_IN, IDI_COUNTRY_FLAG_IO, 
			IDI_COUNTRY_FLAG_IQ, IDI_COUNTRY_FLAG_IR, IDI_COUNTRY_FLAG_IS, IDI_COUNTRY_FLAG_IT, 
			IDI_COUNTRY_FLAG_JE, IDI_COUNTRY_FLAG_JM, IDI_COUNTRY_FLAG_JO, IDI_COUNTRY_FLAG_JP, 
			IDI_COUNTRY_FLAG_KE, IDI_COUNTRY_FLAG_KG, IDI_COUNTRY_FLAG_KH, IDI_COUNTRY_FLAG_KI, 
			IDI_COUNTRY_FLAG_KM, IDI_COUNTRY_FLAG_KN, IDI_COUNTRY_FLAG_KP, IDI_COUNTRY_FLAG_KR, 
			IDI_COUNTRY_FLAG_KW, IDI_COUNTRY_FLAG_KY, IDI_COUNTRY_FLAG_KZ, IDI_COUNTRY_FLAG_LA, 
			IDI_COUNTRY_FLAG_LB, IDI_COUNTRY_FLAG_LC, IDI_COUNTRY_FLAG_LI, IDI_COUNTRY_FLAG_LK, 
			IDI_COUNTRY_FLAG_LR, IDI_COUNTRY_FLAG_LS, IDI_COUNTRY_FLAG_LT, IDI_COUNTRY_FLAG_LU, 
			IDI_COUNTRY_FLAG_LV, IDI_COUNTRY_FLAG_LY, IDI_COUNTRY_FLAG_MA, IDI_COUNTRY_FLAG_MC, 
			IDI_COUNTRY_FLAG_MD, IDI_COUNTRY_FLAG_MG, IDI_COUNTRY_FLAG_MH, IDI_COUNTRY_FLAG_MK, 
			IDI_COUNTRY_FLAG_ML, IDI_COUNTRY_FLAG_MM, IDI_COUNTRY_FLAG_MN, IDI_COUNTRY_FLAG_MO, 
			IDI_COUNTRY_FLAG_MP, IDI_COUNTRY_FLAG_MQ, IDI_COUNTRY_FLAG_MR, IDI_COUNTRY_FLAG_MS, 
			IDI_COUNTRY_FLAG_MT, IDI_COUNTRY_FLAG_MU, IDI_COUNTRY_FLAG_MV, IDI_COUNTRY_FLAG_MW, 
			IDI_COUNTRY_FLAG_MX, IDI_COUNTRY_FLAG_MY, IDI_COUNTRY_FLAG_MZ, IDI_COUNTRY_FLAG_NA, 
			IDI_COUNTRY_FLAG_NC, IDI_COUNTRY_FLAG_NE, IDI_COUNTRY_FLAG_NF, IDI_COUNTRY_FLAG_NG, 
			IDI_COUNTRY_FLAG_NI, IDI_COUNTRY_FLAG_NL, IDI_COUNTRY_FLAG_NO, IDI_COUNTRY_FLAG_NP, 
			IDI_COUNTRY_FLAG_NR, IDI_COUNTRY_FLAG_NU, IDI_COUNTRY_FLAG_NZ, IDI_COUNTRY_FLAG_OM, 
			IDI_COUNTRY_FLAG_PA, IDI_COUNTRY_FLAG_PC, IDI_COUNTRY_FLAG_PE, IDI_COUNTRY_FLAG_PF, 
			IDI_COUNTRY_FLAG_PG, IDI_COUNTRY_FLAG_PH, IDI_COUNTRY_FLAG_PK, IDI_COUNTRY_FLAG_PL, 
			IDI_COUNTRY_FLAG_PM, IDI_COUNTRY_FLAG_PN, IDI_COUNTRY_FLAG_PR, IDI_COUNTRY_FLAG_PS, 
			IDI_COUNTRY_FLAG_PT, IDI_COUNTRY_FLAG_PW, IDI_COUNTRY_FLAG_PY, IDI_COUNTRY_FLAG_QA, 
			IDI_COUNTRY_FLAG_RO, IDI_COUNTRY_FLAG_RU, IDI_COUNTRY_FLAG_RW, IDI_COUNTRY_FLAG_SA, 
			IDI_COUNTRY_FLAG_SB, IDI_COUNTRY_FLAG_SC, IDI_COUNTRY_FLAG_SD, IDI_COUNTRY_FLAG_SE, 
			IDI_COUNTRY_FLAG_SG, IDI_COUNTRY_FLAG_SH, IDI_COUNTRY_FLAG_SI, IDI_COUNTRY_FLAG_SK, 
			IDI_COUNTRY_FLAG_SL, IDI_COUNTRY_FLAG_SM, IDI_COUNTRY_FLAG_SN, IDI_COUNTRY_FLAG_SO, 
			IDI_COUNTRY_FLAG_SR, IDI_COUNTRY_FLAG_ST, IDI_COUNTRY_FLAG_SU, IDI_COUNTRY_FLAG_SV, 
			IDI_COUNTRY_FLAG_SY, IDI_COUNTRY_FLAG_SZ, IDI_COUNTRY_FLAG_TC, IDI_COUNTRY_FLAG_TD, 
			IDI_COUNTRY_FLAG_TF, IDI_COUNTRY_FLAG_TG, IDI_COUNTRY_FLAG_TH, IDI_COUNTRY_FLAG_TJ, 
			IDI_COUNTRY_FLAG_TK, IDI_COUNTRY_FLAG_TL, IDI_COUNTRY_FLAG_TM, IDI_COUNTRY_FLAG_TN, 
			IDI_COUNTRY_FLAG_TO, IDI_COUNTRY_FLAG_TR, IDI_COUNTRY_FLAG_TT, IDI_COUNTRY_FLAG_TV, 
			IDI_COUNTRY_FLAG_TW, IDI_COUNTRY_FLAG_TZ, IDI_COUNTRY_FLAG_UA, IDI_COUNTRY_FLAG_UG, 
			IDI_COUNTRY_FLAG_UM, IDI_COUNTRY_FLAG_US, IDI_COUNTRY_FLAG_UY, IDI_COUNTRY_FLAG_UZ, 
			IDI_COUNTRY_FLAG_VA, IDI_COUNTRY_FLAG_VC, IDI_COUNTRY_FLAG_VE, IDI_COUNTRY_FLAG_VG, 
			IDI_COUNTRY_FLAG_VI, IDI_COUNTRY_FLAG_VN, IDI_COUNTRY_FLAG_VU, IDI_COUNTRY_FLAG_WF, 
			IDI_COUNTRY_FLAG_WS, IDI_COUNTRY_FLAG_YE, IDI_COUNTRY_FLAG_YU, IDI_COUNTRY_FLAG_ZA, 
			IDI_COUNTRY_FLAG_ZM, IDI_COUNTRY_FLAG_ZW, 
			IDI_COUNTRY_FLAG_UK, //by tharghan
			IDI_COUNTRY_FLAG_CS, //by propaganda
			//IDI_COUNTRY_FLAG_TS, //VNN Group by GGSoSo

			65535//the end
		};

		CString countryID[] = {
			_T("N/A"),//first res in image list should be N/A

			_T("AD"), _T("AE"), _T("AF"), _T("AG"), _T("AI"), _T("AL"), _T("AM"), _T("AN"), _T("AO"), _T("AR"), _T("AS"), _T("AT"), _T("AU"), _T("AW"), _T("AZ"), 
			_T("BA"), _T("BB"), _T("BD"), _T("BE"), _T("BF"), _T("BG"), _T("BH"), _T("BI"), _T("BJ"), _T("BM"), _T("BN"), _T("BO"), _T("BR"), _T("BS"), _T("BT"), 
			_T("BW"), _T("BY"), _T("BZ"), _T("CA"), _T("CC"), _T("CD"), _T("CF"), _T("CG"), _T("CH"), _T("CI"), _T("CK"), _T("CL"), _T("CM"), _T("CN"), _T("CO"), 
			_T("CR"), _T("CU"), _T("CV"), _T("CX"), _T("CY"), _T("CZ"), _T("DE"), _T("DJ"), _T("DK"), _T("DM"), _T("DO"), _T("DZ"), _T("EC"), _T("EE"), _T("EG"), 
			_T("EH"), _T("ER"), _T("ES"), _T("ET"), _T("FI"), _T("FJ"), _T("FK"), _T("FM"), _T("FO"), _T("FR"), _T("GA"), _T("GB"), _T("GD"), _T("GE"), _T("GG"), 
			_T("GH"), _T("GI"), _T("GK"), _T("GL"), _T("GM"), _T("GN"), _T("GP"), _T("GQ"), _T("GR"), _T("GS"), _T("GT"), _T("GU"), _T("GW"), _T("GY"), _T("HK"), 
			_T("HN"), _T("HR"), _T("HT"), _T("HU"), _T("ID"), _T("IE"), _T("IL"), _T("IM"), _T("IN"), _T("IO"), _T("IQ"), _T("IR"), _T("IS"), _T("IT"), _T("JE"), 
			_T("JM"), _T("JO"), _T("JP"), _T("KE"), _T("KG"), _T("KH"), _T("KI"), _T("KM"), _T("KN"), _T("KP"), _T("KR"), _T("KW"), _T("KY"), _T("KZ"), _T("LA"), 
			_T("LB"), _T("LC"), _T("LI"), _T("LK"), _T("LR"), _T("LS"), _T("LT"), _T("LU"), _T("LV"), _T("LY"), _T("MA"), _T("MC"), _T("MD"), _T("MG"), _T("MH"), 
			_T("MK"), _T("ML"), _T("MM"), _T("MN"), _T("MO"), _T("MP"), _T("MQ"), _T("MR"), _T("MS"), _T("MT"), _T("MU"), _T("MV"), _T("MW"), _T("MX"), _T("MY"), 
			_T("MZ"), _T("NA"), _T("NC"), _T("NE"), _T("NF"), _T("NG"), _T("NI"), _T("NL"), _T("NO"), _T("NP"), _T("NR"), _T("NU"), _T("NZ"), _T("OM"), _T("PA"), 
			_T("PC"), _T("PE"), _T("PF"), _T("PG"), _T("PH"), _T("PK"), _T("PL"), _T("PM"), _T("PN"), _T("PR"), _T("PS"), _T("PT"), _T("PW"), _T("PY"), _T("QA"), 
			_T("RO"), _T("RU"), _T("RW"), _T("SA"), _T("SB"), _T("SC"), _T("SD"), _T("SE"), _T("SG"), _T("SH"), _T("SI"), _T("SK"), _T("SL"), _T("SM"), _T("SN"), 
			_T("SO"), _T("SR"), _T("ST"), _T("SU"), _T("SV"), _T("SY"), _T("SZ"), _T("TC"), _T("TD"), _T("TF"), _T("TG"), _T("TH"), _T("TJ"), _T("TK"), _T("TL"), 
			_T("TM"), _T("TN"), _T("TO"), _T("TR"), _T("TT"), _T("TV"), _T("TW"), _T("TZ"), _T("UA"), _T("UG"), _T("UM"), _T("US"), _T("UY"), _T("UZ"), _T("VA"), 
			_T("VC"), _T("VE"), _T("VG"), _T("VI"), _T("VN"), _T("VU"), _T("WF"), _T("WS"), _T("YE"), _T("YU"), _T("ZA"), _T("ZM"), _T("ZW"), 
			_T("UK"), //by tharghan
			_T("CS"), //by propaganda
			_T("TS")  //by GGSoSo
		};

		HICON iconHandle;

		CountryFlagImageList.DeleteImageList();
		//MODIFIED by fengwen on 2007/01/16	<begin> :
		//CountryFlagImageList.Create(18,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
		int iDfltImageListColorFlags = m_bInMainThread ? theApp.m_iDfltImageListColorFlags : m_tp_iDfltImageListColorFlags;
		CountryFlagImageList.Create(18,16,iDfltImageListColorFlags|ILC_MASK,0,1);
		//MODIFIED by fengwen on 2007/01/16	<end> :

		CountryFlagImageList.SetBkColor(CLR_NONE);

		//the res Array have one element to be the STOP
		for(int cur_pos = 0; resID[cur_pos] != 65535; cur_pos++){

			CountryIDtoFlagIndex.SetAt(countryID[cur_pos], (uint16) cur_pos);

			iconHandle = LoadIcon(_hCountryFlagDll, MAKEINTRESOURCE(resID[cur_pos]));
			if(iconHandle == NULL) throw CString(_T("Invalid resID, maybe you need to upgrade your flag icon Dll,"));
			
			CountryFlagImageList.Add(iconHandle);
		}
	

	}
	catch(CString error){
		if (m_bInMainThread)	//ADDED by fengwen on 2007/01/16	:
			AddLogLine(false, _T("%s in %s"), error, ip2countryCountryFlag);
		RemoveAllFlags();
		//free lib
		if(_hCountryFlagDll != NULL) FreeLibrary(_hCountryFlagDll);
		AddDebugLogLine(false, _T("Country Flags load failed"));
		return false;
	}

	//free lib
	if(_hCountryFlagDll != NULL) FreeLibrary(_hCountryFlagDll);

	AddDebugLogLine(false, _T("Country Flags have been loaded"));

	return true;

}

void CIP2CountryOld::RemoveAllCountries()
{
	CString key = NULL;
	Country_Struct* value;

//MODIFIED by fengwen on 2007/03/26	<begin> :
	//POSITION pos1;
	//for(POSITION pos = countryList.GetHeadPosition(); pos1 = pos; )
	//{
	//	countryList.GetNextAssoc(pos, key, value);
	//	delete value;
	//	countryList.RemoveAt(pos1);
	//}
	POSITION	pos = countryList.GetHeadPosition();
	while (NULL != pos)
	{
		countryList.GetNextAssoc(pos, key, value);
		delete value;
	}
//MODIFIED by fengwen on 2007/03/26	<end> :

	countryList.RemoveAll();
}

void CIP2CountryOld::RemoveAllIPs()
{
	RemoveAllCountries();
	uint32 key;
	IPRange_Struct2* value;
	POSITION pos1;
	for(POSITION pos = iplist.GetHeadPosition(); pos; )
	{
		pos1 = pos;
		iplist.GetNextAssoc(pos, key, value);
		delete value;
		iplist.RemoveAt(pos1);
	}
	iplist.RemoveAll();

	AddDebugLogLine(false, _T("IP2Countryfile has been unloaded"));
}

void CIP2CountryOld::RemoveAllFlags(){

	//destory all image
	CountryFlagImageList.DeleteImageList();

	//also clean out the map table
	CountryIDtoFlagIndex.RemoveAll();

	AddDebugLogLine(false, _T("Country Flags have been unloaded"));
}

bool CIP2CountryOld::AddCountry(CString shortCountryName, CString midCountryName, CString longCountryName){
	Country_Struct* newCountry = new Country_Struct();

	newCountry->ShortCountryName = shortCountryName;
	newCountry->MidCountryName = midCountryName;
	newCountry->LongCountryName = longCountryName;

	if (EnableCountryFlag) {
		const CRBMap<CString, uint16>::CPair* pair;
		pair = CountryIDtoFlagIndex.Lookup(shortCountryName);

		if(pair != NULL){
			newCountry->FlagIndex = pair->m_value;
		}
		else{
			newCountry->FlagIndex = NO_FLAG;
		}
	}
	else{
		//this valuse is useless if the country flag havn't been load up, should be safe I think ...
		//newRange->FlagIndex = 0;
	}
	
	const CRBMap<CString, Country_Struct*>::CPair* pair;
	pair = countryList.Lookup(longCountryName);
	if (pair == NULL) {
		countryList.SetAt(longCountryName, newCountry);
	}
	else
		delete newCountry;

	return true;
}

bool CIP2CountryOld::AddIPRange(uint32 IPfrom,uint32 IPto, CString shortCountryName, CString midCountryName, CString longCountryName){
	IPRange_Struct2* newRange = new IPRange_Struct2();

	newRange->IPstart = IPfrom;
	newRange->IPend = IPto;

	const CRBMap<CString, Country_Struct*>::CPair* pair;
	pair = countryList.Lookup(longCountryName);
	if (pair == NULL) {
		AddCountry(shortCountryName, midCountryName, longCountryName);
		pair = countryList.Lookup(longCountryName);
	}

	if (pair == NULL) 
	{
		delete newRange;
		return false;
	}

	newRange->country = pair->m_value;

	IPRange_Struct2* oldRange=NULL;
	iplist.Lookup(IPfrom,oldRange);
	if(NULL!=oldRange)
		delete oldRange;
	iplist.SetAt(IPfrom, newRange);
	return true;
}

Country_Struct* CIP2CountryOld::GetCountryFromIP(uint32 ClientIP){

	if(EnableIP2Country == false){
		return &defaultCountry;
	}
	else if (ClientIP == 0){
		//AddDebugLogLine(false, "CIP2CountryOld::GetCountryFromIP doesn't have ip to search for");
		return &defaultCountry;
	}
	else if(iplist.IsEmpty()){
		AddDebugLogLine(false, _T("CIP2CountryOld::GetCountryFromIP iplist doesn't exist"));
		return &defaultCountry;
	}

	ClientIP = htonl(ClientIP);
	POSITION pos = iplist.FindFirstKeyAfter(ClientIP);
	if(!pos){
		pos = iplist.GetTailPosition();
	}
	else{
		iplist.GetPrev(pos);
	}

	while(pos){
		const CRBMap<uint32, IPRange_Struct2*>::CPair* pair = iplist.GetPrev(pos);

		if (ClientIP > pair->m_value->IPend) break;
		if (ClientIP >= pair->m_key && ClientIP <= pair->m_value->IPend) return pair->m_value->country;
	}
	return &defaultCountry;
}

bool CIP2CountryOld::ShowCountryFlag(){

	//ADDED by fengwen on 2007/01/16	<begin> :
	bool bIsIP2CountryShowFlag = m_bInMainThread ? thePrefs.IsIP2CountryShowFlag() : m_tp_bIsIP2CountryShowFlag;
	//ADDED by fengwen on 2007/01/16	<end> :

	return 
		//user wanna see flag,
		//MODIFIED by fengwen on 2007/01/16	<begin> :
		//(thePrefs.IsIP2CountryShowFlag() && 
		(bIsIP2CountryShowFlag && 
		//MODIFIED by fengwen on 2007/01/16	<end> :
		//flag have been loaded
		EnableCountryFlag && 
		//ip table have been loaded
		EnableIP2Country);
}

//EastShare End - added by AndCycle, IP to Country

IMPLEMENT_DYNAMIC(CIP2Country, CWnd)

CIP2Country::CIP2Country()
{
	m_pOldIP2Country = NULL;
	m_pInitOldThread = NULL;
	
	CreateEx(0, _T("static"), NULL, 0, CRect(0, 0, 0, 0), NULL, 0);
	CreateOldI2CAsync();
}

CIP2Country::~CIP2Country()
{
	if (NULL != m_pInitOldThread)
	{
		//ADDED by nightsuns on 2007/11/8 : 修改程序退出时，后台线程仍在运行的错误
		WaitForSingleObject( m_pInitOldThread->m_hThread ,INFINITE );

		delete m_pInitOldThread;
		m_pInitOldThread = NULL;
	}

	if (NULL != m_pOldIP2Country)
	{
		delete m_pOldIP2Country;
		m_pOldIP2Country = NULL;
	}
	
	DestroyWindow();
}

BEGIN_MESSAGE_MAP(CIP2Country, CWnd)
	ON_MESSAGE(UM_IP2COUNTRY_COMPLETED, OnLoadCompleted)
END_MESSAGE_MAP()

bool CIP2Country::ShowCountryFlag()
{
	if (NULL == m_pOldIP2Country)
		return false;
	else
		return m_pOldIP2Country->ShowCountryFlag();
}

CImageList* CIP2Country::GetFlagImageList()
{
	if (NULL == m_pOldIP2Country)
		return NULL;
	else
		return m_pOldIP2Country->GetFlagImageList();
}

Country_Struct* CIP2Country::GetCountryFromIP(uint32 IP)
{
	if (NULL == m_pOldIP2Country)
		return NULL;
	else
		return m_pOldIP2Country->GetCountryFromIP(IP);
}

bool CIP2Country::IsIP2Country()
{
	if (NULL == m_pOldIP2Country)
		return false;
	else
		return m_pOldIP2Country->IsIP2Country();
}

void CIP2Country::Load()
{
	if (NULL == m_pOldIP2Country)
		return;
	else
		return m_pOldIP2Country->Load();
}

void CIP2Country::Refresh()
{
	if (NULL == m_pOldIP2Country)
		return;
	else
		return m_pOldIP2Country->Refresh();
}

Country_Struct* CIP2Country::GetDefaultIP2Country()
{
	if (NULL == m_pOldIP2Country)
		return NULL;
	else
		return m_pOldIP2Country->GetDefaultIP2Country();
}

void CIP2Country::CreateOldI2CAsync(void)
{
	OLD_INIT_THREAD_PARAM	*pParam = new OLD_INIT_THREAD_PARAM;

	pParam->hNotifyWnd					= GetSafeHwnd();
	pParam->bIsIP2CountryShowFlag		= thePrefs.IsIP2CountryShowFlag();
	pParam->iIP2CountryNameMode			= thePrefs.m_iIP2CountryNameMode;
	pParam->iDfltImageListColorFlags	= theApp.m_iDfltImageListColorFlags;
	pParam->strConfigDir				= thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);

	m_pInitOldThread = ::AfxBeginThread(OldProcInitThread, pParam, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
	if (NULL != m_pInitOldThread)
	{
		pParam = NULL;
		m_pInitOldThread->m_bAutoDelete = FALSE;
		m_pInitOldThread->ResumeThread();
	}
	else
	{
		delete pParam;
		pParam = NULL;
	}
}

UINT CIP2Country::OldProcInitThread(LPVOID lpParam)
{
	if (NULL == lpParam)
		return 0;
	OLD_INIT_THREAD_PARAM		*pParam = (OLD_INIT_THREAD_PARAM*) lpParam;
	
	CIP2CountryOld	*pOldIP2Country = new CIP2CountryOld();
	pOldIP2Country->m_bInMainThread					= false;
	pOldIP2Country->m_tp_bIsIP2CountryShowFlag		= pParam->bIsIP2CountryShowFlag;
	pOldIP2Country->m_tp_iIP2CountryNameMode		= pParam->iIP2CountryNameMode;
	pOldIP2Country->m_tp_iDfltImageListColorFlags	= pParam->iDfltImageListColorFlags;
	pOldIP2Country->m_tp_strConfigDir				= pParam->strConfigDir;
	
	pOldIP2Country->Load();

	if ( IsWindow(pParam->hNotifyWnd) )
	{
		::PostMessage(pParam->hNotifyWnd, UM_IP2COUNTRY_COMPLETED, 0, (LPARAM) pOldIP2Country);
	}
	else
	{
		delete pOldIP2Country;
		pOldIP2Country = NULL;
	}

	delete pParam;
	pParam = NULL;
	return 0;
}

LRESULT CIP2Country::OnLoadCompleted(WPARAM /*wParam*/, LPARAM lParam)
{
	if (NULL == lParam)
		return 0;

	if (NULL != m_pOldIP2Country)
	{
		delete m_pOldIP2Country;
		m_pOldIP2Country = NULL;
	}
	m_pOldIP2Country = (CIP2CountryOld*) lParam;
	m_pOldIP2Country->m_bInMainThread = true;
	m_pOldIP2Country->Reset();

	return 0;
}
