/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
/*-------------------------------------------------------------------------------

	For type defines and data structure defines

--------------------------------------------------------------------------------*/


#ifndef __DRV_TYPES_H__
#define __DRV_TYPES_H__

#include <drv_conf.h>
#include <basic_types.h>
#include <osdep_service.h>
#include <rtw_byteorder.h>
#include <wlan_bssdef.h>
#include <wifi.h>
#include <ieee80211.h>


#ifdef PLATFORM_OS_XP
#include <drv_types_xp.h>
#endif

#ifdef PLATFORM_OS_CE
#include <drv_types_ce.h>
#endif

#ifdef PLATFORM_LINUX
#include <drv_types_linux.h>
#endif

enum _NIC_VERSION {

	RTL8711_NIC,
	RTL8712_NIC,
	RTL8713_NIC,
	RTL8716_NIC

};


typedef struct _ADAPTER _adapter, ADAPTER,*PADAPTER;

#include <rtw_debug.h>

#ifdef CONFIG_80211N_HT
#include <rtw_ht.h>
#endif

#ifdef CONFIG_80211AC_VHT
#include <rtw_vht.h>
#endif

#ifdef CONFIG_INTEL_WIDI
#include <rtw_intel_widi.h>
#endif

#include <rtw_rf.h>
#include <rtw_cmd.h>
#include <cmd_osdep.h>
#include <rtw_security.h>
#include <rtw_xmit.h>
#include <xmit_osdep.h>
#include <rtw_recv.h>
#include <recv_osdep.h>
#include <rtw_efuse.h>
#include <rtw_sreset.h>
#include <hal_intf.h>
#include <hal_com.h>
#include <hal_com_led.h>
#include <rtw_qos.h>
#include <rtw_pwrctrl.h>
#include <rtw_mlme.h>
#include <mlme_osdep.h>
#include <rtw_io.h>
#include <rtw_ioctl.h>
#include <rtw_ioctl_set.h>
#include <rtw_ioctl_query.h>
#include <rtw_ioctl_rtl.h>
#include <osdep_intf.h>
#include <rtw_eeprom.h>
#include <sta_info.h>
#include <rtw_event.h>
#include <rtw_mlme_ext.h>
#include <rtw_ap.h>
#include <rtw_efuse.h>
#include <rtw_version.h>

#ifdef CONFIG_P2P
#include <rtw_p2p.h>
#endif // CONFIG_P2P

#ifdef CONFIG_TDLS
#include <rtw_tdls.h>
#endif // CONFIG_TDLS

#ifdef CONFIG_WAPI_SUPPORT
#include <rtw_wapi.h>
#endif // CONFIG_WAPI_SUPPORT

#ifdef CONFIG_DRVEXT_MODULE
#include <drvext_api.h>
#endif // CONFIG_DRVEXT_MODULE

#ifdef CONFIG_MP_INCLUDED
#include <rtw_mp.h>
#endif // CONFIG_MP_INCLUDED

#ifdef CONFIG_BR_EXT
#include <rtw_br_ext.h>
#endif // CONFIG_BR_EXT

#ifdef CONFIG_IOL
#include <rtw_iol.h>
#endif // CONFIG_IOL

#ifdef CONFIG_BT_COEXIST
#include <rtl8723a_hal.h>
#endif // CONFIG_BT_COEXIST

#ifdef CONFIG_IOCTL_CFG80211
#include "ioctl_cfg80211.h"
#endif //CONFIG_IOCTL_CFG80211

#include <ip.h>
#include <if_ether.h>
#include <ethernet.h>
#include <circ_buf.h>

#include <rtw_android.h>

#define SPEC_DEV_ID_NONE BIT(0)
#define SPEC_DEV_ID_DISABLE_HT BIT(1)
#define SPEC_DEV_ID_ENABLE_PS BIT(2)
#define SPEC_DEV_ID_RF_CONFIG_1T1R BIT(3)
#define SPEC_DEV_ID_RF_CONFIG_2T2R BIT(4)
#define SPEC_DEV_ID_ASSIGN_IFNAME BIT(5)

struct specific_device_id{

	u32		flags;

	u16		idVendor;
	u16		idProduct;

};

