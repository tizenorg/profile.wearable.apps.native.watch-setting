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
 * setting-privacy.c
 *
 *  Created on: Jan 7, 2014
 *      Author: Sunyeop Hwang
 */


#include "setting-privacy.h"
#include "setting_data_vconf.h"
#include "util.h"

static void _privacy_lock_cb(void *data, Evas_Object *obj, void *event_info);
static void _privacy_see_pattern_cb(void *data, Evas_Object *obj, void *event_info);
static void _privacy_help_cb(void *data, Evas_Object *obj, void *event_info);
static void _privacy_pattern_enable_cb(void *data, Evas_Object *obj, void *event_info);
static void _privacy_pattern_disable_cb(void *data, Evas_Object *obj, void *event_info);
static void _create_privacy_pattern_list(void *data);

static struct _privacy_menu_item privacy_menu_list[] = {
	{ "IDS_LCKSCN_BODY_PRIVACY_LOCK_ABB",	_privacy_lock_cb },
	{ "IDS_ST_MBODY_HELP",			_privacy_help_cb }
};


static struct _privacy_menu_item privacy_pattern_menu_list[] = {
	{ "IDS_LCKSCN_HEADER_PIN",	_privacy_pattern_enable_cb },
	{ "IDS_LCKSCN_BODY_NONE",	_privacy_pattern_disable_cb }
};

static char *lock_type_str[] = {
	"IDS_LCKSCN_BODY_NONE",
	"IDS_LCKSCN_HEADER_PIN"
};

static Evas_Object *g_privacy_genlist = NULL;

int _get_lock_type_value()
{
	int value = 0;
	if (vconf_get_int("db/setting/lock_type", &value) != 0) {
		ERR("error get vconf value!!");
	}
	return value;
}

int _set_lock_type_value(int value)
{
	if (vconf_set_int("db/setting/lock_type", value) != 0) {
		ERR("error set vconf value!!");
		return FALSE;
	}
	return TRUE;
}

int _get_see_pattern_value()
{
	int value = 0;
	if (vconf_get_bool("db/setting/see_pattern", &value) != 0) {
		ERR("error get vconf value!!");
	}
	return value;
}

int _set_see_pattern_value(int value)
{
	if (vconf_set_bool("db/setting/see_pattern", value) != 0) {
		ERR("error set vconf value!!");
		return FALSE;
	}
	return TRUE;
}

static void _privacy_lock_setting_cb(app_control_h service, app_control_h reply, app_control_result_e result, void *data)
{
	appdata *ad = data;
	if (!ad) {
		ERR("appdata is null");
		return;
	}

	if (result == APP_CONTROL_RESULT_SUCCEEDED) {
		_set_lock_type_value(1);
		elm_naviframe_item_pop(ad->nf);

		if (g_privacy_genlist) {
			elm_genlist_realized_items_update(g_privacy_genlist);
		}
	}
}

static void _privacy_lock_verify_cb(app_control_h service, app_control_h reply, app_control_result_e result, void *data)
{
	appdata *ad = data;
	if (!ad) {
		ERR("appdata is null");
		return;
	}

	if (result == APP_CONTROL_RESULT_SUCCEEDED) {
		_create_privacy_pattern_list(ad);
	}
}

static void _gl_privacy_del(void *data, Evas_Object *obj)
{
	Privacy_Item_Data *id = data;
	if (id)
		free(id);
}

char *_gl_privacy_pattern_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;
	char *device_info = NULL;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(privacy_pattern_menu_list[index].name));
	}

	return strdup(buf);
}

static void _privacy_pattern_enable_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	appdata *ad = data;
	if (!ad) {
		ERR("appdata is null");
		return;
	}

	app_control_h service;
	app_control_create(&service);
	app_control_set_app_id(service, "org.tizen.w-lockscreen-setting");

	app_control_add_extra_data(service, "type", "setting");
	app_control_send_launch_request(service, _privacy_lock_setting_cb, ad);
	app_control_destroy(service);
}

static void _privacy_pattern_disable_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	appdata *ad = data;
	if (!ad) {
		ERR("appdata is null");
		return;
	}

	_set_lock_type_value(0);
	elm_naviframe_item_pop(ad->nf);

	if (g_privacy_genlist) {
		elm_genlist_realized_items_update(g_privacy_genlist);
	}
}

