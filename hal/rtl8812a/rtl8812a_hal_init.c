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
#define _RTL8812A_HAL_INIT_C_

//#include <drv_types.h>
#include <rtl8812a_hal.h>


#if defined(CONFIG_IOL)
void iol_mode_enable(PADAPTER padapter, u8 enable)
{
	u8 reg_0xf0 = 0;
	
	if(enable)
	{
		//Enable initial offload
		reg_0xf0 = rtw_read8(padapter, REG_SYS_CFG);
		//DBG_871X("%s reg_0xf0:0x%02x, write 0x%02x\n", __FUNCTION__, reg_0xf0, reg_0xf0|IOL_ENABLE);
		rtw_write8(padapter, REG_SYS_CFG, reg_0xf0|SW_OFFLOAD_EN);

		_8051Reset8812(padapter);
	}
	else
	{
		//disable initial offload
		reg_0xf0 = rtw_read8(padapter, REG_SYS_CFG);
		//DBG_871X("%s reg_0xf0:0x%02x, write 0x%02x\n", __FUNCTION__, reg_0xf0, reg_0xf0& ~IOL_ENABLE);
		rtw_write8(padapter, REG_SYS_CFG, reg_0xf0 & ~SW_OFFLOAD_EN);
	}
}

s32 iol_execute(PADAPTER padapter, u8 control)
{
	s32 status = _FAIL;
	u8 reg_0x88 = 0;
	u32 start = 0, passing_time = 0;
	control = control&0x0f;
	
	reg_0x88 = rtw_read8(padapter, 0x88);
	//DBG_871X("%s reg_0x88:0x%02x, write 0x%02x\n", __FUNCTION__, reg_0x88, reg_0x88|control);
	rtw_write8(padapter, 0x88,  reg_0x88|control);

	start = rtw_get_current_time();
	while((reg_0x88=rtw_read8(padapter, 0x88)) & control
		&& (passing_time=rtw_get_passing_time_ms(start))<1000
	) {
		DBG_871X("%s polling reg_0x88:0x%02x\n", __FUNCTION__, reg_0x88);
		rtw_usleep_os(100);
	}

	reg_0x88 = rtw_read8(padapter, 0x88);
	status = (reg_0x88 & control)?_FAIL:_SUCCESS;
	if(reg_0x88 & control<<4)
		status = _FAIL;

	DBG_871X("%s in %u ms, reg_0x88:0x%02x\n", __FUNCTION__, passing_time, reg_0x88);
	
	return status;
}

s32 iol_InitLLTTable(
	PADAPTER padapter,
	u8 txpktbuf_bndy
	)
{
	//DBG_871X("%s txpktbuf_bndy:%u\n", __FUNCTION__, txpktbuf_bndy);
	rtw_write8(padapter, REG_TDECTRL+1, txpktbuf_bndy);
	return iol_execute(padapter, IOL_INIT_LLT);
}

static VOID
efuse_phymap_to_logical(u8 * phymap, u16 _offset, u16 _size_byte, u8  *pbuf)
{
	u8	*efuseTbl = NULL;
	u8	rtemp8;
	u16	eFuse_Addr = 0;
	u8	offset, wren;
	u16	i, j;
	u16	**eFuseWord = NULL;
	u16	efuse_utilized = 0;
	u8	efuse_usage = 0;
	u8	u1temp = 0;


	efuseTbl = (u8*)rtw_zmalloc(EFUSE_MAP_LEN_JAGUAR);
	if(efuseTbl == NULL)
	{
		DBG_871X("%s: alloc efuseTbl fail!\n", __FUNCTION__);
		goto exit;
	}

	eFuseWord= (u16 **)rtw_malloc2d(EFUSE_MAX_SECTION_JAGUAR, EFUSE_MAX_WORD_UNIT, sizeof(u16));
	if(eFuseWord == NULL)
	{
		DBG_871X("%s: alloc eFuseWord fail!\n", __FUNCTION__);
		goto exit;
	}

	// 0. Refresh efuse init map as all oxFF.
	for (i = 0; i < EFUSE_MAX_SECTION_JAGUAR; i++)
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++)
			eFuseWord[i][j] = 0xFFFF;

	//
	// 1. Read the first byte to check if efuse is empty!!!
	// 
	//
	rtemp8 = *(phymap+eFuse_Addr);
	if(rtemp8 != 0xFF)
	{
		efuse_utilized++;
		//printk("efuse_Addr-%d efuse_data=%x\n", eFuse_Addr, *rtemp8);
		eFuse_Addr++;
	}
	else
	{
		DBG_871X("EFUSE is empty efuse_Addr-%d efuse_data=%x\n", eFuse_Addr, rtemp8);
		goto exit;
	}


	//
	// 2. Read real efuse content. Filter PG header and every section data.
	//
	while((rtemp8 != 0xFF) && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN_JAGUAR))
	{
		//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("efuse_Addr-%d efuse_data=%x\n", eFuse_Addr-1, *rtemp8));
	
		// Check PG header for section num.
		if((rtemp8 & 0x1F ) == 0x0F)		//extended header
		{			
			u1temp =( (rtemp8 & 0xE0) >> 5);
			//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("extended header u1temp=%x *rtemp&0xE0 0x%x\n", u1temp, *rtemp8 & 0xE0));

			//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("extended header u1temp=%x \n", u1temp));

			rtemp8 = *(phymap+eFuse_Addr);

			//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("extended header efuse_Addr-%d efuse_data=%x\n", eFuse_Addr, *rtemp8));	
			
			if((rtemp8 & 0x0F) == 0x0F)
			{
				eFuse_Addr++;			
				rtemp8 = *(phymap+eFuse_Addr);
				
				if(rtemp8 != 0xFF && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN_JAGUAR))
				{
					eFuse_Addr++;				
				}				
				continue;
			}
			else
			{
				offset = ((rtemp8 & 0xF0) >> 1) | u1temp;
				wren = (rtemp8 & 0x0F);
				eFuse_Addr++;				
			}
		}
		else
		{
			offset = ((rtemp8 >> 4) & 0x0f);
			wren = (rtemp8 & 0x0f);			
		}
		
		if(offset < EFUSE_MAX_SECTION_JAGUAR)
		{
			// Get word enable value from PG header
			//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("Offset-%d Worden=%x\n", offset, wren));

			for(i=0; i<EFUSE_MAX_WORD_UNIT; i++)
			{
				// Check word enable condition in the section				
				if(!(wren & 0x01))
				{
					//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("Addr=%d \n", eFuse_Addr));
					rtemp8 = *(phymap+eFuse_Addr);
					eFuse_Addr++;
					//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("Data=0x%x\n", *rtemp8)); 				
					efuse_utilized++;
					eFuseWord[offset][i] = (rtemp8 & 0xff);
					

					if(eFuse_Addr >= EFUSE_REAL_CONTENT_LEN_JAGUAR) 
						break;

					//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("Addr=%d", eFuse_Addr));
					rtemp8 = *(phymap+eFuse_Addr);
					eFuse_Addr++;
					//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("Data=0x%x\n", *rtemp8)); 				
					
					efuse_utilized++;
					eFuseWord[offset][i] |= (((u2Byte)rtemp8 << 8) & 0xff00);

					if(eFuse_Addr >= EFUSE_REAL_CONTENT_LEN_JAGUAR) 
						break;
				}
				
				wren >>= 1;
				
			}
		}

		// Read next PG header
		rtemp8 = *(phymap+eFuse_Addr);
		//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("Addr=%d rtemp 0x%x\n", eFuse_Addr, *rtemp8));
		
		if(rtemp8 != 0xFF && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN_JAGUAR))
		{
			efuse_utilized++;
			eFuse_Addr++;
		}
	}

	//
	// 3. Collect 16 sections and 4 word unit into Efuse map.
	//
	for(i=0; i<EFUSE_MAX_SECTION_JAGUAR; i++)
	{
		for(j=0; j<EFUSE_MAX_WORD_UNIT; j++)
		{
			efuseTbl[(i*8)+(j*2)]=(eFuseWord[i][j] & 0xff);
			efuseTbl[(i*8)+((j*2)+1)]=((eFuseWord[i][j] >> 8) & 0xff);
		}
	}


	//
	// 4. Copy from Efuse map to output pointer memory!!!
	//
	for(i=0; i<_size_byte; i++)
	{		
		pbuf[i] = efuseTbl[_offset+i];
	}

	//
	// 5. Calculate Efuse utilization.
	//
	efuse_usage = (u1Byte)((efuse_utilized*100)/EFUSE_REAL_CONTENT_LEN_JAGUAR);
	//Adapter->HalFunc.SetHwRegHandler(Adapter, HW_VAR_EFUSE_BYTES, (u8 *)&efuse_utilized);

exit:
	if(efuseTbl)
		rtw_mfree(efuseTbl, EFUSE_MAP_LEN_JAGUAR);

	if(eFuseWord)
		rtw_mfree2d((void *)eFuseWord, EFUSE_MAX_SECTION_JAGUAR, EFUSE_MAX_WORD_UNIT, sizeof(u16));
}

void efuse_read_phymap_from_txpktbuf(
	ADAPTER *adapter,
	int bcnhead,	//beacon head, where FW store len(2-byte) and efuse physical map.
	u8 *content,	//buffer to store efuse physical map
	u16 *size	//for efuse content: the max byte to read. will update to byte read
	)
{
	u16 dbg_addr = 0;
	u32 start  = 0, passing_time = 0;
	u8 reg_0x143 = 0;
	u8 reg_0x106 = 0;
	u32 lo32 = 0, hi32 = 0;
	u16 len = 0, count = 0;
	int i = 0;
	u16 limit = *size;

	u8 *pos = content;
	
	if(bcnhead<0) //if not valid
		bcnhead = rtw_read8(adapter, REG_TDECTRL+1);

	DBG_871X("%s bcnhead:%d\n", __FUNCTION__, bcnhead);

	//reg_0x106 = rtw_read8(adapter, REG_PKT_BUFF_ACCESS_CTRL);
	//DBG_871X("%s reg_0x106:0x%02x, write 0x%02x\n", __FUNCTION__, reg_0x106, 0x69);
	rtw_write8(adapter, REG_PKT_BUFF_ACCESS_CTRL, TXPKT_BUF_SELECT);
	//DBG_871X("%s reg_0x106:0x%02x\n", __FUNCTION__, rtw_read8(adapter, 0x106));

	dbg_addr = bcnhead*128/8; //8-bytes addressing

	while(1)
	{
		//DBG_871X("%s dbg_addr:0x%x\n", __FUNCTION__, dbg_addr+i);
		rtw_write16(adapter, REG_PKTBUF_DBG_ADDR, dbg_addr+i);

		//DBG_871X("%s write reg_0x143:0x00\n", __FUNCTION__);
		rtw_write8(adapter, REG_TXPKTBUF_DBG, 0);
		start = rtw_get_current_time();
		while(!(reg_0x143=rtw_read8(adapter, REG_TXPKTBUF_DBG))//dbg
		//while(rtw_read8(adapter, REG_TXPKTBUF_DBG) & BIT0
			&& (passing_time=rtw_get_passing_time_ms(start))<1000
		) {
			DBG_871X("%s polling reg_0x143:0x%02x, reg_0x106:0x%02x\n", __FUNCTION__, reg_0x143, rtw_read8(adapter, 0x106));
			rtw_usleep_os(100);
		}


		lo32 = rtw_read32(adapter, REG_PKTBUF_DBG_DATA_L);
		hi32 = rtw_read32(adapter, REG_PKTBUF_DBG_DATA_H);

		#if 0
		DBG_871X("%s lo32:0x%08x, %02x %02x %02x %02x\n", __FUNCTION__, lo32
			, rtw_read8(adapter, REG_PKTBUF_DBG_DATA_L)
			, rtw_read8(adapter, REG_PKTBUF_DBG_DATA_L+1)
			, rtw_read8(adapter, REG_PKTBUF_DBG_DATA_L+2)
			, rtw_read8(adapter, REG_PKTBUF_DBG_DATA_L+3)
		);
		DBG_871X("%s hi32:0x%08x, %02x %02x %02x %02x\n", __FUNCTION__, hi32
			, rtw_read8(adapter, REG_PKTBUF_DBG_DATA_H)
			, rtw_read8(adapter, REG_PKTBUF_DBG_DATA_H+1)
			, rtw_read8(adapter, REG_PKTBUF_DBG_DATA_H+2)
			, rtw_read8(adapter, REG_PKTBUF_DBG_DATA_H+3)
		);
		#endif

		if(i==0)
		{
			#if 1 //for debug
			u8 lenc[2];
			u16 lenbak, aaabak;
			u16 aaa;
			lenc[0] = rtw_read8(adapter, REG_PKTBUF_DBG_DATA_L);
			lenc[1] = rtw_read8(adapter, REG_PKTBUF_DBG_DATA_L+1);

			aaabak = le16_to_cpup((u16*)lenc);
			lenbak = le16_to_cpu(*((u16*)lenc));
			aaa = le16_to_cpup((u16*)&lo32);
			#endif
			len = le16_to_cpu(*((u16*)&lo32));

			limit = len-2<limit?len-2:limit;

			DBG_871X("%s len:%u, lenbak:%u, aaa:%u, aaabak:%u\n", __FUNCTION__, len, lenbak, aaa, aaabak);

			_rtw_memcpy(pos, ((u8*)&lo32)+2, limit>=count+2?2:limit-count);
			count+=limit>=count+2?2:limit-count;
			pos=content+count;
			
		}
		else
		{
			_rtw_memcpy(pos, ((u8*)&lo32), limit>=count+4?4:limit-count);
			count+=limit>=count+4?4:limit-count;
			pos=content+count;
			

		}

		if(limit>count && len-2>count) {
			_rtw_memcpy(pos, (u8*)&hi32, limit>=count+4?4:limit-count);
			count+=limit>=count+4?4:limit-count;
			pos=content+count;
		}

		if(limit<=count || len-2<=count)
			break;

		i++;
	}

	rtw_write8(adapter, REG_PKT_BUFF_ACCESS_CTRL, DISABLE_TRXPKT_BUF_ACCESS);
	
	DBG_871X("%s read count:%u\n", __FUNCTION__, count);
	*size = count;

}


static bool efuse_read_phymap(
	PADAPTER	Adapter,
	u8			*pbuf,	//buffer to store efuse physical map
	u16			*size	//the max byte to read. will update to byte read
	)
{
	u8 *pos = pbuf;
	u16 limit = *size;
	u16 addr = 0;
	bool reach_end = _FALSE;

	//
	// Refresh efuse init map as all 0xFF.
	//
	_rtw_memset(pbuf, 0xFF, limit);
		
	
	//
	// Read physical efuse content.
	//
	while(addr < limit)
	{
		ReadEFuseByte(Adapter, addr, pos, _FALSE);
		if(*pos != 0xFF)
		{
			pos++;
			addr++;
		}
		else
		{
			reach_end = _TRUE;
			break;
		}
	}

	*size = addr;

	return reach_end;

}

s32 iol_read_efuse(
	PADAPTER padapter,
	u8 txpktbuf_bndy,
	u16 offset,
	u16 size_byte,
	u8 *logical_map
	)
{
	s32 status = _FAIL;
	u8 reg_0x106 = 0;
	u8 physical_map[512];
	u16 size = 512;
	int i;


	rtw_write8(padapter, REG_TDECTRL+1, txpktbuf_bndy);
	_rtw_memset(physical_map, 0xFF, 512);
	
	///reg_0x106 = rtw_read8(padapter, REG_PKT_BUFF_ACCESS_CTRL);
	//DBG_871X("%s reg_0x106:0x%02x, write 0x%02x\n", __FUNCTION__, reg_0x106, 0x69);
	rtw_write8(padapter, REG_PKT_BUFF_ACCESS_CTRL, TXPKT_BUF_SELECT);
	//DBG_871X("%s reg_0x106:0x%02x\n", __FUNCTION__, rtw_read8(padapter, 0x106));

	status = iol_execute(padapter, IOL_READ_EFUSE_MAP);

	if(status == _SUCCESS)
		efuse_read_phymap_from_txpktbuf(padapter, txpktbuf_bndy, physical_map, &size);

	#if 0
	DBG_871X("%s physical map\n", __FUNCTION__);
	for(i=0;i<size;i++)
	{
		DBG_871X("%02x ", physical_map[i]);
		if(i%16==15)
			DBG_871X("\n");
	}
	DBG_871X("\n");
	#endif

	efuse_phymap_to_logical(physical_map, offset, size_byte, logical_map);

	return status;
}
#endif /* defined(CONFIG_IOL) */

//-------------------------------------------------------------------------
//
// LLT R/W/Init function
//
//-------------------------------------------------------------------------
static s32 _LLTWrite(PADAPTER padapter, u32 address, u32 data)
{
	s32	status = _SUCCESS;
	s32	count = 0;
	u32	value = _LLT_INIT_ADDR(address) | _LLT_INIT_DATA(data) | _LLT_OP(_LLT_WRITE_ACCESS);
	u16	LLTReg = REG_LLT_INIT;


	rtw_write32(padapter, LLTReg, value);

	//polling
	do {
		value = rtw_read32(padapter, LLTReg);
		if (_LLT_NO_ACTIVE == _LLT_OP_VALUE(value)) {
			break;
		}

		if (count > POLLING_LLT_THRESHOLD) {
			RT_TRACE(_module_hal_init_c_, _drv_err_, ("Failed to polling write LLT done at address %d!\n", address));
			status = _FAIL;
			break;
		}
	} while (count++);

	return status;
}

u8 _LLTRead(PADAPTER padapter, u32 address)
{
	s32	count = 0;
	u32	value = _LLT_INIT_ADDR(address) | _LLT_OP(_LLT_READ_ACCESS);
	u16	LLTReg = REG_LLT_INIT;


	rtw_write32(padapter, LLTReg, value);

	//polling and get value
	do {
		value = rtw_read32(padapter, LLTReg);
		if (_LLT_NO_ACTIVE == _LLT_OP_VALUE(value)) {
			return (u8)value;
		}

		if (count > POLLING_LLT_THRESHOLD) {
			RT_TRACE(_module_hal_init_c_, _drv_err_, ("Failed to polling read LLT done at address %d!\n", address));
			break;
		}
	} while (count++);

	return 0xFF;
}

s32 InitLLTTable8812(PADAPTER padapter, u8 txpktbuf_bndy)
{
	s32	status = _FAIL;
	u32	i;
	u32	Last_Entry_Of_TxPktBuf = LAST_ENTRY_OF_TX_PKT_BUFFER_8812;
	//HAL_DATA_TYPE *pHalData	= GET_HAL_DATA(padapter);

#if defined(CONFIG_IOL_LLT)
	if(1 || rtw_IOL_applied(padapter))
	{
		iol_mode_enable(padapter, 1);
		status = iol_InitLLTTable(padapter, txpktbuf_bndy);	
		iol_mode_enable(padapter, 0);
	}
	else
#endif
	{
		for (i = 0; i < (txpktbuf_bndy - 1); i++) {
			status = _LLTWrite(padapter, i, i + 1);
			if (_SUCCESS != status) {
				return status;
			}
		}

		// end of list
		status = _LLTWrite(padapter, (txpktbuf_bndy - 1), 0xFF);
		if (_SUCCESS != status) {
			return status;
		}

		// Make the other pages as ring buffer
		// This ring buffer is used as beacon buffer if we config this MAC as two MAC transfer.
		// Otherwise used as local loopback buffer.
		for (i = txpktbuf_bndy; i < Last_Entry_Of_TxPktBuf; i++) {
			status = _LLTWrite(padapter, i, (i + 1));
			if (_SUCCESS != status) {
				return status;
			}
		}

		// Let last entry point to the start entry of ring buffer
		status = _LLTWrite(padapter, Last_Entry_Of_TxPktBuf, txpktbuf_bndy);
		if (_SUCCESS != status) {
			return status;
		}
	}

	return status;
}

BOOLEAN HalDetectPwrDownMode8812(PADAPTER Adapter)
{
	u8 tmpvalue = 0;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	struct pwrctrl_priv *pwrctrlpriv = &Adapter->pwrctrlpriv;

	EFUSE_ShadowRead(Adapter, 1, EEPROM_RF_OPT3_92C, (u32 *)&tmpvalue);

	// 2010/08/25 MH INF priority > PDN Efuse value.
	if(tmpvalue & BIT(4) && pwrctrlpriv->reg_pdnmode)
	{
		pHalData->pwrdown = _TRUE;
	}
	else
	{
		pHalData->pwrdown = _FALSE;
	}

	DBG_8192C("HalDetectPwrDownMode(): PDN=%d\n", pHalData->pwrdown);

	return pHalData->pwrdown;
}	// HalDetectPwrDownMode

#ifdef CONFIG_WOWLAN
void Hal_DetectWoWMode(PADAPTER pAdapter)
{
	pAdapter->pwrctrlpriv.bSupportRemoteWakeup = _TRUE;
	DBG_871X("%s\n", __func__);
}
#endif

//====================================================================================
//
// 20100209 Joseph:
// This function is used only for 92C to set REG_BCN_CTRL(0x550) register.
// We just reserve the value of the register in variable pHalData->RegBcnCtrlVal and then operate
// the value of the register via atomic operation.
// This prevents from race condition when setting this register.
// The value of pHalData->RegBcnCtrlVal is initialized in HwConfigureRTL8192CE() function.
//
void SetBcnCtrlReg(
	PADAPTER	padapter,
	u8		SetBits,
	u8		ClearBits)
{
	PHAL_DATA_TYPE pHalData;


	pHalData = GET_HAL_DATA(padapter);

	pHalData->RegBcnCtrlVal |= SetBits;
	pHalData->RegBcnCtrlVal &= ~ClearBits;

#if 0
//#ifdef CONFIG_SDIO_HCI
	if (pHalData->sdio_himr & (SDIO_HIMR_TXBCNOK_MSK | SDIO_HIMR_TXBCNERR_MSK))
		pHalData->RegBcnCtrlVal |= EN_TXBCN_RPT;
#endif

	rtw_write8(padapter, REG_BCN_CTRL, (u8)pHalData->RegBcnCtrlVal);
}

static VOID
_FWDownloadEnable_8812(
	IN	PADAPTER		padapter,
	IN	BOOLEAN			enable
	)
{
	u8	tmp;

	if(enable)
	{
		// MCU firmware download enable.
		tmp = rtw_read8(padapter, REG_MCUFWDL);
		rtw_write8(padapter, REG_MCUFWDL, tmp|0x01);

		// 8051 reset
		tmp = rtw_read8(padapter, REG_MCUFWDL+2);
		rtw_write8(padapter, REG_MCUFWDL+2, tmp&0xf7);
	}
	else
	{		
		
		// MCU firmware download disable.
		tmp = rtw_read8(padapter, REG_MCUFWDL);
		rtw_write8(padapter, REG_MCUFWDL, tmp&0xfe);
	}
}
#define MAX_REG_BOLCK_SIZE	196 
static int
_BlockWrite_8812(
	IN		PADAPTER		padapter,
	IN		PVOID		buffer,
	IN		u32			buffSize
	)
{
	int ret = _SUCCESS;

	u32			blockSize_p1 = 4;	// (Default) Phase #1 : PCI muse use 4-byte write to download FW
	u32			blockSize_p2 = 8;	// Phase #2 : Use 8-byte, if Phase#1 use big size to write FW.
	u32			blockSize_p3 = 1;	// Phase #3 : Use 1-byte, the remnant of FW image.
	u32			blockCount_p1 = 0, blockCount_p2 = 0, blockCount_p3 = 0;
	u32			remainSize_p1 = 0, remainSize_p2 = 0;
	u8			*bufferPtr	= (u8*)buffer;
	u32			i=0, offset=0;
#ifdef CONFIG_PCI_HCI
	u8			remainFW[4] = {0, 0, 0, 0};
	u8			*p = NULL;
#endif

#ifdef CONFIG_USB_HCI
	blockSize_p1 = MAX_REG_BOLCK_SIZE;
#endif

	//3 Phase #1
	blockCount_p1 = buffSize / blockSize_p1;
	remainSize_p1 = buffSize % blockSize_p1;

	if (blockCount_p1) {
		RT_TRACE(_module_hal_init_c_, _drv_notice_,
				("_BlockWrite: [P1] buffSize(%d) blockSize_p1(%d) blockCount_p1(%d) remainSize_p1(%d)\n",
				buffSize, blockSize_p1, blockCount_p1, remainSize_p1));
	}

	for (i = 0; i < blockCount_p1; i++)
	{
#ifdef CONFIG_USB_HCI
		ret = rtw_writeN(padapter, (FW_START_ADDRESS + i * blockSize_p1), blockSize_p1, (bufferPtr + i * blockSize_p1));
#else
		ret = rtw_write32(padapter, (FW_START_ADDRESS + i * blockSize_p1), le32_to_cpu(*((u32*)(bufferPtr + i * blockSize_p1))));
#endif

		if(ret == _FAIL)
			goto exit;
	}

#ifdef CONFIG_PCI_HCI
	p = (u8*)((u32*)(bufferPtr + blockCount_p1 * blockSize_p1));
	if (remainSize_p1) {
		switch (remainSize_p1) {
		case 0:
			break;
		case 3:
			remainFW[2]=*(p+2);
		case 2: 	
			remainFW[1]=*(p+1);
		case 1: 	
			remainFW[0]=*(p);
			ret = rtw_write32(padapter, (FW_START_ADDRESS + blockCount_p1 * blockSize_p1), 
				 le32_to_cpu(*(u32*)remainFW));	
		}
		return ret;
	}
#endif

	//3 Phase #2
	if (remainSize_p1)
	{
		offset = blockCount_p1 * blockSize_p1;

		blockCount_p2 = remainSize_p1/blockSize_p2;
		remainSize_p2 = remainSize_p1%blockSize_p2;

		if (blockCount_p2) {
				RT_TRACE(_module_hal_init_c_, _drv_notice_,
						("_BlockWrite: [P2] buffSize_p2(%d) blockSize_p2(%d) blockCount_p2(%d) remainSize_p2(%d)\n",
						(buffSize-offset), blockSize_p2 ,blockCount_p2, remainSize_p2));
		}

#ifdef CONFIG_USB_HCI
		for (i = 0; i < blockCount_p2; i++) {
			ret = rtw_writeN(padapter, (FW_START_ADDRESS + offset + i*blockSize_p2), blockSize_p2, (bufferPtr + offset + i*blockSize_p2));
			
			if(ret == _FAIL)
				goto exit;
		}
#endif
	}

	//3 Phase #3
	if (remainSize_p2)
	{
		offset = (blockCount_p1 * blockSize_p1) + (blockCount_p2 * blockSize_p2);

		blockCount_p3 = remainSize_p2 / blockSize_p3;

		RT_TRACE(_module_hal_init_c_, _drv_notice_,
				("_BlockWrite: [P3] buffSize_p3(%d) blockSize_p3(%d) blockCount_p3(%d)\n",
				(buffSize-offset), blockSize_p3, blockCount_p3));

		for(i = 0 ; i < blockCount_p3 ; i++){
			ret =rtw_write8(padapter, (FW_START_ADDRESS + offset + i), *(bufferPtr + offset + i));
			
			if(ret == _FAIL)
				goto exit;
		}
	}

exit:
	return ret;
}

static int
_PageWrite_8812(
	IN		PADAPTER	padapter,
	IN		u32			page,
	IN		PVOID		buffer,
	IN		u32			size
	)
{
	u8 value8;
	u8 u8Page = (u8) (page & 0x07) ;

	value8 = (rtw_read8(padapter, REG_MCUFWDL+2) & 0xF8) | u8Page ;
	rtw_write8(padapter, REG_MCUFWDL+2,value8);

	return _BlockWrite_8812(padapter,buffer,size);
}

static inline VOID
_FillDummy_8812(
	u8*		pFwBuf,
	u32*	pFwLen
	)
{
	u32	FwLen = *pFwLen;
	u8	remain = (u8)(FwLen%4);
	remain = (remain==0)?0:(4-remain);

	while(remain>0)
	{
		pFwBuf[FwLen] = 0;
		FwLen++;
		remain--;
	}

	*pFwLen = FwLen;
}

static int
_WriteFW_8812(
	IN		PADAPTER		padapter,
	IN		PVOID			buffer,
	IN		u32			size
	)
{
	// Since we need dynamic decide method of dwonload fw, so we call this function to get chip version.
	// We can remove _ReadChipVersion from ReadpadapterInfo8192C later.
	int	ret = _SUCCESS;
	u32	pageNums,remainSize ;
	u32	page, offset;
	u8	*bufferPtr = (u8*)buffer;

#ifdef CONFIG_PCI_HCI
	// 20100120 Joseph: Add for 88CE normal chip.
	// Fill in zero to make firmware image to dword alignment.
//		_FillDummy(bufferPtr, &size);
#endif

	pageNums = size / MAX_DLFW_PAGE_SIZE ;
	//RT_ASSERT((pageNums <= 4), ("Page numbers should not greater then 4 \n"));
	remainSize = size % MAX_DLFW_PAGE_SIZE;

	for (page = 0; page < pageNums; page++) {
		offset = page * MAX_DLFW_PAGE_SIZE;
		ret = _PageWrite_8812(padapter, page, bufferPtr+offset, MAX_DLFW_PAGE_SIZE);
		
		if(ret == _FAIL)
			goto exit;
	}
	if (remainSize) {
		offset = pageNums * MAX_DLFW_PAGE_SIZE;
		page = pageNums;
		ret = _PageWrite_8812(padapter, page, bufferPtr+offset, remainSize);
		
		if(ret == _FAIL)
			goto exit;

	}
	RT_TRACE(_module_hal_init_c_, _drv_info_, ("_WriteFW Done- for Normal chip.\n"));

exit:
	return ret;
}

