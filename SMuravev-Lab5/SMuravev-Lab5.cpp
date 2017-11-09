/*
Вариант world_size. Испорченный телефон
Предполагается, что программа запущена на 3 и более процессах (проверить).
Мастер-процесс является ведущим.
Ведущий:
 генерирует сообщение (строку символов, длина строки равна числу процессов) и
выводит его на экран.
 отсылает сообщение процессу с большим номером
 принимает сообщение от процесса с номером N и выводит его на экран
 генерирует и передает следующее сообщение.
Все процессы-абоненты:
 Принимают сообщение от соседа с меньшим номером;
 Заменяют в строке символ, номер которого совпадает с номером процесса, на
случайную букву.
 Пересылает сообщение дальше.
 Процесс с номером N передает сообщение процессу с номером 0.
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

	char * msg = new char[world_size+1];
	if (proc_num == 0) {
		for(int i=0;i<world_size;++i)
			msg[i] = 'A' + (rand() % 26);
		    msg[world_size] = '\0'; 
		printf("Process 0 start program with \"%s\" \n", msg);
		MPI_Send(msg, world_size+1, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
			//memset(msg[0], 0, sizeof(msg));
			MPI_Recv(msg, world_size+1, MPI_CHAR, world_size-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			printf("Process 0 received %s from process %d \n", msg, status.MPI_SOURCE);
	}
	else {
		char * msgg = new char[world_size+1];
		MPI_Recv(msgg, world_size+1, MPI_CHAR, proc_num-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("Process %d received %s from proccess %d \n", proc_num, msgg,status.MPI_SOURCE);
		msgg[proc_num] = 'A' + (rand() % 26);
		MPI_Send(msgg, world_size+1, MPI_CHAR, proc_num+1 == world_size ? 0 : proc_num + 1, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
}