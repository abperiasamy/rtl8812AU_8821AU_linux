//============================================================
// Description:
//
// This file is for RTL8192E Co-exist mechanism
//
// History
// 2012/11/15 Cosa first check in.
//
//============================================================

//============================================================
// include files
//============================================================
#include "Mp_Precomp.h"
#if(BT_30_SUPPORT == 1)
//============================================================
// Global variables, these are static variables
//============================================================
static COEX_DM_8192E_2ANT		GLCoexDm8192e2Ant;
static PCOEX_DM_8192E_2ANT 	pCoexDm=&GLCoexDm8192e2Ant;
static COEX_STA_8192E_2ANT		GLCoexSta8192e2Ant;
static PCOEX_STA_8192E_2ANT	pCoexSta=&GLCoexSta8192e2Ant;

const char *const GLBtInfoSrc8192e2Ant[]= {
	"BT Info[wifi fw]",
	"BT Info[bt rsp]",
	"BT Info[bt auto report]",
};

u4Byte	GLCoexVerDate8192e2Ant=20130912;
u4Byte	GLCoexVer8192e2Ant=0x35;

//============================================================
// local function proto type if needed
//============================================================
//============================================================
// local function start with halbtc8192e2ant_
//============================================================
u1Byte
halbtc8192e2ant_BtRssiState(
    u1Byte			levelNum,
    u1Byte			rssiThresh,
    u1Byte			rssiThresh1
)
{
	s4Byte			btRssi=0;
	u1Byte			btRssiState=pCoexSta->preBtRssiState;

	btRssi = pCoexSta->btRssi;

	if(levelNum == 2) {
		if( (pCoexSta->preBtRssiState == BTC_RSSI_STATE_LOW) ||
		    (pCoexSta->preBtRssiState == BTC_RSSI_STATE_STAY_LOW)) {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi pre state=LOW\n"));
			if(btRssi >= (rssiThresh+BTC_RSSI_COEX_THRESH_TOL_8192E_2ANT)) {
				btRssiState = BTC_RSSI_STATE_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to High\n"));
			} else {
				btRssiState = BTC_RSSI_STATE_STAY_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state stay at Low\n"));
			}
		} else {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi pre state=HIGH\n"));
			if(btRssi < rssiThresh) {
				btRssiState = BTC_RSSI_STATE_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to Low\n"));
			} else {
				btRssiState = BTC_RSSI_STATE_STAY_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state stay at High\n"));
			}
		}
	} else if(levelNum == 3) {
		if(rssiThresh > rssiThresh1) {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi thresh error!!\n"));
			return pCoexSta->preBtRssiState;
		}

		if( (pCoexSta->preBtRssiState == BTC_RSSI_STATE_LOW) ||
		    (pCoexSta->preBtRssiState == BTC_RSSI_STATE_STAY_LOW)) {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi pre state=LOW\n"));
			if(btRssi >= (rssiThresh+BTC_RSSI_COEX_THRESH_TOL_8192E_2ANT)) {
				btRssiState = BTC_RSSI_STATE_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to Medium\n"));
			} else {
				btRssiState = BTC_RSSI_STATE_STAY_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state stay at Low\n"));
			}
		} else if( (pCoexSta->preBtRssiState == BTC_RSSI_STATE_MEDIUM) ||
		           (pCoexSta->preBtRssiState == BTC_RSSI_STATE_STAY_MEDIUM)) {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi pre state=MEDIUM\n"));
			if(btRssi >= (rssiThresh1+BTC_RSSI_COEX_THRESH_TOL_8192E_2ANT)) {
				btRssiState = BTC_RSSI_STATE_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to High\n"));
			} else if(btRssi < rssiThresh) {
				btRssiState = BTC_RSSI_STATE_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to Low\n"));
			} else {
				btRssiState = BTC_RSSI_STATE_STAY_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state stay at Medium\n"));
			}
		} else {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi pre state=HIGH\n"));
			if(btRssi < rssiThresh1) {
				btRssiState = BTC_RSSI_STATE_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state switch to Medium\n"));
			} else {
				btRssiState = BTC_RSSI_STATE_STAY_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_RSSI_STATE, ("[BTCoex], BT Rssi state stay at High\n"));
			}
		}
	}

	pCoexSta->preBtRssiState = btRssiState;

	return btRssiState;
}

u1Byte
halbtc8192e2ant_WifiRssiState(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte			index,
    IN	u1Byte			levelNum,
    IN	u1Byte			rssiThresh,
    IN	u1Byte			rssiThresh1
)
{
	s4Byte			wifiRssi=0;
	u1Byte			wifiRssiState=pCoexSta->preWifiRssiState[index];

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_S4_WIFI_RSSI, &wifiRssi);

	if(levelNum == 2) {
		if( (pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_LOW) ||
		    (pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_STAY_LOW)) {
			if(wifiRssi >= (rssiThresh+BTC_RSSI_COEX_THRESH_TOL_8192E_2ANT)) {
				wifiRssiState = BTC_RSSI_STATE_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to High\n"));
			} else {
				wifiRssiState = BTC_RSSI_STATE_STAY_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state stay at Low\n"));
			}
		} else {
			if(wifiRssi < rssiThresh) {
				wifiRssiState = BTC_RSSI_STATE_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to Low\n"));
			} else {
				wifiRssiState = BTC_RSSI_STATE_STAY_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state stay at High\n"));
			}
		}
	} else if(levelNum == 3) {
		if(rssiThresh > rssiThresh1) {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI thresh error!!\n"));
			return pCoexSta->preWifiRssiState[index];
		}

		if( (pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_LOW) ||
		    (pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_STAY_LOW)) {
			if(wifiRssi >= (rssiThresh+BTC_RSSI_COEX_THRESH_TOL_8192E_2ANT)) {
				wifiRssiState = BTC_RSSI_STATE_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to Medium\n"));
			} else {
				wifiRssiState = BTC_RSSI_STATE_STAY_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state stay at Low\n"));
			}
		} else if( (pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_MEDIUM) ||
		           (pCoexSta->preWifiRssiState[index] == BTC_RSSI_STATE_STAY_MEDIUM)) {
			if(wifiRssi >= (rssiThresh1+BTC_RSSI_COEX_THRESH_TOL_8192E_2ANT)) {
				wifiRssiState = BTC_RSSI_STATE_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to High\n"));
			} else if(wifiRssi < rssiThresh) {
				wifiRssiState = BTC_RSSI_STATE_LOW;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to Low\n"));
			} else {
				wifiRssiState = BTC_RSSI_STATE_STAY_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state stay at Medium\n"));
			}
		} else {
			if(wifiRssi < rssiThresh1) {
				wifiRssiState = BTC_RSSI_STATE_MEDIUM;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state switch to Medium\n"));
			} else {
				wifiRssiState = BTC_RSSI_STATE_STAY_HIGH;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_WIFI_RSSI_STATE, ("[BTCoex], wifi RSSI state stay at High\n"));
			}
		}
	}

	pCoexSta->preWifiRssiState[index] = wifiRssiState;

	return wifiRssiState;
}

VOID
halbtc8192e2ant_MonitorBtEnableDisable(
    IN 	PBTC_COEXIST		pBtCoexist
)
{
	static BOOLEAN	bPreBtDisabled=FALSE;
	static u4Byte	btDisableCnt=0;
	BOOLEAN			bBtActive=TRUE, bBtDisabled=FALSE;

	// This function check if bt is disabled

	if(	pCoexSta->highPriorityTx == 0 &&
	    pCoexSta->highPriorityRx == 0 &&
	    pCoexSta->lowPriorityTx == 0 &&
	    pCoexSta->lowPriorityRx == 0) {
		bBtActive = FALSE;
	}
	if(	pCoexSta->highPriorityTx == 0xffff &&
	    pCoexSta->highPriorityRx == 0xffff &&
	    pCoexSta->lowPriorityTx == 0xffff &&
	    pCoexSta->lowPriorityRx == 0xffff) {
		bBtActive = FALSE;
	}
	if(bBtActive) {
		btDisableCnt = 0;
		bBtDisabled = FALSE;
		pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_BL_BT_DISABLE, &bBtDisabled);
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], BT is enabled !!\n"));
	} else {
		btDisableCnt++;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], bt all counters=0, %d times!!\n",
		          btDisableCnt));
		if(btDisableCnt >= 2) {
			bBtDisabled = TRUE;
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_BL_BT_DISABLE, &bBtDisabled);
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], BT is disabled !!\n"));
		}
	}
	if(bPreBtDisabled != bBtDisabled) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], BT is from %s to %s!!\n",
		          (bPreBtDisabled ? "disabled":"enabled"),
		          (bBtDisabled ? "disabled":"enabled")));
		bPreBtDisabled = bBtDisabled;
		if(!bBtDisabled) {
		} else {
		}
	}
}

u4Byte
halbtc8192e2ant_DecideRaMask(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte				ssType,
    IN	u4Byte				raMaskType
)
{
	u4Byte	disRaMask=0x0;

	switch(raMaskType) {
	case 0: // normal mode
		if(ssType == 2)
			disRaMask = 0x0;			// enable 2ss
		else
			disRaMask = 0xfff00000;		// disable 2ss
		break;
	case 1: // disable cck 1/2
		if(ssType == 2)
			disRaMask = 0x00000003;		// enable 2ss
		else
			disRaMask = 0xfff00003;		// disable 2ss
		break;
	case 2: // disable cck 1/2/5.5, ofdm 6/9/12/18/24, mcs 0/1/2/3/4
		if(ssType == 2)
			disRaMask = 0x0001f1f7;		// enable 2ss
		else
			disRaMask = 0xfff1f1f7;		// disable 2ss
		break;
	default:
		break;
	}

	return disRaMask;
}

VOID
halbtc8192e2ant_UpdateRaMask(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN				bForceExec,
    IN	u4Byte				disRateMask
)
{
	pCoexDm->curRaMask = disRateMask;

	if( bForceExec || (pCoexDm->preRaMask != pCoexDm->curRaMask)) {
		pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_UPDATE_RAMASK, &pCoexDm->curRaMask);
	}
	pCoexDm->preRaMask = pCoexDm->curRaMask;
}

VOID
halbtc8192e2ant_AutoRateFallbackRetry(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN				bForceExec,
    IN	u1Byte				type
)
{
	BOOLEAN	bWifiUnderBMode=FALSE;

	pCoexDm->curArfrType = type;

	if( bForceExec || (pCoexDm->preArfrType != pCoexDm->curArfrType)) {
		switch(pCoexDm->curArfrType) {
		case 0:	// normal mode
			pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x430, pCoexDm->backupArfrCnt1);
			pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x434, pCoexDm->backupArfrCnt2);
			break;
		case 1:
			pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_UNDER_B_MODE, &bWifiUnderBMode);
			if(bWifiUnderBMode) {
				pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x430, 0x0);
				pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x434, 0x01010101);
			} else {
				pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x430, 0x0);
				pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x434, 0x04030201);
			}
			break;
		default:
			break;
		}
	}

	pCoexDm->preArfrType = pCoexDm->curArfrType;
}

VOID
halbtc8192e2ant_RetryLimit(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN				bForceExec,
    IN	u1Byte				type
)
{
	pCoexDm->curRetryLimitType = type;

	if( bForceExec || (pCoexDm->preRetryLimitType != pCoexDm->curRetryLimitType)) {
		switch(pCoexDm->curRetryLimitType) {
		case 0:	// normal mode
			pBtCoexist->fBtcWrite2Byte(pBtCoexist, 0x42a, pCoexDm->backupRetryLimit);
			break;
		case 1:	// retry limit=8
			pBtCoexist->fBtcWrite2Byte(pBtCoexist, 0x42a, 0x0808);
			break;
		default:
			break;
		}
	}

	pCoexDm->preRetryLimitType = pCoexDm->curRetryLimitType;
}

VOID
halbtc8192e2ant_AmpduMaxTime(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN				bForceExec,
    IN	u1Byte				type
)
{
	pCoexDm->curAmpduTimeType = type;

	if( bForceExec || (pCoexDm->preAmpduTimeType != pCoexDm->curAmpduTimeType)) {
		switch(pCoexDm->curAmpduTimeType) {
		case 0:	// normal mode
			pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x456, pCoexDm->backupAmpduMaxTime);
			break;
		case 1:	// AMPDU timw = 0x38 * 32us
			pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x456, 0x38);
			break;
		default:
			break;
		}
	}

	pCoexDm->preAmpduTimeType = pCoexDm->curAmpduTimeType;
}

VOID
halbtc8192e2ant_LimitedTx(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN				bForceExec,
    IN	u1Byte				raMaskType,
    IN	u1Byte				arfrType,
    IN	u1Byte				retryLimitType,
    IN	u1Byte				ampduTimeType
)
{
	u4Byte	disRaMask=0x0;

	pCoexDm->curRaMaskType = raMaskType;
	disRaMask = halbtc8192e2ant_DecideRaMask(pBtCoexist, pCoexDm->curSsType, raMaskType);
	halbtc8192e2ant_UpdateRaMask(pBtCoexist, bForceExec, disRaMask);

	halbtc8192e2ant_AutoRateFallbackRetry(pBtCoexist, bForceExec, arfrType);
	halbtc8192e2ant_RetryLimit(pBtCoexist, bForceExec, retryLimitType);
	halbtc8192e2ant_AmpduMaxTime(pBtCoexist, bForceExec, ampduTimeType);
}

VOID
halbtc8192e2ant_LimitedRx(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN				bForceExec,
    IN	BOOLEAN				bRejApAggPkt,
    IN	BOOLEAN				bBtCtrlAggBufSize,
    IN	u1Byte				aggBufSize
)
{
	BOOLEAN	bRejectRxAgg=bRejApAggPkt;
	BOOLEAN	bBtCtrlRxAggSize=bBtCtrlAggBufSize;
	u1Byte	rxAggSize=aggBufSize;

	//============================================
	//	Rx Aggregation related setting
	//============================================
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_BL_TO_REJ_AP_AGG_PKT, &bRejectRxAgg);
	// decide BT control aggregation buf size or not
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_BL_BT_CTRL_AGG_SIZE, &bBtCtrlRxAggSize);
	// aggregation buf size, only work when BT control Rx aggregation size.
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_U1_AGG_BUF_SIZE, &rxAggSize);
	// real update aggregation setting
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_AGGREGATE_CTRL, NULL);


}

VOID
halbtc8192e2ant_MonitorBtCtr(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u4Byte 			regHPTxRx, regLPTxRx, u4Tmp;
	u4Byte			regHPTx=0, regHPRx=0, regLPTx=0, regLPRx=0;
	//u1Byte			u1Tmp;

	regHPTxRx = 0x770;
	regLPTxRx = 0x774;

	u4Tmp = pBtCoexist->fBtcRead4Byte(pBtCoexist, regHPTxRx);
	regHPTx = u4Tmp & bMaskLWord;
	regHPRx = (u4Tmp & bMaskHWord)>>16;

	u4Tmp = pBtCoexist->fBtcRead4Byte(pBtCoexist, regLPTxRx);
	regLPTx = u4Tmp & bMaskLWord;
	regLPRx = (u4Tmp & bMaskHWord)>>16;

	pCoexSta->highPriorityTx = regHPTx;
	pCoexSta->highPriorityRx = regHPRx;
	pCoexSta->lowPriorityTx = regLPTx;
	pCoexSta->lowPriorityRx = regLPRx;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], High Priority Tx/Rx (reg 0x%x)=0x%x(%d)/0x%x(%d)\n",
	          regHPTxRx, regHPTx, regHPTx, regHPRx, regHPRx));
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_BT_MONITOR, ("[BTCoex], Low Priority Tx/Rx (reg 0x%x)=0x%x(%d)/0x%x(%d)\n",
	          regLPTxRx, regLPTx, regLPTx, regLPRx, regLPRx));

	// reset counter
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x76e, 0xc);
}

