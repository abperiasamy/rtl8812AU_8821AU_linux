/******************************************************************************
 *
 * Copyright(c) 2013 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifdef CONFIG_BT_COEXIST

#include <drv_types.h>
#include <hal_btcoex.h>
#include <hal_data.h>


void rtw_btcoex_Initialize(PADAPTER padapter)
{
	hal_btcoex_Initialize(padapter);
}

void rtw_btcoex_PowerOnSetting(PADAPTER padapter)
{
	hal_btcoex_PowerOnSetting(padapter);
}

void rtw_btcoex_PreLoadFirmware(PADAPTER padapter)
{
	hal_btcoex_PreLoadFirmware(padapter);
}

void rtw_btcoex_HAL_Initialize(PADAPTER padapter, u8 bWifiOnly)
{
	hal_btcoex_InitHwConfig(padapter, bWifiOnly);
}

void rtw_btcoex_IpsNotify(PADAPTER padapter, u8 type)
{
	PHAL_DATA_TYPE	pHalData;

	pHalData = GET_HAL_DATA(padapter);
	if (_FALSE == pHalData->EEPROMBluetoothCoexist)
		return;

	hal_btcoex_IpsNotify(padapter, type);
}

void rtw_btcoex_LpsNotify(PADAPTER padapter, u8 type)
{
	PHAL_DATA_TYPE	pHalData;

	pHalData = GET_HAL_DATA(padapter);
	if (_FALSE == pHalData->EEPROMBluetoothCoexist)
		return;

	hal_btcoex_LpsNotify(padapter, type);
}

void rtw_btcoex_ScanNotify(PADAPTER padapter, u8 type)
{
	PHAL_DATA_TYPE	pHalData;

	pHalData = GET_HAL_DATA(padapter);
	if (_FALSE == pHalData->EEPROMBluetoothCoexist)
		return;

#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	struct bt_coex_info *pcoex_info = &padapter->coex_info;
	PBT_MGNT	pBtMgnt=&pcoex_info->BtMgnt;
#endif //CONFIG_BT_COEXIST_SOCKET_TRX

#ifdef CONFIG_CONCURRENT_MODE
	if ((_FALSE == type) && (padapter->pbuddy_adapter)) {
		PADAPTER pbuddy = padapter->pbuddy_adapter;
		if (check_fwstate(&pbuddy->mlmepriv, WIFI_SITE_MONITOR) == _TRUE)
			return;
	}
#endif

#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	if(pBtMgnt->ExtConfig.bEnableWifiScanNotify)
		rtw_btcoex_SendScanNotify(padapter, type);
#endif //CONFIG_BT_COEXIST_SOCKET_TRX

	hal_btcoex_ScanNotify(padapter, type);
}

void rtw_btcoex_ConnectNotify(PADAPTER padapter, u8 action)
{
	PHAL_DATA_TYPE	pHalData;

	pHalData = GET_HAL_DATA(padapter);
	if (_FALSE == pHalData->EEPROMBluetoothCoexist)
		return;

#ifdef DBG_CONFIG_ERROR_RESET
	if (_TRUE == rtw_hal_sreset_inprogress(padapter)) {
		DBG_8192C(FUNC_ADPT_FMT ": [BTCoex] under reset, skip notify!\n",
		          FUNC_ADPT_ARG(padapter));
		return;
	}
#endif // DBG_CONFIG_ERROR_RESET

#ifdef CONFIG_CONCURRENT_MODE
	if ((_FALSE == action) && (padapter->pbuddy_adapter)) {
		PADAPTER pbuddy = padapter->pbuddy_adapter;
		if (check_fwstate(&pbuddy->mlmepriv, WIFI_UNDER_LINKING) == _TRUE)
			return;
	}
#endif

	hal_btcoex_ConnectNotify(padapter, action);
}

void rtw_btcoex_MediaStatusNotify(PADAPTER padapter, u8 mediaStatus)
{
	PHAL_DATA_TYPE	pHalData;

	pHalData = GET_HAL_DATA(padapter);
	if (_FALSE == pHalData->EEPROMBluetoothCoexist)
		return;

#ifdef DBG_CONFIG_ERROR_RESET
	if (_TRUE == rtw_hal_sreset_inprogress(padapter)) {
		DBG_8192C(FUNC_ADPT_FMT ": [BTCoex] under reset, skip notify!\n",
		          FUNC_ADPT_ARG(padapter));
		return;
	}
#endif // DBG_CONFIG_ERROR_RESET

#ifdef CONFIG_CONCURRENT_MODE
	if ((RT_MEDIA_DISCONNECT == mediaStatus) && (padapter->pbuddy_adapter)) {
		PADAPTER pbuddy = padapter->pbuddy_adapter;
		if (check_fwstate(&pbuddy->mlmepriv, WIFI_ASOC_STATE) == _TRUE)
			return;
	}
#endif // CONFIG_CONCURRENT_MODE

	if ((RT_MEDIA_CONNECT == mediaStatus)
	    && (check_fwstate(&padapter->mlmepriv, WIFI_AP_STATE) == _TRUE)) {
		rtw_hal_set_hwreg(padapter, HW_VAR_DL_RSVD_PAGE, NULL);
	}

	hal_btcoex_MediaStatusNotify(padapter, mediaStatus);
}

void rtw_btcoex_SpecialPacketNotify(PADAPTER padapter, u8 pktType)
{
	PHAL_DATA_TYPE	pHalData;

	pHalData = GET_HAL_DATA(padapter);
	if (_FALSE == pHalData->EEPROMBluetoothCoexist)
		return;

	hal_btcoex_SpecialPacketNotify(padapter, pktType);
}

void rtw_btcoex_IQKNotify(PADAPTER padapter, u8 state)
{
	PHAL_DATA_TYPE	pHalData;

	pHalData = GET_HAL_DATA(padapter);
	if (_FALSE == pHalData->EEPROMBluetoothCoexist)
		return;

	hal_btcoex_IQKNotify(padapter, state);
}

void rtw_btcoex_BtInfoNotify(PADAPTER padapter, u8 length, u8 *tmpBuf)
{
	PHAL_DATA_TYPE	pHalData;

	pHalData = GET_HAL_DATA(padapter);
	if (_FALSE == pHalData->EEPROMBluetoothCoexist)
		return;

	hal_btcoex_BtInfoNotify(padapter, length, tmpBuf);
}

void rtw_btcoex_SuspendNotify(PADAPTER padapter, u8 state)
{
	PHAL_DATA_TYPE	pHalData;

	pHalData = GET_HAL_DATA(padapter);
	if (_FALSE == pHalData->EEPROMBluetoothCoexist)
		return;

	hal_btcoex_SuspendNotify(padapter, state);
}

void rtw_btcoex_HaltNotify(PADAPTER padapter)
{
	PHAL_DATA_TYPE	pHalData;

	pHalData = GET_HAL_DATA(padapter);
	if (_FALSE == pHalData->EEPROMBluetoothCoexist)
		return;

	if (_FALSE == padapter->bup) {
		DBG_871X(FUNC_ADPT_FMT ": bup=%d Skip!\n",
		         FUNC_ADPT_ARG(padapter), padapter->bup);

		return;
	}

	if (_TRUE == padapter->bSurpriseRemoved) {
		DBG_871X(FUNC_ADPT_FMT ": bSurpriseRemoved=%d Skip!\n",
		         FUNC_ADPT_ARG(padapter), padapter->bSurpriseRemoved);

		return;
	}

	hal_btcoex_HaltNotify(padapter);
}

void rtw_btcoex_SwitchBtTRxMask(PADAPTER padapter)
{
	hal_btcoex_SwitchBtTRxMask(padapter);
}

void rtw_btcoex_Switch(PADAPTER padapter, u8 enable)
{
	hal_btcoex_SetBTCoexist(padapter, enable);
}

u8 rtw_btcoex_IsBtDisabled(PADAPTER padapter)
{
	return hal_btcoex_IsBtDisabled(padapter);
}

void rtw_btcoex_Handler(PADAPTER padapter)
{
	PHAL_DATA_TYPE	pHalData;

	pHalData = GET_HAL_DATA(padapter);

	if (_FALSE == pHalData->EEPROMBluetoothCoexist)
		return;

#if defined(CONFIG_CONCURRENT_MODE)
	if (padapter->adapter_type != PRIMARY_ADAPTER)
		return;
#endif



	hal_btcoex_Hanlder(padapter);
}

s32 rtw_btcoex_IsBTCoexRejectAMPDU(PADAPTER padapter)
{
	s32 coexctrl;

	coexctrl = hal_btcoex_IsBTCoexRejectAMPDU(padapter);

	return coexctrl;
}

s32 rtw_btcoex_IsBTCoexCtrlAMPDUSize(PADAPTER padapter)
{
	s32 coexctrl;

	coexctrl = hal_btcoex_IsBTCoexCtrlAMPDUSize(padapter);

	return coexctrl;
}

u32 rtw_btcoex_GetAMPDUSize(PADAPTER padapter)
{
	u32 size;

	size = hal_btcoex_GetAMPDUSize(padapter);

	return size;
}

void rtw_btcoex_SetManualControl(PADAPTER padapter, u8 manual)
{
	if (_TRUE == manual) {
		hal_btcoex_SetManualControl(padapter, _TRUE);
	} else {
		hal_btcoex_SetManualControl(padapter, _FALSE);
	}
}

u8 rtw_btcoex_1Ant(PADAPTER padapter)
{
	return hal_btcoex_1Ant(padapter);
}

u8 rtw_btcoex_IsBtControlLps(PADAPTER padapter)
{
	return hal_btcoex_IsBtControlLps(padapter);
}

u8 rtw_btcoex_IsLpsOn(PADAPTER padapter)
{
	return hal_btcoex_IsLpsOn(padapter);
}

u8 rtw_btcoex_RpwmVal(PADAPTER padapter)
{
	return hal_btcoex_RpwmVal(padapter);
}

u8 rtw_btcoex_LpsVal(PADAPTER padapter)
{
	return hal_btcoex_LpsVal(padapter);
}

void rtw_btcoex_SetBTCoexist(PADAPTER padapter, u8 bBtExist)
{
	hal_btcoex_SetBTCoexist(padapter, bBtExist);
}

void rtw_btcoex_SetChipType(PADAPTER padapter, u8 chipType)
{
	hal_btcoex_SetChipType(padapter, chipType);
}

void rtw_btcoex_SetPGAntNum(PADAPTER padapter, u8 antNum)
{
	hal_btcoex_SetPgAntNum(padapter, antNum);
}

u8 rtw_btcoex_GetPGAntNum(PADAPTER padapter)
{
	return hal_btcoex_GetPgAntNum(padapter);
}

void rtw_btcoex_SetSingleAntPath(PADAPTER padapter, u8 singleAntPath)
{
	hal_btcoex_SetSingleAntPath(padapter, singleAntPath);
}

u32 rtw_btcoex_GetRaMask(PADAPTER padapter)
{
	return hal_btcoex_GetRaMask(padapter);
}

void rtw_btcoex_RecordPwrMode(PADAPTER padapter, u8 *pCmdBuf, u8 cmdLen)
{
	hal_btcoex_RecordPwrMode(padapter, pCmdBuf, cmdLen);
}

void rtw_btcoex_DisplayBtCoexInfo(PADAPTER padapter, u8 *pbuf, u32 bufsize)
{
	hal_btcoex_DisplayBtCoexInfo(padapter, pbuf, bufsize);
}

void rtw_btcoex_SetDBG(PADAPTER padapter, u32 *pDbgModule)
{
	hal_btcoex_SetDBG(padapter, pDbgModule);
}

u32 rtw_btcoex_GetDBG(PADAPTER padapter, u8 *pStrBuf, u32 bufSize)
{
	return hal_btcoex_GetDBG(padapter, pStrBuf, bufSize);
}

u8 rtw_btcoex_IncreaseScanDeviceNum(PADAPTER padapter)
{
	return hal_btcoex_IncreaseScanDeviceNum(padapter);
}

u8 rtw_btcoex_IsBtLinkExist(PADAPTER padapter)
{
	return hal_btcoex_IsBtLinkExist(padapter);
}

void rtw_btcoex_SetBtPatchVersion(PADAPTER padapter,u16 btHciVer, u16 btPatchVer)
{
	hal_btcoex_SetBtPatchVersion(padapter,btHciVer,btPatchVer);
}

void rtw_btcoex_SetHciVersion(PADAPTER  padapter, u16 hciVersion)
{
	hal_btcoex_SetHciVersion(padapter, hciVersion);
}

void rtw_btcoex_StackUpdateProfileInfo(void)
{
	hal_btcoex_StackUpdateProfileInfo();
}

// ==================================================
// Below Functions are called by BT-Coex
// ==================================================
void rtw_btcoex_rx_ampdu_apply(PADAPTER padapter)
{
	rtw_rx_ampdu_apply(padapter);
}

void rtw_btcoex_LPS_Enter(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrpriv;
	u8 lpsVal;


	pwrpriv = adapter_to_pwrctl(padapter);

	pwrpriv->bpower_saving = _TRUE;
	lpsVal = rtw_btcoex_LpsVal(padapter);
	rtw_set_ps_mode(padapter, PS_MODE_MIN, 0, lpsVal, "BTCOEX");
}

void rtw_btcoex_LPS_Leave(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrpriv;


	pwrpriv = adapter_to_pwrctl(padapter);

	if (pwrpriv->pwr_mode != PS_MODE_ACTIVE) {
		rtw_set_ps_mode(padapter, PS_MODE_ACTIVE, 0, 0, "BTCOEX");
		LPS_RF_ON_check(padapter, 100);
		pwrpriv->bpower_saving = _FALSE;
	}
}


// ==================================================
// Below Functions are BT-Coex socket related function
// ==================================================

#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
_adapter *pbtcoexadapter = NULL;
u8 rtw_btcoex_btinfo_cmd(_adapter *adapter, u8 *buf, u16 len)
{
	struct cmd_obj *ph2c;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	u8 *btinfo;
	struct cmd_priv *pcmdpriv = &adapter->cmdpriv;
	u8	res = _SUCCESS;

	ph2c = (struct cmd_obj*)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm*)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((u8*)ph2c, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	btinfo = rtw_zmalloc(len);
	if (btinfo == NULL) {
		rtw_mfree((u8*)ph2c, sizeof(struct cmd_obj));
		rtw_mfree((u8*)pdrvextra_cmd_parm, sizeof(struct drvextra_cmd_parm));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = BTINFO_WK_CID;
	pdrvextra_cmd_parm->type = 0;
	pdrvextra_cmd_parm->size = len;
	pdrvextra_cmd_parm->pbuf = btinfo;

	_rtw_memcpy(btinfo, buf, len);

	init_h2fwcmd_w_parm_no_rsp(ph2c, pdrvextra_cmd_parm, GEN_CMD_CODE(_Set_Drv_Extra));

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

exit:
	return res;
}

u8 rtw_btcoex_send_event_to_BT(_adapter *padapter, u8 status,  u8 event_code, u8 opcode_low, u8 opcode_high,u8 *dbg_msg)
{
	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0,tx_event_length = 0;
	rtw_HCI_event *pEvent;

	pEvent = (rtw_HCI_event*)(&localBuf[0]);

	pEvent->EventCode = event_code;
	pEvent->Data[0] = 0x1;	//packet #
	pEvent->Data[1] = opcode_low;
	pEvent->Data[2] = opcode_high;
	len = len + 3;

	// Return parameters starts from here
	pRetPar = &pEvent->Data[len];
	pRetPar[0] = status;		//status

	len++;
	pEvent->Length = len;

	//total tx event length + EventCode length + sizeof(length)
	tx_event_length = pEvent->Length + 2;

	rtw_btcoex_dump_tx_msg((u8 *)pEvent, tx_event_length, dbg_msg);

	status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);

	return status;
}

/*
Ref:
Realtek Wi-Fi Driver
Host Controller Interface for
Bluetooth 3.0 + HS V1.4 2013/02/07

Window team code & BT team code
 */


