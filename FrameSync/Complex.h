/********************************************************************************/
/* Copyright(c) 2014 Shenzhen Skycaster microelectronic Ltd. All rights reserved. 
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

t_Complex AddComplex(t_Complex a, t_Complex b);		//�ӷ�
t_Complex SubComplex(t_Complex a, t_Complex b);		//����
t_Complex MultiComplex(t_Complex a, t_Complex b);	//�˷�
t_Complex ConjComplex(t_Complex a);		//����
COMPLEX_TYPE ModComplex(t_Complex a);	//��ģ

#endif