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
#include <feedback.h>
#include <sys/types.h>
#include <dirent.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "setting-sound.h"
#include "util.h"
#include "setting-common-sound.h"
#include "setting_control_haptic.h"
#include "setting_view_toast.h"
#include "setting_data_vconf.h"
#include "setting-volume.h"


#define AUDIO_RESOURCE_EXTENSION	".ogg"

static struct _sound_menu_item sound_menu_its[] = {
	{ "IDS_ST_OPT_VOLUME",					0,		_volume_cb 	},
	{ "IDS_ST_OPT_SOUND_MODE_ABB", 			0, 		_sound_mode_cb  },
	{ "IDS_ST_BODY_TOUCH_SOUNDS_ABB", 		0,		_touch_sound_cb },
	{ "IDS_CST_MBODY_RINGTONES", 		0,		_ringtone_cb 	},
	{ "IDS_ST_BUTTON_NOTIFICATIONS", 		0,		_noti_cb 		},
	{ "IDS_ST_HEADER_VIBRATION_ABB", 		0,		_vibrate_cb 	},
	/*	{ "IDS_ST_BODY_PREFERRED_ARM_ABB", 	0,		_preferred_cb 	}, */
	{ NULL, 0, NULL }
};

char *sound_mode_str[] = {
	"IDS_ST_OPT_SOUND_ABB2",
	"IDS_ST_BODY_VIBRATE_ABB2",
	"IDS_ST_OPT_MUTE"
};
char *ringtone_str[] = {
	"Ringtone1",
	"Ringtone2",
	"Ringtone3"
};
char *notification_str[] = {
	"Notification1",
	"Notification2",
	"Notification3"
};
char *vibration_str[] = {
	"IDS_ST_OPT_STRONG_M_INTENSITY",
	"IDS_ST_OPT_WEAK_M_INTENSITY"
};
char *pref_arm_str[] = {
	"IDS_ST_OPT_LEFT_ARM_ABB",
	"IDS_ST_OPT_RIGHT_ARM_ABB"
};
char *pref_arm_toast_msg[] = {
	"IDS_ST_TPOP_MAIN_MIC_ADJUSTED_FOR_LEFT_ARM",
	"IDS_ST_TPOP_MAIN_MIC_ADJUSTED_FOR_RIGHT_ARM"
};

static char ringtone_arr[RINGTONE_MAX_COUNT][1024];
static char ringtone_name_arr[RINGTONE_MAX_COUNT][1024];
static char notification_arr[RINGTONE_MAX_COUNT][1024];
static char notification_name_arr[RINGTONE_MAX_COUNT][1024];

static appdata *temp_ad 					= NULL;
static Evas_Object *g_sound_genlist 			= NULL;
static Evas_Object *g_sound_mode_genlist 		= NULL;
static Evas_Object *g_ringtone_type_genlist 	= NULL;
static Evas_Object *g_notification_type_genlist = NULL;
static Evas_Object *g_vibration_type_genlist 	= NULL;
static Evas_Object *g_pref_arm_type_genlist 	= NULL;
static Elm_Object_Item *g_vib_item = NULL;

static int sound_mode 	     = 1;			/* Vibrate */
static int ringtone_type 	 = 0;			/* Rigntone type */
static int confirmed_ringtone_type 	 = 0;			/* setted Rigntone type */
static int notification_type = 0;			/* Notification type */
static int confirmed_Notification_type 	 = 0;			/* setted Notification type */
static int vibrate_type 	 = 0;			/* Vibration type */
static int pref_arm_type 	 = 0;			/* Pref_arm_type */
static int changing_sound_mode_myself = 0;

static int ringtone_count = 0;
static int notification_count = 0;

char curr_ringtone_file_path[1023];
char curr_noti_file_path[1023];

static int cur_sound_type;
static int is_loaded_ringtone_data = 0;
static int is_loaded_noti_data = 0;
static int origin_vibration_level;

static Ecore_Timer *vibration_timer = NULL;

static int is_wav_playing = SOUND_STATE_STOP;
static int sound_id = -1;


static void sound_vconf_changed_cb(keynode_t *key, void *data);
static void vibrate_vconf_changed_cb(keynode_t *key, void *data);
static void pm_state_vconf_changed_cb(keynode_t *key, void *data);
static void _vibration_gl_cb(void *data, Evas_Object *obj, void *event_info);
static void stop_wav();
static Eina_Bool _back_sound_naviframe_cb(void *data, Elm_Object_Item *it);


void _initialize()
{
	/* touch sound */
	/*effect_playsound_init(); */
}

Eina_Bool _clear_sound_cb(void *data, Elm_Object_Item *it)
{
	_clear_sound_resource();

	return EINA_TRUE;
}

void _clear_sound_resource()
{
	if (vibration_timer) {
		ecore_timer_del(vibration_timer);
		vibration_timer = NULL;
	}

	_haptic_close();

	temp_ad = NULL;
	g_sound_genlist = NULL;
	g_sound_mode_genlist = NULL;
	g_ringtone_type_genlist = NULL;
	g_notification_type_genlist = NULL;
	g_vibration_type_genlist = NULL;
	g_pref_arm_type_genlist = NULL;
	g_vib_item = NULL;

	sound_mode = 0;
	ringtone_type = 0;
	notification_type = 0;
	vibrate_type = 0;
	pref_arm_type = 0;
	cur_sound_type = 0;
	is_loaded_ringtone_data = 0;
	is_loaded_noti_data = 0;
	origin_vibration_level = 0;

	is_wav_playing = 0;
	sound_id = -1;

	_stop_player();

	/* Unregister sound mode vconf callback */
	unregister_vconf_changing(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, sound_vconf_changed_cb);
	unregister_vconf_changing(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, vibrate_vconf_changed_cb);
}

static Eina_Bool _back_sound_naviframe_cb(void *data, Elm_Object_Item *it)
{
	appdata *ad = data;
	if (ad == NULL)
		return EINA_FALSE;

	eext_rotary_object_event_activated_set(g_sound_genlist, EINA_TRUE);
}

void _stop_all_sound_play()
{
	DBG("Setting - stop all sound");

	stop_wav();

	_stop_player();
}