void _8051Reset8812(PADAPTER padapter)
{
	u8 u1bTmp, u1bTmp2;

	// Reset MCU IO Wrapper- sugggest by SD1-Gimmy
	if(IS_HARDWARE_TYPE_8812(padapter))
	{
		u1bTmp2 = rtw_read8(padapter, REG_RSV_CTRL+1);
		rtw_write8(padapter, REG_RSV_CTRL+1, u1bTmp2&(~BIT3));	
	}
	else if(IS_HARDWARE_TYPE_8821(padapter))
	{
		u1bTmp2 = rtw_read8(padapter, REG_RSV_CTRL+1);
		rtw_write8(padapter, REG_RSV_CTRL+1, u1bTmp2&(~BIT0));
	}

	u1bTmp = rtw_read8(padapter, REG_SYS_FUNC_EN+1);
	rtw_write8(padapter, REG_SYS_FUNC_EN+1, u1bTmp&(~BIT2));

	// Enable MCU IO Wrapper
	if(IS_HARDWARE_TYPE_8812(padapter))
	{
		u1bTmp2 = rtw_read8(padapter, REG_RSV_CTRL+1);
		rtw_write8(padapter, REG_RSV_CTRL+1, u1bTmp2 |(BIT3));	
	}
	else if(IS_HARDWARE_TYPE_8821(padapter))
	{
		u1bTmp2 = rtw_read8(padapter, REG_RSV_CTRL+1);
		rtw_write8(padapter, REG_RSV_CTRL+1, u1bTmp2|(BIT0));
	}

	rtw_write8(padapter, REG_SYS_FUNC_EN+1, u1bTmp|(BIT2));

	DBG_871X("=====> _8051Reset8812(): 8051 reset success .\n");
}

