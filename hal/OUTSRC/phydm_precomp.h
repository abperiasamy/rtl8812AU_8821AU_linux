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

#ifndef	__ODM_PRECOMP_H__
#define __ODM_PRECOMP_H__

#include "phydm_types.h"

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#include "Precomp.h"		// We need to include mp_precomp.h due to batch file setting.
#else
#define		TEST_FALG___		1
#endif

//2 Config Flags and Structs - defined by each ODM Type

#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
#include "../8192cd_cfg.h"
#include "../odm_inc.h"

#include "../8192cd.h"
#include "../8192cd_util.h"
#ifdef _BIG_ENDIAN_
#define	ODM_ENDIAN_TYPE				ODM_ENDIAN_BIG
#else
#define	ODM_ENDIAN_TYPE				ODM_ENDIAN_LITTLE
#endif

#ifdef AP_BUILD_WORKAROUND
#include "../8192cd_headers.h"
#include "../8192cd_debug.h"
#endif

#elif (DM_ODM_SUPPORT_TYPE ==ODM_CE)
#define BEAMFORMING_SUPPORT 0
#elif (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#include "Mp_Precomp.h"
#define	ODM_ENDIAN_TYPE				ODM_ENDIAN_LITTLE
#endif

//2 OutSrc Header Files

#include "phydm.h"
#include "phydm_HWConfig.h"
#include "phydm_debug.h"
#include "phydm_RegDefine11AC.h"
#include "phydm_RegDefine11N.h"
#include "phydm_interface.h"
#include "phydm_reg.h"
#include "HalPhyRf.h"

#if (DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_AP))
#define RTL8821B_SUPPORT		0
#define RTL8822B_SUPPORT		0
#define RTL8703B_SUPPORT		0
#define RTL8188F_SUPPORT		0
#endif

#if (RTL8192C_SUPPORT==1)
#include "rtl8192c/phydm_RTL8192C.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
#include "rtl8192c/Hal8192CEFWImg_AP.h"
#include "rtl8192c/Hal8192CEPHYImg_AP.h"
#include "rtl8192c/Hal8192CEMACImg_AP.h"
#include "rtl8192c/HalDMOutSrc8192C_AP.h"
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#if(RTL8192CE_SUPPORT ==1)
#include "rtl8192c/Hal8192CEFWImg_CE.h"
#include "rtl8192c/Hal8192CEPHYImg_CE.h"
#include "rtl8192c/Hal8192CEMACImg_CE.h"
#elif(RTL8192CU_SUPPORT ==1)
#include "rtl8192c/Hal8192CUFWImg_CE.h"
#include "rtl8192c/Hal8192CUPHYImg_CE.h"
#include "rtl8192c/Hal8192CUMACImg_CE.h"
#endif
#ifdef CONFIG_INTEL_PROXIM
#include "../proxim/intel_proxim.h"
#endif
#include  "rtl8192c/HalDMOutSrc8192C_CE.h"
#include <rtl8192c_hal.h>
#endif
#endif

#if (RTL8192D_SUPPORT==1)
#include "rtl8192d/phydm_RTL8192D.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#if(RTL8192DE_SUPPORT ==1)
#include "rtl8192d/Hal8192DEFWImg_CE.h"
#include "rtl8192d/Hal8192DEPHYImg_CE.h"
#include "rtl8192d/Hal8192DEMACImg_CE.h"
#elif(RTL8192DU_SUPPORT ==1)
#include "rtl8192d/Hal8192DUFWImg_CE.h"
#include "rtl8192d/Hal8192DUPHYImg_CE.h"
#include "rtl8192d/Hal8192DUMACImg_CE.h"
#endif
#include  "rtl8192d/HalDMOutSrc8192D_CE.h"
#include  "rtl8192d_hal.h"
#endif
#endif

#if (RTL8723A_SUPPORT==1)
#include "rtl8723a/HalHWImg8723A_MAC.h"
#include "rtl8723a/HalHWImg8723A_RF.h"
#include "rtl8723a/HalHWImg8723A_BB.h"
#include "rtl8723a/HalHWImg8723A_FW.h"
#include "rtl8723a/phydm_RegConfig8723A.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#if(RTL8723AS_SUPPORT==1)
#include "rtl8723a/Hal8723SHWImg_CE.h"
#elif(RTL8723AU_SUPPORT==1)
#include "rtl8723a/Hal8723UHWImg_CE.h"
#endif
#include "rtl8192c/HalDMOutSrc8192C_CE.h" //for IQK,LCK,Power-tracking
#include "rtl8723a_hal.h"
#endif
#endif

#if (RTL8188E_SUPPORT==1)
#include "rtl8188e/Hal8188ERateAdaptive.h"//for  RA,Power training
#include "rtl8188e/HalHWImg8188E_MAC.h"
#include "rtl8188e/HalHWImg8188E_RF.h"
#include "rtl8188e/HalHWImg8188E_BB.h"
#include "rtl8188e/HalHWImg8188E_FW.h"
#include "rtl8188e/phydm_RegConfig8188E.h"
#include "rtl8188e/phydm_RTL8188E.h"
#include "rtl8188e/HalPhyRf_8188e.h"
#include "rtl8188e/Hal8188EReg.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8188e_hal.h"
#endif
#endif  //88E END

#if (RTL8192E_SUPPORT==1)
#include "rtl8192e/HalPhyRf_8192e.h" //FOR_8192E_IQK
#include "rtl8192e/phydm_RTL8192E.h" //FOR_8192E_IQK
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)
#include "rtl8192e/HalHWImg8192E_BB.h"
#include "rtl8192e/HalHWImg8192E_MAC.h"
#include "rtl8192e/HalHWImg8192E_RF.h"
#include "rtl8192e/phydm_RegConfig8192E.h"
#include "rtl8192e/HalHWImg8192E_FW.h"
#include "rtl8192e/Hal8192EReg.h"
#endif
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8192e_hal.h"
#endif
#endif  //92E END

