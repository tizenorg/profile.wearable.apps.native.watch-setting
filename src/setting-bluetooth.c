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
 * setting-bluetooth.c
 *
 *  Created on: Oct 9, 2013
 *      Author: min-hoyun
 */

#include <feedback.h>

#include "setting-bluetooth.h"
#include "setting_control_bt.h"
#include "setting_data_vconf.h"
#include "setting-common-sound.h"
#include "util.h"

static void _blutooth_cb(void *data, Evas_Object *obj, void *event_info);
static void _visibility_cb(void *data, Evas_Object *obj, void *event_info);
static void _BT_headset_cb(void *data, Evas_Object *obj, void *event_info);

static struct _bt_menu_item bt_menu_its[] = {
	{ "IDS_QP_BUTTON_BLUETOOTH", 		DISABLE, _blutooth_cb   	},
	{ "IDS_VCALL_BODY_BT_HEADSET", 		DISABLE, _BT_headset_cb   	},
	{ "IDS_ST_MBODY_MAKE_VISIBLE_ABB", 	DISABLE, _visibility_cb 	},	/* "IDS_OP_BODY_VISIBILITY" */
	{ NULL, DISABLE, NULL }
};

static char *bluetooth_enable_str[] = {
	"IDS_ST_BODY_OFF_M_STATUS",
	"IDS_ST_BODY_ON_M_STATUS",
	"IDS_COM_POP_PROCESSING"
};

static char *visible_str[] = {
	"IDS_ST_BODY_DISABLED_M_STATUS",
	"IDS_ST_BODY_YOUR_GEAR_IS_VISIBLE_FOR_PS_ABB"
};


static Ecore_Timer *bt_timer = NULL;
static Ecore_Timer *vb_timer = NULL;

static Elm_Object_Item *bt_it = NULL;
static Elm_Object_Item *vb_it = NULL;
static Elm_Object_Item *hf_it = NULL;

static Evas_Object *bt_genlist = NULL;
static Evas_Object *g_bt_check = NULL;
static Evas_Object *g_vb_check = NULL;

static int is_bt_operating = BT_NON_OPERATING;
static int timeout_seconds = VISIBILITY_TIMEOUT;
static bt_adapter_visibility_mode_e visibility_mode = BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE;
static int origin_bt_enable;
static double bt_time = 0.0;
static int is_connected_hf = FALSE;
static char *g_device_name = NULL;


static void _init_bt_value();
static int is_handsfree_connected();
static void _bt_genlist_update();
static int is_disable_visibility_item_view();
static void _update_visibility_view();
static void _alternate_bt_mode(void *data);

static char *_gl_bt_title_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_gl_bt_check_get(void *data, Evas_Object *obj, const char *part);
static void hf_event_handler(int event,  bt_hf_event_param_t *data, void *user_data);



static void sap_state_vconf_change_cb(keynode_t *key, void *data)
{
	_update_visibility_item_view(is_handsfree_connected());
}

static void _bt_adapter_state_enabled_cb(int result, bt_adapter_state_e adapter_state, void *user_data)
{
	DBG("Setting - _bt_adapter_state_enabled_cb() is called!");

	if (adapter_state == BT_ADAPTER_ENABLED) {
		DBG("Setting - BT adapter state : BT_ADAPTER_ENABLED");

		bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable = 1;

		_bt_genlist_update();
	} else if (adapter_state == BT_ADAPTER_DISABLED) {
		DBG("Setting - BT adapter state : BT_ADAPTER_DISABLED");

		bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable = 0;
		is_connected_hf = FALSE;

		_bt_genlist_update();
	}
	_init_bt_value();
}

void initialize_bt()
{
	DBG("Setting - initialize_bt()");

	if (bt_initialize() != BT_ERROR_NONE) {
		ERR("Setting - bt_initialize() is failed....");
		return;
	}
	DBG("Setting - bt_initialize() is success");

	if (bt_adapter_set_state_changed_cb(_bt_adapter_state_enabled_cb, NULL) != BT_ERROR_NONE) {
		ERR("[%s] bt_adapter_set_state_changed_cb() failed.", __FUNCTION__);
		return;
	} else {
		DBG("bt_adapter_set_state_changed_cb() is success!");
	}

	bluetooth_hf_init(hf_event_handler, NULL);

	register_vconf_changing("memory/private/sap/conn_status", sap_state_vconf_change_cb, NULL);

	int ret;
	ret = feedback_initialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		DBG("feedback_initialize failed");
	}
}

