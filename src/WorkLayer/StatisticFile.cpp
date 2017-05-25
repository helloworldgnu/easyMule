/* 
 * $Id: StatisticFile.cpp 4483 2008-01-02 09:19:06Z soarchin $
 * 
 *  parts of this file are based on work from pan One (http://home-3.tiscali.nl/~meost/pms/)
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
#include "StatisticFile.h"
#include "emule.h"
#include "KnownFileList.h"
#include "SharedFileList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CStatisticFile::MergeFileStats( CStatisticFile *toMerge )
{
	requested += toMerge->GetRequests();
	accepted += toMerge->GetAccepts();
	transferred += toMerge->GetTransferred();
	alltimerequested += toMerge->GetAllTimeRequests();
	alltimetransferred += toMerge->GetAllTimeTransferred();
	alltimeaccepted += toMerge->GetAllTimeAccepts();
}

void CStatisticFile::AddRequest(){
	requested++;
	alltimerequested++;
	CGlobalVariable::knownfiles->requested++;
	CGlobalVariable::sharedfiles->UpdateFile(fileParent);
}
	
void CStatisticFile::AddAccepted(){
	accepted++;
	alltimeaccepted++;
	CGlobalVariable::knownfiles->accepted++;
	CGlobalVariable::sharedfiles->UpdateFile(fileParent);
}
	
void CStatisticFile::AddTransferred(uint64 bytes){
	transferred += bytes;
	alltimetransferred += bytes;
	CGlobalVariable::knownfiles->transferred += bytes;
	CGlobalVariable::sharedfiles->UpdateFile(fileParent);
}
