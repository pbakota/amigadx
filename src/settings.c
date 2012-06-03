//============================
// settings.c
// The settings-about dialogs
//============================

#include <windows.h>
#include <stdlib.h>

#include "defs.h"
#include "resource.h"
#include "adflib.h"

#pragma GCC diagnostic ignored "-Wpointer-sign"

BOOL CALLBACK AboutDlgProc(HWND hwndDlg,UINT  uMsg,WPARAM wParam,LPARAM  lParam);
BOOL CALLBACK ConfigDlgProc(HWND hwndDlg,UINT  uMsg,WPARAM wParam,LPARAM  lParam);

extern char szCfgFile[];
extern const char szCfgKey[];
extern const char szDefaultLabel[];
extern HINSTANCE hDllInst;

/*
========================================
Function name	: ConfigDlgProc
Description	    : Config dialog CALLBACK
Return type		: BOOL CALLBACK 
Date-Time       : 25.04.2003 15:36:30 PM
Argument        : HWND hwndDlg
Argument        : UINT  uMsg
Argument        : WPARAM wParam
Argument        : LPARAM  lParam
========================================
*/
BOOL CALLBACK ConfigDlgProc(HWND hwndDlg,UINT  uMsg,WPARAM wParam,LPARAM  lParam)
{
	char szRes[160];
	int fFlags;

	switch (uMsg) {
	case WM_INITDIALOG:
		/* Read configuration */
		GetPrivateProfileString(szCfgKey, "Bootable", "1", szRes, sizeof(szRes), szCfgFile);
		CheckDlgButton(hwndDlg, IDC_CHKBOOT, atoi(szRes)!=0 ? BST_CHECKED : BST_UNCHECKED);
		GetPrivateProfileString(szCfgKey, "Flags", "0", szRes, sizeof(szRes), szCfgFile);
		fFlags = atoi(szRes);
		CheckDlgButton(hwndDlg, IDC_CHKFFS,  isFFS( fFlags ) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHKINTL, isINTL( fFlags ) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHKDIRC, isDIRCACHE( fFlags ) ? BST_CHECKED : BST_UNCHECKED);

		GetPrivateProfileString(szCfgKey, "Type", "0", szRes, sizeof(szRes), szCfgFile);
		CheckRadioButton(hwndDlg, IDC_OPTSTD, IDC_OPTHDF, atoi(szRes)==0 ? IDC_OPTSTD : IDC_OPTHDF);
		GetPrivateProfileString(szCfgKey, "Label", szDefaultLabel, szRes, sizeof(szRes), szCfgFile);
		SetDlgItemText(hwndDlg, IDC_EDTLABEL, szRes);

		// TODO: Implement pleez!
		GetPrivateProfileString(szCfgKey, "HDFF_Size", "4096", szRes, sizeof(szRes), szCfgFile);
		SetDlgItemText(hwndDlg, IDC_EDTHDFFSIZE, szRes);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDTHDFFSIZE), FALSE );
		EnableWindow(GetDlgItem(hwndDlg, IDC_OPTHDFF), FALSE );

		// Misc
#ifdef USE_DELETE_HACK
		GetPrivateProfileString(szCfgKey, "_DELETE_HACK", "0", szRes, sizeof(szRes), szCfgFile);
		CheckDlgButton(hwndDlg, IDC_CHKDELHACK, atoi(szRes)!=0 ? BST_CHECKED : BST_UNCHECKED);
