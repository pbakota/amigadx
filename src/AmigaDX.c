/*
//Copyright (C) 2002 Peter Bakota
========================================
 $Name:         AmigaDX.c

 $Desc:         WinUAE .ADF,.ADZ,.DMS,.HDF and .DMP file support under Windows Commander version 4 and above.

 $Author:       Peter Bakota <bakota@gmail.com>
 $Revision:     3.3
 $Date:         04.03.2002 11:07:30 PM - First release
				24.08.2002 14:51:20 PM - BUGFIX: The extract always must to begin at root directory!
				01.09.2002 17:07:02 PM - BUGFIX: Extract directory now handled correctly! I hope so :-)
				11.04.2003 11:24:30 PM - BUGFIX: Fixed bug in CloseArchive a nasty adf_count variable from previous version, Bug reported by Anony Mousse.
				24.04.2003 11:42:00 PM - NEW FEATURE: Make your own ADF file from Total Commander!
				25.04.2003 12:38:20 PM - Code optimisation!
				18.05.2003 11:15:00 PM - BUGFIX: Fixed bug in TEST archive access violation if no DestName specified
				18.05.2003 11:15:00 PM - BUGFIX: Fixed bug in PackAmigaFile, if the disk image is full the last PC file is not closed.
				14.06.2003 17:08:52 PM - IMPROVEMENT: Added ADFLib Version number ;-)
				18.08.2004 20:11:00 PM - BUGFIX: Fixed bug in VolGetFormat found by Dirk Trowe
				18.08.2004 21:10:21 PM - IMPROVEMENT: Added support for non standard 81 cylinder floppy
				20.08.2004 10:12:10 AM - BUGFIX: Fixed bug in PackFiles i forgot to add PreprocessPath when packing into subdir. sily me :-)
				21.06.2005 11:33:14 AM - IMPROVEMENT: New adflib 0.7.10 added
				21.06.2005 11:40:40 AM - BUGFIX: Fixed bug in adflib: adfReadBitmap function
				22.06.2005 12:23:14 AM - IMPROVEMENT: Added support for non standard floppy up to 83 cylinder
				22.06.2005 14:40:23 PM - TV 6.5 autoinstall
				26.06.2005 11:30:20 AM - BUGFIX: Fixed bug with toooolong (>260!!!?) file names.
				26.06.2005 11:30:20 AM - IMPROVEMENT: Maked workaround for circular references on disk
				28.07.2007             - Ported to MingW and Code::Blocks for source code release.
                03.06.2012             - Fixed bug in MakeDirList
                03.06.2012             - Ported to win64 platform.

 $Comments:		TODO:
				- Move to arhive feature not 100% supported (Directories not deleted!)
				- FIXME: Bug? in "delete from adf file", If user quick deletes files from adf archive
				the plugin don't refresh file list, which cause some failure in reading this file from archive
				but don't worry! Simple u can fix problem by quiting from total commander and entering again
				after this the deleted file is really gone! You can eliminate this bug by selecting checkbox
				in options dialog. The hack will go into sleep for som short time after deleting to allow
				system to wake up and refresh file list ;-)
========================================
*/


#include "wcxapi.h"

#include <assert.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#ifdef __MINGW32__
#include <unistd.h>
#endif

#include "adflib.h"

#define PACKER_SUPPORT		// Comment out this if you don't want DMS or ADZ support :-<
#define USE_DELETE_HACK		// Comment out this if you don't want hacking :-)

#ifdef PACKER_SUPPORT
#include "support.h"
#endif

#include "defs.h"
#include "resource.h"

#pragma GCC diagnostic ignored "-Wpointer-sign"
#pragma GCC diagnostic ignored "-Wunused-value"

//
// adf Directory tree
//
typedef struct FileList_s {
	UCHAR szFileName[MAX_PATH * 32];	/* File name with full path */
	DWORD dwSize;					/* File size in bytes */
	DWORD dwFileDate;				/* File date (Amiga format) */
	UCHAR szComment[80];			/* Comment */
	DWORD dwAttr;					/* File attribute */
	struct FileList_s *lpNext;		/* Next entry */
} FileList_t;

#define ADFF_PACKED 0x0001

