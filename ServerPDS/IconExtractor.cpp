#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include "IconExtractor.h"


#define WIDTHBYTES(bits)      ((((bits) + 31)>>5)<<2)

CIconExtractor::CIconExtractor()
{
}

DWORD CIconExtractor::ExtracttIcon(std::wstring processPath, std::string &iconString)
{
    
	HINSTANCE hResource = LoadLibrary(processPath.c_str());
	if (hResource == NULL)
	{
		return	GetLastError();
	}

	
	
	
	LPICONRESOURCE      lpIR    = NULL;
    HRSRC               hRsrc   = NULL;
    HGLOBAL             hGlobal = NULL;
    LPMEMICONDIR        lpIcon  = NULL;

	m_GKounter = 0;
	m_GNameKounter = 0;
	UINT IconIndex = 0;

	if (!EnumResourceNames(hResource, RT_GROUP_ICON, EnumResNameProc, reinterpret_cast<LPARAM>(this)))
		return GetLastError();

	// Find the group icon resource
	//Finding the resource with the ID first
	if (m_GKounter > 0)
		hRsrc = FindResource(hResource, MAKEINTRESOURCE(m_GroupIconID[IconIndex]), RT_GROUP_ICON);
	if (hRsrc == NULL)//Searching the icon with names if any
		hRsrc = FindResource(hResource, m_GroupIconName[IconIndex], RT_GROUP_ICON);

    // Find the group icon resource
    //hRsrc = FindResource(hResource, id, RT_GROUP_ICON);

    if (hRsrc == NULL)
        return GetLastError();

    if ((hGlobal = LoadResource(hResource, hRsrc)) == NULL)
        return GetLastError();

    if ((lpIcon = (LPMEMICONDIR)LockResource(hGlobal)) == NULL)
        return GetLastError();

    if ((lpIR = (LPICONRESOURCE) malloc(sizeof(ICONRESOURCE) + ((lpIcon->idCount-1) * sizeof(ICONIMAGE)))) == NULL)
        return GetLastError();

    lpIR->nNumImages = lpIcon->idCount;

    // Go through all the icons
    for (UINT i = 0; i < lpIR->nNumImages; ++i)
    {
        // Get the individual icon
        if ((hRsrc = FindResource(hResource, MAKEINTRESOURCE(lpIcon->idEntries[i].nID), RT_ICON )) == NULL)
        {
            free(lpIR);
            return GetLastError();
        }
        if ((hGlobal = LoadResource(hResource, hRsrc )) == NULL)
        {
            free(lpIR);
            return GetLastError();
        }
        // Store a copy of the resource locally
        lpIR->IconImages[i].dwNumBytes = SizeofResource(hResource, hRsrc);
        lpIR->IconImages[i].lpBits =(LPBYTE) malloc(lpIR->IconImages[i].dwNumBytes);
        if (lpIR->IconImages[i].lpBits == NULL)
        {
            free(lpIR);
            return GetLastError();
        }

        memcpy(lpIR->IconImages[i].lpBits, LockResource(hGlobal), lpIR->IconImages[i].dwNumBytes);

        // Adjust internal pointers
        if (!AdjustIconImagePointers(&(lpIR->IconImages[i])))
        {
            free(lpIR);
            return GetLastError();
        }
    }

    DWORD ret = WriteIconToICOString(lpIR,iconString);

    for (UINT i = 0; i < lpIR->nNumImages; ++i)
    {
        free(lpIR->IconImages[i].lpBits);
    }

    if (ret)
    {
        free(lpIR);
        return ret;
    }

    free(lpIR);

    return NO_ERROR;
}

