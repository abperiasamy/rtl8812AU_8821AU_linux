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
#define _HAL_USB_C_

#include <drv_types.h>
#include <hal_data.h>

int	usb_init_recv_priv(_adapter *padapter, u16 ini_in_buf_sz)
{
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	int	i, res = _SUCCESS;
	struct recv_buf *precvbuf;

#ifdef CONFIG_RECV_THREAD_MODE
	_rtw_init_sema(&precvpriv->recv_sema, 0);//will be removed
	_rtw_init_sema(&precvpriv->terminate_recvthread_sema, 0);//will be removed
#endif /* CONFIG_RECV_THREAD_MODE */

#ifdef PLATFORM_LINUX
	tasklet_init(&precvpriv->recv_tasklet,
	             (void(*)(unsigned long))usb_recv_tasklet,
	             (unsigned long)padapter);
#endif /* PLATFORM_LINUX */

#ifdef PLATFORM_FREEBSD
	TASK_INIT(&precvpriv->recv_tasklet, 0, rtl8192du_recv_tasklet, padapter);
#ifdef CONFIG_RX_INDICATE_QUEUE
	TASK_INIT(&precvpriv->rx_indicate_tasklet, 0, rtw_rx_indicate_tasklet, padapter);
#endif /* CONFIG_RX_INDICATE_QUEUE */
#endif /* PLATFORM_FREEBSD */

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
#ifdef PLATFORM_LINUX
	precvpriv->int_in_urb = usb_alloc_urb(0, GFP_KERNEL);
	if(precvpriv->int_in_urb == NULL) {
		res = _FAIL;
		DBG_8192C("alloc_urb for interrupt in endpoint fail !!!!\n");
		goto exit;
	}
#endif /* PLATFORM_LINUX */
	precvpriv->int_in_buf = rtw_zmalloc(ini_in_buf_sz);
	if(precvpriv->int_in_buf == NULL) {
		res = _FAIL;
		DBG_8192C("alloc_mem for interrupt in endpoint fail !!!!\n");
		goto exit;
	}
#endif /* CONFIG_USB_INTERRUPT_IN_PIPE */

	/* init recv_buf */
	_rtw_init_queue(&precvpriv->free_recv_buf_queue);
	_rtw_init_queue(&precvpriv->recv_buf_pending_queue);
#ifndef CONFIG_USE_USB_BUFFER_ALLOC_RX
	/* this is used only when RX_IOBUF is sk_buff */
	skb_queue_head_init(&precvpriv->free_recv_skb_queue);
#endif

	precvpriv->pallocated_recv_buf = rtw_zmalloc(NR_RECVBUFF *sizeof(struct recv_buf) + 4);
	if(precvpriv->pallocated_recv_buf==NULL) {
		res= _FAIL;
		RT_TRACE(_module_rtl871x_recv_c_,_drv_err_,("alloc recv_buf fail!\n"));
		goto exit;
	}

	precvpriv->precv_buf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(precvpriv->pallocated_recv_buf), 4);

	precvbuf = (struct recv_buf*)precvpriv->precv_buf;

	for(i=0; i < NR_RECVBUFF ; i++) {
		_rtw_init_listhead(&precvbuf->list);

		_rtw_spinlock_init(&precvbuf->recvbuf_lock);

		precvbuf->alloc_sz = MAX_RECVBUF_SZ;

		res = rtw_os_recvbuf_resource_alloc(padapter, precvbuf);
		if(res==_FAIL)
			break;

		precvbuf->ref_cnt = 0;
		precvbuf->adapter =padapter;

		//rtw_list_insert_tail(&precvbuf->list, &(precvpriv->free_recv_buf_queue.queue));

		precvbuf++;
	}

	precvpriv->free_recv_buf_queue_cnt = NR_RECVBUFF;

#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD)

	skb_queue_head_init(&precvpriv->rx_skb_queue);

