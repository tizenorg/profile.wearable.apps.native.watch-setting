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

#ifndef __SETTING_H__
#define __SETTING_H__

#include <Elementary.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dlog.h>
#include <app.h>
#include <feedback.h>
#include <bluetooth.h>

#include "util.h"


#define NUM_OF_SETTING_MAIN_MENU	 11

struct _menu_item {
	char *name;
	char *icon_name;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};


/* Main Screen's callback functions */
void clock_cb(void *data, Evas_Object *obj, void *event_info);
void notification_cb(void *data, Evas_Object *obj, void *event_info);
void sound_cb(void *data, Evas_Object *obj, void *event_info);
void homescreen_cb(void *data, Evas_Object *obj, void *event_info);
void volume_cb(void *data, Evas_Object *obj, void *event_info);
void display_cb(void *data, Evas_Object *obj, void *event_info);
void battery_cb(void *data, Evas_Object *obj, void *event_info);
void bluetooth_cb(void *data, Evas_Object *obj, void *event_info);
void motion_cb(void *data, Evas_Object *obj, void *event_info);
void lockscreen_cb(void *data, Evas_Object *obj, void *event_info);
void double_pressing_cb(void *data, Evas_Object *obj, void *event_info);
void language_cb(void *data, Evas_Object *obj, void *event_info);
void safety_cb(void *data, Evas_Object *obj, void *event_info);
void reset_gear_cb(void *data, Evas_Object *obj, void *event_info);
void gear_info_cb(void *data, Evas_Object *obj, void *event_info);
void keyboard_cb(void *data, Evas_Object *obj, void *event_info);

static void _update_main_menu_title(void *data);

#endif
