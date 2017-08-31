// WIN32cdccomtest.cpp : Définit le point d'entrée pour l'application console.
//

#include "stdafx.h"

bool equalTab(unsigned char tab1[], unsigned char tab2[], int size);
void initializeArray(unsigned char tab[], int size);

int main()
{
	//declaration
	int iSize = -1;
	//int iRxIndex = 0;
	int iUserCmd = 0;
	UCHAR cCmd = '0';
	CString sCom = "COM";
	unsigned char cComNum[10];
	unsigned char buff[] = { 0x0, 0x0, 0x0, 0x0 };
	unsigned char test[512];
	unsigned char data[225792];
	//char* data = { 0 };
	CSerialport serialport;
	DWORD dwExpSize = 4;
	DWORD dwIndex = 0;
	DWORD dwReadSize = 0;



	//initialization
	serialport.DefaultParam();
	serialport.SetBaudrate(115200);

	//patern initialization 
	for (int i = 0; i < sizeof(test); i++)
		test[i] = (char)i;

	printf("Select COM number :\n");
	iUserCmd = scanf_s("%9s", cComNum, sizeof(cComNum));

	for (int i = 0; i < sizeof(cComNum); i++) {
		if (cComNum[i] == '\0')
			break;
		sCom.AppendChar(cComNum[i]);
	}

	while ('e' != iUserCmd)
	{
		printf("\nPress 'e' to exit\r\nPress '0' to get data size\r\nPress '1' to get data\r\nPress 's' for stress test\n");
		iUserCmd = _getch();
		system("cls");

		switch (iUserCmd) {

		case '0':
			cCmd = '0';

			serialport.OpenComPort(sCom);
			serialport.WriteBuffer(&(BYTE)cCmd, 1);
			dwExpSize = serialport.ReadBuffer(dwExpSize);

			//iSize = (int)(buff[0] << 24 & 0xFF000000) + (int)(buff[1] << 16 & 0x00FF0000) + (int)(buff[2] << 8 & 0x0000FF00) + (int)(buff[3] & 0x000000FF);
			iSize = (int)(serialport.machReadBuffer[0] << 24 & 0xFF000000) + (int)(serialport.machReadBuffer[1] << 16 & 0x00FF0000) + (int)(serialport.machReadBuffer[2] << 8 & 0x0000FF00) + (int)(serialport.machReadBuffer[3] & 0x000000FF);

			printf("\nSize = %d\r\n", iSize);
			serialport.PurgeComPort();
			serialport.CloseComPort();
			break;

		case '1':
			if (iSize > 0) {
				cCmd = '1';
				dwIndex = 0;
				int buffSize = sizeof(serialport.machReadBuffer);
				int count = iSize / buffSize;
				int mod = iSize % buffSize;
				int temp = 0;
				bool brk = FALSE;

				serialport.OpenComPort(sCom);
				serialport.WriteBuffer(&(BYTE)cCmd, 1);

				dwIndex = 0;
				//fill the data array
				for (int i = 0; i < count; i++) {
					temp = serialport.ReadBuffer(buffSize);
					if (temp != buffSize)
						temp += serialport.ReadBuffer(buffSize - temp);
					memcpy(data + dwIndex, (serialport.machReadBuffer), temp);
					dwIndex += temp;

				}
				//update received size
				dwReadSize = dwIndex;

				dwIndex = 0;
				unsigned char buffer[512];
				for (int i = 0; i < count; i++) {
					memcpy(buffer, (data + dwIndex), 512);
					if (!equalTab(test, buffer, sizeof(test))) {
						printf("\nerror line : %d\n", dwIndex);
						for (int i = 0; i < sizeof(serialport.machReadBuffer); i++)
						{
							printf("%02x ", (unsigned char)serialport.machReadBuffer[i]);//brk = TRUE; 
						}
					}
					dwIndex += 512;
				}
				serialport.CloseComPort();
				printf("\nSize = %d / %d\r\n", dwReadSize, iSize);
				initializeArray(data, sizeof(data));
			}
			/*for (int i = 0; i < count; i++) {
				temp = serialport.ReadBuffer(buffSize);
				if (temp != buffSize) {

				}
				memcpy(data + index, serialport.machReadBuffer, temp);
				if (!equalTab(test, (unsigned char*)serialport.machReadBuffer)) {
					printf("\nerror line : %d\n", index);
					for (int i = 0; i < sizeof(serialport.machReadBuffer); i++)
					{
						printf("%x ", (unsigned char)serialport.machReadBuffer[i]);
					}
				}

				index += temp;
				//if (index + buffSize > sizeof(data))
				//	index = sizeof(data) - buffSize;
			}
			if (mod > 0) {
				temp = serialport.ReadBuffer(mod);
				memcpy(data, (serialport.machReadBuffer + index), temp);
				index += temp;
			}
			printf("\nSize = %d / %d\r\n", index, iSize);
		}
		else {
			printf("\nUnknow data size");
		}*/
			//serialport.CloseComPort();
			break;

		case 's':
			if (iSize > 0)
			{
				cCmd = '1';
				int buffSize = sizeof(serialport.machReadBuffer);
				int count = iSize / buffSize;
				int mod = iSize % buffSize;
				int temp = 0;
				long passed = 0;
				bool brk = FALSE;

				while (!GetAsyncKeyState(VK_F1)) {
					serialport.OpenComPort(sCom);
					serialport.WriteBuffer(&(BYTE)cCmd, 1);
					dwIndex = 0;

					for (int i = 0; i < count; i++) {
						temp = serialport.ReadBuffer(buffSize);
						if (temp != buffSize)
							temp += serialport.ReadBuffer(buffSize - temp);
						memcpy(data + dwIndex, (serialport.machReadBuffer), temp);
						dwIndex += temp;
					}
					//update received size
					dwReadSize = dwIndex;

					dwIndex = 0;
					unsigned char buffer[512];
					for (int i = 0; i < count; i++) {
						memcpy(buffer, (data + dwIndex), 512);
						if (!equalTab(test, buffer, sizeof(test))) {
							printf("\nerror line : %d\n", dwIndex);
							for (int i = 0; i < sizeof(serialport.machReadBuffer); i++)
							{
								printf("%02x ", (unsigned char)serialport.machReadBuffer[i]);
							}							
							brk = TRUE;
							//break;
						}
						dwIndex += 512;
					}

					serialport.CloseComPort();
					//stop the while loop in case of detected issue
					if (brk) {
						
						printf("\n\nPress 'ENTER' to continue", dwIndex);
						fflush(stdin);
						_getch();
						break;
					}
					passed++;

					system("cls");
					printf("Size = %d / %d\r\ncount : %d\nPress 'F1' to stop test", dwReadSize, iSize, passed);
					initializeArray(data, sizeof(data));
				}
			}
			break;
			}
}
		//delete data;
		serialport.PurgeComPort();
		serialport.CloseComPort();
		printf("Progam halt...");
		Sleep(1000);
		return 0;
};

bool equalTab(unsigned char tab1[], unsigned char tab2[], int size) {
	if (sizeof(tab1) == sizeof(tab2)) {
		for (int i = 0; i < size; i++) {
			if (tab1[i] != tab2[i])
				return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

void initializeArray(unsigned char tab[], int size) {
	for (int i = 0; i < size; i++)
		tab[i] = 0;
}
