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

	//��Ɨp�t�H���_���쐬
	CreateDirectory("tmp", NULL);	
	

	// �����n���h�����J��
	XDW_DOCUMENT_HANDLE h = NULL;
	XDW_OPEN_MODE_EX mode = {
		sizeof(XDW_OPEN_MODE_EX), XDW_OPEN_UPDATE, XDW_AUTH_NODIALOGUE };
	api_result = XDW_OpenDocumentHandle(in_path, &h, (XDW_OPEN_MODE*)&mode);
	if (api_result < 0) {
		return 0;
	}

	// XDW_GetDocumentInformation��p���đ��y�[�W���𓾂�
	XDW_DOCUMENT_INFO info = { sizeof(XDW_DOCUMENT_INFO), 0 };
	XDW_GetDocumentInformation(h, &info);
	int last_page = info.nPages;

	//���������y�[�W��xdw�t�@�C���̃p�X������z��pages��p�ӂ���
	char ** pages;
	pages = new char*[last_page];
	for(int i=0; i<last_page; i++){
		pages[i] = new char[_MAX_PATH];
	}

	//�y�[�W�𕪊�����tmp���ɕۑ�����
	char filename[15];
	char out_path[_MAX_PATH];	

	printf("�y�[�W�𕪊����c\n");
	for (int i = 1; i <= last_page; i++) {		
		sprintf_s(filename, "tmp\\%d.xdw",i);

		_fullpath(out_path, filename, _MAX_PATH);

		strcpy(pages[i-1], out_path);

		api_result = XDW_GetPage(h,i,out_path, NULL);
	}

	//�y�[�W�����̃y�[�W����폜����
	printf("�y�[�W���폜���c\n");
	for(int i=last_page; i>=1; i--){
		api_result = XDW_DeletePage(h, i, NULL);
	}
	// �ύX���t�@�C���ɔ��f����
	api_result = XDW_SaveDocument(h, NULL);

	//���������y�[�W�����Ԃɐ擪�ɓ���Ă���
	printf("�y�[�W��}�����c\n");
	for(int i=0; i<last_page;i++){
		api_result = XDW_InsertDocument(h, 1, pages[i], NULL);
	}

	//���������y�[�W�Ɣz��̍폜
	for(int i=0; i<last_page; i++){
		DeleteFile(pages[i]);
		delete[] pages[i];
		pages[i] = 0;
	}
	delete[] pages;

	// �ύX���t�@�C���ɔ��f����
	api_result = XDW_SaveDocument(h, NULL);

	// �����n���h�������
	XDW_CloseDocumentHandle(h, NULL);

	// �I������
	XDW_Finalize(NULL);
	
	if (api_result < 0) return 0;

	return 1;
}

