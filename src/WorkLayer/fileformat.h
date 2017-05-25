/*
 * $Id: fileformat.h 4483 2008-01-02 09:19:06Z soarchin $
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

#define HEADERCHECKSIZE 16

static const unsigned char FILEHEADER_AVI_ID[]	= { 0x52, 0x49, 0x46, 0x46 };
static const unsigned char FILEHEADER_RAR[]		= { 0x52, 0x61, 0x72, 0x21};
static const unsigned char FILEHEADER_ZIP[]		= { 0x50, 0x4b, 0x03, 0x04};
static const unsigned char FILEHEADER_ACE_ID[]	= { 0x2A, 0x2A, 0x41, 0x43, 0x45, 0x2A, 0x2A };
static const unsigned char FILEHEADER_MP3_ID[]	= { 0x49, 0x44, 0x33, 0x03 };
static const unsigned char FILEHEADER_MP3_ID2[]	= { 0xFE, 0xFB };
static const unsigned char FILEHEADER_MPG_ID[]	= { 0x00, 0x00, 0x01, 0xba };
static const unsigned char FILEHEADER_ISO_ID[]	= { 0x01, 0x43, 0x44, 0x30, 0x30, 0x31 };
static const unsigned char FILEHEADER_WM_ID[]	= { 0x30, 0x26, 0xb2, 0x75, 0x8e, 0x66, 0xcf, 0x11, 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c };
static const unsigned char FILEHEADER_PNG_ID[]	= { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
static const unsigned char FILEHEADER_JPG_ID[]	= { 0xff, 0xd8, 0xff };
static const unsigned char FILEHEADER_GIF_ID[]	= { 0x47, 0x49, 0x46, 0x38 };
static const unsigned char FILEHEADER_PDF_ID[]	= { 0x25, 0x50, 0x44, 0x46 };
static const unsigned char FILEHEADER_EXECUTABLE_ID[]={0x4d, 0x5a };

struct SFileExts {
	EFileType ftype;
	LPCTSTR label;
	LPCTSTR extlist;
};

static SFileExts _fileexts[] =
{
	{ARCHIVE_ZIP,	_T("ZIP")	,		_T("|ZIP|JAR|CBZ|")},
	{ARCHIVE_RAR,	_T("RAR")	,		_T("|RAR|CBR|")},
	{ARCHIVE_ACE,	_T("ACE")	,		_T("|ACE|")},
	{AUDIO_MPEG,	_T("MPEG Audio"),	_T("|MP2|MP3|")},
	{IMAGE_ISO,		_T("ISO/NRG"),		_T("|ISO|NRG|")},
	{VIDEO_MPG,		_T("MPEG Video"),	_T("|MPG|MPEG|")},
	{VIDEO_AVI,		_T("AVI")	,		_T("|AVI|DIVX|")},
	{WM,			_T("Microsoft Media Audio/Video"),		_T("|ASF|WMV|WMA|")},
	{PIC_JPG,		_T("JPEG")	,		_T("|JPG|JPEG|")},
	{PIC_PNG,		_T("PNG")	,		_T("|PNG|")},
	{PIC_GIF,		_T("GIF")	,		_T("|GIF|")},
	{DOCUMENT_PDF,	_T("PDF")	,		_T("|PDF|")},
	{FILETYPE_EXECUTABLE, _T("WIN/DOS EXE"),				_T("|EXE|COM|DLL|SYS|CPL|FON|OCX|SCR|VBX|")  },
	{FILETYPE_UNKNOWN,_T(""),	 		_T("")}
};
