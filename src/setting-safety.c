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
 * setting-safety.c
 *
 *	Created on: Jan 14, 2014
 *		Author: min-hoyun
 */

#include "setting-safety.h"
#include "util.h"
#include "setting_debug.h"
#include "setting_data_vconf.h"

/*	*/
static void _emergency_mode_vconf_changed_cb(keynode_t *key, void *data);
static void _emergency_mode_cb(void *data, Evas_Object *obj, void *event_info);
#if 0 /* _NOT_USED_ */
static void _trauma_cb(void *data, Evas_Object *obj, void *event_info);
static void _no_activity_cb(void *data, Evas_Object *obj, void *event_info);
static void _set_interval_cb(void *data, Evas_Object *obj, void *event_info);
#endif
static void _help_cb(void *data, Evas_Object *obj, void *event_info);

#if 0 /* _NOT_USED	*/
static void show_interval_list(void *data);
static void _trauma_interval_cb(void *data, Evas_Object *obj, void *event_info);
static void _no_activity_interval_cb(void *data, Evas_Object *obj, void *event_info);
static void _show_interval_trauma_list(void *data);
static void _show_interval_no_activity_list(void *data);
#endif



static struct _safety_menu_item safety_menu_list[] = {
	{ "IDS_ST_MBODY_ULTRA_POWER_SAVING_MODE",		NULL,	_emergency_mode_cb },
	/*	{ "Trauma",				NULL,		  _trauma_cb		   }, */
	/*	{ "No activity",		NULL,		  _no_activity_cb	 }, */
	/*	{ "Set interval",		NULL,		  _set_interval_cb	 }, */
	{ "IDS_ST_MBODY_HELP",					NULL,		  _help_cb		   }
};

#if 0 /* _NOT_USED	*/
static struct _set_interval_menu_item interval_menu_list[] = {
	{ "Trauma",			"Warning %s",		_trauma_interval_cb		 },
	{ "No activity",	"Warning %s",		_no_activity_interval_cb }
};

static struct _interval_trauma_menu_item interval_trauma_time_arr[] = {
	{ "After %s seconds", "30" },
	{ "After %s minute" , "1"  },
	{ "After %s minutes", "5"  },
	{ "After %s minutes", "10" },
	{ "After %s minutes", "20" },
};

static char *interval_no_activity_hour_arr[] = {
	"6", "12", "24", "48"
};

#endif

static Safety_Data safety_data;
static Evas_Object *g_safety_genlist = NULL;


void initialize_safety()
{
	DBG("initialize safety");

	safety_data.g_safety_genlist = NULL;
	safety_data.temp_ad = NULL;

	safety_data.interval_no_activity = 0;
	safety_data.interval_trauma = 0;
	safety_data.is_enable_no_activity = 0;
	safety_data.is_enable_trauma = 0;
	safety_data.is_support_emergency = 0;

	vconf_get_bool(VCONFKEY_SETAPPL_EMERGENCY_STATUS_BOOL, &safety_data.is_enable_emergency_mode);
	register_vconf_changing(VCONFKEY_SETAPPL_EMERGENCY_STATUS_BOOL, _emergency_mode_vconf_changed_cb, NULL);
}

Eina_Bool clear_safety_cb(void *data, Elm_Object_Item *it)
{
	g_safety_genlist = NULL;

	unregister_vconf_changing(VCONFKEY_SETAPPL_EMERGENCY_STATUS_BOOL, _emergency_mode_vconf_changed_cb);

	return EINA_TRUE;
}

static void _emergency_mode_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - _emergency_mode_vconf_changed_cb() is called!");

	int enable = vconf_keynode_get_bool(key);

	if (enable == safety_data.is_enable_emergency_mode) {
		DBG("Setting - this value was set from Setting!!");
		return;
	} else {
		DBG("Setting - this value is set from WMS");

		safety_data.is_enable_emergency_mode = enable;
		if (g_safety_genlist) {
			elm_genlist_realized_items_update(g_safety_genlist);
		}
	}
}

static void _emergency_mode_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("emergency_mode_cb() is called.");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_emergency_mode_cb - ad is null");
		return;
	}

	if (safety_data.is_enable_emergency_mode) {
		/*confirm popup ok to off to device restart */
		_disable_emergency_popup_cb(data, obj, event_info);
	} else {
		safety_data.is_enable_emergency_mode = !safety_data.is_enable_emergency_mode;
		vconf_set_bool(VCONFKEY_SETAPPL_EMERGENCY_STATUS_BOOL, safety_data.is_enable_emergency_mode);

		/*DBG("Setting - Emergency mode is %d", safety_data.is_enable_emergency_mode); */
		/*elm_genlist_item_update(it); */
		elm_exit();	/*gonna be changed regarding freezer */
	}
}

