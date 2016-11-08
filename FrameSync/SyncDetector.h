/*************************************************************************************/
/* Author:		YinChao
/* Date:		2016.11.3
/* Description: Just for a test
/*************************************************************************************/
#ifndef __SYNC_DETECTOR_H
#define __SYNC_DETECTOR_H

#include <stdio.h>
#include <memory.h>
#include "Complex.h"
#include "VariableDefine.h"

/*********************************Debug Switch************************************************/
#define DEBUG
/*********************************struct define************************************************/
#define AGC2_INI_GAIN		1
#define AGC2_WIDTH			256
#define AGC2_STEP			3

#define TINY				1e-10
#define MAX_OFFSET			256
#define DATA_POINT_NUM		78848
#define SYNC_CORR_LEN		1664


#define DELAY_CORR_TH		240
#define DELAY_CORR_RANGE	512
#define LOCK_THRESHOLD		4
#define LOSS_LOCK_THRESHOLD	16
#define LOCK_SEEK_WINDOWS	256
#define CENTER_CORR_TH		70778

enum
{
	UNLOCK = 0,
	LOCK
};
enum
{
	STATE_IDLE = 0,
	STATE_SYNC_FOUND,
	STATE_LOCK,
	STATE_OFDM,
	STATE_FRAME_END
};

enum FrameType
{
	NOT_DETECTED = 0,
	SUPER_FRAME,
	SUB_FRAME
};

enum 
{
	Disable = 0,
	Enable
};
enum 
{
	FALSE = 0,
	TRUE
};
/**************************此段为CDRdio.h中的定义*******************************************/
#define INT32_MAX 2147483647i32
#define INT32_MIN (-2147483647i32 -1)

#define FRM_HEAD_LEN 1732
#define MAX_SERVICE_NUM 8
#define FRAME_HEAD_LEN 1732 //帧头长度
#define PNSYNC_LEN 1728
#define SYM_FFT_LEN 128
#define OFDM_SYM_LEN (SYM_FFT_LEN * 2)
struct sServiceCode
{
	INT32 FormCode;
	INT32 LdpcNum;
	INT32 LdpcRate;
	INT32 IntvSize;
	INT32 QamType;
};
struct sLinkPara
{
	UINT8 Ready;
	INT32 BerTestFlag;
	INT32 SubFrmNum;
	INT32 ToneIndexL;
	INT32 ToneIndexR;
	struct sServiceCode ServiceInf[MAX_SERVICE_NUM];
};



/************************************function define*********************************************/
//public
#define LimitValue(value,max) (value = (value > max ? (value - max - 1) : value)) //限幅函数的简写
void SyncInit();
void SyncRun(t_Complex *DataIn, struct sLinkPara *RxLinkPara);
INT32 DataRequest();
//private
static void SyncAutoCorr(INT32 *pAutoCorrValue, t_Complex *pCorrValue);
static void SyncAutoCorrDelay(INT64 *pEdSync, INT64 *pGammaDvec);
static void SyncAutoCorrCenter(INT64 *pGammaDvec);
static void SyncTypeDet();
static void SyncHeaderOut();
static void Agc2(t_Complex cDataIn);


/********************************variable define******************************************/

INT32 OfdmCnt;
INT32 SampleOffset;
INT32 SampleOffsetRecord;
INT32 OverPoint; //同步后剩余的数据量
INT32 CorrCnt;
INT8 SyncFlag;
INT8 OutEn;
INT32 AutoCorrTh;
INT32 AutoCorrMean;
INT32 AutoCorrFifoFull;
INT32 AutoCorrFifo[4];
INT32 AutoCorrFifoCnt;
INT32 AutoCorrMax;
INT32 LockPtHold;
INT32 Agc2Gain;
INT8 Agc2Lock;
INT64 PowerIn;
INT32 DataInLen; //向设备请求的数据量
INT32 StartPt; //当前的索引起始位置
INT32 MaxCorrValueSearchRecord;
INT32 MaxCorrValueSearchTmp;
INT32 BuffPoint; //不知道有啥用
INT64 CorrEdSync; //不知道有啥用
INT32 CatchPt;
INT32 BuffFillOneTime;
INT32 Agc2Count; //用于AGC2计数
INT32 TinyCount;
INT32 TypeDetCnt; 
INT32 SyncTypePt;
INT32 k1;
INT32 SyncFound;
INT32 CorrTh;
INT32 CorrValueAbs;
INT8 FsmState;
INT8 CirFlag;
INT32 LockValue;
INT8 IsLock;

INT32 NearCnt;		//帧位置相近计数
INT32 ThisPoint;	//本次最大的点位置
INT32 LastPoint;	//上次锁定的点位置
INT32 NumInFrame;	//帧内数据计数
INT32 CenterPoint;	//与上一次点位置相同的点
INT32 CenterOffset; //在上一次点位置时，偏移量
INT32 LossLockCnt;	//失锁检测

t_Complex PdTmpSync; //不知道干啥用
t_Complex Agc2Out;
t_Complex TypeDetCorrValue;
t_Complex AutoCorrMaxComplex;
t_Complex SpathBuff[2048]; //数据缓存，此处以后可以用堆来实现
t_Complex SuperSubSyncHeaderDetFifo[513];
t_Complex pcDataOutBuf[2048];
t_Complex DataCatch[512];

t_Complex OutBuf[2050]; //定义的是65536的长度！！！
#endif