// Az allomanyok adatai
typedef struct AdfHandle_s {
	unsigned char	szAdfArcName[MAX_PATH * 16];// adf file name
	struct Device	*lpAdfDevice;			// device ptr
	struct Volume   *lpAdfVolume;			// volume ptr
	unsigned		dwAdfFlags;				// Flags
	UCHAR			szPath[MAX_PATH * 16];
	FileList_t		*lpFileListThis;		// Current file path
	FileList_t		*lpFileListHead;
	FileList_t		*lpFileListCur;
	BOOL			mbInvalidEntryFound;
	char			mInvalidReplacer;
	BOOL			mbEnableWarnings;
} AdfHandle_t;

HINSTANCE	hDllInst;
//const char szCfgFile[]		= "wcplugin.ini";
char szCfgFile[MAX_PATH];
const char szCfgKey[]		= "amigadx";
const char szDefaultLabel[]	= "Created with AmigaDX Plugin!";


/* Std. bootblock code */
extern unsigned char bbkData[];
extern tProcessDataProc ProcessDataProc;
extern BOOL CALLBACK ConfigDlgProc(HWND hwndDlg,UINT  uMsg,WPARAM wParam,LPARAM  lParam);

#ifdef DEBUG
#define __DEBUG__ d_printf
/*
=========================
 Debug stuff
=========================
*/
#include <time.h>
static void d_printf(const char *fmt, ...)
{
	va_list va;
	char buf1[256];

	va_start(va, fmt);
	vsprintf( buf1, fmt, va);
	va_end(va);

	DebugString( buf1 );
}
#else
#define __DEBUG__
#endif

/*
========================================
 FreeDirList
========================================
*/
void GetCfgPath(void)
{
	char path[MAX_PATH];

	if( GetModuleFileName( GetModuleHandle( NULL ), path, sizeof(path) ))
	{
		char *p = strrchr(path, '\\');
		if( p )
			*++p = '\0';
		else
			path[0] = '\0';

	}
	else
		path[0] = '\0';

	strcpy(szCfgFile, path);
	strcat(szCfgFile, "plugins\\wcx\\AmigaDX\\AmigaDX.ini");

//	MessageBox(0, szCfgFile, "AmigaDX", MB_OK);
}

/*
========================================
 FreeDirList
========================================
*/
static void FreeDirList(AdfHandle_t *lpHandler)
{
	FileList_t *lpNext;
	while( lpHandler->lpFileListHead ) {
		lpNext = lpHandler->lpFileListHead->lpNext;
		free( lpHandler->lpFileListHead );
		lpHandler->lpFileListHead = lpNext;
	}
}

/*
========================================
 strcat_filtered
========================================
*/
void strcat_filtered(char *to, char *from, char replacer)
{
	/* Replace invalid chars */
	char *p1, *p2;
	p1=to;
	while(*p1!=0) ++p1;
	for(p2=from;*p2!=0; ++p1, ++p2) {
		if(*p2=='*' || *p2=='?' || (*p2 & 127)<32)
			*p1=replacer;
		else
			*p1 = *p2;
	}
	*p1=0;

}

