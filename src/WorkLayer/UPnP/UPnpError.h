/*
 * $Id: UPnpError.h 4483 2008-01-02 09:19:06Z soarchin $
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
#pragma once

//	参数错误
#define E_UNAT_BAD_ARGUMENTS				MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x200)

//	已是公网IP
#define E_UNAT_NOT_IN_LAN					MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x201)

//	找不到路由器
#define E_UNAT_CANNOT_FIND_ROUTER			MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x202)

//	超时无回应
#define E_UNAT_TIMEOUT						MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x203)

//	端口映射的表项可能已满
#define E_UNAT_ENTRY_MAYBE_FULL				MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x205)

//	未知错误
#define E_UNAT_UNKNOWN_ERROR				MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x206)

//	达到随机端口的重试次数
#define E_UNAT_REACH_RAND_PORT_RETRY_TIMES	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x207)

//	正在搜索中，还没有结果
#define E_UNAT_SEARCH_PENDING				MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x208)

//	创建Socket失败
#define E_UNAT_CREATE_SOCKET_FAILED			MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x209)

//	Socket绑定失败
#define E_UNAT_SOCKET_BIND_FAILED			MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x210)

//	Action返回Http失败码
#define E_UNAT_ACTION_HTTP_ERRORCODE		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x211)