u8 rtw_btcoex_parse_BT_info_notify_cmd(_adapter *padapter, u8 *pcmd, u16 cmdlen)
{
#define BT_INFO_LENGTH 8

	u8 curPollEnable = pcmd[0];
	u8 curPollTime = pcmd[1];
	u8 btInfoReason = pcmd[2];
	u8 btInfoLen = pcmd[3];
	u8 btinfo[BT_INFO_LENGTH];

	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0,tx_event_length = 0;
	RTW_HCI_STATUS status = HCI_STATUS_SUCCESS;
	rtw_HCI_event *pEvent;

	DBG_871X("%s\n",__func__);
	DBG_871X("current Poll Enable: %d, currrent Poll Time: %d\n",curPollEnable,curPollTime);
	DBG_871X("BT Info reason: %d, BT Info length: %d\n",btInfoReason,btInfoLen);
	DBG_871X("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n"
	         ,pcmd[4],pcmd[5],pcmd[6],pcmd[7],pcmd[8],pcmd[9],pcmd[10],pcmd[11]);

	_rtw_memset(btinfo, 0, BT_INFO_LENGTH);

#if 1
	if(BT_INFO_LENGTH != btInfoLen) {
		status = HCI_STATUS_INVALID_HCI_CMD_PARA_VALUE;
		DBG_871X("Error BT Info Length: %d\n",btInfoLen);
		//return _FAIL;
	} else
#endif
	{
		if(0x1 == btInfoReason || 0x2 == btInfoReason) {
			_rtw_memcpy(btinfo, &pcmd[4], btInfoLen);
			btinfo[0] = btInfoReason;
			rtw_btcoex_btinfo_cmd(padapter,btinfo,btInfoLen);
		} else {
			DBG_871X("Other BT info reason\n");
		}
	}

	//send complete event to BT
	{

		pEvent = (rtw_HCI_event*)(&localBuf[0]);

		pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
		pEvent->Data[0] = 0x1;	//packet #
		pEvent->Data[1] = HCIOPCODELOW(HCI_BT_INFO_NOTIFY, OGF_EXTENSION);
		pEvent->Data[2] = HCIOPCODEHIGHT(HCI_BT_INFO_NOTIFY, OGF_EXTENSION);
		len = len + 3;

		// Return parameters starts from here
		pRetPar = &pEvent->Data[len];
		pRetPar[0] = status;		//status

		len++;
		pEvent->Length = len;

		//total tx event length + EventCode length + sizeof(length)
		tx_event_length = pEvent->Length + 2;

		rtw_btcoex_dump_tx_msg((u8 *)pEvent, tx_event_length,"BT_info_event");

		status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);

		return status;
		//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);
	}
}