#if 0 /* _NOT_USED_ */
static void _trauma_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("trauma_cb() is called.");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_emergency_mode_cb - ad is null");
		return;
	}

	safety_data.is_enable_trauma = !safety_data.is_enable_trauma;

	elm_genlist_item_update(it);
}

static void _no_activity_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("_no_activity_cb() is called.");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_no_activity_cb - ad is null");
		return;
	}

	safety_data.is_enable_no_activity = !safety_data.is_enable_no_activity;

	elm_genlist_item_update(it);
}

static void _set_interval_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("_set_interval_cb() is called.");

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_set_interval_cb - ad is null");
		return;
	}

	show_interval_list(ad);
}
#endif

static void _help_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("_help_cb() is called.");

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_help_cb - ad is null");
		return;
	}
	_help_popup_cb(data, obj, event_info);
}

static int _is_enable(int index)
{
	int enable = DISABLE;
	switch (index) {
	case 0:
		vconf_get_bool(VCONFKEY_SETAPPL_EMERGENCY_STATUS_BOOL, &safety_data.is_enable_emergency_mode);
		enable = safety_data.is_enable_emergency_mode;
		break;
	case 1:
		enable = safety_data.is_enable_trauma;
		break;
	case 2:
		enable = safety_data.is_enable_no_activity;
		break;
	case 3:
		break;
	case 4:
		break;
	}
	return enable;
}

static char *_gl_safety_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Safety_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(safety_menu_list[index].name));
	} else if (!strcmp(part, "elm.text.2")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _is_enable(index) ? _("IDS_EMAIL_BODY_ENABLED_M_STATUS") : _("IDS_ST_BODY_DISABLED_M_STATUS"));
	}
	return strdup(buf);
}

Evas_Object *_gl_safety_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	Safety_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);
		elm_check_state_set(check, _is_enable(index) ? EINA_TRUE : EINA_FALSE);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	}

	return check;
}

static void _gl_safety_del(void *data, Evas_Object *obj)
{
	Safety_Item_Data *id = data;
	if (id) {
		free(id);
	}
}

Evas_Object *create_safety_list(void *data)
{
	DBG("create_safety_list() is called.");

	appdata *ad = data;
	if (ad == NULL) {
		ERR("appdata is null!!");
		return NULL;
	}

	Evas_Object *genlist = NULL;
	Evas_Object *layout = NULL;
	Elm_Genlist_Item_Class *itc_temp = NULL;
	struct _safety_menu_item *menu_list = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc_1text_1check = elm_genlist_item_class_new();
	itc_1text_1check->item_style = "1text.1icon.1";
	itc_1text_1check->func.text_get = _gl_safety_title_get;
	itc_1text_1check->func.content_get = _gl_safety_check_get;
	itc_1text_1check->func.del = _gl_safety_del;

	Elm_Genlist_Item_Class *itc_2text_1check = elm_genlist_item_class_new();
	itc_2text_1check->item_style = "2text.1icon.1";
	itc_2text_1check->func.text_get = _gl_safety_title_get;
	itc_2text_1check->func.content_get = _gl_safety_check_get;
	itc_2text_1check->func.del = _gl_safety_del;

	Elm_Genlist_Item_Class *itc_1text = elm_genlist_item_class_new();
	itc_1text->item_style = "1text";
	itc_1text->func.text_get = _gl_safety_title_get;
	itc_1text->func.del = _gl_safety_del;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist, ad);

	menu_list = safety_menu_list;

	int count = 0;
	count = sizeof(safety_menu_list) / sizeof(struct _safety_menu_item);

	for (idx = 0; idx < count; idx++) {
		if (idx == 0) {
			itc_temp = itc_2text_1check;
		}
#if 0
		else if (idx > 0 && idx < 3) {
			itc_temp = itc_1text_1check;
		}
#endif
		else {
			itc_temp = itc_1text;
		}

		Safety_Item_Data *id = calloc(sizeof(Safety_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
						   genlist,		/* genlist object */
						   itc_temp,		/* item class */
						   id,				/* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_list[idx].func,	/* call back */
						   ad);
			if (idx == 0) {
				vconf_get_bool("db/setting/support_emergency", &safety_data.is_support_emergency);
				/*not support list for emergency mode */
				if (!safety_data.is_support_emergency)
					elm_object_item_disabled_set(id->item, EINA_TRUE);
			}
		}
	}

	elm_genlist_item_class_free(itc_2text_1check);
	elm_genlist_item_class_free(itc_1text_1check);
	elm_genlist_item_class_free(itc_1text);

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	return layout;
}

