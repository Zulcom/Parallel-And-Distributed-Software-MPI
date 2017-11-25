/*
Исходные данные: одномерный символьный массив размерностью N(строка)
Шифровальная таблица (пара значений исх-замена)
*/
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

	char * msg = new char[world_size + 1];
	if (proc_num == 0) {
		char *subs = "ZYXWVUTSRQPONMLKJIHGFEDCBA"; //27
		for (int i = 0; i<100; ++i)
			msg[i] = 'A' + (rand() % 26);
		msg[100] = '\0';
		printf("Process 0 start program with \"%s\" \n", msg);
		int blocksize = 100 / (world_size - 1);
		for (int i = 1; i < world_size; ++i) {
			MPI_Send(&blocksize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(msg + blocksize*(i - 1), blocksize, MPI_CHAR, i, 0, MPI_COMM_WORLD);
			MPI_Send(table, 27, MPI_CHAR, i, 0, MPI_COMM_WORLD);
		}
		for (int i = 1; i < world_size; ++i) {
			char *rcvmsg = new char[blocksize];
			MPI_Recv(rcvmsg, blocksize + 1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			printf("Process 0 received %s from process %d \n", rcvmsg, status.MPI_SOURCE);
		}
	}
	else {

		int blocksize = 0;
		char *table = new char[27];
		MPI_Recv(&blocksize, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		char * msg = new char[blocksize + 1];
		MPI_Recv(msg, blocksize, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		msg[blocksize] = '\0';
		MPI_Recv(table, 27, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Process %d received %s from proccess %d \n", proc_num, msg, status.MPI_SOURCE);
		for (int i = 0; i < blocksize; ++i) msg[i] = table[msg[i] - 'A'];
		MPI_Send(msg, blocksize + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
}