PLUGGEDIN_EXPORT( long ) PLG_ExecuteCommand(PLUGGEDIN_STRUCT *ps)
{
    /* ���̃R�}���h���s�̓r���A�����Ƃ��ēn�����DW�t�@�C���͊J������Ă��܂��B
       �ǂݏ����\�ŃA�N�Z�X�ł��܂��B
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
		sprintf(buf, "�y�[�W���t���ɂ��܂����B");
		MessageBox(GetFocus(),buf,"ReverseXdwPlugin",MB_OK);
	}else{
		MessageBox(GetFocus(),"�t�@�C�����I������Ă��܂���B","ReverseXdwPlugin",MB_OK);
	}
	
    LocalFree(buf);
    LocalFree(path);
    return 1;
}

/* EnumerateCommands Entry */
PLUGGEDIN_EXPORT( long ) PLG_EnumerateCommands(char* pszEntries, long nBufSize, long* pnRealSize)
{
    // Check buffer size is large enough
    /* *pnRealSize�ɕK�v�ȃo�b�t�@�T�C�Y�����Ă������� */
    *pnRealSize=strlen("ReverseXdwPlugin")+1;

    if (nBufSize<*pnRealSize)
        return 0;

    /* nBufSize���K�v�ȃo�b�t�@�T�C�Y�𒴂��Ă���ꍇ�̂݁ApszEntries�ɃR�s�[��
       �����������Ƃ����� return 1��Ԃ��Ă�������
   */
   /*
     0�^�[�~�l�[�g�̕�����̃��X�g��Ԃ��Ă��������BDWDesk�̃v���O�C��/�ݒ�̌��ɏo�܂�
   */
    // DoCopy Commands!
    strcpy(pszEntries, "ReverseXdwPlugin");

	return 1;
}

PLUGGEDIN_EXPORT( long ) PLG_Executable(const char *pszFunction, long nSel)
{
    //�I���t�@�C���̐� nSel�ɉ����Ď��s�\����Ԃ��Ă�������
	return 1;
}
PLUGGEDIN_EXPORT( long ) PLG_RequireFiles(const char *pszFunction)
{
    // 0��Ԃ��ƁA�t�@�C���I����Ԃɂ�炸�����͓n����܂���
	return 1;
}

PLUGGEDIN_EXPORT( HICON ) PLG_GetCommandIcon(const char *pszFunction, long bNormal)
{
	HICON hIcon=NULL;
	unsigned resID=0;

	if (bNormal)
		hIcon=LoadIcon(myInst,MAKEINTRESOURCE(IDI_APPICON));
	else /* ��ɑ傫��(32x32 or 24x24)�A�C�R���ł����܂��܂��� */
		hIcon=(HICON)LoadImage(myInst,MAKEINTRESOURCE(IDI_APPICON),IMAGE_ICON,16,16,0);
	return hIcon;
}

/* Followin SPI s  are NO-op */
PLUGGEDIN_EXPORT( long ) PLG_Initialize(const char* cmd)
{
    /* �K�� 1��Ԃ��Ă�������. 0��Ԃ��ƃv���O�C�������[�h����܂���*/
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
    IsCloningCommand/CanSetProfile�� 0�ȊO��Ԃ��ƁA
    GetNewClone / SetProfile���R�[������܂�

*/
PLUGGEDIN_EXPORT( long ) PLG_IsParallel(const char *pszFunction)
{
    /* v2.0�ł͂���SPI�͖�������܂� */
	return 0;
}

/* IsCloning Entry*/
PLUGGEDIN_EXPORT( long ) PLG_IsCloningCommand(const char *pszFunction)
{
    /* ����@�\�ŕ����̐ݒ��ۑ����ē��삳�������ꍇ�Ȃǂ́A����
       SPI�� 1 ��Ԃ��Ă�������
       ����PLG_GetNewClone���R�[������܂��B
     */
    return 0;
}


PLUGGEDIN_EXPORT( long ) PLG_CanSetProfile(const char *pszFunction)
{
    /* SetProfile��L���ɂ���ꍇ�A������1��Ԃ��Ă������� */
	return 0;
}

PLUGGEDIN_EXPORT( long ) PLG_SetProfile(const char *pszFunction)
{
    /* ������GUI���o���Ď��g�̐ݒ���s���Ă������� */
	return 0;
}


/* GetNewClone Entry*/
PLUGGEDIN_EXPORT( long ) PLG_GetNewClone(char* pszNewClone, long bufsize)
{
    /* pszNewClone�Ƃ͈قȂ�����������쐬���ApszNewClone�����������ĕԂ��Ă������� */
    /* �Ⴆ�� "�v���O�C���@�\" -> "�v���O�C���@�\01","�v���O�C���@�\02"�Ȃ�
       �ȍ~�̃Z�b�V�����ł́A�N���[�����ꂽ�����񂪈���pszCommand�Ƃ��ēn����܂�
       �N���[�����Ƃɐݒ��ۑ����Ă��������B
       PLUGGEDIN_REGKEY\ �ȉ��̃��W�X�g���ɕۑ����ꂽ�f�[�^�́A
       Desk�̃v���O�C��/�ݒ�_�C�A���O�� OK ��������Ȃ�����A�ꊇ�L�����Z������܂�
     */
    return 0;
}

/* ReleaseClone Entry*/
PLUGGEDIN_EXPORT( long ) PLG_ReleaseClone(const char* pszClone)
{
    /* pszClone�Ŏw�肳�ꂽ�R�}���h���g�p���Ă����̈���J�����Ă������� */
    return 0;
}
