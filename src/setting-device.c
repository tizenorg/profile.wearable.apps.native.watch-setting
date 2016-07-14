/*
 *	Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 *	Licensed under the Flora License, Version 1.0 (the License);
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://floralicense.org/license/
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an AS IS BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */
#include <feedback.h>
#include <sys/types.h>
#include <dirent.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "setting-device.h"
#include "util.h"
#include "setting_control_haptic.h"
#include "setting_view_toast.h"
#include "setting_data_vconf.h"
#include "setting-double.h"

#ifndef VCONFKEY_SETAPPL_AUTO_OPEN_APPS
#define VCONFKEY_SETAPPL_AUTO_OPEN_APPS				"db/setting/auto_open_apps"
#endif

#define AUDIO_RESOURCE_EXTENSION	".ogg"

static struct _device_action_menu_item device_action_menu_its[] = {
	{ "Double press Home key",					0,		_double_press_home_key_cb},
	{ "Auto open apps",					0,		_auto_open_apps_cb},
};

static int DEV_TOP_MENU_SIZE =
	sizeof(device_action_menu_its) / sizeof(device_action_menu_its[0]);

Evas_Object *g_device_action_genlist = NULL;

static appdata *temp_ad						= NULL;

static int device_action_type	 = 0;			/* device_action type */

static Ecore_Timer *device_action_timer = NULL;

static char *
_gl_menu_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	snprintf(buf, 1023, "%s", "Device");
	return strdup(buf);
}

static void _set_auto_open_apps_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = temp_ad;
	Evas_Object *check = (Evas_Object *)data;
	bool is_auto_open = 0;
	vconf_set_bool(VCONFKEY_SETAPPL_AUTO_OPEN_APPS, is_auto_open);

	elm_check_state_set(check,	EINA_FALSE);

	if (ad && ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	back_button_cb_pop();
}

static void _set_auto_open_apps_ok_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = temp_ad;
	Evas_Object *check = (Evas_Object *)data;
	bool is_auto_open = 1;
	vconf_set_bool(VCONFKEY_SETAPPL_AUTO_OPEN_APPS, is_auto_open);

	elm_check_state_set(check,	EINA_TRUE);

	if (ad && ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	back_button_cb_pop();
}

void back_key_auto_open_app_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	bool is_auto_open = 0;
	Evas_Object *check = (Evas_Object *)data;
	vconf_set_bool(VCONFKEY_SETAPPL_AUTO_OPEN_APPS, is_auto_open);
	elm_check_state_set(check, EINA_FALSE);

	appdata *ad = (appdata *)temp_ad;
	if (ad && ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	back_button_cb_pop();
}

static void _auto_open_apps_check_changed_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	appdata *ad = temp_ad;
	Evas_Object *check = (Evas_Object *)data;

	if (ad == NULL) {
		DBG("%s", "_auto_open_apps_check_cb - appdata or check is null");
		return;
	}

	DBG("_auto_open_apps_check_changed_cb is called!!!!!!!");

	int is_auto_open = 0;
	vconf_get_bool(VCONFKEY_SETAPPL_AUTO_OPEN_APPS, &is_auto_open);


	if (!is_auto_open) {
		Evas_Object *popup = NULL;
		Evas_Object *btn1 = NULL;
		Evas_Object *btn2 = NULL;
		Evas_Object *icon;

		popup = elm_popup_add(ad->nf);
		elm_object_style_set(popup, "circle");
		evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(ad->nf, popup);

		ad->popup = popup;

		char buf[1024];

		char *font_setting = "<text_class=tizen><align=center>%s</align></text_class>";
		snprintf(buf, sizeof(buf) - 1, font_setting, "Focusing on icon in Apps screen will open app automatically");

		Evas_Object *layout;
		layout = elm_layout_add(popup);
		elm_layout_theme_set(layout, "layout", "popup", "content/circle/buttons2");

		char *txt = strdup(buf);
		elm_object_text_set(layout, txt);
		elm_object_content_set(popup, layout);

		FREE(txt);

		btn1 = elm_button_add(popup);
		elm_object_style_set(btn1, "popup/circle/left");
		evas_object_size_hint_weight_set(btn1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(popup, "button1", btn1);
		evas_object_smart_callback_add(btn1, "clicked", _set_auto_open_apps_cancel_cb, check);

		icon = elm_image_add(btn1);
		elm_image_file_set(icon, IMG_DIR"tw_ic_popup_btn_delete.png", NULL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(btn1, "elm.swallow.content", icon);
		evas_object_show(icon);

		btn2 = elm_button_add(popup);
		elm_object_style_set(btn2, "popup/circle/right");
		evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(popup, "button2", btn2);
		evas_object_smart_callback_add(btn2, "clicked", _set_auto_open_apps_ok_clicked_cb, check);

		icon = elm_image_add(btn2);
		elm_image_file_set(icon, IMG_DIR"tw_ic_popup_btn_check.png", NULL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(btn2, "elm.swallow.content", icon);
		evas_object_show(icon);

		evas_object_show(popup);
		back_button_cb_push(&back_key_auto_open_app_popup_cb, check, NULL, g_device_action_genlist, "device_action_genlist");
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _hw_back_key_cb, NULL);

	} else {
		bool is_auto_open = 0;
		vconf_set_bool(VCONFKEY_SETAPPL_AUTO_OPEN_APPS, is_auto_open);

	}

}


void _clear_device_action_resource()
{
	if (device_action_timer) {
		ecore_timer_del(device_action_timer);
		device_action_timer = NULL;
	}

	_haptic_close();

	temp_ad = NULL;

	device_action_type = 0;

	/*unregister_vconf_changing(VCONFKEY_SETAPPL_device_action_STATUS_BOOL, vibrate_vconf_changed_cb); */
}

Evas_Object *_gl_device_action_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	device_action_Item_Data *id = data;
	int is_auto_open_apps = 0;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);

		if (vconf_get_bool(VCONFKEY_SETAPPL_AUTO_OPEN_APPS, &is_auto_open_apps) < 0) {
			is_auto_open_apps = 0;
		}

		elm_check_state_set(check, (is_auto_open_apps) ? EINA_TRUE : EINA_FALSE);	/*default */
		evas_object_event_callback_add(check, EVAS_CALLBACK_MOUSE_DOWN, _auto_open_apps_check_changed_cb, (void *)check);
		elm_object_style_set(check, "on&off");
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_TRUE);

		id->check = check;

	}

	return check;
}

