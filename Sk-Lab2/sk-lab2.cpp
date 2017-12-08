#include <algorithm> // место жительства swap()
#include <iostream> // будем использовать потоковый ввод-вывод
#include <vector> // матрицы оформим как векторы
#include <mpi.h> // используем MPI
#include <ctime>


//(N*cстрока) + столбец
using namespace std; // чтобы не дописывать пространство имён к векторам, свопам и консоли

/**
 *  
 * Поиск номера сроки, имеющий максимальный по модулю элемент matrix[искомое][i]
 */
template <typename T>
int col_max(T* matrix, int col, int n)
{
	T max = abs(matrix[col * n + col]);
	T maxPos = col;
#pragma omp parallel
	{
		T loc_max = max;
		T loc_max_pos = maxPos;
#pragma omp for
		for (int i = 0; i < n; ++i)
		{
			T element = abs(matrix[i * n + col]);
			if (element > loc_max)
			{
				loc_max = element;
				loc_max_pos = i;
			}
		}
#pragma omp critical
		{
			if (max < loc_max)
			{
				max = loc_max;
				maxPos = loc_max_pos;
			}
		}
	}
	return maxPos;
}

template <typename T>
unsigned int triangulation(T* matrix, int n)
{
	unsigned int swapCount = 0;
	for (int i = 0; i < n - 1; ++i)
	{
		if (0 == matrix[i * n + i])
		{
			unsigned int imax = col_max(matrix, i, n);
			if (i != imax)
			{
				for (int j = 0; j < n; ++j)//(N*cстрока) + столбец
				{
					swap(matrix[n * i + j], matrix[n * imax + j]);
				}
				//swap(matrix[i], matrix[imax]);
				++swapCount;
			}
		}
#pragma omp parallel for
		for (int j = i + 1; j < n; ++j)
		{
			T mul = matrix[j * n + i] / matrix[i * n + i];
			for (int k = i; k < n; ++k)
				matrix[j * n + k] -= matrix[i * n + k] * mul;
		}
	}
	return swapCount;
}

/* Поиск определителя методом Гаусса
 * vector<vector<T> > &matrix матрица для поиска
 * int n её размерность
 * Возвращает определитель
 */
template <typename T>// шаблон функции, реальная будет создана на этапе компиляции и масимально оптимизированна
T gauss_determinant(T* matrix, int n)
{
	unsigned int swapCount = triangulation(matrix, n); // ищем количество перестановок строк и делаем триангуляцию матрицы
	T determinanit = 1; // объявляем определитель как 1, на случай если перестановок не будет
	if (swapCount % 2 == 1) // если количество перестановок нечётное...
		determinanit = -1;
	// ..очевидно, что определитель будет отрицательный, поскольку при каждой перестановке он меняет знак
	for (int i = 0; i < n; ++i)
		// не параллелим этот цикл поскольку на быстродействии это не скажется - только память копиями забьём
	{
		determinanit *= matrix[n * i + i]; // считаем произведение элементов на главной диагонали
	}
	return determinanit; // возвращаем их
}

int main(int argc, char* argv[]) // точка входа
{
	srand(time(0));
	int size, num_proc = 0;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &num_proc);
	const unsigned int n = 3;
	double mainDet;
	double* matrix = new double[n * n];// матрица целочисленная на указателях (N*cстрока) + столбец
	double* column = new double[n];
	if (0 == num_proc)
	{
		double temparr[9] = {1, 55, 3, 0, 3, 3 , 2, 1, 3};
		for (int i = 0; i < n * n; ++i) // заполнение массива 
			matrix[i] = temparr[i]; //rand() % 100; // заполяем элемент M_ij случайным числом меньшим 10
		double * tempMatrix = new double[n*n];
		for (int i = 0; i < n*n; i++)
				tempMatrix[i] = matrix[i];
		mainDet = gauss_determinant(tempMatrix, n);
		delete[] tempMatrix;
		   if (abs(mainDet) < 0.0001) MPI_Abort(MPI_COMM_WORLD, 1);
		for (int i = 0; i < n; i++)
			column[i] = rand() % 10;
		MPI_Bcast(column, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(matrix, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		double* solution = new double[n];
		for (int i = 0; i < n; ++i)
		{
			double tempAnsw;	
			MPI_Recv(&tempAnsw, 1, MPI_DOUBLE, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &status);
			solution[status.MPI_TAG] = tempAnsw / mainDet;
		}

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				cout << matrix[i * n + j] << " ";
			}
			cout << column[i] << endl;
		}
		for (int i = 0; i < n; i++)
		{
			cout << solution[i] << " ";
		}

		delete[] solution;
	}
	else
	{
		MPI_Bcast(column, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(matrix, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		double * tempMatrix = new double[n*n];
		int m = ceil(n / static_cast<double>(size));
		int begin = num_proc == 1 ? 0 : m*num_proc-1;
		int end = begin + m > n ? n : begin + m;
		cout << m << ": "<< num_proc << " from " << begin << " to " << end<<endl;
		for(int thisCol = begin;thisCol<end;++thisCol){
			for (int j = 0; j < n*n; j++) tempMatrix[j] = matrix[j];
			for (int i = 0; i < n; i++){
				tempMatrix[n * i + thisCol] = column[i];
			}
			double thisDet = gauss_determinant(tempMatrix, n);
			MPI_Send(&thisDet, 1, MPI_DOUBLE, 0, thisCol, MPI_COMM_WORLD);
		}
	
	}
	delete[] matrix;
	delete[] column;
	MPI_Finalize();
	return 0;
}