struct registry_priv
{
	u8	chip_version;
	u8	rfintfs;
	u8	lbkmode;
	u8	hci;
	NDIS_802_11_SSID	ssid;
	u8	network_mode;	//infra, ad-hoc, auto
	u8	channel;//ad-hoc support requirement
	u8	wireless_mode;//A, B, G, auto
	u8 	scan_mode;//active, passive
	u8	radio_enable;
	u8	preamble;//long, short, auto
	u8	vrtl_carrier_sense;//Enable, Disable, Auto
	u8	vcs_type;//RTS/CTS, CTS-to-self
	u16	rts_thresh;
	u16  frag_thresh;
	u8	adhoc_tx_pwr;
	u8	soft_ap;
	u8	power_mgnt;
	u8	ips_mode;
	u8	smart_ps;
	u8	long_retry_lmt;
	u8	short_retry_lmt;
	u16	busy_thresh;
	u8	ack_policy;
	u8	mp_mode;
	u8	software_encrypt;
	u8	software_decrypt;
	#ifdef CONFIG_TX_EARLY_MODE
	u8   early_mode;
	#endif
	u8	acm_method;
	  //UAPSD
	u8	wmm_enable;
	u8	uapsd_enable;
	u8	uapsd_max_sp;
	u8	uapsd_acbk_en;
	u8	uapsd_acbe_en;
	u8	uapsd_acvi_en;
	u8	uapsd_acvo_en;

	WLAN_BSSID_EX    dev_network;

#ifdef CONFIG_80211N_HT
	u8	ht_enable;
	// 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160MHz
	// 2.4G use bit 0 ~ 3, 5G use bit 4 ~ 7
	// 0x21 means enable 2.4G 40MHz & 5G 80MHz
	u8	bw_mode;
	u8	ampdu_enable;//for tx
	u8 	rx_stbc;
	u8	ampdu_amsdu;//A-MPDU Supports A-MSDU is permitted
	// Short GI support Bit Map
	// BIT0 - 20MHz, 1: support, 0: non-support
	// BIT1 - 40MHz, 1: support, 0: non-support
	// BIT2 - 80MHz, 1: support, 0: non-support
	// BIT3 - 160MHz, 1: support, 0: non-support
	u8	short_gi;
#endif //CONFIG_80211N_HT

#ifdef CONFIG_80211AC_VHT
	u8	vht_enable;
	u8	ampdu_factor;
	u8	vht_rate_sel;
	// BIT0: Enable VHT LDPC Rx, BIT1: Enable VHT LDPC Tx, BIT4: Enable HT LDPC Rx, BIT5: Enable HT LDPC Tx
	u8	ldpc_cap;
	// BIT0: Enable VHT STBC Rx, BIT1: Enable VHT STBC Tx, BIT4: Enable HT STBC Rx, BIT5: Enable HT STBC Tx
	u8	stbc_cap;
	// BIT0: Enable VHT Beamformer, BIT1: Enable VHT Beamformee, BIT4: Enable HT Beamformer, BIT5: Enable HT Beamformee
	u8	beamform_cap;
#endif //CONFIG_80211AC_VHT

	u8	lowrate_two_xmit;

	u8	rf_config ;
	u8	low_power ;

	u8	wifi_spec;// !turbo_mode

	u8	channel_plan;
#ifdef CONFIG_BT_COEXIST
	u8	btcoex;
	u8	bt_iso;
	u8	bt_sco;
	u8	bt_ampdu;
#endif
	BOOLEAN	bAcceptAddbaReq;

	u8	antdiv_cfg;
	u8	antdiv_type;

	u8	usbss_enable;//0:disable,1:enable
	u8	hwpdn_mode;//0:disable,1:enable,2:decide by EFUSE config
	u8	hwpwrp_detect;//0:disable,1:enable

	u8	hw_wps_pbc;//0:disable,1:enable

#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
	char	adaptor_info_caching_file_path[PATH_LENGTH_MAX];
#endif

#ifdef CONFIG_LAYER2_ROAMING
	u8	max_roaming_times; // the max number driver will try to roaming
#endif

#ifdef CONFIG_IOL
	u8 fw_iol; //enable iol without other concern
#endif

#ifdef CONFIG_DUALMAC_CONCURRENT
	u8	dmsp;//0:disable,1:enable
#endif

#ifdef CONFIG_80211D
	u8 enable80211d;
#endif

	u8 ifname[16];
	u8 if2name[16];

	u8 notch_filter;

#ifdef CONFIG_SPECIAL_SETTING_FOR_FUNAI_TV
	u8 force_ant;//0 normal,1 main,2 aux
	u8 force_igi;//0 normal
#endif
	u8 regulatory_tid;

