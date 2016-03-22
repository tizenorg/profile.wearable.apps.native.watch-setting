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
 * setting-volume.h
 *
 *  Created on: Oct 8, 2013
 *      Author: min-hoyun
 */

#ifndef SETTING_VOLUME_H_
#define SETTING_VOLUME_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>
#include <sound_manager.h>

#include "setting-common-sound.h"

#include "util.h"

#define VOLUMN_ITEM_COUNT	4


struct _volume_menu_item {
	char *name;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

player_h *player;

int volume_index;
int original_volume;
int original_sound_mode;
int curr_sound_type;
int is_wav_playing_vol;
int sound_id_vol;


/*--------------------------------------------------------------// */

char *_gl_volume_title_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_create_volume_list(void *data);

void _show_multimedia_popup(void *data, Evas_Object *obj, void *event_info);
void _show_ringtone_popup(void *data, Evas_Object *obj, void *event_info);
void _show_notification_popup(void *data, Evas_Object *obj, void *event_info);
void _show_system_popup(void *data, Evas_Object *obj, void *event_info);

void _initialize_volume();
void _clear_volume_resources();
void _clear_volume_cb(void *data , Evas *e, Evas_Object *obj, void *event_info);

void _stop_all_volume_sound();
void _update_volume_screen_on_resume();

#endif /* SETTING_VOLUME_H_ */
