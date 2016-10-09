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

#include "Mp_Precomp.h"
#include "../phydm_precomp.h"



/*---------------------------Define Local Constant---------------------------*/
#define cal_num_8821A 3
#define MACBB_REG_NUM_8821A 8
#define AFE_REG_NUM_8821A 4
#define RF_REG_NUM_8821A 3
/*---------------------------Define Local Constant---------------------------*/
#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
void DoIQK_8821A(
    PDM_ODM_T	pDM_Odm,
    u1Byte 		DeltaThermalIndex,
    u1Byte		ThermalValue,
    u1Byte 		Threshold
)
{
	pDM_Odm->RFCalibrateInfo.ThermalValue_IQK= ThermalValue;
	PHY_IQCalibrate_8821A(pDM_Odm, FALSE);
}
#endif
void _IQK_RX_FillIQC_8821A(
    IN PDM_ODM_T			pDM_Odm,
    IN ODM_RF_RADIO_PATH_E 	Path,
    IN unsigned int			RX_X,
    IN unsigned int			RX_Y
)
{
	switch (Path) {
	case ODM_RF_PATH_A: {
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
		ODM_SetBBReg(pDM_Odm, 0xc10, 0x000003ff, RX_X>>1);
		ODM_SetBBReg(pDM_Odm, 0xc10, 0x03ff0000, (RX_Y>>1) & 0x000003ff);
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X = %x;;RX_Y = %x ====>fill to IQC\n", RX_X>>1, RX_Y>>1));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xc10 = %x ====>fill to IQC\n", ODM_Read4Byte(pDM_Odm, 0xc10)));
	}
	break;
	default:
		break;
	};
}

void _IQK_TX_FillIQC_8821A(
    IN PDM_ODM_T			pDM_Odm,
    IN ODM_RF_RADIO_PATH_E 	Path,
    IN unsigned int			TX_X,
    IN unsigned int			TX_Y
)
{
	switch (Path) {
	case ODM_RF_PATH_A: {
		ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); // [31] = 1 --> Page C1
		ODM_Write4Byte(pDM_Odm, 0xc90, 0x00000080);
		ODM_Write4Byte(pDM_Odm, 0xcc4, 0x20040000);
		ODM_Write4Byte(pDM_Odm, 0xcc8, 0x20000000);
		ODM_SetBBReg(pDM_Odm, 0xccc, 0x000007ff, TX_Y);
		ODM_SetBBReg(pDM_Odm, 0xcd4, 0x000007ff, TX_X);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TX_X = %x;;TX_Y = %x =====> fill to IQC\n", TX_X, TX_Y));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xcd4 = %x;;0xccc = %x ====>fill to IQC\n", ODM_GetBBReg(pDM_Odm, 0xcd4, 0x000007ff), ODM_GetBBReg(pDM_Odm, 0xccc, 0x000007ff)));
	}
	break;
	default:
		break;
	};
}

