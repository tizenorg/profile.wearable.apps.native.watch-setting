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
/*
 * setting-reset.h
 *
 *	Created on: Oct 10, 2013
 *		Author: min-hoyun
 */

#ifndef SETTING_RESET_H_
#define SETTING_RESET_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>
#include <dd-deviced.h>

#define PREDEF_FACTORY_RESET "launchfr"

void _reset_popup_cb(void *data, Evas_Object *obj, void *event_info);


#endif /* SETTING_RESET_H_ */
