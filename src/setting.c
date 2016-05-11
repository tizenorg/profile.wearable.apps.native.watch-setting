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

#include <dlog.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <fcntl.h>
#include <appcore-efl.h>
#include <system_settings.h>
#include <app_control.h>

#include "setting-sound.h"
#include "setting-volume.h"
#include "setting-clock.h"
#include "setting-homescreen.h"
#include "setting-info.h"
#include "setting-bluetooth.h"
#include "setting-motion.h"
#include "setting-display.h"
#include "setting-language.h"
#include "setting-reset.h"
#include "setting-battery.h"
#include "setting-device.h"
#include "setting-privacy.h"
#include "setting-double.h"
#include "setting-connection.h"
#include "setting-safety.h"
#include "setting-notification.h"
#include "setting-profile.h"
#include "util.h"
#include "setting-common-sound.h"
#include "setting.h"
#include "setting_control_bt.h"
#include "setting_view_toast.h"
#include "setting_indicator_util.h"

/*This function will be deprecated..*/
int app_control_set_package(app_control_h app_control, const char *package);
void device_cb(void *data, Evas_Object *obj, void *event_info);
void connection_cb(void *data, Evas_Object *obj, void *event_info);

#define LANGUAGE_ICON_DISABLED		"b_settings_language_disabled.png"
#define LANGUAGE_ICON_ENABLED		"b_settings_language.png"

static struct _menu_item setting_emergency_menu_its[] = {
#ifdef FEATURE_SETTING_EMUL
	{ "IDS_ST_BUTTON_CLOCK", 							"b_settings_change_clock.png", 	clock_cb 	  		},
#endif
	{ "IDS_ST_OPT_SOUND_ABB2", 							"b_settings_volume.png",			sound_cb 	  		},
	{ "IDS_ST_MBODY_DISPLAY_ABB",						"b_setting_display.png",		display_cb 	  		},
#ifndef FEATURE_SETTING_EMUL
	{ "IDS_QP_BUTTON_BLUETOOTH",  						"b_settings_bluetooth.png",		bluetooth_cb  		},
#endif
#if !defined(FEATURE_SETTING_SDK) && !defined(FEATURE_SETTING_EMUL)
	{ "IDS_ST_MBODY_SAFETY_ABB",                    "b_settings_safety.png",       safety_cb           },
#endif
	{ NULL, NULL, NULL }
};

static struct _menu_item setting_menu_its[] = {
//	{ "IDS_ST_BUTTON_CLOCK", 							"b_settings_change_clock.png", 	clock_cb 	  		},
#if !defined(FEATURE_SETTING_SDK) && !defined(FEATURE_SETTING_EMUL)
//	{ "IDS_ST_BODY_WALLPAPERS", 						"b_setting_wallpaper.png", 			homescreen_cb 	  	},
//	{ "IDS_ST_BUTTON_NOTIFICATIONS", 					"b_settings_notifications.png", notification_cb		},
#endif
//	{ "IDS_ST_OPT_SOUND_ABB2", 							"b_settings_volume.png",			sound_cb 	  		},
//	{ "IDS_ST_MBODY_TEXT_INPUT_ABB",					"text_input_icon.png",			keyboard_cb 	  		},
#ifndef FEATURE_SETTING_EMUL
#endif
#ifndef FEATURE_SETTING_EMUL
//	{ "IDS_QP_BUTTON_BLUETOOTH",  						"b_settings_bluetooth.png",		bluetooth_cb  		},
#endif
#if !defined(FEATURE_SETTING_SDK) && !defined(FEATURE_SETTING_EMUL)
//	{ "IDS_ST_MBODY_DOUBLE_PRESS_ABB",					"b_setting_double-press.png",		double_pressing_cb 	},
#endif
	{ "IDS_ST_MBODY_DISPLAY_ABB",						"b_setting_display.png",		display_cb			},
	{ "IDS_ST_OPT_SOUND_ABB2",							"b_settings_volume.png",			sound_cb			},
	{ "Device",							"b_settings_volume.png",			device_cb			},
	{ "Connection",					"b_settings_volume.png",		connection_cb	},
	{ "IDS_ST_MBODY_TEXT_INPUT_ABB",					"text_input_icon.png",			keyboard_cb			},
	{ "IDS_ST_BUTTON_GEAR_INFO",						"b_settings_info.png",			gear_info_cb		},
	{ NULL, NULL, NULL }
};