/*
========================================
 MakeDirList
========================================
*/
static int MakeDirList( AdfHandle_t *lpHandler, struct Volume *lpVolume, UCHAR *lpDirName )
{
	int rc =0;
	int nPathLen;
	struct List *lpList, *lpCell;

	if( strlen(lpDirName) ) {

		nPathLen = strlen( lpHandler->szPath );
		if( nPathLen )
			sprintf( lpHandler->szPath, "%s%s/", lpHandler->szPath, lpDirName );
		else
			sprintf( lpHandler->szPath, "%s/", lpDirName );

		if( adfChangeDir( lpVolume, lpDirName )!=RC_OK )
			return E_BAD_ARCHIVE;
	} else {

		lpHandler->lpFileListHead = lpHandler->lpFileListCur = NULL;
		lpHandler->szPath[0]='\0';
		nPathLen = 0;
	}

	__DEBUG__("entry: %s", strlen(lpHandler->szPath)!=0 ? (void *)lpHandler->szPath : "(root)");
	lpCell = lpList = adfGetDirEnt(lpVolume, lpVolume->curDirPtr);
	while( lpCell && rc==0 ) {
		FileList_t *lpNew = malloc( sizeof(FileList_t) );
		if( lpNew==NULL ) {
			rc = E_NO_MEMORY;
			break;
		}
		ZeroMemory(lpNew, sizeof(FileList_t));

		if( lpHandler->lpFileListHead==NULL )
			lpHandler->lpFileListHead = lpNew;

		if( lpHandler->lpFileListCur )
			lpHandler->lpFileListCur->lpNext = lpNew;

		lpHandler->lpFileListCur = lpNew;

		/* Copy path */
		assert(strlen(lpHandler->szPath)<sizeof(lpNew->szFileName));
		if(lpHandler->mInvalidReplacer!=0) {
			strcat_filtered( lpNew->szFileName, lpHandler->szPath, lpHandler->mInvalidReplacer );
		} else
			strcpy(lpNew->szFileName, lpHandler->szPath);

		/* Copy file name */
		assert(strlen(((struct Entry *)lpCell->content)->name)<sizeof(lpNew->szFileName));
		if(lpHandler->mInvalidReplacer!=0) {
			strcat_filtered( lpNew->szFileName, ((struct Entry *)lpCell->content)->name, lpHandler->mInvalidReplacer );
		} else
			strcat(lpNew->szFileName, ((struct Entry *)lpCell->content)->name);

		// 26.06.2005: Dropped out it is useless anyway
		//strcpy(lpNew->szComment, ((struct Entry *)lpCell->content)->comment);
		lpNew->dwFileDate	= (((struct Entry *)lpCell->content)->year - 1980) << 25 |
				((struct Entry *)lpCell->content)->month << 21 |
				((struct Entry *)lpCell->content)->days  << 16 |
				((struct Entry *)lpCell->content)->hour  << 11 |
				((struct Entry *)lpCell->content)->mins  << 5  |
				((struct Entry *)lpCell->content)->secs/2;
		if( ((struct Entry *)lpCell->content)->type==ST_DIR ) {
			lpNew->dwAttr  = 0x10;
			lpNew->dwSize = 0;
            char saved[MAX_PATH*15];
            strcpy(saved,lpHandler->szPath);
			rc = MakeDirList( lpHandler, lpVolume, ((struct Entry *)lpCell->content)->name );
            strcpy(lpHandler->szPath,saved);
		} else {
			lpNew->dwAttr = 0x00;
			lpNew->dwSize  = ((struct Entry *)lpCell->content)->size;
		}
		lpCell = lpCell->next;
	}
	adfFreeDirList( lpList );

	lpHandler->szPath[nPathLen]='\0';
	if( adfParentDir( lpVolume )!=RC_OK ) rc = E_BAD_ARCHIVE;

	return rc;
}

/*
========================================
 PreprocessPath
========================================
*/
static int PreprocessPath(struct Volume *lpVolume, char *lpFullPath, char *lpBuf)
{
	int n = 0;

	while( *lpFullPath ) {
		if( *lpFullPath!='\\' && *lpFullPath!='/' )
			lpBuf[n++]=*lpFullPath;
		else {
			lpBuf[n] = '\0';
			n = 0;
			if( adfChangeDir(lpVolume, lpBuf)!=RC_OK )
				return E_BAD_DATA;
		}
		++lpFullPath;
	}
	lpBuf[n] = '\0';
	return 0;
}

/*
========================================
Function name	: STDCALL OpenArchive
Description	    : Main entry point to Windows Commander
Return type		: WCX_API HANDLE
Date-Time       : 04.03.2002 11:10:15 PM
Argument        : tOpenArchiveData *ArchiveData
========================================
*/
HANDLE ADX_OpenArchive(tOpenArchiveData *ArchiveData)
{
	AdfHandle_t *lpHandler = (AdfHandle_t *)malloc(sizeof(AdfHandle_t));
	char szBuf[256];

#ifdef PACKER_SUPPORT
	DiskFormat_t type;
	BOOL dwUnpackError;
#endif

	__DEBUG__("openarchive()");
	__DEBUG__("MAX_PATH=%d", MAX_PATH);
	// Check for out of memory
	if(lpHandler==NULL) {
		ArchiveData->OpenResult = E_NO_MEMORY;
		return 0;
	}
	ZeroMemory(lpHandler, sizeof(AdfHandle_t));

	GetCfgPath();
	GetPrivateProfileString(szCfgKey, "EnableWarning", "1", szBuf, sizeof(szBuf), szCfgFile);
	lpHandler->mbEnableWarnings = atoi(szBuf);
	GetPrivateProfileString(szCfgKey, "InvalidCharReplacer", "_", szBuf, sizeof(szBuf), szCfgFile);
	lpHandler->mInvalidReplacer = szBuf[0];

	// Copy archive name to handler
	strcpy(lpHandler->szAdfArcName, ArchiveData->ArcName);

	// Default responding
	ArchiveData->OpenResult = E_BAD_ARCHIVE;

#ifdef PACKER_SUPPORT
	type = VolGetFormat(lpHandler->szAdfArcName, &dwUnpackError);
	if(dwUnpackError) {
		free(lpHandler);
		return 0;
	}
	if(type==T_DMS || type==T_ADZ || type==T_FDI)
		lpHandler->dwAdfFlags = ADFF_PACKED;
#endif

	// Init ADF system
	adfEnvInitDefault();
#ifdef DEBUG
	adfChgEnvProp(PR_VFCT, d_printf);
	adfChgEnvProp(PR_WFCT, d_printf);
	adfChgEnvProp(PR_EFCT, d_printf);
#endif

	// Mount a ReadOnly volume!
	if((lpHandler->lpAdfDevice = adfMountDev(lpHandler->szAdfArcName, TRUE))==NULL) {
		adfEnvCleanUp();
		free( lpHandler );
		return 0;
	}
	if((lpHandler->lpAdfVolume = adfMount(lpHandler->lpAdfDevice, 0, TRUE))==NULL) {
		adfUnMountDev(lpHandler->lpAdfDevice);
		adfEnvCleanUp();
		free( lpHandler );
		return 0;
	}

	if( (ArchiveData->OpenResult = MakeDirList( lpHandler, lpHandler->lpAdfVolume, "" )) ) {
		free( lpHandler );
		return 0;
	}
	/*
	 * Reset entry list
	 */
	lpHandler->lpFileListCur = lpHandler->lpFileListHead;
	lpHandler->mbInvalidEntryFound = FALSE; // Reset invalid entry flag

	return (HANDLE)lpHandler;
}