void _double_press_home_key_cb(void *data, Evas_Object *obj, void *event_info)
{

	Evas_Object *layout = NULL;
	Elm_Object_Item *nf_it = NULL;

	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	init_double_pressing(ad);

	layout = create_double_app_list(data);
	if (layout == NULL) {
		DBG("%s", "double cb - genlist is null");
		return;
	}

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, "empty");
	back_button_cb_push(&back_key_generic_cb, data, NULL, g_device_action_genlist, "device_action_genlist");
	elm_naviframe_item_pop_cb_set(nf_it, clear_double_app_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	ad->MENU_TYPE = SETTING_DOUBLE_PRESSING;
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
}

void _auto_open_apps_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

}

char *_gl_device_action_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;

	char *appid = NULL;

	appid = _get_selected_app_name();

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(device_action_menu_its[index % DEV_TOP_MENU_SIZE].name));
	} else if (!strcmp(part, "elm.text.1")) {
		switch (index) {
		case 0:
			snprintf(buf, sizeof(buf) - 1, "%s", appid);
			break;
		}
		index++;
	}
	return strdup(buf);
}


static void _device_action_gl_del(void *data, Evas_Object *obj)
{
	device_action_Item_Data *id = data;
	FREE(id);
}


Evas_Object *_create_device_action_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_sound_list - appdata is null");
		return NULL;
	}

	temp_ad = ad;

	Evas_Object *genlist  = NULL;
	struct _device_action_menu_item *menu_its = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc_tmp;


	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text";
	itc->func.text_get = _gl_device_action_title_get;
	itc->func.del = _device_action_gl_del;

	Elm_Genlist_Item_Class *itc_auto_open_apps = elm_genlist_item_class_new();
	itc_auto_open_apps->item_style = "1text.1icon.1";
	itc_auto_open_apps->func.text_get = _gl_device_action_title_get;
	itc_auto_open_apps->func.content_get = _gl_device_action_check_get;
	itc_auto_open_apps->func.del = _device_action_gl_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	Evas_Object *circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);
	eext_circle_object_genlist_scroller_policy_set(circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(circle_genlist, EINA_TRUE);

	Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
	title_item->func.text_get = _gl_menu_title_text_get;
	title_item->item_style = "title";
	title_item->func.del = _device_action_gl_del;

	elm_genlist_item_append(genlist, title_item, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_item);

	menu_its = device_action_menu_its;

	for (idx = 0; idx < DEV_TOP_MENU_SIZE; idx++) {
		if (idx == 0)
			itc_tmp = itc;
		else
			itc_tmp = itc_auto_open_apps;

		device_action_Item_Data *id = calloc(sizeof(device_action_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
						   genlist,			/* genlist object */
						   itc_tmp,			/* item class */
						   id,					/* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_its[idx].func,	/* call back */
						   ad);

			if (idx == 0) {
				if (ad)
					ad->double_press_item = id->item;
			}
		}
	}
	elm_genlist_item_class_free(itc_auto_open_apps);
	elm_genlist_item_class_free(itc);

	Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
	padding->item_style = "padding";
	padding->func.del = _device_action_gl_del;

	g_device_action_genlist = genlist;

	elm_genlist_item_append(genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(padding);
	/*register_vconf_changing(VCONFKEY_SETAPPL_device_action_STATUS_BOOL, vibrate_vconf_changed_cb, NULL); */

	return genlist;
}


