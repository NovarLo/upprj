#define _LOCAL_LINEAR

#include "stm32f4xx_hal.h"
#include<stdio.h>
#include<math.h>
#include "app_user.h"
#include "app_linear.h"

void get_coefficient(double matrix[ROW][COL], int row, int col)
{
	int i, j;

	for (i = 0; i < row ; i++)
	{
		for (j = 0; j < col; j++)
		{
			matrix[i][j] = cali_tbl.upsnr_code[i + 1][j] - cali_tbl.upsnr_code[0][j];
		}
	}
	return;
}

void get_vector(double vector[ROW], int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		vector[i] = cali_tbl.upsnr_cornweight;
	}

	return;
}

void create_Ab(double matrix[ROW][COL], double vector[ROW], int row, int col)
{
	int i;

	for (i = 0; i < row; i++) matrix[i][col] = vector[i];

	return;
}

int guass_elimination(double *matrix[ROW], int row, int col)
{
	int result, i, j, k;
	double coe;

	for (i = 0; i < row - 1; i++)
	{
		exchange_row(matrix, i, row);
		if (fabs(*(matrix[i] + i)) < 0.00001) continue;
		for (j = i + 1; j < row; j++)
		{
			coe = *(matrix[j] + i) / *(matrix[i] + i);
			for (k = i; k < col; k++) *(matrix[j] + k) -= coe * *(matrix[i] + k);
		}
	}

	if (col - 1 > row) result = 1;
	else if (col - 1 == row)
	{
		if (fabs(*(matrix[row - 1] + row - 1)) > 0.00001) result = 0;
		else
		{
			if (fabs(*(matrix[row - 1] + row)) > 0.00001) result = -1;
			else result = 1;
		}
	}
	else
	{
		result = 0;
		for (i = 0; i < row; i++) if (fabs(*(matrix[i] + col - 2)) < 0.00001 && fabs(*(matrix[i] + col - 1)) > 0.00001)
			{
				result = -1;
				break;
			}
	}


	return result;
}

void exchange_row(double *matrix[ROW], int flag, int row)
{
	int i;
	double *temp;

	for (i = flag + 1; i < row; i++) if (fabs(*(matrix[flag] + flag)) < fabs(*(matrix[i] + flag)))
		{
			temp = matrix[flag];
			matrix[flag] = matrix[i];
			matrix[i] = temp;
		}

	return;
}

void get_beta(double *matrix[ROW], int row, int col, double B[ROW])
{
	int i, j, k, size;
	double A[ROW][ROW] = { 0 };
	double b[ROW] = { 0 };
	double x[ROW] = { 0 };
	double S;

	size = row;
	for (i = 0; i < row; i++)
	{
		b[i] = *(matrix[i] + col - 1);
	}
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col - 1; j++)
		{
			A[i][j] = *(matrix[i] + j);
		}
	}
	x[size - 1] = b[size - 1] / A[size - 1][size - 1];
	for (k = size - 2; k >= 0; k--)
	{
		S = b[k];
		for (j = k + 1; j < size; j++)
		{
			S = S - A[k][j] * x[j];
		}
		x[k] = S / A[k][k];
	}
	for (i = 0; i < size; i++)
	{
		B[i] = x[i];
	}
}

double get_alpha(double *matrix[ROW], int row, int col, double B[ROW])
{
	int i;
	double msum, K;

	msum = 0;
	for (i = 0; i < col; i++)
	{
		msum += B[i] * (cali_tbl.upsnr_code[row + 1][i] - cali_tbl.upsnr_code[0][i]);
	}

	K = cali_tbl.upsnr_dispweight / msum;
	return K;
}

void get_correctionfactor(double ALPHA[ROW])
{
	double Receptacle[ROW][COL];
	double Vector[ROW];
	double *Ab_pointer[ROW], X[ROW];
	int row, col, i, result;

	row = UPSNR_NUM;
	col = UPSNR_NUM;
	get_coefficient(Receptacle, row, col);
	get_vector(Vector, row);
	create_Ab(Receptacle, Vector, row, col);

	for (i = 0; i < ROW; i++) Ab_pointer[i] = Receptacle[i];

	guass_elimination(Ab_pointer, row, col + 1);

	get_beta(Ab_pointer, row, col + 1, X);
	
	get_alpha(Ab_pointer, row, col, X);

	for (i = 0; i < UPSNR_NUM; i++)
	{
		ALPHA[i] = X[i];
	}
}

#undef _LOCAL_LINEAR
/**************************** end line ****************************/

