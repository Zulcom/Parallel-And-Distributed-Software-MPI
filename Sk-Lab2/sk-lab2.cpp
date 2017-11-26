#include <algorithm> // место жительства swap()
#include <iostream> // будем использовать потоковый ввод-вывод
#include <vector> // матрицы оформим как векторы
#include "omp.h" // используем openMP
//#include <mpi.h> // используем MPI
#include <ctime>

using namespace std; // чтобы не дописывать пространство имён к векторам, свопам и консоли

// Function to get cofactor(это матрица, которую мы можем получить, удалив строку и столбец этого элемента из этой матрицы.) of mat[p][q] in temp[][]. n is current
// dimension of mat[][]
void getCofactor(int* mat, int* temp, int p, int q, int n)
{
	int i = 0, j = 0;

	// Looping for each element of the matrix
	for (int row = 0; row < n; row++)

		for (int col = 0; col < n; col++)

			//  Copying into temporary matrix only those element
			//  which are not in given row and column
			if (row != p && col != q)
			{
				temp[i * n + (j++)] = mat[row * n + col];

				// Row is filled, so increase row index and
				// reset col index
				if (j == n - 1)
				{
					j = 0;
					i++;
				}
			}
}

/* Recursive function for finding determinant of matrix.
n is current dimension of mat[][]. */
int det(int* mat, int n)
{
	int D = 0; // Initialize result

	//  Base case : if matrix contains single element
	if (n == 1)
		return mat[0];

	int* temp = new int[n * n]; // To store cofactors

	int sign = 1; // To store sign multiplier

	// Iterate for each element of first row
	for (int i = 0; i < n; i++)
	{
		// Getting Cofactor of mat[0][f]
		getCofactor(mat, temp, 0, i, n);
		D += sign * mat[i] * det(temp, n - 1);

		// terms are to be added with alternate sign
		sign = -sign;
	}

	return D;
}


int main(int argc, char* argv[]) // точка входа
{
	srand(time(0));
	int ProcNum, ProcRank;
	int n = 2;
	int mainDet = 0;
	int* matrix = new int[n * n];// матрица целочисленная на указателях (N*cстрока) + столбец
	for (int i = 0; i < n * n; ++i) // заполнение массива 
		matrix[i] = rand() % 100; // заполяем элемент M_ij случайным числом меньшим 10
	mainDet = det(matrix, n);
	cout << endl;
	cout << mainDet;
	system("pause.exe");
	return 0;
}