VOID
halbtc8192e2ant_QueryBtInfo(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte			H2C_Parameter[1] = {0};

	pCoexSta->bC2hBtInfoReqSent = TRUE;

	H2C_Parameter[0] |= BIT0;	// trigger

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], Query Bt Info, FW write 0x61=0x%x\n",
	          H2C_Parameter[0]));

	pBtCoexist->fBtcFillH2c(pBtCoexist, 0x61, 1, H2C_Parameter);
}

BOOLEAN
halbtc8192e2ant_IsWifiStatusChanged(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	static BOOLEAN	bPreWifiBusy=FALSE, bPreUnder4way=FALSE, bPreBtHsOn=FALSE;
	BOOLEAN	bWifiBusy=FALSE, bUnder4way=FALSE, bBtHsOn=FALSE;
	BOOLEAN	bWifiConnected=FALSE;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_CONNECTED, &bWifiConnected);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_BUSY, &bWifiBusy);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_HS_OPERATION, &bBtHsOn);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_4_WAY_PROGRESS, &bUnder4way);

	if(bWifiConnected) {
		if(bWifiBusy != bPreWifiBusy) {
			bPreWifiBusy = bWifiBusy;
			return TRUE;
		}
		if(bUnder4way != bPreUnder4way) {
			bPreUnder4way = bUnder4way;
			return TRUE;
		}
		if(bBtHsOn != bPreBtHsOn) {
			bPreBtHsOn = bBtHsOn;
			return TRUE;
		}
	}

	return FALSE;
}

VOID
halbtc8192e2ant_UpdateBtLinkInfo(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	PBTC_BT_LINK_INFO	pBtLinkInfo=&pBtCoexist->btLinkInfo;
	BOOLEAN				bBtHsOn=FALSE;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_HS_OPERATION, &bBtHsOn);

	pBtLinkInfo->bBtLinkExist = pCoexSta->bBtLinkExist;
	pBtLinkInfo->bScoExist = pCoexSta->bScoExist;
	pBtLinkInfo->bA2dpExist = pCoexSta->bA2dpExist;
	pBtLinkInfo->bPanExist = pCoexSta->bPanExist;
	pBtLinkInfo->bHidExist = pCoexSta->bHidExist;

	// work around for HS mode.
	if(bBtHsOn) {
		pBtLinkInfo->bPanExist = TRUE;
		pBtLinkInfo->bBtLinkExist = TRUE;
	}

	// check if Sco only
	if( pBtLinkInfo->bScoExist &&
	    !pBtLinkInfo->bA2dpExist &&
	    !pBtLinkInfo->bPanExist &&
	    !pBtLinkInfo->bHidExist )
		pBtLinkInfo->bScoOnly = TRUE;
	else
		pBtLinkInfo->bScoOnly = FALSE;

	// check if A2dp only
	if( !pBtLinkInfo->bScoExist &&
	    pBtLinkInfo->bA2dpExist &&
	    !pBtLinkInfo->bPanExist &&
	    !pBtLinkInfo->bHidExist )
		pBtLinkInfo->bA2dpOnly = TRUE;
	else
		pBtLinkInfo->bA2dpOnly = FALSE;

	// check if Pan only
	if( !pBtLinkInfo->bScoExist &&
	    !pBtLinkInfo->bA2dpExist &&
	    pBtLinkInfo->bPanExist &&
	    !pBtLinkInfo->bHidExist )
		pBtLinkInfo->bPanOnly = TRUE;
	else
		pBtLinkInfo->bPanOnly = FALSE;

	// check if Hid only
	if( !pBtLinkInfo->bScoExist &&
	    !pBtLinkInfo->bA2dpExist &&
	    !pBtLinkInfo->bPanExist &&
	    pBtLinkInfo->bHidExist )
		pBtLinkInfo->bHidOnly = TRUE;
	else
		pBtLinkInfo->bHidOnly = FALSE;
}

u1Byte
halbtc8192e2ant_ActionAlgorithm(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	PBTC_BT_LINK_INFO	pBtLinkInfo=&pBtCoexist->btLinkInfo;
	PBTC_STACK_INFO 	pStackInfo=&pBtCoexist->stackInfo;
	BOOLEAN				bBtHsOn=FALSE;
	u1Byte				algorithm=BT_8192E_2ANT_COEX_ALGO_UNDEFINED;
	u1Byte				numOfDiffProfile=0;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_HS_OPERATION, &bBtHsOn);

	if(!pBtLinkInfo->bBtLinkExist) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], No BT link exists!!!\n"));
		return algorithm;
	}

	if(pBtLinkInfo->bScoExist)
		numOfDiffProfile++;
	if(pBtLinkInfo->bHidExist)
		numOfDiffProfile++;
	if(pBtLinkInfo->bPanExist)
		numOfDiffProfile++;
	if(pBtLinkInfo->bA2dpExist)
		numOfDiffProfile++;

	if(numOfDiffProfile == 1) {
		if(pBtLinkInfo->bScoExist) {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO only\n"));
			algorithm = BT_8192E_2ANT_COEX_ALGO_SCO;
		} else {
			if(pBtLinkInfo->bHidExist) {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID only\n"));
				algorithm = BT_8192E_2ANT_COEX_ALGO_HID;
			} else if(pBtLinkInfo->bA2dpExist) {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], A2DP only\n"));
				algorithm = BT_8192E_2ANT_COEX_ALGO_A2DP;
			} else if(pBtLinkInfo->bPanExist) {
				if(bBtHsOn) {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], PAN(HS) only\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_PANHS;
				} else {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], PAN(EDR) only\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_PANEDR;
				}
			}
		}
	} else if(numOfDiffProfile == 2) {
		if(pBtLinkInfo->bScoExist) {
			if(pBtLinkInfo->bHidExist) {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + HID\n"));
				algorithm = BT_8192E_2ANT_COEX_ALGO_SCO;
			} else if(pBtLinkInfo->bA2dpExist) {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + A2DP ==> SCO\n"));
				algorithm = BT_8192E_2ANT_COEX_ALGO_PANEDR_HID;
			} else if(pBtLinkInfo->bPanExist) {
				if(bBtHsOn) {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + PAN(HS)\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_SCO;
				} else {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + PAN(EDR)\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_SCO_PAN;
				}
			}
		} else {
			if( pBtLinkInfo->bHidExist &&
			    pBtLinkInfo->bA2dpExist ) {
				if(pStackInfo->numOfHid >= 2) {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID*2 + A2DP\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_HID_A2DP_PANEDR;
				} else {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID + A2DP\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_HID_A2DP;
				}
			} else if( pBtLinkInfo->bHidExist &&
			           pBtLinkInfo->bPanExist ) {
				if(bBtHsOn) {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID + PAN(HS)\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_HID;
				} else {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID + PAN(EDR)\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_PANEDR_HID;
				}
			} else if( pBtLinkInfo->bPanExist &&
			           pBtLinkInfo->bA2dpExist ) {
				if(bBtHsOn) {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], A2DP + PAN(HS)\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_A2DP_PANHS;
				} else {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], A2DP + PAN(EDR)\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_PANEDR_A2DP;
				}
			}
		}
	} else if(numOfDiffProfile == 3) {
		if(pBtLinkInfo->bScoExist) {
			if( pBtLinkInfo->bHidExist &&
			    pBtLinkInfo->bA2dpExist ) {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + HID + A2DP ==> HID\n"));
				algorithm = BT_8192E_2ANT_COEX_ALGO_PANEDR_HID;
			} else if( pBtLinkInfo->bHidExist &&
			           pBtLinkInfo->bPanExist ) {
				if(bBtHsOn) {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + HID + PAN(HS)\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_SCO;
				} else {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + HID + PAN(EDR)\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_SCO_PAN;
				}
			} else if( pBtLinkInfo->bPanExist &&
			           pBtLinkInfo->bA2dpExist ) {
				if(bBtHsOn) {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + A2DP + PAN(HS)\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_SCO;
				} else {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + A2DP + PAN(EDR) ==> HID\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_PANEDR_HID;
				}
			}
		} else {
			if( pBtLinkInfo->bHidExist &&
			    pBtLinkInfo->bPanExist &&
			    pBtLinkInfo->bA2dpExist ) {
				if(bBtHsOn) {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID + A2DP + PAN(HS)\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_HID_A2DP;
				} else {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], HID + A2DP + PAN(EDR)\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_HID_A2DP_PANEDR;
				}
			}
		}
	} else if(numOfDiffProfile >= 3) {
		if(pBtLinkInfo->bScoExist) {
			if( pBtLinkInfo->bHidExist &&
			    pBtLinkInfo->bPanExist &&
			    pBtLinkInfo->bA2dpExist ) {
				if(bBtHsOn) {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Error!!! SCO + HID + A2DP + PAN(HS)\n"));

				} else {
					BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], SCO + HID + A2DP + PAN(EDR)==>PAN(EDR)+HID\n"));
					algorithm = BT_8192E_2ANT_COEX_ALGO_PANEDR_HID;
				}
			}
		}
	}

	return algorithm;
}

VOID
halbtc8192e2ant_SetFwDacSwingLevel(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte			dacSwingLvl
)
{
	u1Byte			H2C_Parameter[1] = {0};

	// There are several type of dacswing
	// 0x18/ 0x10/ 0xc/ 0x8/ 0x4/ 0x6
	H2C_Parameter[0] = dacSwingLvl;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], Set Dac Swing Level=0x%x\n", dacSwingLvl));
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], FW write 0x64=0x%x\n", H2C_Parameter[0]));

	pBtCoexist->fBtcFillH2c(pBtCoexist, 0x64, 1, H2C_Parameter);
}

VOID
halbtc8192e2ant_SetFwDecBtPwr(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte				decBtPwrLvl
)
{
	u1Byte			H2C_Parameter[1] = {0};

	H2C_Parameter[0] = decBtPwrLvl;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], decrease Bt Power level = %d, FW write 0x62=0x%x\n",
	          decBtPwrLvl, H2C_Parameter[0]));

	pBtCoexist->fBtcFillH2c(pBtCoexist, 0x62, 1, H2C_Parameter);
}

VOID
halbtc8192e2ant_DecBtPwr(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN				bForceExec,
    IN	u1Byte				decBtPwrLvl
)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s Dec BT power level = %d\n",
	          (bForceExec? "force to":""), decBtPwrLvl));
	pCoexDm->curBtDecPwrLvl = decBtPwrLvl;

	if(!bForceExec) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], preBtDecPwrLvl=%d, curBtDecPwrLvl=%d\n",
		          pCoexDm->preBtDecPwrLvl, pCoexDm->curBtDecPwrLvl));
#if 0	// work around, avoid h2c command fail.
		if(pCoexDm->preBtDecPwrLvl == pCoexDm->curBtDecPwrLvl)
			return;
#endif
	}
	halbtc8192e2ant_SetFwDecBtPwr(pBtCoexist, pCoexDm->curBtDecPwrLvl);

	pCoexDm->preBtDecPwrLvl = pCoexDm->curBtDecPwrLvl;
}

VOID
halbtc8192e2ant_SetBtAutoReport(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bEnableAutoReport
)
{
	u1Byte			H2C_Parameter[1] = {0};

	H2C_Parameter[0] = 0;

	if(bEnableAutoReport) {
		H2C_Parameter[0] |= BIT0;
	}

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], BT FW auto report : %s, FW write 0x68=0x%x\n",
	          (bEnableAutoReport? "Enabled!!":"Disabled!!"), H2C_Parameter[0]));

	pBtCoexist->fBtcFillH2c(pBtCoexist, 0x68, 1, H2C_Parameter);
}

VOID
halbtc8192e2ant_BtAutoReport(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bForceExec,
    IN	BOOLEAN			bEnableAutoReport
)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s BT Auto report = %s\n",
	          (bForceExec? "force to":""), ((bEnableAutoReport)? "Enabled":"Disabled")));
	pCoexDm->bCurBtAutoReport = bEnableAutoReport;

	if(!bForceExec) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], bPreBtAutoReport=%d, bCurBtAutoReport=%d\n",
		          pCoexDm->bPreBtAutoReport, pCoexDm->bCurBtAutoReport));

		if(pCoexDm->bPreBtAutoReport == pCoexDm->bCurBtAutoReport)
			return;
	}
	halbtc8192e2ant_SetBtAutoReport(pBtCoexist, pCoexDm->bCurBtAutoReport);

	pCoexDm->bPreBtAutoReport = pCoexDm->bCurBtAutoReport;
}

VOID
halbtc8192e2ant_FwDacSwingLvl(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bForceExec,
    IN	u1Byte			fwDacSwingLvl
)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s set FW Dac Swing level = %d\n",
	          (bForceExec? "force to":""), fwDacSwingLvl));
	pCoexDm->curFwDacSwingLvl = fwDacSwingLvl;

	if(!bForceExec) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], preFwDacSwingLvl=%d, curFwDacSwingLvl=%d\n",
		          pCoexDm->preFwDacSwingLvl, pCoexDm->curFwDacSwingLvl));

		if(pCoexDm->preFwDacSwingLvl == pCoexDm->curFwDacSwingLvl)
			return;
	}

	halbtc8192e2ant_SetFwDacSwingLevel(pBtCoexist, pCoexDm->curFwDacSwingLvl);

	pCoexDm->preFwDacSwingLvl = pCoexDm->curFwDacSwingLvl;
}

VOID
halbtc8192e2ant_SetSwRfRxLpfCorner(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bRxRfShrinkOn
)
{
	if(bRxRfShrinkOn) {
		//Shrink RF Rx LPF corner
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Shrink RF Rx LPF corner!!\n"));
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x1e, 0xfffff, 0xffffc);
	} else {
		//Resume RF Rx LPF corner
		// After initialized, we can use pCoexDm->btRf0x1eBackup
		if(pBtCoexist->bInitilized) {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Resume RF Rx LPF corner!!\n"));
			pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x1e, 0xfffff, pCoexDm->btRf0x1eBackup);
		}
	}
}

VOID
halbtc8192e2ant_RfShrink(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bForceExec,
    IN	BOOLEAN			bRxRfShrinkOn
)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s turn Rx RF Shrink = %s\n",
	          (bForceExec? "force to":""), ((bRxRfShrinkOn)? "ON":"OFF")));
	pCoexDm->bCurRfRxLpfShrink = bRxRfShrinkOn;

	if(!bForceExec) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], bPreRfRxLpfShrink=%d, bCurRfRxLpfShrink=%d\n",
		          pCoexDm->bPreRfRxLpfShrink, pCoexDm->bCurRfRxLpfShrink));

		if(pCoexDm->bPreRfRxLpfShrink == pCoexDm->bCurRfRxLpfShrink)
			return;
	}
	halbtc8192e2ant_SetSwRfRxLpfCorner(pBtCoexist, pCoexDm->bCurRfRxLpfShrink);

	pCoexDm->bPreRfRxLpfShrink = pCoexDm->bCurRfRxLpfShrink;
}