#endif

		GetPrivateProfileString(szCfgKey, "EnableWarning", "1", szRes, sizeof(szRes), szCfgFile);
		CheckDlgButton(hwndDlg, IDC_CHKENABLEWARNINGS, atoi(szRes)!=0 ? BST_CHECKED : BST_UNCHECKED);
		GetPrivateProfileString(szCfgKey, "InvalidCharReplacer", "_", szRes, sizeof(szRes), szCfgFile);
		SetDlgItemText(hwndDlg, IDC_EDTINVALIDCHAR, szRes);
		
		break;
	 case WM_COMMAND:
	   switch (LOWORD(wParam)) {
		 case IDOK:		   /* OK  */
			/* Write configuration */
			sprintf(szRes, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHKBOOT)==BST_CHECKED ? 1 : 0 );
			WritePrivateProfileString(szCfgKey, "Bootable", szRes, szCfgFile);
			fFlags  = 0;
			fFlags |= IsDlgButtonChecked(hwndDlg, IDC_CHKFFS)==BST_CHECKED ? FSMASK_FFS : 0;
			fFlags |= IsDlgButtonChecked(hwndDlg, IDC_CHKINTL)==BST_CHECKED ? FSMASK_INTL : 0;
			fFlags |= IsDlgButtonChecked(hwndDlg, IDC_CHKDIRC)==BST_CHECKED ? FSMASK_DIRCACHE : 0;
			sprintf(szRes, "%d", fFlags );
			WritePrivateProfileString(szCfgKey, "Flags", szRes, szCfgFile);

			sprintf(szRes, "%d", IsDlgButtonChecked(hwndDlg, IDC_OPTSTD)==BST_CHECKED ? 0 : 1 );
			WritePrivateProfileString(szCfgKey, "Type", szRes, szCfgFile);
			GetDlgItemText(hwndDlg, IDC_EDTLABEL, szRes, sizeof(szRes));
			WritePrivateProfileString(szCfgKey, "Label", szRes, szCfgFile);
			GetDlgItemText(hwndDlg, IDC_EDTHDFFSIZE, szRes, sizeof(szRes));
			WritePrivateProfileString(szCfgKey, "HDFF_Size", szRes, szCfgFile);

			// Misc
#ifdef USE_DELETE_HACK
			sprintf(szRes, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHKDELHACK)==BST_CHECKED ? 1 : 0 );
			WritePrivateProfileString(szCfgKey, "_DELETE_HACK", szRes, szCfgFile);
#endif
			sprintf(szRes, "%d", IsDlgButtonChecked(hwndDlg, IDC_CHKENABLEWARNINGS)==BST_CHECKED ? 1 : 0 );
			WritePrivateProfileString(szCfgKey, "EnableWarning", szRes, szCfgFile);
			GetDlgItemText(hwndDlg, IDC_EDTINVALIDCHAR, szRes, sizeof(szRes));
			WritePrivateProfileString(szCfgKey, "InvalidCharReplacer", szRes, szCfgFile);

			EndDialog(hwndDlg,1);
			break;
		 case IDCANCEL:		/* CANCEL */
			EndDialog(hwndDlg,0);
			break;
		case IDC_BTNABOUT:	/* About... */
			{
			HWND hWndParent = GetParent(hwndDlg);
			DialogBox(hDllInst,(LPCTSTR)IDD_ABOUTDLG,hWndParent,(DLGPROC)&AboutDlgProc);
			}
	   }
	 break;
	}
	return FALSE;
}

/*
========================================
Function name	: AboutDlgProc
Description	    : About dialog CALLBACK
Return type		: BOOL CALLBACK 
Date-Time       : 13.01.2002 8:36:30 PM
Argument        : HWND hwndDlg
Argument        : UINT  uMsg
Argument        : WPARAM wParam
Argument        : LPARAM  lParam
========================================
*/
BOOL CALLBACK AboutDlgProc(HWND hwndDlg,UINT  uMsg,WPARAM wParam,LPARAM  lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			UCHAR szRes[80];
			_snprintf(szRes, sizeof(szRes), "adflib version: %s (%s)",
			adfGetVersionNumber(),
			adfGetVersionDate());
			SetDlgItemText(hwndDlg, IDC_ADFLIBVERSION, szRes);
			_snprintf(szRes, sizeof(szRes), "V%s", VERSIONTEXT);
			SetDlgItemText(hwndDlg, IDC_VERSION, szRes);
		}
		break;
	 case WM_COMMAND:
	   switch (LOWORD(wParam)) {
		 case IDOK:		   /* OK  */
			EndDialog(hwndDlg,1);
			break;
		 case IDCANCEL:		/* CANCEL */
			EndDialog(hwndDlg,0);
			break;
		case IDC_BTNWEB:	/* Goto my web page */
			ShellExecute(NULL, "open", "http://www.coderbug.rs/projects/amigadx", NULL, NULL, SW_NORMAL);
			break;
		case IDC_BTNEMAIL:	/* send mail to me */
			ShellExecute(NULL, "open", "mailto:bakota@gmail.com", NULL, NULL, SW_NORMAL);
			break;
		case IDC_BTNADFLIB: /* goto adflibs web page */
			ShellExecute(NULL, "open", "http://lclevy.free.fr/adflib", NULL, NULL, SW_NORMAL);
			break;
	   }
	 break;
	}
	return FALSE;
}
