/*
 * setting-theme.h
 *
 *  Created on: Aug 7, 2014
 *      Author: min-hoyun
 */

#ifndef SETTING_THEME_H_
#define SETTING_THEME_H_

#include <Elementary.h>

#include "util.h"

#define COLOR_THEME_COUNT	2

typedef struct _theme_data {
	appdata *ad;

	Evas_Object *theme_layout;
	Evas_Object *mapbuf[COLOR_THEME_COUNT];
	Evas_Object *scroller;
	Evas_Object *index;

	Elm_Object_Item *last_it;
	Elm_Object_Item *new_it;

	Elm_Object_Item *it[COLOR_THEME_COUNT];

	int curr_theme_id;
	int curr_page;
	int min_page;
	int max_page;
	int curr_theme_type;
} Theme_Data;

typedef struct _thumbnail_data {
	int id;
	int theme_type;
	int prev_img_path;
} Thumbnail_Data;


void setting_theme_show_thumbnail(void *data, Evas_Object *obj, void *event_info);

#endif /* SETTING_THEME_H_ */