#ifdef CONFIG_RX_INDICATE_QUEUE
	memset(&precvpriv->rx_indicate_queue, 0, sizeof(struct ifqueue));
	mtx_init(&precvpriv->rx_indicate_queue.ifq_mtx, "rx_indicate_queue", NULL, MTX_DEF);
#endif /* CONFIG_RX_INDICATE_QUEUE */

#ifdef CONFIG_PREALLOC_RECV_SKB
	{
		int i;
		SIZE_PTR tmpaddr=0;
		SIZE_PTR alignment=0;
		struct sk_buff *pskb=NULL;

		for(i=0; i<NR_PREALLOC_RECV_SKB; i++) {
#ifdef CONFIG_PREALLOC_RX_SKB_BUFFER
			pskb = rtw_alloc_skb_premem();
#else
			pskb = rtw_skb_alloc(MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ);
#endif //CONFIG_PREALLOC_RX_SKB_BUFFER

			if(pskb) {
#ifdef PLATFORM_FREEBSD
				pskb->dev = padapter->pifp;
#else
				pskb->dev = padapter->pnetdev;
#endif //PLATFORM_FREEBSD

#ifndef CONFIG_PREALLOC_RX_SKB_BUFFER
				tmpaddr = (SIZE_PTR)pskb->data;
				alignment = tmpaddr & (RECVBUFF_ALIGN_SZ-1);
				skb_reserve(pskb, (RECVBUFF_ALIGN_SZ - alignment));
#endif
				skb_queue_tail(&precvpriv->free_recv_skb_queue, pskb);
			}

			pskb=NULL;
		}
	}
#endif /* CONFIG_PREALLOC_RECV_SKB */

#endif /* defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) */

exit:

	return res;
}

void usb_free_recv_priv (_adapter *padapter, u16 ini_in_buf_sz)
{
	int i;
	struct recv_buf *precvbuf;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	precvbuf = (struct recv_buf *)precvpriv->precv_buf;

	for(i=0; i < NR_RECVBUFF ; i++) {
		rtw_os_recvbuf_resource_free(padapter, precvbuf);
		precvbuf++;
	}

	if(precvpriv->pallocated_recv_buf)
		rtw_mfree(precvpriv->pallocated_recv_buf, NR_RECVBUFF *sizeof(struct recv_buf) + 4);

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
#ifdef PLATFORM_LINUX
	if(precvpriv->int_in_urb) {
		usb_free_urb(precvpriv->int_in_urb);
	}
#endif
	if(precvpriv->int_in_buf)
		rtw_mfree(precvpriv->int_in_buf, ini_in_buf_sz);
#endif /* CONFIG_USB_INTERRUPT_IN_PIPE */

#ifdef PLATFORM_LINUX

	if (skb_queue_len(&precvpriv->rx_skb_queue)) {
		DBG_8192C(KERN_WARNING "rx_skb_queue not empty\n");
	}

	rtw_skb_queue_purge(&precvpriv->rx_skb_queue);

	if (skb_queue_len(&precvpriv->free_recv_skb_queue)) {
		DBG_8192C(KERN_WARNING "free_recv_skb_queue not empty, %d\n", skb_queue_len(&precvpriv->free_recv_skb_queue));
	}

#if !defined(CONFIG_USE_USB_BUFFER_ALLOC_RX)
#if defined(CONFIG_PREALLOC_RECV_SKB) && defined(CONFIG_PREALLOC_RX_SKB_BUFFER)
	{
		struct sk_buff *skb;

		while ((skb = skb_dequeue(&precvpriv->free_recv_skb_queue)) != NULL) {
			if (rtw_free_skb_premem(skb) != 0)
				rtw_skb_free(skb);
		}
	}
#else
	rtw_skb_queue_purge(&precvpriv->free_recv_skb_queue);
#endif /* defined(CONFIG_PREALLOC_RX_SKB_BUFFER) && defined(CONFIG_PREALLOC_RECV_SKB) */
#endif /* !defined(CONFIG_USE_USB_BUFFER_ALLOC_RX) */

#endif /* PLATFORM_LINUX */

#ifdef PLATFORM_FREEBSD
	struct sk_buff  *pskb;
	while (NULL != (pskb = skb_dequeue(&precvpriv->rx_skb_queue))) {
		rtw_skb_free(pskb);
	}

#if !defined(CONFIG_USE_USB_BUFFER_ALLOC_RX)
	rtw_skb_queue_purge(&precvpriv->free_recv_skb_queue);
#endif

#ifdef CONFIG_RX_INDICATE_QUEUE
	struct mbuf *m;
	for (;;) {
		IF_DEQUEUE(&precvpriv->rx_indicate_queue, m);
		if (m == NULL)
			break;
		m_freem(m);
	}
	mtx_destroy(&precvpriv->rx_indicate_queue.ifq_mtx);
#endif /* CONFIG_RX_INDICATE_QUEUE */

#endif /* PLATFORM_FREEBSD */
}

