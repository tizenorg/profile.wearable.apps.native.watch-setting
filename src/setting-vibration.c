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

#include "setting-sound.h"
#include "setting-vibration.h"
#include "util.h"
#include "setting-common-sound.h"
#include "setting_control_haptic.h"
#include "setting_view_toast.h"
#include "setting_data_vconf.h"
#include "setting-volume.h"

#define VCONFKEY_SETAPPL_NOTI_VIBRATION_LONG_BUZZ				"db/setting/sound/noti/vibration_long_buzz"
#define AUDIO_RESOURCE_EXTENSION	".ogg"

static struct _sound_menu_item vibration_menu_its[] = {
	{ "Intensity",/*"IDS_ST_BODY_INTENSITY",*/					0,		_intensity_cb	},
	{ "Long Buzz",/*"WDS_ST_MBODY_LONG_BUZZ_ABB",*/					0,		_long_buzz_cb  },
};

static int VIB_TOP_MENU_SIZE =
	sizeof(vibration_menu_its) / sizeof(vibration_menu_its[0]);

char *vibration_menu_str[] = {
	"IDS_ST_BODY_INTENSITY",
	"WDS_ST_MBODY_LONG_BUZZ_ABB",
};

char *vibration_power_str[] = {
	"IDS_ST_OPT_STRONG_M_INTENSITY",
	"IDS_ST_OPT_WEAK_M_INTENSITY",
};

static appdata *temp_ad						= NULL;
static Evas_Object *g_vibration_genlist				= NULL;
static Evas_Object *g_vibration_type_genlist	= NULL;
static Elm_Object_Item *g_vib_item = NULL;

static int vibrate_type		 = 0;			/* Vibration type */

static int origin_vibration_level;

static Ecore_Timer *vibration_timer = NULL;


static void vibrate_vconf_changed_cb(keynode_t *key, void *data);
static void _vibration_gl_cb(void *data, Evas_Object *obj, void *event_info);

enum {
	VIBRATION_TITLE_VIBRATION,
	VIBRATION_TITLE_INTENSITY
};

static char *
_gl_menu_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	int title_idx = (int)data;
	switch(title_idx) {
		case VIBRATION_TITLE_VIBRATION:
		snprintf(buf, 1023, "%s", _("IDS_ST_HEADER_VIBRATION_ABB"));
		break;
		case VIBRATION_TITLE_INTENSITY:
		snprintf(buf, 1023, "%s", "Intensity");
		break;

	}
	return strdup(buf);
}

void _clear_vibration_resource(void *data, Evas_Object *obj, void *event_info)
{
	if (vibration_timer) {
		ecore_timer_del(vibration_timer);
		vibration_timer = NULL;
	}

	_haptic_close();


	if (temp_ad) {
		elm_naviframe_item_pop(temp_ad->nf);
	} else {
		ERR("data ptr is NULL");
	}

	temp_ad = NULL;
	g_vibration_genlist = NULL;
	g_vibration_type_genlist = NULL;
	g_vib_item = NULL;

	vibrate_type = 0;
	origin_vibration_level = 0;

	unregister_vconf_changing(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, vibrate_vconf_changed_cb);
}


void _long_buzz_chk_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _long_buzz_chk_changed_cb() is called!!");

	int is_long_buzz = 0;
	vconf_get_bool(VCONFKEY_SETAPPL_NOTI_VIBRATION_LONG_BUZZ, &is_long_buzz);
	is_long_buzz =	!(is_long_buzz);
	vconf_set_bool(VCONFKEY_SETAPPL_NOTI_VIBRATION_LONG_BUZZ, is_long_buzz);

}

Evas_Object *_gl_vibration_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	Sound_Item_Data *id = data;
	int is_long_buzz = 0;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);

		if (vconf_get_bool(VCONFKEY_SETAPPL_NOTI_VIBRATION_LONG_BUZZ, &is_long_buzz) < 0) {
			is_long_buzz = 0;
		}

		elm_check_state_set(check, (is_long_buzz) ? EINA_TRUE : EINA_FALSE);	  /*default */
		evas_object_smart_callback_add(check, "changed", _long_buzz_chk_changed_cb, (void *)1);
		elm_object_style_set(check, "on&off");
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_FALSE);

		id->check = check;

	}

	return check;
}