DWORD CIconExtractor::WriteIconToICOString(LPICONRESOURCE lpIR, std::string& targetString)
{
	
	ULONG       dwBytesWritten  = 0;
	ULONG       totbytez        = 0;

	IStream* pStream = 0;
	CreateStreamOnHGlobal(0, TRUE, &pStream);
	LONG cbSize = 0;
	

    // Write the header
	ULONG totalHeaderBytes = 0;
	if (WriteICOHeader(pStream, lpIR->nNumImages, totalHeaderBytes))
		return GetLastError();
	//Aggiorno il numero di bytes scritti 
	totbytez += totalHeaderBytes;
        

    // Write the ICONDIRENTRY's
    for (UINT i = 0; i < lpIR->nNumImages; ++i)
    {
        ICONDIRENTRY    ide;

        // Convert internal format to ICONDIRENTRY
        ide.bWidth      = (BYTE)lpIR->IconImages[i].Width;
        ide.bHeight     = (BYTE)lpIR->IconImages[i].Height;
        ide.bReserved   = 0;
        ide.wPlanes     = lpIR->IconImages[i].lpbi->bmiHeader.biPlanes;
        ide.wBitCount   = lpIR->IconImages[i].lpbi->bmiHeader.biBitCount;

        if ((ide.wPlanes * ide.wBitCount) >= 8)
            ide.bColorCount = 0;
        else
            ide.bColorCount = 1 << (ide.wPlanes * ide.wBitCount);
        ide.dwBytesInRes = lpIR->IconImages[i].dwNumBytes;
        ide.dwImageOffset = CalculateImageOffset( lpIR, i );

        // Write the ICONDIRENTRY to disk
		pStream->Write(&ide, sizeof(ICONDIRENTRY), &dwBytesWritten);
		if (dwBytesWritten != sizeof(ICONDIRENTRY))
			return GetLastError();
		totbytez += dwBytesWritten;
    }

    // Write the image bits for each image
    for (UINT i = 0; i < lpIR->nNumImages; ++i)
    {
        DWORD dwTemp = lpIR->IconImages[i].lpbi->bmiHeader.biSizeImage;
        bool bError = false; // fix size even on error

        // Set the sizeimage member to zero
        lpIR->IconImages[i].lpbi->bmiHeader.biSizeImage = 0;
		pStream->Write(lpIR->IconImages[i].lpBits, lpIR->IconImages[i].dwNumBytes, &dwBytesWritten);
		if (dwBytesWritten != lpIR->IconImages[i].dwNumBytes)
			return GetLastError();
		totbytez += dwBytesWritten;
        // set it back
        lpIR->IconImages[i].lpbi->bmiHeader.biSizeImage = dwTemp;
        if (bError)
            return GetLastError();
    }
	
	LARGE_INTEGER li = { 0 };
	pStream->Seek(li, STREAM_SEEK_SET, NULL);

	HGLOBAL hBuf = 0;
	GetHGlobalFromStream(pStream, &hBuf);
	void* buffer = GlobalLock(hBuf);


	std::string icoString((char*)buffer,totbytez);
	//Pass the icon string
	targetString = icoString;
	
	
	GlobalUnlock(buffer);

	// Cleanup
	pStream->Release();

    return NO_ERROR;
}

DWORD CIconExtractor::CalculateImageOffset(LPICONRESOURCE lpIR, UINT nIndex) const
{
    DWORD   dwSize;

    // Calculate the ICO header size
    dwSize = 3 * sizeof(WORD);
    // Add the ICONDIRENTRY's
    dwSize += lpIR->nNumImages * sizeof(ICONDIRENTRY);
    // Add the sizes of the previous images
    for(UINT i = 0; i < nIndex; ++i)
        dwSize += lpIR->IconImages[i].dwNumBytes;

    return dwSize;
}

BOOL CIconExtractor::EnumResNameProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam)
{
	CIconExtractor* ptr = reinterpret_cast<CIconExtractor*>(lParam);
	return ptr->AddResourceProc(lpszType, lpszName);
}