static void stop_wav()
{
	if (is_wav_playing == SOUND_STATE_PLAY) {
		DBG("Setting - sound id : %d", sound_id);

		wav_player_stop(sound_id);
		is_wav_playing = SOUND_STATE_STOP;
		sound_id = -1;
	}
}

void _stop_wav_player()
{
	stop_wav();
}

void _stop_player()
{
	DBG("Setting - _stop_player() is called!");
	if (is_created_player()) {
		_close_player(NULL, cur_sound_type);
	}
}

static void get_sound_file_list(char *dir, int type)
{
	DIR *dp;
	struct dirent entry;
	struct dirent *result = NULL;
	char *cur_ring_path = NULL;

	if ((dp = opendir(dir)) == NULL) {
		DBG("Setting - dir open error!");
		return;
	}

	if (type) {
		cur_ring_path = vconf_get_str(VCONFKEY_SETAPPL_CALL_RINGTONE_PATH_STR);
		if (cur_ring_path == NULL) {
			cur_ring_path = vconf_get_str(VCONFKEY_SETAPPL_CALL_RINGTONE_DEFAULT_PATH_STR);
		}
		ringtone_count = 0;
	} else {
		cur_ring_path = vconf_get_str(VCONFKEY_SETAPPL_NOTI_MSG_RINGTONE_PATH_STR);
		if (cur_ring_path == NULL) {
			cur_ring_path = vconf_get_str(VCONFKEY_SETAPPL_NOTI_RINGTONE_DEFAULT_PATH_STR);
		}
		notification_count = 0;
	}

	while (!readdir_r(dp, &entry, &result) && result) {
		if (strncmp(result->d_name, ".", 1)
				&& strlen(dir) < 1024
				&& strlen(replace(result->d_name, ".ogg", "")) <1024
				&& strlen(replace(notification_name_arr[notification_count], "_", " ")) < 1024) {
			if (type) {
				strncpy(ringtone_arr[ringtone_count], dir, 1024);
				strncat(ringtone_arr[ringtone_count], result->d_name, 1024);

				strcpy(ringtone_name_arr[ringtone_count], replace(result->d_name, ".ogg", ""));

				/*DBG("Setting - %s", ringtone_arr[ringtone_count]); */

				if (!strcmp(cur_ring_path, ringtone_arr[ringtone_count])) {
					ringtone_type = ringtone_count;
					/*DBG("Settng - %s is same with %s", cur_ring_path, ringtone_arr[ringtone_count]); */
				}
				/*DBG("Settng - %s is same with %s", cur_ring_path, ringtone_arr[ringtone_count]); */

				ringtone_count++;
			} else {
				strcpy(notification_arr[notification_count], dir);
				strcat(notification_arr[notification_count], result->d_name);

				strcpy(notification_name_arr[notification_count], replace(result->d_name, ".ogg", ""));
				strcpy(notification_name_arr[notification_count], replace(notification_name_arr[notification_count], "_", " "));

				/*DBG("Setting - %s", notification_arr[notification_count]); */

				if (!strcmp(cur_ring_path, notification_arr[notification_count])) {
					notification_type = notification_count;
					/*DBG("Settng - %s is same with %s", cur_ring_path, notification_arr[notification_count]); */
				}
				/*DBG("Settng - %s is same with %s", cur_ring_path, notification_arr[notification_count]); */

				notification_count++;
			}
		}
	}
	closedir(dp);
}


static int get_vibration_level()
{
	int mode = 1;
	int level = 0;

	vconf_get_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, &level);

	switch (level) {
		case VIBRATION_LEVEL_LOW_INT :
			mode = VIBRATION_LEVEL_LOW;
			break;
			/*	case VIBRATION_LEVEL_MID_INT : */
			/*		mode = VIBRATION_LEVEL_MID; */
			/*		break; */
		case VIBRATION_LEVEL_HIGH_INT :
			mode = VIBRATION_LEVEL_HIGH;
			break;
	}
	return mode;
}

void _show_volume_list(void *data)
{
	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	_initialize_volume();

	genlist = _create_volume_list(data);
	if (genlist == NULL) {
		DBG("%s", "volume cb - genlist is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _back_sound_naviframe_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_DEL, _clear_volume_cb, ad);
}

void _volume_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_volume_list(data);
}

void _sound_mode_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_sound_mode_list(data);
}

void _touch_sound_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("%s", "_touch_sound_cb - appdata or check is null");
		return;
	}
	sound_menu_its[2].is_enable_touch_sound = sound_menu_its[2].is_enable_touch_sound ? 0 : 1;
	vconf_set_bool(VCONFKEY_SETAPPL_TOUCH_SOUNDS_BOOL, sound_menu_its[2].is_enable_touch_sound);
	vconf_set_bool(VCONFKEY_SETAPPL_BUTTON_SOUNDS_BOOL, sound_menu_its[2].is_enable_touch_sound);

	elm_genlist_item_selected_set(it, EINA_FALSE);

	elm_genlist_item_update(it);
}

void _ringtone_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_ringtone_popup_cb(data, obj, event_info);
}

void _noti_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_notification_popup_cb(data, obj, event_info);
}

void _vibrate_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_vibration_popup_cb(data, obj, event_info);
}

void _preferred_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_pref_arm_mode_list(data);
}

static char *_get_sound_file_name(char *full_name)
{
	char *token = NULL;
	char *temp_token = NULL;
	char sep[] = "*/*";
	char *saveptr = NULL;

	DBG("Setting - %s, %s", token, full_name);

	strtok_r(full_name, sep, &saveptr);

	do {
		token = strtok_r(NULL, sep, &saveptr);
		if (token != NULL) {
			temp_token = token;
		}
	} while (token != NULL);

	char *result_token = NULL;
	if (temp_token) {
		temp_token = replace(temp_token, ".ogg", "");
		result_token = replace(temp_token, "_", " ");
	}
	return result_token;
}

