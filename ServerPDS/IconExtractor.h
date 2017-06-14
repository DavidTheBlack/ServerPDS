#pragma once

const int MAX_ICONS = 5;

// The following structures are taken from iconpro sdk example
#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
    BYTE    bWidth;               // Width of the image
    BYTE    bHeight;              // Height of the image (times 2)
    BYTE    bColorCount;          // Number of colors in image (0 if >=8bpp)
    BYTE    bReserved;            // Reserved
    WORD    wPlanes;              // Color Planes
    WORD    wBitCount;            // Bits per pixel
    DWORD   dwBytesInRes;         // how many bytes in this resource?
    WORD    nID;                  // the ID
} MEMICONDIRENTRY, *LPMEMICONDIRENTRY;

typedef struct
{
    WORD            idReserved;   // Reserved
    WORD            idType;       // resource type (1 for icons)
    WORD            idCount;      // how many images?
    MEMICONDIRENTRY idEntries[1]; // the entries for each image
} MEMICONDIR, *LPMEMICONDIR;
#pragma pack( pop )

typedef struct
{
    UINT            Width, Height, Colors; // Width, Height and bpp
    LPBYTE          lpBits;                // ptr to DIB bits
    DWORD           dwNumBytes;            // how many bytes?
    LPBITMAPINFO    lpbi;                  // ptr to header
    LPBYTE          lpXOR;                 // ptr to XOR image bits
    LPBYTE          lpAND;                 // ptr to AND image bits
} ICONIMAGE, *LPICONIMAGE;
typedef struct
{
    BOOL        bHasChanged;                     // Has image changed?
    UINT        nNumImages;                      // How many images?
    ICONIMAGE   IconImages[1];                   // Image entries
} ICONRESOURCE, *LPICONRESOURCE;


// These next two structs represent how the icon information is stored
// in an ICO file.
typedef struct
{
    BYTE    bWidth;               // Width of the image
    BYTE    bHeight;              // Height of the image (times 2)
    BYTE    bColorCount;          // Number of colors in image (0 if >=8bpp)
    BYTE    bReserved;            // Reserved
    WORD    wPlanes;              // Color Planes
    WORD    wBitCount;            // Bits per pixel
    DWORD   dwBytesInRes;         // how many bytes in this resource?
    DWORD   dwImageOffset;        // where in the file is this image
} ICONDIRENTRY, *LPICONDIRENTRY;
typedef struct
{
    WORD            idReserved;   // Reserved
    WORD            idType;       // resource type (1 for icons)
    WORD            idCount;      // how many images?
    ICONDIRENTRY    idEntries[1]; // the entries for each image
} ICONDIR, *LPICONDIR;


class CIconExtractor
{

public:
    
    DWORD ExtracttIcon(HINSTANCE hResource, LPCTSTR TargetICON);
	CIconExtractor();

protected:

	BOOL AddResourceProc(LPCTSTR lpszType, LPTSTR lpszName);

private:
	DWORD WriteIconToICOFile(LPICONRESOURCE lpIR, LPCTSTR szFileName);
    BOOL AdjustIconImagePointers(LPICONIMAGE lpImage);
    LPSTR FindDIBBits(LPSTR lpbi);
    WORD DIBNumColors(LPSTR lpbi) const;
    WORD PaletteSize(LPSTR lpbi);
    DWORD BytesPerLine(LPBITMAPINFOHEADER lpBMIH) const;
    DWORD WriteICOHeader(HANDLE hFile, UINT nNumEntries) const;
    DWORD CalculateImageOffset(LPICONRESOURCE lpIR, UINT nIndex) const;
	   

	ULONG		 m_GroupIconID[MAX_ICONS];
	LPTSTR      m_GroupIconName[MAX_ICONS];

	UINT 		 m_GKounter;
	UINT         m_GNameKounter;

	static BOOL CALLBACK EnumResNameProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam);

};

