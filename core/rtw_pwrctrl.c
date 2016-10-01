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
#define _RTW_PWRCTRL_C_

#include <drv_types.h>

// Should not include hal dependent herader here, it will remove later. Lucas@20130123
#ifdef CONFIG_BT_COEXIST
#include <rtl8723a_hal.h>
#endif

#ifdef CONFIG_IPS
void ips_enter(_adapter * padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;	

	_enter_pwrlock(&pwrpriv->lock);

	pwrpriv->bips_processing = _TRUE;	

	// syn ips_mode with request
	pwrpriv->ips_mode = pwrpriv->ips_mode_req;
	
	pwrpriv->ips_enter_cnts++;	
	DBG_871X("==>ips_enter cnts:%d\n",pwrpriv->ips_enter_cnts);
#ifdef CONFIG_BT_COEXIST
	BTDM_TurnOffBtCoexistBeforeEnterIPS(padapter);
#endif
	if(rf_off == pwrpriv->change_rfpwrstate )
	{	
		pwrpriv->bpower_saving = _TRUE;
		DBG_871X_LEVEL(_drv_always_, "nolinked power save enter\n");

		if(pwrpriv->ips_mode == IPS_LEVEL_2)
			pwrpriv->bkeepfwalive = _TRUE;
		
		rtw_ips_pwr_down(padapter);
		pwrpriv->rf_pwrstate = rf_off;
	}	
	pwrpriv->bips_processing = _FALSE;	
	
	_exit_pwrlock(&pwrpriv->lock);
	
}

int ips_leave(_adapter * padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct mlme_priv		*pmlmepriv = &(padapter->mlmepriv);
	int result = _SUCCESS;
	sint keyid;


	_enter_pwrlock(&pwrpriv->lock);

	if((pwrpriv->rf_pwrstate == rf_off) &&(!pwrpriv->bips_processing))
	{
		pwrpriv->bips_processing = _TRUE;
		pwrpriv->change_rfpwrstate = rf_on;
		pwrpriv->ips_leave_cnts++;
		DBG_871X("==>ips_leave cnts:%d\n",pwrpriv->ips_leave_cnts);

		if ((result = rtw_ips_pwr_up(padapter)) == _SUCCESS) {
			pwrpriv->rf_pwrstate = rf_on;
		}
		DBG_871X_LEVEL(_drv_always_, "nolinked power save leave\n");

		if((_WEP40_ == psecuritypriv->dot11PrivacyAlgrthm) ||(_WEP104_ == psecuritypriv->dot11PrivacyAlgrthm))
		{
			DBG_871X("==>%s,channel(%d),processing(%x)\n",__FUNCTION__,padapter->mlmeextpriv.cur_channel,pwrpriv->bips_processing);
			set_channel_bwmode(padapter, padapter->mlmeextpriv.cur_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, CHANNEL_WIDTH_20);			
			for(keyid=0;keyid<4;keyid++){
				if(pmlmepriv->key_mask & BIT(keyid)){
					if(keyid == psecuritypriv->dot11PrivacyKeyIndex)
						result=rtw_set_key(padapter,psecuritypriv, keyid, 1);
					else
						result=rtw_set_key(padapter,psecuritypriv, keyid, 0);
				}
			}
		}
		
		DBG_871X("==> ips_leave.....LED(0x%08x)...\n",rtw_read32(padapter,0x4c));
		pwrpriv->bips_processing = _FALSE;

		pwrpriv->bkeepfwalive = _FALSE;
		pwrpriv->bpower_saving = _FALSE;
	}

	_exit_pwrlock(&pwrpriv->lock);

	return result;
}


#endif

#ifdef CONFIG_AUTOSUSPEND
extern void autosuspend_enter(_adapter* padapter);	
extern int autoresume_enter(_adapter* padapter);
#endif

#ifdef SUPPORT_HW_RFOFF_DETECTED
int rtw_hw_suspend(_adapter *padapter );
int rtw_hw_resume(_adapter *padapter);
#endif

bool rtw_pwr_unassociated_idle(_adapter *adapter)
{
	_adapter *buddy = adapter->pbuddy_adapter;
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);
	struct xmit_priv *pxmit_priv = &adapter->xmitpriv;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &(adapter->wdinfo);
#ifdef CONFIG_IOCTL_CFG80211
	//struct cfg80211_wifidirect_info *pcfg80211_wdinfo = &adapter->cfg80211_wdinfo;
#endif
#endif

	bool ret = _FALSE;

	if (adapter->pwrctrlpriv.ips_deny_time >= rtw_get_current_time()) {
		//DBG_871X("%s ips_deny_time\n", __func__);
		goto exit;
	}

	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE|WIFI_SITE_MONITOR)
		|| check_fwstate(pmlmepriv, WIFI_UNDER_LINKING|WIFI_UNDER_WPS)
		|| check_fwstate(pmlmepriv, WIFI_AP_STATE)
		|| check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE|WIFI_ADHOC_STATE)
		#if defined(CONFIG_P2P) && defined(CONFIG_IOCTL_CFG80211) && defined(CONFIG_P2P_IPS)
		|| pcfg80211_wdinfo->is_ro_ch
		#elif defined(CONFIG_P2P)
		|| !rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)
		#endif
	) {
		goto exit;
	}

	/* consider buddy, if exist */
	if (buddy) {
		struct mlme_priv *b_pmlmepriv = &(buddy->mlmepriv);
		#ifdef CONFIG_P2P
		struct wifidirect_info *b_pwdinfo = &(buddy->wdinfo);
		#ifdef CONFIG_IOCTL_CFG80211
		//struct cfg80211_wifidirect_info *b_pcfg80211_wdinfo = &buddy->cfg80211_wdinfo;
		#endif
		#endif

		if (check_fwstate(b_pmlmepriv, WIFI_ASOC_STATE|WIFI_SITE_MONITOR)
			|| check_fwstate(b_pmlmepriv, WIFI_UNDER_LINKING|WIFI_UNDER_WPS)
			|| check_fwstate(b_pmlmepriv, WIFI_AP_STATE)
			|| check_fwstate(b_pmlmepriv, WIFI_ADHOC_MASTER_STATE|WIFI_ADHOC_STATE)
			#if defined(CONFIG_P2P) && defined(CONFIG_IOCTL_CFG80211) && defined(CONFIG_P2P_IPS)
			|| b_pcfg80211_wdinfo->is_ro_ch
			#elif defined(CONFIG_P2P)
			|| !rtw_p2p_chk_state(b_pwdinfo, P2P_STATE_NONE)
			#endif
		) {
			goto exit;
		}
	}

#if (MP_DRIVER == 1)
	if (adapter->registrypriv.mp_mode == 1)
		goto exit;
#endif

#ifdef CONFIG_INTEL_PROXIM
	if(adapter->proximity.proxim_on==_TRUE){
		return;
	}
#endif

	if (pxmit_priv->free_xmitbuf_cnt != NR_XMITBUFF ||
		pxmit_priv->free_xmit_extbuf_cnt != NR_XMIT_EXTBUFF) {
		DBG_871X_LEVEL(_drv_always_, "There are some pkts to transmit\n");
		DBG_871X_LEVEL(_drv_info_, "free_xmitbuf_cnt: %d, free_xmit_extbuf_cnt: %d\n", 
			pxmit_priv->free_xmitbuf_cnt, pxmit_priv->free_xmit_extbuf_cnt);	
		goto exit;
	}

	ret = _TRUE;

exit:
	return ret;
}