static int is_emergency;
static bool running = false;
static Ecore_Timer *running_timer = NULL;
static Ecore_Timer *scrl_timer = NULL;

static Evas_Object *_create_bg(Evas_Object *parent);
static Evas_Object *_create_layout_main(Evas_Object *parent);
static Evas_Object *_create_naviframe_layout(Evas_Object *parent);
static void _create_view_layout(appdata *ad);
static int init_watch_setting(appdata *ad);
static Eina_Bool _app_ctrl_timer_cb(void *data);
static Eina_Bool _scroller_timer_cb(void *data);
static void _update_main_menu_title(void *data);

void clock_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	if (is_running_clock) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	initialize_clock(data);

	/*genlist = _create_clock_list(data); */
	layout = _clock_type_cb(data, obj, event_info);
	if (layout == NULL) {
		DBG("%s", "clock cb - layout is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _clear_clock_cb, ad);
	/*elm_naviframe_item_pop_cb_set(nf_it, _clear_clock_cb, ad); */
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
	is_running_clock = 1;

	ad->MENU_TYPE = SETTING_CLOCK;
}

void notification_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	_initialize_noti();

	genlist = _create_noti_list(data);
	if (genlist == NULL) {
		DBG("%s", "notification_cb - genlist is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _clear_noti_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ad->MENU_TYPE = SETTING_NOTIFICATION;
}

void homescreen_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	/*layout = _create_homescreen_list(data); */
	layout = create_wallpaper_list(data);
	if (layout == NULL) {
		DBG("%s", "homescreen cb - layout is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	/*elm_naviframe_item_pop_cb_set(nf_it, _clear_homescreen_cb, ad); */
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _clear_homescreen_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ad->MENU_TYPE = SETTING_HOMESCREEN;
}


void sound_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	_initialize();

	genlist = _create_sound_list(data);
	if (genlist == NULL) {
		DBG("%s", "sound cb - genlist is null");
		return;
	}

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	back_button_cb_push(&back_key_generic_cb, data, NULL, ad->main_genlist, nf_it);
	evas_object_event_callback_add(nf_it, EVAS_CALLBACK_DEL, _clear_sound_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ad->MENU_TYPE = SETTING_SOUND;
}

void device_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}


	genlist = _create_device_action_list(data);
	if (genlist == NULL) {
		DBG("%s", "sound cb - genlist is null");
		return;
	}

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	back_button_cb_push(&back_key_generic_cb, data, NULL, ad->main_genlist, nf_it);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ad->MENU_TYPE = SETTING_SOUND;
}

void volume_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	_initialize_volume();

	genlist = _create_volume_list(data);
	if (genlist == NULL) {
		DBG("%s", "volume cb - genlist is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	/*elm_naviframe_item_pop_cb_set(nf_it, _clear_volume_cb, ad); */
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ad->MENU_TYPE = SETTING_VOLUME;
}

void display_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	_init_display();

	genlist = _create_display_list(data);
	if (genlist == NULL) {
		DBG("%s", "display cb - genlist is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	back_button_cb_push(&back_key_generic_cb, data, NULL, ad->main_genlist, nf_it);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_DEL, _clear_display_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ad->MENU_TYPE = SETTING_DISPLAY;
}

void keyboard_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		return;
	}

	if (!running) {
		app_control_h service;
		app_control_create(&service);
		app_control_add_extra_data(service, "caller", "settings");
		app_control_set_package(service, "org.tizen.inputmethod-setting-list");
		app_control_send_launch_request(service, NULL, NULL);
		app_control_destroy(service);

		running = true;

		if (running_timer) {
			ecore_timer_del(running_timer);
			running_timer = NULL;
		}
		running_timer = ecore_timer_add(0.5, (Ecore_Task_Cb)_app_ctrl_timer_cb, NULL);
	}
}

void battery_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	Evas_Object *layout = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	_initialize_battery();
	layout = _battery_status_cb(data);
	if (layout == NULL) {
		DBG("%s", "battery cb - layout is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	/*elm_naviframe_item_pop_cb_set(nf_it, _clear_battery_list_cb, ad); */
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _clear_battery_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	ad->MENU_TYPE = SETTING_BATTERY;
}

void bluetooth_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	DBG("bluetooth_cb in");
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	app_control_h service;
	app_control_create(&service);
	app_control_set_package(service, "org.tizen.bluetooth");
	app_control_add_extra_data(service, "launch-type", "setting");
	app_control_send_launch_request(service, NULL, NULL);
	app_control_destroy(service);

	running = true;

	if (running_timer) {
		ecore_timer_del(running_timer);
		running_timer = NULL;
	}
	running_timer = ecore_timer_add(0.5, (Ecore_Task_Cb)_app_ctrl_timer_cb, NULL);
}

#if 0
void motion_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	_initialize_motion();
	genlist = _create_motion_list(data);
	if (genlist == NULL) {
		DBG("%s", "motion cb - genlist is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _clear_motion_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
}
#endif
void lockscreen_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = NULL;
	Elm_Object_Item *nf_it = NULL;

	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	layout = create_privacy_list(data);
	if (layout == NULL) {
		DBG("%s", "lockscreen cb - genlist is null");
		return;
	}

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, clear_privacy_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ad->MENU_TYPE = SETTING_SCREEN_LOCK;
}

void connection_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = NULL;
	Elm_Object_Item *nf_it = NULL;

	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	layout = _create_connection_list(data);
	if (layout == NULL) {
		DBG("%s", "connection_cb - genlist is null");
		return;
	}

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _clear_connection_resource, ad);
	//elm_naviframe_item_pop_cb_set(nf_it, _clear_lang_navi_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ad->MENU_TYPE = SETTING_DOUBLE_PRESSING;
}