	//define for tx power adjust
	u32	RegTxPwrLimit;
	u8	RegEnableTxPowerLimit;
	u8	RegPowerBase;
	u8	RegPwrTblSel;
	u8	RegPwrByRate;
	s8	TxBBSwing_2G;
	s8	TxBBSwing_5G;
	u8	AmplifierType_2G;
	u8	AmplifierType_5G;
	u8	bEn_RFE;
	u8	RFE_Type;

#ifdef CONFIG_MULTI_VIR_IFACES
	u8 ext_iface_num;//primary/secondary iface is excluded
#endif

#ifdef CONFIG_SW_LED
	u8 led_enable;
#endif
};


//For registry parameters
#define RGTRY_OFT(field) ((ULONG)FIELD_OFFSET(struct registry_priv,field))
#define RGTRY_SZ(field)   sizeof(((struct registry_priv*) 0)->field)
#define BSSID_OFT(field) ((ULONG)FIELD_OFFSET(WLAN_BSSID_EX,field))
#define BSSID_SZ(field)   sizeof(((PWLAN_BSSID_EX) 0)->field)

#define MAX_CONTINUAL_URB_ERR 4

#ifdef CONFIG_SDIO_HCI
#include <drv_types_sdio.h>
#define INTF_DATA SDIO_DATA
#elif defined(CONFIG_GSPI_HCI)
#include <drv_types_gspi.h>
#define INTF_DATA GSPI_DATA
#elif defined(CONFIG_PCI_HCI)
#include <drv_types_pci.h>
#endif

#define GET_PRIMARY_ADAPTER(padapter) (((_adapter *)padapter)->dvobj->if1)
#define GET_IFACE_NUMS(padapter) (((_adapter *)padapter)->dvobj->iface_nums)
#define GET_ADAPTER(padapter, iface_id) (((_adapter *)padapter)->dvobj->padapters[iface_id])

enum _IFACE_ID {
	IFACE_ID0, //maping to PRIMARY_ADAPTER
	IFACE_ID1, //maping to SECONDARY_ADAPTER
	IFACE_ID2,
	IFACE_ID3,
	IFACE_ID_MAX,
};

struct dvobj_priv
{
        _adapter *if1; //PRIMARY_ADAPTER
	_adapter *if2; //SECONDARY_ADAPTER

	//for local/global synchronization
	//
	_lock	lock;
	int macid[NUM_STA];

	_mutex hw_init_mutex;
	_mutex h2c_fwcmd_mutex;
	_mutex setch_mutex;
	_mutex setbw_mutex;

	unsigned char	oper_channel; //saved channel info when call set_channel_bw
	unsigned char	oper_bwmode;
	unsigned char	oper_ch_offset;//PRIME_CHNL_OFFSET

	//extend to support mulitu interface
	//padapters[IFACE_ID0] == if1
	//padapters[IFACE_ID1] == if2
	_adapter *padapters[IFACE_ID_MAX];
	u8 iface_nums; // total number of ifaces used runtime


	//For 92D, DMDP have 2 interface.
	u8	InterfaceNumber;
	u8	NumInterfaces;

	//In /Out Pipe information
	int	RtInPipe[2];
	int	RtOutPipe[3];
	u8	Queue2Pipe[HW_QUEUE_ENTRY];//for out pipe mapping

	u8	irq_alloc;

/*-------- below is for SDIO INTERFACE --------*/

#ifdef INTF_DATA
	INTF_DATA intf_data;
#endif

/*-------- below is for USB INTERFACE --------*/

#ifdef CONFIG_USB_HCI

	u8	usb_speed; // 1.1, 2.0 or 3.0
	u8	nr_endpoint;
	u8	RtNumInPipes;
	u8	RtNumOutPipes;
	int	ep_num[6]; //endpoint number

	int	RegUsbSS;

	_sema	usb_suspend_sema;

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	_mutex  usb_vendor_req_mutex;
#endif

#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	u8 * usb_alloc_vendor_req_buf;
	u8 * usb_vendor_req_buf;
#endif

#ifdef PLATFORM_WINDOWS
	//related device objects
	PDEVICE_OBJECT	pphysdevobj;//pPhysDevObj;
	PDEVICE_OBJECT	pfuncdevobj;//pFuncDevObj;
	PDEVICE_OBJECT	pnextdevobj;//pNextDevObj;

	u8	nextdevstacksz;//unsigned char NextDeviceStackSize;	//= (CHAR)CEdevice->pUsbDevObj->StackSize + 1;

