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
 * setting-battery.c
 *
 *  Created on: Oct 12, 2013
 *      Author: min-hoyun
 */
#include <ail.h>
#include <aul.h>
#include "setting_data_vconf.h"

#include "setting-battery.h"
#include "util.h"

int percent;
bool is_charging;
bool prev_charging_state;
int image_index;
int is_alive;

Evas_Object *_create_battery_content2(void *data);
#if 0 // _NOT_USED_
static void _battery_percent_cb(void *data, Evas_Object *obj, void *event_info);
#endif
static void _power_saving_cb(void *data, Evas_Object *obj, void *event_info);
void _pws_popup_cb(void *data, Evas_Object *obj, void *event_info);

static struct _battery_menu_item battery_menu_list[] = {
	{ "IDS_ST_BODY_BATTERY_STATUS",			DISABLE, _battery_status_cb_gen_item },
	/*{ "IDS_ST_BODY_BATTERY_PERCENTAGE_ABB",	DISABLE,	_battery_percent_cb }, */
	{ "IDS_ST_MBODY_POWER_SAVER_ABB",	DISABLE,	_power_saving_cb }
};

char *battery_icons[] = {
	"b_setting_battery_01.png",
	"b_setting_battery_02.png",
	"b_setting_battery_03.png",
	"b_setting_battery_04.png",
	"b_setting_battery_05.png",
	"b_setting_battery_06.png",
	"b_setting_battery_07.png",
	"b_setting_battery_08.png",
	"b_setting_battery_09.png"
};

char *battery_charging_icons[] = {
	"b_setting_battery_charge_01.png",
	"b_setting_battery_charge_02.png",
	"b_setting_battery_charge_03.png",
	"b_setting_battery_charge_04.png",
	"b_setting_battery_charge_05.png",
	"b_setting_battery_charge_06.png",
	"b_setting_battery_charge_07.png",
	"b_setting_battery_charge_08.png",
	"b_setting_battery_charge_09.png"
};

static char *power_saving_str[] = {
	"IDS_ST_BODY_DISABLED_M_STATUS",
	"IDS_EMAIL_BODY_ENABLED_M_STATUS"
};

static appdata *temp_ad = NULL;
static Ecore_Timer *pws_timer = NULL;
static Evas_Object *g_pws_check = NULL;
static Elm_Object_Item *pws_it = NULL;

int _power_saving_terminate_app(void *data);

static Eina_Bool _delete_timer(void *data)
{
	DBG("_delete_timer");
	pws_timer = NULL;
	_power_saving_terminate_app(temp_ad);

	return ECORE_CALLBACK_CANCEL;
}

static void change_pws_key(keynode_t *key, void *data)
{
	int mode_val = 0;
	vconf_get_int(VCONFKEY_SETAPPL_PSMODE, &mode_val);

	if (mode_val) {
		DBG("change_pws_key is power saving");
		if (pws_timer) {
			ecore_timer_del(pws_timer);
			pws_timer = NULL;
		}
		pws_timer = ecore_timer_add(1.0, (Ecore_Task_Cb) _delete_timer, NULL);
	}

	else {
		DBG("change_pws_key is normal");
	}
}

void _initialize_battery()
{
	battery_timer = NULL;

	percent = 0;
	is_charging = false;
	prev_charging_state = false;
	is_alive = 0;

	register_vconf_changing(VCONFKEY_SETAPPL_PSMODE, change_pws_key, NULL);
}

Eina_Bool _clear_battery_list_cb(void *data, Elm_Object_Item *it)
{
	return EINA_TRUE;
}

void _clear_battery_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	if (battery_timer) {
		ecore_timer_del(battery_timer);
		battery_timer = NULL;
	}
	if (pws_timer) {
		ecore_timer_del(pws_timer);
		pws_timer = NULL;
	}
	percent = 0;
	is_charging = false;
	prev_charging_state = false;
	is_alive = 0;

	g_pws_check = NULL;
	pws_it = NULL;

	unregister_vconf_changing(VCONFKEY_SYSTEM_TIME_CHANGED, change_pws_key);

	return;
}