/*
========================================
 ReadHeader
========================================
*/
int	ADX_ReadHeader(HANDLE hArcData, tHeaderData *HeaderData)
{
	AdfHandle_t *lpHandler = (AdfHandle_t *)hArcData;

	__DEBUG__("ReadHeader: %s", lpHandler->lpFileListCur ? (void *)lpHandler->lpFileListCur->szFileName : "(end-of-list)");
	if( lpHandler->lpFileListCur ) {
		UCHAR *lpPtr;

		if( strlen(lpHandler->lpFileListCur->szFileName)>sizeof(HeaderData->FileName))
			lpHandler->mbInvalidEntryFound = TRUE;

		strncpy( HeaderData->FileName, lpHandler->lpFileListCur->szFileName, sizeof(HeaderData->FileName) );
		HeaderData->FileTime = lpHandler->lpFileListCur->dwFileDate;
		HeaderData->FileAttr = lpHandler->lpFileListCur->dwAttr;
		HeaderData->UnpSize  = lpHandler->lpFileListCur->dwSize;
		// The file comment feature is not supported in WinCmd at the moment, but may be in the future.
		HeaderData->CmtBuf   = lpHandler->lpFileListCur->szComment;
		HeaderData->CmtBufSize = 80;
		HeaderData->CmtSize  = strlen(lpHandler->lpFileListCur->szComment);
		// Replace amiga dir separator with pc backslash
		lpPtr = HeaderData->FileName;
		while(*lpPtr) {
			if(*lpPtr=='/') *lpPtr='\\';
			++lpPtr;
		}
		lpHandler->lpFileListThis = lpHandler->lpFileListCur;
		lpHandler->lpFileListCur = lpHandler->lpFileListCur->lpNext;
		return 0;
	}

	if( lpHandler->mbInvalidEntryFound && lpHandler->mbEnableWarnings ) {

		MessageBox(0,
			TEXT("Sorry! But inside this disk image some file or directory\n"
			"names are larger than expected by Total Commander.\n"
			"Because of that some file or directory names are stripped!"),
			"AmigaDX Plugin - Warning", MB_ICONWARNING|MB_OK);

		lpHandler->mbInvalidEntryFound = FALSE; // Reset invalid entry flag
	}

	return E_END_ARCHIVE;
}

/*
========================================
 IsDirExists
========================================
*/
static BOOL IsDirExists(UCHAR *lpDir)
{
	UCHAR szCurrDir[260];
	BOOL res;
	if( GetCurrentDirectory(sizeof(szCurrDir), szCurrDir)==0 ) return FALSE;
	res = SetCurrentDirectory(lpDir);
	SetCurrentDirectory(szCurrDir);
	return res;
}

