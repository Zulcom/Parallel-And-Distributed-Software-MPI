
#include <iostream>
#include <vector>
#include <stdio.h>
#include <climits>
#include <cstdlib>
#include <cmath>
#include <algorithm> // for the transform function
#include <fstream>  // for inputting the text file
#include <string.h> // for string functions
#include <mpi.h>
using namespace std;


const vector<size_t> create_table(const unsigned char* str, size_t str_length)
{
	vector<size_t> occ(UCHAR_MAX + 1, str_length);

	if (str_length >= 1)
	{
		for (size_t i = 0; i<(str_length - 1); ++i)
			occ[str[i]] = (str_length - 1) - i;
	}

	return occ;
}


int main(int argc, char *argv[])
{
	vector<size_t> occ1;
	MPI_File file;
	const char* str;
	char* filename;

	char* text;
	unsigned char occ_char;
	size_t text_pos = 0;
	int  ierr;
	int temp_count = 0, node_num, size, final_count = 0;
	char *block;
	int overlap;
	double start_time;
	MPI_Offset filesize;
	MPI_Offset blocksize;
	MPI_Offset start;
	MPI_Offset end;
	
	filename = "alice.txt";
	str = "alice";
	size_t str_len = strlen(str);

	// Добавляем перекрытие, чтобы гарантировать, что разделение строки не приведет к неправильным результатам.
	overlap = 100;

	//Генерируем таблицу совпадений. reinterpret_cast для соответтвия аргументам функции
	occ1 = create_table(reinterpret_cast <const unsigned char*> (str), str_len);
	//Initialize MPI environment.
	MPI_Init(&argc, &argv);
	// Количество узлов
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	// Номер узла
	MPI_Comm_rank(MPI_COMM_WORLD, &node_num);
	if (0 == node_num) start_time = MPI_Wtime();
	ierr = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
	// Ловим ошибку открытия файла
	if (ierr) {
		if (node_num == 0)
			cout << "MPI_File_open Failed !" << endl;
		MPI_Finalize();
		exit(2);
	}

	// Считаем длину текста и сколько текста должно быть выдано каждому узлу
	MPI_File_get_size(file, &filesize);
	blocksize = filesize / (size-1);

	// Создаём указатели на начало и конец у каждого узла
	/* Выделяем память для блока*/
	block = new char[blocksize + 1 + overlap];

	//Читаем данные в блок
	if (0 == node_num) {
		text = new char[filesize];
		MPI_File_read(file,text,filesize,MPI_CHAR,MPI_STATUS_IGNORE);
		cout << "String '" << str << "' is being searched in file '" << filename << "'" << endl;
		for (int i = 1; i < size; i++)
		{
			MPI_Send(text+blocksize*(i-1),i == size-1 ? blocksize - 1: blocksize-1+overlap, MPI_CHAR, i, 0, MPI_COMM_WORLD);
		}
		
	}
	else
	{
		cout << "Length of file " << filesize << " is divided into blocksize " << blocksize << " for node " << node_num << endl;
		MPI_Recv(block, node_num == size-1 ? blocksize - 1 : blocksize - 1 + overlap, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		block[blocksize] = '\0';
		// Задаём указатель на начало текста
		text = &block[0];
		while (text_pos <= (blocksize - str_len))
		{
			// Выбираем символ с позиции равной длине искомой -1 для зануления
			// нулевой элемент
			occ_char = text[text_pos + str_len - 1];

			// Если последний символ шаблона совпадает с текущим символом в тексте и
			// если символы впереди текущего в тексте содержит искомой, у нас есть совпадение.
			// memcmp сравнивает символы впереди текущего с искомым в блоках длины  str_len -1.

			if (occ_char == str[str_len - 1] && (memcmp(str, text + text_pos, str_len - 1) == 0))
			{
				temp_count++;
			}

			// Смотрим на таблицу совпадений и решаем как увеличивать текстовый указатель
			text_pos += occ1[occ_char];
		}			
	}
	// Добавляем  сумму каждого узла к общей сумме используя mpi_reduce
	MPI_Reduce(&temp_count, &final_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	if (node_num == 0) {
		cout << "Total number of occurances of string in text = " << final_count << endl;
		cout << "In " << (MPI_Wtime() - start_time) << " second" <<endl;
	}
	
	MPI_File_close(&file);

	MPI_Finalize();

	return 0;

}