static s32 _FWFreeToGo8812(PADAPTER padapter)
{
	u32	counter = 0;
	u32	value32;
	//u8 	value8;

	// polling CheckSum report
	do {
		value32 = rtw_read32(padapter, REG_MCUFWDL);
		if (value32 & FWDL_ChkSum_rpt) break;
	} while (counter++ < 6000);

	if (counter >= 6000) {
		DBG_871X("%s: chksum report fail! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32);
		return _FAIL;
	}
	DBG_871X("%s: Checksum report OK! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32);

	value32 = rtw_read32(padapter, REG_MCUFWDL);
	value32 |= MCUFWDL_RDY;
	value32 &= ~WINTINI_RDY;
	rtw_write32(padapter, REG_MCUFWDL, value32);

	_8051Reset8812(padapter);

	// polling for FW ready
	counter = 0;
	do {
		value32 = rtw_read32(padapter, REG_MCUFWDL);
		if (value32 & WINTINI_RDY) {
			DBG_871X("%s: Polling FW ready success!! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32);
			return _SUCCESS;
		}
		rtw_udelay_os(5);
	} while (counter++ < 6000);

	DBG_871X ("%s: Polling FW ready fail!! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32);
	return _FAIL;
}

#ifdef CONFIG_FILE_FWIMG
extern char *rtw_fw_file_path;
u8	FwBuffer8812[FW_SIZE_8812];
#endif //CONFIG_FILE_FWIMG

s32
FirmwareDownload8812(
	IN	PADAPTER			Adapter,
	IN	BOOLEAN			bUsedWoWLANFw
)
{
	s32	rtStatus = _SUCCESS;
	u8	writeFW_retry = 0;
	u32 fwdl_start_time;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);	
	
	//u8				*pFwImageFileName;
	//u8				*pucMappedFile = NULL;
	PRT_FIRMWARE_8812	pFirmware = NULL;
	u8				*pFwHdr = NULL;
	u8				*pFirmwareBuf;
	u32				FirmwareLen;


	RT_TRACE(_module_hal_init_c_, _drv_info_, ("+%s\n", __FUNCTION__));
	pFirmware = (PRT_FIRMWARE_8812)rtw_zmalloc(sizeof(RT_FIRMWARE_8812));
	if(!pFirmware)
	{
		rtStatus = _FAIL;
		goto Exit;
	}

	#ifdef CONFIG_FILE_FWIMG
	if(rtw_is_file_readable(rtw_fw_file_path) == _TRUE)
	{
		DBG_871X("%s accquire FW from file:%s\n", __FUNCTION__, rtw_fw_file_path);
		pFirmware->eFWSource = FW_SOURCE_IMG_FILE;
	}
	else
	#endif //CONFIG_FILE_FWIMG
	{
		pFirmware->eFWSource = FW_SOURCE_HEADER_FILE;
	}

	DBG_871X(" ===> FirmwareDownload8812() fw source from %s.\n", (pFirmware->eFWSource ? "Header": "File"));

	switch(pFirmware->eFWSource)
	{
		case FW_SOURCE_IMG_FILE:
			#ifdef CONFIG_FILE_FWIMG
			rtStatus = rtw_retrive_from_file(rtw_fw_file_path, FwBuffer8812, FW_SIZE_8812);
			pFirmware->ulFwLength = rtStatus>=0?rtStatus:0;
			pFirmware->szFwBuffer = FwBuffer8812;
			#endif //CONFIG_FILE_FWIMG
			break;
		case FW_SOURCE_HEADER_FILE:
			ODM_ConfigFWWithHeaderFile(&pHalData->odmpriv, CONFIG_FW_NIC, (u8 *)&(pFirmware->szFwBuffer), &(pFirmware->ulFwLength));
			DBG_871X(" ===> FirmwareDownload8812() fw:%s, size: %d\n", "Firmware for NIC", pFirmware->ulFwLength);

#ifdef CONFIG_WOWLAN
			ODM_ConfigFWWithHeaderFile(&pHalData->odmpriv, CONFIG_FW_WoWLAN, (u8 *)&(pFirmware->szWoWLANFwBuffer), &(pFirmware->ulWoWLANFwLength));
			DBG_871X(" ===> FirmwareDownload88E() fw:%s, size: %d\n", "Firmware for WoWLAN", pFirmware->ulWoWLANFwLength);
#endif //CONFIG_WOWLAN

			if (pFirmware->ulFwLength > FW_SIZE_8812) {
				rtStatus = _FAIL;
				RT_TRACE(_module_hal_init_c_, _drv_err_, ("Firmware size exceed 0x%X. Check it.\n", FW_SIZE_8812) );
				goto Exit;
			}

			break;
	}

#ifdef CONFIG_WOWLAN
	if (bUsedWoWLANFw) {
		pFirmwareBuf = pFirmware->szWoWLANFwBuffer;
		FirmwareLen = pFirmware->ulWoWLANFwLength;
		pFwHdr = (u8 *)pFirmware->szWoWLANFwBuffer;
	} else
#endif
	{
		pFirmwareBuf = pFirmware->szFwBuffer;
		FirmwareLen = pFirmware->ulFwLength;
		DBG_871X_LEVEL(_drv_info_, "+%s: !bUsedWoWLANFw, FmrmwareLen:%d+\n", __func__, FirmwareLen);
		// To Check Fw header. Added by tynli. 2009.12.04.
		pFwHdr = (u8 *)pFirmware->szFwBuffer;
	}

	pHalData->FirmwareVersion =  (u16)GET_FIRMWARE_HDR_VERSION_8812(pFwHdr);
	pHalData->FirmwareSubVersion = (u16)GET_FIRMWARE_HDR_SUB_VER_8812(pFwHdr);
	pHalData->FirmwareSignature = (u16)GET_FIRMWARE_HDR_SIGNATURE_8812(pFwHdr);

	DBG_871X ("%s: fw_ver=%d fw_subver=%d sig=0x%x\n",
		  __FUNCTION__, pHalData->FirmwareVersion, pHalData->FirmwareSubVersion, pHalData->FirmwareSignature);

	if (IS_FW_HEADER_EXIST_8812(pFwHdr) || IS_FW_HEADER_EXIST_8821(pFwHdr))
	{
		// Shift 32 bytes for FW header
		pFirmwareBuf = pFirmwareBuf + 32;
		FirmwareLen = FirmwareLen - 32;
	}

	// Suggested by Filen. If 8051 is running in RAM code, driver should inform Fw to reset by itself,
	// or it will cause download Fw fail. 2010.02.01. by tynli.
	if (rtw_read8(Adapter, REG_MCUFWDL) & BIT7) //8051 RAM code
	{
		rtw_write8(Adapter, REG_MCUFWDL, 0x00);
		_8051Reset8812(Adapter);		
	}

	_FWDownloadEnable_8812(Adapter, _TRUE);
	fwdl_start_time = rtw_get_current_time();
	while(1) {
		//reset the FWDL chksum
		rtw_write8(Adapter, REG_MCUFWDL, rtw_read8(Adapter, REG_MCUFWDL)|FWDL_ChkSum_rpt);
		
		rtStatus = _WriteFW_8812(Adapter, pFirmwareBuf, FirmwareLen);

		if(rtStatus == _SUCCESS
			||(rtw_get_passing_time_ms(fwdl_start_time) > 500 && writeFW_retry++ >= 3)
		)
			break;

		DBG_871X("%s writeFW_retry:%u, time after fwdl_start_time:%ums\n", __FUNCTION__
			, writeFW_retry
			, rtw_get_passing_time_ms(fwdl_start_time)
		);
	}
	_FWDownloadEnable_8812(Adapter, _FALSE);
	if(_SUCCESS != rtStatus){
		DBG_871X("DL Firmware failed!\n");
		goto Exit;
	}

	rtStatus = _FWFreeToGo8812(Adapter);
	if (_SUCCESS != rtStatus) {
		DBG_871X("DL Firmware failed!\n");
		goto Exit;
	}
	RT_TRACE(_module_hal_init_c_, _drv_info_, ("Firmware is ready to run!\n"));

Exit:

	if (pFirmware)
		rtw_mfree((u8*)pFirmware, sizeof(RT_FIRMWARE_8812));

	//RT_TRACE(COMP_INIT, DBG_LOUD, (" <=== FirmwareDownload91C()\n"));
#ifdef CONFIG_WOWLAN
	if (Adapter->pwrctrlpriv.wowlan_mode)
		InitializeFirmwareVars8812(Adapter);	
	else
		DBG_871X_LEVEL(_drv_always_, "%s: wowland_mode:%d wowlan_wake_reason:%d\n", 
			__func__, Adapter->pwrctrlpriv.wowlan_mode, 
			Adapter->pwrctrlpriv.wowlan_wake_reason);
#endif

	return rtStatus;
}


void InitializeFirmwareVars8812(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	struct pwrctrl_priv *pwrpriv;
	pwrpriv = &padapter->pwrctrlpriv;

	// Init Fw LPS related.
	padapter->pwrctrlpriv.bFwCurrentInPSMode = _FALSE;
	// Init H2C counter. by tynli. 2009.12.09.
	pHalData->LastHMEBoxNum = 0;
}

#ifdef CONFIG_WOWLAN
//===========================================
//
// Description: Prepare some information to Fw for WoWLAN.
//					(1) Download wowlan Fw.
//					(2) Download RSVD page packets.
//					(3) Enable AP offload if needed.
//
// 2011.04.12 by tynli.
//
VOID
SetFwRelatedForWoWLAN8812(
		IN		PADAPTER			padapter,
		IN		u8					bHostIsGoingtoSleep
)
{
		int				status=_FAIL;
		HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
		u8				bRecover = _FALSE;
	//
	// 1. Before WoWLAN we need to re-download WoWLAN Fw.
	//
	status = FirmwareDownload8812(padapter, bHostIsGoingtoSleep);
	if(status != _SUCCESS) {
		DBG_871X("SetFwRelatedForWoWLAN8812(): Re-Download Firmware failed!!\n");
		return;
	} else {
		DBG_871X("SetFwRelatedForWoWLAN8812(): Re-Download Firmware Success !!\n");
	}
	//
	// 2. Re-Init the variables about Fw related setting.
	//
	InitializeFirmwareVars8812(padapter);
}
#endif //CONFIG_WOWLAN

static void rtl8812_free_hal_data(PADAPTER padapter)
{
_func_enter_;
	if (padapter->HalData) {
#ifdef CONFIG_CONCURRENT_MODE
		if(padapter->isprimary)
#endif //CONFIG_CONCURRENT_MODE
			rtw_mfree(padapter->HalData, sizeof(HAL_DATA_TYPE));
		padapter->HalData = NULL;
	}
_func_exit_;
}

//===========================================================
//				Efuse related code
//===========================================================
BOOLEAN 
Hal_GetChnlGroup8812A(
	IN	u8	Channel,
	OUT u8*	pGroup
	)
{
	BOOLEAN bIn24G=_TRUE;

	if(Channel <= 14)
	{
		bIn24G=_TRUE;

		if      (1  <= Channel && Channel <= 2 )   *pGroup = 0;
		else if (3  <= Channel && Channel <= 5 )   *pGroup = 1;
		else if (6  <= Channel && Channel <= 8 )   *pGroup = 2;
		else if (9  <= Channel && Channel <= 11)   *pGroup = 3;
		else if (12 <= Channel && Channel <= 14)   *pGroup = 4;
		else
		{
			DBG_871X("==>mpt_GetChnlGroup8812A in 2.4 G, but Channel %d in Group not found \n", Channel);
		}
	}
	else
	{
		bIn24G=_FALSE;

		if      (36   <= Channel && Channel <=  42)   *pGroup = 0;
		else if (44   <= Channel && Channel <=  48)   *pGroup = 1;
		else if (50   <= Channel && Channel <=  58)   *pGroup = 2;
		else if (60   <= Channel && Channel <=  64)   *pGroup = 3;
		else if (100  <= Channel && Channel <= 106)   *pGroup = 4;
		else if (108  <= Channel && Channel <= 114)   *pGroup = 5;
		else if (116  <= Channel && Channel <= 122)   *pGroup = 6;
		else if (124  <= Channel && Channel <= 130)   *pGroup = 7;
		else if (132  <= Channel && Channel <= 138)   *pGroup = 8;
		else if (140  <= Channel && Channel <= 144)   *pGroup = 9;
		else if (149  <= Channel && Channel <= 155)   *pGroup = 10;
		else if (157  <= Channel && Channel <= 161)   *pGroup = 11;
		else if (165  <= Channel && Channel <= 171)   *pGroup = 12;
		else if (173  <= Channel && Channel <= 177)   *pGroup = 13;
		else
		{
			DBG_871X("==>mpt_GetChnlGroup8812A in 5G, but Channel %d in Group not found \n",Channel);
		}

	}
	//DBG_871X("<==mpt_GetChnlGroup8812A,  (%s) Channel = %d, Group =%d,\n", (bIn24G) ? "2.4G" : "5G", Channel, *pGroup);

	return bIn24G;
}

static void
hal_ReadPowerValueFromPROM8812A(
	IN	PADAPTER 		Adapter,
	IN	PTxPowerInfo24G	pwrInfo24G,
	IN	PTxPowerInfo5G	pwrInfo5G,
	IN	u8*				PROMContent,
	IN	BOOLEAN			AutoLoadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u32 rfPath, eeAddr=EEPROM_TX_PWR_INX_8812, group,TxCount=0;
	
	_rtw_memset(pwrInfo24G, 0, sizeof(TxPowerInfo24G));
	_rtw_memset(pwrInfo5G, 0, sizeof(TxPowerInfo5G));

	//DBG_871X("hal_ReadPowerValueFromPROM8812A(): PROMContent[0x%x]=0x%x\n", (eeAddr+1), PROMContent[eeAddr+1]);
	if(0xFF == PROMContent[eeAddr+1])  //YJ,add,120316
		AutoLoadFail = _TRUE;

	if(AutoLoadFail)
	{
		DBG_871X("hal_ReadPowerValueFromPROM8812A(): Use Default value!\n");
		for(rfPath = 0 ; rfPath < MAX_RF_PATH ; rfPath++)
		{
			// 2.4G default value
			for(group = 0 ; group < MAX_CHNL_GROUP_24G; group++)
			{
				pwrInfo24G->IndexCCK_Base[rfPath][group] =	EEPROM_DEFAULT_24G_INDEX;
				pwrInfo24G->IndexBW40_Base[rfPath][group] =	EEPROM_DEFAULT_24G_INDEX;
			}
			for(TxCount=0;TxCount<MAX_TX_COUNT;TxCount++)
			{
				if(TxCount==0)
				{
					pwrInfo24G->BW20_Diff[rfPath][0] =	EEPROM_DEFAULT_24G_HT20_DIFF;
					pwrInfo24G->OFDM_Diff[rfPath][0] =	EEPROM_DEFAULT_24G_OFDM_DIFF;
				}
				else
				{
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo24G->BW40_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo24G->CCK_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
				}
			}	

			// 5G default value
			for(group = 0 ; group < MAX_CHNL_GROUP_5G; group++)
			{
				pwrInfo5G->IndexBW40_Base[rfPath][group] =		EEPROM_DEFAULT_5G_INDEX;
			}
			
			for(TxCount=0;TxCount<MAX_TX_COUNT;TxCount++)
			{
				if(TxCount==0)
				{
					pwrInfo5G->OFDM_Diff[rfPath][0] =	EEPROM_DEFAULT_5G_OFDM_DIFF;
					pwrInfo5G->BW20_Diff[rfPath][0] =	EEPROM_DEFAULT_5G_HT20_DIFF;
					pwrInfo5G->BW80_Diff[rfPath][0] =	EEPROM_DEFAULT_DIFF;
					pwrInfo5G->BW160_Diff[rfPath][0] =	EEPROM_DEFAULT_DIFF;
				}
				else
				{
					pwrInfo5G->OFDM_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo5G->BW20_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo5G->BW40_Diff[rfPath][TxCount]=	EEPROM_DEFAULT_DIFF;
					pwrInfo5G->BW80_Diff[rfPath][TxCount]=	EEPROM_DEFAULT_DIFF;
					pwrInfo5G->BW160_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;

				}
			}	
			
		}
		
		//pHalData->bNOPG = _TRUE;				
		return;
	}

	pHalData->bTXPowerDataReadFromEEPORM = _TRUE;		//YJ,move,120316

	for(rfPath = 0 ; rfPath < MAX_RF_PATH ; rfPath++)
	{
		// 2.4G default value
		for(group = 0 ; group < MAX_CHNL_GROUP_24G; group++)
		{
			pwrInfo24G->IndexCCK_Base[rfPath][group] =	PROMContent[eeAddr++];
			if(pwrInfo24G->IndexCCK_Base[rfPath][group] == 0xFF)
			{
				pwrInfo24G->IndexCCK_Base[rfPath][group] = EEPROM_DEFAULT_24G_INDEX;
				//pHalData->bNOPG = _TRUE;
			}
			//DBG_871X("8812-2G RF-%d-G-%d CCK-Addr-%x BASE=%x\n", 
			//rfPath, group, eeAddr-1, pwrInfo24G->IndexCCK_Base[rfPath][group]);
		}
		for(group = 0 ; group < MAX_CHNL_GROUP_24G-1; group++)
		{
			pwrInfo24G->IndexBW40_Base[rfPath][group] =	PROMContent[eeAddr++];
			if(pwrInfo24G->IndexBW40_Base[rfPath][group] == 0xFF)
				pwrInfo24G->IndexBW40_Base[rfPath][group] =	EEPROM_DEFAULT_24G_INDEX;
			//DBG_871X("8812-2G RF-%d-G-%d BW40-Addr-%x BASE=%x\n", 
			//rfPath, group, eeAddr-1, pwrInfo24G->IndexBW40_Base[rfPath][group]);
		}
		for(TxCount=0;TxCount<MAX_TX_COUNT;TxCount++)
		{
			if(TxCount==0)
			{
				pwrInfo24G->BW40_Diff[rfPath][TxCount] = 0;

				{
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0xf0)>>4;				
				 	if(pwrInfo24G->BW20_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->BW20_Diff[rfPath][TxCount] |= 0xF0;
				}
				//DBG_871X("8812-2G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n", 
				//rfPath, TxCount, eeAddr, pwrInfo24G->BW20_Diff[rfPath][TxCount]);

				{
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);					
					if(pwrInfo24G->OFDM_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->OFDM_Diff[rfPath][TxCount] |= 0xF0;
				}
				//DBG_871X("8812-2G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n", 
				//rfPath, TxCount, eeAddr, pwrInfo24G->OFDM_Diff[rfPath][TxCount]);

				pwrInfo24G->CCK_Diff[rfPath][TxCount] = 0;
				eeAddr++;
			}
			else
			{				

				{
					pwrInfo24G->BW40_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0xf0)>>4;				
					if(pwrInfo24G->BW40_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->BW40_Diff[rfPath][TxCount] |= 0xF0;
				}
				//DBG_871X("8812-2G RF-%d-SS-%d BW40-Addr-%x DIFF=%d\n", 
				//rfPath, TxCount, eeAddr, pwrInfo24G->BW40_Diff[rfPath][TxCount]);


				{
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);				
					if(pwrInfo24G->BW20_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->BW20_Diff[rfPath][TxCount] |= 0xF0;
				}
				//DBG_871X("8812-2G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n", 
				//rfPath, TxCount, eeAddr, pwrInfo24G->BW20_Diff[rfPath][TxCount]);

				eeAddr++;
				

				{
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0xf0)>>4;				
					if(pwrInfo24G->OFDM_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->OFDM_Diff[rfPath][TxCount] |= 0xF0;
				}
				//DBG_871X("8812-2G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n", 
				//rfPath, TxCount, eeAddr, pwrInfo24G->BW20_Diff[rfPath][TxCount]);


				{
					pwrInfo24G->CCK_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);				
					if(pwrInfo24G->CCK_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo24G->CCK_Diff[rfPath][TxCount] |= 0xF0;
				}
				//DBG_871X("8812-2G RF-%d-SS-%d CCK-Addr-%x DIFF=%d\n", 
				//rfPath, TxCount, eeAddr, pwrInfo24G->CCK_Diff[rfPath][TxCount]);

				eeAddr++;
			}
		}

		//5G default value
		for(group = 0 ; group < MAX_CHNL_GROUP_5G; group++)
		{
			pwrInfo5G->IndexBW40_Base[rfPath][group] =		PROMContent[eeAddr++];
			if(pwrInfo5G->IndexBW40_Base[rfPath][group] == 0xFF)
				pwrInfo5G->IndexBW40_Base[rfPath][group] = EEPROM_DEFAULT_DIFF;												

			//DBG_871X("8812-5G RF-%d-G-%d BW40-Addr-%x BASE=%x\n", 
			//	rfPath, TxCount, eeAddr-1, pwrInfo5G->IndexBW40_Base[rfPath][group]);
		}
		
		for(TxCount=0;TxCount<MAX_TX_COUNT;TxCount++)
		{
			if(TxCount==0)
			{
				pwrInfo5G->BW40_Diff[rfPath][TxCount]=	 0;				
			

				{
					pwrInfo5G->BW20_Diff[rfPath][0] =	(PROMContent[eeAddr]&0xf0)>>4;				
					if(pwrInfo5G->BW20_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo5G->BW20_Diff[rfPath][TxCount] |= 0xF0;		
				}
				//DBG_871X("8812-5G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n", 
				//rfPath, TxCount, eeAddr, pwrInfo5G->BW20_Diff[rfPath][TxCount]);


				{
					pwrInfo5G->OFDM_Diff[rfPath][0] =	(PROMContent[eeAddr]&0x0f);				
					if(pwrInfo5G->OFDM_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
						pwrInfo5G->OFDM_Diff[rfPath][TxCount] |= 0xF0;								
				}
				//DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n", 
				//rfPath, TxCount, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][TxCount]);

				eeAddr++;
			}
			else
			{

				{
					pwrInfo5G->BW40_Diff[rfPath][TxCount]=	 (PROMContent[eeAddr]&0xf0)>>4;					
					if(pwrInfo5G->BW40_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
					pwrInfo5G->BW40_Diff[rfPath][TxCount] |= 0xF0;	
				}
				//DBG_871X("8812-5G RF-%d-SS-%d BW40-Addr-%x DIFF=%d\n", 
				//rfPath, TxCount, eeAddr, pwrInfo5G->BW40_Diff[rfPath][TxCount]);
				

				{
					pwrInfo5G->BW20_Diff[rfPath][TxCount] = (PROMContent[eeAddr]&0x0f);				
					if(pwrInfo5G->BW20_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
					pwrInfo5G->BW20_Diff[rfPath][TxCount] |= 0xF0;					
				}
				//DBG_871X("8812-5G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n", 
				//rfPath, TxCount, eeAddr, pwrInfo5G->BW20_Diff[rfPath][TxCount]);
				
				eeAddr++;
			}
		}	


		{
			pwrInfo5G->OFDM_Diff[rfPath][1] =	(PROMContent[eeAddr]&0xf0)>>4;
			pwrInfo5G->OFDM_Diff[rfPath][2] =	(PROMContent[eeAddr]&0x0f);
		}
		//DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n", 
		//		rfPath, 2, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][2]);
		eeAddr++;		


			pwrInfo5G->OFDM_Diff[rfPath][3] =	(PROMContent[eeAddr]&0x0f);

		//DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n", 
		//rfPath, 3, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][3]);
		eeAddr++;
		
		for(TxCount=1;TxCount<MAX_TX_COUNT;TxCount++)
		{
			if(pwrInfo5G->OFDM_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
				pwrInfo5G->OFDM_Diff[rfPath][TxCount] |= 0xF0;			

			//DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n", 
			//rfPath, TxCount, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][TxCount]);
		}	
		
		for(TxCount=0;TxCount<MAX_TX_COUNT;TxCount++)
		{		

			{
				pwrInfo5G->BW80_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0xf0)>>4;			
				if(pwrInfo5G->BW80_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
					pwrInfo5G->BW80_Diff[rfPath][TxCount] |= 0xF0;			
			}
			//DBG_871X("8812-5G RF-%d-SS-%d BW80-Addr-%x DIFF=%d\n", 
			//rfPath, TxCount, eeAddr, pwrInfo5G->BW80_Diff[rfPath][TxCount]);
			

			{
				pwrInfo5G->BW160_Diff[rfPath][TxCount]=	(PROMContent[eeAddr]&0x0f);			
				if(pwrInfo5G->BW160_Diff[rfPath][TxCount] & BIT3)		//4bit sign number to 8 bit sign number
					pwrInfo5G->BW160_Diff[rfPath][TxCount] |= 0xF0;						
			}
			//DBG_871X("8812-5G RF-%d-SS-%d BW160-Addr-%x DIFF=%d\n", 
			//rfPath, TxCount, eeAddr, pwrInfo5G->BW160_Diff[rfPath][TxCount]);
			eeAddr++;
		}
	}

}

VOID
Hal_EfuseParseBTCoexistInfo8812A(
	IN PADAPTER			Adapter,
	IN pu1Byte			hwinfo,
	IN BOOLEAN			AutoLoadFail
	)
{
#ifdef CONFIG_BT_COEXIST
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	pHalData->EEPROMBluetoothCoexist = 0;
	pHalData->EEPROMBluetoothType = BT_CSR_BC8;
	pHalData->EEPROMBluetoothAntNum = Ant_x2;
	pHalData->EEPROMBluetoothAntIsolation = 1;
	pHalData->EEPROMBluetoothRadioShared = BT_Radio_Shared;
	BT_InitHalVars(Adapter);
#endif
}

void
Hal_EfuseParseIDCode8812A(
	IN	PADAPTER	padapter,
	IN	u8			*hwinfo
	)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);
	u16			EEPROMId;


	// Checl 0x8129 again for making sure autoload status!!
	EEPROMId = le16_to_cpu(*((u16*)hwinfo));
	if (EEPROMId != RTL_EEPROM_ID)
	{
		DBG_8192C("EEPROM ID(%#x) is invalid!!\n", EEPROMId);
		pEEPROM->bautoload_fail_flag = _TRUE;
	}
	else
	{
		pEEPROM->bautoload_fail_flag = _FALSE;
	}

	DBG_8192C("EEPROM ID=0x%04x\n", EEPROMId);
}

VOID
Hal_ReadPROMVersion8812A(
	IN	PADAPTER	Adapter,	
	IN	u8* 			PROMContent,
	IN	BOOLEAN 	AutoloadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	if(AutoloadFail){
		pHalData->EEPROMVersion = EEPROM_Default_Version;		
	}
	else{
		pHalData->EEPROMVersion = *(u8 *)&PROMContent[EEPROM_VERSION_8812];
		if(pHalData->EEPROMVersion == 0xFF)
			pHalData->EEPROMVersion = EEPROM_Default_Version;					
	}
	//DBG_871X("pHalData->EEPROMVersion is 0x%x\n", pHalData->EEPROMVersion);
}

void
Hal_ReadTxPowerInfo8812A(
	IN	PADAPTER 		Adapter,
	IN	u8*				PROMContent,
	IN	BOOLEAN			AutoLoadFail
	)
{	
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	TxPowerInfo24G	pwrInfo24G;
	TxPowerInfo5G	pwrInfo5G;
	u8	rfPath, ch, group, TxCount;
	u8	channel5G[CHANNEL_MAX_NUMBER_5G] = 
			{36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,100,102,104,106,108,110,112,
			114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,149,151,
			153,155,157,159,161,163,165,167,168,169,171,173,175,177};
	u8	channel5G_80M[CHANNEL_MAX_NUMBER_5G_80M] = {42, 58, 106, 122, 138, 155, 171};

	hal_ReadPowerValueFromPROM8812A(Adapter, &pwrInfo24G,&pwrInfo5G, PROMContent, AutoLoadFail);

	//if(!AutoLoadFail)
	//	pHalData->bTXPowerDataReadFromEEPORM = _TRUE;		

	for(rfPath = 0 ; rfPath < MAX_RF_PATH ; rfPath++)
	{
		for(ch = 0 ; ch < CHANNEL_MAX_NUMBER_2G ; ch++)
		{
			Hal_GetChnlGroup8812A(ch+1, &group);

			if(ch == 14-1) 
			{
				pHalData->Index24G_CCK_Base[rfPath][ch] = pwrInfo24G.IndexCCK_Base[rfPath][5];
				pHalData->Index24G_BW40_Base[rfPath][ch] = pwrInfo24G.IndexBW40_Base[rfPath][group];
			}
			else
			{
				pHalData->Index24G_CCK_Base[rfPath][ch] = pwrInfo24G.IndexCCK_Base[rfPath][group];
				pHalData->Index24G_BW40_Base[rfPath][ch] = pwrInfo24G.IndexBW40_Base[rfPath][group];
			}
			//DBG_871X("======= Path %d, ChannelIndex %d, Group %d=======\n",rfPath,ch, group);	
			//DBG_871X("Index24G_CCK_Base[%d][%d] = 0x%x\n",rfPath,ch ,pHalData->Index24G_CCK_Base[rfPath][ch]);
			//DBG_871X("Index24G_BW40_Base[%d][%d] = 0x%x\n",rfPath,ch,pHalData->Index24G_BW40_Base[rfPath][ch]);
		}	

		for(ch = 0 ; ch < CHANNEL_MAX_NUMBER_5G; ch++) 
		{
			Hal_GetChnlGroup8812A(channel5G[ch], &group);
			
			pHalData->Index5G_BW40_Base[rfPath][ch] = pwrInfo5G.IndexBW40_Base[rfPath][group];
			//DBG_871X("======= Path %d, ChannelIndex %d, Group %d=======\n",rfPath,ch, group);			
			//DBG_871X("Index5G_BW40_Base[%d][%d] = 0x%x\n",rfPath,ch,pHalData->Index5G_BW40_Base[rfPath][ch]);
		}
		for(ch = 0 ; ch < CHANNEL_MAX_NUMBER_5G_80M; ch++) 
		{
			u8	upper, lower;
			
			Hal_GetChnlGroup8812A(channel5G_80M[ch], &group);
			upper = pwrInfo5G.IndexBW40_Base[rfPath][group];
			lower = pwrInfo5G.IndexBW40_Base[rfPath][group+1];
			
			pHalData->Index5G_BW80_Base[rfPath][ch] = (upper + lower) / 2;
			
			//DBG_871X("======= Path %d, ChannelIndex %d, Group %d=======\n",rfPath,ch, group);	
			//DBG_871X("Index5G_BW80_Base[%d][%d] = 0x%x\n",rfPath,ch,pHalData->Index5G_BW80_Base[rfPath][ch]);
		}

		for(TxCount=0;TxCount<MAX_TX_COUNT;TxCount++)
		{
			pHalData->CCK_24G_Diff[rfPath][TxCount]=pwrInfo24G.CCK_Diff[rfPath][TxCount];
			pHalData->OFDM_24G_Diff[rfPath][TxCount]=pwrInfo24G.OFDM_Diff[rfPath][TxCount];
			pHalData->BW20_24G_Diff[rfPath][TxCount]=pwrInfo24G.BW20_Diff[rfPath][TxCount];
			pHalData->BW40_24G_Diff[rfPath][TxCount]=pwrInfo24G.BW40_Diff[rfPath][TxCount];
			
			pHalData->OFDM_5G_Diff[rfPath][TxCount]=pwrInfo5G.OFDM_Diff[rfPath][TxCount];
			pHalData->BW20_5G_Diff[rfPath][TxCount]=pwrInfo5G.BW20_Diff[rfPath][TxCount];
			pHalData->BW40_5G_Diff[rfPath][TxCount]=pwrInfo5G.BW40_Diff[rfPath][TxCount];
			pHalData->BW80_5G_Diff[rfPath][TxCount]=pwrInfo5G.BW80_Diff[rfPath][TxCount];
//#if DBG
#if 0
			DBG_871X("--------------------------------------- 2.4G ---------------------------------------\n");
			DBG_871X("CCK_24G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->CCK_24G_Diff[rfPath][TxCount]);
			DBG_871X("OFDM_24G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->OFDM_24G_Diff[rfPath][TxCount]);
			DBG_871X("BW20_24G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->BW20_24G_Diff[rfPath][TxCount]);
			DBG_871X("BW40_24G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->BW40_24G_Diff[rfPath][TxCount]);
			DBG_871X("---------------------------------------- 5G ----------------------------------------\n");
			DBG_871X("OFDM_5G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->OFDM_5G_Diff[rfPath][TxCount]);
			DBG_871X("BW20_5G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->BW20_5G_Diff[rfPath][TxCount]);
			DBG_871X("BW40_5G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->BW40_5G_Diff[rfPath][TxCount]);
			DBG_871X("BW80_5G_Diff[%d][%d]= %d\n",rfPath,TxCount,pHalData->BW80_5G_Diff[rfPath][TxCount]);
#endif							
		}
	}

	
	// 2010/10/19 MH Add Regulator recognize for CU.
	if(!AutoLoadFail)
	{
		struct registry_priv  *registry_par = &Adapter->registrypriv;
		if( registry_par->regulatory_tid == 0xff){		
			
			if(PROMContent[EEPROM_RF_BOARD_OPTION_8812] == 0xFF)
				pHalData->EEPROMRegulatory = (EEPROM_DEFAULT_BOARD_OPTION&0x7);	//bit0~2
			else
				pHalData->EEPROMRegulatory = (PROMContent[EEPROM_RF_BOARD_OPTION_8812]&0x7);	//bit0~2	
		}
		else{
			pHalData->EEPROMRegulatory = registry_par->regulatory_tid;
		}

		// 2012/09/26 MH Add for TX power calibrate rate.
		pHalData->TxPwrCalibrateRate = PROMContent[EEPROM_TX_PWR_CALIBRATE_RATE_8812];
	}
	else
	{
		pHalData->EEPROMRegulatory = 0;
		// 2012/09/26 MH Add for TX power calibrate rate.
		pHalData->TxPwrCalibrateRate = EEPROM_DEFAULT_TX_CALIBRATE_RATE;
	}
	DBG_871X("EEPROMRegulatory = 0x%x TxPwrCalibrateRate=0x%x\n", pHalData->EEPROMRegulatory, pHalData->TxPwrCalibrateRate);

}

VOID
Hal_ReadBoardType8812A(
	IN	PADAPTER	Adapter,	
	IN	u8*			PROMContent,
	IN	BOOLEAN		AutoloadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	if(!AutoloadFail)
	{
		pHalData->InterfaceSel = (PROMContent[EEPROM_RF_BOARD_OPTION_8812]&0xE0)>>5;
		if(PROMContent[EEPROM_RF_BOARD_OPTION_8812] == 0xFF)
			pHalData->InterfaceSel = (EEPROM_DEFAULT_BOARD_OPTION&0xE0)>>5;
	}
	else
	{
		pHalData->InterfaceSel = 0;
	}
	DBG_871X("Board Type: 0x%2x\n", pHalData->InterfaceSel);

}

VOID
Hal_ReadThermalMeter_8812A(
	IN	PADAPTER	Adapter,	
	IN	u8*		 	PROMContent,
	IN	BOOLEAN 	AutoloadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	//u8	tempval;

	//
	// ThermalMeter from EEPROM
	//
	if(!AutoloadFail)	
		pHalData->EEPROMThermalMeter = PROMContent[EEPROM_THERMAL_METER_8812];
	else
		pHalData->EEPROMThermalMeter = EEPROM_Default_ThermalMeter_8812;
	//pHalData->EEPROMThermalMeter = (tempval&0x1f);	//[4:0]

	if(pHalData->EEPROMThermalMeter == 0xff || AutoloadFail)
	{
		pHalData->bAPKThermalMeterIgnore = _TRUE;
		pHalData->EEPROMThermalMeter = 0xFF;		
	}

	//pHalData->ThermalMeter[0] = pHalData->EEPROMThermalMeter;	
	DBG_871X("ThermalMeter = 0x%x\n", pHalData->EEPROMThermalMeter);
}

VOID
Hal_ReadChannelPlan8812A(
	IN	PADAPTER		padapter,
	IN	u8*				hwinfo,
	IN	BOOLEAN			AutoLoadFail
	)
{
	padapter->mlmepriv.ChannelPlan = hal_com_get_channel_plan(
		padapter
		, hwinfo?hwinfo[EEPROM_ChannelPlan_8812]:0xFF
		, padapter->registrypriv.channel_plan
		, RT_CHANNEL_DOMAIN_REALTEK_DEFINE
		, AutoLoadFail
	);

	DBG_871X("mlmepriv.ChannelPlan = 0x%02x\n", padapter->mlmepriv.ChannelPlan);
}

VOID
Hal_EfuseParseXtal_8812A(
	IN	PADAPTER	pAdapter,
	IN	u8*			hwinfo,
	IN	BOOLEAN		AutoLoadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	if(!AutoLoadFail)
	{
		pHalData->CrystalCap = hwinfo[EEPROM_XTAL_8812];
		if(pHalData->CrystalCap == 0xFF)
			pHalData->CrystalCap = EEPROM_Default_CrystalCap_8812;	 //what value should 8812 set?
	}
	else
	{
		pHalData->CrystalCap = EEPROM_Default_CrystalCap_8812;
	}
	DBG_871X("CrystalCap: 0x%2x\n", pHalData->CrystalCap);
}

VOID
Hal_ReadAntennaDiversity8812A(
	IN	PADAPTER		pAdapter,
	IN	u8*				PROMContent,
	IN	BOOLEAN			AutoLoadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	struct registry_priv	*registry_par = &pAdapter->registrypriv;
	
	if(!AutoLoadFail)
	{
		// Antenna Diversity setting.
		if(registry_par->antdiv_cfg == 2)
		{
			pHalData->AntDivCfg = (PROMContent[EEPROM_RF_BOARD_OPTION_8812]&0x18)>>3;
			if(PROMContent[EEPROM_RF_BOARD_OPTION_8812] == 0xFF)			
				pHalData->AntDivCfg = (EEPROM_DEFAULT_BOARD_OPTION&0x18)>>3;;				
		}
		else
		{
			pHalData->AntDivCfg = registry_par->antdiv_cfg;
		}

		if(pHalData->EEPROMBluetoothCoexist!=0 && pHalData->EEPROMBluetoothAntNum==Ant_x1)
			pHalData->AntDivCfg = 0;
        
		pHalData->TRxAntDivType = PROMContent[EEPROM_RF_ANTENNA_OPT_8812];  //todo by page
		if (pHalData->TRxAntDivType == 0xFF)
			pHalData->TRxAntDivType = FIXED_HW_ANTDIV; // For 88EE, 1Tx and 1RxCG are fixed.(1Ant, Tx and RxCG are both on aux port)
	}
	else
	{
		pHalData->AntDivCfg = 0;
		//pHalData->TRxAntDivType = pHalData->TRxAntDivType; // ?????
	}
	
	DBG_871X("SWAS: bHwAntDiv = %x, TRxAntDivType = %x\n", pHalData->AntDivCfg, pHalData->TRxAntDivType);
}

VOID
Hal_ReadPAType_8812A(
	IN	PADAPTER	Adapter,
	IN	u8*			PROMContent,
	IN	BOOLEAN		AutoloadFail
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	if( ! AutoloadFail )
	{
		if (GetRegAmplifierType2G(Adapter) == 0) // AUTO
		{
			pHalData->PAType_2G = EF1Byte( *(u8 *)&PROMContent[EEPROM_PA_TYPE_8812AU] );
			pHalData->LNAType_2G = EF1Byte( *(u8 *)&PROMContent[EEPROM_LNA_TYPE_2G_8812AU] );
			if (pHalData->PAType_2G == 0xFF && pHalData->LNAType_2G == 0xFF) {
				pHalData->PAType_2G = 0;
				pHalData->LNAType_2G = 0;
			}
			pHalData->ExternalPA_2G = ((pHalData->PAType_2G & BIT5) && (pHalData->PAType_2G & BIT4)) ? 1 : 0;
			pHalData->ExternalLNA_2G = ((pHalData->LNAType_2G & BIT7) && (pHalData->LNAType_2G & BIT3)) ? 1 : 0;
		}
		else 
		{
			pHalData->ExternalPA_2G  = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_PA)  ? 1 : 0;
			pHalData->ExternalLNA_2G = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_LNA) ? 1 : 0;
		}

		if (GetRegAmplifierType5G(Adapter) == 0) // AUTO
		{
			pHalData->PAType_5G = EF1Byte( *(u8 *)&PROMContent[EEPROM_PA_TYPE_8812AU] );
			pHalData->LNAType_5G = EF1Byte( *(u8 *)&PROMContent[EEPROM_LNA_TYPE_5G_8812AU] );
			if (pHalData->PAType_5G == 0xFF && pHalData->LNAType_5G == 0xFF) {
				pHalData->PAType_5G = 0;
				pHalData->LNAType_5G = 0;
			}
			pHalData->ExternalPA_5G = ((pHalData->PAType_5G & BIT1) && (pHalData->PAType_5G & BIT0)) ? 1 : 0;
			pHalData->ExternalLNA_5G = ((pHalData->LNAType_5G & BIT7) && (pHalData->LNAType_5G & BIT3)) ? 1 : 0;
		}
		else
		{
			pHalData->ExternalPA_5G  = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;
			pHalData->ExternalLNA_5G = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;
		}
	}
	else
	{
		pHalData->ExternalPA_2G  = EEPROM_Default_PAType; 
		pHalData->ExternalPA_5G  = 0xFF; 
		pHalData->ExternalLNA_2G = EEPROM_Default_LNAType;  
		pHalData->ExternalLNA_5G = 0xFF; 
		
		if (GetRegAmplifierType2G(Adapter) == 0) // AUTO
		{
			pHalData->ExternalPA_2G  = 0;
			pHalData->ExternalLNA_2G = 0;
		}
		else
		{
			pHalData->ExternalPA_2G  = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_PA)  ? 1 : 0;
			pHalData->ExternalLNA_2G = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_LNA) ? 1 : 0;
		}
		if (GetRegAmplifierType5G(Adapter) == 0) // AUTO
		{
			pHalData->ExternalPA_5G  = 0;
			pHalData->ExternalLNA_5G = 0;
		}
		else
		{
			pHalData->ExternalPA_5G  = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;
			pHalData->ExternalLNA_5G = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;
		}
	}
	DBG_871X("pHalData->PAType_2G is 0x%x, pHalData->ExternalPA_2G = %d\n", pHalData->PAType_2G, pHalData->ExternalPA_2G);
	DBG_871X("pHalData->PAType_5G is 0x%x, pHalData->ExternalPA_5G = %d\n", pHalData->PAType_5G, pHalData->ExternalPA_5G);
	DBG_871X("pHalData->LNAType_2G is 0x%x, pHalData->ExternalLNA_2G = %d\n", pHalData->LNAType_2G, pHalData->ExternalLNA_2G);
	DBG_871X("pHalData->LNAType_5G is 0x%x, pHalData->ExternalLNA_5G = %d\n", pHalData->LNAType_5G, pHalData->ExternalLNA_5G);
}

VOID
Hal_ReadPAType_8821A(
	IN	PADAPTER	Adapter,
	IN	u8*			PROMContent,
	IN	BOOLEAN		AutoloadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	if( ! AutoloadFail )
	{
		if (GetRegAmplifierType2G(Adapter) == 0) // AUTO
		{
			pHalData->PAType_2G = EF1Byte( *(u8 *)&PROMContent[EEPROM_PA_TYPE_8812AU] );
			pHalData->LNAType_2G = EF1Byte( *(u8 *)&PROMContent[EEPROM_LNA_TYPE_2G_8812AU] );
			if (pHalData->PAType_2G == 0xFF && pHalData->LNAType_2G == 0xFF) {
				pHalData->PAType_2G = 0;
				pHalData->LNAType_2G = 0;
			}
			pHalData->ExternalPA_2G = (pHalData->PAType_2G & BIT4) ? 1 : 0;
			pHalData->ExternalLNA_2G = (pHalData->LNAType_2G & BIT3) ? 1 : 0;
		}
		else 
		{
			pHalData->ExternalPA_2G  = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_PA)  ? 1 : 0;
			pHalData->ExternalLNA_2G = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_LNA) ? 1 : 0;
		}

		if (GetRegAmplifierType5G(Adapter) == 0) // AUTO
		{
			pHalData->PAType_5G = EF1Byte( *(u8 *)&PROMContent[EEPROM_PA_TYPE_8812AU] );
			pHalData->LNAType_5G = EF1Byte( *(u8 *)&PROMContent[EEPROM_LNA_TYPE_5G_8812AU] );
			if (pHalData->PAType_5G == 0xFF && pHalData->LNAType_5G == 0xFF) {
				pHalData->PAType_5G = 0;
				pHalData->LNAType_5G = 0;
			}
			pHalData->ExternalPA_5G = (pHalData->PAType_5G & BIT0) ? 1 : 0;
			pHalData->ExternalLNA_5G = (pHalData->LNAType_5G & BIT3) ? 1 : 0;
		}
		else
		{
			pHalData->ExternalPA_5G  = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;
			pHalData->ExternalLNA_5G = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;
		}
	}
	else
	{
		pHalData->ExternalPA_2G  = EEPROM_Default_PAType; 
		pHalData->ExternalPA_5G  = 0xFF; 
		pHalData->ExternalLNA_2G = EEPROM_Default_LNAType;	
		pHalData->ExternalLNA_5G = 0xFF; 
		
		if (GetRegAmplifierType2G(Adapter) == 0) // AUTO
		{		
			pHalData->ExternalPA_2G  = 0; 
			pHalData->ExternalLNA_2G = 0;  
		}
		else
		{
			pHalData->ExternalPA_2G  = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_PA)  ? 1 : 0; 
			pHalData->ExternalLNA_2G = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_LNA) ? 1 : 0;	
		}
		if (GetRegAmplifierType5G(Adapter) == 0) // AUTO
		{		
			pHalData->ExternalPA_5G  = 0; 
			pHalData->ExternalLNA_5G = 0;  
		}
		else
		{
			pHalData->ExternalPA_5G  = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;			
			pHalData->ExternalLNA_5G = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;  
		}
	}
	DBG_871X("pHalData->PAType_2G is 0x%x, pHalData->ExternalPA_2G = %d\n", pHalData->PAType_2G, pHalData->ExternalPA_2G);
	DBG_871X("pHalData->PAType_5G is 0x%x, pHalData->ExternalPA_5G = %d\n", pHalData->PAType_5G, pHalData->ExternalPA_5G);
	DBG_871X("pHalData->LNAType_2G is 0x%x, pHalData->ExternalLNA_2G = %d\n", pHalData->LNAType_2G, pHalData->ExternalLNA_2G);
	DBG_871X("pHalData->LNAType_5G is 0x%x, pHalData->ExternalLNA_5G = %d\n", pHalData->LNAType_5G, pHalData->ExternalLNA_5G);
}

VOID
Hal_ReadRFEType_8812A(
	IN	PADAPTER	Adapter,	
	IN	u8*			PROMContent,
	IN	BOOLEAN		AutoloadFail
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	if(!AutoloadFail)
	{
		if(GetRegRFEType(Adapter) != 64)
			pHalData->RFEType = GetRegRFEType(Adapter);
		else if(PROMContent[EEPROM_RFE_OPTION_8812] & BIT7)
		{
			if(pHalData->ExternalLNA_5G)
			{
				if(pHalData->ExternalPA_5G)		
				{
					if(pHalData->ExternalLNA_2G && pHalData->ExternalPA_2G )
						pHalData->RFEType = 3;
					else
						pHalData->RFEType = 0;
				}
				else
					pHalData->RFEType = 2;
			}
			else
				pHalData->RFEType = 4;
		}
		else
		{
			pHalData->RFEType = PROMContent[EEPROM_RFE_OPTION_8812]&0x3F;

			// 2013/03/19 MH Due to othe customer already use incorrect EFUSE map
			// to for their product. We need to add workaround to prevent to modify 
			// spec and notify all customer to revise the IC 0xca content. After
			// discussing with Willis an YN, revise driver code to prevent.
			if (pHalData->RFEType == 4 && 
				(pHalData->ExternalPA_5G == _TRUE || pHalData->ExternalPA_2G == _TRUE ||
				pHalData->ExternalLNA_5G == _TRUE || pHalData->ExternalLNA_2G == _TRUE))
			{
				if (IS_HARDWARE_TYPE_8812AU(Adapter))
					pHalData->RFEType = 0;
				else if (IS_HARDWARE_TYPE_8812E(Adapter))
					pHalData->RFEType = 2;
			}
		}
	}
	else
	{
		if(GetRegRFEType(Adapter) != 64)
			pHalData->RFEType = GetRegRFEType(Adapter);
		else
			pHalData->RFEType = EEPROM_DEFAULT_RFE_OPTION;
	}

	DBG_871X("RFE Type: 0x%2x\n", pHalData->RFEType);
}

//
// 2013/04/15 MH Add 8812AU- VL/VS/VN for different board type.
//
VOID
hal_ReadUsbType_8812AU(
	IN	PADAPTER	Adapter,
	IN	u8			*PROMContent,
	IN	BOOLEAN		AutoloadFail
	)
{
	//if (IS_HARDWARE_TYPE_8812AU(Adapter) && Adapter->UsbModeMechanism.RegForcedUsbMode == 5)
	{
		PHAL_DATA_TYPE pHalData = GET_HAL_DATA(Adapter);
		u8	reg_tmp, i, j, antenna = 0, wmode = 0;
		// Read anenna type from EFUSE 1019/1018
		for (i = 0; i < 2; i++)
		{
			// Check efuse address 1019
			// Check efuse address 1018
			efuse_OneByteRead(Adapter, 1019-i, &reg_tmp, _FALSE);

			for (j = 0; j < 2; j++)
			{
				// CHeck bit 7-5
				// Check bit 3-1
				antenna = ((reg_tmp&0xee) >> (5-(j*4)));
				if (antenna == 0)
					continue;
				else
				{					
					break;
				}	
			}
		}

		// Read anenna type from EFUSE 1021/1020
		for (i = 0; i < 2; i++)
		{
			// Check efuse address 1019
			// Check efuse address 1018
			efuse_OneByteRead(Adapter, 1021-i, &reg_tmp, _FALSE);

			for (j = 0; j < 2; j++)
			{
				// CHeck bit 3-2
				// Check bit 1-0
				wmode = ((reg_tmp&0x0f) >> (2-(j*2)));
				if (wmode)
					continue;
				else
				{					
					break;
				}	
			}
		}

		// Antenna == 1 WMODE = 3 RTL8812AU-VL 11AC + USB2.0 Mode
		if (antenna == 1)
		{
			// Config 8812AU as 1*1 mode AC mode.
			pHalData->rf_type = RF_1T1R;
			//UsbModeSwitch_SetUsbModeMechOn(Adapter, FALSE);
			//pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU_VL;
			DBG_871X("%s(): EFUSE_HIDDEN_812AU_VL\n",__FUNCTION__);
		}
		else if (antenna == 2)
		{
			if (wmode == 3)
			{
				if (PROMContent[EEPROM_USB_MODE_8812] == 0x2)
				{
					// RTL8812AU Normal Mode. No further action.
					//pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU;
					DBG_871X("%s(): EFUSE_HIDDEN_812AU\n",__FUNCTION__);
				}
				else	
				{
					// Antenna == 2 WMODE = 3 RTL8812AU-VS 11AC + USB2.0 Mode
					// Driver will not support USB automatic switch
					//UsbModeSwitch_SetUsbModeMechOn(Adapter, FALSE);
					//pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU_VS;
					DBG_871X("%s(): EFUSE_HIDDEN_812AU_VS\n",__FUNCTION__);
				}
			}
			else if (wmode == 2)
			{
				// Antenna == 2 WMODE = 2 RTL8812AU-VN 11N only + USB2.0 Mode
				//UsbModeSwitch_SetUsbModeMechOn(Adapter, FALSE);
				//pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU_VN;
				DBG_871X("%s(): EFUSE_HIDDEN_812AU_VN\n",__FUNCTION__);
			}
		}	
	}
}

enum{
		VOLTAGE_V25						= 0x03,
		LDOE25_SHIFT						= 28 ,
	};

static VOID
Hal_EfusePowerSwitch8812A(
	IN	PADAPTER	pAdapter,
	IN	u8		bWrite,
	IN	u8		PwrState)
{
	u8	tempval;
	u16	tmpV16;
	#define EFUSE_ACCESS_ON_JAGUAR 0x69
	#define EFUSE_ACCESS_OFF_JAGUAR 0x00
	if (PwrState == _TRUE)
	{
		rtw_write8(pAdapter, REG_EFUSE_BURN_GNT_8812, EFUSE_ACCESS_ON_JAGUAR);

		// 1.2V Power: From VDDON with Power Cut(0x0000h[15]), defualt valid
		tmpV16 = rtw_read16(pAdapter,REG_SYS_ISO_CTRL);
		if( ! (tmpV16 & PWC_EV12V ) ){
			tmpV16 |= PWC_EV12V ;
			//rtw_write16(pAdapter,REG_SYS_ISO_CTRL,tmpV16);
		}
		// Reset: 0x0000h[28], default valid
		tmpV16 =  rtw_read16(pAdapter,REG_SYS_FUNC_EN);
		if( !(tmpV16 & FEN_ELDR) ){
			tmpV16 |= FEN_ELDR ;
			rtw_write16(pAdapter,REG_SYS_FUNC_EN,tmpV16);
		}

		// Clock: Gated(0x0008h[5]) 8M(0x0008h[1]) clock from ANA, default valid
		tmpV16 = rtw_read16(pAdapter,REG_SYS_CLKR);
		if( (!(tmpV16 & LOADER_CLK_EN) )  ||(!(tmpV16 & ANA8M) ) )
		{
			tmpV16 |= (LOADER_CLK_EN |ANA8M ) ;
			rtw_write16(pAdapter,REG_SYS_CLKR,tmpV16);
		}

		if(bWrite == _TRUE)
		{
			// Enable LDO 2.5V before read/write action
			tempval = rtw_read8(pAdapter, EFUSE_TEST+3);
			tempval &= ~(BIT3|BIT4|BIT5|BIT6);
			tempval |= (VOLTAGE_V25 << 3);
			tempval |= BIT7;
			rtw_write8(pAdapter, EFUSE_TEST+3, tempval);
		}
	}
	else
	{
		rtw_write8(pAdapter, REG_EFUSE_BURN_GNT_8812, EFUSE_ACCESS_OFF_JAGUAR);

		if(bWrite == _TRUE){
			// Disable LDO 2.5V after read/write action
			tempval = rtw_read8(pAdapter, EFUSE_TEST+3);
			rtw_write8(pAdapter, EFUSE_TEST+3, (tempval & 0x7F));
		}
	}
}

static VOID
rtl8812_EfusePowerSwitch(
	IN	PADAPTER	pAdapter,
	IN	u8		bWrite,
	IN	u8		PwrState)
{
	Hal_EfusePowerSwitch8812A(pAdapter, bWrite, PwrState);	
}

static inline BOOLEAN
Hal_EfuseSwitchToBank8812A(
	IN		PADAPTER	pAdapter,
	IN		u1Byte		bank,
	IN		BOOLEAN		bPseudoTest
	)
{
	return _FALSE;
}

static VOID
Hal_EfuseReadEFuse8812A(
	PADAPTER		Adapter,
	u16			_offset,
	u16 			_size_byte,
	u8      		*pbuf,
	IN	BOOLEAN	bPseudoTest
	)
{
	u8	*efuseTbl = NULL;
	u16	eFuse_Addr = 0;
	u8	offset=0, wden=0;
	u16	i, j;
	u16	**eFuseWord = NULL;
	u16	efuse_utilized = 0;
	u8	efuse_usage = 0;
	u8	offset_2_0=0;
	u8	efuseHeader=0, efuseExtHdr=0, efuseData=0;

	//
	// Do NOT excess total size of EFuse table. Added by Roger, 2008.11.10.
	//
	if((_offset + _size_byte)>EFUSE_MAP_LEN_JAGUAR)
	{// total E-Fuse table is 512bytes
		DBG_8192C("Hal_EfuseReadEFuse8812A(): Invalid offset(%#x) with read bytes(%#x)!!\n",_offset, _size_byte);
		goto exit;
	}

	efuseTbl = (u8*)rtw_zmalloc(EFUSE_MAP_LEN_JAGUAR);
	if(efuseTbl == NULL)
	{
		DBG_871X("%s: alloc efuseTbl fail!\n", __FUNCTION__);
		goto exit;
	}

	eFuseWord= (u16 **)rtw_malloc2d(EFUSE_MAX_SECTION_JAGUAR, EFUSE_MAX_WORD_UNIT, sizeof(u16));
	if(eFuseWord == NULL)
	{
		DBG_871X("%s: alloc eFuseWord fail!\n", __FUNCTION__);
		goto exit;
	}

	// 0. Refresh efuse init map as all oxFF.
	for (i = 0; i < EFUSE_MAX_SECTION_JAGUAR; i++)
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++)
			eFuseWord[i][j] = 0xFFFF;

	//
	// 1. Read the first byte to check if efuse is empty!!!
	// 
	//
	efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseHeader, bPseudoTest);	

	if(efuseHeader != 0xFF)
	{
		efuse_utilized++;
	}
	else
	{
		DBG_871X("EFUSE is empty\n");
		efuse_utilized = 0;
		goto exit;
	}
	//RT_DISP(FEEPROM, EFUSE_READ_ALL, ("Hal_EfuseReadEFuse8812A(): efuse_utilized: %d\n", efuse_utilized));

	//
	// 2. Read real efuse content. Filter PG header and every section data.
	//
	while((efuseHeader != 0xFF) && AVAILABLE_EFUSE_ADDR_8812(eFuse_Addr))
	{
		//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("efuse_Addr-%d efuse_data=%x\n", eFuse_Addr-1, *rtemp8));
	
		// Check PG header for section num.
		if(EXT_HEADER(efuseHeader))		//extended header
		{
			offset_2_0 = GET_HDR_OFFSET_2_0(efuseHeader);
			//RT_DISP(FEEPROM, EFUSE_READ_ALL, ("extended header offset_2_0=%X\n", offset_2_0));

			efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseExtHdr, bPseudoTest);	

			//RT_DISP(FEEPROM, EFUSE_READ_ALL, ("efuse[%X]=%X\n", eFuse_Addr-1, efuseExtHdr));

			if(efuseExtHdr != 0xff)
			{
				efuse_utilized++;
				if(ALL_WORDS_DISABLED(efuseExtHdr))
				{
					efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseHeader, bPseudoTest);
					if(efuseHeader != 0xff)
					{
						efuse_utilized++;
					}
					break;
				}
				else
				{
					offset = ((efuseExtHdr & 0xF0) >> 1) | offset_2_0;
					wden = (efuseExtHdr & 0x0F);
				}
			}
			else
			{
				DBG_871X("Error condition, extended = 0xff\n");
				// We should handle this condition.
				break;
			}
		}
		else
		{
			offset = ((efuseHeader >> 4) & 0x0f);
			wden = (efuseHeader & 0x0f);
		}
		
		if(offset < EFUSE_MAX_SECTION_JAGUAR)
		{
			// Get word enable value from PG header
			//RT_DISP(FEEPROM, EFUSE_READ_ALL, ("Offset-%X Worden=%X\n", offset, wden));

			for(i=0; i<EFUSE_MAX_WORD_UNIT; i++)
			{
				// Check word enable condition in the section				
				if(!(wden & (0x01<<i)))
				{
					efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseData, bPseudoTest);
					//RT_DISP(FEEPROM, EFUSE_READ_ALL, ("efuse[%X]=%X\n", eFuse_Addr-1, efuseData));
					efuse_utilized++;
					eFuseWord[offset][i] = (efuseData & 0xff);

					if(!AVAILABLE_EFUSE_ADDR_8812(eFuse_Addr))
						break;

					efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseData, bPseudoTest);
					//RT_DISP(FEEPROM, EFUSE_READ_ALL, ("efuse[%X]=%X\n", eFuse_Addr-1, efuseData));
					efuse_utilized++;
					eFuseWord[offset][i] |= (((u16)efuseData << 8) & 0xff00);

					if(!AVAILABLE_EFUSE_ADDR_8812(eFuse_Addr)) 
						break;
				}
			}
		}

		// Read next PG header
		efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseHeader, bPseudoTest);	
		//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("Addr=%d rtemp 0x%x\n", eFuse_Addr, *rtemp8));

		if(efuseHeader != 0xFF)
		{
			efuse_utilized++;
		}
	}

	//
	// 3. Collect 16 sections and 4 word unit into Efuse map.
	//
	for(i=0; i<EFUSE_MAX_SECTION_JAGUAR; i++)
	{
		for(j=0; j<EFUSE_MAX_WORD_UNIT; j++)
		{
			efuseTbl[(i*8)+(j*2)]=(eFuseWord[i][j] & 0xff);
			efuseTbl[(i*8)+((j*2)+1)]=((eFuseWord[i][j] >> 8) & 0xff);
		}
	}

	//RT_DISP(FEEPROM, EFUSE_READ_ALL, ("Hal_EfuseReadEFuse8812A(): efuse_utilized: %d\n", efuse_utilized));

	//
	// 4. Copy from Efuse map to output pointer memory!!!
	//
	for(i=0; i<_size_byte; i++)
	{		
		pbuf[i] = efuseTbl[_offset+i];
	}

	//
	// 5. Calculate Efuse utilization.
	//
	efuse_usage = (u1Byte)((eFuse_Addr*100)/EFUSE_REAL_CONTENT_LEN_JAGUAR);
	rtw_hal_set_hwreg(Adapter, HW_VAR_EFUSE_BYTES, (u8 *)&eFuse_Addr);

