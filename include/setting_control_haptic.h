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
#ifndef _SETTING_CONTROL_HAPTIC_
#define _SETTING_CONTROL_HAPTIC_

#include <Elementary.h>
#include <dd-haptic.h>

#define SETTING_DEFAULT_HAPTIC_PREVIEW_VIB		TZ_SYS_SHARE_D"/settings/Vibrations/B2_System_Vibration_Preview_140221.ivt"
#define SETTING_DEFAULT_SYSTEM_HAPTIC_PREVIEW_VIB		TZ_SYS_SHARE_D"/settings/Vibrations/B2_System_Long_v2_140221.ivt"
#define SETTING_DEFAULT_NOTIFICATION_GENERAL_PREVIEW_VIB		TZ_SYS_SHARE_D"/settings/Vibrations/B2_Notification_General_v5_140306.ivt"
#define SETTING_VIB_FEEDBACK_RATE				20

#define SETTING_VIB_STRONG_RATE			49	/* 70 */
#define SETTING_VIB_MEDIUM_RATE			49
#define SETTING_VIB_WEAK_RATE			20	/* 30 */

struct _haptic_data {
	int is_haptic_opened;
};

int _haptic_open();
int _is_haptic_open();
int _haptic_close();

Eina_Bool __vibration_timeout(void *data);
void _start_vibration(int level, int feedback_rate, char *res_path);

#endif
