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
 * setting-common-sound.h
 *
 *  Created on: Oct 16, 2013
 *      Author: min-hoyun
 */

#ifndef SETTING_COMMON_SOUND_H_
#define SETTING_COMMON_SOUND_H_

#include <player.h>
#include <sound_manager.h>
#include <mm_sound_private.h>
#include <wav_player.h>
/*#include <player_product.h> */


#define SETTING_DEFAULT_RINGTONE_PATH	TZ_SYS_SHARE_D"/settings/Ringtones"
#define SETTING_DEFAULT_ALERT_PATH		TZ_SYS_SHARE_D"/settings/Alerts"
#define SETTING_DEFAULT_PREVIEW_PATH	TZ_SYS_SHARE_D"/settings/Previews"

#define DELIM		"/"

#define SETTING_DEFAULT_MSG_TONE		SETTING_DEFAULT_ALERT_PATH""DELIM"Flicker.ogg"
#define SETTING_DEFAULT_RING_TONE		SETTING_DEFAULT_RINGTONE_PATH""DELIM"Timeless.ogg"
#define SETTING_DEFAULT_MEDIA_TONE		SETTING_DEFAULT_PREVIEW_PATH""DELIM"Media_preview_Over_the_horizon_B2.ogg"
#define SETTING_DEFAULT_SILENT_OFF_TONE		SETTING_DEFAULT_PREVIEW_PATH""DELIM"Silent_mode_off.ogg"
#define SETTING_DEFAULT_SYS_TONE		SETTING_DEFAULT_PREVIEW_PATH""DELIM"B_Touch.ogg"

#define SETTING_RINGTONE_VOLUME_BACKUP	"db/setting/sound/call/rmd_ringtone_volume"

/* test code */
#define PREVIEW_TEMP_PATH	TZ_USER_CONTENT_D"/Sounds/Over the horizon.mp3"

enum {
    SOUND_MODE_SOUND,
    SOUND_MODE_VIBRATE,
    SOUND_MODE_MUTE
};

enum {
    SOUND_STATE_STOP,
    SOUND_STATE_PLAY
};

int get_sound_mode();
int is_created_player();
int is_player_paused();
void set_looping(int);

void play_sound(char *file_path, float volume, sound_type_e sound_type);
void play_sound_for_sound_mode_setting(char *file_path, float volume, sound_type_e sound_type);
int profile_play_sound(void *data, void *cb, char *ringtone_file, float vol, sound_type_e sound_type, int prelistening_enable);
int _close_player(void *data, sound_type_e type);
int _profile_stop_sound(void *data);

#endif /* SETTING_COMMON_SOUND_H_ */
