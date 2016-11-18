/************************************************************************/
/* author:YinChao
** date:2016-11-7
** Describe:
** Note:								*/
/************************************************************************/
#ifndef __UNIVERSAL_FIFO_H__
#define __UNIVERSAL_FIFO_H__

/***************************user's area**********************************/
#define COMPLEX_MODE //开启复数形式

typedef int FIFO_TYPE; //定义FIFO的数据类型（根据实际情况更改）
#define FIFO_MAX_SZIE 2050 //定义fifo最大容量（需修改），注意，可用数据为MAX-1（1个作为队列状态判断）

/**********************donnot change a word!*****************************/
enum 
{
	ERROR = -1,
	MEDIAN, // no use
	OK
};
extern FIFO_TYPE DataLen;
extern FIFO_TYPE TotalRead;
extern FIFO_TYPE TotalWrite;

// basic function
void InitFifo(FIFO_TYPE *Buff, long DataSize); 
void ResetFifo();
char InQueue(FIFO_TYPE *Queue, FIFO_TYPE *Value); // write a data into the FIFO
char OutQueue(FIFO_TYPE *Queue, FIFO_TYPE *OutValue); // read a data from the FIFO
char WriteBlockFifo(FIFO_TYPE *Queue, FIFO_TYPE *Value, long DataSize);
char ReadBlockFifo(FIFO_TYPE *Queue, FIFO_TYPE *OutValue, long DataSize);

// auxiliary function
FIFO_TYPE GetDataSize(void);
char IsEmpty(void);
char IsFull(void);
/************************************************************************/

#endif