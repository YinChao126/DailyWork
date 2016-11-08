#include "SyncDetector.h"
#include "UniversalFifo.h"

void SyncInit()
{
	LossLockCnt = 0;
	CenterOffset = 0;
	CenterPoint = 0;
	NearCnt = 0;
	LastPoint = 0;
	IsLock = FALSE;
	ThisPoint = 0;
	LockValue = 0;
	CirFlag = 0;
	FsmState = STATE_IDLE;
	CorrValueAbs = 0;
	MaxCorrValueSearchTmp = -10000;
	MaxCorrValueSearchRecord = -10000;
	CorrTh = DELAY_CORR_TH;
	SyncFound = 0;
	BuffFillOneTime = 0;
	CatchPt = 0;
	NumInFrame = 0;
	OfdmCnt = 0;
	SampleOffset = 0;
	SampleOffsetRecord = 0;
	OverPoint = 0; //同步后剩余的数据量
	OutEn = Disable;
	CorrCnt = 0;
	SyncFlag = NOT_DETECTED;
	AutoCorrTh = 0;
	AutoCorrMean = 0;
	AutoCorrFifoFull = 0;
	AutoCorrFifoCnt = 0;
	AutoCorrMax = 0;
	LockPtHold = 0;
	StartPt = 0;
	BuffPoint = 0;
	DataInLen = FRAME_HEAD_LEN;
	CorrEdSync = 0;
	TypeDetCnt = 0;
	SyncTypePt = 0;
	k1 = 0;
	AssignComplex(&PdTmpSync, 0, 0);
	AssignComplex(&TypeDetCorrValue, 0, 0);
	AssignComplex(&AutoCorrMaxComplex, 0, 0);


	memset(AutoCorrFifo, 0x00, 4 * sizeof(INT32));
	memset(SpathBuff, 0x00, 2048 * sizeof(t_Complex));
	memset(pcDataOutBuf, 0x00, 2048 * sizeof(t_Complex));
	memset(DataCatch, 0x00, 512 * sizeof(t_Complex));
	memset(SuperSubSyncHeaderDetFifo, 0x00, 513 * sizeof(t_Complex));
	memset(OutBuf, 0x00, sizeof(OutBuf));

	PowerIn = 0;
	Agc2Count = 0;
	Agc2Gain = AGC2_INI_GAIN;
	Agc2Lock = 0;
}

