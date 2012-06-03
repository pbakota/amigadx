/*
========================================
 $Name:         support.c

 $Desc:         xDMS, GZip support routines for AmigaDx

 $Author:
 $Revision:     1
 $Date:         5/3/2002 11:25:44 PM
 $Comments:     This routines is converted from ADFOpus
========================================
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "support.h"
#include "types.h"

char szDirTemp[MAX_PATH];

/*
========================================
Function name	: VolGetFormat
Description	    :
Return type		: Return type of volume
Date-Time       : 5/3/2002 11:45:18 PM
Argument        : char *szFileName
Argument        : char **szInBuf
========================================
// Modified by Peter Bakota for AmigaDX
*/
DiskFormat_t VolGetFormat(char *szFileName, BOOL *dwError)
// Checks input file for ADZ or DMS types.
// Output: returns an enumerated disk type ADF, ADZ or DMS where ADF covers all
//         non-compressed types i.e. disk dumps, hardfiles etc.
{
	int		iLength, i;
	char	fileName[_MAX_FNAME]; // Original has 30 byte length which is too short -- bpeter 18.08.2004
	char	szFileSuf[4];
	char	szFileRoot[MAX_PATH * 3];
	char	szInBuf[MAX_PATH];

	*dwError = FALSE;
	_splitpath(szFileName, NULL, NULL, fileName, NULL);	// Get filename.

	iLength = strlen(szFileName);							// Get name length.
	for(i = 0;i < iLength - 3;i++)							// Get name root.
		szFileRoot[i] = szFileName[i];
	szFileRoot[i] = '\0';

	szFileSuf[0] = szFileName[iLength - 3];					// Get name suffix.
	szFileSuf[1] = szFileName[iLength - 2];
	szFileSuf[2] = szFileName[iLength - 1];
	szFileSuf[3] = '\0';

	// Check for hardfile or dump. Return name unchanged if found.
	if(strcmp(szFileSuf, "dmp") == 0 || strcmp(szFileSuf, "DMP") == 0 ||
	   strcmp(szFileSuf, "hdf") == 0 || strcmp(szFileSuf, "HDF") == 0){

		return T_ADF;
	}

	// FIXME: Need to check is temp directory exists?
	GetTempPath(sizeof(szDirTemp), szDirTemp);

	// Copy input file name.
	strcpy(szInBuf, szFileName);
	// Copy output name and append ".adf".
	strcpy(szFileName, szFileRoot);
	strcat(szFileName, "adf");

	// Open DMS.
	if(strcmp(szFileSuf, "dms") == 0 || strcmp(szFileSuf, "DMS") == 0 ){
		// Build temp file path name and copy back.
		strcpy(szFileName, szDirTemp);
		strcat(szFileName, fileName);
		strcat(szFileName, ".adf");
		if(dmsUnpack(szInBuf, szFileName) != NO_PROBLEM)
			*dwError = TRUE;

		return T_DMS;
	}

	// Open ADZ.
	if(strcmp(szFileSuf, "adz") == 0 || strcmp(szFileSuf, "ADZ") == 0) {
		// Build temp file path name and copy back.
		strcpy(szFileName, szDirTemp);
		strcat(szFileName, fileName);
		strcat(szFileName, ".adf");
		if(GZDecompress(NULL, szInBuf, szFileName)==0)
			*dwError = TRUE;

		return T_ADZ;
	}

#if 0
	// Open HDZ (experimental module)
	if(strcmp(szFileSuf, "hdz") == 0 || strcmp(szFileSuf, "HDZ") == 0) {
		// Build temp file path name and copy back.
		strcpy(szFileName, szDirTemp);
		strcat(szFileName, fileName);
		strcat(szFileName, ".hdf");
		if(GZDecompress(NULL, szInBuf, szFileName)==0)
			*dwError = TRUE;

		return T_HDZ;
	}
#endif

	return T_ADF;
}

/*
========================================
Function name	: GZDecompress
Description	    : Decompress Gziped Adf (.ADZ) file
Return type		: BOOL
Date-Time       : 5/3/2002 11:26:20 PM
Argument        : HWND win
Argument        : char *infile
Argument        : char *outfile
========================================
*/
#define BUFLEN      16384
BOOL GZDecompress(HWND win, char *infile, char *outfile)
// Decompress an adf compressed with gzip.
// Input: input file name, output file name, handle to owner window.
// Send NULL HWND to prevent the overwrite check.
{
    gzFile	in;
    FILE	*out;
    char	buf[BUFLEN];
    int		len;

	if(win != NULL && fopen(outfile, "r")){
		sprintf(buf, "%s already exists.\n Do you want to overwrite this file?", outfile);
		if(MessageBox(win, buf, "ADF Opus Warning", MB_YESNO|MB_ICONEXCLAMATION) == IDNO)
			return FALSE;
    }

    out = fopen(outfile, "wb");
    if(out == NULL)
		return FALSE;

    in = gzopen(infile, "rb");
    if(in == NULL)
		return FALSE;

	for(;;){
        len = gzread(in, buf, sizeof(buf));
        if(len < 0)
			return FALSE;
        if(len == 0)
			break;
        if((int)fwrite(buf, 1, (unsigned)len, out) != len)
			return FALSE;
    }
    if(fclose(out))
		return FALSE;

    if(gzclose(in) != Z_OK)
		return FALSE;

	return TRUE;
}
