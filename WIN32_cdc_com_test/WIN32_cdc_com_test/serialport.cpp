/*******************************************************************************
 ELYCTIS
 Copyright (C) 2011-2015, All Rights Reserved

 Project  : ptt-lite
 Filename : serialport.cpp

 Module Description:
  The serialportlite.cpp is used to open, close, write, read and change settings
 of respective COM port.

 History:
 -------------------------------------------------------------------------------
 Date       Version     Author      Changes
 -------------------------------------------------------------------------------
 01 Oct 13     1.00     Thillai     Initial Version

*******************************************************************************/

#include "stdafx.h"

///<summary>This function opens the COM Port & returns TRUE if successful.</summary>
BOOL CSerialport::OpenComPort (CString COMno)
{
	//
	// This function opens the comport & returns TRUE if it is successful
	//

	BOOL bRetValue = TRUE;

	// The "\\.\" prefix will access the Win32 device namespace instead of the
	// Win32 file namespace. For more info, refer to the following link:
	// http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx#win32_device_namespaces
	mszComPort = L"\\\\.\\";	// "\\" for a single "\" as it is a character for escape sequence.
	mszComPort.Append (COMno);

	// Open the COM port
	mhCom = CreateFile (mszComPort, \
			GENERIC_READ | GENERIC_WRITE, \
			NULL, \
			NULL, \
			OPEN_EXISTING, \
			/*FILE_ATTRIBUTE_NORMAL | */FILE_FLAG_OVERLAPPED, \
			NULL);

	if (INVALID_HANDLE_VALUE == mhCom)
	{
		bRetValue = FALSE;
		//TRACE (L"[serialport.cpp]: ERROR: Can't open %S\n", mszComPort);
		printf("[serialport.cpp]: ERROR: Can't open %S\n", mszComPort);
	}
	else
	{
		//TRACE (L"[serialport.cpp]: Open %S\n", mszComPort);
		//printf("[serialport.cpp]: Open %S\n", mszComPort);
	}

	return bRetValue;
}

///<summary>This function closes the COM port and restores default values.</summary>
BOOL CSerialport::CloseComPort (void)
{
	//
	// This function closes the comport and restores default values.
	//

	BOOL bStatus;

	bStatus = SetCommState (mhCom, &mOrginalDCB);
	Sleep (60);	// Needed as per CP2102 application note

	bStatus = CloseHandle (mhCom);
	//TRACE (L"[serialport.cpp]: Close %s result %d\n", mszComPort, bStatus);
	//printf("[serialport.cpp]: Close %s result %d\n", mszComPort, bStatus);

	mhCom = INVALID_HANDLE_VALUE;

	return bStatus;
}