Eina_Bool _clear_bluetooth_cb(void *data, Elm_Object_Item *it)
{
	clear_bt_resource();

	return EINA_TRUE;
}

static void _disable_visibility_item_view()
{
	bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable = DISABLE;
	timeout_seconds = VISIBILITY_TIMEOUT;

	_update_visibility_view();
}

static void hf_event_handler(int event,  bt_hf_event_param_t *data, void *user_data)
{
	switch (event) {
		case BLUETOOTH_EVENT_HF_CONNECTED:
			DBG("Setting - BLUETOOTH_EVENT_HF_CONNECTED");
			_update_visibility_item_view(TRUE);
			break;

		case BLUETOOTH_EVENT_HF_DISCONNECTED:
			DBG("Setting - BLUETOOTH_EVENT_HF_DISCONNECTED");
			_update_visibility_item_view(is_handsfree_connected());
			break;
	}
}

void _update_visibility_item_view(int is_hf_connected)
{
	DBG("Setting - _update_view() is called!!");

	if (is_hf_connected) {
		is_connected_hf = TRUE;

		if (vb_timer) {
			ecore_timer_del(vb_timer);
			vb_timer = NULL;
		}

		bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable = DISABLE;
		timeout_seconds = VISIBILITY_TIMEOUT;

		_update_visibility_view();
	} else {
		is_connected_hf = FALSE;

		_update_visibility_view();
	}
}

void clear_bt_resource()
{
	bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable = DISABLE;
	is_bt_operating = BT_NON_OPERATING;

	if (timeout_seconds == 0 && vb_timer) {
		ecore_timer_del(vb_timer);
		vb_timer = NULL;

		bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable = DISABLE;
		timeout_seconds = VISIBILITY_TIMEOUT;
	}

	bluetooth_hf_deinit();

	int ret = bt_adapter_unset_state_changed_cb();

	if (ret != BT_ERROR_NONE) {
		ERR("Setting - bt adapter unset state changed cb is fail");
	}

	if (bt_deinitialize() < 0) {
		ERR("Setting - bt_deinitialize() failed.");
		return;
	}
	DBG("Setting - bt_deinitialize() is success");

	bt_genlist = NULL;
	vb_it = NULL;
	bt_it = NULL;
	g_bt_check = NULL;
	g_vb_check = NULL;

	/* Unregister SAP status vconf changeing callback */
	unregister_vconf_changing("memory/private/sap/conn_status", sap_state_vconf_change_cb);

	ret = feedback_deinitialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		DBG("feedback_deinitialize failed");
	}
}

static void _init_bt_value()
{
	bt_timer = NULL;
	bt_time = 0.0;
	timeout_seconds = VISIBILITY_TIMEOUT;
	bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable = DISABLE;
	is_bt_operating = BT_NON_OPERATING;
}

static void _alternate_bt_mode(void *data)
{
	int ret = 0;
	bt_adapter_state_e value;

	/*appdata * ad = data; */

	int prev_bt_enable = bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable;
	bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable = 2;
	if (bt_it != NULL) {
		elm_genlist_item_fields_update(bt_it, "elm.text.2", ELM_GENLIST_ITEM_FIELD_TEXT);
		elm_check_state_set(g_bt_check, !prev_bt_enable);
		edje_object_signal_emit(elm_layout_edje_get(g_bt_check), "elm,state,disabled", "elm");
	}

	if (bt_adapter_get_state(&value) != BT_ERROR_NONE) {
		ERR("Setting - bt_adapter_get_state() is failed ");
		_init_bt_value();
		return;
	}

	if (value == EINA_TRUE) {
		DBG("Setting - Current bt is on....disable...");

		bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable = 0;
		is_connected_hf = TRUE;
		_disable_visibility_item_view();

		if (hf_it) {
			elm_genlist_item_update(hf_it);
		}

		ret = bt_adapter_disable();

		timeout_seconds = 0;	/* vb_timer stop!! */
	} else {
		DBG("Setting - Current bt is off....enable...");
		ret = bt_adapter_enable();
	}

	if (ret != BT_ERROR_NONE) {
		DBG("Setting - Enalbe or Disable failed.... : %d", ret);

		bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable = origin_bt_enable;

		_bt_genlist_update();
		_init_bt_value();

		return;
	}
}