#ifdef CONFIG_USB_SUPPORT_ASYNC_VDN_REQ
int usb_write_async(struct usb_device *udev, u32 addr, void *pdata, u16 len)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	int ret;

	requesttype = VENDOR_WRITE;//write_out
	request = REALTEK_USB_VENQT_CMD_REQ;
	index = REALTEK_USB_VENQT_CMD_IDX;//n/a

	wvalue = (u16)(addr&0x0000ffff);

	ret = _usbctrl_vendorreq_async_write(udev, request, wvalue, index, pdata, len, requesttype);

	return ret;
}

int usb_async_write8(struct intf_hdl *pintfhdl, u32 addr, u8 val)
{
	u8 data;
	int ret;
	struct dvobj_priv  *pdvobjpriv = (struct dvobj_priv  *)pintfhdl->pintf_dev;
	struct usb_device *udev=pdvobjpriv->pusbdev;

	_func_enter_;
	data = val;
	ret = usb_write_async(udev, addr, &data, 1);
	_func_exit_;

	return ret;
}

int usb_async_write16(struct intf_hdl *pintfhdl, u32 addr, u16 val)
{
	u16 data;
	int ret;
	struct dvobj_priv  *pdvobjpriv = (struct dvobj_priv  *)pintfhdl->pintf_dev;
	struct usb_device *udev=pdvobjpriv->pusbdev;

	_func_enter_;
	data = val;
	ret = usb_write_async(udev, addr, &data, 2);
	_func_exit_;

	return ret;
}

int usb_async_write32(struct intf_hdl *pintfhdl, u32 addr, u32 val)
{
	u32 data;
	int ret;
	struct dvobj_priv  *pdvobjpriv = (struct dvobj_priv  *)pintfhdl->pintf_dev;
	struct usb_device *udev=pdvobjpriv->pusbdev;

	_func_enter_;
	data = val;
	ret = usb_write_async(udev, addr, &data, 4);
	_func_exit_;

	return ret;
}
#endif /* CONFIG_USB_SUPPORT_ASYNC_VDN_REQ */



#ifdef CONFIG_RTL8192D
/*	This function only works in 92DU chip.		*/
void usb_read_reg_rf_byfw(struct intf_hdl *pintfhdl,
                          u16 byteCount,
                          u32 registerIndex,
                          PVOID buffer)
{
	u16	wPage = 0x0000, offset;
	u32	BufferLengthRead;
	PADAPTER	Adapter = pintfhdl->padapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8	RFPath=0,nPHY=0;

	RFPath =(u8) ((registerIndex&0xff0000)>>16);

	if (pHalData->interfaceIndex!=0) {
		nPHY = 1; //MAC1
		if(registerIndex&MAC1_ACCESS_PHY0)// MAC1 need to access PHY0
			nPHY = 0;
	} else {
		if(registerIndex&MAC0_ACCESS_PHY1)
			nPHY = 1;
	}
	registerIndex &= 0xFF;
	wPage = ((nPHY<<7)|(RFPath<<5)|8)<<8;
	offset = (u16)registerIndex;

	//
	// IN a vendor request to read back MAC register.
	//
	usbctrl_vendorreq(pintfhdl, 0x05, offset, wPage, buffer, byteCount, 0x01);

}
#endif

