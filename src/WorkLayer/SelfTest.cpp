/*
 * $Id: SelfTest.cpp 4830 2008-02-22 08:04:43Z soarchin $
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
#include "stdafx.h"
#include "MD4.h"
#include "SHA.h"
#pragma warning(disable:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4702) // unreachable code
#include <crypto/sha.h>
#include <crypto/md4.h>
#pragma warning(default:4702) // unreachable code
#pragma warning(default:4100) // unreferenced formal parameter
#pragma warning(default:4244) // conversion from 'type1' to 'type2', possible loss of data

bool CheckHashingImplementations()
{
	static const BYTE _szData[] = "eMule Hash Verification Test Data";

	// Create SHA hash with Crypto Library
	BYTE crypto_sha1_digest[20];
	{
		CryptoPP::SHA1 sha;
		sha.Update(_szData, sizeof _szData);
		sha.Final(crypto_sha1_digest);
	}

	// Create SHA hash with our code
	{
		CSHA sha;
		sha.Add(_szData, sizeof _szData);
		sha.Finish();
		SHA1 digest;
		sha.GetHash(&digest);
		if (memcmp(digest.b, crypto_sha1_digest, sizeof crypto_sha1_digest) != 0)
		{
			AfxMessageBox(_T("Fatal Error: Implementation of SHA hashing is corrupted!"));
			return false;
		}
	}

	// Create MD4 hash with Crypto Library
	BYTE crypto_md4_digest[16];
	{
		CryptoPP::MD4 md4;
		md4.Update(_szData, sizeof _szData);
		md4.Final(crypto_md4_digest);
	}

	// Create MD4 hash with our code
	{
		CMD4 md4;
		md4.Add(_szData, sizeof _szData);
		md4.Finish();
		if (memcmp(md4.GetHash(), crypto_md4_digest, sizeof crypto_md4_digest) != 0)
		{
			AfxMessageBox(_T("Fatal Error: Implementation of MD4 hashing is corrupted!"));
			return false;
		}
	}

	return true;
}

bool SelfTest()
{
	if (!CSHA::VerifyImplementation()){
		AfxMessageBox(_T("Fatal Error: SHA hash implementation is corrupted!"));
		return false; // DO *NOT* START !!!
	}

	if (!CMD4::VerifyImplementation()){
		AfxMessageBox(_T("Fatal Error: MD4 hash implementation is corrupted!"));
		return false; // DO *NOT* START !!!
	}

	if (!CheckHashingImplementations()){
		return false; // DO *NOT* START !!!
	}

	return true;
}
