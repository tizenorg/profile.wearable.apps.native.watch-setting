/*
 * Copyright (c) 2010 Samsung Electronics, Inc.
 * All rights reserved.
 *
 * This software is a confidential and proprietary information
 * of Samsung Electronics, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics.
 */
#include "setting_data_vconf.h"
#include "util.h"


int unregister_vconf_changing(const char *vconf, vconf_callback_fn cb)
{
	int ret = TRUE;
	if (vconf && cb) {
		ret = vconf_ignore_key_changed(vconf, cb);
		if (ret == VCONF_OK) {
			DBG("Setting - vconf's changed callback unregisted!!");
		} else {
			DBG("Setting - vconf's changed callback do not unregist");
			ret = FALSE;
		}
	}
	return ret;
}

int register_vconf_changing(const char *vconf, vconf_callback_fn cb, void *data)
{
	int ret = TRUE;
	if (vconf && cb) {
		ret = vconf_notify_key_changed(vconf, cb, data);
		if (ret == VCONF_OK) {
			DBG("Setting - vconf's changed callback is registed!");
		} else {
			DBG("Setting - vconf's changed callback is not registed!");
			ret = FALSE;
		}
	}
	return ret;
}