exit:
	if(efuseTbl)
		rtw_mfree(efuseTbl, EFUSE_MAP_LEN_JAGUAR);

	if(eFuseWord)
		rtw_mfree2d((void *)eFuseWord, EFUSE_MAX_SECTION_JAGUAR, EFUSE_MAX_WORD_UNIT, sizeof(u16));
}

static VOID
rtl8812_ReadEFuse(
	PADAPTER	Adapter,
	u8		efuseType,
	u16		_offset,
	u16 		_size_byte,
	u8      	*pbuf,
	IN	BOOLEAN	bPseudoTest
	)
{
#ifdef DBG_IOL_READ_EFUSE_MAP
	u8 logical_map[512];
#endif

#ifdef CONFIG_IOL_READ_EFUSE_MAP
	if(!bPseudoTest )//&& rtw_IOL_applied(Adapter))
	{
		int ret = _FAIL;

		rtw_hal_power_on(Adapter);
		
		iol_mode_enable(Adapter, 1);
		#ifdef DBG_IOL_READ_EFUSE_MAP
		iol_read_efuse(Adapter, 0, _offset, _size_byte, logical_map);
		#else
		ret = iol_read_efuse(Adapter, 0, _offset, _size_byte, pbuf);
		#endif
		iol_mode_enable(Adapter, 0);	
		
		if(_SUCCESS == ret) 
			goto exit;
	}
#endif
	Hal_EfuseReadEFuse8812A(Adapter, _offset, _size_byte, pbuf, bPseudoTest);

#ifdef CONFIG_IOL_READ_EFUSE_MAP
exit:
#endif
	
#ifdef DBG_IOL_READ_EFUSE_MAP
	if(_rtw_memcmp(logical_map, Adapter->eeprompriv.efuse_eeprom_data, 0x130) == _FALSE)
	{
		int i;
		DBG_871X("%s compare first 0x130 byte fail\n", __FUNCTION__);
		for(i=0;i<512;i++)
		{
			if(i%16==0)
				DBG_871X("0x%03x: ", i);
			DBG_871X("%02x ", logical_map[i]);
			if(i%16==15)
				DBG_871X("\n");
		}
		DBG_871X("\n");
	}
#endif
}

//Do not support BT
VOID
Hal_EFUSEGetEfuseDefinition8812A(
	IN		PADAPTER	pAdapter,
	IN		u1Byte		efuseType,
	IN		u1Byte		type,
	OUT		PVOID		pOut
	)
{
	switch(type)
	{
		case TYPE_EFUSE_MAX_SECTION:
			{
				u8*	pMax_section;
				pMax_section = (u8*)pOut;
				*pMax_section = EFUSE_MAX_SECTION_JAGUAR;
			}
			break;
		case TYPE_EFUSE_REAL_CONTENT_LEN:
			{
				u16* pu2Tmp;
				pu2Tmp = (u16*)pOut;
				*pu2Tmp = EFUSE_REAL_CONTENT_LEN_JAGUAR;
			}
			break;
		case TYPE_EFUSE_CONTENT_LEN_BANK:
			{
				u16* pu2Tmp;
				pu2Tmp = (u16*)pOut;
				*pu2Tmp = EFUSE_REAL_CONTENT_LEN_JAGUAR;
			}
			break;
		case TYPE_AVAILABLE_EFUSE_BYTES_BANK:
			{
				u16* pu2Tmp;
				pu2Tmp = (u16*)pOut;
				*pu2Tmp = (u16)(EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR);
			}
			break;
		case TYPE_AVAILABLE_EFUSE_BYTES_TOTAL:
			{
				u16* pu2Tmp;
				pu2Tmp = (u16*)pOut;
				*pu2Tmp = (u16)(EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR);
			}
			break;
		case TYPE_EFUSE_MAP_LEN:
			{
				u16* pu2Tmp;
				pu2Tmp = (u16*)pOut;
				*pu2Tmp = (u16)EFUSE_MAP_LEN_JAGUAR;
			}
			break;
		case TYPE_EFUSE_PROTECT_BYTES_BANK:
			{
				u8* pu1Tmp;
				pu1Tmp = (u8*)pOut;
				*pu1Tmp = (u8)(EFUSE_OOB_PROTECT_BYTES_JAGUAR);
			}
			break;
		default:
			{
				u8* pu1Tmp;
				pu1Tmp = (u8*)pOut;
				*pu1Tmp = 0;
			}
			break;
	}
}
VOID
Hal_EFUSEGetEfuseDefinition_Pseudo8812A(
	IN		PADAPTER	pAdapter,
	IN		u8			efuseType,
	IN		u8			type,
	OUT		PVOID		pOut
	)
{
	switch(type)
	{
		case TYPE_EFUSE_MAX_SECTION:
			{
				u8*		pMax_section;
				pMax_section = (pu1Byte)pOut;
				*pMax_section = EFUSE_MAX_SECTION_JAGUAR;
			}
			break;
		case TYPE_EFUSE_REAL_CONTENT_LEN:
			{
				u16* pu2Tmp;
				pu2Tmp = (pu2Byte)pOut;
				*pu2Tmp = EFUSE_REAL_CONTENT_LEN_JAGUAR;
			}
			break;
		case TYPE_EFUSE_CONTENT_LEN_BANK:
			{
				u16* pu2Tmp;
				pu2Tmp = (pu2Byte)pOut;
				*pu2Tmp = EFUSE_REAL_CONTENT_LEN_JAGUAR;
			}
			break;
		case TYPE_AVAILABLE_EFUSE_BYTES_BANK:
			{
				u16* pu2Tmp;
				pu2Tmp = (pu2Byte)pOut;
				*pu2Tmp = (u2Byte)(EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR);
			}
			break;
		case TYPE_AVAILABLE_EFUSE_BYTES_TOTAL:
			{
				u16* pu2Tmp;
				pu2Tmp = (pu2Byte)pOut;
				*pu2Tmp = (u2Byte)(EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR);
			}
			break;
		case TYPE_EFUSE_MAP_LEN:
			{
				u16* pu2Tmp;
				pu2Tmp = (pu2Byte)pOut;
				*pu2Tmp = (u2Byte)EFUSE_MAP_LEN_JAGUAR;
			}
			break;
		case TYPE_EFUSE_PROTECT_BYTES_BANK:
			{
				u8* pu1Tmp;
				pu1Tmp = (u8*)pOut;
				*pu1Tmp = (u8)(EFUSE_OOB_PROTECT_BYTES_JAGUAR);
			}
			break;
		default:
			{
				u8* pu1Tmp;
				pu1Tmp = (u8*)pOut;
				*pu1Tmp = 0;
			}
			break;
	}
}


static VOID
rtl8812_EFUSE_GetEfuseDefinition(
	IN		PADAPTER	pAdapter,
	IN		u8		efuseType,
	IN		u8		type,
	OUT		void		*pOut,
	IN		BOOLEAN		bPseudoTest
	)
{
	if(bPseudoTest)
	{
		Hal_EFUSEGetEfuseDefinition_Pseudo8812A(pAdapter, efuseType, type, pOut);
	}
	else
	{
		Hal_EFUSEGetEfuseDefinition8812A(pAdapter, efuseType, type, pOut);
	}
}

static u8
Hal_EfuseWordEnableDataWrite8812A(	IN	PADAPTER	pAdapter,
							IN	u16		efuse_addr,
							IN	u8		word_en,
							IN	u8		*data,
							IN	BOOLEAN		bPseudoTest)
{
	u16	tmpaddr = 0;
	u16	start_addr = efuse_addr;
	u8	badworden = 0x0F;
	u8	tmpdata[8];

	_rtw_memset((PVOID)tmpdata, 0xff, PGPKT_DATA_SIZE);
	//RT_TRACE(COMP_EFUSE, DBG_LOUD, ("word_en = %x efuse_addr=%x\n", word_en, efuse_addr));

	if(!(word_en&BIT0))
	{
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter,start_addr++, data[0], bPseudoTest);
		efuse_OneByteWrite(pAdapter,start_addr++, data[1], bPseudoTest);

		efuse_OneByteRead(pAdapter,tmpaddr, &tmpdata[0], bPseudoTest);
		efuse_OneByteRead(pAdapter,tmpaddr+1, &tmpdata[1], bPseudoTest);
		if((data[0]!=tmpdata[0])||(data[1]!=tmpdata[1])){
			badworden &= (~BIT0);
		}
	}
	if(!(word_en&BIT1))
	{
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter,start_addr++, data[2], bPseudoTest);
		efuse_OneByteWrite(pAdapter,start_addr++, data[3], bPseudoTest);

		efuse_OneByteRead(pAdapter,tmpaddr    , &tmpdata[2], bPseudoTest);
		efuse_OneByteRead(pAdapter,tmpaddr+1, &tmpdata[3], bPseudoTest);
		if((data[2]!=tmpdata[2])||(data[3]!=tmpdata[3])){
			badworden &=( ~BIT1);
		}
	}
	if(!(word_en&BIT2))
	{
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter,start_addr++, data[4], bPseudoTest);
		efuse_OneByteWrite(pAdapter,start_addr++, data[5], bPseudoTest);

		efuse_OneByteRead(pAdapter,tmpaddr, &tmpdata[4], bPseudoTest);
		efuse_OneByteRead(pAdapter,tmpaddr+1, &tmpdata[5], bPseudoTest);
		if((data[4]!=tmpdata[4])||(data[5]!=tmpdata[5])){
			badworden &=( ~BIT2);
		}
	}
	if(!(word_en&BIT3))
	{
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter,start_addr++, data[6], bPseudoTest);
		efuse_OneByteWrite(pAdapter,start_addr++, data[7], bPseudoTest);

		efuse_OneByteRead(pAdapter,tmpaddr, &tmpdata[6], bPseudoTest);
		efuse_OneByteRead(pAdapter,tmpaddr+1, &tmpdata[7], bPseudoTest);
		if((data[6]!=tmpdata[6])||(data[7]!=tmpdata[7])){
			badworden &=( ~BIT3);
		}
	}
	return badworden;
}

static u8
rtl8812_Efuse_WordEnableDataWrite(	IN	PADAPTER	pAdapter,
							IN	u16		efuse_addr,
							IN	u8		word_en,
							IN	u8		*data,
							IN	BOOLEAN		bPseudoTest)
{
	u8	ret=0;

	ret = Hal_EfuseWordEnableDataWrite8812A(pAdapter, efuse_addr, word_en, data, bPseudoTest);

	return ret;
}


static u16
hal_EfuseGetCurrentSize_8812A(IN	PADAPTER	pAdapter,
		IN		BOOLEAN			bPseudoTest)
{
	int	bContinual = _TRUE;

	u16	efuse_addr = 0;
	u8	hoffset=0,hworden=0;
	u8	efuse_data,word_cnts=0;

	if(bPseudoTest)
	{
		efuse_addr = (u16)(fakeEfuseUsedBytes);
	}
	else
	{
		rtw_hal_get_hwreg(pAdapter, HW_VAR_EFUSE_BYTES, (u8 *)&efuse_addr);
	}
	//RTPRINT(FEEPROM, EFUSE_PG, ("hal_EfuseGetCurrentSize_8723A(), start_efuse_addr = %d\n", efuse_addr));

	while (	bContinual &&
			efuse_OneByteRead(pAdapter, efuse_addr ,&efuse_data, bPseudoTest) &&
			(efuse_addr  < EFUSE_REAL_CONTENT_LEN_JAGUAR))
	{
		if(efuse_data!=0xFF)
		{
			if((efuse_data&0x1F) == 0x0F)		//extended header
			{
				hoffset = efuse_data;
				efuse_addr++;
				efuse_OneByteRead(pAdapter, efuse_addr ,&efuse_data, bPseudoTest);
				if((efuse_data & 0x0F) == 0x0F)
				{
					efuse_addr++;
					continue;
				}
				else
				{
					hoffset = ((hoffset & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
					hworden = efuse_data & 0x0F;
				}
			}
			else
			{
				hoffset = (efuse_data>>4) & 0x0F;
				hworden =  efuse_data & 0x0F;
			}
			word_cnts = Efuse_CalculateWordCnts(hworden);
			//read next header
			efuse_addr = efuse_addr + (word_cnts*2)+1;
		}
		else
		{
			bContinual = _FALSE ;
		}
	}

	if(bPseudoTest)
	{
		fakeEfuseUsedBytes = efuse_addr;
		//RTPRINT(FEEPROM, EFUSE_PG, ("hal_EfuseGetCurrentSize_8723A(), return %d\n", fakeEfuseUsedBytes));
	}
	else
	{
		rtw_hal_set_hwreg(pAdapter, HW_VAR_EFUSE_BYTES, (u8 *)&efuse_addr);
		//RTPRINT(FEEPROM, EFUSE_PG, ("hal_EfuseGetCurrentSize_8723A(), return %d\n", efuse_addr));
	}

	return efuse_addr;
}

static u16
rtl8812_EfuseGetCurrentSize(
	IN	PADAPTER	pAdapter,
	IN	u8			efuseType,
	IN	BOOLEAN		bPseudoTest)
{
	u16	ret=0;

	ret = hal_EfuseGetCurrentSize_8812A(pAdapter, bPseudoTest);

	return ret;
}


static int
hal_EfusePgPacketRead_8812A(
	IN	PADAPTER	pAdapter,
	IN	u8			offset,
	IN	u8			*data,
	IN	BOOLEAN		bPseudoTest)
{
	u8	ReadState = PG_STATE_HEADER;

	int	bContinual = _TRUE;
	int	bDataEmpty = _TRUE ;

	u8	efuse_data,word_cnts = 0;
	u16	efuse_addr = 0;
	u8	hoffset = 0,hworden = 0;
	u8	tmpidx = 0;
	u8	tmpdata[8];
	//u8	max_section = 0;
	u8	tmp_header = 0;

	if(data==NULL)
		return _FALSE;
	if(offset>EFUSE_MAX_SECTION_JAGUAR)
		return _FALSE;

	_rtw_memset((PVOID)data, 0xff, sizeof(u8)*PGPKT_DATA_SIZE);
	_rtw_memset((PVOID)tmpdata, 0xff, sizeof(u8)*PGPKT_DATA_SIZE);


	//
	// <Roger_TODO> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// Skip dummy parts to prevent unexpected data read from Efuse.
	// By pass right now. 2009.02.19.
	//
	while(bContinual && (efuse_addr  < EFUSE_REAL_CONTENT_LEN_JAGUAR) )
	{
		//-------  Header Read -------------
		if(ReadState & PG_STATE_HEADER)
		{
			if(efuse_OneByteRead(pAdapter, efuse_addr ,&efuse_data, bPseudoTest)&&(efuse_data!=0xFF))
			{
				if(EXT_HEADER(efuse_data))
				{
					tmp_header = efuse_data;
					efuse_addr++;
					efuse_OneByteRead(pAdapter, efuse_addr ,&efuse_data, bPseudoTest);
					if(!ALL_WORDS_DISABLED(efuse_data))
					{
						hoffset = ((tmp_header & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
						hworden = efuse_data & 0x0F;
					}
					else
					{
						DBG_8192C("Error, All words disabled\n");
						efuse_addr++;
						break;
					}
				}
				else
				{
					hoffset = (efuse_data>>4) & 0x0F;
					hworden =  efuse_data & 0x0F;
				}
				word_cnts = Efuse_CalculateWordCnts(hworden);
				bDataEmpty = _TRUE ;

				if(hoffset==offset)
				{
					for(tmpidx = 0;tmpidx< word_cnts*2 ;tmpidx++)
					{
						if(efuse_OneByteRead(pAdapter, efuse_addr+1+tmpidx ,&efuse_data, bPseudoTest) )
						{
							tmpdata[tmpidx] = efuse_data;
							if(efuse_data!=0xff)
							{
								bDataEmpty = _FALSE;
							}
						}
					}
					if(bDataEmpty==_FALSE){
						ReadState = PG_STATE_DATA;
					}else{//read next header
						efuse_addr = efuse_addr + (word_cnts*2)+1;
						ReadState = PG_STATE_HEADER;
					}
				}
				else{//read next header
					efuse_addr = efuse_addr + (word_cnts*2)+1;
					ReadState = PG_STATE_HEADER;
				}

			}
			else{
				bContinual = _FALSE ;
			}
		}
		//-------  Data section Read -------------
		else if(ReadState & PG_STATE_DATA)
		{
			efuse_WordEnableDataRead(hworden,tmpdata,data);
			efuse_addr = efuse_addr + (word_cnts*2)+1;
			ReadState = PG_STATE_HEADER;
		}

	}

	if(	(data[0]==0xff) &&(data[1]==0xff) && (data[2]==0xff)  && (data[3]==0xff) &&
		(data[4]==0xff) &&(data[5]==0xff) && (data[6]==0xff)  && (data[7]==0xff))
		return _FALSE;
	else
		return _TRUE;

}

static int
rtl8812_Efuse_PgPacketRead(	IN	PADAPTER	pAdapter,
					IN	u8			offset,
					IN	u8			*data,
					IN	BOOLEAN		bPseudoTest)
{
	int	ret=0;

	ret = hal_EfusePgPacketRead_8812A(pAdapter, offset, data, bPseudoTest);

	return ret;
}

int
hal_EfusePgPacketWrite_8812A(IN	PADAPTER	pAdapter, 
					IN	u8 			offset,
					IN	u8			word_en,
					IN	u8			*data,
					IN	BOOLEAN		bPseudoTest)
{
	u8 WriteState = PG_STATE_HEADER; 	

	int bContinual = _TRUE,bDataEmpty=_TRUE; 
	//int bResult = _TRUE;
	u16	efuse_addr = 0;
	u8	efuse_data;

	u8	pg_header = 0, pg_header_temp = 0;

	u8	tmp_word_cnts=0,target_word_cnts=0;
	u8	tmp_header,match_word_en,tmp_word_en;

	PGPKT_STRUCT target_pkt;	
	PGPKT_STRUCT tmp_pkt;
	
	u8	originaldata[sizeof(u8)*8];
	u8	tmpindex = 0,badworden = 0x0F;

	static int	repeat_times = 0;

	BOOLEAN		bExtendedHeader = _FALSE;
	u8	efuseType=EFUSE_WIFI;
	
	//
	// <Roger_Notes> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// So we have to prevent unexpected data string connection, which will cause
	// incorrect data auto-load from HW. The total size is equal or smaller than 498bytes
	// (i.e., offset 0~497, and dummy 1bytes) expected after CP test.
	// 2009.02.19.
	//
	if( Efuse_GetCurrentSize(pAdapter, efuseType, bPseudoTest) >= (EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR))
	{
		DBG_871X("hal_EfusePgPacketWrite_8812A() error: %x >= %x\n", Efuse_GetCurrentSize(pAdapter, efuseType, bPseudoTest), (EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR));
		return _FALSE;
	}

	// Init the 8 bytes content as 0xff
	target_pkt.offset = offset;
	target_pkt.word_en= word_en;
	// Initial the value to avoid compile warning
	tmp_pkt.offset = 0;
	tmp_pkt.word_en= 0;

	//DBG_871X("hal_EfusePgPacketWrite_8812A target offset 0x%x word_en 0x%x \n", target_pkt.offset, target_pkt.word_en);

	_rtw_memset((PVOID)target_pkt.data, 0xFF, sizeof(u8)*8);
	
	efuse_WordEnableDataRead(word_en, data, target_pkt.data);
	target_word_cnts = Efuse_CalculateWordCnts(target_pkt.word_en);

	//efuse_reg_ctrl(pAdapter,_TRUE);//power on
	//DBG_871X("EFUSE Power ON\n");

	//
	// <Roger_Notes> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// So we have to prevent unexpected data string connection, which will cause
	// incorrect data auto-load from HW. Dummy 1bytes is additional.
	// 2009.02.19.
	//
	while( bContinual && (efuse_addr  < (EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR)) )
	{
		if(WriteState==PG_STATE_HEADER)
		{	
			bDataEmpty=_TRUE;
			badworden = 0x0F;		
			//************	so *******************
			//DBG_871X("EFUSE PG_STATE_HEADER\n");
			if (	efuse_OneByteRead(pAdapter, efuse_addr ,&efuse_data, bPseudoTest) &&
				(efuse_data!=0xFF))
			{	
				if((efuse_data&0x1F) == 0x0F)		//extended header
				{
					tmp_header = efuse_data;
					efuse_addr++;
					efuse_OneByteRead(pAdapter, efuse_addr ,&efuse_data, bPseudoTest);
					if((efuse_data & 0x0F) == 0x0F) //wren fail
					{
						u8 next = 0, next_next = 0, data = 0, i = 0; 
						u8 s = ((tmp_header & 0xF0) >> 4);
						efuse_OneByteRead(pAdapter, efuse_addr+1, &next, bPseudoTest);
						efuse_OneByteRead(pAdapter, efuse_addr+2, &next_next, bPseudoTest);
						if (next == 0xFF && next_next == 0xFF) { // Have enough space to make fake data to recover bad header.
							switch (s) {
								case 0x0:
								case 0x2:
								case 0x4:
								case 0x6:
								case 0x8:
								case 0xA:
								case 0xC:
									for (i = 0; i < 3; ++i) {
								        efuse_OneByteWrite(pAdapter, efuse_addr, 0x27, bPseudoTest);
										efuse_OneByteRead(pAdapter, efuse_addr, &data, bPseudoTest);
										if (data == 0x27)
											break;
									}										
									break;
								case 0xE:
									for (i = 0; i < 3; ++i) {
								        efuse_OneByteWrite(pAdapter, efuse_addr, 0x17, bPseudoTest);
										efuse_OneByteRead(pAdapter, efuse_addr, &data, bPseudoTest);
										if (data == 0x17)
											break;
									}				
									break;
								default:
									break;
							}
							efuse_OneByteWrite(pAdapter, efuse_addr+1, 0xFF, bPseudoTest);
							efuse_OneByteWrite(pAdapter, efuse_addr+2, 0xFF, bPseudoTest);							
							efuse_addr += 3;
						} else {						
							efuse_addr++;
						}
						continue;
					}
					else
					{
						tmp_pkt.offset = ((tmp_header & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
						tmp_pkt.word_en = efuse_data & 0x0F;
					}
				}
				else
				{ 
					u8 i = 0, data = 0;
					tmp_header	=  efuse_data;					
					tmp_pkt.offset	= (tmp_header>>4) & 0x0F;
					tmp_pkt.word_en = tmp_header & 0x0F;	
					
					if (tmp_pkt.word_en == 0xF) {
						u8 next = 0;
						efuse_OneByteRead(pAdapter, efuse_addr+1, &next, bPseudoTest);
						if (next == 0xFF) { // Have enough space to make fake data to recover bad header.
							tmp_header = (tmp_header & 0xF0) | 0x7;
							for (i = 0; i < 3; ++i) {
						        efuse_OneByteWrite(pAdapter, efuse_addr, tmp_header, bPseudoTest);
								efuse_OneByteRead(pAdapter, efuse_addr, &data, bPseudoTest);
								if (data == tmp_header)
									break;
							}											
							efuse_OneByteWrite(pAdapter, efuse_addr+1, 0xFF, bPseudoTest);
							efuse_OneByteWrite(pAdapter, efuse_addr+2, 0xFF, bPseudoTest);
							efuse_addr += 2;				
						}
					}
				}
				tmp_word_cnts =  Efuse_CalculateWordCnts(tmp_pkt.word_en);

				//DBG_871X("section offset 0x%x worden 0x%x\n", tmp_pkt.offset, tmp_pkt.word_en);

				//************	so-1 *******************
				if(tmp_pkt.offset  != target_pkt.offset)
				{
					efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet
#if (EFUSE_ERROE_HANDLE == 1)
					WriteState = PG_STATE_HEADER;
#endif
				}
				else		//write the same offset
				{	
					//DBG_871X("hal_EfusePgPacketWrite_8812A section offset the same\n");
					//************	so-2 *******************
					for(tmpindex=0 ; tmpindex<(tmp_word_cnts*2) ; tmpindex++)
					{
						if(efuse_OneByteRead(pAdapter, (efuse_addr+1+tmpindex) ,&efuse_data, bPseudoTest)&&(efuse_data != 0xFF)){
							bDataEmpty = _FALSE;	
						}
					}	
					//************	so-2-1 *******************
					if(bDataEmpty == _FALSE)
					{		
						//DBG_871X("hal_EfusePgPacketWrite_8812A section offset the same and data is NOT empty\n");
					
						efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet										
#if (EFUSE_ERROE_HANDLE == 1)
						WriteState=PG_STATE_HEADER;
#endif
					}
					else
					{//************  so-2-2 *******************
						//DBG_871X("hal_EfusePgPacketWrite_8812A section data empty\n");
						match_word_en = 0x0F;			//same bit as original wren
						if(   !( (target_pkt.word_en&BIT0)|(tmp_pkt.word_en&BIT0)  ))
						{
							 match_word_en &= (~BIT0);
						}	
						if(   !( (target_pkt.word_en&BIT1)|(tmp_pkt.word_en&BIT1)  ))
						{
							 match_word_en &= (~BIT1);
						}
						if(   !( (target_pkt.word_en&BIT2)|(tmp_pkt.word_en&BIT2)  ))
						{
							 match_word_en &= (~BIT2);
						}
						if(   !( (target_pkt.word_en&BIT3)|(tmp_pkt.word_en&BIT3)  ))
						{
							 match_word_en &= (~BIT3);
						}					
												
						//************	so-2-2-A *******************
						if((match_word_en&0x0F)!=0x0F)
						{							
							badworden = Efuse_WordEnableDataWrite(pAdapter,efuse_addr+1, tmp_pkt.word_en ,target_pkt.data, bPseudoTest);
							
							//************	so-2-2-A-1 *******************
							//############################
							if(0x0F != (badworden&0x0F))
							{														
								u8	reorg_offset = offset;
								u8	reorg_worden=badworden;								
								Efuse_PgPacketWrite(pAdapter, reorg_offset, reorg_worden, target_pkt.data, bPseudoTest);	
							}	
							//############################						

							tmp_word_en = 0x0F; 	//not the same bit as original wren
							if(  (target_pkt.word_en&BIT0)^(match_word_en&BIT0)  )
							{
								tmp_word_en &= (~BIT0);
							}
							if(   (target_pkt.word_en&BIT1)^(match_word_en&BIT1) )
							{
								tmp_word_en &=	(~BIT1);
							}
							if(   (target_pkt.word_en&BIT2)^(match_word_en&BIT2) )
							{
								tmp_word_en &= (~BIT2);
							}						
							if(   (target_pkt.word_en&BIT3)^(match_word_en&BIT3) )
							{
								tmp_word_en &=(~BIT3);
							}							
						
							//************	so-2-2-A-2 *******************	
							if((tmp_word_en&0x0F)!=0x0F){
								//reorganize other pg packet						
//								efuse_addr = efuse_addr + (2*tmp_word_cnts) +1;//next pg packet addr							
								efuse_addr = Efuse_GetCurrentSize(pAdapter, efuseType, bPseudoTest);
								//===========================
								target_pkt.offset = offset;
								target_pkt.word_en= tmp_word_en;					
								//===========================
							}else{								
								bContinual = _FALSE;
							}
#if (EFUSE_ERROE_HANDLE == 1)
							WriteState=PG_STATE_HEADER;
							repeat_times++;
							if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
								bContinual = _FALSE;
								//bResult = _FALSE;
							}
#endif
						}
						else{//************  so-2-2-B *******************
							//reorganize other pg packet						
							efuse_addr = efuse_addr + (2*tmp_word_cnts) +1;//next pg packet addr							
							//===========================
							target_pkt.offset = offset;
							target_pkt.word_en= target_pkt.word_en; 				
							//===========================			
#if (EFUSE_ERROE_HANDLE == 1)
							WriteState=PG_STATE_HEADER;
#endif
						}		
					}				
				}				
				DBG_871X("EFUSE PG_STATE_HEADER-1\n");
			}
			else		//************	s1: header == oxff	*******************
			{
				bExtendedHeader = _FALSE;
			
				if(target_pkt.offset >= EFUSE_MAX_SECTION_BASE)
				{
					pg_header = ((target_pkt.offset &0x07) << 5) | 0x0F;

					//DBG_871X("hal_EfusePgPacketWrite_8812A extended pg_header[2:0] |0x0F 0x%x \n", pg_header);

					efuse_OneByteWrite(pAdapter,efuse_addr, pg_header, bPseudoTest);
					efuse_OneByteRead(pAdapter,efuse_addr, &tmp_header, bPseudoTest);

					while(tmp_header == 0xFF)
					{		
						//DBG_871X("hal_EfusePgPacketWrite_8812A extended pg_header[2:0] wirte fail \n");

						repeat_times++; 	
					
						if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
							bContinual = _FALSE;
							//bResult = _FALSE;
							efuse_addr++;
							break;
						}		
						efuse_OneByteWrite(pAdapter,efuse_addr, pg_header, bPseudoTest);
						efuse_OneByteRead(pAdapter,efuse_addr, &tmp_header, bPseudoTest);	
					}
					
					if(!bContinual)
						break;

					if(tmp_header == pg_header)
					{
						efuse_addr++;
						pg_header_temp = pg_header;
						pg_header = ((target_pkt.offset & 0x78) << 1 ) | target_pkt.word_en;

						//DBG_871X("hal_EfusePgPacketWrite_8812A extended pg_header[6:3] | worden 0x%x word_en 0x%x \n", pg_header, target_pkt.word_en);

						efuse_OneByteWrite(pAdapter,efuse_addr, pg_header, bPseudoTest);
						efuse_OneByteRead(pAdapter,efuse_addr, &tmp_header, bPseudoTest);

						while(tmp_header == 0xFF)
						{											
							repeat_times++; 	

							if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
								bContinual = _FALSE;
								//bResult = _FALSE;
								break;
							}							
							efuse_OneByteWrite(pAdapter,efuse_addr, pg_header, bPseudoTest);
							efuse_OneByteRead(pAdapter,efuse_addr, &tmp_header, bPseudoTest);
						}

						if(!bContinual)
							break;

						if((tmp_header & 0x0F) == 0x0F) //wren PG fail
						{
							repeat_times++; 	

							if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
								bContinual = _FALSE;
								//bResult = _FALSE;
								break;
							}							
							else
							{
								efuse_addr++;
								continue;
							}
						}
						else if(pg_header != tmp_header)	//offset PG fail						
						{
							bExtendedHeader = _TRUE;
							tmp_pkt.offset = ((pg_header_temp & 0xE0) >> 5) | ((tmp_header & 0xF0) >> 1);
							tmp_pkt.word_en=  tmp_header & 0x0F;					
							tmp_word_cnts =  Efuse_CalculateWordCnts(tmp_pkt.word_en);	
						}
					}
					else if ((tmp_header & 0x1F) == 0x0F)		//wrong extended header
					{
						efuse_addr+=2;
						continue;						
					}
				}
				else
				{
					pg_header = ((target_pkt.offset << 4)&0xf0) |target_pkt.word_en;
					efuse_OneByteWrite(pAdapter,efuse_addr, pg_header, bPseudoTest);
					efuse_OneByteRead(pAdapter,efuse_addr, &tmp_header, bPseudoTest);
				}
		
				if(tmp_header == pg_header)
				{ //************  s1-1*******************								
					WriteState = PG_STATE_DATA; 					
				}				
#if (EFUSE_ERROE_HANDLE == 1)
				else if(tmp_header == 0xFF){//************	s1-3: if Write or read func doesn't work *******************		
					//efuse_addr doesn't change
					WriteState = PG_STATE_HEADER;					
					repeat_times++;
					if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
						bContinual = _FALSE;
						//bResult = _FALSE;
					}
				}
#endif
				else
				{//************  s1-2 : fixed the header procedure *******************							
					if(!bExtendedHeader)
					{
						tmp_pkt.offset = (tmp_header>>4) & 0x0F;
						tmp_pkt.word_en=  tmp_header & 0x0F;					
						tmp_word_cnts =  Efuse_CalculateWordCnts(tmp_pkt.word_en);
					}
																											
					//************	s1-2-A :cover the exist data *******************
					_rtw_memset(originaldata, 0xff, sizeof(u8)*8);
					
					if(Efuse_PgPacketRead( pAdapter, tmp_pkt.offset,originaldata, bPseudoTest))
					{	//check if data exist					
						//efuse_reg_ctrl(pAdapter,_TRUE);//power on
						badworden = Efuse_WordEnableDataWrite(pAdapter,efuse_addr+1,tmp_pkt.word_en,originaldata, bPseudoTest);
						//############################
						if(0x0F != (badworden&0x0F))
						{														
							u8	reorg_offset = tmp_pkt.offset;
							u8	reorg_worden=badworden;								
							Efuse_PgPacketWrite(pAdapter,reorg_offset,reorg_worden,originaldata, bPseudoTest);	
							efuse_addr = Efuse_GetCurrentSize(pAdapter, efuseType, bPseudoTest);
						}
						//############################
						else{
							efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet							
						}
					}
					 //************  s1-2-B: wrong address*******************
					else
					{
						efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet
					}

#if (EFUSE_ERROE_HANDLE == 1)
					WriteState=PG_STATE_HEADER; 
					repeat_times++;
					if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
						bContinual = _FALSE;
						//bResult = _FALSE;
					}
#endif

					//DBG_871X("EFUSE PG_STATE_HEADER-2\n");
				}

			}

		}
		//write data state
		else if(WriteState==PG_STATE_DATA) 
		{	//************	s1-1  *******************
			//DBG_871X("EFUSE PG_STATE_DATA\n");
			badworden = 0x0f;
			badworden = Efuse_WordEnableDataWrite(pAdapter,efuse_addr+1,target_pkt.word_en,target_pkt.data, bPseudoTest);	
			if((badworden&0x0F)==0x0F)
			{ //************  s1-1-A *******************
				bContinual = _FALSE;
			}
			else
			{//reorganize other pg packet //************  s1-1-B *******************
				efuse_addr = efuse_addr + (2*target_word_cnts) +1;//next pg packet addr
										
				//===========================
				target_pkt.offset = offset;
				target_pkt.word_en= badworden;		
				target_word_cnts = Efuse_CalculateWordCnts(target_pkt.word_en);
				//===========================			
#if (EFUSE_ERROE_HANDLE == 1)
				WriteState=PG_STATE_HEADER; 
				repeat_times++;
				if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
					bContinual = _FALSE;
					//bResult = _FALSE;
				}
#endif
				//DBG_871X("EFUSE PG_STATE_HEADER-3\n");
			}
		}
	}

	if(efuse_addr  >= (EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR))
	{
		DBG_871X("hal_EfusePgPacketWrite_8812A(): efuse_addr(%#x) Out of size!!\n", efuse_addr);
	}
	//efuse_reg_ctrl(pAdapter,_FALSE);//power off
	
	return _TRUE;
}

