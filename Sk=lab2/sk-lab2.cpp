#include <algorithm> // место жительства swap()
#include <iostream> // будем использовать потоковый ввод-вывод
#include <vector> // матрицы оформим как векторы
#include "omp.h" // используем openMP
#include <mpi.h> // используем MPI

using namespace std; // чтобы не дописывать пространство имён к векторам, свопам и консоли
					 /* Поиск максимального элемента в столбце
					 * vector<vector<int> > &matrix матирца
					 *  int col столбец
					 *   int n размер
					 *   возвращает позицию максимального элемента в столбце
					 */
int ProcNum, ProcRank;

int col_max(int ** matrix, int col, int n)
{
	int max = abs(matrix[col][col]); // берём за минимальный центровой элемент столбца..
	int maxPos = col; // ..запомним его индекс
#pragma omp parallel // параллелим поиски
	{
		int loc_max = max; // для каждого потока будет локальный максимум..
		int loc_max_pos = maxPos; // ...и его позиция
#pragma omp for // параллельно обрабытываем разные части строки
		for (int i = col + 1; i < n; ++i) // мы уже запомнили центровой, поэтому проверяем всё от него до конца
		{
			int element = abs(matrix[i][col]); // запомним текущий элемент
			if (element > loc_max) // если он больше предыдущего максимуму
			{
				loc_max = element; // меняем этот максимум
				loc_max_pos = i; // и запомненную позицию
			}
		}

		if (max < loc_max) // проверяем, больше ли найденные максимум чем тот максимум, который был найден в других потоках
#pragma omp critical // потоки не должны переписывать общий максимум, иначе из-за эффекта гонки будет беда. Помечаем область критической
		{
			max = loc_max; // если да, то меняем "глобальный" максимум
			maxPos = loc_max_pos; // и запомненную позицию	
		}
	}
	return maxPos; // вернём позицию максимального
}
/* Триангуляция матрицы (приведение к "треугольному")
* Треуго́льная матрица — матрица, у которой все элементы, стоящие ниже (или выше) главной диагонали, равны нулю.
* vector<vector<int> > &matrix матрица для триангулцияя
* int n её размерность
* Возвращает количество совершенных перестановок
*/

int triangulation(int ** matrix, int n)
{
	unsigned int swapCount = 0; // здесь будет счётчик количества перестановок. Очевидно, что он только положительный.
	if (0 == n) // если матрица пуста...
		return swapCount; // ...переставлять нечего
	const int num_cols = n; // запомним количество столбцов
	for (int i = 0; i < n - 1; ++i) // пробегаем по всем строкам кроме поледней, потому что мы не сможем переставить последнюю+1
	{
		unsigned int imax = col_max(matrix, i, n); // поиск номера строки содержащей максимальный по модулю элемент столбца с номером i
		if (i != imax) // если текущая строка не максимальна
		{
			swap(matrix[i], matrix[imax]); // ставим ёё его место максимальной
			++swapCount; // накидываем счётчик перестановок
		}
#pragma omp parallel for // параллелим эквивалентные преобразования
		for (int j = i + 1; j < n; ++j) // пробегаем по всем элементам строки
		{
			int mul = -matrix[j][i] / matrix[i][i]; // вычисляем число, на которое нужно домножить
			for (int k = i; k < num_cols; ++k)  // бежим по столбцам снова
			{
				matrix[j][k] += matrix[i][k] * mul; // домножаем элемент на число и складыываем строки
			}
		}
	}
	return swapCount; // возвращаем кол-во перестановок
}
/* Поиск определителя методом Гаусса
* vector<vector<int> > &matrix матрица для поиска
* int n её размерность
* Возвращает определитель
*/
int gauss_determinant(int ** matrix, int n)
{
	unsigned int swapCount = triangulation(matrix, n); // ищем количество перестановок строк и делаем триангуляцию матрицы
	int determinanit = 1; // объявляем определитель как 1, на случай если перестановок не будет
	if (swapCount % 2 == 1) // если количество перестановок нечётное...
		determinanit = -1; // ..очевидно, что определитель будет отрицательный, поскольку при каждой перестановке он меняет знак
	for (int i = 0; i < n; ++i) // не параллелим этот цикл поскольку на быстродействии это не скажется - только память копиями забьём
	{
		determinanit *= matrix[i][i]; // считаем произведение элементов на главной диагонали
	}
	return determinanit; // возвращаем их
}