#if (RTL8812A_SUPPORT==1)
#include "rtl8812a/HalPhyRf_8812A.h" //FOR_8812_IQK
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)
#include "rtl8812a/HalHWImg8812A_BB.h"
#include "rtl8812a/HalHWImg8812A_MAC.h"
#include "rtl8812a/HalHWImg8812A_RF.h"
#include "rtl8812a/phydm_RegConfig8812A.h"
#include "rtl8812a/HalHWImg8812A_FW.h"
#include "rtl8812a/phydm_RTL8812A.h"
#endif
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8812a_hal.h"
#endif
#endif //8812 END

#if (RTL8814A_SUPPORT==1)
#include "rtl8814a/HalPhyRf_8814A.h"
#include "rtl8814a/HalHWImg8814A_MAC.h"
#include "rtl8814a/HalHWImg8814A_RF.h"
#include "rtl8814a/HalHWImg8814A_BB.h"
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)
#include "rtl8814a/HalHWImg8814A_FW.h"
#include "rtl8814a/phydm_RTL8814A.h"
#endif
#include "rtl8814a/phydm_RegConfig8814A.h"
#endif //8814 END

#if (RTL8881A_SUPPORT==1)//FOR_8881_IQK
#include "rtl8821a/PhyDM_IQK_8821A.h"
//#include "rtl8881a/HalHWImg8881A_BB.h"
//#include "rtl8881a/HalHWImg8881A_MAC.h"
//#include "rtl8881a/HalHWImg8881A_RF.h"
//#include "rtl8881a/odm_RegConfig8881A.h"
#endif

#if (RTL8723B_SUPPORT==1)
#include "rtl8723b/HalHWImg8723B_MAC.h"
#include "rtl8723b/HalHWImg8723B_RF.h"
#include "rtl8723b/HalHWImg8723B_BB.h"
#include "rtl8723b/HalHWImg8723B_FW.h"
//#include "rtl8723b/Hal8723BReg.h"
#include "rtl8723b/phydm_RegConfig8723B.h"
#include "rtl8723b/phydm_RTL8723B.h"
#include "rtl8723b/HalPhyRf_8723B.h"
#include "rtl8723b/Hal8723BReg.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8723b/HalHWImg8723B_MP.h"
#include "rtl8723b_hal.h"
#endif
#endif

#if (RTL8821A_SUPPORT==1)
#include "rtl8821a/HalHWImg8821A_MAC.h"
#include "rtl8821a/HalHWImg8821A_RF.h"
#include "rtl8821a/HalHWImg8821A_BB.h"
#include "rtl8821a/HalHWImg8821A_FW.h"
#include "rtl8821a/phydm_RegConfig8821A.h"
#include "rtl8821a/phydm_RTL8821A.h"
#include "rtl8821a/HalPhyRf_8821A.h"
#include "rtl8821a/PhyDM_IQK_8821A.h"//for IQK
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#include "rtl8812a/HalPhyRf_8812A.h"//for IQK,LCK,Power-tracking
#include "rtl8812a_hal.h"
#endif
#endif

#if (RTL8821B_SUPPORT==1)
#include "rtl8821b/HalHWImg8821B_MAC.h"
#include "rtl8821b/HalHWImg8821B_RF.h"
#include "rtl8821b/HalHWImg8821B_BB.h"
#include "rtl8821b/HalHWImg8821B_FW.h"
#include "rtl8821b/phydm_RegConfig8821B.h"
#include "rtl8821b/HalHWImg8821B_TestChip_MAC.h"
#include "rtl8821b/HalHWImg8821B_TestChip_RF.h"
#include "rtl8821b/HalHWImg8821B_TestChip_BB.h"
#include "rtl8821b/HalHWImg8821B_TestChip_FW.h"
#include "rtl8821b/HalPhyRf_8821B.h"
#endif

#if (RTL8822B_SUPPORT==1)
#include "rtl8822B/HalHWImg8822B_MAC.h"
#include "rtl8822B/HalHWImg8822B_RF.h"
#include "rtl8822B/HalHWImg8822B_BB.h"
#include "rtl8822B/HalHWImg8822B_FW.h"
#include "rtl8822B/phydm_RegConfig8822B.h"
#include "rtl8822B/HalHWImg8822B_TestChip_MAC.h"
#include "rtl8822B/HalHWImg8822B_TestChip_RF.h"
#include "rtl8822B/HalHWImg8822B_TestChip_BB.h"
#include "rtl8822B/HalHWImg8822B_TestChip_FW.h"
#include "rtl8822b/HalPhyRf_8822B.h"
#endif

#if (RTL8703B_SUPPORT==1)
#include "rtl8703b/phydm_RegConfig8703B.h"
#include "rtl8703b/HalHWImg8703B_TestChip_MAC.h"
#include "rtl8703b/HalHWImg8703B_TestChip_RF.h"
#include "rtl8703b/HalHWImg8703B_TestChip_BB.h"
#include "rtl8703b/HalHWImg8703B_FW.h"
#endif

#if (RTL8188F_SUPPORT==1)
#include "rtl8188f/phydm_RegConfig8188F.h"
#include "rtl8188f/HalHWImg8188F_TestChip_MAC.h"
#include "rtl8188f/HalHWImg8188F_TestChip_RF.h"
#include "rtl8188f/HalHWImg8188F_TestChip_BB.h"
#include "rtl8188f/HalHWImg8188F_FW.h"
#endif

#endif	// __ODM_PRECOMP_H__