static int
rtl8812_Efuse_PgPacketWrite(IN	PADAPTER	pAdapter,
					IN	u8 			offset,
					IN	u8			word_en,
					IN	u8			*data,
					IN	BOOLEAN		bPseudoTest)
{
	int	ret;

	ret = hal_EfusePgPacketWrite_8812A(pAdapter, offset, word_en, data, bPseudoTest);

	return ret;
}

#ifdef CONFIG_EFUSE_CONFIG_FILE
static s32 _halReadPGDataFromFile(PADAPTER padapter, u8 *pbuf)
{
	u32 i;
	struct file *fp;
	mm_segment_t fs;
	u8 temp[3];
	loff_t pos = 0;


	temp[2] = 0; // add end of string '\0'

	DBG_8192C("%s: Read Efuse from file [%s]\n", __FUNCTION__, EFUSE_FILE_PATH);
	fp = filp_open(EFUSE_FILE_PATH, O_RDONLY,  0);
	if (IS_ERR(fp)) {
		DBG_8192C("%s: Error, Read Efuse configure file FAIL!\n", __FUNCTION__);
		pEEPROM->bloadfile_fail_flag = _TRUE;
		return _FAIL;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);
	for (i=0; i<HWSET_MAX_SIZE_JAGUAR; i++)
	{
		vfs_read(fp, temp, 2, &pos);
		pbuf[i] = simple_strtoul(temp, NULL, 16);
		pos += 1; // Filter the space character
	}
	set_fs(fs);
	filp_close(fp, NULL);

#ifdef CONFIG_DEBUG
	DBG_8192C("Efuse configure file:\n");
	for (i=0; i<HWSET_MAX_SIZE_JAGUAR; i++)
	{
		if (i % 16 == 0)
			printk("\n");

		printk("%02X ", pbuf[i]);
	}
	printk("\n");
	DBG_8192C("\n");
#endif

	pEEPROM->bloadfile_fail_flag = _FALSE;
	return _SUCCESS;
}

static s32 _halReadMACAddrFromFile(PADAPTER padapter, u8 *pbuf)
{
	struct file *fp;
	mm_segment_t fs;
	loff_t pos = 0;
	u8 source_addr[18];
	u8 *head, *end;
	u32	curtime;
	u32 i;
	s32 ret = _SUCCESS;

	u8 null_mac_addr[ETH_ALEN] = {0, 0, 0, 0, 0, 0};
	u8 multi_mac_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


	curtime = rtw_get_current_time();

	_rtw_memset(source_addr, 0, 18);
	_rtw_memset(pbuf, 0, ETH_ALEN);

	fp = filp_open(MAC_ADDRESS_FILE_PATH, O_RDONLY,  0);
	if (IS_ERR(fp))
	{
		ret = _FAIL;
		DBG_8192C("%s: Error, Read MAC address file FAIL!\n", __FUNCTION__);
	}
	else
	{
		fs = get_fs();
		set_fs(KERNEL_DS);

		vfs_read(fp, source_addr, 18, &pos);
		source_addr[17] = ':';

		head = end = source_addr;
		for (i=0; i<ETH_ALEN; i++)
		{
			while (end && (*end != ':') )
				end++;

			if (end && (*end == ':') )
				*end = '\0';

			pbuf[i] = simple_strtoul(head, NULL, 16 );

			if (end) {
				end++;
				head = end;
			}
		}
		set_fs(fs);
		filp_close(fp, NULL);

		DBG_8192C("%s: Read MAC address from file [%s]\n", __FUNCTION__, MAC_ADDRESS_FILE_PATH);
		DBG_8192C("WiFi MAC address: " MAC_FMT "\n", MAC_ARG(pbuf));
	}

	if (_rtw_memcmp(pbuf, null_mac_addr, ETH_ALEN) ||
		_rtw_memcmp(pbuf, multi_mac_addr, ETH_ALEN))
	{
		pbuf[0] = 0x00;
		pbuf[1] = 0xe0;
		pbuf[2] = 0x4c;
		pbuf[3] = (u8)(curtime & 0xff) ;
		pbuf[4] = (u8)((curtime>>8) & 0xff) ;
		pbuf[5] = (u8)((curtime>>16) & 0xff) ;
	}

	DBG_8192C("%s: Permanent Address = " MAC_FMT "\n", __FUNCTION__, MAC_ARG(pbuf));

	return ret;
}
#endif // CONFIG_EFUSE_CONFIG_FILE

void InitRDGSetting8812A(PADAPTER padapter)
{
	rtw_write8(padapter, REG_RD_CTRL, 0xFF);
	rtw_write16(padapter, REG_RD_NAV_NXT, 0x200);
	rtw_write8(padapter, REG_RD_RESP_PKT_TH, 0x05);
}

void ReadRFType8812A(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);

#if DISABLE_BB_RF
	pHalData->rf_chip = RF_PSEUDO_11N;
#else
	pHalData->rf_chip = RF_6052;
#endif

	//if (pHalData->rf_type == RF_1T1R){
	//	pHalData->bRFPathRxEnable[0] = _TRUE;
	//}
	//else {	// Default unknow type is 2T2r
	//	pHalData->bRFPathRxEnable[0] = pHalData->bRFPathRxEnable[1] = _TRUE;
	//}

	if (IsSupported24G(padapter->registrypriv.wireless_mode) && 
		IsSupported5G(padapter->registrypriv.wireless_mode))
		pHalData->BandSet = BAND_ON_BOTH;
	else if (IsSupported5G(padapter->registrypriv.wireless_mode))
		pHalData->BandSet = BAND_ON_5G;
	else
		pHalData->BandSet = BAND_ON_2_4G;

	//if(padapter->bInHctTest)
	//	pHalData->BandSet = BAND_ON_2_4G;
}

void rtl8812_GetHalODMVar(	
	PADAPTER				Adapter,
	HAL_ODM_VARIABLE		eVariable,
	PVOID					pValue1,
	BOOLEAN					bSet)
{
	//HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	//PDM_ODM_T podmpriv = &pHalData->odmpriv;
	switch(eVariable){
		case HAL_ODM_STA_INFO:
			break;
		default:
			break;
	}
}

void rtl8812_SetHalODMVar(
	PADAPTER				Adapter,
	HAL_ODM_VARIABLE		eVariable,
	PVOID					pValue1,
	BOOLEAN					bSet)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T podmpriv = &pHalData->odmpriv;
	//_irqL irqL;
	switch(eVariable){
		case HAL_ODM_STA_INFO:
			{	
				struct sta_info *psta = (struct sta_info *)pValue1;				
				if(bSet){
					DBG_8192C("### Set STA_(%d) info\n",psta->mac_id);
					ODM_CmnInfoPtrArrayHook(podmpriv, ODM_CMNINFO_STA_STATUS,psta->mac_id,psta);
					#if(RATE_ADAPTIVE_SUPPORT==1)
					ODM_RAInfo_Init(podmpriv,psta->mac_id);
					#endif
				}
				else{
					DBG_8192C("### Clean STA_(%d) info\n",psta->mac_id);
					//_enter_critical_bh(&pHalData->odm_stainfo_lock, &irqL);
					ODM_CmnInfoPtrArrayHook(podmpriv, ODM_CMNINFO_STA_STATUS,psta->mac_id,NULL);
					
					//_exit_critical_bh(&pHalData->odm_stainfo_lock, &irqL);
			            }
			}
			break;
		case HAL_ODM_P2P_STATE:		
				ODM_CmnInfoUpdate(podmpriv,ODM_CMNINFO_WIFI_DIRECT,bSet);
			break;
		case HAL_ODM_WIFI_DISPLAY_STATE:
				ODM_CmnInfoUpdate(podmpriv,ODM_CMNINFO_WIFI_DISPLAY,bSet);
			break;
		default:
			break;
	}
}	

void rtl8812_clone_haldata(_adapter* dst_adapter, _adapter* src_adapter)
{
#ifdef CONFIG_SDIO_HCI
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(dst_adapter);
	//_thread_hdl_  SdioXmitThread;
#ifndef CONFIG_SDIO_TX_TASKLET
	_sema             temp_SdioXmitSema;
	_sema             temp_SdioXmitTerminateSema;
#endif
	//u8                    SdioTxFIFOFreePage[SDIO_TX_FREE_PG_QUEUE];
	_lock                temp_SdioTxFIFOFreePageLock;

#ifndef CONFIG_SDIO_TX_TASKLET
	_rtw_memcpy(&temp_SdioXmitSema, &(pHalData->SdioXmitSema), sizeof(_sema));
	_rtw_memcpy(&temp_SdioXmitTerminateSema, &(pHalData->SdioXmitTerminateSema), sizeof(_sema));
#endif
	_rtw_memcpy(&temp_SdioTxFIFOFreePageLock, &(pHalData->SdioTxFIFOFreePageLock), sizeof(_lock));

	_rtw_memcpy(dst_adapter->HalData, src_adapter->HalData, dst_adapter->hal_data_sz);

#ifndef CONFIG_SDIO_TX_TASKLET	
	_rtw_memcpy(&(pHalData->SdioXmitSema), &temp_SdioXmitSema, sizeof(_sema));
	_rtw_memcpy(&(pHalData->SdioXmitTerminateSema), &temp_SdioXmitTerminateSema, sizeof(_sema));
#endif
	_rtw_memcpy(&(pHalData->SdioTxFIFOFreePageLock), &temp_SdioTxFIFOFreePageLock, sizeof(_lock));	

#else
	_rtw_memcpy(dst_adapter->HalData, src_adapter->HalData, dst_adapter->hal_data_sz);
#endif

}

void rtl8812_start_thread(PADAPTER padapter)
{
}

void rtl8812_stop_thread(PADAPTER padapter)
{
}

void hal_notch_filter_8812(_adapter *adapter, bool enable)
{
	if (enable) {
		DBG_871X("Enable notch filter\n");
		//rtw_write8(adapter, rOFDM0_RxDSP+1, rtw_read8(adapter, rOFDM0_RxDSP+1) | BIT1);
	} else {
		DBG_871X("Disable notch filter\n");
		//rtw_write8(adapter, rOFDM0_RxDSP+1, rtw_read8(adapter, rOFDM0_RxDSP+1) & ~BIT1);
	}
}

u8
GetEEPROMSize8812A(
	IN	PADAPTER	Adapter
	)
{
	u8	size = 0;
	u32	curRCR;

	curRCR = rtw_read16(Adapter, REG_SYS_EEPROM_CTRL);
	size = (curRCR & EEPROMSEL) ? 6 : 4; // 6: EEPROM used is 93C46, 4: boot from E-Fuse.
	
	DBG_871X("EEPROM type is %s\n", size==4 ? "E-FUSE" : "93C46");
	//return size;
	return 4; // <20120713, Kordan> The default value of HW is 6 ?!!
}

void CheckAutoloadState8812A(PADAPTER padapter)
{
	PEEPROM_EFUSE_PRIV pEEPROM;
	u8 val8;


	pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

	/* check system boot selection */
	val8 = rtw_read8(padapter, REG_9346CR);
	pEEPROM->EepromOrEfuse = (val8 & BOOT_FROM_EEPROM) ? _TRUE : _FALSE;
	pEEPROM->bautoload_fail_flag = (val8 & EEPROM_EN) ? _FALSE : _TRUE;

	DBG_8192C("%s: 9346CR(%#x)=0x%02x, Boot from %s, Autoload %s!\n",
			__FUNCTION__, REG_9346CR, val8,
			(pEEPROM->EepromOrEfuse ? "EEPROM" : "EFUSE"),
			(pEEPROM->bautoload_fail_flag ? "Fail" : "OK"));
}

void InitPGData8812A(PADAPTER padapter)
{
	PEEPROM_EFUSE_PRIV pEEPROM;
	u32 i;
	//u16 val16;


	pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);
	
#ifdef CONFIG_EFUSE_CONFIG_FILE
	{
		s32 tmp;
		u32 addr;

		tmp = _halReadPGDataFromFile(padapter, pEEPROM->efuse_eeprom_data);
		pEEPROM->bloadfile_fail_flag = ((tmp==_FAIL) ? _TRUE : _FALSE);
		tmp = _halReadMACAddrFromFile(padapter, pEEPROM->mac_addr);
		pEEPROM->bloadmac_fail_flag = ((tmp==_FAIL) ? _TRUE : _FALSE);

#ifdef CONFIG_SDIO_HCI
		addr = EEPROM_MAC_ADDR_8821AS;
#elif defined(CONFIG_USB_HCI)
		if (IS_HARDWARE_TYPE_8812AU(padapter))
			addr = EEPROM_MAC_ADDR_8812AE;
		else
			addr = EEPROM_MAC_ADDR_8821AE;
#elif defined(CONFIG_PCI_HCI)
		if (IS_HARDWARE_TYPE_8812E(padapter))
			addr = EEPROM_MAC_ADDR_8812AE;
		else
			addr = EEPROM_MAC_ADDR_8821AE;
#endif // CONFIG_PCI_HCI
		_rtw_memcpy(&pEEPROM->efuse_eeprom_data[addr], pEEPROM->mac_addr, ETH_ALEN);
	}
#else // !CONFIG_EFUSE_CONFIG_FILE

	if (_FALSE == pEEPROM->bautoload_fail_flag)
	{
		// autoload OK.
		if (is_boot_from_eeprom(padapter))
		{
			// Read all Content from EEPROM or EFUSE.
			for (i = 0; i < HWSET_MAX_SIZE_JAGUAR; i += 2)
			{
				//val16 = EF2Byte(ReadEEprom(pAdapter, (u2Byte) (i>>1)));
				//*((u16*)(&PROMContent[i])) = val16;
			}
		}
		else
		{
			// Read EFUSE real map to shadow.
			EFUSE_ShadowMapUpdate(padapter, EFUSE_WIFI, _FALSE);
		}
	}
	else
	{
		// update to default value 0xFF
		if (!is_boot_from_eeprom(padapter))
			EFUSE_ShadowMapUpdate(padapter, EFUSE_WIFI, _FALSE);
	}
#endif // !CONFIG_EFUSE_CONFIG_FILE
}

void
ReadChipVersion8812A(
	IN	PADAPTER	Adapter
	)
{
	u32	value32;
	HAL_VERSION		ChipVersion;
	PHAL_DATA_TYPE	pHalData;


	pHalData = GET_HAL_DATA(Adapter);

	value32 = rtw_read32(Adapter, REG_SYS_CFG);
	DBG_8192C("%s SYS_CFG(0x%X)=0x%08x \n", __FUNCTION__, REG_SYS_CFG, value32);

	if(IS_HARDWARE_TYPE_8812(Adapter))
		ChipVersion.ICType = CHIP_8812;
	else
		ChipVersion.ICType = CHIP_8821;

	ChipVersion.ChipType = ((value32 & RTL_ID) ? TEST_CHIP : NORMAL_CHIP);

	if (Adapter->registrypriv.rf_config == RF_MAX_TYPE) {
		if(IS_HARDWARE_TYPE_8812(Adapter))
			ChipVersion.RFType = RF_TYPE_2T2R;//RF_2T2R;
		else
			ChipVersion.RFType = RF_TYPE_1T1R;//RF_1T1R;
	}

	if (IS_HARDWARE_TYPE_8812(Adapter))
		ChipVersion.VendorType = ((value32 & VENDOR_ID) ? CHIP_VENDOR_UMC : CHIP_VENDOR_TSMC);
	else
	{
		u32 vendor;

		vendor = (value32 & EXT_VENDOR_ID) >> EXT_VENDOR_ID_SHIFT;
		switch (vendor)
		{
			case 0:
				vendor = CHIP_VENDOR_TSMC;
				break;
			case 1:
				vendor = CHIP_VENDOR_SMIC;
				break;
			case 2:
				vendor = CHIP_VENDOR_UMC;
				break;
		}
		ChipVersion.VendorType = vendor;
	}
	ChipVersion.CUTVersion = (value32 & CHIP_VER_RTL_MASK)>>CHIP_VER_RTL_SHIFT; // IC version (CUT)
	if(IS_HARDWARE_TYPE_8812(Adapter))
		ChipVersion.CUTVersion += 1;

	//value32 = rtw_read32(Adapter, REG_GPIO_OUTSTS);
	ChipVersion.ROMVer = 0;	// ROM code version.

	// For multi-function consideration. Added by Roger, 2010.10.06.
	pHalData->MultiFunc = RT_MULTI_FUNC_NONE;
	value32 = rtw_read32(Adapter, REG_MULTI_FUNC_CTRL);
	pHalData->MultiFunc |= ((value32 & WL_FUNC_EN) ? RT_MULTI_FUNC_WIFI : 0);
	pHalData->MultiFunc |= ((value32 & BT_FUNC_EN) ? RT_MULTI_FUNC_BT : 0);
	pHalData->PolarityCtl = ((value32 & WL_HWPDN_SL) ? RT_POLARITY_HIGH_ACT : RT_POLARITY_LOW_ACT);

#if 1	
	dump_chip_info(ChipVersion);
#endif

	_rtw_memcpy(&pHalData->VersionID, &ChipVersion, sizeof(HAL_VERSION));

	if (IS_1T2R(ChipVersion)){
		pHalData->rf_type = RF_1T2R;
		pHalData->NumTotalRFPath = 2;
	}
	else if (IS_2T2R(ChipVersion)){
		pHalData->rf_type = RF_2T2R;
		pHalData->NumTotalRFPath = 2;
	}
	else{
		pHalData->rf_type = RF_1T1R;
		pHalData->NumTotalRFPath = 1;
	}
	
	DBG_8192C("RF_Type is %x!!\n", pHalData->rf_type);
}