static void _blutooth_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - %s", "Setting - _blutooth_cb is called");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);

	if (is_bt_operating == BT_OPERATING) {
		DBG("Setting - _blutooth_cb() is same status ");

		return;
	}
	is_bt_operating = BT_OPERATING;

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "Setting - _blutooth_cb - ad or check is null");
		return;
	}

	/* previous state backup */
	origin_bt_enable = bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable;

	if (vb_timer != NULL && vb_it != NULL) {
		DBG("Setting - vb_it is disabled");

		bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable = DISABLE;
		timeout_seconds = 0;

		ecore_timer_del(vb_timer);
		vb_timer = NULL;

		_update_visibility_view();

		/*elm_genlist_item_update(vb_it); */
	}

	_alternate_bt_mode(data);
}

static int is_valid_timeout(int seconds)
{
	return ((seconds > 0) && (seconds <= VISIBILITY_TIMEOUT));
}

static void _bt_genlist_update()
{
	if (bt_it) {
		elm_genlist_item_fields_update(bt_it, "elm.text.2", ELM_GENLIST_ITEM_FIELD_TEXT);

		int bt_enable = bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable;
		edje_object_signal_emit(elm_layout_edje_get(g_bt_check), "elm,state,enabled", "elm");
		elm_check_state_set(g_bt_check, (bt_enable == 1) ? EINA_TRUE : EINA_FALSE);
	}
	if (hf_it) {
		elm_genlist_item_update(hf_it);
	}
	if (vb_it) {
		elm_genlist_item_fields_update(vb_it, "elm.text.1", ELM_GENLIST_ITEM_FIELD_TEXT);
		elm_genlist_item_fields_update(vb_it, "elm.text.2", ELM_GENLIST_ITEM_FIELD_TEXT);

		elm_check_state_set(g_vb_check, (bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable == 1) ? EINA_TRUE : EINA_FALSE);

		if (is_disable_visibility_item_view()) {
			edje_object_signal_emit(elm_layout_edje_get(g_vb_check), "elm,state,disabled", "elm");
		} else {
			edje_object_signal_emit(elm_layout_edje_get(g_vb_check), "elm,state,enabled", "elm");
		}
	}
}

static void _update_visibility_view()
{
	DBG("Setting - _update_visibility_view()");

	if (vb_it) {
		elm_genlist_item_fields_update(vb_it, "elm.text.1", ELM_GENLIST_ITEM_FIELD_TEXT);
		elm_genlist_item_fields_update(vb_it, "elm.text.2", ELM_GENLIST_ITEM_FIELD_TEXT);

		elm_check_state_set(g_vb_check, (bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable == TRUE)
		                    ? EINA_TRUE : EINA_FALSE);

		if (is_disable_visibility_item_view()) {
			edje_object_signal_emit(elm_layout_edje_get(g_vb_check), "elm,state,disabled", "elm");
		} else {
			edje_object_signal_emit(elm_layout_edje_get(g_vb_check), "elm,state,enabled", "elm");
		}
	}
}

static void _update_visibility_item_update(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("Setting - ad is null.");
		return;
	}

	if (ad->MENU_TYPE == SETTING_BLUETOOTH) {
		DBG("Setting - update_visibility_item_update");

		_update_visibility_view();
		/*_bt_genlist_update(); */
	}
}

static void _init_vb_data(void *data)
{
	bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable = DISABLE;
	timeout_seconds = VISIBILITY_TIMEOUT;
	_update_visibility_item_update(data);
	vb_timer = NULL;
}

