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
#ifndef __INC_HAL8723PHYCFG_H__
#define __INC_HAL8723PHYCFG_H__

#include <Hal8192CPhyCfg.h>
/* MAC/BB/RF HAL config */
int	PHY_BBConfig8723A(	IN	PADAPTER	Adapter	);
int	PHY_RFConfig8723A(	IN	PADAPTER	Adapter	);
s32 PHY_MACConfig8723A(PADAPTER padapter);

VOID
PHY_SetSwChnlBWMode8723A(
    IN	PADAPTER			Adapter,
    IN	u8					channel,
    IN	CHANNEL_WIDTH	Bandwidth,
    IN	u8					Offset40,
    IN	u8					Offset80
);

#endif

