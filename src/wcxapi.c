//==============================
// wcxapi.c
// WCX DLL Interface
//==============================
#ifdef __cplusplus
extern "C" {
#endif

#include "wcxapi.h"
#include "defs.h"
#include "resource.h"

tProcessDataProc ProcessDataProc;

extern HINSTANCE	hDllInst;

extern void GetCfgPath(void);
extern HANDLE ADX_OpenArchive(tOpenArchiveData *ArchiveData);
extern int ADX_ReadHeader(HANDLE hArcData, tHeaderData *HeaderData);
extern int ADX_ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName);
extern int ADX_CloseArchive(HANDLE hArcData);
extern int ADX_PackFiles (char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags);
extern int ADX_DeleteFiles (char *PackedFile, char *DeleteList);
extern int ADX_GetPackerCaps(void);
extern void ADX_ConfigurePacker (HWND Parent, HINSTANCE DllInstance);
extern void ADX_SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1);
extern void ADX_SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc);
extern void ADX_ConfigurePacker (HWND Parent, HINSTANCE DllInstance);

//------------[ DLL API ]--------------------------

BOOL APIENTRY DllMain( HANDLE hinstDLL, DWORD fdwReason, LPVOID lpReserved )
{
    if( !hinstDLL )
        return FALSE;

    switch( fdwReason )
    {
        case DLL_PROCESS_ATTACH:
            DebugString( "process attach" );
			GetCfgPath();
            break;
        case DLL_THREAD_ATTACH:
            DebugString( "thread attach" );
            break;
        case DLL_THREAD_DETACH:
            DebugString( "thread detach" );
            break;
        case DLL_PROCESS_DETACH:
            DebugString( "process detach" );
            break;
    }

    return TRUE;
}

WCX_API HANDLE STDCALL OpenArchive(tOpenArchiveData *ArchiveData)
{
	DebugString( "open archive" );
	return ADX_OpenArchive( ArchiveData );
}

WCX_API int	STDCALL ReadHeader(HANDLE hArcData, tHeaderData *HeaderData)
{
	DebugString( "read header" );
	return ADX_ReadHeader( hArcData, HeaderData );
}

WCX_API int	STDCALL ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName)
{
	DebugString( "process file" );
	return ADX_ProcessFile( hArcData, Operation, DestPath, DestName );
}

WCX_API int	STDCALL CloseArchive(HANDLE hArcData)
{
	DebugString( "close archive" );
	return ADX_CloseArchive( hArcData );
}

WCX_API int	STDCALL PackFiles(char *PackedFile, char *SubPath, char *SrcPath, 
							  char *AddList, int Flags)
{
	DebugString( "pack files" );
	return ADX_PackFiles( PackedFile, SubPath, SrcPath, AddList, Flags );
}

WCX_API int	STDCALL DeleteFiles(char *PackedFile, char *DeleteList)
{
	DebugString( "delete files" );
	return ADX_DeleteFiles( PackedFile, DeleteList );
}

WCX_API int STDCALL GetPackerCaps(void)
{
	DebugString( "get packer caps." );
	// Return capabilities 
	return PK_CAPS_OPTIONS|PK_CAPS_MULTIPLE|PK_CAPS_NEW|PK_CAPS_DELETE|PK_CAPS_MODIFY;
}

WCX_API void STDCALL ConfigurePacker (HWND Parent, HINSTANCE DllInstance)
{
	DebugString( "configure packer" );
	ADX_ConfigurePacker (Parent, DllInstance);
}

WCX_API void STDCALL SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1)
{
	// Not implemented
}

WCX_API void STDCALL SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc)
{
	ProcessDataProc = pProcessDataProc;
}

#ifdef __cplusplus
}
#endif