VOID
halbtc8192e2ant_SetSwPenaltyTxRateAdaptive(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bLowPenaltyRa
)
{
	u1Byte			H2C_Parameter[6] = {0};

	H2C_Parameter[0] = 0x6;	// opCode, 0x6= Retry_Penalty

	if(bLowPenaltyRa) {
		H2C_Parameter[1] |= BIT0;
		H2C_Parameter[2] = 0x00;  //normal rate except MCS7/6/5, OFDM54/48/36
		H2C_Parameter[3] = 0xf7;  //MCS7 or OFDM54
		H2C_Parameter[4] = 0xf8;  //MCS6 or OFDM48
		H2C_Parameter[5] = 0xf9;	//MCS5 or OFDM36
	}

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], set WiFi Low-Penalty Retry: %s",
	          (bLowPenaltyRa? "ON!!":"OFF!!")) );

	pBtCoexist->fBtcFillH2c(pBtCoexist, 0x69, 6, H2C_Parameter);
}

VOID
halbtc8192e2ant_LowPenaltyRa(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bForceExec,
    IN	BOOLEAN			bLowPenaltyRa
)
{
	//return;
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s turn LowPenaltyRA = %s\n",
	          (bForceExec? "force to":""), ((bLowPenaltyRa)? "ON":"OFF")));
	pCoexDm->bCurLowPenaltyRa = bLowPenaltyRa;

	if(!bForceExec) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], bPreLowPenaltyRa=%d, bCurLowPenaltyRa=%d\n",
		          pCoexDm->bPreLowPenaltyRa, pCoexDm->bCurLowPenaltyRa));

		if(pCoexDm->bPreLowPenaltyRa == pCoexDm->bCurLowPenaltyRa)
			return;
	}
	halbtc8192e2ant_SetSwPenaltyTxRateAdaptive(pBtCoexist, pCoexDm->bCurLowPenaltyRa);

	pCoexDm->bPreLowPenaltyRa = pCoexDm->bCurLowPenaltyRa;
}

VOID
halbtc8192e2ant_SetDacSwingReg(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u4Byte			level
)
{
	u1Byte	val=(u1Byte)level;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Write SwDacSwing = 0x%x\n", level));
	pBtCoexist->fBtcWrite1ByteBitMask(pBtCoexist, 0x883, 0x3e, val);
}

VOID
halbtc8192e2ant_SetSwFullTimeDacSwing(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bSwDacSwingOn,
    IN	u4Byte			swDacSwingLvl
)
{
	if(bSwDacSwingOn) {
		halbtc8192e2ant_SetDacSwingReg(pBtCoexist, swDacSwingLvl);
	} else {
		halbtc8192e2ant_SetDacSwingReg(pBtCoexist, 0x18);
	}
}


VOID
halbtc8192e2ant_DacSwing(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bForceExec,
    IN	BOOLEAN			bDacSwingOn,
    IN	u4Byte			dacSwingLvl
)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s turn DacSwing=%s, dacSwingLvl=0x%x\n",
	          (bForceExec? "force to":""), ((bDacSwingOn)? "ON":"OFF"), dacSwingLvl));
	pCoexDm->bCurDacSwingOn = bDacSwingOn;
	pCoexDm->curDacSwingLvl = dacSwingLvl;

	if(!bForceExec) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], bPreDacSwingOn=%d, preDacSwingLvl=0x%x, bCurDacSwingOn=%d, curDacSwingLvl=0x%x\n",
		          pCoexDm->bPreDacSwingOn, pCoexDm->preDacSwingLvl,
		          pCoexDm->bCurDacSwingOn, pCoexDm->curDacSwingLvl));

		if( (pCoexDm->bPreDacSwingOn == pCoexDm->bCurDacSwingOn) &&
		    (pCoexDm->preDacSwingLvl == pCoexDm->curDacSwingLvl) )
			return;
	}
	delay_ms(30);
	halbtc8192e2ant_SetSwFullTimeDacSwing(pBtCoexist, bDacSwingOn, dacSwingLvl);

	pCoexDm->bPreDacSwingOn = pCoexDm->bCurDacSwingOn;
	pCoexDm->preDacSwingLvl = pCoexDm->curDacSwingLvl;
}

VOID
halbtc8192e2ant_SetAdcBackOff(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bAdcBackOff
)
{
	if(bAdcBackOff) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], BB BackOff Level On!\n"));
		pBtCoexist->fBtcWrite1ByteBitMask(pBtCoexist, 0xc05, 0x30, 0x3);
	} else {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], BB BackOff Level Off!\n"));
		pBtCoexist->fBtcWrite1ByteBitMask(pBtCoexist, 0xc05, 0x30, 0x1);
	}
}

VOID
halbtc8192e2ant_AdcBackOff(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bForceExec,
    IN	BOOLEAN			bAdcBackOff
)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s turn AdcBackOff = %s\n",
	          (bForceExec? "force to":""), ((bAdcBackOff)? "ON":"OFF")));
	pCoexDm->bCurAdcBackOff = bAdcBackOff;

	if(!bForceExec) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], bPreAdcBackOff=%d, bCurAdcBackOff=%d\n",
		          pCoexDm->bPreAdcBackOff, pCoexDm->bCurAdcBackOff));

		if(pCoexDm->bPreAdcBackOff == pCoexDm->bCurAdcBackOff)
			return;
	}
	halbtc8192e2ant_SetAdcBackOff(pBtCoexist, pCoexDm->bCurAdcBackOff);

	pCoexDm->bPreAdcBackOff = pCoexDm->bCurAdcBackOff;
}

VOID
halbtc8192e2ant_SetAgcTable(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bAgcTableEn
)
{
	//u1Byte		rssiAdjustVal=0;

	//=================BB AGC Gain Table
	if(bAgcTableEn) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], BB Agc Table On!\n"));
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0x0a1A0001);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0x091B0001);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0x081C0001);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0x071D0001);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0x061E0001);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0x051F0001);
	} else {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], BB Agc Table Off!\n"));
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0xaa1A0001);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0xa91B0001);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0xa81C0001);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0xa71D0001);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0xa61E0001);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0xc78, 0xa51F0001);
	}

#if 0
	//=================RF Gain
	pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0xef, 0xfffff, 0x02000);
	if(bAgcTableEn) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Agc Table On!\n"));
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x3b, 0xfffff, 0x38fff);
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x3b, 0xfffff, 0x38ffe);
	} else {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Agc Table Off!\n"));
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x3b, 0xfffff, 0x380c3);
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x3b, 0xfffff, 0x28ce6);
	}
	pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0xef, 0xfffff, 0x0);

	pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0xed, 0xfffff, 0x1);
	if(bAgcTableEn) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Agc Table On!\n"));
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x40, 0xfffff, 0x38fff);
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x40, 0xfffff, 0x38ffe);
	} else {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], Agc Table Off!\n"));
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x40, 0xfffff, 0x380c3);
		pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0x40, 0xfffff, 0x28ce6);
	}
	pBtCoexist->fBtcSetRfReg(pBtCoexist, BTC_RF_A, 0xed, 0xfffff, 0x0);

	// set rssiAdjustVal for wifi module.
	if(bAgcTableEn) {
		rssiAdjustVal = 8;
	}
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_U1_RSSI_ADJ_VAL_FOR_AGC_TABLE_ON, &rssiAdjustVal);
#endif

}

VOID
halbtc8192e2ant_AgcTable(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bForceExec,
    IN	BOOLEAN			bAgcTableEn
)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s %s Agc Table\n",
	          (bForceExec? "force to":""), ((bAgcTableEn)? "Enable":"Disable")));
	pCoexDm->bCurAgcTableEn = bAgcTableEn;

	if(!bForceExec) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], bPreAgcTableEn=%d, bCurAgcTableEn=%d\n",
		          pCoexDm->bPreAgcTableEn, pCoexDm->bCurAgcTableEn));

		if(pCoexDm->bPreAgcTableEn == pCoexDm->bCurAgcTableEn)
			return;
	}
	halbtc8192e2ant_SetAgcTable(pBtCoexist, bAgcTableEn);

	pCoexDm->bPreAgcTableEn = pCoexDm->bCurAgcTableEn;
}

VOID
halbtc8192e2ant_SetCoexTable(
    IN	PBTC_COEXIST	pBtCoexist,
    IN	u4Byte		val0x6c0,
    IN	u4Byte		val0x6c4,
    IN	u4Byte		val0x6c8,
    IN	u1Byte		val0x6cc
)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], set coex table, set 0x6c0=0x%x\n", val0x6c0));
	pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x6c0, val0x6c0);

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], set coex table, set 0x6c4=0x%x\n", val0x6c4));
	pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x6c4, val0x6c4);

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], set coex table, set 0x6c8=0x%x\n", val0x6c8));
	pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x6c8, val0x6c8);

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_EXEC, ("[BTCoex], set coex table, set 0x6cc=0x%x\n", val0x6cc));
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x6cc, val0x6cc);
}

VOID
halbtc8192e2ant_CoexTable(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bForceExec,
    IN	u4Byte			val0x6c0,
    IN	u4Byte			val0x6c4,
    IN	u4Byte			val0x6c8,
    IN	u1Byte			val0x6cc
)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW, ("[BTCoex], %s write Coex Table 0x6c0=0x%x, 0x6c4=0x%x, 0x6c8=0x%x, 0x6cc=0x%x\n",
	          (bForceExec? "force to":""), val0x6c0, val0x6c4, val0x6c8, val0x6cc));
	pCoexDm->curVal0x6c0 = val0x6c0;
	pCoexDm->curVal0x6c4 = val0x6c4;
	pCoexDm->curVal0x6c8 = val0x6c8;
	pCoexDm->curVal0x6cc = val0x6cc;

	if(!bForceExec) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], preVal0x6c0=0x%x, preVal0x6c4=0x%x, preVal0x6c8=0x%x, preVal0x6cc=0x%x !!\n",
		          pCoexDm->preVal0x6c0, pCoexDm->preVal0x6c4, pCoexDm->preVal0x6c8, pCoexDm->preVal0x6cc));
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_SW_DETAIL, ("[BTCoex], curVal0x6c0=0x%x, curVal0x6c4=0x%x, curVal0x6c8=0x%x, curVal0x6cc=0x%x !!\n",
		          pCoexDm->curVal0x6c0, pCoexDm->curVal0x6c4, pCoexDm->curVal0x6c8, pCoexDm->curVal0x6cc));

		if( (pCoexDm->preVal0x6c0 == pCoexDm->curVal0x6c0) &&
		    (pCoexDm->preVal0x6c4 == pCoexDm->curVal0x6c4) &&
		    (pCoexDm->preVal0x6c8 == pCoexDm->curVal0x6c8) &&
		    (pCoexDm->preVal0x6cc == pCoexDm->curVal0x6cc) )
			return;
	}
	halbtc8192e2ant_SetCoexTable(pBtCoexist, val0x6c0, val0x6c4, val0x6c8, val0x6cc);

	pCoexDm->preVal0x6c0 = pCoexDm->curVal0x6c0;
	pCoexDm->preVal0x6c4 = pCoexDm->curVal0x6c4;
	pCoexDm->preVal0x6c8 = pCoexDm->curVal0x6c8;
	pCoexDm->preVal0x6cc = pCoexDm->curVal0x6cc;
}

VOID
halbtc8192e2ant_CoexTableWithType(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN				bForceExec,
    IN	u1Byte				type
)
{
	switch(type) {
	case 0:
		halbtc8192e2ant_CoexTable(pBtCoexist, bForceExec, 0x55555555, 0x5a5a5a5a, 0xffffff, 0x3);
		break;
	case 1:
		halbtc8192e2ant_CoexTable(pBtCoexist, bForceExec, 0x5a5a5a5a, 0x5a5a5a5a, 0xffffff, 0x3);
		break;
	case 2:
		halbtc8192e2ant_CoexTable(pBtCoexist, bForceExec, 0x55555555, 0x5ffb5ffb, 0xffffff, 0x3);
		break;
	case 3:
		halbtc8192e2ant_CoexTable(pBtCoexist, bForceExec, 0x5fdf5fdf, 0x5fdb5fdb, 0xffffff, 0x3);
		break;
	case 4:
		halbtc8192e2ant_CoexTable(pBtCoexist, bForceExec, 0xdfffdfff, 0x5ffb5ffb, 0xffffff, 0x3);
		break;

	default:
		break;
	}
}

VOID
halbtc8192e2ant_SetFwIgnoreWlanAct(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bEnable
)
{
	u1Byte			H2C_Parameter[1] = {0};

	if(bEnable) {
		H2C_Parameter[0] |= BIT0;		// function enable
	}

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], set FW for BT Ignore Wlan_Act, FW write 0x63=0x%x\n",
	          H2C_Parameter[0]));

	pBtCoexist->fBtcFillH2c(pBtCoexist, 0x63, 1, H2C_Parameter);
}

VOID
halbtc8192e2ant_IgnoreWlanAct(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bForceExec,
    IN	BOOLEAN			bEnable
)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s turn Ignore WlanAct %s\n",
	          (bForceExec? "force to":""), (bEnable? "ON":"OFF")));
	pCoexDm->bCurIgnoreWlanAct = bEnable;

	if(!bForceExec) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], bPreIgnoreWlanAct = %d, bCurIgnoreWlanAct = %d!!\n",
		          pCoexDm->bPreIgnoreWlanAct, pCoexDm->bCurIgnoreWlanAct));

		if(pCoexDm->bPreIgnoreWlanAct == pCoexDm->bCurIgnoreWlanAct)
			return;
	}
	halbtc8192e2ant_SetFwIgnoreWlanAct(pBtCoexist, bEnable);

	pCoexDm->bPreIgnoreWlanAct = pCoexDm->bCurIgnoreWlanAct;
}

VOID
halbtc8192e2ant_SetFwPstdma(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte			byte1,
    IN	u1Byte			byte2,
    IN	u1Byte			byte3,
    IN	u1Byte			byte4,
    IN	u1Byte			byte5
)
{
	u1Byte			H2C_Parameter[5] = {0};

	H2C_Parameter[0] = byte1;
	H2C_Parameter[1] = byte2;
	H2C_Parameter[2] = byte3;
	H2C_Parameter[3] = byte4;
	H2C_Parameter[4] = byte5;

	pCoexDm->psTdmaPara[0] = byte1;
	pCoexDm->psTdmaPara[1] = byte2;
	pCoexDm->psTdmaPara[2] = byte3;
	pCoexDm->psTdmaPara[3] = byte4;
	pCoexDm->psTdmaPara[4] = byte5;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], FW write 0x60(5bytes)=0x%x%08x\n",
	          H2C_Parameter[0],
	          H2C_Parameter[1]<<24|H2C_Parameter[2]<<16|H2C_Parameter[3]<<8|H2C_Parameter[4]));

	pBtCoexist->fBtcFillH2c(pBtCoexist, 0x60, 5, H2C_Parameter);
}