void SyncRun(t_Complex *DataIn, struct sLinkPara *RxLinkPara) //已核对，未验证
{
	INT32 i,j,k;
	INT64 EdSync, TmpGammaDvec, TmpGamSvec;
	INT32 DiffPoint;
	INT32 Length = DataInLen;

	for (i = 0; i < Length; i++)
	{
		Agc2(DataIn[i]);
		NumInFrame++;
		AssignComplex(&(SpathBuff[BuffPoint]), Agc2Out.Re, Agc2Out.Im);
		AssignComplex(&(DataCatch[CatchPt]), Agc2Out.Re, Agc2Out.Im);

		if (CorrCnt == SYNC_CORR_LEN)
		{
			BuffFillOneTime = 1;
		}

		StartPt = BuffPoint + 385;
		LimitValue(StartPt, 2047);

		if (SyncFound || (NumInFrame > (80580 + MAX_OFFSET - 1)))
		{
			CorrTh = MaxCorrValueSearchRecord >> 2;
		}

		if ((NumInFrame == 0 ) ||
			(NumInFrame == 80580 + MAX_OFFSET) ||
			(NumInFrame == 2 * (80580 + MAX_OFFSET)) ||
			(NumInFrame == 2 * (80580 + MAX_OFFSET)) ||
			(NumInFrame == 2 * (80580 + MAX_OFFSET)) ||
			(NumInFrame == 2 * (80580 + MAX_OFFSET)))
		{
			MaxCorrValueSearchRecord = MaxCorrValueSearchTmp;
			MaxCorrValueSearchTmp = -1000;
		}
		else
		{
			if (CorrValueAbs > MaxCorrValueSearchTmp)
			{
				MaxCorrValueSearchTmp = CorrValueAbs;
			}
		}

		if (FsmState == STATE_IDLE)
		{
			RxLinkPara->Ready = FALSE;
		}

		switch (FsmState)
		{
		case STATE_IDLE:
			if (CirFlag)
			{
				memset(SpathBuff, 0, 2048 * sizeof(t_Complex));
				BuffPoint = -1;
				CorrEdSync = 0;
				AssignComplex(&PdTmpSync, 0, 0);

				for (j = 0; j < 128; j++)
				{
					CorrCnt++;
					BuffPoint++;
					StartPt = BuffPoint + 385;
					k = CatchPt + 385 + j;
					LimitValue(k, 511);
					AssignComplex(&SpathBuff[BuffPoint], DataCatch[k].Re, DataCatch[k].Im);
					SyncAutoCorrDelay(&EdSync, &TmpGammaDvec);
				}
				CirFlag = 0;
			}
			else
			{
				CorrCnt++;
				SyncAutoCorrDelay(&EdSync, &TmpGammaDvec);
			}

			if (BuffFillOneTime)
			{
				CorrValueAbs = (INT32)(TmpGammaDvec * SYNC_CORR_LEN / EdSync);
				if (CorrTh < DELAY_CORR_TH)
				{
					CorrTh = DELAY_CORR_TH;
				}
				if (CorrValueAbs >= CorrTh)
				{
					LockPtHold = StartPt;
					LockValue = CorrValueAbs;
					SyncFound = TRUE;
					SampleOffsetRecord = 0;
					ThisPoint = NumInFrame;
					OutEn = Disable;
					FsmState = STATE_SYNC_FOUND;
				}
			}
			DataIn = 256;
			break;
		case STATE_SYNC_FOUND:
			SampleOffset++;
			SyncAutoCorrDelay(&EdSync, &TmpGammaDvec);
			CorrValueAbs = (INT32)(TmpGammaDvec * SYNC_CORR_LEN / EdSync);
			if (CorrValueAbs >= LockValue)
			{
				LockPtHold = StartPt;
				LockValue = CorrValueAbs;
				SampleOffsetRecord = SampleOffset;
				ThisPoint = NumInFrame;
			}
			if (SampleOffset == DELAY_CORR_RANGE) //256个数据内找最大值，为同步头位置
			{
				MaxCorrValueSearchRecord = LockValue;
				IsLock = TRUE;
				MaxCorrValueSearchTmp = 0;
				DiffPoint = ThisPoint - LastPoint;
				if (DiffPoint < 0)
				{
					DiffPoint = -DiffPoint;
				}
				LastPoint = ThisPoint;

				SyncHeaderOut();

				if (DiffPoint < DELAY_CORR_RANGE / 2)
				{
					NearCnt++;
				}
				else
				{
					NearCnt = 0;
				}
				FsmState = STATE_OFDM;
			}
			DataInLen = 256;
			break;
		case STATE_LOCK:
			if (CirFlag)
			{
				memset(SpathBuff, 0x00, 2048 * sizeof(t_Complex));
				OverPoint = 128;
				BuffPoint = -1;
				CorrEdSync = 0;
				AssignComplex(&PdTmpSync, 0, 0);
				for (j = 0; j < OverPoint; j++)
				{
					CorrCnt++;
					BuffPoint++;
					StartPt = BuffPoint + 385;
					k = CatchPt + 385 + j;
					LimitValue(k, 511);

					AssignComplex(&SpathBuff[BuffPoint], DataCatch[k].Re, DataCatch[k].Im);
					SyncAutoCorrDelay(&EdSync, &TmpGammaDvec);
				}
				CirFlag = FALSE;
				LockValue = -1000;
			}
			else
			{
				CorrCnt++;
				SyncAutoCorrDelay(&EdSync, &TmpGammaDvec);
			}

			if (BuffFillOneTime) //1664 data
			{
				SyncFound = TRUE;
			}

			if (NumInFrame >= LastPoint - LOCK_SEEK_WINDOWS / 2)
			{
				CorrValueAbs = (INT32)(TmpGammaDvec * SYNC_CORR_LEN / EdSync);
				SyncAutoCorrCenter(&TmpGamSvec); //开启中心对称匹配
				CorrValueAbs = (INT32)(TmpGamSvec * CorrValueAbs * SYNC_CORR_LEN / EdSync);
				SampleOffset++;
				if (CorrTh < CENTER_CORR_TH)
				{
					CorrTh = CENTER_CORR_TH;
				}
				if (CorrValueAbs >= LockValue)
				{
					LockPtHold = StartPt;
					LockValue = CorrValueAbs;
					SampleOffsetRecord = SampleOffset;
					ThisPoint = NumInFrame;
				}
				if (NumInFrame == LastPoint)
				{
					CenterPoint = StartPt;
					CenterOffset = SampleOffset;
				}
				if (SampleOffset = LOCK_SEEK_WINDOWS)
				{
					if (LockValue < CorrTh)
					{
						LockPtHold = CenterPoint;
						SampleOffsetRecord = CenterOffset;
						LossLockCnt++;
					}
					else
					{
						LastPoint = ThisPoint;
						LossLockCnt = 0;
					}
					MaxCorrValueSearchRecord = LockValue;
					IsLock = TRUE;
					MaxCorrValueSearchTmp = 0;
					OutEn = Enable;

					SyncHeaderOut();		//成功完成帧头检测
					FsmState = STATE_OFDM;  //完成所有同步工作，下一次写入数据
				}
				DataInLen = 256;
			}
			break;
		case STATE_OFDM:
			if (CorrCnt < DATA_POINT_NUM)
			{
				if (OfdmCnt == OFDM_SYM_LEN)
				{
					if (OutEn == Enable)
					{
						while (0 != WriteBlockFifo((FIFO_TYPE *)OutBuf, (FIFO_TYPE *)pcDataOutBuf, OFDM_SYM_LEN));
						{
							// write failure, delay
							// error handle!
						}
					}
					OfdmCnt = 0;
				}
				AssignComplex(&(pcDataOutBuf[OfdmCnt]), Agc2Out.Re, Agc2Out.Im);
				CorrCnt++;
				OfdmCnt++;
			}
			else if (CorrCnt == DATA_POINT_NUM) //最后一个数据
			{
				AssignComplex(&(pcDataOutBuf[OfdmCnt]), Agc2Out.Re, Agc2Out.Im);
				if (OutEn == TRUE)
				{
					while (0 != WriteBlockFifo((FIFO_TYPE *)OutBuf, (FIFO_TYPE *)pcDataOutBuf, OFDM_SYM_LEN));
					{
						// write failure, delay
						// error handle!
					}
				}
				FsmState = STATE_FRAME_END;
			}
			else
			{
#ifdef DEBUG
				printf("unknown error\n");
#endif // DEBUG
			}
			DataInLen = 1024;
			//写入数据块
			break;
		case STATE_FRAME_END:
			IsLock = FALSE;
			SyncFlag = 0;
			AutoCorrMax = 0;
			CirFlag = 1;
			CorrCnt = 0;
			BuffFillOneTime = 0;
			SyncFound = FALSE;
			SampleOffset = 0;
			DataInLen = 256;
			if (LossLockCnt == LOSS_LOCK_THRESHOLD)
			{
				NearCnt = 0;
				LossLockCnt = 0;
				FsmState = STATE_IDLE;
			}
			else if (NearCnt == LOCK_THRESHOLD)
			{
				FsmState = STATE_LOCK;
			}
			else
			{
				FsmState = STATE_IDLE;
			}
			break;
		default:
#ifdef DEBUG
			printf("Error State\n");
#endif
			break;
		}

		BuffPoint++;
		LimitValue(BuffPoint, 2047);

		CatchPt++;
		LimitValue(CatchPt, 511);

	}
}