void _IQK_BackupMacBB_8821A(
    IN PDM_ODM_T	pDM_Odm,
    IN pu4Byte		MACBB_backup,
    IN pu4Byte		Backup_MACBB_REG,
    IN u4Byte		MACBB_NUM
)
{
	u4Byte i;
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
	//save MACBB default value
	for (i = 0; i < MACBB_NUM; i++) {
		MACBB_backup[i] = ODM_Read4Byte(pDM_Odm, Backup_MACBB_REG[i]);
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupMacBB Success!!!!\n"));
}

void _IQK_BackupRF_8821A(
    IN PDM_ODM_T	pDM_Odm,
    IN pu4Byte		RFA_backup,
    IN pu4Byte		RFB_backup,
    IN pu4Byte		Backup_RF_REG,
    IN u4Byte		RF_NUM
)
{

	u4Byte i;
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
	//Save RF Parameters
	for (i = 0; i < RF_NUM; i++) {
		RFA_backup[i] = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, Backup_RF_REG[i], bMaskDWord);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupRF Success!!!!\n"));
}

void _IQK_BackupAFE_8821A(
    IN PDM_ODM_T		pDM_Odm,
    IN pu4Byte		AFE_backup,
    IN pu4Byte		Backup_AFE_REG,
    IN u4Byte		AFE_NUM
)
{
	u4Byte i;
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
	//Save AFE Parameters
	for (i = 0; i < AFE_NUM; i++) {
		AFE_backup[i] = ODM_Read4Byte(pDM_Odm, Backup_AFE_REG[i]);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BackupAFE Success!!!!\n"));
}

void _IQK_RestoreMacBB_8821A(
    IN PDM_ODM_T		pDM_Odm,
    IN pu4Byte		MACBB_backup,
    IN pu4Byte		Backup_MACBB_REG,
    IN u4Byte		MACBB_NUM
)
{
	u4Byte i;
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
	//Reload MacBB Parameters
	for (i = 0; i < MACBB_NUM; i++) {
		ODM_Write4Byte(pDM_Odm, Backup_MACBB_REG[i], MACBB_backup[i]);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreMacBB Success!!!!\n"));
}

void _IQK_RestoreRF_8821A(
    IN PDM_ODM_T			pDM_Odm,
    IN ODM_RF_RADIO_PATH_E 	Path,
    IN pu4Byte				Backup_RF_REG,
    IN pu4Byte 				RF_backup,
    IN u4Byte				RF_REG_NUM
)
{
	u4Byte i;

	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
	for (i = 0; i < RF_REG_NUM; i++)
		ODM_SetRFReg(pDM_Odm, Path, Backup_RF_REG[i], bRFRegOffsetMask, RF_backup[i]);

	switch(Path) {
	case ODM_RF_PATH_A: {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreRF Path A Success!!!!\n"));
	}
	break;
	default:
		break;
	}
}

void _IQK_RestoreAFE_8821A(
    IN PDM_ODM_T		pDM_Odm,
    IN pu4Byte		AFE_backup,
    IN pu4Byte		Backup_AFE_REG,
    IN u4Byte		AFE_NUM
)
{
	u4Byte i;
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
	//Reload AFE Parameters
	for (i = 0; i < AFE_NUM; i++) {
		ODM_Write4Byte(pDM_Odm, Backup_AFE_REG[i], AFE_backup[i]);
	}
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); // [31] = 1 --> Page C1
	ODM_Write4Byte(pDM_Odm, 0xc80, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xc84, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xc88, 0x0);
	ODM_Write4Byte(pDM_Odm, 0xc8c, 0x3c000000);
	ODM_Write4Byte(pDM_Odm, 0xc90, 0x00000080);
	ODM_Write4Byte(pDM_Odm, 0xc94, 0x00000000);
	ODM_Write4Byte(pDM_Odm, 0xcc4, 0x20040000);
	ODM_Write4Byte(pDM_Odm, 0xcc8, 0x20000000);
	ODM_Write4Byte(pDM_Odm, 0xcb8, 0x0);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RestoreAFE Success!!!!\n"));
}

void _IQK_ConfigureMAC_8821A(
    IN PDM_ODM_T		pDM_Odm
)
{
	// ========MAC register setting========
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
	ODM_Write1Byte(pDM_Odm, 0x522, 0x3f);
	ODM_SetBBReg(pDM_Odm, 0x550, BIT(11)|BIT(3), 0x0);
	ODM_Write1Byte(pDM_Odm, 0x808, 0x00);		//		RX ante off
	ODM_SetBBReg(pDM_Odm, 0x838, 0xf, 0xc);		//		CCA off
	ODM_Write1Byte(pDM_Odm, 0xa07, 0xf);		//  		CCK RX Path off
}

void _IQK_Tx_8821A(
    IN PDM_ODM_T		pDM_Odm,
    IN ODM_RF_RADIO_PATH_E Path
)
{
	u4Byte 		TX_fail, RX_fail, delay_count, IQK_ready, cal_retry, cal = 0;
	int 		TX_X = 0, TX_Y = 0, RX_X = 0, RX_Y = 0, TX_Average = 0, RX_Average = 0, RXIQK_Loop = 0, RX_X_temp = 0, RX_Y_temp = 0;
	int 		TX_X0[cal_num_8821A], TX_Y0[cal_num_8821A], RX_X0[2][cal_num_8821A], RX_Y0[2][cal_num_8821A];
	BOOLEAN 	TX0IQKOK = FALSE, RX0IQKOK = FALSE;
	BOOLEAN  	VDF_enable = FALSE;
	int 			i, k, VDF_Y[3], VDF_X[3], Tx_dt[3], ii, dx = 0, dy = 0, TX_finish = 0, RX_finish1 = 0, RX_finish2 = 0;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BandWidth = %d, SupportInterface = %d, ExtPA = %d, ExtPA5G = %d\n", *pDM_Odm->pBandWidth, pDM_Odm->SupportInterface, pDM_Odm->ExtPA, pDM_Odm->ExtPA5G));
	if (*pDM_Odm->pBandWidth == 2) {
		VDF_enable = TRUE;
	}

	while (cal < cal_num_8821A) {
		switch (Path) {
		case ODM_RF_PATH_A: {
			//Path-A LOK
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
			// ========Path-A AFE all on========
			// Port 0 DAC/ADC on
			ODM_Write4Byte(pDM_Odm, 0xc60, 0x77777777);
			ODM_Write4Byte(pDM_Odm, 0xc64, 0x77777777);

			ODM_Write4Byte(pDM_Odm, 0xc68, 0x19791979);

			ODM_SetBBReg(pDM_Odm, 0xc00, 0xf, 0x4);// 	hardware 3-wire off

			// LOK Setting
			//====== LOK ======
			// 1. DAC/ADC sampling rate (160 MHz)
			ODM_SetBBReg(pDM_Odm, 0xc5c, BIT(26)|BIT(25)|BIT(24), 0x7);

			// 2. LoK RF Setting (at BW = 20M)
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80002);
			ODM_SetRFReg(pDM_Odm, Path, 0x18, 0x00c00, 0x3);     // BW 20M
			ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x20000);
			ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x0003f);
			ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xf3fc3);
			ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d5);
			ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
			ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
			ODM_SetBBReg(pDM_Odm, 0xc94, BIT(0), 0x1);
			ODM_Write4Byte(pDM_Odm, 0x978, 0x29002000);// TX (X,Y)
			ODM_Write4Byte(pDM_Odm, 0x97c, 0xa9002000);// RX (X,Y)
			ODM_Write4Byte(pDM_Odm, 0x984, 0x00462910);// [0]:AGC_en, [15]:idac_K_Mask

			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); // [31] = 1 --> Page C1

			if (pDM_Odm->ExtPA5G)
				ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403f7);
			else
				ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403f4);

			if (*pDM_Odm->pBandType)
				ODM_Write4Byte(pDM_Odm, 0xc8c, 0x68163e96);
			else
				ODM_Write4Byte(pDM_Odm, 0xc8c, 0x28163e96);

			ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c10);// TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16
			ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c10);// RX_Tone_idx[9:0], RxK_Mask[29]
			ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);// cb8[20] 將 SI/PI 使用權切給 iqk_dpk module
			ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
			ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

			ODM_delay_ms(10); //Delay 10ms
			ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);

			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
			ODM_SetRFReg(pDM_Odm, Path, 0x58, 0x7fe00, ODM_GetRFReg(pDM_Odm, Path, 0x8, 0xffc00)); // Load LOK
			switch (*pDM_Odm->pBandWidth) {
			case 1: {
				ODM_SetRFReg(pDM_Odm, Path, 0x18, 0x00c00, 0x1);
			}
			break;
			case 2: {
				ODM_SetRFReg(pDM_Odm, Path, 0x18, 0x00c00, 0x0);
			}
			break;
			default:
				break;
			}
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); // [31] = 1 --> Page C1

			// 3. TX RF Setting
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80000);
			ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x20000);
			ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x0003f);
			ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xf3fc3);
			ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d5);
			ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x8a001);
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x00000);
			ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
			ODM_SetBBReg(pDM_Odm, 0xc94, BIT(0), 0x1);
			ODM_Write4Byte(pDM_Odm, 0x978, 0x29002000);// TX (X,Y)
			ODM_Write4Byte(pDM_Odm, 0x97c, 0xa9002000);// RX (X,Y)
			ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a910);// [0]:AGC_en, [15]:idac_K_Mask

			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); // [31] = 1 --> Page C1

			if (pDM_Odm->ExtPA5G)
				ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403f7);
			else
				ODM_Write4Byte(pDM_Odm, 0xc88, 0x821403e3);

			if (*pDM_Odm->pBandType)
				ODM_Write4Byte(pDM_Odm, 0xc8c, 0x40163e96);
			else
				ODM_Write4Byte(pDM_Odm, 0xc8c, 0x00163e96);

			if (VDF_enable == 1) {
				for (k = 0; k <= 2; k++) {
					switch (k) {
					case 0: {
						ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c38);// TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16
						ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c38);// RX_Tone_idx[9:0], RxK_Mask[29]
						ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x0);
					}
					break;
					case 1: {
						ODM_SetBBReg(pDM_Odm, 0xc80, BIT(28), 0x0);
						ODM_SetBBReg(pDM_Odm, 0xc84, BIT(28), 0x0);
						ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x0);
					}
					break;
					case 2: {
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_Y[1] = %x;;;VDF_Y[0] = %x\n", VDF_Y[1]>>21 & 0x00007ff, VDF_Y[0]>>21 & 0x00007ff));
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("VDF_X[1] = %x;;;VDF_X[0] = %x\n", VDF_X[1]>>21 & 0x00007ff, VDF_X[0]>>21 & 0x00007ff));
						Tx_dt[cal] = (VDF_Y[1]>>20)-(VDF_Y[0]>>20);
						Tx_dt[cal] = ((16*Tx_dt[cal])*10000/15708);
						Tx_dt[cal] = (Tx_dt[cal] >> 1 )+(Tx_dt[cal] & BIT(0));
						ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c20);// TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16
						ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c20);// RX_Tone_idx[9:0], RxK_Mask[29]
						ODM_SetBBReg(pDM_Odm, 0xce8, BIT(31), 0x1);
						ODM_SetBBReg(pDM_Odm, 0xce8, 0x3fff0000, Tx_dt[cal] & 0x00003fff);
					}
					break;
					}
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);// cb8[20] 將 SI/PI 使用權切給 iqk_dpk module
					cal_retry = 0;
					while(1) {
						// one shot
						ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
						ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

						ODM_delay_ms(10); //Delay 10ms
						ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);
						delay_count = 0;
						while (1) {
							IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(10));
							if ((~IQK_ready) || (delay_count>20)) {
								break;
							} else {
								ODM_delay_ms(1);
								delay_count++;
							}
						}

						if (delay_count < 20) {						// If 20ms No Result, then cal_retry++
							// ============TXIQK Check==============
							TX_fail = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(12));

							if (~TX_fail) {
								ODM_Write4Byte(pDM_Odm, 0xcb8, 0x02000000);
								VDF_X[k] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
								ODM_Write4Byte(pDM_Odm, 0xcb8, 0x04000000);
								VDF_Y[k] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
								TX0IQKOK = TRUE;
								break;
							} else {
								ODM_SetBBReg(pDM_Odm, 0xccc, 0x000007ff, 0x0);
								ODM_SetBBReg(pDM_Odm, 0xcd4, 0x000007ff, 0x200);
								TX0IQKOK = FALSE;
								cal_retry++;
								if (cal_retry == 10) {
									break;
								}
							}
						} else {
							TX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10) {
								break;
							}
						}
					}
				}
				if (k == 3) {
					TX_X0[cal] = VDF_X[k-1] ;
					TX_Y0[cal] = VDF_Y[k-1];
				}
			} else {
				ODM_Write4Byte(pDM_Odm, 0xc80, 0x18008c10);// TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16
				ODM_Write4Byte(pDM_Odm, 0xc84, 0x38008c10);// RX_Tone_idx[9:0], RxK_Mask[29]
				ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);// cb8[20] 將 SI/PI 使用權切給 iqk_dpk module
				cal_retry = 0;
				while(1) {
					// one shot
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					ODM_delay_ms(10); //Delay 10ms
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(10));
						if ((~IQK_ready) || (delay_count>20)) {
							break;
						} else {
							ODM_delay_ms(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {						// If 20ms No Result, then cal_retry++
						// ============TXIQK Check==============
						TX_fail = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(12));

						if (~TX_fail) {
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x02000000);
							TX_X0[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x04000000);
							TX_Y0[cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							TX0IQKOK = TRUE;
							break;
						} else {
							ODM_SetBBReg(pDM_Odm, 0xccc, 0x000007ff, 0x0);
							ODM_SetBBReg(pDM_Odm, 0xcd4, 0x000007ff, 0x200);
							TX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10) {
								break;
							}
						}
					} else {
						TX0IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10)
							break;
					}
				}
			}

			if (TX0IQKOK == FALSE)
				break;				// TXK fail, Don't do RXK

			//====== RX IQK ======
			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
			// 1. RX RF Setting
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x80000);
			ODM_SetRFReg(pDM_Odm, Path, 0x30, bRFRegOffsetMask, 0x30000);
			ODM_SetRFReg(pDM_Odm, Path, 0x31, bRFRegOffsetMask, 0x0002f);
			ODM_SetRFReg(pDM_Odm, Path, 0x32, bRFRegOffsetMask, 0xfffbb);
			ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0x88001);
			ODM_SetRFReg(pDM_Odm, Path, 0x65, bRFRegOffsetMask, 0x931d8);
			ODM_SetRFReg(pDM_Odm, Path, 0xef, bRFRegOffsetMask, 0x00000);

			ODM_SetBBReg(pDM_Odm, 0x978, 0x03FF8000, (TX_X0[cal])>>21&0x000007ff);
			ODM_SetBBReg(pDM_Odm, 0x978, 0x000007FF, (TX_Y0[cal])>>21&0x000007ff);
			ODM_SetBBReg(pDM_Odm, 0x978, BIT(31), 0x1);
			ODM_SetBBReg(pDM_Odm, 0x97c, BIT(31), 0x0);
			ODM_Write4Byte(pDM_Odm, 0x90c, 0x00008000);
			ODM_Write4Byte(pDM_Odm, 0x984, 0x0046a911);

			ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); // [31] = 1 --> Page C1
			ODM_Write4Byte(pDM_Odm, 0xc80, 0x38008c10);// TX_Tone_idx[9:0], TxK_Mask[29] TX_Tone = 16
			ODM_Write4Byte(pDM_Odm, 0xc84, 0x18008c10);// RX_Tone_idx[9:0], RxK_Mask[29]
			ODM_Write4Byte(pDM_Odm, 0xc88, 0x02140119);

			if (pDM_Odm->SupportInterface == 1) {
				RXIQK_Loop = 2;				// for 2% fail;
			} else {
				RXIQK_Loop = 1;
			}
			for(i = 0; i < RXIQK_Loop; i++) {
				if (pDM_Odm->SupportInterface == 1)
					if(i == 0)
						ODM_Write4Byte(pDM_Odm, 0xc8c, 0x28161100);  //Good
					else
						ODM_Write4Byte(pDM_Odm, 0xc8c, 0x28160d00);
				else
					ODM_Write4Byte(pDM_Odm, 0xc8c, 0x28160d00);

				ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00100000);// cb8[20] 將 SI/PI 使用權切給 iqk_dpk module

				cal_retry = 0;
				while(1) {
					// one shot
					ODM_Write4Byte(pDM_Odm, 0x980, 0xfa000000);
					ODM_Write4Byte(pDM_Odm, 0x980, 0xf8000000);

					ODM_delay_ms(10); //Delay 10ms
					ODM_Write4Byte(pDM_Odm, 0xcb8, 0x00000000);
					delay_count = 0;
					while (1) {
						IQK_ready = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(10));
						if ((~IQK_ready)||(delay_count>20)) {
							break;
						} else {
							ODM_delay_ms(1);
							delay_count++;
						}
					}

					if (delay_count < 20) {	// If 20ms No Result, then cal_retry++
						// ============RXIQK Check==============
						RX_fail = ODM_GetBBReg(pDM_Odm, 0xd00, BIT(11));
						if (RX_fail == 0) {
							/*
							DbgPrint("====== RXIQK (%d) ======", i);
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x05000000);
							reg1 = ODM_GetBBReg(pDM_Odm, 0xd00, 0xffffffff);
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x06000000);
							reg2 = ODM_GetBBReg(pDM_Odm, 0xd00, 0x0000001f);
							DbgPrint("reg1 = %d, reg2 = %d", reg1, reg2);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("Before PW = %d\n", Image_Power);
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x07000000);
							reg1 = ODM_GetBBReg(pDM_Odm, 0xd00, 0xffffffff);
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x08000000);
							reg2 = ODM_GetBBReg(pDM_Odm, 0xd00, 0x0000001f);
							Image_Power = (reg2<<32)+reg1;
							DbgPrint("After PW = %d\n", Image_Power);
							*/

							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x06000000);
							RX_X0[i][cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							ODM_Write4Byte(pDM_Odm, 0xcb8, 0x08000000);
							RX_Y0[i][cal] = ODM_GetBBReg(pDM_Odm, 0xd00, 0x07ff0000)<<21;
							RX0IQKOK = TRUE;
							break;
						} else {
							ODM_SetBBReg(pDM_Odm, 0xc10, 0x000003ff, 0x200>>1);
							ODM_SetBBReg(pDM_Odm, 0xc10, 0x03ff0000, 0x0>>1);
							RX0IQKOK = FALSE;
							cal_retry++;
							if (cal_retry == 10)
								break;

						}
					} else {
						RX0IQKOK = FALSE;
						cal_retry++;
						if (cal_retry == 10)
							break;
					}
				}
			}

			if (TX0IQKOK)
				TX_Average++;
			if (RX0IQKOK)
				RX_Average++;
		}
		break;
		default:
			break;
		}
		cal++;
	}
	// FillIQK Result
	switch (Path) {
	case ODM_RF_PATH_A: {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("========Path_A =======\n"));
		if (TX_Average == 0)
			break;

		for (i = 0; i < TX_Average; i++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("TX_X0[%d] = %x ;; TX_Y0[%d] = %x\n", i, (TX_X0[i])>>21&0x000007ff, i, (TX_Y0[i])>>21&0x000007ff));
		}
		for (i = 0; i < TX_Average; i++) {
			for (ii = i+1; ii <TX_Average; ii++) {
				dx = (TX_X0[i]>>21) - (TX_X0[ii]>>21);
				if (dx < 3 && dx > -3) {
					dy = (TX_Y0[i]>>21) - (TX_Y0[ii]>>21);
					if (dy < 3 && dy > -3) {
						TX_X = ((TX_X0[i]>>21) + (TX_X0[ii]>>21))/2;
						TX_Y = ((TX_Y0[i]>>21) + (TX_Y0[ii]>>21))/2;
						TX_finish = 1;
						break;
					}
				}
			}
			if (TX_finish == 1)
				break;
		}

		if (TX_finish == 1) {
			_IQK_TX_FillIQC_8821A(pDM_Odm, Path, TX_X, TX_Y);
		} else {
			_IQK_TX_FillIQC_8821A(pDM_Odm, Path, 0x200, 0x0);
		}

		if (RX_Average == 0)
			break;

		for (i = 0; i < RX_Average; i++) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X0[0][%d] = %x ;; RX_Y0[0][%d] = %x\n", i, (RX_X0[0][i])>>21&0x000007ff, i, (RX_Y0[0][i])>>21&0x000007ff));
			if (RXIQK_Loop == 2)
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("RX_X0[1][%d] = %x ;; RX_Y0[1][%d] = %x\n", i, (RX_X0[1][i])>>21&0x000007ff, i, (RX_Y0[1][i])>>21&0x000007ff));
		}
		for (i = 0; i < RX_Average; i++) {
			for (ii = i+1; ii <RX_Average; ii++) {
				dx = (RX_X0[0][i]>>21) - (RX_X0[0][ii]>>21);
				if (dx < 4 && dx > -4) {
					dy = (RX_Y0[0][i]>>21) - (RX_Y0[0][ii]>>21);
					if (dy < 4 && dy > -4) {
						RX_X_temp = ((RX_X0[0][i]>>21) + (RX_X0[0][ii]>>21))/2;
						RX_Y_temp = ((RX_Y0[0][i]>>21) + (RX_Y0[0][ii]>>21))/2;
						RX_finish1 = 1;
						break;
					}
				}
			}
			if (RX_finish1 == 1) {
				RX_X = RX_X_temp;
				RX_Y = RX_Y_temp;
				break;
			}
		}
		if(RXIQK_Loop == 2) {
			for (i = 0; i < RX_Average; i++) {
				for (ii = i+1; ii <RX_Average; ii++) {
					dx = (RX_X0[1][i]>>21) - (RX_X0[1][ii]>>21);
					if (dx < 4 && dx > -4) {
						dy = (RX_Y0[1][i]>>21) - (RX_Y0[1][ii]>>21);
						if (dy < 4 && dy > -4) {
							RX_X = ((RX_X0[1][i]>>21) + (RX_X0[1][ii]>>21))/2;
							RX_Y = ((RX_Y0[1][i]>>21) + (RX_Y0[1][ii]>>21))/2;
							RX_finish2 = 1;
							break;
						}
					}
				}
				if (RX_finish2 == 1)
					break;
			}
			if(RX_finish1 && RX_finish2) {
				RX_X = (RX_X+RX_X_temp)/2;
				RX_Y = (RX_Y+RX_Y_temp)/2;
			}
		}
		if (RX_finish1 || RX_finish2) {
			_IQK_RX_FillIQC_8821A(pDM_Odm, Path, RX_X, RX_Y);
		} else {
			_IQK_RX_FillIQC_8821A(pDM_Odm, Path, 0x200, 0x0);
		}
	}
	break;
	default:
		break;
	}
}