	//urb for control diescriptor request

#ifdef PLATFORM_OS_XP
	struct _URB_CONTROL_DESCRIPTOR_REQUEST descriptor_urb;
	PUSB_CONFIGURATION_DESCRIPTOR	pconfig_descriptor;//UsbConfigurationDescriptor;
#endif

#ifdef PLATFORM_OS_CE
	WCHAR			active_path[MAX_ACTIVE_REG_PATH];	// adapter regpath
	USB_EXTENSION	usb_extension;

	_nic_hdl		pipehdls_r8192c[0x10];
#endif

	u32	config_descriptor_len;//ULONG UsbConfigurationDescriptorLength;
#endif//PLATFORM_WINDOWS

#ifdef PLATFORM_LINUX
	struct usb_interface *pusbintf;
	struct usb_device *pusbdev;
#endif//PLATFORM_LINUX

#ifdef PLATFORM_FREEBSD
	struct usb_interface *pusbintf;
	struct usb_device *pusbdev;
#endif//PLATFORM_FREEBSD
	ATOMIC_T continual_urb_error;
#endif//CONFIG_USB_HCI

/*-------- below is for PCIE INTERFACE --------*/

#ifdef CONFIG_PCI_HCI

#ifdef PLATFORM_LINUX
	struct pci_dev *ppcidev;

	//PCI MEM map
	unsigned long	pci_mem_end;	/* shared mem end	*/
	unsigned long	pci_mem_start;	/* shared mem start	*/

	//PCI IO map
	unsigned long	pci_base_addr;	/* device I/O address	*/

	//PciBridge
	struct pci_priv	pcipriv;

	u16	irqline;
	u8	irq_enabled;
	RT_ISR_CONTENT	isr_content;
	_lock	irq_th_lock;

	//ASPM
	u8	const_pci_aspm;
	u8	const_amdpci_aspm;
	u8	const_hwsw_rfoff_d3;
	u8	const_support_pciaspm;
	// pci-e bridge */
	u8 	const_hostpci_aspm_setting;
	// pci-e device */
	u8 	const_devicepci_aspm_setting;
	u8 	b_support_aspm; // If it supports ASPM, Offset[560h] = 0x40, otherwise Offset[560h] = 0x00.
	u8	b_support_backdoor;
	u8 bdma64;
#endif//PLATFORM_LINUX

#endif//CONFIG_PCI_HCI
};

#ifdef PLATFORM_LINUX
static struct device *dvobj_to_dev(struct dvobj_priv *dvobj)
{
	/* todo: get interface type from dvobj and the return the dev accordingly */
#ifdef RTW_DVOBJ_CHIP_HW_TYPE
#endif

#ifdef CONFIG_USB_HCI
	return &dvobj->pusbintf->dev;
#endif
#ifdef CONFIG_SDIO_HCI
	return &dvobj->intf_data.func->dev;
#endif
#ifdef CONFIG_GSPI_HCI
	return &dvobj->intf_data.func->dev;
#endif
#ifdef CONFIG_PCI_HCI
	return &dvobj->ppcidev->dev;
#endif
}
#endif

enum _IFACE_TYPE {
	IFACE_PORT0, //mapping to port0 for C/D series chips
	IFACE_PORT1, //mapping to port1 for C/D series chip
	MAX_IFACE_PORT,
};

enum _ADAPTER_TYPE {
	PRIMARY_ADAPTER,
	SECONDARY_ADAPTER,
	MAX_ADAPTER = 0xFF,
};

typedef enum _DRIVER_STATE{
	DRIVER_NORMAL = 0,
	DRIVER_DISAPPEAR = 1,
	DRIVER_REPLACE_DONGLE = 2,
}DRIVER_STATE;

#ifdef CONFIG_INTEL_PROXIM
struct proxim {
	bool proxim_support;
	bool proxim_on;

	void *proximity_priv;
	int (*proxim_rx)(_adapter *padapter,
		union recv_frame *precv_frame);
	u8	(*proxim_get_var)(_adapter* padapter, u8 type);
};
#endif	//CONFIG_INTEL_PROXIM

#ifdef CONFIG_MAC_LOOPBACK_DRIVER
typedef struct loopbackdata
{
	_sema	sema;
	_thread_hdl_ lbkthread;
	u8 bstop;
	u32 cnt;
	u16 size;
	u16 txsize;
	u8 txbuf[0x8000];
	u16 rxsize;
	u8 rxbuf[0x8000];
	u8 msg[100];

}LOOPBACKDATA, *PLOOPBACKDATA;
#endif