VOID
halbtc8192e2ant_SwMechanism1(
    IN	PBTC_COEXIST	pBtCoexist,
    IN	BOOLEAN		bShrinkRxLPF,
    IN	BOOLEAN 	bLowPenaltyRA,
    IN	BOOLEAN		bLimitedDIG,
    IN	BOOLEAN		bBTLNAConstrain
)
{
	/*
	u4Byte	wifiBw;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if(BTC_WIFI_BW_HT40 != wifiBw)  //only shrink RF Rx LPF for HT40
	{
		if (bShrinkRxLPF)
			bShrinkRxLPF = FALSE;
	}
	*/

	halbtc8192e2ant_RfShrink(pBtCoexist, NORMAL_EXEC, bShrinkRxLPF);
	//halbtc8192e2ant_LowPenaltyRa(pBtCoexist, NORMAL_EXEC, bLowPenaltyRA);
}

VOID
halbtc8192e2ant_SwMechanism2(
    IN	PBTC_COEXIST	pBtCoexist,
    IN	BOOLEAN		bAGCTableShift,
    IN	BOOLEAN 	bADCBackOff,
    IN	BOOLEAN		bSWDACSwing,
    IN	u4Byte		dacSwingLvl
)
{
	halbtc8192e2ant_AgcTable(pBtCoexist, NORMAL_EXEC, bAGCTableShift);
	//halbtc8192e2ant_AdcBackOff(pBtCoexist, NORMAL_EXEC, bADCBackOff);
	halbtc8192e2ant_DacSwing(pBtCoexist, NORMAL_EXEC, bSWDACSwing, dacSwingLvl);
}

VOID
halbtc8192e2ant_SetAntPath(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte				antPosType,
    IN	BOOLEAN				bInitHwCfg,
    IN	BOOLEAN				bWifiOff
)
{
	u4Byte			u4Tmp=0;

	if(bInitHwCfg) {
		pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x944, 0x24);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x930, 0x700700);
		if(pBtCoexist->chipInterface == BTC_INTF_USB)
			pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x64, 0x30430004);
		else
			pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x64, 0x30030004);

		// 0x4c[27][24]='00', Set Antenna to BB
		u4Tmp = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x4c);
		u4Tmp &= ~BIT24;
		u4Tmp &= ~BIT27;
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x4c, u4Tmp);
	} else if(bWifiOff) {
		if(pBtCoexist->chipInterface == BTC_INTF_USB)
			pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x64, 0x30430004);
		else
			pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x64, 0x30030004);

		// 0x4c[27][24]='11', Set Antenna to BT, 0x64[8:7]=0, 0x64[2]=1
		u4Tmp = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x4c);
		u4Tmp |= BIT24;
		u4Tmp |= BIT27;
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x4c, u4Tmp);
	}

	// ext switch setting
	switch(antPosType) {
	case BTC_ANT_PATH_WIFI:
		pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x92c, 0x4);
		break;
	case BTC_ANT_PATH_BT:
		pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x92c, 0x20);
		break;
	default:
	case BTC_ANT_PATH_PTA:
		pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x92c, 0x4);
		break;
	}
}

VOID
halbtc8192e2ant_PsTdma(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bForceExec,
    IN	BOOLEAN			bTurnOn,
    IN	u1Byte			type
)
{
	//BOOLEAN			bTurnOnByCnt=FALSE;
	//u1Byte			psTdmaTypeByCnt=0;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], %s turn %s PS TDMA, type=%d\n",
	          (bForceExec? "force to":""), (bTurnOn? "ON":"OFF"), type));
	pCoexDm->bCurPsTdmaOn = bTurnOn;
	pCoexDm->curPsTdma = type;

	if(!bForceExec) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], bPrePsTdmaOn = %d, bCurPsTdmaOn = %d!!\n",
		          pCoexDm->bPrePsTdmaOn, pCoexDm->bCurPsTdmaOn));
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], prePsTdma = %d, curPsTdma = %d!!\n",
		          pCoexDm->prePsTdma, pCoexDm->curPsTdma));

		if( (pCoexDm->bPrePsTdmaOn == pCoexDm->bCurPsTdmaOn) &&
		    (pCoexDm->prePsTdma == pCoexDm->curPsTdma) )
			return;
	}
	if(bTurnOn) {
		switch(type) {
		case 1:
		default:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x1a, 0x1a, 0xe1, 0x90);
			break;
		case 2:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x12, 0x12, 0xe1, 0x90);
			break;
		case 3:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x1c, 0x3, 0xf1, 0x90);
			break;
		case 4:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x10, 0x3, 0xf1, 0x90);
			break;
		case 5:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x1a, 0x1a, 0x60, 0x90);
			break;
		case 6:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x12, 0x12, 0x60, 0x90);
			break;
		case 7:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x1c, 0x3, 0x70, 0x90);
			break;
		case 8:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xa3, 0x10, 0x3, 0x70, 0x90);
			break;
		case 9:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x1a, 0x1a, 0xe1, 0x10);
			break;
		case 10:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x12, 0x12, 0xe1, 0x10);
			break;
		case 11:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x1c, 0x3, 0xf1, 0x10);
			break;
		case 12:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x10, 0x3, 0xf1, 0x10);
			break;
		case 13:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x1a, 0x1a, 0xe0, 0x10);
			break;
		case 14:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x12, 0x12, 0xe0, 0x10);
			break;
		case 15:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x1c, 0x3, 0xf0, 0x10);
			break;
		case 16:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x12, 0x3, 0xf0, 0x10);
			break;
		case 17:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0x61, 0x20, 0x03, 0x10, 0x10);
			break;
		case 18:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x5, 0x5, 0xe1, 0x90);
			break;
		case 19:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x25, 0x25, 0xe1, 0x90);
			break;
		case 20:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x25, 0x25, 0x60, 0x90);
			break;
		case 21:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x15, 0x03, 0x70, 0x90);
			break;
		case 71:
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0xe3, 0x1a, 0x1a, 0xe1, 0x90);
			break;
		}
	} else {
		// disable PS tdma
		switch(type) {
		default:
		case 0:  //ANT2PTA, 0x778=1
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0x8, 0x0, 0x0, 0x0, 0x0);
			halbtc8192e2ant_SetAntPath(pBtCoexist, BTC_ANT_PATH_PTA, FALSE, FALSE);
			break;
		case 1:  //ANT2BT, 0x778=3
			halbtc8192e2ant_SetFwPstdma(pBtCoexist, 0x0, 0x0, 0x0, 0x8, 0x0);
			delay_ms(5);
			halbtc8192e2ant_SetAntPath(pBtCoexist, BTC_ANT_PATH_BT, FALSE, FALSE);
			break;

		}
	}

	// update pre state
	pCoexDm->bPrePsTdmaOn = pCoexDm->bCurPsTdmaOn;
	pCoexDm->prePsTdma = pCoexDm->curPsTdma;
}

VOID
halbtc8192e2ant_SetSwitchSsType(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte				ssType
)
{
	u1Byte	mimoPs=BTC_MIMO_PS_DYNAMIC;
	u4Byte	disRaMask=0x0;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], REAL set SS Type = %d\n", ssType));

	disRaMask = halbtc8192e2ant_DecideRaMask(pBtCoexist, ssType, pCoexDm->curRaMaskType);
	halbtc8192e2ant_UpdateRaMask(pBtCoexist, FORCE_EXEC, disRaMask);

	if(ssType == 1) {
		halbtc8192e2ant_PsTdma(pBtCoexist, FORCE_EXEC, FALSE, 1);
		// switch ofdm path
		pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0xc04, 0x11);
		pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0xd04, 0x1);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x90c, 0x81111111);
		// switch cck patch
		//Jenyu suggest to remove 0xe77 this line for tx issue
		//pBtCoexist->fBtcWrite1ByteBitMask(pBtCoexist, 0xe77, 0x4, 0x1);
		//pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0xa07, 0x81);
		mimoPs=BTC_MIMO_PS_STATIC;
	} else if(ssType == 2) {
		halbtc8192e2ant_PsTdma(pBtCoexist, FORCE_EXEC, FALSE, 0);
		pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0xc04, 0x33);
		pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0xd04, 0x3);
		pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x90c, 0x81121313);
		//Jenyu suggest to remove 0xe77 this line for tx issue
		//pBtCoexist->fBtcWrite1ByteBitMask(pBtCoexist, 0xe77, 0x4, 0x0);
		//pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0xa07, 0x41);
		mimoPs=BTC_MIMO_PS_DYNAMIC;
	}

	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_SEND_MIMO_PS, &mimoPs);	// set rx 1ss or 2ss
}

VOID
halbtc8192e2ant_SwitchSsType(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN				bForceExec,
    IN	u1Byte				newSsType
)
{
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], %s Switch SS Type = %d\n",
	          (bForceExec? "force to":""), newSsType));
	pCoexDm->curSsType = newSsType;

	if(!bForceExec) {
		if(pCoexDm->preSsType == pCoexDm->curSsType)
			return;
	}
	halbtc8192e2ant_SetSwitchSsType(pBtCoexist, pCoexDm->curSsType);

	pCoexDm->preSsType = pCoexDm->curSsType;
}

VOID
halbtc8192e2ant_CoexAllOff(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	// fw all off
	halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 1);
	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
	halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);

	// sw all off
	halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
	halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);

	// hw all off
	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 0);
}

VOID
halbtc8192e2ant_InitCoexDm(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	// force to reset coex mechanism

	halbtc8192e2ant_PsTdma(pBtCoexist, FORCE_EXEC, FALSE, 1);
	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, FORCE_EXEC, 6);
	halbtc8192e2ant_DecBtPwr(pBtCoexist, FORCE_EXEC, 0);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, FORCE_EXEC, 0);
	halbtc8192e2ant_SwitchSsType(pBtCoexist, FORCE_EXEC, 2);

	halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
	halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
}

VOID
halbtc8192e2ant_ActionBtInquiry(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	BOOLEAN	bLowPwrDisable=TRUE;

	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_DISABLE_LOW_POWER, &bLowPwrDisable);

	halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 2);
	halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
	halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);

	halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
	halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
}

