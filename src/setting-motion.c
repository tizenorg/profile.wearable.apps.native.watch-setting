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
 * setting-motion.c
 *
 *  Created on: Oct 9, 2013
 *      Author: min-hoyun
 */

/*#include <capability_manager.h> */
#include "setting-motion.h"


static struct _motion_menu_item motion_menu_its[] = {
	{ "IDS_WMGR_MBODY_SMART_RELAY", 	 0, _motion_gl_smart_relay_cb },
	{ "IDS_WMGR_MBODY_WAKE_UP_GESTURE",  0, _motion_gl_wake_up_cb     },
	{ NULL, 0, NULL }
};

static char *wake_up_gesture_str[] = {
	"IDS_ST_BODY_OFF_M_STATUS",
	"IDS_ST_BUTTON_CLOCK",
	"IDS_WMGR_OPT_LAST_VIEWED_SCREEN"
};

static struct _motion_data motion_data;
static int is_called_myself;


static void init_motion_value()
{
	motion_data.is_enable_smart_relay = 0;
	motion_data.is_enable_wake_up_gesture = 0;
	motion_data.wake_up_gesture_type = SETTING_WAKE_UP_GESTURE_LAST_VIEWED_SCREEN;

	motion_data.temp_ad = NULL;
	motion_data.g_motion_genlist = NULL;
	is_called_myself = 0;
}

void _initialize_motion()
{
	init_motion_value();

	/* motion vconf changed callback */
	register_vconf_changing(VCONFKEY_WMS_WAKEUP_BY_GESTURE_SETTING, motion_vconf_changed_cb, NULL);
	register_vconf_changing(VCONFKEY_WMS_SMART_RELAY, motion_vconf_changed_cb, NULL);
}

void motion_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - motion_vconf_changed_cb() is called!");

	char *vconf_name = vconf_keynode_get_name(key);
	if (!vconf_name || is_called_myself) {
		is_called_myself = 0;
		return;
	}

	if (!strcmp(vconf_name, VCONFKEY_WMS_WAKEUP_BY_GESTURE_SETTING)) {
		vconf_get_int(VCONFKEY_WMS_WAKEUP_BY_GESTURE_SETTING, &motion_data.wake_up_gesture_type);

		DBG("Setting - wake_up_gesture_type : %d", motion_data.wake_up_gesture_type);
	} else if (!strcmp(vconf_name, VCONFKEY_WMS_SMART_RELAY)) {
		vconf_get_bool(VCONFKEY_WMS_SMART_RELAY, &motion_data.is_enable_smart_relay);

		DBG("Setting - is_enable_smart_relay : %d", motion_data.is_enable_smart_relay);
	}

	if (motion_data.g_motion_genlist) {
		/*elm_genlist_realized_items_update(motion_data.g_motion_genlist); */
		elm_radio_value_set(motion_data.temp_ad->wake_up_guesture_rdg, motion_data.wake_up_gesture_type);
	}
}

Eina_Bool _clear_motion_cb(void *data, Elm_Object_Item *it)
{
	init_motion_value();

	/* unregister motion vconf changed callback */
	unregister_vconf_changing(VCONFKEY_WMS_WAKEUP_BY_GESTURE_SETTING, motion_vconf_changed_cb);
	unregister_vconf_changing(VCONFKEY_WMS_SMART_RELAY, motion_vconf_changed_cb);

	return EINA_TRUE;
}

void _motion_gl_smart_relay_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("%s", "_motion_gl_smart_relay_cb - ad or check is null");
		return;
	}

	is_called_myself = 1;

	motion_data.is_enable_smart_relay = (motion_data.is_enable_smart_relay) ? 0 : 1;

	vconf_set_bool(VCONFKEY_WMS_SMART_RELAY, motion_data.is_enable_smart_relay);

	elm_genlist_item_selected_set(it, EINA_FALSE);
	elm_genlist_item_update(it);
}

void _motion_gl_wake_up_guesture_radio_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);

	int wake_up_mode = (int)data;

	is_called_myself = 1;

	vconf_set_int(VCONFKEY_WMS_WAKEUP_BY_GESTURE_SETTING, wake_up_mode);

	elm_genlist_item_update(it);

	elm_naviframe_item_pop(motion_data.temp_ad->nf);
	if (motion_data.temp_ad && motion_data.temp_ad->wake_up_guesture_rdg) {
		evas_object_del(motion_data.temp_ad->wake_up_guesture_rdg);
		motion_data.temp_ad->wake_up_guesture_rdg = NULL;
	}

	if (motion_data.g_motion_genlist) {
		elm_genlist_realized_items_update(motion_data.g_motion_genlist);
	}
}

char *_get_wake_up_gesture_sub_title()
{
	int gusture_mode = 0;
	vconf_get_int(VCONFKEY_WMS_WAKEUP_BY_GESTURE_SETTING, &gusture_mode);

	DBG("VCONFKEY_WMS_WAKEUP_BY_GESTURE_SETTING : %d", gusture_mode);

	return _(wake_up_gesture_str[gusture_mode % 3]);
}

char *_gl_wake_up_guesture_mode_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Motion_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text") || !strcmp(part, "elm.text.1")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(wake_up_gesture_str[index % 3]));
	}
	return strdup(buf);
}

Evas_Object *_gl_wake_up_guesture_ridio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;
	int index = id->index;
	int wake_up_mode = 0;

	if (!strcmp(part, "elm.icon")) {
		DBG("radio get");
		radio = elm_radio_add(obj);
		elm_object_style_set(radio, "list");
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);
		evas_object_propagate_events_set(radio, EINA_FALSE);
		evas_object_repeat_events_set(radio, EINA_TRUE);
		evas_object_smart_callback_add(radio, "changed", NULL, (void *)id->index);

		vconf_get_int(VCONFKEY_WMS_WAKEUP_BY_GESTURE_SETTING, &wake_up_mode);
		if (wake_up_mode == id->index) {
			elm_radio_value_set(radio_main, wake_up_mode);
		}
	}
	return radio;
}

