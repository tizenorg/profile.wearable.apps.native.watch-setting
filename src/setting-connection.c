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

#include "setting-connection.h"
#include "util.h"
#include "setting_view_toast.h"
#include "setting_data_vconf.h"
#include "setting-volume.h"

#include <app_control.h>
int app_control_set_package(app_control_h app_control, const char *package);
static Evas_Object *g_connection_genlist = NULL;

static Elm_Object_Item *g_BT_item = NULL;
static Elm_Object_Item *g_WIFI_item = NULL;
static Elm_Object_Item *g_NFC_item = NULL;
#define AUDIO_RESOURCE_EXTENSION	".ogg"

void _bluetooth_cb(void *data, Evas_Object *obj, void *event_info);
void _wifi_cb(void *data, Evas_Object *obj, void *event_info);
/*void _alerts_cb(void *data, Evas_Object *obj, void *event_info); */
void _flight_mode_cb(void *data, Evas_Object *obj, void *event_info);
void _nfc_cb(void *data, Evas_Object *obj, void *event_info);

static void bt_status_vconf_changed_cb(keynode_t *key, void *data);
static void wifi_status_vconf_changed_cb(keynode_t *key, void *data);
static void nfc_status_vconf_changed_cb(keynode_t *key, void *data);

static struct _connection_menu_item connection_menu_its[] = {
#ifndef FEATURE_SETTING_EMUL
	{ "Bluetooth",				SETTING_CONNECTION_BLUETOOTH,		_bluetooth_cb },
#endif
#ifndef FEATURE_SETTING_EMUL
	{ "Wi-Fi",					SETTING_CONNECTION_WIFI,			_wifi_cb	},
#endif
	{ "NFC",					SETTING_CONNECTION_NFC,				_nfc_cb	},
	/*	{ "Alerts",					SETTING_CONNECTION_BT_ALERTS,			_alerts_cb	}, */
#ifndef FEATURE_SETTING_EMUL
	{ "Flight mode",			SETTING_CONNECTION_FLIGHT_MODE,		_flight_mode_cb	 },
#endif
};

static int CONNECT_TOP_MENU_SIZE =
	sizeof(connection_menu_its) / sizeof(connection_menu_its[0]);


static appdata *temp_ad					= NULL;
static bool running_connection			= false;
static Ecore_Timer *running_timer = NULL;

static char *
_gl_menu_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	snprintf(buf, 1023, "%s", "Connections");
	return strdup(buf);
}

static Eina_Bool _app_ctrl_timer_cb(void *data)
{
	DBG("reset flag");
	running_connection = false;
	running_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

void _clear_connection_resource()
{
	if (running_timer) {
		ecore_timer_del(running_timer);
		running_timer = NULL;
	}

	g_connection_genlist = NULL;
	g_BT_item = NULL;
	g_WIFI_item = NULL;
	g_NFC_item = NULL;

	temp_ad = NULL;

	unregister_vconf_changing(VCONFKEY_BT_STATUS, bt_status_vconf_changed_cb);
	unregister_vconf_changing(VCONFKEY_WIFI_STATE, wifi_status_vconf_changed_cb);
	unregister_vconf_changing(VCONFKEY_NFC_STATE, nfc_status_vconf_changed_cb);
}


void _bt_alerts_chk_changed_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _bt_alerts_chk_changed_cb() is called!!");
	/*	int is_long_buzz = 0; */
	/*	vconf_get_bool(VCONFKEY_SETAPPL_NOTI_connection_LONG_BUZZ, &is_long_buzz); */
	/*	is_long_buzz =	!(is_long_buzz); */
	/*	vconf_set_bool(VCONFKEY_SETAPPL_NOTI_connection_LONG_BUZZ, is_long_buzz); */

}

static void _set_flight_mode_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = temp_ad;
	Evas_Object *check = (Evas_Object *)data;

	int is_flight_mode = 0;
	vconf_set_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, is_flight_mode);

	elm_check_state_set(check,	EINA_FALSE);

	if (ad && ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	back_button_cb_pop();
}

static void _set_flight_mode_ok_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = temp_ad;
	Evas_Object *check = (Evas_Object *)data;

	int is_flight_mode = 1;
	vconf_set_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, is_flight_mode);
	elm_check_state_set(check,	EINA_TRUE);

	if (ad && ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	back_button_cb_pop();
}

void back_key_flight_mode_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	int is_flight_mode = 0;
	Evas_Object *check = (Evas_Object *)data;
	vconf_set_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, is_flight_mode);
	elm_check_state_set(check, EINA_FALSE);

	appdata *ad = (appdata *)temp_ad;
	if (ad && ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	back_button_cb_pop();
}