void double_pressing_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = NULL;
	Elm_Object_Item *nf_it = NULL;

	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	init_double_pressing(ad);

	layout = create_double_app_list(data);
	if (layout == NULL) {
		DBG("%s", "double cb - genlist is null");
		return;
	}

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, clear_double_app_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ad->MENU_TYPE = SETTING_DOUBLE_PRESSING;
}

void language_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	if (is_connected_GM()) {
		DBG("Setting - language can not change!!");

		/* automatic freed!! */
		struct _toast_data *toast = _create_toast(ad, _("IDS_ST_TPOP_CHANGE_LANGUAGE_ON_MOBILE_DEVICE"));
		if (toast) {
			_show_toast(ad, toast);
		}
		return;
	}

	_initialize_language(ad);
	_set_launguage_update_cb(_update_main_menu_title);

	genlist = _create_lang_list(data);
	if (genlist == NULL) {
		DBG("%s", "language cb - genlist is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	back_button_cb_push(&back_key_generic_cb, data, NULL, ad->main_genlist, nf_it);
	elm_naviframe_item_pop_cb_set(nf_it, _clear_lang_navi_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	ad->MENU_TYPE = SETTING_LANGUAGE;
}

void safety_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	appdata *ad = data;
	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	Evas_Object *layout = NULL;
	Elm_Object_Item *nf_it = NULL;

	layout = create_safety_list(ad);
	if (layout == NULL) {
		DBG("%s", "safety cb - genlist is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	back_button_cb_push(&back_key_generic_cb, data, NULL, ad->main_genlist, nf_it);
	elm_naviframe_item_pop_cb_set(nf_it, clear_safety_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	ad->MENU_TYPE = SETTING_SAFETY;
}

void reset_gear_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	_reset_popup_cb(data, obj, event_info);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ad->MENU_TYPE = SETTING_RESET;
}

void gear_info_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	genlist = _create_info_list(data);
	if (genlist == NULL) {
		DBG("%s", "info cb - genlist is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _clear_info_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	ad->MENU_TYPE = SETTING_INFO;
}

static Eina_Bool _app_ctrl_timer_cb(void *data)
{
	DBG("reset flag");
	running = false;
	running_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _scroller_timer_cb(void *data)
{
	DBG("hide scroller bar");

	Evas_Object *genlist = data;

	if (genlist)
		elm_layout_signal_emit(genlist, "do-hide-vbar", "");

	return ECORE_CALLBACK_CANCEL;
}

void profile_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("profile cb");
	appdata *ad = data;
	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (running) {
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	if (!running) {
		_create_profile(data, obj, event_info);
		running = true;

		if (running_timer) {
			ecore_timer_del(running_timer);
			running_timer = NULL;
		}
		running_timer = ecore_timer_add(0.5, (Ecore_Task_Cb)_app_ctrl_timer_cb, NULL);
	}
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
}

static void _quit_cb(void *data, Evas_Object *obj, void *ei)
{
	/*
	//To make your application go to background,
	//Call the elm_win_lower() instead
	Evas_Object *win = (Evas_Object *) data;
	elm_win_lower(win);
	*/
	elm_exit();
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	/* Stop timer and task */
	appdata *ad = data;
	if (running_timer) {
		ecore_timer_del(running_timer);
		running_timer = NULL;
	}
	if (running)
		running = false;

	if (scrl_timer) {
		ecore_timer_del(scrl_timer);
		scrl_timer = NULL;
	}

	if (ad && ad->win_main) {
		elm_win_lower(ad->win_main);
	}

	return EINA_FALSE;
}

static Evas_Object *_create_bg(Evas_Object *parent)
{
	Evas_Object *bg;

	if (parent == NULL) return NULL;

	bg = elm_bg_add(parent);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, bg);
	evas_object_show(bg);

	return bg;
}

static Evas_Object *_create_conform(Evas_Object *parent)
{
	Evas_Object *conform;

	if (parent == NULL) return NULL;

	conform = elm_conformant_add(parent);
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, conform);
	evas_object_show(conform);

	return conform;
}

static Evas_Object *_create_layout_main(Evas_Object *parent)
{
	Evas_Object *layout;

	if (parent == NULL) return NULL;

	layout = elm_layout_add(parent);

	if (layout == NULL) {
		DBG("Failed elm_layout_add.\n");
		return NULL;
	}

	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, layout);

	evas_object_show(layout);

	return layout;
}

static void _lang_changed(app_event_info_h event_info, void *data)
{
	appdata *ad = data;
	if (ad && ad->MENU_TYPE == SETTING_LANGUAGE)
		return;

	char *locale = vconf_get_str(VCONFKEY_LANGSET);
	if (locale) {
		DBG("Setting - language is changed : %s", locale);
		elm_language_set(locale);
		elm_config_all_flush();
	}
}

#if 0 /* _NOT_USED_ */
static void _window_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	appdata *ad = (appdata *)data;
	evas_object_geometry_get(ad->win_main, NULL, NULL, &ad->root_w, &ad->root_h);
}
#endif

static void init_values(appdata *ad)
{
	if (ad == NULL) {
		return;
	}

	ad->is_motion_smart_relay_on = 1;
	ad->is_motion_wake_up_on = 1;
}

static int init_watch_setting(appdata *ad)
{
	DBG("init_watch_setting() is called!");

	if (ad == NULL)
		return EINA_FALSE;

	ad->bg = _create_bg(ad->win_main);

	/* Conformant *** disable comform : lauching time issue *** */
	/* Base Layout */
	ad->layout_main = _create_layout_main(ad->win_main);
	if (ad->layout_main == NULL)
		return -1;

	/* Indicator */
	elm_win_indicator_mode_set(ad->win_main, ELM_WIN_INDICATOR_HIDE);

	/* Naviframe */
	ad->nf = _create_naviframe_layout(ad->layout_main);

	/* Naviframe Content */
	_create_view_layout(ad);

	init_values(ad);

	int vibration_level = 0;
	vconf_get_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, &vibration_level);

	DBG("Setting - vibration level : %d", vibration_level);

	if (vibration_level == 2) {	/* wrong value!! */
		vconf_set_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, VIBRATION_LEVEL_HIGH_INT);
		vconf_set_int(VCONFKEY_SETAPPL_TOUCH_FEEDBACK_VIBRATION_LEVEL_INT, VIBRATION_LEVEL_HIGH_INT);
	}
	return EINA_TRUE;
}