VOID
Hal_PatchwithJaguar_8812(
	IN PADAPTER				Adapter,
	IN RT_MEDIA_STATUS		MediaStatus
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct mlme_ext_priv	*pmlmeext = &(Adapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if(	(MediaStatus == RT_MEDIA_CONNECT) && 
		(pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_REALTEK_JAGUAR_BCUTAP ))
	{
		rtw_write8(Adapter, rVhtlen_Use_Lsig_Jaguar, 0x1);
		rtw_write8(Adapter, REG_TCR+3, BIT2);
	}
	else
	{
		rtw_write8(Adapter, rVhtlen_Use_Lsig_Jaguar, 0x3F);
		rtw_write8(Adapter, REG_TCR+3, BIT0|BIT1|BIT2);
	}


	if(	(MediaStatus == RT_MEDIA_CONNECT) && 
		((pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_REALTEK_JAGUAR_BCUTAP) ||
		 (pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_REALTEK_JAGUAR_CCUTAP)))
	{
		pHalData->Reg837 |= BIT2;
		rtw_write8(Adapter, rBWIndication_Jaguar+3, pHalData->Reg837);
	}
	else
	{
		pHalData->Reg837 &= (~BIT2);
		rtw_write8(Adapter, rBWIndication_Jaguar+3, pHalData->Reg837);
	}
}

void UpdateHalRAMask8812A(PADAPTER padapter, u32 mac_id, u8 rssi_level)
{
	//volatile unsigned int result;
	u8	init_rate=0;
	u8	networkType, raid;	
	u32	mask,rate_bitmap;
	u8	shortGIrate = _FALSE;
	int	supportRateNum = 0;
	u8	arg[4] = {0};
	struct sta_info	*psta;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	//struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);

	if (mac_id >= NUM_STA) //CAM_SIZE
	{
		return;
	}

	psta = pmlmeinfo->FW_sta_info[mac_id].psta;
	if(psta == NULL)
	{
		return;
	}

	switch (mac_id)
	{
		case 0:// for infra mode
#ifdef CONFIG_CONCURRENT_MODE
		case 2:// first station uses macid=0, second station uses macid=2
#endif
			supportRateNum = rtw_get_rateset_len(cur_network->SupportedRates);
			networkType = judge_network_type(padapter, cur_network->SupportedRates, supportRateNum);
			//pmlmeext->cur_wireless_mode = networkType;
			//raid = networktype_to_raid(networkType);
			raid = rtw_hal_networktype_to_raid(padapter, networkType);

			mask = update_supported_rate(cur_network->SupportedRates, supportRateNum);
#ifdef CONFIG_80211AC_VHT
			if (pmlmeinfo->VHT_enable) {
				mask |= (rtw_vht_rate_to_bitmap(psta->vhtpriv.vht_mcs_map) << 12);
				shortGIrate = psta->vhtpriv.sgi;
			}
			else
#endif
			{
				mask |= (pmlmeinfo->HT_enable)? update_MCS_rate(&(pmlmeinfo->HT_caps)): 0;
				if (support_short_GI(padapter, &(pmlmeinfo->HT_caps)))
					shortGIrate = _TRUE;
			}
			
			break;

		case 1://for broadcast/multicast
			supportRateNum = rtw_get_rateset_len(pmlmeinfo->FW_sta_info[mac_id].SupportedRates);
			if(pmlmeext->cur_wireless_mode & WIRELESS_11B)
				networkType = WIRELESS_11B;
			else
				networkType = WIRELESS_11G;
			//raid = networktype_to_raid(networkType);
			raid = rtw_hal_networktype_to_raid(padapter, networkType);
			mask = update_basic_rate(cur_network->SupportedRates, supportRateNum);


			break;

		default: //for each sta in IBSS
			supportRateNum = rtw_get_rateset_len(pmlmeinfo->FW_sta_info[mac_id].SupportedRates);
			networkType = judge_network_type(padapter, pmlmeinfo->FW_sta_info[mac_id].SupportedRates, supportRateNum) & 0xf;
			//pmlmeext->cur_wireless_mode = networkType;
			//raid = networktype_to_raid(networkType);			
			raid = rtw_hal_networktype_to_raid(padapter, networkType);
			mask = update_supported_rate(cur_network->SupportedRates, supportRateNum);
			
			//todo: support HT in IBSS
			
			break;
	}
	
	//mask &=0x0fffffff;
	rate_bitmap = 0xffffffff;	
#ifdef	CONFIG_ODM_REFRESH_RAMASK
	{				
		rate_bitmap = ODM_Get_Rate_Bitmap(&pHalData->odmpriv,mac_id,mask,rssi_level);
		printk("%s => mac_id:%d, networkType:0x%02x, mask:0x%08x\n\t ==> rssi_level:%d, rate_bitmap:0x%08x\n",
			__FUNCTION__,mac_id,networkType,mask,rssi_level,rate_bitmap);
	}
#endif
	
	mask &= rate_bitmap;
	init_rate = get_highest_rate_idx(mask)&0x3f;

	//arg[0] = macid
	//arg[1] = raid
	//arg[2] = shortGIrate
	//arg[3] = init_rate

	arg[0] = mac_id;
	arg[1] = raid;
	arg[2] = shortGIrate;
	arg[3] = init_rate;

	rtl8812_set_raid_cmd(padapter, mask, arg);

	//set ra_id
	psta->raid = raid;
	psta->init_rate = init_rate;
}

void InitDefaultValue8821A(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;
	struct pwrctrl_priv *pwrctrlpriv;
	struct dm_priv *pdmpriv;
	u8 i;


	pHalData = GET_HAL_DATA(padapter);
	pwrctrlpriv = &padapter->pwrctrlpriv;
	pdmpriv = &pHalData->dmpriv;

	// init default value
	pHalData->fw_ractrl = _FALSE;		
	if (!pwrctrlpriv->bkeepfwalive)
		pHalData->LastHMEBoxNum = 0;	

	// init dm default value
	pHalData->bChnlBWInitialzed = _FALSE;
	pHalData->odmpriv.RFCalibrateInfo.bIQKInitialized = _FALSE;
	pHalData->odmpriv.RFCalibrateInfo.TM_Trigger = 0;//for IQK
	pHalData->pwrGroupCnt = 0;
	pHalData->PGMaxGroup = MAX_PG_GROUP;
	pHalData->odmpriv.RFCalibrateInfo.ThermalValue_HP_index = 0;
	for (i = 0; i < HP_THERMAL_NUM; i++)
		pHalData->odmpriv.RFCalibrateInfo.ThermalValue_HP[i] = 0;
}

VOID
_InitBeaconParameters_8812A(
	IN  PADAPTER Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	rtw_write16(Adapter, REG_BCN_CTRL, 0x1010);

	// TODO: Remove these magic number
	rtw_write16(Adapter, REG_TBTT_PROHIBIT,0x6404);// ms
	rtw_write8(Adapter, REG_DRVERLYINT, DRIVER_EARLY_INT_TIME_8812);// 5ms
	rtw_write8(Adapter, REG_BCNDMATIM, BCN_DMA_ATIME_INT_TIME_8812); // 2ms

	// Suggested by designer timchen. Change beacon AIFS to the largest number
	// beacause test chip does not contension before sending beacon. by tynli. 2009.11.03
	rtw_write16(Adapter, REG_BCNTCFG, 0x660F);

	pHalData->RegBcnCtrlVal = rtw_read8(Adapter, REG_BCN_CTRL);
	pHalData->RegTxPause = rtw_read8(Adapter, REG_TXPAUSE); 
	pHalData->RegFwHwTxQCtrl = rtw_read8(Adapter, REG_FWHW_TXQ_CTRL+2);
	pHalData->RegReg542 = rtw_read8(Adapter, REG_TBTT_PROHIBIT+2);
	pHalData->RegCR_1 = rtw_read8(Adapter, REG_CR+1);
}

static VOID
_BeaconFunctionEnable(
	IN	PADAPTER		Adapter,
	IN	BOOLEAN			Enable,
	IN	BOOLEAN			Linked
	)
{
	rtw_write8(Adapter, REG_BCN_CTRL, (BIT4 | BIT3 | BIT1));
	//SetBcnCtrlReg(Adapter, (BIT4 | BIT3 | BIT1), 0x00);
	//RT_TRACE(COMP_BEACON, DBG_LOUD, ("_BeaconFunctionEnable 0x550 0x%x\n", PlatformEFIORead1Byte(Adapter, 0x550)));			

	rtw_write8(Adapter, REG_RD_CTRL+1, 0x6F);	
}

static void ResumeTxBeacon(_adapter *padapter)
{
	HAL_DATA_TYPE*	pHalData = GET_HAL_DATA(padapter);	

	// 2010.03.01. Marked by tynli. No need to call workitem beacause we record the value
	// which should be read from register to a global variable.

	rtw_write8(padapter, REG_FWHW_TXQ_CTRL+2, (pHalData->RegFwHwTxQCtrl) | BIT6);
	pHalData->RegFwHwTxQCtrl |= BIT6;
	rtw_write8(padapter, REG_TBTT_PROHIBIT+1, 0xff);
	pHalData->RegReg542 |= BIT0;
	rtw_write8(padapter, REG_TBTT_PROHIBIT+2, pHalData->RegReg542);
}

static void StopTxBeacon(_adapter *padapter)
{
	HAL_DATA_TYPE*	pHalData = GET_HAL_DATA(padapter);

	rtw_write8(padapter, REG_FWHW_TXQ_CTRL+2, (pHalData->RegFwHwTxQCtrl) & (~BIT6));
	pHalData->RegFwHwTxQCtrl &= (~BIT6);
	rtw_write8(padapter, REG_TBTT_PROHIBIT+1, 0x64);
	pHalData->RegReg542 &= ~(BIT0);
	rtw_write8(padapter, REG_TBTT_PROHIBIT+2, pHalData->RegReg542);

	 //todo: CheckFwRsvdPageContent(Adapter);  // 2010.06.23. Added by tynli.
}

void SetBeaconRelatedRegisters8812A(PADAPTER padapter)
{
	u32	value32;
	//HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u32 bcn_ctrl_reg 			= REG_BCN_CTRL;
	//reset TSF, enable update TSF, correcting TSF On Beacon 
	
	//REG_BCN_INTERVAL
	//REG_BCNDMATIM
	//REG_ATIMWND
	//REG_TBTT_PROHIBIT
	//REG_DRVERLYINT
	//REG_BCN_MAX_ERR	
	//REG_BCNTCFG //(0x510)
	//REG_DUAL_TSF_RST
	//REG_BCN_CTRL //(0x550) 

	//BCN interval
#ifdef CONFIG_CONCURRENT_MODE
        if (padapter->iface_type == IFACE_PORT1){
		bcn_ctrl_reg = REG_BCN_CTRL_1;
        }
#endif	
	rtw_write16(padapter, REG_BCN_INTERVAL, pmlmeinfo->bcn_interval);
	rtw_write8(padapter, REG_ATIMWND, 0x02);// 2ms

	_InitBeaconParameters_8812A(padapter);

	rtw_write8(padapter, REG_SLOT, 0x09);

	value32 =rtw_read32(padapter, REG_TCR); 
	value32 &= ~TSFRST;
	rtw_write32(padapter,  REG_TCR, value32); 

	value32 |= TSFRST;
	rtw_write32(padapter, REG_TCR, value32); 

	// NOTE: Fix test chip's bug (about contention windows's randomness)
	rtw_write8(padapter,  REG_RXTSF_OFFSET_CCK, 0x50);
	rtw_write8(padapter, REG_RXTSF_OFFSET_OFDM, 0x50);

	_BeaconFunctionEnable(padapter, _TRUE, _TRUE);

	ResumeTxBeacon(padapter);

	//rtw_write8(padapter, 0x422, rtw_read8(padapter, 0x422)|BIT(6));
	
	//rtw_write8(padapter, 0x541, 0xff);

	//rtw_write8(padapter, 0x542, rtw_read8(padapter, 0x541)|BIT(0));

	rtw_write8(padapter, bcn_ctrl_reg, rtw_read8(padapter, bcn_ctrl_reg)|BIT(1));

}

static void hw_var_set_opmode(PADAPTER Adapter, u8 variable, u8* val)
{
	u8	val8;	
	u8	mode = *((u8 *)val);

#ifdef CONFIG_CONCURRENT_MODE
	if(Adapter->iface_type == IFACE_PORT1)
	{
		// disable Port1 TSF update
		rtw_write8(Adapter, REG_BCN_CTRL_1, rtw_read8(Adapter, REG_BCN_CTRL_1)|DIS_TSF_UDT);
		
		// set net_type
		val8 = rtw_read8(Adapter, MSR)&0x03;
		val8 |= (mode<<2);
		rtw_write8(Adapter, MSR, val8);
		
		DBG_871X("%s()-%d mode = %d\n", __FUNCTION__, __LINE__, mode);

		if((mode == _HW_STATE_STATION_) || (mode == _HW_STATE_NOLINK_))
		{
			if(!check_buddy_mlmeinfo_state(Adapter, WIFI_FW_AP_STATE))			
			{
				#ifdef CONFIG_INTERRUPT_BASED_TXBCN	

				#ifdef  CONFIG_INTERRUPT_BASED_TXBCN_EARLY_INT	
				rtw_write8(Adapter, REG_DRVERLYINT, 0x05);//restore early int time to 5ms
				UpdateInterruptMask8812AU(Adapter,_TRUE, 0, IMR_BCNDMAINT1_8812);	
				#endif // CONFIG_INTERRUPT_BASED_TXBCN_EARLY_INT
				
				#ifdef CONFIG_INTERRUPT_BASED_TXBCN_BCN_OK_ERR		
				UpdateInterruptMask8812AU(Adapter,_TRUE ,0, (IMR_TXBCN0ERR_8812|IMR_TXBCN0OK_8812));	
				#endif// CONFIG_INTERRUPT_BASED_TXBCN_BCN_OK_ERR
					
				#endif //CONFIG_INTERRUPT_BASED_TXBCN		
			

				StopTxBeacon(Adapter);
			}
			
			rtw_write8(Adapter,REG_BCN_CTRL_1, 0x19);//disable atim wnd
			//rtw_write8(Adapter,REG_BCN_CTRL_1, 0x18);
		}
		else if((mode == _HW_STATE_ADHOC_) /*|| (mode == _HW_STATE_AP_)*/)
		{
			ResumeTxBeacon(Adapter);
			rtw_write8(Adapter,REG_BCN_CTRL_1, 0x1a);
		}
		else if(mode == _HW_STATE_AP_)
		{
#ifdef CONFIG_INTERRUPT_BASED_TXBCN			
			#ifdef  CONFIG_INTERRUPT_BASED_TXBCN_EARLY_INT
			UpdateInterruptMask8812AU(Adapter,_TRUE ,IMR_BCNDMAINT1_8812, 0);
			#endif//CONFIG_INTERRUPT_BASED_TXBCN_EARLY_INT

			#ifdef CONFIG_INTERRUPT_BASED_TXBCN_BCN_OK_ERR	
			UpdateInterruptMask8812AU(Adapter,_TRUE ,(IMR_TXBCN0ERR_8812|IMR_TXBCN0OK_8812), 0);
			#endif//CONFIG_INTERRUPT_BASED_TXBCN_BCN_OK_ERR
					
#endif //CONFIG_INTERRUPT_BASED_TXBCN

			ResumeTxBeacon(Adapter);
					
			rtw_write8(Adapter, REG_BCN_CTRL_1, 0x12);

			//Set RCR
			//rtw_write32(padapter, REG_RCR, 0x70002a8e);//CBSSID_DATA must set to 0
			//rtw_write32(Adapter, REG_RCR, 0x7000228e);//CBSSID_DATA must set to 0
			rtw_write32(Adapter, REG_RCR, 0x7000208e);//CBSSID_DATA must set to 0,reject ICV_ERR packet
			//enable to rx data frame				
			rtw_write16(Adapter, REG_RXFLTMAP2, 0xFFFF);
			//enable to rx ps-poll
			rtw_write16(Adapter, REG_RXFLTMAP1, 0x0400);

			//Beacon Control related register for first time 
			rtw_write8(Adapter, REG_BCNDMATIM, 0x02); // 2ms		

			//rtw_write8(Adapter, REG_BCN_MAX_ERR, 0xFF);
			rtw_write8(Adapter, REG_ATIMWND_1, 0x0a); // 10ms for port1
			rtw_write16(Adapter, REG_BCNTCFG, 0x00);
			rtw_write16(Adapter, REG_TBTT_PROHIBIT, 0xff04);
			rtw_write16(Adapter, REG_TSFTR_SYN_OFFSET, 0x7fff);// +32767 (~32ms)
	
			//reset TSF2	
			rtw_write8(Adapter, REG_DUAL_TSF_RST, BIT(1));

			//enable BCN1 Function for if2
			//don't enable update TSF1 for if2 (due to TSF update when beacon/probe rsp are received)
			rtw_write8(Adapter, REG_BCN_CTRL_1, (DIS_TSF_UDT|EN_BCN_FUNCTION | EN_TXBCN_RPT|DIS_BCNQ_SUB));

			if(IS_HARDWARE_TYPE_8821(Adapter))
			{
				// select BCN on port 1
				rtw_write8(Adapter, REG_CCK_CHECK_8812,	 rtw_read8(Adapter, REG_CCK_CHECK_8812)|BIT(5));				
			}	

#ifdef CONFIG_CONCURRENT_MODE
			if(check_buddy_fwstate(Adapter, WIFI_FW_NULL_STATE))
				rtw_write8(Adapter, REG_BCN_CTRL, 
					rtw_read8(Adapter, REG_BCN_CTRL) & ~EN_BCN_FUNCTION);
#endif
			//BCN1 TSF will sync to BCN0 TSF with offset(0x518) if if1_sta linked
			//rtw_write8(Adapter, REG_BCN_CTRL_1, rtw_read8(Adapter, REG_BCN_CTRL_1)|BIT(5));
			//rtw_write8(Adapter, REG_DUAL_TSF_RST, BIT(3));
					
			//dis BCN0 ATIM  WND if if1 is station
			rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)|DIS_ATIM);

#ifdef CONFIG_TSF_RESET_OFFLOAD
			// Reset TSF for STA+AP concurrent mode
			if ( check_buddy_fwstate(Adapter, (WIFI_STATION_STATE|WIFI_ASOC_STATE)) ) {
				if (reset_tsf(Adapter, IFACE_PORT1) == _FALSE)
					DBG_871X("ERROR! %s()-%d: Reset port1 TSF fail\n",
						__FUNCTION__, __LINE__);
			}
#endif	// CONFIG_TSF_RESET_OFFLOAD			
		}
	}
	else //else for port0
#endif // CONFIG_CONCURRENT_MODE
	{
		// disable Port0 TSF update
		rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)|DIS_TSF_UDT);
		
		// set net_type
		val8 = rtw_read8(Adapter, MSR)&0x0c;
		val8 |= mode;
		rtw_write8(Adapter, MSR, val8);
		
		DBG_871X("%s()-%d mode = %d\n", __FUNCTION__, __LINE__, mode);
		
		if((mode == _HW_STATE_STATION_) || (mode == _HW_STATE_NOLINK_))
		{
#ifdef CONFIG_CONCURRENT_MODE
			if(!check_buddy_mlmeinfo_state(Adapter, WIFI_FW_AP_STATE))		
#endif // CONFIG_CONCURRENT_MODE
			{
			#ifdef CONFIG_INTERRUPT_BASED_TXBCN	
				#ifdef  CONFIG_INTERRUPT_BASED_TXBCN_EARLY_INT
				rtw_write8(Adapter, REG_DRVERLYINT, 0x05);//restore early int time to 5ms					
				UpdateInterruptMask8812AU(Adapter,_TRUE, 0, IMR_BCNDMAINT0_8812);	
				#endif//CONFIG_INTERRUPT_BASED_TXBCN_EARLY_INT
				
				#ifdef CONFIG_INTERRUPT_BASED_TXBCN_BCN_OK_ERR		
				UpdateInterruptMask8812AU(Adapter,_TRUE ,0, (IMR_TXBCN0ERR_8812|IMR_TXBCN0OK_8812));	
				#endif //CONFIG_INTERRUPT_BASED_TXBCN_BCN_OK_ERR
					
			#endif //CONFIG_INTERRUPT_BASED_TXBCN		
				StopTxBeacon(Adapter);
			}
			
			rtw_write8(Adapter,REG_BCN_CTRL, 0x19);//disable atim wnd
			//rtw_write8(Adapter,REG_BCN_CTRL, 0x18);
		}
		else if((mode == _HW_STATE_ADHOC_) /*|| (mode == _HW_STATE_AP_)*/)
		{
			ResumeTxBeacon(Adapter);
			rtw_write8(Adapter,REG_BCN_CTRL, 0x1a);
		}
		else if(mode == _HW_STATE_AP_)
		{
#ifdef CONFIG_INTERRUPT_BASED_TXBCN			
			#ifdef  CONFIG_INTERRUPT_BASED_TXBCN_EARLY_INT
			UpdateInterruptMask8812AU(Adapter,_TRUE ,IMR_BCNDMAINT0_8812, 0);
			#endif//CONFIG_INTERRUPT_BASED_TXBCN_EARLY_INT

			#ifdef CONFIG_INTERRUPT_BASED_TXBCN_BCN_OK_ERR	
			UpdateInterruptMask8812AU(Adapter,_TRUE ,(IMR_TXBCN0ERR_8812|IMR_TXBCN0OK_8812), 0);
			#endif//CONFIG_INTERRUPT_BASED_TXBCN_BCN_OK_ERR
					
#endif //CONFIG_INTERRUPT_BASED_TXBCN


			ResumeTxBeacon(Adapter);

			rtw_write8(Adapter, REG_BCN_CTRL, 0x12);

			//Set RCR
			//rtw_write32(padapter, REG_RCR, 0x70002a8e);//CBSSID_DATA must set to 0
			//rtw_write32(Adapter, REG_RCR, 0x7000228e);//CBSSID_DATA must set to 0
			rtw_write32(Adapter, REG_RCR, 0x7000208e);//CBSSID_DATA must set to 0,reject ICV_ERR packet
			//enable to rx data frame
			rtw_write16(Adapter, REG_RXFLTMAP2, 0xFFFF);
			//enable to rx ps-poll
			rtw_write16(Adapter, REG_RXFLTMAP1, 0x0400);

			//Beacon Control related register for first time
			rtw_write8(Adapter, REG_BCNDMATIM, 0x02); // 2ms			
			
			//rtw_write8(Adapter, REG_BCN_MAX_ERR, 0xFF);
			rtw_write8(Adapter, REG_ATIMWND, 0x0a); // 10ms
			rtw_write16(Adapter, REG_BCNTCFG, 0x00);
			rtw_write16(Adapter, REG_TBTT_PROHIBIT, 0xff04);
			rtw_write16(Adapter, REG_TSFTR_SYN_OFFSET, 0x7fff);// +32767 (~32ms)

			//reset TSF
			rtw_write8(Adapter, REG_DUAL_TSF_RST, BIT(0));
	
			//enable BCN0 Function for if1
			//don't enable update TSF0 for if1 (due to TSF update when beacon/probe rsp are received)
			rtw_write8(Adapter, REG_BCN_CTRL, (DIS_TSF_UDT|EN_BCN_FUNCTION | EN_TXBCN_RPT|DIS_BCNQ_SUB));

			if(IS_HARDWARE_TYPE_8821(Adapter))
			{
				// select BCN on port 0
				rtw_write8(Adapter, REG_CCK_CHECK_8812,	rtw_read8(Adapter, REG_CCK_CHECK_8812)&(~BIT(5)));				
			}

#ifdef CONFIG_CONCURRENT_MODE
			if(check_buddy_fwstate(Adapter, WIFI_FW_NULL_STATE))
				rtw_write8(Adapter, REG_BCN_CTRL_1, 
					rtw_read8(Adapter, REG_BCN_CTRL_1) & ~EN_BCN_FUNCTION);
#endif

			//dis BCN1 ATIM  WND if if2 is station
			rtw_write8(Adapter, REG_BCN_CTRL_1, rtw_read8(Adapter, REG_BCN_CTRL_1)|DIS_ATIM);
#ifdef CONFIG_TSF_RESET_OFFLOAD
			// Reset TSF for STA+AP concurrent mode
			if ( check_buddy_fwstate(Adapter, (WIFI_STATION_STATE|WIFI_ASOC_STATE)) ) {
				if (reset_tsf(Adapter, IFACE_PORT0) == _FALSE)
					DBG_871X("ERROR! %s()-%d: Reset port0 TSF fail\n",
						__FUNCTION__, __LINE__);
			}
#endif	// CONFIG_TSF_RESET_OFFLOAD		
		}
	}

}

static void hw_var_set_macaddr(PADAPTER Adapter, u8 variable, u8* val)
{
	u8 idx = 0;
	u32 reg_macid;

#ifdef CONFIG_CONCURRENT_MODE
	if(Adapter->iface_type == IFACE_PORT1)
	{
		reg_macid = REG_MACID1;
	}
	else
#endif
	{
		reg_macid = REG_MACID;
	}

	for(idx = 0 ; idx < 6; idx++)
	{
		rtw_write8(Adapter, (reg_macid+idx), val[idx]);
	}
	
}

static void hw_var_set_bssid(PADAPTER Adapter, u8 variable, u8* val)
{
	u8	idx = 0;
	u32 reg_bssid;

#ifdef CONFIG_CONCURRENT_MODE
	if(Adapter->iface_type == IFACE_PORT1)
	{
		reg_bssid = REG_BSSID1;
	}
	else
#endif
	{
		reg_bssid = REG_BSSID;
	}

	for(idx = 0 ; idx < 6; idx++)
	{
		rtw_write8(Adapter, (reg_bssid+idx), val[idx]);
	}

}

static void hw_var_set_bcn_func(PADAPTER Adapter, u8 variable, u8* val)
{
	u32 bcn_ctrl_reg;

#ifdef CONFIG_CONCURRENT_MODE
	if(Adapter->iface_type == IFACE_PORT1)
	{
		bcn_ctrl_reg = REG_BCN_CTRL_1;
	}	
	else
#endif		
	{		
		bcn_ctrl_reg = REG_BCN_CTRL;
	}

	if(*((u8 *)val))
	{
		rtw_write8(Adapter, bcn_ctrl_reg, (EN_BCN_FUNCTION | EN_TXBCN_RPT));
	}
	else
	{
		rtw_write8(Adapter, bcn_ctrl_reg, rtw_read8(Adapter, bcn_ctrl_reg)&(~(EN_BCN_FUNCTION | EN_TXBCN_RPT)));
	}
	

}