u8 rtw_btcoex_parse_BT_patch_ver_info_cmd(_adapter *padapter, u8 *pcmd, u16 cmdlen)
{
	RTW_HCI_STATUS status=HCI_STATUS_SUCCESS;
	u16		btPatchVer=0x0, btHciVer=0x0;
	//u16		*pU2tmp;

	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0, tx_event_length =0;
	rtw_HCI_event *pEvent;

	btHciVer = pcmd[0] | pcmd[1]<<8;
	btPatchVer = pcmd[2] | pcmd[3]<<8;


	DBG_871X("%s, cmd:%02x %02x %02x %02x\n",__func__, pcmd[0] ,pcmd[1] ,pcmd[2] ,pcmd[3]);
	DBG_871X("%s, HCI Ver:%d, Patch Ver:%d\n",__func__, btHciVer,btPatchVer);

	rtw_btcoex_SetBtPatchVersion(padapter,btHciVer,btPatchVer);


	//send complete event to BT
	{
		pEvent = (rtw_HCI_event *)(&localBuf[0]);


		pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
		pEvent->Data[0] = 0x1;	//packet #
		pEvent->Data[1] = HCIOPCODELOW(HCI_BT_PATCH_VERSION_NOTIFY, OGF_EXTENSION);
		pEvent->Data[2] = HCIOPCODEHIGHT(HCI_BT_PATCH_VERSION_NOTIFY, OGF_EXTENSION);
		len = len + 3;

		// Return parameters starts from here
		pRetPar = &pEvent->Data[len];
		pRetPar[0] = status;		//status

		len++;
		pEvent->Length = len;

		//total tx event length + EventCode length + sizeof(length)
		tx_event_length = pEvent->Length + 2;

		rtw_btcoex_dump_tx_msg((u8 *)pEvent, tx_event_length,"BT_patch_event");

		status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);
		return status;
		//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);
	}
}

u8 rtw_btcoex_parse_HCI_Ver_notify_cmd(_adapter *padapter, u8 *pcmd, u16 cmdlen)
{
	RTW_HCI_STATUS status=HCI_STATUS_SUCCESS;
	u16 hciver = pcmd[0] | pcmd[1] <<8;

	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0, tx_event_length =0;
	rtw_HCI_event *pEvent;

	struct bt_coex_info *pcoex_info = &padapter->coex_info;
	PBT_MGNT	pBtMgnt=&pcoex_info->BtMgnt;
	pBtMgnt->ExtConfig.HCIExtensionVer = hciver;
	DBG_871X("%s, HCI Version: %d\n",__func__,pBtMgnt->ExtConfig.HCIExtensionVer);
	if(pBtMgnt->ExtConfig.HCIExtensionVer  < 4) {
		status = HCI_STATUS_INVALID_HCI_CMD_PARA_VALUE;
		DBG_871X("%s, Version = %d, HCI Version < 4\n",__func__,pBtMgnt->ExtConfig.HCIExtensionVer );
	} else {
		rtw_btcoex_SetHciVersion(padapter,hciver);
	}
	//send complete event to BT
	{
		pEvent = (rtw_HCI_event *)(&localBuf[0]);


		pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
		pEvent->Data[0] = 0x1;	//packet #
		pEvent->Data[1] = HCIOPCODELOW(HCI_EXTENSION_VERSION_NOTIFY, OGF_EXTENSION);
		pEvent->Data[2] = HCIOPCODEHIGHT(HCI_EXTENSION_VERSION_NOTIFY, OGF_EXTENSION);
		len = len + 3;

		// Return parameters starts from here
		pRetPar = &pEvent->Data[len];
		pRetPar[0] = status;		//status

		len++;
		pEvent->Length = len;

		//total tx event length + EventCode length + sizeof(length)
		tx_event_length = pEvent->Length + 2;

		status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);
		return status;
		//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);
	}

}

u8 rtw_btcoex_parse_WIFI_scan_notify_cmd(_adapter *padapter, u8 *pcmd, u16 cmdlen)
{
	RTW_HCI_STATUS status=HCI_STATUS_SUCCESS;

	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0, tx_event_length =0;
	rtw_HCI_event *pEvent;

	struct bt_coex_info *pcoex_info = &padapter->coex_info;
	PBT_MGNT	pBtMgnt=&pcoex_info->BtMgnt;
	pBtMgnt->ExtConfig.bEnableWifiScanNotify= pcmd[0];
	DBG_871X("%s, bEnableWifiScanNotify: %d\n",__func__,pBtMgnt->ExtConfig.bEnableWifiScanNotify);

	//send complete event to BT
	{
		pEvent = (rtw_HCI_event *)(&localBuf[0]);


		pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
		pEvent->Data[0] = 0x1;	//packet #
		pEvent->Data[1] = HCIOPCODELOW(HCI_ENABLE_WIFI_SCAN_NOTIFY, OGF_EXTENSION);
		pEvent->Data[2] = HCIOPCODEHIGHT(HCI_ENABLE_WIFI_SCAN_NOTIFY, OGF_EXTENSION);
		len = len + 3;

		// Return parameters starts from here
		pRetPar = &pEvent->Data[len];
		pRetPar[0] = status;		//status

		len++;
		pEvent->Length = len;

		//total tx event length + EventCode length + sizeof(length)
		tx_event_length = pEvent->Length + 2;

		status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);
		return status;
		//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);
	}
}