#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
VOID
phy_IQCalibrate_By_FW_8821A(
    IN 	PDM_ODM_T	pDM_Odm
)
{

	u1Byte			IQKcmd[3] = {*pDM_Odm->pChannel, 0x0, 0x0};
	u1Byte			Buf1 = 0x0;
	u1Byte			Buf2 = 0x0;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("pChannel: %d \n", *pDM_Odm->pChannel));


//Byte 2, Bit 4 ~ Bit 5 : BandType
	if(*pDM_Odm->pBandType)
		Buf1 = 0x2<<4;
	else
		Buf1 = 0x1<<4;

//Byte 2, Bit 0 ~ Bit 3 : Bandwidth
	if(*pDM_Odm->pBandWidth == ODM_BW20M)
		Buf2 = 0x1;
	else if(*pDM_Odm->pBandWidth == ODM_BW40M)
		Buf2 = 0x1<<1;
	else if(*pDM_Odm->pBandWidth == ODM_BW80M)
		Buf2 = 0x1<<2;
	else
		Buf2 = 0x1<<3;

	IQKcmd[1] = Buf1 | Buf2;
	IQKcmd[2] = pDM_Odm->ExtPA5G | pDM_Odm->ExtLNA5G<<1;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("== FW IQK Start ==\n"));
	pDM_Odm->RFCalibrateInfo.IQK_StartTime = 0;
	pDM_Odm->RFCalibrateInfo.IQK_StartTime = ODM_GetCurrentTime( pDM_Odm);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("== StartTime: %lld\n", pDM_Odm->RFCalibrateInfo.IQK_StartTime));
	ODM_FillH2CCmd(pDM_Odm, ODM_H2C_IQ_CALIBRATION, 3, IQKcmd);


}
#endif