static int get_battery_img_index(int percent)
{
	int index = 0;

	if (percent > 0 && percent < 5) {
		index = 0;
	} else if (percent >= 5 && percent <= 10) {
		index = 1;
	} else if (percent >= 11 && percent <= 20) {
		index = 2;
	} else if (percent >= 21 && percent <= 34) {
		index = 3;
	} else if (percent >= 35 && percent <= 49) {
		index = 4;
	} else if (percent >= 50 && percent <= 64) {
		index = 5;
	} else if (percent >= 65 && percent <= 79) {
		index = 6;
	} else if (percent >= 80 && percent <= 94) {
		index = 7;
	} else if (percent >= 95 && percent <= 100) {
		index = 8;
	}
	return index;
}

static int is_updated_battery_state()
{
	int current_percent = 0;
	bool current_charging_state = false;

	if (device_battery_get_percent(&current_percent) != DEVICE_ERROR_NONE) {
		DBG("Setting - Battery percent get error");
	}

	if (device_battery_is_charging(&current_charging_state) != DEVICE_ERROR_NONE) {
		DBG("Setting - Battery charging do not get. error");
	}

	if ((current_percent != percent) || (current_charging_state != is_charging)) {
		percent = current_percent;
		is_charging = current_charging_state;

		DBG("Setting - Updated battery state !! percent: %d, charging: %d", percent, is_charging);

		return TRUE;
	}

	return FALSE;
}

static int is_type_of_charging()
{
	int status = 0;

	vconf_get_int(VCONFKEY_SYSMAN_CHARGER_STATUS, &status);

	if (status == VCONFKEY_SYSMAN_CHARGER_CONNECTED) {
		vconf_get_int(VCONFKEY_SYSMAN_USB_STATUS, &status);
		if (status == VCONFKEY_SYSMAN_USB_AVAILABLE) {
			return CHARGING_USB;
		}
	}
	return CHARGING_AC;
}

static Evas_Object *create_icon(Evas_Object *parent, char *img_path)
{
	if (img_path == NULL)
		return NULL;

	Evas_Object *icon = elm_image_add(parent);
	elm_image_file_set(icon, img_path, NULL);
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	return icon;
}

static Eina_Bool _battery_timeout(void *data)
{
	Evas_Object *layout = (Evas_Object *)data;
	if (layout == NULL) {
		return ECORE_CALLBACK_CANCEL;
	}

	char buf[512];
	char *ret_str = NULL;
	ret_str = _get_strnum_from_icu(percent);
	if (is_updated_battery_state()) {
		if (layout) {
			snprintf(buf, sizeof(buf) - 1, "%s%c", ret_str, '\%');
			char *temp_percent = strdup(buf);
			elm_object_part_text_set(layout, "text1", temp_percent);
			free(temp_percent);

			if (is_charging) {
				snprintf(buf, sizeof(buf) - 1, "%s%s", _("IDS_COM_BODY_CHARGING_GA"), (is_type_of_charging() == CHARGING_AC) ? "(AC)" : "(USB)");
				char *temp_charging_msg = strdup(buf);
				elm_object_part_text_set(layout, "text2", temp_charging_msg);
				free(temp_charging_msg);
			} else {
				elm_object_part_text_set(layout, "text2", _("IDS_ST_BODY_NOT_CHARGING_ABB"));
			}

			int img_index = get_battery_img_index(percent);
			if ((img_index != image_index) || (prev_charging_state != is_charging)) {
				snprintf(buf, sizeof(buf) - 1, "%s%s", IMG_DIR, (is_charging) ? battery_charging_icons[img_index] : battery_icons[img_index]);

				Evas_Object *del_icon = elm_object_part_content_get(layout, "image");
				if (del_icon) {
					evas_object_del(del_icon);
					del_icon = NULL;
				}
				elm_object_part_content_set(layout, "image", create_icon(layout, buf));
			}

			prev_charging_state = is_charging;

			DBG("Setting - Update Battery Layout!!");
		}
	}

	DBG("Setting - check battery state!!");
	free(ret_str);

	return ECORE_CALLBACK_RENEW;
}

