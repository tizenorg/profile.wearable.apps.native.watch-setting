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
 * setting-sound.h
 *
 *  Created on: Oct 8, 2013
 *      Author: min-hoyun
 */

#ifndef SETTING_SOUND_H_
#define SETTING_SOUND_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>

#include <player.h>
#include <sound_manager.h>

#define ITEM_SIZE			6
/*#define RINGTONE_MAX_COUNT		5 */
#define RINGTONE_MAX_COUNT		6

#define TOUCH_SOUND_ENABLE 		1
#define TOUCH_SOUND_DISABLE 	0


enum {
	VIBRATION_LEVEL_HIGH,
	VIBRATION_LEVEL_LOW
};

enum {
	VIBRATION_LEVEL_LOW_INT  = 1,
	VIBRATION_LEVEL_MID_INT  = 2,
	VIBRATION_LEVEL_HIGH_INT = 3
};

struct _sound_menu_item {
	char *name;
	int is_enable_touch_sound;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

typedef struct Sound_Item_Data {
	int index;
	Elm_Object_Item *item;
	Evas_Object *check;
} Sound_Item_Data;

char *_gl_Sound_title_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_gl_sound_check_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_create_sound_list(void *data);
void _sound_chk_changed_cb(void *data, Evas_Object *obj, void *event_info);
void _volume_cb(void *data, Evas_Object *obj, void *event_info);
void _sound_mode_cb(void *data, Evas_Object *obj, void *event_info);
void _touch_sound_cb(void *data, Evas_Object *obj, void *event_info);
void _ringtone_cb(void *data, Evas_Object *obj, void *event_info);
void _noti_cb(void *data, Evas_Object *obj, void *event_info);
void _vibrate_cb(void *data, Evas_Object *obj, void *event_info);
void _preferred_cb(void *data, Evas_Object *obj, void *event_info);
void _show_sound_mode_list(void *data);
void _show_ringtone_popup_cb(void *data, Evas_Object *obj, void *event_info);
void _show_notification_popup_cb(void *data, Evas_Object *obj, void *event_info);
void _show_vibration_popup_cb(void *data, Evas_Object *obj, void *event_info);
void _show_pref_arm_mode_list(void *data);

void _initialize();
void _clear_sound_resource();
Eina_Bool _clear_sound_cb(void *data, Elm_Object_Item *it);
void _stop_player();
void _stop_wav_player();
void _stop_all_sound_play();

#endif /* SETTING_SOUND_H_ */
