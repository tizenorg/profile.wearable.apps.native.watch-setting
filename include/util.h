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
#ifndef _UTIL_H_
#define _UTIL_H_

#include <Elementary.h>
#include <app.h>

#include "setting_debug.h"

#define IMG_DIR		"/usr/apps/org.tizen.watch-setting/data/images/"
#define EDJE_PATH 	"/usr/apps/org.tizen.watch-setting/res/edje/watch-setting.edj"

#ifndef FEATURE_SETTING_CHANGEABLE
#define FEATURE_SETTING_CHANGEABLE
#endif

#ifdef FEATURE_SETTING_CHANGEABLE
#define	COLOR_INFO_TABLE "/usr/apps/org.tizen.watch-setting/shared/res/tables/org.tizen.watch-setting_ChangeableColorInfo.xml"
#endif

#define TRUE	1
#define FALSE	0

#define SETTING_PACKAGE			"watch-setting"
#define SYSTEM_PACKAGE			"sys-string"

#define _(s)			setting_gettext(s)
#define REPL(s,o,r)	replace(s,o,r)
#define ICU_NUM(n)	_get_strnum_from_icu(n)

#define WIN_SIZE	320

enum {
    SETTING_CLOCK,
    SETTING_NOTIFICATION,
    SETTING_HOMESCREEN,
    SETTING_SOUND,
    SETTING_VOLUME,
    SETTING_DISPLAY,
    SETTING_BATTERY,
    SETTING_BLUETOOTH,
    SETTING_SCREEN_LOCK,
    SETTING_DOUBLE_PRESSING,
    SETTING_LANGUAGE,
    SETTING_SAFETY,
    SETTING_RESET,
    SETTING_INFO,
    SETTING_MAIN,
    SETTING_VOLUME_2_DEPTH,
    SETTING_SOUND_RINGTONE,
    SETTING_SOUND_NOTIFICATION
};

enum {
    DISABLE,
    ENABLE
};

enum {
    NONE,
    TOUCH_DOWN,
    TOUCH_UP,
    TOUCH_MOVE
};

typedef struct _appdata {
	Evas_Coord root_w;
	Evas_Coord root_h;

	Evas *evas;
	Evas_Object *win_main;
	Evas_Object *bg;
	Evas_Object *conform;
	Evas_Object *layout_main;
	Evas_Object *nf;
	Evas_Object *datetime;
	Evas_Object *alert_rdg;
	Evas_Object *sound_mode_rdg;
	Evas_Object *ringtone_type_rdg;
	Evas_Object *notification_rdg;
	Evas_Object *vibration_rdg;
	Evas_Object *pref_arm_rdg;
	Evas_Object *homescreen_rdg;
	Evas_Object *double_rdg;
	Evas_Object *safety_interval_trauma_rdg;
	Evas_Object *safety_interval_no_activity_rdg;

	Evas_Object *screen_timeout_rdg;
	Evas_Object *font_size_rdg;
	Evas_Object *font_style_rdg;
	Evas_Object *popup;
	Evas_Object *language_rdg;
	Evas_Object *wake_up_guesture_rdg;
	Evas_Object *rotate_screen_rdg;
	Elm_Object_Item *dt_genlist_item_of_time;
	Elm_Object_Item *dt_genlist_item_of_date;
	Elm_Object_Item *language_item;

	Evas_Object *main_genlist;

	char *device_name;

	int is_motion_smart_relay_on;
	int is_motion_wake_up_on;

	int is_show_ringtone_toast;
	int is_show_noti_toast;

	int is_first_launch;

	int MENU_TYPE;

	Evas_Object *indicator_layout;

} appdata;

typedef struct _Item_Data {
	int index;
	Elm_Object_Item *item;
} Item_Data;

typedef struct _spin_date spin_date;
struct _spin_date {
	Evas_Object *spinner;
	Evas_Object *layout;

	double posx1, posy1, posx2, posy2;
};

char *replace(char *str, char *orig, char *repl);
char *setting_gettext(const char *s);

void setting_popup_back_cb(void *data, Evas_Object *obj, void *event_info);

int is_connected_GM();

bool colorstr_to_decimal(char *color, int *R, int *G, int *B);

bool is_file_exist(char *file_path);

#endif