static void _create_privacy_pattern_list(void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return;
	}

	Evas_Object *genlist = NULL;
	Evas_Object *layout = NULL;
	Elm_Object_Item *nf_it = NULL;
	struct _privacy_menu_item *menu_list = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text";
	itc->func.text_get = _gl_privacy_pattern_title_get;
	itc->func.del = _gl_privacy_del;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);

	menu_list = privacy_pattern_menu_list;

	for (idx = 0; idx < sizeof(privacy_pattern_menu_list) / sizeof(struct _privacy_menu_item); idx++) {
		Privacy_Item_Data *id = calloc(sizeof(Privacy_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
					genlist,		/* genlist object */
					itc,			/* item class */
					id,		        /* data */
					NULL,
					ELM_GENLIST_ITEM_NONE,
					menu_list[idx].func,	/* call back */
					ad);
		}
	}

	elm_genlist_item_class_free(itc);

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}

char *_gl_privacy_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;
	char *device_info = NULL;

	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(privacy_menu_list[index].name));
	} else if (!strcmp(part, "elm.text.2")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(lock_type_str[_get_lock_type_value()]));
	}

	return strdup(buf);
}

Evas_Object *_gl_privacy_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	Privacy_Item_Data *id = data;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);
		elm_check_state_set(check, (_get_see_pattern_value()) ? EINA_TRUE : EINA_FALSE);
		/*evas_object_smart_callback_add(check, "changed", _see_pattern_chk_changed_cb, (void *)1); */
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		id->check = check;
	}
	return check;
}

static void _privacy_lock_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	appdata *ad = data;
	if (!ad) {
		ERR("appdata is null");
		return;
	}

	if (!_get_lock_type_value()) {
		_create_privacy_pattern_list(ad);
	} else {
		app_control_h service;
		app_control_create(&service);
		app_control_set_app_id(service, "org.tizen.w-lockscreen-setting");

		app_control_add_extra_data(service, "type", "verify");
		app_control_send_launch_request(service, _privacy_lock_verify_cb, ad);
		app_control_destroy(service);
	}
}

void _create_help_popup(void *data)
{
	Evas_Object *popup = NULL;
	Evas_Object *btn = NULL;
	Evas_Object *scroller = NULL;
	Evas_Object *label = NULL;

	appdata *ad = (appdata *) data;
	if (ad == NULL)
		return;

	popup = elm_popup_add(ad->nf);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_part_text_set(popup, "title,text", "IDS_ST_MBODY_HELP");

	ad->popup = popup;

	scroller = elm_scroller_add(popup);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_style_set(scroller, "effect");
	elm_object_content_set(popup, scroller);
	evas_object_show(scroller);

	label = elm_label_add(scroller);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_translatable_text_set(label, "IDS_LCKSCN_BODY_THE_PRIVACY_LOCK_OPTION_WILL_BE_SHOWN_WHEN_BLUETOOTH_IS_DISCONNECTED");

	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(scroller, label);
	evas_object_show(label);

	//ea_object_event_callback_add(popup, EA_CALLBACK_BACK, setting_popup_back_cb, ad);

	evas_object_show(popup);
}

static void _privacy_help_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_privcay_help_cb - ad is null");
		return;
	}

	_create_help_popup(ad);
}

Evas_Object *create_privacy_list(void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return NULL;
	}

	Evas_Object *genlist = NULL;
	Evas_Object *layout = NULL;
	struct _privacy_menu_item *menu_list = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc = NULL;

	Elm_Genlist_Item_Class *itc_2text = elm_genlist_item_class_new();
	itc_2text->item_style = "2text";
	itc_2text->func.text_get = _gl_privacy_title_get;
	itc_2text->func.del = _gl_privacy_del;

	Elm_Genlist_Item_Class *itc_1text = elm_genlist_item_class_new();
	itc_1text->item_style = "1text";
	itc_1text->func.text_get = _gl_privacy_title_get;
	itc_1text->func.del = _gl_privacy_del;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	/*elm_genlist_block_count_set(genlist, 14); */

	menu_list = privacy_menu_list;

	for (idx = 0; idx < sizeof(privacy_menu_list) / sizeof(struct _privacy_menu_item); idx++) {
		if (idx == 0) {
			itc = itc_2text;
		} else {
			itc = itc_1text;
		}

		Privacy_Item_Data *id = calloc(sizeof(Privacy_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
					genlist,		/* genlist object */
					itc,			/* item class */
					id,		        /* data */
					NULL,
					ELM_GENLIST_ITEM_NONE,
					menu_list[idx].func,	/* call back */
					ad);
		}
	}

	elm_genlist_item_class_free(itc_2text);
	elm_genlist_item_class_free(itc_1text);
	itc = NULL;

	g_privacy_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	return layout;
}

Eina_Bool clear_privacy_cb(void *data, Elm_Object_Item *it)
{
	g_privacy_genlist = NULL;

	return EINA_TRUE;
}