static void _flight_mode_check_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	appdata *ad = temp_ad;
	Evas_Object *check = (Evas_Object *)data;
	int is_flight_mode = 0;

	if (ad == NULL) {
		DBG("%s", "_flight_mode_check_cb - appdata or check is null");
		return;
	}

	DBG("_flight_mode_check_cb is called!!!!!!!");

	vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &is_flight_mode);
	DBG("is_flight_mode:%d ", is_flight_mode);

	if (!is_flight_mode) {
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
		snprintf(buf, sizeof(buf) - 1, font_setting, "Flight mode disables calls, messaging and all connections. To use Wi-Fi and Bluetooth go to Settings");

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
		evas_object_smart_callback_add(btn1, "clicked", _set_flight_mode_cancel_cb, check);

		icon = elm_image_add(btn1);
		elm_image_file_set(icon, IMG_DIR"tw_ic_popup_btn_delete.png", NULL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(btn1, "elm.swallow.content", icon);
		evas_object_show(icon);

		btn2 = elm_button_add(popup);
		elm_object_style_set(btn2, "popup/circle/right");
		evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(popup, "button2", btn2);
		evas_object_smart_callback_add(btn2, "clicked", _set_flight_mode_ok_clicked_cb, check);

		icon = elm_image_add(btn2);
		elm_image_file_set(icon, IMG_DIR"tw_ic_popup_btn_check.png", NULL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(btn2, "elm.swallow.content", icon);
		evas_object_show(icon);

		evas_object_show(popup);
		back_button_cb_push(&back_key_flight_mode_popup_cb, check, NULL, g_connection_genlist, "g_connection_genlist");
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _hw_back_key_cb, NULL);

	} else {
		/* disable Flight mode off with out popup */
		int flight_mode = 0;
		DBG("Cancel Flight mode!");
		vconf_set_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, flight_mode);

	}

}

Evas_Object *_gl_connection_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	connection_Item_Data *id = data;
	int index = id->index;
	int is_value = 0;


	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);

		switch (index) {
		/*		case SETTING_CONNECTION_BT_ALERTS: */
		/*			evas_object_event_callback_add(check, EVAS_CALLBACK_MOUSE_DOWN, _bt_alerts_chk_changed_cb, (void *)check); */
		/*			break; */
		case SETTING_CONNECTION_FLIGHT_MODE:
			vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &is_value);
			evas_object_event_callback_add(check, EVAS_CALLBACK_MOUSE_DOWN, _flight_mode_check_cb, (void *)check);
			break;
		}

		elm_check_state_set(check, (is_value) ? EINA_TRUE : EINA_FALSE);	  /*default */

		elm_object_style_set(check, "on&off");
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_TRUE);

		id->check = check;

	}

	return check;
}


void _bluetooth_cb(void *data, Evas_Object *obj, void *event_info)
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

	ad->service_bt = service;

	running_connection = true;

	if (running_timer) {
		ecore_timer_del(running_timer);
		running_timer = NULL;
	}
	running_timer = ecore_timer_add(0.5, (Ecore_Task_Cb)_app_ctrl_timer_cb, NULL);
}

void _wifi_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	DBG("wifi_cb in");
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	app_control_h service;
	app_control_create(&service);
	app_control_set_package(service, "org.tizen.w-wifi");
	app_control_send_launch_request(service, NULL, NULL);

	ad->service_wifi = service;

	running_connection = true;

	if (running_timer) {
		ecore_timer_del(running_timer);
		running_timer = NULL;
	}
	running_timer = ecore_timer_add(0.5, (Ecore_Task_Cb)_app_ctrl_timer_cb, NULL);
}

void _nfc_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	DBG("_nfc_cb in");
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	app_control_h service;
	app_control_create(&service);
	app_control_set_package(service, "org.tizen.nfc-setting-app");
	app_control_send_launch_request(service, NULL, NULL);

	ad->service_nfc = service;

	running_connection = true;

	if (running_timer) {
		ecore_timer_del(running_timer);
		running_timer = NULL;
	}
	running_timer = ecore_timer_add(0.5, (Ecore_Task_Cb)_app_ctrl_timer_cb, NULL);
}

/*void _alerts_cb(void *data, Evas_Object *obj, void *event_info) */
/*{ */
/*	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE); */
/* */
/*} */

void _flight_mode_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("_flight_mode_cb is called!!!!!!!");
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

}