u8 rtw_btcoex_parse_HCI_link_status_notify_cmd(_adapter *padapter, u8 *pcmd, u16 cmdlen)
{
	RTW_HCI_STATUS	status=HCI_STATUS_SUCCESS;
	struct bt_coex_info	*pcoex_info=&padapter->coex_info;
	PBT_MGNT	pBtMgnt=&pcoex_info->BtMgnt;
	//PBT_DBG		pBtDbg=&padapter->MgntInfo.BtInfo.BtDbg;
	u8		i, numOfHandle=0, numOfAcl=0;
	u16		conHandle;
	u8		btProfile, btCoreSpec, linkRole;
	u8		*pTriple;

	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0, tx_event_length =0;
	rtw_HCI_event *pEvent;

	//pBtDbg->dbgHciInfo.hciCmdCntLinkStatusNotify++;
	//RT_DISP_DATA(FIOCTL, IOCTL_BT_HCICMD_EXT, "LinkStatusNotify, Hex Data :\n",
	//		&pHciCmd->Data[0], pHciCmd->Length);

	DBG_871X("BTLinkStatusNotify\n");

	// Current only RTL8723 support this command.
	//pBtMgnt->bSupportProfile = TRUE;
	pBtMgnt->bSupportProfile = _FALSE;

	pBtMgnt->ExtConfig.NumberOfACL = 0;
	pBtMgnt->ExtConfig.NumberOfSCO = 0;

	numOfHandle = pcmd[0];
	//RT_DISP(FIOCTL, IOCTL_BT_HCICMD_EXT, ("numOfHandle = 0x%x\n", numOfHandle));
	//RT_DISP(FIOCTL, IOCTL_BT_HCICMD_EXT, ("HCIExtensionVer = %d\n", pBtMgnt->ExtConfig.HCIExtensionVer));
	DBG_871X("numOfHandle = 0x%x\n", numOfHandle);
	DBG_871X("HCIExtensionVer = %d\n", pBtMgnt->ExtConfig.HCIExtensionVer);

	pTriple = &pcmd[1];
	for(i=0; i<numOfHandle; i++) {
		if(pBtMgnt->ExtConfig.HCIExtensionVer < 1) {
			conHandle = *((u8 *)&pTriple[0]);
			btProfile = pTriple[2];
			btCoreSpec = pTriple[3];
			if(BT_PROFILE_SCO == btProfile) {
				pBtMgnt->ExtConfig.NumberOfSCO++;
			} else {
				pBtMgnt->ExtConfig.NumberOfACL++;
				pBtMgnt->ExtConfig.aclLink[i].ConnectHandle = conHandle;
				pBtMgnt->ExtConfig.aclLink[i].BTProfile = btProfile;
				pBtMgnt->ExtConfig.aclLink[i].BTCoreSpec = btCoreSpec;
			}
			//RT_DISP(FIOCTL, IOCTL_BT_HCICMD_EXT,
			//	("Connection_Handle=0x%x, BTProfile=%d, BTSpec=%d\n",
			//		conHandle, btProfile, btCoreSpec));
			DBG_871X("Connection_Handle=0x%x, BTProfile=%d, BTSpec=%d\n", conHandle, btProfile, btCoreSpec);
			pTriple += 4;
		} else if(pBtMgnt->ExtConfig.HCIExtensionVer >= 1) {
			conHandle = *((pu2Byte)&pTriple[0]);
			btProfile = pTriple[2];
			btCoreSpec = pTriple[3];
			linkRole = pTriple[4];
			if(BT_PROFILE_SCO == btProfile) {
				pBtMgnt->ExtConfig.NumberOfSCO++;
			} else {
				pBtMgnt->ExtConfig.NumberOfACL++;
				pBtMgnt->ExtConfig.aclLink[i].ConnectHandle = conHandle;
				pBtMgnt->ExtConfig.aclLink[i].BTProfile = btProfile;
				pBtMgnt->ExtConfig.aclLink[i].BTCoreSpec = btCoreSpec;
				pBtMgnt->ExtConfig.aclLink[i].linkRole = linkRole;
			}
			//RT_DISP(FIOCTL, IOCTL_BT_HCICMD_EXT,
			DBG_871X("Connection_Handle=0x%x, BTProfile=%d, BTSpec=%d, LinkRole=%d\n",
			         conHandle, btProfile, btCoreSpec, linkRole);
			pTriple += 5;
		}
	}
	rtw_btcoex_StackUpdateProfileInfo();

	//send complete event to BT
	{
		pEvent = (rtw_HCI_event *)(&localBuf[0]);


		pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
		pEvent->Data[0] = 0x1;	//packet #
		pEvent->Data[1] = HCIOPCODELOW(HCI_LINK_STATUS_NOTIFY, OGF_EXTENSION);
		pEvent->Data[2] = HCIOPCODEHIGHT(HCI_LINK_STATUS_NOTIFY, OGF_EXTENSION);
		len = len + 3;

		// Return parameters starts from here
		pRetPar = &pEvent->Data[len];
		pRetPar[0] = status;		//status

		len++;
		pEvent->Length = len;

		//total tx event length + EventCode length + sizeof(length)
		tx_event_length = pEvent->Length + 2;

		status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);
		return status;
		//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);
	}


}

u8 rtw_btcoex_parse_HCI_BT_coex_notify_cmd(_adapter *padapter, u8 *pcmd, u16 cmdlen)
{
	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0, tx_event_length =0;
	rtw_HCI_event *pEvent;
	RTW_HCI_STATUS	status=HCI_STATUS_SUCCESS;

	{
		pEvent = (rtw_HCI_event *)(&localBuf[0]);


		pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
		pEvent->Data[0] = 0x1;	//packet #
		pEvent->Data[1] = HCIOPCODELOW(HCI_BT_COEX_NOTIFY, OGF_EXTENSION);
		pEvent->Data[2] = HCIOPCODEHIGHT(HCI_BT_COEX_NOTIFY, OGF_EXTENSION);
		len = len + 3;

		// Return parameters starts from here
		pRetPar = &pEvent->Data[len];
		pRetPar[0] = status;		//status

		len++;
		pEvent->Length = len;

		//total tx event length + EventCode length + sizeof(length)
		tx_event_length = pEvent->Length + 2;

		status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);
		return status;
		//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);
	}
}

u8 rtw_btcoex_parse_HCI_BT_operation_notify_cmd(_adapter *padapter, u8 *pcmd, u16 cmdlen)
{
	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0, tx_event_length =0;
	rtw_HCI_event *pEvent;
	RTW_HCI_STATUS	status=HCI_STATUS_SUCCESS;

	DBG_871X("%s, OP code: %d\n",__func__,pcmd[0]);

	switch(pcmd[0]) {
	//case HCI_BT_OP_NONE:
	case 0x0:
		DBG_871X("[bt operation] : Operation None!!\n");
		break;
	//case HCI_BT_OP_INQUIRY_START:
	case 0x1:
		DBG_871X("[bt operation] : Inquiry start!!\n");
		break;
	//case HCI_BT_OP_INQUIRY_FINISH:
	case 0x2:
		DBG_871X("[bt operation] : Inquiry finished!!\n");
		break;
	//case HCI_BT_OP_PAGING_START:
	case 0x3:
		DBG_871X("[bt operation] : Paging is started!!\n");
		break;
	//case HCI_BT_OP_PAGING_SUCCESS:
	case 0x4:
		DBG_871X("[bt operation] : Paging complete successfully!!\n");
		break;
	//case HCI_BT_OP_PAGING_UNSUCCESS:
	case 0x5:
		DBG_871X("[bt operation] : Paging complete unsuccessfully!!\n");
		break;
	//case HCI_BT_OP_PAIRING_START:
	case 0x6:
		DBG_871X("[bt operation] : Pairing start!!\n");
		break;
	//case HCI_BT_OP_PAIRING_FINISH:
	case 0x7:
		DBG_871X("[bt operation] : Pairing finished!!\n");
		break;
	//case HCI_BT_OP_BT_DEV_ENABLE:
	case 0x8:
		DBG_871X("[bt operation] : BT Device is enabled!!\n");
		break;
	//case HCI_BT_OP_BT_DEV_DISABLE:
	case 0x9:
		DBG_871X("[bt operation] : BT Device is disabled!!\n");
		break;
	default:
		DBG_871X("[bt operation] : Unknown, error!!\n");
		break;
	}

	//send complete event to BT
	{
		pEvent = (rtw_HCI_event *)(&localBuf[0]);


		pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
		pEvent->Data[0] = 0x1;	//packet #
		pEvent->Data[1] = HCIOPCODELOW(HCI_BT_OPERATION_NOTIFY, OGF_EXTENSION);
		pEvent->Data[2] = HCIOPCODEHIGHT(HCI_BT_OPERATION_NOTIFY, OGF_EXTENSION);
		len = len + 3;

		// Return parameters starts from here
		pRetPar = &pEvent->Data[len];
		pRetPar[0] = status;		//status

		len++;
		pEvent->Length = len;

		//total tx event length + EventCode length + sizeof(length)
		tx_event_length = pEvent->Length + 2;

		status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);
		return status;
		//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);
	}
}

