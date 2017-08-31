#pragma once
#include "stdafx.h"

#define SRL_BUFFER_SIZE 512

class CSerialport //: public CDialog
{
	HANDLE  mhCom;

public:
	CString mszComPort;
	DCB   mDCB;
	DCB   mOrginalDCB;
	COMMTIMEOUTS mCommTimeouts;
	COMMTIMEOUTS mOrginalCommTimeouts;
	char machReadBuffer [SRL_BUFFER_SIZE];

	// fuctions
	BOOL OpenComPort (CString COMno);
	BOOL CloseComPort (void);
	void PurgeComPort (void);
	void DefaultParam (void);
	void SetBaudrate (DWORD Baudrate);
	//void ChangeCommTimeouts (DWORD ReadTotalTimeoutConstant);
	DWORD WriteBuffer (BYTE *pabyData, DWORD dwLenData);
	DWORD ReadBuffer (DWORD dwExpResponseLen);
};