void rtw_ps_processor(_adapter*padapter)
{
#ifdef CONFIG_P2P
	//struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
#endif //CONFIG_P2P
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
#ifdef CONFIG_DEBUG
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
#endif
#ifdef SUPPORT_HW_RFOFF_DETECTED
	rt_rf_power_state rfpwrstate;
#endif //SUPPORT_HW_RFOFF_DETECTED

	pwrpriv->ps_processing = _TRUE;

#ifdef SUPPORT_HW_RFOFF_DETECTED
	if(pwrpriv->bips_processing == _TRUE)
		goto exit;
	
	//DBG_871X("==> fw report state(0x%x)\n",rtw_read8(padapter,0x1ca));	
	if(padapter->pwrctrlpriv.bHWPwrPindetect) 
	{
	#ifdef CONFIG_AUTOSUSPEND
		if(padapter->registrypriv.usbss_enable)
		{
			if(pwrpriv->rf_pwrstate == rf_on)
			{
				if(padapter->net_closed == _TRUE)
					pwrpriv->ps_flag = _TRUE;

				rfpwrstate = RfOnOffDetect(padapter);
				DBG_871X("@@@@- #1  %s==> rfstate:%s \n",__FUNCTION__,(rfpwrstate==rf_on)?"rf_on":"rf_off");
				if(rfpwrstate!= pwrpriv->rf_pwrstate)
				{
					if(rfpwrstate == rf_off)
					{
						pwrpriv->change_rfpwrstate = rf_off;
						
						pwrpriv->bkeepfwalive = _TRUE;	
						pwrpriv->brfoffbyhw = _TRUE;						
						
						autosuspend_enter(padapter);							
					}
				}
			}			
		}
		else
	#endif //CONFIG_AUTOSUSPEND
		{
			rfpwrstate = RfOnOffDetect(padapter);
			DBG_871X("@@@@- #2  %s==> rfstate:%s \n",__FUNCTION__,(rfpwrstate==rf_on)?"rf_on":"rf_off");

			if(rfpwrstate!= pwrpriv->rf_pwrstate)
			{
				if(rfpwrstate == rf_off)
				{	
					pwrpriv->change_rfpwrstate = rf_off;														
					pwrpriv->brfoffbyhw = _TRUE;
					padapter->bCardDisableWOHSM = _TRUE;
					rtw_hw_suspend(padapter );	
				}
				else
				{
					pwrpriv->change_rfpwrstate = rf_on;
					rtw_hw_resume(padapter );			
				}
				DBG_871X("current rf_pwrstate(%s)\n",(pwrpriv->rf_pwrstate == rf_off)?"rf_off":"rf_on");
			}
		}
		pwrpriv->pwr_state_check_cnts ++;	
	}
#endif //SUPPORT_HW_RFOFF_DETECTED

	if (pwrpriv->ips_mode_req == IPS_NONE
		#ifdef CONFIG_CONCURRENT_MODE
		|| padapter->pbuddy_adapter->pwrctrlpriv.ips_mode_req == IPS_NONE
		#endif
	)
		goto exit;

	if (rtw_pwr_unassociated_idle(padapter) == _FALSE)
		goto exit;

	if((pwrpriv->rf_pwrstate == rf_on) && ((pwrpriv->pwr_state_check_cnts%4)==0))
	{
		DBG_871X("==>%s .fw_state(%x)\n",__FUNCTION__,get_fwstate(pmlmepriv));
		#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
		#else
		pwrpriv->change_rfpwrstate = rf_off;
		#endif
		#ifdef CONFIG_AUTOSUSPEND
		if(padapter->registrypriv.usbss_enable)
		{
			if(pwrpriv->bHWPwrPindetect) 
				pwrpriv->bkeepfwalive = _TRUE;
			
			if(padapter->net_closed == _TRUE)
				pwrpriv->ps_flag = _TRUE;

			#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
			if (_TRUE==pwrpriv->bInternalAutoSuspend) {
				DBG_871X("<==%s .pwrpriv->bInternalAutoSuspend)(%x)\n",__FUNCTION__,pwrpriv->bInternalAutoSuspend);
			} else {
				pwrpriv->change_rfpwrstate = rf_off;
				padapter->bCardDisableWOHSM = _TRUE;
				DBG_871X("<==%s .pwrpriv->bInternalAutoSuspend)(%x) call autosuspend_enter\n",__FUNCTION__,pwrpriv->bInternalAutoSuspend);
				autosuspend_enter(padapter);
			}		
			#else
			padapter->bCardDisableWOHSM = _TRUE;
			autosuspend_enter(padapter);
			#endif	//if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
		}		
		else if(pwrpriv->bHWPwrPindetect)
		{
		}
		else
		#endif //CONFIG_AUTOSUSPEND
		{
			#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
			pwrpriv->change_rfpwrstate = rf_off;
			#endif	//defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)

			#ifdef CONFIG_IPS
			ips_enter(padapter);			
			#endif
		}
	}
exit:
	rtw_set_pwr_state_check_timer(&padapter->pwrctrlpriv);
	pwrpriv->ps_processing = _FALSE;
	return;
}

void pwr_state_check_handler(RTW_TIMER_HDL_ARGS);
void pwr_state_check_handler(RTW_TIMER_HDL_ARGS)
{
	_adapter *padapter = (_adapter *)FunctionContext;
	rtw_ps_cmd(padapter);
}


#ifdef CONFIG_LPS
/*
 *
 * Parameters
 *	padapter
 *	pslv			power state level, only could be PS_STATE_S0 ~ PS_STATE_S4
 *
 */
void rtw_set_rpwm(PADAPTER padapter, u8 pslv)
{
	u8	rpwm;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;

_func_enter_;

	pslv = PS_STATE(pslv);


	if (_TRUE == pwrpriv->btcoex_rfon)
	{
		if (pslv < PS_STATE_S4)
			pslv = PS_STATE_S3;
	}

#ifdef CONFIG_LPS_RPWM_TIMER
	if (pwrpriv->brpwmtimeout == _TRUE)
	{
		DBG_871X("%s: RPWM timeout, force to set RPWM(0x%02X) again!\n", __FUNCTION__, pslv);
	}
	else
#endif // CONFIG_LPS_RPWM_TIMER
	{
	if ( (pwrpriv->rpwm == pslv)
#ifdef CONFIG_LPS_LCLK
		|| ((pwrpriv->rpwm >= PS_STATE_S2)&&(pslv >= PS_STATE_S2))
#endif
		)
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,
			("%s: Already set rpwm[0x%02X], new=0x%02X!\n", __FUNCTION__, pwrpriv->rpwm, pslv));
		return;
	}
	}

	if ((padapter->bSurpriseRemoved == _TRUE) ||
		(padapter->hw_init_completed == _FALSE))
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
				 ("%s: SurpriseRemoved(%d) hw_init_completed(%d)\n",
				  __FUNCTION__, padapter->bSurpriseRemoved, padapter->hw_init_completed));

		pwrpriv->cpwm = PS_STATE_S4;

		return;
	}

	if (padapter->bDriverStopped == _TRUE)
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
				 ("%s: change power state(0x%02X) when DriverStopped\n", __FUNCTION__, pslv));

		if (pslv < PS_STATE_S2) {
			RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
					 ("%s: Reject to enter PS_STATE(0x%02X) lower than S2 when DriverStopped!!\n", __FUNCTION__, pslv));
			return;
		}
	}

	rpwm = pslv | pwrpriv->tog;
#ifdef CONFIG_LPS_LCLK
	// only when from PS_STATE S0/S1 to S2 and higher needs ACK
	if ((pwrpriv->cpwm < PS_STATE_S2) && (pslv >= PS_STATE_S2))
		rpwm |= PS_ACK;
#endif
	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("rtw_set_rpwm: rpwm=0x%02x cpwm=0x%02x\n", rpwm, pwrpriv->cpwm));

	pwrpriv->rpwm = pslv;

#ifdef CONFIG_LPS_RPWM_TIMER
	if (rpwm & PS_ACK)
		_set_timer(&pwrpriv->pwr_rpwm_timer, LPS_RPWM_WAIT_MS);
