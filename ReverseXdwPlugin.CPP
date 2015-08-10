// File: plgsmpl.cpp

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpplugin.h"
#include "resource.h"
#include <xdw_api.h>

HINSTANCE  myInst=NULL;
__declspec(dllexport) long __stdcall DllMain(HANDLE hInst, unsigned long reason,
											 void* lpReserved)
{
	myInst=(HINSTANCE)hInst;
	return 1;
}

int ReverseXdw(char* in_path){
	int api_result = 0;

	//作業用フォルダを作成
	CreateDirectory("tmp", NULL);	
	

	// 文書ハンドルを開く
	XDW_DOCUMENT_HANDLE h = NULL;
	XDW_OPEN_MODE_EX mode = {
		sizeof(XDW_OPEN_MODE_EX), XDW_OPEN_UPDATE, XDW_AUTH_NODIALOGUE };
	api_result = XDW_OpenDocumentHandle(in_path, &h, (XDW_OPEN_MODE*)&mode);
	if (api_result < 0) {
		return 0;
	}

	// XDW_GetDocumentInformationを用いて総ページ数を得る
	XDW_DOCUMENT_INFO info = { sizeof(XDW_DOCUMENT_INFO), 0 };
	XDW_GetDocumentInformation(h, &info);
	int last_page = info.nPages;

	//分割したページのxdwファイルのパスを入れる配列pagesを用意する
	char ** pages;
	pages = new char*[last_page];
	for(int i=0; i<last_page; i++){
		pages[i] = new char[_MAX_PATH];
	}

	//ページを分割してtmp内に保存する
	char filename[15];
	char out_path[_MAX_PATH];	

	printf("ページを分割中…\n");
	for (int i = 1; i <= last_page; i++) {		
		sprintf_s(filename, "tmp\\%d.xdw",i);

		_fullpath(out_path, filename, _MAX_PATH);

		strcpy(pages[i-1], out_path);

		api_result = XDW_GetPage(h,i,out_path, NULL);
	}

	//ページを後ろのページから削除する
	printf("ページを削除中…\n");
	for(int i=last_page; i>=1; i--){
		api_result = XDW_DeletePage(h, i, NULL);
	}
	// 変更をファイルに反映する
	api_result = XDW_SaveDocument(h, NULL);

	//分割したページを順番に先頭に入れていく
	printf("ページを挿入中…\n");
	for(int i=0; i<last_page;i++){
		api_result = XDW_InsertDocument(h, 1, pages[i], NULL);
	}

	//分割したページと配列の削除
	for(int i=0; i<last_page; i++){
		DeleteFile(pages[i]);
		delete[] pages[i];
		pages[i] = 0;
	}
	delete[] pages;

	// 変更をファイルに反映する
	api_result = XDW_SaveDocument(h, NULL);

	// 文書ハンドルを閉じる
	XDW_CloseDocumentHandle(h, NULL);

	// 終了処理
	XDW_Finalize(NULL);
	
	if (api_result < 0) return 0;

	return 1;
}

PLUGGEDIN_EXPORT( long ) PLG_ExecuteCommand(PLUGGEDIN_STRUCT *ps)
{
    /* このコマンド実行の途中、引数として渡されるDWファイルは開放されています。
       読み書き可能でアクセスできます。
    */
    char *buf=NULL;
    char *tmp1=NULL;
    char *path=NULL;
    int i;
    buf=(char *)LocalAlloc(LMEM_FIXED, 32);
    path=(char *)LocalAlloc(LMEM_FIXED, MAX_PATH);
    for (i=0;i< ps->ps_nFiles; i++){
        strcat(buf," ");
		ReverseXdw((char*)(ps->ps_pszFiles[i]));
    }
	if(ps->ps_nFiles > 0){
		sprintf(buf, "ページを逆順にしました。");
		MessageBox(GetFocus(),buf,"ReverseXdwPlugin",MB_OK);
	}else{
		MessageBox(GetFocus(),"ファイルが選択されていません。","ReverseXdwPlugin",MB_OK);
	}
	
    LocalFree(buf);
    LocalFree(path);
    return 1;
}