VOID
phy_IQCalibrate_8821A(
    IN PDM_ODM_T		pDM_Odm
)
{
	u4Byte	MACBB_backup[MACBB_REG_NUM_8821A], AFE_backup[AFE_REG_NUM_8821A], RFA_backup[RF_REG_NUM_8821A], RFB_backup[RF_REG_NUM_8821A];
	u4Byte 	Backup_MACBB_REG[MACBB_REG_NUM_8821A] = {0x520, 0x550, 0x808, 0xa04, 0x90c, 0xc00, 0x838, 0x82c};
	u4Byte 	Backup_AFE_REG[AFE_REG_NUM_8821A] = {0xc5c, 0xc60, 0xc64, 0xc68};
	u4Byte 	Backup_RF_REG[RF_REG_NUM_8821A] = {0x65, 0x8f, 0x0};

	_IQK_BackupMacBB_8821A(pDM_Odm, MACBB_backup, Backup_MACBB_REG, MACBB_REG_NUM_8821A);
	_IQK_BackupAFE_8821A(pDM_Odm, AFE_backup, Backup_AFE_REG, AFE_REG_NUM_8821A);
	_IQK_BackupRF_8821A(pDM_Odm, RFA_backup, RFB_backup, Backup_RF_REG, RF_REG_NUM_8821A);

	_IQK_ConfigureMAC_8821A(pDM_Odm);
	_IQK_Tx_8821A(pDM_Odm, ODM_RF_PATH_A);

	_IQK_RestoreRF_8821A(pDM_Odm, ODM_RF_PATH_A, Backup_RF_REG, RFA_backup, RF_REG_NUM_8821A);
	_IQK_RestoreAFE_8821A(pDM_Odm, AFE_backup, Backup_AFE_REG, AFE_REG_NUM_8821A);
	_IQK_RestoreMacBB_8821A(pDM_Odm, MACBB_backup, Backup_MACBB_REG, MACBB_REG_NUM_8821A);
}