#if 0 /* _NOT_USED	*/
static void _trauma_interval_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("_trauma_interval_cb() is called.");

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_interval_trauma_list(data);
}

static void _no_activity_interval_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("_no_activity_interval_cb() is called.");

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_interval_no_activity_list(data);
}

static char *_gl_interval_title_get(void *data, Evas_Object *obj, const char *part)
{
	DBG("_gl_interval_title_get() is called.");

	char buf[1024] = {0,};
	Safety_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text.1")) {
		snprintf(buf, sizeof(buf) - 1, "%s", interval_menu_list[index].title);
	} else if (!strcmp(part, "elm.text.2")) {
		snprintf(buf, sizeof(buf) - 1, interval_menu_list[index].sub_title, "after 30 seconds");
	}
	return strdup(buf);
}

static void show_interval_list(void *data)
{
	DBG("show_interval_list() is called.");

	appdata *ad = data;
	if (!ad) {
		ERR("appdata is null!!");
		return;
	}

	Evas_Object *genlist = NULL;
	Evas_Object *layout = NULL;
	Elm_Object_Item *nf_it = NULL;
	struct _set_interval_menu_item *menu_list = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc_2text = elm_genlist_item_class_new();
	itc_2text->item_style = "2text";
	itc_2text->func.text_get = _gl_interval_title_get;
	itc_2text->func.del = _gl_safety_del;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	menu_list = interval_menu_list;

	int count = 0;
	count = sizeof(interval_menu_list) / sizeof(struct _set_interval_menu_item);

	for (idx = 0; idx < count; idx++) {
		Safety_Item_Data *id = calloc(sizeof(Safety_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
						   genlist,		/* genlist object */
						   itc_2text,		/* item class */
						   id,				/* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_list[idx].func,	/* call back */
						   ad);
		}
	}

	elm_genlist_item_class_free(itc_2text);

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}


static char *_gl_interval_trauma_title_get(void *data, Evas_Object *obj, const char *part)
{
	DBG("_gl_interval_trauma_title_get() is called.");

	char buf[512] = {0,};
	Safety_Item_Data *id = data;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, interval_trauma_time_arr[id->index].str,
				 interval_trauma_time_arr[id->index].time);
	}
	return strdup(buf);
}

static Evas_Object *_gl_interval_trauma_ridio_get(void *data, Evas_Object *obj, const char *part)
{
	DBG("_gl_interval_trauma_ridio_get() is called.");

	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Safety_Item_Data *id = data;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);

		if (safety_data.interval_trauma == id->index) {
			elm_radio_value_set(radio_main, safety_data.interval_trauma);
		}
	}
	return radio;
}

static void _trauma_interval_radio_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("_trauma_interval_radio_cb is called!");

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	DBG("selected trauma interval : %d", (int *)data);

	if (safety_data.temp_ad) {
		elm_naviframe_item_pop(safety_data.temp_ad->nf);
	}
}

static void _show_interval_trauma_list(void *data)
{
	DBG("_show_interval_trauma_list() is called.");

	appdata *ad = data;
	if (ad == NULL) {
		ERR("%s", "_show_interval_trauma_list - appdata is null");
		return;
	}
	Evas_Object *genlist  = NULL;
	Elm_Object_Item *nf_it = NULL;
	int idx;

	safety_data.temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_interval_trauma_title_get;
	itc->func.content_get = _gl_interval_trauma_ridio_get;
	itc->func.del = _gl_safety_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist, ad);

	const int count = sizeof(interval_trauma_time_arr) / sizeof(interval_trauma_time_arr[0]);
	for (idx = 0; idx < count; idx++) {
		Safety_Item_Data *id = calloc(sizeof(Safety_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist,
											   itc,
											   id,
											   NULL,
											   ELM_GENLIST_ITEM_NONE,
											   _trauma_interval_radio_cb,
											   (void *)idx);
		}
	}

	ad->safety_interval_trauma_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->safety_interval_trauma_rdg, count);
	elm_radio_value_set(ad->safety_interval_trauma_rdg, count);

	evas_object_data_set(genlist, "radio_main", ad->safety_interval_trauma_rdg);

	elm_genlist_item_class_free(itc);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}

static char *_gl_interval_no_activity_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[512] = {0,};
	Safety_Item_Data *id = data;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "After %s hours", interval_no_activity_hour_arr[id->index]);
	}
	return strdup(buf);
}

static Evas_Object *_gl_interval_no_activity_ridio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Safety_Item_Data *id = data;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);

		if (safety_data.interval_no_activity == id->index) {
			elm_radio_value_set(radio_main, safety_data.interval_no_activity);
		}
	}
	return radio;
}