#endif // CONFIG_LPS_RPWM_TIMER
	rtw_hal_set_hwreg(padapter, HW_VAR_SET_RPWM, (u8 *)(&rpwm));

	pwrpriv->tog += 0x80;

#ifdef CONFIG_LPS_LCLK
	// No LPS 32K, No Ack
	if (!(rpwm & PS_ACK))
#endif
	{
		pwrpriv->cpwm = pslv;
	}

_func_exit_;
}

u8 PS_RDY_CHECK(_adapter * padapter);
u8 PS_RDY_CHECK(_adapter * padapter)
{
	u32 curr_time, delta_time;
	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	curr_time = rtw_get_current_time();	

	delta_time = curr_time -pwrpriv->DelayLPSLastTimeStamp;

	if(delta_time < LPS_DELAY_TIME)
	{		
		return _FALSE;
	}

	if ((check_fwstate(pmlmepriv, _FW_LINKED) == _FALSE) ||
		(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE) )
		return _FALSE;
#ifdef CONFIG_WOWLAN
	if(_TRUE == pwrpriv->bInSuspend && pwrpriv->wowlan_mode)
		return _TRUE;
	else
		return _FALSE;
#else
	if(_TRUE == pwrpriv->bInSuspend )
		return _FALSE;
#endif
	if( (padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) && (padapter->securitypriv.binstallGrpkey == _FALSE) )
	{
		DBG_871X("Group handshake still in progress !!!\n");
		return _FALSE;
	}
#ifdef CONFIG_IOCTL_CFG80211
	if (!rtw_cfg80211_pwr_mgmt(padapter))
		return _FALSE;
#endif	

	return _TRUE;
}

void rtw_set_ps_mode(PADAPTER padapter, u8 ps_mode, u8 smart_ps, u8 bcn_ant_mode)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
#endif //CONFIG_P2P
#ifdef CONFIG_TDLS
	struct sta_priv *pstapriv = &padapter->stapriv;
	_irqL irqL;
	int i, j;
	_list	*plist, *phead;
	struct sta_info *ptdls_sta;
#endif //CONFIG_TDLS

_func_enter_;

	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("%s: PowerMode=%d Smart_PS=%d\n",
			  __FUNCTION__, ps_mode, smart_ps));

	if(ps_mode > PM_Card_Disable) {
		RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("ps_mode:%d error\n", ps_mode));
		return;
	}

	if (pwrpriv->pwr_mode == ps_mode)
	{
		if (PS_MODE_ACTIVE == ps_mode) return;

		if ((pwrpriv->smart_ps == smart_ps) &&
			(pwrpriv->bcn_ant_mode == bcn_ant_mode))
		{
			return;
		}
	}

#ifdef CONFIG_LPS_LCLK
	_enter_pwrlock(&pwrpriv->lock);
#endif

	//if(pwrpriv->pwr_mode == PS_MODE_ACTIVE)
	if(ps_mode == PS_MODE_ACTIVE)
	{
#ifdef CONFIG_P2P_PS
		if(pwdinfo->opp_ps == 0)
#endif //CONFIG_P2P_PS
		{
			DBG_871X("rtw_set_ps_mode: Leave 802.11 power save\n");

#ifdef CONFIG_TDLS
			_enter_critical_bh(&pstapriv->sta_hash_lock, &irqL);

			for(i=0; i< NUM_STA; i++)
			{
				phead = &(pstapriv->sta_hash[i]);
				plist = get_next(phead);

				while ((rtw_end_of_queue_search(phead, plist)) == _FALSE)
				{
					ptdls_sta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

					if( ptdls_sta->tdls_sta_state & TDLS_LINKED_STATE )
						issue_nulldata_to_TDLS_peer_STA(padapter, ptdls_sta, 0);
					plist = get_next(plist);
				}
			}

			_exit_critical_bh(&pstapriv->sta_hash_lock, &irqL);
#endif //CONFIG_TDLS

			pwrpriv->pwr_mode = ps_mode;
			rtw_set_rpwm(padapter, PS_STATE_S4);
			rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_PWRMODE, (u8 *)(&ps_mode));
			pwrpriv->bFwCurrentInPSMode = _FALSE;
		}
	}
	else
	{
		if (PS_RDY_CHECK(padapter)
#ifdef CONFIG_BT_COEXIST
			|| (BT_1Ant(padapter) == _TRUE)
#endif
			)
		{
			DBG_871X("%s: Enter 802.11 power save\n", __FUNCTION__);

#ifdef CONFIG_TDLS
			_enter_critical_bh(&pstapriv->sta_hash_lock, &irqL);

			for(i=0; i< NUM_STA; i++)
			{
				phead = &(pstapriv->sta_hash[i]);
				plist = get_next(phead);

				while ((rtw_end_of_queue_search(phead, plist)) == _FALSE)
				{
					ptdls_sta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

					if( ptdls_sta->tdls_sta_state & TDLS_LINKED_STATE )
						issue_nulldata_to_TDLS_peer_STA(padapter, ptdls_sta, 1);
					plist = get_next(plist);
				}
			}

			_exit_critical_bh(&pstapriv->sta_hash_lock, &irqL);
#endif //CONFIG_TDLS

			pwrpriv->bFwCurrentInPSMode = _TRUE;
			pwrpriv->pwr_mode = ps_mode;
			pwrpriv->smart_ps = smart_ps;
			pwrpriv->bcn_ant_mode = bcn_ant_mode;
			rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_PWRMODE, (u8 *)(&ps_mode));

#ifdef CONFIG_P2P_PS
			// Set CTWindow after LPS
			if(pwdinfo->opp_ps == 1)
				p2p_ps_wk_cmd(padapter, P2P_PS_ENABLE, 0);
#endif //CONFIG_P2P_PS

#ifdef CONFIG_LPS_LCLK
			if (pwrpriv->alives == 0)
				rtw_set_rpwm(padapter, PS_STATE_S0);
#else
			rtw_set_rpwm(padapter, PS_STATE_S2);
#endif
		}
	}

#ifdef CONFIG_LPS_LCLK
	_exit_pwrlock(&pwrpriv->lock);
#endif

_func_exit_;
}

/*
 * Return:
 *	0:	Leave OK
 *	-1:	Timeout
 *	-2:	Other error
 */
s32 LPS_RF_ON_check(PADAPTER padapter, u32 delay_ms)
{
	u32 start_time;
	u8 bAwake = _FALSE;
	s32 err = 0;


	start_time = rtw_get_current_time();
	while (1)
	{
		rtw_hal_get_hwreg(padapter, HW_VAR_FWLPS_RF_ON, &bAwake);
		if (_TRUE == bAwake)
			break;

		if (_TRUE == padapter->bSurpriseRemoved)
		{
			err = -2;
			DBG_871X("%s: device surprise removed!!\n", __FUNCTION__);
			break;
		}

		if (rtw_get_passing_time_ms(start_time) > delay_ms)
		{
			err = -1;
			DBG_871X("%s: Wait for FW LPS leave more than %u ms!!!\n", __FUNCTION__, delay_ms);
			break;
		}
		rtw_usleep_os(100);
	}

	return err;
}

