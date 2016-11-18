#include <stdio.h>
#include <stdlib.h>
#include "SyncDetector.h"
#include <time.h>
FILE *WriteLog;
clock_t start, end, elapse;
clock_t TotalTime = 0;

#if 0
int main()
{
	struct sLinkPara NoUse;

	//int a = 101;
	//int b = 100;

	int i = 0, j = 0;
	FILE *in;
	int RawData[2048] = { 0 };
	int RawDataNum = 0; //总共有多少个数据？（如果是复数形式，必须除2）
	int *DataBuff = (int *)malloc(10010000 * sizeof(int));
	int *td = DataBuff;
	int DataHandleCnt = 0; //已处理的数据统计
	int RunNum = 0; //单次运行请求的数据
	t_Complex InputData[2048] = { 0 }; //最终输入Run的原始数据
#if 0
	FILE *out;
#endif
	//1. 读文件，数据存入堆内存。
	if ((in = fopen("D://Data//RawData_0.txt", "rb")) == NULL)
	{
		printf("cannot open this file!\n");
		system("pause");
		return 0;
	}
	while (!feof(in))
	{
		fscanf(in, "%d", DataBuff);
		DataBuff++;
		RawDataNum++;
	}
	fclose(in);
	DataBuff = td; //返回最开始的地方

#if 0 //检测输出是不是和输入一致——经过严格检测，是一样的。DataBuff成功读取了文件数据
	out = fopen("D://Data//OutData.txt", "wb");
	for (i = 0; i < RawDataNum; i++)
	{
		if (i % 2 == 0)
		{
			fprintf(out, "%d\t", DataBuff[i]);
		}
		else
		{
			fprintf(out, "%d\n", DataBuff[i]);
		}
	}
	fclose(out);
#endif

#if 1
	WriteLog = fopen("D://Data//result.txt", "wb");
#endif
	//2.开始初始化程序并启动算法
	SyncInit();
	RawDataNum >>= 1; //复数个数要除2
	while (DataHandleCnt < RawDataNum - 2000) // 留2000的余量，否则后面的数是乱码
	{
		RunNum = DataRequest();
		DataHandleCnt += RunNum;
		for (i = 0; i < RunNum; i++, j += 2)
		{
			InputData[i].Re = DataBuff[j];
			InputData[i].Im = DataBuff[j + 1];
		}
		SyncRun(InputData, &NoUse);
	}

	fclose(WriteLog);


	DataBuff = td;
	free(DataBuff);
	DataBuff = NULL;

	return 0;
}
#endif

int RawData[4096] = { 0 };
t_Complex InputData[2048] = { 0 }; //最终输入Run的原始数据
int main()
{
	struct sLinkPara NoUse;

	int i = 0, j = 0;
	FILE *in,*out;

	int RawDataNum = 0; //总共有多少个数据？（如果是复数形式，必须除2）
	int DataHandleCnt = 0; //已处理的数据统计
	int RunNum = 0; //单次运行请求的数据

	int g_cnt = 0;

	start = clock();
	end = clock();
	elapse = end - start;

	//1. 读文件，数据存入堆内存。
	if ((in = fopen("D://Data//RawData_0.txt", "rb")) == NULL)
	{
		printf("cannot open this file!\n");
		system("pause");
		return 0;
	}

	out = fopen("D://Data//OutData.txt", "wb");
	//2.开始初始化程序并启动算法
	SyncInit();

	while (DataHandleCnt < 2500000)
	{
		RunNum = DataRequest();
		for (i = 0; i < (RunNum << 1); i++)
		{
			fscanf(in, "%d", &(RawData[i]));
		}
		DataHandleCnt += (RunNum << 1);
		for (i = 0,j = 0; i < RunNum; i++, j += 2)
		{
			InputData[i].Re = RawData[j];
			InputData[i].Im = RawData[j + 1];
		}
		start = clock();
		SyncRun(InputData, &NoUse);
		end = clock();
		TotalTime += (end - start - elapse);
#if 0 //检测输出是不是和输入一致——经过严格检测，是一样的。DataBuff成功读取了文件数据

		for (i = 0; i < RunNum; i++)
		{
			fprintf(out, "%d\t", InputData[i].Re);
			fprintf(out, "%d\n", InputData[i].Im);
		}
#endif
	}
	printf("TotalTime = %ld\n",TotalTime);
	fclose(in);
//	fclose(out);


#if 0 //检测输出是不是和输入一致——经过严格检测，是一样的。DataBuff成功读取了文件数据
	out = fopen("D://Data//OutData.txt", "wb");
	for (i = 0; i < RawDataNum; i++)
	{
		if (i % 2 == 0)
		{
			fprintf(out, "%d\t", DataBuff[i]);
		}
		else
		{
			fprintf(out, "%d\n", DataBuff[i]);
		}
	}
	fclose(out);
#endif



	return 0;
}