struct _ADAPTER{
	int	DriverState;// for disable driver using module, use dongle to replace module.
	int	pid[3];//process id from UI, 0:wps, 1:hostapd, 2:dhcpcd
	int	bDongle;//build-in module or external dongle
	u16 	chip_type;
	u16	HardwareType;
	u16	interface_type;//USB,SDIO,SPI,PCI

	struct dvobj_priv *dvobj;
	struct	mlme_priv mlmepriv;
	struct	mlme_ext_priv mlmeextpriv;
	struct	cmd_priv	cmdpriv;
	struct	evt_priv	evtpriv;
	//struct	io_queue	*pio_queue;
	struct 	io_priv	iopriv;
	struct	xmit_priv	xmitpriv;
	struct	recv_priv	recvpriv;
	struct	sta_priv	stapriv;
	struct	security_priv	securitypriv;
	struct	registry_priv	registrypriv;
	struct	pwrctrl_priv	pwrctrlpriv;
	struct 	eeprom_priv eeprompriv;
	struct	led_priv	ledpriv;

#ifdef CONFIG_MP_INCLUDED
       struct	mp_priv	mppriv;
#endif

#ifdef CONFIG_DRVEXT_MODULE
	struct	drvext_priv	drvextpriv;
#endif

#ifdef CONFIG_AP_MODE
	struct	hostapd_priv	*phostapdpriv;
#endif

#ifdef CONFIG_IOCTL_CFG80211
#ifdef CONFIG_P2P
	struct cfg80211_wifidirect_info	cfg80211_wdinfo;
#endif //CONFIG_P2P
#endif //CONFIG_IOCTL_CFG80211
	u32	setband;
#ifdef CONFIG_P2P
	struct wifidirect_info	wdinfo;
#endif //CONFIG_P2P

#ifdef CONFIG_TDLS
	struct tdls_info	tdlsinfo;
#endif //CONFIG_TDLS

#ifdef CONFIG_WAPI_SUPPORT
	u8	WapiSupport;
	RT_WAPI_T	wapiInfo;
#endif


#ifdef CONFIG_WFD
	struct wifi_display_info wfd_info;
#endif //CONFIG_WFD

	PVOID			HalData;
	u32 hal_data_sz;
	struct hal_ops	HalFunc;

	s32	bDriverStopped;
	s32	bSurpriseRemoved;
	s32  bCardDisableWOHSM;

	u32	IsrContent;
	u32	ImrContent;

	u8	EepromAddressSize;
	u8	hw_init_completed;
	u8	bDriverIsGoingToUnload;
	u8	init_adpt_in_progress;
	u8	bHaltInProgress;

	_thread_hdl_ cmdThread;
	_thread_hdl_ evtThread;
	_thread_hdl_ xmitThread;
	_thread_hdl_ recvThread;

#ifndef PLATFORM_LINUX
	NDIS_STATUS (*dvobj_init)(struct dvobj_priv *dvobj);
	void (*dvobj_deinit)(struct dvobj_priv *dvobj);
#endif

	void (*intf_start)(_adapter * adapter);
	void (*intf_stop)(_adapter * adapter);

#ifdef PLATFORM_WINDOWS
	_nic_hdl		hndis_adapter;//hNdisAdapter(NDISMiniportAdapterHandle);
	_nic_hdl		hndis_config;//hNdisConfiguration;
	NDIS_STRING fw_img;

	u32	NdisPacketFilter;
	u8	MCList[MAX_MCAST_LIST_NUM][6];
	u32	MCAddrCount;
#endif //end of PLATFORM_WINDOWS


#ifdef PLATFORM_LINUX
	_nic_hdl pnetdev;

	// used by rtw_rereg_nd_name related function
	struct rereg_nd_name_data {
		_nic_hdl old_pnetdev;
		char old_ifname[IFNAMSIZ];
		u8 old_ips_mode;
		u8 old_bRegUseLed;
	} rereg_nd_name_priv;

	int bup;
	struct net_device_stats stats;
	struct iw_statistics iwstats;
	struct proc_dir_entry *dir_dev;// for proc directory

#ifdef CONFIG_IOCTL_CFG80211
	struct wireless_dev *rtw_wdev;
#endif //CONFIG_IOCTL_CFG80211

#endif //end of PLATFORM_LINUX

#ifdef PLATFORM_FREEBSD
	_nic_hdl pifp;
	int bup;
	_lock glock;
#endif //PLATFORM_FREEBSD
	int net_closed;