static int get_vibration_level()
{
	int mode = 1;
	int level = 0;

	vconf_get_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, &level);

	switch (level) {
	case VIBRATION_LEVEL_NONE_INT:
		mode = VIBRATION_LEVEL_NONE;
		break;
	case VIBRATION_LEVEL_LOW_INT:
		mode = VIBRATION_LEVEL_LOW;
		break;
	/*	case VIBRATION_LEVEL_MID_INT : */
	/*		mode = VIBRATION_LEVEL_MID; */
	/*		break; */
	case VIBRATION_LEVEL_HIGH_INT:
		mode = VIBRATION_LEVEL_HIGH;
		break;
	}
	return mode;
}


void _intensity_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_intensity_list_cb(data, obj, event_info);
}

void _long_buzz_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

}

char *_gl_vibration_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(vibration_menu_its[index % VIB_TOP_MENU_SIZE].name));
	} else if (!strcmp(part, "elm.text.sub")) {
		switch (index) {
		case 0:
			vibrate_type = get_vibration_level();
			snprintf(buf, sizeof(buf) - 1, "%s", _(vibration_power_str[vibrate_type % 3]));
			break;
		}
		index++;
	}
	return strdup(buf);
}


static void _sound_gl_del(void *data, Evas_Object *obj)
{
	Sound_Item_Data *id = data;
	if (id)
		free(id);
}


Evas_Object *_create_vibration_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_sound_list - appdata is null");
		return NULL;
	}

	temp_ad = ad;

	Evas_Object *genlist  = NULL;
	struct _sound_menu_item *menu_its = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc_tmp;


	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text";
	itc->func.text_get = _gl_vibration_title_get;
	itc->func.del = _sound_gl_del;


	Elm_Genlist_Item_Class *itc_long_buzz = elm_genlist_item_class_new();
	itc_long_buzz->item_style = "1text.1icon.1";
	itc_long_buzz->func.text_get = _gl_vibration_title_get;
	itc_long_buzz->func.content_get = _gl_vibration_check_get;
	itc_long_buzz->func.del = _sound_gl_del;


	genlist = elm_genlist_add(ad->nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	Evas_Object *circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);
	eext_circle_object_genlist_scroller_policy_set(circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(circle_genlist, EINA_TRUE);

	Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
	title_item ->func.text_get = _gl_menu_title_text_get;
	title_item->item_style = "title";
	title_item->func.del = NULL;

	elm_genlist_item_append(genlist, title_item, (void*)VIBRATION_TITLE_VIBRATION, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_item);

	menu_its = vibration_menu_its;

	for (idx = 0; idx < VIB_TOP_MENU_SIZE; idx++) {
		if (idx == 0)
			itc_tmp = itc;
		else
			itc_tmp = itc_long_buzz;

		Sound_Item_Data *id = calloc(sizeof(Sound_Item_Data), 1);
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
				g_vib_item = id->item;
			}
		}
	}
	elm_genlist_item_class_free(itc_long_buzz);
	elm_genlist_item_class_free(itc);

	Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
	padding->item_style = "padding";
	padding->func.del = _sound_gl_del;

	elm_genlist_item_append(genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(padding);
	g_vibration_genlist = genlist;

	register_vconf_changing(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, vibrate_vconf_changed_cb, NULL);

	return genlist;
}



static void vibrate_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - vibrate_vconf_changed_cb() is called!!");


}

static char *_gl_vibration_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *item = data;
	if (item == NULL)
		return NULL;

	return strdup(_(vibration_power_str[item->index % 2]));
}

static Evas_Object *_gl_vibration_radio_get(void *data, Evas_Object *obj, const char *part)
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
		evas_object_smart_callback_add(radio, "changed", _vibration_gl_cb, (void *)id->index);
		evas_object_propagate_events_set(radio, EINA_FALSE);

		if (vibrate_type == id->index) {
			elm_radio_value_set(radio_main, vibrate_type);
		}
		index++;
	}
	return radio;
}