/*
========================================
 ExtractAmigaFile
========================================
*/
static int ExtractAmigaFile(struct Volume *lpVolume, UCHAR *lpAmiFileName, UCHAR *lpDest)
{
	int			rc;
	UCHAR		szName[256], *lpszName;
	struct File *lpAmiFile;
	FILE		*PcFile;
	DWORD		nReadLength, nBytes;
	BOOL		abort=0;
	UCHAR		szCopyBuf[4096];

	PcFile = fopen(lpDest, "wb");
	if( PcFile==NULL ) {
		if( errno==ENOENT ) {
			strcpy(szName, lpDest);
			lpszName = strrchr(szName, '\\'); *lpszName='\0';
			if( IsDirExists( szName )==FALSE ) {
				if( CreateDirectory( szName, NULL )!=0 )
					return E_EWRITE;
			}
			PcFile = fopen(lpDest, "w");
			if( PcFile==NULL )
				return E_EWRITE;
		} else
			return E_EWRITE;
	}

	if( PreprocessPath( lpVolume, lpAmiFileName, szName) == 0 ) {
		lpAmiFile = adfOpenFile( lpVolume, szName, "r" );
		if( lpAmiFile!=NULL ) {
			rc = 0;
			nBytes = 0; //lpAmiFile->fileHdr->byteSize;
			nReadLength = adfReadFile(lpAmiFile, sizeof(szCopyBuf), szCopyBuf);
			while( !adfEndOfFile( lpAmiFile ) && abort==0 && rc==0) {
				rc = fwrite(szCopyBuf, sizeof(UCHAR), nReadLength, PcFile)!=nReadLength ? E_EREAD : 0;
				nReadLength = adfReadFile(lpAmiFile, sizeof(szCopyBuf), szCopyBuf);

				if( ProcessDataProc ) {
					abort = ProcessDataProc(lpAmiFileName, nBytes )==0;
					nBytes += nReadLength;
				}

			}
			if( nReadLength>0 && rc==0 )
				rc = fwrite(szCopyBuf, sizeof(UCHAR), nReadLength, PcFile)!=nReadLength ? E_EREAD : 0;

			adfCloseFile( lpAmiFile );
		} else
			rc = E_EREAD;
	} else {
		__DEBUG__("Can't change directory!");
		rc = E_EREAD;
	}
	fclose( PcFile );

	return rc;
}

/*
========================================
 ProcessFile
========================================
*/
int	ADX_ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName)
{
	int rc = 0;
	switch(Operation) {
		case PK_SKIP:
			return 0;
		case PK_TEST:	// There is no TEST mode for ADF files so EXTRACT is applied
		case PK_EXTRACT:
			{
			AdfHandle_t *lpHandler = (AdfHandle_t*)hArcData;

			__DEBUG__("extract/test file: %s -> %s", lpHandler->lpFileListThis->szFileName, DestName);
			if( DestName == NULL ) {
				// In case if DestName not exits (TEST),
				// we make one temp file in system's temp directory
				UCHAR szTempDir[MAX_PATH];
				UCHAR szTempName[MAX_PATH * 2];

				GetTempPath( sizeof(szTempDir), szTempDir );
				if( GetTempFileName( szTempDir, "$$$", 0, szTempName )==0 )
					return rc;
				else {
					rc = ExtractAmigaFile( lpHandler->lpAdfVolume,
						lpHandler->lpFileListThis->szFileName,
						szTempName );
					DeleteFile( szTempName );
				}
			} else {
				rc = ExtractAmigaFile( lpHandler->lpAdfVolume,
					lpHandler->lpFileListThis->szFileName,
					DestName );
			}
			if( adfToRootDir( lpHandler->lpAdfVolume )!=RC_OK )
				return E_BAD_ARCHIVE;
			}
	}
	return rc;
}

/*
========================================
 CloseArchive
========================================
*/
int	ADX_CloseArchive(HANDLE hArcData)
{
	AdfHandle_t *lpHandler = (AdfHandle_t *)hArcData;
	__DEBUG__("closearchive()");

	FreeDirList( lpHandler );
	adfUnMount( lpHandler->lpAdfVolume );
	adfUnMountDev(lpHandler->lpAdfDevice );
	adfEnvCleanUp();

#ifdef PACKER_SUPPORT
	// If packed then delete temp file!
	if(lpHandler->dwAdfFlags & ADFF_PACKED)
		unlink(lpHandler->szAdfArcName);
#endif

	free( lpHandler );
	return 0;
}