static Evas_Object *_create_naviframe_layout(Evas_Object *parent)
{
	Evas_Object *nf = NULL;

	if (parent == NULL) return NULL;

	nf = elm_naviframe_add(parent);
	elm_object_part_content_set(parent, "elm.swallow.content", nf);

	evas_object_show(nf);

	return nf;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[512];
	Item_Data *id = data;
	int index = id->index;
	/*
	char expression[32];

	if( index == LANGUAGE_MENU_INDEX ) {
		if( is_connected_GM() ) {
			strcpy(expression, "<font color=#515151>%s</font>");
		} else {
			strcpy(expression, "%s");
		}
	} else {
		strcpy(expression, "%s");
	}*/

	if (is_emergency)
		snprintf(buf, sizeof(buf) - 1, "%s", _(setting_emergency_menu_its[index].name));
	else
		snprintf(buf, sizeof(buf) - 1, "%s", _(setting_menu_its[index].name));

	return strdup(buf);
}

static Evas_Object *_gl_icon_get(void *data, Evas_Object *obj, const char *part)
{
#if 0
	char buf[1024];
	Item_Data *id = data;
	int index = id->index;

	if (part && !strcmp(part, "elm.icon")) {
		if (is_emergency)
			snprintf(buf, sizeof(buf) - 1, "%s", setting_emergency_menu_its[index].icon_name);
		else
			snprintf(buf, sizeof(buf) - 1, "%s", setting_menu_its[index].icon_name);

		Evas_Object *icon = elm_image_add(obj);
		elm_image_file_set(icon, EDJE_PATH, buf);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_min_set(icon, 52, 52);

		return icon;
	}
#endif
	return NULL;
}