static void _vibration_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	vconf_get_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, &origin_vibration_level);

	vibrate_type = (int)data;

	int level = VIBRATION_LEVEL_LOW_INT;
	int feedback_rate = SETTING_VIB_MEDIUM_RATE;
	switch (vibrate_type) {
	case VIBRATION_LEVEL_NONE:
		level = VIBRATION_LEVEL_NONE_INT;
		feedback_rate = 0;
		break;
	case VIBRATION_LEVEL_LOW:
		level = VIBRATION_LEVEL_LOW_INT;
		feedback_rate = SETTING_VIB_WEAK_RATE;
		break;
	case VIBRATION_LEVEL_HIGH:
		level = VIBRATION_LEVEL_HIGH_INT;
		feedback_rate = SETTING_VIB_STRONG_RATE;
		break;
	}
	vconf_set_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, level);
	vconf_set_int(VCONFKEY_SETAPPL_TOUCH_FEEDBACK_VIBRATION_LEVEL_INT, level);

	DBG("Setting - feedback level: %d, rate: %d", level, feedback_rate);

	_start_vibration(level, feedback_rate, SETTING_DEFAULT_NOTIFICATION_GENERAL_PREVIEW_VIB);
	elm_genlist_realized_items_update(g_vibration_type_genlist);


	_haptic_close();

	DBG("Setting - vibration level : %d", level);

	if (g_vibration_type_genlist != NULL) {
		evas_object_del(g_vibration_type_genlist);
		g_vibration_type_genlist = NULL;
	}

	if (temp_ad->vibration_rdg != NULL) {
		evas_object_del(temp_ad->vibration_rdg);
		temp_ad->vibration_rdg = NULL;
	}

	back_key_generic_cb(temp_ad, obj, event_info);

	if (g_vib_item != NULL) {
		elm_genlist_item_update(g_vib_item);
	}

}

static void _vibration_layout_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _vibration_layout_del_cb() is called!");

	_haptic_close();

	if (g_vib_item) {
		elm_genlist_item_update(g_vib_item);
	}
}

static int get_vibration_type()
{
	int type = 1;

	vconf_get_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, &origin_vibration_level);
	switch (origin_vibration_level) {
	case VIBRATION_LEVEL_NONE_INT:
		type = 2;
		break;
	case VIBRATION_LEVEL_LOW_INT:
		type = 1;
		break;
	/*	case VIBRATION_LEVEL_MID_INT: */
	/*		type = 1; */
	/*		break; */
	case VIBRATION_LEVEL_HIGH_INT:
		type = 0;
		break;
	}

	return type;
}

void _show_intensity_list_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *nf_it = NULL;
	Evas_Object *popup;
	unsigned int index;
	appdata *ad = data;

	vibrate_type = get_vibration_type();

	popup = elm_layout_add(ad->win_main);
	/*elm_layout_file_set(popup, EDJE_PATH, "setting/genlist/2button-layout"); */
	elm_layout_file_set(popup, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(popup, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _vibration_layout_del_cb, ad);

	Elm_Genlist_Item_Class *itc = NULL;
	itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_vibration_text_get;
	itc->func.content_get = _gl_vibration_radio_get;
	itc->func.del = _sound_gl_del;

	Evas_Object *genlist;
	genlist = elm_genlist_add(popup);
	elm_object_style_set(genlist, "popup");
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);

	Evas_Object *circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);
	eext_circle_object_genlist_scroller_policy_set(circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(circle_genlist, EINA_TRUE);


	Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
	title_item ->func.text_get = _gl_menu_title_text_get;
	title_item->item_style = "title";
	title_item->func.del = NULL;

	elm_genlist_item_append(genlist, title_item, (void*)VIBRATION_TITLE_INTENSITY, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_item);

	int count = sizeof(vibration_power_str) / sizeof(vibration_power_str[0]);

	for (index = 0; index < count; index++) {
		Item_Data *item = (Item_Data *)calloc(sizeof(Item_Data), 1);
		if (item) {
			item->index = index;
			item->item = elm_genlist_item_append(genlist,
												 itc,
												 item,
												 NULL,
												 ELM_GENLIST_ITEM_NONE,
												 _vibration_gl_cb,
												 (void *)index);
		}
	}

	ad->vibration_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->vibration_rdg, 3);
	elm_radio_value_set(ad->vibration_rdg, vibrate_type);

	evas_object_data_set(genlist, "radio_main", ad->vibration_rdg);

	Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
	padding->item_style = "padding";
	padding->func.del = _sound_gl_del;

	elm_genlist_item_append(genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(padding);
	g_vibration_type_genlist = genlist;

	elm_object_part_content_set(popup, "elm.genlist", genlist);
	evas_object_show(genlist);
	elm_genlist_item_class_free(itc);

	/*elm_naviframe_item_push(ad->nf, _("IDS_ST_HEADER_VIBRATION_ABB"), NULL, NULL, popup, NULL); */
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, popup, NULL);
	back_button_cb_push(&back_key_generic_cb, data, NULL, g_vibration_genlist, "g_vibration_genlist");
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}