static Eina_Bool __vb_timeout(void *data)
{
	if (is_valid_timeout(timeout_seconds)) {
		DBG("Setting - %d seconds", timeout_seconds);

		--timeout_seconds;

		_update_visibility_item_update(data);

		return ECORE_CALLBACK_RENEW;
	}

	bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable = DISABLE;
	_update_visibility_item_update(data);

	DBG("Setting - visibility time is out.");

	if (visibility_mode != BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE) {
		if (bt_adapter_set_visibility(BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE, 0) != BT_ERROR_NONE) {
			DBG("[%s] Setting - bt_adapter_set_visibility(NON) failed.", __FUNCTION__);
			_init_vb_data(data);
			return ECORE_CALLBACK_CANCEL;
		}
		visibility_mode = BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE;
		DBG("[%s] Setting - bt_adapter_set_visibility(NON) success.", __FUNCTION__);
	}

	_init_vb_data(data);

	return ECORE_CALLBACK_CANCEL;
}

static void _start_visibility_timer(void *data)
{
	if (vb_timer) {	/* Timer Start */
		ecore_timer_del(vb_timer);
		vb_timer = NULL;
	}
	vb_timer = ecore_timer_add(1.0, (Ecore_Task_Cb) __vb_timeout, data);
}

static void _bt_visibility_mode(void *data)
{
	bt_adapter_state_e bt_state;

	if (bt_adapter_get_state(&bt_state) != BT_ERROR_NONE) {
		DBG("[%s] Setting - bt_adapter_get_state() failed.", __FUNCTION__);
		return;
	}

	int state = DISABLE;
	vconf_get_int(VCONFKEY_BT_STATUS, &state);

	DBG("Setting - bt state : %d", state);

	if (bt_state == BT_ADAPTER_ENABLED) {
		if (bt_adapter_get_visibility(&visibility_mode, NULL) != BT_ERROR_NONE) {
			DBG("[%s] Setting - bt_adapter_get_visibility() failed.", __FUNCTION__);
			return;
		}

		DBG("Setting - visibility_mode : %d", visibility_mode);

		if (visibility_mode == BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE) {
			if (bt_adapter_set_visibility(BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE, 0) != BT_ERROR_NONE) {
				DBG("[%s] Setting - bt_adapter_set_visibility(VISIBLE) failed.", __FUNCTION__);
			} else {
				visibility_mode = BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE;
				DBG("[%s] Setting - bt_adapter_set_visibility(VISIBLE) success.", __FUNCTION__);

				bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable = ENABLE;
				timeout_seconds = VISIBILITY_TIMEOUT;

				_update_visibility_view();

				_start_visibility_timer(data);	/* Timer start */
			}
		} else if (visibility_mode == BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE) {
			DBG("[%s] Setting - Visibility mode was already set as BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE.", __FUNCTION__);

			timeout_seconds = 0;	/* Timer stop; */

			if (bt_adapter_set_visibility(BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE, 0) != BT_ERROR_NONE) {
				DBG("[%s] Setting - bt_adapter_set_visibility(NON) failed.", __FUNCTION__);
			} else {
				visibility_mode = BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE;
				DBG("[%s] Setting - bt_adapter_set_visibility(NON) success.", __FUNCTION__);
			}
		}
	}
}

static int is_disable_visibility_item_view()
{
	return (!bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable || is_connected_hf);
}

static void _visibility_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("%s", "Setting - _visibility_cb is called");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);

	int touch_sound_enable = false;
	if (get_sound_mode() == SOUND_MODE_SOUND) {
		vconf_get_bool(VCONFKEY_SETAPPL_TOUCH_SOUNDS_BOOL, &touch_sound_enable);
		if (touch_sound_enable) {
			feedback_play(FEEDBACK_PATTERN_TAP);
		}
	}

	if (is_disable_visibility_item_view()) {
		DBG("Setting - disable visibility!!");
		return;
	}

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "Setting - _visibility_cb - ad or check is null");
		return;
	}

	_bt_visibility_mode(data);
}

static int _is_enable_BT_headset()
{
	int enable = FALSE;

	vconf_get_int(VCONFKEY_BT_STATUS, &enable);

	return (enable != VCONFKEY_BT_STATUS_OFF) && bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable;
}