//
//	Description:
//		Enter the leisure power save mode.
//
void LPS_Enter(PADAPTER padapter)
{
	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	//struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	//_adapter *buddy = padapter->pbuddy_adapter;

_func_enter_;

//	DBG_871X("+LeisurePSEnter\n");

#ifdef CONFIG_CONCURRENT_MODE
	if (padapter->iface_type != IFACE_PORT0)
		return; /* Skip power saving for concurrent mode port 1*/

	/* consider buddy, if exist */
	if (buddy) {
		struct mlme_priv *b_pmlmepriv = &(buddy->mlmepriv);
		#ifdef CONFIG_P2P
		struct wifidirect_info *b_pwdinfo = &(buddy->wdinfo);
		#ifdef CONFIG_IOCTL_CFG80211		
		struct cfg80211_wifidirect_info *b_pcfg80211_wdinfo = &buddy->cfg80211_wdinfo;
		#endif
		#endif
		
		if (check_fwstate(b_pmlmepriv, WIFI_ASOC_STATE|WIFI_SITE_MONITOR)
			|| check_fwstate(b_pmlmepriv, WIFI_UNDER_LINKING|WIFI_UNDER_WPS)
			|| check_fwstate(b_pmlmepriv, WIFI_AP_STATE)
			|| check_fwstate(b_pmlmepriv, WIFI_ADHOC_MASTER_STATE|WIFI_ADHOC_STATE)
			#if defined(CONFIG_P2P) && defined(CONFIG_IOCTL_CFG80211) && defined(CONFIG_P2P_IPS)
			|| b_pcfg80211_wdinfo->is_ro_ch
			#elif defined(CONFIG_P2P)
			|| !rtw_p2p_chk_state(b_pwdinfo, P2P_STATE_NONE)
			#endif
			|| rtw_is_scan_deny(buddy)
		) {
			return;
		}
	}
#endif

	if (PS_RDY_CHECK(padapter) == _FALSE)
		return;

	if (pwrpriv->bLeisurePs)
	{
		// Idle for a while if we connect to AP a while ago.
		if(pwrpriv->LpsIdleCount >= 2) //  4 Sec 
		{
			if(pwrpriv->pwr_mode == PS_MODE_ACTIVE)
			{
				pwrpriv->bpower_saving = _TRUE;
				rtw_set_ps_mode(padapter, pwrpriv->power_mgnt, padapter->registrypriv.smart_ps, 0);
			}
		}
		else
			pwrpriv->LpsIdleCount++;
	}

//	DBG_871X("-LeisurePSEnter\n");

_func_exit_;
}

//
//	Description:
//		Leave the leisure power save mode.
//
void LPS_Leave(PADAPTER padapter)
{
#define LPS_LEAVE_TIMEOUT_MS 100

	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	//u32 start_time;
	//u8 bAwake = _FALSE;

_func_enter_;

#ifdef CONFIG_CONCURRENT_MODE
	if (padapter->iface_type != IFACE_PORT0)
		return; /* Skip power saving for concurrent mode port 1*/
#endif

//	DBG_871X("+LeisurePSLeave\n");

	if (pwrpriv->bLeisurePs)
	{
		if(pwrpriv->pwr_mode != PS_MODE_ACTIVE)
		{
			rtw_set_ps_mode(padapter, PS_MODE_ACTIVE, 0, 0);

			if(pwrpriv->pwr_mode == PS_MODE_ACTIVE)
				LPS_RF_ON_check(padapter, LPS_LEAVE_TIMEOUT_MS);
		}
	}

	pwrpriv->bpower_saving = _FALSE;

//	DBG_871X("-LeisurePSLeave\n");

_func_exit_;
}
#endif

//
// Description: Leave all power save mode: LPS, FwLPS, IPS if needed.
// Move code to function by tynli. 2010.03.26. 
//
void LeaveAllPowerSaveMode(IN PADAPTER Adapter)
{
	struct mlme_priv	*pmlmepriv = &(Adapter->mlmepriv);
	u8	enqueue = 0;

_func_enter_;

	//DBG_871X("%s.....\n",__FUNCTION__);
	if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE)
	{ //connect
#ifdef CONFIG_LPS_LCLK
		enqueue = 1;
#endif

#ifdef CONFIG_P2P_PS
		p2p_ps_wk_cmd(Adapter, P2P_PS_DISABLE, enqueue);
#endif //CONFIG_P2P_PS

#ifdef CONFIG_LPS
		rtw_lps_ctrl_wk_cmd(Adapter, LPS_CTRL_LEAVE, enqueue);
#endif

#ifdef CONFIG_LPS_LCLK
		LPS_Leave_check(Adapter);
#endif	
	}
	else
	{
		if(Adapter->pwrctrlpriv.rf_pwrstate== rf_off)
		{
			#ifdef CONFIG_AUTOSUSPEND
			if(Adapter->registrypriv.usbss_enable)
			{
				#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
				usb_disable_autosuspend(adapter_to_dvobj(Adapter)->pusbdev);
				#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22) && LINUX_VERSION_CODE<=KERNEL_VERSION(2,6,34))
				adapter_to_dvobj(Adapter)->pusbdev->autosuspend_disabled = Adapter->bDisableAutosuspend;//autosuspend disabled by the user
				#endif
			}
			else
			#endif
			{
#if defined(CONFIG_PLATFORM_SPRD) && defined(CONFIG_RTL8188E)
				#ifdef CONFIG_IPS
				if(_FALSE == ips_leave(Adapter))
				{
					DBG_871X("======> ips_leave fail.............\n");			
				}
				#endif
#endif //CONFIG_PLATFORM_SPRD && CONFIG_RTL8188E
			}				
		}	
	}

_func_exit_;
}

#ifdef CONFIG_LPS_LCLK
void LPS_Leave_check(
	PADAPTER padapter)
{
	struct pwrctrl_priv *pwrpriv;
	u32	start_time;
	u8	bReady;

_func_enter_;

	pwrpriv = &padapter->pwrctrlpriv;

	bReady = _FALSE;
	start_time = rtw_get_current_time();

	rtw_yield_os();
	
	while(1)
	{
		_enter_pwrlock(&pwrpriv->lock);

		if ((padapter->bSurpriseRemoved == _TRUE)
			|| (padapter->hw_init_completed == _FALSE)
#ifdef CONFIG_USB_HCI
			|| (padapter->bDriverStopped== _TRUE)
#endif
			|| (pwrpriv->pwr_mode == PS_MODE_ACTIVE)
			)
		{
			bReady = _TRUE;
		}

		_exit_pwrlock(&pwrpriv->lock);

		if(_TRUE == bReady)
			break;

		if(rtw_get_passing_time_ms(start_time)>100)
		{
			DBG_871X("Wait for cpwm event  than 100 ms!!!\n");
			break;
		}
		rtw_msleep_os(1);
	}

_func_exit_;
}

/*
 * Caller:ISR handler...
 *
 * This will be called when CPWM interrupt is up.
 *
 * using to update cpwn of drv; and drv willl make a decision to up or down pwr level
 */
void cpwm_int_hdl(
	PADAPTER padapter,
	struct reportpwrstate_parm *preportpwrstate)
{
	struct pwrctrl_priv *pwrpriv;

_func_enter_;

	pwrpriv = &padapter->pwrctrlpriv;
#if 0
	if (pwrpriv->cpwm_tog == (preportpwrstate->state & PS_TOGGLE)) {
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
				 ("cpwm_int_hdl: tog(old)=0x%02x cpwm(new)=0x%02x toggle bit didn't change!?\n",
				  pwrpriv->cpwm_tog, preportpwrstate->state));
		goto exit;
	}
#endif

	_enter_pwrlock(&pwrpriv->lock);

#ifdef CONFIG_LPS_RPWM_TIMER
	if (pwrpriv->rpwm < PS_STATE_S2)
	{
		DBG_871X("%s: Redundant CPWM Int. RPWM=0x%02X CPWM=0x%02x\n", __func__, pwrpriv->rpwm, pwrpriv->cpwm);
		_exit_pwrlock(&pwrpriv->lock);
		goto exit;
	}
