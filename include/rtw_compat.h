/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Mon, 03 Oct 2016 23:17:42 +0800
 */
#ifndef _RTW_COMPAT_H_
#define _RTW_COMPAT_H_

#include <linux/version.h>

#ifdef CONFIG_COMPAT
#ifdef in_compat_syscall
	#define rtw_is_compat_task in_compat_syscall
#else
	#define rtw_is_compat_task is_compat_task
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0)
#else
#define ieee80211_band nl80211_band
#define IEEE80211_BAND_2GHZ NL80211_BAND_2GHZ
#define IEEE80211_BAND_5GHZ NL80211_BAND_5GHZ
#define IEEE80211_BAND_60GHZ NL80211_BAND_60GHZ
#define IEEE80211_NUM_BANDS NUM_NL80211_BANDS
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
#else
#define STATION_INFO_SIGNAL BIT(NL80211_STA_INFO_SIGNAL)
#define STATION_INFO_TX_BITRATE BIT(NL80211_STA_INFO_TX_BITRATE)
#define STATION_INFO_RX_PACKETS BIT(NL80211_STA_INFO_RX_PACKETS)
#define STATION_INFO_TX_PACKETS BIT(NL80211_STA_INFO_RX_PACKETS)

#define strnicmp strncasecmp

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0) */

#endif /* _RTW_COMPAT_H_ */
