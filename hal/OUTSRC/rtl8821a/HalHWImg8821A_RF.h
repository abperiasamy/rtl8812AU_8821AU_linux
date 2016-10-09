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

#if (RTL8821A_SUPPORT == 1)
#ifndef __INC_MP_RF_HW_IMG_8821A_H
#define __INC_MP_RF_HW_IMG_8821A_H


/******************************************************************************
*                           RadioA.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8821A_RadioA( // TC: Test Chip, MP: MP Chip
    IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8821A_RadioA(void);

/******************************************************************************
*                           TxPowerTrack_AP.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8821A_TxPowerTrack_AP( // TC: Test Chip, MP: MP Chip
    IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8821A_TxPowerTrack_AP(void);

/******************************************************************************
*                           TxPowerTrack_PCIE.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8821A_TxPowerTrack_PCIE( // TC: Test Chip, MP: MP Chip
    IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8821A_TxPowerTrack_PCIE(void);

/******************************************************************************
*                           TxPowerTrack_SDIO.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8821A_TxPowerTrack_SDIO( // TC: Test Chip, MP: MP Chip
    IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8821A_TxPowerTrack_SDIO(void);

/******************************************************************************
*                           TxPowerTrack_USB.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8821A_TxPowerTrack_USB( // TC: Test Chip, MP: MP Chip
    IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8821A_TxPowerTrack_USB(void);

/******************************************************************************
*                           TXPWR_LMT_8811AU_FEM.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8821A_TXPWR_LMT_8811AU_FEM( // TC: Test Chip, MP: MP Chip
    IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8821A_TXPWR_LMT_8811AU_FEM(void);

/******************************************************************************
*                           TXPWR_LMT_8811AU_IPA.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8821A_TXPWR_LMT_8811AU_IPA( // TC: Test Chip, MP: MP Chip
    IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8821A_TXPWR_LMT_8811AU_IPA(void);

/******************************************************************************
*                           TXPWR_LMT_8821A.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8821A_TXPWR_LMT_8821A( // TC: Test Chip, MP: MP Chip
    IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8821A_TXPWR_LMT_8821A(void);

/******************************************************************************
*                           TXPWR_LMT_8821A_SAR_13dBm.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8821A_TXPWR_LMT_8821A_SAR_13dBm( // TC: Test Chip, MP: MP Chip
    IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8821A_TXPWR_LMT_8821A_SAR_13dBm(void);

/******************************************************************************
*                           TXPWR_LMT_8821A_SAR_5mm.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8821A_TXPWR_LMT_8821A_SAR_5mm( // TC: Test Chip, MP: MP Chip
    IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8821A_TXPWR_LMT_8821A_SAR_5mm(void);

/******************************************************************************
*                           TXPWR_LMT_8821A_SAR_8mm.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8821A_TXPWR_LMT_8821A_SAR_8mm( // TC: Test Chip, MP: MP Chip
    IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8821A_TXPWR_LMT_8821A_SAR_8mm(void);

#endif
#endif // end of HWIMG_SUPPORT