	u8 bFWReady;
	u8 bBTFWReady;
	u8 bReadPortCancel;
	u8 bWritePortCancel;
	u8 bLinkInfoDump;
	//	Added by Albert 2012/10/26
	//	The driver will show up the desired channel number when this flag is 1.
	u8 bNotifyChannelChange;
#ifdef CONFIG_P2P
	//	Added by Albert 2012/12/06
	//	The driver will show the current P2P status when the upper application reads it.
	u8 bShowGetP2PState;
#endif
#ifdef CONFIG_AUTOSUSPEND
	u8	bDisableAutosuspend;
#endif

	//pbuddy_adapter is used only in  two inteface case, (iface_nums=2 in struct dvobj_priv)
	//PRIMARY_ADAPTER's buddy is SECONDARY_ADAPTER
	//SECONDARY_ADAPTER's buddy is PRIMARY_ADAPTER
	//for iface_id > SECONDARY_ADAPTER(IFACE_ID1), refer to padapters[iface_id]  in struct dvobj_priv
	//and their pbuddy_adapter is PRIMARY_ADAPTER.
	//for PRIMARY_ADAPTER(IFACE_ID0) can directly refer to if1 in struct dvobj_priv
	_adapter *pbuddy_adapter;

#if defined(CONFIG_CONCURRENT_MODE) || defined(CONFIG_DUALMAC_CONCURRENT)
	u8 isprimary; //is primary adapter or not
	//notes:
	// if isprimary is true, the adapter_type value is 0, iface_id is IFACE_ID0 for PRIMARY_ADAPTER
	// if isprimary is false, the adapter_type value is 1, iface_id is IFACE_ID1 for SECONDARY_ADAPTER
	// refer to iface_id if iface_nums>2 and isprimary is false and the adapter_type value is 0xff.
	u8 adapter_type;//used only in  two inteface case(PRIMARY_ADAPTER and SECONDARY_ADAPTER) .
	u8 iface_type; //interface port type, it depends on HW port
#endif //CONFIG_CONCURRENT_MODE || CONFIG_DUALMAC_CONCURRENT

	//extend to support multi interface
       //IFACE_ID0 is equals to PRIMARY_ADAPTER
       //IFACE_ID1 is equals to SECONDARY_ADAPTER
	u8 iface_id;

#ifdef CONFIG_DUALMAC_CONCURRENT
	u8 DualMacConcurrent; // 1: DMSP 0:DMDP
#endif

#ifdef CONFIG_BR_EXT
	_lock					br_ext_lock;
	//unsigned int			macclone_completed;
	struct nat25_network_db_entry	*nethash[NAT25_HASH_SIZE];
	int				pppoe_connection_in_progress;
	unsigned char			pppoe_addr[MACADDRLEN];
	unsigned char			scdb_mac[MACADDRLEN];
	unsigned char			scdb_ip[4];
	struct nat25_network_db_entry	*scdb_entry;
	unsigned char			br_mac[MACADDRLEN];
	unsigned char			br_ip[4];

	struct br_ext_info		ethBrExtInfo;
#endif	// CONFIG_BR_EXT

#ifdef CONFIG_INTEL_PROXIM
	/* intel Proximity, should be alloc mem
	 * in intel Proximity module and can only
	 * be used in intel Proximity mode */
	struct proxim proximity;
#endif	//CONFIG_INTEL_PROXIM

#ifdef CONFIG_MAC_LOOPBACK_DRIVER
	PLOOPBACKDATA ploopback;
#endif

        u8    fix_rate;

	unsigned char     in_cta_test;

};

#define adapter_to_dvobj(adapter) (adapter->dvobj)

int rtw_handle_dualmac(_adapter *adapter, bool init);

__inline static u8 *myid(struct eeprom_priv *peepriv)
{
	return (peepriv->mac_addr);
}

// HCI Related header file
#ifdef CONFIG_USB_HCI
#include <usb_osintf.h>
#include <usb_ops.h>
#include <usb_hal.h>
#endif

#ifdef CONFIG_SDIO_HCI
#include <sdio_osintf.h>
#include <sdio_ops.h>
#include <sdio_hal.h>
#endif

#ifdef CONFIG_GSPI_HCI
#include <gspi_osintf.h>
#include <gspi_ops.h>
#include <gspi_hal.h>
#endif

#ifdef CONFIG_PCI_HCI
#include <pci_osintf.h>
#include <pci_ops.h>
#include <pci_hal.h>
#endif


#endif //__DRV_TYPES_H__