VOID
PHY_ResetIQKResult_8821A(
    IN	PDM_ODM_T	pDM_Odm
)
{
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x1); // [31] = 1 --> Page C1
	ODM_SetBBReg(pDM_Odm, 0xccc, 0x000007ff, 0x0);
	ODM_SetBBReg(pDM_Odm, 0xcd4, 0x000007ff, 0x200);
	ODM_Write4Byte(pDM_Odm, 0xce8, 0x0);
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT(31), 0x0); // [31] = 0 --> Page C
	ODM_SetBBReg(pDM_Odm, 0xc10, 0x000003ff, 0x100);
}


#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
VOID
PHY_IQCalibrate_8821A(
    IN	PDM_ODM_T	pDM_Odm,
    IN	BOOLEAN 	bReCovery
)
{
#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	PADAPTER 		pAdapter = pDM_Odm->Adapter;
	//HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	u4Byte			counter = 0;
#endif
#endif

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN )
	if (ODM_CheckPowerStatus(pAdapter) == FALSE)
		return;
#endif

	if (pDM_Odm->mp_mode) {  		//(MP_DRIVER == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#if (MP_DRIVER == 1)
		PMPT_CONTEXT	pMptCtx = &(pAdapter->MptCtx);
		if( pMptCtx->bSingleTone || pMptCtx->bCarrierSuppression)
			return;
#endif
#else// (DM_ODM_SUPPORT_TYPE == ODM_CE)
		PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.MptCtx);
		if( pMptCtx->bSingleTone || pMptCtx->bCarrierSuppression)
			return;