static void SyncAutoCorrDelay(INT64 *pEdSync, INT64 *pGammaDvec) //已核对，未验证
{
	INT64 ShiftInPower = 0;
	INT64 ShiftOutPower = 0;
	t_Complex MinusResult, PlusResult;
	INT32 p0, p1, p2, p3, p4, p5, p6;
	t_Complex FirstCmp, LastCmp;
	t_Complex tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
	t_Complex CalcTmp1, CalcTmp2;

	p0 = StartPt + 2047;
	if (p0 > 2047)
	{
		p0 -= 2048;
	}
	AssignComplex(&FirstCmp, SpathBuff[p0].Re, SpathBuff[p0].Im);
	AssignComplex(&LastCmp, SpathBuff[BuffPoint].Re, SpathBuff[BuffPoint].Im);
	ShiftOutPower = NormComplex(FirstCmp);
	ShiftInPower = NormComplex(LastCmp);

	p1 = p0 + 576;
	p2 = p0 + 832;
	p3 = p0 + 1152;
	p4 = p0 + 512;
	p5 = p0 + 1088;
	p6 = p0 + 1408;

	if (p1 > 2047)
	{
		p1 -= 2048;
	}
	if (p2 > 2047)
	{
		p2 -= 2048;
	}
	if (p3 > 2047)
	{
		p3 -= 2048;
	}
	if (p4 > 2047)
	{
		p4 -= 2048;
	}
	if (p5 > 2047)
	{
		p5 -= 2048;
	}
	if (p6 > 2047)
	{
		p6 -= 2048;
	}


	AssignComplex(&tmp1, SpathBuff[p1].Re, SpathBuff[p1].Im);
	AssignComplex(&tmp2, SpathBuff[p2].Re, SpathBuff[p2].Im);
	AssignComplex(&tmp3, SpathBuff[p3].Re, SpathBuff[p3].Im);
	AssignComplex(&tmp4, SpathBuff[p4].Re, SpathBuff[p4].Im);
	AssignComplex(&tmp5, SpathBuff[p5].Re, SpathBuff[p5].Im);
	AssignComplex(&tmp6, SpathBuff[p6].Re, SpathBuff[p6].Im);

	CalcTmp1 = MultiComplex(ConjComplex(FirstCmp), tmp1);
	CalcTmp2 = MultiComplex(ConjComplex(tmp2), tmp3);
	MinusResult = AddComplex(CalcTmp1, CalcTmp2);

	CalcTmp1 = MultiComplex(ConjComplex(tmp4), tmp5);
	CalcTmp2 = MultiComplex(ConjComplex(tmp5), tmp6);
	PlusResult = AddComplex(CalcTmp1, CalcTmp2);

	CorrEdSync = CorrEdSync - ShiftOutPower + ShiftInPower;
	PdTmpSync = SubComplex(PdTmpSync, MinusResult);
	PdTmpSync = AddComplex(PdTmpSync, PlusResult);

	*pGammaDvec = ModComplex(PdTmpSync);
	*pEdSync = CorrEdSync;
}