BOOLEAN
halbtc8192e2ant_IsCommonAction(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	PBTC_BT_LINK_INFO	pBtLinkInfo=&pBtCoexist->btLinkInfo;
	BOOLEAN			bCommon=FALSE, bWifiConnected=FALSE, bWifiBusy=FALSE;
	BOOLEAN			bBtHsOn=FALSE, bLowPwrDisable=FALSE;

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_HS_OPERATION, &bBtHsOn);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_CONNECTED, &bWifiConnected);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_BUSY, &bWifiBusy);

	if(pBtLinkInfo->bScoExist || pBtLinkInfo->bHidExist) {
		halbtc8192e2ant_LimitedTx(pBtCoexist, NORMAL_EXEC, 1, 0, 0, 0);
	} else {
		halbtc8192e2ant_LimitedTx(pBtCoexist, NORMAL_EXEC, 0, 0, 0, 0);
	}

	if(!bWifiConnected) {
		bLowPwrDisable = FALSE;
		pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_DISABLE_LOW_POWER, &bLowPwrDisable);

		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Wifi non-connected idle!!\n"));

		if( (BT_8192E_2ANT_BT_STATUS_NON_CONNECTED_IDLE == pCoexDm->btStatus) ||
		    (BT_8192E_2ANT_BT_STATUS_CONNECTED_IDLE == pCoexDm->btStatus) ) {
			halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 2);
			halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 1);
			halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 0);
		} else {
			halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
			halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 0);
			halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 1);
		}

		halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);

		halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
		halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);

		bCommon = TRUE;
	} else {
		if(BT_8192E_2ANT_BT_STATUS_NON_CONNECTED_IDLE == pCoexDm->btStatus) {
			bLowPwrDisable = FALSE;
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_DISABLE_LOW_POWER, &bLowPwrDisable);

			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Wifi connected + BT non connected-idle!!\n"));

			halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 2);
			halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 1);
			halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 0);
			halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
			halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);

			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);

			bCommon = TRUE;
		} else if(BT_8192E_2ANT_BT_STATUS_CONNECTED_IDLE == pCoexDm->btStatus) {
			bLowPwrDisable = TRUE;
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_DISABLE_LOW_POWER, &bLowPwrDisable);

			if(bBtHsOn)
				return FALSE;
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Wifi connected + BT connected-idle!!\n"));

			halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 2);
			halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 1);
			halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 0);
			halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
			halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);

			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);

			bCommon = TRUE;
		} else {
			bLowPwrDisable = TRUE;
			pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_ACT_DISABLE_LOW_POWER, &bLowPwrDisable);

			if(bWifiBusy) {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Wifi Connected-Busy + BT Busy!!\n"));
				bCommon = FALSE;
			} else {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Wifi Connected-Idle + BT Busy!!\n"));

				halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
				halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 2);
				halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 21);
				halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
				halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
				halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
				halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
				bCommon = TRUE;
			}
		}
	}

	return bCommon;
}
VOID
halbtc8192e2ant_TdmaDurationAdjust(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN			bScoHid,
    IN	BOOLEAN			bTxPause,
    IN	u1Byte			maxInterval
)
{
	static s4Byte		up,dn,m,n,WaitCount;
	s4Byte			result;   //0: no change, +1: increase WiFi duration, -1: decrease WiFi duration
	u1Byte			retryCount=0;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW, ("[BTCoex], TdmaDurationAdjust()\n"));

	if(!pCoexDm->bAutoTdmaAdjust) {
		pCoexDm->bAutoTdmaAdjust = TRUE;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], first run TdmaDurationAdjust()!!\n"));
		{
			if(bScoHid) {
				if(bTxPause) {
					if(maxInterval == 1) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 13);
						pCoexDm->psTdmaDuAdjType = 13;
					} else if(maxInterval == 2) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 14);
						pCoexDm->psTdmaDuAdjType = 14;
					} else if(maxInterval == 3) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
						pCoexDm->psTdmaDuAdjType = 15;
					} else {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
						pCoexDm->psTdmaDuAdjType = 15;
					}
				} else {
					if(maxInterval == 1) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 9);
						pCoexDm->psTdmaDuAdjType = 9;
					} else if(maxInterval == 2) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
						pCoexDm->psTdmaDuAdjType = 10;
					} else if(maxInterval == 3) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
						pCoexDm->psTdmaDuAdjType = 11;
					} else {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
						pCoexDm->psTdmaDuAdjType = 11;
					}
				}
			} else {
				if(bTxPause) {
					if(maxInterval == 1) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 5);
						pCoexDm->psTdmaDuAdjType = 5;
					} else if(maxInterval == 2) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 6);
						pCoexDm->psTdmaDuAdjType = 6;
					} else if(maxInterval == 3) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
						pCoexDm->psTdmaDuAdjType = 7;
					} else {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
						pCoexDm->psTdmaDuAdjType = 7;
					}
				} else {
					if(maxInterval == 1) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 1);
						pCoexDm->psTdmaDuAdjType = 1;
					} else if(maxInterval == 2) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
						pCoexDm->psTdmaDuAdjType = 2;
					} else if(maxInterval == 3) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
						pCoexDm->psTdmaDuAdjType = 3;
					} else {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
						pCoexDm->psTdmaDuAdjType = 3;
					}
				}
			}
		}
		//============
		up = 0;
		dn = 0;
		m = 1;
		n= 3;
		result = 0;
		WaitCount = 0;
	} else {
		//accquire the BT TRx retry count from BT_Info byte2
		retryCount = pCoexSta->btRetryCnt;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], retryCount = %d\n", retryCount));
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], up=%d, dn=%d, m=%d, n=%d, WaitCount=%d\n",
		          up, dn, m, n, WaitCount));
		result = 0;
		WaitCount++;

		if(retryCount == 0) { // no retry in the last 2-second duration
			up++;
			dn--;

			if (dn <= 0)
				dn = 0;

			if(up >= n) {	// if �s�� n ��2�� retry count��0, �h�ռeWiFi duration
				WaitCount = 0;
				n = 3;
				up = 0;
				dn = 0;
				result = 1;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], Increase wifi duration!!\n"));
			}
		} else if (retryCount <= 3) {	// <=3 retry in the last 2-second duration
			up--;
			dn++;

			if (up <= 0)
				up = 0;

			if (dn == 2) {	// if �s�� 2 ��2�� retry count< 3, �h�կ�WiFi duration
				if (WaitCount <= 2)
					m++; // �קK�@���b���level���Ӧ^
				else
					m = 1;

				if ( m >= 20) //m �̤j�� = 20 ' �̤j120�� recheck�O�_�վ� WiFi duration.
					m = 20;

				n = 3*m;
				up = 0;
				dn = 0;
				WaitCount = 0;
				result = -1;
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], Decrease wifi duration for retryCounter<3!!\n"));
			}
		} else { //retry count > 3, �u�n1�� retry count > 3, �h�կ�WiFi duration
			if (WaitCount == 1)
				m++; // �קK�@���b���level���Ӧ^
			else
				m = 1;

			if ( m >= 20) //m �̤j�� = 20 ' �̤j120�� recheck�O�_�վ� WiFi duration.
				m = 20;

			n = 3*m;
			up = 0;
			dn = 0;
			WaitCount = 0;
			result = -1;
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], Decrease wifi duration for retryCounter>3!!\n"));
		}

		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], max Interval = %d\n", maxInterval));
		if(maxInterval == 1) {
			if(bTxPause) {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], TxPause = 1\n"));

				if(pCoexDm->curPsTdma == 71) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 5);
					pCoexDm->psTdmaDuAdjType = 5;
				} else if(pCoexDm->curPsTdma == 1) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 5);
					pCoexDm->psTdmaDuAdjType = 5;
				} else if(pCoexDm->curPsTdma == 2) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 6);
					pCoexDm->psTdmaDuAdjType = 6;
				} else if(pCoexDm->curPsTdma == 3) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
					pCoexDm->psTdmaDuAdjType = 7;
				} else if(pCoexDm->curPsTdma == 4) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 8);
					pCoexDm->psTdmaDuAdjType = 8;
				}
				if(pCoexDm->curPsTdma == 9) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 13);
					pCoexDm->psTdmaDuAdjType = 13;
				} else if(pCoexDm->curPsTdma == 10) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 14);
					pCoexDm->psTdmaDuAdjType = 14;
				} else if(pCoexDm->curPsTdma == 11) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
					pCoexDm->psTdmaDuAdjType = 15;
				} else if(pCoexDm->curPsTdma == 12) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 16);
					pCoexDm->psTdmaDuAdjType = 16;
				}

				if(result == -1) {
					if(pCoexDm->curPsTdma == 5) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 6);
						pCoexDm->psTdmaDuAdjType = 6;
					} else if(pCoexDm->curPsTdma == 6) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
						pCoexDm->psTdmaDuAdjType = 7;
					} else if(pCoexDm->curPsTdma == 7) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 8);
						pCoexDm->psTdmaDuAdjType = 8;
					} else if(pCoexDm->curPsTdma == 13) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 14);
						pCoexDm->psTdmaDuAdjType = 14;
					} else if(pCoexDm->curPsTdma == 14) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
						pCoexDm->psTdmaDuAdjType = 15;
					} else if(pCoexDm->curPsTdma == 15) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 16);
						pCoexDm->psTdmaDuAdjType = 16;
					}
				} else if (result == 1) {
					if(pCoexDm->curPsTdma == 8) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
						pCoexDm->psTdmaDuAdjType = 7;
					} else if(pCoexDm->curPsTdma == 7) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 6);
						pCoexDm->psTdmaDuAdjType = 6;
					} else if(pCoexDm->curPsTdma == 6) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 5);
						pCoexDm->psTdmaDuAdjType = 5;
					} else if(pCoexDm->curPsTdma == 16) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
						pCoexDm->psTdmaDuAdjType = 15;
					} else if(pCoexDm->curPsTdma == 15) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 14);
						pCoexDm->psTdmaDuAdjType = 14;
					} else if(pCoexDm->curPsTdma == 14) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 13);
						pCoexDm->psTdmaDuAdjType = 13;
					}
				}
			} else {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], TxPause = 0\n"));
				if(pCoexDm->curPsTdma == 5) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 71);
					pCoexDm->psTdmaDuAdjType = 71;
				} else if(pCoexDm->curPsTdma == 6) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
					pCoexDm->psTdmaDuAdjType = 2;
				} else if(pCoexDm->curPsTdma == 7) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
					pCoexDm->psTdmaDuAdjType = 3;
				} else if(pCoexDm->curPsTdma == 8) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 4);
					pCoexDm->psTdmaDuAdjType = 4;
				}
				if(pCoexDm->curPsTdma == 13) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 9);
					pCoexDm->psTdmaDuAdjType = 9;
				} else if(pCoexDm->curPsTdma == 14) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
					pCoexDm->psTdmaDuAdjType = 10;
				} else if(pCoexDm->curPsTdma == 15) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
					pCoexDm->psTdmaDuAdjType = 11;
				} else if(pCoexDm->curPsTdma == 16) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 12);
					pCoexDm->psTdmaDuAdjType = 12;
				}

				if(result == -1) {
					if(pCoexDm->curPsTdma == 71) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 1);
						pCoexDm->psTdmaDuAdjType = 1;
					} else if(pCoexDm->curPsTdma == 1) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
						pCoexDm->psTdmaDuAdjType = 2;
					} else if(pCoexDm->curPsTdma == 2) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
						pCoexDm->psTdmaDuAdjType = 3;
					} else if(pCoexDm->curPsTdma == 3) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 4);
						pCoexDm->psTdmaDuAdjType = 4;
					} else if(pCoexDm->curPsTdma == 9) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
						pCoexDm->psTdmaDuAdjType = 10;
					} else if(pCoexDm->curPsTdma == 10) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
						pCoexDm->psTdmaDuAdjType = 11;
					} else if(pCoexDm->curPsTdma == 11) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 12);
						pCoexDm->psTdmaDuAdjType = 12;
					}
				} else if (result == 1) {
					if(pCoexDm->curPsTdma == 4) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
						pCoexDm->psTdmaDuAdjType = 3;
					} else if(pCoexDm->curPsTdma == 3) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
						pCoexDm->psTdmaDuAdjType = 2;
					} else if(pCoexDm->curPsTdma == 2) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 1);
						pCoexDm->psTdmaDuAdjType = 1;
					} else if(pCoexDm->curPsTdma == 1) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 71);
						pCoexDm->psTdmaDuAdjType = 71;
					} else if(pCoexDm->curPsTdma == 12) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
						pCoexDm->psTdmaDuAdjType = 11;
					} else if(pCoexDm->curPsTdma == 11) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
						pCoexDm->psTdmaDuAdjType = 10;
					} else if(pCoexDm->curPsTdma == 10) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 9);
						pCoexDm->psTdmaDuAdjType = 9;
					}
				}
			}
		} else if(maxInterval == 2) {
			if(bTxPause) {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], TxPause = 1\n"));
				if(pCoexDm->curPsTdma == 1) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 6);
					pCoexDm->psTdmaDuAdjType = 6;
				} else if(pCoexDm->curPsTdma == 2) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 6);
					pCoexDm->psTdmaDuAdjType = 6;
				} else if(pCoexDm->curPsTdma == 3) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
					pCoexDm->psTdmaDuAdjType = 7;
				} else if(pCoexDm->curPsTdma == 4) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 8);
					pCoexDm->psTdmaDuAdjType = 8;
				}
				if(pCoexDm->curPsTdma == 9) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 14);
					pCoexDm->psTdmaDuAdjType = 14;
				} else if(pCoexDm->curPsTdma == 10) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 14);
					pCoexDm->psTdmaDuAdjType = 14;
				} else if(pCoexDm->curPsTdma == 11) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
					pCoexDm->psTdmaDuAdjType = 15;
				} else if(pCoexDm->curPsTdma == 12) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 16);
					pCoexDm->psTdmaDuAdjType = 16;
				}
				if(result == -1) {
					if(pCoexDm->curPsTdma == 5) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 6);
						pCoexDm->psTdmaDuAdjType = 6;
					} else if(pCoexDm->curPsTdma == 6) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
						pCoexDm->psTdmaDuAdjType = 7;
					} else if(pCoexDm->curPsTdma == 7) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 8);
						pCoexDm->psTdmaDuAdjType = 8;
					} else if(pCoexDm->curPsTdma == 13) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 14);
						pCoexDm->psTdmaDuAdjType = 14;
					} else if(pCoexDm->curPsTdma == 14) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
						pCoexDm->psTdmaDuAdjType = 15;
					} else if(pCoexDm->curPsTdma == 15) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 16);
						pCoexDm->psTdmaDuAdjType = 16;
					}
				} else if (result == 1) {
					if(pCoexDm->curPsTdma == 8) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
						pCoexDm->psTdmaDuAdjType = 7;
					} else if(pCoexDm->curPsTdma == 7) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 6);
						pCoexDm->psTdmaDuAdjType = 6;
					} else if(pCoexDm->curPsTdma == 6) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 6);
						pCoexDm->psTdmaDuAdjType = 6;
					} else if(pCoexDm->curPsTdma == 16) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
						pCoexDm->psTdmaDuAdjType = 15;
					} else if(pCoexDm->curPsTdma == 15) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 14);
						pCoexDm->psTdmaDuAdjType = 14;
					} else if(pCoexDm->curPsTdma == 14) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 14);
						pCoexDm->psTdmaDuAdjType = 14;
					}
				}
			} else {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], TxPause = 0\n"));
				if(pCoexDm->curPsTdma == 5) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
					pCoexDm->psTdmaDuAdjType = 2;
				} else if(pCoexDm->curPsTdma == 6) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
					pCoexDm->psTdmaDuAdjType = 2;
				} else if(pCoexDm->curPsTdma == 7) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
					pCoexDm->psTdmaDuAdjType = 3;
				} else if(pCoexDm->curPsTdma == 8) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 4);
					pCoexDm->psTdmaDuAdjType = 4;
				}
				if(pCoexDm->curPsTdma == 13) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
					pCoexDm->psTdmaDuAdjType = 10;
				} else if(pCoexDm->curPsTdma == 14) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
					pCoexDm->psTdmaDuAdjType = 10;
				} else if(pCoexDm->curPsTdma == 15) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
					pCoexDm->psTdmaDuAdjType = 11;
				} else if(pCoexDm->curPsTdma == 16) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 12);
					pCoexDm->psTdmaDuAdjType = 12;
				}
				if(result == -1) {
					if(pCoexDm->curPsTdma == 1) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
						pCoexDm->psTdmaDuAdjType = 2;
					} else if(pCoexDm->curPsTdma == 2) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
						pCoexDm->psTdmaDuAdjType = 3;
					} else if(pCoexDm->curPsTdma == 3) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 4);
						pCoexDm->psTdmaDuAdjType = 4;
					} else if(pCoexDm->curPsTdma == 9) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
						pCoexDm->psTdmaDuAdjType = 10;
					} else if(pCoexDm->curPsTdma == 10) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
						pCoexDm->psTdmaDuAdjType = 11;
					} else if(pCoexDm->curPsTdma == 11) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 12);
						pCoexDm->psTdmaDuAdjType = 12;
					}
				} else if (result == 1) {
					if(pCoexDm->curPsTdma == 4) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
						pCoexDm->psTdmaDuAdjType = 3;
					} else if(pCoexDm->curPsTdma == 3) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
						pCoexDm->psTdmaDuAdjType = 2;
					} else if(pCoexDm->curPsTdma == 2) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 2);
						pCoexDm->psTdmaDuAdjType = 2;
					} else if(pCoexDm->curPsTdma == 12) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
						pCoexDm->psTdmaDuAdjType = 11;
					} else if(pCoexDm->curPsTdma == 11) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
						pCoexDm->psTdmaDuAdjType = 10;
					} else if(pCoexDm->curPsTdma == 10) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
						pCoexDm->psTdmaDuAdjType = 10;
					}
				}
			}
		} else if(maxInterval == 3) {
			if(bTxPause) {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], TxPause = 1\n"));
				if(pCoexDm->curPsTdma == 1) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
					pCoexDm->psTdmaDuAdjType = 7;
				} else if(pCoexDm->curPsTdma == 2) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
					pCoexDm->psTdmaDuAdjType = 7;
				} else if(pCoexDm->curPsTdma == 3) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
					pCoexDm->psTdmaDuAdjType = 7;
				} else if(pCoexDm->curPsTdma == 4) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 8);
					pCoexDm->psTdmaDuAdjType = 8;
				}
				if(pCoexDm->curPsTdma == 9) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
					pCoexDm->psTdmaDuAdjType = 15;
				} else if(pCoexDm->curPsTdma == 10) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
					pCoexDm->psTdmaDuAdjType = 15;
				} else if(pCoexDm->curPsTdma == 11) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
					pCoexDm->psTdmaDuAdjType = 15;
				} else if(pCoexDm->curPsTdma == 12) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 16);
					pCoexDm->psTdmaDuAdjType = 16;
				}
				if(result == -1) {
					if(pCoexDm->curPsTdma == 5) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
						pCoexDm->psTdmaDuAdjType = 7;
					} else if(pCoexDm->curPsTdma == 6) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
						pCoexDm->psTdmaDuAdjType = 7;
					} else if(pCoexDm->curPsTdma == 7) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 8);
						pCoexDm->psTdmaDuAdjType = 8;
					} else if(pCoexDm->curPsTdma == 13) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
						pCoexDm->psTdmaDuAdjType = 15;
					} else if(pCoexDm->curPsTdma == 14) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
						pCoexDm->psTdmaDuAdjType = 15;
					} else if(pCoexDm->curPsTdma == 15) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 16);
						pCoexDm->psTdmaDuAdjType = 16;
					}
				} else if (result == 1) {
					if(pCoexDm->curPsTdma == 8) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
						pCoexDm->psTdmaDuAdjType = 7;
					} else if(pCoexDm->curPsTdma == 7) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
						pCoexDm->psTdmaDuAdjType = 7;
					} else if(pCoexDm->curPsTdma == 6) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 7);
						pCoexDm->psTdmaDuAdjType = 7;
					} else if(pCoexDm->curPsTdma == 16) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
						pCoexDm->psTdmaDuAdjType = 15;
					} else if(pCoexDm->curPsTdma == 15) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
						pCoexDm->psTdmaDuAdjType = 15;
					} else if(pCoexDm->curPsTdma == 14) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 15);
						pCoexDm->psTdmaDuAdjType = 15;
					}
				}
			} else {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], TxPause = 0\n"));
				if(pCoexDm->curPsTdma == 5) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
					pCoexDm->psTdmaDuAdjType = 3;
				} else if(pCoexDm->curPsTdma == 6) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
					pCoexDm->psTdmaDuAdjType = 3;
				} else if(pCoexDm->curPsTdma == 7) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
					pCoexDm->psTdmaDuAdjType = 3;
				} else if(pCoexDm->curPsTdma == 8) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 4);
					pCoexDm->psTdmaDuAdjType = 4;
				}
				if(pCoexDm->curPsTdma == 13) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
					pCoexDm->psTdmaDuAdjType = 11;
				} else if(pCoexDm->curPsTdma == 14) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
					pCoexDm->psTdmaDuAdjType = 11;
				} else if(pCoexDm->curPsTdma == 15) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
					pCoexDm->psTdmaDuAdjType = 11;
				} else if(pCoexDm->curPsTdma == 16) {
					halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 12);
					pCoexDm->psTdmaDuAdjType = 12;
				}
				if(result == -1) {
					if(pCoexDm->curPsTdma == 1) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
						pCoexDm->psTdmaDuAdjType = 3;
					} else if(pCoexDm->curPsTdma == 2) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
						pCoexDm->psTdmaDuAdjType = 3;
					} else if(pCoexDm->curPsTdma == 3) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 4);
						pCoexDm->psTdmaDuAdjType = 4;
					} else if(pCoexDm->curPsTdma == 9) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
						pCoexDm->psTdmaDuAdjType = 11;
					} else if(pCoexDm->curPsTdma == 10) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
						pCoexDm->psTdmaDuAdjType = 11;
					} else if(pCoexDm->curPsTdma == 11) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 12);
						pCoexDm->psTdmaDuAdjType = 12;
					}
				} else if (result == 1) {
					if(pCoexDm->curPsTdma == 4) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
						pCoexDm->psTdmaDuAdjType = 3;
					} else if(pCoexDm->curPsTdma == 3) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
						pCoexDm->psTdmaDuAdjType = 3;
					} else if(pCoexDm->curPsTdma == 2) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 3);
						pCoexDm->psTdmaDuAdjType = 3;
					} else if(pCoexDm->curPsTdma == 12) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
						pCoexDm->psTdmaDuAdjType = 11;
					} else if(pCoexDm->curPsTdma == 11) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
						pCoexDm->psTdmaDuAdjType = 11;
					} else if(pCoexDm->curPsTdma == 10) {
						halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 11);
						pCoexDm->psTdmaDuAdjType = 11;
					}
				}
			}
		}
	}

	// if current PsTdma not match with the recorded one (when scan, dhcp...),
	// then we have to adjust it back to the previous record one.
	if(pCoexDm->curPsTdma != pCoexDm->psTdmaDuAdjType) {
		BOOLEAN	bScan=FALSE, bLink=FALSE, bRoam=FALSE;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], PsTdma type dismatch!!!, curPsTdma=%d, recordPsTdma=%d\n",
		          pCoexDm->curPsTdma, pCoexDm->psTdmaDuAdjType));

		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_SCAN, &bScan);
		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_LINK, &bLink);
		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_ROAM, &bRoam);

		if( !bScan && !bLink && !bRoam) {
			halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, pCoexDm->psTdmaDuAdjType);
		} else {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_DETAIL, ("[BTCoex], roaming/link/scan is under progress, will adjust next time!!!\n"));
		}
	}
}