char *_gl_Sound_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;
	char *pa_cur_ringtone = NULL;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(sound_menu_its[index % ITEM_SIZE].name));
	} else if (!strcmp(part, "elm.text.1")) {
		switch (index) {
			case 1 :
				sound_mode = get_sound_mode();
				snprintf(buf, sizeof(buf) - 1, "%s", _(sound_mode_str[sound_mode % 3]));
				break;
			case 3 :
				pa_cur_ringtone = vconf_get_str(VCONFKEY_SETAPPL_CALL_RINGTONE_PATH_STR);
				if (pa_cur_ringtone == NULL) {
					pa_cur_ringtone = vconf_get_str(VCONFKEY_SETAPPL_CALL_RINGTONE_DEFAULT_PATH_STR);
				}
				if (strlen(pa_cur_ringtone) < 1024) {
					strcpy(curr_ringtone_file_path, pa_cur_ringtone);
					DBG("Setting - ringtone path : %s", pa_cur_ringtone);

					snprintf(buf, sizeof(buf) - 1, "%s", _get_sound_file_name(pa_cur_ringtone));
				}
				break;
			case 4 :
				pa_cur_ringtone = vconf_get_str(VCONFKEY_SETAPPL_NOTI_MSG_RINGTONE_PATH_STR);
				if (pa_cur_ringtone == NULL) {
					pa_cur_ringtone = vconf_get_str(VCONFKEY_SETAPPL_NOTI_RINGTONE_DEFAULT_PATH_STR);
				}
				if (strlen(pa_cur_ringtone) < 1024) {
					strcpy(curr_noti_file_path, pa_cur_ringtone);
					DBG("Setting - noti's ringtone path : %s", pa_cur_ringtone);

					snprintf(buf, sizeof(buf) - 1, "%s", _get_sound_file_name(pa_cur_ringtone));
				}
				break;
			case 5 :
				vibrate_type = get_vibration_level();
				snprintf(buf, sizeof(buf) - 1, "%s", _(vibration_str[vibrate_type % 2]));
				break;
				/*
				   case 5 :
				   vconf_get_bool(VCONFKEY_SETAPPL_PERFERED_ARM_LEFT_BOOL, &pref_arm_type);
				   pref_arm_type = (pref_arm_type == TRUE) ? 0 : 1;
				   snprintf(buf, sizeof(buf)-1, "%s", _(pref_arm_str[pref_arm_type % 2]));
				   break;
				 */
		}
		index++;
	}
	return strdup(buf);
}

void _sound_chk_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _sound_chk_changed_cb() is called!!");

	vconf_get_bool(VCONFKEY_SETAPPL_TOUCH_SOUNDS_BOOL, &sound_menu_its[2].is_enable_touch_sound);

	sound_menu_its[2].is_enable_touch_sound = !sound_menu_its[2].is_enable_touch_sound;

	/* Update touch sound enable state */
	vconf_set_bool(VCONFKEY_SETAPPL_TOUCH_SOUNDS_BOOL, sound_menu_its[2].is_enable_touch_sound);
	vconf_set_bool(VCONFKEY_SETAPPL_BUTTON_SOUNDS_BOOL, sound_menu_its[2].is_enable_touch_sound);
}

static void _sound_gl_del(void *data, Evas_Object *obj)
{
	Sound_Item_Data *id = data;
	if (id)
		free(id);
}

Evas_Object *_gl_sound_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	Sound_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);

		if (vconf_get_bool(VCONFKEY_SETAPPL_TOUCH_SOUNDS_BOOL, &sound_menu_its[2].is_enable_touch_sound) < 0) {
			sound_menu_its[2].is_enable_touch_sound = TOUCH_SOUND_ENABLE;	/*  default value of touch sounds : on */
		}
		elm_check_state_set(check, (sound_menu_its[2].is_enable_touch_sound) ? EINA_TRUE : EINA_FALSE);
		evas_object_smart_callback_add(check, "changed", _sound_chk_changed_cb, (void *)1);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_FALSE);

		id->check = check;

		index++;
	}

	return check;
}

Evas_Object *_create_sound_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_sound_list - appdata is null");
		return NULL;
	}

	temp_ad = ad;

	Evas_Object *genlist  = NULL;
	struct _sound_menu_item *menu_its = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc_tmp;

	Elm_Genlist_Item_Class *itc_1text = elm_genlist_item_class_new();
	itc_1text->item_style = "1text";
	itc_1text->func.text_get = _gl_Sound_title_get;
	itc_1text->func.del = _sound_gl_del;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text";
	itc->func.text_get = _gl_Sound_title_get;
	itc->func.del = _sound_gl_del;

	Elm_Genlist_Item_Class *itc_touch_snd = elm_genlist_item_class_new();
	itc_touch_snd->item_style = "1text.1icon.1";
	itc_touch_snd->func.text_get = _gl_Sound_title_get;
	itc_touch_snd->func.content_get = _gl_sound_check_get;
	itc_touch_snd->func.del = _sound_gl_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	menu_its = sound_menu_its;

	connect_to_wheel_with_genlist(genlist,ad);

	for (idx = 0; idx < ITEM_SIZE; idx++) {
		if (idx == 0) {
			itc_tmp = itc_1text;
		} else if (idx == 2) {
			itc_tmp = itc_touch_snd;
		} else {
			itc_tmp = itc;
		}

		Sound_Item_Data *id = calloc(sizeof(Sound_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
					genlist,			/* genlist object */
					itc_tmp,			/* item class */
					id,		            /* data */
					NULL,
					ELM_GENLIST_ITEM_NONE,
					menu_its[ idx ].func,	/* call back */
					ad);

			if (idx == ITEM_SIZE - 1) {
				g_vib_item = id->item;
			}
		}
	}
	elm_genlist_item_class_free(itc_1text);
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc_touch_snd);

	g_sound_genlist = genlist;

	/* Register sound mode vconf callback */
	register_vconf_changing(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, sound_vconf_changed_cb, NULL);
	register_vconf_changing(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, vibrate_vconf_changed_cb, NULL);

	return genlist;
}

static char *_gl_sound_mode_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(sound_mode_str[index % 3]));
		index++;
	}
	return strdup(buf);
}

static Evas_Object *_gl_sound_mode_ridio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_object_style_set(radio, "list");
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);
		evas_object_smart_callback_add(radio, "changed", NULL, (void *)id->index);
		if (sound_mode == id->index)
			elm_radio_value_set(radio_main, sound_mode);

		index++;
	}
	return radio;
}