static inline void hw_var_set_correct_tsf(PADAPTER Adapter, u8 variable, u8* val)
{
#ifdef CONFIG_CONCURRENT_MODE
	u64	tsf;
	struct mlme_ext_priv	*pmlmeext = &Adapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	//tsf = pmlmeext->TSFValue - ((u32)pmlmeext->TSFValue % (pmlmeinfo->bcn_interval*1024)) -1024; //us
	tsf = pmlmeext->TSFValue - rtw_modular64(pmlmeext->TSFValue, (pmlmeinfo->bcn_interval*1024)) -1024; //us

	if(((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE) || ((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE))
	{				
		//pHalData->RegTxPause |= STOP_BCNQ;BIT(6)
		//rtw_write8(Adapter, REG_TXPAUSE, (rtw_read8(Adapter, REG_TXPAUSE)|BIT(6)));
		StopTxBeacon(Adapter);
	}

	if(Adapter->iface_type == IFACE_PORT1)
	{
		//disable related TSF function
		rtw_write8(Adapter, REG_BCN_CTRL_1, rtw_read8(Adapter, REG_BCN_CTRL_1)&(~BIT(3)));
							
		rtw_write32(Adapter, REG_TSFTR1, tsf);
		rtw_write32(Adapter, REG_TSFTR1+4, tsf>>32);


		//enable related TSF function
		rtw_write8(Adapter, REG_BCN_CTRL_1, rtw_read8(Adapter, REG_BCN_CTRL_1)|BIT(3));	

		// Update buddy port's TSF if it is SoftAP for beacon TX issue!
		if ( (pmlmeinfo->state&0x03) == WIFI_FW_STATION_STATE
			&& check_buddy_fwstate(Adapter, WIFI_AP_STATE)
		) { 
			//disable related TSF function
			rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)&(~BIT(3)));

			rtw_write32(Adapter, REG_TSFTR, tsf);
			rtw_write32(Adapter, REG_TSFTR+4, tsf>>32);

			//enable related TSF function
			rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)|BIT(3));
#ifdef CONFIG_TSF_RESET_OFFLOAD
		// Update buddy port's TSF(TBTT) if it is SoftAP for beacon TX issue!
			if (reset_tsf(Adapter, IFACE_PORT0) == _FALSE)
				DBG_871X("ERROR! %s()-%d: Reset port0 TSF fail\n",
					__FUNCTION__, __LINE__);

#endif	// CONFIG_TSF_RESET_OFFLOAD	
		}		

		
	}
	else
	{
		//disable related TSF function
		rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)&(~BIT(3)));
							
		rtw_write32(Adapter, REG_TSFTR, tsf);
		rtw_write32(Adapter, REG_TSFTR+4, tsf>>32);

		//enable related TSF function
		rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)|BIT(3));
		
		// Update buddy port's TSF if it is SoftAP for beacon TX issue!
		if ( (pmlmeinfo->state&0x03) == WIFI_FW_STATION_STATE
			&& check_buddy_fwstate(Adapter, WIFI_AP_STATE)
		) { 
			//disable related TSF function
			rtw_write8(Adapter, REG_BCN_CTRL_1, rtw_read8(Adapter, REG_BCN_CTRL_1)&(~BIT(3)));

			rtw_write32(Adapter, REG_TSFTR1, tsf);
			rtw_write32(Adapter, REG_TSFTR1+4, tsf>>32);

			//enable related TSF function
			rtw_write8(Adapter, REG_BCN_CTRL_1, rtw_read8(Adapter, REG_BCN_CTRL_1)|BIT(3));
#ifdef CONFIG_TSF_RESET_OFFLOAD
		// Update buddy port's TSF if it is SoftAP for beacon TX issue!
			if (reset_tsf(Adapter, IFACE_PORT1) == _FALSE)
				DBG_871X("ERROR! %s()-%d: Reset port1 TSF fail\n",
					__FUNCTION__, __LINE__);
#endif	// CONFIG_TSF_RESET_OFFLOAD
		}		

	}
				
							
	if(((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE) || ((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE))
	{
		//pHalData->RegTxPause  &= (~STOP_BCNQ);
		//rtw_write8(Adapter, REG_TXPAUSE, (rtw_read8(Adapter, REG_TXPAUSE)&(~BIT(6))));
		ResumeTxBeacon(Adapter);
	}
#endif
}

static inline void hw_var_set_mlme_disconnect(PADAPTER Adapter, u8 variable, u8* val)
{
#ifdef CONFIG_CONCURRENT_MODE
				
	if(check_buddy_mlmeinfo_state(Adapter, _HW_STATE_NOLINK_))	
		rtw_write16(Adapter, REG_RXFLTMAP2, 0x00);
	

	if(Adapter->iface_type == IFACE_PORT1)
	{
		//reset TSF1
		rtw_write8(Adapter, REG_DUAL_TSF_RST, BIT(1));

		//disable update TSF1
		rtw_write8(Adapter, REG_BCN_CTRL_1, rtw_read8(Adapter, REG_BCN_CTRL_1)|BIT(4));
	}
	else
	{
		//reset TSF
		rtw_write8(Adapter, REG_DUAL_TSF_RST, BIT(0));

		//disable update TSF
		rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)|BIT(4));
	}
#endif
}

static void hw_var_set_mlme_sitesurvey(PADAPTER Adapter, u8 variable, u8* val)
{
	u32	value_rcr, rcr_clear_bit, reg_bcn_ctl;
	u16	value_rxfltmap2;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	struct mlme_priv *pmlmepriv=&(Adapter->mlmepriv);

#ifdef CONFIG_CONCURRENT_MODE
	if(Adapter->iface_type == IFACE_PORT1)
		reg_bcn_ctl = REG_BCN_CTRL_1;
	else
#endif
		reg_bcn_ctl = REG_BCN_CTRL;

#ifdef CONFIG_FIND_BEST_CHANNEL

	rcr_clear_bit = (RCR_CBSSID_BCN | RCR_CBSSID_DATA);

	// Recieve all data frames
	value_rxfltmap2 = 0xFFFF;

#else /* CONFIG_FIND_BEST_CHANNEL */

	rcr_clear_bit = RCR_CBSSID_BCN;

	//config RCR to receive different BSSID & not to receive data frame
	value_rxfltmap2 = 0;

#endif /* CONFIG_FIND_BEST_CHANNEL */

	if( (check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
#ifdef CONFIG_CONCURRENT_MODE
		|| (check_buddy_fwstate(Adapter, WIFI_AP_STATE) == _TRUE)
#endif
		)
	{	
		rcr_clear_bit = RCR_CBSSID_BCN;	
	}
#ifdef CONFIG_TDLS
	// TDLS will clear RCR_CBSSID_DATA bit for connection.
	else if (Adapter->tdlsinfo.setup_state & TDLS_LINKED_STATE)
	{
		rcr_clear_bit = RCR_CBSSID_BCN;
	}
#endif // CONFIG_TDLS

	value_rcr = rtw_read32(Adapter, REG_RCR);

	if(*((u8 *)val))//under sitesurvey
	{
		value_rcr &= ~(rcr_clear_bit);
		rtw_write32(Adapter, REG_RCR, value_rcr);

		rtw_write16(Adapter, REG_RXFLTMAP2, value_rxfltmap2);

		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_ADHOC_STATE |WIFI_ADHOC_MASTER_STATE)) {
			//disable update TSF
			rtw_write8(Adapter, reg_bcn_ctl, rtw_read8(Adapter, reg_bcn_ctl)|DIS_TSF_UDT);
		}

		// Save orignal RRSR setting.
		pHalData->RegRRSR = rtw_read16(Adapter, REG_RRSR);

#ifdef CONFIG_CONCURRENT_MODE
		if(check_buddy_mlmeinfo_state(Adapter, WIFI_FW_AP_STATE) &&
			check_buddy_fwstate(Adapter, _FW_LINKED))
		{
			StopTxBeacon(Adapter);
		}
#endif
	}
	else//sitesurvey done
	{
		if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) 
#ifdef CONFIG_CONCURRENT_MODE
			|| check_buddy_fwstate(Adapter, (_FW_LINKED|WIFI_AP_STATE))
#endif
			)
		{
			//enable to rx data frame
			rtw_write16(Adapter, REG_RXFLTMAP2,0xFFFF);
		}

		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_ADHOC_STATE |WIFI_ADHOC_MASTER_STATE)) {
			//enable update TSF
			rtw_write8(Adapter, reg_bcn_ctl, rtw_read8(Adapter, reg_bcn_ctl)&(~(DIS_TSF_UDT)));
		}

		value_rcr |= rcr_clear_bit;
		rtw_write32(Adapter, REG_RCR, value_rcr);

		// Restore orignal RRSR setting.
		rtw_write16(Adapter, REG_RRSR, pHalData->RegRRSR);

#ifdef CONFIG_CONCURRENT_MODE
		if(check_buddy_mlmeinfo_state(Adapter, WIFI_FW_AP_STATE) &&
			check_buddy_fwstate(Adapter, _FW_LINKED))
		{
			ResumeTxBeacon(Adapter);			
		}
#endif
	}		
}

static inline void hw_var_set_mlme_join(PADAPTER Adapter, u8 variable, u8* val)
{
#ifdef CONFIG_CONCURRENT_MODE
	u8	RetryLimit = 0x30;
	u8	type = *((u8 *)val);
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	struct mlme_priv	*pmlmepriv = &Adapter->mlmepriv;
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);

	if(type == 0) // prepare to join
	{		
		if(check_buddy_mlmeinfo_state(Adapter, WIFI_FW_AP_STATE) &&
			check_buddy_fwstate(Adapter, _FW_LINKED))		
		{
			StopTxBeacon(Adapter);
		}
	
		//enable to rx data frame.Accept all data frame
		//rtw_write32(padapter, REG_RCR, rtw_read32(padapter, REG_RCR)|RCR_ADF);
		rtw_write16(Adapter, REG_RXFLTMAP2,0xFFFF);

		if(check_buddy_mlmeinfo_state(Adapter, WIFI_FW_AP_STATE))
		{
			rtw_write32(Adapter, REG_RCR, rtw_read32(Adapter, REG_RCR)|RCR_CBSSID_BCN);			
		}	
		else
		{
			rtw_write32(Adapter, REG_RCR, rtw_read32(Adapter, REG_RCR)|RCR_CBSSID_DATA|RCR_CBSSID_BCN);
		}	

		if(check_fwstate(pmlmepriv, WIFI_STATION_STATE) == _TRUE)
		{
			RetryLimit = (pEEPROM->CustomerID == RT_CID_CCX) ? 7 : 48;
		}
		else // Ad-hoc Mode
		{
			RetryLimit = 0x7;
		}
	}
	else if(type == 1) //joinbss_event call back when join res < 0
	{		
		if(check_buddy_mlmeinfo_state(Adapter, _HW_STATE_NOLINK_))		
			rtw_write16(Adapter, REG_RXFLTMAP2,0x00);

		if(check_buddy_mlmeinfo_state(Adapter, WIFI_FW_AP_STATE) &&
			check_buddy_fwstate(Adapter, _FW_LINKED))
		{
			ResumeTxBeacon(Adapter);			
			
			//reset TSF 1/2 after ResumeTxBeacon
			rtw_write8(Adapter, REG_DUAL_TSF_RST, BIT(1)|BIT(0));	
			
		}
	}
	else if(type == 2) //sta add event call back
	{
	 
		//enable update TSF
		if(Adapter->iface_type == IFACE_PORT1)
			rtw_write8(Adapter, REG_BCN_CTRL_1, rtw_read8(Adapter, REG_BCN_CTRL_1)&(~BIT(4)));
		else		
			rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)&(~BIT(4)));
				 
	
		if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE))
		{
			//fixed beacon issue for 8191su...........
			rtw_write8(Adapter,0x542 ,0x02);
			RetryLimit = 0x7;
		}


		if(check_buddy_mlmeinfo_state(Adapter, WIFI_FW_AP_STATE) &&
			check_buddy_fwstate(Adapter, _FW_LINKED))
		{
			ResumeTxBeacon(Adapter);			
			
			//reset TSF 1/2 after ResumeTxBeacon
			rtw_write8(Adapter, REG_DUAL_TSF_RST, BIT(1)|BIT(0));
		}
		
	}

	rtw_write16(Adapter, REG_RL, RetryLimit << RETRY_LIMIT_SHORT_SHIFT | RetryLimit << RETRY_LIMIT_LONG_SHIFT);
	
#endif
}