/*
========================================
 MakeDirectoryTree
========================================
*/
static int MakeDirectoryTree( struct Volume *lpVolume, char *lpFullPath, char *lpBuf )
{
	int n=0;
	int rc=0;

	while( *lpFullPath && rc==0 ) {
		if( *lpFullPath!='\\' )
			lpBuf[n++]=*lpFullPath++;
		else {
			++lpFullPath;
			lpBuf[n] = '\0';
			n = 0;
			if( adfChangeDir( lpVolume, lpBuf )!=RC_OK ) {
				if( adfCreateDir( lpVolume, lpVolume->curDirPtr, lpBuf)!=RC_OK ) {
					rc = E_ECREATE;
				} else
					/*
					 * Try again: enter into directory
					 */
					if( adfChangeDir( lpVolume, lpBuf )!=RC_OK )
						rc = E_ECREATE;
			}
		}
	}
	lpBuf[n] = '\0';
	return rc;
}

/*
========================================
 PackAmigaFile
========================================
*/
#define MAXNAMELEN 30 // With floppy disk the max filename is 30 bytes
static int PackAmigaFile(struct Volume *lpVolume, UCHAR *lpSrcPath, UCHAR *lpAmiDest)
{
	struct File *lpAmiFile;
	FILE		*PcFile;
	DWORD		nReadLength, nBytes;
	BOOL		abort=0;
	int			rc, nSize;
	UCHAR		szName[256], szSrcName[256];
	UCHAR		szCopyBuf[4096];

	rc = MakeDirectoryTree( lpVolume, lpAmiDest, szName);
	if( rc==0 ) {

		if( strlen(szName)>MAXNAMELEN ) {
			__DEBUG__("File name is too long!: %s", szName);
			return E_EWRITE;
		}

		lpAmiFile = adfOpenFile( lpVolume, szName, "r" );
		if(lpAmiFile!=NULL) {
			adfCloseFile(lpAmiFile);
			/* Remove "old" file
			 */
			if( (rc = adfRemoveEntry( lpVolume, lpVolume->curDirPtr, szName))!=RC_OK )
				return E_EWRITE;
		}
		sprintf(szSrcName, "%s%s", lpSrcPath, lpAmiDest );
		__DEBUG__("pack: %s -> %s", szSrcName, lpAmiDest);
		PcFile = fopen(szSrcName, "rb");
		if( PcFile==NULL )
			rc = E_EWRITE;
		else {
			fseek(PcFile, 0, SEEK_END);
			nSize = ftell(PcFile);
			fseek(PcFile, 0, SEEK_SET);

			/*
			 * Hmm, ADF Opus has a bug at here (when there is no enough free space on disk!),
			 * the adfCountFreeBlocks will return all free blocks including
			 * boot(2x), root & bitmap sectors as free too!! where bitmap isn't mater but if we don't have it the disk
			 * validator is always started when the disk inserted (of course this will happening on REAL amiga)
			 * from the plugin side we don't need it! But never knows, better idea is to keep some space
			 * on the disk for this blocks. The expression is: 2xboot+root+bitmap=4 -- peterb
			 */
			if( adfFileRealSize( nSize, LOGICAL_BLOCK_SIZE, NULL,NULL) < adfCountFreeBlocks( lpVolume ) - 4 ) {
				lpAmiFile = adfOpenFile( lpVolume, szName, "w" );
				if( lpAmiFile ) {
					nBytes = 0; //nSize;
					nReadLength = fread(szCopyBuf, sizeof(UCHAR), sizeof(szCopyBuf), PcFile);
					while( !feof( PcFile ) && abort==0 && rc==0) {
						rc = (DWORD)adfWriteFile(lpAmiFile, nReadLength, szCopyBuf)!=nReadLength ? E_EWRITE : 0;
						nReadLength = fread(szCopyBuf, sizeof(UCHAR), sizeof(szCopyBuf), PcFile);

						if( ProcessDataProc ) {
							abort = ProcessDataProc(lpAmiDest, nBytes )==0;
							nBytes += nReadLength;
						}

					}
					if( nReadLength>0 && rc==0 )
						rc = (DWORD)adfWriteFile(lpAmiFile, nReadLength, szCopyBuf)!=nReadLength ? E_EWRITE : 0;

					adfCloseFile(lpAmiFile);
				} else
					rc = E_EWRITE;
			} else
				rc = E_TOO_MANY_FILES;
			fclose( PcFile );
		}
	}

	return rc;
}

/*
========================================
 PackFiles
========================================
*/
#define NTRACKS 80
#define NSIDES	2
#define NSECTS	11