static void _BT_headset_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("%s", "Setting - _BT_headset_cb is called");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);

	if (!_is_enable_BT_headset())
		return;

	app_control_h service;
	app_control_create(&service);
	app_control_set_app_id(service, "org.tizen.bluetooth");
	app_control_add_extra_data(service, "launch-type", "setting");
	app_control_send_launch_request(service, NULL, NULL);
	app_control_destroy(service);
}

static char *_get_device_name()
{
	char *bt_adapter_name = NULL;

#if 0
	bt_adapter_state_e bt_state;
	if (bt_adapter_get_state(&bt_state) != BT_ERROR_NONE) {
		DBG("[%s] Setting - bt_adapter_get_state() failed.", __FUNCTION__);
		return NULL;
	}
	if (bt_adapter_get_name(&bt_adapter_name) != BT_ERROR_NONE) {
		ERR("%s,%d: bt get name fail", __func__, __LINE__);
		return NULL;
	}
#endif
	bt_adapter_name = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);

	return strdup(bt_adapter_name);
}

static char *_gl_bt_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Bt_Item_Data *id = data;
	int index = id->index;
	char expression[32];

	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
		if (index == BT_MENU_TYPE_BT_ON_OFF) {
			snprintf(buf, sizeof(buf) - 1, "%s", _(bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].name));
		} else if (index == BT_MENU_TYPE_VISIBLE_ON_OFF) {
			if (is_disable_visibility_item_view()) {
				strcpy(expression, "<font color=#515151>%s</font>");
			} else {
				strcpy(expression, "%s");
			}
			snprintf(buf, sizeof(buf) - 1, expression, _(bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].name));
		} else {
			if (!_is_enable_BT_headset()) {
				strcpy(expression, "<font color=#515151>%s</font>");
			} else {
				strcpy(expression, "%s");
			}
			snprintf(buf, sizeof(buf) - 1, expression, _(bt_menu_its[BT_MENU_TYPE_BT_HEADSET].name));
		}
	} else if (!strcmp(part, "elm.text.2")) {
		if (index == BT_MENU_TYPE_BT_ON_OFF) {
			DBG("BT item subtitle updated!!");
			int sub_title_type = bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable;
			char *subtitle = _(bluetooth_enable_str[sub_title_type]);
			snprintf(buf, sizeof(buf) - 1, "%s", subtitle);
		} else if (index == BT_MENU_TYPE_VISIBLE_ON_OFF) {
			if (is_disable_visibility_item_view()) {
				strcpy(expression, "<font color=#515151>%s</font>");

				if (g_device_name == NULL) {
					g_device_name = _get_device_name();
					if (g_device_name == NULL) {
						g_device_name = _(visible_str[bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable]);
					}
				}

				snprintf(buf, sizeof(buf) - 1, expression, g_device_name);
			} else {
				if (bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable) {
					int hour = timeout_seconds / 60;
					int minutes = timeout_seconds % 60;

					char time_buf[16] = {0,};
					snprintf(time_buf, sizeof(time_buf) - 1, "%02d:%02d", hour, minutes);

					snprintf(buf, sizeof(buf) - 1, _(visible_str[bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable]), time_buf);
				} else {
					g_device_name = _get_device_name();
					if (g_device_name == NULL) {
						g_device_name = _(visible_str[bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable]);
					}

					snprintf(buf, sizeof(buf) - 1, "%s", g_device_name);
				}
			}
		}
	}
	return strdup(buf);
}
#if 0
static Evas_Object *_get_emtpy_layout(Evas_Object *parent)
{
	if (parent == NULL)
		return NULL;

	Evas_Object *layout = elm_layout_add(parent);
	elm_layout_file_set(layout, EDJE_PATH, "setting-empty/swallow");

	return layout;
}
#endif

static Evas_Object *_gl_bt_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	Bt_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);
		elm_object_style_set(check, "list");
		elm_check_state_set(check, (bt_menu_its[index].is_enable == TRUE) ? EINA_TRUE : EINA_FALSE);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_repeat_events_set(check, EINA_TRUE);

		if (index == BT_MENU_TYPE_VISIBLE_ON_OFF) {
			if (is_disable_visibility_item_view()) {
				edje_object_signal_emit(elm_layout_edje_get(check), "elm,state,disabled", "elm");
			}
			g_vb_check = check;
		} else if (index == BT_MENU_TYPE_BT_ON_OFF) {
			g_bt_check = check;
		}
	}
	return check;
}