///<summary>Purge the COM Port to clar any existing data going to or from the port</summary>
void CSerialport::PurgeComPort (void)
{
	//
	// Purge the COM port to clear any existing data going to or from the port
	//

	PurgeComm (mhCom, \
		PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
}

///<summary> This function sets the default comport settings & timeout to the COM object.< / summary>
void CSerialport::DefaultParam (void)
{
	//
	// This function sets the default comport settings & timeout to the COM
	// object
	//

	// Initialize the port to abort & clear any existing transactions
	PurgeComPort ();

	// Get defult port settings, modify & set necessary settings
	if (GetCommState (mhCom, &mOrginalDCB))
	{
		// Copy the default structure values
		mDCB = mOrginalDCB;

		// Modify the structure to the required baudrate settings
		mDCB.BaudRate = CBR_9600;
		mDCB.ByteSize = 8;
		mDCB.Parity = NOPARITY;
		mDCB.StopBits = ONESTOPBIT;

		// Save the changes to the port
		if (SetCommState (mhCom, &mDCB))
		{
			Sleep (60);	// Needed as per CP2102 application note
		}
	}
}

///<summary>This function changes the baudrate of com port.</summary>
void CSerialport::SetBaudrate (DWORD dwBaudrate)
{
	//
	// This function changes the baudrate of comport
	//

	// Set baudrate value
	mDCB.BaudRate = dwBaudrate;

	// Save the changes and check update has succeeded or not
	if (SetCommState (mhCom, &mDCB))
	{
		Sleep (60);	// Needed as per CP2102 application note
	}
	else
	{
		//TRACE (L"[serialport.cpp]: ERROR: Can't overwrite baudrate\n");
		printf("[serialport.cpp]: ERROR: Can't overwrite baudrate\n");
	}
}

///<summary>This function writes data to comport & returns no of bytes written to comport. </summary>
DWORD CSerialport::WriteBuffer (BYTE *pabyData, DWORD dwLenData)
{
	//
	// This function writes data to comport & returns no of bytes written to
	// comport
	//

	bool bRetVal = false;
	DWORD dwBytesWritten;
	OVERLAPPED sOverlapped = {0};

	// Create an overlapped event
	sOverlapped.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
	if (sOverlapped.hEvent == NULL)
	{
		// error creating overlapped event handle
		return FALSE;
	}

	// Write data to comport
	if (!WriteFile (mhCom, (LPCVOID) pabyData, dwLenData, &dwBytesWritten, \
		&sOverlapped))
	{
		if (ERROR_IO_PENDING == GetLastError ())
		{
			Sleep (60);	// Needed for FTDI chips based readers (ACMB test adapters)

			if (WAIT_OBJECT_0 == \
				WaitForSingleObject (sOverlapped.hEvent, INFINITE))
			{
				if (GetOverlappedResult (mhCom, &sOverlapped, \
					&dwBytesWritten, FALSE))
				{
					bRetVal = true;
				}
			}
			else
			{
				//TRACE (L"[serialport.cpp]: ERROR: Waiting in WaitForSingleObject\n");
				printf("[serialport.cpp]: ERROR: Waiting in WaitForSingleObject\n");
			}
		}
		else
		{
			//TRACE (L"[serialport.cpp]: ERROR: Checking for GetLastError\n");
			printf("[serialport.cpp]: ERROR: Checking for GetLastError\n");
		}
	}
	else
	{
		bRetVal = true;
	}

	/*if (dwBytesWritten != dwLenData)
	{
		bRetVal = false;
	}*/

	CloseHandle(sOverlapped.hEvent);

	return dwBytesWritten;
}

///<summary>This function reads data from comport & returns the total no of bytes read.</summary>
DWORD CSerialport::ReadBuffer (DWORD dwExpResponseLen)
{
	//
	// This function reads data from comport & returns the total no of bytes
	// read.
	//

	#define READ_TIMEOUT	1000

	DWORD dwBytesRead = 0, dwRes;
	BOOL bRetStatus = false;
	OVERLAPPED sOverlapped = {0};

	do
	{
		sOverlapped.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (sOverlapped.hEvent == NULL)
		{
			// error creating overlapped event handle
			return FALSE;
		}

		if (dwExpResponseLen > SRL_BUFFER_SIZE)
		{
			break;
		}

		// Initializing the user buffer
		memset (machReadBuffer, 0, sizeof (machReadBuffer));

		if (!ReadFile (mhCom, machReadBuffer, dwExpResponseLen, \
			&dwBytesRead, &sOverlapped))
		{
			if (ERROR_IO_PENDING == GetLastError ())
			{
				dwRes = WaitForSingleObject (sOverlapped.hEvent, READ_TIMEOUT);

				if (WAIT_OBJECT_0 == dwRes)
				{
					//Sleep (30);
					if (GetOverlappedResult (mhCom, &sOverlapped, \
						&dwBytesRead, FALSE))
					{
						bRetStatus = true;
					}
				}
				else
				{
					dwBytesRead = 0;

					// Initialize the port to abort & clear any existing transactions
					PurgeComPort ();	// Needed when the wait has timed out

					break;
				}
			}
		}

	} while (FALSE);

	CloseHandle (sOverlapped.hEvent);

	return dwBytesRead;
}