DWORD CIconExtractor::WriteICOHeader(IStream* is, UINT nNumEntries, ULONG &totalBytes) const
{
    WORD    Output          = 0;
    ULONG   dwBytesWritten  = 0;
	

    // Write 'reserved' WORD
	
	if (is->Write(&Output, sizeof(WORD), &dwBytesWritten) != S_OK)
		return GetLastError();
	if (dwBytesWritten != sizeof(WORD) )
		return GetLastError();
	totalBytes += dwBytesWritten;
    // Write 'type' WORD (1)
    Output = 1;
	if (is->Write(&Output, sizeof(WORD), &dwBytesWritten) != S_OK)
		return GetLastError();
	if (dwBytesWritten != sizeof(WORD))
		return GetLastError();
	totalBytes += dwBytesWritten;
    // Write Number of Entries
    Output = (WORD)nNumEntries;
	if (is->Write(&Output, sizeof(WORD), &dwBytesWritten) != S_OK)
		return GetLastError();
	if (dwBytesWritten != sizeof(WORD))
		return GetLastError();
	totalBytes += dwBytesWritten;

    return NO_ERROR;
}

BOOL CIconExtractor::AdjustIconImagePointers(LPICONIMAGE lpImage)
{
    if (lpImage == NULL)
        return FALSE;

    // BITMAPINFO is at beginning of bits
    lpImage->lpbi = (LPBITMAPINFO)lpImage->lpBits;
    // Width - simple enough
    lpImage->Width = lpImage->lpbi->bmiHeader.biWidth;
    // Icons are stored in funky format where height is doubled - account for it
    lpImage->Height = (lpImage->lpbi->bmiHeader.biHeight)/2;
    // How many colors?
    lpImage->Colors = lpImage->lpbi->bmiHeader.biPlanes * lpImage->lpbi->bmiHeader.biBitCount;
    // XOR bits follow the header and color table
    lpImage->lpXOR = (PBYTE)FindDIBBits((LPSTR)lpImage->lpbi);
    // AND bits follow the XOR bits
    lpImage->lpAND = lpImage->lpXOR + (lpImage->Height*BytesPerLine((LPBITMAPINFOHEADER)(lpImage->lpbi)));

    return TRUE;
}

LPSTR CIconExtractor::FindDIBBits(LPSTR lpbi)
{
   return (lpbi + *(LPDWORD)lpbi + PaletteSize(lpbi));
}

WORD CIconExtractor::PaletteSize(LPSTR lpbi)
{
    return (DIBNumColors(lpbi) * sizeof(RGBQUAD));
}

DWORD CIconExtractor::BytesPerLine(LPBITMAPINFOHEADER lpBMIH) const
{
    return WIDTHBYTES(lpBMIH->biWidth * lpBMIH->biPlanes * lpBMIH->biBitCount);
}

WORD CIconExtractor::DIBNumColors(LPSTR lpbi) const
{
    WORD wBitCount;
    DWORD dwClrUsed;

    dwClrUsed = ((LPBITMAPINFOHEADER) lpbi)->biClrUsed;

    if (dwClrUsed)
        return (WORD) dwClrUsed;

    wBitCount = ((LPBITMAPINFOHEADER) lpbi)->biBitCount;

    switch (wBitCount)
    {
        case 1: return 2;
        case 4: return 16;
        case 8: return 256;
        default:return 0;
    }
    //return 0;
}

BOOL CIconExtractor::AddResourceProc(LPCTSTR lpszType, LPTSTR lpszName)
{

	if (m_GKounter >= MAX_ICONS || m_GNameKounter >= MAX_ICONS)
		return TRUE;

	if (lpszType == RT_GROUP_ICON)
	{
		if ((ULONG)lpszName < 65536)//ID
		{
			m_GroupIconID[m_GKounter] = (ULONG)lpszName;
			m_GKounter += 1;
		}
		else //NAME
		{
			m_GroupIconName[m_GNameKounter] = lpszName;
			m_GNameKounter++;
		}
	}
	return TRUE;
}