u8 rtw_btcoex_parse_BT_AFH_MAP_notify_cmd(_adapter *padapter, u8 *pcmd, u16 cmdlen)
{
	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0, tx_event_length =0;
	rtw_HCI_event *pEvent;
	RTW_HCI_STATUS	status=HCI_STATUS_SUCCESS;

	{
		pEvent = (rtw_HCI_event *)(&localBuf[0]);


		pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
		pEvent->Data[0] = 0x1;	//packet #
		pEvent->Data[1] = HCIOPCODELOW(HCI_BT_AFH_MAP_NOTIFY, OGF_EXTENSION);
		pEvent->Data[2] = HCIOPCODEHIGHT(HCI_BT_AFH_MAP_NOTIFY, OGF_EXTENSION);
		len = len + 3;

		// Return parameters starts from here
		pRetPar = &pEvent->Data[len];
		pRetPar[0] = status;		//status

		len++;
		pEvent->Length = len;

		//total tx event length + EventCode length + sizeof(length)
		tx_event_length = pEvent->Length + 2;

		status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);
		return status;
		//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);
	}
}

u8 rtw_btcoex_parse_BT_register_val_notify_cmd(_adapter *padapter, u8 *pcmd, u16 cmdlen)
{

	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0, tx_event_length =0;
	rtw_HCI_event *pEvent;
	RTW_HCI_STATUS	status=HCI_STATUS_SUCCESS;

	{
		pEvent = (rtw_HCI_event *)(&localBuf[0]);


		pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
		pEvent->Data[0] = 0x1;	//packet #
		pEvent->Data[1] = HCIOPCODELOW(HCI_BT_REGISTER_VALUE_NOTIFY, OGF_EXTENSION);
		pEvent->Data[2] = HCIOPCODEHIGHT(HCI_BT_REGISTER_VALUE_NOTIFY, OGF_EXTENSION);
		len = len + 3;

		// Return parameters starts from here
		pRetPar = &pEvent->Data[len];
		pRetPar[0] = status;		//status

		len++;
		pEvent->Length = len;

		//total tx event length + EventCode length + sizeof(length)
		tx_event_length = pEvent->Length + 2;

		status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);
		return status;
		//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);
	}
}

u8 rtw_btcoex_parse_HCI_BT_abnormal_notify_cmd(_adapter *padapter, u8 *pcmd, u16 cmdlen)
{
	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0, tx_event_length =0;
	rtw_HCI_event *pEvent;
	RTW_HCI_STATUS	status=HCI_STATUS_SUCCESS;

	{
		pEvent = (rtw_HCI_event *)(&localBuf[0]);


		pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
		pEvent->Data[0] = 0x1;	//packet #
		pEvent->Data[1] = HCIOPCODELOW(HCI_BT_ABNORMAL_NOTIFY, OGF_EXTENSION);
		pEvent->Data[2] = HCIOPCODEHIGHT(HCI_BT_ABNORMAL_NOTIFY, OGF_EXTENSION);
		len = len + 3;

		// Return parameters starts from here
		pRetPar = &pEvent->Data[len];
		pRetPar[0] = status;		//status

		len++;
		pEvent->Length = len;

		//total tx event length + EventCode length + sizeof(length)
		tx_event_length = pEvent->Length + 2;

		status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);
		return status;
		//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);
	}
}

u8 rtw_btcoex_parse_HCI_query_RF_status_cmd(_adapter *padapter, u8 *pcmd, u16 cmdlen)
{
	u8 localBuf[6] = "";
	u8 *pRetPar;
	u8	len=0, tx_event_length =0;
	rtw_HCI_event *pEvent;
	RTW_HCI_STATUS	status=HCI_STATUS_SUCCESS;

	{
		pEvent = (rtw_HCI_event *)(&localBuf[0]);


		pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
		pEvent->Data[0] = 0x1;	//packet #
		pEvent->Data[1] = HCIOPCODELOW(HCI_QUERY_RF_STATUS, OGF_EXTENSION);
		pEvent->Data[2] = HCIOPCODEHIGHT(HCI_QUERY_RF_STATUS, OGF_EXTENSION);
		len = len + 3;

		// Return parameters starts from here
		pRetPar = &pEvent->Data[len];
		pRetPar[0] = status;		//status

		len++;
		pEvent->Length = len;

		//total tx event length + EventCode length + sizeof(length)
		tx_event_length = pEvent->Length + 2;

		status = rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);
		return status;
		//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);
	}
}

/*****************************************
* HCI cmd format :
*| 15 - 0						|
*| OPcode (OCF|OGF<<10)		|
*| 15 - 8		|7 - 0			|
*|Cmd para 	|Cmd para Length	|
*|Cmd para......				|
******************************************/

//bit 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
//	 |	OCF			             |	   OGF       |
void rtw_btcoex_parse_hci_extend_cmd(_adapter *padapter, u8 *pcmd, u16 len,const u16 hci_OCF)
{

	DBG_871X("%s: OCF: %x\n",__func__,hci_OCF);
	switch(hci_OCF) {
	case HCI_EXTENSION_VERSION_NOTIFY:
		DBG_871X("HCI_EXTENSION_VERSION_NOTIFY\n");
		rtw_btcoex_parse_HCI_Ver_notify_cmd(padapter,pcmd, len);
		break;
	case HCI_LINK_STATUS_NOTIFY:
		DBG_871X("HCI_LINK_STATUS_NOTIFY\n");
		rtw_btcoex_parse_HCI_link_status_notify_cmd(padapter,pcmd, len);
		break;
	case HCI_BT_OPERATION_NOTIFY:
		// only for 8723a 2ant
		DBG_871X("HCI_BT_OPERATION_NOTIFY\n");
		rtw_btcoex_parse_HCI_BT_operation_notify_cmd(padapter,pcmd, len);
		//
		break;
	case HCI_ENABLE_WIFI_SCAN_NOTIFY:
		DBG_871X("HCI_ENABLE_WIFI_SCAN_NOTIFY\n");
		rtw_btcoex_parse_WIFI_scan_notify_cmd(padapter,pcmd, len);
		break;
	case HCI_QUERY_RF_STATUS:
		// only for 8723b 2ant
		DBG_871X("HCI_QUERY_RF_STATUS\n");
		rtw_btcoex_parse_HCI_query_RF_status_cmd(padapter,pcmd, len);
		break;
	case HCI_BT_ABNORMAL_NOTIFY:
		DBG_871X("HCI_BT_ABNORMAL_NOTIFY\n");
		rtw_btcoex_parse_HCI_BT_abnormal_notify_cmd(padapter,pcmd, len);
		break;
	case HCI_BT_INFO_NOTIFY:
		DBG_871X("HCI_BT_INFO_NOTIFY\n");
		rtw_btcoex_parse_BT_info_notify_cmd(padapter,pcmd, len);
		break;
	case HCI_BT_COEX_NOTIFY:
		DBG_871X("HCI_BT_COEX_NOTIFY\n");
		rtw_btcoex_parse_HCI_BT_coex_notify_cmd(padapter,pcmd, len);
		break;
	case HCI_BT_PATCH_VERSION_NOTIFY:
		DBG_871X("HCI_BT_PATCH_VERSION_NOTIFY\n");
		rtw_btcoex_parse_BT_patch_ver_info_cmd(padapter,pcmd, len);
		break;
	case HCI_BT_AFH_MAP_NOTIFY:
		DBG_871X("HCI_BT_AFH_MAP_NOTIFY\n");
		rtw_btcoex_parse_BT_AFH_MAP_notify_cmd(padapter,pcmd, len);
		break;
	case HCI_BT_REGISTER_VALUE_NOTIFY:
		DBG_871X("HCI_BT_REGISTER_VALUE_NOTIFY\n");
		rtw_btcoex_parse_BT_register_val_notify_cmd(padapter,pcmd, len);
		break;
	default:
		DBG_871X("ERROR!!! Unknown OCF: %x\n",hci_OCF);
		break;

	}
}