static void _sound_mode_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}

static int curr_touch_sound = 0;


static void _sound_mode_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	sound_mode = (int)data;

	changing_sound_mode_myself = 1;

	if (g_sound_mode_genlist) {
		elm_genlist_realized_items_update(g_sound_mode_genlist);
	}

	int ringtone_level = 0;
	int prev_sound_mode = get_sound_mode();

	switch (sound_mode) {
		case SOUND_MODE_SOUND:
			if (get_sound_mode() != SOUND_MODE_SOUND) {
				DBG("Setting - current sound mode is not sound!! Change sound mode to sound!!");

				vconf_get_bool(VCONFKEY_SETAPPL_TOUCH_SOUNDS_BOOL, &curr_touch_sound);
				if (curr_touch_sound) {
					vconf_set_bool(VCONFKEY_SETAPPL_TOUCH_SOUNDS_BOOL, FALSE);
				}

				vconf_get_int(SETTING_RINGTONE_VOLUME_BACKUP, &ringtone_level);
				DBG("Setting - ringtone backup level: %d", ringtone_level);

				vconf_set_int(VCONFKEY_SETAPPL_CALL_RINGTONE_SOUND_VOLUME_INT, ringtone_level);

				if (is_created_player()) {
					_close_player(NULL, SOUND_TYPE_RINGTONE);
				}
				play_sound_for_sound_mode_setting(SETTING_DEFAULT_SILENT_OFF_TONE, 0.0, SOUND_TYPE_RINGTONE);
				set_looping(FALSE);

				sound_manager_get_volume(SOUND_TYPE_RINGTONE, &ringtone_level);
				if (ringtone_level == 0) {
					DBG("Setting - Ringtone volume is 0!! Restore to 1!!");
					sound_manager_set_volume(SOUND_TYPE_RINGTONE, 1);
				}

				vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, TRUE);
				vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, FALSE);

				vconf_set_bool(VCONFKEY_SETAPPL_TOUCH_SOUNDS_BOOL, curr_touch_sound);
				curr_touch_sound = 0;
			}
			break;
		case SOUND_MODE_VIBRATE:
			if (prev_sound_mode != SOUND_MODE_VIBRATE) {
				DBG("Setting - current sound mode is not vibration. Change sound mode to vibration!!");

				vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, FALSE);
				vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, TRUE);

				vconf_get_int(VCONFKEY_SETAPPL_CALL_RINGTONE_SOUND_VOLUME_INT, &ringtone_level);
				DBG("Setting - ringtone original level: %d", ringtone_level);

				vconf_set_int(SETTING_RINGTONE_VOLUME_BACKUP, ringtone_level);
				/*sound_manager_set_muteall(TRUE);	// mute!! */
			}
			_start_vibration(5, SETTING_VIB_STRONG_RATE, SETTING_DEFAULT_SYSTEM_HAPTIC_PREVIEW_VIB);
			break;
		case SOUND_MODE_MUTE:
			if (prev_sound_mode != SOUND_MODE_MUTE) {
				DBG("Setting - current sound mode is not mute. Change sound mode to mute!!");

				vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, FALSE);
				vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, FALSE);

				vconf_get_int(VCONFKEY_SETAPPL_CALL_RINGTONE_SOUND_VOLUME_INT, &ringtone_level);
				DBG("Setting - ringtone original level: %d", ringtone_level);

				vconf_set_int(SETTING_RINGTONE_VOLUME_BACKUP, ringtone_level);
				/*sound_manager_set_muteall(TRUE);	// mute!! */
			}
			break;
		default:
			ERR("Setting - wrong sound mode value!!");
			break;
	}

	elm_naviframe_item_pop(temp_ad->nf);
	if (!temp_ad->sound_mode_rdg) {
		evas_object_del(temp_ad->sound_mode_rdg);
		temp_ad->sound_mode_rdg = NULL;
	}

	if (g_sound_genlist) {
		elm_genlist_realized_items_update(g_sound_genlist);
	}
}

static void sound_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - sound_vconf_changed_cb() is called!!");

	if (changing_sound_mode_myself) {
		DBG("Setting - changing_sound_mode_myself!!");

		changing_sound_mode_myself = 0;
		return;
	}

	sound_mode = get_sound_mode();

	if (g_sound_mode_genlist) {
		elm_genlist_realized_items_update(g_sound_mode_genlist);
	}

	if (g_sound_genlist) {
		elm_genlist_realized_items_update(g_sound_genlist);
	}
}

static void vibrate_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - vibrate_vconf_changed_cb() is called!!");

	sound_mode = get_sound_mode();

	if (g_sound_mode_genlist) {
		elm_genlist_realized_items_update(g_sound_mode_genlist);
	}

	if (g_sound_genlist) {
		elm_genlist_realized_items_update(g_sound_genlist);
	}
}

Eina_Bool _sound_mode_back_cb(void *data, Elm_Object_Item *it)
{
	g_sound_mode_genlist = NULL;
	eext_rotary_object_event_activated_set(g_sound_genlist, EINA_TRUE);

	return EINA_TRUE;
}

