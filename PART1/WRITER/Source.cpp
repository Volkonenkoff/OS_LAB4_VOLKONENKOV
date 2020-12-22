#include <iostream>
#include <fstream>
#include <windows.h>
#include <ctime>
#include <string>
#define PG_SIZE 4096 //размер страницы 
#define PAGES_COUNT 16 //кол-во страниц (3+0+8+5)

#pragma comment(lib, "winmm.lib")

using namespace std;

//Путь до файла с логами для читателя
LPCTSTR LOG = TEXT("D:\\OS_LAB4\\LOG_WRITER.txt");
LPCTSTR NAME = TEXT("WRITER_MUTEX");
//Путь до проекции файла
TCHAR mname[] = TEXT("Local\\MyFileMapObj");
int main()
{
	
	
	srand(time(NULL));
	char msg[PG_SIZE];
	HANDLE WriteSemaphores[PAGES_COUNT];
	HANDLE ReadSemaphores[PAGES_COUNT];
	HANDLE WRITER_MUTEX = OpenMutex(MUTEX_MODIFY_STATE | SYNCHRONIZE, FALSE, NAME);  // Открытие созданных мьютексов по имени
	HANDLE Mapping = OpenFileMapping(GENERIC_WRITE, FALSE, mname);
	LPVOID MAP_OF_FILE = MapViewOfFile(Mapping, FILE_MAP_WRITE, 0, 0, PG_SIZE * PAGES_COUNT);
	
	DWORD CurrentPage;
	string temp;


	for (int i = 0; i < PAGES_COUNT; i++) {
		//Открытие созданных семафоров по имени
		temp = "WRITE_SEM_ID_" + to_string(i);
		WriteSemaphores[i] = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, (LPCTSTR)(temp.c_str()));
		temp = "READ_SEM_ID_" + to_string(i);
		ReadSemaphores[i] = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, (LPCTSTR)(temp.c_str()));
		
	}
	
	VirtualLock(MAP_OF_FILE, PG_SIZE * PAGES_COUNT); //Блокировка страниц буферной памяти в оперативной памяти
	ofstream Log;
	Log.open(LOG, fstream::out | fstream::app);//Файл с логами
	
	while (true)
	{
		
		//Запись сообщения
		for (int i = 0; i < PG_SIZE; i++)
		{
			if(i%2==0)
				msg[i] = '1' + rand() % 10;

		}
		//Получение текущей страницы
		
		CurrentPage = WaitForMultipleObjects(PAGES_COUNT, WriteSemaphores, FALSE, INFINITE) - WAIT_OBJECT_0;
		//Заимствование мьютекса и запись в логи
		WaitForSingleObject(WRITER_MUTEX, INFINITE);
		{
			Log << "Время: " << (timeGetTime() / 1000.0) << "| ID процесса: " << GetCurrentProcessId() << "| Писатель начал работу на стр: " << CurrentPage << "| Сообщение: " << msg[0] << endl;
		}
		
		//Освобождение мьютекса
		ReleaseMutex(WRITER_MUTEX);
		//Запись в память
		CopyMemory((LPVOID)((intptr_t)MAP_OF_FILE+ (CurrentPage * PG_SIZE)), msg, PG_SIZE);
		//Выжидание
		Sleep(500 + (rand() % 1000));
		//Заимствование мьютекса и запись в логи
		WaitForSingleObject(WRITER_MUTEX, INFINITE);
		{
			Log << "Время: " << (timeGetTime() / 1000.0) << "| ID процесса: " << GetCurrentProcessId() << "| Писатель закончил работу на стр: " << CurrentPage << endl;
		}
		
		//Освобождение мьютекса
		ReleaseMutex(WRITER_MUTEX);
		//Освобождение семафора - разрешеие на чтение
		ReleaseSemaphore(ReadSemaphores[CurrentPage], 1, NULL);
		
	}
	
	system("pause");
	return 0;
}
