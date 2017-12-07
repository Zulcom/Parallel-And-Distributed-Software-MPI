#include <algorithm> // место жительства swap()
#include <iostream> // будем использовать потоковый ввод-вывод
#include <vector> // матрицы оформим как векторы
#include <mpi.h> // используем MPI
#include <ctime>

using namespace std; // чтобы не дописывать пространство имён к векторам, свопам и консоли

// Function to get cofactor(это матрица, которую мы можем получить, удалив строку и столбец этого элемента из этой матрицы.) of mat[p][q] in temp[][]. n is current
// dimension of mat[][]
void getCofactor(double* mat, double* temp, int p, int q, int n)
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
int det(double* mat, int n)
{
	int D = 0; // Initialize result

	//  Base case : if matrix contains single element
	if (n == 1)
		return mat[0];

	double* temp = new double[n * n]; // To store cofactors

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
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			cout << temp[i*n + j] << " ";
		}
		cout << endl;
	}
	delete[] temp;
	return D;
}


int main(int argc, char* argv[]) // точка входа
{
	srand(time(0));
	int size, num_proc;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &num_proc);
	const unsigned int n = 3;
	double mainDet = 0;
	double* matrix = new double[n * n];// матрица целочисленная на указателях (N*cстрока) + столбец
	double * column = new double[n];
	if (0 == num_proc) {
		
		double temparr[9] = { 37, 95, 49, 48, 0, 26, 3, 61, 86 };
		for (int i = 0; i < n * n; ++i) // заполнение массива 
			matrix[i] = temparr[i]; //rand() % 100; // заполяем элемент M_ij случайным числом меньшим 10
		
		mainDet = det(matrix, n);
		cout << "Main det: " << det(matrix, n) << endl;
		for (int i = 0; i < n; i++) {
			column[i] = rand() % 10;
		}
		MPI_Bcast(column, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(matrix, n*n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		for (int i = 1; i < size; ++i) {
			int currCol = i - 1;
			MPI_Send(&currCol, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
		double *solution = new double[n];
		for (int i = 1; i < size; ++i) {
			double tempAnsw;
			MPI_Recv(&tempAnsw, 1, MPI_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			solution[status.MPI_TAG] = tempAnsw/mainDet;
		}
	/*	for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				cout << matrix[i*n + j] << " ";
			}
			cout << column[i] << endl;
		}
		for (int i = 0; i < n; i++)
		{
			cout << solution[i] << " ";
		}*/
		delete[] solution;
	}
	else
	{
		MPI_Bcast(column, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(matrix, n*n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		int myCol;
		MPI_Recv(&myCol, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		for (int i = 0; i < n; i++)
		{
			matrix[n*i + myCol] = column[i];
		}
		double thisDet = det(matrix, n);
	    MPI_Send(&thisDet, 1, MPI_DOUBLE, 0, myCol, MPI_COMM_WORLD);
	}
	delete[] matrix;
	delete[] column;
	MPI_Finalize();
	return 0;
}
