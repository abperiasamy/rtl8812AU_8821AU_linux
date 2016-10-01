/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
#define _HAL_COM_C_

#include <drv_types.h>

#include "../hal/OUTSRC/odm_precomp.h"


void dump_chip_info(HAL_VERSION	ChipVersion)
{
	int cnt = 0;
	u8 buf[128];
	
	if(IS_81XXC(ChipVersion)){
		cnt += sprintf((buf+cnt), "Chip Version Info: %s_", IS_92C_SERIAL(ChipVersion)?"CHIP_8192C":"CHIP_8188C");
	}
	else if(IS_92D(ChipVersion)){
		cnt += sprintf((buf+cnt), "Chip Version Info: CHIP_8192D_");
	}
	else if(IS_8723_SERIES(ChipVersion)){
		cnt += sprintf((buf+cnt), "Chip Version Info: CHIP_8723A_");
	}
	else if(IS_8188E(ChipVersion)){
		cnt += sprintf((buf+cnt), "Chip Version Info: CHIP_8188E_");
	}
	else if(IS_8812_SERIES(ChipVersion)){
		cnt += sprintf((buf+cnt), "Chip Version Info: CHIP_8812_");
	}
    else if(IS_8192E(ChipVersion)){
		cnt += sprintf((buf+cnt), "Chip Version Info: CHIP_8192E_");
	}
	else if(IS_8821_SERIES(ChipVersion)){
		cnt += sprintf((buf+cnt), "Chip Version Info: CHIP_8821_");
	}
	else if(IS_8723B_SERIES(ChipVersion)){
		cnt += sprintf((buf+cnt), "Chip Version Info: CHIP_8723B_");
	}

	cnt += sprintf((buf+cnt), "%s_", IS_NORMAL_CHIP(ChipVersion)?"Normal_Chip":"Test_Chip");
	if(IS_CHIP_VENDOR_TSMC(ChipVersion))
		cnt += sprintf((buf+cnt), "%s_","TSMC");
	else if(IS_CHIP_VENDOR_UMC(ChipVersion))	
		cnt += sprintf((buf+cnt), "%s_","UMC");
	else if(IS_CHIP_VENDOR_SMIC(ChipVersion))
		cnt += sprintf((buf+cnt), "%s_","SMIC");		
	
	if(IS_A_CUT(ChipVersion)) cnt += sprintf((buf+cnt), "A_CUT_");
	else if(IS_B_CUT(ChipVersion)) cnt += sprintf((buf+cnt), "B_CUT_");
	else if(IS_C_CUT(ChipVersion)) cnt += sprintf((buf+cnt), "C_CUT_");
	else if(IS_D_CUT(ChipVersion)) cnt += sprintf((buf+cnt), "D_CUT_");
	else if(IS_E_CUT(ChipVersion)) cnt += sprintf((buf+cnt), "E_CUT_");
	else cnt += sprintf((buf+cnt), "UNKNOWN_CUT(%d)_", ChipVersion.CUTVersion);

	if(IS_1T1R(ChipVersion)) cnt += sprintf((buf+cnt), "1T1R_");
	else if(IS_1T2R(ChipVersion)) cnt += sprintf((buf+cnt), "1T2R_");
	else if(IS_2T2R(ChipVersion)) cnt += sprintf((buf+cnt), "2T2R_");
	else cnt += sprintf((buf+cnt), "UNKNOWN_RFTYPE(%d)_", ChipVersion.RFType);

	cnt += sprintf((buf+cnt), "RomVer(%d)\n", ChipVersion.ROMVer);

	DBG_871X("%s", buf);
}


#define	EEPROM_CHANNEL_PLAN_BY_HW_MASK	0x80

u8	//return the final channel plan decision
hal_com_get_channel_plan(
	IN	PADAPTER	padapter,
	IN	u8			hw_channel_plan,	//channel plan from HW (efuse/eeprom)
	IN	u8			sw_channel_plan,	//channel plan from SW (registry/module param)
	IN	u8			def_channel_plan,	//channel plan used when the former two is invalid
	IN	BOOLEAN		AutoLoadFail
	)
{
	u8 swConfig;
	u8 chnlPlan;

	swConfig = _TRUE;
	if (!AutoLoadFail)
	{
		if (!rtw_is_channel_plan_valid(sw_channel_plan))
			swConfig = _FALSE;
		if (hw_channel_plan & EEPROM_CHANNEL_PLAN_BY_HW_MASK)
			swConfig = _FALSE;
	}

	if (swConfig == _TRUE)
		chnlPlan = sw_channel_plan;
	else
		chnlPlan = hw_channel_plan & (~EEPROM_CHANNEL_PLAN_BY_HW_MASK);

	if (!rtw_is_channel_plan_valid(chnlPlan))
		chnlPlan = def_channel_plan;

	return chnlPlan;
}