void rtw_btcoex_parse_hci_cmd(_adapter *padapter, u8 *pcmd, u16 len)
{
	u16 opcode = pcmd[0] | pcmd[1]<<8;
	u16 hci_OGF = HCI_OGF(opcode);
	u16 hci_OCF = HCI_OCF(opcode);
	u8 cmdlen = len -3;
	u8 pare_len = pcmd[2];

	DBG_871X("%s\n",__func__);
	DBG_871X("OGF: %x,OCF: %x\n",hci_OGF,hci_OCF);
	switch(hci_OGF) {
	case OGF_EXTENSION:
		DBG_871X("HCI_EXTENSION_CMD_OGF\n");
		rtw_btcoex_parse_hci_extend_cmd(padapter, &pcmd[3], cmdlen, hci_OCF);
		break;
	default:
		DBG_871X("Other OGF: %x\n",hci_OGF);
		break;
	}
}

u16 rtw_btcoex_parse_recv_data(u8 *msg, u8 msg_size)
{
	u8 cmp_msg1[128] = attend_ack;
	u8 cmp_msg2[128] = leave_ack;
	u8 cmp_msg3[128] = bt_leave;
	u8 cmp_msg4[128] = invite_req;
	//u8 btinfonotifycmd[2] = {0x06,0x01};
	u8 res = OTHER;

	if(_rtw_memcmp(cmp_msg1,msg,msg_size) == _TRUE) {
		DBG_871X("%s, msg:%s\n",__func__,msg);
		res = RX_ATTEND_ACK;
	} else if(_rtw_memcmp(cmp_msg2,msg,msg_size) == _TRUE) {
		DBG_871X("%s, msg:%s\n",__func__,msg);
		res = RX_LEAVE_ACK;
	} else if(_rtw_memcmp(cmp_msg3,msg,msg_size) == _TRUE) {
		DBG_871X("%s, msg:%s\n",__func__,msg);
		res = RX_BT_LEAVE;
	} else if(_rtw_memcmp(cmp_msg4,msg,msg_size) == _TRUE) {
		DBG_871X("%s, msg:%s\n",__func__,msg);
		res = RX_INVITE_REQ;
	}
#if 0
	else if (_rtw_memcmp(btinfonotifycmd,msg,sizeof(btinfonotifycmd)) {
	DBG_871X("%s, OCF:%02x%02x\n",__func__,msg[0],msg[1]);
		DBG_871X("%s, msg:BT_INFO_NOTIFY_CMD\n",__func__);
		res = BT_INFO_NOTIFY_CMD;
	}
#endif
	else {
		DBG_871X("%s, OGF|OCF:%02x%02x\n",__func__,msg[1],msg[0]);
		res = OTHER;
	}

	DBG_871X("%s, res:%d\n",__func__,res);

	return res;
}

void rtw_btcoex_recvmsgbynetlink(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	u32 msg_size;
	u8 msg[255]= {0};
	u8 rsp_msg[255] = invite_rsp;
	u8 ack_msg[255] = leave_ack;
	u8 res = 0;
	u16 parse_res = 0;
	u8 is_invite;
	struct bt_coex_info *pcoex_info = &pbtcoexadapter->coex_info;


	DBG_871X("Entering: %s\n", __FUNCTION__);
	DBG_871X("========> pbtcoexadapter: %p\n",pbtcoexadapter);

	_rtw_memset(msg,0,sizeof(msg));
	nlh=(struct nlmsghdr*)skb->data;
	DBG_871X("Netlink received msg payload:%s\n",(char*)nlmsg_data(nlh));

	pcoex_info->pid = nlh->nlmsg_pid; /*pid of sending process */
	msg_size = sizeof(msg);
	_rtw_memcpy(msg,NLMSG_DATA(nlh),strlen(NLMSG_DATA(nlh)));
	DBG_871X("Message: %s,size: %d\n",msg,msg_size);
	parse_res = rtw_btcoex_parse_recv_data(msg,strlen(msg));
	DBG_871X("parse_res: %d\n",parse_res);
	DBG_871X("================> PID: %d\n", pcoex_info->pid);

	if(RX_INVITE_REQ == parse_res) {

		is_invite = _TRUE;
		res = rtw_btcoex_create_kernel_socket(pbtcoexadapter,is_invite);
		if(_SUCCESS == res) {
			//res = sendmsgbynetlink(pbtcoexadapter, rsp_msg, sizeof(rsp_msg)); //inform BT to close netlink socket
			res = rtw_btcoex_sendmsgbysocket(pbtcoexadapter, rsp_msg, sizeof(rsp_msg)); //inform BT to close netlink socket
			if(res >= 0) {
				pcoex_info->BT_attend =  _TRUE;
				DBG_871X("sock_open:%d, BT_attend:%d\n",pcoex_info->sock_open,pcoex_info->BT_attend);
				//if(pcoex_info->BT_attend == _TRUE)
				//{
				// must send
				//sendmsgbysocket(pbtcoexadapter,"hello, this is wifi!!",sizeof("hello, this is wifi!!"));
				//msleep(20);
				//}
			} else
				DBG_871X("sendmsgbynetlink fail!!\n");
		}
	}
#if 0
	else if(RX_BT_LEAVE == parse_res) {
		res = sendmsgbynetlink(pbtcoexadapter, ack_msg, sizeof(ack_msg));// Tx leave_ack;
		pcoex_info->BT_attend = _FALSE;
		close_kernel_socket(pbtcoexadapter);
		DBG_871X("sock_open:%d, BT_attend:%d\n",pcoex_info->sock_open,pcoex_info->BT_attend);
	}
#endif
	else {
		DBG_871X("Error msg\n");
	}
}

s8 rtw_btcoex_sendmsgbynetlink(_adapter *padapter, u8 *msg, u8 msg_size)
{
	struct nlmsghdr *nlh;
	struct sk_buff *skb_out;
	struct bt_coex_info *pcoex_info = &padapter->coex_info;

	u8 res = 0;
	DBG_871X("%s\n", __FUNCTION__);
	DBG_871X("========> padapter: %p\n",padapter);
	DBG_871X("Message: %s,size: %d\n",msg,msg_size);

	skb_out = nlmsg_new(msg_size,0);
	if(!skb_out) {

		DBG_871X("Failed to allocate new skb\n");
		return -1;

	}

	nlh=nlmsg_put(skb_out,0,0,NLMSG_DONE,msg_size,0);
	NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
	NETLINK_CB(skb_out).portid = 0;
#else
	NETLINK_CB(skb_out).pid = 0;
#endif
	_rtw_memcpy(nlmsg_data(nlh),msg,msg_size);

	DBG_871X("===> send\n");
	if(pcoex_info->nl_sk) {
		res=netlink_unicast(pcoex_info->nl_sk, skb_out, pcoex_info->pid, MSG_WAITALL);
		msleep(100);
	}
	DBG_871X("send <===\n");

	if(res<0) {
		//netlink_is_kernel(sk);
		DBG_871X("pid:%d\n",pcoex_info->pid);
		DBG_871X("%s, Error %d\n",__func__,res);
	}
	return res;

}

void rtw_btcoex_recvmsgbysocket(struct sock *sk, sint bytes)
{
	u8 recv_data[255];
	u8 tx_msg[255] = leave_ack;
	u32 len = 0;

	u16 recv_length;
	u16 parse_res = 0;
#if 0
	u8 para_len = 0, polling_enable = 0, poling_interval = 0, reason = 0, btinfo_len = 0;
	u8 btinfo[BT_INFO_LEN] = {0};
#endif
	struct bt_coex_info *pcoex_info = &pbtcoexadapter->coex_info;
	struct sk_buff * skb;

	DBG_871X("%s\n",__func__);
	DBG_871X("========> pbtcoexadapter: %p\n",pbtcoexadapter);

	if(_TRUE == pcoex_info->BT_attend) {
		len = skb_queue_len(&sk->sk_receive_queue);
		DBG_871X("skb queue len %i\n",len);
		while(len > 0) {
			skb = skb_dequeue(&sk->sk_receive_queue);

			/*important: cut the udp header from skb->data!
			  header length is 8 byte*/
			recv_length = skb->len-8;
			_rtw_memset(recv_data,0,sizeof(recv_data));
			_rtw_memcpy(recv_data, skb->data+8, recv_length);

			DBG_871X("received data: %s :with len %u\n",recv_data, skb->len);
			parse_res = rtw_btcoex_parse_recv_data(recv_data,recv_length);
			DBG_871X("parse_res; %d\n",parse_res);
			if(RX_ATTEND_ACK == parse_res) { //attend ack
				pcoex_info ->BT_attend = _TRUE;
				DBG_871X("sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);

			} else if (RX_LEAVE_ACK == parse_res) { //mean BT know wifi  will leave
				pcoex_info ->BT_attend = _FALSE;
				DBG_871X("sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);

			} else if(RX_BT_LEAVE == parse_res) { //BT leave
				rtw_btcoex_sendmsgbysocket(pbtcoexadapter, tx_msg,sizeof(tx_msg)); // no ack
				pcoex_info ->BT_attend = _FALSE;
				//rtw_btcoex_close_kernel_socket(pbtcoexadapter); //Don't close or it will case soft lock
				DBG_871X("sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);
			}
#if 0
			else if(BT_INFO_NOTIFY_CMD == parse_res) {
				para_len = recv_data[2];
				polling_enable = recv_data[3];
				poling_interval = recv_data[4];
				reason = recv_data[5];
				btinfo_len = recv_data[6];
				if(btinfo_len != BT_INFO_LEN) {
					DBG_871X("%s: Error BT info length: %d\n",__func__,btinfo_len);
				} else {
					DBG_871X("para_len: %d\n",para_len);
					DBG_871X("polling_enable: %d\n",polling_enable);
					DBG_871X("poling_interval: %d\n",poling_interval);
					DBG_871X("reason: %d\n",reason);
					DBG_871X("btinfo_len: %d\n",btinfo_len);
					_rtw_memcpy(btinfo, &recv_data[7], btinfo_len);
					if(coex_info ->BT_attend == _TRUE) {
						rtw_btinfo_cmd(pbtcoexadapter,btinfo,btinfo_len);
					}

				}

			}
#endif
			else { //todo: check if recv data are really hci cmds
				rtw_btcoex_parse_hci_cmd(pbtcoexadapter,recv_data,recv_length);
				if(pcoex_info ->BT_attend == _TRUE) {
					//sendmsgbysocket("hello, this is wifi 1111!!",sizeof("hello, this is wifi 1111!!"));
					//msleep(20);
				}
			}
			len--;
			kfree_skb(skb);
			/*never do a sleep in this context!*/
		}
	}
}


u8 rtw_btcoex_sendmsgbysocket(_adapter *padapter, u8 *msg, u8 msg_size)
{
	u8 error;
	struct msghdr	udpmsg;
	mm_segment_t	oldfs;
	struct iovec	iov;
	struct bt_coex_info *pcoex_info = &padapter->coex_info;

	if(_TRUE == pcoex_info->BT_attend) {
		DBG_871X("%s msg:%s\n", __FUNCTION__,msg);
		DBG_871X("========> Padapter: %p\n",padapter);

		iov.iov_base	 = (void *)msg;
		iov.iov_len	 = msg_size;
		udpmsg.msg_name	 = &pcoex_info->sin;
		udpmsg.msg_namelen	= sizeof(struct sockaddr_in);
		udpmsg.msg_iov	 = &iov;
		udpmsg.msg_iovlen	= 1;
		udpmsg.msg_control	= NULL;
		udpmsg.msg_controllen = 0;
		udpmsg.msg_flags	= MSG_DONTWAIT | MSG_NOSIGNAL;
		oldfs = get_fs();
		set_fs(KERNEL_DS);
		error = sock_sendmsg(pcoex_info->udpsock, &udpmsg, msg_size);
		//rtw_msleep_os(20);
		set_fs(oldfs);
		if(error < 0) {
			DBG_871X("Error when sendimg msg, error:%d\n",error);
			return _FAIL;
		} else
			return _SUCCESS;
	} else {
		DBG_871X("Tx error: BT isn't up\n");
		return _FAIL;
	}

}

void rtw_btcoex_create_nl_socket(_adapter *padapter)
{
	struct bt_coex_info *pcoex_info = &padapter->coex_info;
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
	struct netlink_kernel_cfg nl_cfg = {
		.groups = 0,
		.input = rtw_btcoex_recvmsgbynetlink,
		.cb_mutex = NULL,
	};
	pcoex_info->pnl_cfg = &nl_cfg;
#endif

	DBG_871X("%s\n",__func__);

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3,5,0))
	pcoex_info->nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, rtw_btcoex_recvmsgbynetlink, NULL,THIS_MODULE);
#elif(LINUX_VERSION_CODE == KERNEL_VERSION(3,6,0))
	pcoex_info->nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, THIS_MODULE, &nl_cfg);
#elif(LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
	pcoex_info->nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &nl_cfg);
