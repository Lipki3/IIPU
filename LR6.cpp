#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <BluetoothAPIs.h>
#include <Winsock2.h>
#include <Ws2bth.h>
#include <mmsystem.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "irprops.lib")
#pragma comment(lib, "bluetoothapis.lib")
#pragma comment(lib, "winmm")
#pragma warning(disable : 4995)

BOOL CALLBACK BTHeadsetAuthCallbackEx(__in_opt LPVOID /*pvParam*/, __in PBLUETOOTH_AUTHENTICATION_CALLBACK_PARAMS pAuthCallbackParams)
{
	BLUETOOTH_AUTHENTICATE_RESPONSE resp;
	resp.bthAddressRemote = pAuthCallbackParams->deviceInfo.Address;
	resp.authMethod = pAuthCallbackParams->authenticationMethod;
	resp.negativeResponse = FALSE;
	resp.passkeyInfo.passkey = pAuthCallbackParams->Passkey;
	DWORD ret = BluetoothSendAuthenticationResponseEx(NULL, &resp);
	if (ret != ERROR_SUCCESS)
	{
		std::cout << "BluetoothSendAuthenticationResponseEx failed with %u" << ret << std::endl;
		return FALSE;
	}

	return TRUE;
}

int PlaySoundd(int ch) {
	ch = 0;
	std::cout << "\n1 - Play, 2 - Stop\nYour choice: ";
	std::cin >> ch;
	switch (ch) {
	case 1:
		PlaySound(L"file.wav", NULL, SND_FILENAME | SND_ASYNC);
		break;
	case 2:
		PlaySound(NULL, 0, 0);
		break;
	case 3:
		break;
	}

	return ch;
}

int main()
{
	BLUETOOTH_DEVICE_SEARCH_PARAMS btSearchParams;

	btSearchParams.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
	btSearchParams.cTimeoutMultiplier = 5;  //5*1.28s search timeout
	btSearchParams.fIssueInquiry = true;    //new inquiry

	//return all known and unknown devices
	btSearchParams.fReturnAuthenticated = true;
	btSearchParams.fReturnConnected = true;
	btSearchParams.fReturnRemembered = true;
	btSearchParams.fReturnUnknown = true;

	btSearchParams.hRadio = NULL;   //search on all local radios



	BLUETOOTH_DEVICE_INFO btDeviceInfo;
	ZeroMemory(&btDeviceInfo, sizeof(BLUETOOTH_DEVICE_INFO));   //"initialize"
	btDeviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);

	HBLUETOOTH_DEVICE_FIND btDeviceFindHandle = NULL;
	GUID state1 = { 0x0000110b, 0x0000, 0x1000, { 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb } };
	GUID state2 = { 0x0000110e, 0x0000, 0x1000, { 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb } };
	char flag;
	int ch = 0;
	int choise = 0;
	btDeviceFindHandle = BluetoothFindFirstDevice(&btSearchParams, &btDeviceInfo);
	do {
		if (btDeviceFindHandle)
		{
			printf("Class: %ul\n", btDeviceInfo.ulClassofDevice);
			printf("Adress: %X\n", btDeviceInfo.Address);
			printf("Name: %S\n", btDeviceInfo.szName);
			printf("1 - Connect\n2 - Skip\n3 - Exit\nYour choice: ");
			scanf_s("%c", &flag);
			if (flag == '1') {
				HBLUETOOTH_AUTHENTICATION_REGISTRATION authCallbackHandle = NULL;
				DWORD err = BluetoothRegisterForAuthenticationEx(&btDeviceInfo, &authCallbackHandle, (PFN_AUTHENTICATION_CALLBACK_EX)&BTHeadsetAuthCallbackEx, NULL);
				if (err != ERROR_SUCCESS)
				{
					DWORD err = GetLastError();
					std::cout << "Error " << err << std::endl;
				}
				HBLUETOOTH_AUTHENTICATION_REGISTRATION regHandle;
				err = BluetoothSetServiceState(&btDeviceFindHandle, &btDeviceInfo, &state1, BLUETOOTH_SERVICE_ENABLE);
				if (err != ERROR_SUCCESS && err != 87)
				{
					std::cout << "Failed with " << err << std::endl;
					//return false;
				}
				err = BluetoothSetServiceState(&btDeviceFindHandle, &btDeviceInfo, &state2, BLUETOOTH_SERVICE_ENABLE);
				if (err != ERROR_SUCCESS && err != 87)
				{
					std::cout << "Failed with " << err << std::endl;
					//return false;
				}
				std::cout << "Successfull" << std::endl;
				if (!BluetoothFindNextDevice(btDeviceFindHandle, &btDeviceInfo)) {
					err = GetLastError();
					std::cout << "Find next device failed with " << err << std::endl;
					break;
				}
				
				choise = 0;
				do {
					choise = PlaySoundd(choise);
				} while (choise!=3);

			}
			if (flag == '2') {
				if (!BluetoothFindNextDevice(btDeviceFindHandle, &btDeviceInfo)) {
					int err = GetLastError();
					std::cout << "Find next device failed with " << err << std::endl;
					break;
				}
				continue;
			}

		}
	} while (flag != '3');
}