// SCO only or SCO+PAN(HS)
VOID
halbtc8192e2ant_ActionSco(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte	wifiRssiState, btRssiState=BTC_RSSI_STATE_STAY_LOW;
	u4Byte	wifiBw;

	wifiRssiState = halbtc8192e2ant_WifiRssiState(pBtCoexist, 0, 2, 15, 0);

	halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
	halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, FALSE, 0x8);

	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 4);

	btRssiState = halbtc8192e2ant_BtRssiState(3, 34, 42);

	if( (btRssiState == BTC_RSSI_STATE_LOW) ||
	    (btRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 13);
	} else if( (btRssiState == BTC_RSSI_STATE_MEDIUM) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_MEDIUM) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 2);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 9);
	} else if( (btRssiState == BTC_RSSI_STATE_HIGH) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 4);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 9);
	}

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	// sw mechanism
	if(BTC_WIFI_BW_HT40 == wifiBw) {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x6);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x6);
		}
	} else {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x6);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x6);
		}
	}
}

VOID
halbtc8192e2ant_ActionScoPan(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte	wifiRssiState, btRssiState=BTC_RSSI_STATE_STAY_LOW;
	u4Byte	wifiBw;

	wifiRssiState = halbtc8192e2ant_WifiRssiState(pBtCoexist, 0, 2, 15, 0);

	halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
	halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, FALSE, 0x8);

	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 4);

	btRssiState = halbtc8192e2ant_BtRssiState(3, 34, 42);

	if( (btRssiState == BTC_RSSI_STATE_LOW) ||
	    (btRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 14);
	} else if( (btRssiState == BTC_RSSI_STATE_MEDIUM) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_MEDIUM) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 2);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
	} else if( (btRssiState == BTC_RSSI_STATE_HIGH) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 4);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
	}

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	// sw mechanism
	if(BTC_WIFI_BW_HT40 == wifiBw) {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x6);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x6);
		}
	} else {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x6);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x6);
		}
	}
}

VOID
halbtc8192e2ant_ActionHid(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte	wifiRssiState, btRssiState=BTC_RSSI_STATE_HIGH;
	u4Byte	wifiBw;

	wifiRssiState = halbtc8192e2ant_WifiRssiState(pBtCoexist, 0, 2, 15, 0);
	btRssiState = halbtc8192e2ant_BtRssiState(3, 34, 42);

	halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
	halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, FALSE, 0x8);

	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 3);

	if( (btRssiState == BTC_RSSI_STATE_LOW) ||
	    (btRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 13);
	} else if( (btRssiState == BTC_RSSI_STATE_MEDIUM) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_MEDIUM) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 2);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 9);
	} else if( (btRssiState == BTC_RSSI_STATE_HIGH) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 4);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 9);
	}

	// sw mechanism
	if(BTC_WIFI_BW_HT40 == wifiBw) {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	} else {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

//A2DP only / PAN(EDR) only/ A2DP+PAN(HS)
VOID
halbtc8192e2ant_ActionA2dp(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte		wifiRssiState, btRssiState=BTC_RSSI_STATE_HIGH;
	u4Byte		wifiBw;
	BOOLEAN		bLongDist=FALSE;

	wifiRssiState = halbtc8192e2ant_WifiRssiState(pBtCoexist, 0, 2, 15, 0);
	btRssiState = halbtc8192e2ant_BtRssiState(3, 34, 42);

	if( (btRssiState == BTC_RSSI_STATE_LOW||btRssiState == BTC_RSSI_STATE_STAY_LOW) &&
	    (wifiRssiState == BTC_RSSI_STATE_LOW||wifiRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], A2dp, wifi/bt rssi both LOW!!\n"));
		bLongDist = TRUE;
	}
	if(bLongDist) {
		halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 2);
		halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, TRUE, 0x4);
	} else {
		halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
		halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, FALSE, 0x8);
	}

	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	if(bLongDist)
		halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 0);
	else
		halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 2);


	if(bLongDist) {
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 17);
		pCoexDm->bAutoTdmaAdjust = FALSE;
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
	} else {
		if( (btRssiState == BTC_RSSI_STATE_LOW) ||
		    (btRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
			halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, FALSE, TRUE, 1);
			halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
		} else if( (btRssiState == BTC_RSSI_STATE_MEDIUM) ||
		           (btRssiState == BTC_RSSI_STATE_STAY_MEDIUM) ) {
			halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, FALSE, FALSE, 1);
			halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 2);
		} else if( (btRssiState == BTC_RSSI_STATE_HIGH) ||
		           (btRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, FALSE, FALSE, 1);
			halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 4);
		}
	}

	// sw mechanism
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);
	if(BTC_WIFI_BW_HT40 == wifiBw) {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	} else {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

VOID
halbtc8192e2ant_ActionA2dpPanHs(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte		wifiRssiState, btRssiState=BTC_RSSI_STATE_HIGH;
	u4Byte		wifiBw;

	wifiRssiState = halbtc8192e2ant_WifiRssiState(pBtCoexist, 0, 2, 15, 0);
	btRssiState = halbtc8192e2ant_BtRssiState(3, 34, 42);

	halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
	halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, FALSE, 0x8);

	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);
	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 2);

	if( (btRssiState == BTC_RSSI_STATE_LOW) ||
	    (btRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, FALSE, TRUE, 2);
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
	} else if( (btRssiState == BTC_RSSI_STATE_MEDIUM) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_MEDIUM) ) {
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, FALSE, FALSE, 2);
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 2);
	} else if( (btRssiState == BTC_RSSI_STATE_HIGH) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, FALSE, FALSE, 2);
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 4);
	}

	// sw mechanism
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);
	if(BTC_WIFI_BW_HT40 == wifiBw) {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,TRUE,0x6);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,TRUE,0x6);
		}
	} else {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,TRUE,0x6);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,TRUE,0x6);
		}
	}
}

VOID
halbtc8192e2ant_ActionPanEdr(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte		wifiRssiState, btRssiState=BTC_RSSI_STATE_HIGH;
	u4Byte		wifiBw;

	wifiRssiState = halbtc8192e2ant_WifiRssiState(pBtCoexist, 0, 2, 15, 0);
	btRssiState = halbtc8192e2ant_BtRssiState(3, 34, 42);

	halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
	halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, FALSE, 0x8);

	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 2);

	if( (btRssiState == BTC_RSSI_STATE_LOW) ||
	    (btRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 5);
	} else if( (btRssiState == BTC_RSSI_STATE_MEDIUM) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_MEDIUM) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 2);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 1);
	} else if( (btRssiState == BTC_RSSI_STATE_HIGH) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 4);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 1);
	}

	// sw mechanism
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);
	if(BTC_WIFI_BW_HT40 == wifiBw) {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	} else {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

//PAN(HS) only
VOID
halbtc8192e2ant_ActionPanHs(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte		wifiRssiState, btRssiState=BTC_RSSI_STATE_HIGH;
	u4Byte		wifiBw;

	wifiRssiState = halbtc8192e2ant_WifiRssiState(pBtCoexist, 0, 2, 15, 0);
	btRssiState = halbtc8192e2ant_BtRssiState(3, 34, 42);

	halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
	halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, FALSE, 0x8);

	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 2);

	if( (btRssiState == BTC_RSSI_STATE_LOW) ||
	    (btRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
	} else if( (btRssiState == BTC_RSSI_STATE_MEDIUM) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_MEDIUM) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 2);
	} else if( (btRssiState == BTC_RSSI_STATE_HIGH) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 4);
	}
	halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, FALSE, 1);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);
	if(BTC_WIFI_BW_HT40 == wifiBw) {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	} else {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

//PAN(EDR)+A2DP
VOID
halbtc8192e2ant_ActionPanEdrA2dp(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte		wifiRssiState, btRssiState=BTC_RSSI_STATE_HIGH;
	u4Byte		wifiBw;

	wifiRssiState = halbtc8192e2ant_WifiRssiState(pBtCoexist, 0, 2, 15, 0);
	btRssiState = halbtc8192e2ant_BtRssiState(3, 34, 42);

	halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
	halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, FALSE, 0x8);

	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 2);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	if( (btRssiState == BTC_RSSI_STATE_LOW) ||
	    (btRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, FALSE, TRUE, 3);
	} else if( (btRssiState == BTC_RSSI_STATE_MEDIUM) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_MEDIUM) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 2);
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, FALSE, FALSE, 3);
	} else if( (btRssiState == BTC_RSSI_STATE_HIGH) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 4);
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, FALSE, FALSE, 3);
	}

	// sw mechanism
	if(BTC_WIFI_BW_HT40 == wifiBw) {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	} else {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,FALSE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

VOID
halbtc8192e2ant_ActionPanEdrHid(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte		wifiRssiState, btRssiState=BTC_RSSI_STATE_HIGH;
	u4Byte		wifiBw;

	wifiRssiState = halbtc8192e2ant_WifiRssiState(pBtCoexist, 0, 2, 15, 0);
	btRssiState = halbtc8192e2ant_BtRssiState(3, 34, 42);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
	halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, FALSE, 0x8);

	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 3);

	if( (btRssiState == BTC_RSSI_STATE_LOW) ||
	    (btRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 14);
	} else if( (btRssiState == BTC_RSSI_STATE_MEDIUM) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_MEDIUM) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 2);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
	} else if( (btRssiState == BTC_RSSI_STATE_HIGH) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 4);
		halbtc8192e2ant_PsTdma(pBtCoexist, NORMAL_EXEC, TRUE, 10);
	}

	// sw mechanism
	if(BTC_WIFI_BW_HT40 == wifiBw) {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	} else {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

// HID+A2DP+PAN(EDR)
VOID
halbtc8192e2ant_ActionHidA2dpPanEdr(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte		wifiRssiState, btRssiState=BTC_RSSI_STATE_HIGH;
	u4Byte		wifiBw;

	wifiRssiState = halbtc8192e2ant_WifiRssiState(pBtCoexist, 0, 2, 15, 0);
	btRssiState = halbtc8192e2ant_BtRssiState(3, 34, 42);

	halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
	halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, FALSE, 0x8);

	halbtc8192e2ant_FwDacSwingLvl(pBtCoexist, NORMAL_EXEC, 6);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 3);

	if( (btRssiState == BTC_RSSI_STATE_LOW) ||
	    (btRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, TRUE, TRUE, 3);
	} else if( (btRssiState == BTC_RSSI_STATE_MEDIUM) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_MEDIUM) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 2);
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, TRUE, FALSE, 3);
	} else if( (btRssiState == BTC_RSSI_STATE_HIGH) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 4);
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, TRUE, FALSE, 3);
	}

	// sw mechanism
	if(BTC_WIFI_BW_HT40 == wifiBw) {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	} else {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

VOID
halbtc8192e2ant_ActionHidA2dp(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	u1Byte		wifiRssiState, btRssiState=BTC_RSSI_STATE_HIGH;
	u4Byte		wifiBw;

	wifiRssiState = halbtc8192e2ant_WifiRssiState(pBtCoexist, 0, 2, 15, 0);
	btRssiState = halbtc8192e2ant_BtRssiState(3, 34, 42);

	halbtc8192e2ant_SwitchSsType(pBtCoexist, NORMAL_EXEC, 1);
	halbtc8192e2ant_LimitedRx(pBtCoexist, NORMAL_EXEC, FALSE, FALSE, 0x8);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, NORMAL_EXEC, 3);

	if( (btRssiState == BTC_RSSI_STATE_LOW) ||
	    (btRssiState == BTC_RSSI_STATE_STAY_LOW) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 0);
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, TRUE, TRUE, 2);
	} else if( (btRssiState == BTC_RSSI_STATE_MEDIUM) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_MEDIUM) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 2);
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, TRUE, FALSE, 2);
	} else if( (btRssiState == BTC_RSSI_STATE_HIGH) ||
	           (btRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
		halbtc8192e2ant_DecBtPwr(pBtCoexist, NORMAL_EXEC, 4);
		halbtc8192e2ant_TdmaDurationAdjust(pBtCoexist, TRUE, FALSE, 2);
	}

	// sw mechanism
	if(BTC_WIFI_BW_HT40 == wifiBw) {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,TRUE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	} else {
		if( (wifiRssiState == BTC_RSSI_STATE_HIGH) ||
		    (wifiRssiState == BTC_RSSI_STATE_STAY_HIGH) ) {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,TRUE,FALSE,FALSE,0x18);
		} else {
			halbtc8192e2ant_SwMechanism1(pBtCoexist,FALSE,TRUE,FALSE,FALSE);
			halbtc8192e2ant_SwMechanism2(pBtCoexist,FALSE,FALSE,FALSE,0x18);
		}
	}
}

