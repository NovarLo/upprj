#ifndef _APP_LINEARH
#define _APP_LINEARH


#ifdef	_LOCAL_LINEAR
	#define	_EXTERN
#else
	#define	_EXTERN	extern
#endif

#define ROW 32
#define COL 32

void get_coefficient(double [][COL], int, int);
void get_vector(double [], int);
void create_Ab(double [][COL], double [], int, int);
int guass_elimination(double *[ROW], int, int);
void exchange_row(double *[ROW], int, int);
void get_beta(double *[ROW], int row, int col, double[]);
double get_alpha(double *[ROW], int row, int col, double[]);
void get_correctionfactor(double ALPHA[ROW]);


#undef	_EXTERN
#endif

