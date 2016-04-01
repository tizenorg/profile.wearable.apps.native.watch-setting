/*
 * Copyright (c) 2013-2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of
 * SAMSUNG ELECTRONICS ("Confidential Information").
 * You shall not disclose such Confidential Information and shall
 * use it only in accordance with the terms of the license agreement
 * you entered into with SAMSUNG ELECTRONICS.
 * SAMSUNG make no representations or warranties about the suitability
 * of the software, either express or implied, including but not
 * limited to the implied warranties of merchantability, fitness for
 * a particular purpose, or non-infringement.
 * SAMSUNG shall not be liable for any damages suffered by licensee as
 * a result of using, modifying or distributing this software or its derivatives.
 */

/*
 * setting-language.c
 *
 *  Created on: Oct 9, 2013
 *      Author: min-hoyun
 */

#include "setting_data_vconf.h"
#include <unicode/ustring.h>
#include <unicode/udat.h>
#include <unicode/udatpg.h>

#include <glib.h>

#include "setting-language.h"
#include "setting_view_toast.h"
#include "util.h"

static Eina_List *s_langlist;
static void (*lang_update_cb)(void *);
static void _parseLangListXML(char *docname);
static void _tree_walk_langlist(xmlNodePtr cur);

static struct _lang_menu_item lang_menu_its[LANGUAGE_ITEM_COUNT];
static Evas_Object *g_lang_radio = NULL;

static void change_language_enabling(keynode_t *key, void *data)
{
	if (is_connected_GM()) {
		DBG("Setting - language can not change!!");

		/* automatic freed!! */
		struct _toast_data *toast = _create_toast(tmp_ad, _("IDS_ST_TPOP_CHANGE_LANGUAGE_ON_MOBILE_DEVICE"));
		if (toast) {
			_show_toast(tmp_ad, toast);
		}
		if (tmp_ad) {
			elm_naviframe_item_pop(tmp_ad->nf);

		}
	}
}

void _initialize_language(void *data)
{
	g_lang_radio = NULL;
	tmp_ad = NULL;
	if (data) {
		tmp_ad = data;
	}

	register_vconf_changing(VCONFKEY_WMS_WMANAGER_CONNECTED, change_language_enabling, NULL);
}

void _set_launguage_update_cb(void (*cb)(void *))
{
	DBG("_set_launguage_update_cb() is called!");

	if (cb == NULL) {
		DBG("callback is NULL");
		return;
	}

	lang_update_cb = cb;
}

void _clear_g_lang_menu_items()
{
	DBG("_clear_g_lang_menu_items");

	int i;
	for (i = 0; i < LANGUAGE_ITEM_COUNT; i++) {
		if (lang_menu_its[i].name)
			free(lang_menu_its[i].name);
		if (lang_menu_its[i].sub_name)
			free(lang_menu_its[i].sub_name);
		if (lang_menu_its[i].id)
			free(lang_menu_its[i].id);
	}
}

void _clear_lang_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("_clear_lang_cb");

	appdata *ad = data;
	if (ad == NULL) {
		return;
	}

	if (ad->language_rdg) {
		ad->language_rdg = NULL;
	}

	g_lang_radio = NULL;

	_langlist_destroy();

	_clear_g_lang_menu_items();

	unregister_vconf_changing(VCONFKEY_WMS_WMANAGER_CONNECTED, change_language_enabling);

	ad->MENU_TYPE = SETTING_DISPLAY;
}

Eina_Bool _clear_lang_navi_cb(void *data , Elm_Object_Item *it)
{
	DBG("_clear_lang_cb");

	appdata *ad = data;
	if (ad == NULL) {
		return EINA_FALSE;
	}

	if (ad->language_rdg) {
		ad->language_rdg = NULL;
	}

	g_lang_radio = NULL;

	_langlist_destroy();

	_clear_g_lang_menu_items();

	unregister_vconf_changing(VCONFKEY_WMS_WMANAGER_CONNECTED, change_language_enabling);

	ad->MENU_TYPE = SETTING_DISPLAY;
	return EINA_TRUE;
}

Ecore_Timer *lang_timer = NULL;