static void SyncAutoCorrCenter(INT64 *pGammaDvec) //已核对，未校正
{
	t_Complex CenterSymmCorr00, CenterSymmCorr01, TmpCorr, AddTmp;
	t_Complex tmp1, tmp2, tmp3, tmp4;
	INT32 Point;
	INT32 p0, p1, p2, p3;
	AssignComplex(&CenterSymmCorr00, 0, 0);
	for (Point = 0; Point < 512; Point++)
	{
		p0 = StartPt + Point;
		p1 = StartPt + 1087 - Point;
		if (p0 > 2047)
		{
			p0 -= 2048;
		}
		if (p1 > 2047)
		{
			p1 -= 2048;
		}
		AssignComplex(&tmp1, SpathBuff[p0].Re, SpathBuff[p0].Im);
		AssignComplex(&tmp2, SpathBuff[p1].Re, SpathBuff[p1].Im);
		TmpCorr = MultiComplex(tmp1, tmp2);
		CenterSymmCorr00 = AddComplex(CenterSymmCorr00, TmpCorr);
	}

	AssignComplex(&CenterSymmCorr01, 0, 0);
	for (Point = 0; Point < 256; Point++)
	{
		p2 = StartPt + 576 + Point;
		p3 = StartPt + 1407 - Point;
		if (p2 > 2047)
		{
			p2 -= 2048;
		}
		if (p3 > 2047)
		{
			p3 -= 2048;
		}
		AssignComplex(&tmp3, SpathBuff[p2].Re, SpathBuff[p2].Im);
		AssignComplex(&tmp4, SpathBuff[p3].Re, SpathBuff[p3].Im);
		TmpCorr = MultiComplex(tmp3, tmp4);
		CenterSymmCorr01 = AddComplex(CenterSymmCorr01, TmpCorr);
	}

	AddTmp = AddComplex(CenterSymmCorr00, CenterSymmCorr01);
	*pGammaDvec = ModComplex(AddTmp);
}