BOOLEAN
HAL_IsLegalChannel(
	IN	PADAPTER	Adapter,
	IN	u32			Channel
	)
{
	BOOLEAN bLegalChannel = _TRUE;

	if (Channel > 14) {
		if(IsSupported5G(Adapter->registrypriv.wireless_mode) == _FALSE) {
			bLegalChannel = _FALSE;
			DBG_871X("Channel > 14 but wireless_mode do not support 5G\n");
		}
	} else if ((Channel <= 14) && (Channel >=1)){
		if(IsSupported24G(Adapter->registrypriv.wireless_mode) == _FALSE) {
			bLegalChannel = _FALSE;
			DBG_871X("(Channel <= 14) && (Channel >=1) but wireless_mode do not support 2.4G\n");
		}
	} else {
		bLegalChannel = _FALSE;
		DBG_871X("Channel is Invalid !!!\n");
	}

	return bLegalChannel;
}	

u8	MRateToHwRate(u8 rate)
{
	u8	ret = DESC_RATE1M;
		
	switch(rate)
	{
		// CCK and OFDM non-HT rates
	case IEEE80211_CCK_RATE_1MB:	ret = DESC_RATE1M;	break;
	case IEEE80211_CCK_RATE_2MB:	ret = DESC_RATE2M;	break;
	case IEEE80211_CCK_RATE_5MB:	ret = DESC_RATE5_5M;	break;
	case IEEE80211_CCK_RATE_11MB:	ret = DESC_RATE11M;	break;
	case IEEE80211_OFDM_RATE_6MB:	ret = DESC_RATE6M;	break;
	case IEEE80211_OFDM_RATE_9MB:	ret = DESC_RATE9M;	break;
	case IEEE80211_OFDM_RATE_12MB:	ret = DESC_RATE12M;	break;
	case IEEE80211_OFDM_RATE_18MB:	ret = DESC_RATE18M;	break;
	case IEEE80211_OFDM_RATE_24MB:	ret = DESC_RATE24M;	break;
	case IEEE80211_OFDM_RATE_36MB:	ret = DESC_RATE36M;	break;
	case IEEE80211_OFDM_RATE_48MB:	ret = DESC_RATE48M;	break;
	case IEEE80211_OFDM_RATE_54MB:	ret = DESC_RATE54M;	break;

		// HT rates since here
	//case MGN_MCS0:		ret = DESC_RATEMCS0;	break;
	//case MGN_MCS1:		ret = DESC_RATEMCS1;	break;
	//case MGN_MCS2:		ret = DESC_RATEMCS2;	break;
	//case MGN_MCS3:		ret = DESC_RATEMCS3;	break;
	//case MGN_MCS4:		ret = DESC_RATEMCS4;	break;
	//case MGN_MCS5:		ret = DESC_RATEMCS5;	break;
	//case MGN_MCS6:		ret = DESC_RATEMCS6;	break;
	//case MGN_MCS7:		ret = DESC_RATEMCS7;	break;

	default:		break;
	}

	return ret;
}

void	HalSetBrateCfg(
	IN PADAPTER		Adapter,
	IN u8			*mBratesOS,
	OUT u16			*pBrateCfg)
{
	u8	i, is_brate, brate;

	for(i=0;i<NDIS_802_11_LENGTH_RATES_EX;i++)
	{
		is_brate = mBratesOS[i] & IEEE80211_BASIC_RATE_MASK;
		brate = mBratesOS[i] & 0x7f;
		
		if( is_brate )
		{		
			switch(brate)
			{
				case IEEE80211_CCK_RATE_1MB:	*pBrateCfg |= RATE_1M;	break;
				case IEEE80211_CCK_RATE_2MB:	*pBrateCfg |= RATE_2M;	break;
				case IEEE80211_CCK_RATE_5MB:	*pBrateCfg |= RATE_5_5M;break;
				case IEEE80211_CCK_RATE_11MB:	*pBrateCfg |= RATE_11M;	break;
				case IEEE80211_OFDM_RATE_6MB:	*pBrateCfg |= RATE_6M;	break;
				case IEEE80211_OFDM_RATE_9MB:	*pBrateCfg |= RATE_9M;	break;
				case IEEE80211_OFDM_RATE_12MB:	*pBrateCfg |= RATE_12M;	break;
				case IEEE80211_OFDM_RATE_18MB:	*pBrateCfg |= RATE_18M;	break;
				case IEEE80211_OFDM_RATE_24MB:	*pBrateCfg |= RATE_24M;	break;
				case IEEE80211_OFDM_RATE_36MB:	*pBrateCfg |= RATE_36M;	break;
				case IEEE80211_OFDM_RATE_48MB:	*pBrateCfg |= RATE_48M;	break;
				case IEEE80211_OFDM_RATE_54MB:	*pBrateCfg |= RATE_54M;	break;
			}
		}
	}
}