void _show_sound_mode_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_sound_mode_list - appdata is null");
		return;
	}
	Evas_Object *genlist  = NULL;
	Elm_Object_Item *item = NULL;
	Elm_Object_Item *nf_it = NULL;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_sound_mode_title_get;
	itc->func.content_get = _gl_sound_mode_ridio_get;
	itc->func.del = _sound_mode_gl_del;

	sound_mode = get_sound_mode();

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	connect_to_wheel_with_genlist(genlist,ad);

	Item_Data *id = calloc(sizeof(Item_Data), 1);
	if (id) {
		id->index = 0;
		item = elm_genlist_item_append(genlist,	itc, id, NULL, ELM_GENLIST_ITEM_NONE, _sound_mode_gl_cb, (void *)0);
		id->item = item;
	}

	Item_Data *id2 = calloc(sizeof(Item_Data), 1);
	if (id2) {
		id2->index = 1;
		item = elm_genlist_item_append(genlist, itc, id2, NULL, ELM_GENLIST_ITEM_NONE, _sound_mode_gl_cb, (void *)1);
		id2->item = item;
	}

	Item_Data *id3 = calloc(sizeof(Item_Data), 1);
	if (id3) {
		id3->index = 2;
		item = elm_genlist_item_append(genlist, itc, id3, NULL, ELM_GENLIST_ITEM_NONE, _sound_mode_gl_cb, (void *)2);
		id3->item = item;
	}

	ad->sound_mode_rdg = elm_radio_add(genlist);
	elm_object_style_set(ad->sound_mode_rdg, "elm/radio/base/list");
	elm_radio_state_value_set(ad->sound_mode_rdg, 3);
	elm_radio_value_set(ad->sound_mode_rdg, sound_mode);

	evas_object_data_set(genlist, "radio_main", ad->sound_mode_rdg);

	elm_genlist_item_class_free(itc);

	g_sound_mode_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
	elm_naviframe_item_pop_cb_set(nf_it, _sound_mode_back_cb, ad);
}

static void _response_ringtone_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = (appdata *)data;

	_stop_player();

#if 0
	/* stop wav sound! */
	stop_wav();
#endif
	/* save a ringtone type. */
	vconf_set_str(VCONFKEY_SETAPPL_CALL_RINGTONE_PATH_STR , ringtone_arr[ringtone_type]);

	if (g_ringtone_type_genlist != NULL) {
		evas_object_del(g_ringtone_type_genlist);
		g_ringtone_type_genlist = NULL;
	}

	if (temp_ad->ringtone_type_rdg != NULL) {
		evas_object_del(temp_ad->ringtone_type_rdg);
		temp_ad->ringtone_type_rdg = NULL;
	}

	if (ad != NULL) {
		elm_naviframe_item_pop(ad->nf);
	}

	if (g_sound_genlist != NULL) {
		elm_genlist_realized_items_update(g_sound_genlist);
	}

	confirmed_ringtone_type = ringtone_type;
}

static void _response_ringtone_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (g_ringtone_type_genlist != NULL) {
		evas_object_del(g_ringtone_type_genlist);
		g_ringtone_type_genlist = NULL;
	}

	appdata *ad = (appdata *)data;
	if (ad != NULL) {
		elm_naviframe_item_pop(ad->nf);
	}

	_stop_player();

	ringtone_type = confirmed_ringtone_type;

#if 0
	/* stop wav sound! */
	stop_wav();
#endif
}

static char *_gl_ringtone_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *item = data;
	if (item == NULL)
		return NULL;

	char buf[1024];

	/*DBG("---> %d ---> %s ", (item->index%ringtone_count), ringtone_name_arr[item->index % ringtone_count]); */
	sprintf(buf, "%s", ringtone_name_arr[item->index % ringtone_count]);

	return strdup(buf);
}

static Evas_Object *_gl_ringtone_radio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_object_style_set(radio, "list");
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);

		if (id->index == ringtone_type) {
			elm_radio_value_set(radio_main, id->index);
		}
		index++;
	}
	return radio;
}

static void _ringtone_type_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ringtone_type = (int)data;

#if 0
	stop_wav();

	is_wav_playing = SOUND_STATE_PLAY;
	wav_player_start(ringtone_arr[ringtone_type], SOUND_TYPE_RINGTONE, NULL, (void *)1, &sound_id);
#endif

	int volume = 0;
	sound_manager_get_volume(SOUND_TYPE_RINGTONE, &volume);

	if (volume == 0 || get_sound_mode() != SOUND_MODE_SOUND) {
		if (temp_ad) {
			struct _toast_data *toast = _create_toast(temp_ad, _("IDS_ST_TPOP_VOLUME_CURRENTLY_SET_TO_0"));
			if (toast) {
				_show_toast(temp_ad, toast);
			}
		}
	} else {
		play_sound(ringtone_arr[ringtone_type], 0.0, SOUND_TYPE_RINGTONE);
	}

	elm_genlist_realized_items_update(g_ringtone_type_genlist);
}

#if 0 // _NOT_USED_
static void _ringtone_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL)
		return;

	ringtone_type = confirmed_ringtone_type;

	_stop_all_sound_play();

	unregister_vconf_changing(VCONFKEY_PM_STATE, pm_state_vconf_changed_cb);
}
#endif


static void pm_state_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - pm_state_vconf_changed_cb() is called!");

	int pm_state = 0;
	vconf_get_int(VCONFKEY_PM_STATE, &pm_state);

	if (pm_state == VCONFKEY_PM_STATE_LCDOFF) {
		DBG("Setting - LCD Off!! stop sound!");
		_stop_all_sound_play();
	}
}

int cstring_cmp(const void *a, const void *b)
{
	const char *ia = (const char *)a;
	const char *ib = (const char *)b;
	return strcmp(ia, ib);
}