#endif

	}
	pDM_Odm->IQKFWOffload = 0;

	//3 == FW IQK ==
	if(pDM_Odm->IQKFWOffload) {
		if ( ! pDM_Odm->RFCalibrateInfo.bIQKInProgress) {
			ODM_AcquireSpinLock( pDM_Odm, RT_IQK_SPINLOCK);
			pDM_Odm->RFCalibrateInfo.bIQKInProgress = TRUE;
			ODM_ReleaseSpinLock( pDM_Odm, RT_IQK_SPINLOCK);
			phy_IQCalibrate_By_FW_8821A(pDM_Odm);
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
			for(counter = 0; counter < 10; counter++) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("== FW IQK IN PROGRESS == #%d\n", counter));
				ODM_delay_ms(50);
				if ( ! pDM_Odm->RFCalibrateInfo.bIQKInProgress) {
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("== FW IQK RETURN FROM WAITING ==\n"));
					break;
				}
			}
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
			rtl8812_iqk_wait(pAdapter, 500);
#endif
			if (pDM_Odm->RFCalibrateInfo.bIQKInProgress) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("== FW IQK TIMEOUT (Still in progress after 500ms) ==\n"));
				ODM_AcquireSpinLock( pDM_Odm, RT_IQK_SPINLOCK);
				pDM_Odm->RFCalibrateInfo.bIQKInProgress = FALSE;
				ODM_ReleaseSpinLock( pDM_Odm, RT_IQK_SPINLOCK);
			}
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("== Return the IQK CMD, because the IQK in Progress ==\n"));
		}
	}
	//3 == Driver IQK ==
	else {
		if ( ! pDM_Odm->RFCalibrateInfo.bIQKInProgress) {
			ODM_AcquireSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
			pDM_Odm->RFCalibrateInfo.bIQKInProgress = TRUE;
			ODM_ReleaseSpinLock(pDM_Odm, RT_IQK_SPINLOCK);

			pDM_Odm->RFCalibrateInfo.IQK_StartTime = ODM_GetCurrentTime( pDM_Odm);
			phy_IQCalibrate_8821A(pDM_Odm);
			pDM_Odm->RFCalibrateInfo.IQK_ProgressingTime = ODM_GetProgressingTime( pDM_Odm, pDM_Odm->RFCalibrateInfo.IQK_StartTime);
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD,  ("IQK ProgressingTime = %lld ms\n", pDM_Odm->RFCalibrateInfo.IQK_ProgressingTime));

			ODM_AcquireSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
			pDM_Odm->RFCalibrateInfo.bIQKInProgress = FALSE;
			ODM_ReleaseSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("== Return the IQK CMD, because the IQK in Progress ==\n"));
		}
	}

}
#endif
