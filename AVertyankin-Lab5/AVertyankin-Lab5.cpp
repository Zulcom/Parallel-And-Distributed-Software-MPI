/*
Предполагается, что программа запущена на 3 и более процессах (проверить).
Мастер-процесс (ведущий):
 генерирует и отправляет сообщение (строку символов фиксированной длины)
другим процессам по очереди;
 задает тег сообщения случайным образом;
 принимает ответ от процесса-адресата и выводи его на экран.
Прочие процессы
 при наличии в буфере приема сообщения от ведущего, оно принимается.
 Если тег нечетный, сообщение возвращается процессу-ведущему с добавлением к
сообщению строки «несьедобно».
 Если тег четный, то добавляется строка «съедобно»
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
	// We are assuming at least 2 processes for this task
	if (world_size < 3) {
		fprintf(stderr, "World size must be greater than 3 for %s\n", argv[0]);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	char msg[2];
	if (proc_num == 0) {
	
		for (int i = 1; i < world_size; i++)
		{		
			msg[0] = 'A' + (rand() % 26);
			msg[1] = '\0';
			MPI_Send(&msg, 2, MPI_CHAR, i, rand()%100, MPI_COMM_WORLD);
		}
		char  b[11];
		for (int i = 1; i < world_size; i++)
		{
		MPI_Recv(&b, 11, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("Recived: %s from process %d \n", b,status.MPI_SOURCE);
		}
	}
	else {
		MPI_Recv(&msg, 2, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("Process %d received %s with tag %d\n",proc_num,&msg, status.MPI_TAG);
		string s;
		s+=(msg[0]);
		
		if(status.MPI_TAG%2 != 0)
			s.append("-inedible\0");
		else
			s.append(" - edible\0");
		MPI_Send(s.c_str(), 11, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
		
	}
	MPI_Finalize();
}