#include <algorithm> // место жительства swap()
#include <iostream> // будем использовать потоковый ввод-вывод
#include <vector> // матрицы оформим как векторы
#include "omp.h" // используем openMP
//#include <mpi.h> // используем MPI
#include <ctime>

using namespace std; // чтобы не дописывать пространство имён к векторам, свопам и консоли
					 /* Поиск максимального элемента в столбце
					 * vector<vector<int> > &matrix матирца
					 *  int col столбец
					 *   int n размер
					 *   возвращает позицию максимального элемента в столбце
					 */
int ProcNum, ProcRank;
/* Поиск номера строки от col до n
 * содержащей максимальный по модулю
 * элемент столбца с номером col
 */
int col_max(int *matrix, int col, int n) {
	int max = abs(matrix[n*col+col]);
	int maxPos = col;
	for (int i = col + 1; i < n; ++i) {
		int element = abs(matrix[n*i+col]);
		if (element > max) {
			max = element;
			maxPos = i;
		}
	}
	return maxPos;
}
/* Триангуляция матрицы (приведение к "треугольному")
* Треугольная матрица — матрица, у которой все элементы, стоящие ниже (или выше) главной диагонали, равны нулю.
* vector<vector<int> > &matrix матрица для триангулцияя
* int n её размерность
* Возвращает количество совершенных перестановок
*/
int triangulation(int * matrix, int n) {
	unsigned int swapCount = 0;
	for (int i = 0; i < n - 1; ++i) {
		unsigned int imax = col_max(matrix, i, n);
		if (i != imax) {
			for (int j = 0; j < n; j++)
			{
				swap(matrix[n*i+j], matrix[n*imax+j]);
			}
			++swapCount;
		}
		for (int j = i + 1; j < n; ++j) {
			int mul = -matrix[n*j+i] / matrix[n*i+i];
			for (int k = i; k < n; ++k) {
				matrix[n*j+k] += matrix[n*i+k] * mul;
			}
		}
	}
	return swapCount;
}
/*
int triangulation(int * matrix, int n)
{
	unsigned int swapCount = 0; // здесь будет счётчик количества перестановок. Очевидно, что он только положительный.
	if (0 == n) // если матрица пуста...
		return swapCount; // ...переставлять нечего

	for (int i = 0; i < n - 1; ++i) // пробегаем по всем строкам кроме поледней, потому что мы не сможем переставить последнюю+1
	{
		unsigned int imax = col_max(matrix, i, n); // поиск номера строки содержащей максимальный по модулю элемент столбца с номером i
		if (i != imax) // если текущая строка не максимальна
		{
			swap(matrix[i], matrix[imax]); // ставим ёё его место максимальной
			++swapCount; // накидываем счётчик перестановок
		} 
		for (int j = i + 1; j < n; ++j) // пробегаем по всем элементам строки
		{
			int mul = -matrix[n*j+i] / matrix[n*i+i]; // вычисляем число, на которое нужно домножить
			for (int k = i; k < n; ++k)  // бежим по столбцам снова
			{
				matrix[n*j+k] += matrix[n*i+k] * mul; // домножаем элемент на число и складыываем строки
			}
		}
	}cout << endl;
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			cout << matrix[n*i + j] << " ";
		}
		cout << endl;
	}
	return swapCount; // возвращаем кол-во перестановок
}*/
/* Поиск определителя методом Гаусса
* vector<vector<int> > &matrix матрица для поиска
* int n её размерность
* Возвращает определитель 
*/
int gauss_determinant(int * matrix, int n)
{
	unsigned int swapCount = triangulation(matrix, n); // ищем количество перестановок строк и делаем триангуляцию матрицы
	int determinanit = 1; // объявляем определитель как 1, на случай если перестановок не будет
	if (swapCount % 2 == 1) // если количество перестановок нечётное...
		determinanit = -1; // ..очевидно, что определитель будет отрицательный, поскольку при каждой перестановке он меняет знак
	for (int i = 0; i < n; ++i) // не параллелим этот цикл поскольку на быстродействии это не скажется - только память копиями забьём
	{
		determinanit *= matrix[i + (n*i)]; // считаем произведение элементов на главной диагонали
	}
	return determinanit; // возвращаем их
}


int main(int argc, char *argv[]) // точка входа
{
	srand(time(0));
	int ProcNum, ProcRank;
//	MPI_Init(&argc, &argv);
//	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
//	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	int n = 2;
	int mainDet = 0;
	//if (ProcRank == 0)
	//{
		int * matrix = new int[n*n];// матрица целочисленная на указателях (N*cстрока) + столбец
		for (int i = 0; i < n*n; ++i) // заполнение массива 
				matrix[i] = rand() % 100; // заполяем элемент M_ij случайным числом меньшим 10
	//	int * column = new int[n]; // создаем столбец свободных членов
	//	for (int j = 0; j < n; ++j) //заполняем его - строки
		//	column[j] = rand() % 10; // заполняем элемент Column_j случайным числом
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				cout << matrix[n*i + j] << " ";
			}
			cout << endl;
		}
	//	for (int j = 0; j < n; ++j) 
	//	cout << column[j] << " ";
		mainDet = gauss_determinant(matrix, n); cout << endl;
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				cout << matrix[n*i + j] << " ";
			}
			cout << endl;
		}
		cout << mainDet;
	//	double start_time = MPI_Wtime(); //запоминаем время	
	//	cout << n << " " << MPI_Wtime() - start_time << endl; // выводим в консоль размерность и затраченное на вычисление время
//	}
//	else
	//{
		/*
		cout << ProcRank << " - номер процесса";
		int * private_matrix = new int[n*n]; // сюда будем писать получаемую матрицу
		MPI_Recv(private_matrix,n*n, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // получаем матрицу (получаемое, колчиестово, тип, откуда, тег, коомутатор, статус)
		for (int i = 0; i < n*n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				cout << private_matrix[i] << " ";
			}
			cout << endl;
		}
		int temp_solution = gauss_determinant(private_matrix, n) / mainDet; // ищем определитель и делим на главный, записываем ответ
		MPI_Send(&temp_solution, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); // отправляем этот ответ обратно в главный процесс
		*/
//	}
//	MPI_Finalize();
		system("pause.exe");
	return 0;
}