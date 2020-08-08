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
#define _IOCTL_LINUX_C_

#include <drv_types.h>

//#ifdef CONFIG_MP_INCLUDED
#include <rtw_mp.h>
#include <rtw_mp_ioctl.h>
#include "../../hal/OUTSRC/phydm_precomp.h"
//#endif

#if defined(CONFIG_RTL8723A)
#include "rtl8723a_hal.h"
#include <rtw_bt_mp.h>
#endif

#if defined(CONFIG_RTL8723B)
#include <rtw_bt_mp.h>
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27))
#define  iwe_stream_add_event(a, b, c, d, e)  iwe_stream_add_event(b, c, d, e)
#define  iwe_stream_add_point(a, b, c, d, e)  iwe_stream_add_point(b, c, d, e)
#endif

#ifdef CONFIG_80211N_HT
extern int rtw_ht_enable;
#endif


#define RTL_IOCTL_WPA_SUPPLICANT	SIOCIWFIRSTPRIV+30

#define SCAN_ITEM_SIZE 768
#define MAX_CUSTOM_LEN 64
#define RATE_COUNT 4

#ifdef CONFIG_GLOBAL_UI_PID
extern int ui_pid[3];
#endif

// combo scan
#define WEXT_CSCAN_AMOUNT 9
#define WEXT_CSCAN_BUF_LEN		360
#define WEXT_CSCAN_HEADER		"CSCAN S\x01\x00\x00S\x00"
#define WEXT_CSCAN_HEADER_SIZE		12
#define WEXT_CSCAN_SSID_SECTION		'S'
#define WEXT_CSCAN_CHANNEL_SECTION	'C'
#define WEXT_CSCAN_NPROBE_SECTION	'N'
#define WEXT_CSCAN_ACTV_DWELL_SECTION	'A'
#define WEXT_CSCAN_PASV_DWELL_SECTION	'P'
#define WEXT_CSCAN_HOME_DWELL_SECTION	'H'
#define WEXT_CSCAN_TYPE_SECTION		'T'


extern u8 key_2char2num(u8 hch, u8 lch);
extern u8 str_2char2num(u8 hch, u8 lch);
extern void macstr2num(u8 *dst, u8 *src);
extern u8 convert_ip_addr(u8 hch, u8 mch, u8 lch);

u32 rtw_rates[] = {1000000,2000000,5500000,11000000,
                   6000000,9000000,12000000,18000000,24000000,36000000,48000000,54000000
                  };

static const char * const iw_operation_mode[] = {
	"Auto", "Ad-Hoc", "Managed",  "Master", "Repeater", "Secondary", "Monitor"
};

#ifdef CONFIG_WIRELESS_EXT

static int hex2num_i(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int hex2byte_i(const char *hex)
{
	int a, b;
	a = hex2num_i(*hex++);
	if (a < 0)
		return -1;
	b = hex2num_i(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}

/**
 * hwaddr_aton - Convert ASCII string to MAC address
 * @txt: MAC address as a string (e.g., "00:11:22:33:44:55")
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * Returns: 0 on success, -1 on failure (e.g., string not a MAC address)
 */
static int hwaddr_aton_i(const char *txt, u8 *addr)
{
	int i;

	for (i = 0; i < 6; i++) {
		int a, b;

		a = hex2num_i(*txt++);
		if (a < 0)
			return -1;
		b = hex2num_i(*txt++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
		if (i < 5 && *txt++ != ':')
			return -1;
	}

	return 0;
}
#endif

static inline void indicate_wx_custom_event(_adapter *padapter, char *msg)
{
	u8 *buff;
	union iwreq_data wrqu;

	if (strlen(msg) > IW_CUSTOM_MAX) {
		DBG_871X("%s strlen(msg):%zu > IW_CUSTOM_MAX:%u\n", __FUNCTION__ , strlen(msg), IW_CUSTOM_MAX);
		return;
	}

	buff = rtw_zmalloc(IW_CUSTOM_MAX+1);
	if(!buff)
		return;

	_rtw_memcpy(buff, msg, strlen(msg));

	_rtw_memset(&wrqu,0,sizeof(wrqu));
	wrqu.data.length = strlen(msg);

	DBG_871X("%s %s\n", __FUNCTION__, buff);
#ifndef CONFIG_IOCTL_CFG80211
	wireless_send_event(padapter->pnetdev, IWEVCUSTOM, &wrqu, buff);
#endif

	rtw_mfree(buff, IW_CUSTOM_MAX+1);

}


static inline void request_wps_pbc_event(_adapter *padapter)
{
	u8 *buff, *p;
	union iwreq_data wrqu;


	buff = rtw_malloc(IW_CUSTOM_MAX);
	if(!buff)
		return;

	_rtw_memset(buff, 0, IW_CUSTOM_MAX);

	p=buff;

	p+=sprintf(p, "WPS_PBC_START.request=TRUE");

	_rtw_memset(&wrqu,0,sizeof(wrqu));

	wrqu.data.length = p-buff;

	wrqu.data.length = (wrqu.data.length<IW_CUSTOM_MAX) ? wrqu.data.length:IW_CUSTOM_MAX;

	DBG_871X("%s\n", __FUNCTION__);

#ifndef CONFIG_IOCTL_CFG80211
	wireless_send_event(padapter->pnetdev, IWEVCUSTOM, &wrqu, buff);
#endif

	if(buff) {
		rtw_mfree(buff, IW_CUSTOM_MAX);
	}

}

#ifdef CONFIG_SUPPORT_HW_WPS_PBC
void rtw_request_wps_pbc_event(_adapter *padapter)
{
#ifdef RTK_DMP_PLATFORM
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12))
	kobject_uevent(&padapter->pnetdev->dev.kobj, KOBJ_NET_PBC);
#else
	kobject_hotplug(&padapter->pnetdev->class_dev.kobj, KOBJ_NET_PBC);
#endif
#else

	if ( padapter->pid[0] == 0 ) {
		//	0 is the default value and it means the application monitors the HW PBC doesn't privde its pid to driver.
		return;
	}

	rtw_signal_process(padapter->pid[0], SIGUSR1);

#endif

	rtw_led_control(padapter, LED_CTL_START_WPS_BOTTON);
}
#endif//#ifdef CONFIG_SUPPORT_HW_WPS_PBC

void indicate_wx_scan_complete_event(_adapter *padapter)
{
	union iwreq_data wrqu;
	//struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

	_rtw_memset(&wrqu, 0, sizeof(union iwreq_data));

	//DBG_871X("+rtw_indicate_wx_scan_complete_event\n");
#ifndef CONFIG_IOCTL_CFG80211
	wireless_send_event(padapter->pnetdev, SIOCGIWSCAN, &wrqu, NULL);
#endif
}


void rtw_indicate_wx_assoc_event(_adapter *padapter)
{
	union iwreq_data wrqu;
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*pnetwork = (WLAN_BSSID_EX*)(&(pmlmeinfo->network));

	_rtw_memset(&wrqu, 0, sizeof(union iwreq_data));

	wrqu.ap_addr.sa_family = ARPHRD_ETHER;

	if(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE)==_TRUE )
		_rtw_memcpy(wrqu.ap_addr.sa_data, pnetwork->MacAddress, ETH_ALEN);
	else
		_rtw_memcpy(wrqu.ap_addr.sa_data, pmlmepriv->cur_network.network.MacAddress, ETH_ALEN);

	DBG_871X_LEVEL(_drv_always_, "assoc success\n");
#ifndef CONFIG_IOCTL_CFG80211
	wireless_send_event(padapter->pnetdev, SIOCGIWAP, &wrqu, NULL);
#endif
}

void rtw_indicate_wx_disassoc_event(_adapter *padapter)
{
	union iwreq_data wrqu;

	_rtw_memset(&wrqu, 0, sizeof(union iwreq_data));

	wrqu.ap_addr.sa_family = ARPHRD_ETHER;
	_rtw_memset(wrqu.ap_addr.sa_data, 0, ETH_ALEN);

#ifndef CONFIG_IOCTL_CFG80211
	DBG_871X_LEVEL(_drv_always_, "indicate disassoc\n");
	wireless_send_event(padapter->pnetdev, SIOCGIWAP, &wrqu, NULL);
#endif
}

/*
uint	rtw_is_cckrates_included(u8 *rate)
{
		u32	i = 0;

		while(rate[i]!=0)
		{
			if  (  (((rate[i]) & 0x7f) == 2)	|| (((rate[i]) & 0x7f) == 4) ||
			(((rate[i]) & 0x7f) == 11)  || (((rate[i]) & 0x7f) == 22) )
			return _TRUE;
			i++;
		}

		return _FALSE;
}

uint	rtw_is_cckratesonly_included(u8 *rate)
{
	u32 i = 0;

	while(rate[i]!=0)
	{
			if  (  (((rate[i]) & 0x7f) != 2) && (((rate[i]) & 0x7f) != 4) &&
				(((rate[i]) & 0x7f) != 11)  && (((rate[i]) & 0x7f) != 22) )
			return _FALSE;
			i++;
	}

	return _TRUE;
}
*/

#ifdef CONFIG_WIRELESS_EXT

static int search_p2p_wfd_ie(_adapter *padapter,
                             struct iw_request_info* info, struct wlan_network *pnetwork,
                             char *start, char *stop)
{
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &padapter->wdinfo;
#ifdef CONFIG_WFD
	if ( SCAN_RESULT_ALL == pwdinfo->wfd_info->scan_result_type ) {

	} else if ( ( SCAN_RESULT_P2P_ONLY == pwdinfo->wfd_info->scan_result_type ) ||
	            ( SCAN_RESULT_WFD_TYPE == pwdinfo->wfd_info->scan_result_type ) )
#endif // CONFIG_WFD
	{
		if(!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)) {
			u32	blnGotP2PIE = _FALSE;

			//	User is doing the P2P device discovery
			//	The prefix of SSID should be "DIRECT-" and the IE should contains the P2P IE.
			//	If not, the driver should ignore this AP and go to the next AP.

			//	Verifying the SSID
			if ( _rtw_memcmp( pnetwork->network.Ssid.Ssid, pwdinfo->p2p_wildcard_ssid, P2P_WILDCARD_SSID_LEN ) ) {
				u32	p2pielen = 0;

				//	Verifying the P2P IE
				if (rtw_get_p2p_ie_from_scan_queue(&pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &p2pielen, pnetwork->network.Reserved[0])) {
					blnGotP2PIE = _TRUE;
				}
			}

			if ( blnGotP2PIE == _FALSE ) {
				return _FALSE;
			}

		}
	}

#ifdef CONFIG_WFD
	if ( SCAN_RESULT_WFD_TYPE == pwdinfo->wfd_info->scan_result_type ) {
		u32	blnGotWFD = _FALSE;
		u8	wfd_ie[ 128 ] = { 0x00 };
		uint	wfd_ielen = 0;

		if ( rtw_get_wfd_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength,  wfd_ie, &wfd_ielen, pnetwork->network.Reserved[0]) ) {
			u8	wfd_devinfo[ 6 ] = { 0x00 };
			uint	wfd_devlen = 6;

			if ( rtw_get_wfd_attr_content( wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, wfd_devinfo, &wfd_devlen) ) {
				if ( pwdinfo->wfd_info->wfd_device_type == WFD_DEVINFO_PSINK ) {
					//	the first two bits will indicate the WFD device type
					if ( ( wfd_devinfo[ 1 ] & 0x03 ) == WFD_DEVINFO_SOURCE ) {
						//	If this device is Miracast PSink device, the scan reuslt should just provide the Miracast source.
						blnGotWFD = _TRUE;
					}
				} else if ( pwdinfo->wfd_info->wfd_device_type == WFD_DEVINFO_SOURCE ) {
					//	the first two bits will indicate the WFD device type
					if ( ( wfd_devinfo[ 1 ] & 0x03 ) == WFD_DEVINFO_PSINK ) {
						//	If this device is Miracast source device, the scan reuslt should just provide the Miracast PSink.
						//	Todo: How about the SSink?!
						blnGotWFD = _TRUE;
					}
				}
			}
		}

		if ( blnGotWFD == _FALSE ) {
			return _FALSE;
		}
	}
#endif // CONFIG_WFD

#endif //CONFIG_P2P
	return _TRUE;
}
#endif

static inline char *iwe_stream_mac_addr_proess(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe)
{
	/*  AP MAC address  */
	iwe->cmd = SIOCGIWAP;
	iwe->u.ap_addr.sa_family = ARPHRD_ETHER;

	_rtw_memcpy(iwe->u.ap_addr.sa_data, pnetwork->network.MacAddress, ETH_ALEN);
	start = iwe_stream_add_event(info, start, stop, iwe, IW_EV_ADDR_LEN);
	return start;
}
static inline char * iwe_stream_essid_proess(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe)
{

	/* Add the ESSID */
	iwe->cmd = SIOCGIWESSID;
	iwe->u.data.flags = 1;
	iwe->u.data.length = min((u16)pnetwork->network.Ssid.SsidLength, (u16)32);
	start = iwe_stream_add_point(info, start, stop, iwe, pnetwork->network.Ssid.Ssid);
	return start;
}

static inline char * iwe_stream_chan_process(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe)
{
	if(pnetwork->network.Configuration.DSConfig<1 /*|| pnetwork->network.Configuration.DSConfig>14*/)
		pnetwork->network.Configuration.DSConfig = 1;

	/* Add frequency/channel */
	iwe->cmd = SIOCGIWFREQ;
	iwe->u.freq.m = rtw_ch2freq(pnetwork->network.Configuration.DSConfig) * 100000;
	iwe->u.freq.e = 1;
	iwe->u.freq.i = pnetwork->network.Configuration.DSConfig;
	start = iwe_stream_add_event(info, start, stop, iwe, IW_EV_FREQ_LEN);
	return start;
}
static inline char * iwe_stream_mode_process(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe,u16 cap)
{
	/* Add mode */
	if(cap & (WLAN_CAPABILITY_IBSS |WLAN_CAPABILITY_BSS)) {
		iwe->cmd = SIOCGIWMODE;
		if (cap & WLAN_CAPABILITY_BSS)
			iwe->u.mode = IW_MODE_MASTER;
		else
			iwe->u.mode = IW_MODE_ADHOC;

		start = iwe_stream_add_event(info, start, stop, iwe, IW_EV_UINT_LEN);
	}
	return start;
}
static inline char * iwe_stream_encryption_process(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe,u16 cap)
{

	/* Add encryption capability */
	iwe->cmd = SIOCGIWENCODE;
	if (cap & WLAN_CAPABILITY_PRIVACY)
		iwe->u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
	else
		iwe->u.data.flags = IW_ENCODE_DISABLED;
	iwe->u.data.length = 0;
	start = iwe_stream_add_point(info, start, stop, iwe, pnetwork->network.Ssid.Ssid);
	return start;

}

static inline char * iwe_stream_protocol_process(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe)
{
	u16 ht_cap=_FALSE,vht_cap = _FALSE;
	u32 ht_ielen = 0, vht_ielen = 0;
	char *p;
	u8 ie_offset = (pnetwork->network.Reserved[0] == 2? 0:12);// Probe Request

	//parsing HT_CAP_IE
	p = rtw_get_ie(&pnetwork->network.IEs[ie_offset], _HT_CAPABILITY_IE_, &ht_ielen, pnetwork->network.IELength-ie_offset);
	if(p && ht_ielen>0)
		ht_cap = _TRUE;

#ifdef CONFIG_80211AC_VHT
	//parsing VHT_CAP_IE
	p = rtw_get_ie(&pnetwork->network.IEs[ie_offset], EID_VHTCapability, &vht_ielen, pnetwork->network.IELength-ie_offset);
	if(p && vht_ielen>0)
		vht_cap = _TRUE;
#endif
	/* Add the protocol name */
	iwe->cmd = SIOCGIWNAME;
	if ((rtw_is_cckratesonly_included((u8*)&pnetwork->network.SupportedRates)) == _TRUE) {
		if(ht_cap == _TRUE)
			snprintf(iwe->u.name, IFNAMSIZ, "IEEE 802.11bn");
		else
			snprintf(iwe->u.name, IFNAMSIZ, "IEEE 802.11b");
	} else if ((rtw_is_cckrates_included((u8*)&pnetwork->network.SupportedRates)) == _TRUE) {
		if(ht_cap == _TRUE)
			snprintf(iwe->u.name, IFNAMSIZ, "IEEE 802.11bgn");
		else
			snprintf(iwe->u.name, IFNAMSIZ, "IEEE 802.11bg");
	} else {
		if(pnetwork->network.Configuration.DSConfig > 14) {
#ifdef CONFIG_80211AC_VHT
			if(vht_cap == _TRUE) {
				snprintf(iwe->u.name, IFNAMSIZ, "IEEE 802.11AC");
			} else
#endif
			{
				if(ht_cap == _TRUE)
					snprintf(iwe->u.name, IFNAMSIZ, "IEEE 802.11an");
				else
					snprintf(iwe->u.name, IFNAMSIZ, "IEEE 802.11a");
			}
		} else {
			if(ht_cap == _TRUE)
				snprintf(iwe->u.name, IFNAMSIZ, "IEEE 802.11gn");
			else
				snprintf(iwe->u.name, IFNAMSIZ, "IEEE 802.11g");
		}
	}
	start = iwe_stream_add_event(info, start, stop, iwe, IW_EV_CHAR_LEN);
	return start;
}

static inline char * iwe_stream_rate_process(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe)
{
	u32 ht_ielen = 0, vht_ielen = 0;
	char *p;
	u16 max_rate=0, rate, ht_cap=_FALSE, vht_cap = _FALSE;
	u32 i = 0;
	u8 bw_40MHz=0, short_GI=0, bw_160MHz=0, vht_highest_rate = 0;
	u16 mcs_rate=0, vht_data_rate=0;
	char custom[MAX_CUSTOM_LEN]= {0};
	u8 ie_offset = (pnetwork->network.Reserved[0] == 2? 0:12);// Probe Request

	//parsing HT_CAP_IE
	p = rtw_get_ie(&pnetwork->network.IEs[ie_offset], _HT_CAPABILITY_IE_, &ht_ielen, pnetwork->network.IELength-ie_offset);
	if(p && ht_ielen>0) {
		struct rtw_ieee80211_ht_cap *pht_capie;
		ht_cap = _TRUE;
		pht_capie = (struct rtw_ieee80211_ht_cap *)(p+2);
		_rtw_memcpy(&mcs_rate , pht_capie->supp_mcs_set, 2);
		bw_40MHz = (pht_capie->cap_info&IEEE80211_HT_CAP_SUP_WIDTH) ? 1:0;
		short_GI = (pht_capie->cap_info&(IEEE80211_HT_CAP_SGI_20|IEEE80211_HT_CAP_SGI_40)) ? 1:0;
	}

#ifdef CONFIG_80211AC_VHT
	//parsing VHT_CAP_IE
	p = rtw_get_ie(&pnetwork->network.IEs[ie_offset], EID_VHTCapability, &vht_ielen, pnetwork->network.IELength-ie_offset);
	if(p && vht_ielen>0) {
		u8	mcs_map[2];

		vht_cap = _TRUE;
		bw_160MHz = GET_VHT_CAPABILITY_ELE_CHL_WIDTH(p+2);
		if(bw_160MHz)
			short_GI = GET_VHT_CAPABILITY_ELE_SHORT_GI160M(p+2);
		else
			short_GI = GET_VHT_CAPABILITY_ELE_SHORT_GI80M(p+2);

		_rtw_memcpy(mcs_map, GET_VHT_CAPABILITY_ELE_TX_MCS(p+2), 2);

		vht_highest_rate = rtw_get_vht_highest_rate(mcs_map);
		vht_data_rate = rtw_vht_mcs_to_data_rate(CHANNEL_WIDTH_80, short_GI, vht_highest_rate);
	}
#endif

	/*Add basic and extended rates */
	p = custom;
	p += snprintf(p, MAX_CUSTOM_LEN - (p - custom), " Rates (Mb/s): ");
	while(pnetwork->network.SupportedRates[i]!=0) {
		rate = pnetwork->network.SupportedRates[i]&0x7F;
		if (rate > max_rate)
			max_rate = rate;
		p += snprintf(p, MAX_CUSTOM_LEN - (p - custom),
		              "%d%s ", rate >> 1, (rate & 1) ? ".5" : "");
		i++;
	}
#ifdef CONFIG_80211AC_VHT
	if(vht_cap == _TRUE) {
		max_rate = vht_data_rate;
	} else
#endif
		if(ht_cap == _TRUE) {
			if(mcs_rate&0x8000) { //MCS15
				max_rate = (bw_40MHz) ? ((short_GI)?300:270):((short_GI)?144:130);

			} else if(mcs_rate&0x0080) { //MCS7
				max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
			} else { //default MCS7
				//DBG_871X("wx_get_scan, mcs_rate_bitmap=0x%x\n", mcs_rate);
				max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
			}

			max_rate = max_rate*2;//Mbps/2;
		}

	iwe->cmd = SIOCGIWRATE;
	iwe->u.bitrate.fixed = iwe->u.bitrate.disabled = 0;
	iwe->u.bitrate.value = max_rate * 500000;
	start =iwe_stream_add_event(info, start, stop, iwe, IW_EV_PARAM_LEN);
	return start ;
}

static inline char * iwe_stream_wpa_wpa2_process(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe)
{
	int buf_size = MAX_WPA_IE_LEN*2;
	//u8 pbuf[buf_size]={0};
	u8 *pbuf = rtw_zmalloc(buf_size);

	u8 wpa_ie[255]= {0},rsn_ie[255]= {0};
	u16 i, wpa_len=0,rsn_len=0;
	u8 *p;
	sint out_len=0;


	if(pbuf) {
		p=pbuf;

		//parsing WPA/WPA2 IE
		if (pnetwork->network.Reserved[0] != 2) { // Probe Request
			out_len=rtw_get_sec_ie(pnetwork->network.IEs ,pnetwork->network.IELength,rsn_ie,&rsn_len,wpa_ie,&wpa_len);
			RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: ssid=%s\n",pnetwork->network.Ssid.Ssid));
			RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: wpa_len=%d rsn_len=%d\n",wpa_len,rsn_len));

			if (wpa_len > 0) {

				_rtw_memset(pbuf, 0, buf_size);
				p += sprintf(p, "wpa_ie=");
				for (i = 0; i < wpa_len; i++) {
					p += sprintf(p, "%02x", wpa_ie[i]);
				}

				if (wpa_len > 100) {
					printk("-----------------Len %d----------------\n", wpa_len);
					for (i = 0; i < wpa_len; i++) {
						printk("%02x ", wpa_ie[i]);
					}
					printk("\n");
					printk("-----------------Len %d----------------\n", wpa_len);
				}

				_rtw_memset(iwe, 0, sizeof(*iwe));
				iwe->cmd = IWEVCUSTOM;
				iwe->u.data.length = strlen(pbuf);
				start = iwe_stream_add_point(info, start, stop, iwe,pbuf);

				_rtw_memset(iwe, 0, sizeof(*iwe));
				iwe->cmd =IWEVGENIE;
				iwe->u.data.length = wpa_len;
				start = iwe_stream_add_point(info, start, stop, iwe, wpa_ie);
			}
			if (rsn_len > 0) {

				_rtw_memset(pbuf, 0, buf_size);
				p += sprintf(p, "rsn_ie=");
				for (i = 0; i < rsn_len; i++) {
					p += sprintf(p, "%02x", rsn_ie[i]);
				}
				_rtw_memset(iwe, 0, sizeof(*iwe));
				iwe->cmd = IWEVCUSTOM;
				iwe->u.data.length = strlen(pbuf);
				start = iwe_stream_add_point(info, start, stop, iwe,pbuf);

				_rtw_memset(iwe, 0, sizeof(*iwe));
				iwe->cmd =IWEVGENIE;
				iwe->u.data.length = rsn_len;
				start = iwe_stream_add_point(info, start, stop, iwe, rsn_ie);
			}
		}

		rtw_mfree(pbuf, buf_size);
	}
	return start;
}

static inline char * iwe_stream_wps_process(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe)
{
	//parsing WPS IE
	uint cnt = 0,total_ielen;
	u8 *wpsie_ptr=NULL;
	uint wps_ielen = 0;
	u8 ie_offset = (pnetwork->network.Reserved[0] == 2? 0:12);

	u8 *ie_ptr = pnetwork->network.IEs + ie_offset;
	total_ielen= pnetwork->network.IELength - ie_offset;

	if (pnetwork->network.Reserved[0] == 2) { // Probe Request
		ie_ptr = pnetwork->network.IEs;
		total_ielen = pnetwork->network.IELength;
	} else { // Beacon or Probe Respones
		ie_ptr = pnetwork->network.IEs + _FIXED_IE_LENGTH_;
		total_ielen = pnetwork->network.IELength - _FIXED_IE_LENGTH_;
	}
	while(cnt < total_ielen) {
		if(rtw_is_wps_ie(&ie_ptr[cnt], &wps_ielen) && (wps_ielen>2)) {
			wpsie_ptr = &ie_ptr[cnt];
			iwe->cmd =IWEVGENIE;
			iwe->u.data.length = (u16)wps_ielen;
			start = iwe_stream_add_point(info, start, stop,iwe, wpsie_ptr);
		}
		cnt+=ie_ptr[cnt+1]+2; //goto next
	}
	return start;
}

static inline char * iwe_stream_wapi_process(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe)
{
#ifdef CONFIG_WAPI_SUPPORT
	char *p;

	if (pnetwork->network.Reserved[0] != 2) { // Probe Request
		sint out_len_wapi=0;
		/* here use static for stack size */
		static u8 buf_wapi[MAX_WAPI_IE_LEN*2]= {0};
		static u8 wapi_ie[MAX_WAPI_IE_LEN]= {0};
		u16 wapi_len=0;
		u16  i;

		out_len_wapi=rtw_get_wapi_ie(pnetwork->network.IEs ,pnetwork->network.IELength,wapi_ie,&wapi_len);
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: ssid=%s\n",pnetwork->network.Ssid.Ssid));
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: wapi_len=%d \n",wapi_len));

		DBG_871X("rtw_wx_get_scan: %s ",pnetwork->network.Ssid.Ssid);
		DBG_871X("rtw_wx_get_scan: ssid = %d ",wapi_len);


		if (wapi_len > 0) {
			p=buf_wapi;
			//_rtw_memset(buf_wapi, 0, MAX_WAPI_IE_LEN*2);
			p += sprintf(p, "wapi_ie=");
			for (i = 0; i < wapi_len; i++) {
				p += sprintf(p, "%02x", wapi_ie[i]);
			}

			_rtw_memset(iwe, 0, sizeof(*iwe));
			iwe->cmd = IWEVCUSTOM;
			iwe->u.data.length = strlen(buf_wapi);
			start = iwe_stream_add_point(info, start, stop, iwe,buf_wapi);

			_rtw_memset(iwe, 0, sizeof(*iwe));
			iwe->cmd =IWEVGENIE;
			iwe->u.data.length = wapi_len;
			start = iwe_stream_add_point(info, start, stop, iwe, wapi_ie);
		}
	}
#endif//#ifdef CONFIG_WAPI_SUPPORT
	return start;
}

static inline char *  iwe_stream_rssi_process(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe)
{
	u8 ss, sq;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	/* Add quality statistics */
	iwe->cmd = IWEVQUAL;
	iwe->u.qual.updated = IW_QUAL_QUAL_UPDATED | IW_QUAL_LEVEL_UPDATED
#if defined(CONFIG_SIGNAL_DISPLAY_DBM) && defined(CONFIG_BACKGROUND_NOISE_MONITOR)
	                      | IW_QUAL_NOISE_UPDATED
#else
	                      | IW_QUAL_NOISE_INVALID
#endif
#ifdef CONFIG_SIGNAL_DISPLAY_DBM
	                      | IW_QUAL_DBM
#endif
	                      ;

	if ( check_fwstate(pmlmepriv, _FW_LINKED)== _TRUE &&
	     is_same_network(&pmlmepriv->cur_network.network, &pnetwork->network, 0)) {
		ss = padapter->recvpriv.signal_strength;
		sq = padapter->recvpriv.signal_qual;
	} else {
		ss = pnetwork->network.PhyInfo.SignalStrength;
		sq = pnetwork->network.PhyInfo.SignalQuality;
	}


#ifdef CONFIG_SIGNAL_DISPLAY_DBM
	iwe->u.qual.level = (u8) translate_percentage_to_dbm(ss);//dbm
#else
#ifdef CONFIG_SKIP_SIGNAL_SCALE_MAPPING
	{
		/* Do signal scale mapping when using percentage as the unit of signal strength, since the scale mapping is skipped in odm */

		HAL_DATA_TYPE *pHal = GET_HAL_DATA(padapter);

		iwe->u.qual.level = (u8)odm_SignalScaleMapping(&pHal->odmpriv, ss);
	}
#else
	iwe->u.qual.level = (u8)ss;//%
#endif
#endif

	iwe->u.qual.qual = (u8)sq;   // signal quality

#ifdef CONFIG_PLATFORM_ROCKCHIPS
	iwe->u.qual.noise = -100; // noise level suggest by zhf@rockchips
#else
#if defined(CONFIG_SIGNAL_DISPLAY_DBM) && defined(CONFIG_BACKGROUND_NOISE_MONITOR)
	{
		s16 tmp_noise=0;
		rtw_hal_get_odm_var(padapter, HAL_ODM_NOISE_MONITOR,&(pnetwork->network.Configuration.DSConfig), &(tmp_noise));
		iwe->u.qual.noise = tmp_noise ;
	}
#else
	iwe->u.qual.noise = 0; // noise level
#endif
#endif //CONFIG_PLATFORM_ROCKCHIPS

	//DBG_871X("iqual=%d, ilevel=%d, inoise=%d, iupdated=%d\n", iwe.u.qual.qual, iwe.u.qual.level , iwe.u.qual.noise, iwe.u.qual.updated);

	start = iwe_stream_add_event(info, start, stop, iwe, IW_EV_QUAL_LEN);
	return start;
}

static inline char *  iwe_stream_net_rsv_process(_adapter *padapter,
        struct iw_request_info* info, struct wlan_network *pnetwork,
        char *start, char *stop,struct iw_event *iwe)
{
	u8 buf[32] = {0};
	u8 * p,*pos;
	//int len;
	p = buf;
	pos = pnetwork->network.Reserved;

	p += sprintf(p, "fm=%02X%02X", pos[1], pos[0]);
	_rtw_memset(iwe, 0, sizeof(*iwe));
	iwe->cmd = IWEVCUSTOM;
	iwe->u.data.length = strlen(buf);
	start = iwe_stream_add_point(info, start, stop,iwe, buf);
	return start;
}

#ifdef CONFIG_WIRELESS_EXT

#if 1
static char *translate_scan(_adapter *padapter,
                            struct iw_request_info* info, struct wlan_network *pnetwork,
                            char *start, char *stop)
{
	struct iw_event iwe;
	u16 cap = 0;
	_rtw_memset(&iwe, 0, sizeof(iwe));

	if(_FALSE == search_p2p_wfd_ie(padapter,info,pnetwork,start,stop))
		return start;

	start = iwe_stream_mac_addr_proess(padapter,info,pnetwork,start,stop,&iwe);
	start = iwe_stream_essid_proess(padapter,info,pnetwork,start,stop,&iwe);
	start = iwe_stream_protocol_process(padapter,info,pnetwork,start,stop,&iwe);
	if (pnetwork->network.Reserved[0] == 2) { // Probe Request
		cap = 0;
	} else {
		_rtw_memcpy((u8 *)&cap, rtw_get_capability_from_ie(pnetwork->network.IEs), 2);
		cap = le16_to_cpu(cap);
	}

	start = iwe_stream_mode_process(padapter,info,pnetwork,start,stop,&iwe,cap);
	start = iwe_stream_chan_process(padapter,info,pnetwork,start,stop,&iwe);
	start = iwe_stream_encryption_process(padapter,info,pnetwork,start,stop,&iwe,cap);
	start = iwe_stream_rate_process(padapter,info,pnetwork,start,stop,&iwe);
	start = iwe_stream_wpa_wpa2_process(padapter,info,pnetwork,start,stop,&iwe);
	start = iwe_stream_wps_process(padapter,info,pnetwork,start,stop,&iwe);
	start = iwe_stream_wapi_process(padapter,info,pnetwork,start,stop,&iwe);
	start = iwe_stream_rssi_process(padapter,info,pnetwork,start,stop,&iwe);
	start = iwe_stream_net_rsv_process(padapter,info,pnetwork,start,stop,&iwe);

	return start;
}
#else
static char *translate_scan(_adapter *padapter,
                            struct iw_request_info* info, struct wlan_network *pnetwork,
                            char *start, char *stop)
{
	struct iw_event iwe;
	u16 cap;
	u32 ht_ielen = 0, vht_ielen = 0;
	char custom[MAX_CUSTOM_LEN];
	char *p;
	u16 max_rate=0, rate, ht_cap=_FALSE, vht_cap = _FALSE;
	u32 i = 0;
	char	*current_val;
	long rssi;
	u8 bw_40MHz=0, short_GI=0, bw_160MHz=0, vht_highest_rate = 0;
	u16 mcs_rate=0, vht_data_rate=0;
	u8 ie_offset = (pnetwork->network.Reserved[0] == 2? 0:12);
	struct registry_priv *pregpriv = &padapter->registrypriv;

	if(_FALSE == search_p2p_wfd_ie(padapter,info,pnetwork,start,stop))
		return start;

	/*  AP MAC address  */
	iwe.cmd = SIOCGIWAP;
	iwe.u.ap_addr.sa_family = ARPHRD_ETHER;

	_rtw_memcpy(iwe.u.ap_addr.sa_data, pnetwork->network.MacAddress, ETH_ALEN);
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_ADDR_LEN);

	/* Add the ESSID */
	iwe.cmd = SIOCGIWESSID;
	iwe.u.data.flags = 1;
	iwe.u.data.length = min((u16)pnetwork->network.Ssid.SsidLength, (u16)32);
	start = iwe_stream_add_point(info, start, stop, &iwe, pnetwork->network.Ssid.Ssid);

	//parsing HT_CAP_IE
	if (pnetwork->network.Reserved[0] == 2) { // Probe Request
		p = rtw_get_ie(&pnetwork->network.IEs[0], _HT_CAPABILITY_IE_, &ht_ielen, pnetwork->network.IELength);
	} else {
		p = rtw_get_ie(&pnetwork->network.IEs[12], _HT_CAPABILITY_IE_, &ht_ielen, pnetwork->network.IELength-12);
	}
	if(p && ht_ielen>0) {
		struct rtw_ieee80211_ht_cap *pht_capie;
		ht_cap = _TRUE;
		pht_capie = (struct rtw_ieee80211_ht_cap *)(p+2);
		_rtw_memcpy(&mcs_rate , pht_capie->supp_mcs_set, 2);
		bw_40MHz = (pht_capie->cap_info&IEEE80211_HT_CAP_SUP_WIDTH) ? 1:0;
		short_GI = (pht_capie->cap_info&(IEEE80211_HT_CAP_SGI_20|IEEE80211_HT_CAP_SGI_40)) ? 1:0;
	}

#ifdef CONFIG_80211AC_VHT
	//parsing VHT_CAP_IE
	p = rtw_get_ie(&pnetwork->network.IEs[ie_offset], EID_VHTCapability, &vht_ielen, pnetwork->network.IELength-ie_offset);
	if(p && vht_ielen>0) {
		u8	mcs_map[2];

		vht_cap = _TRUE;
		bw_160MHz = GET_VHT_CAPABILITY_ELE_CHL_WIDTH(p+2);
		if(bw_160MHz)
			short_GI = GET_VHT_CAPABILITY_ELE_SHORT_GI160M(p+2);
		else
			short_GI = GET_VHT_CAPABILITY_ELE_SHORT_GI80M(p+2);

		_rtw_memcpy(mcs_map, GET_VHT_CAPABILITY_ELE_TX_MCS(p+2), 2);

		vht_highest_rate = rtw_get_vht_highest_rate(mcs_map);
		vht_data_rate = rtw_vht_mcs_to_data_rate(CHANNEL_WIDTH_80, short_GI, vht_highest_rate);
	}
#endif

	/* Add the protocol name */
	iwe.cmd = SIOCGIWNAME;
	if ((rtw_is_cckratesonly_included((u8*)&pnetwork->network.SupportedRates)) == _TRUE) {
		if(ht_cap == _TRUE)
			snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11bn");
		else
			snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11b");
	} else if ((rtw_is_cckrates_included((u8*)&pnetwork->network.SupportedRates)) == _TRUE) {
		if(ht_cap == _TRUE)
			snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11bgn");
		else
			snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11bg");
	} else {
		if(pnetwork->network.Configuration.DSConfig > 14) {
			if(vht_cap == _TRUE)
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11AC");
			else if(ht_cap == _TRUE)
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11an");
			else
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11a");
		} else {
			if(ht_cap == _TRUE)
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11gn");
			else
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11g");
		}
	}

	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_CHAR_LEN);

	/* Add mode */
	if (pnetwork->network.Reserved[0] == 2) { // Probe Request
		cap = 0;
	} else {
		iwe.cmd = SIOCGIWMODE;
		_rtw_memcpy((u8 *)&cap, rtw_get_capability_from_ie(pnetwork->network.IEs), 2);
		cap = le16_to_cpu(cap);
	}

	if(cap & (WLAN_CAPABILITY_IBSS |WLAN_CAPABILITY_BSS)) {
		if (cap & WLAN_CAPABILITY_BSS)
			iwe.u.mode = IW_MODE_MASTER;
		else
			iwe.u.mode = IW_MODE_ADHOC;

		start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_UINT_LEN);
	}

	if(pnetwork->network.Configuration.DSConfig<1 /*|| pnetwork->network.Configuration.DSConfig>14*/)
		pnetwork->network.Configuration.DSConfig = 1;

	/* Add frequency/channel */
	iwe.cmd = SIOCGIWFREQ;
	iwe.u.freq.m = rtw_ch2freq(pnetwork->network.Configuration.DSConfig) * 100000;
	iwe.u.freq.e = 1;
	iwe.u.freq.i = pnetwork->network.Configuration.DSConfig;
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_FREQ_LEN);

	/* Add encryption capability */
	iwe.cmd = SIOCGIWENCODE;
	if (cap & WLAN_CAPABILITY_PRIVACY)
		iwe.u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
	else
		iwe.u.data.flags = IW_ENCODE_DISABLED;
	iwe.u.data.length = 0;
	start = iwe_stream_add_point(info, start, stop, &iwe, pnetwork->network.Ssid.Ssid);

	/*Add basic and extended rates */
	max_rate = 0;
	p = custom;
	p += snprintf(p, MAX_CUSTOM_LEN - (p - custom), " Rates (Mb/s): ");
	while(pnetwork->network.SupportedRates[i]!=0) {
		rate = pnetwork->network.SupportedRates[i]&0x7F;
		if (rate > max_rate)
			max_rate = rate;
		p += snprintf(p, MAX_CUSTOM_LEN - (p - custom),
		              "%d%s ", rate >> 1, (rate & 1) ? ".5" : "");
		i++;
	}

	if(vht_cap == _TRUE) {
		max_rate = vht_data_rate;
	} else if(ht_cap == _TRUE) {
		if(mcs_rate&0x8000) { //MCS15
			max_rate = (bw_40MHz) ? ((short_GI)?300:270):((short_GI)?144:130);

		} else if(mcs_rate&0x0080) { //MCS7
			max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
		} else { //default MCS7
			//DBG_871X("wx_get_scan, mcs_rate_bitmap=0x%x\n", mcs_rate);
			max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
		}

		max_rate = max_rate*2;//Mbps/2;
	}

	iwe.cmd = SIOCGIWRATE;
	iwe.u.bitrate.fixed = iwe.u.bitrate.disabled = 0;
	iwe.u.bitrate.value = max_rate * 500000;
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_PARAM_LEN);

	//parsing WPA/WPA2 IE
	if (pnetwork->network.Reserved[0] != 2) { // Probe Request
		u8 buf[MAX_WPA_IE_LEN*2];
		u8 wpa_ie[255],rsn_ie[255];
		u16 wpa_len=0,rsn_len=0;
		u8 *p;
		sint out_len=0;
		out_len=rtw_get_sec_ie(pnetwork->network.IEs ,pnetwork->network.IELength,rsn_ie,&rsn_len,wpa_ie,&wpa_len);
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: ssid=%s\n",pnetwork->network.Ssid.Ssid));
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: wpa_len=%d rsn_len=%d\n",wpa_len,rsn_len));

		if (wpa_len > 0) {
			p=buf;
			_rtw_memset(buf, 0, MAX_WPA_IE_LEN*2);
			p += sprintf(p, "wpa_ie=");
			for (i = 0; i < wpa_len; i++) {
				p += sprintf(p, "%02x", wpa_ie[i]);
			}

			if (wpa_len > 100) {
				printk("-----------------Len %d----------------\n", wpa_len);
				for (i = 0; i < wpa_len; i++) {
					printk("%02x ", wpa_ie[i]);
				}
				printk("\n");
				printk("-----------------Len %d----------------\n", wpa_len);
			}

			_rtw_memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = IWEVCUSTOM;
			iwe.u.data.length = strlen(buf);
			start = iwe_stream_add_point(info, start, stop, &iwe,buf);

			_rtw_memset(&iwe, 0, sizeof(iwe));
			iwe.cmd =IWEVGENIE;
			iwe.u.data.length = wpa_len;
			start = iwe_stream_add_point(info, start, stop, &iwe, wpa_ie);
		}
		if (rsn_len > 0) {
			p = buf;
			_rtw_memset(buf, 0, MAX_WPA_IE_LEN*2);
			p += sprintf(p, "rsn_ie=");
			for (i = 0; i < rsn_len; i++) {
				p += sprintf(p, "%02x", rsn_ie[i]);
			}
			_rtw_memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = IWEVCUSTOM;
			iwe.u.data.length = strlen(buf);
			start = iwe_stream_add_point(info, start, stop, &iwe,buf);

			_rtw_memset(&iwe, 0, sizeof(iwe));
			iwe.cmd =IWEVGENIE;
			iwe.u.data.length = rsn_len;
			start = iwe_stream_add_point(info, start, stop, &iwe, rsn_ie);
		}
	}

	{
		//parsing WPS IE
		uint cnt = 0,total_ielen;
		u8 *wpsie_ptr=NULL;
		uint wps_ielen = 0;

		u8 *ie_ptr = pnetwork->network.IEs + ie_offset;
		total_ielen= pnetwork->network.IELength - ie_offset;

		if (pnetwork->network.Reserved[0] == 2) { // Probe Request
			ie_ptr = pnetwork->network.IEs;
			total_ielen = pnetwork->network.IELength;
		} else { // Beacon or Probe Respones
			ie_ptr = pnetwork->network.IEs + _FIXED_IE_LENGTH_;
			total_ielen = pnetwork->network.IELength - _FIXED_IE_LENGTH_;
		}

		while(cnt < total_ielen) {
			if(rtw_is_wps_ie(&ie_ptr[cnt], &wps_ielen) && (wps_ielen>2)) {
				wpsie_ptr = &ie_ptr[cnt];
				iwe.cmd =IWEVGENIE;
				iwe.u.data.length = (u16)wps_ielen;
				start = iwe_stream_add_point(info, start, stop, &iwe, wpsie_ptr);
			}
			cnt+=ie_ptr[cnt+1]+2; //goto next
		}
	}

#ifdef CONFIG_WAPI_SUPPORT
	if (pnetwork->network.Reserved[0] != 2) { // Probe Request
		sint out_len_wapi=0;
		/* here use static for stack size */
		static u8 buf_wapi[MAX_WAPI_IE_LEN*2];
		static u8 wapi_ie[MAX_WAPI_IE_LEN];
		u16 wapi_len=0;
		u16  i;

		_rtw_memset(buf_wapi, 0, MAX_WAPI_IE_LEN);
		_rtw_memset(wapi_ie, 0, MAX_WAPI_IE_LEN);

		out_len_wapi=rtw_get_wapi_ie(pnetwork->network.IEs ,pnetwork->network.IELength,wapi_ie,&wapi_len);
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: ssid=%s\n",pnetwork->network.Ssid.Ssid));
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: wapi_len=%d \n",wapi_len));

		DBG_871X("rtw_wx_get_scan: %s ",pnetwork->network.Ssid.Ssid);
		DBG_871X("rtw_wx_get_scan: ssid = %d ",wapi_len);


		if (wapi_len > 0) {
			p=buf_wapi;
			_rtw_memset(buf_wapi, 0, MAX_WAPI_IE_LEN*2);
			p += sprintf(p, "wapi_ie=");
			for (i = 0; i < wapi_len; i++) {
				p += sprintf(p, "%02x", wapi_ie[i]);
			}

			_rtw_memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = IWEVCUSTOM;
			iwe.u.data.length = strlen(buf_wapi);
			start = iwe_stream_add_point(info, start, stop, &iwe,buf_wapi);

			_rtw_memset(&iwe, 0, sizeof(iwe));
			iwe.cmd =IWEVGENIE;
			iwe.u.data.length = wapi_len;
			start = iwe_stream_add_point(info, start, stop, &iwe, wapi_ie);
		}
	}
#endif

	{
		struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
		u8 ss, sq;

		/* Add quality statistics */
		iwe.cmd = IWEVQUAL;
		iwe.u.qual.updated = IW_QUAL_QUAL_UPDATED | IW_QUAL_LEVEL_UPDATED
#if defined(CONFIG_SIGNAL_DISPLAY_DBM) && defined(CONFIG_BACKGROUND_NOISE_MONITOR)
		                     | IW_QUAL_NOISE_UPDATED
#else
		                     | IW_QUAL_NOISE_INVALID
#endif
#ifdef CONFIG_SIGNAL_DISPLAY_DBM
		                     | IW_QUAL_DBM
#endif
		                     ;

		if ( check_fwstate(pmlmepriv, _FW_LINKED)== _TRUE &&
		     is_same_network(&pmlmepriv->cur_network.network, &pnetwork->network, 0)) {
			ss = padapter->recvpriv.signal_strength;
			sq = padapter->recvpriv.signal_qual;
		} else {
			ss = pnetwork->network.PhyInfo.SignalStrength;
			sq = pnetwork->network.PhyInfo.SignalQuality;
		}


#ifdef CONFIG_SIGNAL_DISPLAY_DBM
		iwe.u.qual.level = (u8) translate_percentage_to_dbm(ss);//dbm
#else
#ifdef CONFIG_SKIP_SIGNAL_SCALE_MAPPING
		{
			/* Do signal scale mapping when using percentage as the unit of signal strength, since the scale mapping is skipped in odm */

			HAL_DATA_TYPE *pHal = GET_HAL_DATA(padapter);

			iwe.u.qual.level = (u8)odm_SignalScaleMapping(&pHal->odmpriv, ss);
		}
#else
		iwe.u.qual.level = (u8)ss;//%
#endif
#endif

		iwe.u.qual.qual = (u8)sq;   // signal quality

#ifdef CONFIG_PLATFORM_ROCKCHIPS
		iwe.u.qual.noise = -100; // noise level suggest by zhf@rockchips
#else
#if defined(CONFIG_SIGNAL_DISPLAY_DBM) && defined(CONFIG_BACKGROUND_NOISE_MONITOR)
		{
			s16 tmp_noise=0;
			rtw_hal_get_odm_var(padapter, HAL_ODM_NOISE_MONITOR,&(pnetwork->network.Configuration.DSConfig), &(tmp_noise));
			iwe.u.qual.noise = tmp_noise ;
		}
#else
		iwe.u.qual.noise = 0; // noise level
#endif
#endif //CONFIG_PLATFORM_ROCKCHIPS

		//DBG_871X("iqual=%d, ilevel=%d, inoise=%d, iupdated=%d\n", iwe.u.qual.qual, iwe.u.qual.level , iwe.u.qual.noise, iwe.u.qual.updated);

		start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_QUAL_LEN);
	}

	{
		u8 buf[MAX_WPA_IE_LEN];
		u8 * p,*pos;
		int len;
		p = buf;
		pos = pnetwork->network.Reserved;
		_rtw_memset(buf, 0, MAX_WPA_IE_LEN);
		p += sprintf(p, "fm=%02X%02X", pos[1], pos[0]);
		_rtw_memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = IWEVCUSTOM;
		iwe.u.data.length = strlen(buf);
		start = iwe_stream_add_point(info, start, stop, &iwe, buf);
	}

	return start;
}
#endif

static int wpa_set_auth_algs(struct net_device *dev, u32 value)
{
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
	int ret = 0;

	if ((value & AUTH_ALG_SHARED_KEY)&&(value & AUTH_ALG_OPEN_SYSTEM)) {
		DBG_871X("wpa_set_auth_algs, AUTH_ALG_SHARED_KEY and  AUTH_ALG_OPEN_SYSTEM [value:0x%x]\n",value);
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeAutoSwitch;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
	} else if (value & AUTH_ALG_SHARED_KEY) {
		DBG_871X("wpa_set_auth_algs, AUTH_ALG_SHARED_KEY  [value:0x%x]\n",value);
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;

#ifdef CONFIG_PLATFORM_MT53XX
		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeAutoSwitch;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeShared;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Shared;
#endif
	} else if(value & AUTH_ALG_OPEN_SYSTEM) {
		DBG_871X("wpa_set_auth_algs, AUTH_ALG_OPEN_SYSTEM\n");
		//padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
		if(padapter->securitypriv.ndisauthtype < Ndis802_11AuthModeWPAPSK) {
#ifdef CONFIG_PLATFORM_MT53XX
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeAutoSwitch;
			padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeOpen;
			padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Open;
#endif
		}

	} else if(value & AUTH_ALG_LEAP) {
		DBG_871X("wpa_set_auth_algs, AUTH_ALG_LEAP\n");
	} else {
		DBG_871X("wpa_set_auth_algs, error!\n");
		ret = -EINVAL;
	}

	return ret;

}

static int wpa_set_encryption(struct net_device *dev, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len,wep_total_len;
	NDIS_802_11_WEP	 *pwep = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv 	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
#ifdef CONFIG_P2P
	struct wifidirect_info* pwdinfo = &padapter->wdinfo;
#endif //CONFIG_P2P

	_func_enter_;

	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	if (param_len < (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len) {
		ret =  -EINVAL;
		goto exit;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) {

		if (param->u.crypt.idx >= WEP_KEYS
#ifdef CONFIG_IEEE80211W
		    && param->u.crypt.idx > BIP_MAX_KEYID
#endif //CONFIG_IEEE80211W
		   ) {
			ret = -EINVAL;
			goto exit;
		}
	} else {
#ifdef CONFIG_WAPI_SUPPORT
		if (strcmp(param->u.crypt.alg, "SMS4"))
#endif
		{
			ret = -EINVAL;
			goto exit;
		}
	}

	if (strcmp(param->u.crypt.alg, "WEP") == 0) {
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("wpa_set_encryption, crypt.alg = WEP\n"));
		DBG_871X("wpa_set_encryption, crypt.alg = WEP\n");

		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
		padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
		padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;

		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("(1)wep_key_idx=%d\n", wep_key_idx));
		DBG_871X("(1)wep_key_idx=%d\n", wep_key_idx);

		if (wep_key_idx > WEP_KEYS)
			return -EINVAL;

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("(2)wep_key_idx=%d\n", wep_key_idx));

		if (wep_key_len > 0) {
			wep_key_len = wep_key_len <= 5 ? 5 : 13;
			wep_total_len = wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);
			pwep =(NDIS_802_11_WEP	 *) rtw_malloc(wep_total_len);
			if(pwep == NULL) {
				RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,(" wpa_set_encryption: pwep allocate fail !!!\n"));
				goto exit;
			}

			_rtw_memset(pwep, 0, wep_total_len);

			pwep->KeyLength = wep_key_len;
			pwep->Length = wep_total_len;

			if(wep_key_len==13) {
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
			}
		} else {
			ret = -EINVAL;
			goto exit;
		}

		pwep->KeyIndex = wep_key_idx;
		pwep->KeyIndex |= 0x80000000;

		_rtw_memcpy(pwep->KeyMaterial,  param->u.crypt.key, pwep->KeyLength);

		if(param->u.crypt.set_tx) {
			DBG_871X("wep, set_tx=1\n");

			if(rtw_set_802_11_add_wep(padapter, pwep) == (u8)_FAIL) {
				ret = -EOPNOTSUPP ;
			}
		} else {
			DBG_871X("wep, set_tx=0\n");

			//don't update "psecuritypriv->dot11PrivacyAlgrthm" and
			//"psecuritypriv->dot11PrivacyKeyIndex=keyid", but can rtw_set_key to fw/cam

			if (wep_key_idx >= WEP_KEYS) {
				ret = -EOPNOTSUPP ;
				goto exit;
			}

			_rtw_memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);
			psecuritypriv->dot11DefKeylen[wep_key_idx]=pwep->KeyLength;
			rtw_set_key(padapter, psecuritypriv, wep_key_idx, 0, _TRUE);
		}

		goto exit;
	}

	if(padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) { // 802_1x
		struct sta_info * psta,*pbcmc_sta;
		struct sta_priv * pstapriv = &padapter->stapriv;

		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_MP_STATE) == _TRUE) { //sta mode
			psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
			if (psta == NULL) {
				//DEBUG_ERR( ("Set wpa_set_encryption: Obtain Sta_info fail \n"));
			} else {
				//Jeff: don't disable ieee8021x_blocked while clearing key
				if (strcmp(param->u.crypt.alg, "none") != 0)
					psta->ieee8021x_blocked = _FALSE;

				if((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
				   (padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled)) {
					psta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}

				if(param->u.crypt.set_tx ==1) { //pairwise key
					_rtw_memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					if(strcmp(param->u.crypt.alg, "TKIP") == 0) { //set mic key
						//DEBUG_ERR(("\nset key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len));
						_rtw_memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
						_rtw_memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

						padapter->securitypriv.busetkipkey=_FALSE;
						//_set_timer(&padapter->securitypriv.tkip_timer, 50);
					}

					//DEBUG_ERR((" param->u.crypt.key_len=%d\n",param->u.crypt.key_len));
					DBG_871X(" ~~~~set sta key:unicastkey\n");

					rtw_setstakey_cmd(padapter, psta, UNICAST_KEY, _TRUE);
				} else { //group key
					if(strcmp(param->u.crypt.alg, "TKIP") == 0 || strcmp(param->u.crypt.alg, "CCMP") == 0) {
						_rtw_memcpy(padapter->securitypriv.dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key,(param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
						//only TKIP group key need to install this
						if(param->u.crypt.key_len > 16) {
							_rtw_memcpy(padapter->securitypriv.dot118021XGrptxmickey[param->u.crypt.idx].skey,&(param->u.crypt.key[16]),8);
							_rtw_memcpy(padapter->securitypriv.dot118021XGrprxmickey[param->u.crypt.idx].skey,&(param->u.crypt.key[24]),8);
						}
						padapter->securitypriv.binstallGrpkey = _TRUE;
						//DEBUG_ERR((" param->u.crypt.key_len=%d\n", param->u.crypt.key_len));
						DBG_871X(" ~~~~set sta key:groupkey\n");

						padapter->securitypriv.dot118021XGrpKeyid = param->u.crypt.idx;

						rtw_set_key(padapter,&padapter->securitypriv,param->u.crypt.idx, 1, _TRUE);
					}
#ifdef CONFIG_IEEE80211W
					else if(strcmp(param->u.crypt.alg, "BIP") == 0) {
						int no;
						//printk("BIP key_len=%d , index=%d @@@@@@@@@@@@@@@@@@\n", param->u.crypt.key_len, param->u.crypt.idx);
						//save the IGTK key, length 16 bytes
						_rtw_memcpy(padapter->securitypriv.dot11wBIPKey[param->u.crypt.idx].skey,  param->u.crypt.key,(param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
						/*printk("IGTK key below:\n");
						for(no=0;no<16;no++)
							printk(" %02x ", padapter->securitypriv.dot11wBIPKey[param->u.crypt.idx].skey[no]);
						printk("\n");*/
						padapter->securitypriv.dot11wBIPKeyid = param->u.crypt.idx;
						padapter->securitypriv.binstallBIPkey = _TRUE;
						DBG_871X(" ~~~~set sta key:IGKT\n");
					}
#endif //CONFIG_IEEE80211W

#ifdef CONFIG_P2P
					if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_PROVISIONING_ING)) {
						rtw_p2p_set_state(pwdinfo, P2P_STATE_PROVISIONING_DONE);
					}
#endif //CONFIG_P2P

				}
			}

			pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
			if(pbcmc_sta==NULL) {
				//DEBUG_ERR( ("Set OID_802_11_ADD_KEY: bcmc stainfo is null \n"));
			} else {
				//Jeff: don't disable ieee8021x_blocked while clearing key
				if (strcmp(param->u.crypt.alg, "none") != 0)
					pbcmc_sta->ieee8021x_blocked = _FALSE;

				if((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
				   (padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled)) {
					pbcmc_sta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}
			}
		} else if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE)) { //adhoc mode
		}
	}

#ifdef CONFIG_WAPI_SUPPORT
	if (strcmp(param->u.crypt.alg, "SMS4") == 0) {
		PRT_WAPI_T			pWapiInfo = &padapter->wapiInfo;
		PRT_WAPI_STA_INFO	pWapiSta;
		const u8					WapiASUEPNInitialValueSrc[16] = {0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C} ;
		const u8					WapiAEPNInitialValueSrc[16] = {0x37,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C} ;
		const u8 					WapiAEMultiCastPNInitialValueSrc[16] = {0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C} ;

		if(param->u.crypt.set_tx == 1) {
			list_for_each_entry(pWapiSta, &pWapiInfo->wapiSTAUsedList, list) {
				if(_rtw_memcmp(pWapiSta->PeerMacAddr,param->sta_addr,6)) {
					_rtw_memcpy(pWapiSta->lastTxUnicastPN,WapiASUEPNInitialValueSrc,16);

					pWapiSta->wapiUsk.bSet = true;
					_rtw_memcpy(pWapiSta->wapiUsk.dataKey,param->u.crypt.key,16);
					_rtw_memcpy(pWapiSta->wapiUsk.micKey,param->u.crypt.key+16,16);
					pWapiSta->wapiUsk.keyId = param->u.crypt.idx ;
					pWapiSta->wapiUsk.bTxEnable = true;

					_rtw_memcpy(pWapiSta->lastRxUnicastPNBEQueue,WapiAEPNInitialValueSrc,16);
					_rtw_memcpy(pWapiSta->lastRxUnicastPNBKQueue,WapiAEPNInitialValueSrc,16);
					_rtw_memcpy(pWapiSta->lastRxUnicastPNVIQueue,WapiAEPNInitialValueSrc,16);
					_rtw_memcpy(pWapiSta->lastRxUnicastPNVOQueue,WapiAEPNInitialValueSrc,16);
					_rtw_memcpy(pWapiSta->lastRxUnicastPN,WapiAEPNInitialValueSrc,16);
					pWapiSta->wapiUskUpdate.bTxEnable = false;
					pWapiSta->wapiUskUpdate.bSet = false;

					if (psecuritypriv->sw_encrypt== false || psecuritypriv->sw_decrypt == false) {
						//set unicast key for ASUE
						rtw_wapi_set_key(padapter, &pWapiSta->wapiUsk, pWapiSta, false, false);
					}
				}
			}
		} else {
			list_for_each_entry(pWapiSta, &pWapiInfo->wapiSTAUsedList, list) {
				if(_rtw_memcmp(pWapiSta->PeerMacAddr,get_bssid(pmlmepriv),6)) {
					pWapiSta->wapiMsk.bSet = true;
					_rtw_memcpy(pWapiSta->wapiMsk.dataKey,param->u.crypt.key,16);
					_rtw_memcpy(pWapiSta->wapiMsk.micKey,param->u.crypt.key+16,16);
					pWapiSta->wapiMsk.keyId = param->u.crypt.idx ;
					pWapiSta->wapiMsk.bTxEnable = false;
					if(!pWapiSta->bSetkeyOk)
						pWapiSta->bSetkeyOk = true;
					pWapiSta->bAuthenticateInProgress = false;

					_rtw_memcpy(pWapiSta->lastRxMulticastPN, WapiAEMultiCastPNInitialValueSrc, 16);

					if (psecuritypriv->sw_decrypt == false) {
						//set rx broadcast key for ASUE
						rtw_wapi_set_key(padapter, &pWapiSta->wapiMsk, pWapiSta, true, false);
					}
				}

			}
		}
	}
#endif

exit:

	if (pwep) {
		rtw_mfree((u8 *)pwep, wep_total_len);
	}

	_func_exit_;

	return ret;
}

static int rtw_set_wpa_ie(_adapter *padapter, char *pie, unsigned short ielen)
{
	u8 *buf=NULL, *pos=NULL;
	//u32 left;
	int group_cipher = 0, pairwise_cipher = 0;
	int ret = 0;
	const u8 null_addr[]= {0,0,0,0,0,0};
#ifdef CONFIG_P2P
	struct wifidirect_info* pwdinfo = &padapter->wdinfo;
#endif //CONFIG_P2P

	if((ielen > MAX_WPA_IE_LEN) || (pie == NULL)) {
		_clr_fwstate_(&padapter->mlmepriv, WIFI_UNDER_WPS);
		if(pie == NULL)
			return ret;
		else
			return -EINVAL;
	}

	if(ielen) {
		buf = rtw_zmalloc(ielen);
		if (buf == NULL) {
			ret =  -ENOMEM;
			goto exit;
		}

		_rtw_memcpy(buf, pie , ielen);

		//dump
		{
			int i;
			DBG_871X("\n wpa_ie(length:%d):\n", ielen);
			for(i=0; i<ielen; i=i+8)
				DBG_871X("0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x \n",buf[i],buf[i+1],buf[i+2],buf[i+3],buf[i+4],buf[i+5],buf[i+6],buf[i+7]);
		}

		pos = buf;
		if(ielen < RSN_HEADER_LEN) {
			RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("Ie len too short %d\n", ielen));
			ret  = -1;
			goto exit;
		}

#if 0
		pos += RSN_HEADER_LEN;
		left  = ielen - RSN_HEADER_LEN;

		if (left >= RSN_SELECTOR_LEN) {
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		} else if (left > 0) {
			RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("Ie length mismatch, %u too much \n", left));
			ret =-1;
			goto exit;
		}
#endif

		if(rtw_parse_wpa_ie(buf, ielen, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS) {
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPAPSK;
			_rtw_memcpy(padapter->securitypriv.supplicant_ie, &buf[0], ielen);
		}

		if(rtw_parse_wpa2_ie(buf, ielen, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS) {
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPA2PSK;
			_rtw_memcpy(padapter->securitypriv.supplicant_ie, &buf[0], ielen);
		}

		if (group_cipher == 0) {
			group_cipher = WPA_CIPHER_NONE;
		}
		if (pairwise_cipher == 0) {
			pairwise_cipher = WPA_CIPHER_NONE;
		}

		switch(group_cipher) {
		case WPA_CIPHER_NONE:
			padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
			padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
			break;
		case WPA_CIPHER_WEP40:
			padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
			break;
		case WPA_CIPHER_TKIP:
			padapter->securitypriv.dot118021XGrpPrivacy=_TKIP_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
			break;
		case WPA_CIPHER_CCMP:
			padapter->securitypriv.dot118021XGrpPrivacy=_AES_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
			break;
		case WPA_CIPHER_WEP104:
			padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
			break;
		}

		switch(pairwise_cipher) {
		case WPA_CIPHER_NONE:
			padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
			padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
			break;
		case WPA_CIPHER_WEP40:
			padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
			break;
		case WPA_CIPHER_TKIP:
			padapter->securitypriv.dot11PrivacyAlgrthm=_TKIP_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
			break;
		case WPA_CIPHER_CCMP:
			padapter->securitypriv.dot11PrivacyAlgrthm=_AES_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
			break;
		case WPA_CIPHER_WEP104:
			padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
			break;
		}

		_clr_fwstate_(&padapter->mlmepriv, WIFI_UNDER_WPS);
		{
			//set wps_ie
			u16 cnt = 0;
			u8 eid;
			const u8 wps_oui[4]= {0x0,0x50,0xf2,0x04};

			while( cnt < ielen ) {
				eid = buf[cnt];

				if((eid==_VENDOR_SPECIFIC_IE_)&&(_rtw_memcmp(&buf[cnt+2], wps_oui, 4)==_TRUE)) {
					DBG_871X("SET WPS_IE\n");

					padapter->securitypriv.wps_ie_len = ((buf[cnt+1]+2) < MAX_WPS_IE_LEN) ? (buf[cnt+1]+2):MAX_WPS_IE_LEN;

					_rtw_memcpy(padapter->securitypriv.wps_ie, &buf[cnt], padapter->securitypriv.wps_ie_len);

					set_fwstate(&padapter->mlmepriv, WIFI_UNDER_WPS);

#ifdef CONFIG_P2P
					if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_GONEGO_OK)) {
						rtw_p2p_set_state(pwdinfo, P2P_STATE_PROVISIONING_ING);
					}
#endif //CONFIG_P2P
					cnt += buf[cnt+1]+2;

					break;
				} else {
					cnt += buf[cnt+1]+2; //goto next
				}
			}
		}
	}

	//TKIP and AES disallow multicast packets until installing group key
	if(padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_
	   || padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_WTMIC_
	   || padapter->securitypriv.dot11PrivacyAlgrthm == _AES_)
		//WPS open need to enable multicast
		//|| check_fwstate(&padapter->mlmepriv, WIFI_UNDER_WPS) == _TRUE)
		rtw_hal_set_hwreg(padapter, HW_VAR_OFF_RCR_AM, null_addr);

	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
	         ("rtw_set_wpa_ie: pairwise_cipher=0x%08x padapter->securitypriv.ndisencryptstatus=%d padapter->securitypriv.ndisauthtype=%d\n",
	          pairwise_cipher, padapter->securitypriv.ndisencryptstatus, padapter->securitypriv.ndisauthtype));

exit:

	if (buf) rtw_mfree(buf, ielen);

	return ret;
}
#endif

#ifdef CONFIG_WIRELESS_EXT

static int rtw_wx_get_name(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//u16 cap;
	u32 ht_ielen = 0;
	char *p;
	u8 ht_cap=_FALSE, vht_cap=_FALSE;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;
	NDIS_802_11_RATES_EX* prates = NULL;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("cmd_code=%x\n", info->cmd));

	_func_enter_;

	if (check_fwstate(pmlmepriv, _FW_LINKED|WIFI_ADHOC_MASTER_STATE) == _TRUE) {
		//parsing HT_CAP_IE
		p = rtw_get_ie(&pcur_bss->IEs[12], _HT_CAPABILITY_IE_, &ht_ielen, pcur_bss->IELength-12);
		if(p && ht_ielen>0) {
			ht_cap = _TRUE;
		}

#ifdef CONFIG_80211AC_VHT
		if(pmlmepriv->vhtpriv.vht_option == _TRUE)
			vht_cap = _TRUE;
#endif

		prates = &pcur_bss->SupportedRates;

		if (rtw_is_cckratesonly_included((u8*)prates) == _TRUE) {
			if(ht_cap == _TRUE)
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bn");
			else
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11b");
		} else if ((rtw_is_cckrates_included((u8*)prates)) == _TRUE) {
			if(ht_cap == _TRUE)
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bgn");
			else
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bg");
		} else {
			if(pcur_bss->Configuration.DSConfig > 14) {
#ifdef CONFIG_80211AC_VHT
				if(vht_cap == _TRUE) {
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11AC");
				} else
#endif
				{
					if(ht_cap == _TRUE)
						snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11an");
					else
						snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11a");
				}
			} else {
				if(ht_cap == _TRUE)
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11gn");
				else
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11g");
			}
		}
	} else {
		//prates = &padapter->registrypriv.dev_network.SupportedRates;
		//snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11g");
		snprintf(wrqu->name, IFNAMSIZ, "unassociated");
	}

	_func_exit_;

	return 0;
}

static int rtw_wx_set_freq(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	_func_enter_;

	RT_TRACE(_module_rtl871x_mlme_c_, _drv_notice_, ("+rtw_wx_set_freq\n"));

	_func_exit_;

	return 0;
}

static int rtw_wx_get_freq(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;

	if(check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
		//wrqu->freq.m = ieee80211_wlan_frequencies[pcur_bss->Configuration.DSConfig-1] * 100000;
		wrqu->freq.m = rtw_ch2freq(pcur_bss->Configuration.DSConfig) * 100000;
		wrqu->freq.e = 1;
		wrqu->freq.i = pcur_bss->Configuration.DSConfig;

	} else {
		wrqu->freq.m = rtw_ch2freq(padapter->mlmeextpriv.cur_channel) * 100000;
		wrqu->freq.e = 1;
		wrqu->freq.i = padapter->mlmeextpriv.cur_channel;
	}

	return 0;
}

static int rtw_wx_set_mode(struct net_device *dev, struct iw_request_info *a,
                           union iwreq_data *wrqu, char *b)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	NDIS_802_11_NETWORK_INFRASTRUCTURE networkType ;
	int ret = 0;

	_func_enter_;

	if(_FAIL == rtw_pwr_wakeup(padapter)) {
		ret= -EPERM;
		goto exit;
	}

	if (padapter->hw_init_completed==_FALSE) {
		ret = -EPERM;
		goto exit;
	}

	/* initial default type */
	dev->type = ARPHRD_ETHER;

	switch(wrqu->mode) {
	case IW_MODE_MONITOR:
		networkType = Ndis802_11Monitor;
#if 0
		dev->type = ARPHRD_IEEE80211; /* IEEE 802.11 : 801 */
#endif
		dev->type = ARPHRD_IEEE80211_RADIOTAP; /* IEEE 802.11 + radiotap header : 803 */
		DBG_871X("set_mode = IW_MODE_MONITOR\n");
		break;

	case IW_MODE_AUTO:
		networkType = Ndis802_11AutoUnknown;
		DBG_871X("set_mode = IW_MODE_AUTO\n");
		break;
	case IW_MODE_ADHOC:
		networkType = Ndis802_11IBSS;
		DBG_871X("set_mode = IW_MODE_ADHOC\n");
		break;
	case IW_MODE_MASTER:
		networkType = Ndis802_11APMode;
		DBG_871X("set_mode = IW_MODE_MASTER\n");
		//rtw_setopmode_cmd(padapter, networkType,_TRUE);
		break;
	case IW_MODE_INFRA:
		networkType = Ndis802_11Infrastructure;
		DBG_871X("set_mode = IW_MODE_INFRA\n");
		break;

	default :
		ret = -EINVAL;;
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("\n Mode: %s is not supported  \n", iw_operation_mode[wrqu->mode]));
		goto exit;
	}

	/*
		if(Ndis802_11APMode == networkType)
		{
			rtw_setopmode_cmd(padapter, networkType,_TRUE);
		}
		else
		{
			rtw_setopmode_cmd(padapter, Ndis802_11AutoUnknown,_TRUE);
		}
	*/

	if (rtw_set_802_11_infrastructure_mode(padapter, networkType) ==_FALSE) {

		ret = -EPERM;
		goto exit;

	}

	rtw_setopmode_cmd(padapter, networkType,_TRUE);

exit:

	_func_exit_;

	return ret;

}

static int rtw_wx_get_mode(struct net_device *dev, struct iw_request_info *a,
                           union iwreq_data *wrqu, char *b)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,(" rtw_wx_get_mode \n"));

	_func_enter_;

	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) == _TRUE) {
		wrqu->mode = IW_MODE_INFRA;
	} else if  ((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE) ||
	            (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE))

	{
		wrqu->mode = IW_MODE_ADHOC;
	} else if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) {
		wrqu->mode = IW_MODE_MASTER;
	} else {
		wrqu->mode = IW_MODE_AUTO;
	}

	_func_exit_;

	return 0;

}


static int rtw_wx_set_pmkid(struct net_device *dev,
                            struct iw_request_info *a,
                            union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8          j,blInserted = _FALSE;
	int         intReturn = _FALSE;
	//struct mlme_priv  *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct iw_pmksa*  pPMK = ( struct iw_pmksa* ) extra;
	u8     strZeroMacAddress[ ETH_ALEN ] = { 0x00 };
	u8     strIssueBssid[ ETH_ALEN ] = { 0x00 };

	/*
	        struct iw_pmksa
	        {
	            __u32   cmd;
	            struct sockaddr bssid;
	            __u8    pmkid[IW_PMKID_LEN];   //IW_PMKID_LEN=16
	        }
	        There are the BSSID information in the bssid.sa_data array.
	        If cmd is IW_PMKSA_FLUSH, it means the wpa_suppplicant wants to clear all the PMKID information.
	        If cmd is IW_PMKSA_ADD, it means the wpa_supplicant wants to add a PMKID/BSSID to driver.
	        If cmd is IW_PMKSA_REMOVE, it means the wpa_supplicant wants to remove a PMKID/BSSID from driver.
	        */

	_rtw_memcpy( strIssueBssid, pPMK->bssid.sa_data, ETH_ALEN);
	if ( pPMK->cmd == IW_PMKSA_ADD ) {
		DBG_871X( "[rtw_wx_set_pmkid] IW_PMKSA_ADD!\n" );
		if ( _rtw_memcmp( strIssueBssid, strZeroMacAddress, ETH_ALEN ) == _TRUE ) {
			return( intReturn );
		} else {
			intReturn = _TRUE;
		}
		blInserted = _FALSE;

		//overwrite PMKID
		for(j=0 ; j<NUM_PMKID_CACHE; j++) {
			if( _rtw_memcmp( psecuritypriv->PMKIDList[j].Bssid, strIssueBssid, ETH_ALEN) ==_TRUE ) {
				// BSSID is matched, the same AP => rewrite with new PMKID.

				DBG_871X( "[rtw_wx_set_pmkid] BSSID exists in the PMKList.\n" );

				_rtw_memcpy( psecuritypriv->PMKIDList[j].PMKID, pPMK->pmkid, IW_PMKID_LEN);
				psecuritypriv->PMKIDList[ j ].bUsed = _TRUE;
				psecuritypriv->PMKIDIndex = j+1;
				blInserted = _TRUE;
				break;
			}
		}

		if(!blInserted) {
			// Find a new entry
			DBG_871X( "[rtw_wx_set_pmkid] Use the new entry index = %d for this PMKID.\n",
			          psecuritypriv->PMKIDIndex );

			_rtw_memcpy(psecuritypriv->PMKIDList[psecuritypriv->PMKIDIndex].Bssid, strIssueBssid, ETH_ALEN);
			_rtw_memcpy(psecuritypriv->PMKIDList[psecuritypriv->PMKIDIndex].PMKID, pPMK->pmkid, IW_PMKID_LEN);

			psecuritypriv->PMKIDList[ psecuritypriv->PMKIDIndex ].bUsed = _TRUE;
			psecuritypriv->PMKIDIndex++ ;
			if(psecuritypriv->PMKIDIndex==16) {
				psecuritypriv->PMKIDIndex =0;
			}
		}
	} else if ( pPMK->cmd == IW_PMKSA_REMOVE ) {
		DBG_871X( "[rtw_wx_set_pmkid] IW_PMKSA_REMOVE!\n" );
		intReturn = _TRUE;
		for(j=0 ; j<NUM_PMKID_CACHE; j++) {
			if( _rtw_memcmp( psecuritypriv->PMKIDList[j].Bssid, strIssueBssid, ETH_ALEN) ==_TRUE ) {
				// BSSID is matched, the same AP => Remove this PMKID information and reset it.
				_rtw_memset( psecuritypriv->PMKIDList[ j ].Bssid, 0x00, ETH_ALEN );
				psecuritypriv->PMKIDList[ j ].bUsed = _FALSE;
				break;
			}
		}
	} else if ( pPMK->cmd == IW_PMKSA_FLUSH ) {
		DBG_871X( "[rtw_wx_set_pmkid] IW_PMKSA_FLUSH!\n" );
		_rtw_memset( &psecuritypriv->PMKIDList[ 0 ], 0x00, sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
		psecuritypriv->PMKIDIndex = 0;
		intReturn = _TRUE;
	}
	return( intReturn );
}

static int rtw_wx_get_sens(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
#ifdef CONFIG_PLATFORM_ROCKCHIPS
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	/*
	*  20110311 Commented by Jeff
	*  For rockchip platform's wpa_driver_wext_get_rssi
	*/
	if(check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
		//wrqu->sens.value=-padapter->recvpriv.signal_strength;
		wrqu->sens.value=-padapter->recvpriv.rssi;
		//DBG_871X("%s: %d\n", __FUNCTION__, wrqu->sens.value);
		wrqu->sens.fixed = 0; /* no auto select */
	} else
#endif
	{
		wrqu->sens.value = 0;
		wrqu->sens.fixed = 0;	/* no auto select */
		wrqu->sens.disabled = 1;
	}
	return 0;
}

static int rtw_wx_get_range(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	struct iw_range *range = (struct iw_range *)extra;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	u16 val;
	int i;

	_func_enter_;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_range. cmd_code=%x\n", info->cmd));

	wrqu->data.length = sizeof(*range);
	_rtw_memset(range, 0, sizeof(*range));

	/* Let's try to keep this struct in the same order as in
	 * linux/include/wireless.h
	 */

	/* TODO: See what values we can set, and remove the ones we can't
	 * set, or fill them with some default data.
	 */

	/* ~5 Mb/s real (802.11b) */
	range->throughput = 5 * 1000 * 1000;

	// TODO: Not used in 802.11b?
//	range->min_nwid;	/* Minimal NWID we are able to set */
	// TODO: Not used in 802.11b?
//	range->max_nwid;	/* Maximal NWID we are able to set */

	/* Old Frequency (backward compat - moved lower ) */
//	range->old_num_channels;
//	range->old_num_frequency;
//	range->old_freq[6]; /* Filler to keep "version" at the same offset */

	/* signal level threshold range */

	//percent values between 0 and 100.
	range->max_qual.qual = 100;
	range->max_qual.level = 100;
	range->max_qual.noise = 100;
	range->max_qual.updated = 7; /* Updated all three */


	range->avg_qual.qual = 92; /* > 8% missed beacons is 'bad' */
	/* TODO: Find real 'good' to 'bad' threshol value for RSSI */
	range->avg_qual.level = 20 + -98;
	range->avg_qual.noise = 0;
	range->avg_qual.updated = 7; /* Updated all three */

	range->num_bitrates = RATE_COUNT;

	for (i = 0; i < RATE_COUNT && i < IW_MAX_BITRATES; i++) {
		range->bitrate[i] = rtw_rates[i];
	}

	range->min_frag = MIN_FRAG_THRESHOLD;
	range->max_frag = MAX_FRAG_THRESHOLD;

	range->pm_capa = 0;

	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 16;

//	range->retry_capa;	/* What retry options are supported */
//	range->retry_flags;	/* How to decode max/min retry limit */
//	range->r_time_flags;	/* How to decode max/min retry life */
//	range->min_retry;	/* Minimal number of retries */
//	range->max_retry;	/* Maximal number of retries */
//	range->min_r_time;	/* Minimal retry lifetime */
//	range->max_r_time;	/* Maximal retry lifetime */

	for (i = 0, val = 0; i < MAX_CHANNEL_NUM; i++) {

		// Include only legal frequencies for some countries
		if(pmlmeext->channel_set[i].ChannelNum != 0) {
			range->freq[val].i = pmlmeext->channel_set[i].ChannelNum;
			range->freq[val].m = rtw_ch2freq(pmlmeext->channel_set[i].ChannelNum) * 100000;
			range->freq[val].e = 1;
			val++;
		}

		if (val == IW_MAX_FREQUENCIES)
			break;
	}

	range->num_channels = val;
	range->num_frequency = val;

// Commented by Albert 2009/10/13
// The following code will proivde the security capability to network manager.
// If the driver doesn't provide this capability to network manager,
// the WPA/WPA2 routers can't be choosen in the network manager.

	/*
	#define IW_SCAN_CAPA_NONE		0x00
	#define IW_SCAN_CAPA_ESSID		0x01
	#define IW_SCAN_CAPA_BSSID		0x02
	#define IW_SCAN_CAPA_CHANNEL	0x04
	#define IW_SCAN_CAPA_MODE		0x08
	#define IW_SCAN_CAPA_RATE		0x10
	#define IW_SCAN_CAPA_TYPE		0x20
	#define IW_SCAN_CAPA_TIME		0x40
	*/

#if WIRELESS_EXT > 17
	range->enc_capa = IW_ENC_CAPA_WPA|IW_ENC_CAPA_WPA2|
	                  IW_ENC_CAPA_CIPHER_TKIP|IW_ENC_CAPA_CIPHER_CCMP;
#endif

#ifdef IW_SCAN_CAPA_ESSID //WIRELESS_EXT > 21
	range->scan_capa = IW_SCAN_CAPA_ESSID | IW_SCAN_CAPA_TYPE |IW_SCAN_CAPA_BSSID|
	                   IW_SCAN_CAPA_CHANNEL|IW_SCAN_CAPA_MODE|IW_SCAN_CAPA_RATE;
#endif


	_func_exit_;

	return 0;

}

//set bssid flow
//s1. rtw_set_802_11_infrastructure_mode()
//s2. rtw_set_802_11_authentication_mode()
//s3. set_802_11_encryption_mode()
//s4. rtw_set_802_11_bssid()
static int rtw_wx_set_wap(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *awrq,
                          char *extra)
{
	_irqL	irqL;
	uint ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sockaddr *temp = (struct sockaddr *)awrq;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	_list	*phead;
	u8 *dst_bssid, *src_bssid;
	_queue	*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	NDIS_802_11_AUTHENTICATION_MODE	authmode;

	_func_enter_;
	/*
	#ifdef CONFIG_CONCURRENT_MODE
		if(padapter->iface_type > PRIMARY_IFACE)
		{
			ret = -EINVAL;
			goto exit;
		}
	#endif
	*/

#ifdef CONFIG_CONCURRENT_MODE
	if (check_buddy_fwstate(padapter, _FW_UNDER_SURVEY|_FW_UNDER_LINKING) == _TRUE) {
		DBG_871X("set bssid, but buddy_intf is under scanning or linking\n");

		ret = -EINVAL;

		goto exit;
	}
#endif

#ifdef CONFIG_DUALMAC_CONCURRENT
	if (dc_check_fwstate(padapter, _FW_UNDER_SURVEY|_FW_UNDER_LINKING)== _TRUE) {
		DBG_871X("set bssid, but buddy_intf is under scanning or linking\n");
		ret = -EINVAL;
		goto exit;
	}
#endif

	rtw_ps_deny(padapter, PS_DENY_JOIN);
	if(_FAIL == rtw_pwr_wakeup(padapter)) {
		ret= -1;
		goto exit;
	}

	if(!padapter->bup) {
		ret = -1;
		goto exit;
	}


	if (temp->sa_family != ARPHRD_ETHER) {
		ret = -EINVAL;
		goto exit;
	}

	authmode = padapter->securitypriv.ndisauthtype;
	_enter_critical_bh(&queue->lock, &irqL);
	phead = get_list_head(queue);
	pmlmepriv->pscanned = get_next(phead);

	while (1) {

		if ((rtw_end_of_queue_search(phead, pmlmepriv->pscanned)) == _TRUE) {
#if 0
			ret = -EINVAL;
			goto exit;

			if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE) {
				rtw_set_802_11_bssid(padapter, temp->sa_data);
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
#endif

			break;
		}

		pnetwork = LIST_CONTAINOR(pmlmepriv->pscanned, struct wlan_network, list);

		pmlmepriv->pscanned = get_next(pmlmepriv->pscanned);

		dst_bssid = pnetwork->network.MacAddress;

		src_bssid = temp->sa_data;

		if ((_rtw_memcmp(dst_bssid, src_bssid, ETH_ALEN)) == _TRUE) {
			if(!rtw_set_802_11_infrastructure_mode(padapter, pnetwork->network.InfrastructureMode)) {
				ret = -1;
				_exit_critical_bh(&queue->lock, &irqL);
				goto exit;
			}

			break;
		}

	}
	_exit_critical_bh(&queue->lock, &irqL);

	rtw_set_802_11_authentication_mode(padapter, authmode);
	//set_802_11_encryption_mode(padapter, padapter->securitypriv.ndisencryptstatus);
	if (rtw_set_802_11_bssid(padapter, temp->sa_data) == _FALSE) {
		ret = -1;
		goto exit;
	}

exit:

	rtw_ps_deny_cancel(padapter, PS_DENY_JOIN);

	_func_exit_;

	return ret;
}

static int rtw_wx_get_wap(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;

	wrqu->ap_addr.sa_family = ARPHRD_ETHER;

	_rtw_memset(wrqu->ap_addr.sa_data, 0, ETH_ALEN);

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_wap\n"));

	_func_enter_;

	if  ( ((check_fwstate(pmlmepriv, _FW_LINKED)) == _TRUE) ||
	      ((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE)) == _TRUE) ||
	      ((check_fwstate(pmlmepriv, WIFI_AP_STATE)) == _TRUE) ) {

		_rtw_memcpy(wrqu->ap_addr.sa_data, pcur_bss->MacAddress, ETH_ALEN);
	} else {
		_rtw_memset(wrqu->ap_addr.sa_data, 0, ETH_ALEN);
	}

	_func_exit_;

	return 0;

}

static int rtw_wx_set_mlme(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
#if 0
	/* SIOCSIWMLME data */
	struct	iw_mlme {
		__u16		cmd; /* IW_MLME_* */
		__u16		reason_code;
		struct sockaddr	addr;
	};
#endif

	int ret=0;
	u16 reason;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct iw_mlme *mlme = (struct iw_mlme *) extra;


	if(mlme==NULL)
		return -1;

	DBG_871X("%s\n", __FUNCTION__);

	reason = cpu_to_le16(mlme->reason_code);


	DBG_871X("%s, cmd=%d, reason=%d\n", __FUNCTION__, mlme->cmd, reason);


	switch (mlme->cmd) {
	case IW_MLME_DEAUTH:
		if(!rtw_set_802_11_disassociate(padapter))
			ret = -1;
		break;

	case IW_MLME_DISASSOC:
		if(!rtw_set_802_11_disassociate(padapter))
			ret = -1;

		break;

	default:
		return -EOPNOTSUPP;
	}

	return ret;
}

static int rtw_wx_set_scan(struct net_device *dev, struct iw_request_info *a,
                           union iwreq_data *wrqu, char *extra)
{
	u8 _status = _FALSE;
	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	NDIS_802_11_SSID ssid[RTW_SSID_SCAN_AMOUNT];
	_irqL	irqL;
#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
#endif //CONFIG_P2P
	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_set_scan\n"));

	_func_enter_;

#ifdef DBG_IOCTL
	DBG_871X("DBG_IOCTL %s:%d\n",__FUNCTION__, __LINE__);
#endif
	/*
	#ifdef CONFIG_CONCURRENT_MODE
		if(padapter->iface_type > PRIMARY_IFACE)
		{
			ret = -1;
			goto exit;
		}
	#endif
	*/
#ifdef CONFIG_MP_INCLUDED
	if (padapter->registrypriv.mp_mode == 1) {
		DBG_871X(FUNC_ADPT_FMT ": MP mode block Scan request\n", FUNC_ADPT_ARG(padapter));
		ret = -1;
		goto exit;
	}
#ifdef CONFIG_CONCURRENT_MODE
	if (padapter->pbuddy_adapter) {
		if (padapter->pbuddy_adapter->registrypriv.mp_mode == 1) {
			DBG_871X(FUNC_ADPT_FMT ": MP mode block Scan request\n", FUNC_ADPT_ARG(padapter->pbuddy_adapter));
			ret = -1;
			goto exit;
		}
	}
#endif //CONFIG_CONCURRENT_MODE
#endif

	rtw_ps_deny(padapter, PS_DENY_SCAN);
	if(_FAIL == rtw_pwr_wakeup(padapter)) {
		ret= -1;
		goto exit;
	}

	if(padapter->bDriverStopped) {
		DBG_871X("bDriverStopped=%d\n", padapter->bDriverStopped);
		ret= -1;
		goto exit;
	}

	if(!padapter->bup) {
		ret = -1;
		goto exit;
	}

	if (padapter->hw_init_completed==_FALSE) {
		ret = -1;
		goto exit;
	}

	// When Busy Traffic, driver do not site survey. So driver return success.
	// wpa_supplicant will not issue SIOCSIWSCAN cmd again after scan timeout.
	// modify by thomas 2011-02-22.
	if (pmlmepriv->LinkDetectInfo.bBusyTraffic == _TRUE
#ifdef CONFIG_CONCURRENT_MODE
	    || rtw_get_buddy_bBusyTraffic(padapter) == _TRUE
#endif //CONFIG_CONCURRENT_MODE
	   ) {
		indicate_wx_scan_complete_event(padapter);
		goto exit;
	}

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY|_FW_UNDER_LINKING) == _TRUE) {
		indicate_wx_scan_complete_event(padapter);
		goto exit;
	}

#ifdef CONFIG_CONCURRENT_MODE
	if (check_buddy_fwstate(padapter,
	                        _FW_UNDER_SURVEY|_FW_UNDER_LINKING|WIFI_UNDER_WPS) == _TRUE) {
		indicate_wx_scan_complete_event(padapter);
		goto exit;
	}
#endif

#ifdef CONFIG_DUALMAC_CONCURRENT
	if (dc_check_fwstate(padapter, _FW_UNDER_SURVEY|_FW_UNDER_LINKING)== _TRUE) {
		indicate_wx_scan_complete_event(padapter);
		goto exit;
	}
#endif

#ifdef CONFIG_P2P
	if ( pwdinfo->p2p_state != P2P_STATE_NONE ) {
		rtw_p2p_set_pre_state( pwdinfo, rtw_p2p_state( pwdinfo ) );
		rtw_p2p_set_state(pwdinfo, P2P_STATE_FIND_PHASE_SEARCH);
		rtw_p2p_findphase_ex_set(pwdinfo, P2P_FINDPHASE_EX_FULL);
		rtw_free_network_queue(padapter, _TRUE);
	}
#endif //CONFIG_P2P

	_rtw_memset(ssid, 0, sizeof(NDIS_802_11_SSID)*RTW_SSID_SCAN_AMOUNT);

#if WIRELESS_EXT >= 17
	if (wrqu->data.length == sizeof(struct iw_scan_req)) {
		struct iw_scan_req *req = (struct iw_scan_req *)extra;

		if (wrqu->data.flags & IW_SCAN_THIS_ESSID) {
			int len = min((int)req->essid_len, IW_ESSID_MAX_SIZE);

			_rtw_memcpy(ssid[0].Ssid, req->essid, len);
			ssid[0].SsidLength = len;

			DBG_871X("IW_SCAN_THIS_ESSID, ssid=%s, len=%d\n", req->essid, req->essid_len);

			_enter_critical_bh(&pmlmepriv->lock, &irqL);

			_status = rtw_sitesurvey_cmd(padapter, ssid, 1, NULL, 0);

			_exit_critical_bh(&pmlmepriv->lock, &irqL);

		} else if (req->scan_type == IW_SCAN_TYPE_PASSIVE) {
			DBG_871X("rtw_wx_set_scan, req->scan_type == IW_SCAN_TYPE_PASSIVE\n");
		}

	} else
#endif

		if(	wrqu->data.length >= WEXT_CSCAN_HEADER_SIZE
		    && _rtw_memcmp(extra, WEXT_CSCAN_HEADER, WEXT_CSCAN_HEADER_SIZE) == _TRUE
		  ) {
			int len = wrqu->data.length -WEXT_CSCAN_HEADER_SIZE;
			char *pos = extra+WEXT_CSCAN_HEADER_SIZE;
			char section;
			char sec_len;
			int ssid_index = 0;

			//DBG_871X("%s COMBO_SCAN header is recognized\n", __FUNCTION__);

			while(len >= 1) {
				section = *(pos++);
				len-=1;

				switch(section) {
				case WEXT_CSCAN_SSID_SECTION:
					//DBG_871X("WEXT_CSCAN_SSID_SECTION\n");
					if(len < 1) {
						len = 0;
						break;
					}

					sec_len = *(pos++);
					len-=1;

					if(sec_len>0 && sec_len<=len) {
						ssid[ssid_index].SsidLength = sec_len;
						_rtw_memcpy(ssid[ssid_index].Ssid, pos, ssid[ssid_index].SsidLength);
						//DBG_871X("%s COMBO_SCAN with specific ssid:%s, %d\n", __FUNCTION__
						//	, ssid[ssid_index].Ssid, ssid[ssid_index].SsidLength);
						ssid_index++;
					}

					pos+=sec_len;
					len-=sec_len;
					break;


				case WEXT_CSCAN_CHANNEL_SECTION:
					//DBG_871X("WEXT_CSCAN_CHANNEL_SECTION\n");
					pos+=1;
					len-=1;
					break;
				case WEXT_CSCAN_ACTV_DWELL_SECTION:
					//DBG_871X("WEXT_CSCAN_ACTV_DWELL_SECTION\n");
					pos+=2;
					len-=2;
					break;
				case WEXT_CSCAN_PASV_DWELL_SECTION:
					//DBG_871X("WEXT_CSCAN_PASV_DWELL_SECTION\n");
					pos+=2;
					len-=2;
					break;
				case WEXT_CSCAN_HOME_DWELL_SECTION:
					//DBG_871X("WEXT_CSCAN_HOME_DWELL_SECTION\n");
					pos+=2;
					len-=2;
					break;
				case WEXT_CSCAN_TYPE_SECTION:
					//DBG_871X("WEXT_CSCAN_TYPE_SECTION\n");
					pos+=1;
					len-=1;
					break;
#if 0
				case WEXT_CSCAN_NPROBE_SECTION:
					DBG_871X("WEXT_CSCAN_NPROBE_SECTION\n");
					break;
#endif

				default:
					//DBG_871X("Unknown CSCAN section %c\n", section);
					len = 0; // stop parsing
				}
				//DBG_871X("len:%d\n", len);

			}

			//jeff: it has still some scan paramater to parse, we only do this now...
			_status = rtw_set_802_11_bssid_list_scan(padapter, ssid, RTW_SSID_SCAN_AMOUNT);

		} else

		{
			_status = rtw_set_802_11_bssid_list_scan(padapter, NULL, 0);
		}

	if(_status == _FALSE)
		ret = -1;

exit:

	rtw_ps_deny_cancel(padapter, PS_DENY_SCAN);

#ifdef DBG_IOCTL
	DBG_871X("DBG_IOCTL %s:%d return %d\n",__FUNCTION__, __LINE__, ret);
#endif

	_func_exit_;

	return ret;
}

static int rtw_wx_get_scan(struct net_device *dev, struct iw_request_info *a,
                           union iwreq_data *wrqu, char *extra)
{
	_irqL	irqL;
	_list					*plist, *phead;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	_queue				*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	char *ev = extra;
	char *stop = ev + wrqu->data.length;
	u32 ret = 0;
	//u32 cnt=0;
	u32 wait_for_surveydone;
	sint wait_status;
#ifdef CONFIG_CONCURRENT_MODE
	//PADAPTER pbuddy_adapter = padapter->pbuddy_adapter;
	//struct mlme_priv *pbuddy_mlmepriv = &(pbuddy_adapter->mlmepriv);
#endif
#ifdef CONFIG_P2P
	struct	wifidirect_info*	pwdinfo = &padapter->wdinfo;
#endif //CONFIG_P2P
	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan\n"));
	RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_, (" Start of Query SIOCGIWSCAN .\n"));

	_func_enter_;

#ifdef DBG_IOCTL
	DBG_871X("DBG_IOCTL %s:%d\n",__FUNCTION__, __LINE__);
#endif
	/*
	#ifdef CONFIG_CONCURRENT_MODE
		if(padapter->iface_type > PRIMARY_IFACE)
		{
			ret = -EINVAL;
			goto exit;
		}
	#endif
	*/
	if(adapter_to_pwrctl(padapter)->brfoffbyhw && padapter->bDriverStopped) {
		ret = -EINVAL;
		goto exit;
	}

#ifdef CONFIG_P2P
	if(!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)) {
		//	P2P is enabled
		if ( padapter->chip_type == RTL8192D )
			wait_for_surveydone = 300;	//	Because the 8192du supports more channels.
		else
			wait_for_surveydone = 200;
	} else {
		//	P2P is disabled
		wait_for_surveydone = 100;
	}
#else
	{
		wait_for_surveydone = 100;
	}
#endif //CONFIG_P2P

#if 1 // Wireless Extension use EAGAIN to try
	wait_status = _FW_UNDER_SURVEY
#ifndef CONFIG_ANDROID
	              | _FW_UNDER_LINKING
#endif
	              ;

	while (check_fwstate(pmlmepriv, wait_status) == _TRUE) {
		return -EAGAIN;
	}
#else
	wait_status = _FW_UNDER_SURVEY
#ifndef CONFIG_ANDROID
	              |_FW_UNDER_LINKING
#endif
	              ;

#ifdef CONFIG_DUALMAC_CONCURRENT
	while(dc_check_fwstate(padapter, wait_status)== _TRUE) {
		rtw_msleep_os(30);
		cnt++;
		if(cnt > wait_for_surveydone )
			break;
	}
#endif // CONFIG_DUALMAC_CONCURRENT

	while(check_fwstate(pmlmepriv, wait_status) == _TRUE) {
		rtw_msleep_os(30);
		cnt++;
		if(cnt > wait_for_surveydone )
			break;
	}
#endif
	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while(1) {
		if (rtw_end_of_queue_search(phead,plist)== _TRUE)
			break;

		if((stop - ev) < SCAN_ITEM_SIZE) {
			ret = -E2BIG;
			break;
		}

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		//report network only if the current channel set contains the channel to which this network belongs
		if(rtw_ch_set_search_ch(padapter->mlmeextpriv.channel_set, pnetwork->network.Configuration.DSConfig) >= 0
		   && rtw_mlme_band_check(padapter, pnetwork->network.Configuration.DSConfig) == _TRUE
		   && _TRUE == rtw_validate_ssid(&(pnetwork->network.Ssid))
		  ) {
			ev=translate_scan(padapter, a, pnetwork, ev, stop);
		}

		plist = get_next(plist);

	}

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	wrqu->data.length = ev-extra;
	wrqu->data.flags = 0;

exit:

	_func_exit_;

#ifdef DBG_IOCTL
	DBG_871X("DBG_IOCTL %s:%d return %d\n",__FUNCTION__, __LINE__, ret);
#endif

	return ret ;

}

//set ssid flow
//s1. rtw_set_802_11_infrastructure_mode()
//s2. set_802_11_authenticaion_mode()
//s3. set_802_11_encryption_mode()
//s4. rtw_set_802_11_ssid()
static int rtw_wx_set_essid(struct net_device *dev,
                            struct iw_request_info *a,
                            union iwreq_data *wrqu, char *extra)
{
	_irqL irqL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	_queue *queue = &pmlmepriv->scanned_queue;
	_list *phead;
	//s8 status = _TRUE;
	struct wlan_network *pnetwork = NULL;
	NDIS_802_11_AUTHENTICATION_MODE authmode;
	NDIS_802_11_SSID ndis_ssid;
	u8 *dst_ssid, *src_ssid;

	uint ret = 0, len;

	_func_enter_;

#ifdef DBG_IOCTL
	DBG_871X("DBG_IOCTL %s:%d\n",__FUNCTION__, __LINE__);
#endif

	/*
	#ifdef CONFIG_CONCURRENT_MODE
		if(padapter->iface_type > PRIMARY_IFACE)
		{
			ret = -EINVAL;
			goto exit;
		}
	#endif
	*/

#ifdef CONFIG_CONCURRENT_MODE
	if (check_buddy_fwstate(padapter, _FW_UNDER_SURVEY|_FW_UNDER_LINKING) == _TRUE) {
		DBG_871X("set ssid, but buddy_intf is under scanning or linking\n");

		ret = -EINVAL;

		goto exit;
	}
#endif

#ifdef CONFIG_DUALMAC_CONCURRENT
	if (dc_check_fwstate(padapter, _FW_UNDER_SURVEY|_FW_UNDER_LINKING)== _TRUE) {
		DBG_871X("set bssid, but buddy_intf is under scanning or linking\n");
		ret = -EINVAL;
		goto exit;
	}
#endif

	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
	         ("+rtw_wx_set_essid: fw_state=0x%08x\n", get_fwstate(pmlmepriv)));

	rtw_ps_deny(padapter, PS_DENY_JOIN);
	if(_FAIL == rtw_pwr_wakeup(padapter)) {
		ret = -1;
		goto exit;
	}

	if(!padapter->bup) {
		ret = -1;
		goto exit;
	}

#if WIRELESS_EXT <= 20
	if ((wrqu->essid.length-1) > IW_ESSID_MAX_SIZE) {
#else
	if (wrqu->essid.length > IW_ESSID_MAX_SIZE) {
#endif
		ret= -E2BIG;
		goto exit;
	}

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		ret = -1;
		goto exit;
	}

	authmode = padapter->securitypriv.ndisauthtype;
	DBG_871X("=>%s\n",__FUNCTION__);
	if (wrqu->essid.flags && wrqu->essid.length) {
		// Commented by Albert 20100519
		// We got the codes in "set_info" function of iwconfig source code.
		//	=========================================
		//	wrq.u.essid.length = strlen(essid) + 1;
		//	if(we_kernel_version > 20)
		//		wrq.u.essid.length--;
		//	=========================================
		//	That means, if the WIRELESS_EXT less than or equal to 20, the correct ssid len should subtract 1.
#if WIRELESS_EXT <= 20
		len = ((wrqu->essid.length-1) < IW_ESSID_MAX_SIZE) ? (wrqu->essid.length-1) : IW_ESSID_MAX_SIZE;
#else
		len = (wrqu->essid.length < IW_ESSID_MAX_SIZE) ? wrqu->essid.length : IW_ESSID_MAX_SIZE;
#endif

		if( wrqu->essid.length != 33 )
			DBG_871X("ssid=%s, len=%d\n", extra, wrqu->essid.length);

		_rtw_memset(&ndis_ssid, 0, sizeof(NDIS_802_11_SSID));
		ndis_ssid.SsidLength = len;
		_rtw_memcpy(ndis_ssid.Ssid, extra, len);
		src_ssid = ndis_ssid.Ssid;

		RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_, ("rtw_wx_set_essid: ssid=[%s]\n", src_ssid));
		_enter_critical_bh(&queue->lock, &irqL);
		phead = get_list_head(queue);
		pmlmepriv->pscanned = get_next(phead);

		while (1) {
			if (rtw_end_of_queue_search(phead, pmlmepriv->pscanned) == _TRUE) {
#if 0
				if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE) {
					rtw_set_802_11_ssid(padapter, &ndis_ssid);

					goto exit;
				} else {
					RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("rtw_wx_set_ssid(): scanned_queue is empty\n"));
					ret = -EINVAL;
					goto exit;
				}
#endif
				RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_warning_,
				         ("rtw_wx_set_essid: scan_q is empty, set ssid to check if scanning again!\n"));

				break;
			}

			pnetwork = LIST_CONTAINOR(pmlmepriv->pscanned, struct wlan_network, list);

			pmlmepriv->pscanned = get_next(pmlmepriv->pscanned);

			dst_ssid = pnetwork->network.Ssid.Ssid;

			RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
			         ("rtw_wx_set_essid: dst_ssid=%s\n",
			          pnetwork->network.Ssid.Ssid));

			if ((_rtw_memcmp(dst_ssid, src_ssid, ndis_ssid.SsidLength) == _TRUE) &&
			    (pnetwork->network.Ssid.SsidLength==ndis_ssid.SsidLength)) {
				RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
				         ("rtw_wx_set_essid: find match, set infra mode\n"));

				if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE) {
					if(pnetwork->network.InfrastructureMode != pmlmepriv->cur_network.network.InfrastructureMode)
						continue;
				}

				if (rtw_set_802_11_infrastructure_mode(padapter, pnetwork->network.InfrastructureMode) == _FALSE) {
					ret = -1;
					_exit_critical_bh(&queue->lock, &irqL);
					goto exit;
				}

				break;
			}
		}
		_exit_critical_bh(&queue->lock, &irqL);
		RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
		         ("set ssid: set_802_11_auth. mode=%d\n", authmode));
		rtw_set_802_11_authentication_mode(padapter, authmode);
		//set_802_11_encryption_mode(padapter, padapter->securitypriv.ndisencryptstatus);
		if (rtw_set_802_11_ssid(padapter, &ndis_ssid) == _FALSE) {
			ret = -1;
			goto exit;
		}
	}

exit:

	rtw_ps_deny_cancel(padapter, PS_DENY_JOIN);

	DBG_871X("<=%s, ret %d\n",__FUNCTION__, ret);

#ifdef DBG_IOCTL
	DBG_871X("DBG_IOCTL %s:%d return %d\n",__FUNCTION__, __LINE__, ret);
#endif

	_func_exit_;

	return ret;
}

static int rtw_wx_get_essid(struct net_device *dev,
                            struct iw_request_info *a,
                            union iwreq_data *wrqu, char *extra)
{
	u32 len,ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_essid\n"));

	_func_enter_;

	if ( (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) ||
	     (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE)) {
		len = pcur_bss->Ssid.SsidLength;

		wrqu->essid.length = len;

		_rtw_memcpy(extra, pcur_bss->Ssid.Ssid, len);

		wrqu->essid.flags = 1;
	} else {
		ret = -1;
		goto exit;
	}

exit:

	_func_exit_;

	return ret;

}

static int rtw_wx_set_rate(struct net_device *dev,
                           struct iw_request_info *a,
                           union iwreq_data *wrqu, char *extra)
{
	int	i, ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8	datarates[NumRates];
	u32	target_rate = wrqu->bitrate.value;
	u32	fixed = wrqu->bitrate.fixed;
	u32	ratevalue = 0;
	u8 mpdatarate[NumRates]= {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0xff};

	_func_enter_;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,(" rtw_wx_set_rate \n"));
	RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("target_rate = %d, fixed = %d\n",target_rate,fixed));

	if(target_rate == -1) {
		ratevalue = 11;
		goto set_rate;
	}
	target_rate = target_rate/100000;

	switch(target_rate) {
	case 10:
		ratevalue = 0;
		break;
	case 20:
		ratevalue = 1;
		break;
	case 55:
		ratevalue = 2;
		break;
	case 60:
		ratevalue = 3;
		break;
	case 90:
		ratevalue = 4;
		break;
	case 110:
		ratevalue = 5;
		break;
	case 120:
		ratevalue = 6;
		break;
	case 180:
		ratevalue = 7;
		break;
	case 240:
		ratevalue = 8;
		break;
	case 360:
		ratevalue = 9;
		break;
	case 480:
		ratevalue = 10;
		break;
	case 540:
		ratevalue = 11;
		break;
	default:
		ratevalue = 11;
		break;
	}

set_rate:

	for(i=0; i<NumRates; i++) {
		if(ratevalue==mpdatarate[i]) {
			datarates[i] = mpdatarate[i];
			if(fixed == 0)
				break;
		} else {
			datarates[i] = 0xff;
		}

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("datarate_inx=%d\n",datarates[i]));
	}

	if( rtw_setdatarate_cmd(padapter, datarates) !=_SUCCESS) {
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("rtw_wx_set_rate Fail!!!\n"));
		ret = -1;
	}

	_func_exit_;

	return ret;
}

static int rtw_wx_get_rate(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	u16 max_rate = 0;

	max_rate = rtw_get_cur_max_rate((_adapter *)rtw_netdev_priv(dev));

	if(max_rate == 0)
		return -EPERM;

	wrqu->bitrate.fixed = 0;	/* no auto select */
	wrqu->bitrate.value = max_rate * 100000;

	return 0;
}

static int rtw_wx_set_rts(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	_func_enter_;

	if (wrqu->rts.disabled)
		padapter->registrypriv.rts_thresh = 2347;
	else {
		if (wrqu->rts.value < 0 ||
		    wrqu->rts.value > 2347)
			return -EINVAL;

		padapter->registrypriv.rts_thresh = wrqu->rts.value;
	}

	DBG_871X("%s, rts_thresh=%d\n", __func__, padapter->registrypriv.rts_thresh);

	_func_exit_;

	return 0;

}

static int rtw_wx_get_rts(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	_func_enter_;

	DBG_871X("%s, rts_thresh=%d\n", __func__, padapter->registrypriv.rts_thresh);

	wrqu->rts.value = padapter->registrypriv.rts_thresh;
	wrqu->rts.fixed = 0;	/* no auto select */
	//wrqu->rts.disabled = (wrqu->rts.value == DEFAULT_RTS_THRESHOLD);

	_func_exit_;

	return 0;
}

static int rtw_wx_set_frag(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	_func_enter_;

	if (wrqu->frag.disabled)
		padapter->xmitpriv.frag_len = MAX_FRAG_THRESHOLD;
	else {
		if (wrqu->frag.value < MIN_FRAG_THRESHOLD ||
		    wrqu->frag.value > MAX_FRAG_THRESHOLD)
			return -EINVAL;

		padapter->xmitpriv.frag_len = wrqu->frag.value & ~0x1;
	}

	DBG_871X("%s, frag_len=%d\n", __func__, padapter->xmitpriv.frag_len);

	_func_exit_;

	return 0;

}

static int rtw_wx_get_frag(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	_func_enter_;

	DBG_871X("%s, frag_len=%d\n", __func__, padapter->xmitpriv.frag_len);

	wrqu->frag.value = padapter->xmitpriv.frag_len;
	wrqu->frag.fixed = 0;	/* no auto select */
	//wrqu->frag.disabled = (wrqu->frag.value == DEFAULT_FRAG_THRESHOLD);

	_func_exit_;

	return 0;
}

static int rtw_wx_get_retry(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);


	wrqu->retry.value = 7;
	wrqu->retry.fixed = 0;	/* no auto select */
	wrqu->retry.disabled = 1;

	return 0;

}

#if 0
#define IW_ENCODE_INDEX		0x00FF	/* Token index (if needed) */
#define IW_ENCODE_FLAGS		0xFF00	/* Flags defined below */
#define IW_ENCODE_MODE		0xF000	/* Modes defined below */
#define IW_ENCODE_DISABLED	0x8000	/* Encoding disabled */
#define IW_ENCODE_ENABLED	0x0000	/* Encoding enabled */
#define IW_ENCODE_RESTRICTED	0x4000	/* Refuse non-encoded packets */
#define IW_ENCODE_OPEN		0x2000	/* Accept non-encoded packets */
#define IW_ENCODE_NOKEY		0x0800  /* Key is write only, so not present */
#define IW_ENCODE_TEMP		0x0400  /* Temporary key */
/*
iwconfig wlan0 key on -> flags = 0x6001 -> maybe it means auto
iwconfig wlan0 key off -> flags = 0x8800
iwconfig wlan0 key open -> flags = 0x2800
iwconfig wlan0 key open 1234567890 -> flags = 0x2000
iwconfig wlan0 key restricted -> flags = 0x4800
iwconfig wlan0 key open [3] 1234567890 -> flags = 0x2003
iwconfig wlan0 key restricted [2] 1234567890 -> flags = 0x4002
iwconfig wlan0 key open [3] -> flags = 0x2803
iwconfig wlan0 key restricted [2] -> flags = 0x4802
*/
#endif

static int rtw_wx_set_enc(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *keybuf)
{
	u32 key, ret = 0;
	u32 keyindex_provided;
	NDIS_802_11_WEP	 wep;
	NDIS_802_11_AUTHENTICATION_MODE authmode;

	struct iw_point *erq = &(wrqu->encoding);
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	DBG_871X("+rtw_wx_set_enc, flags=0x%x\n", erq->flags);

	_rtw_memset(&wep, 0, sizeof(NDIS_802_11_WEP));

	key = erq->flags & IW_ENCODE_INDEX;

	_func_enter_;

	if (erq->flags & IW_ENCODE_DISABLED) {
		DBG_871X("EncryptionDisabled\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; //open system
		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;

		goto exit;
	}

	if (key) {
		if (key > WEP_KEYS)
			return -EINVAL;
		key--;
		keyindex_provided = 1;
	} else {
		keyindex_provided = 0;
		key = padapter->securitypriv.dot11PrivacyKeyIndex;
		DBG_871X("rtw_wx_set_enc, key=%d\n", key);
	}

	//set authentication mode
	if(erq->flags & IW_ENCODE_OPEN) {
		DBG_871X("rtw_wx_set_enc():IW_ENCODE_OPEN\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;//Ndis802_11EncryptionDisabled;

#ifdef CONFIG_PLATFORM_MT53XX
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open;
#endif

		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;
	} else if(erq->flags & IW_ENCODE_RESTRICTED) {
		DBG_871X("rtw_wx_set_enc():IW_ENCODE_RESTRICTED\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;

#ifdef CONFIG_PLATFORM_MT53XX
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Shared;
#endif

		padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
		padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;
		authmode = Ndis802_11AuthModeShared;
		padapter->securitypriv.ndisauthtype=authmode;
	} else {
		DBG_871X("rtw_wx_set_enc():erq->flags=0x%x\n", erq->flags);

		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;//Ndis802_11EncryptionDisabled;
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; //open system
		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;
	}

	wep.KeyIndex = key;
	if (erq->length > 0) {
		wep.KeyLength = erq->length <= 5 ? 5 : 13;

		wep.Length = wep.KeyLength + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);
	} else {
		wep.KeyLength = 0 ;

		if(keyindex_provided == 1) { // set key_id only, no given KeyMaterial(erq->length==0).
			padapter->securitypriv.dot11PrivacyKeyIndex = key;

			DBG_871X("(keyindex_provided == 1), keyid=%d, key_len=%d\n", key, padapter->securitypriv.dot11DefKeylen[key]);

			switch(padapter->securitypriv.dot11DefKeylen[key]) {
			case 5:
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
				break;
			case 13:
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				break;
			default:
				padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
				break;
			}

			goto exit;

		}

	}

	wep.KeyIndex |= 0x80000000;

	_rtw_memcpy(wep.KeyMaterial, keybuf, wep.KeyLength);

	if (rtw_set_802_11_add_wep(padapter, &wep) == _FALSE) {
		if(rf_on == pwrpriv->rf_pwrstate )
			ret = -EOPNOTSUPP;
		goto exit;
	}

exit:

	_func_exit_;

	return ret;

}

static int rtw_wx_get_enc(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *keybuf)
{
	uint key, ret =0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct iw_point *erq = &(wrqu->encoding);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	_func_enter_;

	if(check_fwstate(pmlmepriv, _FW_LINKED) != _TRUE) {
		if(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) != _TRUE) {
			erq->length = 0;
			erq->flags |= IW_ENCODE_DISABLED;
			return 0;
		}
	}


	key = erq->flags & IW_ENCODE_INDEX;

	if (key) {
		if (key > WEP_KEYS)
			return -EINVAL;
		key--;
	} else {
		key = padapter->securitypriv.dot11PrivacyKeyIndex;
	}

	erq->flags = key + 1;

	//if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeOpen)
	//{
	//      erq->flags |= IW_ENCODE_OPEN;
	//}

	switch(padapter->securitypriv.ndisencryptstatus) {
	case Ndis802_11EncryptionNotSupported:
	case Ndis802_11EncryptionDisabled:

		erq->length = 0;
		erq->flags |= IW_ENCODE_DISABLED;

		break;

	case Ndis802_11Encryption1Enabled:

		erq->length = padapter->securitypriv.dot11DefKeylen[key];

		if(erq->length) {
			_rtw_memcpy(keybuf, padapter->securitypriv.dot11DefKey[key].skey, padapter->securitypriv.dot11DefKeylen[key]);

			erq->flags |= IW_ENCODE_ENABLED;

			if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeOpen) {
				erq->flags |= IW_ENCODE_OPEN;
			} else if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeShared) {
				erq->flags |= IW_ENCODE_RESTRICTED;
			}
		} else {
			erq->length = 0;
			erq->flags |= IW_ENCODE_DISABLED;
		}

		break;

	case Ndis802_11Encryption2Enabled:
	case Ndis802_11Encryption3Enabled:

		erq->length = 16;
		erq->flags |= (IW_ENCODE_ENABLED | IW_ENCODE_OPEN | IW_ENCODE_NOKEY);

		break;

	default:
		erq->length = 0;
		erq->flags |= IW_ENCODE_DISABLED;

		break;

	}

	_func_exit_;

	return ret;

}

static int rtw_wx_get_power(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	wrqu->power.value = 0;
	wrqu->power.fixed = 0;	/* no auto select */
	wrqu->power.disabled = 1;

	return 0;

}

static int rtw_wx_set_gen_ie(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra)
{
	int ret;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	ret = rtw_set_wpa_ie(padapter, extra, wrqu->data.length);

	return ret;
}

static int rtw_wx_set_auth(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct iw_param *param = (struct iw_param*)&(wrqu->param);
	//struct mlme_priv 	*pmlmepriv = &padapter->mlmepriv;
	//struct security_priv *psecuritypriv = &padapter->securitypriv;
	//struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	//struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	//u32 value = param->value;
	int ret = 0;

	switch (param->flags & IW_AUTH_INDEX) {

	case IW_AUTH_WPA_VERSION:
#ifdef CONFIG_WAPI_SUPPORT
#ifndef CONFIG_IOCTL_CFG80211
		padapter->wapiInfo.bWapiEnable = false;
		if(value == IW_AUTH_WAPI_VERSION_1) {
			padapter->wapiInfo.bWapiEnable = true;
			psecuritypriv->dot11PrivacyAlgrthm = _SMS4_;
			psecuritypriv->dot118021XGrpPrivacy = _SMS4_;
			psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_WAPI;
			pmlmeinfo->auth_algo = psecuritypriv->dot11AuthAlgrthm;
			padapter->wapiInfo.extra_prefix_len = WAPI_EXT_LEN;
			padapter->wapiInfo.extra_postfix_len = SMS4_MIC_LEN;
		}
#endif
#endif
		break;
	case IW_AUTH_CIPHER_PAIRWISE:

		break;
	case IW_AUTH_CIPHER_GROUP:

		break;
	case IW_AUTH_KEY_MGMT:
#ifdef CONFIG_WAPI_SUPPORT
#ifndef CONFIG_IOCTL_CFG80211
		DBG_871X("rtw_wx_set_auth: IW_AUTH_KEY_MGMT case \n");
		if(value == IW_AUTH_KEY_MGMT_WAPI_PSK)
			padapter->wapiInfo.bWapiPSK = true;
		else
			padapter->wapiInfo.bWapiPSK = false;
		DBG_871X("rtw_wx_set_auth: IW_AUTH_KEY_MGMT bwapipsk %d \n",padapter->wapiInfo.bWapiPSK);
#endif
#endif
		/*
		 *  ??? does not use these parameters
		 */
		break;

	case IW_AUTH_TKIP_COUNTERMEASURES: {
		if ( param->value ) {
			// wpa_supplicant is enabling the tkip countermeasure.
			padapter->securitypriv.btkip_countermeasure = _TRUE;
		} else {
			// wpa_supplicant is disabling the tkip countermeasure.
			padapter->securitypriv.btkip_countermeasure = _FALSE;
		}
		break;
	}
	case IW_AUTH_DROP_UNENCRYPTED: {
		/* HACK:
		 *
		 * wpa_supplicant calls set_wpa_enabled when the driver
		 * is loaded and unloaded, regardless of if WPA is being
		 * used.  No other calls are made which can be used to
		 * determine if encryption will be used or not prior to
		 * association being expected.  If encryption is not being
		 * used, drop_unencrypted is set to false, else true -- we
		 * can use this to determine if the CAP_PRIVACY_ON bit should
		 * be set.
		 */

		if(padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption1Enabled) {
			break;//it means init value, or using wep, ndisencryptstatus = Ndis802_11Encryption1Enabled,
			// then it needn't reset it;
		}

		if(param->value) {
			padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
			padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
			padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; //open system
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeOpen;
		}

		break;
	}

	case IW_AUTH_80211_AUTH_ALG:

#if defined(CONFIG_ANDROID) || 1
		/*
		 *  It's the starting point of a link layer connection using wpa_supplicant
		*/
		if(check_fwstate(&padapter->mlmepriv, _FW_LINKED)) {
			LeaveAllPowerSaveMode(padapter);
			rtw_disassoc_cmd(padapter, 500, _FALSE);
			DBG_871X("%s...call rtw_indicate_disconnect\n ",__FUNCTION__);
			rtw_indicate_disconnect(padapter);
			rtw_free_assoc_resources(padapter, 1);
		}
#endif


		ret = wpa_set_auth_algs(dev, (u32)param->value);

		break;

	case IW_AUTH_WPA_ENABLED:

		//if(param->value)
		//	padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_8021X; //802.1x
		//else
		//	padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Open;//open system

		//_disassociate(priv);

		break;

	case IW_AUTH_RX_UNENCRYPTED_EAPOL:
		//ieee->ieee802_1x = param->value;
		break;

	case IW_AUTH_PRIVACY_INVOKED:
		//ieee->privacy_invoked = param->value;
		break;

#ifdef CONFIG_WAPI_SUPPORT
#ifndef CONFIG_IOCTL_CFG80211
	case IW_AUTH_WAPI_ENABLED:
		break;
#endif
#endif

	default:
		return -EOPNOTSUPP;

	}

	return ret;

}

static int rtw_wx_set_enc_ext(struct net_device *dev,
                              struct iw_request_info *info,
                              union iwreq_data *wrqu, char *extra)
{
	char *alg_name;
	u32 param_len;
	struct ieee_param *param = NULL;
	struct iw_point *pencoding = &wrqu->encoding;
	struct iw_encode_ext *pext = (struct iw_encode_ext *)extra;
	int ret=0;

	param_len = sizeof(struct ieee_param) + pext->key_len;
	param = (struct ieee_param *)rtw_malloc(param_len);
	if (param == NULL)
		return -1;

	_rtw_memset(param, 0, param_len);

	param->cmd = IEEE_CMD_SET_ENCRYPTION;
	_rtw_memset(param->sta_addr, 0xff, ETH_ALEN);


	switch (pext->alg) {
	case IW_ENCODE_ALG_NONE:
		//todo: remove key
		//remove = 1;
		alg_name = "none";
		break;
	case IW_ENCODE_ALG_WEP:
		alg_name = "WEP";
		break;
	case IW_ENCODE_ALG_TKIP:
		alg_name = "TKIP";
		break;
	case IW_ENCODE_ALG_CCMP:
		alg_name = "CCMP";
		break;
#ifdef CONFIG_IEEE80211W
	case IW_ENCODE_ALG_AES_CMAC:
		alg_name = "BIP";
		break;
#endif //CONFIG_IEEE80211W
#ifdef CONFIG_WAPI_SUPPORT
#ifndef CONFIG_IOCTL_CFG80211
	case IW_ENCODE_ALG_SM4:
		alg_name= "SMS4";
		_rtw_memcpy(param->sta_addr, pext->addr.sa_data, ETH_ALEN);
		DBG_871X("rtw_wx_set_enc_ext: SMS4 case \n");
		break;
#endif
#endif
	default:
		ret = -1;
		goto exit;
	}

	strncpy((char *)param->u.crypt.alg, alg_name, IEEE_CRYPT_ALG_NAME_LEN);

	if (pext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY) {
		param->u.crypt.set_tx = 1;
	}

	/* cliW: WEP does not have group key
	 * just not checking GROUP key setting
	 */
	if ((pext->alg != IW_ENCODE_ALG_WEP) &&
	    ((pext->ext_flags & IW_ENCODE_EXT_GROUP_KEY)
#ifdef CONFIG_IEEE80211W
	     || (pext->ext_flags & IW_ENCODE_ALG_AES_CMAC)
#endif //CONFIG_IEEE80211W
	    )) {
		param->u.crypt.set_tx = 0;
	}

	param->u.crypt.idx = (pencoding->flags&0x00FF) -1 ;

	if (pext->ext_flags & IW_ENCODE_EXT_RX_SEQ_VALID) {
#ifdef CONFIG_WAPI_SUPPORT
#ifndef CONFIG_IOCTL_CFG80211
		if(pext->alg == IW_ENCODE_ALG_SM4)
			_rtw_memcpy(param->u.crypt.seq, pext->rx_seq, 16);
		else
#endif //CONFIG_IOCTL_CFG80211
#endif //CONFIG_WAPI_SUPPORT
			_rtw_memcpy(param->u.crypt.seq, pext->rx_seq, 8);
	}

	if(pext->key_len) {
		param->u.crypt.key_len = pext->key_len;
		//_rtw_memcpy(param + 1, pext + 1, pext->key_len);
		_rtw_memcpy(param->u.crypt.key, pext + 1, pext->key_len);
	}

	if (pencoding->flags & IW_ENCODE_DISABLED) {
		//todo: remove key
		//remove = 1;
	}

	ret =  wpa_set_encryption(dev, param, param_len);

exit:
	if(param) {
		rtw_mfree((u8*)param, param_len);
	}

	return ret;
}


static int rtw_wx_get_nick(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	//struct security_priv *psecuritypriv = &padapter->securitypriv;

	if(extra) {
		wrqu->data.length = 14;
		wrqu->data.flags = 1;
		_rtw_memcpy(extra, "<WIFI@REALTEK>", 14);
	}

	//rtw_signal_process(pid, SIGUSR1); //for test

	//dump debug info here
	/*
		u32 dot11AuthAlgrthm;		// 802.11 auth, could be open, shared, and 8021x
		u32 dot11PrivacyAlgrthm;	// This specify the privacy for shared auth. algorithm.
		u32 dot118021XGrpPrivacy;	// This specify the privacy algthm. used for Grp key
		u32 ndisauthtype;
		u32 ndisencryptstatus;
	*/

	//DBG_871X("auth_alg=0x%x, enc_alg=0x%x, auth_type=0x%x, enc_type=0x%x\n",
	//		psecuritypriv->dot11AuthAlgrthm, psecuritypriv->dot11PrivacyAlgrthm,
	//		psecuritypriv->ndisauthtype, psecuritypriv->ndisencryptstatus);

	//DBG_871X("enc_alg=0x%x\n", psecuritypriv->dot11PrivacyAlgrthm);
	//DBG_871X("auth_type=0x%x\n", psecuritypriv->ndisauthtype);
	//DBG_871X("enc_type=0x%x\n", psecuritypriv->ndisencryptstatus);

#if 0
	DBG_871X("dbg(0x210)=0x%x\n", rtw_read32(padapter, 0x210));
	DBG_871X("dbg(0x608)=0x%x\n", rtw_read32(padapter, 0x608));
	DBG_871X("dbg(0x280)=0x%x\n", rtw_read32(padapter, 0x280));
	DBG_871X("dbg(0x284)=0x%x\n", rtw_read32(padapter, 0x284));
	DBG_871X("dbg(0x288)=0x%x\n", rtw_read32(padapter, 0x288));

	DBG_871X("dbg(0x664)=0x%x\n", rtw_read32(padapter, 0x664));


	DBG_871X("\n");

	DBG_871X("dbg(0x430)=0x%x\n", rtw_read32(padapter, 0x430));
	DBG_871X("dbg(0x438)=0x%x\n", rtw_read32(padapter, 0x438));

	DBG_871X("dbg(0x440)=0x%x\n", rtw_read32(padapter, 0x440));

	DBG_871X("dbg(0x458)=0x%x\n", rtw_read32(padapter, 0x458));

	DBG_871X("dbg(0x484)=0x%x\n", rtw_read32(padapter, 0x484));
	DBG_871X("dbg(0x488)=0x%x\n", rtw_read32(padapter, 0x488));

	DBG_871X("dbg(0x444)=0x%x\n", rtw_read32(padapter, 0x444));
	DBG_871X("dbg(0x448)=0x%x\n", rtw_read32(padapter, 0x448));
	DBG_871X("dbg(0x44c)=0x%x\n", rtw_read32(padapter, 0x44c));
	DBG_871X("dbg(0x450)=0x%x\n", rtw_read32(padapter, 0x450));
#endif

	return 0;

}

static int rtw_wx_read32(struct net_device *dev,
                         struct iw_request_info *info,
                         union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter;
	struct iw_point *p;
	u16 len;
	u32 addr;
	u32 data32;
	u32 bytes;
	u8 *ptmp;
	int ret;


	ret = 0;
	padapter = (PADAPTER)rtw_netdev_priv(dev);
	p = &wrqu->data;
	len = p->length;
	if (0 == len)
		return -EINVAL;

	ptmp = (u8*)rtw_malloc(len);
	if (NULL == ptmp)
		return -ENOMEM;

	if (copy_from_user(ptmp, p->pointer, len)) {
		ret = -EFAULT;
		goto exit;
	}

	bytes = 0;
	addr = 0;
	sscanf(ptmp, "%d,%x", &bytes, &addr);

	switch (bytes) {
	case 1:
		data32 = rtw_read8(padapter, addr);
		sprintf(extra, "0x%02X", data32);
		break;
	case 2:
		data32 = rtw_read16(padapter, addr);
		sprintf(extra, "0x%04X", data32);
		break;
	case 4:
		data32 = rtw_read32(padapter, addr);
		sprintf(extra, "0x%08X", data32);
		break;
	default:
		DBG_871X(KERN_INFO "%s: usage> read [bytes],[address(hex)]\n", __func__);
		ret = -EINVAL;
		goto exit;
	}
	DBG_871X(KERN_INFO "%s: addr=0x%08X data=%s\n", __func__, addr, extra);

exit:
	rtw_mfree(ptmp, len);

	return 0;
}

static int rtw_wx_write32(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = (PADAPTER)rtw_netdev_priv(dev);

	u32 addr;
	u32 data32;
	u32 bytes;


	bytes = 0;
	addr = 0;
	data32 = 0;
	sscanf(extra, "%d,%x,%x", &bytes, &addr, &data32);

	switch (bytes) {
	case 1:
		rtw_write8(padapter, addr, (u8)data32);
		DBG_871X(KERN_INFO "%s: addr=0x%08X data=0x%02X\n", __func__, addr, (u8)data32);
		break;
	case 2:
		rtw_write16(padapter, addr, (u16)data32);
		DBG_871X(KERN_INFO "%s: addr=0x%08X data=0x%04X\n", __func__, addr, (u16)data32);
		break;
	case 4:
		rtw_write32(padapter, addr, data32);
		DBG_871X(KERN_INFO "%s: addr=0x%08X data=0x%08X\n", __func__, addr, data32);
		break;
	default:
		DBG_871X(KERN_INFO "%s: usage> write [bytes],[address(hex)],[data(hex)]\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int rtw_wx_read_rf(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u32 path, addr, data32;


	path = *(u32*)extra;
	addr = *((u32*)extra + 1);
	data32 = rtw_hal_read_rfreg(padapter, path, addr, 0xFFFFF);
//	DBG_871X("%s: path=%d addr=0x%02x data=0x%05x\n", __func__, path, addr, data32);
	/*
	 * IMPORTANT!!
	 * Only when wireless private ioctl is at odd order,
	 * "extra" would be copied to user space.
	 */
	sprintf(extra, "0x%05x", data32);

	return 0;
}

static int rtw_wx_write_rf(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u32 path, addr, data32;


	path = *(u32*)extra;
	addr = *((u32*)extra + 1);
	data32 = *((u32*)extra + 2);
//	DBG_871X("%s: path=%d addr=0x%02x data=0x%05x\n", __func__, path, addr, data32);
	rtw_hal_write_rfreg(padapter, path, addr, 0xFFFFF, data32);

	return 0;
}

static int rtw_wx_priv_null(struct net_device *dev, struct iw_request_info *a,
                            union iwreq_data *wrqu, char *b)
{
	return -1;
}

static int dummy(struct net_device *dev, struct iw_request_info *a,
                 union iwreq_data *wrqu, char *b)
{
	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	//DBG_871X("cmd_code=%x, fwstate=0x%x\n", a->cmd, get_fwstate(pmlmepriv));

	return -1;

}

static int rtw_wx_set_channel_plan(struct net_device *dev,
                                   struct iw_request_info *info,
                                   union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct registry_priv *pregistrypriv = &padapter->registrypriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	//extern int rtw_channel_plan;
	u8 channel_plan_req = (u8) (*((int *)wrqu));

#if 0
	rtw_channel_plan = (int)wrqu->data.pointer;
	pregistrypriv->channel_plan = rtw_channel_plan;
	pmlmepriv->ChannelPlan = pregistrypriv->channel_plan;
#endif

	if (_SUCCESS == rtw_set_chplan_cmd(padapter, channel_plan_req, 1, 1)) {
		DBG_871X("%s set channel_plan = 0x%02X\n", __func__, pmlmepriv->ChannelPlan);
	} else
		return -EPERM;

	return 0;
}

static int rtw_wx_set_mtk_wps_probe_ie(struct net_device *dev,
                                       struct iw_request_info *a,
                                       union iwreq_data *wrqu, char *b)
{
#ifdef CONFIG_PLATFORM_MT53XX
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_notice_,
	         ("WLAN IOCTL: cmd_code=%x, fwstate=0x%x\n",
	          a->cmd, get_fwstate(pmlmepriv)));
#endif
	return 0;
}

static int rtw_wx_get_sensitivity(struct net_device *dev,
                                  struct iw_request_info *info,
                                  union iwreq_data *wrqu, char *buf)
{
#ifdef CONFIG_PLATFORM_MT53XX
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	//	Modified by Albert 20110914
	//	This is in dbm format for MTK platform.
	wrqu->qual.level = padapter->recvpriv.rssi;
	DBG_871X(" level = %u\n",  wrqu->qual.level );
#endif
	return 0;
}

static int rtw_wx_set_mtk_wps_ie(struct net_device *dev,
                                 struct iw_request_info *info,
                                 union iwreq_data *wrqu, char *extra)
{
#ifdef CONFIG_PLATFORM_MT53XX
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	return rtw_set_wpa_ie(padapter, wrqu->data.pointer, wrqu->data.length);
#else
	return 0;
#endif
}

/*
typedef int (*iw_handler)(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra);
*/
/*
 *	For all data larger than 16 octets, we need to use a
 *	pointer to memory allocated in user space.
 */
static  int rtw_drvext_hdl(struct net_device *dev, struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{

#if 0
	struct	iw_point {
		void __user	*pointer;	/* Pointer to the data  (in user space) */
		__u16		length;		/* number of fields or size in bytes */
		__u16		flags;		/* Optional params */
	};
#endif

#ifdef CONFIG_DRVEXT_MODULE
	u8 res;
	struct drvext_handler *phandler;
	struct drvext_oidparam *poidparam;
	int ret;
	u16 len;
	u8 *pparmbuf, bset;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct iw_point *p = &wrqu->data;

	if( (!p->length) || (!p->pointer)) {
		ret = -EINVAL;
		goto _rtw_drvext_hdl_exit;
	}


	bset = (u8)(p->flags&0xFFFF);
	len = p->length;
	pparmbuf = (u8*)rtw_malloc(len);
	if (pparmbuf == NULL) {
		ret = -ENOMEM;
		goto _rtw_drvext_hdl_exit;
	}

	if(bset) { //set info
		if (copy_from_user(pparmbuf, p->pointer,len)) {
			rtw_mfree(pparmbuf, len);
			ret = -EFAULT;
			goto _rtw_drvext_hdl_exit;
		}
	} else { //query info

	}


	//
	poidparam = (struct drvext_oidparam *)pparmbuf;

	RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("drvext set oid subcode [%d], len[%d], InformationBufferLength[%d]\r\n",
	         poidparam->subcode, poidparam->len, len));


	//check subcode
	if ( poidparam->subcode >= MAX_DRVEXT_HANDLERS) {
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("no matching drvext handlers\r\n"));
		ret = -EINVAL;
		goto _rtw_drvext_hdl_exit;
	}


	if ( poidparam->subcode >= MAX_DRVEXT_OID_SUBCODES) {
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("no matching drvext subcodes\r\n"));
		ret = -EINVAL;
		goto _rtw_drvext_hdl_exit;
	}


	phandler = drvextoidhandlers + poidparam->subcode;

	if (poidparam->len != phandler->parmsize) {
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("no matching drvext param size %d vs %d\r\n",
		         poidparam->len , phandler->parmsize));
		ret = -EINVAL;
		goto _rtw_drvext_hdl_exit;
	}


	res = phandler->handler(&padapter->drvextpriv, bset, poidparam->data);

	if(res==0) {
		ret = 0;

		if (bset == 0x00) { //query info
			//_rtw_memcpy(p->pointer, pparmbuf, len);
			if (copy_to_user(p->pointer, pparmbuf, len))
				ret = -EFAULT;
		}
	} else
		ret = -EFAULT;


_rtw_drvext_hdl_exit:

	return ret;

#endif

	return 0;

}

static void rtw_dbg_mode_hdl(_adapter *padapter, u32 id, u8 *pdata, u32 len)
{
	pRW_Reg 	RegRWStruct;
	struct rf_reg_param *prfreg;
	u8 path;
	u8 offset;
	u32 value;

	DBG_871X("%s\n", __FUNCTION__);

	switch(id) {
	case GEN_MP_IOCTL_SUBCODE(MP_START):
		DBG_871X("871x_driver is only for normal mode, can't enter mp mode\n");
		break;
	case GEN_MP_IOCTL_SUBCODE(READ_REG):
		RegRWStruct = (pRW_Reg)pdata;
		switch (RegRWStruct->width) {
		case 1:
			RegRWStruct->value = rtw_read8(padapter, RegRWStruct->offset);
			break;
		case 2:
			RegRWStruct->value = rtw_read16(padapter, RegRWStruct->offset);
			break;
		case 4:
			RegRWStruct->value = rtw_read32(padapter, RegRWStruct->offset);
			break;
		default:
			break;
		}

		break;
	case GEN_MP_IOCTL_SUBCODE(WRITE_REG):
		RegRWStruct = (pRW_Reg)pdata;
		switch (RegRWStruct->width) {
		case 1:
			rtw_write8(padapter, RegRWStruct->offset, (u8)RegRWStruct->value);
			break;
		case 2:
			rtw_write16(padapter, RegRWStruct->offset, (u16)RegRWStruct->value);
			break;
		case 4:
			rtw_write32(padapter, RegRWStruct->offset, (u32)RegRWStruct->value);
			break;
		default:
			break;
		}

		break;
	case GEN_MP_IOCTL_SUBCODE(READ_RF_REG):

		prfreg = (struct rf_reg_param *)pdata;

		path = (u8)prfreg->path;
		offset = (u8)prfreg->offset;

		value = rtw_hal_read_rfreg(padapter, path, offset, 0xffffffff);

		prfreg->value = value;

		break;
	case GEN_MP_IOCTL_SUBCODE(WRITE_RF_REG):

		prfreg = (struct rf_reg_param *)pdata;

		path = (u8)prfreg->path;
		offset = (u8)prfreg->offset;
		value = prfreg->value;

		rtw_hal_write_rfreg(padapter, path, offset, 0xffffffff, value);

		break;
	case GEN_MP_IOCTL_SUBCODE(TRIGGER_GPIO):
		DBG_871X("==> trigger gpio 0\n");
		rtw_hal_set_hwreg(padapter, HW_VAR_TRIGGER_GPIO_0, 0);
		break;
#ifdef CONFIG_BT_COEXIST
	case GEN_MP_IOCTL_SUBCODE(SET_DM_BT):
		DBG_871X("==> set dm_bt_coexist:%x\n",*(u8 *)pdata);
		rtw_hal_set_hwreg(padapter, HW_VAR_BT_SET_COEXIST, pdata);
		break;
	case GEN_MP_IOCTL_SUBCODE(DEL_BA):
		DBG_871X("==> delete ba:%x\n",*(u8 *)pdata);
		rtw_hal_set_hwreg(padapter, HW_VAR_BT_ISSUE_DELBA, pdata);
		break;
#endif
#ifdef DBG_CONFIG_ERROR_DETECT
	case GEN_MP_IOCTL_SUBCODE(GET_WIFI_STATUS):
		*pdata = rtw_hal_sreset_get_wifi_status(padapter);
		break;
#endif

	default:
		break;
	}

}

static int rtw_mp_ioctl_hdl(struct net_device *dev, struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	u32 BytesRead, BytesWritten, BytesNeeded;
	struct oid_par_priv	oid_par;
	struct mp_ioctl_handler	*phandler;
	struct mp_ioctl_param	*poidparam;
	uint status=0;
	u16 len;
	u8 *pparmbuf = NULL, bset;
	PADAPTER padapter = (PADAPTER)rtw_netdev_priv(dev);
	struct iw_point *p = &wrqu->data;

	//DBG_871X("+rtw_mp_ioctl_hdl\n");

	//mutex_lock(&ioctl_mutex);

	if ((!p->length) || (!p->pointer)) {
		ret = -EINVAL;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	pparmbuf = NULL;
	bset = (u8)(p->flags & 0xFFFF);
	len = p->length;
	pparmbuf = (u8*)rtw_malloc(len);
	if (pparmbuf == NULL) {
		ret = -ENOMEM;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	if (copy_from_user(pparmbuf, p->pointer, len)) {
		ret = -EFAULT;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	poidparam = (struct mp_ioctl_param *)pparmbuf;
	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
	         ("rtw_mp_ioctl_hdl: subcode [%d], len[%d], buffer_len[%d]\r\n",
	          poidparam->subcode, poidparam->len, len));

	if (poidparam->subcode >= MAX_MP_IOCTL_SUBCODE) {
		RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_err_, ("no matching drvext subcodes\r\n"));
		ret = -EINVAL;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	//DBG_871X("%s: %d\n", __func__, poidparam->subcode);
#ifdef CONFIG_MP_INCLUDED
	if (padapter->registrypriv.mp_mode == 1) {
		phandler = mp_ioctl_hdl + poidparam->subcode;

		if ((phandler->paramsize != 0) && (poidparam->len < phandler->paramsize)) {
			RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_err_,
			         ("no matching drvext param size %d vs %d\r\n",
			          poidparam->len, phandler->paramsize));
			ret = -EINVAL;
			goto _rtw_mp_ioctl_hdl_exit;
		}

		if (phandler->handler) {
			oid_par.adapter_context = padapter;
			oid_par.oid = phandler->oid;
			oid_par.information_buf = poidparam->data;
			oid_par.information_buf_len = poidparam->len;
			oid_par.dbg = 0;

			BytesWritten = 0;
			BytesNeeded = 0;

			if (bset) {
				oid_par.bytes_rw = &BytesRead;
				oid_par.bytes_needed = &BytesNeeded;
				oid_par.type_of_oid = SET_OID;
			} else {
				oid_par.bytes_rw = &BytesWritten;
				oid_par.bytes_needed = &BytesNeeded;
				oid_par.type_of_oid = QUERY_OID;
			}

			status = phandler->handler(&oid_par);

			//todo:check status, BytesNeeded, etc.
		} else {
			DBG_871X("rtw_mp_ioctl_hdl(): err!, subcode=%d, oid=%d, handler=%p\n",
			         poidparam->subcode, phandler->oid, phandler->handler);
			ret = -EFAULT;
			goto _rtw_mp_ioctl_hdl_exit;
		}
	} else
#endif
	{
		rtw_dbg_mode_hdl(padapter, poidparam->subcode, poidparam->data, poidparam->len);
	}

	if (bset == 0x00) { //query info
		if (copy_to_user(p->pointer, pparmbuf, len))
			ret = -EFAULT;
	}

	if (status) {
		ret = -EFAULT;
		goto _rtw_mp_ioctl_hdl_exit;
	}

_rtw_mp_ioctl_hdl_exit:

	if (pparmbuf)
		rtw_mfree(pparmbuf, len);

	//mutex_unlock(&ioctl_mutex);

	return ret;
}

static int rtw_get_ap_info(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	u32 cnt=0, wpa_ielen;
	_irqL	irqL;
	_list	*plist, *phead;
	unsigned char *pbuf;
	u8 bssid[ETH_ALEN];
	char data[32];
	struct wlan_network *pnetwork = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	_queue *queue = &(pmlmepriv->scanned_queue);
	struct iw_point *pdata = &wrqu->data;

	DBG_871X("+rtw_get_aplist_info\n");

	if((padapter->bDriverStopped) || (pdata==NULL)) {
		ret= -EINVAL;
		goto exit;
	}

	while((check_fwstate(pmlmepriv, (_FW_UNDER_SURVEY|_FW_UNDER_LINKING))) == _TRUE) {
		rtw_msleep_os(30);
		cnt++;
		if(cnt > 100)
			break;
	}


	//pdata->length = 0;//?
	pdata->flags = 0;
	if(pdata->length>=32) {
		if(copy_from_user(data, pdata->pointer, 32)) {
			ret= -EINVAL;
			goto exit;
		}
	} else {
		ret= -EINVAL;
		goto exit;
	}

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while(1) {
		if (rtw_end_of_queue_search(phead,plist)== _TRUE)
			break;


		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		//if(hwaddr_aton_i(pdata->pointer, bssid))
		if(hwaddr_aton_i(data, bssid)) {
			DBG_871X("Invalid BSSID '%s'.\n", (u8*)data);
			_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
			return -EINVAL;
		}


		if(_rtw_memcmp(bssid, pnetwork->network.MacAddress, ETH_ALEN) == _TRUE) { //BSSID match, then check if supporting wpa/wpa2
			DBG_871X("BSSID:" MAC_FMT "\n", MAC_ARG(bssid));

			pbuf = rtw_get_wpa_ie(&pnetwork->network.IEs[12], &wpa_ielen, pnetwork->network.IELength-12);
			if(pbuf && (wpa_ielen>0)) {
				pdata->flags = 1;
				break;
			}

			pbuf = rtw_get_wpa2_ie(&pnetwork->network.IEs[12], &wpa_ielen, pnetwork->network.IELength-12);
			if(pbuf && (wpa_ielen>0)) {
				pdata->flags = 2;
				break;
			}

		}

		plist = get_next(plist);

	}

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	if(pdata->length>=34) {
		if(copy_to_user((u8*)pdata->pointer+32, (u8*)&pdata->flags, 1)) {
			ret= -EINVAL;
			goto exit;
		}
	}

exit:

	return ret;

}

static int rtw_set_pid(struct net_device *dev,
                       struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = rtw_netdev_priv(dev);
	int *pdata = (int *)wrqu;
	int selector;

	if((padapter->bDriverStopped) || (pdata==NULL)) {
		ret= -EINVAL;
		goto exit;
	}

	selector = *pdata;
	if(selector < 3 && selector >=0) {
		padapter->pid[selector] = *(pdata+1);
#ifdef CONFIG_GLOBAL_UI_PID
		ui_pid[selector] = *(pdata+1);
#endif
		DBG_871X("%s set pid[%d]=%d\n", __FUNCTION__, selector ,padapter->pid[selector]);
	} else
		DBG_871X("%s selector %d error\n", __FUNCTION__, selector);

exit:

	return ret;

}

static int rtw_wps_start(struct net_device *dev,
                         struct iw_request_info *info,
                         union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	u32   u32wps_start = 0;
	unsigned int uintRet = 0;

	if((_TRUE == padapter->bDriverStopped) ||(_TRUE==padapter->bSurpriseRemoved) || (NULL== pdata)) {
		ret= -EINVAL;
		goto exit;
	}

	uintRet = copy_from_user( ( void* ) &u32wps_start, pdata->pointer, 4 );
	if ( u32wps_start == 0 ) {
		u32wps_start = *extra;
	}

	DBG_871X( "[%s] wps_start = %d\n", __FUNCTION__, u32wps_start );

	if ( u32wps_start == 1 ) { // WPS Start
		rtw_led_control(padapter, LED_CTL_START_WPS);
	} else if ( u32wps_start == 2 ) { // WPS Stop because of wps success
		rtw_led_control(padapter, LED_CTL_STOP_WPS);
	} else if ( u32wps_start == 3 ) { // WPS Stop because of wps fail
		rtw_led_control(padapter, LED_CTL_STOP_WPS_FAIL);
	}

#ifdef CONFIG_INTEL_WIDI
	process_intel_widi_wps_status(padapter, u32wps_start);
#endif //CONFIG_INTEL_WIDI

exit:

	return ret;

}

#ifdef CONFIG_P2P
static int rtw_wext_p2p_enable(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	enum P2P_ROLE init_role = P2P_ROLE_DISABLE;

	if(*extra == '0' )
		init_role = P2P_ROLE_DISABLE;
	else if(*extra == '1')
		init_role = P2P_ROLE_DEVICE;
	else if(*extra == '2')
		init_role = P2P_ROLE_CLIENT;
	else if(*extra == '3')
		init_role = P2P_ROLE_GO;

	if(_FAIL == rtw_p2p_enable(padapter, init_role)) {
		ret = -EFAULT;
		goto exit;
	}

	//set channel/bandwidth
	if(init_role != P2P_ROLE_DISABLE) {
		u8 channel, ch_offset;
		u16 bwmode;

		if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_LISTEN)) {
			//	Stay at the listen state and wait for discovery.
			channel = pwdinfo->listen_channel;
			pwdinfo->operating_channel = pwdinfo->listen_channel;
			ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
			bwmode = CHANNEL_WIDTH_20;
		}
#ifdef CONFIG_CONCURRENT_MODE
		else if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_IDLE)) {
			_adapter				*pbuddy_adapter = padapter->pbuddy_adapter;
			//struct wifidirect_info	*pbuddy_wdinfo = &pbuddy_adapter->wdinfo;
			struct mlme_priv		*pbuddy_mlmepriv = &pbuddy_adapter->mlmepriv;
			struct mlme_ext_priv	*pbuddy_mlmeext = &pbuddy_adapter->mlmeextpriv;

			_set_timer( &pwdinfo->ap_p2p_switch_timer, pwdinfo->ext_listen_interval );
			if ( check_fwstate( pbuddy_mlmepriv, _FW_LINKED ) ) {
				pwdinfo->operating_channel = pbuddy_mlmeext->cur_channel;
				//	How about the ch_offset and bwmode ??
			} else {
				pwdinfo->operating_channel = pwdinfo->listen_channel;
			}

			channel = pbuddy_mlmeext->cur_channel;
			ch_offset = pbuddy_mlmeext->cur_ch_offset;
			bwmode = pbuddy_mlmeext->cur_bwmode;
		}
#endif
		else {
			pwdinfo->operating_channel = pmlmeext->cur_channel;

			channel = pwdinfo->operating_channel;
			ch_offset = pmlmeext->cur_ch_offset;
			bwmode = pmlmeext->cur_bwmode;
		}

		set_channel_bwmode(padapter, channel, ch_offset, bwmode);
	}

exit:
	return ret;

}

static int rtw_p2p_set_go_nego_ssid(struct net_device *dev,
                                    struct iw_request_info *info,
                                    union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	DBG_871X( "[%s] ssid = %s, len = %zu\n", __FUNCTION__, extra, strlen( extra ) );
	_rtw_memcpy( pwdinfo->nego_ssid, extra, strlen( extra ) );
	pwdinfo->nego_ssidlen = strlen( extra );

	return ret;

}


static int rtw_p2p_set_intent(struct net_device *dev,
                              struct iw_request_info *info,
                              union iwreq_data *wrqu, char *extra)
{
	int 							ret = 0;
	_adapter 						*padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info 			*pwdinfo= &(padapter->wdinfo);
	u8							intent = pwdinfo->intent;

	extra[ wrqu->data.length ] = 0x00;

	intent = rtw_atoi( extra );

	if ( intent <= 15 ) {
		pwdinfo->intent= intent;
	} else {
		ret = -1;
	}

	DBG_871X( "[%s] intent = %d\n", __FUNCTION__, intent);

	return ret;

}

static int rtw_p2p_set_listen_ch(struct net_device *dev,
                                 struct iw_request_info *info,
                                 union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	u8	listen_ch = pwdinfo->listen_channel;	//	Listen channel number

	extra[ wrqu->data.length ] = 0x00;
	listen_ch = rtw_atoi( extra );

	if ( ( listen_ch == 1 ) || ( listen_ch == 6 ) || ( listen_ch == 11 ) ) {
		pwdinfo->listen_channel = listen_ch;
		set_channel_bwmode(padapter, pwdinfo->listen_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, CHANNEL_WIDTH_20);
	} else {
		ret = -1;
	}

	DBG_871X( "[%s] listen_ch = %d\n", __FUNCTION__, pwdinfo->listen_channel );

	return ret;

}

static int rtw_p2p_set_op_ch(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra)
{
//	Commented by Albert 20110524
//	This function is used to set the operating channel if the driver will become the group owner

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	u8	op_ch = pwdinfo->operating_channel;	//	Operating channel number

	extra[ wrqu->data.length ] = 0x00;

	op_ch = ( u8 ) rtw_atoi( extra );
	if ( op_ch > 0 ) {
		pwdinfo->operating_channel = op_ch;
	} else {
		ret = -1;
	}

	DBG_871X( "[%s] op_ch = %d\n", __FUNCTION__, pwdinfo->operating_channel );

	return ret;

}


static int rtw_p2p_profilefound(struct net_device *dev,
                                struct iw_request_info *info,
                                union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	//	Comment by Albert 2010/10/13
	//	Input data format:
	//	Ex:  0
	//	Ex:  1XX:XX:XX:XX:XX:XXYYSSID
	//	0 => Reflush the profile record list.
	//	1 => Add the profile list
	//	XX:XX:XX:XX:XX:XX => peer's MAC Address ( ex: 00:E0:4C:00:00:01 )
	//	YY => SSID Length
	//	SSID => SSID for persistence group

	DBG_871X( "[%s] In value = %s, len = %d \n", __FUNCTION__, extra, wrqu->data.length -1);


	//	The upper application should pass the SSID to driver by using this rtw_p2p_profilefound function.
	if(!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)) {
		if ( extra[ 0 ] == '0' ) {
			//	Remove all the profile information of wifidirect_info structure.
			_rtw_memset( &pwdinfo->profileinfo[ 0 ], 0x00, sizeof( struct profile_info ) * P2P_MAX_PERSISTENT_GROUP_NUM );
			pwdinfo->profileindex = 0;
		} else {
			if ( pwdinfo->profileindex >= P2P_MAX_PERSISTENT_GROUP_NUM ) {
				ret = -1;
			} else {
				int jj, kk;

				//	Add this profile information into pwdinfo->profileinfo
				//	Ex:  1XX:XX:XX:XX:XX:XXYYSSID
				for( jj = 0, kk = 1; jj < ETH_ALEN; jj++, kk += 3 ) {
					pwdinfo->profileinfo[ pwdinfo->profileindex ].peermac[ jj ] = key_2char2num(extra[ kk ], extra[ kk+ 1 ]);
				}

				//pwdinfo->profileinfo[ pwdinfo->profileindex ].ssidlen = ( extra[18] - '0' ) * 10 + ( extra[ 19 ] - '0' );
				//_rtw_memcpy( pwdinfo->profileinfo[ pwdinfo->profileindex ].ssid, &extra[ 20 ], pwdinfo->profileinfo[ pwdinfo->profileindex ].ssidlen );
				pwdinfo->profileindex++;
			}
		}
	}

	return ret;

}

static int rtw_p2p_setDN(struct net_device *dev,
                         struct iw_request_info *info,
                         union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);


	DBG_871X( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );
	_rtw_memset( pwdinfo->device_name, 0x00, WPS_MAX_DEVICE_NAME_LEN );
	_rtw_memcpy( pwdinfo->device_name, extra, wrqu->data.length - 1 );
	pwdinfo->device_name_len = wrqu->data.length - 1;

	return ret;

}


static int rtw_p2p_get_status(struct net_device *dev,
                              struct iw_request_info *info,
                              union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
#ifdef CONFIG_CONCURRENT_MODE
	//_adapter				*pbuddy_adapter = padapter->pbuddy_adapter;
	//struct wifidirect_info	*pbuddy_wdinfo = &pbuddy_adapter->wdinfo;
	//struct mlme_priv		*pbuddy_mlmepriv = &pbuddy_adapter->mlmepriv;
	//struct mlme_ext_priv	*pbuddy_mlmeext = &pbuddy_adapter->mlmeextpriv;
#endif

	if ( padapter->bShowGetP2PState ) {
		DBG_871X( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
		          pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
		          pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);
	}

	//	Commented by Albert 2010/10/12
	//	Because of the output size limitation, I had removed the "Role" information.
	//	About the "Role" information, we will use the new private IOCTL to get the "Role" information.
	sprintf( extra, "\n\nStatus=%.2d\n", rtw_p2p_state(pwdinfo) );
	wrqu->data.length = strlen( extra );

	return ret;

}

//	Commented by Albert 20110520
//	This function will return the config method description
//	This config method description will show us which config method the remote P2P device is intented to use
//	by sending the provisioning discovery request frame.

static int rtw_p2p_get_req_cm(struct net_device *dev,
                              struct iw_request_info *info,
                              union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n\nCM=%s\n", pwdinfo->rx_prov_disc_info.strconfig_method_desc_of_prov_disc_req );
	wrqu->data.length = strlen( extra );
	return ret;

}


static int rtw_p2p_get_role(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_871X( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
	          pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
	          pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);

	sprintf( extra, "\n\nRole=%.2d\n", rtw_p2p_role(pwdinfo) );
	wrqu->data.length = strlen( extra );
	return ret;

}


static int rtw_p2p_get_peer_ifaddr(struct net_device *dev,
                                   struct iw_request_info *info,
                                   union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_871X( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
	          pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
	          pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);

	sprintf( extra, "\nMAC %.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
	         pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
	         pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_peer_devaddr(struct net_device *dev,
                                    struct iw_request_info *info,
                                    union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	DBG_871X( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
	          pwdinfo->rx_prov_disc_info.peerDevAddr[ 0 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 1 ],
	          pwdinfo->rx_prov_disc_info.peerDevAddr[ 2 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 3 ],
	          pwdinfo->rx_prov_disc_info.peerDevAddr[ 4 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 5 ]);
	sprintf( extra, "\n%.2X%.2X%.2X%.2X%.2X%.2X",
	         pwdinfo->rx_prov_disc_info.peerDevAddr[ 0 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 1 ],
	         pwdinfo->rx_prov_disc_info.peerDevAddr[ 2 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 3 ],
	         pwdinfo->rx_prov_disc_info.peerDevAddr[ 4 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 5 ]);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_peer_devaddr_by_invitation(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	DBG_871X( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
	          pwdinfo->p2p_peer_device_addr[ 0 ], pwdinfo->p2p_peer_device_addr[ 1 ],
	          pwdinfo->p2p_peer_device_addr[ 2 ], pwdinfo->p2p_peer_device_addr[ 3 ],
	          pwdinfo->p2p_peer_device_addr[ 4 ], pwdinfo->p2p_peer_device_addr[ 5 ]);
	sprintf( extra, "\nMAC %.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
	         pwdinfo->p2p_peer_device_addr[ 0 ], pwdinfo->p2p_peer_device_addr[ 1 ],
	         pwdinfo->p2p_peer_device_addr[ 2 ], pwdinfo->p2p_peer_device_addr[ 3 ],
	         pwdinfo->p2p_peer_device_addr[ 4 ], pwdinfo->p2p_peer_device_addr[ 5 ]);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_groupid(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n%.2X:%.2X:%.2X:%.2X:%.2X:%.2X %s",
	         pwdinfo->groupid_info.go_device_addr[ 0 ], pwdinfo->groupid_info.go_device_addr[ 1 ],
	         pwdinfo->groupid_info.go_device_addr[ 2 ], pwdinfo->groupid_info.go_device_addr[ 3 ],
	         pwdinfo->groupid_info.go_device_addr[ 4 ], pwdinfo->groupid_info.go_device_addr[ 5 ],
	         pwdinfo->groupid_info.ssid);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_op_ch(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_871X( "[%s] Op_ch = %02x\n", __FUNCTION__, pwdinfo->operating_channel);

	sprintf( extra, "\n\nOp_ch=%.2d\n", pwdinfo->operating_channel );
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_wps_configmethod(struct net_device *dev,
                                        struct iw_request_info *info,
                                        union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	_irqL irqL;
	_list * plist,*phead;
	_queue *queue = &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork = NULL;
	u8 blnMatch = 0;
	u16	attr_content = 0;
	uint attr_contentlen = 0;
	u8	attr_content_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	//	Commented by Albert 20110727
	//	The input data is the MAC address which the application wants to know its WPS config method.
	//	After knowing its WPS config method, the application can decide the config method for provisioning discovery.
	//	Format: iwpriv wlanx p2p_get_wpsCM 00:E0:4C:00:00:05

	DBG_871X("[%s] data = %s\n", __FUNCTION__, subcmd);

	macstr2num(peerMAC, subcmd);

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1) {
		if (rtw_end_of_queue_search(phead, plist) == _TRUE) break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (_rtw_memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN)) {
			u8 *wpsie;
			uint	wpsie_len = 0;

			//	The mac address is matched.

			if ( (wpsie=rtw_get_wps_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &wpsie_len, pnetwork->network.Reserved[0])) ) {
				rtw_get_wps_attr_content(wpsie, wpsie_len, WPS_ATTR_CONF_METHOD, (u8 *)&attr_content, &attr_contentlen);
				if (attr_contentlen) {
					attr_content = be16_to_cpu(attr_content);
					sprintf(attr_content_str, "\n\nM=%.4d", attr_content);
					blnMatch = 1;
				}
			}

			break;
		}

		plist = get_next(plist);

	}

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	if (!blnMatch) {
		sprintf(attr_content_str, "\n\nM=0000");
	}

	wrqu->data.length = strlen(attr_content_str);
	_rtw_memcpy(extra, attr_content_str, wrqu->data.length);

	return ret;

}

#ifdef CONFIG_WFD
static int rtw_p2p_get_peer_wfd_port(struct net_device *dev,
                                     struct iw_request_info *info,
                                     union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	DBG_871X( "[%s] p2p_state = %d\n", __FUNCTION__, rtw_p2p_state(pwdinfo) );

	sprintf( extra, "\n\nPort=%d\n", pwdinfo->wfd_info->peer_rtsp_ctrlport );
	DBG_871X( "[%s] remote port = %d\n", __FUNCTION__, pwdinfo->wfd_info->peer_rtsp_ctrlport );

	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_peer_wfd_preferred_connection(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n\nwfd_pc=%d\n", pwdinfo->wfd_info->wfd_pc );
	DBG_871X( "[%s] wfd_pc = %d\n", __FUNCTION__, pwdinfo->wfd_info->wfd_pc );

	wrqu->data.length = strlen( extra );
	pwdinfo->wfd_info->wfd_pc = _FALSE;	//	Reset the WFD preferred connection to P2P
	return ret;

}

static int rtw_p2p_get_peer_wfd_session_available(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n\nwfd_sa=%d\n", pwdinfo->wfd_info->peer_session_avail );
	DBG_871X( "[%s] wfd_sa = %d\n", __FUNCTION__, pwdinfo->wfd_info->peer_session_avail );

	wrqu->data.length = strlen( extra );
	pwdinfo->wfd_info->peer_session_avail = _TRUE;	//	Reset the WFD session available
	return ret;

}

#endif // CONFIG_WFD

static int rtw_p2p_get_go_device_address(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	_irqL irqL;
	_list *plist, *phead;
	_queue *queue	= &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork = NULL;
	u8 blnMatch = 0;
	u8 *p2pie;
	uint p2pielen = 0, attr_contentlen = 0;
	u8 attr_content[100] = { 0x00 };
	u8 go_devadd_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	//	Commented by Albert 20121209
	//	The input data is the GO's interface address which the application wants to know its device address.
	//	Format: iwpriv wlanx p2p_get2 go_devadd=00:E0:4C:00:00:05

	DBG_871X("[%s] data = %s\n", __FUNCTION__, subcmd);

	macstr2num(peerMAC, subcmd);

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1) {
		if (rtw_end_of_queue_search(phead, plist) == _TRUE) break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (_rtw_memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN)) {
			//	Commented by Albert 2011/05/18
			//	Match the device address located in the P2P IE
			//	This is for the case that the P2P device address is not the same as the P2P interface address.

			if ((p2pie = rtw_get_p2p_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &p2pielen, pnetwork->network.Reserved[0]))) {
				while (p2pie) {
					//	The P2P Device ID attribute is included in the Beacon frame.
					//	The P2P Device Info attribute is included in the probe response frame.

					_rtw_memset(attr_content, 0x00, 100);
					if (rtw_get_p2p_attr_content(p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen)) {
						//	Handle the P2P Device ID attribute of Beacon first
						blnMatch = 1;
						break;

					} else if (rtw_get_p2p_attr_content(p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen)) {
						//	Handle the P2P Device Info attribute of probe response
						blnMatch = 1;
						break;
					}

					//Get the next P2P IE
					p2pie = rtw_get_p2p_ie(p2pie + p2pielen, pnetwork->network.IELength - 12 - (p2pie - &pnetwork->network.IEs[12] + p2pielen), NULL, &p2pielen);
				}
			}
		}

		plist = get_next(plist);

	}

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	if (!blnMatch) {
		sprintf(go_devadd_str, "\n\ndev_add=NULL");
	} else {
		sprintf(go_devadd_str, "\n\ndev_add=%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
		        attr_content[0], attr_content[1], attr_content[2], attr_content[3], attr_content[4], attr_content[5]);
	}

	wrqu->data.length = strlen(go_devadd_str);
	_rtw_memcpy(extra, go_devadd_str, wrqu->data.length);

	return ret;

}

static int rtw_p2p_get_device_type(struct net_device *dev,
                                   struct iw_request_info *info,
                                   union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	_irqL irqL;
	_list *plist, *phead;
	_queue *queue = &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork = NULL;
	u8 blnMatch = 0;
	u8 dev_type[8] = { 0x00 };
	uint dev_type_len = 0;
	u8 dev_type_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };    // +9 is for the str "dev_type=", we have to clear it at wrqu->data.pointer

	//	Commented by Albert 20121209
	//	The input data is the MAC address which the application wants to know its device type.
	//	Such user interface could know the device type.
	//	Format: iwpriv wlanx p2p_get2 dev_type=00:E0:4C:00:00:05

	DBG_871X("[%s] data = %s\n", __FUNCTION__, subcmd);

	macstr2num(peerMAC, subcmd);

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1) {
		if (rtw_end_of_queue_search(phead, plist) == _TRUE) break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (_rtw_memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN)) {
			u8 *wpsie;
			uint	wpsie_len = 0;

			//	The mac address is matched.

			if ( (wpsie=rtw_get_wps_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &wpsie_len, pnetwork->network.Reserved[0])) ) {
				rtw_get_wps_attr_content(wpsie, wpsie_len, WPS_ATTR_PRIMARY_DEV_TYPE, dev_type, &dev_type_len);
				if (dev_type_len) {
					u16	type = 0;

					_rtw_memcpy(&type, dev_type, 2);
					type = be16_to_cpu(type);
					sprintf(dev_type_str, "\n\nN=%.2d", type);
					blnMatch = 1;
				}
			}
			break;
		}

		plist = get_next(plist);

	}

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	if (!blnMatch) {
		sprintf(dev_type_str, "\n\nN=00");
	}

	wrqu->data.length = strlen(dev_type_str);
	_rtw_memcpy(extra, dev_type_str, wrqu->data.length);

	return ret;

}

static int rtw_p2p_get_device_name(struct net_device *dev,
                                   struct iw_request_info *info,
                                   union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	_irqL irqL;
	_list *plist, *phead;
	_queue *queue = &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork = NULL;
	u8 blnMatch = 0;
	u8 dev_name[WPS_MAX_DEVICE_NAME_LEN] = { 0x00 };
	uint dev_len = 0;
	u8 dev_name_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	//	Commented by Albert 20121225
	//	The input data is the MAC address which the application wants to know its device name.
	//	Such user interface could show peer device's device name instead of ssid.
	//	Format: iwpriv wlanx p2p_get2 devN=00:E0:4C:00:00:05

	DBG_871X("[%s] data = %s\n", __FUNCTION__, subcmd);

	macstr2num(peerMAC, subcmd);

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1) {
		if (rtw_end_of_queue_search(phead, plist) == _TRUE) break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (_rtw_memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN)) {
			u8 *wpsie;
			uint	wpsie_len = 0;

			//	The mac address is matched.

			if ( (wpsie=rtw_get_wps_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &wpsie_len, pnetwork->network.Reserved[0])) ) {
				rtw_get_wps_attr_content(wpsie, wpsie_len, WPS_ATTR_DEVICE_NAME, dev_name, &dev_len);
				if (dev_len) {
					sprintf(dev_name_str, "\n\nN=%s", dev_name);
					blnMatch = 1;
				}
			}
			break;
		}

		plist = get_next(plist);

	}

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	if (!blnMatch) {
		sprintf(dev_name_str, "\n\nN=0000");
	}

	wrqu->data.length = strlen(dev_name_str);
	_rtw_memcpy(extra, dev_name_str, wrqu->data.length);

	return ret;

}

static int rtw_p2p_get_invitation_procedure(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	_irqL irqL;
	_list *plist, *phead;
	_queue *queue	= &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork = NULL;
	u8 blnMatch = 0;
	u8 *p2pie;
	uint p2pielen = 0, attr_contentlen = 0;
	u8 attr_content[2] = { 0x00 };
	u8 inv_proc_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	//	Commented by Ouden 20121226
	//	The application wants to know P2P initation procedure is support or not.
	//	Format: iwpriv wlanx p2p_get2 InvProc=00:E0:4C:00:00:05

	DBG_871X("[%s] data = %s\n", __FUNCTION__, subcmd);

	macstr2num(peerMAC, subcmd);

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1) {
		if (rtw_end_of_queue_search(phead, plist) == _TRUE) break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (_rtw_memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN)) {
			//	Commented by Albert 20121226
			//	Match the device address located in the P2P IE
			//	This is for the case that the P2P device address is not the same as the P2P interface address.

			if ((p2pie = rtw_get_p2p_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &p2pielen, pnetwork->network.Reserved[0]))) {
				while (p2pie) {
					//_rtw_memset( attr_content, 0x00, 2);
					if (rtw_get_p2p_attr_content(p2pie, p2pielen, P2P_ATTR_CAPABILITY, attr_content, &attr_contentlen)) {
						//	Handle the P2P capability attribute
						blnMatch = 1;
						break;

					}

					//Get the next P2P IE
					p2pie = rtw_get_p2p_ie(p2pie + p2pielen, pnetwork->network.IELength - 12 - (p2pie - &pnetwork->network.IEs[12] + p2pielen), NULL, &p2pielen);
				}
			}
		}

		plist = get_next(plist);

	}

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	if (!blnMatch) {
		sprintf(inv_proc_str, "\nIP=-1");
	} else {
		if ((attr_content[0] & 0x20) == 0x20)
			sprintf(inv_proc_str, "\nIP=1");
		else
			sprintf(inv_proc_str, "\nIP=0");
	}

	wrqu->data.length = strlen(inv_proc_str);
	_rtw_memcpy(extra, inv_proc_str, wrqu->data.length);

	return ret;

}

static int rtw_p2p_connect(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter 				*padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int 					jj,kk;
	//u8   					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	_irqL				irqL;
	_list					*plist, *phead;
	_queue				*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	uint					uintPeerChannel = 0;
#ifdef CONFIG_CONCURRENT_MODE
	_adapter				*pbuddy_adapter = padapter->pbuddy_adapter;
	struct mlme_priv		*pbuddy_mlmepriv = &pbuddy_adapter->mlmepriv;
	struct mlme_ext_priv	*pbuddy_mlmeext = &pbuddy_adapter->mlmeextpriv;
#endif // CONFIG_CONCURRENT_MODE

	//	Commented by Albert 20110304
	//	The input data contains two informations.
	//	1. First information is the MAC address which wants to formate with
	//	2. Second information is the WPS PINCode or "pbc" string for push button method
	//	Format: 00:E0:4C:00:00:05
	//	Format: 00:E0:4C:00:00:05

	DBG_871X( "[%s] data = %s\n", __FUNCTION__, extra );

	if ( pwdinfo->p2p_state == P2P_STATE_NONE ) {
		DBG_871X( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	}

#ifdef CONFIG_INTEL_WIDI
	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) == _TRUE) {
		DBG_871X( "[%s] WiFi is under survey!\n", __FUNCTION__ );
		return ret;
	}
#endif //CONFIG_INTEL_WIDI

	if ( pwdinfo->ui_got_wps_info == P2P_NO_WPSINFO ) {
		return -1;
	}

	for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 ) {
		peerMAC[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while(1) {
		if (rtw_end_of_queue_search(phead,plist)== _TRUE)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if ( _rtw_memcmp( pnetwork->network.MacAddress, peerMAC, ETH_ALEN ) ) {
			uintPeerChannel = pnetwork->network.Configuration.DSConfig;
			break;
		}

		plist = get_next(plist);

	}

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	if ( uintPeerChannel ) {
#ifdef CONFIG_CONCURRENT_MODE
		if ( check_fwstate( pbuddy_mlmepriv, _FW_LINKED ) ) {
			_cancel_timer_ex( &pwdinfo->ap_p2p_switch_timer );
		}
#endif // CONFIG_CONCURRENT_MODE

		_rtw_memset( &pwdinfo->nego_req_info, 0x00, sizeof( struct tx_nego_req_info ) );
		_rtw_memset( &pwdinfo->groupid_info, 0x00, sizeof( struct group_id_info ) );

		pwdinfo->nego_req_info.peer_channel_num[ 0 ] = uintPeerChannel;
		_rtw_memcpy( pwdinfo->nego_req_info.peerDevAddr, pnetwork->network.MacAddress, ETH_ALEN );
		pwdinfo->nego_req_info.benable = _TRUE;

		_cancel_timer_ex( &pwdinfo->restore_p2p_state_timer );
		if ( rtw_p2p_state(pwdinfo) != P2P_STATE_GONEGO_OK ) {
			//	Restore to the listen state if the current p2p state is not nego OK
			rtw_p2p_set_state(pwdinfo, P2P_STATE_LISTEN );
		}

		rtw_p2p_set_pre_state(pwdinfo, rtw_p2p_state(pwdinfo));
		rtw_p2p_set_state(pwdinfo, P2P_STATE_GONEGO_ING);

#ifdef CONFIG_CONCURRENT_MODE
		if ( check_fwstate( pbuddy_mlmepriv, _FW_LINKED ) ) {
			//	Have to enter the power saving with the AP
			set_channel_bwmode(padapter, pbuddy_mlmeext->cur_channel, pbuddy_mlmeext->cur_ch_offset, pbuddy_mlmeext->cur_bwmode);

			issue_nulldata(pbuddy_adapter, NULL, 1, 3, 500);
		}
#endif // CONFIG_CONCURRENT_MODE

		DBG_871X( "[%s] Start PreTx Procedure!\n", __FUNCTION__ );
		_set_timer( &pwdinfo->pre_tx_scan_timer, P2P_TX_PRESCAN_TIMEOUT );
#ifdef CONFIG_CONCURRENT_MODE
		if ( check_fwstate( pbuddy_mlmepriv, _FW_LINKED ) ) {
			_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_CONCURRENT_GO_NEGO_TIMEOUT );
		} else {
			_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_GO_NEGO_TIMEOUT );
		}
#else
		_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_GO_NEGO_TIMEOUT );
#endif // CONFIG_CONCURRENT_MODE

	} else {
		DBG_871X( "[%s] Not Found in Scanning Queue~\n", __FUNCTION__ );
#ifdef CONFIG_INTEL_WIDI
		_cancel_timer_ex( &pwdinfo->restore_p2p_state_timer );
		rtw_p2p_set_state(pwdinfo, P2P_STATE_FIND_PHASE_SEARCH);
		rtw_p2p_findphase_ex_set(pwdinfo, P2P_FINDPHASE_EX_NONE);
		rtw_free_network_queue(padapter, _TRUE);
		/**
		 * For WiDi, if we can't find candidate device in scanning queue,
		 * driver will do scanning itself
		 */
		_enter_critical_bh(&pmlmepriv->lock, &irqL);
		rtw_sitesurvey_cmd(padapter, NULL, 0, NULL, 0);
		_exit_critical_bh(&pmlmepriv->lock, &irqL);
#endif //CONFIG_INTEL_WIDI
		ret = -1;
	}
//exit:
	return ret;
}

static int rtw_p2p_invite_req(struct net_device *dev,
                              struct iw_request_info *info,
                              union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter 					*padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point 			*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	int 						jj,kk;
	//u8   						peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv			*pmlmepriv = &padapter->mlmepriv;
	_list						*plist, *phead;
	_queue					*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network		*pnetwork = NULL;
	uint						uintPeerChannel = 0;
	u8						attr_content[50] = { 0x00 };
	u8 						*p2pie;
	uint						p2pielen = 0, attr_contentlen = 0;
	_irqL					irqL;
	struct tx_invite_req_info*	pinvite_req_info = &pwdinfo->invitereq_info;
	u8 ie_offset = 12;
#ifdef CONFIG_CONCURRENT_MODE
	_adapter					*pbuddy_adapter = padapter->pbuddy_adapter;
	struct mlme_priv			*pbuddy_mlmepriv = &pbuddy_adapter->mlmepriv;
	struct mlme_ext_priv		*pbuddy_mlmeext = &pbuddy_adapter->mlmeextpriv;
#endif // CONFIG_CONCURRENT_MODE

#ifdef CONFIG_WFD
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;
#endif // CONFIG_WFD

	//	Commented by Albert 20120321
	//	The input data contains two informations.
	//	1. First information is the P2P device address which you want to send to.
	//	2. Second information is the group id which combines with GO's mac address, space and GO's ssid.
	//	Command line sample: iwpriv wlan0 p2p_set invite="00:11:22:33:44:55 00:E0:4C:00:00:05 DIRECT-xy"
	//	Format: 00:11:22:33:44:55 00:E0:4C:00:00:05 DIRECT-xy

	DBG_871X( "[%s] data = %s\n", __FUNCTION__, extra );

	if ( wrqu->data.length <=  37 ) {
		DBG_871X( "[%s] Wrong format!\n", __FUNCTION__ );
		return ret;
	}

	if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)) {
		DBG_871X( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	} else {
		//	Reset the content of struct tx_invite_req_info
		pinvite_req_info->benable = _FALSE;
		_rtw_memset( pinvite_req_info->go_bssid, 0x00, ETH_ALEN );
		_rtw_memset( pinvite_req_info->go_ssid, 0x00, WLAN_SSID_MAXLEN );
		pinvite_req_info->ssidlen = 0x00;
		pinvite_req_info->operating_ch = pwdinfo->operating_channel;
		_rtw_memset( pinvite_req_info->peer_macaddr, 0x00, ETH_ALEN );
		pinvite_req_info->token = 3;
	}

	for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 ) {
		pinvite_req_info->peer_macaddr[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while(1) {
		if (rtw_end_of_queue_search(phead,plist)== _TRUE)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		//	Commented by Albert 2011/05/18
		//	Match the device address located in the P2P IE
		//	This is for the case that the P2P device address is not the same as the P2P interface address.

		ie_offset = (pnetwork->network.Reserved[0] == 2? 0:12);
		if ( (p2pie=rtw_get_p2p_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &p2pielen, pnetwork->network.Reserved[0]))) {
			//	The P2P Device ID attribute is included in the Beacon frame.
			//	The P2P Device Info attribute is included in the probe response frame.

			if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen) ) {
				//	Handle the P2P Device ID attribute of Beacon first
				if ( _rtw_memcmp( attr_content, pinvite_req_info->peer_macaddr, ETH_ALEN ) ) {
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			} else if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen) ) {
				//	Handle the P2P Device Info attribute of probe response
				if ( _rtw_memcmp( attr_content, pinvite_req_info->peer_macaddr, ETH_ALEN ) ) {
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			}

		}

		plist = get_next(plist);

	}

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

#ifdef CONFIG_WFD
	if ( uintPeerChannel ) {
		u8	wfd_ie[ 128 ] = { 0x00 };
		uint	wfd_ielen = 0;

		if ( rtw_get_wfd_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength,  wfd_ie, &wfd_ielen, pnetwork->network.Reserved[0]) ) {
			u8	wfd_devinfo[ 6 ] = { 0x00 };
			uint	wfd_devlen = 6;

			DBG_871X( "[%s] Found WFD IE!\n", __FUNCTION__ );
			if ( rtw_get_wfd_attr_content( wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, wfd_devinfo, &wfd_devlen ) ) {
				u16	wfd_devinfo_field = 0;

				//	Commented by Albert 20120319
				//	The first two bytes are the WFD device information field of WFD device information subelement.
				//	In big endian format.
				wfd_devinfo_field = RTW_GET_BE16(wfd_devinfo);
				if ( wfd_devinfo_field & WFD_DEVINFO_SESSION_AVAIL ) {
					pwfd_info->peer_session_avail = _TRUE;
				} else {
					pwfd_info->peer_session_avail = _FALSE;
				}
			}
		}

		if ( _FALSE == pwfd_info->peer_session_avail ) {
			DBG_871X( "[%s] WFD Session not avaiable!\n", __FUNCTION__ );
			goto exit;
		}
	}
#endif // CONFIG_WFD

	if ( uintPeerChannel ) {
#ifdef CONFIG_CONCURRENT_MODE
		if ( check_fwstate( pbuddy_mlmepriv, _FW_LINKED ) ) {
			_cancel_timer_ex( &pwdinfo->ap_p2p_switch_timer );
		}
#endif // CONFIG_CONCURRENT_MODE

		//	Store the GO's bssid
		for( jj = 0, kk = 18; jj < ETH_ALEN; jj++, kk += 3 ) {
			pinvite_req_info->go_bssid[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
		}

		//	Store the GO's ssid
		pinvite_req_info->ssidlen = wrqu->data.length - 36;
		_rtw_memcpy( pinvite_req_info->go_ssid, &extra[ 36 ], (u32) pinvite_req_info->ssidlen );
		pinvite_req_info->benable = _TRUE;
		pinvite_req_info->peer_ch = uintPeerChannel;

		rtw_p2p_set_pre_state(pwdinfo, rtw_p2p_state(pwdinfo));
		rtw_p2p_set_state(pwdinfo, P2P_STATE_TX_INVITE_REQ);

#ifdef CONFIG_CONCURRENT_MODE
		if ( check_fwstate( pbuddy_mlmepriv, _FW_LINKED ) ) {
			//	Have to enter the power saving with the AP
			set_channel_bwmode(padapter, pbuddy_mlmeext->cur_channel, pbuddy_mlmeext->cur_ch_offset, pbuddy_mlmeext->cur_bwmode);

			issue_nulldata(pbuddy_adapter, NULL, 1, 3, 500);
		} else {
			set_channel_bwmode(padapter, uintPeerChannel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, CHANNEL_WIDTH_20);
		}
#else
		set_channel_bwmode(padapter, uintPeerChannel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, CHANNEL_WIDTH_20);
#endif

		_set_timer( &pwdinfo->pre_tx_scan_timer, P2P_TX_PRESCAN_TIMEOUT );

#ifdef CONFIG_CONCURRENT_MODE
		if ( check_fwstate( pbuddy_mlmepriv, _FW_LINKED ) ) {
			_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_CONCURRENT_INVITE_TIMEOUT );
		} else {
			_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_INVITE_TIMEOUT );
		}
#else
		_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_INVITE_TIMEOUT );
#endif // CONFIG_CONCURRENT_MODE


	} else {
		DBG_871X( "[%s] NOT Found in the Scanning Queue!\n", __FUNCTION__ );
	}
exit:

	return ret;

}

static int rtw_p2p_set_persistent(struct net_device *dev,
                                  struct iw_request_info *info,
                                  union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter 					*padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point 			*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	//int 						jj,kk;
	//u8   						peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	//struct mlme_priv			*pmlmepriv = &padapter->mlmepriv;
	//_list						*plist, *phead;
	//_queue					*queue	= &(pmlmepriv->scanned_queue);
	//struct	wlan_network		*pnetwork = NULL;
	//uint						uintPeerChannel = 0;
	//u8						attr_content[50] = { 0x00 }, _status = 0;
	//u8 						*p2pie;
	//uint						p2pielen = 0, attr_contentlen = 0;
	//_irqL					irqL;
	//struct tx_invite_req_info*	pinvite_req_info = &pwdinfo->invitereq_info;
#ifdef CONFIG_CONCURRENT_MODE
	//_adapter					*pbuddy_adapter = padapter->pbuddy_adapter;
	//struct mlme_priv			*pbuddy_mlmepriv = &pbuddy_adapter->mlmepriv;
	//struct mlme_ext_priv		*pbuddy_mlmeext = &pbuddy_adapter->mlmeextpriv;
#endif // CONFIG_CONCURRENT_MODE

#ifdef CONFIG_WFD
	//struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;
#endif // CONFIG_WFD

	//	Commented by Albert 20120328
	//	The input data is 0 or 1
	//	0: disable persistent group functionality
	//	1: enable persistent group founctionality

	DBG_871X( "[%s] data = %s\n", __FUNCTION__, extra );

	if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)) {
		DBG_871X( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	} else {
		if ( extra[ 0 ] == '0' ) {	//	Disable the persistent group function.
			pwdinfo->persistent_supported = _FALSE;
		} else if ( extra[ 0 ] == '1' ) {	//	Enable the persistent group function.
			pwdinfo->persistent_supported = _TRUE;
		} else {
			pwdinfo->persistent_supported = _FALSE;
		}
	}
	printk( "[%s] persistent_supported = %d\n", __FUNCTION__, pwdinfo->persistent_supported );

//exit:

	return ret;

}

static int hexstr2bin(const char *hex, u8 *buf, size_t len)
{
	size_t i;
	int a;
	const char *ipos = hex;
	u8 *opos = buf;

	for (i = 0; i < len; i++) {
		a = hex2byte_i(ipos);
		if (a < 0)
			return -1;
		*opos++ = a;
		ipos += 2;
	}
	return 0;
}

static int uuid_str2bin(const char *str, u8 *bin)
{
	const char *pos;
	u8 *opos;

	pos = str;
	opos = bin;

	if (hexstr2bin(pos, opos, 4))
		return -1;
	pos += 8;
	opos += 4;

	if (*pos++ != '-' || hexstr2bin(pos, opos, 2))
		return -1;
	pos += 4;
	opos += 2;

	if (*pos++ != '-' || hexstr2bin(pos, opos, 2))
		return -1;
	pos += 4;
	opos += 2;

	if (*pos++ != '-' || hexstr2bin(pos, opos, 2))
		return -1;
	pos += 4;
	opos += 2;

	if (*pos++ != '-' || hexstr2bin(pos, opos, 6))
		return -1;

	return 0;
}

static int rtw_p2p_set_wps_uuid(struct net_device *dev,
                                struct iw_request_info *info,
                                union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter				*padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info			*pwdinfo = &(padapter->wdinfo);

	DBG_871X("[%s] data = %s\n", __FUNCTION__, extra);

	if ((36 == strlen(extra)) && (uuid_str2bin(extra, pwdinfo->uuid) == 0)) {
		pwdinfo->external_uuid = 1;
	} else {
		pwdinfo->external_uuid = 0;
		ret = -EINVAL;
	}

	return ret;

}
#ifdef CONFIG_WFD
static int rtw_p2p_set_pc(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter 				*padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point 		*pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int 					jj,kk;
	//u8   					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	_list					*plist, *phead;
	_queue				*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	u8					attr_content[50] = { 0x00 };
	u8 *p2pie;
	uint					p2pielen = 0, attr_contentlen = 0;
	_irqL				irqL;
	uint					uintPeerChannel = 0;
#ifdef CONFIG_CONCURRENT_MODE
	//_adapter				*pbuddy_adapter = padapter->pbuddy_adapter;
	//struct mlme_ext_priv	*pbuddy_mlmeext = &pbuddy_adapter->mlmeextpriv;
#endif // CONFIG_CONCURRENT_MODE

	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;

	//	Commented by Albert 20120512
	//	1. Input information is the MAC address which wants to know the Preferred Connection bit (PC bit)
	//	Format: 00:E0:4C:00:00:05

	DBG_871X( "[%s] data = %s\n", __FUNCTION__, extra );

	if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)) {
		DBG_871X( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	}

	for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 ) {
		peerMAC[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while(1) {
		if (rtw_end_of_queue_search(phead,plist)== _TRUE)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		//	Commented by Albert 2011/05/18
		//	Match the device address located in the P2P IE
		//	This is for the case that the P2P device address is not the same as the P2P interface address.

		if ( (p2pie=rtw_get_p2p_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &p2pielen, pnetwork->network.Reserved[0]))) {
			//	The P2P Device ID attribute is included in the Beacon frame.
			//	The P2P Device Info attribute is included in the probe response frame.
			printk( "[%s] Got P2P IE\n", __FUNCTION__ );
			if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen) ) {
				//	Handle the P2P Device ID attribute of Beacon first
				printk( "[%s] P2P_ATTR_DEVICE_ID \n", __FUNCTION__ );
				if ( _rtw_memcmp( attr_content, peerMAC, ETH_ALEN ) ) {
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			} else if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen) ) {
				//	Handle the P2P Device Info attribute of probe response
				printk( "[%s] P2P_ATTR_DEVICE_INFO \n", __FUNCTION__ );
				if ( _rtw_memcmp( attr_content, peerMAC, ETH_ALEN ) ) {
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			}

		}

		plist = get_next(plist);

	}

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
	printk( "[%s] channel = %d\n", __FUNCTION__, uintPeerChannel );

	if ( uintPeerChannel ) {
		u8	wfd_ie[ 128 ] = { 0x00 };
		uint	wfd_ielen = 0;
		//u8 ie_offset = (pnetwork->network.Reserved[0] == 2 ? 0:12);

		if ( rtw_get_wfd_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength,  wfd_ie, &wfd_ielen, pnetwork->network.Reserved[0]) ) {
			u8	wfd_devinfo[ 6 ] = { 0x00 };
			uint	wfd_devlen = 6;

			DBG_871X( "[%s] Found WFD IE!\n", __FUNCTION__ );
			if ( rtw_get_wfd_attr_content( wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, wfd_devinfo, &wfd_devlen ) ) {
				u16	wfd_devinfo_field = 0;

				//	Commented by Albert 20120319
				//	The first two bytes are the WFD device information field of WFD device information subelement.
				//	In big endian format.
				wfd_devinfo_field = RTW_GET_BE16(wfd_devinfo);
				if ( wfd_devinfo_field & WFD_DEVINFO_PC_TDLS ) {
					pwfd_info->wfd_pc = _TRUE;
				} else {
					pwfd_info->wfd_pc = _FALSE;
				}
			}
		}
	} else {
		DBG_871X( "[%s] NOT Found in the Scanning Queue!\n", __FUNCTION__ );
	}

//exit:

	return ret;

}

static int rtw_p2p_set_wfd_device_type(struct net_device *dev,
                                       struct iw_request_info *info,
                                       union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter 					*padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point 			*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	struct wifi_display_info		*pwfd_info = pwdinfo->wfd_info;

	//	Commented by Albert 20120328
	//	The input data is 0 or 1
	//	0: specify to Miracast source device
	//	1 or others: specify to Miracast sink device (display device)

	DBG_871X( "[%s] data = %s\n", __FUNCTION__, extra );

	if ( extra[ 0 ] == '0' ) {	//	Set to Miracast source device.
		pwfd_info->wfd_device_type = WFD_DEVINFO_SOURCE;
	} else {				//	Set to Miracast sink device.
		pwfd_info->wfd_device_type = WFD_DEVINFO_PSINK;
	}

//exit:

	return ret;

}

static int rtw_p2p_set_wfd_enable(struct net_device *dev,
                                  struct iw_request_info *info,
                                  union iwreq_data *wrqu, char *extra)
{
//	Commented by Kurt 20121206
//	This function is used to set wfd enabled

	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	if(*extra == '0' )
		pwdinfo->wfd_info->wfd_enable = _FALSE;
	else if(*extra == '1')
		pwdinfo->wfd_info->wfd_enable = _TRUE;

	DBG_871X( "[%s] wfd_enable = %d\n", __FUNCTION__, pwdinfo->wfd_info->wfd_enable );

	return ret;

}

static int rtw_p2p_set_driver_iface(struct net_device *dev,
                                    struct iw_request_info *info,
                                    union iwreq_data *wrqu, char *extra)
{
//	Commented by Kurt 20121206
//	This function is used to set driver iface is WEXT or CFG80211
	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	if(*extra == '1' ) {
		pwdinfo->driver_interface = DRIVER_WEXT;
		DBG_871X( "[%s] driver_interface = WEXT\n", __FUNCTION__);
	} else if(*extra == '2') {
		pwdinfo->driver_interface = DRIVER_CFG80211;
		DBG_871X( "[%s] driver_interface = CFG80211\n", __FUNCTION__);
	}

	return ret;

}

//	To set the WFD session available to enable or disable
static int rtw_p2p_set_sa(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter 					*padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct iw_point 			*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	//struct wifi_display_info		*pwfd_info = pwdinfo->wfd_info;

	DBG_871X( "[%s] data = %s\n", __FUNCTION__, extra );

	if( 0 ) {
		DBG_871X( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	} else {
		if ( extra[ 0 ] == '0' ) {	//	Disable the session available.
			pwdinfo->session_available = _FALSE;
		} else if ( extra[ 0 ] == '1' ) {	//	Enable the session available.
			pwdinfo->session_available = _TRUE;
		} else {
			pwdinfo->session_available = _FALSE;
		}
	}
	printk( "[%s] session available = %d\n", __FUNCTION__, pwdinfo->session_available );

//exit:

	return ret;

}
#endif // CONFIG_WFD

static int rtw_p2p_prov_disc(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	_adapter 				*padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int 					jj,kk;
	//u8   					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	_list					*plist, *phead;
	_queue				*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	uint					uintPeerChannel = 0;
	u8					attr_content[100] = { 0x00 };
	u8 *p2pie;
	uint					p2pielen = 0, attr_contentlen = 0;
	_irqL				irqL;
	u8 					ie_offset = 12;
#ifdef CONFIG_CONCURRENT_MODE
	_adapter				*pbuddy_adapter = padapter->pbuddy_adapter;
	struct mlme_priv		*pbuddy_mlmepriv = &pbuddy_adapter->mlmepriv;
	struct mlme_ext_priv	*pbuddy_mlmeext = &pbuddy_adapter->mlmeextpriv;
#endif // CONFIG_CONCURRENT_MODE
#ifdef CONFIG_WFD
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;
#endif // CONFIG_WFD

	//	Commented by Albert 20110301
	//	The input data contains two informations.
	//	1. First information is the MAC address which wants to issue the provisioning discovery request frame.
	//	2. Second information is the WPS configuration method which wants to discovery
	//	Format: 00:E0:4C:00:00:05_display
	//	Format: 00:E0:4C:00:00:05_keypad
	//	Format: 00:E0:4C:00:00:05_pbc
	//	Format: 00:E0:4C:00:00:05_label

	DBG_871X( "[%s] data = %s\n", __FUNCTION__, extra );

	if ( pwdinfo->p2p_state == P2P_STATE_NONE ) {
		DBG_871X( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	} else {
#ifdef CONFIG_INTEL_WIDI
		if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) == _TRUE) {
			DBG_871X( "[%s] WiFi is under survey!\n", __FUNCTION__ );
			return ret;
		}
#endif //CONFIG_INTEL_WIDI

		//	Reset the content of struct tx_provdisc_req_info excluded the wps_config_method_request.
		_rtw_memset( pwdinfo->tx_prov_disc_info.peerDevAddr, 0x00, ETH_ALEN );
		_rtw_memset( pwdinfo->tx_prov_disc_info.peerIFAddr, 0x00, ETH_ALEN );
		_rtw_memset( &pwdinfo->tx_prov_disc_info.ssid, 0x00, sizeof( NDIS_802_11_SSID ) );
		pwdinfo->tx_prov_disc_info.peer_channel_num[ 0 ] = 0;
		pwdinfo->tx_prov_disc_info.peer_channel_num[ 1 ] = 0;
		pwdinfo->tx_prov_disc_info.benable = _FALSE;
	}

	for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 ) {
		peerMAC[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	if ( _rtw_memcmp( &extra[ 18 ], "display", 7 ) ) {
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_DISPLYA;
	} else if ( _rtw_memcmp( &extra[ 18 ], "keypad", 7 ) ) {
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_KEYPAD;
	} else if ( _rtw_memcmp( &extra[ 18 ], "pbc", 3 ) ) {
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_PUSH_BUTTON;
	} else if ( _rtw_memcmp( &extra[ 18 ], "label", 5 ) ) {
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_LABEL;
	} else {
		DBG_871X( "[%s] Unknown WPS config methodn", __FUNCTION__ );
		return( ret );
	}

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while(1) {
		if (rtw_end_of_queue_search(phead,plist)== _TRUE)
			break;

		if( uintPeerChannel != 0 )
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		//	Commented by Albert 2011/05/18
		//	Match the device address located in the P2P IE
		//	This is for the case that the P2P device address is not the same as the P2P interface address.

		ie_offset = (pnetwork->network.Reserved[0] == 2? 0:12);
		if ( (p2pie=rtw_get_p2p_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &p2pielen, pnetwork->network.Reserved[0]))) {
			while ( p2pie ) {
				//	The P2P Device ID attribute is included in the Beacon frame.
				//	The P2P Device Info attribute is included in the probe response frame.

				if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen) ) {
					//	Handle the P2P Device ID attribute of Beacon first
					if ( _rtw_memcmp( attr_content, peerMAC, ETH_ALEN ) ) {
						uintPeerChannel = pnetwork->network.Configuration.DSConfig;
						break;
					}
				} else if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen) ) {
					//	Handle the P2P Device Info attribute of probe response
					if ( _rtw_memcmp( attr_content, peerMAC, ETH_ALEN ) ) {
						uintPeerChannel = pnetwork->network.Configuration.DSConfig;
						break;
					}
				}

				//Get the next P2P IE
				p2pie = rtw_get_p2p_ie(p2pie+p2pielen, pnetwork->network.IELength - ie_offset -(p2pie -&pnetwork->network.IEs[ie_offset] + p2pielen), NULL, &p2pielen);
			}

		}

#ifdef CONFIG_INTEL_WIDI
		// Some Intel WiDi source may not provide P2P IE,
		// so we could only compare mac addr by 802.11 Source Address
		if( pmlmepriv->widi_state == INTEL_WIDI_STATE_WFD_CONNECTION
		    && uintPeerChannel == 0 ) {
			if ( _rtw_memcmp( pnetwork->network.MacAddress, peerMAC, ETH_ALEN ) ) {
				uintPeerChannel = pnetwork->network.Configuration.DSConfig;
				break;
			}
		}
#endif //CONFIG_INTEL_WIDI

		plist = get_next(plist);

	}

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	if ( uintPeerChannel ) {
#ifdef CONFIG_WFD
		{
			u8	wfd_ie[ 128 ] = { 0x00 };
			uint	wfd_ielen = 0;

			if ( rtw_get_wfd_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength,  wfd_ie, &wfd_ielen, pnetwork->network.Reserved[0]) )
			{
				u8	wfd_devinfo[ 6 ] = { 0x00 };
				uint	wfd_devlen = 6;

				DBG_871X( "[%s] Found WFD IE!\n", __FUNCTION__ );
				if ( rtw_get_wfd_attr_content( wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, wfd_devinfo, &wfd_devlen ) ) {
					u16	wfd_devinfo_field = 0;

					//	Commented by Albert 20120319
					//	The first two bytes are the WFD device information field of WFD device information subelement.
					//	In big endian format.
					wfd_devinfo_field = RTW_GET_BE16(wfd_devinfo);
					if ( wfd_devinfo_field & WFD_DEVINFO_SESSION_AVAIL ) {
						pwfd_info->peer_session_avail = _TRUE;
					} else {
						pwfd_info->peer_session_avail = _FALSE;
					}
				}
			}

			if ( _FALSE == pwfd_info->peer_session_avail )
			{
				DBG_871X( "[%s] WFD Session not avaiable!\n", __FUNCTION__ );
				goto exit;
			}
		}
#endif // CONFIG_WFD

		DBG_871X( "[%s] peer channel: %d!\n", __FUNCTION__, uintPeerChannel );
#ifdef CONFIG_CONCURRENT_MODE
		if ( check_fwstate( pbuddy_mlmepriv, _FW_LINKED ) ) {
			_cancel_timer_ex( &pwdinfo->ap_p2p_switch_timer );
		}
#endif // CONFIG_CONCURRENT_MODE
		_rtw_memcpy( pwdinfo->tx_prov_disc_info.peerIFAddr, pnetwork->network.MacAddress, ETH_ALEN );
		_rtw_memcpy( pwdinfo->tx_prov_disc_info.peerDevAddr, peerMAC, ETH_ALEN );
		pwdinfo->tx_prov_disc_info.peer_channel_num[0] = ( u16 ) uintPeerChannel;
		pwdinfo->tx_prov_disc_info.benable = _TRUE;
		rtw_p2p_set_pre_state(pwdinfo, rtw_p2p_state(pwdinfo));
		rtw_p2p_set_state(pwdinfo, P2P_STATE_TX_PROVISION_DIS_REQ);

		if(rtw_p2p_chk_role(pwdinfo, P2P_ROLE_CLIENT)) {
			_rtw_memcpy( &pwdinfo->tx_prov_disc_info.ssid, &pnetwork->network.Ssid, sizeof( NDIS_802_11_SSID ) );
		} else if(rtw_p2p_chk_role(pwdinfo, P2P_ROLE_DEVICE) || rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO)) {
			_rtw_memcpy( pwdinfo->tx_prov_disc_info.ssid.Ssid, pwdinfo->p2p_wildcard_ssid, P2P_WILDCARD_SSID_LEN );
			pwdinfo->tx_prov_disc_info.ssid.SsidLength= P2P_WILDCARD_SSID_LEN;
		}

#ifdef CONFIG_CONCURRENT_MODE
		if ( check_fwstate( pbuddy_mlmepriv, _FW_LINKED ) ) {
			//	Have to enter the power saving with the AP
			set_channel_bwmode(padapter, pbuddy_mlmeext->cur_channel, pbuddy_mlmeext->cur_ch_offset, pbuddy_mlmeext->cur_bwmode);

			issue_nulldata(pbuddy_adapter, NULL, 1, 3, 500);
		} else {
			set_channel_bwmode(padapter, uintPeerChannel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, CHANNEL_WIDTH_20);
		}
#else
		set_channel_bwmode(padapter, uintPeerChannel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, CHANNEL_WIDTH_20);
#endif

		_set_timer( &pwdinfo->pre_tx_scan_timer, P2P_TX_PRESCAN_TIMEOUT );

#ifdef CONFIG_CONCURRENT_MODE
		if ( check_fwstate( pbuddy_mlmepriv, _FW_LINKED ) ) {
			_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_CONCURRENT_PROVISION_TIMEOUT );
		} else {
			_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_PROVISION_TIMEOUT );
		}
#else
		_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_PROVISION_TIMEOUT );
#endif // CONFIG_CONCURRENT_MODE

	} else {
		DBG_871X( "[%s] NOT Found in the Scanning Queue!\n", __FUNCTION__ );
#ifdef CONFIG_INTEL_WIDI
		_cancel_timer_ex( &pwdinfo->restore_p2p_state_timer );
		rtw_p2p_set_state(pwdinfo, P2P_STATE_FIND_PHASE_SEARCH);
		rtw_p2p_findphase_ex_set(pwdinfo, P2P_FINDPHASE_EX_NONE);
		rtw_free_network_queue(padapter, _TRUE);
		_enter_critical_bh(&pmlmepriv->lock, &irqL);
		rtw_sitesurvey_cmd(padapter, NULL, 0, NULL, 0);
		_exit_critical_bh(&pmlmepriv->lock, &irqL);
#endif //CONFIG_INTEL_WIDI
	}
exit:

	return ret;

}

//	Added by Albert 20110328
//	This function is used to inform the driver the user had specified the pin code value or pbc
//	to application.

static int rtw_p2p_got_wpsinfo(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	_adapter 				*padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_871X( "[%s] data = %s\n", __FUNCTION__, extra );
	//	Added by Albert 20110328
	//	if the input data is P2P_NO_WPSINFO -> reset the wpsinfo
	//	if the input data is P2P_GOT_WPSINFO_PEER_DISPLAY_PIN -> the utility just input the PIN code got from the peer P2P device.
	//	if the input data is P2P_GOT_WPSINFO_SELF_DISPLAY_PIN -> the utility just got the PIN code from itself.
	//	if the input data is P2P_GOT_WPSINFO_PBC -> the utility just determine to use the PBC

	if ( *extra == '0' ) {
		pwdinfo->ui_got_wps_info = P2P_NO_WPSINFO;
	} else if ( *extra == '1' ) {
		pwdinfo->ui_got_wps_info = P2P_GOT_WPSINFO_PEER_DISPLAY_PIN;
	} else if ( *extra == '2' ) {
		pwdinfo->ui_got_wps_info = P2P_GOT_WPSINFO_SELF_DISPLAY_PIN;
	} else if ( *extra == '3' ) {
		pwdinfo->ui_got_wps_info = P2P_GOT_WPSINFO_PBC;
	} else {
		pwdinfo->ui_got_wps_info = P2P_NO_WPSINFO;
	}

	return ret;

}

#endif //CONFIG_P2P

static int rtw_p2p_set(struct net_device *dev,
                       struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
#ifdef CONFIG_P2P

	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	//struct iw_point *pdata = &wrqu->data;
	//struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	//struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	DBG_871X( "[%s] extra = %s\n", __FUNCTION__, extra );

	if ( _rtw_memcmp( extra, "enable=", 7 ) ) {
		rtw_wext_p2p_enable( dev, info, wrqu, &extra[7] );
	} else if ( _rtw_memcmp( extra, "setDN=", 6 ) ) {
		wrqu->data.length -= 6;
		rtw_p2p_setDN( dev, info, wrqu, &extra[6] );
	} else if ( _rtw_memcmp( extra, "profilefound=", 13 ) ) {
		wrqu->data.length -= 13;
		rtw_p2p_profilefound( dev, info, wrqu, &extra[13] );
	} else if ( _rtw_memcmp( extra, "prov_disc=", 10 ) ) {
		wrqu->data.length -= 10;
		rtw_p2p_prov_disc( dev, info, wrqu, &extra[10] );
	} else if ( _rtw_memcmp( extra, "nego=", 5 ) ) {
		wrqu->data.length -= 5;
		rtw_p2p_connect( dev, info, wrqu, &extra[5] );
	} else if ( _rtw_memcmp( extra, "intent=", 7 ) ) {
		//	Commented by Albert 2011/03/23
		//	The wrqu->data.length will include the null character
		//	So, we will decrease 7 + 1
		wrqu->data.length -= 8;
		rtw_p2p_set_intent( dev, info, wrqu, &extra[7] );
	} else if ( _rtw_memcmp( extra, "ssid=", 5 ) ) {
		wrqu->data.length -= 5;
		rtw_p2p_set_go_nego_ssid( dev, info, wrqu, &extra[5] );
	} else if ( _rtw_memcmp( extra, "got_wpsinfo=", 12 ) ) {
		wrqu->data.length -= 12;
		rtw_p2p_got_wpsinfo( dev, info, wrqu, &extra[12] );
	} else if ( _rtw_memcmp( extra, "listen_ch=", 10 ) ) {
		//	Commented by Albert 2011/05/24
		//	The wrqu->data.length will include the null character
		//	So, we will decrease (10 + 1)
		wrqu->data.length -= 11;
		rtw_p2p_set_listen_ch( dev, info, wrqu, &extra[10] );
	} else if ( _rtw_memcmp( extra, "op_ch=", 6 ) ) {
		//	Commented by Albert 2011/05/24
		//	The wrqu->data.length will include the null character
		//	So, we will decrease (6 + 1)
		wrqu->data.length -= 7;
		rtw_p2p_set_op_ch( dev, info, wrqu, &extra[6] );
	} else if ( _rtw_memcmp( extra, "invite=", 7 ) ) {
		wrqu->data.length -= 8;
		rtw_p2p_invite_req( dev, info, wrqu, &extra[7] );
	} else if ( _rtw_memcmp( extra, "persistent=", 11 ) ) {
		wrqu->data.length -= 11;
		rtw_p2p_set_persistent( dev, info, wrqu, &extra[11] );
	} else if ( _rtw_memcmp ( extra, "uuid=", 5) ) {
		wrqu->data.length -= 5;
		ret = rtw_p2p_set_wps_uuid( dev, info, wrqu, &extra[5] );
	}
#ifdef CONFIG_WFD
	else if ( _rtw_memcmp( extra, "sa=", 3 ) ) {
		//	sa: WFD Session Available information
		wrqu->data.length -= 3;
		rtw_p2p_set_sa( dev, info, wrqu, &extra[3] );
	} else if ( _rtw_memcmp( extra, "pc=", 3 ) ) {
		//	pc: WFD Preferred Connection
		wrqu->data.length -= 3;
		rtw_p2p_set_pc( dev, info, wrqu, &extra[3] );
	} else if ( _rtw_memcmp( extra, "wfd_type=", 9 ) ) {
		//	pc: WFD Preferred Connection
		wrqu->data.length -= 9;
		rtw_p2p_set_wfd_device_type( dev, info, wrqu, &extra[9] );
	} else if ( _rtw_memcmp( extra, "wfd_enable=", 11 ) ) {
		wrqu->data.length -= 11;
		rtw_p2p_set_wfd_enable( dev, info, wrqu, &extra[11] );
	} else if ( _rtw_memcmp( extra, "driver_iface=", 13 ) ) {
		wrqu->data.length -= 13;
		rtw_p2p_set_driver_iface( dev, info, wrqu, &extra[13] );
	}
#endif //CONFIG_WFD

#endif //CONFIG_P2P

	return ret;

}

static int rtw_p2p_get(struct net_device *dev,
                       struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{

	int ret = 0;

#ifdef CONFIG_P2P

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	//struct iw_point *pdata = &wrqu->data;
	//struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	//struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	if ( padapter->bShowGetP2PState ) {
		DBG_871X( "[%s] extra = %s\n", __FUNCTION__, (char*) wrqu->data.pointer );
	}

	if ( _rtw_memcmp( wrqu->data.pointer, "status", 6 ) ) {
		rtw_p2p_get_status( dev, info, wrqu, extra );
	} else if ( _rtw_memcmp( wrqu->data.pointer, "role", 4 ) ) {
		rtw_p2p_get_role( dev, info, wrqu, extra);
	} else if ( _rtw_memcmp( wrqu->data.pointer, "peer_ifa", 8 ) ) {
		rtw_p2p_get_peer_ifaddr( dev, info, wrqu, extra);
	} else if ( _rtw_memcmp( wrqu->data.pointer, "req_cm", 6 ) ) {
		rtw_p2p_get_req_cm( dev, info, wrqu, extra);
	} else if ( _rtw_memcmp( wrqu->data.pointer, "peer_deva", 9 ) ) {
		//	Get the P2P device address when receiving the provision discovery request frame.
		rtw_p2p_get_peer_devaddr( dev, info, wrqu, extra);
	} else if ( _rtw_memcmp( wrqu->data.pointer, "group_id", 8 ) ) {
		rtw_p2p_get_groupid( dev, info, wrqu, extra);
	} else if ( _rtw_memcmp( wrqu->data.pointer, "inv_peer_deva", 13 ) ) {
		//	Get the P2P device address when receiving the P2P Invitation request frame.
		rtw_p2p_get_peer_devaddr_by_invitation( dev, info, wrqu, extra);
	} else if ( _rtw_memcmp( wrqu->data.pointer, "op_ch", 5 ) ) {
		rtw_p2p_get_op_ch( dev, info, wrqu, extra);
	}
#ifdef CONFIG_WFD
	else if ( _rtw_memcmp( wrqu->data.pointer, "peer_port", 9 ) ) {
		rtw_p2p_get_peer_wfd_port( dev, info, wrqu, extra );
	} else if ( _rtw_memcmp( wrqu->data.pointer, "wfd_sa", 6 ) ) {
		rtw_p2p_get_peer_wfd_session_available( dev, info, wrqu, extra );
	} else if ( _rtw_memcmp( wrqu->data.pointer, "wfd_pc", 6 ) ) {
		rtw_p2p_get_peer_wfd_preferred_connection( dev, info, wrqu, extra );
	}
#endif // CONFIG_WFD

#endif //CONFIG_P2P

	return ret;

}

static int rtw_p2p_get2(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{

	int ret = 0;

#ifdef CONFIG_P2P

	int length = wrqu->data.length;
	char *buffer = (u8 *)rtw_malloc(length);

	if (buffer == NULL) {
		ret = -ENOMEM;
		goto bad;
	}

	if (copy_from_user(buffer, wrqu->data.pointer, wrqu->data.length)) {
		ret = -EFAULT;
		goto bad;
	}

	DBG_871X("[%s] buffer = %s\n", __FUNCTION__, buffer);

	if (_rtw_memcmp(buffer, "wpsCM=", 6)) {
		ret = rtw_p2p_get_wps_configmethod(dev, info, wrqu, extra, &buffer[6]);
	} else if (_rtw_memcmp(buffer, "devN=", 5)) {
		ret = rtw_p2p_get_device_name(dev, info, wrqu, extra, &buffer[5]);
	} else if (_rtw_memcmp(buffer, "dev_type=", 9)) {
		ret = rtw_p2p_get_device_type(dev, info, wrqu, extra, &buffer[9]);
	} else if (_rtw_memcmp(buffer, "go_devadd=", 10)) {
		ret = rtw_p2p_get_go_device_address(dev, info, wrqu, extra, &buffer[10]);
	} else if (_rtw_memcmp(buffer, "InvProc=", 8)) {
		ret = rtw_p2p_get_invitation_procedure(dev, info, wrqu, extra, &buffer[8]);
	} else {
		snprintf(extra, sizeof("Command not found."), "Command not found.");
		wrqu->data.length = strlen(extra);
	}

bad:
	if (buffer) {
		rtw_mfree(buffer, length);
	}

#endif //CONFIG_P2P

	return ret;

}

static int rtw_cta_test_start(struct net_device *dev,
                              struct iw_request_info *info,
                              union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	_adapter	*padapter = (_adapter *)rtw_netdev_priv(dev);
	DBG_871X("%s %s\n", __func__, extra);
	if (!strcmp(extra, "1"))
		padapter->in_cta_test = 1;
	else
		padapter->in_cta_test = 0;

	if(padapter->in_cta_test) {
		u32 v = rtw_read32(padapter, REG_RCR);
		v &= ~(RCR_CBSSID_DATA | RCR_CBSSID_BCN );//| RCR_ADF
		rtw_write32(padapter, REG_RCR, v);
		DBG_871X("enable RCR_ADF\n");
	} else {
		u32 v = rtw_read32(padapter, REG_RCR);
		v |= RCR_CBSSID_DATA | RCR_CBSSID_BCN ;//| RCR_ADF
		rtw_write32(padapter, REG_RCR, v);
		DBG_871X("disable RCR_ADF\n");
	}
	return ret;
}


extern int rtw_change_ifname(_adapter *padapter, const char *ifname);
static int rtw_rereg_nd_name(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	_adapter *padapter = rtw_netdev_priv(dev);
	struct rereg_nd_name_data *rereg_priv = &padapter->rereg_nd_name_priv;
	char new_ifname[IFNAMSIZ];

	if(rereg_priv->old_ifname[0] == 0) {
		char *reg_ifname;
#ifdef CONFIG_CONCURRENT_MODE
		if (padapter->isprimary)
			reg_ifname = padapter->registrypriv.ifname;
		else
#endif
			reg_ifname = padapter->registrypriv.if2name;

		strncpy(rereg_priv->old_ifname, reg_ifname, IFNAMSIZ);
		rereg_priv->old_ifname[IFNAMSIZ-1] = 0;
	}

	//DBG_871X("%s wrqu->data.length:%d\n", __FUNCTION__, wrqu->data.length);
	if(wrqu->data.length > IFNAMSIZ)
		return -EFAULT;

	if ( copy_from_user(new_ifname, wrqu->data.pointer, IFNAMSIZ) ) {
		return -EFAULT;
	}

	if( 0 == strcmp(rereg_priv->old_ifname, new_ifname) ) {
		return ret;
	}

	DBG_871X("%s new_ifname:%s\n", __FUNCTION__, new_ifname);
	if( 0 != (ret = rtw_change_ifname(padapter, new_ifname)) ) {
		goto exit;
	}

	if(_rtw_memcmp(rereg_priv->old_ifname, "disable%d", 9) == _TRUE) {
		padapter->ledpriv.bRegUseLed= rereg_priv->old_bRegUseLed;
		rtw_hal_sw_led_init(padapter);
		//rtw_ips_mode_req(&padapter->pwrctrlpriv, rereg_priv->old_ips_mode);
	}

	strncpy(rereg_priv->old_ifname, new_ifname, IFNAMSIZ);
	rereg_priv->old_ifname[IFNAMSIZ-1] = 0;

	if(_rtw_memcmp(new_ifname, "disable%d", 9) == _TRUE) {

		DBG_871X("%s disable\n", __FUNCTION__);
		// free network queue for Android's timming issue
		rtw_free_network_queue(padapter, _TRUE);

		// close led
		rtw_led_control(padapter, LED_CTL_POWER_OFF);
		rereg_priv->old_bRegUseLed = padapter->ledpriv.bRegUseLed;
		padapter->ledpriv.bRegUseLed= _FALSE;
		rtw_hal_sw_led_deinit(padapter);

		// the interface is being "disabled", we can do deeper IPS
		//rereg_priv->old_ips_mode = rtw_get_ips_mode_req(&padapter->pwrctrlpriv);
		//rtw_ips_mode_req(&padapter->pwrctrlpriv, IPS_NORMAL);
	}
exit:
	return ret;

}

#ifdef CONFIG_IOL
#include <rtw_iol.h>
#endif

#ifdef DBG_CMD_QUEUE
u8 dump_cmd_id=0;
#endif
static int rtw_dbg_port(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
	_irqL irqL;
	int ret = 0;
	u8 major_cmd, minor_cmd;
	u16 arg;
	u32 extra_arg, *pdata, val32;
	struct sta_info *psta;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	struct sta_priv *pstapriv = &padapter->stapriv;


	pdata = (u32*)&wrqu->data;

	val32 = *pdata;
	arg = (u16)(val32&0x0000ffff);
	major_cmd = (u8)(val32>>24);
	minor_cmd = (u8)((val32>>16)&0x00ff);

	extra_arg = *(pdata+1);

	switch(major_cmd) {
	case 0x70://read_reg
		switch(minor_cmd) {
		case 1:
			DBG_871X("rtw_read8(0x%x)=0x%02x\n", arg, rtw_read8(padapter, arg));
			break;
		case 2:
			DBG_871X("rtw_read16(0x%x)=0x%04x\n", arg, rtw_read16(padapter, arg));
			break;
		case 4:
			DBG_871X("rtw_read32(0x%x)=0x%08x\n", arg, rtw_read32(padapter, arg));
			break;
		}
		break;
	case 0x71://write_reg
		switch(minor_cmd) {
		case 1:
			rtw_write8(padapter, arg, extra_arg);
			DBG_871X("rtw_write8(0x%x)=0x%02x\n", arg, rtw_read8(padapter, arg));
			break;
		case 2:
			rtw_write16(padapter, arg, extra_arg);
			DBG_871X("rtw_write16(0x%x)=0x%04x\n", arg, rtw_read16(padapter, arg));
			break;
		case 4:
			rtw_write32(padapter, arg, extra_arg);
			DBG_871X("rtw_write32(0x%x)=0x%08x\n", arg, rtw_read32(padapter, arg));
			break;
		}
		break;
	case 0x72://read_bb
		DBG_871X("read_bbreg(0x%x)=0x%x\n", arg, rtw_hal_read_bbreg(padapter, arg, 0xffffffff));
		break;
	case 0x73://write_bb
		rtw_hal_write_bbreg(padapter, arg, 0xffffffff, extra_arg);
		DBG_871X("write_bbreg(0x%x)=0x%x\n", arg, rtw_hal_read_bbreg(padapter, arg, 0xffffffff));
		break;
	case 0x74://read_rf
		DBG_871X("read RF_reg path(0x%02x),offset(0x%x),value(0x%08x)\n",minor_cmd,arg,rtw_hal_read_rfreg(padapter, minor_cmd, arg, 0xffffffff));
		break;
	case 0x75://write_rf
		rtw_hal_write_rfreg(padapter, minor_cmd, arg, 0xffffffff, extra_arg);
		DBG_871X("write RF_reg path(0x%02x),offset(0x%x),value(0x%08x)\n",minor_cmd,arg, rtw_hal_read_rfreg(padapter, minor_cmd, arg, 0xffffffff));
		break;

	case 0x76:
		switch(minor_cmd) {
		case 0x00: //normal mode,
			padapter->recvpriv.is_signal_dbg = 0;
			break;
		case 0x01: //dbg mode
			padapter->recvpriv.is_signal_dbg = 1;
			extra_arg = extra_arg>100?100:extra_arg;
			padapter->recvpriv.signal_strength_dbg=extra_arg;
			break;
		}
		break;
	case 0x78: //IOL test
		switch(minor_cmd) {
#ifdef CONFIG_IOL
		case 0x04: { //LLT table initialization test
			u8 page_boundary = 0xf9;
			{
				struct xmit_frame	*xmit_frame;

				if((xmit_frame=rtw_IOL_accquire_xmit_frame(padapter)) == NULL) {
					ret = -ENOMEM;
					break;
				}

				rtw_IOL_append_LLT_cmd(xmit_frame, page_boundary);


				if(_SUCCESS != rtw_IOL_exec_cmds_sync(padapter, xmit_frame, 500,0) )
					ret = -EPERM;
			}
		}
		break;
		case 0x05: { //blink LED test
			u16 reg = 0x4c;
			u32 blink_num = 50;
			u32 blink_delay_ms = 200;
			int i;

			{
				struct xmit_frame	*xmit_frame;

				if((xmit_frame=rtw_IOL_accquire_xmit_frame(padapter)) == NULL) {
					ret = -ENOMEM;
					break;
				}

				for(i=0; i<blink_num; i++) {
#ifdef CONFIG_IOL_NEW_GENERATION
					rtw_IOL_append_WB_cmd(xmit_frame, reg, 0x00,0xff);
					rtw_IOL_append_DELAY_MS_cmd(xmit_frame, blink_delay_ms);
					rtw_IOL_append_WB_cmd(xmit_frame, reg, 0x08,0xff);
					rtw_IOL_append_DELAY_MS_cmd(xmit_frame, blink_delay_ms);
#else
					rtw_IOL_append_WB_cmd(xmit_frame, reg, 0x00);
					rtw_IOL_append_DELAY_MS_cmd(xmit_frame, blink_delay_ms);
					rtw_IOL_append_WB_cmd(xmit_frame, reg, 0x08);
					rtw_IOL_append_DELAY_MS_cmd(xmit_frame, blink_delay_ms);
#endif
				}
				if(_SUCCESS != rtw_IOL_exec_cmds_sync(padapter, xmit_frame, (blink_delay_ms*blink_num*2)+200,0) )
					ret = -EPERM;
			}
		}
		break;

		case 0x06: { //continuous wirte byte test
			u16 reg = arg;
			u16 start_value = 0;
			u32 write_num = extra_arg;
			int i;
			u8 final;

			{
				struct xmit_frame	*xmit_frame;

				if((xmit_frame=rtw_IOL_accquire_xmit_frame(padapter)) == NULL) {
					ret = -ENOMEM;
					break;
				}

				for(i=0; i<write_num; i++) {
#ifdef CONFIG_IOL_NEW_GENERATION
					rtw_IOL_append_WB_cmd(xmit_frame, reg, i+start_value,0xFF);
#else
					rtw_IOL_append_WB_cmd(xmit_frame, reg, i+start_value);
#endif
				}
				if(_SUCCESS != rtw_IOL_exec_cmds_sync(padapter, xmit_frame, 5000,0))
					ret = -EPERM;
			}

			if(start_value+write_num-1 == (final=rtw_read8(padapter, reg)) ) {
				DBG_871X("continuous IOL_CMD_WB_REG to 0x%x %u times Success, start:%u, final:%u\n", reg, write_num, start_value, final);
			} else {
				DBG_871X("continuous IOL_CMD_WB_REG to 0x%x %u times Fail, start:%u, final:%u\n", reg, write_num, start_value, final);
			}
		}
		break;

		case 0x07: { //continuous wirte word test
			u16 reg = arg;
			u16 start_value = 200;
			u32 write_num = extra_arg;

			int i;
			u16 final;

			{
				struct xmit_frame	*xmit_frame;

				if((xmit_frame=rtw_IOL_accquire_xmit_frame(padapter)) == NULL) {
					ret = -ENOMEM;
					break;
				}

				for(i=0; i<write_num; i++) {
#ifdef CONFIG_IOL_NEW_GENERATION
					rtw_IOL_append_WW_cmd(xmit_frame, reg, i+start_value,0xFFFF);
#else
					rtw_IOL_append_WW_cmd(xmit_frame, reg, i+start_value);
#endif
				}
				if(_SUCCESS !=rtw_IOL_exec_cmds_sync(padapter, xmit_frame, 5000,0))
					ret = -EPERM;
			}

			if(start_value+write_num-1 == (final=rtw_read16(padapter, reg)) ) {
				DBG_871X("continuous IOL_CMD_WW_REG to 0x%x %u times Success, start:%u, final:%u\n", reg, write_num, start_value, final);
			} else {
				DBG_871X("continuous IOL_CMD_WW_REG to 0x%x %u times Fail, start:%u, final:%u\n", reg, write_num, start_value, final);
			}
		}
		break;

		case 0x08: { //continuous wirte dword test
			u16 reg = arg;
			u32 start_value = 0x110000c7;
			u32 write_num = extra_arg;

			int i;
			u32 final;

			{
				struct xmit_frame	*xmit_frame;

				if((xmit_frame=rtw_IOL_accquire_xmit_frame(padapter)) == NULL) {
					ret = -ENOMEM;
					break;
				}

				for(i=0; i<write_num; i++) {
#ifdef CONFIG_IOL_NEW_GENERATION
					rtw_IOL_append_WD_cmd(xmit_frame, reg, i+start_value,0xFFFFFFFF);
#else
					rtw_IOL_append_WD_cmd(xmit_frame, reg, i+start_value);
#endif
				}
				if(_SUCCESS !=rtw_IOL_exec_cmds_sync(padapter, xmit_frame, 5000,0))
					ret = -EPERM;

			}

			if(start_value+write_num-1 == (final=rtw_read32(padapter, reg)) ) {
				DBG_871X("continuous IOL_CMD_WD_REG to 0x%x %u times Success, start:%u, final:%u\n", reg, write_num, start_value, final);
			} else {
				DBG_871X("continuous IOL_CMD_WD_REG to 0x%x %u times Fail, start:%u, final:%u\n", reg, write_num, start_value, final);
			}
		}
		break;
#endif //CONFIG_IOL
		}
		break;
	case 0x79: {
		/*
		* dbg 0x79000000 [value], set RESP_TXAGC to + value, value:0~15
		* dbg 0x79010000 [value], set RESP_TXAGC to - value, value:0~15
		*/
		u8 value =  extra_arg & 0x0f;
		u8 sign = minor_cmd;
		u16 write_value = 0;

		DBG_871X("%s set RESP_TXAGC to %s %u\n", __func__, sign?"minus":"plus", value);

		if (sign)
			value = value | 0x10;

		write_value = value | (value << 5);
		rtw_write16(padapter, 0x6d9, write_value);
	}
	break;
	case 0x7a:
		receive_disconnect(padapter, pmlmeinfo->network.MacAddress
		                   , WLAN_REASON_EXPIRATION_CHK);
		break;
	case 0x7F:
		switch(minor_cmd) {
		case 0x0:
			DBG_871X("fwstate=0x%x\n", get_fwstate(pmlmepriv));
			break;
		case 0x01:
			DBG_871X("auth_alg=0x%x, enc_alg=0x%x, auth_type=0x%x, enc_type=0x%x\n",
			         psecuritypriv->dot11AuthAlgrthm, psecuritypriv->dot11PrivacyAlgrthm,
			         psecuritypriv->ndisauthtype, psecuritypriv->ndisencryptstatus);
			break;
		case 0x02:
			DBG_871X("pmlmeinfo->state=0x%x\n", pmlmeinfo->state);
			DBG_871X("DrvBcnEarly=%d\n", pmlmeext->DrvBcnEarly);
			DBG_871X("DrvBcnTimeOut=%d\n", pmlmeext->DrvBcnTimeOut);
			break;
		case 0x03:
			DBG_871X("qos_option=%d\n", pmlmepriv->qospriv.qos_option);
#ifdef CONFIG_80211N_HT
			DBG_871X("ht_option=%d\n", pmlmepriv->htpriv.ht_option);
#endif //CONFIG_80211N_HT
			break;
		case 0x04:
			DBG_871X("cur_ch=%d\n", pmlmeext->cur_channel);
			DBG_871X("cur_bw=%d\n", pmlmeext->cur_bwmode);
			DBG_871X("cur_ch_off=%d\n", pmlmeext->cur_ch_offset);

			DBG_871X("oper_ch=%d\n", rtw_get_oper_ch(padapter));
			DBG_871X("oper_bw=%d\n", rtw_get_oper_bw(padapter));
			DBG_871X("oper_ch_offet=%d\n", rtw_get_oper_choffset(padapter));

			break;
		case 0x05:
			psta = rtw_get_stainfo(pstapriv, cur_network->network.MacAddress);
			if(psta) {
				DBG_871X("SSID=%s\n", cur_network->network.Ssid.Ssid);
				DBG_871X("sta's macaddr:" MAC_FMT "\n", MAC_ARG(psta->hwaddr));
				DBG_871X("cur_channel=%d, cur_bwmode=%d, cur_ch_offset=%d\n", pmlmeext->cur_channel, pmlmeext->cur_bwmode, pmlmeext->cur_ch_offset);
				DBG_871X("rtsen=%d, cts2slef=%d\n", psta->rtsen, psta->cts2self);
				DBG_871X("state=0x%x, aid=%d, macid=%d, raid=%d\n", psta->state, psta->aid, psta->mac_id, psta->raid);
#ifdef CONFIG_80211N_HT
				DBG_871X("qos_en=%d, ht_en=%d, init_rate=%d\n", psta->qos_option, psta->htpriv.ht_option, psta->init_rate);
				DBG_871X("bwmode=%d, ch_offset=%d, sgi_20m=%d,sgi_40m=%d\n", psta->bw_mode, psta->htpriv.ch_offset, psta->htpriv.sgi_20m, psta->htpriv.sgi_40m);
				DBG_871X("ampdu_enable = %d\n", psta->htpriv.ampdu_enable);
				DBG_871X("agg_enable_bitmap=%x, candidate_tid_bitmap=%x\n", psta->htpriv.agg_enable_bitmap, psta->htpriv.candidate_tid_bitmap);
#endif //CONFIG_80211N_HT

				sta_rx_reorder_ctl_dump(RTW_DBGDUMP, psta);
			} else {
				DBG_871X("can't get sta's macaddr, cur_network's macaddr:" MAC_FMT "\n", MAC_ARG(cur_network->network.MacAddress));
			}
			break;
		case 0x06: {
			u32	ODMFlag = 0;
			rtw_hal_get_hwreg(padapter, HW_VAR_DM_FLAG, (u8*)(&ODMFlag));
			DBG_871X("(B)DMFlag=0x%x, arg=0x%x\n", ODMFlag, arg);
			ODMFlag = (u32)(0x0f&arg);
			DBG_871X("(A)DMFlag=0x%x\n", ODMFlag);
			rtw_hal_set_hwreg(padapter, HW_VAR_DM_FLAG, (u8 *)(&ODMFlag));
		}
		break;
		case 0x07:
			DBG_871X("bSurpriseRemoved=%d, bDriverStopped=%d\n",
			         padapter->bSurpriseRemoved, padapter->bDriverStopped);
			break;
		case 0x08: {
			struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
			struct recv_priv  *precvpriv = &padapter->recvpriv;

			DBG_871X("free_xmitbuf_cnt=%d, free_xmitframe_cnt=%d"
			         ", free_xmit_extbuf_cnt=%d, free_xframe_ext_cnt=%d"
			         ", free_recvframe_cnt=%d\n",
			         pxmitpriv->free_xmitbuf_cnt, pxmitpriv->free_xmitframe_cnt,
			         pxmitpriv->free_xmit_extbuf_cnt, pxmitpriv->free_xframe_ext_cnt,
			         precvpriv->free_recvframe_cnt);
#ifdef CONFIG_USB_HCI
			DBG_871X("rx_urb_pending_cn=%d\n", ATOMIC_READ(&(precvpriv->rx_pending_cnt)));
#endif
		}
		break;
		case 0x09: {
			int i;
			_list	*plist, *phead;

#ifdef CONFIG_AP_MODE
			DBG_871X("sta_dz_bitmap=0x%x, tim_bitmap=0x%x\n", pstapriv->sta_dz_bitmap, pstapriv->tim_bitmap);
#endif
			_enter_critical_bh(&pstapriv->sta_hash_lock, &irqL);

			for(i=0; i< NUM_STA; i++) {
				phead = &(pstapriv->sta_hash[i]);
				plist = get_next(phead);

				while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
					psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

					plist = get_next(plist);

					if(extra_arg == psta->aid) {
						DBG_871X("sta's macaddr:" MAC_FMT "\n", MAC_ARG(psta->hwaddr));
						DBG_871X("rtsen=%d, cts2slef=%d\n", psta->rtsen, psta->cts2self);
						DBG_871X("state=0x%x, aid=%d, macid=%d, raid=%d\n", psta->state, psta->aid, psta->mac_id, psta->raid);
#ifdef CONFIG_80211N_HT
						DBG_871X("qos_en=%d, ht_en=%d, init_rate=%d\n", psta->qos_option, psta->htpriv.ht_option, psta->init_rate);
						DBG_871X("bwmode=%d, ch_offset=%d, sgi_20m=%d,sgi_40m=%d\n", psta->bw_mode, psta->htpriv.ch_offset, psta->htpriv.sgi_20m, psta->htpriv.sgi_40m);
						DBG_871X("ampdu_enable = %d\n", psta->htpriv.ampdu_enable);
						DBG_871X("agg_enable_bitmap=%x, candidate_tid_bitmap=%x\n", psta->htpriv.agg_enable_bitmap, psta->htpriv.candidate_tid_bitmap);
#endif //CONFIG_80211N_HT

#ifdef CONFIG_AP_MODE
						DBG_871X("capability=0x%x\n", psta->capability);
						DBG_871X("flags=0x%x\n", psta->flags);
						DBG_871X("wpa_psk=0x%x\n", psta->wpa_psk);
						DBG_871X("wpa2_group_cipher=0x%x\n", psta->wpa2_group_cipher);
						DBG_871X("wpa2_pairwise_cipher=0x%x\n", psta->wpa2_pairwise_cipher);
						DBG_871X("qos_info=0x%x\n", psta->qos_info);
#endif
						DBG_871X("dot118021XPrivacy=0x%x\n", psta->dot118021XPrivacy);

						sta_rx_reorder_ctl_dump(RTW_DBGDUMP, psta);
					}

				}
			}

			_exit_critical_bh(&pstapriv->sta_hash_lock, &irqL);

		}
		break;
		case 0x0a: {
			int max_mac_id = 0;
			max_mac_id = rtw_search_max_mac_id( padapter);
			printk("%s ==> max_mac_id = %d \n",__FUNCTION__,max_mac_id);
		}
		break;
		case 0x0b: { //Enable=1, Disable=0 driver control vrtl_carrier_sense.
			//u8 driver_vcs_en; //Enable=1, Disable=0 driver control vrtl_carrier_sense.
			//u8 driver_vcs_type;//force 0:disable VCS, 1:RTS-CTS, 2:CTS-to-self when vcs_en=1.

			if(arg == 0) {
				DBG_871X("disable driver ctrl vcs\n");
				padapter->driver_vcs_en = 0;
			} else if(arg == 1) {
				DBG_871X("enable driver ctrl vcs = %d\n", extra_arg);
				padapter->driver_vcs_en = 1;

				if(extra_arg>2)
					padapter->driver_vcs_type = 1;
				else
					padapter->driver_vcs_type = extra_arg;
			}
		}
		break;
		case 0x0c: { //dump rx/tx packet
			if(arg == 0) {
				DBG_871X("dump rx packet (%d)\n",extra_arg);
				//pHalData->bDumpRxPkt =extra_arg;
				rtw_hal_set_def_var(padapter, HAL_DEF_DBG_DUMP_RXPKT, &(extra_arg));
			} else if(arg==1) {
				DBG_871X("dump tx packet (%d)\n",extra_arg);
				rtw_hal_set_def_var(padapter, HAL_DEF_DBG_DUMP_TXPKT, &(extra_arg));
			}
		}
		break;
#if 0
		case 0x0d: { //dump cam
			//u8 entry = (u8) extra_arg;
			u8 entry=0;
			//dump cam
			for(entry=0; entry<32; entry++)
				read_cam(padapter,entry);
		}
		break;
#endif
		case 0x0e: {
			if(arg == 0) {
				DBG_871X("disable driver ctrl rx_ampdu_factor\n");
				padapter->driver_rx_ampdu_factor = 0xFF;
			} else if(arg == 1) {

				DBG_871X("enable driver ctrl rx_ampdu_factor = %d\n", extra_arg);

				if(extra_arg > 0x03)
					padapter->driver_rx_ampdu_factor = 0xFF;
				else
					padapter->driver_rx_ampdu_factor = extra_arg;
			}
		}
		break;
#ifdef DBG_CONFIG_ERROR_DETECT
		case 0x0f: {
			if(extra_arg == 0) {
				DBG_871X("###### silent reset test.......#####\n");
				rtw_hal_sreset_reset(padapter);
			} else {
				HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
				struct sreset_priv *psrtpriv = &pHalData->srestpriv;
				psrtpriv->dbg_trigger_point = extra_arg;
			}

		}
		break;
		case 0x15: {
			struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
			DBG_871X("==>silent resete cnts:%d\n",pwrpriv->ips_enter_cnts);
		}
		break;

#endif

		case 0x10:// driver version display
			dump_drv_version(RTW_DBGDUMP);
			break;
		case 0x11: { //dump linked status
			int pre_mode;
			pre_mode=padapter->bLinkInfoDump;
			// linked_info_dump(padapter,extra_arg);
			if(extra_arg==1 || (extra_arg==0 && pre_mode==1) ) { //not consider pwr_saving 0:
				padapter->bLinkInfoDump = extra_arg;

			} else if( (extra_arg==2 ) || (extra_arg==0 && pre_mode==2)) { //consider power_saving
				//DBG_871X("linked_info_dump =%s \n", (padapter->bLinkInfoDump)?"enable":"disable")
				linked_info_dump(padapter,extra_arg);
			}



		}
		break;
#ifdef CONFIG_80211N_HT
		case 0x12: { //set rx_stbc
			struct registry_priv	*pregpriv = &padapter->registrypriv;
			// 0: disable, bit(0):enable 2.4g, bit(1):enable 5g, 0x3: enable both 2.4g and 5g
			//default is set to enable 2.4GHZ for IOT issue with bufflao's AP at 5GHZ
			if( pregpriv && (extra_arg == 0 || extra_arg == 1|| extra_arg == 2 || extra_arg == 3)) {
				pregpriv->rx_stbc= extra_arg;
				DBG_871X("set rx_stbc=%d\n",pregpriv->rx_stbc);
			} else
				DBG_871X("get rx_stbc=%d\n",pregpriv->rx_stbc);

		}
		break;
		case 0x13: { //set ampdu_enable
			struct registry_priv	*pregpriv = &padapter->registrypriv;
			// 0: disable, 0x1:enable (but wifi_spec should be 0), 0x2: force enable (don't care wifi_spec)
			if( pregpriv && extra_arg < 3 ) {
				pregpriv->ampdu_enable= extra_arg;
				DBG_871X("set ampdu_enable=%d\n",pregpriv->ampdu_enable);
			} else
				DBG_871X("get ampdu_enable=%d\n",pregpriv->ampdu_enable);

		}
		break;
#endif
		case 0x14: { //get wifi_spec
			struct registry_priv	*pregpriv = &padapter->registrypriv;
			DBG_871X("get wifi_spec=%d\n",pregpriv->wifi_spec);

		}
		break;
		case 0x16: {
			if(arg == 0xff) {
				rtw_odm_dbg_comp_msg(RTW_DBGDUMP,padapter);
			} else {
				u64 dbg_comp = (u64)extra_arg;
				rtw_odm_dbg_comp_set(padapter, dbg_comp);
			}
		}
		break;
#ifdef DBG_FIXED_CHAN
		case 0x17: {
			struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
			printk("===>  Fixed channel to %d \n",extra_arg);
			pmlmeext->fixed_chan = extra_arg;

		}
		break;
#endif
		case 0x18: {
			printk("===>  Switch USB Mode %d \n",extra_arg);
			rtw_hal_set_hwreg(padapter, HW_VAR_USB_MODE, (u8 *)&extra_arg);
		}
		break;
#ifdef CONFIG_80211N_HT
		case 0x19: {
			struct registry_priv	*pregistrypriv = &padapter->registrypriv;
			// extra_arg :
			// BIT0: Enable VHT LDPC Rx, BIT1: Enable VHT LDPC Tx,
			// BIT4: Enable HT LDPC Rx, BIT5: Enable HT LDPC Tx
			if(arg == 0) {
				DBG_871X("driver disable LDPC\n");
				pregistrypriv->ldpc_cap = 0x00;
			} else if(arg == 1) {
				DBG_871X("driver set LDPC cap = 0x%x\n", extra_arg);
				pregistrypriv->ldpc_cap = (u8)(extra_arg&0x33);
			}
		}
		break;
		case 0x1a: {
			struct registry_priv	*pregistrypriv = &padapter->registrypriv;
			// extra_arg :
			// BIT0: Enable VHT STBC Rx, BIT1: Enable VHT STBC Tx,
			// BIT4: Enable HT STBC Rx, BIT5: Enable HT STBC Tx
			if(arg == 0) {
				DBG_871X("driver disable STBC\n");
				pregistrypriv->stbc_cap = 0x00;
			} else if(arg == 1) {
				DBG_871X("driver set STBC cap = 0x%x\n", extra_arg);
				pregistrypriv->stbc_cap = (u8)(extra_arg&0x33);
			}
		}
		break;
#endif //CONFIG_80211N_HT
		case 0x1b: {
			struct registry_priv	*pregistrypriv = &padapter->registrypriv;

			if(arg == 0) {
				DBG_871X("disable driver ctrl max_rx_rate, reset to default_rate_set\n");
				init_mlme_default_rate_set(padapter);
#ifdef CONFIG_80211N_HT
				pregistrypriv->ht_enable = (u8)rtw_ht_enable;
#endif //CONFIG_80211N_HT
			} else if(arg == 1) {

				int i;
				u8 max_rx_rate;

				DBG_871X("enable driver ctrl max_rx_rate = 0x%x\n", extra_arg);

				max_rx_rate = (u8)extra_arg;

				if(max_rx_rate < 0xc) { // max_rx_rate < MSC0 -> B or G -> disable HT
#ifdef CONFIG_80211N_HT
					pregistrypriv->ht_enable = 0;
#endif //CONFIG_80211N_HT
					for(i=0; i<NumRates; i++) {
						if(pmlmeext->datarate[i] > max_rx_rate)
							pmlmeext->datarate[i] = 0xff;
					}

				}
#ifdef CONFIG_80211N_HT
				else if(max_rx_rate < 0x1c) { // mcs0~mcs15
					u32 mcs_bitmap=0x0;

					for(i=0; i<((max_rx_rate+1)-0xc); i++)
						mcs_bitmap |= BIT(i);

					set_mcs_rate_by_mask(pmlmeext->default_supported_mcs_set, mcs_bitmap);
				}
#endif //CONFIG_80211N_HT
			}
		}
		break;
		case 0x1c: { //enable/disable driver control AMPDU Density for peer sta's rx
			if(arg == 0) {
				DBG_871X("disable driver ctrl ampdu density\n");
				padapter->driver_ampdu_spacing = 0xFF;
			} else if(arg == 1) {

				DBG_871X("enable driver ctrl ampdu density = %d\n", extra_arg);

				if(extra_arg > 0x07)
					padapter->driver_ampdu_spacing = 0xFF;
				else
					padapter->driver_ampdu_spacing = extra_arg;
			}
		}
		break;
#ifdef CONFIG_BACKGROUND_NOISE_MONITOR
		case 0x1e: {
			HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
			PDM_ODM_T pDM_Odm = &pHalData->odmpriv;
			u8 chan = rtw_get_oper_ch(padapter);
			DBG_871X("===========================================\n");
			ODM_InbandNoise_Monitor(pDM_Odm,_TRUE,0x1e,100);
			DBG_871X("channel(%d),noise_a = %d, noise_b = %d , noise_all:%d \n",
			         chan,pDM_Odm->noise_level.noise[ODM_RF_PATH_A],
			         pDM_Odm->noise_level.noise[ODM_RF_PATH_B],
			         pDM_Odm->noise_level.noise_all);
			DBG_871X("===========================================\n");

		}
		break;
#endif
		case 0x23: {
			DBG_871X("turn %s the bNotifyChannelChange Variable\n",(extra_arg==1)?"on":"off");
			padapter->bNotifyChannelChange = extra_arg;
			break;
		}
		case 0x24: {
#ifdef CONFIG_P2P
			DBG_871X("turn %s the bShowGetP2PState Variable\n",(extra_arg==1)?"on":"off");
			padapter->bShowGetP2PState = extra_arg;
#endif // CONFIG_P2P
			break;
		}
#ifdef CONFIG_GPIO_API
		case 0x25: { //Get GPIO register
			/*
			* dbg 0x7f250000 [gpio_num], Get gpio value, gpio_num:0~7
			*/

			u8 value;
			DBG_871X("Read GPIO Value  extra_arg = %d\n",extra_arg);
			value = rtw_hal_get_gpio(padapter,extra_arg);
			DBG_871X("Read GPIO Value = %d\n",value);
			break;
		}
		case 0x26: { //Set GPIO direction

			/* dbg 0x7f26000x [y], Set gpio direction,
			* x: gpio_num,4~7  y: indicate direction, 0~1
			*/

			int value;
			DBG_871X("Set GPIO Direction! arg = %d ,extra_arg=%d\n",arg ,extra_arg);
			value = rtw_hal_config_gpio(padapter, arg, extra_arg);
			DBG_871X("Set GPIO Direction %s \n",(value==-1)?"Fail!!!":"Success");
			break;
		}
		case 0x27: { //Set GPIO output direction value
			/*
			* dbg 0x7f27000x [y], Set gpio output direction value,
			* x: gpio_num,4~7  y: indicate direction, 0~1
			*/

			int value;
			DBG_871X("Set GPIO Value! arg = %d ,extra_arg=%d\n",arg ,extra_arg);
			value = rtw_hal_set_gpio_output_value(padapter,arg,extra_arg);
			DBG_871X("Set GPIO Value %s \n",(value==-1)?"Fail!!!":"Success");
			break;
		}
#endif
#ifdef DBG_CMD_QUEUE
		case 0x28: {
			dump_cmd_id = extra_arg;
			DBG_871X("dump_cmd_id:%d\n",dump_cmd_id);
		}
		break;
#endif //DBG_CMD_QUEUE
		case 0xaa: {
			if((extra_arg & 0x7F)> 0x3F) extra_arg = 0xFF;
			DBG_871X("chang data rate to :0x%02x\n",extra_arg);
			padapter->fix_rate = extra_arg;
		}
		break;
		case 0xdd: { //registers dump , 0 for mac reg,1 for bb reg, 2 for rf reg
			if(extra_arg==0) {
				mac_reg_dump(RTW_DBGDUMP, padapter);
			} else if(extra_arg==1) {
				bb_reg_dump(RTW_DBGDUMP, padapter);
			} else if(extra_arg==2) {
				rf_reg_dump(RTW_DBGDUMP, padapter);
			}
		}
		break;

		case 0xee: { //turn on/off dynamic funcs
			u32 odm_flag = 0;

			if(0xf==extra_arg) {
				rtw_hal_get_def_var(padapter, HAL_DEF_DBG_DM_FUNC,&odm_flag);
				DBG_871X(" === DMFlag(0x%08x) === \n",odm_flag);
				DBG_871X("extra_arg = 0  - disable all dynamic func \n");
				DBG_871X("extra_arg = 1  - disable DIG- BIT(0)\n");
				DBG_871X("extra_arg = 2  - disable High power - BIT(1)\n");
				DBG_871X("extra_arg = 3  - disable tx power tracking - BIT(2)\n");
				DBG_871X("extra_arg = 4  - disable BT coexistence - BIT(3)\n");
				DBG_871X("extra_arg = 5  - disable antenna diversity - BIT(4)\n");
				DBG_871X("extra_arg = 6  - enable all dynamic func \n");
			} else {
				/*	extra_arg = 0  - disable all dynamic func
					extra_arg = 1  - disable DIG
					extra_arg = 2  - disable tx power tracking
					extra_arg = 3  - turn on all dynamic func
				*/
				rtw_hal_set_def_var(padapter, HAL_DEF_DBG_DM_FUNC, &(extra_arg));
				rtw_hal_get_def_var(padapter, HAL_DEF_DBG_DM_FUNC,&odm_flag);
				DBG_871X(" === DMFlag(0x%08x) === \n",odm_flag);
			}
		}
		break;

		case 0xfd:
			rtw_write8(padapter, 0xc50, arg);
			DBG_871X("wr(0xc50)=0x%x\n", rtw_read8(padapter, 0xc50));
			rtw_write8(padapter, 0xc58, arg);
			DBG_871X("wr(0xc58)=0x%x\n", rtw_read8(padapter, 0xc58));
			break;
		case 0xfe:
			DBG_871X("rd(0xc50)=0x%x\n", rtw_read8(padapter, 0xc50));
			DBG_871X("rd(0xc58)=0x%x\n", rtw_read8(padapter, 0xc58));
			break;
		case 0xff: {
			DBG_871X("dbg(0x210)=0x%x\n", rtw_read32(padapter, 0x210));
			DBG_871X("dbg(0x608)=0x%x\n", rtw_read32(padapter, 0x608));
			DBG_871X("dbg(0x280)=0x%x\n", rtw_read32(padapter, 0x280));
			DBG_871X("dbg(0x284)=0x%x\n", rtw_read32(padapter, 0x284));
			DBG_871X("dbg(0x288)=0x%x\n", rtw_read32(padapter, 0x288));

			DBG_871X("dbg(0x664)=0x%x\n", rtw_read32(padapter, 0x664));


			DBG_871X("\n");

			DBG_871X("dbg(0x430)=0x%x\n", rtw_read32(padapter, 0x430));
			DBG_871X("dbg(0x438)=0x%x\n", rtw_read32(padapter, 0x438));

			DBG_871X("dbg(0x440)=0x%x\n", rtw_read32(padapter, 0x440));

			DBG_871X("dbg(0x458)=0x%x\n", rtw_read32(padapter, 0x458));

			DBG_871X("dbg(0x484)=0x%x\n", rtw_read32(padapter, 0x484));
			DBG_871X("dbg(0x488)=0x%x\n", rtw_read32(padapter, 0x488));

			DBG_871X("dbg(0x444)=0x%x\n", rtw_read32(padapter, 0x444));
			DBG_871X("dbg(0x448)=0x%x\n", rtw_read32(padapter, 0x448));
			DBG_871X("dbg(0x44c)=0x%x\n", rtw_read32(padapter, 0x44c));
			DBG_871X("dbg(0x450)=0x%x\n", rtw_read32(padapter, 0x450));
		}
		break;
		}
		break;
	default:
		DBG_871X("error dbg cmd!\n");
		break;
	}


	return ret;

}

static int wpa_set_param(struct net_device *dev, u8 name, u32 value)
{
	uint ret=0;
	//u32 flags;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	switch (name) {
	case IEEE_PARAM_WPA_ENABLED:

		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X; //802.1x

		//ret = ieee80211_wpa_enable(ieee, value);

		switch((value)&0xff) {
		case 1 : //WPA
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeWPAPSK; //WPA_PSK
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
			break;
		case 2: //WPA2
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeWPA2PSK; //WPA2_PSK
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
			break;
		}

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("wpa_set_param:padapter->securitypriv.ndisauthtype=%d\n", padapter->securitypriv.ndisauthtype));

		break;

	case IEEE_PARAM_TKIP_COUNTERMEASURES:
		//ieee->tkip_countermeasures=value;
		break;

	case IEEE_PARAM_DROP_UNENCRYPTED: {
		/* HACK:
		 *
		 * wpa_supplicant calls set_wpa_enabled when the driver
		 * is loaded and unloaded, regardless of if WPA is being
		 * used.  No other calls are made which can be used to
		 * determine if encryption will be used or not prior to
		 * association being expected.  If encryption is not being
		 * used, drop_unencrypted is set to false, else true -- we
		 * can use this to determine if the CAP_PRIVACY_ON bit should
		 * be set.
		 */

#if 0
		struct ieee80211_security sec = {
			.flags = SEC_ENABLED,
			.enabled = value,
		};
		ieee->drop_unencrypted = value;
		/* We only change SEC_LEVEL for open mode. Others
		 * are set by ipw_wpa_set_encryption.
		 */
		if (!value) {
			sec.flags |= SEC_LEVEL;
			sec.level = SEC_LEVEL_0;
		} else {
			sec.flags |= SEC_LEVEL;
			sec.level = SEC_LEVEL_1;
		}
		if (ieee->set_security)
			ieee->set_security(ieee->dev, &sec);
#endif
		break;

	}
	case IEEE_PARAM_PRIVACY_INVOKED:

		//ieee->privacy_invoked=value;

		break;

	case IEEE_PARAM_AUTH_ALGS:

		ret = wpa_set_auth_algs(dev, value);

		break;

	case IEEE_PARAM_IEEE_802_1X:

		//ieee->ieee802_1x=value;

		break;

	case IEEE_PARAM_WPAX_SELECT:

		// added for WPA2 mixed mode
		//DBG_871X(KERN_WARNING "------------------------>wpax value = %x\n", value);
		/*
		spin_lock_irqsave(&ieee->wpax_suitlist_lock,flags);
		ieee->wpax_type_set = 1;
		ieee->wpax_type_notify = value;
		spin_unlock_irqrestore(&ieee->wpax_suitlist_lock,flags);
		*/

		break;

	default:



		ret = -EOPNOTSUPP;


		break;

	}

	return ret;

}

static int wpa_mlme(struct net_device *dev, u32 command, u32 reason)
{
	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	switch (command) {
	case IEEE_MLME_STA_DEAUTH:

		if(!rtw_set_802_11_disassociate(padapter))
			ret = -1;

		break;

	case IEEE_MLME_STA_DISASSOC:

		if(!rtw_set_802_11_disassociate(padapter))
			ret = -1;

		break;

	default:
		ret = -EOPNOTSUPP;
		break;
	}

	return ret;

}

static int wpa_supplicant_ioctl(struct net_device *dev, struct iw_point *p)
{
	struct ieee_param *param;
	uint ret=0;

	//down(&ieee->wx_sem);

	if (p->length < sizeof(struct ieee_param) || !p->pointer) {
		ret = -EINVAL;
		goto out;
	}

	param = (struct ieee_param *)rtw_malloc(p->length);
	if (param == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	if (copy_from_user(param, p->pointer, p->length)) {
		rtw_mfree((u8*)param, p->length);
		ret = -EFAULT;
		goto out;
	}

	switch (param->cmd) {

	case IEEE_CMD_SET_WPA_PARAM:
		ret = wpa_set_param(dev, param->u.wpa_param.name, param->u.wpa_param.value);
		break;

	case IEEE_CMD_SET_WPA_IE:
		//ret = wpa_set_wpa_ie(dev, param, p->length);
		ret =  rtw_set_wpa_ie((_adapter *)rtw_netdev_priv(dev), (char*)param->u.wpa_ie.data, (u16)param->u.wpa_ie.len);
		break;

	case IEEE_CMD_SET_ENCRYPTION:
		ret = wpa_set_encryption(dev, param, p->length);
		break;

	case IEEE_CMD_MLME:
		ret = wpa_mlme(dev, param->u.mlme.command, param->u.mlme.reason_code);
		break;

	default:
		DBG_871X("Unknown WPA supplicant request: %d\n", param->cmd);
		ret = -EOPNOTSUPP;
		break;

	}

	if (ret == 0 && copy_to_user(p->pointer, param, p->length))
		ret = -EFAULT;

	rtw_mfree((u8 *)param, p->length);

out:

	//up(&ieee->wx_sem);

	return ret;

}

#ifdef CONFIG_AP_MODE
static int rtw_set_encryption(struct net_device *dev, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len,wep_total_len;
	NDIS_802_11_WEP	 *pwep = NULL;
	struct sta_info *psta = NULL, *pbcmc_sta = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv 	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_871X("%s\n", __FUNCTION__);

	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	//sizeof(struct ieee_param) = 64 bytes;
	//if (param_len !=  (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len)
	if (param_len !=  sizeof(struct ieee_param) + param->u.crypt.key_len) {
		ret =  -EINVAL;
		goto exit;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) {
		if (param->u.crypt.idx >= WEP_KEYS) {
			ret = -EINVAL;
			goto exit;
		}
	} else {
		psta = rtw_get_stainfo(pstapriv, param->sta_addr);
		if(!psta) {
			//ret = -EINVAL;
			DBG_871X("rtw_set_encryption(), sta has already been removed or never been added\n");
			goto exit;
		}
	}

	if (strcmp(param->u.crypt.alg, "none") == 0 && (psta==NULL)) {
		//todo:clear default encryption keys

		psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open;
		psecuritypriv->ndisencryptstatus = Ndis802_11EncryptionDisabled;
		psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;
		psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;

		DBG_871X("clear default encryption keys, keyid=%d\n", param->u.crypt.idx);

		goto exit;
	}


	if (strcmp(param->u.crypt.alg, "WEP") == 0 && (psta==NULL)) {
		DBG_871X("r871x_set_encryption, crypt.alg = WEP\n");

		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;

		DBG_871X("r871x_set_encryption, wep_key_idx=%d, len=%d\n", wep_key_idx, wep_key_len);

		if((wep_key_idx >= WEP_KEYS) || (wep_key_len<=0)) {
			ret = -EINVAL;
			goto exit;
		}


		if (wep_key_len > 0) {
			wep_key_len = wep_key_len <= 5 ? 5 : 13;
			wep_total_len = wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);
			pwep =(NDIS_802_11_WEP *)rtw_malloc(wep_total_len);
			if(pwep == NULL) {
				DBG_871X(" r871x_set_encryption: pwep allocate fail !!!\n");
				goto exit;
			}

			_rtw_memset(pwep, 0, wep_total_len);

			pwep->KeyLength = wep_key_len;
			pwep->Length = wep_total_len;

		}

		pwep->KeyIndex = wep_key_idx;

		_rtw_memcpy(pwep->KeyMaterial,  param->u.crypt.key, pwep->KeyLength);

		if(param->u.crypt.set_tx) {
			DBG_871X("wep, set_tx=1\n");

			psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
			psecuritypriv->ndisencryptstatus = Ndis802_11Encryption1Enabled;
			psecuritypriv->dot11PrivacyAlgrthm=_WEP40_;
			psecuritypriv->dot118021XGrpPrivacy=_WEP40_;

			if(pwep->KeyLength==13) {
				psecuritypriv->dot11PrivacyAlgrthm=_WEP104_;
				psecuritypriv->dot118021XGrpPrivacy=_WEP104_;
			}


			psecuritypriv->dot11PrivacyKeyIndex = wep_key_idx;

			_rtw_memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);

			psecuritypriv->dot11DefKeylen[wep_key_idx]=pwep->KeyLength;

			rtw_ap_set_wep_key(padapter, pwep->KeyMaterial, pwep->KeyLength, wep_key_idx, 1);
		} else {
			DBG_871X("wep, set_tx=0\n");

			//don't update "psecuritypriv->dot11PrivacyAlgrthm" and
			//"psecuritypriv->dot11PrivacyKeyIndex=keyid", but can rtw_set_key to cam

			_rtw_memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);

			psecuritypriv->dot11DefKeylen[wep_key_idx] = pwep->KeyLength;

			rtw_ap_set_wep_key(padapter, pwep->KeyMaterial, pwep->KeyLength, wep_key_idx, 0);
		}

		goto exit;

	}


	if(!psta && check_fwstate(pmlmepriv, WIFI_AP_STATE)) { // //group key
		if(param->u.crypt.set_tx ==1) {
			if(strcmp(param->u.crypt.alg, "WEP") == 0) {
				DBG_871X("%s, set group_key, WEP\n", __FUNCTION__);

				_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				psecuritypriv->dot118021XGrpPrivacy = _WEP40_;
				if(param->u.crypt.key_len==13) {
					psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
				}

			} else if(strcmp(param->u.crypt.alg, "TKIP") == 0) {
				DBG_871X("%s, set group_key, TKIP\n", __FUNCTION__);

				psecuritypriv->dot118021XGrpPrivacy = _TKIP_;

				_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
				//set mic key
				_rtw_memcpy(psecuritypriv->dot118021XGrptxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[16]), 8);
				_rtw_memcpy(psecuritypriv->dot118021XGrprxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[24]), 8);

				psecuritypriv->busetkipkey = _TRUE;

			} else if(strcmp(param->u.crypt.alg, "CCMP") == 0) {
				DBG_871X("%s, set group_key, CCMP\n", __FUNCTION__);

				psecuritypriv->dot118021XGrpPrivacy = _AES_;

				_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
			} else {
				DBG_871X("%s, set group_key, none\n", __FUNCTION__);

				psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
			}

			psecuritypriv->dot118021XGrpKeyid = param->u.crypt.idx;

			psecuritypriv->binstallGrpkey = _TRUE;

			psecuritypriv->dot11PrivacyAlgrthm = psecuritypriv->dot118021XGrpPrivacy;//!!!

			rtw_ap_set_group_key(padapter, param->u.crypt.key, psecuritypriv->dot118021XGrpPrivacy, param->u.crypt.idx);

			pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
			if(pbcmc_sta) {
				pbcmc_sta->ieee8021x_blocked = _FALSE;
				pbcmc_sta->dot118021XPrivacy= psecuritypriv->dot118021XGrpPrivacy;//rx will use bmc_sta's dot118021XPrivacy
			}

		}

		goto exit;

	}

	if(psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X && psta) { // psk/802_1x
		if(check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
			if(param->u.crypt.set_tx ==1) {
				_rtw_memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				if(strcmp(param->u.crypt.alg, "WEP") == 0) {
					DBG_871X("%s, set pairwise key, WEP\n", __FUNCTION__);

					psta->dot118021XPrivacy = _WEP40_;
					if(param->u.crypt.key_len==13) {
						psta->dot118021XPrivacy = _WEP104_;
					}
				} else if(strcmp(param->u.crypt.alg, "TKIP") == 0) {
					DBG_871X("%s, set pairwise key, TKIP\n", __FUNCTION__);

					psta->dot118021XPrivacy = _TKIP_;

					//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
					//set mic key
					_rtw_memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
					_rtw_memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

					psecuritypriv->busetkipkey = _TRUE;

				} else if(strcmp(param->u.crypt.alg, "CCMP") == 0) {

					DBG_871X("%s, set pairwise key, CCMP\n", __FUNCTION__);

					psta->dot118021XPrivacy = _AES_;
				} else {
					DBG_871X("%s, set pairwise key, none\n", __FUNCTION__);

					psta->dot118021XPrivacy = _NO_PRIVACY_;
				}

				rtw_ap_set_pairwise_key(padapter, psta);

				psta->ieee8021x_blocked = _FALSE;

			} else { //group key???
				if(strcmp(param->u.crypt.alg, "WEP") == 0) {
					_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					psecuritypriv->dot118021XGrpPrivacy = _WEP40_;
					if(param->u.crypt.key_len==13) {
						psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
					}
				} else if(strcmp(param->u.crypt.alg, "TKIP") == 0) {
					psecuritypriv->dot118021XGrpPrivacy = _TKIP_;

					_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
					//set mic key
					_rtw_memcpy(psecuritypriv->dot118021XGrptxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[16]), 8);
					_rtw_memcpy(psecuritypriv->dot118021XGrprxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[24]), 8);

					psecuritypriv->busetkipkey = _TRUE;

				} else if(strcmp(param->u.crypt.alg, "CCMP") == 0) {
					psecuritypriv->dot118021XGrpPrivacy = _AES_;

					_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
				} else {
					psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
				}

				psecuritypriv->dot118021XGrpKeyid = param->u.crypt.idx;

				psecuritypriv->binstallGrpkey = _TRUE;

				psecuritypriv->dot11PrivacyAlgrthm = psecuritypriv->dot118021XGrpPrivacy;//!!!

				rtw_ap_set_group_key(padapter, param->u.crypt.key, psecuritypriv->dot118021XGrpPrivacy, param->u.crypt.idx);

				pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
				if(pbcmc_sta) {
					pbcmc_sta->ieee8021x_blocked = _FALSE;
					pbcmc_sta->dot118021XPrivacy= psecuritypriv->dot118021XGrpPrivacy;//rx will use bmc_sta's dot118021XPrivacy
				}

			}

		}

	}

exit:

	if(pwep) {
		rtw_mfree((u8 *)pwep, wep_total_len);
	}

	return ret;

}

static int rtw_set_beacon(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	unsigned char *pbuf = param->u.bcn_ie.buf;


	DBG_871X("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	_rtw_memcpy(&pstapriv->max_num_sta, param->u.bcn_ie.reserved, 2);

	if((pstapriv->max_num_sta>NUM_STA) || (pstapriv->max_num_sta<=0))
		pstapriv->max_num_sta = NUM_STA;


	if(rtw_check_beacon_data(padapter, pbuf,  (len-12-2)) == _SUCCESS)// 12 = param header, 2:no packed
		ret = 0;
	else
		ret = -EINVAL;


	return ret;

}

static int rtw_hostapd_sta_flush(struct net_device *dev)
{
	//_irqL irqL;
	//_list	*phead, *plist;
	int ret=0;
	//struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	//struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_871X("%s\n", __FUNCTION__);

	flush_all_cam_entry(padapter);	//clear CAM

	ret = rtw_sta_flush(padapter);

	return ret;

}

static int rtw_add_sta(struct net_device *dev, struct ieee_param *param)
{
	//_irqL irqL;
	int ret=0;
	struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_871X("rtw_add_sta(aid=%d)=" MAC_FMT "\n", param->u.add_sta.aid, MAC_ARG(param->sta_addr));

	if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != _TRUE) {
		return -EINVAL;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) {
		return -EINVAL;
	}

	/*
		psta = rtw_get_stainfo(pstapriv, param->sta_addr);
		if(psta)
		{
			DBG_871X("rtw_add_sta(), free has been added psta=%p\n", psta);
			//_enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL);
			rtw_free_stainfo(padapter,  psta);
			//_exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL);

			psta = NULL;
		}
	*/
	//psta = rtw_alloc_stainfo(pstapriv, param->sta_addr);
	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if(psta) {
		int flags = param->u.add_sta.flags;

		//DBG_871X("rtw_add_sta(), init sta's variables, psta=%p\n", psta);

		psta->aid = param->u.add_sta.aid;//aid=1~2007

		_rtw_memcpy(psta->bssrateset, param->u.add_sta.tx_supp_rates, 16);


		//check wmm cap.
		if(WLAN_STA_WME&flags)
			psta->qos_option = 1;
		else
			psta->qos_option = 0;

		if(pmlmepriv->qospriv.qos_option == 0)
			psta->qos_option = 0;


#ifdef CONFIG_80211N_HT
		//chec 802.11n ht cap.
		if(WLAN_STA_HT&flags) {
			psta->htpriv.ht_option = _TRUE;
			psta->qos_option = 1;
			_rtw_memcpy((void*)&psta->htpriv.ht_cap, (void*)&param->u.add_sta.ht_cap, sizeof(struct rtw_ieee80211_ht_cap));
		} else {
			psta->htpriv.ht_option = _FALSE;
		}

		if(pmlmepriv->htpriv.ht_option == _FALSE)
			psta->htpriv.ht_option = _FALSE;
#endif


		update_sta_info_apmode(padapter, psta);


	} else {
		ret = -ENOMEM;
	}

	return ret;

}

static int rtw_del_sta(struct net_device *dev, struct ieee_param *param)
{
	_irqL irqL;
	int ret=0;
	struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_871X("rtw_del_sta=" MAC_FMT "\n", MAC_ARG(param->sta_addr));

	if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != _TRUE) {
		return -EINVAL;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) {
		return -EINVAL;
	}

	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if(psta) {
		u8 updated=_FALSE;

		//DBG_871X("free psta=%p, aid=%d\n", psta, psta->aid);

		_enter_critical_bh(&pstapriv->asoc_list_lock, &irqL);
		if(rtw_is_list_empty(&psta->asoc_list)==_FALSE) {
			rtw_list_delete(&psta->asoc_list);
			pstapriv->asoc_list_cnt--;
			updated = ap_free_sta(padapter, psta, _TRUE, WLAN_REASON_DEAUTH_LEAVING);

		}
		_exit_critical_bh(&pstapriv->asoc_list_lock, &irqL);

		associated_clients_update(padapter, updated);

		psta = NULL;

	} else {
		DBG_871X("rtw_del_sta(), sta has already been removed or never been added\n");

		//ret = -1;
	}


	return ret;

}

static int rtw_ioctl_get_sta_data(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct ieee_param_ex *param_ex = (struct ieee_param_ex *)param;
	struct sta_data *psta_data = (struct sta_data *)param_ex->data;

	DBG_871X("rtw_ioctl_get_sta_info, sta_addr: " MAC_FMT "\n", MAC_ARG(param_ex->sta_addr));

	if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != _TRUE) {
		return -EINVAL;
	}

	if (param_ex->sta_addr[0] == 0xff && param_ex->sta_addr[1] == 0xff &&
	    param_ex->sta_addr[2] == 0xff && param_ex->sta_addr[3] == 0xff &&
	    param_ex->sta_addr[4] == 0xff && param_ex->sta_addr[5] == 0xff) {
		return -EINVAL;
	}

	psta = rtw_get_stainfo(pstapriv, param_ex->sta_addr);
	if(psta) {
#if 0
		struct {
			u16 aid;
			u16 capability;
			int flags;
			u32 sta_set;
			u8 tx_supp_rates[16];
			u32 tx_supp_rates_len;
			struct rtw_ieee80211_ht_cap ht_cap;
			u64	rx_pkts;
			u64	rx_bytes;
			u64	rx_drops;
			u64	tx_pkts;
			u64	tx_bytes;
			u64	tx_drops;
		} get_sta;
#endif
		psta_data->aid = (u16)psta->aid;
		psta_data->capability = psta->capability;
		psta_data->flags = psta->flags;

		/*
				nonerp_set : BIT(0)
				no_short_slot_time_set : BIT(1)
				no_short_preamble_set : BIT(2)
				no_ht_gf_set : BIT(3)
				no_ht_set : BIT(4)
				ht_20mhz_set : BIT(5)
		*/

		psta_data->sta_set =((psta->nonerp_set) |
		                     (psta->no_short_slot_time_set <<1) |
		                     (psta->no_short_preamble_set <<2) |
		                     (psta->no_ht_gf_set <<3) |
		                     (psta->no_ht_set <<4) |
		                     (psta->ht_20mhz_set <<5));

		psta_data->tx_supp_rates_len =  psta->bssratelen;
		_rtw_memcpy(psta_data->tx_supp_rates, psta->bssrateset, psta->bssratelen);
#ifdef CONFIG_80211N_HT
		_rtw_memcpy(&psta_data->ht_cap, &psta->htpriv.ht_cap, sizeof(struct rtw_ieee80211_ht_cap));
#endif //CONFIG_80211N_HT
		psta_data->rx_pkts = psta->sta_stats.rx_data_pkts;
		psta_data->rx_bytes = psta->sta_stats.rx_bytes;
		psta_data->rx_drops = psta->sta_stats.rx_drops;

		psta_data->tx_pkts = psta->sta_stats.tx_pkts;
		psta_data->tx_bytes = psta->sta_stats.tx_bytes;
		psta_data->tx_drops = psta->sta_stats.tx_drops;


	} else {
		ret = -1;
	}

	return ret;

}

static int rtw_get_sta_wpaie(struct net_device *dev, struct ieee_param *param)
{
	int ret=0;
	struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_871X("rtw_get_sta_wpaie, sta_addr: " MAC_FMT "\n", MAC_ARG(param->sta_addr));

	if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != _TRUE) {
		return -EINVAL;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) {
		return -EINVAL;
	}

	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if(psta) {
		if((psta->wpa_ie[0] == WLAN_EID_RSN) || (psta->wpa_ie[0] == WLAN_EID_GENERIC)) {
			int wpa_ie_len;
			int copy_len;

			wpa_ie_len = psta->wpa_ie[1];

			copy_len = ((wpa_ie_len+2) > sizeof(psta->wpa_ie)) ? (sizeof(psta->wpa_ie)):(wpa_ie_len+2);

			param->u.wpa_ie.len = copy_len;

			_rtw_memcpy(param->u.wpa_ie.reserved, psta->wpa_ie, copy_len);
		} else {
			//ret = -1;
			DBG_871X("sta's wpa_ie is NONE\n");
		}
	} else {
		ret = -1;
	}

	return ret;

}

static int rtw_set_wps_beacon(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	const unsigned char wps_oui[4]= {0x0,0x50,0xf2,0x04};
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	//struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	int ie_len;

	DBG_871X("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	ie_len = len-12-2;// 12 = param header, 2:no packed


	if(pmlmepriv->wps_beacon_ie) {
		rtw_mfree(pmlmepriv->wps_beacon_ie, pmlmepriv->wps_beacon_ie_len);
		pmlmepriv->wps_beacon_ie = NULL;
	}

	if(ie_len>0) {
		pmlmepriv->wps_beacon_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_beacon_ie_len = ie_len;
		if ( pmlmepriv->wps_beacon_ie == NULL) {
			DBG_871X("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}

		_rtw_memcpy(pmlmepriv->wps_beacon_ie, param->u.bcn_ie.buf, ie_len);

		update_beacon(padapter, _VENDOR_SPECIFIC_IE_, wps_oui, _TRUE);

		pmlmeext->bstart_bss = _TRUE;

	}


	return ret;

}

static int rtw_set_wps_probe_resp(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int ie_len;

	DBG_871X("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	ie_len = len-12-2;// 12 = param header, 2:no packed


	if(pmlmepriv->wps_probe_resp_ie) {
		rtw_mfree(pmlmepriv->wps_probe_resp_ie, pmlmepriv->wps_probe_resp_ie_len);
		pmlmepriv->wps_probe_resp_ie = NULL;
	}

	if(ie_len>0) {
		pmlmepriv->wps_probe_resp_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_probe_resp_ie_len = ie_len;
		if ( pmlmepriv->wps_probe_resp_ie == NULL) {
			DBG_871X("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}
		_rtw_memcpy(pmlmepriv->wps_probe_resp_ie, param->u.bcn_ie.buf, ie_len);
	}


	return ret;

}

static int rtw_set_wps_assoc_resp(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int ie_len;

	DBG_871X("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	ie_len = len-12-2;// 12 = param header, 2:no packed


	if(pmlmepriv->wps_assoc_resp_ie) {
		rtw_mfree(pmlmepriv->wps_assoc_resp_ie, pmlmepriv->wps_assoc_resp_ie_len);
		pmlmepriv->wps_assoc_resp_ie = NULL;
	}

	if(ie_len>0) {
		pmlmepriv->wps_assoc_resp_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_assoc_resp_ie_len = ie_len;
		if ( pmlmepriv->wps_assoc_resp_ie == NULL) {
			DBG_871X("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}

		_rtw_memcpy(pmlmepriv->wps_assoc_resp_ie, param->u.bcn_ie.buf, ie_len);
	}


	return ret;

}

static int rtw_set_hidden_ssid(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *mlmepriv = &(adapter->mlmepriv);
	struct mlme_ext_priv	*mlmeext = &(adapter->mlmeextpriv);
	struct mlme_ext_info	*mlmeinfo = &(mlmeext->mlmext_info);
	int ie_len;
	u8 *ssid_ie;
	char ssid[NDIS_802_11_LENGTH_SSID + 1];
	sint ssid_len = 0;
	u8 ignore_broadcast_ssid;

	if(check_fwstate(mlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EPERM;

	if (param->u.bcn_ie.reserved[0] != 0xea)
		return -EINVAL;

	mlmeinfo->hidden_ssid_mode = ignore_broadcast_ssid = param->u.bcn_ie.reserved[1];

	ie_len = len-12-2;// 12 = param header, 2:no packed
	ssid_ie = rtw_get_ie(param->u.bcn_ie.buf,  WLAN_EID_SSID, &ssid_len, ie_len);

	if (ssid_ie && ssid_len > 0 && ssid_len <= NDIS_802_11_LENGTH_SSID) {
		WLAN_BSSID_EX *pbss_network = &mlmepriv->cur_network.network;
		WLAN_BSSID_EX *pbss_network_ext = &mlmeinfo->network;

		_rtw_memcpy(ssid, ssid_ie+2, ssid_len);
		ssid[ssid_len] = 0x0;

		if(0)
			DBG_871X(FUNC_ADPT_FMT" ssid:(%s,%d), from ie:(%s,%d), (%s,%d)\n", FUNC_ADPT_ARG(adapter),
			         ssid, ssid_len,
			         pbss_network->Ssid.Ssid, pbss_network->Ssid.SsidLength,
			         pbss_network_ext->Ssid.Ssid, pbss_network_ext->Ssid.SsidLength);

		_rtw_memcpy(pbss_network->Ssid.Ssid, (void *)ssid, ssid_len);
		pbss_network->Ssid.SsidLength = ssid_len;
		_rtw_memcpy(pbss_network_ext->Ssid.Ssid, (void *)ssid, ssid_len);
		pbss_network_ext->Ssid.SsidLength = ssid_len;

		if(0)
			DBG_871X(FUNC_ADPT_FMT" after ssid:(%s,%d), (%s,%d)\n", FUNC_ADPT_ARG(adapter),
			         pbss_network->Ssid.Ssid, pbss_network->Ssid.SsidLength,
			         pbss_network_ext->Ssid.Ssid, pbss_network_ext->Ssid.SsidLength);
	}

	DBG_871X(FUNC_ADPT_FMT" ignore_broadcast_ssid:%d, %s,%d\n", FUNC_ADPT_ARG(adapter),
	         ignore_broadcast_ssid, ssid, ssid_len);

	return ret;
}

static int rtw_ioctl_acl_remove_sta(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) {
		return -EINVAL;
	}

	ret = rtw_acl_remove_sta(padapter, param->sta_addr);

	return ret;

}

static int rtw_ioctl_acl_add_sta(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) {
		return -EINVAL;
	}

	ret = rtw_acl_add_sta(padapter, param->sta_addr);

	return ret;

}

static int rtw_ioctl_set_macaddr_acl(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	rtw_set_macaddr_acl(padapter, param->u.mlme.command);

	return ret;
}

static int rtw_hostapd_ioctl(struct net_device *dev, struct iw_point *p)
{
	struct ieee_param *param;
	int ret=0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	//DBG_871X("%s\n", __FUNCTION__);

	/*
	* this function is expect to call in master mode, which allows no power saving
	* so, we just check hw_init_completed
	*/

	if (padapter->hw_init_completed==_FALSE) {
		ret = -EPERM;
		goto out;
	}


	//if (p->length < sizeof(struct ieee_param) || !p->pointer){
	if(!p->pointer) {
		ret = -EINVAL;
		goto out;
	}

	param = (struct ieee_param *)rtw_malloc(p->length);
	if (param == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	if (copy_from_user(param, p->pointer, p->length)) {
		rtw_mfree((u8*)param, p->length);
		ret = -EFAULT;
		goto out;
	}

	//DBG_871X("%s, cmd=%d\n", __FUNCTION__, param->cmd);

	switch (param->cmd) {
	case RTL871X_HOSTAPD_FLUSH:

		ret = rtw_hostapd_sta_flush(dev);

		break;

	case RTL871X_HOSTAPD_ADD_STA:

		ret = rtw_add_sta(dev, param);

		break;

	case RTL871X_HOSTAPD_REMOVE_STA:

		ret = rtw_del_sta(dev, param);

		break;

	case RTL871X_HOSTAPD_SET_BEACON:

		ret = rtw_set_beacon(dev, param, p->length);

		break;

	case RTL871X_SET_ENCRYPTION:

		ret = rtw_set_encryption(dev, param, p->length);

		break;

	case RTL871X_HOSTAPD_GET_WPAIE_STA:

		ret = rtw_get_sta_wpaie(dev, param);

		break;

	case RTL871X_HOSTAPD_SET_WPS_BEACON:

		ret = rtw_set_wps_beacon(dev, param, p->length);

		break;

	case RTL871X_HOSTAPD_SET_WPS_PROBE_RESP:

		ret = rtw_set_wps_probe_resp(dev, param, p->length);

		break;

	case RTL871X_HOSTAPD_SET_WPS_ASSOC_RESP:

		ret = rtw_set_wps_assoc_resp(dev, param, p->length);

		break;

	case RTL871X_HOSTAPD_SET_HIDDEN_SSID:

		ret = rtw_set_hidden_ssid(dev, param, p->length);

		break;

	case RTL871X_HOSTAPD_GET_INFO_STA:

		ret = rtw_ioctl_get_sta_data(dev, param, p->length);

		break;

	case RTL871X_HOSTAPD_SET_MACADDR_ACL:

		ret = rtw_ioctl_set_macaddr_acl(dev, param, p->length);

		break;

	case RTL871X_HOSTAPD_ACL_ADD_STA:

		ret = rtw_ioctl_acl_add_sta(dev, param, p->length);

		break;

	case RTL871X_HOSTAPD_ACL_REMOVE_STA:

		ret = rtw_ioctl_acl_remove_sta(dev, param, p->length);

		break;

	default:
		DBG_871X("Unknown hostapd request: %d\n", param->cmd);
		ret = -EOPNOTSUPP;
		break;

	}

	if (ret == 0 && copy_to_user(p->pointer, param, p->length))
		ret = -EFAULT;


	rtw_mfree((u8 *)param, p->length);

out:

	return ret;

}
#endif

static int rtw_wx_set_priv(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *awrq,
                           char *extra)
{

#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	char *ext_dbg;
#endif

	int ret = 0;
	int len = 0;
	char *ext;
#ifdef CONFIG_ANDROID
	int i;
#endif

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct iw_point *dwrq = (struct iw_point*)awrq;

	//RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_notice_, ("+rtw_wx_set_priv\n"));
	if(dwrq->length == 0)
		return -EFAULT;

	len = dwrq->length;
	if (!(ext = rtw_vmalloc(len)))
		return -ENOMEM;

	if (copy_from_user(ext, dwrq->pointer, len)) {
		rtw_vmfree(ext, len);
		return -EFAULT;
	}


	//RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_notice_,
	//	 ("rtw_wx_set_priv: %s req=%s\n",
	//	  dev->name, ext));

#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	if (!(ext_dbg = rtw_vmalloc(len))) {
		rtw_vmfree(ext, len);
		return -ENOMEM;
	}

	_rtw_memcpy(ext_dbg, ext, len);
#endif

	//added for wps2.0 @20110524
	if(dwrq->flags == 0x8766 && len > 8) {
		u32 cp_sz;
		struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
		u8 *probereq_wpsie = ext;
		int probereq_wpsie_len = len;
		const u8 wps_oui[4]= {0x0,0x50,0xf2,0x04};

		if((_VENDOR_SPECIFIC_IE_ == probereq_wpsie[0]) &&
		   (_rtw_memcmp(&probereq_wpsie[2], wps_oui, 4) ==_TRUE)) {
			cp_sz = probereq_wpsie_len>MAX_WPS_IE_LEN ? MAX_WPS_IE_LEN:probereq_wpsie_len;

			if(pmlmepriv->wps_probe_req_ie) {
				u32 free_len = pmlmepriv->wps_probe_req_ie_len;
				pmlmepriv->wps_probe_req_ie_len = 0;
				rtw_mfree(pmlmepriv->wps_probe_req_ie, free_len);
				pmlmepriv->wps_probe_req_ie = NULL;
			}

			pmlmepriv->wps_probe_req_ie = rtw_malloc(cp_sz);
			if ( pmlmepriv->wps_probe_req_ie == NULL) {
				printk("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
				ret =  -EINVAL;
				goto FREE_EXT;

			}

			_rtw_memcpy(pmlmepriv->wps_probe_req_ie, probereq_wpsie, cp_sz);
			pmlmepriv->wps_probe_req_ie_len = cp_sz;

		}

		goto FREE_EXT;

	}

	if(	len >= WEXT_CSCAN_HEADER_SIZE
	    && _rtw_memcmp(ext, WEXT_CSCAN_HEADER, WEXT_CSCAN_HEADER_SIZE) == _TRUE
	  ) {
		ret = rtw_wx_set_scan(dev, info, awrq, ext);
		goto FREE_EXT;
	}

#ifdef CONFIG_ANDROID
	//DBG_871X("rtw_wx_set_priv: %s req=%s\n", dev->name, ext);

	i = rtw_android_cmdstr_to_num(ext);

	switch(i) {
	case ANDROID_WIFI_CMD_START :
		indicate_wx_custom_event(padapter, "START");
		break;
	case ANDROID_WIFI_CMD_STOP :
		indicate_wx_custom_event(padapter, "STOP");
		break;
	case ANDROID_WIFI_CMD_RSSI : {
		struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
		struct	wlan_network	*pcur_network = &pmlmepriv->cur_network;

		if(check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
			sprintf(ext, "%s rssi %d", pcur_network->network.Ssid.Ssid, padapter->recvpriv.rssi);
		} else {
			sprintf(ext, "OK");
		}
	}
	break;
	case ANDROID_WIFI_CMD_LINKSPEED : {
		u16 mbps = rtw_get_cur_max_rate(padapter)/10;
		sprintf(ext, "LINKSPEED %d", mbps);
	}
	break;
	case ANDROID_WIFI_CMD_MACADDR :
		sprintf(ext, "MACADDR = " MAC_FMT, MAC_ARG(dev->dev_addr));
		break;
	case ANDROID_WIFI_CMD_SCAN_ACTIVE : {
		//rtw_set_scan_mode(padapter, SCAN_ACTIVE);
		sprintf(ext, "OK");
	}
	break;
	case ANDROID_WIFI_CMD_SCAN_PASSIVE : {
		//rtw_set_scan_mode(padapter, SCAN_PASSIVE);
		sprintf(ext, "OK");
	}
	break;

	case ANDROID_WIFI_CMD_COUNTRY : {
		char country_code[10];
		sscanf(ext, "%*s %s", country_code);
		rtw_set_country(padapter, country_code);
		sprintf(ext, "OK");
	}
	break;
	default :
#ifdef  CONFIG_DEBUG_RTW_WX_SET_PRIV
		DBG_871X("%s: %s unknowned req=%s\n", __FUNCTION__,
		         dev->name, ext_dbg);
#endif

		sprintf(ext, "OK");

	}

	if (copy_to_user(dwrq->pointer, ext, min(dwrq->length, (u16)(strlen(ext)+1)) ) )
		ret = -EFAULT;

#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	DBG_871X("%s: %s req=%s rep=%s dwrq->length=%d, strlen(ext)+1=%d\n", __FUNCTION__,
	         dev->name, ext_dbg ,ext, dwrq->length, (u16)(strlen(ext)+1));
#endif
#endif //end of CONFIG_ANDROID


FREE_EXT:

	rtw_vmfree(ext, len);
#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	rtw_vmfree(ext_dbg, len);
#endif

	//DBG_871X("rtw_wx_set_priv: (SIOCSIWPRIV) %s ret=%d\n",
	//		dev->name, ret);

	return ret;

}
#ifdef CONFIG_WOWLAN
static int rtw_wowlan_ctrl(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter =  (_adapter *)rtw_netdev_priv(dev);
	struct wowlan_ioctl_param poidparam;
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	//struct net_device *pnetdev = padapter->pnetdev;
#ifdef CONFIG_CONCURRENT_MODE
	//struct net_device *pbuddy_netdev = padapter->pbuddy_adapter->pnetdev;
#endif
	//struct sta_info	*psta = NULL;
	int ret = 0;
	u32 start_time = rtw_get_current_time();
	poidparam.subcode = 0;

	DBG_871X("+rtw_wowlan_ctrl: %s\n", extra);

	if(pwrctrlpriv->bSupportRemoteWakeup==_FALSE) {
		ret = -EPERM;
		DBG_871X("+rtw_wowlan_ctrl: Device didn't support the remote wakeup!!\n");
		goto _rtw_wowlan_ctrl_exit_free;
	}

	if (!check_fwstate(pmlmepriv, _FW_LINKED) &&
	    check_fwstate(pmlmepriv, WIFI_STATION_STATE)) {
#ifdef CONFIG_PNO_SUPPORT
		pwrctrlpriv->wowlan_pno_enable = _TRUE;
#else
		DBG_871X("[%s] WARNING: Please Connect With AP First!!\n", __func__);
		goto _rtw_wowlan_ctrl_exit_free;
#endif //CONFIG_PNO_SUPPORT
	}

	if (_rtw_memcmp( extra, "enable", 6 )) {

		padapter->registrypriv.mp_mode = 1;

		pwrctrlpriv->wowlan_from_cmd = _TRUE;

		rtw_suspend_common(padapter);

	} else if (_rtw_memcmp( extra, "disable", 6 )) {

		rtw_resume_common(padapter);

		pwrctrlpriv->wowlan_from_cmd = _FALSE;

#ifdef CONFIG_PNO_SUPPORT
		pwrctrlpriv->wowlan_pno_enable = _FALSE;
#endif //CONFIG_PNO_SUPPORT

		padapter->registrypriv.mp_mode = 0;
	} else {
		DBG_871X("[%s] Invalid Parameter.\n", __func__);
		goto _rtw_wowlan_ctrl_exit_free;
	}
	//mutex_lock(&ioctl_mutex);
_rtw_wowlan_ctrl_exit_free:
	DBG_871X("-rtw_wowlan_ctrl( subcode = %d)\n", poidparam.subcode);
	DBG_871X_LEVEL(_drv_always_, "%s in %d ms\n", __func__,
	               rtw_get_passing_time_ms(start_time));
//_rtw_wowlan_ctrl_exit:
	return ret;
}
#endif //CONFIG_WOWLAN

#ifdef CONFIG_AP_WOWLAN
static int rtw_ap_wowlan_ctrl(struct net_device *dev,
                              struct iw_request_info *info,
                              union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter =  (_adapter *)rtw_netdev_priv(dev);
	struct wowlan_ioctl_param poidparam;
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct sta_info	*psta = NULL;
	int ret = 0;
	u32 start_time = rtw_get_current_time();
	poidparam.subcode = 0;

	DBG_871X("+rtw_ap_wowlan_ctrl: %s\n", extra);

	if(pwrctrlpriv->bSupportRemoteWakeup==_FALSE) {
		ret = -EPERM;
		DBG_871X("+rtw_wowlan_ctrl: Device didn't support the remote wakeup!!\n");
		goto _rtw_ap_wowlan_ctrl_exit_free;
	}

	if (!check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		DBG_871X("[%s] It is not AP mode!!\n", __func__);
		goto _rtw_ap_wowlan_ctrl_exit_free;
	}

	if (_rtw_memcmp( extra, "enable", 6 )) {
		pwrctrlpriv->wowlan_ap_mode = _TRUE;
		while (pwrctrlpriv->bips_processing == _TRUE)
			rtw_msleep_os(1);

		rtw_cancel_all_timer(padapter);

		padapter->bDriverStopped = _TRUE;	//for stop thread
		rtw_stop_drv_threads(padapter);
		padapter->bDriverStopped = _FALSE;	//for 32k command

#ifdef CONFIG_LPS
		LeaveAllPowerSaveModeDirect(padapter);
#endif
		rtw_hal_disable_interrupt(padapter); // It need wait for leaving 32K.

		// 2.1 clean interupt
		if (padapter->HalFunc.clear_interrupt)
			padapter->HalFunc.clear_interrupt(padapter);

		poidparam.subcode = WOWLAN_AP_ENABLE;

		rtw_hal_set_hwreg(padapter, HW_VAR_AP_WOWLAN,(u8 *)&poidparam);
	} else if (_rtw_memcmp( extra, "disable", 6 )) {
#ifdef CONFIG_LPS
		LeaveAllPowerSaveModeDirect(padapter);
#endif //CONFIG_LPS
		pwrctrlpriv->bFwCurrentInPSMode = _FALSE;

		rtw_hal_disable_interrupt(padapter);

		if (padapter->HalFunc.clear_interrupt)
			padapter->HalFunc.clear_interrupt(padapter);

		poidparam.subcode = WOWLAN_AP_ENABLE;

		rtw_hal_set_hwreg(padapter, HW_VAR_AP_WOWLAN,(u8 *)&poidparam);

		pwrctrlpriv->wowlan_ap_mode = _FALSE;

		psta = rtw_get_stainfo(&padapter->stapriv, get_bssid(&padapter->mlmepriv));
		if (psta) {
			set_sta_rate(padapter, psta);
		}

		padapter->bDriverStopped = _FALSE;
		DBG_871X("%s: wowmode resuming, DriverStopped:%d\n", __func__, padapter->bDriverStopped);
		rtw_start_drv_threads(padapter);

		rtw_hal_enable_interrupt(padapter);

		_set_timer(&padapter->mlmepriv.dynamic_chk_timer, 2000);
		pwrctrlpriv->bips_processing = _FALSE;
		rtw_set_pwr_state_check_timer(pwrctrlpriv);

	} else {
		DBG_871X("[%s] Invalid Parameter.\n", __func__);
		goto _rtw_ap_wowlan_ctrl_exit_free;
	}
	//mutex_lock(&ioctl_mutex);
_rtw_ap_wowlan_ctrl_exit_free:
	DBG_871X("-rtw_ap_wowlan_ctrl( subcode = %d)\n", poidparam.subcode);
	DBG_871X_LEVEL(_drv_always_, "%s in %d ms\n", __func__,
	               rtw_get_passing_time_ms(start_time));
_rtw_ap_wowlan_ctrl_exit:
	return ret;
}
#endif //CONFIG_AP_WOWLAN

static int rtw_pm_set(struct net_device *dev,
                      struct iw_request_info *info,
                      union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	unsigned	mode = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	DBG_871X( "[%s] extra = %s\n", __FUNCTION__, extra );

	if ( _rtw_memcmp( extra, "lps=", 4 ) ) {
		sscanf(extra+4, "%u", &mode);
		ret = rtw_pm_set_lps(padapter,mode);
	} else if ( _rtw_memcmp( extra, "ips=", 4 ) ) {
		sscanf(extra+4, "%u", &mode);
		ret = rtw_pm_set_ips(padapter,mode);
	} else {
		ret = -EINVAL;
	}

	return ret;
}

static int rtw_mp_efuse_get(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wdata, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL pEfuseHal;
	struct iw_point *wrqu;

	u8	*PROMContent = pEEPROM->efuse_eeprom_data;
	u8 ips_mode = IPS_NUM; // init invalid value
	u8 lps_mode = PS_MODE_NUM; // init invalid value
	struct pwrctrl_priv *pwrctrlpriv ;
	u8 *data = NULL;
	u8 *rawdata = NULL;
	char *pch, *ptmp, *token, *tmp[3]= {0x00,0x00,0x00};
	u16 i=0, j=0, mapLen=0, addr=0, cnts=0;
	u16 max_available_size=0, raw_cursize=0, raw_maxsize=0;
	int err;
#ifdef CONFIG_IOL
	u8 org_fw_iol = padapter->registrypriv.fw_iol;// 0:Disable, 1:enable, 2:by usb speed
#endif

	wrqu = (struct iw_point*)wdata;
	pwrctrlpriv = adapter_to_pwrctl(padapter);
	pEfuseHal = &pHalData->EfuseHal;

	err = 0;
	data = rtw_zmalloc(EFUSE_BT_MAX_MAP_LEN);
	if (data == NULL) {
		err = -ENOMEM;
		goto exit;
	}
	rawdata = rtw_zmalloc(EFUSE_BT_MAX_MAP_LEN);
	if (rawdata == NULL) {
		err = -ENOMEM;
		goto exit;
	}

	if (copy_from_user(extra, wrqu->pointer, wrqu->length)) {
		err = -EFAULT;
		goto exit;
	}
#ifdef CONFIG_LPS
	lps_mode = pwrctrlpriv->power_mgnt;//keep org value
	rtw_pm_set_lps(padapter,PS_MODE_ACTIVE);
#endif

#ifdef CONFIG_IPS
	ips_mode = pwrctrlpriv->ips_mode;//keep org value
	rtw_pm_set_ips(padapter,IPS_NONE);
#endif

	pch = extra;
	DBG_871X("%s: in=%s\n", __FUNCTION__, extra);

	i = 0;
	//mac 16 "00e04c871200" rmap,00,2
	while ((token = strsep(&pch, ",")) != NULL) {
		if (i > 2) break;
		tmp[i] = token;
		i++;
	}
#ifdef CONFIG_IOL
	padapter->registrypriv.fw_iol = 0;// 0:Disable, 1:enable, 2:by usb speed
#endif

	if(strcmp(tmp[0], "status") == 0) {
		sprintf(extra, "Load File efuse=%s,Load File MAC=%s",(pEEPROM->bloadfile_fail_flag? "FAIL" : "OK"),(pEEPROM->bloadmac_fail_flag? "FAIL" : "OK"));

		goto exit;
	} else if (strcmp(tmp[0], "drvmap") == 0) {
		mapLen = EFUSE_MAP_SIZE;

		sprintf(extra, "\n");
		for (i = 0; i < EFUSE_MAP_SIZE; i += 16) {
//			DBG_871X("0x%02x\t", i);
			sprintf(extra + strlen(extra), "0x%02x\t", i);
			for (j=0; j<8; j++) {
//				DBG_871X("%02X ", data[i+j]);
				sprintf(extra + strlen(extra), "%02X ", PROMContent[i+j]);
			}
//			DBG_871X("\t");
			sprintf(extra + strlen(extra), "\t");
			for (; j<16; j++) {
//				DBG_871X("%02X ", data[i+j]);
				sprintf(extra + strlen(extra), "%02X ", PROMContent[i+j]);
			}
//			DBG_871X("\n");
			sprintf(extra + strlen(extra), "\n");
		}
//		DBG_871X("\n");
	} else if (strcmp(tmp[0], "realmap") == 0) {
		mapLen = EFUSE_MAP_SIZE;
		if (rtw_efuse_map_read(padapter, EFUSE_WIFI , mapLen, pEfuseHal->fakeEfuseInitMap) == _FAIL) {
			DBG_871X("%s: read realmap Fail!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

//		DBG_871X("OFFSET\tVALUE(hex)\n");
		sprintf(extra, "\n");
		for (i = 0; i < EFUSE_MAP_SIZE; i += 16) {
//			DBG_871X("0x%02x\t", i);
			sprintf(extra + strlen(extra), "0x%02x\t", i);
			for (j=0; j<8; j++) {
//				DBG_871X("%02X ", data[i+j]);
				sprintf(extra + strlen(extra), "%02X ", pEfuseHal->fakeEfuseInitMap[i+j]);
			}
//			DBG_871X("\t");
			sprintf(extra + strlen(extra), "\t");
			for (; j<16; j++) {
//				DBG_871X("%02X ", data[i+j]);
				sprintf(extra + strlen(extra), "%02X ", pEfuseHal->fakeEfuseInitMap[i+j]);
			}
//			DBG_871X("\n");
			sprintf(extra + strlen(extra), "\n");
		}
//		DBG_871X("\n");
	} else if (strcmp(tmp[0], "rmap") == 0) {
		if ((tmp[1]==NULL) || (tmp[2]==NULL)) {
			DBG_871X("%s: rmap Fail!! Parameters error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}

		// rmap addr cnts
		addr = simple_strtoul(tmp[1], &ptmp, 16);
		DBG_871X("%s: addr=%x\n", __FUNCTION__, addr);

		cnts = simple_strtoul(tmp[2], &ptmp, 10);
		if (cnts == 0) {
			DBG_871X("%s: rmap Fail!! cnts error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);

		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN , (PVOID)&max_available_size, _FALSE);
		if ((addr+ cnts) > max_available_size) {
			DBG_871X("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EINVAL;
			goto exit;
		}

		if (rtw_efuse_map_read(padapter, addr, cnts, data) == _FAIL) {
			DBG_871X("%s: rtw_efuse_map_read error!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

//		DBG_871X("%s: data={", __FUNCTION__);
		*extra = 0;
		for (i=0; i<cnts; i++) {
//			DBG_871X("0x%02x ", data[i]);
			sprintf(extra + strlen(extra), "0x%02X ", data[i]);
		}
//		DBG_871X("}\n");
	} else if (strcmp(tmp[0], "realraw") == 0) {
		addr = 0;
		mapLen = EFUSE_MAX_SIZE;
		if (rtw_efuse_access(padapter, _FALSE, addr, mapLen, rawdata) == _FAIL) {
			DBG_871X("%s: rtw_efuse_access Fail!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
		_rtw_memset(extra,'\0',strlen(extra));
		//		DBG_871X("%s: realraw={\n", __FUNCTION__);
		sprintf(extra, "\n0x00\t");
		for (i=0; i< mapLen; i++) {
			//			DBG_871X("%02X", rawdata[i]);
			sprintf(extra + strlen(extra), "%02X", rawdata[i]);
			if ((i & 0xF) == 0xF) {
				//				DBG_871X("\n");
				sprintf(extra + strlen(extra), "\n");
				sprintf(extra + strlen(extra), "0x%02x\t", i+1);
			} else if ((i & 0x7) == 0x7) {
				//				DBG_871X("\t");
				sprintf(extra + strlen(extra), " \t");
			} else {
				//				DBG_871X(" ");
				sprintf(extra + strlen(extra), " ");
			}
		}
		//		DBG_871X("}\n");
	} else if (strcmp(tmp[0], "mac") == 0) {
#ifdef CONFIG_RTL8192C
		addr = EEPROM_MAC_ADDR_92C;
#endif // CONFIG_RTL8192C
#ifdef CONFIG_RTL8192D
#ifdef CONFIG_USB_HCI
		if (pHalData->interfaceIndex == 0)
			addr = EEPROM_MAC_ADDR_MAC0_92DU;
		else
			addr = EEPROM_MAC_ADDR_MAC1_92DU;
#else
		if (pHalData->interfaceIndex == 0)
			addr = EEPROM_MAC_ADDR_MAC0_92DE;
		else
			addr = EEPROM_MAC_ADDR_MAC1_92DE;
#endif
#endif // CONFIG_RTL8192D
#ifdef CONFIG_RTL8723A
#ifdef CONFIG_SDIO_HCI
		addr = EEPROM_MAC_ADDR_8723AS;
#endif
#ifdef CONFIG_GSPI_HCI
		addr = EEPROM_MAC_ADDR_8723AS;
#endif
#ifdef CONFIG_USB_HCI
		addr = EEPROM_MAC_ADDR_8723AU;
#endif
#endif // CONFIG_RTL8723A
#ifdef CONFIG_RTL8188E
#ifdef CONFIG_USB_HCI
		addr = EEPROM_MAC_ADDR_88EU;
#endif
#ifdef CONFIG_SDIO_HCI
		addr = EEPROM_MAC_ADDR_88ES;
#endif
#ifdef CONFIG_PCI_HCI
		addr = EEPROM_MAC_ADDR_88EE;
#endif
#endif // CONFIG_RTL8188E

#ifdef CONFIG_RTL8192E
#ifdef CONFIG_USB_HCI
		addr = EEPROM_MAC_ADDR_8192EU;
#endif
#ifdef CONFIG_SDIO_HCI
		addr = EEPROM_MAC_ADDR_8192ES;
#endif
#ifdef CONFIG_PCI_HCI
		addr = EEPROM_MAC_ADDR_8192EE;
#endif
#endif
#ifdef CONFIG_RTL8723B
#ifdef CONFIG_SDIO_HCI
		addr = EEPROM_MAC_ADDR_8723BS;
#endif
#ifdef CONFIG_GSPI_HCI
		addr = EEPROM_MAC_ADDR_8723BS;
#endif
#ifdef CONFIG_USB_HCI
		addr = EEPROM_MAC_ADDR_8723BU;
#endif
#endif // CONFIG_RTL8723B
		cnts = 6;

		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		if ((addr + cnts) > max_available_size) {
			DBG_871X("%s: addr(0x%02x)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_efuse_map_read(padapter, addr, cnts, data) == _FAIL) {
			DBG_871X("%s: rtw_efuse_map_read error!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

//		DBG_871X("%s: MAC address={", __FUNCTION__);
		*extra = 0;
		for (i=0; i<cnts; i++) {
//			DBG_871X("%02X", data[i]);
			sprintf(extra + strlen(extra), "%02X", data[i]);
			if (i != (cnts-1)) {
//				DBG_871X(":");
				sprintf(extra + strlen(extra), ":");
			}
		}
//		DBG_871X("}\n");
	} else if (strcmp(tmp[0], "vidpid") == 0) {
#ifdef CONFIG_RTL8192C
		addr = EEPROM_VID_92C;
#endif // CONFIG_RTL8192C
#ifdef CONFIG_RTL8192D
#ifdef CONFIG_USB_HCI
		addr = EEPROM_VID_92DU;
#else
		addr = EEPROM_VID_92DE;
#endif
#endif // CONFIG_RTL8192D
#ifdef CONFIG_RTL8723A
#ifdef CONFIG_USB_HCI
		addr = EEPROM_VID_8723AU;
#endif
#endif // CONFIG_RTL8723A
#ifdef CONFIG_RTL8188E
#ifdef CONFIG_USB_HCI
		addr = EEPROM_VID_88EU;
#endif
#ifdef CONFIG_PCI_HCI
		addr = EEPROM_VID_88EE;
#endif
#endif // CONFIG_RTL8188E

#ifdef CONFIG_RTL8192E
#ifdef CONFIG_USB_HCI
		addr = EEPROM_VID_8192EU;
#endif
#ifdef CONFIG_PCI_HCI
		addr = EEPROM_VID_8192EE;
#endif
#endif // CONFIG_RTL8192E
#ifdef CONFIG_RTL8723B
		addr = EEPROM_VID_8723BU;
#endif // CONFIG_RTL8192E
		cnts = 4;

		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		if ((addr + cnts) > max_available_size) {
			DBG_871X("%s: addr(0x%02x)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}
		if (rtw_efuse_map_read(padapter, addr, cnts, data) == _FAIL) {
			DBG_871X("%s: rtw_efuse_access error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

//		DBG_871X("%s: {VID,PID}={", __FUNCTION__);
		*extra = 0;
		for (i=0; i<cnts; i++) {
//			DBG_871X("0x%02x", data[i]);
			sprintf(extra + strlen(extra), "0x%02X", data[i]);
			if (i != (cnts-1)) {
//				DBG_871X(",");
				sprintf(extra + strlen(extra), ",");
			}
		}
//		DBG_871X("}\n");
	} else if (strcmp(tmp[0], "ableraw") == 0) {
		efuse_GetCurrentSize(padapter,&raw_cursize);
		raw_maxsize = efuse_GetMaxSize(padapter);
		sprintf(extra, "[available raw size]= %d bytes", raw_maxsize-raw_cursize);
	} else if (strcmp(tmp[0], "btfmap") == 0) {
		BTEfuse_PowerSwitch(padapter,1,_TRUE);

		mapLen = EFUSE_BT_MAX_MAP_LEN;
		if (rtw_BT_efuse_map_read(padapter, 0, mapLen, pEfuseHal->BTEfuseInitMap) == _FAIL) {
			DBG_871X("%s: rtw_BT_efuse_map_read Fail!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

//		DBG_871X("OFFSET\tVALUE(hex)\n");
		sprintf(extra, "\n");
		for (i=0; i<512; i+=16) { // set 512 because the iwpriv's extra size have limit 0x7FF
//			DBG_871X("0x%03x\t", i);
			sprintf(extra + strlen(extra), "0x%03x\t", i);
			for (j=0; j<8; j++) {
//				DBG_871X("%02X ", pEfuseHal->BTEfuseInitMap[i+j]);
				sprintf(extra + strlen(extra), "%02X ", pEfuseHal->BTEfuseInitMap[i+j]);
			}
//			DBG_871X("\t");
			sprintf(extra + strlen(extra), "\t");
			for (; j<16; j++) {
//				DBG_871X("%02X ", pEfuseHal->BTEfuseInitMap[i+j]);
				sprintf(extra + strlen(extra), "%02X ", pEfuseHal->BTEfuseInitMap[i+j]);
			}
//			DBG_871X("\n");
			sprintf(extra + strlen(extra), "\n");
		}
//		DBG_871X("\n");
	} else if (strcmp(tmp[0],"btbmap") == 0) {
		BTEfuse_PowerSwitch(padapter,1,_TRUE);

		mapLen = EFUSE_BT_MAX_MAP_LEN;
		if (rtw_BT_efuse_map_read(padapter, 0, mapLen, pEfuseHal->BTEfuseInitMap) == _FAIL) {
			DBG_871X("%s: rtw_BT_efuse_map_read Fail!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

//		DBG_871X("OFFSET\tVALUE(hex)\n");
		sprintf(extra, "\n");
		for (i=512; i<1024 ; i+=16) {
//			DBG_871X("0x%03x\t", i);
			sprintf(extra + strlen(extra), "0x%03x\t", i);
			for (j=0; j<8; j++) {
//				DBG_871X("%02X ", data[i+j]);
				sprintf(extra + strlen(extra), "%02X ", pEfuseHal->BTEfuseInitMap[i+j]);
			}
//			DBG_871X("\t");
			sprintf(extra + strlen(extra), "\t");
			for (; j<16; j++) {
//				DBG_871X("%02X ", data[i+j]);
				sprintf(extra + strlen(extra), "%02X ", pEfuseHal->BTEfuseInitMap[i+j]);
			}
//			DBG_871X("\n");
			sprintf(extra + strlen(extra), "\n");
		}
//		DBG_871X("\n");
	} else if (strcmp(tmp[0],"btrmap") == 0) {
		if ((tmp[1]==NULL) || (tmp[2]==NULL)) {
			err = -EINVAL;
			goto exit;
		}

		BTEfuse_PowerSwitch(padapter,1,_TRUE);

		// rmap addr cnts
		addr = simple_strtoul(tmp[1], &ptmp, 16);
		DBG_871X("%s: addr=0x%X\n", __FUNCTION__, addr);

		cnts = simple_strtoul(tmp[2], &ptmp, 10);
		if (cnts == 0) {
			DBG_871X("%s: btrmap Fail!! cnts error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);

		EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		if ((addr + cnts) > max_available_size) {
			DBG_871X("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_BT_efuse_map_read(padapter, addr, cnts, data) == _FAIL) {
			DBG_871X("%s: rtw_BT_efuse_map_read error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

		*extra = 0;
//		DBG_871X("%s: bt efuse data={", __FUNCTION__);
		for (i=0; i<cnts; i++) {
//			DBG_871X("0x%02x ", data[i]);
			sprintf(extra + strlen(extra), " 0x%02X ", data[i]);
		}
//		DBG_871X("}\n");
		DBG_871X(FUNC_ADPT_FMT ": BT MAC=[%s]\n", FUNC_ADPT_ARG(padapter), extra);
	} else if (strcmp(tmp[0], "btffake") == 0) {
//		DBG_871X("OFFSET\tVALUE(hex)\n");
		sprintf(extra, "\n");
		for (i=0; i<512; i+=16) {
//			DBG_871X("0x%03x\t", i);
			sprintf(extra + strlen(extra), "0x%03x\t", i);
			for (j=0; j<8; j++) {
//				DBG_871X("%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
				sprintf(extra + strlen(extra), "%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
			}
//			DBG_871X("\t");
			sprintf(extra + strlen(extra), "\t");
			for (; j<16; j++) {
//				DBG_871X("%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
				sprintf(extra + strlen(extra), "%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
			}
//			DBG_871X("\n");
			sprintf(extra + strlen(extra), "\n");
		}
//		DBG_871X("\n");
	} else if (strcmp(tmp[0],"btbfake") == 0) {
//		DBG_871X("OFFSET\tVALUE(hex)\n");
		sprintf(extra, "\n");
		for (i=512; i<1024; i+=16) {
//			DBG_871X("0x%03x\t", i);
			sprintf(extra + strlen(extra), "0x%03x\t", i);
			for (j=0; j<8; j++) {
//				DBG_871X("%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
				sprintf(extra + strlen(extra), "%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
			}
//			DBG_871X("\t");
			sprintf(extra + strlen(extra), "\t");
			for (; j<16; j++) {
//				DBG_871X("%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
				sprintf(extra + strlen(extra), "%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
			}
//			DBG_871X("\n");
			sprintf(extra + strlen(extra), "\n");
		}
//		DBG_871X("\n");
	} else if (strcmp(tmp[0],"wlrfkmap")== 0) {
//		DBG_871X("OFFSET\tVALUE(hex)\n");
		sprintf(extra, "\n");
		for (i=0; i<EFUSE_MAP_SIZE; i+=16) {
//			DBG_871X("\t0x%02x\t", i);
			sprintf(extra + strlen(extra), "0x%02x\t", i);
			for (j=0; j<8; j++) {
//				DBG_871X("%02X ", pEfuseHal->fakeEfuseModifiedMap[i+j]);
				sprintf(extra + strlen(extra), "%02X ", pEfuseHal->fakeEfuseModifiedMap[i+j]);
			}
//			DBG_871X("\t");
			sprintf(extra + strlen(extra), "\t");
			for (; j<16; j++) {
//				DBG_871X("%02X ", pEfuseHal->fakeEfuseModifiedMap[i+j]);
				sprintf(extra + strlen(extra), " %02X", pEfuseHal->fakeEfuseModifiedMap[i+j]);
			}
//			DBG_871X("\n");
			sprintf(extra + strlen(extra), "\n");
		}
//		DBG_871X("\n");

	} else if (strcmp(tmp[0],"wlrfkrmap")== 0) {
		if ((tmp[1]==NULL) || (tmp[2]==NULL)) {
			DBG_871X("%s: rmap Fail!! Parameters error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		// rmap addr cnts
		addr = simple_strtoul(tmp[1], &ptmp, 16);
		DBG_871X("%s: addr=%x\n", __FUNCTION__, addr);

		cnts = simple_strtoul(tmp[2], &ptmp, 10);
		if (cnts == 0) {
			DBG_871X("%s: rmap Fail!! cnts error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);

		//		DBG_871X("%s: data={", __FUNCTION__);
		*extra = 0;
		for (i=0; i<cnts; i++) {
			DBG_871X("wlrfkrmap = 0x%02x \n", pEfuseHal->fakeEfuseModifiedMap[addr+i]);
			sprintf(extra + strlen(extra), "0x%02X ", pEfuseHal->fakeEfuseModifiedMap[addr+i]);
		}
	} else if (strcmp(tmp[0],"btrfkrmap")== 0) {
		if ((tmp[1]==NULL) || (tmp[2]==NULL)) {
			DBG_871X("%s: rmap Fail!! Parameters error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		// rmap addr cnts
		addr = simple_strtoul(tmp[1], &ptmp, 16);
		DBG_871X("%s: addr=%x\n", __FUNCTION__, addr);

		cnts = simple_strtoul(tmp[2], &ptmp, 10);
		if (cnts == 0) {
			DBG_871X("%s: rmap Fail!! cnts error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);

		//		DBG_871X("%s: data={", __FUNCTION__);
		*extra = 0;
		for (i=0; i<cnts; i++) {
			DBG_871X("wlrfkrmap = 0x%02x \n", pEfuseHal->fakeBTEfuseModifiedMap[addr+i]);
			sprintf(extra + strlen(extra), "0x%02X ", pEfuseHal->fakeBTEfuseModifiedMap[addr+i]);
		}
	} else {
		sprintf(extra, "Command not found!");
	}

exit:
	if (data)
		rtw_mfree(data, EFUSE_BT_MAX_MAP_LEN);
	if (rawdata)
		rtw_mfree(rawdata, EFUSE_BT_MAX_MAP_LEN);
	if (!err)
		wrqu->length = strlen(extra);

	if (padapter->registrypriv.mp_mode == 0) {
#ifdef CONFIG_IPS
		rtw_pm_set_ips(padapter, ips_mode);
#endif // CONFIG_IPS

#ifdef CONFIG_LPS
		rtw_pm_set_lps(padapter, lps_mode);
#endif // CONFIG_LPS
	}

#ifdef CONFIG_IOL
	padapter->registrypriv.fw_iol = org_fw_iol;// 0:Disable, 1:enable, 2:by usb speed
#endif
	return err;
}

static int rtw_mp_efuse_set(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wdata, char *extra)
{
	struct iw_point *wrqu;
	PADAPTER padapter;
	struct pwrctrl_priv *pwrctrlpriv ;
	PHAL_DATA_TYPE pHalData;
	PEFUSE_HAL pEfuseHal;

	u8 ips_mode = IPS_NUM; // init invalid value
	u8 lps_mode = PS_MODE_NUM; // init invalid value
	u32 i=0,j=0, jj, kk;
	u8 *setdata = NULL;
	u8 *ShadowMapBT = NULL;
	u8 *ShadowMapWiFi = NULL;
	u8 *setrawdata = NULL;
	char *pch, *ptmp, *token, *tmp[3]= {0x00,0x00,0x00};
	u16 addr=0xFF, cnts=0, BTStatus=0 , max_available_size=0;
	int err;

	wrqu = (struct iw_point*)wdata;
	padapter = rtw_netdev_priv(dev);
	pwrctrlpriv = adapter_to_pwrctl(padapter);
	pHalData = GET_HAL_DATA(padapter);
	pEfuseHal = &pHalData->EfuseHal;
	err = 0;

	if (copy_from_user(extra, wrqu->pointer, wrqu->length))
		return -EFAULT;

	setdata = rtw_zmalloc(1024);
	if (setdata == NULL) {
		err = -ENOMEM;
		goto exit;
	}
	ShadowMapBT = rtw_malloc(EFUSE_BT_MAX_MAP_LEN);
	if (ShadowMapBT == NULL) {
		err = -ENOMEM;
		goto exit;
	}
	ShadowMapWiFi = rtw_malloc(EFUSE_MAP_SIZE);
	if (ShadowMapWiFi == NULL) {
		err = -ENOMEM;
		goto exit;
	}
	setrawdata = rtw_malloc(EFUSE_MAX_SIZE);
	if (setrawdata == NULL) {
		err = -ENOMEM;
		goto exit;
	}

#ifdef CONFIG_LPS
	lps_mode = pwrctrlpriv->power_mgnt;//keep org value
	rtw_pm_set_lps(padapter,PS_MODE_ACTIVE);
#endif

#ifdef CONFIG_IPS
	ips_mode = pwrctrlpriv->ips_mode;//keep org value
	rtw_pm_set_ips(padapter,IPS_NONE);
#endif

	pch = extra;
	DBG_871X("%s: in=%s\n", __FUNCTION__, extra);

	i = 0;
	while ((token = strsep(&pch, ",")) != NULL) {
		if (i > 2) break;
		tmp[i] = token;
		i++;
	}

	// tmp[0],[1],[2]
	// wmap,addr,00e04c871200
	if (strcmp(tmp[0], "wmap") == 0) {
		if ((tmp[1]==NULL) || (tmp[2]==NULL)) {
			err = -EINVAL;
			goto exit;
		}

#if 1
		// unknown bug workaround, need to fix later
		addr=0x1ff;
		rtw_write8(padapter, EFUSE_CTRL+1, (addr & 0xff));
		rtw_msleep_os(10);
		rtw_write8(padapter, EFUSE_CTRL+2, ((addr >> 8) & 0x03));
		rtw_msleep_os(10);
		rtw_write8(padapter, EFUSE_CTRL+3, 0x72);
		rtw_msleep_os(10);
		rtw_read8(padapter, EFUSE_CTRL);
#endif

		addr = simple_strtoul(tmp[1], &ptmp, 16);
		addr &= 0xFFF;

		cnts = strlen(tmp[2]);
		if (cnts%2) {
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0) {
			err = -EINVAL;
			goto exit;
		}

		DBG_871X("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_871X("%s: map data=%s\n", __FUNCTION__, tmp[2]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2) {
			setdata[jj] = key_2char2num(tmp[2][kk], tmp[2][kk+1]);
		}
#ifndef CONFIG_RTL8188E
		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
#else
		//Change to check TYPE_EFUSE_MAP_LEN ,beacuse 8188E raw 256,logic map over 256.
		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (PVOID)&max_available_size, _FALSE);
#endif
		if ((addr+cnts) > max_available_size) {
			DBG_871X("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL) {
			DBG_871X("%s: rtw_efuse_map_write error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
		*extra = 0;
		DBG_871X("%s: after rtw_efuse_map_write to _rtw_memcmp \n", __FUNCTION__);
		if ( (rtw_efuse_map_read(padapter, addr, cnts, ShadowMapWiFi) == _SUCCESS ) ) {
			if (_rtw_memcmp((void*)ShadowMapWiFi ,(void*)setdata,cnts)) {
				DBG_871X("%s: WiFi write map afterf compare success\n", __FUNCTION__);
				sprintf(extra, "WiFi write map compare OK\n");
				err = 0;
				goto exit;
			} else {
				sprintf(extra, "WiFi write map compare FAIL\n");
				DBG_871X("%s: WiFi write map compare Fail\n", __FUNCTION__);
				err = 0;
				goto exit;
			}
		}
	} else if (strcmp(tmp[0], "wraw") == 0) {
		if ((tmp[1]==NULL) || (tmp[2]==NULL)) {
			err = -EINVAL;
			goto exit;
		}

		addr = simple_strtoul( tmp[1], &ptmp, 16 );
		addr &= 0xFFF;

		cnts = strlen(tmp[2]);
		if (cnts%2) {
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0) {
			err = -EINVAL;
			goto exit;
		}

		DBG_871X("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_871X("%s: raw data=%s\n", __FUNCTION__, tmp[2]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2) {
			setrawdata[jj] = key_2char2num(tmp[2][kk], tmp[2][kk+1]);
		}

		if (rtw_efuse_access(padapter, _TRUE, addr, cnts, setrawdata) == _FAIL) {
			DBG_871X("%s: rtw_efuse_access error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
	} else if (strcmp(tmp[0], "mac") == 0) {
		if (tmp[1]==NULL) {
			err = -EINVAL;
			goto exit;
		}

		//mac,00e04c871200
#ifdef CONFIG_RTL8192C
		addr = EEPROM_MAC_ADDR_92C;
#endif
#ifdef CONFIG_RTL8192D
#ifdef CONFIG_USB_HCI
		if (pHalData->interfaceIndex == 0)
			addr = EEPROM_MAC_ADDR_MAC0_92DU;
		else
			addr = EEPROM_MAC_ADDR_MAC1_92DU;
#else
		if (pHalData->interfaceIndex == 0)
			addr = EEPROM_MAC_ADDR_MAC0_92DE;
		else
			addr = EEPROM_MAC_ADDR_MAC1_92DE;
#endif
#endif
#ifdef CONFIG_RTL8723A
#ifdef CONFIG_SDIO_HCI
		addr = EEPROM_MAC_ADDR_8723AS;
#endif
#ifdef CONFIG_GSPI_HCI
		addr = EEPROM_MAC_ADDR_8723AS;
#endif
#ifdef CONFIG_USB_HCI
		addr = EEPROM_MAC_ADDR_8723AU;
#endif
#endif // CONFIG_RTL8723A
#ifdef CONFIG_RTL8188E
#ifdef CONFIG_USB_HCI
		addr = EEPROM_MAC_ADDR_88EU;
#endif
#ifdef CONFIG_SDIO_HCI
		addr = EEPROM_MAC_ADDR_88ES;
#endif
#ifdef CONFIG_PCI_HCI
		addr = EEPROM_MAC_ADDR_88EE;
#endif
#endif //#ifdef CONFIG_RTL8188E

#ifdef CONFIG_RTL8192E
#ifdef CONFIG_USB_HCI
		addr = EEPROM_MAC_ADDR_8192EU;
#endif
#ifdef CONFIG_SDIO_HCI
		addr = EEPROM_MAC_ADDR_8192ES;
#endif
#ifdef CONFIG_PCI_HCI
		addr = EEPROM_MAC_ADDR_8192EE;
#endif
#endif //#ifdef CONFIG_RTL8192E

#ifdef CONFIG_RTL8723B
#ifdef CONFIG_SDIO_HCI
		addr = EEPROM_MAC_ADDR_8723BS;
#endif
#ifdef CONFIG_GSPI_HCI
		addr = EEPROM_MAC_ADDR_8723BS;
#endif
#ifdef CONFIG_USB_HCI
		addr = EEPROM_MAC_ADDR_8723BU;
#endif
#endif // CONFIG_RTL8723B

#if defined(CONFIG_RTL8812A) || defined(CONFIG_RTL8821A)
#ifdef CONFIG_SDIO_HCI
		addr = EEPROM_MAC_ADDR_8821AS;
#endif
#ifdef CONFIG_PCI_HCI
		addr = EEPROM_MAC_ADDR_8821AE;
#endif
#ifdef CONFIG_USB_HCI
		addr = EEPROM_MAC_ADDR_8821AU;
#endif

#endif // CONFIG_RTL8812A/CONFIG_RTL8821A

		cnts = strlen(tmp[1]);
		if (cnts%2) {
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0) {
			err = -EINVAL;
			goto exit;
		}
		if (cnts > 6) {
			DBG_871X("%s: error data for mac addr=\"%s\"\n", __FUNCTION__, tmp[1]);
			err = -EFAULT;
			goto exit;
		}

		DBG_871X("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_871X("%s: MAC address=%s\n", __FUNCTION__, tmp[1]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2) {
			setdata[jj] = key_2char2num(tmp[1][kk], tmp[1][kk+1]);
		}
#ifndef CONFIG_RTL8188E
		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
#else
		//Change to check TYPE_EFUSE_MAP_LEN ,beacuse 8188E raw 256,logic map over 256.
		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (PVOID)&max_available_size, _FALSE);
#endif
		if ((addr+cnts) > max_available_size) {
			DBG_871X("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL) {
			DBG_871X("%s: rtw_efuse_map_write error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
	} else if (strcmp(tmp[0], "vidpid") == 0) {
		if (tmp[1]==NULL) {
			err = -EINVAL;
			goto exit;
		}

		// pidvid,da0b7881
#ifdef CONFIG_RTL8192C
		addr = EEPROM_VID_92C;
#endif // CONFIG_RTL8192C
#ifdef CONFIG_RTL8192D
#ifdef CONFIG_USB_HCI
		addr = EEPROM_VID_92DU;
#else
		addr = EEPROM_VID_92DE;
#endif
#endif // CONFIG_RTL8192D
#ifdef CONFIG_RTL8723A
#ifdef CONFIG_USB_HCI
		addr = EEPROM_VID_8723AU;
#endif
#endif // CONFIG_RTL8723A
#ifdef CONFIG_RTL8188E
#ifdef CONFIG_USB_HCI
		addr = EEPROM_VID_88EU;
#endif
#ifdef CONFIG_PCI_HCI
		addr = EEPROM_VID_88EE;
#endif
#endif // CONFIG_RTL8188E

#ifdef CONFIG_RTL8192E
#ifdef CONFIG_USB_HCI
		addr = EEPROM_VID_8192EU;
#endif
#ifdef CONFIG_PCI_HCI
		addr = EEPROM_VID_8192EE;
#endif
#endif // CONFIG_RTL8188E

#ifdef CONFIG_RTL8723B
		addr = EEPROM_VID_8723BU;
#endif

		cnts = strlen(tmp[1]);
		if (cnts%2) {
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0) {
			err = -EINVAL;
			goto exit;
		}

		DBG_871X("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_871X("%s: VID/PID=%s\n", __FUNCTION__, tmp[1]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2) {
			setdata[jj] = key_2char2num(tmp[1][kk], tmp[1][kk+1]);
		}

		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		if ((addr+cnts) > max_available_size) {
			DBG_871X("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL) {
			DBG_871X("%s: rtw_efuse_map_write error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
	} else if (strcmp(tmp[0], "wldumpfake") == 0) {
		if (rtw_efuse_map_read(padapter, 0, EFUSE_MAP_SIZE,  pEfuseHal->fakeEfuseModifiedMap) == _SUCCESS) {
			DBG_871X("%s: WiFi hw efuse dump to Fake map success \n", __FUNCTION__);
		} else {
			DBG_871X("%s: WiFi hw efuse dump to Fake map Fail \n", __FUNCTION__);
			err = -EFAULT;
		}
	} else if (strcmp(tmp[0], "btwmap") == 0) {
		rtw_write8(padapter, 0xa3, 0x05); //For 8723AB ,8821S ?
		BTStatus=rtw_read8(padapter, 0xa0);
		DBG_871X("%s: btwmap before read 0xa0 BT Status =0x%x \n", __FUNCTION__,BTStatus);
		if (BTStatus != 0x04) {
			sprintf(extra, "BT Status not Active Write FAIL\n");
			goto exit;
		}

		if ((tmp[1]==NULL) || (tmp[2]==NULL)) {
			err = -EINVAL;
			goto exit;
		}
		BTEfuse_PowerSwitch(padapter,1,_TRUE);

		addr=0x1ff;
		rtw_write8(padapter, EFUSE_CTRL+1, (addr & 0xff));
		rtw_msleep_os(10);
		rtw_write8(padapter, EFUSE_CTRL+2, ((addr >> 8) & 0x03));
		rtw_msleep_os(10);
		rtw_write8(padapter, EFUSE_CTRL+3, 0x72);
		rtw_msleep_os(10);
		rtw_read8(padapter, EFUSE_CTRL);

		addr = simple_strtoul(tmp[1], &ptmp, 16);
		addr &= 0xFFF;

		cnts = strlen(tmp[2]);
		if (cnts%2) {
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0) {
			err = -EINVAL;
			goto exit;
		}

		DBG_871X("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_871X("%s: BT data=%s\n", __FUNCTION__, tmp[2]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2) {
			setdata[jj] = key_2char2num(tmp[2][kk], tmp[2][kk+1]);
		}

		EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		if ((addr+cnts) > max_available_size) {
			DBG_871X("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_BT_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL) {
			DBG_871X("%s: rtw_BT_efuse_map_write error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
		*extra = 0;
		DBG_871X("%s: after rtw_BT_efuse_map_write to _rtw_memcmp \n", __FUNCTION__);
		if ( (rtw_BT_efuse_map_read(padapter, addr, cnts, ShadowMapBT ) == _SUCCESS ) ) {
			if (_rtw_memcmp((void*)ShadowMapBT ,(void*)setdata,cnts)) {
				DBG_871X("%s: BT write map compare OK BTStatus=0x%x\n", __FUNCTION__,BTStatus);
				sprintf(extra, "BT write map compare OK");
				err = 0;
				goto exit;
			} else {
				sprintf(extra, "BT write map compare FAIL");
				DBG_871X("%s: BT write map compare FAIL BTStatus=0x%x\n", __FUNCTION__,BTStatus);
				err = 0;
				goto exit;
			}
		}
	} else if (strcmp(tmp[0], "btwfake") == 0) {
		if ((tmp[1]==NULL) || (tmp[2]==NULL)) {
			err = -EINVAL;
			goto exit;
		}

		addr = simple_strtoul(tmp[1], &ptmp, 16);
		addr &= 0xFFF;

		cnts = strlen(tmp[2]);
		if (cnts%2) {
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0) {
			err = -EINVAL;
			goto exit;
		}

		DBG_871X("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_871X("%s: BT tmp data=%s\n", __FUNCTION__, tmp[2]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2) {
			pEfuseHal->fakeBTEfuseModifiedMap[addr+jj] = key_2char2num(tmp[2][kk], tmp[2][kk+1]);
		}
	} else if (strcmp(tmp[0], "btdumpfake") == 0) {
		if (rtw_BT_efuse_map_read(padapter, 0, EFUSE_BT_MAX_MAP_LEN, pEfuseHal->fakeBTEfuseModifiedMap) == _SUCCESS) {
			DBG_871X("%s: BT read all map success\n", __FUNCTION__);
		} else {
			DBG_871X("%s: BT read all map Fail!\n", __FUNCTION__);
			err = -EFAULT;
		}
	} else if (strcmp(tmp[0], "btfk2map") == 0) {
		rtw_write8(padapter, 0xa3, 0x05);
		BTStatus=rtw_read8(padapter, 0xa0);
		DBG_871X("%s: btwmap before read 0xa0 BT Status =0x%x \n", __FUNCTION__,BTStatus);
		if (BTStatus != 0x04) {
			sprintf(extra, "BT Status not Active Write FAIL\n");
			goto exit;
		}

		BTEfuse_PowerSwitch(padapter,1,_TRUE);

		addr=0x1ff;
		rtw_write8(padapter, EFUSE_CTRL+1, (addr & 0xff));
		rtw_msleep_os(10);
		rtw_write8(padapter, EFUSE_CTRL+2, ((addr >> 8) & 0x03));
		rtw_msleep_os(10);
		rtw_write8(padapter, EFUSE_CTRL+3, 0x72);
		rtw_msleep_os(10);
		rtw_read8(padapter, EFUSE_CTRL);

		_rtw_memcpy(pEfuseHal->BTEfuseModifiedMap, pEfuseHal->fakeBTEfuseModifiedMap, EFUSE_BT_MAX_MAP_LEN);

		EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		if (max_available_size < 1) {
			err = -EFAULT;
			goto exit;
		}

		if (rtw_BT_efuse_map_write(padapter, 0x00, EFUSE_BT_MAX_MAP_LEN, pEfuseHal->fakeBTEfuseModifiedMap) == _FAIL) {
			DBG_871X("%s: rtw_BT_efuse_map_write error!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

		DBG_871X("pEfuseHal->fakeBTEfuseModifiedMap OFFSET\tVALUE(hex)\n");
		for (i = 0; i < EFUSE_BT_MAX_MAP_LEN; i += 16) {
			printk("0x%02x\t", i);
			for (j=0; j<8; j++) {
				printk("%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
			}
			printk("\t");

			for (; j<16; j++) {
				printk("%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
			}
			printk("\n");
		}
		printk("\n");
#if 1
		err = -EFAULT;
		DBG_871X("%s: rtw_BT_efuse_map_read _rtw_memcmp \n", __FUNCTION__);
		if ( (rtw_BT_efuse_map_read(padapter, 0x00, EFUSE_BT_MAX_MAP_LEN, pEfuseHal->fakeBTEfuseInitMap) == _SUCCESS ) ) {
			if (_rtw_memcmp((void*)pEfuseHal->fakeBTEfuseModifiedMap,(void*)pEfuseHal->fakeBTEfuseInitMap,EFUSE_BT_MAX_MAP_LEN)) {
				sprintf(extra, "BT write map compare OK");
				DBG_871X("%s: BT write map afterf compare success BTStatus=0x%x \n", __FUNCTION__,BTStatus);
				err = 0;
				goto exit;
			} else {
				sprintf(extra, "BT write map compare FAIL");
				if (rtw_BT_efuse_map_write(padapter, 0x00, EFUSE_BT_MAX_MAP_LEN, pEfuseHal->fakeBTEfuseModifiedMap) == _FAIL) {
					DBG_871X("%s: rtw_BT_efuse_map_write compare error,retry = %d!\n", __FUNCTION__,i);
				}

				if (rtw_BT_efuse_map_read(padapter, EFUSE_BT, EFUSE_BT_MAX_MAP_LEN, pEfuseHal->fakeBTEfuseInitMap) == _SUCCESS) {
					DBG_871X("pEfuseHal->fakeBTEfuseInitMap OFFSET\tVALUE(hex)\n");

					for (i = 0; i < EFUSE_BT_MAX_MAP_LEN; i += 16) {
						printk("0x%02x\t", i);
						for (j=0; j<8; j++) {
							printk("%02X ", pEfuseHal->fakeBTEfuseInitMap[i+j]);
						}
						printk("\t");
						for (; j<16; j++) {
							printk("%02X ", pEfuseHal->fakeBTEfuseInitMap[i+j]);
						}
						printk("\n");
					}
					printk("\n");
				}
				DBG_871X("%s: BT write map afterf compare not match to write efuse try write Map again , BTStatus=0x%x\n", __FUNCTION__,BTStatus);
				goto exit;
			}
		}
#endif

	} else if (strcmp(tmp[0], "wlfk2map") == 0) {
		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		if (max_available_size < 1) {
			err = -EFAULT;
			goto exit;
		}
		if (rtw_efuse_map_write(padapter, 0x00, EFUSE_MAP_SIZE, pEfuseHal->fakeEfuseModifiedMap) == _FAIL) {
			DBG_871X("%s: rtw_efuse_map_write fakeEfuseModifiedMap error!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
		*extra = 0;
		DBG_871X("%s: after rtw_BT_efuse_map_write to _rtw_memcmp \n", __FUNCTION__);
		if ( (rtw_efuse_map_read(padapter, 0x00, EFUSE_MAP_SIZE, ShadowMapWiFi) == _SUCCESS ) ) {
			if (_rtw_memcmp((void*)ShadowMapWiFi ,(void*)setdata,cnts)) {
				DBG_871X("%s: WiFi write map afterf compare OK\n", __FUNCTION__);
				sprintf(extra, "WiFi write map compare OK\n");
				err = 0;
				goto exit;
			} else {
				sprintf(extra, "WiFi write map compare FAIL\n");
				DBG_871X("%s: WiFi write map compare Fail\n", __FUNCTION__);
				err = 0;
				goto exit;
			}
		}
	} else if (strcmp(tmp[0], "wlwfake") == 0) {
		if ((tmp[1]==NULL) || (tmp[2]==NULL)) {
			err = -EINVAL;
			goto exit;
		}

		addr = simple_strtoul(tmp[1], &ptmp, 16);
		addr &= 0xFFF;

		cnts = strlen(tmp[2]);
		if (cnts%2) {
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0) {
			err = -EINVAL;
			goto exit;
		}

		DBG_871X("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_871X("%s: map tmp data=%s\n", __FUNCTION__, tmp[2]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2) {
			pEfuseHal->fakeEfuseModifiedMap[addr+jj] = key_2char2num(tmp[2][kk], tmp[2][kk+1]);
		}
	}

exit:
	if (setdata)
		rtw_mfree(setdata, 1024);
	if (ShadowMapBT)
		rtw_mfree(ShadowMapBT, EFUSE_BT_MAX_MAP_LEN);
	if (ShadowMapWiFi)
		rtw_mfree(ShadowMapWiFi, EFUSE_MAP_SIZE);
	if (setrawdata)
		rtw_mfree(setrawdata, EFUSE_MAX_SIZE);

	wrqu->length = strlen(extra);

	if (padapter->registrypriv.mp_mode == 0) {
#ifdef CONFIG_IPS
		rtw_pm_set_ips(padapter, ips_mode);
#endif // CONFIG_IPS

#ifdef CONFIG_LPS
		rtw_pm_set_lps(padapter, lps_mode);
#endif // CONFIG_LPS
	}

	return err;
}

#if defined(CONFIG_MP_INCLUDED) && defined(CONFIG_MP_IWPRIV_SUPPORT)
/*
 * Input Format: %s,%d,%d
 *	%s is width, could be
 *		"b" for 1 byte
 *		"w" for WORD (2 bytes)
 *		"dw" for DWORD (4 bytes)
 *	1st %d is address(offset)
 *	2st %d is data to write
 */
static int rtw_mp_write_reg(struct net_device *dev,
                            struct iw_request_info *info,
                            struct iw_point *wrqu, char *extra)
{
	char *pch, *pnext, *ptmp;
	char *width_str;
	char width;
	u32 addr, data;
	int ret = 0;
	PADAPTER padapter = rtw_netdev_priv(dev);
	char* input = kmalloc(wrqu->length, GFP_KERNEL);

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	_rtw_memset(extra, 0, wrqu->length);

	pch = input;

	pnext = strpbrk(pch, " ,.-");
	if (pnext == NULL) {
		ret = -EINVAL;
		goto out;
	}
	*pnext = 0;
	width_str = pch;

	pch = pnext + 1;
	pnext = strpbrk(pch, " ,.-");
	if (pnext == NULL) {
		ret = -EINVAL;
		goto out;
	}
	*pnext = 0;
	addr = simple_strtoul(pch, &ptmp, 16);
	if (addr > 0x3FFF) {
		ret = -EINVAL;
		goto out;
	}

	pch = pnext + 1;
	if ((pch - extra) >= wrqu->length) {
		ret = -EINVAL;
		goto out;
	}
	data = simple_strtoul(pch, &ptmp, 16);

	ret = 0;
	width = width_str[0];
	switch (width) {
	case 'b':
		// 1 byte
		if (data > 0xFF) {
			ret = -EINVAL;
			break;
		}
		rtw_write8(padapter, addr, data);
		break;
	case 'w':
		// 2 bytes
		if (data > 0xFFFF) {
			ret = -EINVAL;
			break;
		}
		rtw_write16(padapter, addr, data);
		break;
	case 'd':
		// 4 bytes
		rtw_write32(padapter, addr, data);
		break;
	default:
		ret = -EINVAL;
		break;
	}

out:
	kfree(input);
	return ret;
}

/*
 * Input Format: %s,%d
 *	%s is width, could be
 *		"b" for 1 byte
 *		"w" for WORD (2 bytes)
 *		"dw" for DWORD (4 bytes)
 *	%d is address(offset)
 *
 * Return:
 *	%d for data readed
 */
static int rtw_mp_read_reg(struct net_device *dev,
                           struct iw_request_info *info,
                           struct iw_point *wrqu, char *extra)
{
	char input[128];
	char *pch, *pnext, *ptmp;
	char *width_str;
	char width;
	char data[20],tmp[20];
	u32 addr;
	//u32 *data = (u32*)extra;
	u32 ret, i=0, j=0, strtout=0;
	PADAPTER padapter = rtw_netdev_priv(dev);


	if (wrqu->length > 128)
		return -EFAULT;

	if (copy_from_user(input, wrqu->pointer, wrqu->length))
		return -EFAULT;

	_rtw_memset(data, 0, 20);
	_rtw_memset(tmp, 0, 20);
	_rtw_memset(extra, 0, wrqu->length);

	pch = input;
	pnext = strpbrk(pch, " ,.-");
	if (pnext == NULL) return -EINVAL;
	*pnext = 0;
	width_str = pch;

	pch = pnext + 1;
	if ((pch - input) >= wrqu->length) return -EINVAL;

	addr = simple_strtoul(pch, &ptmp, 16);
	if (addr > 0x3FFF) return -EINVAL;

	ret = 0;
	width = width_str[0];
	switch (width) {
	case 'b':
		// 1 byte
		// *(u8*)data = rtw_read8(padapter, addr);
		sprintf(extra, "%d\n",  rtw_read8(padapter, addr));
		wrqu->length = strlen(extra);
		break;
	case 'w':
		// 2 bytes
		//*(u16*)data = rtw_read16(padapter, addr);
		sprintf(data, "%04x\n", rtw_read16(padapter, addr));
		for( i=0 ; i <= strlen(data) ; i++) {
			if( i%2==0 ) {
				tmp[j]=' ';
				j++;
			}
			if ( data[i] != '\0' )
				tmp[j] = data[i];

			j++;
		}
		pch = tmp;
		DBG_871X("pch=%s",pch);

		while( *pch != '\0' ) {
			pnext = strpbrk(pch, " ");
			if (!pnext)
				break;

			pnext++;
			if ( *pnext != '\0' ) {
				strtout = simple_strtoul (pnext , &ptmp, 16);
				sprintf(extra + strlen(extra), " %d"  ,strtout );
			} else {
				break;
			}
			pch = pnext;
		}
		wrqu->length = 7;
		break;
	case 'd':
		// 4 bytes
		//*data = rtw_read32(padapter, addr);
		sprintf(data, "%08x", rtw_read32(padapter, addr));
		//add read data format blank
		for( i=0 ; i <= strlen(data) ; i++) {
			if( i%2==0 ) {
				tmp[j]=' ';
				j++;
			}
			if ( data[i] != '\0' )
				tmp[j] = data[i];

			j++;
		}
		pch = tmp;
		DBG_871X("pch=%s",pch);

		while( *pch != '\0' ) {
			pnext = strpbrk(pch, " ");
			if (!pnext)
				break;

			pnext++;
			if ( *pnext != '\0' ) {
				strtout = simple_strtoul (pnext , &ptmp, 16);
				sprintf(extra + strlen(extra), " %d"  ,strtout );
			} else {
				break;
			}
			pch = pnext;
		}
		wrqu->length = strlen(extra);
		break;

	default:
		wrqu->length = 0;
		ret = -EINVAL;
		break;

	}

	return ret;
}

/*
 * Input Format: %d,%x,%x
 *	%d is RF path, should be smaller than MAX_RF_PATH_NUMS
 *	1st %x is address(offset)
 *	2st %x is data to write
 */
static int rtw_mp_write_rf(struct net_device *dev,
                           struct iw_request_info *info,
                           struct iw_point *wrqu, char *extra)
{
	/*static int rtw_mp_write_rf(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
	*/
	u32 path, addr, data;
	int ret = 0;
	PADAPTER padapter = rtw_netdev_priv(dev);
	char* input = kmalloc(wrqu->length, GFP_KERNEL);

	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}


	ret = sscanf(input, "%d,%x,%x", &path, &addr, &data);
	if (ret < 3) {
		ret = -EINVAL;
		goto out;
	}

	if ((path >= GET_HAL_RFPATH_NUM(padapter)) ||
	    (addr > 0xFF) ||
	    (data > 0xFFFFF)) {
		ret = -EINVAL;
		goto out;
	}

	_rtw_memset(extra, 0, wrqu->length);

	write_rfreg(padapter, path, addr, data);

	sprintf(extra, "write_rf completed \n");
	wrqu->length = strlen(extra);

out:
	kfree(input);
	return ret;
}

/*
 * Input Format: %d,%x
 *	%d is RF path, should be smaller than MAX_RF_PATH_NUMS
 *	%x is address(offset)
 *
 * Return:
 *	%d for data readed
 */
static int rtw_mp_read_rf(struct net_device *dev,
                          struct iw_request_info *info,
                          struct iw_point *wrqu, char *extra)
{
	char input[128];
	char *pch, *pnext, *ptmp;
	char data[20],tmp[20];
	//u32 *data = (u32*)extra;
	u32 path, addr;
	u32 ret,i=0 ,j=0,strtou=0;
	PADAPTER padapter = rtw_netdev_priv(dev);


	if (wrqu->length > 128) return -EFAULT;
	if (copy_from_user(input, wrqu->pointer, wrqu->length))
		return -EFAULT;

	ret = sscanf(input, "%d,%x", &path, &addr);
	if (ret < 2) return -EINVAL;

	if (path >= GET_HAL_RFPATH_NUM(padapter)) return -EINVAL;
	if (addr > 0xFF) return -EINVAL;

	_rtw_memset(extra, 0, wrqu->length);

	//*data = read_rfreg(padapter, path, addr);
	sprintf(data, "%08x", read_rfreg(padapter, path, addr));
	//add read data format blank
	for( i=0 ; i <= strlen(data) ; i++) {
		if( i%2==0 ) {
			tmp[j]=' ';
			j++;
		}
		tmp[j] = data[i];
		j++;
	}
	pch = tmp;
	DBG_871X("pch=%s",pch);

	while( *pch != '\0' ) {
		pnext = strpbrk(pch, " ");
		if (!pnext)
			break;
		pnext++;
		if ( *pnext != '\0' ) {
			strtou = simple_strtoul (pnext , &ptmp, 16);
			sprintf(extra + strlen(extra), " %d"  ,strtou );
		} else {
			break;
		}
		pch = pnext;
	}
	wrqu->length = strlen(extra);

	return 0;
}

static int rtw_mp_start(struct net_device *dev,
                        struct iw_request_info *info,
                        struct iw_point *wrqu, char *extra)
{
	//u8 val8;
	PADAPTER padapter = rtw_netdev_priv(dev);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
#ifdef CONFIG_BT_COEXIST
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
#endif
	struct hal_ops *pHalFunc = &padapter->HalFunc;

	rtw_pm_set_ips(padapter,IPS_NONE);
	LeaveAllPowerSaveMode(padapter);

	if(padapter->registrypriv.mp_mode ==0) {

#ifdef CONFIG_BT_COEXIST
		pdmpriv->DMFlag &= ~DYNAMIC_FUNC_BT;
#endif
		pHalFunc->hal_deinit(padapter);
		padapter->registrypriv.mp_mode =1;
		pHalFunc->hal_init(padapter);

		rtw_pm_set_ips(padapter,IPS_NONE);
		LeaveAllPowerSaveMode(padapter);
	}

	if (padapter->registrypriv.mp_mode == 0)
		return -EPERM;

	if (padapter->mppriv.mode == MP_OFF) {
		if (mp_start_test(padapter) == _FAIL)
			return -EPERM;
		padapter->mppriv.mode = MP_ON;
		MPT_PwrCtlDM(padapter,0);
	}
	padapter->mppriv.bmac_filter = _FALSE;
#ifdef CONFIG_RTL8723B
#ifdef CONFIG_USB_HCI
	rtw_write32(padapter, 0x765, 0x0000);
	rtw_write32(padapter, 0x948, 0x0280);
#else
	rtw_write32(padapter, 0x765, 0x0000);
	rtw_write32(padapter, 0x948, 0x0000);
#endif
#ifdef CONFIG_FOR_RTL8723BS_VQ0
	rtw_write32(padapter, 0x765, 0x0000);
	rtw_write32(padapter, 0x948, 0x0280);
#endif
	rtw_write8(padapter, 0x66, 0x27); //Open BT uart Log
	rtw_write8(padapter, 0xc50, 0x20); //for RX init Gain
#endif
	ODM_Write_DIG(&pHalData->odmpriv,0x20);

	return 0;
}

static int rtw_mp_stop(struct net_device *dev,
                       struct iw_request_info *info,
                       struct iw_point *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	struct hal_ops *pHalFunc = &padapter->HalFunc;

	if(padapter->registrypriv.mp_mode ==1) {

		MPT_DeInitAdapter(padapter);
		pHalFunc->hal_deinit(padapter);
		padapter->registrypriv.mp_mode=0;
		pHalFunc->hal_init(padapter);
	}

	if (padapter->mppriv.mode != MP_OFF) {
		mp_stop_test(padapter);
		padapter->mppriv.mode = MP_OFF;
	}

	return 0;
}

extern int wifirate2_ratetbl_inx(unsigned char rate);

static int rtw_mp_rate(struct net_device *dev,
                       struct iw_request_info *info,
                       struct iw_point *wrqu, char *extra)
{
	u32 rate = MPT_RATE_1M;
	u8* input = kmalloc(wrqu->length, GFP_KERNEL);
	PADAPTER padapter = rtw_netdev_priv(dev);
	int ret = 0;

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	rate = rtw_mpRateParseFunc(padapter,input);

	if (rate ==0 && strcmp(input,"1M")!=0) {
		rate = rtw_atoi(input);
		if(rate <= 0x7f)
			rate = wifirate2_ratetbl_inx( (u8)rate);
		else if (rate < 0xC8)
			rate =(rate - 0x80 + MPT_RATE_MCS0);
		//HT  rate 0x80(MCS0)  ~ 0x8F(MCS15) ~ 0x9F(MCS31) 128~159
		//VHT1SS~2SS rate 0xA0 (VHT1SS_MCS0 44) ~ 0xB3 (VHT2SS_MCS9 #63) 160~179
		//VHT rate 0xB4 (VHT3SS_MCS0 64) ~ 0xC7 (VHT2SS_MCS9 #83) 180~199
		//else
		//VHT rate 0x90(VHT1SS_MCS0) ~ 0x99(VHT1SS_MCS9) 144~153
		//	rate =(rate - MPT_RATE_VHT1SS_MCS0);
	}
	_rtw_memset(extra, 0, wrqu->length);

	sprintf( extra, "Set data rate to %s index %d" ,input,rate );
	DBG_871X("%s: %s rate index=%d \n", __func__,input,rate);

	if (rate >= MPT_RATE_LAST ) {
		ret = -EINVAL;
		goto out;
	}

	padapter->mppriv.rateidx = rate;
	Hal_SetDataRate(padapter);

	wrqu->length = strlen(extra);

out:
	kfree(input);
	return ret;
}

static int rtw_mp_channel(struct net_device *dev,
                          struct iw_request_info *info,
                          struct iw_point *wrqu, char *extra)
{

	PADAPTER padapter = rtw_netdev_priv(dev);
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(padapter);
	u8* input = kmalloc(wrqu->length, GFP_KERNEL);
	u32 	channel = 1;
	int ret = 0;
	//int cur_ch_offset;

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	channel = rtw_atoi(input);
	//DBG_871X("%s: channel=%d\n", __func__, channel);
	_rtw_memset(extra, 0, wrqu->length);
	sprintf( extra, "Change channel %d to channel %d", padapter->mppriv.channel , channel );
	padapter->mppriv.channel = channel;
	Hal_SetChannel(padapter);
	pHalData->CurrentChannel = channel;

	wrqu->length = strlen(extra);
out:
	kfree(input);
	return ret;
}

static int rtw_mp_bandwidth(struct net_device *dev,
                            struct iw_request_info *info,
                            struct iw_point *wrqu, char *extra)
{
	u32 bandwidth=0, sg=0;
	//int cur_ch_offset;
	//u8 buffer[40];
	PADAPTER padapter = rtw_netdev_priv(dev);
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(padapter);
	//if (copy_from_user(buffer, (void*)wrqu->data.pointer, wrqu->data.length))
	//            return -EFAULT;

	//DBG_871X("%s:iwpriv in=%s\n", __func__, extra);

	sscanf(extra, "40M=%d,shortGI=%d", &bandwidth, &sg);

	if (bandwidth == 1)
		bandwidth=CHANNEL_WIDTH_40;
	else if (bandwidth == 2)
		bandwidth=CHANNEL_WIDTH_80;
	DBG_871X("%s: bw=%d sg=%d \n", __func__, bandwidth , sg);

	padapter->mppriv.bandwidth = (u8)bandwidth;
	padapter->mppriv.preamble = sg;

	SetBandwidth(padapter);
	pHalData->CurrentChannelBW = bandwidth;
	//cur_ch_offset =  rtw_get_offset_by_ch(padapter->mppriv.channel);
	//set_channel_bwmode(padapter, padapter->mppriv.channel, cur_ch_offset, bandwidth);

	return 0;
}


static int rtw_mp_txpower_index(struct net_device *dev,
                                struct iw_request_info *info,
                                struct iw_point *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	char input[128];
	u32 rfpath;
	u32 txpower_inx;

	if (wrqu->length > 128)
		return -EFAULT;

	if (copy_from_user(input, wrqu->pointer, wrqu->length))
		return -EFAULT;

	rfpath = rtw_atoi(input);
	txpower_inx = mpt_ProQueryCalTxPower(padapter, rfpath);
	sprintf(extra, " %d", txpower_inx);
	wrqu->length = strlen(extra);

	return 0;
}


static int rtw_mp_txpower(struct net_device *dev,
                          struct iw_request_info *info,
                          struct iw_point *wrqu, char *extra)
{
	u32 idx_a = 0, idx_b = 0;
	int MsetPower = 1;
	u8* input = kmalloc(wrqu->length, GFP_KERNEL);
	int ret = 0;
	PADAPTER padapter = rtw_netdev_priv(dev);

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	MsetPower = strncmp(input, "off", 3);
	sscanf(input,"patha=%d,pathb=%d",&idx_a,&idx_b);
	//DBG_871X("%s: tx_pwr_idx_a=%x b=%x\n", __func__, idx_a, idx_b);
	if(MsetPower==0) {
		padapter->mppriv.bSetTxPower = 0;
		sprintf( extra, "MP Set power off");
	} else {
		sprintf( extra, "Set power level path_A:%d path_B:%d", idx_a , idx_b );
		padapter->mppriv.txpoweridx = (u8)idx_a;
		padapter->mppriv.txpoweridx_b = (u8)idx_b;
		padapter->mppriv.bSetTxPower = 1;
		Hal_SetAntennaPathPower(padapter);
	}
	wrqu->length = strlen(extra);

out:
	kfree(input);
	return 0;
}

static int rtw_mp_ant_tx(struct net_device *dev,
                         struct iw_request_info *info,
                         struct iw_point *wrqu, char *extra)
{
	u8 i;
	u8 *input = kmalloc(wrqu->length, GFP_KERNEL);
	u16 antenna = 0;
	PADAPTER padapter = rtw_netdev_priv(dev);
	int ret = 0;

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	//DBG_871X("%s: input=%s\n", __func__, input);

	sprintf( extra, "switch Tx antenna to %s", input);

	for (i=0; i < strlen(input); i++) {
		switch(input[i]) {
		case 'a' :
			antenna|=ANTENNA_A;
			break;
		case 'b':
			antenna|=ANTENNA_B;
			break;
		}
	}
	//antenna |= BIT(extra[i]-'a');
	//DBG_871X("%s: antenna=0x%x\n", __func__, antenna);
	padapter->mppriv.antenna_tx = antenna;
	padapter->mppriv.antenna_rx = antenna;
	//DBG_871X("%s:mppriv.antenna_rx=%d\n", __func__, padapter->mppriv.antenna_tx);

	Hal_SetAntenna(padapter);

	wrqu->length = strlen(extra);
out:
	kfree(input);
	return ret;
}

static int rtw_mp_ant_rx(struct net_device *dev,
                         struct iw_request_info *info,
                         struct iw_point *wrqu, char *extra)
{
	u8 i;
	u16 antenna = 0;
	u8 *input = kmalloc(wrqu->length, GFP_KERNEL);
	PADAPTER padapter = rtw_netdev_priv(dev);
	int ret = 0;

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	//DBG_871X("%s: input=%s\n", __func__, input);
	_rtw_memset(extra, 0, wrqu->length);

	sprintf( extra, "switch Rx antenna to %s", input );

	for (i=0; i < strlen(input); i++) {
		switch( input[i] ) {
		case 'a' :
			antenna|=ANTENNA_A;
			break;
		case 'b':
			antenna|=ANTENNA_B;
			break;
		case 'c' :
			antenna|=ANTENNA_C;
			break;
		}
	}

	//DBG_871X("%s: antenna=0x%x\n", __func__, antenna);
	padapter->mppriv.antenna_tx = antenna;
	padapter->mppriv.antenna_rx = antenna;
	//DBG_871X("%s:mppriv.antenna_rx=%d\n", __func__, padapter->mppriv.antenna_rx);
	Hal_SetAntenna(padapter);
	wrqu->length = strlen(extra);

out:
	kfree(input);
	return ret;
}

static int rtw_set_ctx_destAddr(struct net_device *dev,
                                struct iw_request_info *info,
                                struct iw_point *wrqu, char *extra)
{
	int jj,kk=0;

	struct pkt_attrib *pattrib;
	struct mp_priv *pmp_priv;
	PADAPTER padapter = rtw_netdev_priv(dev);
	pmp_priv = &padapter->mppriv;
	pattrib = &pmp_priv->tx.attrib;

	if( strlen(extra) < 5 )
		return _FAIL;

	DBG_871X("%s: in=%s\n", __func__, extra);
	for(jj = 0,kk = 0; jj < ETH_ALEN; jj++, kk += 3 ) {
		pattrib->dst[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	DBG_871X("pattrib->dst:%x %x %x %x %x %x\n",pattrib->dst[0],pattrib->dst[1],pattrib->dst[2],pattrib->dst[3],pattrib->dst[4],pattrib->dst[5]);
	return 0;
}

static int rtw_mp_ctx(struct net_device *dev,
                      struct iw_request_info *info,
                      struct iw_point *wrqu, char *extra)
{
	u32 pkTx = 1;
	int countPkTx = 1, cotuTx = 1, CarrSprTx = 1, scTx = 1, sgleTx = 1, stop = 1;
	u32 bStartTest = 1;
	u32 count = 0,pktinterval=0,pktlen=0;
	struct mp_priv *pmp_priv;
	struct pkt_attrib *pattrib;
	PADAPTER padapter = rtw_netdev_priv(dev);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	pmp_priv = &padapter->mppriv;
	pattrib = &pmp_priv->tx.attrib;

	if (copy_from_user(extra, wrqu->pointer, wrqu->length))
		return -EFAULT;

	DBG_871X("%s: in=%s\n", __func__, extra);

	countPkTx = strncmp(extra, "count=", 5); // strncmp TRUE is 0
	cotuTx = strncmp(extra, "background", 20);
	CarrSprTx = strncmp(extra, "background,cs", 20);
	scTx = strncmp(extra, "background,sc", 20);
	sgleTx = strncmp(extra, "background,stone", 20);
	pkTx = strncmp(extra, "background,pkt", 20);
	stop = strncmp(extra, "stop", 4);
	sscanf(extra, "count=%d,pkt", &count);
	sscanf(extra, "pktinterval=%d", &pktinterval);
	sscanf(extra, "pktlen=%d", &pktlen);

	if ( _rtw_memcmp( extra, "destmac=", 8 ) ) {
		wrqu->length -= 8;
		rtw_set_ctx_destAddr (dev,info,wrqu,&extra[8]);
		sprintf( extra, "Set dest mac OK ! \n");
		return 0;
	}

	//DBG_871X("%s: count=%d countPkTx=%d cotuTx=%d CarrSprTx=%d scTx=%d sgleTx=%d pkTx=%d stop=%d\n", __func__, count, countPkTx, cotuTx, CarrSprTx, pkTx, sgleTx, scTx, stop);
	_rtw_memset(extra, '\0', strlen(extra));

	if( pktinterval !=0 ) {
		sprintf( extra, "Pkt Interval = %d",pktinterval);
		padapter->mppriv.pktInterval = pktinterval;
		wrqu->length = strlen(extra);
		return 0;
	}
	if( pktlen !=0 ) {
		sprintf( extra, "Pkt len = %d",pktlen);
		pattrib->pktlen = pktlen ;
		wrqu->length = strlen(extra);
		return 0;
	}
	if (stop == 0) {
		bStartTest = 0; // To set Stop
		pmp_priv->tx.stop = 1;
		sprintf( extra, "Stop continuous Tx");
	} else {
		bStartTest = 1;
		if (pmp_priv->mode != MP_ON) {
			if (pmp_priv->tx.stop != 1) {
				DBG_871X("%s: MP_MODE != ON %d\n", __func__, pmp_priv->mode);
				return  -EFAULT;
			}
		}
	}

	if (pkTx == 0 || countPkTx == 0)
		pmp_priv->mode = MP_PACKET_TX;
	if (sgleTx == 0)
		pmp_priv->mode = MP_SINGLE_TONE_TX;
	if (cotuTx == 0)
		pmp_priv->mode = MP_CONTINUOUS_TX;
	if (CarrSprTx == 0)
		pmp_priv->mode = MP_CARRIER_SUPPRISSION_TX;
	if (scTx == 0)
		pmp_priv->mode = MP_SINGLE_CARRIER_TX;

	switch (pmp_priv->mode) {
	case MP_PACKET_TX:

		//DBG_871X("%s:pkTx %d\n", __func__,bStartTest);
		if (bStartTest == 0) {
			pmp_priv->tx.stop = 1;
			pmp_priv->mode = MP_ON;
			sprintf( extra, "Stop continuous Tx");
		} else if (pmp_priv->tx.stop == 1) {
			sprintf( extra, "Start continuous DA=ffffffffffff len=1500 count=%u\n",count);
			//DBG_871X("%s:countPkTx %d\n", __func__,count);
			pmp_priv->tx.stop = 0;
			pmp_priv->tx.count = count;
#ifdef CONFIG_80211AC_VHT
			if (pHalData->rf_type == RF_1T1R)
				pmp_priv->tx.attrib.raid = RATEID_IDX_VHT_1SS;
			else
				pmp_priv->tx.attrib.raid = RATEID_IDX_VHT_2SS;
#endif
			SetPacketTx(padapter);
		} else {
			//DBG_871X("%s: pkTx not stop\n", __func__);
			return -EFAULT;
		}
		wrqu->length = strlen(extra);
		return 0;

	case MP_SINGLE_TONE_TX:
		//DBG_871X("%s: sgleTx %d \n", __func__, bStartTest);
		if (bStartTest != 0) {
			sprintf( extra, "Start continuous DA=ffffffffffff len=1500 \n infinite=yes.");
		}
		Hal_SetSingleToneTx(padapter, (u8)bStartTest);
		break;

	case MP_CONTINUOUS_TX:
		//DBG_871X("%s: cotuTx %d\n", __func__, bStartTest);
		if (bStartTest != 0) {
			sprintf( extra, "Start continuous DA=ffffffffffff len=1500 \n infinite=yes.");
		}
		Hal_SetContinuousTx(padapter, (u8)bStartTest);
		break;

	case MP_CARRIER_SUPPRISSION_TX:
		//DBG_871X("%s: CarrSprTx %d\n", __func__, bStartTest);
		if (bStartTest != 0) {
			if( pmp_priv->rateidx <= MPT_RATE_11M ) {
				sprintf( extra, "Start continuous DA=ffffffffffff len=1500 \n infinite=yes.");

			} else
				sprintf( extra, "Specify carrier suppression but not CCK rate");
		}
		Hal_SetCarrierSuppressionTx(padapter, (u8)bStartTest);
		break;

	case MP_SINGLE_CARRIER_TX:
		//DBG_871X("%s: scTx %d\n", __func__, bStartTest);
		if (bStartTest != 0) {
			sprintf( extra, "Start continuous DA=ffffffffffff len=1500 \n infinite=yes.");
		}
		Hal_SetSingleCarrierTx(padapter, (u8)bStartTest);
		break;

	default:
		//DBG_871X("%s:No Match MP_MODE\n", __func__);
		sprintf( extra, "Error! Continuous-Tx is not on-going.");
		return -EFAULT;
	}

	if ( bStartTest==1 && pmp_priv->mode != MP_ON) {
		struct mp_priv *pmp_priv = &padapter->mppriv;
		if (pmp_priv->tx.stop == 0) {
			pmp_priv->tx.stop = 1;
			//DBG_871X("%s: pkt tx is running...\n", __func__);
			rtw_msleep_os(5);
		}
#ifdef CONFIG_80211N_HT
		pmp_priv->tx.attrib.ht_en = 1;
#endif
#ifdef CONFIG_80211AC_VHT
		if (pHalData->rf_type == RF_1T1R)
			pmp_priv->tx.attrib.raid = RATEID_IDX_VHT_1SS;
		else
			pmp_priv->tx.attrib.raid = RATEID_IDX_VHT_2SS;
#endif
		pmp_priv->tx.stop = 0;
		pmp_priv->tx.count = 1;
		SetPacketTx(padapter);
	} else {
		pmp_priv->mode = MP_ON;
	}

	wrqu->length = strlen(extra);
	return 0;
}


static int rtw_mp_disable_bt_coexist(struct net_device *dev,
                                     struct iw_request_info *info,
                                     union iwreq_data *wrqu, char *extra)
{
#ifdef CONFIG_BT_COEXIST
	PADAPTER padapter = (PADAPTER)rtw_netdev_priv(dev);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	//struct hal_ops *pHalFunc = &padapter->HalFunc;
#endif

	u8 *input = kmalloc(wrqu->data.length, GFP_KERNEL);
	u32 bt_coexist;
	int ret = 0;

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->data.pointer, wrqu->data.length)) {
		ret = -EFAULT;
		goto out;
	}

	bt_coexist = rtw_atoi(input);

	if( bt_coexist == 0 ) {
		RT_TRACE(_module_mp_, _drv_info_,
		         ("Set OID_RT_SET_DISABLE_BT_COEXIST: disable BT_COEXIST\n"));
		DBG_871X("Set OID_RT_SET_DISABLE_BT_COEXIST: disable BT_COEXIST\n");
#ifdef CONFIG_BT_COEXIST
		rtw_btcoex_HaltNotify(padapter);
		rtw_btcoex_SetManualControl(padapter, _TRUE);
		pdmpriv->DMFlag &= ~DYNAMIC_FUNC_BT;
		// Force to switch Antenna to WiFi
		rtw_write16(padapter, 0x870, 0x300);
		rtw_write16(padapter, 0x860, 0x110);
#endif // CONFIG_BT_COEXIST
	} else {
		RT_TRACE(_module_mp_, _drv_info_,
		         ("Set OID_RT_SET_DISABLE_BT_COEXIST: enable BT_COEXIST\n"));
#ifdef CONFIG_BT_COEXIST
		pdmpriv->DMFlag |= DYNAMIC_FUNC_BT;
		rtw_btcoex_SetManualControl(padapter, _FALSE);
#endif
	}

out:
	kfree(input);
	return ret;
}


static int rtw_mp_arx(struct net_device *dev,
                      struct iw_request_info *info,
                      struct iw_point *wrqu, char *extra)
{
	int bStartRx = 0, bStopRx = 0, bQueryPhy = 0, bQueryMac = 0, bSetBssid = 0;
	int bmac_filter = 0, bmon = 0;
	u8 *input = kmalloc(wrqu->length, GFP_KERNEL);
	char *pch, *token, *tmp[2]= {0x00,0x00};
	u32 i = 0, jj = 0, kk = 0, cnts = 0;
	PADAPTER padapter = rtw_netdev_priv(dev);
	struct mp_priv *pmppriv = &padapter->mppriv;
	struct dbg_rx_counter rx_counter;
	int ret = 0;

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	DBG_871X("%s: %s\n", __func__, input);

	bStartRx = (strncmp(input, "start", 5)==0)?1:0; // strncmp TRUE is 0
	bStopRx = (strncmp(input, "stop", 5)==0)?1:0; // strncmp TRUE is 0
	bQueryPhy = (strncmp(input, "phy", 3)==0)?1:0; // strncmp TRUE is 0
	bQueryMac = (strncmp(input, "mac", 3)==0)?1:0; // strncmp TRUE is 0
	bSetBssid = (strncmp(input, "setbssid=", 8)==0)?1:0; // strncmp TRUE is 0
	//bfilter_init = (strncmp(input, "filter_init",11)==0)?1:0;
	bmac_filter = (strncmp(input, "accept_mac",10)==0)?1:0;
	bmon = (strncmp(input, "mon=",4)==0)?1:0;

	if(bSetBssid==1) {
		pch = input;
		while ((token = strsep(&pch, "=")) != NULL) {
			if (i > 1) break;
			tmp[i] = token;
			i++;
		}
		if ((tmp[0]==NULL) && (tmp[1]==NULL)) {
			ret = -EFAULT;
			goto out;
		} else {
			cnts = strlen(tmp[1])/2;
			if (cnts<1) {
				ret = -EFAULT;
				goto out;
			}
			DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);
			DBG_871X("%s: data=%s\n", __FUNCTION__, tmp[1]);
			for (jj=0, kk=0; jj < cnts ; jj++, kk+=2) {
				pmppriv->network_macaddr[jj] = key_2char2num(tmp[1][kk], tmp[1][kk+1]);
				DBG_871X("network_macaddr[%d]=%x \n",jj, pmppriv->network_macaddr[jj]);
			}
		}
		pmppriv->bSetRxBssid = _TRUE;
	}

	if(bmac_filter) {
		pmppriv->bmac_filter = bmac_filter;
		pch = input;
		while ((token = strsep(&pch, "=")) != NULL) {
			if (i > 1) break;
			tmp[i] = token;
			i++;
		}
		if ((tmp[0]==NULL) && (tmp[1]==NULL)) {
			ret = -EFAULT;
			goto out;
		} else {
			cnts = strlen(tmp[1])/2;
			if (cnts<1) {
				ret = -EFAULT;
				goto out;
			}
			DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);
			DBG_871X("%s: data=%s\n", __FUNCTION__, tmp[1]);
			for (jj=0, kk=0; jj < cnts ; jj++, kk+=2) {
				pmppriv->mac_filter[jj] = key_2char2num(tmp[1][kk], tmp[1][kk+1]);
				DBG_871X("%s mac_filter[%d]=%x \n",__FUNCTION__,jj, pmppriv->mac_filter[jj]);
			}
		}
	}

	if(bStartRx) {
		sprintf( extra, "start");
		SetPacketRx(padapter, bStartRx);
	} else if(bStopRx) {
		SetPacketRx(padapter, 0);
		pmppriv->bmac_filter = _FALSE;
		sprintf( extra, "Received packet OK:%d CRC error:%d ,Filter out:%d",padapter->mppriv.rx_pktcount,padapter->mppriv.rx_crcerrpktcount,padapter->mppriv.rx_pktcount_filter_out);
	} else if(bQueryPhy) {
		_rtw_memset(&rx_counter,0,sizeof(struct dbg_rx_counter));
		rtw_dump_phy_rx_counters(padapter,&rx_counter);

		DBG_871X("%s: OFDM_FA =%d\n", __FUNCTION__, rx_counter.rx_ofdm_fa);
		DBG_871X("%s: CCK_FA =%d\n", __FUNCTION__, rx_counter.rx_cck_fa);
		sprintf( extra, "Phy Received packet OK:%d CRC error:%d FA Counter: %d",rx_counter.rx_pkt_ok,rx_counter.rx_pkt_crc_error,rx_counter.rx_cck_fa+rx_counter.rx_ofdm_fa);


	} else if(bQueryMac) {
		_rtw_memset(&rx_counter,0,sizeof(struct dbg_rx_counter));
		rtw_dump_mac_rx_counters(padapter,&rx_counter);
		sprintf( extra, "Mac Received packet OK: %d , CRC error: %d , Drop Packets: %d\n",
		         rx_counter.rx_pkt_ok,rx_counter.rx_pkt_crc_error,rx_counter.rx_pkt_drop);

	}

	if( bmon==1 ) {
		sscanf(input, "mon=%d", &bmon);

		if(bmon==1) {
			pmppriv->rx_bindicatePkt= _TRUE;
			sprintf( extra, "Indicating Receive Packet to network start\n");
		} else {
			pmppriv->rx_bindicatePkt= _FALSE;
			sprintf( extra, "Indicating Receive Packet to network Stop\n");
		}
	}

	wrqu->length = strlen(extra) + 1;

out:
	kfree(input);
	return ret;
}

static int rtw_mp_trx_query(struct net_device *dev,
                            struct iw_request_info *info,
                            struct iw_point *wrqu, char *extra)
{
	u32 txok,txfail,rxok,rxfail,rxfilterout;
	PADAPTER padapter = rtw_netdev_priv(dev);
	//if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
	//	return -EFAULT;

	txok=padapter->mppriv.tx.sended;
	txfail=0;
	rxok = padapter->mppriv.rx_pktcount;
	rxfail = padapter->mppriv.rx_crcerrpktcount;
	rxfilterout = padapter->mppriv.rx_pktcount_filter_out;

	_rtw_memset(extra, '\0', 128);

	sprintf(extra, "Tx OK:%d, Tx Fail:%d, Rx OK:%d, CRC error:%d ,Rx Filter out:%d \n", txok, txfail,rxok,rxfail,rxfilterout);

	wrqu->length=strlen(extra)+1;

	return 0;
}

static int rtw_mp_pwrtrk(struct net_device *dev,
                         struct iw_request_info *info,
                         struct iw_point *wrqu, char *extra)
{
	u8 enable;
	u32 thermal;
	s32 ret;
	PADAPTER padapter = rtw_netdev_priv(dev);
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);
	u8 *input = kmalloc(wrqu->length, GFP_KERNEL);

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	_rtw_memset(extra, 0, wrqu->length);

	enable = 1;
	if (wrqu->length > 1) { // not empty string
		if (strncmp(input, "stop", 4) == 0) {
			enable = 0;
			sprintf(extra, "mp tx power tracking stop");
			pHalData->TxPowerTrackControl = _FALSE;
		} else if (sscanf(input, "ther=%d", &thermal)) {
			pHalData->TxPowerTrackControl = _TRUE;
			ret = Hal_SetThermalMeter(padapter, (u8)thermal);
			if (ret == _FAIL) return -EPERM;
			sprintf(extra, "mp tx power tracking start,target value=%d ok ",thermal);
		} else {
			return -EINVAL;
		}
	}

	ret = Hal_SetPowerTracking(padapter, enable);
	if (ret == _FAIL) {
		ret = -EPERM;
		goto out;
	}

	wrqu->length = strlen(extra);
	ret = 0;

out:
	kfree(input);
	return ret;
}

static int rtw_mp_psd(struct net_device *dev,
                      struct iw_request_info *info,
                      struct iw_point *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	u8 *input = kmalloc(wrqu->length, GFP_KERNEL);
	int ret = 0;

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	strcpy(extra,input);

	wrqu->length = mp_query_psd(padapter, extra);

out:
	kfree(input);
	return ret;
}

static int rtw_mp_thermal(struct net_device *dev,
                          struct iw_request_info *info,
                          struct iw_point *wrqu, char *extra)
{
	u8 val;
	int bwrite=1;

#ifdef CONFIG_RTL8192C
	u16 addr=EEPROM_THERMAL_METER_92C;
#endif
#ifdef CONFIG_RTL8192D
	u16 addr=EEPROM_THERMAL_METER_92D;
#endif
#ifdef CONFIG_RTL8723A
	u16 addr=EEPROM_THERMAL_METER_8723A;
#endif
#ifdef CONFIG_RTL8188E
	u16 addr=EEPROM_THERMAL_METER_88E;
#endif
#if defined(CONFIG_RTL8812A) || defined(CONFIG_RTL8821A)
	u16 addr=EEPROM_THERMAL_METER_8812;
#endif
#ifdef CONFIG_RTL8192E
	u16 addr=EEPROM_THERMAL_METER_8192E;
#endif
#ifdef CONFIG_RTL8723B
	u16 addr=EEPROM_THERMAL_METER_8723B;
#endif
	u16 cnt=1;
	u16 max_available_size=0;
	PADAPTER padapter = rtw_netdev_priv(dev);

	if (copy_from_user(extra, wrqu->pointer, wrqu->length))
		return -EFAULT;

	//DBG_871X("print extra %s \n",extra);

	bwrite = strncmp(extra, "write", 6); // strncmp TRUE is 0

	Hal_GetThermalMeter(padapter, &val);

	if( bwrite == 0 ) {
		//DBG_871X("to write val:%d",val);
		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		if( 2 > max_available_size ) {
			DBG_871X("no available efuse!\n");
			return -EFAULT;
		}
		if ( rtw_efuse_map_write(padapter, addr, cnt, &val) == _FAIL ) {
			DBG_871X("rtw_efuse_map_write error \n");
			return -EFAULT;
		} else {
			sprintf(extra, " efuse write ok :%d", val);
		}
	} else {
		sprintf(extra, "%d", val);
	}
	wrqu->length = strlen(extra);

	return 0;
}

static int rtw_mp_reset_stats(struct net_device *dev,
                              struct iw_request_info *info,
                              struct iw_point *wrqu, char *extra)
{
	struct mp_priv *pmp_priv;
	//struct pkt_attrib *pattrib;
	PADAPTER padapter = rtw_netdev_priv(dev);

	pmp_priv = &padapter->mppriv;

	pmp_priv->tx.sended = 0;
	pmp_priv->tx_pktcount = 0;
	pmp_priv->rx_pktcount = 0;
	pmp_priv->rx_pktcount_filter_out=0;
	pmp_priv->rx_crcerrpktcount = 0;

	rtw_reset_phy_rx_counters(padapter);
	rtw_reset_mac_rx_counters(padapter);

	return 0;
}

static int rtw_mp_dump(struct net_device *dev,
                       struct iw_request_info *info,
                       struct iw_point *wrqu, char *extra)
{
	struct mp_priv *pmp_priv;
	//struct pkt_attrib *pattrib;
	//u32 value;
	u8 *input = kmalloc(wrqu->length, GFP_KERNEL);
	//u8 rf_type,path_nums = 0;
	//u32 i,j=1,path;
	PADAPTER padapter = rtw_netdev_priv(dev);
	int ret = 0;

	if (!input)
		return -ENOMEM;
	
	pmp_priv = &padapter->mppriv;

	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	if ( strncmp(input, "all", 4)==0 ) {
		mac_reg_dump(RTW_DBGDUMP, padapter);
		bb_reg_dump(RTW_DBGDUMP, padapter);
		rf_reg_dump(RTW_DBGDUMP, padapter);
	}

out:
	kfree(input);
	return ret;
}

static int rtw_mp_phypara(struct net_device *dev,
                          struct iw_request_info *info,
                          struct iw_point *wrqu, char *extra)
{

	PADAPTER padapter = rtw_netdev_priv(dev);
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(padapter);
	char *input = kmalloc(wrqu->length, GFP_KERNEL);
	u32		valxcap;
	int ret = 0;

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	DBG_871X("%s:iwpriv in=%s\n", __func__, input);

	sscanf(input, "xcap=%d", &valxcap);

	pHalData->CrystalCap = (u8)valxcap;
	Hal_ProSetCrystalCap( padapter , valxcap );

	sprintf( extra, "Set xcap=%d",valxcap );
	wrqu->length = strlen(extra) + 1;

out:
	kfree(input);
	return ret;

}

static int rtw_mp_SetRFPath(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	char* input = kmalloc(wrqu->data.length, GFP_KERNEL);
	int		bMain = 1, bTurnoff = 1;
	int ret = 0;

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->data.pointer, wrqu->data.length)) {
		ret = -EFAULT;
		goto out;
	}
	DBG_871X("%s:iwpriv in=%s\n", __func__, input);

	bMain = strncmp(input, "1", 2); // strncmp TRUE is 0
	bTurnoff = strncmp(input, "0", 3); // strncmp TRUE is 0

	if(bMain==0) {
		MP_PHY_SetRFPathSwitch(padapter,_TRUE);
		DBG_871X("%s:PHY_SetRFPathSwitch=TRUE\n", __func__);
	} else if(bTurnoff==0) {
		MP_PHY_SetRFPathSwitch(padapter,_FALSE);
		DBG_871X("%s:PHY_SetRFPathSwitch=FALSE\n", __func__);
	}

out:
	kfree(input);
	return ret;
}

static int rtw_mp_QueryDrv(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	char* input = kmalloc(wrqu->data.length, GFP_KERNEL);
	int	qAutoLoad = 1;
	int ret = 0;

	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

	if (!input)
		return -ENOMEM;

	if (copy_from_user(input, wrqu->data.pointer, wrqu->data.length)) {
		ret = -EFAULT;
		goto out;
	}
	DBG_871X("%s:iwpriv in=%s\n", __func__, input);

	qAutoLoad = strncmp(input, "autoload", 8); // strncmp TRUE is 0

	if(qAutoLoad==0) {
		DBG_871X("%s:qAutoLoad\n", __func__);

		if(pEEPROM->bautoload_fail_flag)
			sprintf(extra, "fail");
		else
			sprintf(extra, "ok");
	}
	wrqu->data.length = strlen(extra) + 1;

out:
	kfree(input);
	return ret;
}

/* update Tx AGC offset */
static inline int rtw_mp_antBdiff(struct net_device *dev,
                           struct iw_request_info *info,
                           struct iw_point *wrqu, char *extra)
{


	// MPT_ProSetTxAGCOffset
	return 0;
}


static int rtw_mp_PwrCtlDM(struct net_device *dev,
                           struct iw_request_info *info,
                           struct iw_point *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	u8 *input = kmalloc(wrqu->length, GFP_KERNEL);
	int		bstart=1;
	int ret = 0;

	if (!input)
		return -ENOMEM;
	
	if (copy_from_user(input, wrqu->pointer, wrqu->length)) {
		ret = -EFAULT;
		goto out;
	}

	bstart = strncmp(input, "start", 5); // strncmp TRUE is 0
	if(bstart==0) {
		sprintf(extra, "PwrCtlDM start \n");
		MPT_PwrCtlDM(padapter,1);
	} else {
		sprintf(extra, "PwrCtlDM stop \n");
		MPT_PwrCtlDM(padapter,0);
	}
	wrqu->length = strlen(extra);

out:
	kfree(input);
	return ret;
}

static int rtw_mp_getver(struct net_device *dev,
                         struct iw_request_info *info,
                         union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	struct mp_priv *pmp_priv;

	pmp_priv = &padapter->mppriv;

	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	sprintf(extra, "rtwpriv=%d\n",RTWPRIV_VER_INFO);
	wrqu->data.length = strlen(extra);
	return 0;
}

static int rtw_mp_mon(struct net_device *dev,
                      struct iw_request_info *info,
                      union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	struct mp_priv *pmp_priv = &padapter->mppriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct hal_ops *pHalFunc = &padapter->HalFunc;
	NDIS_802_11_NETWORK_INFRASTRUCTURE networkType ;
	int bstart = 1, bstop = 1;
	networkType=Ndis802_11Infrastructure;
	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	rtw_pm_set_ips(padapter,IPS_NONE);
	LeaveAllPowerSaveMode(padapter);

#ifdef CONFIG_MP_INCLUDED
	if (init_mp_priv(padapter) == _FAIL) {
		DBG_871X("%s: initialize MP private data Fail!\n", __func__);
	}
	padapter->mppriv.channel = 6;

	bstart = strncmp(extra, "start", 5); // strncmp TRUE is 0
	bstop = strncmp(extra, "stop", 4); // strncmp TRUE is 0
	if(bstart==0) {
		mp_join(padapter,WIFI_FW_ADHOC_STATE);
		SetPacketRx(padapter, _TRUE);
		Hal_SetChannel(padapter);
		pmp_priv->rx_bindicatePkt= _TRUE;
		pmp_priv->bRTWSmbCfg = _TRUE;
		sprintf(extra, "monitor mode start\n");
	} else if (bstop==0) {
		SetPacketRx(padapter, _FALSE);
		pmp_priv->rx_bindicatePkt= _FALSE;
		pmp_priv->bRTWSmbCfg = _FALSE;
		padapter->registrypriv.mp_mode=1;
		pHalFunc->hal_deinit(padapter);
		padapter->registrypriv.mp_mode=0;
		pHalFunc->hal_init(padapter);
		//rtw_disassoc_cmd(padapter, 0, _TRUE);
		if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
			rtw_disassoc_cmd(padapter, 500, _TRUE);
			rtw_indicate_disconnect(padapter);
			//rtw_free_assoc_resources(padapter, 1);
		}
		rtw_pm_set_ips(padapter,IPS_NORMAL);
		sprintf(extra, "monitor mode Stop\n");
	}
#endif
	wrqu->data.length = strlen(extra);
	return 0;
}

static int rtw_efuse_mask_file(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	char *rtw_efuse_mask_file_path;
	u8 Status;
	PADAPTER padapter = rtw_netdev_priv(dev);
	_rtw_memset(maskfileBuffer,0x00,sizeof(maskfileBuffer));

	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	if(strncmp(extra, "off",3) ==0 && strlen(extra)<4) {
		padapter->registrypriv.boffefusemask=1;
		sprintf(extra, "Turn off Efuse Mask\n");
		wrqu->data.length = strlen(extra);
		return 0;
	}
	if(strncmp(extra, "on",2) ==0 && strlen(extra)<3) {
		padapter->registrypriv.boffefusemask=0;
		sprintf(extra, "Turn on Efuse Mask\n");
		wrqu->data.length = strlen(extra);
		return 0;
	}
	rtw_efuse_mask_file_path=extra;

	if (rtw_is_file_readable(rtw_efuse_mask_file_path) == _TRUE) {
		DBG_871X("%s do rtw_efuse_mask_file_read = %s! ,sizeof maskfileBuffer %zu\n",__FUNCTION__,rtw_efuse_mask_file_path,sizeof(maskfileBuffer));
		Status=rtw_efuse_file_read(padapter,rtw_efuse_mask_file_path,maskfileBuffer,sizeof(maskfileBuffer));
		if(Status==_TRUE)
			padapter->registrypriv.bFileMaskEfuse = _TRUE;
		sprintf(extra, "efuse mask file read OK\n");
	} else {
		padapter->registrypriv.bFileMaskEfuse = _FALSE;
		sprintf(extra, "efuse mask file readable FAIL\n");
		DBG_871X("%s rtw_is_file_readable fail !\n",__FUNCTION__);
	}
	wrqu->data.length = strlen(extra);
	return 0;
}


static int rtw_efuse_file_map(struct net_device *dev,
                              struct iw_request_info *info,
                              union iwreq_data *wrqu, char *extra)
{
	char *rtw_efuse_file_map_path;
	u8 Status;
	PEFUSE_HAL pEfuseHal;
	PADAPTER padapter = rtw_netdev_priv(dev);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	pEfuseHal = &pHalData->EfuseHal;
	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	rtw_efuse_file_map_path=extra;

	if (rtw_is_file_readable(rtw_efuse_file_map_path) == _TRUE) {
		DBG_871X("%s do rtw_efuse_mask_file_read = %s! \n",__FUNCTION__,rtw_efuse_file_map_path);
		Status=rtw_efuse_file_read(padapter,rtw_efuse_file_map_path,pEfuseHal->fakeEfuseModifiedMap,sizeof(pEfuseHal->fakeEfuseModifiedMap));
		if(Status==_TRUE)
			sprintf(extra, "efuse file file_read OK\n");
		else
			sprintf(extra, "efuse file file_read FAIL\n");
	} else {
		sprintf(extra, "efuse file readable FAIL\n");
		DBG_871X("%s rtw_is_file_readable fail !\n",__FUNCTION__);
	}
	wrqu->data.length = strlen(extra);
	return 0;
}


#if (defined(CONFIG_RTL8723A) || defined(CONFIG_RTL8723B))
/* update Tx AGC offset */
static int rtw_mp_SetBT(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	struct hal_ops *pHalFunc = &padapter->HalFunc;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	BT_REQ_CMD	BtReq;
	PMPT_CONTEXT	pMptCtx=&(padapter->mppriv.MptCtx);
	PBT_RSP_CMD 	pBtRsp=(PBT_RSP_CMD)&pMptCtx->mptOutBuf[0];
	char	input[128];
	char *pch, *ptmp, *token, *tmp[2]= {0x00,0x00};
	u8 setdata[100];
	u8 resetbt=0x00;
	u8 tempval,BTStatus;
	u8 H2cSetbtmac[6];
	u8 u1H2CBtMpOperParm[4]= {0x01};
	int testmode = 1, ready = 1, trxparam = 1, setgen = 1, getgen = 1, testctrl = 1, testbt = 1, readtherm = 1, setbtmac = 1;
	u32 i = 0, ii = 0, jj = 0, kk = 0, cnts = 0, status = 0;
	PRT_MP_FIRMWARE	pBTFirmware = NULL;

	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;
	if(strlen(extra)<1) return -EFAULT;

	DBG_871X("%s:iwpriv in=%s\n", __FUNCTION__, extra);
	ready = strncmp(extra, "ready", 5);
	testmode = strncmp(extra, "testmode", 8); // strncmp TRUE is 0
	trxparam = strncmp(extra, "trxparam", 8);
	setgen = strncmp(extra, "setgen", 6);
	getgen = strncmp(extra, "getgen", 6);
	testctrl = strncmp(extra, "testctrl", 8);
	testbt = strncmp(extra, "testbt", 6);
	readtherm = strncmp(extra, "readtherm", 9);
	setbtmac = strncmp(extra, "setbtmac", 8);

	if ( strncmp(extra, "dlbt", 4) == 0) {
		pHalData->LastHMEBoxNum=0;
		padapter->bBTFWReady = _FALSE;
		rtw_write8(padapter, 0xa3, 0x05);
		BTStatus=rtw_read8(padapter, 0xa0);
		DBG_871X("%s: btwmap before read 0xa0 BT Status =0x%x \n", __FUNCTION__,BTStatus);
		if (BTStatus != 0x04) {
			sprintf(extra, "BT Status not Active DLFW FAIL\n");
			goto exit;
		}

		tempval = rtw_read8(padapter, 0x6B);
		tempval |= BIT7;
		rtw_write8(padapter, 0x6B, tempval);

		// Attention!! Between 0x6A[14] and 0x6A[15] setting need 100us delay
		// So don't wirte 0x6A[14]=1 and 0x6A[15]=0 together!
		rtw_usleep_os(100);
		// disable BT power cut
		// 0x6A[14] = 0
		tempval = rtw_read8(padapter, 0x6B);
		tempval &= ~BIT6;
		rtw_write8(padapter, 0x6B, tempval);
		rtw_usleep_os(100);
		MPT_PwrCtlDM(padapter,0);
		rtw_write32(padapter, 0xcc, (rtw_read32(padapter, 0xcc)| 0x00000004));
		rtw_write32(padapter, 0x6b, (rtw_read32(padapter, 0x6b)& 0xFFFFFFEF));
		rtw_msleep_os(600);
		rtw_write32(padapter, 0x6b, (rtw_read32(padapter, 0x6b)| 0x00000010));
		rtw_write32(padapter, 0xcc, (rtw_read32(padapter, 0xcc)& 0xFFFFFFFB));
		rtw_msleep_os(1200);
		pBTFirmware = (PRT_MP_FIRMWARE)rtw_zmalloc(sizeof(RT_MP_FIRMWARE));
		if(pBTFirmware==NULL)
			goto exit;
		padapter->bBTFWReady = _FALSE;
		FirmwareDownloadBT(padapter, pBTFirmware);
		if (pBTFirmware)
			rtw_mfree((u8*)pBTFirmware, sizeof(RT_MP_FIRMWARE));

		DBG_871X("Wait for FirmwareDownloadBT fw boot!\n");
		rtw_msleep_os(2000);
		_rtw_memset(extra,'\0', wrqu->data.length);
		BtReq.opCodeVer = 1;
		BtReq.OpCode = 0;
		BtReq.paraLength = 0;
		mptbt_BtControlProcess(padapter, &BtReq);
		rtw_msleep_os(100);

		DBG_8192C("FirmwareDownloadBT ready = 0x%x 0x%x", pMptCtx->mptOutBuf[4],pMptCtx->mptOutBuf[5]);
		if ((pMptCtx->mptOutBuf[4] == 0x00) && (pMptCtx->mptOutBuf[5] == 0x00)) {

			if (padapter->mppriv.bTxBufCkFail == _TRUE)
				sprintf(extra, "check TxBuf Fail.\n");
			else
				sprintf(extra, "download FW Fail.\n");
		} else {
			sprintf(extra, "download FW OK.\n");
			goto exit;
		}
		goto exit;
	}
	if ( strncmp(extra, "dlfw", 4) == 0) {
		pHalData->LastHMEBoxNum=0;
		padapter->bBTFWReady = _FALSE;
		rtw_write8(padapter, 0xa3, 0x05);
		BTStatus=rtw_read8(padapter, 0xa0);
		DBG_871X("%s: btwmap before read 0xa0 BT Status =0x%x \n", __FUNCTION__,BTStatus);
		if (BTStatus != 0x04) {
			sprintf(extra, "BT Status not Active DLFW FAIL\n");
			goto exit;
		}

		tempval = rtw_read8(padapter, 0x6B);
		tempval |= BIT7;
		rtw_write8(padapter, 0x6B, tempval);

		// Attention!! Between 0x6A[14] and 0x6A[15] setting need 100us delay
		// So don't wirte 0x6A[14]=1 and 0x6A[15]=0 together!
		rtw_usleep_os(100);
		// disable BT power cut
		// 0x6A[14] = 0
		tempval = rtw_read8(padapter, 0x6B);
		tempval &= ~BIT6;
		rtw_write8(padapter, 0x6B, tempval);
		rtw_usleep_os(100);

		MPT_PwrCtlDM(padapter,0);
		rtw_write32(padapter, 0xcc, (rtw_read32(padapter, 0xcc)| 0x00000004));
		rtw_write32(padapter, 0x6b, (rtw_read32(padapter, 0x6b)& 0xFFFFFFEF));
		rtw_msleep_os(600);
		rtw_write32(padapter, 0x6b, (rtw_read32(padapter, 0x6b)| 0x00000010));
		rtw_write32(padapter, 0xcc, (rtw_read32(padapter, 0xcc)& 0xFFFFFFFB));
		rtw_msleep_os(1200);

#if defined(CONFIG_PLATFORM_SPRD) && (MP_DRIVER == 1)
		// Pull up BT reset pin.
		DBG_871X("%s: pull up BT reset pin when bt start mp test\n", __FUNCTION__);
		rtw_wifi_gpio_wlan_ctrl(WLAN_BT_PWDN_ON);
#endif
		DBG_871X(" rtl8723a_FirmwareDownload!\n");

#ifdef CONFIG_RTL8723A
		status = rtl8723a_FirmwareDownload(padapter);
#elif defined(CONFIG_RTL8723B)
		status = rtl8723b_FirmwareDownload(padapter, _FALSE);
#endif
		DBG_871X("Wait for FirmwareDownloadBT fw boot!\n");
		rtw_msleep_os(1000);
#ifdef CONFIG_BT_COEXIST
		rtw_btcoex_HaltNotify(padapter);
		DBG_871X("SetBT btcoex HaltNotify !\n");
		//hal_btcoex1ant_SetAntPath(padapter);
		rtw_btcoex_SetManualControl(padapter, _TRUE);
#endif
		_rtw_memset(extra,'\0', wrqu->data.length);
		BtReq.opCodeVer = 1;
		BtReq.OpCode = 0;
		BtReq.paraLength = 0;
		mptbt_BtControlProcess(padapter, &BtReq);
		rtw_msleep_os(200);

		DBG_8192C("FirmwareDownloadBT ready = 0x%x 0x%x", pMptCtx->mptOutBuf[4],pMptCtx->mptOutBuf[5]);
		if( (pMptCtx->mptOutBuf[4]==0x00) && (pMptCtx->mptOutBuf[5]==0x00)) {
			if(padapter->mppriv.bTxBufCkFail==_TRUE)
				sprintf(extra, "check TxBuf Fail.\n");
			else
				sprintf(extra, "download FW Fail.\n");
		} else {
#ifdef CONFIG_BT_COEXIST
			rtw_btcoex_SwitchBtTRxMask(padapter);
#endif
			rtw_msleep_os(200);
			sprintf(extra, "download FW OK.\n");
			goto exit;
		}
		goto exit;
	}

	if ( strncmp(extra, "down", 4) == 0) {
		DBG_871X("SetBT down for to hal_init !\n");
#ifdef CONFIG_BT_COEXIST
		rtw_btcoex_SetManualControl(padapter, _FALSE);
		rtw_btcoex_Initialize(padapter);
#endif
		pHalFunc->read_adapter_info(padapter);
		pHalFunc->hal_deinit(padapter);
		pHalFunc->hal_init(padapter);
		rtw_pm_set_ips(padapter,IPS_NONE);
		LeaveAllPowerSaveMode(padapter);
		MPT_PwrCtlDM(padapter,0);
		rtw_write32(padapter, 0xcc, (rtw_read32(padapter, 0xcc)| 0x00000004));
		rtw_write32(padapter, 0x6b, (rtw_read32(padapter, 0x6b)& 0xFFFFFFEF));
		rtw_msleep_os(600);
		//rtw_write32(padapter, 0x6a, (rtw_read32(padapter, 0x6a)& 0xFFFFFFFE));
		rtw_write32(padapter, 0x6b, (rtw_read32(padapter, 0x6b)| 0x00000010));
		rtw_write32(padapter, 0xcc, (rtw_read32(padapter, 0xcc)& 0xFFFFFFFB));
		rtw_msleep_os(1200);
		goto exit;
	}
	if ( strncmp(extra, "disable", 7) == 0) {
		DBG_871X("SetBT disable !\n");
		rtw_write32(padapter, 0x6a, (rtw_read32(padapter, 0x6a)& 0xFFFFFFFB));
		rtw_msleep_os(500);
		goto exit;
	}
	if ( strncmp(extra, "enable", 6) == 0) {
		DBG_871X("SetBT enable !\n");
		rtw_write32(padapter, 0x6a, (rtw_read32(padapter, 0x6a)| 0x00000004));
		rtw_msleep_os(500);
		goto exit;
	}
	if ( strncmp(extra, "h2c", 3) == 0) {
		DBG_871X("SetBT h2c !\n");
		padapter->bBTFWReady = _TRUE;
		rtw_hal_fill_h2c_cmd(padapter, 0x63, 1, u1H2CBtMpOperParm);
		goto exit;
	}
	if ( strncmp(extra, "2ant", 4) == 0) {
		DBG_871X("Set BT 2ant use!\n");
		PHY_SetMacReg(padapter,0x67,BIT5,0x1);
		rtw_write32(padapter, 0x948, 0000);

		goto exit;
	}

	if( ready!=0 && testmode!=0 && trxparam!=0 && setgen!=0 && getgen!=0 && testctrl!=0 && testbt!=0 && readtherm!=0 &&setbtmac!=0)
		return -EFAULT;

	if( testbt==0 ) {
		BtReq.opCodeVer=1;
		BtReq.OpCode=6;
		BtReq.paraLength=cnts/2;
		goto todo;
	}
	if( ready==0 ) {
		BtReq.opCodeVer=1;
		BtReq.OpCode=0;
		BtReq.paraLength=0;
		goto todo;
	}

	pch = extra;
	i = 0;
	while ((token = strsep(&pch, ",")) != NULL) {
		if (i > 1) break;
		tmp[i] = token;
		i++;
	}

	if ((tmp[0]==NULL) && (tmp[1]==NULL)) {
		return -EFAULT;
	} else {
		cnts = strlen(tmp[1]);
		if (cnts<1) return -EFAULT;

		DBG_871X("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_871X("%s: data=%s\n", __FUNCTION__, tmp[1]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2) {
			BtReq.pParamStart[jj] = key_2char2num(tmp[1][kk], tmp[1][kk+1]);
//			DBG_871X("BtReq.pParamStart[%d]=0x%02x\n", jj, BtReq.pParamStart[jj]);
		}
	}

	if( testmode==0 ) {
		BtReq.opCodeVer=1;
		BtReq.OpCode=1;
		BtReq.paraLength=1;
	}
	if( trxparam==0 ) {
		BtReq.opCodeVer=1;
		BtReq.OpCode=2;
		BtReq.paraLength=cnts/2;
	}
	if( setgen==0 ) {
		DBG_871X("%s: BT_SET_GENERAL \n", __func__);
		BtReq.opCodeVer=1;
		BtReq.OpCode=3; //BT_SET_GENERAL	3
		BtReq.paraLength=cnts/2;
	}
	if( getgen==0 ) {
		DBG_871X("%s: BT_GET_GENERAL \n", __func__);
		BtReq.opCodeVer=1;
		BtReq.OpCode=4;		//BT_GET_GENERAL	4
		BtReq.paraLength=cnts/2;
	}
	if( readtherm==0 ) {
		DBG_871X("%s: BT_GET_GENERAL \n", __func__);
		BtReq.opCodeVer=1;
		BtReq.OpCode=4;		//BT_GET_GENERAL	4
		BtReq.paraLength=cnts/2;
	}

	if( testctrl==0 ) {
		DBG_871X("%s: BT_TEST_CTRL \n", __func__);
		BtReq.opCodeVer=1;
		BtReq.OpCode=5;		//BT_TEST_CTRL	5
		BtReq.paraLength=cnts/2;
	}

	DBG_871X("%s: Req opCodeVer=%d OpCode=%d paraLength=%d\n",
	         __FUNCTION__, BtReq.opCodeVer, BtReq.OpCode, BtReq.paraLength);

	if(BtReq.paraLength<1)
		goto todo;
	for (i=0; i<BtReq.paraLength; i++) {
		DBG_871X("%s: BtReq.pParamStart[%d] = 0x%02x \n",
		         __FUNCTION__, i, BtReq.pParamStart[i]);
	}

todo:
	_rtw_memset(extra,'\0', wrqu->data.length);

	if (padapter->bBTFWReady == _FALSE) {
		sprintf(extra, "BTFWReady = FALSE.\n");
		goto exit;
	}

	mptbt_BtControlProcess(padapter, &BtReq);

	if (readtherm == 0) {
		sprintf(extra, "BT thermal=");
		for (i=4; i<pMptCtx->mptOutLen; i++) {
			if ((pMptCtx->mptOutBuf[i]==0x00) && (pMptCtx->mptOutBuf[i+1]==0x00))
				goto exit;

#ifdef CONFIG_RTL8723A
			sprintf(extra + strlen(extra), " %d ", (pMptCtx->mptOutBuf[i]& 0x3f));
#else
			sprintf(extra + strlen(extra), " %d ", (pMptCtx->mptOutBuf[i]& 0x1f));
#endif
		}
	} else {
		for (i=4; i<pMptCtx->mptOutLen; i++) {
			sprintf(extra + strlen(extra), " 0x%x ", pMptCtx->mptOutBuf[i]);
		}
	}

exit:
	wrqu->data.length = strlen(extra) + 1;
	DBG_871X("-%s: output len=%d data=%s\n", __FUNCTION__, wrqu->data.length, extra);

	return status;
}

#endif //#ifdef CONFIG_RTL8723A

static int rtw_mp_set(struct net_device *dev,
                      struct iw_request_info *info,
                      union iwreq_data *wdata, char *extra)
{
	struct iw_point *wrqu = (struct iw_point *)wdata;
	u32 subcmd = wrqu->flags;
	PADAPTER padapter = rtw_netdev_priv(dev);

	if (padapter == NULL) {
		return -ENETDOWN;
	}

	if((padapter->bup == _FALSE )) {
		DBG_871X(" %s fail =>(padapter->bup == _FALSE )\n",__FUNCTION__);
		return -ENETDOWN;
	}

	if( (padapter->bSurpriseRemoved == _TRUE) || ( padapter->bDriverStopped == _TRUE)) {
		DBG_871X("%s fail =>(padapter->bSurpriseRemoved == _TRUE) || ( padapter->bDriverStopped == _TRUE) \n",__FUNCTION__);
		return -ENETDOWN;
	}


	//_rtw_memset(extra, 0x00, IW_PRIV_SIZE_MASK);

	if (extra == NULL) {
		wrqu->length = 0;
		return -EIO;
	}

	switch(subcmd) {
	case MP_START:
		DBG_871X("set case mp_start \n");
		rtw_mp_start (dev,info,wrqu,extra);
		break;

	case MP_STOP:
		DBG_871X("set case mp_stop \n");
		rtw_mp_stop (dev,info,wrqu,extra);
		break;

	case MP_BANDWIDTH:
		DBG_871X("set case mp_bandwidth \n");
		rtw_mp_bandwidth (dev,info,wrqu,extra);
		break;

	case MP_RESET_STATS:
		DBG_871X("set case MP_RESET_STATS \n");
		rtw_mp_reset_stats	(dev,info,wrqu,extra);
		break;
	case MP_SetRFPathSwh:
		DBG_871X("set MP_SetRFPathSwitch \n");
		rtw_mp_SetRFPath  (dev,info,wdata,extra);
		break;
	case CTA_TEST:
		DBG_871X("set CTA_TEST\n");
		rtw_cta_test_start (dev, info, wdata, extra);
		break;
	case MP_DISABLE_BT_COEXIST:
		DBG_871X("set case MP_DISABLE_BT_COEXIST \n");
		rtw_mp_disable_bt_coexist(dev, info, wdata, extra);
		break;
#ifdef CONFIG_WOWLAN
	case MP_WOW_ENABLE:
		DBG_871X("set case MP_WOW_ENABLE: %s \n", extra);
		rtw_wowlan_ctrl(dev, info, wdata, extra);
		break;
#endif
#ifdef CONFIG_AP_WOWLAN
	case MP_AP_WOW_ENABLE:
		DBG_871X("set case MP_AP_WOW_ENABLE: %s \n", extra);
		rtw_ap_wowlan_ctrl(dev, info, wdata, extra);
		break;
#endif
	}


	return 0;
}


static int rtw_mp_get(struct net_device *dev,
                      struct iw_request_info *info,
                      union iwreq_data *wdata, char *extra)
{
	struct iw_point *wrqu = (struct iw_point *)wdata;
	u32 subcmd = wrqu->flags;
	PADAPTER padapter = rtw_netdev_priv(dev);

	//DBG_871X("in mp_get extra= %s \n",extra);

	if (padapter == NULL) {
		return -ENETDOWN;
	}
	if((padapter->bup == _FALSE )) {
		DBG_871X(" %s fail =>(padapter->bup == _FALSE )\n",__FUNCTION__);
		return -ENETDOWN;
	}

	if( (padapter->bSurpriseRemoved == _TRUE) || ( padapter->bDriverStopped == _TRUE)) {
		DBG_871X("%s fail =>(padapter->bSurpriseRemoved == _TRUE) || ( padapter->bDriverStopped == _TRUE) \n",__FUNCTION__);
		return -ENETDOWN;
	}

	if (extra == NULL) {
		wrqu->length = 0;
		return -EIO;
	}

	switch(subcmd) {
	case WRITE_REG :
		rtw_mp_write_reg (dev,info,wrqu,extra);
		break;

	case WRITE_RF:
		rtw_mp_write_rf (dev,info,wrqu,extra);
		break;

	case MP_PHYPARA:
		DBG_871X("mp_get  MP_PHYPARA \n");
		rtw_mp_phypara(dev,info,wrqu,extra);
		break;

	case MP_CHANNEL:
		DBG_871X("set case mp_channel \n");
		rtw_mp_channel (dev,info,wrqu,extra);
		break;

	case READ_REG:
		DBG_871X("mp_get  READ_REG \n");
		rtw_mp_read_reg (dev,info,wrqu,extra);
		break;
	case READ_RF:
		DBG_871X("mp_get  READ_RF \n");
		rtw_mp_read_rf (dev,info,wrqu,extra);
		break;

	case MP_RATE:
		DBG_871X("set case mp_rate \n");
		rtw_mp_rate (dev,info,wrqu,extra);
		break;

	case MP_TXPOWER:
		DBG_871X("set case MP_TXPOWER \n");
		rtw_mp_txpower (dev,info,wrqu,extra);
		break;

	case MP_ANT_TX:
		DBG_871X("set case MP_ANT_TX \n");
		rtw_mp_ant_tx (dev,info,wrqu,extra);
		break;

	case MP_ANT_RX:
		DBG_871X("set case MP_ANT_RX \n");
		rtw_mp_ant_rx (dev,info,wrqu,extra);
		break;

	case MP_QUERY:
		//DBG_871X("mp_get mp_query MP_QUERY \n");
		rtw_mp_trx_query(dev,info,wrqu,extra);
		break;

	case MP_CTX:
		DBG_871X("set case MP_CTX \n");
		rtw_mp_ctx (dev,info,wrqu,extra);
		break;

	case MP_ARX:
		DBG_871X("set case MP_ARX \n");
		rtw_mp_arx (dev,info,wrqu,extra);
		break;

	case EFUSE_GET:
		DBG_871X("efuse get EFUSE_GET \n");
		rtw_mp_efuse_get(dev,info,wdata,extra);
		break;

	case MP_DUMP:
		DBG_871X("set case MP_DUMP \n");
		rtw_mp_dump (dev,info,wrqu,extra);
		break;
	case MP_PSD:
		DBG_871X("set case MP_PSD \n");
		rtw_mp_psd (dev,info,wrqu,extra);
		break;
	case MP_THER:
		DBG_871X("set case MP_THER \n");
		rtw_mp_thermal (dev,info,wrqu,extra);
		break;
	case MP_PwrCtlDM:
		DBG_871X("set MP_PwrCtlDM\n");
		rtw_mp_PwrCtlDM (dev,info,wrqu,extra);
		break;
	case MP_QueryDrvStats:
		DBG_871X("mp_get MP_QueryDrvStats \n");
		rtw_mp_QueryDrv(dev,info,wdata,extra);
		break;
	case MP_PWRTRK:
		DBG_871X("set case MP_PWRTRK \n");
		rtw_mp_pwrtrk(dev,info,wrqu,extra);
		break;
	case EFUSE_SET:
		DBG_871X("set case efuse set \n");
		rtw_mp_efuse_set(dev,info,wdata,extra);
		break;
	case MP_GET_TXPOWER_INX:
		DBG_871X("mp_get MP_GET_TXPOWER_INX \n");
		rtw_mp_txpower_index(dev,info,wrqu,extra);
		break;
	case MP_GETVER:
		DBG_871X("mp_get MP_GETVER \n");
		rtw_mp_getver(dev,info,wdata,extra);
		break;
	case  MP_MON:
		DBG_871X("mp_get MP_MON \n");
		rtw_mp_mon(dev,info,wdata,extra);
		break;
	case  EFUSE_MASK:
		DBG_871X("mp_get MP_MON \n");
		rtw_efuse_mask_file(dev,info,wdata,extra);
		break;
	case  EFUSE_FILE:
		DBG_871X("mp_get MP_MON \n");
		rtw_efuse_file_map(dev,info,wdata,extra);
		break;
#if defined(CONFIG_RTL8723A) || defined(CONFIG_RTL8723B)
	case MP_SetBT:
		DBG_871X("set MP_SetBT \n");
		rtw_mp_SetBT(dev,info,wdata,extra);
		break;
#endif

	}

	rtw_msleep_os(10); //delay 5ms for sending pkt before exit adb shell operation
	return 0;
}

#endif //#if defined(CONFIG_MP_INCLUDED) && defined(CONFIG_MP_IWPRIV_SUPPORT)

static inline int rtw_wfd_tdls_enable(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS
#ifdef CONFIG_WFD

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	DBG_871X( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );

	if ( extra[ 0 ] == '0' )
		padapter->wdinfo.wfd_tdls_enable = 0;
	else
		padapter->wdinfo.wfd_tdls_enable = 1;

#endif /* CONFIG_WFD */
#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_weaksec(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS

	//u8 i, j;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	DBG_871X( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );

	if ( extra[ 0 ] == '0' )
		padapter->wdinfo.wfd_tdls_weaksec = 0;
	else
		padapter->wdinfo.wfd_tdls_weaksec = 1;

#endif /* CONFIG_TDLS */

	return ret;
}


static inline int rtw_tdls_enable(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_info	*ptdlsinfo = &padapter->tdlsinfo;
	_irqL	 irqL;
	_list	*plist, *phead;
	s32	index;
	struct sta_info *psta = NULL;
	struct	sta_priv *pstapriv = &padapter->stapriv;
	u8 tdls_sta[NUM_STA][ETH_ALEN];
	u8 empty_hwaddr[ETH_ALEN] = { 0x00 };
	struct tdls_txmgmt txmgmt;

	DBG_871X( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );

	_rtw_memset(tdls_sta, 0x00, sizeof(tdls_sta));
	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));

	if (extra[ 0 ] == '0') {
		ptdlsinfo->tdls_enable = 0;

		if(pstapriv->asoc_sta_count==1)
			return ret;

		_enter_critical_bh(&pstapriv->sta_hash_lock, &irqL);
		for (index=0; index< NUM_STA; index++) {
			phead = &(pstapriv->sta_hash[index]);
			plist = get_next(phead);

			while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
				psta = LIST_CONTAINOR(plist, struct sta_info ,hash_list);

				plist = get_next(plist);

				if (psta->tdls_sta_state != TDLS_STATE_NONE) {
					_rtw_memcpy(tdls_sta[index], psta->hwaddr, ETH_ALEN);
				}
			}
		}
		_exit_critical_bh(&pstapriv->sta_hash_lock, &irqL);

		for (index=0; index< NUM_STA; index++) {
			if (!_rtw_memcmp(tdls_sta[index], empty_hwaddr, ETH_ALEN)) {
				DBG_871X("issue tear down to "MAC_FMT"\n", MAC_ARG(tdls_sta[index]));
				txmgmt.status_code = _RSON_TDLS_TEAR_UN_RSN_;
				_rtw_memcpy(txmgmt.peer, tdls_sta[index], ETH_ALEN);
				issue_tdls_teardown(padapter, &txmgmt, _FALSE);
			}
		}
		rtw_tdls_cmd(padapter, NULL, TDLS_RS_RCR);
		rtw_reset_tdls_info(padapter);
	} else if (extra[0] == '1') {
		ptdlsinfo->tdls_enable = 1;
	}
#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_setup(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
#ifdef CONFIG_TDLS
	u8 i, j;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_txmgmt txmgmt;
#ifdef CONFIG_WFD
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
#endif /* CONFIG_WFD */

	DBG_871X("[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1);

	if (wrqu->data.length - 1 != 17) {
		DBG_871X("[%s] length:%d != 17\n", __FUNCTION__, (wrqu->data.length -1));
		return ret;
	}

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	for( i=0, j=0 ; i < ETH_ALEN; i++, j+=3 ) {
		txmgmt.peer[i]=key_2char2num(*(extra+j), *(extra+j+1));
	}

#ifdef CONFIG_WFD
	if (_AES_ != padapter->securitypriv.dot11PrivacyAlgrthm) {
		/* Weak Security situation with AP. */
		if (0 == pwdinfo->wfd_tdls_weaksec) {
			/* Can't send the tdls setup request out!! */
			DBG_871X("[%s] Current link is not AES, "
			         "SKIP sending the tdls setup request!!\n", __FUNCTION__);
		} else {
			issue_tdls_setup_req(padapter, &txmgmt, _TRUE);
		}
	} else
#endif /* CONFIG_WFD */
	{
		issue_tdls_setup_req(padapter, &txmgmt, _TRUE);
	}
#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_teardown(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS

	u8 i,j;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_info *ptdls_sta = NULL;
	struct tdls_txmgmt txmgmt;

	DBG_871X( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );

	if (wrqu->data.length - 1 != 17 && wrqu->data.length - 1 != 19) {
		DBG_871X("[%s] length:%d != 17 or 19\n",
		         __FUNCTION__, (wrqu->data.length -1));
		return ret;
	}

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	for (i=0, j=0; i < ETH_ALEN; i++, j+=3)
		txmgmt.peer[i]=key_2char2num(*(extra+j), *(extra+j+1));

	ptdls_sta = rtw_get_stainfo( &(padapter->stapriv), txmgmt.peer);

	if (ptdls_sta != NULL) {
		txmgmt.status_code = _RSON_TDLS_TEAR_UN_RSN_;
		if(wrqu->data.length - 1 == 17)
			issue_tdls_teardown(padapter, &txmgmt, _FALSE);
		else if(wrqu->data.length - 1 == 19)
			issue_tdls_teardown(padapter, &txmgmt, _TRUE);
	} else {
		DBG_871X( "TDLS peer not found\n");
	}
#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_discovery(struct net_device *dev,
                              struct iw_request_info *info,
                              union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_txmgmt	txmgmt;
	int i = 0, j=0;

	DBG_871X( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	for( i=0, j=0 ; i < ETH_ALEN; i++, j+=3 ) {
		txmgmt.peer[i]=key_2char2num(*(extra+j), *(extra+j+1));
	}

	issue_tdls_dis_req(padapter, &txmgmt);

#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_ch_switch(struct net_device *dev,
                              struct iw_request_info *info,
                              union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS
#ifdef CONFIG_TDLS_CH_SW
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_ch_switch *pchsw_info = &padapter->tdlsinfo.chsw_info;
	u8 i, j;
	struct sta_info *ptdls_sta = NULL;

	DBG_8192C( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );

	if (padapter->tdlsinfo.ch_switch_prohibited == _TRUE) {
		DBG_871X("Can't do TDLS channel switch since ch_switch_prohibited = _TRUE\n");
		return ret;
	}

	for( i=0, j=0 ; i < ETH_ALEN; i++, j+=3 ) {
		pchsw_info->addr[i] = key_2char2num(*(extra+j), *(extra+j+1));
	}

	ptdls_sta = rtw_get_stainfo(&padapter->stapriv, pchsw_info->addr);
	if( ptdls_sta == NULL )
		return ret;

	pchsw_info->ch_sw_state |= TDLS_CH_SW_INITIATOR_STATE;

	if (ptdls_sta != NULL) {
		if (pchsw_info->off_ch_num == 0)
			pchsw_info->off_ch_num = 11;
	} else {
		DBG_871X( "TDLS peer not found\n");
	}


	//issue_tdls_ch_switch_req(padapter, ptdls_sta);
	/* DBG_871X("issue tdls ch switch req\n"); */

#endif /* CONFIG_TDLS_CH_SW */
#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_ch_switch_off(struct net_device *dev,
                                  struct iw_request_info *info,
                                  union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS
#ifdef CONFIG_TDLS_CH_SW

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_ch_switch *pchsw_info = &padapter->tdlsinfo.chsw_info;
	u8 i, j, mac_addr[ETH_ALEN];
	struct sta_info *ptdls_sta = NULL;
	struct tdls_txmgmt txmgmt;

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));

	DBG_871X( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );

	if (padapter->tdlsinfo.ch_switch_prohibited == _TRUE) {
		DBG_871X("Can't do TDLS channel switch since ch_switch_prohibited = _TRUE\n");
		return ret;
	}

	if (wrqu->data.length >= 17) {
		for (i=0, j=0 ; i < ETH_ALEN; i++, j+=3)
			mac_addr[i]=key_2char2num(*(extra+j), *(extra+j+1));
		ptdls_sta = rtw_get_stainfo(&padapter->stapriv, mac_addr);
	}

	if (padapter->mlmeextpriv.cur_channel != rtw_get_oper_ch(padapter)) {
		SelectChannel(padapter, padapter->mlmeextpriv.cur_channel);
	}

	pchsw_info->ch_sw_state &= ~(TDLS_CH_SW_INITIATOR_STATE |
	                             TDLS_CH_SWITCH_ON_STATE |
	                             TDLS_PEER_AT_OFF_STATE);
	ATOMIC_SET(&pchsw_info->chsw_on, _FALSE);
	_rtw_memset(pchsw_info->addr, 0x00, ETH_ALEN);

	if (ptdls_sta != NULL) {
		ptdls_sta->ch_switch_time = 0;
		ptdls_sta->ch_switch_timeout = 0;
		_cancel_timer_ex(&ptdls_sta->ch_sw_timer);
		_cancel_timer_ex(&ptdls_sta->delay_timer);
	}

#endif /* CONFIG_TDLS_CH_SW */
#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_dump_ch(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS
#ifdef CONFIG_TDLS_CH_SW
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;

	DBG_8192C("[%s] dump_stack:%s\n", __FUNCTION__, extra);

	extra[ wrqu->data.length ] = 0x00;
	ptdlsinfo->chsw_info.dump_stack = rtw_atoi( extra );

	return ret;

#endif
#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_off_ch_num(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS
#ifdef CONFIG_TDLS_CH_SW
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;

	DBG_8192C("[%s] off_ch_num:%s\n", __FUNCTION__, extra);

	extra[ wrqu->data.length ] = 0x00;
	ptdlsinfo->chsw_info.off_ch_num = rtw_atoi(extra);

	return ret;

#endif
#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_ch_offset(struct net_device *dev,
                              struct iw_request_info *info,
                              union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS
#ifdef CONFIG_TDLS_CH_SW
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;

	DBG_8192C("[%s] ch_offset:%s\n", __FUNCTION__, extra);

	extra[ wrqu->data.length ] = 0x00;
	ptdlsinfo->chsw_info.ch_offset = rtw_atoi( extra );

	return ret;

#endif
#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_pson(struct net_device *dev,
                         struct iw_request_info *info,
                         union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 i, j, mac_addr[ETH_ALEN];
	struct sta_info *ptdls_sta = NULL;

	DBG_871X( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );

	for (i=0, j=0; i < ETH_ALEN; i++, j+=3)
		mac_addr[i]=key_2char2num(*(extra+j), *(extra+j+1));

	ptdls_sta = rtw_get_stainfo(&padapter->stapriv, mac_addr);

	issue_nulldata_to_TDLS_peer_STA(padapter, ptdls_sta->hwaddr, 1, 3, 500);

#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_psoff(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 i, j, mac_addr[ETH_ALEN];
	struct sta_info *ptdls_sta = NULL;

	DBG_8192C( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );

	for (i=0, j=0; i < ETH_ALEN; i++, j+=3)
		mac_addr[i]=key_2char2num(*(extra+j), *(extra+j+1));

	ptdls_sta = rtw_get_stainfo(&padapter->stapriv, mac_addr);

	if(ptdls_sta)
		issue_nulldata_to_TDLS_peer_STA(padapter, ptdls_sta->hwaddr, 0, 3, 500);

#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_setip(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS
#ifdef CONFIG_WFD

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	struct wifi_display_info *pwfd_info = ptdlsinfo->wfd_info;
	u8 i=0, j=0, tag=0;

	DBG_871X("[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length - 1);

	while (i < 4) {
		for (j=0; j < 4; j++) {
			if (*( extra + j + tag ) == '.' || *( extra + j + tag ) == '\0') {
				if( j == 1 )
					pwfd_info->ip_address[i]=convert_ip_addr( '0', '0', *(extra+(j-1)+tag));
				if( j == 2 )
					pwfd_info->ip_address[i]=convert_ip_addr( '0', *(extra+(j-2)+tag), *(extra+(j-1)+tag));
				if( j == 3 )
					pwfd_info->ip_address[i]=convert_ip_addr( *(extra+(j-3)+tag), *(extra+(j-2)+tag), *(extra+(j-1)+tag));

				tag += j + 1;
				break;
			}
		}
		i++;
	}

	DBG_871X( "[%s] Set IP = %u.%u.%u.%u \n", __FUNCTION__,
	          ptdlsinfo->wfd_info->ip_address[0],
	          ptdlsinfo->wfd_info->ip_address[1],
	          ptdlsinfo->wfd_info->ip_address[2],
	          ptdlsinfo->wfd_info->ip_address[3]);

#endif /* CONFIG_WFD */
#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_getip(struct net_device *dev,
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS
#ifdef CONFIG_WFD

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	struct wifi_display_info *pwfd_info = ptdlsinfo->wfd_info;

	DBG_871X( "[%s]\n", __FUNCTION__);

	sprintf( extra, "\n\n%u.%u.%u.%u\n",
	         pwfd_info->peer_ip_address[0], pwfd_info->peer_ip_address[1],
	         pwfd_info->peer_ip_address[2], pwfd_info->peer_ip_address[3]);

	DBG_871X( "[%s] IP=%u.%u.%u.%u\n", __FUNCTION__,
	          pwfd_info->peer_ip_address[0], pwfd_info->peer_ip_address[1],
	          pwfd_info->peer_ip_address[2], pwfd_info->peer_ip_address[3]);

	wrqu->data.length = strlen( extra );

#endif /* CONFIG_WFD */
#endif /* CONFIG_TDLS */

	return ret;
}

static inline int rtw_tdls_getport(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{

	int ret = 0;

#ifdef CONFIG_TDLS
#ifdef CONFIG_WFD

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	struct wifi_display_info *pwfd_info = ptdlsinfo->wfd_info;

	DBG_871X( "[%s]\n", __FUNCTION__);

	sprintf( extra, "\n\n%d\n", pwfd_info->peer_rtsp_ctrlport );
	DBG_871X( "[%s] remote port = %d\n",
	          __FUNCTION__, pwfd_info->peer_rtsp_ctrlport );

	wrqu->data.length = strlen( extra );

#endif /* CONFIG_WFD */
#endif /* CONFIG_TDLS */

	return ret;

}

/* WFDTDLS, for sigma test */
static inline int rtw_tdls_dis_result(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;

#ifdef CONFIG_TDLS
#ifdef CONFIG_WFD

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;

	DBG_871X( "[%s]\n", __FUNCTION__);

	if (ptdlsinfo->dev_discovered == _TRUE) {
		sprintf( extra, "\n\nDis=1\n" );
		ptdlsinfo->dev_discovered = _FALSE;
	}

	wrqu->data.length = strlen( extra );

#endif /* CONFIG_WFD */
#endif /* CONFIG_TDLS */

	return ret;

}

/* WFDTDLS, for sigma test */
static inline int rtw_wfd_tdls_status(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;

#ifdef CONFIG_TDLS

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;

	DBG_871X("[%s]\n", __FUNCTION__);

	sprintf( extra, "\nlink_established:%d \n"
	         "sta_cnt:%d \n"
	         "sta_maximum:%d \n"
	         "cur_channel:%d \n"
	         "tdls_enable:%d"
#ifdef CONFIG_TDLS_CH_SW
	         "ch_sw_state:%08x\n"
	         "chsw_on:%d\n"
	         "off_ch_num:%d\n"
	         "cur_time:%d\n"
	         "ch_offset:%d\n"
	         "delay_swtich_back:%d"
#endif
	         ,
	         ptdlsinfo->link_established, ptdlsinfo->sta_cnt,
	         ptdlsinfo->sta_maximum, ptdlsinfo->cur_channel,
	         ptdlsinfo->tdls_enable
#ifdef CONFIG_TDLS_CH_SW
	         ,
	         ptdlsinfo->chsw_info.ch_sw_state,
	         ATOMIC_READ(&padapter->tdlsinfo.chsw_info.chsw_on),
	         ptdlsinfo->chsw_info.off_ch_num,
	         ptdlsinfo->chsw_info.cur_time,
	         ptdlsinfo->chsw_info.ch_offset,
	         ptdlsinfo->chsw_info.delay_switch_back
#endif
	       );

	wrqu->data.length = strlen( extra );

#endif /* CONFIG_TDLS */

	return ret;

}

static inline int rtw_tdls_getsta(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
#ifdef CONFIG_TDLS
	u8 i, j;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 addr[ETH_ALEN] = {0};
	char charmac[17];
	struct sta_info *ptdls_sta = NULL;

	DBG_871X("[%s] %s %d\n", __FUNCTION__,
	         (char *)wrqu->data.pointer, wrqu->data.length -1);

	if(copy_from_user(charmac, wrqu->data.pointer+9, 17)) {
		ret = -EFAULT;
		goto exit;
	}

	DBG_871X("[%s] %d, charmac:%s\n", __FUNCTION__, __LINE__, charmac);
	for (i=0, j=0 ; i < ETH_ALEN; i++, j+=3)
		addr[i]=key_2char2num(*(charmac+j), *(charmac+j+1));

	DBG_871X("[%s] %d, charmac:%s, addr:"MAC_FMT"\n",
	         __FUNCTION__, __LINE__, charmac, MAC_ARG(addr));
	ptdls_sta = rtw_get_stainfo(&padapter->stapriv, addr);
	if(ptdls_sta) {
		sprintf(extra, "\n\ntdls_sta_state=0x%08x\n", ptdls_sta->tdls_sta_state);
		DBG_871X("\n\ntdls_sta_state=%d\n", ptdls_sta->tdls_sta_state);
	} else {
		sprintf(extra, "\n\nNot found this sta\n");
		DBG_871X("\n\nNot found this sta\n");
	}
	wrqu->data.length = strlen( extra );

exit:
#endif /* CONFIG_TDLS */
	return ret;

}

static inline int rtw_tdls_get_best_ch(struct net_device *dev,
                                struct iw_request_info *info,
                                union iwreq_data *wrqu, char *extra)
{
#ifdef CONFIG_FIND_BEST_CHANNEL
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	u32 i, best_channel_24G = 1, best_channel_5G = 36, index_24G = 0, index_5G = 0;

	for (i=0; pmlmeext->channel_set[i].ChannelNum !=0; i++) {
		if (pmlmeext->channel_set[i].ChannelNum == 1)
			index_24G = i;
		if (pmlmeext->channel_set[i].ChannelNum == 36)
			index_5G = i;
	}

	for (i=0; pmlmeext->channel_set[i].ChannelNum !=0; i++) {
		/* 2.4G */
		if (pmlmeext->channel_set[i].ChannelNum == 6 || pmlmeext->channel_set[i].ChannelNum == 11) {
			if (pmlmeext->channel_set[i].rx_count < pmlmeext->channel_set[index_24G].rx_count) {
				index_24G = i;
				best_channel_24G = pmlmeext->channel_set[i].ChannelNum;
			}
		}

		/* 5G */
		if (pmlmeext->channel_set[i].ChannelNum >= 36
		    && pmlmeext->channel_set[i].ChannelNum < 140) {
			/* Find primary channel */
			if (((pmlmeext->channel_set[i].ChannelNum - 36) % 8 == 0)
			    && (pmlmeext->channel_set[i].rx_count < pmlmeext->channel_set[index_5G].rx_count)) {
				index_5G = i;
				best_channel_5G = pmlmeext->channel_set[i].ChannelNum;
			}
		}

		if (pmlmeext->channel_set[i].ChannelNum >= 149
		    && pmlmeext->channel_set[i].ChannelNum < 165) {
			/* Find primary channel */
			if (((pmlmeext->channel_set[i].ChannelNum - 149) % 8 == 0)
			    && (pmlmeext->channel_set[i].rx_count < pmlmeext->channel_set[index_5G].rx_count)) {
				index_5G = i;
				best_channel_5G = pmlmeext->channel_set[i].ChannelNum;
			}
		}
#if 1 /* debug */
		DBG_871X("The rx cnt of channel %3d = %d\n",
		         pmlmeext->channel_set[i].ChannelNum,
		         pmlmeext->channel_set[i].rx_count);
#endif
	}

	sprintf( extra, "\nbest_channel_24G = %d\n", best_channel_24G );
	DBG_871X("best_channel_24G = %d\n", best_channel_24G);

	if (index_5G != 0) {
		sprintf(extra, "best_channel_5G = %d\n", best_channel_5G);
		DBG_871X("best_channel_5G = %d\n", best_channel_5G);
	}

	wrqu->data.length = strlen( extra );

#endif

	return 0;

}

static int rtw_tdls(struct net_device *dev,
                    struct iw_request_info *info,
                    union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	DBG_871X( "[%s] extra = %s\n", __FUNCTION__, extra );
	/* WFD Sigma will use the tdls enable command to let the driver know we want to test the tdls now! */
	if (_rtw_memcmp(extra, "wfdenable=", 10)) {
		wrqu->data.length -=10;
		rtw_wfd_tdls_enable( dev, info, wrqu, &extra[10] );
		return ret;
	} else if (_rtw_memcmp(extra, "weaksec=", 8)) {
		wrqu->data.length -=8;
		rtw_tdls_weaksec( dev, info, wrqu, &extra[8] );
		return ret;
	} else if (_rtw_memcmp( extra, "tdlsenable=", 11)) {
		wrqu->data.length -=11;
		rtw_tdls_enable( dev, info, wrqu, &extra[11] );
		return ret;
	}

	if (padapter->tdlsinfo.tdls_enable == 0) {
		DBG_871X("tdls haven't enabled\n");
		return 0;
	}

	if (_rtw_memcmp(extra, "setup=", 6)) {
		wrqu->data.length -=6;
		rtw_tdls_setup( dev, info, wrqu, &extra[6] );
	} else if (_rtw_memcmp(extra, "tear=", 5)) {
		wrqu->data.length -= 5;
		rtw_tdls_teardown( dev, info, wrqu, &extra[5] );
	} else if (_rtw_memcmp(extra, "dis=", 4)) {
		wrqu->data.length -= 4;
		rtw_tdls_discovery( dev, info, wrqu, &extra[4] );
	} else if (_rtw_memcmp(extra, "swoff=", 6)) {
		wrqu->data.length -= 6;
		rtw_tdls_ch_switch_off(dev, info, wrqu, &extra[6]);
	} else if (_rtw_memcmp(extra, "sw=", 3)) {
		wrqu->data.length -= 3;
		rtw_tdls_ch_switch( dev, info, wrqu, &extra[3] );
	} else if (_rtw_memcmp(extra, "dumpstack=", 10)) {
		wrqu->data.length -= 10;
		rtw_tdls_dump_ch(dev, info, wrqu, &extra[10]);
	} else if (_rtw_memcmp(extra, "offchnum=", 9)) {
		wrqu->data.length -= 9;
		rtw_tdls_off_ch_num(dev, info, wrqu, &extra[9]);
	} else if (_rtw_memcmp(extra, "choffset=", 9)) {
		wrqu->data.length -= 9;
		rtw_tdls_ch_offset(dev, info, wrqu, &extra[9]);
	} else if (_rtw_memcmp(extra, "pson=", 5)) {
		wrqu->data.length -= 5;
		rtw_tdls_pson( dev, info, wrqu, &extra[5] );
	} else if (_rtw_memcmp(extra, "psoff=", 6)) {
		wrqu->data.length -= 6;
		rtw_tdls_psoff( dev, info, wrqu, &extra[6] );
	}
#ifdef CONFIG_WFD
	else if (_rtw_memcmp(extra, "setip=", 6)) {
		wrqu->data.length -= 6;
		rtw_tdls_setip( dev, info, wrqu, &extra[6] );
	} else if (_rtw_memcmp(extra, "tprobe=", 6)) {
		issue_tunneled_probe_req((_adapter *)rtw_netdev_priv(dev));
	}
#endif /* CONFIG_WFD */

#endif /* CONFIG_TDLS */

	return ret;
}


static int rtw_tdls_get(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_TDLS

	DBG_871X( "[%s] extra = %s\n", __FUNCTION__, (char*) wrqu->data.pointer );

	if ( _rtw_memcmp( wrqu->data.pointer, "ip", 2 ) )
		rtw_tdls_getip( dev, info, wrqu, extra );
	else if (_rtw_memcmp(wrqu->data.pointer, "port", 4))
		rtw_tdls_getport( dev, info, wrqu, extra );
	/* WFDTDLS, for sigma test */
	else if ( _rtw_memcmp(wrqu->data.pointer, "dis", 3))
		rtw_tdls_dis_result( dev, info, wrqu, extra );
	else if ( _rtw_memcmp(wrqu->data.pointer, "status", 6))
		rtw_wfd_tdls_status( dev, info, wrqu, extra );
	else if ( _rtw_memcmp(wrqu->data.pointer, "tdls_sta=", 9))
		rtw_tdls_getsta( dev, info, wrqu, extra );
	else if (_rtw_memcmp(wrqu->data.pointer, "best_ch", 7))
		rtw_tdls_get_best_ch(dev, info, wrqu, extra);
#endif /* CONFIG_TDLS */

	return ret;
}





#ifdef CONFIG_INTEL_WIDI
static int rtw_widi_set(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	process_intel_widi_cmd(padapter, extra);

	return ret;
}

static int rtw_widi_set_probe_request(struct net_device *dev,
                                      struct iw_request_info *info,
                                      union iwreq_data *wrqu, char *extra)
{
	int	ret = 0;
	u8	*pbuf = NULL;
	_adapter	*padapter = (_adapter *)rtw_netdev_priv(dev);

	pbuf = rtw_malloc(sizeof(l2_msg_t));
	if(pbuf) {
		if ( copy_from_user(pbuf, wrqu->data.pointer, wrqu->data.length) )
			ret = -EFAULT;
		//_rtw_memcpy(pbuf, wrqu->data.pointer, wrqu->data.length);

		if( wrqu->data.flags == 0 )
			intel_widi_wk_cmd(padapter, INTEL_WIDI_ISSUE_PROB_WK, pbuf, sizeof(l2_msg_t));
		else if( wrqu->data.flags == 1 )
			rtw_set_wfd_rds_sink_info( padapter, (l2_msg_t *)pbuf );
	}
	return ret;
}
#endif // CONFIG_INTEL_WIDI

#ifdef CONFIG_MAC_LOOPBACK_DRIVER

#ifdef CONFIG_RTL8723A
extern void rtl8723a_cal_txdesc_chksum(struct tx_desc *ptxdesc);
#define cal_txdesc_chksum rtl8723a_cal_txdesc_chksum
extern void rtl8723a_fill_default_txdesc(struct xmit_frame *pxmitframe, u8 *pbuf);
#define fill_default_txdesc rtl8723a_fill_default_txdesc
#endif
#if defined(CONFIG_RTL8188E)
#include <rtl8188e_hal.h>
extern void rtl8188e_cal_txdesc_chksum(struct tx_desc *ptxdesc);
#define cal_txdesc_chksum rtl8188e_cal_txdesc_chksum
#ifdef CONFIG_SDIO_HCI || defined(CONFIG_GSPI_HCI)
extern void rtl8188es_fill_default_txdesc(struct xmit_frame *pxmitframe, u8 *pbuf);
#define fill_default_txdesc rtl8188es_fill_default_txdesc
#endif // CONFIG_SDIO_HCI
#endif // CONFIG_RTL8188E
#if defined(CONFIG_RTL8723B)
extern void rtl8723b_cal_txdesc_chksum(struct tx_desc *ptxdesc);
#define cal_txdesc_chksum rtl8723b_cal_txdesc_chksum
extern void rtl8723b_fill_default_txdesc(struct xmit_frame *pxmitframe, u8 *pbuf);
#define fill_default_txdesc rtl8723b_fill_default_txdesc
#endif // CONFIG_RTL8723B
#if defined(CONFIG_RTL8192E)
extern void rtl8192e_cal_txdesc_chksum(struct tx_desc *ptxdesc);
#define cal_txdesc_chksum rtl8192e_cal_txdesc_chksum
#ifdef CONFIG_SDIO_HCI || defined(CONFIG_GSPI_HCI)
extern void rtl8192es_fill_default_txdesc(struct xmit_frame *pxmitframe, u8 *pbuf);
#define fill_default_txdesc rtl8192es_fill_default_txdesc
#endif // CONFIG_SDIO_HCI
#endif //CONFIG_RTL8192E

static s32 initLoopback(PADAPTER padapter)
{
	PLOOPBACKDATA ploopback;


	if (padapter->ploopback == NULL) {
		ploopback = (PLOOPBACKDATA)rtw_zmalloc(sizeof(LOOPBACKDATA));
		if (ploopback == NULL) return -ENOMEM;

		_rtw_init_sema(&ploopback->sema, 0);
		ploopback->bstop = _TRUE;
		ploopback->cnt = 0;
		ploopback->size = 300;
		_rtw_memset(ploopback->msg, 0, sizeof(ploopback->msg));

		padapter->ploopback = ploopback;
	}

	return 0;
}

static void freeLoopback(PADAPTER padapter)
{
	PLOOPBACKDATA ploopback;


	ploopback = padapter->ploopback;
	if (ploopback) {
		rtw_mfree((u8*)ploopback, sizeof(LOOPBACKDATA));
		padapter->ploopback = NULL;
	}
}

static s32 initpseudoadhoc(PADAPTER padapter)
{
	NDIS_802_11_NETWORK_INFRASTRUCTURE networkType;
	s32 err;

	networkType = Ndis802_11IBSS;
	err = rtw_set_802_11_infrastructure_mode(padapter, networkType);
	if (err == _FALSE) return _FAIL;

	err = rtw_setopmode_cmd(padapter, networkType,_TRUE);
	if (err == _FAIL) return _FAIL;

	return _SUCCESS;
}

static s32 createpseudoadhoc(PADAPTER padapter)
{
	NDIS_802_11_AUTHENTICATION_MODE authmode;
	struct mlme_priv *pmlmepriv;
	NDIS_802_11_SSID *passoc_ssid;
	WLAN_BSSID_EX *pdev_network;
	u8 *pibss;
	u8 ssid[] = "pseduo_ad-hoc";
	s32 err;
	_irqL irqL;


	pmlmepriv = &padapter->mlmepriv;

	authmode = Ndis802_11AuthModeOpen;
	err = rtw_set_802_11_authentication_mode(padapter, authmode);
	if (err == _FALSE) return _FAIL;

	passoc_ssid = &pmlmepriv->assoc_ssid;
	_rtw_memset(passoc_ssid, 0, sizeof(NDIS_802_11_SSID));
	passoc_ssid->SsidLength = sizeof(ssid) - 1;
	_rtw_memcpy(passoc_ssid->Ssid, ssid, passoc_ssid->SsidLength);

	pdev_network = &padapter->registrypriv.dev_network;
	pibss = padapter->registrypriv.dev_network.MacAddress;
	_rtw_memcpy(&pdev_network->Ssid, passoc_ssid, sizeof(NDIS_802_11_SSID));

	rtw_update_registrypriv_dev_network(padapter);
	rtw_generate_random_ibss(pibss);

	_enter_critical_bh(&pmlmepriv->lock, &irqL);
	pmlmepriv->fw_state = WIFI_ADHOC_MASTER_STATE;
	_exit_critical_bh(&pmlmepriv->lock, &irqL);

#if 0
	err = rtw_createbss_cmd(padapter);
	if (err == _FAIL) return _FAIL;
#else
	{
		struct wlan_network *pcur_network;
		struct sta_info *psta;

		//3  create a new psta
		pcur_network = &pmlmepriv->cur_network;

		//clear psta in the cur_network, if any
		psta = rtw_get_stainfo(&padapter->stapriv, pcur_network->network.MacAddress);
		if (psta) rtw_free_stainfo(padapter, psta);

		psta = rtw_alloc_stainfo(&padapter->stapriv, pibss);
		if (psta == NULL) return _FAIL;

		//3  join psudo AdHoc
		pcur_network->join_res = 1;
		pcur_network->aid = psta->aid = 1;
		_rtw_memcpy(&pcur_network->network, pdev_network, get_WLAN_BSSID_EX_sz(pdev_network));

		// set msr to WIFI_FW_ADHOC_STATE
#if 0
		Set_NETYPE0_MSR(padapter, WIFI_FW_ADHOC_STATE);
#else
		{
			u8 val8;

			val8 = rtw_read8(padapter, MSR);
			val8 &= 0xFC; // clear NETYPE0
			val8 |= WIFI_FW_ADHOC_STATE & 0x3;
			rtw_write8(padapter, MSR, val8);
		}
#endif
	}
#endif

	return _SUCCESS;
}

static struct xmit_frame* createloopbackpkt(PADAPTER padapter, u32 size)
{
	struct xmit_priv *pxmitpriv;
	struct xmit_frame *pframe;
	struct xmit_buf *pxmitbuf;
	struct pkt_attrib *pattrib;
	struct tx_desc *desc;
	u8 *pkt_start, *pkt_end, *ptr;
	struct rtw_ieee80211_hdr *hdr;
	s32 bmcast;
	_irqL irqL;


	if ((TXDESC_SIZE + WLANHDR_OFFSET + size) > MAX_XMITBUF_SZ) return NULL;

	pxmitpriv = &padapter->xmitpriv;
	pframe = NULL;

	//2 1. allocate xmit frame
	pframe = rtw_alloc_xmitframe(pxmitpriv);
	if (pframe == NULL) return NULL;
	pframe->padapter = padapter;

	//2 2. allocate xmit buffer
	_enter_critical_bh(&pxmitpriv->lock, &irqL);
	pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
	_exit_critical_bh(&pxmitpriv->lock, &irqL);
	if (pxmitbuf == NULL) {
		rtw_free_xmitframe(pxmitpriv, pframe);
		return NULL;
	}

	pframe->pxmitbuf = pxmitbuf;
	pframe->buf_addr = pxmitbuf->pbuf;
	pxmitbuf->priv_data = pframe;

	//2 3. update_attrib()
	pattrib = &pframe->attrib;

	// init xmitframe attribute
	_rtw_memset(pattrib, 0, sizeof(struct pkt_attrib));

	pattrib->ether_type = 0x8723;
	_rtw_memcpy(pattrib->src, padapter->eeprompriv.mac_addr, ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
	_rtw_memset(pattrib->dst, 0xFF, ETH_ALEN);
	_rtw_memcpy(pattrib->ra, pattrib->dst, ETH_ALEN);

//	pattrib->dhcp_pkt = 0;
//	pattrib->pktlen = 0;
	pattrib->ack_policy = 0;
//	pattrib->pkt_hdrlen = ETH_HLEN;
	pattrib->hdrlen = WLAN_HDR_A3_LEN;
	pattrib->subtype = WIFI_DATA;
	pattrib->priority = 0;
	pattrib->qsel = pattrib->priority;
//	do_queue_select(padapter, pattrib);
	pattrib->nr_frags = 1;
	pattrib->encrypt = 0;
	pattrib->bswenc = _FALSE;
	pattrib->qos_en = _FALSE;

	bmcast = IS_MCAST(pattrib->ra);
	if (bmcast) {
		pattrib->mac_id = 1;
		pattrib->psta = rtw_get_bcmc_stainfo(padapter);
	} else {
		pattrib->mac_id = 0;
		pattrib->psta = rtw_get_stainfo(&padapter->stapriv, get_bssid(&padapter->mlmepriv));
	}

	pattrib->pktlen = size;
	pattrib->last_txcmdsz = pattrib->hdrlen + pattrib->pktlen;

	//2 4. fill TX descriptor
	desc = (struct tx_desc*)pframe->buf_addr;
	_rtw_memset(desc, 0, TXDESC_SIZE);

	fill_default_txdesc(pframe, (u8*)desc);

	// Hw set sequence number
	((PTXDESC)desc)->hwseq_en = 0; // HWSEQ_EN, 0:disable, 1:enable
//	((PTXDESC)desc)->hwseq_sel = 0; // HWSEQ_SEL

	((PTXDESC)desc)->disdatafb = 1;

	// convert to little endian
	desc->txdw0 = cpu_to_le32(desc->txdw0);
	desc->txdw1 = cpu_to_le32(desc->txdw1);
	desc->txdw2 = cpu_to_le32(desc->txdw2);
	desc->txdw3 = cpu_to_le32(desc->txdw3);
	desc->txdw4 = cpu_to_le32(desc->txdw4);
	desc->txdw5 = cpu_to_le32(desc->txdw5);
	desc->txdw6 = cpu_to_le32(desc->txdw6);
	desc->txdw7 = cpu_to_le32(desc->txdw7);
#ifdef CONFIG_PCI_HCI
	desc->txdw8 = cpu_to_le32(desc->txdw8);
	desc->txdw9 = cpu_to_le32(desc->txdw9);
	desc->txdw10 = cpu_to_le32(desc->txdw10);
	desc->txdw11 = cpu_to_le32(desc->txdw11);
	desc->txdw12 = cpu_to_le32(desc->txdw12);
	desc->txdw13 = cpu_to_le32(desc->txdw13);
	desc->txdw14 = cpu_to_le32(desc->txdw14);
	desc->txdw15 = cpu_to_le32(desc->txdw15);
#endif

	cal_txdesc_chksum(desc);

	//2 5. coalesce
	pkt_start = pframe->buf_addr + TXDESC_SIZE;
	pkt_end = pkt_start + pattrib->last_txcmdsz;

	//3 5.1. make wlan header, make_wlanhdr()
	hdr = (struct rtw_ieee80211_hdr *)pkt_start;
	SetFrameSubType(&hdr->frame_ctl, pattrib->subtype);
	_rtw_memcpy(hdr->addr1, pattrib->dst, ETH_ALEN); // DA
	_rtw_memcpy(hdr->addr2, pattrib->src, ETH_ALEN); // SA
	_rtw_memcpy(hdr->addr3, get_bssid(&padapter->mlmepriv), ETH_ALEN); // RA, BSSID

	//3 5.2. make payload
	ptr = pkt_start + pattrib->hdrlen;
	get_random_bytes(ptr, pkt_end - ptr);

	pxmitbuf->len = TXDESC_SIZE + pattrib->last_txcmdsz;
	pxmitbuf->ptail += pxmitbuf->len;

	return pframe;
}

static void freeloopbackpkt(PADAPTER padapter, struct xmit_frame *pframe)
{
	struct xmit_priv *pxmitpriv;
	struct xmit_buf *pxmitbuf;


	pxmitpriv = &padapter->xmitpriv;
	pxmitbuf = pframe->pxmitbuf;

	rtw_free_xmitframe(pxmitpriv, pframe);
	rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
}

static void printdata(u8 *pbuf, u32 len)
{
	u32 i, val;


	for (i = 0; (i+4) <= len; i+=4) {
		printk("%08X", *(u32*)(pbuf + i));
		if ((i+4) & 0x1F) printk(" ");
		else printk("\n");
	}

	if (i < len) {
#ifdef CONFIG_BIG_ENDIAN
		for (; i < len, i++)
			printk("%02X", pbuf+i);
#else // CONFIG_LITTLE_ENDIAN
#if 0
		val = 0;
		_rtw_memcpy(&val, pbuf + i, len - i);
		printk("%8X", val);
#else
		u8 str[9];
		u8 n;
		val = 0;
		n = len - i;
		_rtw_memcpy(&val, pbuf+i, n);
		sprintf(str, "%08X", val);
		n = (4 - n) * 2;
		printk("%8s", str+n);
#endif
#endif // CONFIG_LITTLE_ENDIAN
	}
	printk("\n");
}

static u8 pktcmp(PADAPTER padapter, u8 *txbuf, u32 txsz, u8 *rxbuf, u32 rxsz)
{
	PHAL_DATA_TYPE phal;
	struct recv_stat *prxstat;
	struct recv_stat report;
	PRXREPORT prxreport;
	u32 drvinfosize;
	u32 rxpktsize;
	u8 fcssize;
	u8 ret = _FALSE;

	prxstat = (struct recv_stat*)rxbuf;
	report.rxdw0 = le32_to_cpu(prxstat->rxdw0);
	report.rxdw1 = le32_to_cpu(prxstat->rxdw1);
	report.rxdw2 = le32_to_cpu(prxstat->rxdw2);
	report.rxdw3 = le32_to_cpu(prxstat->rxdw3);
	report.rxdw4 = le32_to_cpu(prxstat->rxdw4);
	report.rxdw5 = le32_to_cpu(prxstat->rxdw5);

	prxreport = (PRXREPORT)&report;
	drvinfosize = prxreport->drvinfosize << 3;
	rxpktsize = prxreport->pktlen;

	phal = GET_HAL_DATA(padapter);
	if (phal->ReceiveConfig & RCR_APPFCS) fcssize = IEEE80211_FCS_LEN;
	else fcssize = 0;

	if ((txsz - TXDESC_SIZE) != (rxpktsize - fcssize)) {
		DBG_8192C("%s: ERROR! size not match tx/rx=%d/%d !\n",
		          __func__, txsz - TXDESC_SIZE, rxpktsize - fcssize);
		ret = _FALSE;
	} else {
		ret = _rtw_memcmp(txbuf + TXDESC_SIZE,\
		                  rxbuf + RXDESC_SIZE + drvinfosize,\
		                  txsz - TXDESC_SIZE);
		if (ret == _FALSE) {
			DBG_8192C("%s: ERROR! pkt content mismatch!\n", __func__);
		}
	}

	if (ret == _FALSE) {
		DBG_8192C("\n%s: TX PKT total=%d, desc=%d, content=%d\n",
		          __func__, txsz, TXDESC_SIZE, txsz - TXDESC_SIZE);
		DBG_8192C("%s: TX DESC size=%d\n", __func__, TXDESC_SIZE);
		printdata(txbuf, TXDESC_SIZE);
		DBG_8192C("%s: TX content size=%d\n", __func__, txsz - TXDESC_SIZE);
		printdata(txbuf + TXDESC_SIZE, txsz - TXDESC_SIZE);

		DBG_8192C("\n%s: RX PKT read=%d offset=%d(%d,%d) content=%d\n",
		          __func__, rxsz, RXDESC_SIZE + drvinfosize, RXDESC_SIZE, drvinfosize, rxpktsize);
		if (rxpktsize != 0) {
			DBG_8192C("%s: RX DESC size=%d\n", __func__, RXDESC_SIZE);
			printdata(rxbuf, RXDESC_SIZE);
			DBG_8192C("%s: RX drvinfo size=%d\n", __func__, drvinfosize);
			printdata(rxbuf + RXDESC_SIZE, drvinfosize);
			DBG_8192C("%s: RX content size=%d\n", __func__, rxpktsize);
			printdata(rxbuf + RXDESC_SIZE + drvinfosize, rxpktsize);
		} else {
			DBG_8192C("%s: RX data size=%d\n", __func__, rxsz);
			printdata(rxbuf, rxsz);
		}
	}

	return ret;
}

thread_return lbk_thread(thread_context context)
{
	s32 err;
	PADAPTER padapter;
	PLOOPBACKDATA ploopback;
	struct xmit_frame *pxmitframe;
	u32 cnt, ok, fail, headerlen;
	u32 pktsize;
	u32 ff_hwaddr;


	padapter = (PADAPTER)context;
	ploopback = padapter->ploopback;
	if (ploopback == NULL) return -1;
	cnt = 0;
	ok = 0;
	fail = 0;

	daemonize("%s", "RTW_LBK_THREAD");
	allow_signal(SIGTERM);

	do {
		if (ploopback->size == 0) {
			get_random_bytes(&pktsize, 4);
			pktsize = (pktsize % 1535) + 1; // 1~1535
		} else
			pktsize = ploopback->size;

		pxmitframe = createloopbackpkt(padapter, pktsize);
		if (pxmitframe == NULL) {
			sprintf(ploopback->msg, "loopback FAIL! 3. create Packet FAIL!");
			break;
		}

		ploopback->txsize = TXDESC_SIZE + pxmitframe->attrib.last_txcmdsz;
		_rtw_memcpy(ploopback->txbuf, pxmitframe->buf_addr, ploopback->txsize);
		ff_hwaddr = rtw_get_ff_hwaddr(pxmitframe);
		cnt++;
		DBG_8192C("%s: wirte port cnt=%d size=%d\n", __func__, cnt, ploopback->txsize);
		pxmitframe->pxmitbuf->pdata = ploopback->txbuf;
		rtw_write_port(padapter, ff_hwaddr, ploopback->txsize, (u8 *)pxmitframe->pxmitbuf);

		// wait for rx pkt
		_rtw_down_sema(&ploopback->sema);

		err = pktcmp(padapter, ploopback->txbuf, ploopback->txsize, ploopback->rxbuf, ploopback->rxsize);
		if (err == _TRUE)
			ok++;
		else
			fail++;

		ploopback->txsize = 0;
		_rtw_memset(ploopback->txbuf, 0, 0x8000);
		ploopback->rxsize = 0;
		_rtw_memset(ploopback->rxbuf, 0, 0x8000);

		freeloopbackpkt(padapter, pxmitframe);
		pxmitframe = NULL;

		if (signal_pending(current)) {
			flush_signals(current);
		}

		if ((ploopback->bstop == _TRUE) ||
		    ((ploopback->cnt != 0) && (ploopback->cnt == cnt))) {
			u32 ok_rate, fail_rate, all;
			all = cnt;
			ok_rate = (ok*100)/all;
			fail_rate = (fail*100)/all;
			sprintf(ploopback->msg,\
			        "loopback result: ok=%d%%(%d/%d),error=%d%%(%d/%d)",\
			        ok_rate, ok, all, fail_rate, fail, all);
			break;
		}
	} while (1);

	ploopback->bstop = _TRUE;

	thread_exit();
}

static void loopbackTest(PADAPTER padapter, u32 cnt, u32 size, u8* pmsg)
{
	PLOOPBACKDATA ploopback;
	u32 len;
	s32 err;


	ploopback = padapter->ploopback;

	if (ploopback) {
		if (ploopback->bstop == _FALSE) {
			ploopback->bstop = _TRUE;
			_rtw_up_sema(&ploopback->sema);
		}
		len = 0;
		do {
			len = strlen(ploopback->msg);
			if (len) break;
			rtw_msleep_os(1);
		} while (1);
		_rtw_memcpy(pmsg, ploopback->msg, len+1);
		freeLoopback(padapter);

		return;
	}

	// disable dynamic algorithm
	{
		u32 DMFlag = DYNAMIC_FUNC_DISABLE;
		rtw_hal_get_hwreg(padapter, HW_VAR_DM_FLAG, (u8*)&DMFlag);
	}

	// create pseudo ad-hoc connection
	err = initpseudoadhoc(padapter);
	if (err == _FAIL) {
		sprintf(pmsg, "loopback FAIL! 1.1 init ad-hoc FAIL!");
		return;
	}

	err = createpseudoadhoc(padapter);
	if (err == _FAIL) {
		sprintf(pmsg, "loopback FAIL! 1.2 create ad-hoc master FAIL!");
		return;
	}

	err = initLoopback(padapter);
	if (err) {
		sprintf(pmsg, "loopback FAIL! 2. init FAIL! error code=%d", err);
		return;
	}

	ploopback = padapter->ploopback;

	ploopback->bstop = _FALSE;
	ploopback->cnt = cnt;
	ploopback->size = size;
	ploopback->lbkthread = kthread_run(lbk_thread, padapter, "RTW_LBK_THREAD");
	if (IS_ERR(padapter->lbkthread)) {
		freeLoopback(padapter);
		sprintf(pmsg, "loopback start FAIL! cnt=%d", cnt);
		return;
	}

	sprintf(pmsg, "loopback start! cnt=%d", cnt);
}
#endif // CONFIG_MAC_LOOPBACK_DRIVER

static int rtw_test(
    struct net_device *dev,
    struct iw_request_info *info,
    union iwreq_data *wrqu, char *extra)
{
	u32 len;
	u8 *pbuf, *pch;
	char *ptmp;
	u8 *delim = ",";
#if defined(CONFIG_BT_COEXIST) || defined(CONFIG_MAC_LOOPBACK_DRIVER)
	PADAPTER padapter = rtw_netdev_priv(dev);
#endif

	DBG_871X("+%s\n", __func__);
	len = wrqu->data.length;

	pbuf = (u8*)rtw_zmalloc(len);
	if (pbuf == NULL) {
		DBG_871X("%s: no memory!\n", __func__);
		return -ENOMEM;
	}

	if (copy_from_user(pbuf, wrqu->data.pointer, len)) {
		rtw_mfree(pbuf, len);
		DBG_871X("%s: copy from user fail!\n", __func__);
		return -EFAULT;
	}
	DBG_871X("%s: string=\"%s\"\n", __func__, pbuf);

	ptmp = (char*)pbuf;
	pch = strsep(&ptmp, delim);
	if ((pch == NULL) || (strlen(pch) == 0)) {
		rtw_mfree(pbuf, len);
		DBG_871X("%s: parameter error(level 1)!\n", __func__);
		return -EFAULT;
	}

#ifdef CONFIG_MAC_LOOPBACK_DRIVER
	if (strcmp(pch, "loopback") == 0) {
		s32 cnt = 0;
		u32 size = 64;

		pch = strsep(&ptmp, delim);
		if ((pch == NULL) || (strlen(pch) == 0)) {
			rtw_mfree(pbuf, len);
			DBG_871X("%s: parameter error(level 2)!\n", __func__);
			return -EFAULT;
		}

		sscanf(pch, "%d", &cnt);
		DBG_871X("%s: loopback cnt=%d\n", __func__, cnt);

		pch = strsep(&ptmp, delim);
		if ((pch == NULL) || (strlen(pch) == 0)) {
			rtw_mfree(pbuf, len);
			DBG_871X("%s: parameter error(level 2)!\n", __func__);
			return -EFAULT;
		}

		sscanf(pch, "%d", &size);
		DBG_871X("%s: loopback size=%d\n", __func__, size);

		loopbackTest(padapter, cnt, size, extra);
		wrqu->data.length = strlen(extra) + 1;

		rtw_mfree(pbuf, len);
		return 0;
	}
#endif

#if 0
//#ifdef CONFIG_RTL8723A
	if (strcmp(pch, "poweron") == 0) {
		s32 ret;

		ret = _InitPowerOn(padapter);
		DBG_871X("%s: power on %s\n", __func__, (_FAIL==ret) ? "FAIL!":"OK.");
		sprintf(extra, "Power ON %s", (_FAIL==ret) ? "FAIL!":"OK.");
		wrqu->data.length = strlen(extra) + 1;

		rtw_mfree(pbuf, len);
		return 0;
	}

	if (strcmp(pch, "dlfw") == 0) {
		s32 ret;

		ret = rtl8723a_FirmwareDownload(padapter);
		DBG_871X("%s: download FW %s\n", __func__, (_FAIL==ret) ? "FAIL!":"OK.");
		sprintf(extra, "download FW %s", (_FAIL==ret) ? "FAIL!":"OK.");
		wrqu->data.length = strlen(extra) + 1;

		rtw_mfree(pbuf, len);
		return 0;
	}
#endif

#ifdef CONFIG_BT_COEXIST
	if (strcmp(pch, "bton") == 0) {
		rtw_btcoex_SetManualControl(padapter, _FALSE);
	} else if (strcmp(pch, "btoff") == 0) {
		rtw_btcoex_SetManualControl(padapter, _TRUE);
	} else if (strcmp(pch, "h2c") == 0) {
		u8 param[8];
		u8 count = 0;
		u32 tmp;
		u8 i;
		u32 pos;
		s32 ret;


		do {
			pch = strsep(&ptmp, delim);
			if ((pch == NULL) || (strlen(pch) == 0))
				break;

			sscanf(pch, "%x", &tmp);
			param[count++] = (u8)tmp;
		} while (count < 8);

		if (count == 0) {
			rtw_mfree(pbuf, len);
			DBG_871X("%s: parameter error(level 2)!\n", __func__);
			return -EFAULT;
		}

		ret = rtw_hal_fill_h2c_cmd(padapter, param[0], count-1, &param[1]);

		pos = sprintf(extra, "H2C ID=0x%02x content=", param[0]);
		for (i=1; i<count; i++) {
			pos += sprintf(extra+pos, "%02x,", param[i]);
		}
		extra[pos] = 0;
		pos--;
		pos += sprintf(extra+pos, " %s", ret==_FAIL?"FAIL":"OK");

		wrqu->data.length = strlen(extra) + 1;
	}
#endif // CONFIG_BT_COEXIST

	rtw_mfree(pbuf, len);
	return 0;
}

#ifdef CONFIG_WIRELESS_EXT
static iw_handler rtw_handlers[] = {
	NULL,					/* SIOCSIWCOMMIT */
	rtw_wx_get_name,		/* SIOCGIWNAME */
	dummy,					/* SIOCSIWNWID */
	dummy,					/* SIOCGIWNWID */
	rtw_wx_set_freq,		/* SIOCSIWFREQ */
	rtw_wx_get_freq,		/* SIOCGIWFREQ */
	rtw_wx_set_mode,		/* SIOCSIWMODE */
	rtw_wx_get_mode,		/* SIOCGIWMODE */
	dummy,					/* SIOCSIWSENS */
	rtw_wx_get_sens,		/* SIOCGIWSENS */
	NULL,					/* SIOCSIWRANGE */
	rtw_wx_get_range,		/* SIOCGIWRANGE */
	rtw_wx_set_priv,		/* SIOCSIWPRIV */
	NULL,					/* SIOCGIWPRIV */
	NULL,					/* SIOCSIWSTATS */
	NULL,					/* SIOCGIWSTATS */
	dummy,					/* SIOCSIWSPY */
	dummy,					/* SIOCGIWSPY */
	NULL,					/* SIOCGIWTHRSPY */
	NULL,					/* SIOCWIWTHRSPY */
	rtw_wx_set_wap,		/* SIOCSIWAP */
	rtw_wx_get_wap,		/* SIOCGIWAP */
	rtw_wx_set_mlme,		/* request MLME operation; uses struct iw_mlme */
	dummy,					/* SIOCGIWAPLIST -- depricated */
	rtw_wx_set_scan,		/* SIOCSIWSCAN */
	rtw_wx_get_scan,		/* SIOCGIWSCAN */
	rtw_wx_set_essid,		/* SIOCSIWESSID */
	rtw_wx_get_essid,		/* SIOCGIWESSID */
	dummy,					/* SIOCSIWNICKN */
	rtw_wx_get_nick,		/* SIOCGIWNICKN */
	NULL,					/* -- hole -- */
	NULL,					/* -- hole -- */
	rtw_wx_set_rate,		/* SIOCSIWRATE */
	rtw_wx_get_rate,		/* SIOCGIWRATE */
	rtw_wx_set_rts,			/* SIOCSIWRTS */
	rtw_wx_get_rts,			/* SIOCGIWRTS */
	rtw_wx_set_frag,		/* SIOCSIWFRAG */
	rtw_wx_get_frag,		/* SIOCGIWFRAG */
	dummy,					/* SIOCSIWTXPOW */
	dummy,					/* SIOCGIWTXPOW */
	dummy,					/* SIOCSIWRETRY */
	rtw_wx_get_retry,		/* SIOCGIWRETRY */
	rtw_wx_set_enc,			/* SIOCSIWENCODE */
	rtw_wx_get_enc,			/* SIOCGIWENCODE */
	dummy,					/* SIOCSIWPOWER */
	rtw_wx_get_power,		/* SIOCGIWPOWER */
	NULL,					/*---hole---*/
	NULL,					/*---hole---*/
	rtw_wx_set_gen_ie,		/* SIOCSIWGENIE */
	NULL,					/* SIOCGWGENIE */
	rtw_wx_set_auth,		/* SIOCSIWAUTH */
	NULL,					/* SIOCGIWAUTH */
	rtw_wx_set_enc_ext,		/* SIOCSIWENCODEEXT */
	NULL,					/* SIOCGIWENCODEEXT */
	rtw_wx_set_pmkid,		/* SIOCSIWPMKSA */
	NULL,					/*---hole---*/
};


static const struct iw_priv_args rtw_private_args[] = {
	{
		SIOCIWFIRSTPRIV + 0x0,
		IW_PRIV_TYPE_CHAR | 0x7FF, 0, "write"
	},
	{
		SIOCIWFIRSTPRIV + 0x1,
		IW_PRIV_TYPE_CHAR | 0x7FF,
		IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | IFNAMSIZ, "read"
	},
	{
		SIOCIWFIRSTPRIV + 0x2, 0, 0, "driver_ext"
	},
	{
		SIOCIWFIRSTPRIV + 0x3, 0, 0, "mp_ioctl"
	},
	{
		SIOCIWFIRSTPRIV + 0x4,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "apinfo"
	},
	{
		SIOCIWFIRSTPRIV + 0x5,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "setpid"
	},
	{
		SIOCIWFIRSTPRIV + 0x6,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_start"
	},
//for PLATFORM_MT53XX
	{
		SIOCIWFIRSTPRIV + 0x7,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "get_sensitivity"
	},
	{
		SIOCIWFIRSTPRIV + 0x8,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_prob_req_ie"
	},
	{
		SIOCIWFIRSTPRIV + 0x9,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_assoc_req_ie"
	},

//for RTK_DMP_PLATFORM
	{
		SIOCIWFIRSTPRIV + 0xA,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "channel_plan"
	},

	{
		SIOCIWFIRSTPRIV + 0xB,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "dbg"
	},
	{
		SIOCIWFIRSTPRIV + 0xC,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "rfw"
	},
	{
		SIOCIWFIRSTPRIV + 0xD,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | IFNAMSIZ, "rfr"
	},
#if 0
	{
		SIOCIWFIRSTPRIV + 0xE,0,0, "wowlan_ctrl"
	},
#endif
	{
		SIOCIWFIRSTPRIV + 0x10,
		IW_PRIV_TYPE_CHAR | 1024, 0, "p2p_set"
	},
	{
		SIOCIWFIRSTPRIV + 0x11,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK , "p2p_get"
	},
	{
		SIOCIWFIRSTPRIV + 0x12, 0, 0, "NULL"
	},
	{
		SIOCIWFIRSTPRIV + 0x13,
		IW_PRIV_TYPE_CHAR | 64, IW_PRIV_TYPE_CHAR | 64 , "p2p_get2"
	},
	{
		SIOCIWFIRSTPRIV + 0x14,
		IW_PRIV_TYPE_CHAR  | 64, 0, "tdls"
	},
	{
		SIOCIWFIRSTPRIV + 0x15,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024 , "tdls_get"
	},
	{
		SIOCIWFIRSTPRIV + 0x16,
		IW_PRIV_TYPE_CHAR | 64, 0, "pm_set"
	},

	{SIOCIWFIRSTPRIV + 0x18, IW_PRIV_TYPE_CHAR | IFNAMSIZ , 0 , "rereg_nd_name"},
#ifdef CONFIG_MP_INCLUDED
	{SIOCIWFIRSTPRIV + 0x1A, IW_PRIV_TYPE_CHAR | 1024, 0,  "NULL"},
	{SIOCIWFIRSTPRIV + 0x1B, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "NULL"},
#else
	{SIOCIWFIRSTPRIV + 0x1A, IW_PRIV_TYPE_CHAR | 1024, 0, "efuse_set"},
	{SIOCIWFIRSTPRIV + 0x1B, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "efuse_get"},
#endif
	{
		SIOCIWFIRSTPRIV + 0x1D,
		IW_PRIV_TYPE_CHAR | 40, IW_PRIV_TYPE_CHAR | 0x7FF, "test"
	},

#ifdef CONFIG_INTEL_WIDI
	{
		SIOCIWFIRSTPRIV + 0x1E,
		IW_PRIV_TYPE_CHAR | 1024, 0, "widi_set"
	},
	{
		SIOCIWFIRSTPRIV + 0x1F,
		IW_PRIV_TYPE_CHAR | 128, 0, "widi_prob_req"
	},
#endif // CONFIG_INTEL_WIDI

#ifdef CONFIG_MP_INCLUDED
	{ SIOCIWFIRSTPRIV + 0x0E, IW_PRIV_TYPE_CHAR | 1024, 0 , ""},  //set
	{ SIOCIWFIRSTPRIV + 0x0F, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK , ""},//get
	/* --- sub-ioctls definitions --- */
	{ MP_START , IW_PRIV_TYPE_CHAR | 1024, 0, "mp_start" }, //set
	{ MP_PHYPARA, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_phypara" },//get
	{ MP_STOP , IW_PRIV_TYPE_CHAR | 1024, 0, "mp_stop" }, //set
	{ MP_CHANNEL , IW_PRIV_TYPE_CHAR | 1024 , IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_channel" },//get
	{ MP_BANDWIDTH , IW_PRIV_TYPE_CHAR | 1024, 0, "mp_bandwidth"}, //set
	{ MP_RATE , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_rate" },//get
	{ MP_RESET_STATS , IW_PRIV_TYPE_CHAR | 1024, 0, "mp_reset_stats"},
	{ MP_QUERY , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK , "mp_query"}, //get
	{ READ_REG , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "read_reg" },
	{ MP_RATE , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_rate" },
	{ READ_RF , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "read_rf" },
	{ MP_PSD , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_psd"},
	{ MP_DUMP, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_dump" },
	{ MP_TXPOWER , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_txpower"},
	{ MP_ANT_TX , IW_PRIV_TYPE_CHAR | 1024,  IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_ant_tx"},
	{ MP_ANT_RX , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_ant_rx"},
	{ WRITE_REG , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "write_reg" },
	{ WRITE_RF , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "write_rf" },
	{ MP_CTX , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_ctx"},
	{ MP_ARX , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_arx"},
	{ MP_THER , IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_ther"},
	{ EFUSE_SET, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "efuse_set" },
	{ EFUSE_GET, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "efuse_get" },
	{ MP_PWRTRK , IW_PRIV_TYPE_CHAR | 1024, 0, "mp_pwrtrk"},
	{ MP_QueryDrvStats, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_drvquery" },
	{ MP_IOCTL, IW_PRIV_TYPE_CHAR | 1024, 0, "mp_ioctl"}, // mp_ioctl
	{ MP_SetRFPathSwh, IW_PRIV_TYPE_CHAR | 1024, 0, "mp_setrfpath" },
	{ MP_PwrCtlDM, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_pwrctldm" },
	{ MP_GET_TXPOWER_INX, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_get_txpower" },
	{ MP_GETVER, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_priv_ver" },
	{ MP_MON, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_mon" },
	{ EFUSE_MASK, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "efuse_mask" },
	{ EFUSE_FILE, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "efuse_file" },
#if defined(CONFIG_RTL8723A) || defined(CONFIG_RTL8723B)
	{ MP_SetBT, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "mp_setbt" },
	{ MP_DISABLE_BT_COEXIST, IW_PRIV_TYPE_CHAR | 1024, 0, "mp_disa_btcoex"},
#endif
	{ CTA_TEST, IW_PRIV_TYPE_CHAR | 1024, 0, "cta_test"},
#endif
#ifdef CONFIG_WOWLAN
	{ MP_WOW_ENABLE , IW_PRIV_TYPE_CHAR | 1024, 0, "wow_mode" }, //set
#endif
#ifdef CONFIG_AP_WOWLAN
	{ MP_AP_WOW_ENABLE , IW_PRIV_TYPE_CHAR | 1024, 0, "ap_wow_mode" }, //set
#endif
};

static iw_handler rtw_private_handler[] = {
	rtw_wx_write32,					//0x00
	rtw_wx_read32,					//0x01
	rtw_drvext_hdl,					//0x02
	rtw_mp_ioctl_hdl,				//0x03

// for MM DTV platform
	rtw_get_ap_info,					//0x04

	rtw_set_pid,						//0x05
	rtw_wps_start,					//0x06

// for PLATFORM_MT53XX
	rtw_wx_get_sensitivity,			//0x07
	rtw_wx_set_mtk_wps_probe_ie,	//0x08
	rtw_wx_set_mtk_wps_ie,			//0x09

// for RTK_DMP_PLATFORM
// Set Channel depend on the country code
	rtw_wx_set_channel_plan,		//0x0A

	rtw_dbg_port,					//0x0B
	rtw_wx_write_rf,					//0x0C
	rtw_wx_read_rf,					//0x0D
#ifdef CONFIG_MP_INCLUDED
	rtw_mp_set,					//0x0E
	rtw_mp_get,					//0x0F
#else
	rtw_wx_priv_null,				//0x0E
	rtw_wx_priv_null,				//0x0F
#endif
	rtw_p2p_set,					//0x10
	rtw_p2p_get,					//0x11
	NULL,							//0x12
	rtw_p2p_get2,					//0x13

	rtw_tdls,						//0x14
	rtw_tdls_get,					//0x15

	rtw_pm_set,						//0x16
	rtw_wx_priv_null,				//0x17
	rtw_rereg_nd_name,				//0x18
	rtw_wx_priv_null,				//0x19
#ifdef CONFIG_MP_INCLUDED
	rtw_wx_priv_null,				//0x1A
	rtw_wx_priv_null,				//0x1B
#else
	rtw_mp_efuse_set,				//0x1A
	rtw_mp_efuse_get,				//0x1B
#endif
	NULL,							// 0x1C is reserved for hostapd
	rtw_test,						// 0x1D
#ifdef CONFIG_INTEL_WIDI
	rtw_widi_set,					//0x1E
	rtw_widi_set_probe_request,		//0x1F
#endif // CONFIG_INTEL_WIDI
};
#endif

#ifdef CONFIG_WIRELESS_EXT
#if WIRELESS_EXT >= 17
static struct iw_statistics *rtw_get_wireless_stats(struct net_device *dev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct iw_statistics *piwstats=&padapter->iwstats;
	int tmp_level = 0;
	int tmp_qual = 0;
	int tmp_noise = 0;

	if (check_fwstate(&padapter->mlmepriv, _FW_LINKED) != _TRUE) {
		piwstats->qual.qual = 0;
		piwstats->qual.level = 0;
		piwstats->qual.noise = 0;
		//DBG_871X("No link  level:%d, qual:%d, noise:%d\n", tmp_level, tmp_qual, tmp_noise);
	} else {
#ifdef CONFIG_SIGNAL_DISPLAY_DBM
		tmp_level = translate_percentage_to_dbm(padapter->recvpriv.signal_strength);
#else
#ifdef CONFIG_SKIP_SIGNAL_SCALE_MAPPING
		{
			/* Do signal scale mapping when using percentage as the unit of signal strength, since the scale mapping is skipped in odm */

			HAL_DATA_TYPE *pHal = GET_HAL_DATA(padapter);

			tmp_level = (u8)odm_SignalScaleMapping(&pHal->odmpriv, padapter->recvpriv.signal_strength);
		}
#else
		tmp_level = padapter->recvpriv.signal_strength;
#endif
#endif

		tmp_qual = padapter->recvpriv.signal_qual;
		rtw_get_noise(padapter);
		tmp_noise = padapter->recvpriv.noise;
		//DBG_871X("level:%d, qual:%d, noise:%d, rssi (%d)\n", tmp_level, tmp_qual, tmp_noise,padapter->recvpriv.rssi);

		piwstats->qual.level = tmp_level;
		piwstats->qual.qual = tmp_qual;
		piwstats->qual.noise = tmp_noise;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14))
	piwstats->qual.updated = IW_QUAL_ALL_UPDATED ;//|IW_QUAL_DBM;
#else
#ifdef RTK_DMP_PLATFORM
	//IW_QUAL_DBM= 0x8, if driver use this flag, wireless extension will show value of dbm.
	//remove this flag for show percentage 0~100
	piwstats->qual.updated = 0x07;
#else
	piwstats->qual.updated = 0x0f;
#endif
#endif

#ifdef CONFIG_SIGNAL_DISPLAY_DBM
	piwstats->qual.updated = piwstats->qual.updated | IW_QUAL_DBM;
#endif

	return &padapter->iwstats;
}
#endif
#endif

#ifdef CONFIG_WIRELESS_EXT
struct iw_handler_def rtw_handlers_def = {
	.standard = rtw_handlers,
	.num_standard = sizeof(rtw_handlers) / sizeof(iw_handler),
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)) || defined(CONFIG_WEXT_PRIV)
	.private = rtw_private_handler,
	.private_args = (struct iw_priv_args *)rtw_private_args,
	.num_private = sizeof(rtw_private_handler) / sizeof(iw_handler),
	.num_private_args = sizeof(rtw_private_args) / sizeof(struct iw_priv_args),
#endif
#if WIRELESS_EXT >= 17
	.get_wireless_stats = rtw_get_wireless_stats,
#endif
};

// copy from net/wireless/wext.c start
/* ---------------------------------------------------------------- */
/*
 * Calculate size of private arguments
 */
static const char iw_priv_type_size[] = {
	0,                              /* IW_PRIV_TYPE_NONE */
	1,                              /* IW_PRIV_TYPE_BYTE */
	1,                              /* IW_PRIV_TYPE_CHAR */
	0,                              /* Not defined */
	sizeof(__u32),                  /* IW_PRIV_TYPE_INT */
	sizeof(struct iw_freq),         /* IW_PRIV_TYPE_FLOAT */
	sizeof(struct sockaddr),        /* IW_PRIV_TYPE_ADDR */
	0,                              /* Not defined */
};

static int get_priv_size(__u16 args)
{
	int num = args & IW_PRIV_SIZE_MASK;
	int type = (args & IW_PRIV_TYPE_MASK) >> 12;

	return num * iw_priv_type_size[type];
}
// copy from net/wireless/wext.c end


static int _rtw_ioctl_wext_private(struct net_device *dev, union iwreq_data *wrq_data)
{
	int err = 0;
	u8 *input = NULL;
	u32 input_len = 0;
	const char delim[] = " ";
	u8 *output = NULL;
	u32 output_len = 0;
	u32 count = 0;
	u8 *buffer= NULL;
	u32 buffer_len = 0;
	char *ptr = NULL;
	u8 cmdname[17] = {0}; // IFNAMSIZ+1
	u32 cmdlen;
	s32 len;
	u8 *extra = NULL;
	u32 extra_size = 0;

	s32 k;
	const iw_handler *priv;		/* Private ioctl */
	const struct iw_priv_args *priv_args;	/* Private ioctl description */
	u32 num_priv;				/* Number of ioctl */
	u32 num_priv_args;			/* Number of descriptions */
	iw_handler handler;
	int temp;
	int subcmd = 0;				/* sub-ioctl index */
	int offset = 0;				/* Space for sub-ioctl index */

	union iwreq_data wdata;

	_rtw_memcpy(&wdata, wrq_data, sizeof(wdata));

	input_len = wdata.data.length;
	input = rtw_zmalloc(input_len);
	if (NULL == input || input_len == 0)
		return -ENOMEM;
	if (copy_from_user(input, wdata.data.pointer, input_len)) {
		err = -EFAULT;
		goto exit;
	}
	input[input_len - 1] = '\0';
	ptr = input;
	len = input_len;
	sscanf(ptr, "%16s", cmdname);
	cmdlen = strlen(cmdname);
	DBG_871X("%s: cmd=%s\n", __func__, cmdname);

	// skip command string
	if (cmdlen > 0)
		cmdlen += 1; // skip one space
	ptr += cmdlen;
	len -= cmdlen;
	DBG_871X("%s: parameters=%s\n", __func__, ptr);

	priv = rtw_private_handler;
	priv_args = rtw_private_args;
	num_priv = sizeof(rtw_private_handler) / sizeof(iw_handler);
	num_priv_args = sizeof(rtw_private_args) / sizeof(struct iw_priv_args);

	if (num_priv_args == 0) {
		err = -EOPNOTSUPP;
		goto exit;
	}

	/* Search the correct ioctl */
	k = -1;
	while((++k < num_priv_args) && strcmp(priv_args[k].name, cmdname));

	/* If not found... */
	if (k == num_priv_args) {
		err = -EOPNOTSUPP;
		goto exit;
	}

	/* Watch out for sub-ioctls ! */
	if (priv_args[k].cmd < SIOCDEVPRIVATE) {
		int j = -1;

		/* Find the matching *real* ioctl */
		while ((++j < num_priv_args) && ((priv_args[j].name[0] != '\0') ||
		                                 (priv_args[j].set_args != priv_args[k].set_args) ||
		                                 (priv_args[j].get_args != priv_args[k].get_args)));

		/* If not found... */
		if (j == num_priv_args) {
			err = -EINVAL;
			goto exit;
		}

		/* Save sub-ioctl number */
		subcmd = priv_args[k].cmd;
		/* Reserve one int (simplify alignment issues) */
		offset = sizeof(__u32);
		/* Use real ioctl definition from now on */
		k = j;
	}

	buffer = rtw_zmalloc(4096);
	if (NULL == buffer) {
		err = -ENOMEM;
		goto exit;
	}

	/* If we have to set some data */
	if ((priv_args[k].set_args & IW_PRIV_TYPE_MASK) &&
	    (priv_args[k].set_args & IW_PRIV_SIZE_MASK)) {
		u8 *str;

		switch (priv_args[k].set_args & IW_PRIV_TYPE_MASK) {
		case IW_PRIV_TYPE_BYTE:
			/* Fetch args */
			count = 0;
			do {
				str = strsep(&ptr, delim);
				if (NULL == str) break;
				sscanf(str, "%i", &temp);
				buffer[count++] = (u8)temp;
			} while (1);
			buffer_len = count;

			/* Number of args to fetch */
			wdata.data.length = count;
			if (wdata.data.length > (priv_args[k].set_args & IW_PRIV_SIZE_MASK))
				wdata.data.length = priv_args[k].set_args & IW_PRIV_SIZE_MASK;

			break;

		case IW_PRIV_TYPE_INT:
			/* Fetch args */
			count = 0;
			do {
				str = strsep(&ptr, delim);
				if (NULL == str) break;
				sscanf(str, "%i", &temp);
				((s32*)buffer)[count++] = (s32)temp;
			} while (1);
			buffer_len = count * sizeof(s32);

			/* Number of args to fetch */
			wdata.data.length = count;
			if (wdata.data.length > (priv_args[k].set_args & IW_PRIV_SIZE_MASK))
				wdata.data.length = priv_args[k].set_args & IW_PRIV_SIZE_MASK;

			break;

		case IW_PRIV_TYPE_CHAR:
			if (len > 0) {
				/* Size of the string to fetch */
				wdata.data.length = len;
				if (wdata.data.length > (priv_args[k].set_args & IW_PRIV_SIZE_MASK))
					wdata.data.length = priv_args[k].set_args & IW_PRIV_SIZE_MASK;

				/* Fetch string */
				_rtw_memcpy(buffer, ptr, wdata.data.length);
			} else {
				wdata.data.length = 1;
				buffer[0] = '\0';
			}
			buffer_len = wdata.data.length;
			break;

		default:
			DBG_8192C("%s: Not yet implemented...\n", __func__);
			err = -1;
			goto exit;
		}

		if ((priv_args[k].set_args & IW_PRIV_SIZE_FIXED) &&
		    (wdata.data.length != (priv_args[k].set_args & IW_PRIV_SIZE_MASK))) {
			DBG_8192C("%s: The command %s needs exactly %d argument(s)...\n",
			          __func__, cmdname, priv_args[k].set_args & IW_PRIV_SIZE_MASK);
			err = -EINVAL;
			goto exit;
		}
	}   /* if args to set */
	else {
		wdata.data.length = 0L;
	}

	/* Those two tests are important. They define how the driver
	* will have to handle the data */
	if ((priv_args[k].set_args & IW_PRIV_SIZE_FIXED) &&
	    ((get_priv_size(priv_args[k].set_args) + offset) <= IFNAMSIZ)) {
		/* First case : all SET args fit within wrq */
		if (offset)
			wdata.mode = subcmd;
		_rtw_memcpy(wdata.name + offset, buffer, IFNAMSIZ - offset);
	} else {
		if ((priv_args[k].set_args == 0) &&
		    (priv_args[k].get_args & IW_PRIV_SIZE_FIXED) &&
		    (get_priv_size(priv_args[k].get_args) <= IFNAMSIZ)) {
			/* Second case : no SET args, GET args fit within wrq */
			if (offset)
				wdata.mode = subcmd;
		} else {
			/* Third case : args won't fit in wrq, or variable number of args */
			if (copy_to_user(wdata.data.pointer, buffer, buffer_len)) {
				err = -EFAULT;
				goto exit;
			}
			wdata.data.flags = subcmd;
		}
	}

	rtw_mfree(input, input_len);
	input = NULL;

	extra_size = 0;
	if (IW_IS_SET(priv_args[k].cmd)) {
		/* Size of set arguments */
		extra_size = get_priv_size(priv_args[k].set_args);

		/* Does it fits in iwr ? */
		if ((priv_args[k].set_args & IW_PRIV_SIZE_FIXED) &&
		    ((extra_size + offset) <= IFNAMSIZ))
			extra_size = 0;
	} else {
		/* Size of get arguments */
		extra_size = get_priv_size(priv_args[k].get_args);

		/* Does it fits in iwr ? */
		if ((priv_args[k].get_args & IW_PRIV_SIZE_FIXED) &&
		    (extra_size <= IFNAMSIZ))
			extra_size = 0;
	}

	if (extra_size == 0) {
		extra = (u8*)&wdata;
		rtw_mfree(buffer, 4096);
		buffer = NULL;
	} else
		extra = buffer;

	handler = priv[priv_args[k].cmd - SIOCIWFIRSTPRIV];
	err = handler(dev, NULL, &wdata, extra);

	/* If we have to get some data */
	if ((priv_args[k].get_args & IW_PRIV_TYPE_MASK) &&
	    (priv_args[k].get_args & IW_PRIV_SIZE_MASK)) {
		int j;
		int n = 0;	/* number of args */
		u8 str[20] = {0};

		/* Check where is the returned data */
		if ((priv_args[k].get_args & IW_PRIV_SIZE_FIXED) &&
		    (get_priv_size(priv_args[k].get_args) <= IFNAMSIZ))
			n = priv_args[k].get_args & IW_PRIV_SIZE_MASK;
		else
			n = wdata.data.length;

		output = rtw_zmalloc(4096);
		if (NULL == output) {
			err =  -ENOMEM;
			goto exit;
		}

		switch (priv_args[k].get_args & IW_PRIV_TYPE_MASK) {
		case IW_PRIV_TYPE_BYTE:
			/* Display args */
			for (j = 0; j < n; j++) {
				sprintf(str, "%d  ", extra[j]);
				len = strlen(str);
				output_len = strlen(output);
				if ((output_len + len + 1) > 4096) {
					err = -E2BIG;
					goto exit;
				}
				_rtw_memcpy(output+output_len, str, len);
			}
			break;

		case IW_PRIV_TYPE_INT:
			/* Display args */
			for (j = 0; j < n; j++) {
				sprintf(str, "%d  ", ((__s32*)extra)[j]);
				len = strlen(str);
				output_len = strlen(output);
				if ((output_len + len + 1) > 4096) {
					err = -E2BIG;
					goto exit;
				}
				_rtw_memcpy(output+output_len, str, len);
			}
			break;

		case IW_PRIV_TYPE_CHAR:
			/* Display args */
			_rtw_memcpy(output, extra, n);
			break;

		default:
			DBG_8192C("%s: Not yet implemented...\n", __func__);
			err = -1;
			goto exit;
		}

		output_len = strlen(output) + 1;
		wrq_data->data.length = output_len;
		if (copy_to_user(wrq_data->data.pointer, output, output_len)) {
			err = -EFAULT;
			goto exit;
		}
	}   /* if args to set */
	else {
		wrq_data->data.length = 0;
	}

exit:
	if (input)
		rtw_mfree(input, input_len);
	if (buffer)
		rtw_mfree(buffer, 4096);
	if (output)
		rtw_mfree(output, 4096);

	return err;
}
#endif

#ifdef CONFIG_COMPAT
static int rtw_ioctl_compat_wext_private(struct net_device *dev, struct ifreq *rq)
{
	struct compat_iw_point iwp_compat;
	union iwreq_data wrq_data;
	int err = 0;
	DBG_871X("%s:...\n", __func__);
	if (copy_from_user(&iwp_compat, rq->ifr_ifru.ifru_data, sizeof(struct compat_iw_point)))
		return -EFAULT;

	wrq_data.data.pointer = compat_ptr(iwp_compat.pointer);
	wrq_data.data.length = iwp_compat.length;
	wrq_data.data.flags = iwp_compat.flags;

	err = _rtw_ioctl_wext_private(dev, &wrq_data);

	iwp_compat.pointer = ptr_to_compat(wrq_data.data.pointer);
	iwp_compat.length = wrq_data.data.length;
	iwp_compat.flags = wrq_data.data.flags;
	if (copy_to_user(rq->ifr_ifru.ifru_data, &iwp_compat, sizeof(struct compat_iw_point)))
		return -EFAULT;

	return err;
}
#endif // CONFIG_COMPAT

#ifdef CONFIG_WIRELESS_EXT
static inline int rtw_ioctl_standard_wext_private(struct net_device *dev, struct ifreq *rq)
{
	struct iw_point *iwp;
	//struct ifreq ifrq;
	union iwreq_data wrq_data;
	int err = 0;
	iwp = &wrq_data.data;
	DBG_871X("%s:...\n", __func__);
	if (copy_from_user(iwp, rq->ifr_ifru.ifru_data, sizeof(struct iw_point)))
		return -EFAULT;

	err = _rtw_ioctl_wext_private(dev, &wrq_data);

	if (copy_to_user(rq->ifr_ifru.ifru_data, iwp, sizeof(struct iw_point)))
		return -EFAULT;

	return err;
}

static int rtw_ioctl_wext_private(struct net_device *dev, struct ifreq *rq)
{
#ifdef CONFIG_COMPAT
	if(rtw_is_compat_task())
		return rtw_ioctl_compat_wext_private( dev, rq );
	else
#endif // CONFIG_COMPAT
		return rtw_ioctl_standard_wext_private( dev, rq );
}

int rtw_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct iwreq *wrq = (struct iwreq *)rq;
	int ret=0;

	switch (cmd) {
	case RTL_IOCTL_WPA_SUPPLICANT:
		ret = wpa_supplicant_ioctl(dev, &wrq->u.data);
		break;
#ifdef CONFIG_AP_MODE
	case RTL_IOCTL_HOSTAPD:
		ret = rtw_hostapd_ioctl(dev, &wrq->u.data);
		break;
#ifdef CONFIG_NO_WIRELESS_HANDLERS
	case SIOCSIWMODE:
		ret = rtw_wx_set_mode(dev, NULL, &wrq->u, NULL);
		break;
#endif
#endif // CONFIG_AP_MODE
	case SIOCDEVPRIVATE:
		ret = rtw_ioctl_wext_private(dev, rq);
		break;
	case (SIOCDEVPRIVATE+1):
		ret = rtw_android_priv_cmd(dev, rq, cmd);
		break;
	default:
		ret = -EOPNOTSUPP;
		break;
	}

	return ret;
}

#endif
#endif