void _start_timer_for_update(Evas_Object *obj)
{
	is_alive = 1;
	if (battery_timer) {
		ecore_timer_del(battery_timer);
		battery_timer = NULL;
	}
	battery_timer = ecore_timer_add(1.0, (Ecore_Task_Cb)_battery_timeout, obj);
}

void _battery_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _battery_lang_changed is called!");

	char buf[512];
	char *ret_str = NULL;
	ret_str = (char *)_get_strnum_from_icu(percent);
	if (obj) {
		snprintf(buf, sizeof(buf) - 1, "%s%c", ret_str, '\%');
		char *temp_percent = strdup(buf);
		elm_object_part_text_set(obj, "text1", temp_percent);
		free(temp_percent);

		if (is_charging) {
			snprintf(buf, sizeof(buf) - 1, "%s%s", _("IDS_COM_BODY_CHARGING_GA"), (is_type_of_charging() == CHARGING_AC) ? "(AC)" : "(USB)");
			char *temp_charging_msg = strdup(buf);
			elm_object_part_text_set(obj, "text2", temp_charging_msg);
			free(temp_charging_msg);
		} else {
			elm_object_part_text_set(obj, "text2", _("IDS_ST_BODY_NOT_CHARGING_ABB"));
		}
	}
	free(ret_str);
}

Evas_Object *_create_battery_content2(void *data)
{
	Evas_Object *layout;
	char buf[1024];
	appdata *ad = (appdata *)data;
	if (ad == NULL)
		return NULL;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting-test/battery");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(layout, "language,changed", _battery_lang_changed, NULL);

	if (device_battery_get_percent(&percent) != DEVICE_ERROR_NONE) {
		DBG("Setting - Battery percent get error");
	}
	char *ret_str = NULL;
	ret_str = _get_strnum_from_icu(percent);
	snprintf(buf, sizeof(buf) - 1, "%s%c", ret_str, '\%');

	elm_object_part_text_set(layout, "text1", buf);

	if (device_battery_is_charging(&is_charging) != DEVICE_ERROR_NONE) {
		DBG("Setting - Battery charging do not get. error");
	}
	char *charging_text = NULL;
	if (is_charging) {
		snprintf(buf, sizeof(buf) - 1, "%s%s", _("IDS_COM_BODY_CHARGING_GA"), (is_type_of_charging() == CHARGING_AC) ? "(AC)" : "(USB)");
		charging_text = strdup(buf);
	}
	elm_object_part_text_set(layout, "text2", (is_charging) ? charging_text : _("IDS_ST_BODY_NOT_CHARGING_ABB"));
	free(charging_text);

	prev_charging_state = is_charging;

	image_index = get_battery_img_index(percent);

	snprintf(buf, sizeof(buf) - 1, "%s%s", IMG_DIR, (is_charging) ? battery_charging_icons[image_index] : battery_icons[image_index]);

	elm_object_part_content_set(layout, "image", create_icon(layout, buf));

	_start_timer_for_update(layout);	/* updating timer start! */

	free(ret_str);
	return layout;
}

void _battery_status_cb_gen_item(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	_initialize_battery();

	return;
}

Evas_Object *_battery_status_cb(void *data)
{
	Evas_Object *layout = NULL;
	/* Elm_Object_Item *nf_it = NULL; */
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return NULL;
	}

	_initialize_battery();
	layout = _create_battery_content2(data);
	if (layout == NULL) {
		DBG("%s", "battery cb - layout is null");
		return NULL;;
	}

	return layout;
	/*nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL); */
	/*elm_naviframe_item_pop_cb_set(nf_it, _clear_battery_cb, ad); */
	/*elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE); */
}

int _get_battery_percent_value()
{
	int value = 0;
	if (vconf_get_bool(VCONFKEY_SETAPPL_BATTERY_PERCENTAGE_BOOL, &value) != 0) {
		ERR("error get vconf value!!");
	}
	return value;
}

int _set_battery_percent_value(int value)
{
	if (vconf_set_bool(VCONFKEY_SETAPPL_BATTERY_PERCENTAGE_BOOL, value) != 0) {
		ERR("error set vconf value!!");
		return FALSE;
	}
	return TRUE;
}