#endif // CONFIG_LPS_RPWM_TIMER

	pwrpriv->cpwm = PS_STATE(preportpwrstate->state);
	pwrpriv->cpwm_tog = preportpwrstate->state & PS_TOGGLE;

	if (pwrpriv->cpwm >= PS_STATE_S2)
	{
		if (pwrpriv->alives & CMD_ALIVE)
			_rtw_up_sema(&padapter->cmdpriv.cmd_queue_sema);

		if (pwrpriv->alives & XMIT_ALIVE)
			_rtw_up_sema(&padapter->xmitpriv.xmit_sema);
	}

	_exit_pwrlock(&pwrpriv->lock);

exit:
	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("cpwm_int_hdl: cpwm=0x%02x\n", pwrpriv->cpwm));

_func_exit_;
}

static void cpwm_event_callback(struct work_struct *work)
{
	struct pwrctrl_priv *pwrpriv = container_of(work, struct pwrctrl_priv, cpwm_event);
	_adapter *adapter = container_of(pwrpriv, _adapter, pwrctrlpriv);
	struct reportpwrstate_parm report;

	//DBG_871X("%s\n",__FUNCTION__);

	report.state = PS_STATE_S2;
	cpwm_int_hdl(adapter, &report);
}

#ifdef CONFIG_LPS_RPWM_TIMER
static void rpwmtimeout_workitem_callback(struct work_struct *work)
{
	PADAPTER padapter;
	struct pwrctrl_priv *pwrpriv;


	pwrpriv = container_of(work, struct pwrctrl_priv, rpwmtimeoutwi);
	padapter = container_of(pwrpriv, _adapter, pwrctrlpriv);
//	DBG_871X("+%s: rpwm=0x%02X cpwm=0x%02X\n", __func__, pwrpriv->rpwm, pwrpriv->cpwm);

	_enter_pwrlock(&pwrpriv->lock);
	if ((pwrpriv->rpwm == pwrpriv->cpwm) || (pwrpriv->cpwm >= PS_STATE_S2))
	{
		DBG_871X("%s: rpwm=0x%02X cpwm=0x%02X CPWM done!\n", __func__, pwrpriv->rpwm, pwrpriv->cpwm);
		goto exit;
	}
	_exit_pwrlock(&pwrpriv->lock);

	if (rtw_read8(padapter, 0x100) != 0xEA)
	{
#if 1
		struct reportpwrstate_parm report;

		report.state = PS_STATE_S2;
		DBG_871X("\n%s: FW already leave 32K!\n\n", __func__);
		cpwm_int_hdl(padapter, &report);
#else
		DBG_871X("\n%s: FW already leave 32K!\n\n", __func__);
		cpwm_event_callback(&pwrpriv->cpwm_event);
#endif
		return;
	}

	_enter_pwrlock(&pwrpriv->lock);

	if ((pwrpriv->rpwm == pwrpriv->cpwm) || (pwrpriv->cpwm >= PS_STATE_S2))
	{
		DBG_871X("%s: cpwm=%d, nothing to do!\n", __func__, pwrpriv->cpwm);
		goto exit;
	}
	pwrpriv->brpwmtimeout = _TRUE;
	rtw_set_rpwm(padapter, pwrpriv->rpwm);
	pwrpriv->brpwmtimeout = _FALSE;

exit:
	_exit_pwrlock(&pwrpriv->lock);
}

/*
 * This function is a timer handler, can't do any IO in it.
 */
static void pwr_rpwm_timeout_handler(void *FunctionContext)
{
	PADAPTER padapter;
	struct pwrctrl_priv *pwrpriv;


	padapter = (PADAPTER)FunctionContext;
	pwrpriv = &padapter->pwrctrlpriv;
//	DBG_871X("+%s: rpwm=0x%02X cpwm=0x%02X\n", __func__, pwrpriv->rpwm, pwrpriv->cpwm);

	if ((pwrpriv->rpwm == pwrpriv->cpwm) || (pwrpriv->cpwm >= PS_STATE_S2))
	{
		DBG_871X("+%s: cpwm=%d, nothing to do!\n", __func__, pwrpriv->cpwm);
		return;
	}

	_set_workitem(&pwrpriv->rpwmtimeoutwi);
}
#endif // CONFIG_LPS_RPWM_TIMER

__inline static void register_task_alive(struct pwrctrl_priv *pwrctrl, u32 tag)
{
	pwrctrl->alives |= tag;
}

__inline static void unregister_task_alive(struct pwrctrl_priv *pwrctrl, u32 tag)
{
	pwrctrl->alives &= ~tag;
}

/*
 * Caller: rtw_xmit_thread
 * 
 * Check if the fw_pwrstate is okay for xmit.
 * If not (cpwm is less than S3), then the sub-routine
 * will raise the cpwm to be greater than or equal to S3. 
 *
 * Calling Context: Passive
 * 
 * Return Value:
 *	 _SUCCESS	rtw_xmit_thread can write fifo/txcmd afterwards.
 *	 _FAIL		rtw_xmit_thread can not do anything.
 */
s32 rtw_register_tx_alive(PADAPTER padapter)
{
	s32 res;
	struct pwrctrl_priv *pwrctrl;
	u8 pslv;

_func_enter_;

	res = _SUCCESS;
	pwrctrl = &padapter->pwrctrlpriv;
#ifdef CONFIG_BT_COEXIST
	if (_TRUE == pwrctrl->btcoex_rfon)
		pslv = PS_STATE_S3;
	else
#endif
	{
		pslv = PS_STATE_S2;
	}

	_enter_pwrlock(&pwrctrl->lock);

	register_task_alive(pwrctrl, XMIT_ALIVE);

	if (pwrctrl->bFwCurrentInPSMode == _TRUE)
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
				 ("rtw_register_tx_alive: cpwm=0x%02x alives=0x%08x\n",
				  pwrctrl->cpwm, pwrctrl->alives));

		if (pwrctrl->cpwm < pslv)
		{
			if (pwrctrl->cpwm < PS_STATE_S2)
				res = _FAIL;
			if (pwrctrl->rpwm < pslv)
				rtw_set_rpwm(padapter, pslv);
		}
	}

	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;

	return res;	
}

/*
 * Caller: rtw_cmd_thread
 *
 * Check if the fw_pwrstate is okay for issuing cmd.
 * If not (cpwm should be is less than S2), then the sub-routine
 * will raise the cpwm to be greater than or equal to S2.
 *
 * Calling Context: Passive
 *
 * Return Value:
 *	_SUCCESS	rtw_cmd_thread can issue cmds to firmware afterwards.
 *	_FAIL		rtw_cmd_thread can not do anything.
 */
s32 rtw_register_cmd_alive(PADAPTER padapter)
{
	s32 res;
	struct pwrctrl_priv *pwrctrl;
	u8 pslv;

_func_enter_;

	res = _SUCCESS;
	pwrctrl = &padapter->pwrctrlpriv;
#ifdef CONFIG_BT_COEXIST
	if (_TRUE == pwrctrl->btcoex_rfon)
		pslv = PS_STATE_S3;
	else
#endif
	{
		pslv = PS_STATE_S2;
	}

	_enter_pwrlock(&pwrctrl->lock);

	register_task_alive(pwrctrl, CMD_ALIVE);

	if (pwrctrl->bFwCurrentInPSMode == _TRUE)
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_info_,
				 ("rtw_register_cmd_alive: cpwm=0x%02x alives=0x%08x\n",
				  pwrctrl->cpwm, pwrctrl->alives));

		if (pwrctrl->cpwm < pslv)
		{
			if (pwrctrl->cpwm < PS_STATE_S2)
				res = _FAIL;
			if (pwrctrl->rpwm < pslv)
				rtw_set_rpwm(padapter, pslv);
		}
	}

	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;

	return res;
}

/*
 * Caller: rx_isr
 *
 * Calling Context: Dispatch/ISR
 *
 * Return Value:
 *	_SUCCESS
 *	_FAIL
 */
