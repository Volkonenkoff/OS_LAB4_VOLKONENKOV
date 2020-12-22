#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <ctime>
#define PG_SIZE 4096 //размер страницы
#define PAGES_COUNT 16 //кол-во страниц (3+0+8+5)

#pragma comment(lib, "winmm.lib")

using namespace std;
//Путь до файла с логами для читателя
LPCTSTR  LOG = TEXT("D:\\OS_LAB4\\LOG_READER.txt");
LPCTSTR NAME = TEXT("READER_MUTEX");
//Путь до проекции файла
TCHAR mname[] = TEXT("Local\\MyFileMapObj");
int main()
{
	
	
	srand(time(NULL));
	HANDLE WriteSemaphores[PAGES_COUNT];
	HANDLE ReadSemaphores[PAGES_COUNT];
	HANDLE READER_MUTEX = OpenMutex(MUTEX_MODIFY_STATE | SYNCHRONIZE, FALSE, NAME); // Открытие созданных мьютексов по имени
	HANDLE Mapping = OpenFileMapping(GENERIC_READ, FALSE, mname);
	LPVOID MAP_OF_FILE = MapViewOfFile(Mapping, FILE_MAP_READ, 0, 0, PG_SIZE * PAGES_COUNT);
	char msg[PG_SIZE];
	
	DWORD CurrentPage;
	string temp;
	for (int i = 0; i < PAGES_COUNT; i++) {
		//Открытие созданных семафоров по имени
		temp = "WRITE_SEM_ID_" + to_string(i);
		WriteSemaphores[i] = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, (LPCTSTR)(temp.c_str()));
		temp = "READ_SEM_ID_" + to_string(i);
		ReadSemaphores[i] = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, (LPCTSTR)(temp.c_str()));
	}
	ofstream Log;
	Log.open(LOG, fstream::out | fstream::app);	//Файл с логами

	VirtualLock(MAP_OF_FILE, PG_SIZE * PAGES_COUNT); //Блокировка страниц буферной памяти в оперативной памяти

	while (true) {
		//Получение текущей страницы
		CurrentPage = WaitForMultipleObjects(PAGES_COUNT, ReadSemaphores, FALSE, INFINITE) - WAIT_OBJECT_0;
		//Заимствование мьютекса и запись в логи
		WaitForSingleObject(READER_MUTEX, INFINITE);
		{
			Log << "Время: " << (timeGetTime() / 1000.0) << "| ID процесса:  " << GetCurrentProcessId() << "| Читатель начал работу на стр: " << CurrentPage << endl;
		}
		//Освобождение мьютекса
		ReleaseMutex(READER_MUTEX);
		//Запись в память
		CopyMemory(msg,(LPVOID)((intptr_t)MAP_OF_FILE + (CurrentPage * PG_SIZE)), PG_SIZE);
		//Выжидание
		Sleep(500 + (rand() % 1000));
		//Заимствование мьютекса и запись в логи
		WaitForSingleObject(READER_MUTEX, INFINITE);
		{
			Log << "Время: " << (timeGetTime() / 1000.0) << "| ID процесса:  " << GetCurrentProcessId() << "| Читатель закончил работу на стр: " << CurrentPage << "| Сообщение: " << msg[0] << endl;
		}
		//Освобождение мьютекса
		ReleaseMutex(READER_MUTEX);
		//Освобождение семафора - разрешеие на запись
		ReleaseSemaphore(WriteSemaphores[CurrentPage], 1, NULL);

	}
	system("pause");
	
	CloseHandle(Mapping);
	return 0;
}