#endif

	if(!pcoex_info->nl_sk) {
		DBG_871X("Error creating netlink socket.\n");
		return ;
	} else {
		DBG_871X("Creating netlink socket successfully.\n");
		pcoex_info->sock_open |=  NETLINK_SOCKET_OK;
	}
}

u8 rtw_btcoex_create_kernel_socket(_adapter *padapter, u8 is_invite)
{
	s8 kernel_socket_err;
	u8 tx_msg[255] = attend_req;
	struct bt_coex_info *pcoex_info = &padapter->coex_info;
	DBG_871X("%s CONNECT_PORT %d\n",__func__,CONNECT_PORT);
	DBG_871X("========> Padapter: %p\n",padapter);

	if(NULL == pcoex_info) {
		DBG_871X("coex_info: NULL\n");
		return _FAIL;
	}

	kernel_socket_err = sock_create(PF_INET, SOCK_DGRAM, 0, &pcoex_info->udpsock);

	if (kernel_socket_err<0) {
		DBG_871X("Error during creation of socket error:%d\n",kernel_socket_err);
		return _FAIL;

	} else {
		_rtw_memset(&(pcoex_info->sin), 0, sizeof(pcoex_info->sin));
		pcoex_info->sin.sin_family = AF_INET;
		pcoex_info->sin.sin_port = htons(CONNECT_PORT);
		pcoex_info->sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

		kernel_socket_err = pcoex_info->udpsock->ops->bind(pcoex_info->udpsock,(struct sockaddr *)&pcoex_info->sin,sizeof(pcoex_info->sin));

		// there is one process using this IP(127.0.0.1), means BT is on.
		if(kernel_socket_err == -98) {
			DBG_871X("binding socket success\n");
			pcoex_info->udpsock->sk->sk_data_ready = rtw_btcoex_recvmsgbysocket;
			pcoex_info->sock_open |=  KERNEL_SOCKET_OK;
			pcoex_info->BT_attend = _TRUE;

			if(is_invite != _TRUE) // attend req
				rtw_btcoex_sendmsgbysocket(padapter,tx_msg,sizeof(tx_msg));

			return _SUCCESS;
		} else {
			pcoex_info->BT_attend = _FALSE;
			sock_release(pcoex_info->udpsock); // bind fail release socket
			DBG_871X("Error binding socket: %d\n",kernel_socket_err);
			return _FAIL;
		}

	}
}

