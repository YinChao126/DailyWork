#include "UniversalFifo.h"
#include <stdio.h>
#include <memory.h>

FIFO_TYPE BuffLen = FIFO_MAX_SZIE; 
FIFO_TYPE DataLen; // current data number in FIFO.
FIFO_TYPE TotalRead; 
FIFO_TYPE TotalWrite; 

static long front = 0;
static long tail = 0;

void InitFifo(FIFO_TYPE *Buff, long BuffSize)
{
	if (BuffSize > FIFO_MAX_SZIE)
	{
		printf("maybe you should change the FIFO_MAX_SZIE bigger!\n");
	}

	front = 0;
	tail = 0;
	DataLen = 0;
	TotalRead = 0;
	TotalWrite = 0;
}

void ResetFifo()
{
	front = 0;
	tail = 0;
	DataLen = 0;
}

char  InQueue(FIFO_TYPE *Queue, FIFO_TYPE *Value) 
{
	if (((tail + 1) % FIFO_MAX_SZIE) == front)
	{
		printf("FIFO is full, cannot write any more\n");
		return ERROR;
	}
	Queue[tail] = Value[0];
	tail = (tail + 1) % FIFO_MAX_SZIE;
	TotalWrite++;
#ifdef COMPLEX_MODE
	Queue[tail] = Value[1];
	tail = (tail + 1) % FIFO_MAX_SZIE;
#endif
	return 0;
}

char OutQueue(FIFO_TYPE *Queue, FIFO_TYPE *OutValue)
{
	if (front == tail)
	{
		printf("FIFO is empty, read nothing!\n");
		return ERROR;
	}
	*OutValue = Queue[front];
	front = (front + 1) % FIFO_MAX_SZIE;
	TotalRead++;
#ifdef COMPLEX_MODE
	*(OutValue + 1) = Queue[front];
	front = (front + 1) % FIFO_MAX_SZIE;
#endif
	return 0;
}

char WriteBlockFifo(FIFO_TYPE *Queue, FIFO_TYPE *Value, long DataSize)
{
	long Cnt = 0;
	long pt = 0;
	long ErrStore = tail;
#ifdef COMPLEX_MODE
	DataSize *= 2;
#endif
	for (Cnt = 0; Cnt < DataSize; Cnt++)
	{
		if (((tail + 1) % FIFO_MAX_SZIE) == front)
		{
			printf("FIFO is full\n");
			tail = ErrStore;//recovery FIFO to the past, equal to write nothing.
			return ERROR; 
		}
		Queue[tail] = *(Value + pt);
		tail = (tail + 1) % FIFO_MAX_SZIE;
		TotalWrite++;
		pt++;
#ifdef COMPLEX_MODE
		Queue[tail] = *(Value + pt);
		tail = (tail + 1) % FIFO_MAX_SZIE;
		pt++;
		Cnt++;
#endif
	}
	return 0;
}

char ReadBlockFifo(FIFO_TYPE *Queue, FIFO_TYPE *OutValue, long DataSize)
{
	long Cnt = 0;
	long pt = 0;
	long ErrStore = 0;

#ifdef COMPLEX_MODE
	DataSize *= 2;
#endif
	ErrStore = front;
	for (Cnt = 0; Cnt < DataSize; Cnt++)
	{
		if (front == tail)
		{
			printf("FIFO is empty, read nothing!\n");
			front = ErrStore; // recovery to the past
			return ERROR;
		}
		*(OutValue + pt) = Queue[front];
		front = (front + 1) % FIFO_MAX_SZIE;
		pt++;
		TotalRead++;
#ifdef COMPLEX_MODE
		*(OutValue + pt) = Queue[front];
		front = (front + 1) % FIFO_MAX_SZIE;
		pt++;
		Cnt++;
#endif
	}
	return 0;
}

FIFO_TYPE GetDataSize(void)
{
	long tmp;
	tmp = tail - front;
#ifdef COMPLEX_MODE
	tmp >>= 1;
#endif
	DataLen = tmp;
	return tmp;
}

/* make sure the FIFO is empty , 0->not empty	1->is empty*/
char IsEmpty(void)
{
	return (front == tail) ? 1 : 0;
}


/* make sure the FIFO is full. 0->not full	1->is full*/
/* 对于队列已满的检查有两种实现方式：
**	1.引用一个标志位来标示rear是否超过front一圈
** 	2.保证队列最后一个位置为空，通过条件来判断： (rear + 1)%MAXSIZE == front */
char IsFull(void)			// 
{
#ifdef COMPLEX_MODE
	return ((tail + 2) % FIFO_MAX_SZIE) == front ? 1 : 0;
#else
	return ((tail + 1) % FIFO_MAX_SZIE) == front ? 1 : 0;
#endif

}