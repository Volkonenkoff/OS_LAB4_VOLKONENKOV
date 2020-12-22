#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>
#define PG_SIZE 4096 //размер страницы
#define PAGES_COUNT 16 //кол-во страниц (3+0+8+5)
#define PROCESS_COUNT 5 //кол-во процессов

using namespace std;

//Путь до файла для которого будет создаваться проекция
LPCTSTR MF = TEXT("D:\\OS_LAB4\\MAPPING_FILE.txt");
//Логи
LPCTSTR WRITELOG = TEXT("D:\\OS_LAB4\\LOG_WRITER.txt");
LPCTSTR READLOG = TEXT("D:\\OS_LAB4\\LOG_READER.txt");
//Путь до исполняемых файлов
LPCTSTR WRITER_EXE = TEXT("D:\\OS_LAB4\\OS_LAB4_VOLKONENKOV_PART1_WRITER.exe");
LPCTSTR READER_EXE = TEXT("D:\\OS_LAB4\\OS_LAB4_VOLKONENKOV_PART1_READER.exe");
//Путь до проекции файла
TCHAR mname[] = TEXT("Local\\MyFileMapObj");
LPCTSTR NAME = TEXT("READER_MUTEX");
LPCTSTR NAME2 = TEXT("WRITER_MUTEX");

int main()
{
	system("chcp 1251");
	BOOL b; //Для обработки ошибок
	ofstream CREATE_FILES; //Для создания файлов
	HANDLE WriteSemaphores[PAGES_COUNT]; //массив семафоров для писателей
	HANDLE ReadSemaphores[PAGES_COUNT]; //массив семафоров для читателей
	HANDLE WRITER_MUTEX = CreateMutex(NULL, FALSE, NAME2); //мьютекс для записи в логи
	HANDLE READER_MUTEX = CreateMutex(NULL, FALSE, NAME); //мьютекс для записи в логи
	HANDLE MapFileHandle = CreateFile(MF, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); //Проецируемый файл
	HANDLE MapFileMapping = CreateFileMapping(MapFileHandle, NULL, PAGE_READWRITE, 0, PG_SIZE * PAGES_COUNT, mname); //Проекция
	STARTUPINFO READER_SINFO;
	STARTUPINFO WRITER_SINFO;
	PROCESS_INFORMATION WRITER_PINFO[PROCESS_COUNT];
	PROCESS_INFORMATION READER_PINFO[PROCESS_COUNT];

	string temp;
	
	CREATE_FILES.open(WRITELOG, ofstream::out | ofstream::trunc); //Создание файла с логами читателей
	CREATE_FILES.close();
	CREATE_FILES.open(READLOG, ofstream::out | ofstream::trunc); //Создание файла с логами читателей
	CREATE_FILES.close();

	for (int i = 0; i < PAGES_COUNT; i++)
	{
		temp = "WRITE_SEM_ID_" + to_string(i);
		WriteSemaphores[i] = CreateSemaphore(NULL, 1, 1, (LPCTSTR)(temp.c_str())); //Создание семафоров
		temp = "READ_SEM_ID_" + to_string(i);
		ReadSemaphores[i] = CreateSemaphore(NULL, 0, 1, (LPCTSTR)(temp.c_str())); //Создание семафоров
		WaitForSingleObject(ReadSemaphores[i], INFINITY);
		if (WriteSemaphores[i] == NULL || ReadSemaphores[i] == NULL) {
			cerr << "Ошибка при создании семафора: "<< GetLastError()<< endl;
			return -1;
		}
	}
	
	for (int i = 0; i < PROCESS_COUNT; i++)
	{

		ZeroMemory(&WRITER_SINFO, sizeof(WRITER_SINFO));
		WRITER_SINFO.cb = sizeof(WRITER_SINFO);
		ZeroMemory(&(WRITER_PINFO[i]), sizeof(WRITER_PINFO[i]));

		ZeroMemory(&READER_SINFO, sizeof(READER_SINFO));
		READER_SINFO.cb = sizeof(READER_SINFO);
		ZeroMemory(&(READER_PINFO[i]), sizeof(READER_PINFO[i]));


		b = CreateProcess(WRITER_EXE, NULL, NULL, NULL, FALSE, //Создание процесса
			0, NULL, NULL, &WRITER_SINFO, &(WRITER_PINFO[i]));
		if (b==FALSE) {
			cerr << "Ошибка при создании процесса-писателя: " << GetLastError() << endl;
			return -1;
		}
		b = CreateProcess(READER_EXE, NULL, NULL, NULL, FALSE, //Создание процесса
			0, NULL, NULL, &READER_SINFO, &(READER_PINFO[i]));
		if (b==FALSE) {
			cerr << "Ошибка при создании процесса-читателя: " << GetLastError() << endl;
			return -1;
		}
	}

	cout << "Нажмите любую клавишу для остановки процессов\n\n" << endl;
	system("pause");

	CloseHandle(MapFileHandle);
	CloseHandle(MapFileMapping);
	for (int i = 0; i < PROCESS_COUNT; i++)
	{
		CloseHandle(WRITER_PINFO[i].hProcess);
		CloseHandle(WRITER_PINFO[i].hThread);
		CloseHandle(READER_PINFO[i].hProcess);
		CloseHandle(READER_PINFO[i].hThread);
		CloseHandle(WriteSemaphores[i]);
		CloseHandle(ReadSemaphores[i]);
	}
	return 0;
}
