/*
//Copyright (C) 2002 Peter Bakota
//License: GNU GPL Version 2 (*not* later versions)
========================================
 $Name:         support.h

 $Desc:         Support header file

 $Author:       Peter Bakota <bakota@tippnet.co.yu>
 $Revision:     1
 $Date:         5/3/2002 11:28:38 PM
 $Comments:     21/06/2005 Added support for FDI disk image format
========================================
*/

#ifdef __cplusplus
extern "C" {
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "xDMS.h"
#include "zLib.h"
#include "types.h"

typedef enum adfDiskFormat_e {T_ADF, T_ADZ, T_DMS, T_FDI} DiskFormat_t;

BOOL GZDecompress(HWND win, char *infile, char *outfile);
DiskFormat_t VolGetFormat(char *szFileName, BOOL *dwError);

#ifdef __cplusplus
}
#endif