static void _wake_up_guesture_gl_del(void *data, Evas_Object *obj)
{
	/* FIXME: Unrealized callback can be called after this. */
	/* Accessing Item_Data can be dangerous on unrealized callback. */
	struct _motion_wake_up_guesture_item *id = data;
	if (id)
		free(id);
}

void _show_wake_up_guesture_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_wake_up_guesture_list - appdata is null");
		return;
	}

	Evas_Object *genlist  = NULL;
	Elm_Object_Item *nf_it = NULL;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_wake_up_guesture_mode_title_get;
	itc->func.content_get = _gl_wake_up_guesture_ridio_get;
	itc->func.del = _wake_up_guesture_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	connect_to_wheel_with_genlist(genlist,ad);

	int idx;
	for (idx = 0; idx < MOTION_WAKE_UP_ITEM_COUNT; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist,	itc, id, NULL, ELM_GENLIST_ITEM_NONE,
					_motion_gl_wake_up_guesture_radio_cb,
					(void *)idx);
		}
	}
	motion_data.g_motion_genlist = genlist;
	ad->wake_up_guesture_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->wake_up_guesture_rdg, MOTION_WAKE_UP_ITEM_COUNT);
	elm_radio_value_set(ad->wake_up_guesture_rdg, 0);

	evas_object_data_set(genlist, "radio_main", ad->wake_up_guesture_rdg);

	elm_genlist_item_class_free(itc);

	motion_data.temp_ad = ad;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _clear_motion_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}

void _motion_gl_wake_up_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);

	_initialize_motion();

	appdata *ad = data;

	if (ad == NULL) {
		DBG("%s", "_motion_gl_smart_relay_cb - ad or check is null");
		return;
	}

	_show_wake_up_guesture_list(ad);
}

void _motion_chk_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *check = obj;

	if (check == NULL)
		return;

	/* check state callback */
	DBG("%s : %d", "_motion_chk_changed_cb - current state", elm_check_state_get(check));
}

char *_gl_motion_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Motion_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text.1")) {
		bool is_operator_dcm = 0;
		char *salescode = vconf_get_str(VCONFKEY_CSC_SALESCODE);
		if (salescode && (!strcmp(salescode, "DCM") || !strcmp(salescode, "KDI") || !strcmp(salescode, "XJP")))
			is_operator_dcm = 1;
		FREE(salescode);

		if (is_operator_dcm && !strcmp(motion_menu_its[index % 3].name, "IDS_WMGR_MBODY_SMART_RELAY")) {
			strncpy(buf, _("IDS_WMGR_MBODY_SMART_TOSS_JPN"), sizeof(buf) - 1);
		} else {
			snprintf(buf, sizeof(buf) - 1, "%s", _(motion_menu_its[index % 3].name));
		}
	} else if (!strcmp(part, "elm.text.2")) {
		if (index == 0) {
			vconf_get_bool(VCONFKEY_WMS_SMART_RELAY, &motion_data.is_enable_smart_relay);
			snprintf(buf, sizeof(buf) - 1, "%s", (motion_data.is_enable_smart_relay) ? _("IDS_ST_BODY_ON_M_STATUS") : _("IDS_ST_BODY_OFF_M_STATUS"));
		} else {
			snprintf(buf, sizeof(buf) - 1, "%s", _get_wake_up_gesture_sub_title());
		}
		index++;
	}
	return strdup(buf);
}

Evas_Object *_gl_motion_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;
	Motion_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);
		vconf_get_bool(VCONFKEY_WMS_SMART_RELAY, &motion_data.is_enable_smart_relay);
		elm_check_state_set(check, (motion_data.is_enable_smart_relay) ? EINA_TRUE : EINA_FALSE);
		evas_object_smart_callback_add(check, "changed", _motion_chk_changed_cb, (void *)1);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		index++;
	}

	return check;
}

static void _motion_gl_del(void *data, Evas_Object *obj)
{
	/* FIXME: Unrealized callback can be called after this. */
	/* Accessing Item_Data can be dangerous on unrealized callback. */
	Motion_Item_Data *id = data;
	if (id)
		free(id);
}

Evas_Object *_create_motion_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_motion_list - appdata is null");
		return NULL;
	}
	Evas_Object *genlist  = NULL;
	Elm_Genlist_Item_Class *temp_itc = NULL;
	struct _motion_menu_item *menu_its = NULL;
	int idx;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text.1icon.1";
	itc->func.text_get = _gl_motion_title_get;
	itc->func.content_get = _gl_motion_check_get;
	itc->func.del = _motion_gl_del;

	Elm_Genlist_Item_Class *itc_wake_up = elm_genlist_item_class_new();
	itc_wake_up->item_style = "2text";
	itc_wake_up->func.text_get = _gl_motion_title_get;
	itc_wake_up->func.del = _motion_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	connect_to_wheel_with_genlist(genlist,ad);
	menu_its = motion_menu_its;

	char *val = NULL;

	for (idx = 0; idx < MOTION_ITEM_COUNT; idx++) {
		if (idx == 0) {
			temp_itc = itc;
		} else {
			temp_itc = itc_wake_up;
		}

		Motion_Item_Data *id = calloc(sizeof(Motion_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
					genlist,				/* genlist object */
					temp_itc,				/* item class */
					id,		            	/* data */
					NULL,
					ELM_GENLIST_ITEM_NONE,
					menu_its[ idx ].func,	/* call back */
					ad);
		}
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc_wake_up);

	motion_data.g_motion_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	return layout;
}