int	ADX_PackFiles(char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags)
{
	struct Device	*lpAdfDevice;			// device ptr
	struct Volume	*lpAdfVolume;			// Volume ptr
	UCHAR			szBuf[256];
	UINT			fdType;
	BOOL			fBootable = FALSE;
	BOOL			fNewAdf = FALSE;
	int				fFlags;
	int				rc;
	UCHAR			*lpszList;
	SECTNUM			nCurDir;

	/* Init ADF system */
	adfEnvInitDefault();
#ifdef DEBUG
	adfChgEnvProp(PR_VFCT, d_printf);
	adfChgEnvProp(PR_WFCT, d_printf);
	adfChgEnvProp(PR_EFCT, d_printf);
#endif

	if((lpAdfDevice = adfMountDev(PackedFile, FALSE))==NULL) {

			WIN32_FIND_DATA	findData;
			HANDLE hFile;

			/* Check is we got an error or the file not exists! */
			hFile = FindFirstFile(PackedFile, &findData);
			if(hFile!=INVALID_HANDLE_VALUE) {
				FindClose(hFile);
				adfEnvCleanUp();
				return E_BAD_DATA;
			}

			GetCfgPath();

			GetPrivateProfileString(szCfgKey, "Bootable", "1", szBuf, sizeof(szBuf), szCfgFile);
			fBootable = atoi(szBuf);

			GetPrivateProfileString(szCfgKey, "Type", "0", szBuf, sizeof(szBuf), szCfgFile);
			fdType = atoi(szBuf);
			lpAdfDevice = adfCreateDumpDevice(PackedFile,
				NTRACKS,
				NSIDES,
				fdType==0 ? NSECTS : NSECTS*2); /* Standard or HD floppy ? */

			if(lpAdfDevice == NULL ) {
				adfEnvCleanUp();
				return E_ECREATE;
			}

			GetPrivateProfileString(szCfgKey, "Flags", "0", szBuf, sizeof(szBuf), szCfgFile);
			fFlags = atoi(szBuf);
			GetPrivateProfileString(szCfgKey, "Label", szDefaultLabel, szBuf, sizeof(szBuf), szCfgFile);
			rc = adfCreateFlop( lpAdfDevice, szBuf, fFlags );
			if( rc!=RC_OK ) {
				adfUnMountDev( lpAdfDevice );
				adfEnvCleanUp();
				return E_ECREATE;
			}

			fNewAdf = TRUE;
	}

	if((lpAdfVolume = adfMount(lpAdfDevice, 0, FALSE))==NULL) {
		adfUnMountDev(lpAdfDevice);
		adfEnvCleanUp();
		return E_ECREATE;
	}
	if( fNewAdf && fBootable ) {
			rc = adfInstallBootBlock(lpAdfVolume, bbkData );
			if(rc!=RC_OK)
				MessageBox(NULL, "Can't install boot block!", "AmigaDX error", MB_ICONWARNING|MB_OK );
	}

	if( SubPath!=NULL && PreprocessPath( lpAdfVolume, SubPath, szBuf )!=RC_OK) {
		__DEBUG__("Can't change to destination directory: %s", SubPath );
		rc = E_EWRITE;
	} else {
		if(SubPath!=NULL && adfChangeDir( lpAdfVolume, szBuf )!=RC_OK)
			rc = E_EWRITE;
		else {
			nCurDir = lpAdfVolume->curDirPtr;
			lpszList = AddList;
			rc = 0;
			while( *lpszList && rc==0 ) {
				if( lpszList[ strlen(lpszList)-1 ]!='\\')
					// Pack file
					rc = PackAmigaFile( lpAdfVolume, SrcPath, lpszList );
				else {
					// Directory only
					rc = MakeDirectoryTree( lpAdfVolume, lpszList, szBuf );
					if( rc==0 && strlen(szBuf)>0 )
						rc = adfCreateDir( lpAdfVolume, lpAdfVolume->curDirPtr, szBuf );
				}

				/* Delete original file */
				if( Flags & PK_PACK_MOVE_FILES ) {
					sprintf(szBuf, "%s%s", SrcPath, lpszList );
					__DEBUG__("removing: %s", szBuf );
					DeleteFile( szBuf );
				}

				while(*lpszList++);
				lpAdfVolume->curDirPtr = nCurDir;
			}
		}
	}
	/*
	 * Cleanup
	 */
	adfUnMount	  ( lpAdfVolume );
	adfUnMountDev ( lpAdfDevice );
	adfEnvCleanUp();

	return rc;
}

