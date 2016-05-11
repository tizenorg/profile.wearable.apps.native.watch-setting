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

#include "setting-device.h"
#include "util.h"
#include "setting-common-sound.h"
#include "setting_control_haptic.h"
#include "setting_view_toast.h"
#include "setting_data_vconf.h"
#include "setting-volume.h"
#include "setting-double.h"

#define VCONFKEY_SETAPPL_NOTI_device_action_auto_open_apps			    "db/setting/sound/noti/device_action_auto_open_apps"
#define AUDIO_RESOURCE_EXTENSION	".ogg"

static struct _device_action_menu_item device_action_menu_its[] = {
	{ "Double press Home key",					0,		_double_press_home_key_cb},
	{ "Auto open apps",					0,		_auto_open_apps_cb},
};

static int DEV_TOP_MENU_SIZE =
	sizeof(device_action_menu_its) / sizeof(device_action_menu_its[0]);

char *device_action_menu_str[] = {
	"IDS_ST_BODY_INTENSITY",
	"WDS_ST_MBODY_auto_open_apps_ABB",
};


static appdata *temp_ad 					= NULL;
static Evas_Object *g_device_action_type_genlist 	= NULL;
static Elm_Object_Item *g_vib_item = NULL;

static int device_action_type 	 = 0;			/* device_action type */

static Ecore_Timer *device_action_timer = NULL;

static void _device_action_gl_cb(void *data, Evas_Object *obj, void *event_info);

void _clear_device_action_resource()
{
	if (device_action_timer) {
		ecore_timer_del(device_action_timer);
		device_action_timer = NULL;
	}

	_haptic_close();

	temp_ad = NULL;
	g_device_action_type_genlist = NULL;
	g_vib_item = NULL;

	device_action_type = 0;

	//unregister_vconf_changing(VCONFKEY_SETAPPL_device_action_STATUS_BOOL, vibrate_vconf_changed_cb);
}


void _auto_open_apps_chk_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _auto_open_apps_chk_changed_cb() is called!!");

	int is_auto_open_apps = 0;
	vconf_get_bool(VCONFKEY_SETAPPL_NOTI_device_action_auto_open_apps, &is_auto_open_apps);
	is_auto_open_apps =  !(is_auto_open_apps);
	vconf_set_bool(VCONFKEY_SETAPPL_NOTI_device_action_auto_open_apps, is_auto_open_apps);

}

Evas_Object *_gl_device_action_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	device_action_Item_Data *id = data;
	int is_auto_open_apps = 0;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);

		if (vconf_get_bool(VCONFKEY_SETAPPL_NOTI_device_action_auto_open_apps, &is_auto_open_apps)<0) {
			is_auto_open_apps = 0;
		}

		elm_check_state_set(check, (is_auto_open_apps)? EINA_TRUE : EINA_FALSE);   /*default */
		evas_object_smart_callback_add(check, "changed", _auto_open_apps_chk_changed_cb, (void *)1);
		elm_object_style_set(check, "on&off");
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_FALSE);

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

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, clear_double_app_cb, ad);
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

	appid = vconf_get_str(VCONFKEY_WMS_POWERKEY_DOUBLE_PRESSING);

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(device_action_menu_its[index % DEV_TOP_MENU_SIZE].name));
	} else if (!strcmp(part, "elm.text.sub")) {
		switch (index) {
		case 0:
			snprintf(buf, sizeof(buf) - 1, "%s", appid);
			break;
		}
		index++;
	}
	return strdup(buf);
}


static void _sound_gl_del(void *data, Evas_Object *obj)
{
	device_action_Item_Data *id = data;
	if (id)
		free(id);
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
	itc->func.del = _sound_gl_del;


	Elm_Genlist_Item_Class *itc_auto_open_apps = elm_genlist_item_class_new();
	itc_auto_open_apps->item_style = "1text.1icon.1";
	itc_auto_open_apps->func.text_get = _gl_device_action_title_get;
	itc_auto_open_apps->func.content_get = _gl_device_action_check_get;
	itc_auto_open_apps->func.del = _sound_gl_del;


	genlist = elm_genlist_add(ad->nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
#ifdef O_TYPE
	Evas_Object *circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);
	eext_circle_object_genlist_scroller_policy_set(circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(circle_genlist, EINA_TRUE);
#endif


	menu_its = device_action_menu_its;

	for (idx = 0; idx < DEV_TOP_MENU_SIZE; idx++) {
		if (idx==0)
			itc_tmp = itc;
		else
			itc_tmp = itc_auto_open_apps;

		device_action_Item_Data *id = calloc(sizeof(device_action_Item_Data), 1);
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

			if (idx == 0) {
				g_vib_item = id->item;
			}
		}
	}
	elm_genlist_item_class_free(itc_auto_open_apps);
	elm_genlist_item_class_free(itc);

	//register_vconf_changing(VCONFKEY_SETAPPL_device_action_STATUS_BOOL, vibrate_vconf_changed_cb, NULL);

	return genlist;
}

static Evas_Object *_gl_device_action_radio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_object_style_set(radio, "list");
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);
		evas_object_smart_callback_add(radio, "changed", _device_action_gl_cb, (void *)id->index);
		evas_object_propagate_events_set(radio, EINA_FALSE);

		if (device_action_type == id->index) {
			elm_radio_value_set(radio_main, device_action_type);
		}
		index++;
	}
	return radio;
}

static void _device_action_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (g_device_action_type_genlist != NULL) {
		evas_object_del(g_device_action_type_genlist);
		g_device_action_type_genlist = NULL;
	}


	elm_naviframe_item_pop(temp_ad->nf);

	if (g_vib_item != NULL) {
		elm_genlist_item_update(g_vib_item);
	}

}

static void _device_action_layout_del_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _device_action_layout_del_cb() is called!");

	_haptic_close();

	if (g_vib_item) {
		elm_genlist_item_update(g_vib_item);
	}
}