#if 0 // _NOT_USED_
static Eina_Bool _update_language(void *data)
{
	char *locale = vconf_get_str(VCONFKEY_LANGSET);
	elm_language_set(locale);
	/*elm_config_all_flush(); */

	if (lang_update_cb) {
		lang_update_cb(tmp_ad);
	}

	lang_timer = NULL;

	return ECORE_CALLBACK_CANCEL;
}
#endif

static int _set_dateformat(const char *region)
{
	char *ret_str = NULL;
	UChar *uret = NULL;
	UChar customSkeleton[256] = { 0, };
	UErrorCode status = U_ZERO_ERROR;
	UDateTimePatternGenerator *pattern_generator;

	UChar bestPattern[256] = { 0, };
	char bestPatternString[256] = { 0, };
	char *skeleton = "yMd";

	uret = u_uastrncpy(customSkeleton, skeleton, strlen(skeleton));
	DBG("u_uastrncpy ret =%d", uret);

	pattern_generator = udatpg_open(region, &status);

	int32_t bestPatternCapacity =
	    (int32_t)(sizeof(bestPattern) / sizeof((bestPattern)[0]));
	(void)udatpg_getBestPattern(pattern_generator, customSkeleton,
	                            u_strlen(customSkeleton), bestPattern,
	                            bestPatternCapacity, &status);

	ret_str = u_austrcpy(bestPatternString, bestPattern);
	DBG("u_uastrncpy ret_str=%d", ret_str);

	int i = 0;
	int j = 0;
	int len = strlen(bestPatternString);
	char region_format[4] = {0, };
	int ymd[3] = {0, };
	/*  only save 'y', 'M', 'd' charactor */
	for (; i < len; i++) {
		if (bestPatternString[i] == 'y' && ymd[0] == 0) {
			region_format[j++] = bestPatternString[i];
			ymd[0] = 1;
		} else if (bestPatternString[i] == 'M' && ymd[1] == 0) {
			region_format[j++] = bestPatternString[i];
			ymd[1] = 1;
		} else if (bestPatternString[i] == 'd' && ymd[2] == 0) {
			region_format[j++] = bestPatternString[i];
			ymd[2] = 1;
		}
	}

	region_format[3] = '\0';

	char *date_format_str[4] = {
		"dMy", "Mdy", "yMd", "ydM"
	};
	int date_format_vconf_value = 1;	/*  default is "Mdy" */
	for (i = 0; i < 4; i++) {
		if (strlen(region_format) != 0 && !strcmp(region_format, date_format_str[i])) {
			date_format_vconf_value = i;
		}
	}

	DBG("bestPatternString : %s, format: %s, index: %d",
	    bestPatternString, region_format, date_format_vconf_value);

	vconf_set_int(VCONFKEY_SETAPPL_DATE_FORMAT_INT, date_format_vconf_value);

	return 1;
}

void _gl_lang_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	int lang_index = (int)data;

	char buf[32] = {0,};
	snprintf(buf, sizeof(buf) - 1, "%s.UTF-8", lang_menu_its[lang_index].id);

	vconf_set_str(VCONFKEY_LANGSET, buf);
	vconf_set_str(VCONFKEY_REGIONFORMAT, buf);

	_set_dateformat(lang_menu_its[lang_index].id);

	const char *temp = vconf_get_str(VCONFKEY_LANGSET);

	DBG("Setting - %s", temp);

	if (g_lang_radio) {
		elm_radio_value_set(g_lang_radio, lang_index);
	}

	char *locale = vconf_get_str(VCONFKEY_LANGSET);
	elm_language_set(locale);

	if (lang_update_cb) {
		lang_update_cb(tmp_ad);
	}

	if (tmp_ad) {
		elm_naviframe_item_pop(tmp_ad->nf);
	}
}

void _lang_sel_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	/* do nothing */
}

char *_gl_lang_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text.2")) {
		snprintf(buf, sizeof(buf) - 1, "%s", lang_menu_its[index].sub_name);
	} else if (!strcmp(part, "elm.text") || !strcmp(part, "elm.text.1")) {
		snprintf(buf, sizeof(buf) - 1, "%s", lang_menu_its[index].name);
	}
	return strdup(buf);
}