void rtw_btcoex_close_nl_socket(_adapter *padapter)
{
	struct bt_coex_info *pcoex_info = &padapter->coex_info;
	if(pcoex_info->sock_open & NETLINK_SOCKET_OK) {
		DBG_871X("release netlink socket\n");
		netlink_kernel_release(pcoex_info->nl_sk);
		pcoex_info->sock_open &= ~(NETLINK_SOCKET_OK);
		if(_TRUE == pcoex_info->BT_attend)
			pcoex_info->BT_attend = _FALSE;

		DBG_871X("sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);
	}
}

void rtw_btcoex_close_kernel_socket(_adapter *padapter)
{
	struct bt_coex_info *pcoex_info = &padapter->coex_info;
	if(pcoex_info->sock_open & KERNEL_SOCKET_OK) {
		DBG_871X("release kernel socket\n");
		sock_release(pcoex_info->udpsock);
		pcoex_info->sock_open &= ~(KERNEL_SOCKET_OK);
		if(_TRUE == pcoex_info->BT_attend)
			pcoex_info->BT_attend = _FALSE;

		DBG_871X("sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);
	}
}

void rtw_btcoex_init_socket(_adapter *padapter)
{

	u8 is_invite = _FALSE;
	struct bt_coex_info *pcoex_info = &padapter->coex_info;
	DBG_871X("========> Padapter: %p\n",padapter);

	if(_FALSE == pcoex_info->is_exist) {
		_rtw_memset(pcoex_info,0,sizeof(struct bt_coex_info));
		pbtcoexadapter = padapter;
		rtw_btcoex_create_nl_socket(padapter);
		rtw_btcoex_create_kernel_socket(padapter,is_invite);
		pcoex_info->is_exist = _TRUE;
		DBG_871X("========> coex_info->is_exist: %d\n",pcoex_info->is_exist);
		DBG_871X("========> pbtcoexadapter: %p\n",pbtcoexadapter);
	}
}

void rtw_btcoex_close_socket(_adapter *padapter)
{
	u8 msg[255] = wifi_leave;
	struct bt_coex_info *pcoex_info = &padapter->coex_info;
	DBG_871X("========> coex_info->is_exist: %d\n",pcoex_info->is_exist);
	if( _TRUE == pcoex_info->is_exist) {
		if(pcoex_info->BT_attend == _TRUE) { //inform BT wifi leave
			rtw_btcoex_sendmsgbysocket(padapter,msg,sizeof(msg));
			msleep(50);
		}
		rtw_btcoex_close_nl_socket(padapter);
		rtw_btcoex_close_kernel_socket(padapter);
		pbtcoexadapter = NULL;
		pcoex_info->is_exist = _FALSE;
	}
}

void rtw_btcoex_dump_tx_msg(u8 *tx_msg, u8 len, u8 *msg_name)
{
	u8 	i = 0;
	DBG_871X("======> Msg name: %s\n",msg_name);
	for(i=0; i<len; i++) {
		printk("%02x ",tx_msg[i]);
	}
	printk("\n");
	DBG_871X("Msg name: %s <======\n",msg_name);
}

/* Porting from Windows team */
void rtw_btcoex_SendEventExtBtCoexControl(PADAPTER padapter, u8 bNeedDbgRsp, u8 dataLen, void *pData)
{
	u8			len=0, tx_event_length = 0;
	u8 			localBuf[32] = "";
	u8			*pRetPar;
	u8			opCode=0;
	u8			*pInBuf=(pu1Byte)pData;
	u8			*pOpCodeContent;
	rtw_HCI_event *pEvent;

	opCode = pInBuf[0];

	DBG_871X("%s, OPCode:%02x\n",__func__,opCode);

	pEvent = (rtw_HCI_event*)(&localBuf[0]);

	//len += bthci_ExtensionEventHeaderRtk(&localBuf[0],
	//	HCI_EVENT_EXT_BT_COEX_CONTROL);
	pEvent->EventCode = HCI_EVENT_EXTENSION_RTK;
	pEvent->Data[0] = HCI_EVENT_EXT_BT_COEX_CONTROL;	//extension event code
	len ++;

	// Return parameters starts from here
	pRetPar = &pEvent->Data[len];
	_rtw_memcpy(&pRetPar[0], pData, dataLen);

	len += dataLen;

	pEvent->Length = len;

	//total tx event length + EventCode length + sizeof(length)
	tx_event_length = pEvent->Length + 2;

	rtw_btcoex_dump_tx_msg((u8 *)pEvent, tx_event_length, "BT COEX CONTROL");

	rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);

}

/* Porting from Windows team */
void rtw_btcoex_SendEventExtBtInfoControl(PADAPTER padapter, u8 dataLen, void *pData)
{
	rtw_HCI_event *pEvent;
	u8			*pRetPar;
	u8			len=0, tx_event_length = 0;
	u8 			localBuf[32] = "";

	struct bt_coex_info *pcoex_info = &padapter->coex_info;
	PBT_MGNT		pBtMgnt = &pcoex_info->BtMgnt;

	DBG_871X("%s\n",__func__);
	if(pBtMgnt->ExtConfig.HCIExtensionVer < 4) { //not support
		DBG_871X("ERROR: HCIExtensionVer = %d, HCIExtensionVer<4 !!!!\n",pBtMgnt->ExtConfig.HCIExtensionVer);
		return;
	}

	pEvent = (rtw_HCI_event *)(&localBuf[0]);

	//len += bthci_ExtensionEventHeaderRtk(&localBuf[0],
	//		HCI_EVENT_EXT_BT_INFO_CONTROL);
	pEvent->EventCode = HCI_EVENT_EXTENSION_RTK;
	pEvent->Data[0] = HCI_EVENT_EXT_BT_INFO_CONTROL;		//extension event code
	len ++;

	// Return parameters starts from here
	pRetPar = &pEvent->Data[len];
	_rtw_memcpy(&pRetPar[0], pData, dataLen);

	len += dataLen;

	pEvent->Length = len;

	//total tx event length + EventCode length + sizeof(length)
	tx_event_length = pEvent->Length + 2;

	rtw_btcoex_dump_tx_msg((u8 *)pEvent, tx_event_length, "BT INFO CONTROL");

	rtw_btcoex_sendmsgbysocket(padapter,(u8 *)pEvent, tx_event_length);

}

void rtw_btcoex_SendScanNotify(PADAPTER padapter, u8 scanType)
{
	u8	len=0, tx_event_length=0;
	u8 	localBuf[7] = "";
	u8	*pRetPar;
	u8	*pu1Temp;
	rtw_HCI_event *pEvent;
	struct bt_coex_info *pcoex_info = &padapter->coex_info;
	PBT_MGNT		pBtMgnt = &pcoex_info->BtMgnt;

//	if(!pBtMgnt->BtOperationOn)
//		return;

	pEvent = (rtw_HCI_event *)(&localBuf[0]);

//	len += bthci_ExtensionEventHeaderRtk(&localBuf[0],
//			HCI_EVENT_EXT_WIFI_SCAN_NOTIFY);

	pEvent->EventCode = HCI_EVENT_EXTENSION_RTK;
	pEvent->Data[0] = HCI_EVENT_EXT_WIFI_SCAN_NOTIFY;		//extension event code
	len ++;

	// Return parameters starts from here
	//pRetPar = &PPacketIrpEvent->Data[len];
	//pu1Temp = (u8 *)&pRetPar[0];
	//*pu1Temp = scanType;
	pEvent->Data[len] = scanType;
	len += 1;

	pEvent->Length = len;

	//total tx event length + EventCode length + sizeof(length)
	tx_event_length = pEvent->Length + 2;

	rtw_btcoex_dump_tx_msg((u8 *)pEvent, tx_event_length, "WIFI SCAN OPERATION");

	rtw_btcoex_sendmsgbysocket(padapter, (u8 *)pEvent, tx_event_length);
}
#endif //CONFIG_BT_COEXIST_SOCKET_TRX
#endif // CONFIG_BT_COEXIST