/* EnumerateCommands Entry */
PLUGGEDIN_EXPORT( long ) PLG_EnumerateCommands(char* pszEntries, long nBufSize, long* pnRealSize)
{
    // Check buffer size is large enough
    /* *pnRealSizeに必要なバッファサイズを入れてください */
    *pnRealSize=strlen("ReverseXdwPlugin")+1;

    if (nBufSize<*pnRealSize)
        return 0;

    /* nBufSizeが必要なバッファサイズを超えている場合のみ、pszEntriesにコピーし
       成功したことを示す return 1を返してください
   */
   /*
     0ターミネートの文字列のリストを返してください。DWDeskのプラグイン/設定の候補に出ます
   */
    // DoCopy Commands!
    strcpy(pszEntries, "ReverseXdwPlugin");

	return 1;
}

PLUGGEDIN_EXPORT( long ) PLG_Executable(const char *pszFunction, long nSel)
{
    //選択ファイルの数 nSelに応じて実行可能性を返してください
	return 1;
}
PLUGGEDIN_EXPORT( long ) PLG_RequireFiles(const char *pszFunction)
{
    // 0を返すと、ファイル選択状態によらず引数は渡されません
	return 1;
}

PLUGGEDIN_EXPORT( HICON ) PLG_GetCommandIcon(const char *pszFunction, long bNormal)
{
	HICON hIcon=NULL;
	unsigned resID=0;

	if (bNormal)
		hIcon=LoadIcon(myInst,MAKEINTRESOURCE(IDI_APPICON));
	else /* 常に大きな(32x32 or 24x24)アイコンでもかまいません */
		hIcon=(HICON)LoadImage(myInst,MAKEINTRESOURCE(IDI_APPICON),IMAGE_ICON,16,16,0);
	return hIcon;
}

/* Followin SPI s  are NO-op */
PLUGGEDIN_EXPORT( long ) PLG_Initialize(const char* cmd)
{
    /* 必ず 1を返してください. 0を返すとプラグインがロードされません*/
	return 1;
}

PLUGGEDIN_EXPORT( long ) PLG_Finalize(const char*  cmd)
{
	return 1;
}

PLUGGEDIN_EXPORT( long ) PLG_CanFinalize(const char*  cmd)
{
	return 1;
}

/*
    IsCloningCommand/CanSetProfileは 0以外を返すと、
    GetNewClone / SetProfileがコールされます

*/
PLUGGEDIN_EXPORT( long ) PLG_IsParallel(const char *pszFunction)
{
    /* v2.0ではこのSPIは無視されます */
	return 0;
}

/* IsCloning Entry*/
PLUGGEDIN_EXPORT( long ) PLG_IsCloningCommand(const char *pszFunction)
{
    /* 同一機能で複数の設定を保存して動作させたい場合などは、この
       SPIで 1 を返してください
       次にPLG_GetNewCloneがコールされます。
     */
    return 0;
}


PLUGGEDIN_EXPORT( long ) PLG_CanSetProfile(const char *pszFunction)
{
    /* SetProfileを有効にする場合、ここで1を返してください */
	return 0;
}

PLUGGEDIN_EXPORT( long ) PLG_SetProfile(const char *pszFunction)
{
    /* ここでGUIを出して自身の設定を行ってください */
	return 0;
}


/* GetNewClone Entry*/
PLUGGEDIN_EXPORT( long ) PLG_GetNewClone(char* pszNewClone, long bufsize)
{
    /* pszNewCloneとは異なった文字列を作成し、pszNewCloneを書き換えて返してください */
    /* 例えば "プラグイン機能" -> "プラグイン機能01","プラグイン機能02"など
       以降のセッションでは、クローンされた文字列が引数pszCommandとして渡されます
       クローンごとに設定を保存してください。
       PLUGGEDIN_REGKEY\ 以下のレジストリに保存されたデータは、
       Deskのプラグイン/設定ダイアログで OK が押されない限り、一括キャンセルされます
     */
    return 0;
}

/* ReleaseClone Entry*/
PLUGGEDIN_EXPORT( long ) PLG_ReleaseClone(const char* pszClone)
{
    /* pszCloneで指定されたコマンドが使用していた領域を開放してください */
    return 0;
}

