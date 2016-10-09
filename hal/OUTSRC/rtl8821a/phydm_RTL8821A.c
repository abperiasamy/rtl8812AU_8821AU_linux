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

//============================================================
// include files
//============================================================

#include "Mp_Precomp.h"

#include "../phydm_precomp.h"

#if (RTL8821A_SUPPORT == 1)

VOID
odm_DynamicTryStateAgg_8821A(
    IN		PDM_ODM_T		pDM_Odm
)
{
	if((pDM_Odm->SupportICType == ODM_RTL8821) && (pDM_Odm->SupportInterface == ODM_ITRF_USB)) {
		if(pDM_Odm->RSSI_Min > 25)
			ODM_Write1Byte(pDM_Odm, 0x4CF, 0x02);
		else if(pDM_Odm->RSSI_Min < 20)
			ODM_Write1Byte(pDM_Odm, 0x4CF, 0x00);
	}
}

VOID
odm_HWSetting_8821A(
    IN		PDM_ODM_T		pDM_Odm
)
{
	odm_DynamicTryStateAgg_8821A(pDM_Odm);
}

#endif //#if (RTL8821A_SUPPORT == 1)