void _show_ringtone_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("ENTER _show_ringtone_popup_cb");
	Evas_Object *popup, *btn;
	Elm_Object_Item *nf_it = NULL;
	unsigned int index;
	appdata *ad = data;

	if (ad == NULL)
		return;

	ad->MENU_TYPE = SETTING_SOUND_RINGTONE;

	if (is_loaded_ringtone_data == 0) {
		get_sound_file_list(TZ_SYS_RO_APP_D"/org.tizen.watch-setting/shared/res/settings/Ringtones/", 1);
		is_loaded_ringtone_data = 1;

		/* @todo apply eina_sort */
		/*qsort (ringtone_name_arr, ringtone_count-1, sizeof(char*), cstring_cmp); */
	}

	cur_sound_type = SOUND_TYPE_RINGTONE;

	popup = elm_layout_add(ad->win_main);

	Eina_Bool bret = elm_layout_file_set(popup, EDJE_PATH, "setting/genlist/2button-layout");
	if (bret == EINA_FALSE) {
		DBG("elm_layout_file_set FAILED with setting/genlist/2button-layout");
	} else {
		DBG("elm_layout_file_set OK with setting/genlist/2button-layout");
	}
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(popup, EVAS_HINT_FILL, EVAS_HINT_FILL);


	Elm_Genlist_Item_Class *itc = NULL;
	itc = elm_genlist_item_class_new();
	/*itc->item_style = "settings.1text.1icon.1"; //"1text.1icon.1"; */
	itc->item_style = "1text.1icon.1"; /*"1text.1icon.1"; */
	itc->func.text_get = _gl_ringtone_text_get;
	itc->func.content_get = _gl_ringtone_radio_get;

	Evas_Object *genlist;
	genlist = elm_genlist_add(popup);
	elm_object_style_set(genlist, "popup");
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist,ad);

	DBG("---> ringtone_count %d  to GENLIST", ringtone_count);
	for (index = 0; index < ringtone_count; index++) {
		/*DBG("---> add item to list %d  to GENLIST", index); */
		Item_Data *item = (Item_Data *)calloc(sizeof(Item_Data), 1);
		if (item) {
			item->index = index;
			item->item = elm_genlist_item_append(genlist,
					itc,
					item,
					NULL,
					ELM_GENLIST_ITEM_NONE,
					_ringtone_type_gl_cb,
					(void *)index);
		}
	}

	ad->ringtone_type_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->ringtone_type_rdg, ringtone_count);
	elm_radio_value_set(ad->ringtone_type_rdg, ringtone_type);
	confirmed_ringtone_type = ringtone_type;

	evas_object_data_set(genlist, "radio_main", ad->ringtone_type_rdg);

	g_ringtone_type_genlist = genlist;

	elm_object_part_content_set(popup, "elm.genlist", genlist);
	/*	evas_object_show(popup); */
	evas_object_show(genlist);

	elm_genlist_item_class_free(itc);

	btn = elm_button_add(popup);
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, _("IDS_ST_BUTTON_CANCEL_ABB2"));
	elm_object_part_content_set(popup, "btn.left", btn);
	evas_object_smart_callback_add(btn, "clicked", _response_ringtone_cancel_cb, ad);

	btn = elm_button_add(popup);
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, _("IDS_WNOTI_BUTTON_OK_ABB2"));
	elm_object_part_content_set(popup, "btn.right", btn);
	evas_object_smart_callback_add(btn, "clicked", _response_ringtone_ok_cb, ad);

	nf_it = elm_naviframe_item_push(ad->nf, _("IDS_ST_HEADER_RINGTONES_ABB"), NULL, NULL, popup, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _back_sound_naviframe_cb, ad);
	//ea_object_event_callback_add(ad->nf, EA_CALLBACK_BACK, _ringtone_back_cb, ad);

	/*VCONFKEY_PM_STATE */
	register_vconf_changing(VCONFKEY_PM_STATE, pm_state_vconf_changed_cb, NULL);
	DBG("LEAVE _show_ringtone_popup_cb");
}


#if 0 // _NOT_USED_
static void _notification_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL)
		return;

	notification_type = confirmed_Notification_type;

	_stop_all_sound_play();

	unregister_vconf_changing(VCONFKEY_PM_STATE, pm_state_vconf_changed_cb);
}
#endif

static void _response_notification_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	/*stop_wav(); */

	if (is_created_player()) {
		_close_player(NULL, SOUND_TYPE_NOTIFICATION);
	}

	/* save a notification type. */
	vconf_set_str(VCONFKEY_SETAPPL_NOTI_MSG_RINGTONE_PATH_STR , notification_arr[notification_type]);

	if (g_notification_type_genlist != NULL) {
		evas_object_del(g_notification_type_genlist);
		g_notification_type_genlist = NULL;
	}

	if (temp_ad->notification_rdg != NULL) {
		evas_object_del(temp_ad->notification_rdg);
		temp_ad->notification_rdg = NULL;
	}

	confirmed_Notification_type = notification_type;

	appdata *ad = (appdata *)data;
	if (ad != NULL) {
		elm_naviframe_item_pop(ad->nf);
	}

	if (g_sound_genlist != NULL) {
		elm_genlist_realized_items_update(g_sound_genlist);
	}
}

static void _response_notification_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (g_notification_type_genlist != NULL) {
		evas_object_del(g_notification_type_genlist);
		g_notification_type_genlist = NULL;
	}

	appdata *ad = (appdata *)data;
	if (ad != NULL) {
		elm_naviframe_item_pop(ad->nf);
	}

	notification_type = confirmed_Notification_type;

	if (is_created_player()) {
		_close_player(NULL, SOUND_TYPE_RINGTONE);
	}

	/*stop_wav(); */
}

static char *_gl_notification_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *item = data;
	if (item == NULL)
		return NULL;

	char buf[1024];

	sprintf(buf, "%s", notification_name_arr[item->index % notification_count]);

	return strdup(buf);
}

static Evas_Object *_gl_notification_radio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_object_style_set(radio, "list");
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);
		if (id->index == notification_type) {
			elm_radio_value_set(radio_main, id->index);
		}
		index++;
	}
	return radio;
}

static void _notification_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	notification_type = (int)data;

#if 0
	is_wav_playing = SOUND_STATE_PLAY;
	wav_player_start(notification_arr[notification_type], SOUND_TYPE_NOTIFICATION, NULL, (void *)2, &sound_id);
#endif

	int volume = 0;
	sound_manager_get_volume(SOUND_TYPE_NOTIFICATION, &volume);

	if (volume == 0 || get_sound_mode() != SOUND_MODE_SOUND) {
		if (temp_ad) {
			struct _toast_data *toast = _create_toast(temp_ad, _("IDS_ST_TPOP_VOLUME_CURRENTLY_SET_TO_0"));
			if (toast) {
				_show_toast(temp_ad, toast);
			}
		}
	} else {
		play_sound(notification_arr[notification_type], 0.0, SOUND_TYPE_NOTIFICATION);
		set_looping(FALSE);
	}

	if (g_notification_type_genlist) {
		elm_genlist_realized_items_update(g_notification_type_genlist);
	}
}

