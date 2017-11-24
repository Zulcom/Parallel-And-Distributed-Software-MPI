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
int main(int argc, char** argv) {
	srand(time(0));
	// Initialize the MPI environment
	MPI_Init(NULL, NULL);
	// Find out rank, size
	int proc_num;
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_num);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Status status;
	// We are assuming at least 3 processes for this task
	if (world_size < 3) {
		fprintf(stderr, "World size must be greater than 3 for %s\n", argv[0]);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	char * msg = new char[100];
	if (proc_num == 0) {
		char *table =  "ZYXWVUTSRQPONMLKJIHGFEDCBA"; //27
		for(int i=0;i<100;++i)
			msg[i] = 'A' + (rand() % 26);
		    msg[100] = '\0'; 
		printf("Process 0 start program with \"%s\" \n", msg);
		int blocksize = 100 / (world_size-1);
		MPI_Bcast(&blocksize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(table, 27, MPI_CHAR, 0, MPI_COMM_WORLD);
		MPI_Bcast(msg, 101, MPI_CHAR, 0, MPI_COMM_WORLD);
		for (int i = 1; i < world_size; ++i) {
			MPI_Send(msg + blocksize*(i-1), blocksize, MPI_CHAR, i, 0, MPI_COMM_WORLD);
		}
		for (int i = 1; i < world_size; ++i) {
			char *rcvmsg = new char[blocksize];
			MPI_Recv(rcvmsg, blocksize+1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			printf("Process 0 received %s from process %d \n", rcvmsg, status.MPI_SOURCE);
		}
	}
	else {
		
		int blocksize = 0;
		char *table = new char[27];
		char * msg = new char[100];
		
		MPI_Bcast(&blocksize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(table, 27, MPI_CHAR, 0, MPI_COMM_WORLD);
		MPI_Bcast(msg, 101, MPI_CHAR, 0, MPI_COMM_WORLD);
		char * toedit = new char[blocksize + 1];
		copy(msg + blocksize*(proc_num - 1), msg + blocksize*(proc_num - 1) + blocksize, toedit);
		toedit[blocksize] = '\0';
		for (int i = 0; i < blocksize; ++i) toedit[i] = table[toedit[i] - 'A'];
		MPI_Send(toedit, blocksize+1, MPI_CHAR,0, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
}