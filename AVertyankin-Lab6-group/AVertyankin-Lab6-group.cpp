/*
Исходные данные: одномерный символьный массив размерностью N(строка)
Шифровальная таблица (пара значений исх-замена)
*/
#pragma warning(disable:4996)
#include<iostream>
#include<mpi.h>
#include <string>
#include <windows.h>
#include <stdio.h>
#include <ctime>

using namespace std;

int main(int *argc, char*** argv)
{
	srand(time(0));
	MPI_Init(argc, argv);
	int proc_num;
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_num);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Status status;

	char* msg = new char[100];
	char* rcv = new char[100];
	char* rcvBuf;
	char* table = new char[27];
	if (proc_num == 0)
	{
		table = "ZYXWVUTSRQPONMLKJIHGFEDCBA"; //27
		for (int i = 0; i < 100; ++i)
			msg[i] = 'A' + (rand() % 26);
		msg[100] = '\0';
		printf("Process 0 start program with \"%s\" \n", msg);
		int blocksize = 100 / world_size;
		MPI_Bcast(&blocksize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		rcvBuf = new char[blocksize];
		MPI_Bcast(table, 27, MPI_CHAR, 0, MPI_COMM_WORLD);
		MPI_Scatter(msg, blocksize, MPI_CHAR, rcvBuf, blocksize, MPI_CHAR, 0, MPI_COMM_WORLD);
		char* toedit = new char[blocksize];
		copy(msg + blocksize * proc_num, msg + blocksize * proc_num + blocksize, toedit);
		for (int i = 0; i < blocksize; ++i) toedit[i] = table[toedit[i] - 'A'];
		MPI_Gather(toedit, blocksize, MPI_CHAR, rcv, blocksize, MPI_CHAR, 0, MPI_COMM_WORLD);
		rcv[100] = '\0';
		printf("Process 0 received %s\n", rcv);
	}
	else
	{
		int blocksize = 0;
		MPI_Bcast(&blocksize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		rcvBuf = new char[blocksize];
		MPI_Bcast(table, 27, MPI_CHAR, 0, MPI_COMM_WORLD);
		MPI_Scatter(msg, blocksize, MPI_CHAR, rcvBuf, blocksize, MPI_CHAR, 0, MPI_COMM_WORLD);
		printf("Process %d from %c to %c\n", proc_num, rcvBuf[0], rcvBuf[blocksize - 1]);
		for (int i = 0; i < blocksize; ++i) rcvBuf[i] = table[rcvBuf[i] - 'A'];
		MPI_Gather(rcvBuf, blocksize, MPI_CHAR, rcv, blocksize, MPI_CHAR, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
}