s32 rtw_register_rx_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;

_func_enter_;

	pwrctrl = &padapter->pwrctrlpriv;

	_enter_pwrlock(&pwrctrl->lock);

	register_task_alive(pwrctrl, RECV_ALIVE);
	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("rtw_register_rx_alive: cpwm=0x%02x alives=0x%08x\n",
			  pwrctrl->cpwm, pwrctrl->alives));

	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;

	return _SUCCESS;
}

/*
 * Caller: evt_isr or evt_thread
 *
 * Calling Context: Dispatch/ISR or Passive
 *
 * Return Value:
 *	_SUCCESS
 *	_FAIL
 */
s32 rtw_register_evt_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;

_func_enter_;

	pwrctrl = &padapter->pwrctrlpriv;

	_enter_pwrlock(&pwrctrl->lock);

	register_task_alive(pwrctrl, EVT_ALIVE);
	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("rtw_register_evt_alive: cpwm=0x%02x alives=0x%08x\n",
			  pwrctrl->cpwm, pwrctrl->alives));

	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;

	return _SUCCESS;
}

/*
 * Caller: ISR
 *
 * If ISR's txdone,
 * No more pkts for TX,
 * Then driver shall call this fun. to power down firmware again.
 */
void rtw_unregister_tx_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;

_func_enter_;

	pwrctrl = &padapter->pwrctrlpriv;

	_enter_pwrlock(&pwrctrl->lock);

	unregister_task_alive(pwrctrl, XMIT_ALIVE);

	if ((pwrctrl->pwr_mode != PS_MODE_ACTIVE) &&
		(pwrctrl->bFwCurrentInPSMode == _TRUE))
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
				 ("%s: cpwm=0x%02x alives=0x%08x\n",
				  __FUNCTION__, pwrctrl->cpwm, pwrctrl->alives));

		if ((pwrctrl->alives == 0) &&
			(pwrctrl->cpwm > PS_STATE_S0))
		{
			rtw_set_rpwm(padapter, PS_STATE_S0);
		}
	}

	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;
}

/*
 * Caller: ISR
 *
 * If all commands have been done,
 * and no more command to do,
 * then driver shall call this fun. to power down firmware again.
 */
void rtw_unregister_cmd_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;

_func_enter_;

	pwrctrl = &padapter->pwrctrlpriv;

	_enter_pwrlock(&pwrctrl->lock);

	unregister_task_alive(pwrctrl, CMD_ALIVE);

	if ((pwrctrl->pwr_mode != PS_MODE_ACTIVE) &&
		(pwrctrl->bFwCurrentInPSMode == _TRUE))
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_info_,
				 ("%s: cpwm=0x%02x alives=0x%08x\n",
				  __FUNCTION__, pwrctrl->cpwm, pwrctrl->alives));

		if ((pwrctrl->alives == 0) &&
			(pwrctrl->cpwm > PS_STATE_S0))
		{
			rtw_set_rpwm(padapter, PS_STATE_S0);
		}
	}

	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;
}

/*
 * Caller: ISR
 */
void rtw_unregister_rx_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;

_func_enter_;

	pwrctrl = &padapter->pwrctrlpriv;

	_enter_pwrlock(&pwrctrl->lock);

	unregister_task_alive(pwrctrl, RECV_ALIVE);

	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("rtw_unregister_rx_alive: cpwm=0x%02x alives=0x%08x\n",
			  pwrctrl->cpwm, pwrctrl->alives));

	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;
}

void rtw_unregister_evt_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;

_func_enter_;

	pwrctrl = &padapter->pwrctrlpriv;

	unregister_task_alive(pwrctrl, EVT_ALIVE);

	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("rtw_unregister_evt_alive: cpwm=0x%02x alives=0x%08x\n",
			  pwrctrl->cpwm, pwrctrl->alives));

	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;
}
#endif	/* CONFIG_LPS_LCLK */

#ifdef CONFIG_RESUME_IN_WORKQUEUE
static void resume_workitem_callback(struct work_struct *work);
#endif //CONFIG_RESUME_IN_WORKQUEUE

void rtw_init_pwrctrl_priv(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

_func_enter_;

#ifdef PLATFORM_WINDOWS
	pwrctrlpriv->pnp_current_pwr_state=NdisDeviceStateD0;
#endif

	_init_pwrlock(&pwrctrlpriv->lock);
	pwrctrlpriv->rf_pwrstate = rf_on;
	pwrctrlpriv->ips_enter_cnts=0;
	pwrctrlpriv->ips_leave_cnts=0;
	pwrctrlpriv->bips_processing = _FALSE;

	pwrctrlpriv->ips_mode = padapter->registrypriv.ips_mode;
	pwrctrlpriv->ips_mode_req = padapter->registrypriv.ips_mode;

	pwrctrlpriv->pwr_state_check_interval = RTW_PWR_STATE_CHK_INTERVAL;
	pwrctrlpriv->pwr_state_check_cnts = 0;
	pwrctrlpriv->bInternalAutoSuspend = _FALSE;
	pwrctrlpriv->bInSuspend = _FALSE;
	pwrctrlpriv->bkeepfwalive = _FALSE;

#ifdef CONFIG_AUTOSUSPEND
#ifdef SUPPORT_HW_RFOFF_DETECTED
	pwrctrlpriv->pwr_state_check_interval = (pwrctrlpriv->bHWPwrPindetect) ?1000:2000;		
#endif
#endif

	pwrctrlpriv->LpsIdleCount = 0;
	//pwrctrlpriv->FWCtrlPSMode =padapter->registrypriv.power_mgnt;// PS_MODE_MIN;
	if (padapter->registrypriv.mp_mode == 1)
		pwrctrlpriv->power_mgnt =PS_MODE_ACTIVE ;
	else	
		pwrctrlpriv->power_mgnt =padapter->registrypriv.power_mgnt;// PS_MODE_MIN;
	pwrctrlpriv->bLeisurePs = (PS_MODE_ACTIVE != pwrctrlpriv->power_mgnt)?_TRUE:_FALSE;

	pwrctrlpriv->bFwCurrentInPSMode = _FALSE;

	pwrctrlpriv->rpwm = 0;
	pwrctrlpriv->cpwm = PS_STATE_S4;

	pwrctrlpriv->pwr_mode = PS_MODE_ACTIVE;
	pwrctrlpriv->smart_ps = padapter->registrypriv.smart_ps;
	pwrctrlpriv->bcn_ant_mode = 0;

	pwrctrlpriv->tog = 0x80;

	pwrctrlpriv->btcoex_rfon = _FALSE;

#ifdef CONFIG_LPS_LCLK
	rtw_hal_set_hwreg(padapter, HW_VAR_SET_RPWM, (u8 *)(&pwrctrlpriv->rpwm));

	_init_workitem(&pwrctrlpriv->cpwm_event, cpwm_event_callback, NULL);

#ifdef CONFIG_LPS_RPWM_TIMER
	pwrctrlpriv->brpwmtimeout = _FALSE;
	_init_workitem(&pwrctrlpriv->rpwmtimeoutwi, rpwmtimeout_workitem_callback, NULL);
	_init_timer(&pwrctrlpriv->pwr_rpwm_timer, padapter->pnetdev, pwr_rpwm_timeout_handler, padapter);
#endif // CONFIG_LPS_RPWM_TIMER
#endif // CONFIG_LPS_LCLK

	rtw_init_timer(&pwrctrlpriv->pwr_state_check_timer, padapter, pwr_state_check_handler);

	#ifdef CONFIG_RESUME_IN_WORKQUEUE
	_init_workitem(&pwrctrlpriv->resume_work, resume_workitem_callback, NULL);
	pwrctrlpriv->rtw_workqueue = create_singlethread_workqueue("rtw_workqueue");
	#endif //CONFIG_RESUME_IN_WORKQUEUE

	#if defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_ANDROID_POWER)
	pwrctrlpriv->early_suspend.suspend = NULL;
	rtw_register_early_suspend(pwrctrlpriv);
	#endif //CONFIG_HAS_EARLYSUSPEND || CONFIG_ANDROID_POWER


_func_exit_;

}