void _show_notification_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup, *btn;
	Elm_Object_Item *nf_it = NULL;
	unsigned int index;
	appdata *ad = data;

	if (ad == NULL)
		return;

	ad->MENU_TYPE = SETTING_SOUND_NOTIFICATION;

	if (is_loaded_noti_data == 0) {
		get_sound_file_list(TZ_SYS_RO_APP_D"/org.tizen.watch-setting/shared/res/settings/Alerts/", 0);
		is_loaded_noti_data = 1;

		/*sorting_file_list(0); */
	}

	cur_sound_type = SOUND_TYPE_NOTIFICATION;

	popup = elm_layout_add(ad->win_main);
	elm_layout_file_set(popup, EDJE_PATH, "setting/genlist/2button-layout");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(popup, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Elm_Genlist_Item_Class *itc = NULL;
	itc = elm_genlist_item_class_new();
	/*itc->item_style = "settings.1text.1icon.1"; //"1text.1icon.1"; */
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_notification_text_get;
	itc->func.content_get = _gl_notification_radio_get;

	Evas_Object *genlist;
	genlist = elm_genlist_add(popup);
	elm_object_style_set(genlist, "popup");
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist,ad);


	for (index = 0; index < notification_count; index++) {
		Item_Data *item = (Item_Data *)calloc(sizeof(Item_Data), 1);
		if (item) {
			item->index = index;
			item->item = elm_genlist_item_append(genlist,
					itc,
					item,
					NULL,
					ELM_GENLIST_ITEM_NONE,
					_notification_gl_cb,
					(void *)index);
		}
	}

	ad->notification_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->notification_rdg, notification_count);
	elm_radio_value_set(ad->notification_rdg, notification_type);
	confirmed_Notification_type = notification_type;

	evas_object_data_set(genlist, "radio_main", ad->notification_rdg);

	g_notification_type_genlist = genlist;

	elm_object_part_content_set(popup, "elm.genlist", genlist);
	evas_object_show(genlist);
	elm_genlist_item_class_free(itc);

	btn = elm_button_add(popup);
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, _("IDS_ST_BUTTON_CANCEL_ABB2"));
	elm_object_part_content_set(popup, "btn.left", btn);
	evas_object_smart_callback_add(btn, "clicked", _response_notification_cancel_cb, ad);

	btn = elm_button_add(popup);
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, _("IDS_WNOTI_BUTTON_OK_ABB2"));
	elm_object_part_content_set(popup, "btn.right", btn);
	evas_object_smart_callback_add(btn, "clicked", _response_notification_ok_cb, ad);

	nf_it = elm_naviframe_item_push(ad->nf, _("IDS_ST_BUTTON_NOTIFICATIONS"), NULL, NULL, popup, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _back_sound_naviframe_cb, ad);
	//ea_object_event_callback_add(ad->nf, EA_CALLBACK_BACK, _notification_back_cb, ad);

	/*VCONFKEY_PM_STATE */
	register_vconf_changing(VCONFKEY_PM_STATE, pm_state_vconf_changed_cb, NULL);
}

static void _response_vibration_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	/* save a vibration level */
	int level = 0;

	_haptic_close();

	switch (vibrate_type) {
		case VIBRATION_LEVEL_LOW:
			vconf_set_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, VIBRATION_LEVEL_LOW_INT);
			vconf_set_int(VCONFKEY_SETAPPL_TOUCH_FEEDBACK_VIBRATION_LEVEL_INT, VIBRATION_LEVEL_LOW_INT);
			break;
		case VIBRATION_LEVEL_HIGH:
			vconf_set_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, VIBRATION_LEVEL_HIGH_INT);
			vconf_set_int(VCONFKEY_SETAPPL_TOUCH_FEEDBACK_VIBRATION_LEVEL_INT, VIBRATION_LEVEL_HIGH_INT);
			break;
		default:
			break;
	}

	vconf_get_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, &level);
	DBG("Setting - vibration level : %d", level);

	if (g_vibration_type_genlist != NULL) {
		evas_object_del(g_vibration_type_genlist);
		g_vibration_type_genlist = NULL;
	}

	if (temp_ad->vibration_rdg != NULL) {
		evas_object_del(temp_ad->vibration_rdg);
		temp_ad->vibration_rdg = NULL;
	}

	appdata *ad = (appdata *)data;
	if (ad != NULL) {
		elm_naviframe_item_pop(ad->nf);
	}

	if (g_vib_item != NULL) {
		elm_genlist_item_update(g_vib_item);
	}
}

static void _response_vibration_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	_haptic_close();

	vconf_get_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, &vibrate_type);

	DBG("Setting - vibrate type have backed : %d", vibrate_type);

	if (g_vibration_type_genlist != NULL) {
		evas_object_del(g_vibration_type_genlist);
		g_vibration_type_genlist = NULL;
	}

	appdata *ad = (appdata *)data;
	if (ad != NULL) {
		elm_naviframe_item_pop(ad->nf);
	}
}

static char *_gl_vibration_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *item = data;
	if (item == NULL)
		return NULL;

	return strdup(_(vibration_str[item->index % 2]));
}

static Evas_Object *_gl_vibration_radio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_object_style_set(radio, "list");
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);
		evas_object_smart_callback_add(radio, "changed", _vibration_gl_cb, (void *)id->index);
		evas_object_propagate_events_set(radio, EINA_FALSE);

		if (vibrate_type == id->index) {
			elm_radio_value_set(radio_main, vibrate_type);
		}
		index++;
	}
	return radio;
}

static void _vibration_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	vconf_get_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, &origin_vibration_level);

	vibrate_type = (int)data;

	int level = VIBRATION_LEVEL_LOW_INT;
	int feedback_rate = SETTING_VIB_MEDIUM_RATE;
	switch (vibrate_type) {
			/*	case 2: */
			/*		level = VIBRATION_LEVEL_LOW_INT; */
			/*		feedback_rate = SETTING_VIB_WEAK_RATE; */
			/*		break; */
		case 1:
			level = VIBRATION_LEVEL_LOW_INT;
			feedback_rate = SETTING_VIB_WEAK_RATE;
			break;
		case 0:
			level = VIBRATION_LEVEL_HIGH_INT;
			feedback_rate = SETTING_VIB_STRONG_RATE;
			break;
	}

	DBG("Setting - feedback level: %d, rate: %d", level, feedback_rate);

	_start_vibration(level, feedback_rate, SETTING_DEFAULT_NOTIFICATION_GENERAL_PREVIEW_VIB);

	elm_genlist_realized_items_update(g_vibration_type_genlist);
}