/*
	92DU chip needs to remask "value" parameter,  this function only works in 92DU chip.
*/
static inline void usb_value_remask(struct intf_hdl *pintfhdl, u16 *value)
{
#ifdef CONFIG_RTL8192D
	_adapter	*padapter = pintfhdl->padapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	if ((IS_HARDWARE_TYPE_8192DU(padapter)) && (pHalData->interfaceIndex!=0)) {
		if(*value<0x1000)
			*value|=0x4000;
		else if ((*value&MAC1_ACCESS_PHY0) && !(*value&0x8000))   // MAC1 need to access PHY0
			*value &= 0xFFF;
	}
#endif
}

u8 usb_read8(struct intf_hdl *pintfhdl, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u8 data=0;

	_func_enter_;

	request = 0x05;
	requesttype = 0x01;//read_in
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;
	usb_value_remask(pintfhdl, &wvalue);
	usbctrl_vendorreq(pintfhdl, request, wvalue, index,
	                  &data, len, requesttype);

	_func_exit_;

	return data;
}

u16 usb_read16(struct intf_hdl *pintfhdl, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u16 data=0;

	_func_enter_;

	request = 0x05;
	requesttype = 0x01;//read_in
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;
	usb_value_remask(pintfhdl, &wvalue);
	usbctrl_vendorreq(pintfhdl, request, wvalue, index,
	                  &data, len, requesttype);

	_func_exit_;

	return data;

}

u32 usb_read32(struct intf_hdl *pintfhdl, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data=0;

	_func_enter_;

	request = 0x05;
	requesttype = 0x01;//read_in
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;
#ifdef CONFIG_RTL8192D
	if ((IS_HARDWARE_TYPE_8192DU(pintfhdl->padapter)) && ((addr&0xff000000)>>24 == 0x66)) {
		usb_read_reg_rf_byfw(pintfhdl, len, addr, &data);
	} else
#endif
	{
		usb_value_remask(pintfhdl, &wvalue);
		usbctrl_vendorreq(pintfhdl, request, wvalue, index,
		                  &data, len, requesttype);
	}

	_func_exit_;

	return data;
}

int usb_write8(struct intf_hdl *pintfhdl, u32 addr, u8 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u8 data;
	int ret;

	_func_enter_;

	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;

	data = val;
	usb_value_remask(pintfhdl, &wvalue);
	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index,
	                        &data, len, requesttype);

	_func_exit_;

	return ret;
}

int usb_write16(struct intf_hdl *pintfhdl, u32 addr, u16 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u16 data;
	int ret;

	_func_enter_;

	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;

	data = val;
	usb_value_remask(pintfhdl, &wvalue);
	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index,
	                        &data, len, requesttype);

	_func_exit_;

	return ret;

}

int usb_write32(struct intf_hdl *pintfhdl, u32 addr, u32 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u32 data;
	int ret;

	_func_enter_;

	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;
	data =val;
	usb_value_remask(pintfhdl, &wvalue);
	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index,
	                        &data, len, requesttype);

	_func_exit_;

	return ret;

}

int usb_writeN(struct intf_hdl *pintfhdl, u32 addr, u32 length, u8 *pdata)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u8 buf[VENDOR_CMD_MAX_DATA_LEN]= {0};
	int ret;

	_func_enter_;

	request = 0x05;
	requesttype = 0x00;//write_out
	index = 0;//n/a

	wvalue = (u16)(addr&0x0000ffff);
	len = length;
	_rtw_memcpy(buf, pdata, len );
	usb_value_remask(pintfhdl, &wvalue);
	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index,
	                        buf, len, requesttype);

	_func_exit_;

	return ret;

}