void rtw_free_pwrctrl_priv(PADAPTER adapter)
{
	struct pwrctrl_priv *pwrctrlpriv = &adapter->pwrctrlpriv;

_func_enter_;

	//_rtw_memset((unsigned char *)pwrctrlpriv, 0, sizeof(struct pwrctrl_priv));


	#ifdef CONFIG_RESUME_IN_WORKQUEUE
	if (pwrctrlpriv->rtw_workqueue) { 
		flush_workqueue(pwrctrlpriv->rtw_workqueue);
		destroy_workqueue(pwrctrlpriv->rtw_workqueue);
	}
	#endif


	#if defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_ANDROID_POWER)
	rtw_unregister_early_suspend(pwrctrlpriv);
	#endif //CONFIG_HAS_EARLYSUSPEND || CONFIG_ANDROID_POWER

	_free_pwrlock(&pwrctrlpriv->lock);

_func_exit_;
}

#ifdef CONFIG_RESUME_IN_WORKQUEUE
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
extern int rtw_resume_process(_adapter *padapter);
#endif
static void resume_workitem_callback(struct work_struct *work)
{
	struct pwrctrl_priv *pwrpriv = container_of(work, struct pwrctrl_priv, resume_work);
	_adapter *adapter = container_of(pwrpriv, _adapter, pwrctrlpriv);

	DBG_871X("%s\n",__FUNCTION__);

	#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
	rtw_resume_process(adapter);
	#endif
	
}

void rtw_resume_in_workqueue(struct pwrctrl_priv *pwrpriv)
{
	// accquire system's suspend lock preventing from falliing asleep while resume in workqueue
	rtw_lock_suspend();
	
	#if 1
	queue_work(pwrpriv->rtw_workqueue, &pwrpriv->resume_work);	
	#else
	_set_workitem(&pwrpriv->resume_work);
	#endif
}
#endif //CONFIG_RESUME_IN_WORKQUEUE

#if defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_ANDROID_POWER)
inline bool rtw_is_earlysuspend_registered(struct pwrctrl_priv *pwrpriv)
{
	return (pwrpriv->early_suspend.suspend) ? _TRUE : _FALSE;
}

inline bool rtw_is_do_late_resume(struct pwrctrl_priv *pwrpriv)
{
	return (pwrpriv->do_late_resume) ? _TRUE : _FALSE;
}

inline void rtw_set_do_late_resume(struct pwrctrl_priv *pwrpriv, bool enable)
{
	pwrpriv->do_late_resume = enable;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
extern int rtw_resume_process(_adapter *padapter);
#endif
static void rtw_early_suspend(struct early_suspend *h)
{
	struct pwrctrl_priv *pwrpriv = container_of(h, struct pwrctrl_priv, early_suspend);
	DBG_871X("%s\n",__FUNCTION__);

	rtw_set_do_late_resume(pwrpriv, _FALSE);
}

static void rtw_late_resume(struct early_suspend *h)
{
	struct pwrctrl_priv *pwrpriv = container_of(h, struct pwrctrl_priv, early_suspend);
	_adapter *adapter = container_of(pwrpriv, _adapter, pwrctrlpriv);

	DBG_871X("%s\n",__FUNCTION__);
	if(pwrpriv->do_late_resume) {
		#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
		rtw_set_do_late_resume(pwrpriv, _FALSE);
		rtw_resume_process(adapter);
		#endif
	}
}

void rtw_register_early_suspend(struct pwrctrl_priv *pwrpriv)
{
	_adapter *adapter = container_of(pwrpriv, _adapter, pwrctrlpriv);

#if defined(CONFIG_CONCURRENT_MODE)
	if (adapter->adapter_type != PRIMARY_ADAPTER)
		return;
#endif	

	DBG_871X("%s\n", __FUNCTION__);

	//jeff: set the early suspend level before blank screen, so we wll do late resume after scree is lit
	pwrpriv->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 20;
	pwrpriv->early_suspend.suspend = rtw_early_suspend;
	pwrpriv->early_suspend.resume = rtw_late_resume;
	register_early_suspend(&pwrpriv->early_suspend);	

	
}

void rtw_unregister_early_suspend(struct pwrctrl_priv *pwrpriv)
{
	_adapter *adapter = container_of(pwrpriv, _adapter, pwrctrlpriv);

#if defined(CONFIG_CONCURRENT_MODE)
	if (adapter->adapter_type != PRIMARY_ADAPTER)
		return;
#endif

	DBG_871X("%s\n", __FUNCTION__);

	rtw_set_do_late_resume(pwrpriv, _FALSE);

	if (pwrpriv->early_suspend.suspend) 
		unregister_early_suspend(&pwrpriv->early_suspend);

	pwrpriv->early_suspend.suspend = NULL;
	pwrpriv->early_suspend.resume = NULL;
}
#endif //CONFIG_HAS_EARLYSUSPEND

#ifdef CONFIG_ANDROID_POWER
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
extern int rtw_resume_process(PADAPTER padapter);
#endif
static void rtw_early_suspend(android_early_suspend_t *h)
{
	struct pwrctrl_priv *pwrpriv = container_of(h, struct pwrctrl_priv, early_suspend);
	DBG_871X("%s\n",__FUNCTION__);

	rtw_set_do_late_resume(pwrpriv, _FALSE);
}

static void rtw_late_resume(android_early_suspend_t *h)
{
	struct pwrctrl_priv *pwrpriv = container_of(h, struct pwrctrl_priv, early_suspend);
	_adapter *adapter = container_of(pwrpriv, _adapter, pwrctrlpriv);

	DBG_871X("%s\n",__FUNCTION__);
	if(pwrpriv->do_late_resume) {
		#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
		rtw_set_do_late_resume(pwrpriv, _FALSE);
		rtw_resume_process(adapter);
		#endif
	}
}

void rtw_register_early_suspend(struct pwrctrl_priv *pwrpriv)
{
	_adapter *adapter = container_of(pwrpriv, _adapter, pwrctrlpriv);

#if defined(CONFIG_CONCURRENT_MODE)
	if (adapter->adapter_type != PRIMARY_ADAPTER)
		return;
#endif

	DBG_871X("%s\n", __FUNCTION__);

	//jeff: set the early suspend level before blank screen, so we wll do late resume after scree is lit
	pwrpriv->early_suspend.level = ANDROID_EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 20;
	pwrpriv->early_suspend.suspend = rtw_early_suspend;
	pwrpriv->early_suspend.resume = rtw_late_resume;
	android_register_early_suspend(&pwrpriv->early_suspend);	
}

void rtw_unregister_early_suspend(struct pwrctrl_priv *pwrpriv)
{
	_adapter *adapter = container_of(pwrpriv, _adapter, pwrctrlpriv);

#if defined(CONFIG_CONCURRENT_MODE)
	if (adapter->adapter_type != PRIMARY_ADAPTER)
		return;
#endif

	DBG_871X("%s\n", __FUNCTION__);

	rtw_set_do_late_resume(pwrpriv, _FALSE);

	if (pwrpriv->early_suspend.suspend) 
		android_unregister_early_suspend(&pwrpriv->early_suspend);

	pwrpriv->early_suspend.suspend = NULL;
	pwrpriv->early_suspend.resume = NULL;
}
#endif //CONFIG_ANDROID_POWER

u8 rtw_interface_ps_func(_adapter *padapter,HAL_INTF_PS_FUNC efunc_id,u8* val)
{
	u8 bResult = _TRUE;
	rtw_hal_intf_ps_func(padapter,efunc_id,val);
	
	return bResult;
}


inline void rtw_set_ips_deny(_adapter *padapter, u32 ms)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	pwrpriv->ips_deny_time = rtw_get_current_time() + rtw_ms_to_systime(ms);
}