static void SyncAutoCorr(INT32 *pAutoCorrValue, t_Complex *pCorrValue) //已核对，未验证
{
	INT32 TmpRe, TmpIm;
	INT32 pt1, pt2;
	t_Complex CorrValueAdd, CorrValueMin;
	t_Complex tmp1, tmp2, tmp3, tmp4;
	t_Complex temp;

	TypeDetCnt++;
	pt1 = SyncTypePt + 255;
	LimitValue(pt1, 512);
	pt2 = pt1 + 1;
	LimitValue(pt2, 512);
	AssignComplex(&tmp1, SuperSubSyncHeaderDetFifo[pt2].Re, SuperSubSyncHeaderDetFifo[pt2].Im);
	AssignComplex(&tmp2, SuperSubSyncHeaderDetFifo[k1].Re, SuperSubSyncHeaderDetFifo[k1].Im);
	AssignComplex(&tmp3, SuperSubSyncHeaderDetFifo[SyncTypePt].Re, SuperSubSyncHeaderDetFifo[SyncTypePt].Im);
	AssignComplex(&tmp4, SuperSubSyncHeaderDetFifo[pt1].Re, SuperSubSyncHeaderDetFifo[pt1].Im);

	CorrValueAdd = MultiComplex(tmp1, ConjComplex(tmp2));
	CorrValueMin = MultiComplex(tmp3, ConjComplex(tmp4));
	temp = SubComplex(CorrValueAdd, CorrValueMin);
	TypeDetCorrValue = AddComplex(TypeDetCorrValue, temp);

	if (TypeDetCnt >= 512)
	{
		TmpRe = (TypeDetCorrValue.Re < 0) ? -1 : 1;
		TmpIm = (TypeDetCorrValue.Im < 0) ? -1 : 1;
		*pAutoCorrValue = ModComplex(TypeDetCorrValue);
		AssignComplex(pCorrValue, TmpRe, TmpIm); //检查此处是否能够正确赋值！
	}
	else
	{
		*pAutoCorrValue = 0;
		AssignComplex(pCorrValue, 0, 0);
	}

}

static void SyncTypeDet() //已校对，未验证
{
	INT32 tk = 0,j,t;
	INT32 AutoValue = 0;
	t_Complex TypeCorrValue;
	INT32 TmpRe = 0, TmpIm = 0;


	AssignComplex(&TypeDetCorrValue, 0, 0);
	TypeDetCnt = 0;

	for (j = 0; j < 574; j++)
	{
		tk = LockPtHold + 1090 + j;
		LimitValue(tk, 2047);
		k1 = SyncTypePt + 512;
		LimitValue(k1, 512);

		TmpRe = SpathBuff[tk].Re > 0 ? 1 : -1;
		TmpIm = SpathBuff[tk].Im > 0 ? 1 : -1;
		AssignComplex(&(SuperSubSyncHeaderDetFifo[k1]), TmpRe, TmpIm);
		SyncAutoCorr(&AutoValue, &TypeCorrValue);

		SyncTypePt++;
		LimitValue(SyncTypePt, 512);

		if (j > 510)
		{
			if (AutoValue > AutoCorrMax)
			{
				AutoCorrMax = AutoValue;
				AssignComplex(&AutoCorrMaxComplex, TypeCorrValue.Re, TypeCorrValue.Im);
			}
			if (j == 570)
			{
				AutoCorrFifoCnt++;
				for (t = 0; t < 3; t++)
				{
					AutoCorrFifo[t] = AutoCorrFifo[t + 1];
				}
				AutoCorrFifo[3] = AutoCorrMax;
				if (AutoCorrFifoCnt >= 4)
				{
					AutoCorrFifoFull = 1;
				}
				if (AutoCorrFifoFull == 1) //此段可优化
				{
					AutoCorrMean = 0;  //该变量应该可以改写为局部变量，仅仅只是计算了AutoCorrFifo[4]的平均值而已。
					for (t = 0; t < 4; t++)
					{
						AutoCorrMean += AutoCorrFifo[t];
					}
					AutoCorrMean >>= 2;
					AutoCorrTh = AutoCorrMean + ((AutoCorrMean * 51) >> 8);
				}
				else
				{
					AutoCorrTh = 163;
				}

				if (AutoCorrMax > AutoCorrTh && AutoCorrMaxComplex.Re == -1)
				{
					SyncFlag = SUPER_FRAME;
				}
				else
				{
					SyncFlag = SUB_FRAME;
				}
				AutoCorrMax = 0;
				AutoValue = 0;
			}
		}


	}
}