/*
========================================
 RemoveAmigaDir
========================================
*/
static int RemoveAmigaDir( struct Volume *lpVolume, char *szDirName )
{
	int rc = 0;
	struct List *lpList, *lpCell;

	rc = adfChangeDir( lpVolume, szDirName );
	if( rc==0 ) {

		lpCell = lpList = adfGetDirEnt( lpVolume, lpVolume->curDirPtr );
		while( lpCell && rc==0 ) {
			if( ((struct Entry *)lpCell->content)->type==ST_DIR )
				rc = RemoveAmigaDir( lpVolume, ((struct Entry *)lpCell->content)->name );
			else {
				__DEBUG__("remove: %s", ((struct Entry *)lpCell->content)->name);
				rc = adfRemoveEntry( lpVolume, lpVolume->curDirPtr, ((struct Entry *)lpCell->content)->name );
			}
			lpCell = lpCell->next;
		}
		adfFreeDirList( lpList );
		adfParentDir( lpVolume );
		__DEBUG__("remdir: %s", szDirName);
		adfRemoveEntry( lpVolume, lpVolume->curDirPtr, szDirName );

	}

	return rc;
}

/*
========================================
Function name	: DeleteFiles
Description	    : Delete file(s) from ADF file
Return type		: STDCALL
Date-Time       : 04.03.2002 11:17:52 PM
Argument        : char *PackedFile
Argument        : char *DeleteList
========================================
*/
int	ADX_DeleteFiles(char *PackedFile, char *DeleteList)
{
	struct Device	*lpAdfDevice;			// device ptr
	struct Volume	*lpAdfVolume;			// Volume ptr
	UCHAR			*lpDirEntry;
	int				rc = 0;
	char			szBuf[256];
	SECTNUM			nParentDir;
#ifdef USE_DELETE_HACK
	int				nDeletedFiles=0;
#endif

	/* Init ADF system */
	adfEnvInitDefault();

#ifdef DEBUG
	adfChgEnvProp(PR_VFCT, d_printf);
	adfChgEnvProp(PR_WFCT, d_printf);
	adfChgEnvProp(PR_EFCT, d_printf);
#endif

	if((lpAdfDevice = adfMountDev(PackedFile, FALSE))==NULL) {
		adfEnvCleanUp();
		return E_EOPEN;
	}
	if((lpAdfVolume = adfMount(lpAdfDevice, 0, FALSE))==NULL) {
		adfUnMountDev(lpAdfDevice);
		adfEnvCleanUp();
		return E_EOPEN;
	}

	lpDirEntry = DeleteList;
	nParentDir = lpAdfVolume->curDirPtr;
	while(*lpDirEntry && rc==0) {

		if( strstr(lpDirEntry, "*.*") ) {
			/* Remove directory */
			UCHAR szPath[256], *lpPtr;

			strcpy(szPath, lpDirEntry); lpPtr = strstr(szPath, "*.*"); lpPtr[-1]='\0';
			rc = PreprocessPath( lpAdfVolume, szPath, szBuf );
			if( rc==0 )
				rc = RemoveAmigaDir( lpAdfVolume, szBuf );
		} else {
			/* Remove file */
			rc = PreprocessPath( lpAdfVolume, lpDirEntry, szBuf );
			if( rc==0 ) {
				__DEBUG__("remove entry: %s", szBuf);
				rc = adfRemoveEntry( lpAdfVolume, lpAdfVolume->curDirPtr, szBuf );
				assert(rc==0);
			}
		}

		/* Restore directory sector num */
		lpAdfVolume->curDirPtr = nParentDir;

		/* Next entry */
		while(*lpDirEntry++);
#ifdef USE_DELETE_HACK
		++nDeletedFiles;
#endif
	}

	/*
	 * Cleanup
	 */
	adfUnMount	  ( lpAdfVolume );
	adfUnMountDev ( lpAdfDevice );
	adfEnvCleanUp();
	__DEBUG__("end-of-delete: rc=%d", rc);

#ifdef USE_DELETE_HACK
	GetCfgPath();

	GetPrivateProfileString(szCfgKey, "_DELETE_HACK", "0", szBuf, sizeof(szBuf), szCfgFile);
	if( atoi(szBuf) && nDeletedFiles==1 )
		SleepEx(1000, TRUE);
#endif

	return rc;
}

void ADX_ConfigurePacker (HWND Parent, HINSTANCE DllInstance)
{
	// Show configuration for new adf file
	hDllInst = DllInstance;
	DialogBox(DllInstance,(LPCTSTR)IDD_CONFIGDLG, Parent,(DLGPROC)&ConfigDlgProc);
}