VOID
halbtc8192e2ant_RunCoexistMechanism(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	//BOOLEAN				bWifiUnder5G=FALSE;
	//u1Byte				btInfoOriginal=0, btRetryCnt=0;
	u1Byte				algorithm=0;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], RunCoexistMechanism()===>\n"));

	if(pBtCoexist->bManualControl) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], RunCoexistMechanism(), return for Manual CTRL <===\n"));
		return;
	}

	if(pCoexSta->bUnderIps) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], wifi is under IPS !!!\n"));
		return;
	}

	algorithm = halbtc8192e2ant_ActionAlgorithm(pBtCoexist);
	if(pCoexSta->bC2hBtInquiryPage && (BT_8192E_2ANT_COEX_ALGO_PANHS!=algorithm)) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BT is under inquiry/page scan !!\n"));
		halbtc8192e2ant_ActionBtInquiry(pBtCoexist);
		return;
	}

	pCoexDm->curAlgorithm = algorithm;
	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Algorithm = %d \n", pCoexDm->curAlgorithm));

	if(halbtc8192e2ant_IsCommonAction(pBtCoexist)) {
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant common.\n"));
		pCoexDm->bAutoTdmaAdjust = FALSE;
	} else {
		if(pCoexDm->curAlgorithm != pCoexDm->preAlgorithm) {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], preAlgorithm=%d, curAlgorithm=%d\n",
			          pCoexDm->preAlgorithm, pCoexDm->curAlgorithm));
			pCoexDm->bAutoTdmaAdjust = FALSE;
		}
		switch(pCoexDm->curAlgorithm) {
		case BT_8192E_2ANT_COEX_ALGO_SCO:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = SCO.\n"));
			halbtc8192e2ant_ActionSco(pBtCoexist);
			break;
		case BT_8192E_2ANT_COEX_ALGO_SCO_PAN:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = SCO+PAN(EDR).\n"));
			halbtc8192e2ant_ActionScoPan(pBtCoexist);
			break;
		case BT_8192E_2ANT_COEX_ALGO_HID:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = HID.\n"));
			halbtc8192e2ant_ActionHid(pBtCoexist);
			break;
		case BT_8192E_2ANT_COEX_ALGO_A2DP:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = A2DP.\n"));
			halbtc8192e2ant_ActionA2dp(pBtCoexist);
			break;
		case BT_8192E_2ANT_COEX_ALGO_A2DP_PANHS:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = A2DP+PAN(HS).\n"));
			halbtc8192e2ant_ActionA2dpPanHs(pBtCoexist);
			break;
		case BT_8192E_2ANT_COEX_ALGO_PANEDR:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = PAN(EDR).\n"));
			halbtc8192e2ant_ActionPanEdr(pBtCoexist);
			break;
		case BT_8192E_2ANT_COEX_ALGO_PANHS:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = HS mode.\n"));
			halbtc8192e2ant_ActionPanHs(pBtCoexist);
			break;
		case BT_8192E_2ANT_COEX_ALGO_PANEDR_A2DP:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = PAN+A2DP.\n"));
			halbtc8192e2ant_ActionPanEdrA2dp(pBtCoexist);
			break;
		case BT_8192E_2ANT_COEX_ALGO_PANEDR_HID:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = PAN(EDR)+HID.\n"));
			halbtc8192e2ant_ActionPanEdrHid(pBtCoexist);
			break;
		case BT_8192E_2ANT_COEX_ALGO_HID_A2DP_PANEDR:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = HID+A2DP+PAN.\n"));
			halbtc8192e2ant_ActionHidA2dpPanEdr(pBtCoexist);
			break;
		case BT_8192E_2ANT_COEX_ALGO_HID_A2DP:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = HID+A2DP.\n"));
			halbtc8192e2ant_ActionHidA2dp(pBtCoexist);
			break;
		default:
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], Action 2-Ant, algorithm = unknown!!\n"));
			//halbtc8192e2ant_CoexAllOff(pBtCoexist);
			break;
		}
		pCoexDm->preAlgorithm = pCoexDm->curAlgorithm;
	}
}

VOID
halbtc8192e2ant_InitHwConfig(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN				bBackUp
)
{
	//u4Byte	u4Tmp=0;
	u2Byte	u2Tmp=0;
	u1Byte	u1Tmp=0;

	BTC_PRINT(BTC_MSG_INTERFACE, INTF_INIT, ("[BTCoex], 2Ant Init HW Config!!\n"));

	if(bBackUp) {
		// backup rf 0x1e value
		pCoexDm->btRf0x1eBackup =
		    pBtCoexist->fBtcGetRfReg(pBtCoexist, BTC_RF_A, 0x1e, 0xfffff);

		pCoexDm->backupArfrCnt1 = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x430);
		pCoexDm->backupArfrCnt2 = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x434);
		pCoexDm->backupRetryLimit = pBtCoexist->fBtcRead2Byte(pBtCoexist, 0x42a);
		pCoexDm->backupAmpduMaxTime = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x456);
	}

	// antenna sw ctrl to bt
	halbtc8192e2ant_SetAntPath(pBtCoexist, BTC_ANT_PATH_BT, TRUE, FALSE);

	halbtc8192e2ant_CoexTableWithType(pBtCoexist, FORCE_EXEC, 0);

	// antenna switch control parameter
	pBtCoexist->fBtcWrite4Byte(pBtCoexist, 0x858, 0x55555555);

	// coex parameters
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x778, 0x3);
	// 0x790[5:0]=0x5
	u1Tmp = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x790);
	u1Tmp &= 0xc0;
	u1Tmp |= 0x5;
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x790, u1Tmp);

	// enable counter statistics
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x76e, 0x4);

	// enable PTA
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x40, 0x20);
	// enable mailbox interface
	u2Tmp = pBtCoexist->fBtcRead2Byte(pBtCoexist, 0x40);
	u2Tmp |= BIT9;
	pBtCoexist->fBtcWrite2Byte(pBtCoexist, 0x40, u2Tmp);

	// enable PTA I2C mailbox
	u1Tmp = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x101);
	u1Tmp |= BIT4;
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x101, u1Tmp);

	// enable bt clock when wifi is disabled.
	u1Tmp = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x93);
	u1Tmp |= BIT0;
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x93, u1Tmp);
	// enable bt clock when suspend.
	u1Tmp = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x7);
	u1Tmp |= BIT0;
	pBtCoexist->fBtcWrite1Byte(pBtCoexist, 0x7, u1Tmp);
}

//============================================================
// work around function start with wa_halbtc8192e2ant_
//============================================================
//============================================================
// extern function start with EXhalbtc8192e2ant_
//============================================================
VOID
EXhalbtc8192e2ant_PowerOnSetting(
    IN	PBTC_COEXIST		pBtCoexist
)
{
}

VOID
EXhalbtc8192e2ant_InitHwConfig(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	BOOLEAN				bWifiOnly
)
{
	halbtc8192e2ant_InitHwConfig(pBtCoexist, TRUE);
}

VOID
EXhalbtc8192e2ant_InitCoexDm(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	BTC_PRINT(BTC_MSG_INTERFACE, INTF_INIT, ("[BTCoex], Coex Mechanism Init!!\n"));

	halbtc8192e2ant_InitCoexDm(pBtCoexist);
}

VOID
EXhalbtc8192e2ant_DisplayCoexInfo(
    IN	PBTC_COEXIST		pBtCoexist
)
{
	PBTC_BOARD_INFO		pBoardInfo=&pBtCoexist->boardInfo;
	PBTC_STACK_INFO		pStackInfo=&pBtCoexist->stackInfo;
	pu1Byte				cliBuf=pBtCoexist->cliBuf;
	u1Byte				u1Tmp[4], i, btInfoExt, psTdmaCase=0;
	u2Byte				u2Tmp[4];
	u4Byte				u4Tmp[4];
	//u4Byte				faOfdm, faCck;
	u4Byte				fwVer=0, btPatchVer=0;

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n ============[BT Coexist info]============");
	CL_PRINTF(cliBuf);

	if(pBtCoexist->bManualControl) {
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n ============[Under Manual Control]============");
		CL_PRINTF(cliBuf);
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n ==========================================");
		CL_PRINTF(cliBuf);
	}

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d ", "Ant PG number/ Ant mechanism:", \
	           pBoardInfo->pgAntNum, pBoardInfo->btdmAntNum);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s / %d", "BT stack/ hci ext ver", \
	           ((pStackInfo->bProfileNotified)? "Yes":"No"), pStackInfo->hciVersion);
	CL_PRINTF(cliBuf);

	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_BT_PATCH_VER, &btPatchVer);
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_FW_VER, &fwVer);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d_%d/ 0x%x/ 0x%x(%d)", "CoexVer/ FwVer/ PatchVer", \
	           GLCoexVerDate8192e2Ant, GLCoexVer8192e2Ant, fwVer, btPatchVer, btPatchVer);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %02x %02x %02x ", "Wifi channel informed to BT", \
	           pCoexDm->wifiChnlInfo[0], pCoexDm->wifiChnlInfo[1],
	           pCoexDm->wifiChnlInfo[2]);
	CL_PRINTF(cliBuf);

	// wifi status
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Wifi Status]============");
	CL_PRINTF(cliBuf);
	pBtCoexist->fBtcDispDbgMsg(pBtCoexist, BTC_DBG_DISP_WIFI_STATUS);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[BT Status]============");
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = [%s/ %d/ %d] ", "BT [status/ rssi/ retryCnt]", \
	           ((pBtCoexist->btInfo.bBtDisabled)? ("disabled"):	((pCoexSta->bC2hBtInquiryPage)?("inquiry/page scan"):((BT_8192E_2ANT_BT_STATUS_NON_CONNECTED_IDLE == pCoexDm->btStatus)? "non-connected idle":
	                   (  (BT_8192E_2ANT_BT_STATUS_CONNECTED_IDLE == pCoexDm->btStatus)? "connected-idle":"busy")))),
	           pCoexSta->btRssi, pCoexSta->btRetryCnt);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d / %d / %d / %d", "SCO/HID/PAN/A2DP", \
	           pStackInfo->bScoExist, pStackInfo->bHidExist, pStackInfo->bPanExist, pStackInfo->bA2dpExist);
	CL_PRINTF(cliBuf);
	pBtCoexist->fBtcDispDbgMsg(pBtCoexist, BTC_DBG_DISP_BT_LINK_INFO);

	btInfoExt = pCoexSta->btInfoExt;
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s", "BT Info A2DP rate", \
	           (btInfoExt&BIT0)? "Basic rate":"EDR rate");
	CL_PRINTF(cliBuf);

	for(i=0; i<BT_INFO_SRC_8192E_2ANT_MAX; i++) {
		if(pCoexSta->btInfoC2hCnt[i]) {
			CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %02x %02x %02x %02x %02x %02x %02x(%d)", GLBtInfoSrc8192e2Ant[i], \
			           pCoexSta->btInfoC2h[i][0], pCoexSta->btInfoC2h[i][1],
			           pCoexSta->btInfoC2h[i][2], pCoexSta->btInfoC2h[i][3],
			           pCoexSta->btInfoC2h[i][4], pCoexSta->btInfoC2h[i][5],
			           pCoexSta->btInfoC2h[i][6], pCoexSta->btInfoC2hCnt[i]);
			CL_PRINTF(cliBuf);
		}
	}

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x ", "SS Type", \
	           pCoexDm->curSsType);
	CL_PRINTF(cliBuf);

	// Sw mechanism
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Sw mechanism]============");
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d ", "SM1[ShRf/ LpRA/ LimDig]", \
	           pCoexDm->bCurRfRxLpfShrink, pCoexDm->bCurLowPenaltyRa, pCoexDm->bLimitedDig);
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d(0x%x) ", "SM2[AgcT/ AdcB/ SwDacSwing(lvl)]", \
	           pCoexDm->bCurAgcTableEn, pCoexDm->bCurAdcBackOff, pCoexDm->bCurDacSwingOn, pCoexDm->curDacSwingLvl);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x ", "Rate Mask", \
	           pBtCoexist->btInfo.raMask);
	CL_PRINTF(cliBuf);

	// Fw mechanism
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Fw mechanism]============");
	CL_PRINTF(cliBuf);

	psTdmaCase = pCoexDm->curPsTdma;
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %02x %02x %02x %02x %02x case-%d (auto:%d)", "PS TDMA", \
	           pCoexDm->psTdmaPara[0], pCoexDm->psTdmaPara[1],
	           pCoexDm->psTdmaPara[2], pCoexDm->psTdmaPara[3],
	           pCoexDm->psTdmaPara[4], psTdmaCase, pCoexDm->bAutoTdmaAdjust);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d ", "DecBtPwr/ IgnWlanAct", \
	           pCoexDm->curBtDecPwrLvl, pCoexDm->bCurIgnoreWlanAct);
	CL_PRINTF(cliBuf);

	// Hw setting
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Hw setting]============");
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x", "RF-A, 0x1e initVal", \
	           pCoexDm->btRf0x1eBackup);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/0x%x/0x%x/0x%x", "backup ARFR1/ARFR2/RL/AMaxTime", \
	           pCoexDm->backupArfrCnt1, pCoexDm->backupArfrCnt2, pCoexDm->backupRetryLimit, pCoexDm->backupAmpduMaxTime);
	CL_PRINTF(cliBuf);

	u4Tmp[0] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x430);
	u4Tmp[1] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x434);
	u2Tmp[0] = pBtCoexist->fBtcRead2Byte(pBtCoexist, 0x42a);
	u1Tmp[0] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x456);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/0x%x/0x%x/0x%x", "0x430/0x434/0x42a/0x456", \
	           u4Tmp[0], u4Tmp[1], u2Tmp[0], u1Tmp[0]);
	CL_PRINTF(cliBuf);

	u4Tmp[0] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0xc04);
	u4Tmp[1] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0xd04);
	u4Tmp[2] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x90c);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x/ 0x%x", "0xc04/ 0xd04/ 0x90c", \
	           u4Tmp[0], u4Tmp[1], u4Tmp[2]);
	CL_PRINTF(cliBuf);

	u1Tmp[0] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x778);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x", "0x778", \
	           u1Tmp[0]);
	CL_PRINTF(cliBuf);

	u1Tmp[0] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x92c);
	u4Tmp[0] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x930);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x", "0x92c/ 0x930", \
	           (u1Tmp[0]), u4Tmp[0]);
	CL_PRINTF(cliBuf);

	u1Tmp[0] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x40);
	u1Tmp[1] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x4f);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x", "0x40/ 0x4f", \
	           u1Tmp[0], u1Tmp[1]);
	CL_PRINTF(cliBuf);

	u4Tmp[0] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x550);
	u1Tmp[0] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x522);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x", "0x550(bcn ctrl)/0x522", \
	           u4Tmp[0], u1Tmp[0]);
	CL_PRINTF(cliBuf);

	u4Tmp[0] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0xc50);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x", "0xc50(dig)", \
	           u4Tmp[0]);
	CL_PRINTF(cliBuf);

	u4Tmp[0] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x6c0);
	u4Tmp[1] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x6c4);
	u4Tmp[2] = pBtCoexist->fBtcRead4Byte(pBtCoexist, 0x6c8);
	u1Tmp[0] = pBtCoexist->fBtcRead1Byte(pBtCoexist, 0x6cc);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x/ 0x%x/ 0x%x/ 0x%x", "0x6c0/0x6c4/0x6c8/0x6cc(coexTable)", \
	           u4Tmp[0], u4Tmp[1], u4Tmp[2], u1Tmp[0]);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d", "0x770(hp rx[31:16]/tx[15:0])", \
	           pCoexSta->highPriorityRx, pCoexSta->highPriorityTx);
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d", "0x774(lp rx[31:16]/tx[15:0])", \
	           pCoexSta->lowPriorityRx, pCoexSta->lowPriorityTx);
	CL_PRINTF(cliBuf);
