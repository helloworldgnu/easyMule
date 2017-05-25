//*********************************************************************
// 更新程序返回值
//*********************************************************************

const int RESULT_NEWVERSION			=  100;

//*********************************************************************
// USER MESSAGE
//*********************************************************************

// WPARAM: 线程类型
// LPARAM: 
#define UM_THREADMESSAGE				WM_APP + 1

// WPARAM:
// LPARAM:
#define UM_ERROR						WM_APP + 2
#define UM_DOWNLOAD_COMPLETE			WM_APP + 3

//*********************************************************************
// 线程类型
//*********************************************************************
const int THREAD_START			= 0;
const int THREAD_PAUSE			= 1;
const int THREAD_RESUME			= 2;
const int THREAD_FINISH			= 3;
const int THREAD_CANCEL			= 4;

//*********************************************************************
// ERROR
//*********************************************************************

const int ERROR_UNKNOWN			= 0;	// 未知错误
const int ERROR_NOCONNECTION	= 1;	// 无连接
const int ERROR_SERVER			= 2;	// 无法连接到服务器

const int ERROR_CHECKFAIL		= 3;	// 校验失败
const int ERROR_MEMNOTCREATE	= 4;	// 共享内存创建失败
const int ERROR_MEMNOTOPEN		= 5;	// 共享内存打开失败
const int ERROR_MEMNOTMAP		= 6;	// 共享内存映射失败
const int ERROR_WRITEMEM		= 7;	// 无法写入共享内存

const int ERROR_NONEWVERSION	= 8;	// 没有新版本可用
const int ERROR_LOADFAIL		= 9;	// Load失败



//*********************************************************************
// HTTP VERBS
//*********************************************************************

const HTTP_VERB_POST	= 0;
const HTTP_VERB_GET		= 1;
const HTTP_VERB_HEAD	= 2;
const HTTP_VERB_PUT		= 3;
const HTTP_VERB_LINK	= 4;
const HTTP_VERB_DELETE	= 5;
const HTTP_VERB_UNLINK	= 6;

//*********************************************************************
// PARAMETERS
//*********************************************************************
const CString PARAMETER_CHECKNEWVERSION		= _T("-checkforupdates");

//*********************************************************************
// HTTP ADDRES
//*********************************************************************
const CString HTTP_REMOTE_ADDRES_ENGLISH_DOWNLOAD		=_T("http://update.easymule.com/ezmuleupdate_en");
const CString HTTP_REMOTE_ADDRES_REALEASE_DOWNLOAD		=_T("http://update.easymule.com/ezmuleupdate");
const CString HTTP_REMOTE_ADDRES_BEAT_DOWNLOAD			=_T("http://update.easymule.com/ezmulebetaupdate");
const CString HTTP_LOCAL_ADDRES_DEBUG_DOWNLOAD			=_T("http://update.easymule.com/ezmulebetaupdate");
const CString HTTP_REMOTE_ADDRES_ALPHA_DOWNLOAD			=_T("http://update.easymule.com/alpha/ezmulealphaupdate");


//*********************************************************************
// BUFFERS
//*********************************************************************

const int BUFFER_DOWNLOADFILE	= 1024;