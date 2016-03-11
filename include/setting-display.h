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
 * setting-display.h
 *
 *  Created on: Oct 9, 2013
 *      Author: min-hoyun
 */

#ifndef SETTING_DISPLAY_H_
#define SETTING_DISPLAY_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>

#define MENU_COUNT		7
#define FONT_SIZE_COUNT		3
#define SCREEN_TIME_COUNT	5
#define ROTATE_SCREEN_COUNT		4
#define SETTING_FONT_PRELOAD_FONT_PATH "/usr/share/fonts"
#define SETTING_FONT_DOWNLOADED_FONT_PATH TZ_SYS_SHARE_D"/fonts"

struct _display_menu_item {
	char *name;
	int type;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

struct _font_menu_item {
	char *name;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

typedef struct _Display_Item_Data {
	int index;
	Elm_Object_Item *item;
} Display_Item_Data;

typedef struct _Font_Style_Item_Data {
	int index;
	char *font_name;
	Elm_Object_Item *item;
} Font_Style_Item_Data;

enum {
    SETTING_DISPLAY_BRIGTHNESS,
    SETTING_DISPLAY_SCREEN_TIME,
    SETTING_DISPLAY_FONT,
    SETTING_DISPLAY_LANG,
    SETTING_DISPLAY_GESTURE,
    SETTING_DISPLAY_ICON_SIZE,
    SETTING_DISPLAY_EDIT_HOME,
    SETTING_DISPLAY_EDIT_APPS
};

enum {
    SETTING_DISPLAY_FONT_STYLE,
    SETTING_DISPLAY_FONT_SIZE
};


char *_gl_display_title_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_gl_display_check_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_create_display_list(void *data);
void _display_gl_Battery_percent_cb(void *data, Evas_Object *obj, void *event_info);
void _display_gl_screen_timeout_cb(void *data, Evas_Object *obj, void *event_info);
void _show_screen_timeout_list(void *data);
void _display_gl_language_cb(void *data, Evas_Object *obj, void *event_info);
void _display_gl_font_cb(void *data, Evas_Object *obj, void *event_info);
void _display_gl_font_style_cb(void *data, Evas_Object *obj, void *event_info);
void _display_gl_font_size_cb(void *data, Evas_Object *obj, void *event_info);
void _show_font_list(void *data);
int _show_font_style_list(void *data);
void _show_font_size_list(void *data);
void _display_gl_rotate_screen_cb(void *data, Evas_Object *obj, void *event_info);
void _show_rotate_screen_list(void *data);
static void _display_brightness_cb(void *data, Evas_Object *obj, void *event_info);
Evas_Object *_show_brightness_popup(void *data, Evas_Object *obj, void *event_info);
void _back_brightness_naviframe_cb(void *data, Elm_Object_Item *it);

void _init_display();
void _init_screen_rotate();
Eina_Bool _clear_display_cb(void *data, Elm_Object_Item *it);
static void _set_rotate_screen(const int rotation);
static int _get_rotate_screen();

#endif /* SETTING_DISPLAY_H_ */
