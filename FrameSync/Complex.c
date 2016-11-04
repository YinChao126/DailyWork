#include "Complex.h"

t_Complex AddComplex(t_Complex a, t_Complex b)
{
	t_Complex c = { 0 };
	c.Re = a.Re + b.Re;
	c.Im = a.Im + b.Im;
	return c;
}

t_Complex SubComplex(t_Complex a, t_Complex b)
{
	t_Complex c = { 0 };
	c.Re = a.Re - b.Re;
	c.Im = a.Im - b.Im;
	return c;
}

t_Complex MultiComplex(t_Complex a, t_Complex b)
{
	t_Complex c = { 0 };
	c.Re = a.Re * b.Re - a.Im * b.Im;
	c.Im = a.Im * b.Re + a.Re * b.Im;
	return c;
}

t_Complex ConjComplex(t_Complex a)
{
	t_Complex c = { 0 };
	c.Re = a.Re;
	c.Im = -a.Im;
	return c;
}

COMPLEX_TYPE NormComplex(t_Complex a)
{
	return a.Im * a.Im + a.Re * a.Re;
}

void AssignComplex(t_Complex *DataOut, COMPLEX_TYPE Re, COMPLEX_TYPE Im)
{
	DataOut->Re = Re;
	DataOut->Im = Im;
}
#if 0 //�����ͨ��DSPӲ���˷���ʵ�ֵģ����ֱ��ʹ�ó˷�
COMPLEX_TYPE ModComplex(t_Complex a)
{
	return sqrt(a.Re * a.Re + a.Im * a.Im);
}
#else //������ǣ���ʹ����λ������˷����Լ��ټ�����
COMPLEX_TYPE ModComplex(t_Complex a)
{
	COMPLEX_TYPE Re, Im, Max, Min, Adder;
	Re = (a.Re > 0) ? a.Re : (-1 * a.Re);
	Im = (a.Im > 0) ? a.Im : (-1 * a.Im);
	Max = (Re > Im) ? Re : Im;
	Min = (Re < Im) ? Re : Im;
	Adder = (Min >> 1) - (Max >> 3);
	return (Adder < 0) ? Max : (Max + Adder);
}
#endif