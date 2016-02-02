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
#ifndef _SETTING_INDICATOR_UTIL_H_
#define _SETTING_INDICATOR_UTIL_H_

#include <Elementary.h>
#include "setting_debug.h"
#include "util.h"

void indicator_set_vconf_changed_cb(void *data);
void indicator_unset_vconf_changed_cb();

void indicator_view_update(Evas_Object *layout);

#endif
