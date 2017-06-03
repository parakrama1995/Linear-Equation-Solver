#include "Matrix.h"
#include"GameState.h"
#include<iostream>
#include<iomanip>
#include<cmath>
using namespace std;

Matrix::Matrix()
{
	n = 0;
}

Matrix::~Matrix()
{
}

void Matrix::pushRow(vector<double>r)
{
	//add whole row to matrix at the end
	M.push_back(r);
	n++;	
}

void Matrix::displayMatrix()
{
	b_columnIndex = n + 1;
	for (int y = 0; y < n; y++)
	{
		for (int x = 0; x < b_columnIndex; x++)//n+1 because column of intercept is interesting also
		{
			cout << "|" <<setprecision(3)<< setw(8) << M[y][x];
		}cout << endl << endl;
	}
}

void Matrix::displayResults()
{
	cout << "x(1,0,0) = Gauss-Seidel        \t\t" << setprecision(36) << x1[0] << endl;
	cout << "x(1,0,0) = Gaussian Elimination\t\t" << setprecision(36)  << x2[0] << endl;
}

vector<vector<double>> Matrix::getMatrix()
{
	return M;
}

void Matrix::loadMatrix(vector<vector<double>> copyofM)
{
	M = copyofM;
}

void Matrix::gaussSeidel()
{
	b_columnIndex = n;

	for (int k = 0; k < n - 1; k++)
	{
		for (int i = k + 1; i < n; i++)
		{
			double m = M[i][k] / M[k][k];
			for (int j = k + 1; j < n; j++)
				M[i][j] -= m * M[k][j];
			M[i][b_columnIndex] -= m * M[k][b_columnIndex];
		}
	}
	//reserve memory for results
	x1.assign(n, 0);
	x1[n - 1] = M[n - 1][b_columnIndex] / M[n - 1][b_columnIndex - 1];

	for (int i = n - 2; i >= 0; i--)
	{
		x1[i] = M[i][b_columnIndex];//i albo l
		for (int j = i + 1; j < n; j++)
		{
			x1[i] -= M[i][j] * x1[j];
		}
		x1[i] /= M[i][i];
	}
}

void Matrix::gaussElimination()
{
	b_columnIndex = n;

	for (int i = 0; i < n - 1; i++)
	{
		//find max in 'i' column
		double maxElement = abs(M[i][i]);
		int rowWithMax = i;
		for (int k = i + 1; k < n; k++)
		{
			if (abs(M[k][i]) > maxElement)
			{
				maxElement = abs(M[k][i]);
				rowWithMax = k;
			}
		}

		//swap maximum row with current row (column by column)
		for (int k = i; k <= b_columnIndex; k++) {
			double tmp = M[rowWithMax][k];
			M[rowWithMax][k] = M[i][k];
			M[i][k] = tmp;
		}

		//make all rows below this one 0 in current column
		for (int k = i + 1; k < n; k++) {
			double c = -M[k][i] / M[i][i];
			for (int j = i; j < n + 1; j++) {
				if (i == j) {
					M[k][j] = 0;
				}
				else {
					M[k][j] += c * M[i][j];
				}
			}
		}
	}
	// Solve equation Ax=b for an upper triangular matrix A
	x2.assign(n, 0);
	for (int i = n - 1; i >= 0; i--) {
		x2[i] = M[i][n] / M[i][i];
		for (int k = i - 1; k >= 0; k--) {
			M[k][n] -= M[k][i] * x2[i];
		}
	}
}