/* Решение СЛАУ методом Крамера
* vector<vector<int> > &matrix - матрица неизвестых
*  vector<int> &free_term_column - слобец свободных членов
* int n - размерность
* Возвращает вектор, содержащий решения к матрице
*/

int * cramer_solving(int ** matrix, int * free_term_column, int n)
{

	int * solution = new int[n]; // создаем вектор решений
	
	for (int i = 1; i < ProcNum; ++i) //бежим по всем столбцам
	{
		int ** private_matrix = new int*[n];
		for (int k = 0; k < n; ++k) // делаем копию матрицы, чтобы избежать работы в критической секции и эффекта гонки
		{
			private_matrix[k] = new int[n];
			for (int j = 0; j < n; j++)
				private_matrix[k][j] = matrix[k][j];
		}
		for (int j = 0; j < n; j++)
			private_matrix[j][i] = free_term_column[j]; // заменяем столбец на столбец свободных члено
		cout << "From " << i;
		MPI_Send(private_matrix, n*n, MPI_INT, i, 0, MPI_COMM_WORLD); // Отправляем в другой процесс ( посылаемое,количество, тип, куда,тэг, коммунитатор	)	
		
		MPI_Recv(&solution[i], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE); // получаем из другого процесса ответ в матрицу ответов
		cout << "Recived " << &solution[i] << endl;
	}
	
	return solution; // возвращаем решения
}
int main(int argc, char *argv[]) // точка входа
{

	int ProcNum, ProcRank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	int n = 50;
	int mainDet = 0;
	if (ProcNum < 3) {
		fprintf(stderr, "World size must be greater than 3 for %s\n", argv[0]);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	if (ProcRank == 0)
	{
		for (n = 50; n < 350; n += 50) // основной цикл
		{
			int ** matrix = new int*[n];// матрица целочисленная на указателях
			for (int i = 0; i < n; ++i) // заполнение массива - строки
			{
				matrix[i] = new int[n];
				for (int j = 0; j < n; ++j) // заполнение массива - столбцы
					matrix[i][j] = rand() % 10; // заполяем элемент M_ij случайным числом меньшим 10
			}
			int * column = new int[n]; // создаем столбец свободных членов
			for (int j = 0; j < n; ++j) //заполняем его - строки
				column[j] = rand() % 10; // заполняем элемент Column_j случайным числом
			mainDet = gauss_determinant(matrix, n);
			double start_time = MPI_Wtime(); //запоминаем время
			int * solution = cramer_solving(matrix, column, n); // запоминаем вектор корней СЛАУ, полученный из автоматически оптимизированной функции cramer_solving,
																//которая принимает матрицу для поиска, столбец свободных членов, размерность матрицы (без столбца св. членов)
			cout << n << " " << MPI_Wtime() - start_time << endl; // выводим в консоль размерность и затраченное на вычисление время
			for (int i = 0; i < n; i++)
			{
		//		cout << solution[0] << " ";
			}
		//	cout << endl;
		}
	}
	else
	{
		cout << ProcRank << " - номер процесса";
		int ** private_matrix = new int*[n]; // сюда будем писать получаемую матрицу
		MPI_Recv(private_matrix,n*n, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // получаем матрицу (получаемое, колчиестово, тип, откуда, тег, коомутатор, статус)
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				cout << private_matrix[i][j] << " ";
			}
			cout << endl;
		}
		int temp_solution = gauss_determinant(private_matrix, n) / mainDet; // ищем определитель и делим на главный, записываем ответ
		MPI_Send(&temp_solution, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); // отправляем этот ответ обратно в главный процесс
	}
	MPI_Finalize();
	
	return 0;
}