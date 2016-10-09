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
#ifndef __INC_HAL8723BPHYREG_H__
#define __INC_HAL8723BPHYREG_H__

#include <Hal8192CPhyReg.h>

#define		rSYM_WLBT_PAPE_SEL		0x64
// BB Register Definition
//
// 4. Page9(0x900)
//
#define rDPDT_control				0x92c
#define rfe_ctrl_anta_src				0x930
#define rS0S1_PathSwitch   			0x948
#define rAGC_table_select				0xb2c

//
// PageB(0xB00)
//
#define rPdp_AntA						0xb00
#define rPdp_AntA_4						0xb04
#define rPdp_AntA_8						0xb08
#define rPdp_AntA_C						0xb0c
#define rPdp_AntA_10					0xb10
#define rPdp_AntA_14					0xb14
#define rPdp_AntA_18					0xb18
#define rPdp_AntA_1C					0xb1c
#define rPdp_AntA_20					0xb20
#define rPdp_AntA_24					0xb24

#define rConfig_Pmpd_AntA 				0xb28
#define rConfig_ram64x16				0xb2c

#define rBndA							0xb30
#define rHssiPar						0xb34

#define rConfig_AntA					0xb68
#define rConfig_AntB					0xb6c

#define rPdp_AntB						0xb70
#define rPdp_AntB_4						0xb74
#define rPdp_AntB_8						0xb78
#define rPdp_AntB_C						0xb7c
#define rPdp_AntB_10					0xb80
#define rPdp_AntB_14					0xb84
#define rPdp_AntB_18					0xb88
#define rPdp_AntB_1C					0xb8c
#define rPdp_AntB_20					0xb90
#define rPdp_AntB_24					0xb94

#define rConfig_Pmpd_AntB				0xb98

#define rBndB							0xba0

#define rAPK							0xbd8
#define rPm_Rx0_AntA					0xbdc
#define rPm_Rx1_AntA					0xbe0
#define rPm_Rx2_AntA					0xbe4
#define rPm_Rx3_AntA					0xbe8
#define rPm_Rx0_AntB					0xbec
#define rPm_Rx1_AntB					0xbf0
#define rPm_Rx2_AntB					0xbf4
#define rPm_Rx3_AntB					0xbf8

#endif

