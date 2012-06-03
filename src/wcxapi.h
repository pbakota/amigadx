/*
//Copyright (C) 2002 Peter Bakota
========================================
 $Name:         wcxapi.h

 $Desc:         Windows Commander API definitions

 $Author:       Peter Bakota <bakota@tippnet.co.yu>
 $Revision:     1
 $Date:         13.01.2002 8:38:08 PM
 $Comments:     This code based on Christian Ghisler (support@ghisler.com) sources
========================================
*/
#ifndef __WCXAPI_H
#define __WCXAPI_H


#ifdef __cplusplus
extern "C" {
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "wcxhead.h"

#ifdef AMIGADX_EXPORTS
#define WCX_API __declspec(dllexport)
#define STDCALL __stdcall
#else
#define WCX_API
#define STDCALL
#endif

// Windows Commander Interface
WCX_API HANDLE	STDCALL OpenArchive(tOpenArchiveData *ArchiveData);
WCX_API int		STDCALL ReadHeader(HANDLE hArcData, tHeaderData *HeaderData);
WCX_API int		STDCALL ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName);
WCX_API int		STDCALL CloseArchive(HANDLE hArcData);
WCX_API int		STDCALL PackFiles (char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags);
WCX_API int		STDCALL DeleteFiles (char *PackedFile, char *DeleteList);
WCX_API int		STDCALL GetPackerCaps(void);
WCX_API void	STDCALL ConfigurePacker (HWND Parent, HINSTANCE DllInstance);
WCX_API void	STDCALL SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1);
WCX_API void	STDCALL SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc);

#ifdef __cplusplus
}
#endif

#endif //!__WCX_API
