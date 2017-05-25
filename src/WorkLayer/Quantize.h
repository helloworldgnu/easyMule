/*
 * $Id: Quantize.h 4483 2008-01-02 09:19:06Z soarchin $
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
/* === C R E D I T S  &  D I S C L A I M E R S ==============
 * Permission is given by the author to freely redistribute and include
 * this code in any program as long as this credit is given where due.
 *
 * CQuantizer (c)  1996-1997 Jeff Prosise
 *
 * COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
 * OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
 * THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
 * OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
 * CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
 * THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
 * SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
 * PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
 * THIS DISCLAIMER.
 *
 * Use at your own risk!
 * ==========================================================
 */
#pragma once

class CQuantizer
{
typedef struct _NODE {
    BOOL bIsLeaf;               // TRUE if node has no children
    UINT nPixelCount;           // Number of pixels represented by this leaf
    UINT nRedSum;               // Sum of red components
    UINT nGreenSum;             // Sum of green components
    UINT nBlueSum;              // Sum of blue components
    struct _NODE* pChild[8];    // Pointers to child nodes
    struct _NODE* pNext;        // Pointer to next reducible node
} NODE;
protected:
    NODE* m_pTree;
    UINT m_nLeafCount;
    NODE* m_pReducibleNodes[9];
    UINT m_nMaxColors;
    UINT m_nColorBits;

public:
    CQuantizer (UINT nMaxColors, UINT nColorBits);
    virtual ~CQuantizer ();
    BOOL ProcessImage (HANDLE hImage);
    UINT GetColorCount ();
    void SetColorTable (RGBQUAD* prgb);

protected:
    void AddColor (NODE** ppNode, BYTE r, BYTE g, BYTE b, UINT nColorBits,
        UINT nLevel, UINT* pLeafCount, NODE** pReducibleNodes);
    void* CreateNode (UINT nLevel, UINT nColorBits, UINT* pLeafCount,
        NODE** pReducibleNodes);
    void ReduceTree (UINT nColorBits, UINT* pLeafCount,
        NODE** pReducibleNodes);
    void DeleteTree (NODE** ppNode);
    void GetPaletteColors (NODE* pTree, RGBQUAD* prgb, UINT* pIndex);
	BYTE GetPixelIndex(long x,long y, int nbit, long effwdt, BYTE *pimage);
};