static void _bt_gl_del(void *data, Evas_Object *obj)
{
	Bt_Item_Data *id = data;
	if (id)
		free(id);
}

static void init_values()
{
	if (!vb_timer) {
		bt_menu_its[BT_MENU_TYPE_VISIBLE_ON_OFF].is_enable = DISABLE;
	}
}

static int is_handsfree_connected()
{
	int ret = FALSE;
	int headset_connected = FALSE;
	int sap_connected = FALSE;

	vconf_get_int(VCONFKEY_BT_DEVICE, &headset_connected);
	DBG("Heaadset connected : %x", headset_connected);

	if (headset_connected & VCONFKEY_BT_DEVICE_AG_CONNECTED) {
		DBG("Setting - Heaadset connected");
		return TRUE;
	}

	vconf_get_int("memory/private/sap/conn_status", &sap_connected);
	DBG("Sap connected : %d", sap_connected);

	if (sap_connected) {
		DBG("Setting - Sap connected");
		return TRUE;
	}

	return ret;
}

static int is_add_BT_headset()
{
	/* temporary code!! */
	/* this code will be changed!! */
	return TRUE;
}

static int is_BT_enable()
{
	bt_adapter_state_e enable = BT_ADAPTER_DISABLED;

	if (bt_adapter_get_state(&enable) != BT_ERROR_NONE) {
		ERR("Setting - bt_adapter_get_state() is failed ");
		_init_bt_value();
		enable = DISABLE;
	}

	return enable;
}

Evas_Object *_create_bt_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_bluetooth_list - appdata is null");
		return NULL;
	}

	Evas_Object *genlist  = NULL;
	struct _bt_menu_item *menu_its = NULL;
	int idx = 0;

	init_values();

	bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable = is_BT_enable();

	DBG("Setting - BT status is %d", bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable);

	if (bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable > 1) {
		bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable = ENABLE;
	}

	if (bt_menu_its[BT_MENU_TYPE_BT_ON_OFF].is_enable) {
		is_connected_hf = is_handsfree_connected();
	} else {
		is_connected_hf = FALSE;
	}

	elm_theme_extension_add(NULL, EDJE_PATH);
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text.1icon.1";
	itc->func.text_get = _gl_bt_title_get;
	itc->func.content_get = _gl_bt_check_get;
	itc->func.del = _bt_gl_del;

	Elm_Genlist_Item_Class *itc2 = elm_genlist_item_class_new();
	itc2->item_style = "1text";
	itc2->func.text_get = _gl_bt_title_get;
	itc2->func.del = _bt_gl_del;

	Elm_Genlist_Item_Class *itc3 = elm_genlist_item_class_new();
	itc3->item_style = "multiline.2text.1icon";
	itc3->func.text_get = _gl_bt_title_get;
	itc3->func.content_get = _gl_bt_check_get;
	itc3->func.del = _bt_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist,ad);

	menu_its = bt_menu_its;

	Elm_Genlist_Item_Class *itc_arr[3] = { itc, itc2, itc3 };

	for (idx = 0; idx < BT_LIST_ITEM_COUNT; idx++) {
		/* if bt headset is disable, continue */
		if (idx == BT_MENU_TYPE_BT_HEADSET && !is_add_BT_headset())
			continue;

		Bt_Item_Data *id = calloc(sizeof(Bt_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
					genlist,				/* genlist object */
					itc_arr[idx],			/* item class */
					id,		            	/* data */
					NULL,
					ELM_GENLIST_ITEM_NONE,
					menu_its[ idx ].func,	/* call back */
					ad);

			if (idx == BT_MENU_TYPE_BT_ON_OFF) {
				bt_it = id->item;
			} else if (idx == BT_MENU_TYPE_BT_HEADSET) {
				hf_it = id->item;
			} else {
				vb_it = id->item;
			}
		}
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc2);
	elm_genlist_item_class_free(itc3);

	bt_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	return layout;
}
