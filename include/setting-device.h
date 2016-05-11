/*
 *  Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 *  Licensed under the Flora License, Version 1.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://floralicense.org/license/
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
/*
 * setting-device_action.h
 *
 *  Created on: Dec 8, 2015
 *      Author: JinWang An
 */

#ifndef SETTING_SOUND_H_
#define SETTING_SOUND_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>

#include <player.h>

#define ITEM_SIZE			6
/*#define RINGTONE_MAX_COUNT		5 */
#define RINGTONE_MAX_COUNT		6

enum {
	device_action_LEVEL_HIGH,
	device_action_LEVEL_LOW,
	device_action_LEVEL_NONE,
};

enum {
	device_action_LEVEL_LOW_INT  = 1,
	device_action_LEVEL_MID_INT  = 2,
	device_action_LEVEL_HIGH_INT = 3,
	device_action_LEVEL_NONE_INT = 0
};

struct _device_action_menu_item {
	char *name;
	int is_enable_touch_device_action;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

typedef struct device_action_Item_Data {
	int index;
	Elm_Object_Item *item;
	Evas_Object *check;
} device_action_Item_Data;

char *_gl_device_action_title_get(void *data, Evas_Object *obj, const char *part);
void _double_press_home_key_cb(void *data, Evas_Object *obj, void *event_info);
void _auto_open_apps_cb(void *data, Evas_Object *obj, void *event_info);
Evas_Object *_create_device_action_list(void *data);

void _clear_device_action_resource();

#endif /* SETTING_SOUND_H_ */