static void _no_activity_interval_radio_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("_no_activity_interval_radio_cb is called!");

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	DBG("selected no activity interval : %d", (int *)data);

	if (safety_data.temp_ad) {
		elm_naviframe_item_pop(safety_data.temp_ad->nf);
	}
}

static void _show_interval_no_activity_list(void *data)
{
	DBG("_show_interval_no_activity_list() is called.");

	appdata *ad = data;
	if (ad == NULL) {
		ERR("%s", "_show_interval_no_activity_list - appdata is null");
		return;
	}
	Evas_Object *genlist  = NULL;
	Elm_Object_Item *nf_it = NULL;
	int idx;

	safety_data.temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_interval_no_activity_title_get;
	itc->func.content_get = _gl_interval_no_activity_ridio_get;
	itc->func.del = _gl_safety_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist, ad);

	const int count = sizeof(interval_no_activity_hour_arr) / sizeof(interval_no_activity_hour_arr[0]);
	for (idx = 0; idx < count; idx++) {
		Safety_Item_Data *id = calloc(sizeof(Safety_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist,
											   itc,
											   id,
											   NULL,
											   ELM_GENLIST_ITEM_NONE,
											   _no_activity_interval_radio_cb,
											   (void *)idx);
		}
	}

	ad->safety_interval_no_activity_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->safety_interval_no_activity_rdg, count);
	elm_radio_value_set(ad->safety_interval_no_activity_rdg, count);

	evas_object_data_set(genlist, "radio_main", ad->safety_interval_no_activity_rdg);

	elm_genlist_item_class_free(itc);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}
#endif

void _help_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = NULL;
	Evas_Object *scroller = NULL;
	Evas_Object *label = NULL;

	appdata *ad = (appdata *) data;
	if (ad == NULL)
		return;

	popup = elm_popup_add(ad->nf);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(popup, "title,text", _("IDS_ST_MBODY_HELP"));

	ad->popup = popup;

	scroller = elm_scroller_add(popup);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_content_set(popup, scroller);
	evas_object_show(scroller);

	label = elm_label_add(scroller);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	char *txt = elm_entry_utf8_to_markup(_("IDS_ST_BODY_THE_HOME_SCREEN_WILL_BE_CHANGED_TO_THE_BLACK_THEME_TO_REDUCE_BATTERY_CONSUMPTION_MSG"));
	elm_object_text_set(label, txt);
	free(txt);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(scroller, label);
	evas_object_show(label);

	/*ea_object_event_callback_add(popup, EA_CALLBACK_BACK, setting_popup_back_cb, ad); */

	evas_object_show(popup);
}

static void _disable_emergency_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = (appdata *) data;
	if (ad == NULL)
		return;

	if (ad->popup) {
		evas_object_del(data);
		ad->popup = NULL;
	}
}

static void _disable_emergency_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;

	if (ad == NULL) {
		DBG("%s", "_disable_emergency_ok_cb : appdata is null");
		return;
	}

	if (ad->popup) {
		evas_object_del(data);
		ad->popup = NULL;
	}

	safety_data.is_enable_emergency_mode = EINA_FALSE;
	vconf_set_bool(VCONFKEY_SETAPPL_EMERGENCY_STATUS_BOOL, safety_data.is_enable_emergency_mode);
	vconf_set_int(VCONFKEY_SYSMAN_POWER_OFF_STATUS, VCONFKEY_SYSMAN_POWER_OFF_RESTART);
}

void _disable_emergency_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = NULL;
	Evas_Object *btn = NULL;
	Evas_Object *scroller = NULL;
	Evas_Object *label = NULL;

	appdata *ad = (appdata *) data;
	if (ad == NULL)
		return;

	popup = elm_popup_add(ad->nf);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(popup, "title,text", _("Disable Ultra power saving mode"));

	ad->popup = popup;

	scroller = elm_scroller_add(popup);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(popup, scroller);
	evas_object_show(scroller);

	label = elm_label_add(scroller);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	char *txt = elm_entry_utf8_to_markup(_("IDS_ST_POP_TO_DISABLE_ULTRA_POWER_SAVING_MODE_YOUR_DEVICE_WILL_RESTART"));
	elm_object_text_set(label, txt);
	free(txt);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(scroller, label);
	evas_object_show(label);

	/*ea_object_event_callback_add(popup, EA_CALLBACK_BACK, setting_popup_back_cb, ad); */

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, _("IDS_ST_BUTTON_CANCEL_ABB2"));
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", _disable_emergency_cancel_cb, popup);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, _("IDS_WNOTI_BUTTON_OK_ABB2"));
	elm_object_part_content_set(popup, "button2", btn);
	evas_object_smart_callback_add(btn, "clicked", _disable_emergency_ok_cb, popup);

	evas_object_show(popup);
}


