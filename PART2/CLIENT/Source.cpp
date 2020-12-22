#include <windows.h>
#include <iostream>

using namespace std;

void WINAPI ReadCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
	cout << "Информация получена" << endl;
}

int main()
{
	system("chcp 1251");
	CHAR mes[512];
	BOOL b = FALSE;
	LPCTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");
	HANDLE EVENT = CreateEvent(NULL, FALSE, FALSE, NULL);
	HANDLE PIPE = CreateFile(lpszPipename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	int x;
	OVERLAPPED overlapped = OVERLAPPED();

	if (EVENT != INVALID_HANDLE_VALUE && PIPE != INVALID_HANDLE_VALUE)
	{
		do
		{
			cout << "1. Принять сообщение" << endl;
			cout << "2. Отсоединиться от именованного канала" << endl;
			cout << "0. Выйти" << endl;
			cin >> x;
			switch (x) {

			case 1:
				
				overlapped.hEvent = EVENT;
				b = ReadFileEx(PIPE, (LPVOID)mes, 512, &overlapped, ReadCallback);
				if (b)
					cout << mes << endl;
				else
					cout << "Чтение не удалось" << endl;
				break;

			case 2:
				b = CloseHandle(PIPE);
				if (b) cout << "Вы были отсоединены от именованного канала" << endl;
				else cout << "Не удалось отсоединиться" << endl;
				b = FALSE;
				break;
			case 0:
				break;
			default:
				break;
			}
		} while (x != 0);
	}
	else
		cout << "Не удалось создать именованный канал, перезапустите программу" << endl;

	return 0;
}