static void _vibration_layout_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _vibration_layout_del_cb() is called!");

	_haptic_close();

	if (g_vib_item) {
		elm_genlist_item_update(g_vib_item);
	}
}

static int get_vibration_type()
{
	int type = 1;

	vconf_get_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, &origin_vibration_level);
	switch (origin_vibration_level) {
		case VIBRATION_LEVEL_LOW_INT:
			type = 1;
			break;
			/*	case VIBRATION_LEVEL_MID_INT: */
			/*		type = 1; */
			/*		break; */
		case VIBRATION_LEVEL_HIGH_INT:
			type = 0;
			break;
	}

	return type;
}

void _show_vibration_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup, *btn;
	Elm_Object_Item *nf_it = NULL;
	unsigned int index;
	appdata *ad = data;

	vibrate_type = get_vibration_type();

	popup = elm_layout_add(ad->win_main);
	elm_layout_file_set(popup, EDJE_PATH, "setting/genlist/2button-layout");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(popup, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _vibration_layout_del_cb, ad);

	Elm_Genlist_Item_Class *itc = NULL;
	itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_vibration_text_get;
	itc->func.content_get = _gl_vibration_radio_get;

	Evas_Object *genlist;
	genlist = elm_genlist_add(popup);
	elm_object_style_set(genlist, "popup");
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	connect_to_wheel_with_genlist(genlist,ad);

	int count = sizeof(vibration_str) / sizeof(vibration_str[0]);

	for (index = 0; index < count; index++) {
		Item_Data *item = (Item_Data *)calloc(sizeof(Item_Data), 1);
		if (item) {
			item->index = index;
			item->item = elm_genlist_item_append(genlist,
					itc,
					item,
					NULL,
					ELM_GENLIST_ITEM_NONE,
					_vibration_gl_cb,
					(void *)index);
		}
	}

	ad->vibration_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->vibration_rdg, 3);
	elm_radio_value_set(ad->vibration_rdg, notification_type);

	evas_object_data_set(genlist, "radio_main", ad->vibration_rdg);

	g_vibration_type_genlist = genlist;

	elm_object_part_content_set(popup, "elm.genlist", genlist);
	evas_object_show(genlist);
	elm_genlist_item_class_free(itc);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, _("IDS_ST_BUTTON_CANCEL_ABB2"));
	elm_object_part_content_set(popup, "btn.left", btn);
	evas_object_smart_callback_add(btn, "clicked", _response_vibration_cancel_cb, ad);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, _("IDS_WNOTI_BUTTON_OK_ABB2"));
	elm_object_part_content_set(popup, "btn.right", btn);
	evas_object_smart_callback_add(btn, "clicked", _response_vibration_ok_cb, ad);

	nf_it = elm_naviframe_item_push(ad->nf, _("IDS_ST_HEADER_VIBRATION_ABB"), NULL, NULL, popup, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _back_sound_naviframe_cb, ad);
}

static char *_gl_pref_arm_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(pref_arm_str[index % 2]));
		index++;
	}
	return strdup(buf);
}

static Evas_Object *_gl_pref_arm_ridio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;
	int index = id->index;
	int prefered_arm_type = 0;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);
		evas_object_smart_callback_add(radio, "changed", NULL, (void *)id->index);

		vconf_get_bool(VCONFKEY_SETAPPL_PERFERED_ARM_LEFT_BOOL, &prefered_arm_type);

		prefered_arm_type = (prefered_arm_type) ? 0 : 1;

		if (pref_arm_type == id->index) {
			elm_radio_value_set(radio_main, pref_arm_type);
		}
		index++;
	}
	return radio;
}

static void _pref_arm_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}

static void _pref_arm_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	pref_arm_type = (int)data;

	elm_genlist_realized_items_update(g_pref_arm_type_genlist);

	if (temp_ad) {
		elm_naviframe_item_pop(temp_ad->nf);
		if (!temp_ad->pref_arm_rdg) {
			evas_object_del(temp_ad->pref_arm_rdg);
			temp_ad->pref_arm_rdg = NULL;
		}
	}
	/* save a perfered arm type */
	vconf_set_bool(VCONFKEY_SETAPPL_PERFERED_ARM_LEFT_BOOL, (pref_arm_type == 0) ? TRUE : FALSE);

	if (g_sound_genlist) {
		elm_genlist_realized_items_update(g_sound_genlist);
	}

	/* show toast message */
	if (temp_ad) {
		struct _toast_data *toast = _create_toast(temp_ad, _(pref_arm_toast_msg[pref_arm_type % 2]));
		if (toast) {
			_show_toast(temp_ad, toast);
		}
	}
}

void _show_pref_arm_mode_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_pref_arm_mode_list - appdata is null");
		return;
	}
	Evas_Object *genlist  = NULL;
	Elm_Object_Item *item = NULL;
	Elm_Object_Item *nf_it = NULL;
	/*struct _dt_menu_item *menu_its = NULL; */

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_pref_arm_title_get;
	itc->func.content_get = _gl_pref_arm_ridio_get;
	itc->func.del = _pref_arm_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	connect_to_wheel_with_genlist(genlist,ad);

	Item_Data *id = calloc(sizeof(Item_Data), 1);
	if (id) {
		id->index = 0;
		item = elm_genlist_item_append(genlist,	itc, id, NULL, ELM_GENLIST_ITEM_NONE, _pref_arm_gl_cb, (void *)0);
		id->item = item;
	}

	Item_Data *id2 = calloc(sizeof(Item_Data), 1);
	if (id2) {
		id2->index = 1;
		item = elm_genlist_item_append(genlist, itc, id2, NULL, ELM_GENLIST_ITEM_NONE, _pref_arm_gl_cb, (void *)1);
		id2->item = item;
	}

	ad->pref_arm_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->pref_arm_rdg, 2);
	elm_radio_value_set(ad->pref_arm_rdg, pref_arm_type);

	evas_object_data_set(genlist, "radio_main", ad->pref_arm_rdg);

	elm_genlist_item_class_free(itc);

	g_pref_arm_type_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}