static void _gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}

static Evas_Object *_gl_indicator_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *layout = NULL;

	layout = elm_layout_add(obj);
	elm_layout_file_set(layout, EDJE_PATH, "setting/indicator");

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	indicator_view_update(layout);
	evas_object_show(layout);
	DBG("indicator get!!!!!!!!");

	return layout;
}

static Evas_Object *_create_mainlist_winset(Evas_Object *parent, appdata *ad)
{
	Evas_Object *genlist = NULL;
	struct _menu_item *menu_its = NULL;
	int idx = 0;

	/* Create item class */
	elm_theme_extension_add(NULL, EDJE_PATH);
	Elm_Genlist_Item_Class *itc_tmp = elm_genlist_item_class_new();
	itc_tmp->item_style = "setting_indicator";
	itc_tmp->func.content_get = _gl_indicator_get;
	itc_tmp->func.del = _gl_del;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon";
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_icon_get;
	itc->func.del = _gl_del;

	genlist = elm_genlist_add(ad->nf);
	if (genlist == NULL) {
		elm_genlist_item_class_free(itc);
		elm_genlist_item_class_free(itc_tmp);
		return NULL;
	}

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
#if 0
	Item_Data *id_indi = calloc(sizeof(Item_Data), 1);
	if (id_indi) {
		id_indi->index = idx;
		id_indi->item = elm_genlist_item_append(
							genlist,
							itc_tmp,
							id_indi,
							NULL,
							ELM_GENLIST_ITEM_NONE,
							NULL, NULL);
		elm_genlist_item_select_mode_set(id_indi->item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

#endif

#if 0
	vconf_get_bool(VCONFKEY_SETAPPL_EMERGENCY_STATUS_BOOL, &is_emergency);
#else
	is_emergency = 0;
#endif
	int item_count = 0;
	if (is_emergency) {
		menu_its = setting_emergency_menu_its;
		item_count = sizeof(setting_emergency_menu_its) / sizeof(struct _menu_item);
	} else {
		menu_its = setting_menu_its;
		item_count = sizeof(setting_menu_its) / sizeof(struct _menu_item);
	}

	DBG("Setting - Main menu item count: %d", item_count);

	/*while (menu_its[ idx ].name != NULL) { */
	for (idx = 0; idx < item_count - 1; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
						   genlist,			/* genlist object */
						   itc,				/* item class */
						   id,	            	/* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_its[ idx ].func,
						   ad);
		}
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc_tmp);

	ad->main_genlist = genlist;

	indicator_set_vconf_changed_cb(ad);

	return genlist;
}

static void _update_main_menu_title(void *data)
{
	DBG("_update_main_menu_title() is called");

	appdata *ad = data;
	if (ad == NULL) {
		DBG("appdata is NULL");
		return;
	}

	if (ad->main_genlist) {
		elm_genlist_realized_items_update(ad->main_genlist);
	}
}

#if 0 /* _NOT_USED_ */
static void _naviframe_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL)
		return;

	if (ad->MENU_TYPE == SETTING_BLUETOOTH) {
		clear_bt_resource();
		ad->MENU_TYPE = SETTING_MAIN;
	} else if (ad->MENU_TYPE == SETTING_VOLUME_2_DEPTH) {
		_clear_volume_resources();

		ad->MENU_TYPE = SETTING_VOLUME;
	} else if (ad->MENU_TYPE == SETTING_SOUND_RINGTONE) {
		_stop_player();

		ad->MENU_TYPE = SETTING_SOUND;
	} else if (ad->MENU_TYPE == SETTING_SOUND_NOTIFICATION) {
		_stop_wav_player();

		ad->MENU_TYPE = SETTING_SOUND;
	}

	elm_naviframe_item_pop(obj);
}
#endif

static void _create_view_layout(appdata *ad)
{
	Evas_Object *genlist = NULL;
	Evas_Object *btn  = NULL;
	Elm_Object_Item *nf_it = NULL;

	if (ad == NULL) return;
	if (ad->nf == NULL) return;

	genlist = _create_mainlist_winset(ad->win_main, ad);
	/*ea_object_event_callback_add(ad->nf, EA_CALLBACK_BACK, _naviframe_back_cb, ad); */
	/*ea_object_event_callback_add(ad->nf, EA_CALLBACK_MORE, ea_naviframe_more_cb, NULL); */

	connect_to_wheel_with_genlist(genlist,ad);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, btn, NULL, genlist, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_naviframe_item_pop_cb_set(nf_it, _pop_cb, ad); /* ad->win_main */

	/*register_vconf_changing(VCONFKEY_WMS_WMANAGER_CONNECTED, change_language_enabling, NULL); */
}

static Evas_Object *create_win(const char *name)
{
	Evas_Object *eo;

	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (!eo)
		return NULL;

	elm_win_title_set(eo, name);
	elm_win_autodel_set(eo, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(eo)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(eo, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(eo, "delete,request", _quit_cb, NULL);

	return eo;
}
int _time_cb(system_settings_key_e key, system_settings_changed_cb callback, void *user_data)
{
	DBG("_time_cb");
	return 0;
}

static void _exit_app(void *data, Evas_Object *obj, void *event_info)
{
	back_button_cb_pop();
	ui_app_exit();
}


static Eina_Bool _hw_back_key_cb(void *data, int type, void *event)
{
	back_button_cb_call();
	return ECORE_CALLBACK_RENEW;
}


bool app_create(void *data)
{
	/*DBG("[TIME] 3. it taked %d msec from main to setting_main_app_create ", appcore_measure_time()); */
	/*appcore_measure_start(); */

	DBG("Setting - app_create");

	appdata *ad = (appdata *) data;
	char *locale;

	DBG("app_create start.");

	ad->win_main = create_win(PACKAGE);
	if (ad->win_main == NULL)
		return false;

	elm_win_screen_size_get(ad->win_main, NULL, NULL, &ad->root_w, &ad->root_h);

	ad->conform = _create_conform(ad->win_main);

	evas_object_resize(ad->win_main, ad->root_w, ad->root_h);

	ad->circle_surface = eext_circle_surface_conformant_add(ad->conform);

	double scale = elm_config_scale_get();
	if (scale < 1.0) {
		DBG("Setting - scale is more less than 1.0");
		elm_config_scale_set(1.0);
	}

	ad->evas = evas_object_evas_get(ad->win_main);

	locale = vconf_get_str(VCONFKEY_LANGSET);
	if (locale) {
		elm_language_set(locale);
	}

	ad->is_first_launch = 1;

	init_watch_setting(ad);

	DBG("app_create finish.");

	DBG("[TIME] 4. setting_main_app_create taked %d msec ", appcore_measure_time());
	/*appcore_measure_start(); */

	evas_object_show(ad->win_main);

	/*int ret = system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE, _time_cb, NULL); */
	/*DBG("ret = %d", ret); */
	DBG("app_create finish. with skip locale");

	back_button_cb_push(&_exit_app, NULL, NULL, NULL, NULL);
	ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _hw_back_key_cb, NULL);

	return true;
}

void app_terminate(void *data)
{
	DBG("Setting - app_terminate");

	appdata *ad = data;

	indicator_unset_vconf_changed_cb();

	if (ad->alert_rdg) {
		ad->alert_rdg = NULL;
	}
	if (ad->datetime) {
		ad->datetime = NULL;
	}
	if (ad->dt_genlist_item_of_date) {
		ad->dt_genlist_item_of_date = NULL;
	}
	if (ad->dt_genlist_item_of_time) {
		ad->dt_genlist_item_of_time = NULL;
	}
	if (ad->pref_arm_rdg) {
		ad->pref_arm_rdg = NULL;
	}
	if (ad->main_genlist) {
		ad->main_genlist = NULL;
	}
	if (ad->double_rdg) {
		ad->double_rdg = NULL;
	}
	if (ad->font_size_rdg) {
		ad->font_size_rdg = NULL;
	}
	if (ad->language_item) {
		ad->language_item = NULL;
	}
	if (ad->language_rdg) {
		ad->language_rdg = NULL;
	}

	/* unregister motion vconf changed callback */
	/*unregister_vconf_changing(VCONFKEY_WMS_WMANAGER_CONNECTED, change_language_enabling); */

	eext_circle_surface_del(ad->circle_surface);

	int ret = system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE);
	DBG("locale tzone unset cb ret = %s", get_error_message(ret));
}

void app_pause(void *data)
{
	DBG("Setting - app_pause()");

	appdata *ad = data;
	if (ad) {
		if (ad->MENU_TYPE == SETTING_VOLUME_2_DEPTH) {
			_stop_all_volume_sound();
		} else if (ad->MENU_TYPE == SETTING_SOUND_RINGTONE ||
				   ad->MENU_TYPE == SETTING_SOUND_NOTIFICATION) {
			_stop_all_sound_play();
		} else if (ad->MENU_TYPE == SETTING_BATTERY) {
			_clear_battery_cb(NULL, NULL, NULL, NULL);
		} else if (ad->MENU_TYPE == SETTING_SOUND) {
			_stop_all_sound_play();
		}
	}
}

void app_resume(void *data)
{
	DBG("Setting - app_resume()");

	if (running)
		running = false;
}

void app_reset(app_control_h service, void *data)
{
	DBG("Setting - app_reset()");

	appdata *ad = data;

	char *operation = NULL;
	app_control_get_operation(service, &operation);
	DBG("operation : %s", operation);
	if (!ad->is_first_launch) {
		if (operation && !strcmp(operation, "http://tizen.org/appcontrol/operation/main")) {
			Elm_Object_Item *bottom = elm_naviframe_bottom_item_get(ad->nf);
			Elm_Object_Item *top = elm_naviframe_top_item_get(ad->nf);

			if (ad->popup) {
				evas_object_del(ad->popup);
				ad->popup = NULL;
			}

			while (bottom != top) {
				elm_object_item_del(top);
				top = elm_naviframe_top_item_get(ad->nf);
			}

			if (ad->main_genlist) {
				elm_genlist_item_show(elm_genlist_first_item_get(ad->main_genlist),
									  ELM_GENLIST_ITEM_SCROLLTO_TOP);
				elm_layout_signal_emit(ad->main_genlist, "do-show-vbar", "");

				if (scrl_timer) {
					ecore_timer_del(scrl_timer);
					scrl_timer = NULL;
				}
				scrl_timer = ecore_timer_add(1, (Ecore_Task_Cb)_scroller_timer_cb, ad->main_genlist);
			}
		}
	} else {
		ad->is_first_launch = 0;
	}

	if (ad->win_main) {
		elm_win_activate(ad->win_main);
	}

	ad->MENU_TYPE = SETTING_MAIN;

	/* Register language change vconf */
	/*register_vconf_changing(VCONFKEY_WMS_WMANAGER_CONNECTED, change_language_enabling, NULL); */
	/*return TRUE; */
}

void app_temp_(void *data)
{
	DBG("Setting - temp()");
}

int main(int argc, char *argv[])
{
	DBG("%s,%d", __func__, __LINE__);

	appdata ad;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	int ret = 0;
#if 0
	event_callback.create = app_temp_;
	event_callback.terminate = app_temp_;
	event_callback.pause = app_temp_;
	event_callback.resume = app_temp_;
	event_callback.app_control = app_temp_;
#else
	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_reset;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, NULL, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, NULL, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, _lang_changed, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, NULL, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, NULL, NULL);
#endif
	memset(&ad, 0x0, sizeof(appdata));

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (APP_ERROR_NONE != ret) {
		\
		ERR("app_main() is failed. err = %d", ret);
	}

	return ret;
}

#if 0
int main(int argc, char *argv[])
{
	appdata ad;

	DBG("[TIME] 1. aul_launch -> main :: Setting main : %d msec ", appcore_measure_time_from("APP_START_TIME"));
	appcore_measure_start();

	struct appcore_ops ops = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.reset = app_reset,
	};

	memset(&ad, 0x0, sizeof(appdata));
	ops.data = &ad;

	ad.MENU_TYPE = SETTING_MAIN;

	DBG("[TIME] 2. main : %d msec ", appcore_measure_time());
	appcore_measure_start();

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
#endif
