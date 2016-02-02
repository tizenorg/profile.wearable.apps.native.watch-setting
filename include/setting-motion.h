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
 * setting-motion.h
 *
 *  Created on: Oct 9, 2013
 *      Author: min-hoyun
 */

#ifndef SETTING_MOTION_H_
#define SETTING_MOTION_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>

#include "setting_data_vconf.h"
#include "util.h"


#define MOTION_ITEM_COUNT			2
#define MOTION_WAKE_UP_ITEM_COUNT	3

struct _motion_menu_item {
	char *name;
	int state;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

struct _motion_wake_up_guesture_item {
	char *title;
	int value;
};

typedef struct Motion_Item_Data {
	int index;
	Elm_Object_Item *item;
} Motion_Item_Data;


typedef struct _motion_data {
	int is_enable_smart_relay;
	int is_enable_wake_up_gesture;
	int wake_up_gesture_type;

	appdata *temp_ad;

	Evas_Object *g_motion_genlist;

} Motion_Data;


char *_gl_motion_title_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_gl_motion_check_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_create_motion_list(void *data);
void _motion_chk_changed_cb(void *data, Evas_Object *obj, void *event_info);
void _motion_gl_smart_relay_cb(void *data, Evas_Object *obj, void *event_info);
void _motion_gl_wake_up_cb(void *data, Evas_Object *obj, void *event_info);
char *_get_wake_up_gesture_sub_title();

void _initialize_motion();
Eina_Bool _clear_motion_cb(void *data, Elm_Object_Item *it);

void motion_vconf_changed_cb(keynode_t *key, void *data);

#endif /* SETTING_MOTION_H_ */