#if 0 // _NOT_USED_
void _battery_percent_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_set_battery_percent_value(!_get_battery_percent_value());

	elm_genlist_item_update((Elm_Object_Item *)event_info);
}
#endif

int _power_saving_runapp_info_get(const aul_app_info *ainfo, void *data)
{
	DBG("_power_saving_runapp_info_get");
	ail_appinfo_h handle;
	ail_error_e ret;

	bool valb;

	if (ainfo == NULL)
		return -1;
	if (ainfo->pid <= 0)
		return -1;
	if (ainfo->pkg_name == NULL)
		return 0;

	/* �� PID ���͸� */
	/*
	if(ainfo->pid == getpid()) {
		return 0;
	}
	*/
	ret = ail_get_appinfo(ainfo->pkg_name, &handle);

	if (ret != AIL_ERROR_OK) {
		return -1;
	}
	/* APP�� TASKMANAGE üũ */
	ret = ail_appinfo_get_bool(handle, AIL_PROP_X_SLP_TASKMANAGE_BOOL, &valb);
	if (valb == 0) {
		ret = ail_destroy_appinfo(handle);
		return 0;
	}
	/* ���� */
	aul_terminate_pid(ainfo->pid);

	ret = ail_destroy_appinfo(handle);
	return 0;
}
/*
static Eina_Bool _pws_timer(void *data)
{
	DBG("_pws_timer");
	appdata *ad = (appdata *) data;
	if( ad == NULL )
		return ECORE_CALLBACK_CANCEL;

	if(ad->win_main)
	{
		evas_object_del(ad->win_main);
		ad->win_main = NULL;
	}
	pws_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}
*/
int _power_saving_terminate_app(void *data)
{
	int ret = AUL_R_ERROR;
	appdata *ad = (appdata *) data;

	if (ad->win_main) {
		evas_object_del(ad->win_main);
		ad->win_main = NULL;
	}

	ret = aul_app_get_running_app_info(_power_saving_runapp_info_get, NULL);
	if (ret != AUL_R_OK) {
		DBG("aul_app_get_running_app_info() failed");
	}

	return 0;
}

int _get_power_saving_status()
{
	int value = 0;
	if (vconf_get_int(VCONFKEY_SETAPPL_PSMODE, &value) != 0) {
		ERR("error get vconf value!!");
	}
	if (value)
		battery_menu_list[BATT_MENU_POWER_SAVING].is_enable = ENABLE;
	else
		battery_menu_list[BATT_MENU_POWER_SAVING].is_enable = DISABLE;
	return value;
}

int _set_power_saving_status(int value)
{
	if (vconf_set_int(VCONFKEY_SETAPPL_PSMODE, value) != 0) {
		ERR("error set vconf value!!");
		return FALSE;
	}
	if (value) {
		battery_menu_list[BATT_MENU_POWER_SAVING].is_enable = ENABLE;
	} else
		battery_menu_list[BATT_MENU_POWER_SAVING].is_enable = DISABLE;
	return TRUE;
}

static void _power_saving_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	int pws_status;	/*int , 0:normal, 3:pws on */
	pws_status = _get_power_saving_status();

	if (pws_status)	{
		/*on -> off */
		DBG("psmode state [%d]", pws_status);
		_set_power_saving_status(SETTING_PSMODE_NORMAL);
	} else if (!pws_status) {
		/*off -> on */
		_pws_popup_cb(data, obj, event_info);
	}

	elm_genlist_item_update((Elm_Object_Item *)event_info);
}

char *_gl_battery_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;
	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(battery_menu_list[index].name));
	} else if (!strcmp(part, "elm.text.2") && index == BATT_MENU_POWER_SAVING) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(power_saving_str[battery_menu_list[index].is_enable]));
	}
	return strdup(buf);
}


Evas_Object *_gl_battery_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);
		if (index == BATT_MENU_POWER_SAVING) {
			elm_check_state_set(check, (_get_power_saving_status()) ? EINA_TRUE : EINA_FALSE);
			g_pws_check = check;
		} else {
			elm_check_state_set(check, (_get_battery_percent_value()) ? EINA_TRUE : EINA_FALSE);
		}
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	}

	return check;
}