/*
* rtw_pwr_wakeup - Wake the NIC up from: 1)IPS. 2)USB autosuspend
* @adapter: pointer to _adapter structure
* @ips_deffer_ms: the ms wiil prevent from falling into IPS after wakeup
* Return _SUCCESS or _FAIL
*/

int _rtw_pwr_wakeup(_adapter *padapter, u32 ips_deffer_ms, const char *caller)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;	
	int ret = _SUCCESS;
	u32 start = rtw_get_current_time();

#ifdef CONFIG_CONCURRENT_MODE
	if (padapter->pbuddy_adapter)
		LeaveAllPowerSaveMode(padapter->pbuddy_adapter);

	if ((padapter->isprimary == _FALSE) && padapter->pbuddy_adapter){
		padapter = padapter->pbuddy_adapter;
		pwrpriv = &padapter->pwrctrlpriv;
		pmlmepriv = &padapter->mlmepriv;
	}
#endif

	if (pwrpriv->ips_deny_time < rtw_get_current_time() + rtw_ms_to_systime(ips_deffer_ms))
		pwrpriv->ips_deny_time = rtw_get_current_time() + rtw_ms_to_systime(ips_deffer_ms);


	if (pwrpriv->ps_processing) {
		DBG_871X("%s wait ps_processing...\n", __func__);
		while (pwrpriv->ps_processing && rtw_get_passing_time_ms(start) <= 3000)
			rtw_msleep_os(10);
		if (pwrpriv->ps_processing)
			DBG_871X("%s wait ps_processing timeout\n", __func__);
		else
			DBG_871X("%s wait ps_processing done\n", __func__);
	}

#ifdef DBG_CONFIG_ERROR_DETECT
	if (rtw_hal_sreset_inprogress(padapter)) {
		DBG_871X("%s wait sreset_inprogress...\n", __func__);
		while (rtw_hal_sreset_inprogress(padapter) && rtw_get_passing_time_ms(start) <= 4000)
			rtw_msleep_os(10);
		if (rtw_hal_sreset_inprogress(padapter))
			DBG_871X("%s wait sreset_inprogress timeout\n", __func__);
		else
			DBG_871X("%s wait sreset_inprogress done\n", __func__);
	}
#endif

	if (pwrpriv->bInternalAutoSuspend == _FALSE && pwrpriv->bInSuspend) {
		DBG_871X("%s wait bInSuspend...\n", __func__);
		while (pwrpriv->bInSuspend 
			&& ((rtw_get_passing_time_ms(start) <= 3000 && !rtw_is_do_late_resume(pwrpriv))
				|| (rtw_get_passing_time_ms(start) <= 500 && rtw_is_do_late_resume(pwrpriv)))
		) {
			rtw_msleep_os(10);
		}
		if (pwrpriv->bInSuspend)
			DBG_871X("%s wait bInSuspend timeout\n", __func__);
		else
			DBG_871X("%s wait bInSuspend done\n", __func__);
	}

	//System suspend is not allowed to wakeup
	if((pwrpriv->bInternalAutoSuspend == _FALSE) && (_TRUE == pwrpriv->bInSuspend )){
		ret = _FAIL;
		goto exit;
	}

	//block???
	if((pwrpriv->bInternalAutoSuspend == _TRUE)  && (padapter->net_closed == _TRUE)) {
		ret = _FAIL;
		goto exit;
	}

	//I think this should be check in IPS, LPS, autosuspend functions...
	if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE)
	{
#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
		if(_TRUE==pwrpriv->bInternalAutoSuspend){
			if(0==pwrpriv->autopm_cnt){
			#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))				
				if (usb_autopm_get_interface(adapter_to_dvobj(padapter)->pusbintf) < 0) 
				{
					DBG_871X( "can't get autopm: \n");
				}			
			#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,20))				
				usb_autopm_disable(adapter_to_dvobj(padapter)->pusbintf);
			#else
				usb_autoresume_device(adapter_to_dvobj(padapter)->pusbdev, 1);
			#endif
			pwrpriv->autopm_cnt++;
			}
#endif	//#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
		ret = _SUCCESS;
		goto exit;
#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
		}
#endif	//#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
	}	

	if(rf_off == pwrpriv->rf_pwrstate )
	{		
#ifdef CONFIG_USB_HCI
#ifdef CONFIG_AUTOSUSPEND
		 if(pwrpriv->brfoffbyhw==_TRUE)
		{
			DBG_8192C("hw still in rf_off state ...........\n");
			ret = _FAIL;
			goto exit;
		}
		else if(padapter->registrypriv.usbss_enable)
		{
			DBG_8192C("%s call autoresume_enter....\n",__FUNCTION__);
			if(_FAIL ==  autoresume_enter(padapter))
			{
				DBG_8192C("======> autoresume fail.............\n");
				ret = _FAIL;
				goto exit;
			}	
		}
		else
#endif
#endif
		{
#ifdef CONFIG_IPS
			DBG_8192C("%s call ips_leave....\n",__FUNCTION__);
			if(_FAIL ==  ips_leave(padapter))
			{
				DBG_8192C("======> ips_leave fail.............\n");
				ret = _FAIL;
				goto exit;
			}
#endif
		}
	}

	//TODO: the following checking need to be merged...
	if(padapter->bDriverStopped
		|| !padapter->bup
		|| !padapter->hw_init_completed
	){
		DBG_8192C("%s: bDriverStopped=%d, bup=%d, hw_init_completed=%u\n"
			, caller
		   	, padapter->bDriverStopped
		   	, padapter->bup
		   	, padapter->hw_init_completed);
		ret= _FALSE;
		goto exit;
	}

exit:
	if (pwrpriv->ips_deny_time < rtw_get_current_time() + rtw_ms_to_systime(ips_deffer_ms))
		pwrpriv->ips_deny_time = rtw_get_current_time() + rtw_ms_to_systime(ips_deffer_ms);
	return ret;

}

int rtw_pm_set_lps(_adapter *padapter, u8 mode)
{
	int	ret = 0;	
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;
	
	if ( mode < PS_MODE_NUM )
	{
		if(pwrctrlpriv->power_mgnt !=mode)
		{
			if(PS_MODE_ACTIVE == mode)
			{
				LeaveAllPowerSaveMode(padapter);
			}
			else
			{
				pwrctrlpriv->LpsIdleCount = 2;
			}
			pwrctrlpriv->power_mgnt = mode;
			pwrctrlpriv->bLeisurePs = (PS_MODE_ACTIVE != pwrctrlpriv->power_mgnt)?_TRUE:_FALSE;
		}
	}
	else
	{
		ret = -EINVAL;
	}

	return ret;
}

int rtw_pm_set_ips(_adapter *padapter, u8 mode)
{
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

	if( mode == IPS_NORMAL || mode == IPS_LEVEL_2 ) {
		rtw_ips_mode_req(pwrctrlpriv, mode);
		DBG_871X("%s %s\n", __FUNCTION__, mode == IPS_NORMAL?"IPS_NORMAL":"IPS_LEVEL_2");
		return 0;
	} 
	else if(mode ==IPS_NONE){
		rtw_ips_mode_req(pwrctrlpriv, mode);
		DBG_871X("%s %s\n", __FUNCTION__, "IPS_NONE");
		if((padapter->bSurpriseRemoved ==0)&&(_FAIL == rtw_pwr_wakeup(padapter)) )
			return -EFAULT;
	}
	else {
		return -EINVAL;
	}
	return 0;
}


