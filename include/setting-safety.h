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
 * setting-safety.h
 *
 *  Created on: Jan 14, 2014
 *      Author: min-hoyun
 */

#ifndef SETTING_SAFETY_H_
#define SETTING_SAFETY_H_

#include <Elementary.h>
#include "util.h"

#define SAFETY_MENU_ITEM_COUNT	5

struct _safety_menu_item {
	char *name;
	char *sub_name;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

struct _set_interval_menu_item {
	char *title;
	char *sub_title;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

typedef struct safety_Item_Data {
	int index;
	Elm_Object_Item *item;
} Safety_Item_Data;

struct _interval_trauma_menu_item {
	char *str;
	char *time;
};

typedef struct _safety_data {
	int is_support_emergency;
	int is_enable_emergency_mode;
	int is_enable_trauma;
	int is_enable_no_activity;
	int interval_trauma;
	int interval_no_activity;

	appdata *temp_ad;

	Evas_Object *g_safety_genlist;
	Evas_Object *g_interval_genlist;

} Safety_Data;

Evas_Object *create_safety_list(void *data);
Eina_Bool clear_safety_cb(void *data, Elm_Object_Item *it);

void _help_popup_cb(void *data, Evas_Object *obj, void *event_info);
void _disable_emergency_popup_cb(void *data, Evas_Object *obj, void *event_info);

#endif /* SETTING_SAFETY_H_ */