#if(BT_AUTO_REPORT_ONLY_8192E_2ANT == 1)
	halbtc8192e2ant_MonitorBtCtr(pBtCoexist);
#endif
	pBtCoexist->fBtcDispDbgMsg(pBtCoexist, BTC_DBG_DISP_COEX_STATISTICS);
}


VOID
EXhalbtc8192e2ant_IpsNotify(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte			type
)
{
	if(BTC_IPS_ENTER == type) {
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], IPS ENTER notify\n"));
		pCoexSta->bUnderIps = TRUE;
		halbtc8192e2ant_CoexAllOff(pBtCoexist);
		halbtc8192e2ant_SetAntPath(pBtCoexist, BTC_ANT_PATH_BT, FALSE, TRUE);
	} else if(BTC_IPS_LEAVE == type) {
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], IPS LEAVE notify\n"));
		pCoexSta->bUnderIps = FALSE;
	}
}

VOID
EXhalbtc8192e2ant_LpsNotify(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte			type
)
{
	if(BTC_LPS_ENABLE == type) {
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], LPS ENABLE notify\n"));
		pCoexSta->bUnderLps = TRUE;
	} else if(BTC_LPS_DISABLE == type) {
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], LPS DISABLE notify\n"));
		pCoexSta->bUnderLps = FALSE;
	}
}

VOID
EXhalbtc8192e2ant_ScanNotify(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte			type
)
{
	if(BTC_SCAN_START == type) {
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], SCAN START notify\n"));
	} else if(BTC_SCAN_FINISH == type) {
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], SCAN FINISH notify\n"));
	}
}

VOID
EXhalbtc8192e2ant_ConnectNotify(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte			type
)
{
	if(BTC_ASSOCIATE_START == type) {
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], CONNECT START notify\n"));
	} else if(BTC_ASSOCIATE_FINISH == type) {
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], CONNECT FINISH notify\n"));
	}
}

VOID
EXhalbtc8192e2ant_MediaStatusNotify(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	u1Byte				type
)
{
	u1Byte			H2C_Parameter[3] = {0};
	u4Byte			wifiBw;
	u1Byte			wifiCentralChnl;

	if(pBtCoexist->bManualControl ||
	   pBtCoexist->bStopCoexDm ||
	   pBtCoexist->btInfo.bBtDisabled )
		return;

	if(BTC_MEDIA_CONNECT == type) {
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], MEDIA connect notify\n"));
	} else {
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], MEDIA disconnect notify\n"));
	}

	// only 2.4G we need to inform bt the chnl mask
	pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U1_WIFI_CENTRAL_CHNL, &wifiCentralChnl);
	if( (BTC_MEDIA_CONNECT == type) &&
	    (wifiCentralChnl <= 14) ) {
		H2C_Parameter[0] = 0x1;
		H2C_Parameter[1] = wifiCentralChnl;
		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);
		if(BTC_WIFI_BW_HT40 == wifiBw)
			H2C_Parameter[2] = 0x30;
		else
			H2C_Parameter[2] = 0x20;
	}

	pCoexDm->wifiChnlInfo[0] = H2C_Parameter[0];
	pCoexDm->wifiChnlInfo[1] = H2C_Parameter[1];
	pCoexDm->wifiChnlInfo[2] = H2C_Parameter[2];

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE_FW_EXEC, ("[BTCoex], FW write 0x66=0x%x\n",
	          H2C_Parameter[0]<<16|H2C_Parameter[1]<<8|H2C_Parameter[2]));

	pBtCoexist->fBtcFillH2c(pBtCoexist, 0x66, 3, H2C_Parameter);
}

VOID
EXhalbtc8192e2ant_SpecialPacketNotify(
    IN	PBTC_COEXIST			pBtCoexist,
    IN	u1Byte				type
)
{
	if(type == BTC_PACKET_DHCP) {
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], DHCP Packet notify\n"));
	}
}

VOID
EXhalbtc8192e2ant_BtInfoNotify(
    IN	PBTC_COEXIST		pBtCoexist,
    IN	pu1Byte			tmpBuf,
    IN	u1Byte			length
)
{
	//PBTC_BT_LINK_INFO	pBtLinkInfo=&pBtCoexist->btLinkInfo;
	u1Byte			btInfo=0;
	u1Byte			i, rspSource=0;
	BOOLEAN			bBtBusy=FALSE, bLimitedDig=FALSE;
	BOOLEAN			bWifiConnected=FALSE;

	pCoexSta->bC2hBtInfoReqSent = FALSE;

	rspSource = tmpBuf[0]&0xf;
	if(rspSource >= BT_INFO_SRC_8192E_2ANT_MAX)
		rspSource = BT_INFO_SRC_8192E_2ANT_WIFI_FW;
	pCoexSta->btInfoC2hCnt[rspSource]++;

	BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], Bt info[%d], length=%d, hex data=[", rspSource, length));
	for(i=0; i<length; i++) {
		pCoexSta->btInfoC2h[rspSource][i] = tmpBuf[i];
		if(i == 1)
			btInfo = tmpBuf[i];
		if(i == length-1) {
			BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("0x%02x]\n", tmpBuf[i]));
		} else {
			BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("0x%02x, ", tmpBuf[i]));
		}
	}

	if(BT_INFO_SRC_8192E_2ANT_WIFI_FW != rspSource) {
		pCoexSta->btRetryCnt =	// [3:0]
		    pCoexSta->btInfoC2h[rspSource][2]&0xf;

		pCoexSta->btRssi =
		    pCoexSta->btInfoC2h[rspSource][3]*2+10;

		pCoexSta->btInfoExt =
		    pCoexSta->btInfoC2h[rspSource][4];

		// Here we need to resend some wifi info to BT
		// because bt is reset and loss of the info.
		if( (pCoexSta->btInfoExt & BIT1) ) {
			BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BT ext info bit1 check, send wifi BW&Chnl to BT!!\n"));
			pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_BL_WIFI_CONNECTED, &bWifiConnected);
			if(bWifiConnected) {
				EXhalbtc8192e2ant_MediaStatusNotify(pBtCoexist, BTC_MEDIA_CONNECT);
			} else {
				EXhalbtc8192e2ant_MediaStatusNotify(pBtCoexist, BTC_MEDIA_DISCONNECT);
			}
		}

		if( (pCoexSta->btInfoExt & BIT3) ) {
			if(!pBtCoexist->bManualControl && !pBtCoexist->bStopCoexDm) {
				BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BT ext info bit3 check, set BT NOT to ignore Wlan active!!\n"));
				halbtc8192e2ant_IgnoreWlanAct(pBtCoexist, FORCE_EXEC, FALSE);
			}
		} else {
			// BT already NOT ignore Wlan active, do nothing here.
		}

#if(BT_AUTO_REPORT_ONLY_8192E_2ANT == 0)
		if( (pCoexSta->btInfoExt & BIT4) ) {
			// BT auto report already enabled, do nothing
		} else {
			halbtc8192e2ant_BtAutoReport(pBtCoexist, FORCE_EXEC, TRUE);
		}
#endif
	}

	// check BIT2 first ==> check if bt is under inquiry or page scan
	if(btInfo & BT_INFO_8192E_2ANT_B_INQ_PAGE)
		pCoexSta->bC2hBtInquiryPage = TRUE;
	else
		pCoexSta->bC2hBtInquiryPage = FALSE;

	// set link exist status
	if(!(btInfo&BT_INFO_8192E_2ANT_B_CONNECTION)) {
		pCoexSta->bBtLinkExist = FALSE;
		pCoexSta->bPanExist = FALSE;
		pCoexSta->bA2dpExist = FALSE;
		pCoexSta->bHidExist = FALSE;
		pCoexSta->bScoExist = FALSE;
	} else {	// connection exists
		pCoexSta->bBtLinkExist = TRUE;
		if(btInfo & BT_INFO_8192E_2ANT_B_FTP)
			pCoexSta->bPanExist = TRUE;
		else
			pCoexSta->bPanExist = FALSE;
		if(btInfo & BT_INFO_8192E_2ANT_B_A2DP)
			pCoexSta->bA2dpExist = TRUE;
		else
			pCoexSta->bA2dpExist = FALSE;
		if(btInfo & BT_INFO_8192E_2ANT_B_HID)
			pCoexSta->bHidExist = TRUE;
		else
			pCoexSta->bHidExist = FALSE;
		if(btInfo & BT_INFO_8192E_2ANT_B_SCO_ESCO)
			pCoexSta->bScoExist = TRUE;
		else
			pCoexSta->bScoExist = FALSE;
	}

	halbtc8192e2ant_UpdateBtLinkInfo(pBtCoexist);

	if(!(btInfo&BT_INFO_8192E_2ANT_B_CONNECTION)) {
		pCoexDm->btStatus = BT_8192E_2ANT_BT_STATUS_NON_CONNECTED_IDLE;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), BT Non-Connected idle!!!\n"));
	} else if(btInfo == BT_INFO_8192E_2ANT_B_CONNECTION) {	// connection exists but no busy
		pCoexDm->btStatus = BT_8192E_2ANT_BT_STATUS_CONNECTED_IDLE;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), BT Connected-idle!!!\n"));
	} else if((btInfo&BT_INFO_8192E_2ANT_B_SCO_ESCO) ||
	          (btInfo&BT_INFO_8192E_2ANT_B_SCO_BUSY)) {
		pCoexDm->btStatus = BT_8192E_2ANT_BT_STATUS_SCO_BUSY;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), BT SCO busy!!!\n"));
	} else if(btInfo&BT_INFO_8192E_2ANT_B_ACL_BUSY) {
		pCoexDm->btStatus = BT_8192E_2ANT_BT_STATUS_ACL_BUSY;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), BT ACL busy!!!\n"));
	} else {
		pCoexDm->btStatus = BT_8192E_2ANT_BT_STATUS_MAX;
		BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], BtInfoNotify(), BT Non-Defined state!!!\n"));
	}

	if( (BT_8192E_2ANT_BT_STATUS_ACL_BUSY == pCoexDm->btStatus) ||
	    (BT_8192E_2ANT_BT_STATUS_SCO_BUSY == pCoexDm->btStatus) ||
	    (BT_8192E_2ANT_BT_STATUS_ACL_SCO_BUSY == pCoexDm->btStatus) ) {
		bBtBusy = TRUE;
		bLimitedDig = TRUE;
	} else {
		bBtBusy = FALSE;
		bLimitedDig = FALSE;
	}

	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_BL_BT_TRAFFIC_BUSY, &bBtBusy);

	pCoexDm->bLimitedDig = bLimitedDig;
	pBtCoexist->fBtcSet(pBtCoexist, BTC_SET_BL_BT_LIMITED_DIG, &bLimitedDig);

	halbtc8192e2ant_RunCoexistMechanism(pBtCoexist);
}

VOID
EXhalbtc8192e2ant_HaltNotify(
    IN	PBTC_COEXIST			pBtCoexist
)
{
	BTC_PRINT(BTC_MSG_INTERFACE, INTF_NOTIFY, ("[BTCoex], Halt notify\n"));

	halbtc8192e2ant_SetAntPath(pBtCoexist, BTC_ANT_PATH_BT, FALSE, TRUE);
	halbtc8192e2ant_IgnoreWlanAct(pBtCoexist, FORCE_EXEC, TRUE);
	EXhalbtc8192e2ant_MediaStatusNotify(pBtCoexist, BTC_MEDIA_DISCONNECT);
}

VOID
EXhalbtc8192e2ant_Periodical(
    IN	PBTC_COEXIST			pBtCoexist
)
{
	static u1Byte		disVerInfoCnt=0;
	u4Byte				fwVer=0, btPatchVer=0;
	//PBTC_BOARD_INFO		pBoardInfo=&pBtCoexist->boardInfo;
	//PBTC_STACK_INFO		pStackInfo=&pBtCoexist->stackInfo;

	BTC_PRINT(BTC_MSG_ALGORITHM, ALGO_TRACE, ("[BTCoex], ==========================Periodical===========================\n"));

	if(disVerInfoCnt <= 5) {
		disVerInfoCnt += 1;
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_INIT, ("[BTCoex], ****************************************************************\n"));
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_INIT, ("[BTCoex], Ant PG Num/ Ant Mech/ Ant Pos = %d/ %d/ %d\n", \
		          pBoardInfo->pgAntNum, pBoardInfo->btdmAntNum, pBoardInfo->btdmAntPos));
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_INIT, ("[BTCoex], BT stack/ hci ext ver = %s / %d\n", \
		          ((pStackInfo->bProfileNotified)? "Yes":"No"), pStackInfo->hciVersion));
		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_BT_PATCH_VER, &btPatchVer);
		pBtCoexist->fBtcGet(pBtCoexist, BTC_GET_U4_WIFI_FW_VER, &fwVer);
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_INIT, ("[BTCoex], CoexVer/ FwVer/ PatchVer = %d_%x/ 0x%x/ 0x%x(%d)\n", \
		          GLCoexVerDate8192e2Ant, GLCoexVer8192e2Ant, fwVer, btPatchVer, btPatchVer));
		BTC_PRINT(BTC_MSG_INTERFACE, INTF_INIT, ("[BTCoex], ****************************************************************\n"));
	}

#if(BT_AUTO_REPORT_ONLY_8192E_2ANT == 0)
	halbtc8192e2ant_QueryBtInfo(pBtCoexist);
	halbtc8192e2ant_MonitorBtCtr(pBtCoexist);
	halbtc8192e2ant_MonitorBtEnableDisable(pBtCoexist);
#else
	if( halbtc8192e2ant_IsWifiStatusChanged(pBtCoexist) ||
	    pCoexDm->bAutoTdmaAdjust) {
		halbtc8192e2ant_RunCoexistMechanism(pBtCoexist);
	}
#endif
}


#endif
