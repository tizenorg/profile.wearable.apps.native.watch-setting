/*
 *  Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 *  Licensed under the Flora License, Version 1.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://floralicense.org/license/
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
#include <feedback.h>
#include <sys/types.h>
#include <dirent.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "setting-connection.h"
#include "util.h"
#include "setting-common-sound.h"
#include "setting_control_haptic.h"
#include "setting_view_toast.h"
#include "setting_data_vconf.h"
#include "setting-volume.h"

#define AUDIO_RESOURCE_EXTENSION	".ogg"

void _bluetooth_cb(void *data, Evas_Object *obj, void *event_info);
void _wifi_cb(void *data, Evas_Object *obj, void *event_info);
void _alerts_cb(void *data, Evas_Object *obj, void *event_info);
void _flight_mode_cb(void *data, Evas_Object *obj, void *event_info);

static void bt_status_vconf_changed_cb(keynode_t *key, void *data);
static void wifi_status_vconf_changed_cb(keynode_t *key, void *data);
static void nfc_status_vconf_changed_cb(keynode_t *key, void *data);

static struct _connection_menu_item connection_menu_its[] = {
	{ "Bluetooth",				SETTING_CONNECTION_BLUETOOTH,		_bluetooth_cb },
	{ "Wi-Fi",					SETTING_CONNECTION_WIFI,			_wifi_cb	},
	{ "NFC",					SETTING_CONNECTION_NFC,				_nfc_cb	},
	{ "Alerts",					SETTING_CONNECTION_BT_ALERTS,			_alerts_cb	},
	{ "Flight mode",			SETTING_CONNECTION_FLIGHT_MODE,		_flight_mode_cb  },
};

static int VIB_TOP_MENU_SIZE =
	sizeof(connection_menu_its) / sizeof(connection_menu_its[0]);


static appdata *temp_ad 					= NULL;
static Evas_Object *g_sound_genlist 			= NULL;
static Evas_Object *g_connection_type_genlist 	= NULL;

static int vibrate_type 	 = 0;			/* connection type */

static int ringtone_count = 0;
static int origin_connection_level;

static Ecore_Timer *connection_timer = NULL;


static void vibrate_vconf_changed_cb(keynode_t *key, void *data);

void _clear_connection_resource()
{
	if (connection_timer) {
		ecore_timer_del(connection_timer);
		connection_timer = NULL;
	}

	_haptic_close();

	temp_ad = NULL;
	g_sound_genlist = NULL;
	g_connection_type_genlist = NULL;

	vibrate_type = 0;
	origin_connection_level = 0;

	unregister_vconf_changing(VCONFKEY_SETAPPL_connection_STATUS_BOOL, vibrate_vconf_changed_cb);
}


void _bt_alerts_chk_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _bt_alerts_chk_changed_cb() is called!!");
//	int is_long_buzz = 0;
//	vconf_get_bool(VCONFKEY_SETAPPL_NOTI_connection_LONG_BUZZ, &is_long_buzz);
//	is_long_buzz =  !(is_long_buzz);
//	vconf_set_bool(VCONFKEY_SETAPPL_NOTI_connection_LONG_BUZZ, is_long_buzz);

}

void _flight_mode_chk_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _flight_mode_chk_changed_cb() is called!!");
//	int is_long_buzz = 0;
//	vconf_get_bool(VCONFKEY_SETAPPL_NOTI_connection_LONG_BUZZ, &is_long_buzz);
//	is_long_buzz =  !(is_long_buzz);
//	vconf_set_bool(VCONFKEY_SETAPPL_NOTI_connection_LONG_BUZZ, is_long_buzz);

}

Evas_Object *_gl_connection_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	connection_Item_Data *id = data;
	int index = id->index;
	int is_value= 0;


	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);
		elm_object_style_set(check, "on&off");

//		if (vconf_get_bool(VCONFKEY_SETAPPL_NOTI_connection_LONG_BUZZ, &is_value)<0) {
//			is_value = 0;
//		}

		switch(index) {
			case SETTING_CONNECTION_BT_ALERTS:
				evas_object_smart_callback_add(check, "changed", _bt_alerts_chk_changed_cb, (void *)1);
				break;
			case SETTING_CONNECTION_FLIGHT_MODE:
				evas_object_smart_callback_add(check, "changed", _flight_mode_chk_changed_cb, (void *)1);
				break;
		}

		elm_check_state_set(check, (is_value)? EINA_TRUE : EINA_FALSE);   /*default */

		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_FALSE);


		id->check = check;

	}

	return check;
}

