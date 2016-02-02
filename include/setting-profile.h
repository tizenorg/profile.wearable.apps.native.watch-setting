/*
 * Copyright (c) 2010 Samsung Electronics, Inc.
 * All rights reserved.
 *
 * This software is a confidential and proprietary information
 * of Samsung Electronics, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics.
 *
 * setting-profile.h (s-health)
 *
 */

#ifndef SETTING_PROFILE_H_
#define SETTING_PROFILE_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>

#define PROFILE_APP_ID	"org.tizen.shealth.userprofile"
#define PROFILE_OP_ID	"http://tizen.org/appcontrol/operation/launch_profile_setting"

void _create_profile(void *data, Evas_Object *obj, void *event_info);

#endif /* SETTING_PROFILE_H_ */