Evas_Object *_gl_lang_ridio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");

	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_object_style_set(radio, "list");
		elm_radio_state_value_set(radio, index);
		evas_object_smart_callback_add(radio, "changed", _lang_sel_changed_cb, (void *)index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(radio, EINA_FALSE);
		evas_object_repeat_events_set(radio, EINA_TRUE);

		elm_radio_group_add(radio, radio_main);

		const char *lang_set = vconf_get_str(VCONFKEY_LANGSET);
		char buf[32] = {0,};
		snprintf(buf, sizeof(buf) - 1, "%s.UTF-8", lang_menu_its[index].id);
		char *alt_lang_set = strdup(buf);

		if (!strcasecmp(lang_set, buf)) {
			elm_radio_value_set(radio_main, index);

			if (id->item) {
				elm_genlist_item_show(id->item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
			}
		}
		free(alt_lang_set);

		snprintf(buf, sizeof(buf) - 1, "%s.UTF8", lang_menu_its[index].id);
		char *alt_lang_set2 = strdup(buf);
		if (!strcasecmp(lang_set, buf)) {
			elm_radio_value_set(radio_main, index);

			if (id->item) {
				elm_genlist_item_show(id->item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
			}
		}
		free(alt_lang_set2);

		DBG("Setting - current language is %s", lang_set);

		index++;
	}

	return radio;
}

static void _lang_gl_del(void *data, Evas_Object *obj)
{
	/* FIXME: Unrealized callback can be called after this. */
	/* Accessing Item_Data can be dangerous on unrealized callback. */
	Item_Data *id = data;
	if (id)
		free(id);
}

Evas_Object *_create_lang_list(void *data)
{
	DBG("_create_lang_list:clear");

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_display_list - appdata is null");
		return NULL;
	}
	Evas_Object *genlist  = NULL;
	Elm_Genlist_Item_Class *itc_temp = NULL;
	int idx = 0;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text.1icon.1";
	itc->func.text_get = _gl_lang_title_get;
	itc->func.content_get = _gl_lang_ridio_get;
	itc->func.del = _lang_gl_del;

	Elm_Genlist_Item_Class *itc_1line = elm_genlist_item_class_new();
	itc_1line->item_style = "1text.1icon.1";
	itc_1line->func.text_get = _gl_lang_title_get;
	itc_1line->func.content_get = _gl_lang_ridio_get;
	itc_1line->func.del = _lang_gl_del;

	genlist = elm_genlist_add(layout);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist,ad);


	Eina_List *lang_list = _get_language_list();
	struct _lang_menu_item *node = NULL;
	int index = 0;

	int lang_count = eina_list_count(lang_list);

	DBG("Setting - language count: %d", lang_count);

	while (lang_list) {
		node = (struct _lang_menu_item *) eina_list_data_get(lang_list);
		if (node) {
			lang_menu_its[index].name     = strdup(node->name);
			if (strlen(node->sub_name) > 0)
				lang_menu_its[index].sub_name = strdup(node->sub_name);
			else
				lang_menu_its[index].sub_name = NULL;
			lang_menu_its[index].id		  = strdup(node->id);

			index++;
		}
		lang_list = eina_list_next(lang_list);
	}

	for (idx = 0; idx < lang_count; idx++) {
		if (lang_menu_its[idx].sub_name != NULL) {
			itc_temp = itc;
		} else {
			itc_temp = itc_1line;
		}
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
					genlist,				/* genlist object */
					itc_temp,				/* item class */
					id,		            	/* data */
					NULL,
					ELM_GENLIST_ITEM_NONE,
					_gl_lang_sel_cb, 		/* call back */
					(const void *)idx);
		}
	}

	ad->language_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->language_rdg, lang_count);
	elm_radio_value_set(ad->language_rdg, lang_count);

	g_lang_radio = ad->language_rdg;

	evas_object_data_set(genlist, "radio_main", ad->language_rdg);

	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc_1line);

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	return layout;
}

void _langlist_load()
{
	if (!s_langlist) {
		if (is_file_exist(LANGLIST_FILE_PATH_IN_RW)) {
			_parseLangListXML(LANGLIST_FILE_PATH_IN_RW);
		} else {
			_parseLangListXML(LANGLIST_FILE_PATH_IN_RO);
		}
	}
}