static int get_connection_level()
{
	int mode = 1;
	int level = 0;

	vconf_get_int(VCONFKEY_SETAPPL_NOTI_connection_LEVEL_INT, &level);

	switch (level) {
	case connection_LEVEL_NONE_INT:
		mode = connection_LEVEL_NONE;
		break;
	case connection_LEVEL_LOW_INT:
		mode = connection_LEVEL_LOW;
		break;
		/*	case connection_LEVEL_MID_INT : */
		/*		mode = connection_LEVEL_MID; */
		/*		break; */
	case connection_LEVEL_HIGH_INT:
		mode = connection_LEVEL_HIGH;
		break;
	}
	return mode;
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
    app_control_destroy(service);

    running = true;

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
    app_control_destroy(service);

    running = true;

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
    app_control_set_package(service, "ug-nfc-efl");
    app_control_send_launch_request(service, NULL, NULL);
    app_control_destroy(service);

    running = true;

    if (running_timer) {
        ecore_timer_del(running_timer);
        running_timer = NULL;
    }
    running_timer = ecore_timer_add(0.5, (Ecore_Task_Cb)_app_ctrl_timer_cb, NULL);
}

void _alerts_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

}

void _flight_mode_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

}

char *_gl_connection_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;
	int tmp_int=0;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(connection_menu_its[index % VIB_TOP_MENU_SIZE].name));
	} else if (!strcmp(part, "elm.text.sub")) {
		switch (index) {
		case SETTING_CONNECTION_BLUETOOTH:
			vconf_get_int(VCONFKEY_BT_STATUS, &tmp_int);
			snprintf(buf, sizeof(buf) - 1, "%s", tmp_int?"Off":"On");
		case SETTING_CONNECTION_WIFI:
			vconf_get_int(VCONFKEY_WIFI_STATE, &tmp_int);
			snprintf(buf, sizeof(buf) - 1, "%s", tmp_int?"Off":"On");
		case SETTING_CONNECTION_NFC:
			vconf_get_bool(VCONFKEY_NFC_STATE, (bool*)&tmp_int);
			snprintf(buf, sizeof(buf) - 1, "%s", tmp_int?"Off":"On");
			break;
		case SETTING_CONNECTION_BT_ALERTS:
			snprintf(buf, sizeof(buf) - 1, "Receive BT disconnection alerts.");
			break;
		}
	}
	return strdup(buf);
}


static void _connection_gl_del(void *data, Evas_Object *obj)
{
	connection_Item_Data *id = data;
	if (id)
		free(id);
}


Evas_Object *_create_connection_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_sound_list - appdata is null");
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
#ifdef O_TYPE
	Evas_Object *circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);
	eext_circle_object_genlist_scroller_policy_set(circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(circle_genlist, EINA_TRUE);
#endif

	menu_its = connection_menu_its;

	for (idx = 0; idx < VIB_TOP_MENU_SIZE; idx++) {
		switch(idx) {
			case SETTING_CONNECTION_BLUETOOTH:
			case SETTING_CONNECTION_WIFI:
			case SETTING_CONNECTION_NFC:
				itc_tmp = itc;
				break;
			case SETTING_CONNECTION_BT_ALERTS:
				itc_tmp = text2_icon;
				break;
			case SETTING_CONNECTION_FLIGHT_MODE:
				itc_tmp = text_icon;
				break;
			default:
				itc_tmp = text_icon;
				break;
		}

		connection_Item_Data *id = calloc(sizeof(connection_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
						   genlist,			/* genlist object */
						   itc_tmp,			/* item class */
						   id,		            /* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_its[idx].func,	/* call back */
						   ad);

		}
	}
	elm_genlist_item_class_free(text_icon);
	elm_genlist_item_class_free(text2_icon);
	elm_genlist_item_class_free(itc);

	g_sound_genlist = genlist;

	register_vconf_changing(VCONFKEY_BT_STATUS, bt_status_vconf_changed_cb, ad);
	register_vconf_changing(VCONFKEY_WIFI_STATE, wifi_status_vconf_changed_cb, ad);
	register_vconf_changing(VCONFKEY_NFC_STATE, nfc_status_vconf_changed_cb, ad);
	return genlist;
}

static void bt_status_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - bt_status_vconf_changed_cb() is called!!");
}

static void wifi_status_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - wifi_status_vconf_changed_cb() is called!!");
}

static void nfc_status_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - nfc_status_vconf_changed_cb() is called!!");
}