char *_gl_connection_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;
	int val = 0;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(connection_menu_its[index % CONNECT_TOP_MENU_SIZE].name));
	} else if (!strcmp(part, "elm.text.1")) {
		switch (index) {
		case SETTING_CONNECTION_BLUETOOTH:
			vconf_get_int(VCONFKEY_BT_STATUS, &val);
			snprintf(buf, sizeof(buf) - 1, "%s", val ? "On" : "Off");
			break;
		case SETTING_CONNECTION_WIFI:
			vconf_get_int(VCONFKEY_WIFI_STATE, &val);
			snprintf(buf, sizeof(buf) - 1, "%s", val ? "On" : "Off");
			break;
		case SETTING_CONNECTION_NFC:
			vconf_get_bool(VCONFKEY_NFC_STATE, &val);
			snprintf(buf, sizeof(buf) - 1, "%s", val ? "On" : "Off");
			break;
			/*		case SETTING_CONNECTION_BT_ALERTS: */
			/*			snprintf(buf, sizeof(buf) - 1, "Receive BT disconnection alerts."); */
			/*			break; */
		}
	}
	ERR("_gl_connection_title_get part: %s, %s get status : %d", part, buf, val);
	return strdup(buf);
}


static void _connection_gl_del(void *data, Evas_Object *obj)
{
	connection_Item_Data *id = data;
	FREE(id);
}


Evas_Object *_create_connection_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_connection_list - appdata is null");
		return NULL;
	}

	temp_ad = ad;

	Evas_Object *genlist  = NULL;
	struct _connection_menu_item *menu_its = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc_tmp;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text";
	itc->func.text_get = _gl_connection_title_get;
	itc->func.del = _connection_gl_del;

	Elm_Genlist_Item_Class *text_icon = elm_genlist_item_class_new();
	text_icon->item_style = "1text.1icon.1";
	text_icon->func.text_get = _gl_connection_title_get;
	text_icon->func.content_get = _gl_connection_check_get;
	text_icon->func.del = _connection_gl_del;

	Elm_Genlist_Item_Class *text2_icon = elm_genlist_item_class_new();
	text2_icon->item_style = "2text.1icon.1";
	text2_icon->func.text_get = _gl_connection_title_get;
	text2_icon->func.content_get = _gl_connection_check_get;
	text2_icon->func.del = _connection_gl_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	Evas_Object *circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);
	eext_circle_object_genlist_scroller_policy_set(circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(circle_genlist, EINA_TRUE);

	Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
	title_item->func.text_get = _gl_menu_title_text_get;
	title_item->item_style = "title";
	title_item->func.del = _connection_gl_del;

	elm_genlist_item_append(genlist, title_item, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_item);

	menu_its = connection_menu_its;


	for (idx = 0; idx < CONNECT_TOP_MENU_SIZE; idx++) {
		switch (connection_menu_its[idx].type) {
		case SETTING_CONNECTION_BLUETOOTH:
		case SETTING_CONNECTION_WIFI:
		case SETTING_CONNECTION_NFC:
			itc_tmp = itc;
			break;
		/*		case SETTING_CONNECTION_BT_ALERTS: */
		/*			itc_tmp = text2_icon; */
		/*			break; */
		case SETTING_CONNECTION_FLIGHT_MODE:
			itc_tmp = text_icon;
			break;
		default:
			itc_tmp = text_icon;
			break;
		}

		connection_Item_Data *id = calloc(sizeof(connection_Item_Data), 1);
		if (id) {
			id->index = connection_menu_its[idx].type;
			id->item = elm_genlist_item_append(
						   genlist,			/* genlist object */
						   itc_tmp,			/* item class */
						   id,					/* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_its[idx].func,	/* call back */
						   ad);
			switch (connection_menu_its[idx].type) {
			case SETTING_CONNECTION_BLUETOOTH:
				g_BT_item = id->item;
				break;
			case SETTING_CONNECTION_WIFI:
				g_WIFI_item = id->item;
				break;
			case SETTING_CONNECTION_NFC:
				g_NFC_item = id->item;
				break;
			}
		}
	}
	elm_genlist_item_class_free(text_icon);
	elm_genlist_item_class_free(text2_icon);
	elm_genlist_item_class_free(itc);

	Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
	padding->item_style = "padding";
	padding->func.del = _connection_gl_del;

	elm_genlist_item_append(genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(padding);

	g_connection_genlist = genlist;

	register_vconf_changing(VCONFKEY_BT_STATUS, bt_status_vconf_changed_cb, ad);
	register_vconf_changing(VCONFKEY_WIFI_STATE, wifi_status_vconf_changed_cb, ad);
	register_vconf_changing(VCONFKEY_NFC_STATE, nfc_status_vconf_changed_cb, ad);
	return genlist;
}

void _update_connection_list(Elm_Object_Item *item)
{
	if (item) {
		DBG("update connection genlist");
		elm_genlist_item_update(item);
	}
}

static void bt_status_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - bt_status_vconf_changed_cb() is called!!");
	_update_connection_list(g_BT_item);
}

static void wifi_status_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - wifi_status_vconf_changed_cb() is called!!");
	_update_connection_list(g_WIFI_item);
}

static void nfc_status_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - nfc_status_vconf_changed_cb() is called!!");
	_update_connection_list(g_NFC_item);
}