static void SyncHeaderOut() //已校对，未验证
{
	INT32 k, PtCnt;
	SyncTypeDet(); //帧头类型检测

	if (SyncFlag == SUPER_FRAME)
	{
		memset(pcDataOutBuf, INT32_MAX, 4 * sizeof(t_Complex));
#ifdef DEBUG
		printf("找到超帧头\n");
#endif // DEBUG
	}
	else if (SyncFlag == SUB_FRAME)
	{
		memset(pcDataOutBuf, INT32_MIN, 4 * sizeof(t_Complex));
#ifdef DEBUG
		printf("找到子帧头\n");
#endif // DEBUG
	}
	else
	{
#ifdef DEBUG
		printf("出错，未找到帧头\n");
#endif // DEBUG
	}

	for (PtCnt = 0; PtCnt < PNSYNC_LEN; PtCnt++)
	{
		k = LockPtHold + 1984 + PtCnt;
		if (k > 4095)	//此段是何意？？？
		{
			k -= 4096;
		}
		else if (k > 2047)
		{
			k -= 2048;
		}
		AssignComplex(&(pcDataOutBuf[PtCnt + 4]), SpathBuff[k].Re, SpathBuff[k].Im);
	}

	if (OutEn == Enable)
	{
		while (0 != WriteBlockFifo((FIFO_TYPE *)OutBuf, (FIFO_TYPE *)pcDataOutBuf, FRM_HEAD_LEN));
		{
			// write failure, delay
			// error handle!
		}
	}
	CorrCnt = 0;
	OverPoint = SampleOffset - SampleOffsetRecord;
	OfdmCnt = 0;
	NumInFrame = 0;

	for (PtCnt = 0; PtCnt < OverPoint; PtCnt++)
	{
		k = LockPtHold + 1664 + PtCnt;
		LimitValue(k, 2047);

		pcDataOutBuf[CorrCnt] = SpathBuff[k];
		CorrCnt++;
		OfdmCnt++;
		NumInFrame++;

		if (OfdmCnt == OFDM_SYM_LEN)
		{
			if (OutEn == Enable)
			{
				while (0 != WriteBlockFifo((FIFO_TYPE *)OutBuf, (FIFO_TYPE *)pcDataOutBuf, OFDM_SYM_LEN));
				{
					// write failure, delay
					// error handle!
				}
			}
			OfdmCnt = 0;
		}
	}
}


INT32 DataRequest()
{
	return DataInLen;
}

static void Agc2(t_Complex DataIn)	//已校对、未验证
{
	INT32 AvePowerIndex = 0;
	INT32 CurGain = 0;

	if (Agc2Lock)
	{
		Agc2Out.Re = (DataIn.Re * Agc2Gain) >> 10;
		Agc2Out.Im = (DataIn.Im * Agc2Gain) >> 10;
	}
	else
	{
		Agc2Out.Re = 0;
		Agc2Out.Im = 0;
	}

	PowerIn += (DataIn.Re * DataIn.Re + DataIn.Im * DataIn.Im);

	if (++Agc2Count == AGC2_WIDTH)
	{
		Agc2Count = 0;
		PowerIn >>= 8;
		AvePowerIndex = PowerIn >> 16; //???
		switch (AvePowerIndex)
		{
		case 0:
			CurGain = 4463;
				break;
		case 1:
			CurGain = 4463;
				break;
		case 2:
			CurGain = 3156;
				break;
		case 3:
			CurGain = 2577;
				break;
		case 4:
			CurGain = 2231;
				break;
		case 5:
			CurGain = 1996;
				break;
		case 6:
			CurGain = 1822;
				break;
		case 7:
			CurGain = 1687;
				break;
		case 8:
			CurGain = 1578;
				break;
		case 9:
			CurGain = 1487;
				break;
		case 10:
			CurGain = 1411;
				break;
		case 11:
			CurGain = 1345;
				break;
		case 12:
			CurGain = 1288;
				break;
		case 13:
			CurGain = 1237;
				break;
		case 14:
			CurGain = 1192;
				break;
		case 15:
			CurGain = 1152;
				break;
		case 16:
			CurGain = 1115;
				break;
		case 17:
			CurGain = 1082;
				break;
		case 18:
			CurGain = 1052;
				break;
		case 19:
			CurGain = 1024;
				break;
		case 20:
			CurGain = 998;
				break;
		case 21:
			CurGain = 974;
				break;
		case 22:
			CurGain = 951;
				break;
		case 23:
			CurGain = 930;
				break;
		case 24:
			CurGain = 911;
				break;
		case 25:
			CurGain = 892;
				break;
		case 26:
			CurGain = 875;
				break;
		case 27:
			CurGain = 859;
				break;
		case 28:
			CurGain = 843;
				break;
		case 29:
			CurGain = 828;
				break;
		case 30:
			CurGain = 814;
				break;
		case 31:
			CurGain = 801;
				break;
		default:
			CurGain = 789;
				break;
		}
		if (Agc2Lock == UNLOCK)
		{
			Agc2Lock = LOCK;
			Agc2Gain = CurGain;
		}
		else
		{
			Agc2Gain += ((CurGain - Agc2Gain) >> AGC2_STEP);
		}
		PowerIn = 0;
	}
}