static VOID
_OneOutPipeMapping(
	IN	PADAPTER	pAdapter
	)
{
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(pAdapter);

	pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0];//VO
	pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[0];//VI
	pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[0];//BE
	pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[0];//BK
	
	pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0];//BCN
	pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0];//MGT
	pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0];//HIGH
	pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0];//TXCMD
}

static VOID
_TwoOutPipeMapping(
	IN	PADAPTER	pAdapter,
	IN	BOOLEAN	 	bWIFICfg
	)
{
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(pAdapter);

	if(bWIFICfg){ //WMM
		
		//	BK, 	BE, 	VI, 	VO, 	BCN,	CMD,MGT,HIGH,HCCA 
		//{  0, 	1, 	0, 	1, 	0, 	0, 	0, 	0, 		0	};
		//0:H, 1:N 
		
		pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[1];//VO
		pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[0];//VI
		pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[1];//BE
		pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[0];//BK
		
		pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0];//BCN
		pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0];//MGT
		pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0];//HIGH
		pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0];//TXCMD
		
	}
	else{//typical setting

		
		//BK, 	BE, 	VI, 	VO, 	BCN,	CMD,MGT,HIGH,HCCA 
		//{  1, 	1, 	0, 	0, 	0, 	0, 	0, 	0, 		0	};			
		//0:H, 1:N 
		
		pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0];//VO
		pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[0];//VI
		pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[1];//BE
		pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[1];//BK
		
		pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0];//BCN
		pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0];//MGT
		pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0];//HIGH
		pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0];//TXCMD	
		
	}
	
}

static VOID _ThreeOutPipeMapping(
	IN	PADAPTER	pAdapter,
	IN	BOOLEAN	 	bWIFICfg
	)
{
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(pAdapter);

	if(bWIFICfg){//for WMM
		
		//	BK, 	BE, 	VI, 	VO, 	BCN,	CMD,MGT,HIGH,HCCA 
		//{  1, 	2, 	1, 	0, 	0, 	0, 	0, 	0, 		0	};
		//0:H, 1:N, 2:L 
		
		pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0];//VO
		pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[1];//VI
		pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[2];//BE
		pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[1];//BK
		
		pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0];//BCN
		pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0];//MGT
		pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0];//HIGH
		pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0];//TXCMD
		
	}
	else{//typical setting

		
		//	BK, 	BE, 	VI, 	VO, 	BCN,	CMD,MGT,HIGH,HCCA 
		//{  2, 	2, 	1, 	0, 	0, 	0, 	0, 	0, 		0	};			
		//0:H, 1:N, 2:L 
		
		pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0];//VO
		pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[1];//VI
		pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[2];//BE
		pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[2];//BK
		
		pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0];//BCN
		pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0];//MGT
		pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[0];//HIGH
		pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0];//TXCMD	
	}

}
static inline VOID _FourOutPipeMapping(
	IN	PADAPTER	pAdapter,
	IN	BOOLEAN	 	bWIFICfg
	)
{
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(pAdapter);

	if(bWIFICfg){//for WMM
		
		//	BK, 	BE, 	VI, 	VO, 	BCN,	CMD,MGT,HIGH,HCCA 
		//{  1, 	2, 	1, 	0, 	0, 	0, 	0, 	0, 		0	};
		//0:H, 1:N, 2:L ,3:E
		
		pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0];//VO
		pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[1];//VI
		pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[2];//BE
		pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[1];//BK
		
		pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0];//BCN
		pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0];//MGT
		pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[3];//HIGH
		pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0];//TXCMD
		
	}
	else{//typical setting

		
		//	BK, 	BE, 	VI, 	VO, 	BCN,	CMD,MGT,HIGH,HCCA 
		//{  2, 	2, 	1, 	0, 	0, 	0, 	0, 	0, 		0	};			
		//0:H, 1:N, 2:L 
		
		pdvobjpriv->Queue2Pipe[0] = pdvobjpriv->RtOutPipe[0];//VO
		pdvobjpriv->Queue2Pipe[1] = pdvobjpriv->RtOutPipe[1];//VI
		pdvobjpriv->Queue2Pipe[2] = pdvobjpriv->RtOutPipe[2];//BE
		pdvobjpriv->Queue2Pipe[3] = pdvobjpriv->RtOutPipe[2];//BK
		
		pdvobjpriv->Queue2Pipe[4] = pdvobjpriv->RtOutPipe[0];//BCN
		pdvobjpriv->Queue2Pipe[5] = pdvobjpriv->RtOutPipe[0];//MGT
		pdvobjpriv->Queue2Pipe[6] = pdvobjpriv->RtOutPipe[3];//HIGH
		pdvobjpriv->Queue2Pipe[7] = pdvobjpriv->RtOutPipe[0];//TXCMD	
	}

}
BOOLEAN
Hal_MappingOutPipe(
	IN	PADAPTER	pAdapter,
	IN	u8		NumOutPipe
	)
{
	struct registry_priv *pregistrypriv = &pAdapter->registrypriv;

	BOOLEAN	 bWIFICfg = (pregistrypriv->wifi_spec) ?_TRUE:_FALSE;
	
	BOOLEAN result = _TRUE;

	switch(NumOutPipe)
	{
		case 2:
			_TwoOutPipeMapping(pAdapter, bWIFICfg);
			break;
		case 3:
		case 4:
			_ThreeOutPipeMapping(pAdapter, bWIFICfg);
			break;			
		case 1:
			_OneOutPipeMapping(pAdapter);
			break;
		default:
			result = _FALSE;
			break;
	}

	return result;
	
}

