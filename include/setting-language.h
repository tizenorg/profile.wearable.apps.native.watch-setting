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
 * setting-language.h
 *
 *  Created on: Oct 9, 2013
 *      Author: min-hoyun
 */

#ifndef SETTING_LANGUAGE_H_
#define SETTING_LANGUAGE_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "util.h"

#define LANGLIST_FILE_PATH_IN_RW		TZ_SYS_DATA_D"/setting/langlist.xml"
#define LANGLIST_FILE_PATH_IN_RO		TZ_SYS_RO_APP_D"/org.tizen.watch-setting/data/langlist.xml"
/*#define LANGLIST_FILE_PATH		"/usr/apps/org.tizen.watch-setting/data/langlist_all.xml" */

#define ITEM_COUNT				2

#if defined(FEATURE_SETTING_SDK) || defined(FEATURE_SETTING_EMUL)
#define LANGUAGE_ITEM_COUNT	49
#else
#define LANGUAGE_ITEM_COUNT	84
#endif

struct _lang_menu_item {
	char *name;
	char *sub_name;
	char *id;
	char *lang;
};

appdata *tmp_ad;
static Eina_List *s_langlist;
static void (*lang_update_cb)(void *);


char *_gl_lang_title_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_gl_lang_ridio_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_create_lang_list(void *data);
void _lang_sel_changed_cb(void *data, Evas_Object *obj, void *event_info);
void _gl_lang_sel_cb(void *data, Evas_Object *obj, void *event_info);
void _clear_lang_cb(void *data , Evas *e, Evas_Object *obj, void *event_info);
void _initialize_language(void *data);
void _set_launguage_update_cb(void (*cb)(void *));

void _langlist_load();
Eina_List *_get_language_list();
void _langlist_destroy();
static void _parseLangListXML(char *docname);
static void _tree_walk_langlist(xmlNodePtr cur);



#endif /* SETTING_LANGUAGE_H_ */
