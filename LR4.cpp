#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib,"vfw32.lib")
#include <iostream>
#include <Windows.h>
#include <vfw.h>
#include <string>

//  ESC - выход
//	H   - показать/скрыть окно
//	V   - старт/стоп записи видео
//	P   - сделать фото

HHOOK hHookKeyboard;  
HWND capture_handle; 
bool recording_video = false;

LRESULT CALLBACK keyboard_hook_handle(int nCode, WPARAM wParam, LPARAM lParam);
DWORD WINAPI capture_photo(LPVOID lpParam);
void start_video_capture();
void stop_video_capture();


int main()
{
	bool has_device = false;
	CHAR name[256];
	CHAR description[256];
	if (capGetDriverDescriptionA(0, name, 255, description, 255))
	{
		std::cout << name << std::endl;
		std::cout << description << std::endl;
		system("pause");
		has_device = true;
	}
	if (!has_device)
	{
		std::cout << "No webcam installed";
		system("pause");
		return 1;
	}

	HWND window_handle = GetForegroundWindow();		
	ShowWindow(window_handle, SW_HIDE); 

	if (!(capture_handle = capCreateCaptureWindowA("Capture", WS_VISIBLE, 0, 0, 640, 480, window_handle, 0))) 
	{
		std::cout << "Failed to create capture window";
		system("pause");
		return 1;
	}

	while (!capDriverConnect(capture_handle, 0)); 

	hHookKeyboard = SetWindowsHookExW(WH_KEYBOARD_LL, keyboard_hook_handle, GetModuleHandle(NULL), 0); 

	capPreviewScale(capture_handle, TRUE);	
	capPreviewRate(capture_handle, 30);		
	capPreview(capture_handle, TRUE);	

	MSG message = { 0 };
	while (GetMessage(&message, NULL, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	capDriverDisconnect(capture_handle);
	DestroyWindow(capture_handle);
	return 0;
}


LRESULT CALLBACK keyboard_hook_handle(int nCode, WPARAM wParam, LPARAM lParam)
{
	static bool window_is_hidden = false;
	KBDLLHOOKSTRUCT* ks = (KBDLLHOOKSTRUCT*)lParam;		
	if (ks->vkCode == 0x1B)							
	{
		if (recording_video)
		{
			stop_video_capture();
		}
		TerminateProcess(GetCurrentProcess(), NO_ERROR);
	}

	if (ks->vkCode == 0x50 && (ks->flags & 0x80) == 0)	
	{
		CreateThread(NULL, 0, capture_photo, NULL, 0, NULL);
	}

	if (ks->vkCode == 0x48 && (ks->flags & 0x80) == 0)	
	{
		if (window_is_hidden == true)
		{
			ShowWindow(capture_handle, SW_NORMAL);
			window_is_hidden = false;
		}
		else
		{
			ShowWindow(capture_handle, SW_HIDE);
			window_is_hidden = true;
		}
		return -1;
	}

	if (ks->vkCode == 0x56 && (ks->flags & 0x80) == 0)	
	{
		CAPTUREPARMS capparms;
		capCaptureGetSetup(capture_handle, &capparms, sizeof(CAPTUREPARMS));

		capparms.fMakeUserHitOKToCapture = TRUE;	
		capparms.fYield = TRUE;						
		capparms.fCaptureAudio = TRUE;				
		capparms.fAbortLeftMouse = FALSE;		
		capparms.fAbortRightMouse = FALSE;			
		capparms.dwRequestMicroSecPerFrame = 33333;

		capCaptureSetSetup(capture_handle, &capparms, sizeof(CAPTUREPARMS));

		if (recording_video)
		{
			stop_video_capture();
		}
		else
		{
			start_video_capture();
		}
		return -1;
	}
	return CallNextHookEx(hHookKeyboard, nCode, wParam, lParam);
}

DWORD WINAPI capture_photo(LPVOID lpParam)
{
	static int number_of_image_files = 0;
	if (!recording_video) {
		capCaptureSingleFrameOpen(capture_handle); 
		capCaptureSingleFrame(capture_handle);
		capCaptureSingleFrameClose(capture_handle); 
	}
	WCHAR filename[10];
	HANDLE file_handle = 0;
	while (file_handle != INVALID_HANDLE_VALUE)
	{
		memset(filename, 0, sizeof(filename));
		_itow(number_of_image_files++, filename, 10);
		wcscat(filename, L".jpg");
		file_handle = CreateFileW(filename, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	capFileSaveDIB(capture_handle, filename);
	return -1;
}

void stop_video_capture()
{
	capCaptureAbort(capture_handle);
	recording_video = false;
}

void start_video_capture()
{
	static int number_of_video_files = 0;
	WCHAR filename[10];
	HANDLE file_handle = 0;
	while (file_handle != INVALID_HANDLE_VALUE)
	{
		memset(filename, 0, sizeof(filename));
		_itow(number_of_video_files++, filename, 10);
		wcscat(filename, L".avi");
		file_handle = CreateFileW(filename, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	capFileSetCaptureFile(capture_handle, filename);
	capCaptureSequence(capture_handle);
	recording_video = true;
}