Eina_List *_get_language_list()
{
	if (NULL == s_langlist) {
		_langlist_load();
	}
	return s_langlist;
}

void _langlist_destroy()
{
	Eina_List *list = s_langlist;
	struct _lang_menu_item *node;
	while (list) {
		node = (struct _lang_menu_item *) eina_list_data_get(list);
		if (node) {
			if (node->name)
				free(node->name);
			if (node->sub_name)
				free(node->sub_name);
			if (node->id)
				free(node->id);
		}
		list = eina_list_next(list);
	}
	s_langlist = eina_list_free(s_langlist);
}

static void _parseLangListXML(char *docname)
{
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(docname);
	if (doc == NULL) {
		DBG("Setting - Documentation is not parsed successfully");
		return;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		DBG("Setting - Empty documentation");
		xmlFreeDoc(doc);
		return;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "langlist")) {
		DBG("Setting - Documentation of the wrong type, root node != settings");
		xmlFreeDoc(doc);
		return;
	}

	cur = cur->xmlChildrenNode;
	_tree_walk_langlist(cur);

	if (doc) {
		xmlFreeDoc(doc);
	}

	return;
}

static char * setting_language_string_replace( char * str, char * orig, char * repl)
{
	static char buffer[124];
	char * ch;
	if( !(ch = strstr(str, orig)))
	{
		return str;
	}
	strncpy(buffer, str, ch - str);
	buffer[ch - str] = 0;
	sprintf(buffer + (ch - str), "%s%s", repl, ch + strlen(orig));

	return buffer;
}

static void _tree_walk_langlist(xmlNodePtr cur)
{
	xmlNode *cur_node = NULL;
	char *id = NULL;			/* ex. ko_KR */
	char *name = NULL;
	char *sub_name = NULL;

	for (cur_node = cur; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			id	 = (char *)g_strdup((char *)xmlGetProp(cur_node, (const xmlChar *)"id"));
			name = (char *)g_strdup((char *)xmlGetProp(cur_node, (const xmlChar *)"string"));
			/*mcc  = (char *)g_strdup((char*) xmlGetProp(cur_node, (const xmlChar *)"mcc")); */

			struct _lang_menu_item *pitem = (struct _lang_menu_item *)calloc(1, sizeof(struct _lang_menu_item));
			if (pitem) {
				pitem->id = id;
				pitem->name = strdup(name);

				sub_name = strdup(name);
				if (sub_name && pitem->name)
					pitem->sub_name = (char *)g_strdup(setting_language_string_replace(sub_name, pitem->name, ""));

				free(name);
				free(sub_name);

				s_langlist = eina_list_append(s_langlist, pitem);
			}
		}
	}
}

const char *setting_get_lang_title(void)
{
	DBG("%s", __func__);

	if (s_langlist == NULL) {
		_langlist_load();
	}

	Eina_List *lang_list = s_langlist;

	struct _lang_menu_item *lang_entry;

	char *title = NULL;
	char *language = NULL;
	char buf[32];

	language = vconf_get_str(VCONFKEY_LANGSET);

	DBG("current language : %s", language);

	if (language == NULL) {
		return NULL;
	}

	while (lang_list) {
		lang_entry = (struct _lang_menu_item *) eina_list_data_get(lang_list);
		if (lang_entry) {
			DBG("%s : language -> %s, locale -> %s", __func__, language, lang_entry->id);

			snprintf(buf, sizeof(buf) - 1, "%s.UTF-8", lang_entry->id);
			if (!strcmp(buf, language)) {
				char pull_title_buf[128];
				if (lang_entry->sub_name && strlen(lang_entry->sub_name) > 1)
					snprintf(pull_title_buf, sizeof(pull_title_buf) - 1, "%s %s", lang_entry->name, lang_entry->sub_name);
				else
					snprintf(pull_title_buf, sizeof(pull_title_buf) - 1, "%s", lang_entry->name);
				title = strdup(pull_title_buf);
				break;
			}
		}
		lang_list = eina_list_next(lang_list);
	}

	return title;
}