void SetHwReg8812A(PADAPTER padapter, u8 variable, u8 *pval)
{
	PHAL_DATA_TYPE pHalData;
	struct dm_priv *pdmpriv;
	PDM_ODM_T podmpriv;
	u8 val8;
	u16 val16;
	u32 val32;

_func_enter_;

	pHalData = GET_HAL_DATA(padapter);
	pdmpriv = &pHalData->dmpriv;
	podmpriv = &pHalData->odmpriv;

	switch (variable)
	{
		case HW_VAR_MEDIA_STATUS:
			val8 = rtw_read8(padapter, MSR) & 0x0c;
			val8 |= *pval;
			rtw_write8(padapter, MSR, val8);
			break;

		case HW_VAR_MEDIA_STATUS1:
			val8 = rtw_read8(padapter, MSR) & 0x03;
			val8 |= *pval << 2;
			rtw_write8(padapter, MSR, val8);
			break;

		case HW_VAR_SET_OPMODE:
			hw_var_set_opmode(padapter, variable, pval);
			break;

		case HW_VAR_MAC_ADDR:
			hw_var_set_macaddr(padapter, variable, pval);
			break;

		case HW_VAR_BSSID:
			hw_var_set_bssid(padapter, variable, pval);
			break;

		case HW_VAR_BASIC_RATE:
			{
				u16 BrateCfg = 0;
				//u8 RateIndex = 0;

				// 2007.01.16, by Emily
				// Select RRSR (in Legacy-OFDM and CCK)
				// For 8190, we select only 24M, 12M, 6M, 11M, 5.5M, 2M, and 1M from the Basic rate.
				// We do not use other rates.
				HalSetBrateCfg(padapter, pval, &BrateCfg);

				if(pHalData->CurrentBandType == BAND_ON_2_4G)
				{
					//CCK 2M ACK should be disabled for some BCM and Atheros AP IOT
					//because CCK 2M has poor TXEVM
					//CCK 5.5M & 11M ACK should be enabled for better performance

					pHalData->BasicRateSet = BrateCfg = (BrateCfg |0xd) & 0x15d;
					BrateCfg |= 0x01; // default enable 1M ACK rate
				}
				else // 5G
				{
					pHalData->BasicRateSet &= 0xFF0;
					BrateCfg |= 0x10; // default enable 6M ACK rate
				}
//				DBG_8192C("HW_VAR_BASIC_RATE: BrateCfg(%#x)\n", BrateCfg);

				// Set RRSR rate table.
				rtw_write8(padapter, REG_RRSR, BrateCfg&0xff);
				rtw_write8(padapter, REG_RRSR+1, (BrateCfg>>8)&0xff);
				rtw_write8(padapter, REG_RRSR+2, rtw_read8(padapter, REG_RRSR+2)&0xf0);
			}
			break;

		case HW_VAR_TXPAUSE:
			rtw_write8(padapter, REG_TXPAUSE, *pval);
			break;

		case HW_VAR_BCN_FUNC:
			hw_var_set_bcn_func(padapter, variable, pval);
			break;

		case HW_VAR_CORRECT_TSF:
#ifdef CONFIG_CONCURRENT_MODE
			hw_var_set_correct_tsf(padapter, variable, pval);
#else
			{
				u64	tsf;
				struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
				struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

				//tsf = pmlmeext->TSFValue - ((u32)pmlmeext->TSFValue % (pmlmeinfo->bcn_interval*1024)) -1024; //us
				tsf = pmlmeext->TSFValue - rtw_modular64(pmlmeext->TSFValue, (pmlmeinfo->bcn_interval*1024)) -1024; //us

				if(((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE) || ((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE))
				{				
					//pHalData->RegTxPause |= STOP_BCNQ;BIT(6)
					//rtw_write8(padapter, REG_TXPAUSE, (rtw_read8(padapter, REG_TXPAUSE)|BIT(6)));
					StopTxBeacon(padapter);
				}

				//disable related TSF function
				rtw_write8(padapter, REG_BCN_CTRL, rtw_read8(padapter, REG_BCN_CTRL)&(~BIT(3)));
							
				rtw_write32(padapter, REG_TSFTR, tsf);
				rtw_write32(padapter, REG_TSFTR+4, tsf>>32);

				//enable related TSF function
				rtw_write8(padapter, REG_BCN_CTRL, rtw_read8(padapter, REG_BCN_CTRL)|BIT(3));
				
							
				if(((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE) || ((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE))
				{
					//pHalData->RegTxPause  &= (~STOP_BCNQ);
					//rtw_write8(padapter, REG_TXPAUSE, (rtw_read8(padapter, REG_TXPAUSE)&(~BIT(6))));
					ResumeTxBeacon(padapter);
				}
			}
#endif
			break;

		case HW_VAR_CHECK_BSSID:
			val32 = rtw_read32(padapter, REG_RCR);
			if (*pval)
				val32 |= RCR_CBSSID_DATA|RCR_CBSSID_BCN;
			else
				val32 &= ~(RCR_CBSSID_DATA|RCR_CBSSID_BCN);
			rtw_write32(padapter, REG_RCR, val32);
			break;

		case HW_VAR_MLME_DISCONNECT:
#ifdef CONFIG_CONCURRENT_MODE
			hw_var_set_mlme_disconnect(padapter, variable, pval);
#else
			{
				// Set RCR to not to receive data frame when NO LINK state
//				val32 = rtw_read32(padapter, REG_RCR);
//				val32 &= ~RCR_ADF;
//				rtw_write32(padapter, REG_RCR, val32);

				// reject all data frames
				rtw_write16(padapter, REG_RXFLTMAP2, 0x00);

				// reset TSF
				val8 = BIT(0) | BIT(1);
				rtw_write8(padapter, REG_DUAL_TSF_RST, val8);

				// disable update TSF
				val8 = rtw_read8(padapter, REG_BCN_CTRL);
				val8 |= BIT(4);
				rtw_write8(padapter, REG_BCN_CTRL, val8);
			}
#endif
			break;

		case HW_VAR_MLME_SITESURVEY:
			hw_var_set_mlme_sitesurvey(padapter, variable,  pval);

#ifdef CONFIG_BT_COEXIST
			BT_WifiScanNotify(padapter, *pval?_TRUE:_FALSE);
#endif
			break;

		case HW_VAR_MLME_JOIN:
#ifdef CONFIG_CONCURRENT_MODE
			hw_var_set_mlme_join(padapter, variable, pval);
#else // !CONFIG_CONCURRENT_MODE
			{
				u8 RetryLimit = 0x30;
				u8 type = *(u8*)pval;
				struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
				EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

				if (type == 0) // prepare to join
				{
					//enable to rx data frame.Accept all data frame
					//rtw_write32(padapter, REG_RCR, rtw_read32(padapter, REG_RCR)|RCR_ADF);
					rtw_write16(padapter, REG_RXFLTMAP2, 0xFFFF);

					val32 = rtw_read32(padapter, REG_RCR);
					if (padapter->in_cta_test)
						val32 &= ~(RCR_CBSSID_DATA | RCR_CBSSID_BCN);//| RCR_ADF
					else
						val32 |= RCR_CBSSID_DATA|RCR_CBSSID_BCN;
					rtw_write32(padapter, REG_RCR, val32);

					if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) == _TRUE)
					{
						RetryLimit = (pEEPROM->CustomerID == RT_CID_CCX) ? 7 : 48;
					}
					else // Ad-hoc Mode
					{
						RetryLimit = 0x7;
					}

					pHalData->bNeedIQK = _TRUE;
				}
				else if (type == 1) //joinbss_event call back when join res < 0
				{
					rtw_write16(padapter, REG_RXFLTMAP2, 0x00);
				}
				else if (type == 2) //sta add event call back
				{
					//enable update TSF
					val8 = rtw_read8(padapter, REG_BCN_CTRL);
					val8 &= ~BIT(4);
					rtw_write8(padapter, REG_BCN_CTRL, val8);

					if (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE))
					{
						RetryLimit = 0x7;
					}
				}

				val16 = RetryLimit << RETRY_LIMIT_SHORT_SHIFT | RetryLimit << RETRY_LIMIT_LONG_SHIFT;
				rtw_write16(padapter, REG_RL, val16);
			}
#endif // !CONFIG_CONCURRENT_MODE

#ifdef CONFIG_BT_COEXIST
			switch (*pval)
			{
				case 0:
					// prepare to join
					BT_WifiAssociateNotify(padapter, _TRUE);
					break;
				case 1:
					// joinbss_event callback when join res < 0
					BT_WifiAssociateNotify(padapter, _FALSE);
					break;
				case 2:
					// sta add event callback
//					BT_WifiMediaStatusNotify(padapter, RT_MEDIA_CONNECT);
					break;
			}
#endif
			break;

		case HW_VAR_ON_RCR_AM:
			val32 = rtw_read32(padapter, REG_RCR);
			val32 |= RCR_AM;
            rtw_write32(padapter, REG_RCR, val32);
            DBG_8192C("%s, %d, RCR= %x\n", __FUNCTION__, __LINE__, rtw_read32(padapter, REG_RCR));
            break;
						
		case HW_VAR_OFF_RCR_AM:
			val32 = rtw_read32(padapter, REG_RCR);
			val32 &= ~RCR_AM;
            rtw_write32(padapter, REG_RCR, val32);
            DBG_8192C("%s, %d, RCR= %x\n", __FUNCTION__, __LINE__, rtw_read32(padapter, REG_RCR));
            break;

		case HW_VAR_BEACON_INTERVAL:
			rtw_write16(padapter, REG_BCN_INTERVAL, *(u16*)pval);
#ifdef CONFIG_INTERRUPT_BASED_TXBCN_EARLY_INT
			{
				struct mlme_ext_priv *pmlmeext;
				struct mlme_ext_info *pmlmeinfo;
				u16 bcn_interval;

				pmlmeext = &padapter->mlmeextpriv;
				pmlmeinfo = &pmlmeext->mlmext_info;
				bcn_interval = *((u16*)pval);

				if ((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE)
				{
					DBG_8192C("%s==> bcn_interval:%d, eraly_int:%d\n", __FUNCTION__, bcn_interval, bcn_interval>>1);
					rtw_write8(padapter, REG_DRVERLYINT, bcn_interval>>1);// 50ms for sdio 
				}			
			}
#endif // CONFIG_INTERRUPT_BASED_TXBCN_EARLY_INT
			break;

		case HW_VAR_SLOT_TIME:
			rtw_write8(padapter, REG_SLOT, *pval);
			break;

		case HW_VAR_RESP_SIFS:
			// SIFS_Timer = 0x0a0a0808;
			// RESP_SIFS for CCK
			rtw_write8(padapter, REG_RESP_SIFS_CCK, pval[0]); // SIFS_T2T_CCK (0x08)
			rtw_write8(padapter, REG_RESP_SIFS_CCK+1, pval[1]); //SIFS_R2T_CCK(0x08)
			// RESP_SIFS for OFDM
			rtw_write8(padapter, REG_RESP_SIFS_OFDM, pval[2]); //SIFS_T2T_OFDM (0x0a)
			rtw_write8(padapter, REG_RESP_SIFS_OFDM+1, pval[3]); //SIFS_R2T_OFDM(0x0a)
			break;

		case HW_VAR_ACK_PREAMBLE:
			{
				u8 bShortPreamble = *pval;

				// Joseph marked out for Netgear 3500 TKIP channel 7 issue.(Temporarily)
				val8 = (pHalData->nCur40MhzPrimeSC) << 5;
				if (bShortPreamble)
					val8 |= 0x80;
				rtw_write8(padapter, REG_RRSR+2, val8);
			}
			break;

		case HW_VAR_SEC_CFG:
#ifdef CONFIG_CONCURRENT_MODE
			// enable tx enc and rx dec engine, and no key search for MC/BC
			val8 = 0x0c | BIT(5);
#else
			val8 = *pval;
#endif
			rtw_write8(padapter, REG_SECCFG, val8);
			break;

		case HW_VAR_DM_FLAG:
			podmpriv->SupportAbility = *(u32*)pval;
			break;

		case HW_VAR_DM_FUNC_OP:
			if (*pval) // save dm flag
				podmpriv->BK_SupportAbility = podmpriv->SupportAbility;
			else // restore dm flag
				podmpriv->SupportAbility = podmpriv->BK_SupportAbility;
			break;

		case HW_VAR_DM_FUNC_SET:
			val32 = *(u32*)pval;
			if (val32 == DYNAMIC_ALL_FUNC_ENABLE) {
				pdmpriv->DMFlag = pdmpriv->InitDMFlag;
				podmpriv->SupportAbility = pdmpriv->InitODMFlag;
			} else {
				podmpriv->SupportAbility |= val32;
			}
			break;

		case HW_VAR_DM_FUNC_CLR:
			val32 = *(u32*)pval;
			// input is already a mask to clear function
			// don't invert it again! George,Lucas@20130513
			podmpriv->SupportAbility &= val32;
			break;

		case HW_VAR_CAM_EMPTY_ENTRY:
			{
				u8 ucIndex = *pval;
				u8 i;
				u32	ulCommand = 0;
				u32	ulContent = 0;
				u32	ulEncAlgo = CAM_AES;

				for (i=0; i<CAM_CONTENT_COUNT; i++)
				{
					// filled id in CAM config 2 byte
					if (i == 0)
					{
						ulContent |= (ucIndex & 0x03) | ((u16)(ulEncAlgo)<<2);
						//ulContent |= CAM_VALID;
					}
					else
					{
						ulContent = 0;
					}
					// polling bit, and No Write enable, and address
					ulCommand = CAM_CONTENT_COUNT*ucIndex+i;
					ulCommand = ulCommand | CAM_POLLINIG | CAM_WRITE;
					// write content 0 is equall to mark invalid
					rtw_write32(padapter, WCAMI, ulContent);  //delay_ms(40);
					rtw_write32(padapter, RWCAM, ulCommand);  //delay_ms(40);
				}
			}
			break;

		case HW_VAR_CAM_INVALID_ALL:
			val32 = BIT(31) | BIT(30);
			rtw_write32(padapter, RWCAM, val32);
			break;

		case HW_VAR_CAM_WRITE:
			{
				u32 cmd;
				u32 *cam_val = (u32*)pval;

				rtw_write32(padapter, WCAMI, cam_val[0]);

				cmd = CAM_POLLINIG | CAM_WRITE | cam_val[1];
				rtw_write32(padapter, RWCAM, cmd);
			}
			break;

		case HW_VAR_CAM_READ:
			break;

		case HW_VAR_AC_PARAM_VO:
			rtw_write32(padapter, REG_EDCA_VO_PARAM, *(u32*)pval);
			break;

		case HW_VAR_AC_PARAM_VI:
			rtw_write32(padapter, REG_EDCA_VI_PARAM, *(u32*)pval);
			break;

		case HW_VAR_AC_PARAM_BE:
			pHalData->AcParam_BE = *(u32*)pval;
			rtw_write32(padapter, REG_EDCA_BE_PARAM, *(u32*)pval);
			break;

		case HW_VAR_AC_PARAM_BK:
			rtw_write32(padapter, REG_EDCA_BK_PARAM, *(u32*)pval);
			break;

		case HW_VAR_ACM_CTRL:
			{
				u8 acm_ctrl;
				u8 AcmCtrl;

				acm_ctrl = *(u8*)pval;
				AcmCtrl = rtw_read8(padapter, REG_ACMHWCTRL);

				if (acm_ctrl > 1)
					AcmCtrl = AcmCtrl | 0x1;

				if (acm_ctrl & BIT(3))
					AcmCtrl |= AcmHw_VoqEn;
				else
					AcmCtrl &= (~AcmHw_VoqEn);

				if (acm_ctrl & BIT(2))
					AcmCtrl |= AcmHw_ViqEn;
				else
					AcmCtrl &= (~AcmHw_ViqEn);

				if (acm_ctrl & BIT(1))
					AcmCtrl |= AcmHw_BeqEn;
				else
					AcmCtrl &= (~AcmHw_BeqEn);

				DBG_8192C("[HW_VAR_ACM_CTRL] Write 0x%X\n", AcmCtrl);
				rtw_write8(padapter, REG_ACMHWCTRL, AcmCtrl );
			}
			break;

		case HW_VAR_AMPDU_MIN_SPACE:
			pHalData->AMPDUDensity = *(u8*)pval;
			break;

		case HW_VAR_AMPDU_FACTOR:
			{
				u32	AMPDULen = *(u8*)pval;

				if (IS_HARDWARE_TYPE_8812(padapter))
				{
					if (AMPDULen < VHT_AGG_SIZE_128K)
						AMPDULen = (0x2000 << *(u8*)pval) - 1;
					else
						AMPDULen = 0x1ffff;
				}
				else if(IS_HARDWARE_TYPE_8821(padapter))
				{
					if (AMPDULen < HT_AGG_SIZE_64K)
						AMPDULen = (0x2000 << *(u8*)pval) - 1;
					else
						AMPDULen = 0xffff;
				}
				AMPDULen |= BIT(31);
				rtw_write32(padapter, REG_AMPDU_MAX_LENGTH_8812, AMPDULen);
			}
			break;
#if 0
		case HW_VAR_RXDMA_AGG_PG_TH:
			rtw_write8(padapter, REG_RXDMA_AGG_PG_TH, *pval);
			break;
#endif
		case HW_VAR_H2C_FW_PWRMODE:
			{
				u8 psmode = *pval;

				// Forece leave RF low power mode for 1T1R to prevent conficting setting in Fw power
				// saving sequence. 2010.06.07. Added by tynli. Suggested by SD3 yschang.
				if ((psmode != PS_MODE_ACTIVE) && (!IS_92C_SERIAL(pHalData->VersionID)))
				{
					ODM_RF_Saving(podmpriv, _TRUE);
				}
				rtl8812_set_FwPwrMode_cmd(padapter, psmode);
			}
			break;

		case HW_VAR_H2C_FW_JOINBSSRPT:
			rtl8812_set_FwJoinBssReport_cmd(padapter, *pval);
			break;

#ifdef CONFIG_P2P_PS
		case HW_VAR_H2C_FW_P2P_PS_OFFLOAD:
			rtl8812_set_p2p_ps_offload_cmd(padapter, *pval);
			break;
#endif // CONFIG_P2P_PS

#ifdef CONFIG_TDLS
		case HW_VAR_TDLS_WRCR:
			val32 = rtw_read32(padapter, REG_RCR);
			val32 &= ~RCR_CBSSID_DATA;
			rtw_write32(padapter, REG_RCR, val32);
			break;

		case HW_VAR_TDLS_INIT_CH_SEN:
			val32 = rtw_read32(padapter, REG_RCR);
			val32 &= (~RCR_CBSSID_DATA) & (~RCR_CBSSID_BCN);
			rtw_write32(padapter, REG_RCR, val32);
			rtw_write16(padapter, REG_RXFLTMAP2, 0xffff);

			// disable update TSF
			val8 = rtw_read8(padapter, REG_BCN_CTRL);
			val8 |= BIT(4);
			rtw_write8(padapter, REG_BCN_CTRL, val8);
			break;

		case HW_VAR_TDLS_DONE_CH_SEN:
			// enable update TSF
			val8 = rtw_read8(padapter, REG_BCN_CTRL);
			val8 &= ~BIT(4);
			rtw_write8(padapter, REG_BCN_CTRL, val8);

			val32 = rtw_read32(padapter, REG_RCR);
			val32 |= RCR_CBSSID_BCN;
			rtw_write32(padapter, REG_RCR, val32);
			break;

		case HW_VAR_TDLS_RS_RCR:
			val32 = rtw_read32(padapter, REG_RCR);
			val32 |= RCR_CBSSID_DATA;
			rtw_write32(padapter, REG_RCR, val32);
			break;
#endif // CONFIG_TDLS

		case HW_VAR_INITIAL_GAIN:
			{
				pDIG_T pDigTable = &podmpriv->DM_DigTable;
				u32 rx_gain = *(u32*)pval;

				if (rx_gain == 0xff) {//restore rx gain
					ODM_Write_DIG(podmpriv, pDigTable->BackupIGValue);
				} else {
					pDigTable->BackupIGValue = pDigTable->CurIGValue;
					ODM_Write_DIG(podmpriv, rx_gain);
				}
			}
			break;

#ifdef CONFIG_BT_COEXIST
		case HW_VAR_BT_SET_COEXIST:
			rtl8812_set_dm_bt_coexist(padapter, *pval);
			break;

		case HW_VAR_BT_ISSUE_DELBA:
			rtl8812_issue_delete_ba(padapter, *pval);
			break;
#endif

#if (RATE_ADAPTIVE_SUPPORT==1)
		case HW_VAR_RPT_TIMER_SETTING:
			{
				val16 = *(u16*)pval;
				ODM_RA_Set_TxRPT_Time(podmpriv, val16);
			}
			break;
#endif

#ifdef CONFIG_SW_ANTENNA_DIVERSITY
		case HW_VAR_ANTENNA_DIVERSITY_LINK:
			//SwAntDivRestAfterLink8192C(padapter);
			ODM_SwAntDivRestAfterLink(podmpriv);
			break;

		case HW_VAR_ANTENNA_DIVERSITY_SELECT:
			{
				u8 Optimum_antenna = *pval;
				u8 	Ant;

				//switch antenna to Optimum_antenna
				//DBG_8192C("==> HW_VAR_ANTENNA_DIVERSITY_SELECT , Ant_(%s)\n",(Optimum_antenna==2)?"A":"B");
				if (pHalData->CurAntenna != Optimum_antenna)
				{
					Ant = (Optimum_antenna==2) ? MAIN_ANT : AUX_ANT;
					ODM_UpdateRxIdleAnt_88E(podmpriv, Ant);

					pHalData->CurAntenna = Optimum_antenna;
					//DBG_8192C("==> HW_VAR_ANTENNA_DIVERSITY_SELECT , Ant_(%s)\n",(Optimum_antenna==2)?"A":"B");
				}
			}
			break;
#endif

		case HW_VAR_EFUSE_USAGE:
			pHalData->EfuseUsedPercentage = *pval;
			break;

		case HW_VAR_EFUSE_BYTES:
			pHalData->EfuseUsedBytes = *(u16*)pval;
			break;
#if 0
		case HW_VAR_EFUSE_BT_USAGE:
#ifdef HAL_EFUSE_MEMORY
			pHalData->EfuseHal.BTEfuseUsedPercentage = *pval;
#endif
			break;

		case HW_VAR_EFUSE_BT_BYTES:
#ifdef HAL_EFUSE_MEMORY
			pHalData->EfuseHal.BTEfuseUsedBytes = *(u16*)pval;
#else
			BTEfuseUsedBytes = *(u16*)pval;
#endif
			break;
#endif
		case HW_VAR_FIFO_CLEARN_UP:
			{
				struct pwrctrl_priv *pwrpriv;
				u8 trycnt = 100;	

				pwrpriv = &padapter->pwrctrlpriv;

				// pause tx
				rtw_write8(padapter, REG_TXPAUSE, 0xff);

				// keep sn
				padapter->xmitpriv.nqos_ssn = rtw_read16(padapter, REG_NQOS_SEQ);

				if (pwrpriv->bkeepfwalive != _TRUE)
				{
					// RX DMA stop
					val32 = rtw_read32(padapter, REG_RXPKT_NUM);
					val32 |= RW_RELEASE_EN;
					rtw_write32(padapter, REG_RXPKT_NUM, val32);
					do {
						val32 = rtw_read32(padapter, REG_RXPKT_NUM);
						val32 &= RXDMA_IDLE;
						if (!val32)
							break;
					} while (trycnt--);
					if (trycnt == 0)
					{
						DBG_8192C("[HW_VAR_FIFO_CLEARN_UP] Stop RX DMA failed......\n");
					}

					//RQPN Load 0
					rtw_write16(padapter, REG_RQPN_NPQ, 0x0);
					rtw_write32(padapter, REG_RQPN, 0x80000000);
					rtw_mdelay_os(10);
				}
			}
			break;

#ifdef CONFIG_CONCURRENT_MODE
		case HW_VAR_CHECK_TXBUF:
			{
				u32 i;
				u8 RetryLimit;
				u32 reg_200, reg_204;

				RetryLimit = 0x01;

				val16 = RetryLimit << RETRY_LIMIT_SHORT_SHIFT | RetryLimit << RETRY_LIMIT_LONG_SHIFT;
				rtw_write16(padapter, REG_RL, val16);

				for (i=0; i<1000; i++)
				{
					reg_200 = rtw_read32(padapter, 0x200);
					reg_204 = rtw_read32(padapter, 0x204);
					if (reg_200 != reg_204)
					{
//						DBG_8192C("%s: (HW_VAR_CHECK_TXBUF)Tx buffer NOT empty - 0x204=0x%x, 0x200=0x%x (%d)\n", __FUNCTION__, reg_204, reg_200, i);
						rtw_msleep_os(10);
					}
					else
					{
						DBG_8192C("%s: (HW_VAR_CHECK_TXBUF)Tx buffer Empty(%d)\n", __FUNCTION__, i);
						break;
					}
				}
				if (i == 1000)
				{
					DBG_8192C("%s: (HW_VAR_CHECK_TXBUF)TXBUF is still not Empty after %d times check!\n", __FUNCTION__, i);
					DBG_8192C("%s: (HW_VAR_CHECK_TXBUF)0x204=0x%08x, 0x200=0x%08x\n", __FUNCTION__, reg_204, reg_200);
				}

				RetryLimit = 0x30;
				val16 = RetryLimit << RETRY_LIMIT_SHORT_SHIFT | RetryLimit << RETRY_LIMIT_LONG_SHIFT;
				rtw_write16(padapter, REG_RL, val16);
			}
			break;
#endif

#if (RATE_ADAPTIVE_SUPPORT == 1)
		case HW_VAR_TX_RPT_MAX_MACID:
			{
				u8 maxMacid = *pval;
				DBG_8192C("### MacID(%d),Set Max Tx RPT MID(%d)\n", maxMacid, maxMacid+1);
				rtw_write8(padapter, REG_TX_RPT_CTRL+1, maxMacid+1);
			}
			break;
#endif

		case HW_VAR_H2C_MEDIA_STATUS_RPT:
			{
				struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
				RT_MEDIA_STATUS	mstatus = *(u16*)pval & 0xFF;

				rtl8812_set_FwMediaStatus_cmd(padapter, *(u16*)pval);

				if (check_fwstate(pmlmepriv, WIFI_STATION_STATE))
					Hal_PatchwithJaguar_8812(padapter, mstatus);
			}
			break;

		case HW_VAR_APFM_ON_MAC:
			pHalData->bMacPwrCtrlOn = *pval;
			DBG_8192C("%s: bMacPwrCtrlOn=%d\n", __FUNCTION__, pHalData->bMacPwrCtrlOn);
			break;

		case HW_VAR_NAV_UPPER:
			{
				u32 usNavUpper = *((u32*)pval);

				if (usNavUpper > HAL_NAV_UPPER_UNIT * 0xFF)
				{
					DBG_8192C("%s: [HW_VAR_NAV_UPPER] set value(0x%08X us) is larger than (%d * 0xFF)!\n",
						__FUNCTION__, usNavUpper, HAL_NAV_UPPER_UNIT);
					break;
				}

				// The value of ((usNavUpper + HAL_NAV_UPPER_UNIT - 1) / HAL_NAV_UPPER_UNIT)
				// is getting the upper integer.
				usNavUpper = (usNavUpper + HAL_NAV_UPPER_UNIT - 1) / HAL_NAV_UPPER_UNIT;
				rtw_write8(padapter, REG_NAV_UPPER, (u8)usNavUpper);
			}
			break;

		case HW_VAR_BCN_VALID:
#ifdef CONFIG_CONCURRENT_MODE
			if (IS_HARDWARE_TYPE_8821(padapter) && padapter->iface_type == IFACE_PORT1)
			{
				val8 = rtw_read8(padapter, REG_TDECTRL1_8812+2);
				val8 |= BIT(0);
				rtw_write8(padapter, REG_TDECTRL1_8812+2, val8); 
			}
			else
#endif
			{
				// BCN_VALID, BIT16 of REG_TDECTRL = BIT0 of REG_TDECTRL+2, write 1 to clear, Clear by sw
				val8 = rtw_read8(padapter, REG_TDECTRL+2);
				val8 |= BIT(0);
				rtw_write8(padapter, REG_TDECTRL+2, val8);
			}
			break;

		case HW_VAR_DL_BCN_SEL:
#ifdef CONFIG_CONCURRENT_MODE
			if (IS_HARDWARE_TYPE_8821(padapter) && padapter->iface_type == IFACE_PORT1)
			{
				// SW_BCN_SEL - Port1
				val8 = rtw_read8(padapter, REG_TDECTRL1_8812+2);
				val8 |= BIT(4);
				rtw_write8(padapter, REG_TDECTRL1_8812+2, val8);
			}
			else
#endif
			{
				// SW_BCN_SEL - Port0
				val8 = rtw_read8(padapter, REG_TDECTRL1_8812+2);
				val8 &= ~BIT(4);
				rtw_write8(padapter, REG_TDECTRL1_8812+2, val8);	
			}
			break;

		case HW_VAR_WIRELESS_MODE:
			{
				u8	R2T_SIFS = 0, SIFS_Timer = 0;
				u8	wireless_mode = *pval;

				if ((wireless_mode == WIRELESS_11BG) || (wireless_mode == WIRELESS_11G))
					SIFS_Timer = 0xa;
				else
					SIFS_Timer = 0xe;

				// SIFS for OFDM Data ACK
				rtw_write8(padapter, REG_SIFS_CTX+1, SIFS_Timer);
				// SIFS for OFDM consecutive tx like CTS data!
				rtw_write8(padapter, REG_SIFS_TRX+1, SIFS_Timer);
				
				rtw_write8(padapter,REG_SPEC_SIFS+1, SIFS_Timer);
				rtw_write8(padapter,REG_MAC_SPEC_SIFS+1, SIFS_Timer);

				// 20100719 Joseph: Revise SIFS setting due to Hardware register definition change.
				rtw_write8(padapter, REG_RESP_SIFS_OFDM+1, SIFS_Timer);
				rtw_write8(padapter, REG_RESP_SIFS_OFDM, SIFS_Timer);

				//
				// Adjust R2T SIFS for IOT issue. Add by hpfan 2013.01.25
				// Set R2T SIFS to 0x0a for Atheros IOT. Add by hpfan 2013.02.22
				//
				// Mac has 10 us delay so use 0xa value is enough. 
				R2T_SIFS = 0xa;

				rtw_write8(padapter, REG_RESP_SIFS_OFDM+1, R2T_SIFS);
			}
			break;

		default:
			DBG_8192C("%s: [WARNNING] variable(%d) not defined!\n", __FUNCTION__, variable);
			break;
	}

_func_exit_;
}

void GetHwReg8812A(PADAPTER padapter, u8 variable, u8 *pval)
{
	PHAL_DATA_TYPE pHalData;
	PDM_ODM_T podmpriv;
	u8 val8;
	u16 val16;
	//u32 val32;

_func_enter_;

	pHalData = GET_HAL_DATA(padapter);
	podmpriv = &pHalData->odmpriv;

	switch (variable)
	{
		case HW_VAR_BASIC_RATE:
			*(u16*)pval = pHalData->BasicRateSet;
			break;

		case HW_VAR_TXPAUSE:
			*pval = rtw_read8(padapter, REG_TXPAUSE);
			break;

		case HW_VAR_BCN_VALID:
#ifdef CONFIG_CONCURRENT_MODE
			if (IS_HARDWARE_TYPE_8821(padapter) && padapter->iface_type == IFACE_PORT1)
			{
				val8 = rtw_read8(padapter, REG_TDECTRL1_8812+2);
				*pval = (BIT(0) & val8) ? _TRUE:_FALSE;
			}
			else
#endif
			{
				//BCN_VALID, BIT16 of REG_TDECTRL = BIT0 of REG_TDECTRL+2
				val8 = rtw_read8(padapter, REG_TDECTRL+2);
				*pval = (BIT(0) & val8) ? _TRUE:_FALSE;
			}
			break;

		case HW_VAR_DM_FLAG:
			*pval = podmpriv->SupportAbility;
			break;

		case HW_VAR_RF_TYPE:
			*pval = pHalData->rf_type;
			break;

		case HW_VAR_FWLPS_RF_ON:
			//When we halt NIC, we should check if FW LPS is leave.
			if(padapter->pwrctrlpriv.rf_pwrstate == rf_off)
			{
				// If it is in HW/SW Radio OFF or IPS state, we do not check Fw LPS Leave,
				// because Fw is unload.
				*pval = _TRUE;
			}
			else
			{
				u32 valRCR;
				valRCR = rtw_read32(padapter, REG_RCR);
				valRCR &= 0x00070000;
				if(valRCR)
					*pval = _FALSE;
				else
					*pval = _TRUE;
			}
			
			break;

#ifdef CONFIG_ANTENNA_DIVERSITY
		case HW_VAR_CURRENT_ANTENNA:
			*pval = pHalData->CurAntenna;
			break;
#endif

		case HW_VAR_EFUSE_BYTES: // To get EFUE total used bytes, added by Roger, 2008.12.22.
			*(u16*)pval = pHalData->EfuseUsedBytes;	
			break;

		case HW_VAR_APFM_ON_MAC:
			*pval = pHalData->bMacPwrCtrlOn;
			break;

		case HW_VAR_CHK_HI_QUEUE_EMPTY:
			val16 = rtw_read16(padapter, REG_TXPKT_EMPTY);
			*pval = (val16 & BIT(10)) ? _TRUE:_FALSE;
			break;

		default:
			DBG_8192C("%s: [WARNNING] variable(%d) not defined!\n", __FUNCTION__, variable);
			break;
	}

_func_exit_;
}

/*
 *	Description:
 *		Change default setting of specified variable.
 */
u8 SetHalDefVar8812A(PADAPTER padapter, HAL_DEF_VARIABLE variable, void *pval)
{
	PHAL_DATA_TYPE pHalData;
	u8 bResult;


	pHalData = GET_HAL_DATA(padapter);
	bResult = _SUCCESS;

	switch (variable)
	{
		case HAL_DEF_DBG_DM_FUNC:
			{
				u8 dm_func;
				struct dm_priv *pdmpriv;
				DM_ODM_T *podmpriv;


				dm_func = *((u8*)pval);
				pdmpriv = &pHalData->dmpriv;
				podmpriv = &pHalData->odmpriv;

				if (dm_func == 0)
				{
					//disable all dynamic func
					podmpriv->SupportAbility = DYNAMIC_FUNC_DISABLE;
					DBG_8192C("==> Disable all dynamic function...\n");
				}
				else if (dm_func == 1)
				{
					// disable DIG
					podmpriv->SupportAbility &= (~DYNAMIC_BB_DIG);
					DBG_8192C("==> Disable DIG...\n");
				}
				else if (dm_func == 2)
				{
					// disable High power
					podmpriv->SupportAbility &= (~DYNAMIC_BB_DYNAMIC_TXPWR);
				}
				else if (dm_func == 3)
				{
					// disable tx power tracking
					podmpriv->SupportAbility &= (~DYNAMIC_RF_CALIBRATION);
					DBG_8192C("==> Disable tx power tracking...\n");
				}
//				else if (dm_func == 4)
//				{
					// disable BT coexistence
//					pdmpriv->DMFlag &= (~DYNAMIC_FUNC_BT);
//				}
				else if (dm_func == 5)
				{
					// disable antenna diversity
					podmpriv->SupportAbility &= (~DYNAMIC_BB_ANT_DIV);
				}
				else if (dm_func == 6)
				{
					// turn on all dynamic func
					if (!(podmpriv->SupportAbility & DYNAMIC_BB_DIG))
					{
						pDIG_T pDigTable = &podmpriv->DM_DigTable;
						pDigTable->CurIGValue = rtw_read8(padapter, 0xc50);
					}
//					pdmpriv->DMFlag |= DYNAMIC_FUNC_BT;
					podmpriv->SupportAbility = DYNAMIC_ALL_FUNC_ENABLE;
					DBG_8192C("==> Turn on all dynamic function...\n");
				}
			}
			break;

		case HAL_DEF_DBG_DUMP_RXPKT:
			pHalData->bDumpRxPkt = *(u8*)pval;
			break;

		case HAL_DEF_DBG_DUMP_TXPKT:
			pHalData->bDumpTxPkt = *(u8*)pval;
			break;				

		case HW_DEF_FA_CNT_DUMP:
			{
				u8 mac_id;
				PDM_ODM_T pDM_Odm;


				mac_id = *(u8*)pval;
				pDM_Odm = &pHalData->odmpriv;

				if (padapter->bLinkInfoDump & BIT(1))
					pDM_Odm->DebugComponents |=	ODM_COMP_DIG;
				else
					pDM_Odm->DebugComponents &= ~ODM_COMP_DIG;

		 		if (padapter->bLinkInfoDump & BIT(2))
					pDM_Odm->DebugComponents |=	ODM_COMP_FA_CNT;
				else
					pDM_Odm->DebugComponents &= ~ODM_COMP_FA_CNT;
			}
			break;

		case HW_DEF_ODM_DBG_FLAG:
			{
				u64 DebugComponents;
				PDM_ODM_T pDM_Odm;


				DebugComponents = *((u64*)pval);
				pDM_Odm = &pHalData->odmpriv;
				pDM_Odm->DebugComponents = DebugComponents;
			}
			break;

		default:
			DBG_8192C("%s: [ERROR] HAL_DEF_VARIABLE(%d) not defined!\n", __FUNCTION__, variable);
			bResult = _FAIL;
			break;
	}

	return bResult;
}

/*
 *	Description: 
 *		Query setting of specified variable.
 */
u8 GetHalDefVar8812A(PADAPTER padapter, HAL_DEF_VARIABLE variable, void *pval)
{
	PHAL_DATA_TYPE pHalData;
	u8 bResult;


	pHalData = GET_HAL_DATA(padapter);
	bResult = _SUCCESS;

	switch (variable)
	{
		case HAL_DEF_UNDERCORATEDSMOOTHEDPWDB:
			{
				struct mlme_priv *pmlmepriv;
				struct sta_priv *pstapriv;
				struct sta_info *psta;

				pmlmepriv = &padapter->mlmepriv;
				pstapriv = &padapter->stapriv;
				psta = rtw_get_stainfo(pstapriv, pmlmepriv->cur_network.network.MacAddress);
				if (psta)
				{
					*((int*)pval) = psta->rssi_stat.UndecoratedSmoothedPWDB;     
				}
			}
			break;

#ifdef CONFIG_ANTENNA_DIVERSITY
		case HAL_DEF_IS_SUPPORT_ANT_DIV:
			*((u8*)pval) = (pHalData->AntDivCfg==0) ? _FALSE : _TRUE;
			break;
#endif

#ifdef CONFIG_ANTENNA_DIVERSITY
		case HAL_DEF_CURRENT_ANTENNA:
			*((u8*)pval) = pHalData->CurAntenna;
			break;
#endif

		case HAL_DEF_DRVINFO_SZ:
			*((u32*)pval) = DRVINFO_SZ;
			break;

		case HAL_DEF_MAX_RECVBUF_SZ:
			*((u32*)pval) = MAX_RECVBUF_SZ;
			break;

		case HAL_DEF_RX_PACKET_OFFSET:
			*((u32*)pval) = RXDESC_SIZE + DRVINFO_SZ;
			break;

		case HAL_DEF_DBG_DM_FUNC:
			*((u32*)pval) = pHalData->odmpriv.SupportAbility;
			break;

#if (RATE_ADAPTIVE_SUPPORT == 1)
		case HAL_DEF_RA_DECISION_RATE:
			{
				u8 MacID = *(u8*)pval;
				*((u8*)pval) = ODM_RA_GetDecisionRate_8812A(&pHalData->odmpriv, MacID);
			}
			break;

		case HAL_DEF_RA_SGI:
			{
				u8 MacID = *(u8*)pval;
				*((u8*)pval) = ODM_RA_GetShortGI_8812A(&pHalData->odmpriv, MacID);
			}
			break;
#endif // (RATE_ADAPTIVE_SUPPORT == 1)

#if (POWER_TRAINING_ACTIVE == 1)
		case HAL_DEF_PT_PWR_STATUS:
			{
				u8 MacID = *(u8*)pval;
				*(u8*)pval = ODM_RA_GetHwPwrStatus_8812A(&pHalData->odmpriv, MacID);
			}
			break;
#endif //(POWER_TRAINING_ACTIVE == 1)

		case HW_VAR_MAX_RX_AMPDU_FACTOR:
			*((u32*)pval) = MAX_AMPDU_FACTOR_64K;
			break;

		case HAL_DEF_LDPC:
			if (IS_VENDOR_8812A_C_CUT(padapter))
				*(u8*)pval = _TRUE;
			else if (IS_HARDWARE_TYPE_8821(padapter))
				*(u8*)pval = _FALSE;
			else
				*(u8*)pval = _FALSE;
#if (MP_DRIVER == 1) 
			if (padapter->registrypriv.mp_mode == 1)
				*(u8*)pval = _TRUE;
#endif
			break;

		case HAL_DEF_TX_STBC:
			if (pHalData->rf_type == RF_2T2R)
				*(u8*)pval = 1;
			else
				*(u8*)pval = 0;
			break;

		case HAL_DEF_RX_STBC:
			*(u8*)pval = 1;
			break;

		case HW_DEF_RA_INFO_DUMP:
			{
				u8 mac_id = *(u8*)pval;
				u32 cmd = 0x40000400 | mac_id;
				u32 ra_info1, ra_info2;
				u32 rate_mask1, rate_mask2;

				if ((padapter->bLinkInfoDump & BIT(0))
					&& (check_fwstate(&padapter->mlmepriv, _FW_LINKED) == _TRUE))
				{
					DBG_8192C("============ RA status check  Mac_id:%d ===================\n", mac_id);

					rtw_write32(padapter, REG_HMEBOX_E2_E3_8812,cmd);
					ra_info1 = rtw_read32(padapter, REG_RSVD5_8812);
					ra_info2 = rtw_read32(padapter, REG_RSVD6_8812);
					rate_mask1 = rtw_read32(padapter,REG_RSVD7_8812);
					rate_mask2 = rtw_read32(padapter,REG_RSVD8_8812);

					DBG_8192C("[ ra_info1:0x%08x ] =>RSSI=%d, BW_setting=0x%02x, DISRA=0x%02x, VHT_EN=0x%02x\n",
						ra_info1,
						ra_info1&0xFF,
						(ra_info1>>8)  & 0xFF,
						(ra_info1>>16) & 0xFF,
						(ra_info1>>24) & 0xFF);
					
					DBG_8192C("[ ra_info2:0x%08x ] =>hight_rate=0x%02x, lowest_rate=0x%02x, SGI=0x%02x, RateID=%d\n",
						ra_info2,
						ra_info2&0xFF,
						(ra_info2>>8)  & 0xFF,
						(ra_info2>>16) & 0xFF,
						(ra_info2>>24) & 0xFF);

					DBG_8192C("rate_mask2=0x%08x, rate_mask1=0x%08x\n", rate_mask2, rate_mask1);
				}
			}
			break;

		case HAL_DEF_DBG_DUMP_RXPKT:
			*(u8*)pval = pHalData->bDumpRxPkt;
			break;

		case HAL_DEF_DBG_DUMP_TXPKT:
			*(u8*)pval = pHalData->bDumpTxPkt;
			break;

		case HW_DEF_ODM_DBG_FLAG:
			{
				u64	DebugComponents;
				PDM_ODM_T pDM_Odm;

				pDM_Odm = &pHalData->odmpriv;
				DebugComponents = pDM_Odm->DebugComponents;
				DBG_8192C("%s: pDM_Odm->DebugComponents=0x%llx\n", __FUNCTION__, DebugComponents);
				*((u64*)pval) = DebugComponents;
			}
			break;

		case HAL_DEF_TX_PAGE_BOUNDARY:
			if (!padapter->registrypriv.wifi_spec)
			{
				if (IS_HARDWARE_TYPE_8812(padapter))
					*(u8*)pval = TX_PAGE_BOUNDARY_8812;
				else
					*(u8*)pval = TX_PAGE_BOUNDARY_8821;
			}
			else
			{
				if (IS_HARDWARE_TYPE_8812(padapter))
					*(u8*)pval = WMM_NORMAL_TX_PAGE_BOUNDARY_8812;
				else
					*(u8*)pval = WMM_NORMAL_TX_PAGE_BOUNDARY_8821;
			}
			break;

		case HAL_DEF_TX_PAGE_BOUNDARY_WOWLAN:
			*(u8*)pval = TX_PAGE_BOUNDARY_WOWLAN_8812;
			break;

		default:
			DBG_8192C("%s: [ERROR] HAL_DEF_VARIABLE(%d) not defined!\n", __FUNCTION__, variable);
			bResult = _FAIL;
			break;
	}

	return bResult;
}

void rtl8812_set_hal_ops(struct hal_ops *pHalFunc)
{
	pHalFunc->free_hal_data = &rtl8812_free_hal_data;

	pHalFunc->dm_init = &rtl8812_init_dm_priv;
	pHalFunc->dm_deinit = &rtl8812_deinit_dm_priv;

	pHalFunc->UpdateRAMaskHandler = &UpdateHalRAMask8812A;

	pHalFunc->read_chip_version = &ReadChipVersion8812A;

	pHalFunc->set_bwmode_handler = &PHY_SetBWMode8812;
	pHalFunc->set_channel_handler = &PHY_SwChnl8812;
	pHalFunc->set_chnl_bw_handler = &PHY_SetSwChnlBWMode8812;

	pHalFunc->hal_dm_watchdog = &rtl8812_HalDmWatchDog;

	pHalFunc->Add_RateATid = &rtl8812_Add_RateATid;
#ifdef CONFIG_CONCURRENT_MODE		
	pHalFunc->clone_haldata = &rtl8812_clone_haldata;
#endif
	pHalFunc->run_thread= &rtl8812_start_thread;
	pHalFunc->cancel_thread= &rtl8812_stop_thread;

#ifdef CONFIG_ANTENNA_DIVERSITY
	pHalFunc->AntDivBeforeLinkHandler = &AntDivBeforeLink8812;
	pHalFunc->AntDivCompareHandler = &AntDivCompare8812;
#endif

	pHalFunc->read_bbreg = &PHY_QueryBBReg8812;
	pHalFunc->write_bbreg = &PHY_SetBBReg8812;
	pHalFunc->read_rfreg = &PHY_QueryRFReg8812;
	pHalFunc->write_rfreg = &PHY_SetRFReg8812;


	// Efuse related function
	pHalFunc->EfusePowerSwitch = &rtl8812_EfusePowerSwitch;
	pHalFunc->ReadEFuse = &rtl8812_ReadEFuse;
	pHalFunc->EFUSEGetEfuseDefinition = &rtl8812_EFUSE_GetEfuseDefinition;
	pHalFunc->EfuseGetCurrentSize = &rtl8812_EfuseGetCurrentSize;
	pHalFunc->Efuse_PgPacketRead = &rtl8812_Efuse_PgPacketRead;
	pHalFunc->Efuse_PgPacketWrite = &rtl8812_Efuse_PgPacketWrite;
	pHalFunc->Efuse_WordEnableDataWrite = &rtl8812_Efuse_WordEnableDataWrite;

#ifdef DBG_CONFIG_ERROR_DETECT
	pHalFunc->sreset_init_value = &sreset_init_value;
	pHalFunc->sreset_reset_value = &sreset_reset_value;
	pHalFunc->silentreset = &sreset_reset;
	pHalFunc->sreset_xmit_status_check = &rtl8812_sreset_xmit_status_check;
	pHalFunc->sreset_linked_status_check  = &rtl8812_sreset_linked_status_check;
	pHalFunc->sreset_get_wifi_status  = &sreset_get_wifi_status;
	pHalFunc->sreset_inprogress= &sreset_inprogress;
#endif //DBG_CONFIG_ERROR_DETECT

	pHalFunc->GetHalODMVarHandler = &rtl8812_GetHalODMVar;
	pHalFunc->SetHalODMVarHandler = &rtl8812_SetHalODMVar;
	pHalFunc->hal_notch_filter = &hal_notch_filter_8812;

	pHalFunc->SetBeaconRelatedRegistersHandler = &SetBeaconRelatedRegisters8812A;
}


