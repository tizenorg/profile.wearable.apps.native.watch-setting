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
#ifndef _SETTING_DATA_VCONF_H_
#define _SETTING_DATA_VCONF_H_

#include <vconf.h>
#include <vconf-keys.h>


int unregister_vconf_changing(const char *vconf, vconf_callback_fn cb);
int register_vconf_changing(const char *vconf, vconf_callback_fn cb, void *data);

#endif