void _gl_battery_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}

Evas_Object *_create_battery_list(void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return NULL;
	}

	Evas_Object *genlist = NULL;
	Evas_Object *layout = NULL;
	struct _battery_menu_item *menu_list = NULL;
	int idx = 0;

	temp_ad = ad;

	Elm_Genlist_Item_Class *itc = NULL;

	Elm_Genlist_Item_Class *itc_1text = elm_genlist_item_class_new();
	itc_1text->item_style = "1text";
	itc_1text->func.text_get = _gl_battery_title_get;
	itc_1text->func.del = _gl_battery_del;

	Elm_Genlist_Item_Class *itc_2text_1icon = elm_genlist_item_class_new();
	itc_2text_1icon->item_style = "2text.1icon.1";
	itc_2text_1icon->func.text_get = _gl_battery_title_get;
	itc_2text_1icon->func.content_get = _gl_battery_check_get;
	itc_2text_1icon->func.del = _gl_battery_del;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist,ad);

	menu_list = battery_menu_list;

	for (idx = 0; idx < sizeof(battery_menu_list) / sizeof(struct _battery_menu_item); idx++) {
		if (idx == BATT_MENU_POWER_SAVING) {
			itc = itc_2text_1icon;
		} else if (idx == BATT_MENU_STATUS) {
			itc = itc_1text;
		}

		Item_Data *id = calloc(sizeof(Item_Data), 1);
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

			if (idx == BATT_MENU_POWER_SAVING) {
				pws_it = id->item;
			}
		}
	}

	elm_genlist_item_class_free(itc_2text_1icon);
	/*elm_genlist_item_class_free(itc_1text_1icon); */
	elm_genlist_item_class_free(itc_1text);
	itc = NULL;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	return layout;
}

static void _pws_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = (appdata *) data;
	if (ad == NULL)
		return;

	if (ad->popup) {
		evas_object_del(data);
		ad->popup = NULL;
	}
}

static void _pws_ok_cb(void *data, Evas_Object *obj, void *event_info)
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

	if (pws_it) {
		battery_menu_list[BATT_MENU_POWER_SAVING].is_enable = ENABLE;
		elm_genlist_item_fields_update(pws_it, "elm.text.2", ELM_GENLIST_ITEM_FIELD_TEXT);
		elm_check_state_set(g_pws_check, EINA_TRUE);
	}
	_set_power_saving_status(SETTING_PSMODE_WEARABLE);
}

void _pws_popup_cb(void *data, Evas_Object *obj, void *event_info)
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
	elm_object_part_text_set(popup, "title,text", _("IDS_ST_MBODY_POWER_SAVER_ABB"));

	ad->popup = popup;

	scroller = elm_scroller_add(popup);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_style_set(scroller, "effect");
	elm_object_content_set(popup, scroller);
	evas_object_show(scroller);

	label = elm_label_add(scroller);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);

	char buf[1024];

	char *font_size_frame = "<font_size=34>%s</font_size>";
	snprintf(buf, sizeof(buf) - 1, font_size_frame, _("IDS_ST_POP_POWER_SAVER_WILL_BE_ENABLED_THIS_WILL_LIMIT_THE_MAXIMUM_PERFORMANCE_OF_THE_CPU_TURN_OFF_BLUETOOTH_AND_A_LOWER_SCREEN_POWER_LEVEL_WILL_BE_USED_MSG"));

	char *txt = strdup(buf);
	elm_object_text_set(label, txt);
	free(txt);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(scroller, label);
	evas_object_show(label);

	//ea_object_event_callback_add(popup, EA_CALLBACK_BACK, setting_popup_back_cb, ad);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, _("IDS_ST_BUTTON_CANCEL_ABB2"));
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", _pws_cancel_cb, popup);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, _("IDS_WNOTI_BUTTON_OK_ABB2"));
	elm_object_part_content_set(popup, "button2", btn);
	evas_object_smart_callback_add(btn, "clicked", _pws_ok_cb, popup);

	evas_object_show(popup);
}