void hal_init_macaddr(_adapter *adapter)
{
	rtw_hal_set_hwreg(adapter, HW_VAR_MAC_ADDR, adapter->eeprompriv.mac_addr);
#ifdef  CONFIG_CONCURRENT_MODE
	if (adapter->pbuddy_adapter)
		rtw_hal_set_hwreg(adapter->pbuddy_adapter, HW_VAR_MAC_ADDR, adapter->pbuddy_adapter->eeprompriv.mac_addr);
#endif
}

/* 
* C2H event format:
* Field	 TRIGGER		CONTENT	   CMD_SEQ 	CMD_LEN		 CMD_ID
* BITS	 [127:120]	[119:16]      [15:8]		  [7:4]	 	   [3:0]
*/

void c2h_evt_clear(_adapter *adapter)
{
	rtw_write8(adapter, REG_C2HEVT_CLEAR, C2H_EVT_HOST_CLOSE);
}

s32 c2h_evt_read(_adapter *adapter, u8 *buf)
{
	s32 ret = _FAIL;
	struct c2h_evt_hdr *c2h_evt;
	int i;
	u8 trigger;

	if (buf == NULL)
		goto exit;

	trigger = rtw_read8(adapter, REG_C2HEVT_CLEAR);

	if (trigger == C2H_EVT_HOST_CLOSE) {
		goto exit; /* Not ready */
	} else if (trigger != C2H_EVT_FW_CLOSE) {
		goto clear_evt; /* Not a valid value */
	}

	c2h_evt = (struct c2h_evt_hdr *)buf;

	_rtw_memset(c2h_evt, 0, 16);

	*buf = rtw_read8(adapter, REG_C2HEVT_MSG_NORMAL);
	*(buf+1) = rtw_read8(adapter, REG_C2HEVT_MSG_NORMAL + 1);	

	RT_PRINT_DATA(_module_hal_init_c_, _drv_info_, "c2h_evt_read(): ",
		&c2h_evt , sizeof(c2h_evt));

	if (0) {
		DBG_871X("%s id:%u, len:%u, seq:%u, trigger:0x%02x\n", __func__
			, c2h_evt->id, c2h_evt->plen, c2h_evt->seq, trigger);
	}

	/* Read the content */
	for (i = 0; i < c2h_evt->plen; i++)
		c2h_evt->payload[i] = rtw_read8(adapter, REG_C2HEVT_MSG_NORMAL + sizeof(*c2h_evt) + i);

	RT_PRINT_DATA(_module_hal_init_c_, _drv_info_, "c2h_evt_read(): Command Content:\n",
		c2h_evt->payload, c2h_evt->plen);

	ret = _SUCCESS;

clear_evt:
	/* 
	* Clear event to notify FW we have read the command.
	* If this field isn't clear, the FW won't update the next command message.
	*/
	c2h_evt_clear(adapter);
exit:
	return ret;
}

u8  rtw_hal_networktype_to_raid(_adapter *adapter,unsigned char network_type)
{
	if(IS_NEW_GENERATION_IC(adapter)){
		return networktype_to_raid_ex(adapter,network_type);
	}
	else{
		return networktype_to_raid(adapter,network_type);
	}

}
u8 rtw_get_mgntframe_raid(_adapter *adapter,unsigned char network_type)
{	

	u8 raid;
	if(IS_NEW_GENERATION_IC(adapter)){
		
		raid = (network_type & WIRELESS_11B)	?RATEID_IDX_B
											:RATEID_IDX_G;		
	}
	else{
		raid = (network_type & WIRELESS_11B)	?RATR_INX_WIRELESS_B
											:RATR_INX_WIRELESS_G;		
	}	
	return raid;
}


