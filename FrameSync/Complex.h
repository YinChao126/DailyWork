/********************************************************************************/
/* Author:		YinChao
/* Date:		2016.11.2
/* Description: Library for complex's mathematical computation
/********************************************************************************/
#ifndef __COMPLEX_H
#define __COMPLEX_H

#include <math.h>

typedef int COMPLEX_TYPE;

typedef struct  
{
	COMPLEX_TYPE Re;
	COMPLEX_TYPE Im;
}t_Complex;

t_Complex AddComplex(t_Complex a, t_Complex b);		//加法
t_Complex SubComplex(t_Complex a, t_Complex b);		//减法 a - b
t_Complex MultiComplex(t_Complex a, t_Complex b);	//乘法
t_Complex ConjComplex(t_Complex a);		//共轭
COMPLEX_TYPE ModComplex(t_Complex a);	//求模
COMPLEX_TYPE NormComplex(t_Complex a);  //模的平方
void AssignComplex(t_Complex *DataOut, COMPLEX_TYPE Re, COMPLEX_TYPE Im); //赋值
#endif