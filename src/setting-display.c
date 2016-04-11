/*
 * Copyright (c) 2013-2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of
 * SAMSUNG ELECTRONICS ("Confidential Information").
 * You shall not disclose such Confidential Information and shall
 * use it only in accordance with the terms of the license agreement
 * you entered into with SAMSUNG ELECTRONICS.
 * SAMSUNG make no representations or warranties about the suitability
 * of the software, either express or implied, including but not
 * limited to the implied warranties of merchantability, fitness for
 * a particular purpose, or non-infringement.
 * SAMSUNG shall not be liable for any damages suffered by licensee as
 * a result of using, modifying or distributing this software or its derivatives.
 */

/*
 * setting-display.c
 *
 *  Created on: Oct 9, 2013
 *      Author: min-hoyun
 */

#include <device.h>
#include <system_settings.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <fontconfig/fontconfig.h>
#include <glib.h>

#include "setting_data_vconf.h"
#include "setting-display.h"
#include "setting_view_toast.h"
#include "setting-language.h"
#include "setting-homescreen.h"
#include "setting-motion.h"
#include "util.h"

static struct _display_menu_item display_menu_its[] = {
	{ "IDS_ST_MBODY_SCREEN_TIMEOUT_ABB",	SETTING_DISPLAY_SCREEN_TIME,	_display_gl_screen_timeout_cb	},
	{ "IDS_ST_BODY_FONT",					SETTING_DISPLAY_FONT,	_display_gl_font_cb		},
	{ "IDS_ST_BUTTON_LANGUAGE",				SETTING_DISPLAY_LANG,	_display_gl_language_cb	},
#if !defined(FEATURE_SETTING_SDK) && !defined(FEATURE_SETTING_EMUL)
	{ "IDS_ST_MBODY_MANAGE_APPS_ABB",	SETTING_DISPLAY_EDIT_APPS,	_homescreen_gl_edit_apps_cb	},
#endif
};

static struct _font_menu_item font_menu_its[] = {
	{ "IDS_ST_BODY_FONT_STYLE",			SETTING_DISPLAY_FONT_STYLE, _display_gl_font_style_cb },
	{ "IDS_ST_BODY_FONT_SIZE_ABB",		SETTING_DISPLAY_FONT_SIZE,	_display_gl_font_size_cb },
};

static int screen_time_str[] = {
	10, 15, 30, 1, 5
};
static int timeout_arr[] = {
	10, 15, 30, 60, 300
};

static char *font_size_str[] = {
	"IDS_EMAIL_OPT_SMALL_M_FONT_SIZE",
	"IDS_ST_OPT_MEDIUM_M_FONT_SIZE",
	"IDS_ST_BODY_LARGE_M_FONT_SIZE_ABB2"
};

static char *rotate_screen_str[] = {
	"IDS_COM_BODY_DEFAULT", "IDS_COM_OPT_ROTATE_CW", "IDS_COM_OPT_ROTATE_CCW", "IDS_ST_SBODY_180_DEGREE"
};

static appdata *temp_ad = NULL;
static Evas_Object *g_display_genlist = NULL;
static Evas_Object *g_screen_time_genlist = NULL;
static Evas_Object *g_font_size_genlist = NULL;
static Evas_Object *g_font_style_genlist = NULL;
static Evas_Object *g_rotate_screen_genlist = NULL;

static int screen_time_index = 1;		/* default: 10 seconds */
static int font_size_index   = 1;		/* default: normal */
static int rotate_screen_rot  = 0;		/* default: 0(0degree), vconf enum */
static int rotate_screen_index  = 0;		/* default: 0, list index */

static int touch_mode = NONE;

/* Main display list item */
static Elm_Object_Item *lang_item = NULL;
static Elm_Object_Item *wake_up_item = NULL;
static Elm_Object_Item *edit_icon_item = NULL;
static Elm_Object_Item *screen_time_item = NULL;

/* Font list item */
static char *font_name = NULL;

static Elm_Object_Item *font_style_item = NULL;
static Elm_Object_Item *font_size_item = NULL;

static void _font_size_gl_cb(void *data, Evas_Object *obj, void *event_info);
static void _font_style_gl_cb(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool setting_font_style_pop_cb(void *data, Elm_Object_Item *it);
static void settings_font_style_changed_cb(system_settings_key_e key, void *user_data);
static void change_language_enabling(keynode_t *key, void *data);
static void change_screen_time_cb(keynode_t *key, void *data);
static void change_language_cb(keynode_t *key, void *data);
static Eina_Bool setting_font_list_pop_cb(void *data, Elm_Object_Item *it);
static void _lang_update_font_style_list(void *data, Evas_Object *obj, void *event_info);
static void _set_rotate_screen(const int rotation);
static int _get_rotate_screen();

void _init_display()
{
	register_vconf_changing(VCONFKEY_WMS_WMANAGER_CONNECTED, change_language_enabling, NULL);
	register_vconf_changing(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, change_screen_time_cb, NULL);
	register_vconf_changing(VCONFKEY_LANGSET, change_language_cb, NULL);

	/*_init_screen_rotate(); */
}

void _init_screen_rotate()
{
	int rotate;
	rotate = _get_rotate_screen();
	rotate_screen_rot = rotate;

	if (rotate == SETTING_SCREENROTATION_90_DEGREE) {
		/*90R */
		rotate_screen_index = 1;
	} else if (rotate == SETTING_SCREENROTATION_270_DEGREE) {
		/*90L */
		rotate_screen_index = 2;
	} else if (rotate == SETTING_SCREENROTATION_180_DEGREE) {
		/*180 */
		rotate_screen_index = 3;
	}

	if (rotate == -1) {
		rotate_screen_rot = SETTING_SCREENROTATION_0_DEGREE;
	}
}

void _update_menu_text_when_lang_changed()
{
	DBG("Setting - Language is changed...update display list");

	if (g_display_genlist) {
		elm_genlist_realized_items_update(g_display_genlist);
	}

	if (temp_ad && temp_ad->main_genlist) {
		elm_genlist_realized_items_update(temp_ad->main_genlist);
	}
}

void _clear_display_cb(void *data, Evas *e, Elm_Object_Item *it, void *event_info)
{
	temp_ad = NULL;
	g_screen_time_genlist = NULL;
	g_font_size_genlist = NULL;
	g_font_style_genlist = NULL;
	g_rotate_screen_genlist = NULL;

	touch_mode = NONE;

	unregister_vconf_changing(VCONFKEY_WMS_WMANAGER_CONNECTED, change_language_enabling);
	unregister_vconf_changing(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, change_screen_time_cb);
	unregister_vconf_changing(VCONFKEY_LANGSET, change_language_cb);

	return;
}

void _display_gl_font_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (data != NULL) {
		_show_font_list(data);
	} else {
		DBG("ad->font_name is NULL !!!!!!");
	}
}

void _display_gl_font_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("_display_gl_font_style_cb");

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (data != NULL) {
		_show_font_style_list(data);
	}
}

void _display_gl_font_size_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (data != NULL) {
		_show_font_size_list(data);
	}
}

void _display_gl_rotate_screen_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (data != NULL) {
		_show_rotate_screen_list(data);
	}
}

void _display_gl_screen_timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (data != NULL) {
		_show_screen_timeout_list(data);
	}
}

void _display_gl_language_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("Setting - ad is null");
		return;
	}

	if (ad->MENU_TYPE == SETTING_LANGUAGE) {
		DBG("Already language screen enter:clear");
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

	ad->MENU_TYPE = SETTING_LANGUAGE;

	_initialize_language(ad);
	_set_launguage_update_cb(_update_menu_text_when_lang_changed);

	genlist = _create_lang_list(data);
	if (genlist == NULL) {
		DBG("%s", "language cb - genlist is null");
		return;
	}

	back_button_cb_push(&back_key_generic_cb, data, NULL);
	nf_it = elm_naviframe_item_push(ad->nf, "IDS_ST_BUTTON_LANGUAGE", NULL, NULL, genlist, NULL);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_DEL, _clear_lang_cb, ad);
#if !defined(FEATURE_SETTING_TELEPHONY)
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
#endif
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);

	ad->MENU_TYPE = SETTING_LANGUAGE;
}

char *_gl_display_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Display_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text")) {
		if (id->item == lang_item) {
			if (is_connected_GM()) {
				snprintf(buf, sizeof(buf) - 1, "<font color=#515151>%s</font>",
				         _(display_menu_its[index].name));
			} else {
				snprintf(buf, sizeof(buf) - 1, "%s",
				         _(display_menu_its[index].name));
			}
			DBG("buf --->%s", buf);
		} else {
			snprintf(buf, sizeof(buf) - 1, "%s", _(display_menu_its[index].name));
		}
	} else if (!strcmp(part, "elm.text.1")) {
		if (id->item == edit_icon_item) {
			char *str = _get_homeview_type_subtitle();
			snprintf(buf, sizeof(buf) - 1, "%s", str);
			FREE(str);
		} else if (id->item == lang_item) {
			const char *curr_lang = setting_get_lang_title();
			if (curr_lang) {
				if (is_connected_GM()) {
					snprintf(buf, sizeof(buf) - 1, "<font color=#515151>%s</font>", curr_lang);
				} else {
					snprintf(buf, sizeof(buf) - 1, "%s", curr_lang);
				}
			}
			FREE(curr_lang);
		} else if (id->item == screen_time_item) {
			int time = 10;
			vconf_get_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, &time);
			switch (time) {
				case 10:
					snprintf(buf, sizeof(buf) - 1, _("IDS_ST_BODY_10SEC"), screen_time_str[0]);
					break;
				case 15:
					snprintf(buf, sizeof(buf) - 1, _("IDS_ST_BODY_15SEC"), screen_time_str[1]);
					break;
				case 30:
					snprintf(buf, sizeof(buf) - 1, _("IDS_ST_BODY_30SEC"), screen_time_str[2]);
					break;
				case 60:
					snprintf(buf, sizeof(buf) - 1, _("IDS_ST_BODY_1_MINUTE_ABB2"), screen_time_str[3]);
					break;
				case 300:
					snprintf(buf, sizeof(buf) - 1, _("IDS_ST_BODY_5_MINUTES"), screen_time_str[4]);
					break;
			}
		} else {
			/*snprintf(buf, sizeof(buf) - 1, "%s", _get_wake_up_gesture_sub_title()); */
			snprintf(buf, sizeof(buf)-1, "%s", "Motion is unsupported now.");
		}
	}
	return strdup(buf);
}

void _display_gl_del(void *data, Evas_Object *obj)
{
	Display_Item_Data *id = data;
	if (id)
		free(id);
}

Evas_Object *_create_display_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_display_list - appdata is null");
		return NULL;
	}
	Evas_Object *genlist  = NULL;
	struct _display_menu_item *menu_its = NULL;
	int idx = 0;

	temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text";
	itc->func.text_get = _gl_display_title_get;
	itc->func.del = _display_gl_del;

	Elm_Genlist_Item_Class *itc2 = elm_genlist_item_class_new();
	itc2->item_style = "2text";
	itc2->func.text_get = _gl_display_title_get;
	itc2->func.del = _display_gl_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist,ad);

	menu_its = display_menu_its;

	int size = sizeof(display_menu_its) / sizeof(struct _display_menu_item);

	for (idx = 0; idx < size; idx++) {
		Elm_Genlist_Item_Class *itc_tmp = NULL;

		if (menu_its[idx].type == SETTING_DISPLAY_GESTURE
		    || menu_its[idx].type == SETTING_DISPLAY_ICON_SIZE
		    || menu_its[idx].type == SETTING_DISPLAY_LANG
		    || menu_its[idx].type == SETTING_DISPLAY_SCREEN_TIME) {
			itc_tmp = itc2;
		} else {
			itc_tmp = itc;
		}

		Display_Item_Data *id = calloc(sizeof(Display_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
					genlist,		/* genlist object */
					itc_tmp,		/* item class */
					id,			/* data */
					NULL,
					ELM_GENLIST_ITEM_NONE,
					menu_its[ idx ].func,	/* call back */
					ad);

			if (menu_its[idx].type == SETTING_DISPLAY_LANG) {
				lang_item = id->item;
			} else if (menu_its[idx].type == SETTING_DISPLAY_GESTURE) {
				DBG("wakeup item@!!!");
				wake_up_item = id->item;
			} else if (menu_its[idx].type == SETTING_DISPLAY_ICON_SIZE) {
				DBG("edit icon item@!!!");
				edit_icon_item = id->item;
			} else if (menu_its[idx].type == SETTING_DISPLAY_SCREEN_TIME) {
				DBG("screen time item@!!!");
				screen_time_item = id->item;
			}
		}
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc2);

	g_display_genlist = genlist;

	return genlist;
}

static char *_gl_screen_timeout_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;

	if (!strcmp(part, "elm.text")) {
		if (id->index == 0) {
			snprintf(buf, sizeof(buf) - 1, _("IDS_ST_BODY_10SEC"), screen_time_str[id->index]);
		} else if (id->index == 1) {
			snprintf(buf, sizeof(buf) - 1, _("IDS_ST_BODY_15SEC"), screen_time_str[id->index]);
		} else if (id->index == 2) {
			snprintf(buf, sizeof(buf) - 1, _("IDS_ST_BODY_30SEC"), screen_time_str[id->index]);
		} else if (id->index == 3) {
			snprintf(buf, sizeof(buf) - 1, _("IDS_ST_BODY_1_MINUTE_ABB2"), screen_time_str[id->index]);
		} else {
			snprintf(buf, sizeof(buf) - 1, _("IDS_ST_BODY_5_MINUTES"), screen_time_str[id->index]);
		}
	}
	return strdup(buf);
}

static int _get_timeout_index(int seconds)
{
	int index;
	int length = 0;
	length = sizeof(timeout_arr) / sizeof(timeout_arr[0]);

	DBG("Setting - timeout_arr's length: %d", length);

	for (index = 0; index < length; index++) {
		if (timeout_arr[index] == seconds) {
			DBG("Setting - timeout index : %d, sec : %d", index, timeout_arr[index]);
			break;
		}
	}

	return index;
}

static void _screen_timeout_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	screen_time_index = (int)data;

	vconf_set_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL , timeout_arr[screen_time_index]);

	elm_genlist_realized_items_update(g_screen_time_genlist);

	elm_naviframe_item_pop(temp_ad->nf);
	if (!temp_ad->screen_timeout_rdg) {
		evas_object_del(temp_ad->screen_timeout_rdg);
		temp_ad->screen_timeout_rdg = NULL;
	}
}

static Evas_Object *_gl_screen_timeout_ridio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;
	static int timeout = -1;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);
		evas_object_smart_callback_add(radio, "changed", _screen_timeout_gl_cb, (void *)id->index);
		evas_object_propagate_events_set(radio, EINA_FALSE);

		if (timeout == -1) {
			vconf_get_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, &timeout);
			screen_time_index = _get_timeout_index(timeout);
		}

		if (screen_time_index == id->index) {
			elm_radio_value_set(radio_main, screen_time_index);
		}
	}
	return radio;
}

static void _screen_timeout_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}

void _show_screen_timeout_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_screen_timeout_list - appdata is null");
		return;
	}
	Evas_Object *genlist  = NULL;
	Elm_Object_Item *nf_it = NULL;
	int idx;

	temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_screen_timeout_title_get;
	itc->func.content_get = _gl_screen_timeout_ridio_get;
	itc->func.del = _screen_timeout_gl_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	connect_to_wheel_with_genlist(genlist,ad);

	int timeout = 0;
	vconf_get_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, &timeout);
	screen_time_index = _get_timeout_index(timeout);

	Elm_Object_Item *curr_item = NULL;

	for (idx = 0; idx < SCREEN_TIME_COUNT; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _screen_timeout_gl_cb, (void *)idx);

			if (idx == screen_time_index) {
				curr_item = id->item;
			}
		}
	}

	ad->screen_timeout_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->screen_timeout_rdg, 6);
	elm_radio_value_set(ad->screen_timeout_rdg, screen_time_index);

	evas_object_data_set(genlist, "radio_main", ad->screen_timeout_rdg);

	if (curr_item) {
		elm_genlist_item_show(curr_item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
	}

	g_screen_time_genlist = genlist;

	elm_genlist_item_class_free(itc);

	back_button_cb_push(&back_key_generic_cb, data, NULL);
	nf_it = elm_naviframe_item_push(ad->nf, "IDS_ST_MBODY_SCREEN_TIMEOUT_ABB", NULL, NULL, genlist, NULL);
#if !defined(FEATURE_SETTING_TELEPHONY)
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
#endif
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
}

static char *_gl_font_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0};
	Item_Data *id = data;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(font_menu_its[id->index].name));
	} else if (!strcmp(part, "elm.text.1")) {
		if (id->index == SETTING_DISPLAY_FONT_STYLE) {
			char *font_name = NULL;
			if (system_settings_get_value_string(SYSTEM_SETTINGS_KEY_FONT_TYPE, &font_name)
			    != SYSTEM_SETTINGS_ERROR_NONE) {
				ERR("failed to call system_setting_get_value_string with err");
			}

			if (font_name) {

				DBG(" font_name <---------------------- (%s) ", font_name);

				if (strstr(font_name, "Samsung")) {
					snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_DEFAULT_FONT"));
				} else {
					if (!strcmp(font_name, "Choco cooky")) {
						snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_LCKSCN_BODY_CHOCO_COOKY_M_FONT"));
					} else if (!strcmp(font_name, "Cool jazz")) {
						snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_COOL_JAZZ"));
					} else if (!strcmp(font_name, "Rosemary")) {
						snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_FONTSTYLE_ROSEMARY"));
					} else if (!strcmp(font_name, "Tinkerbell")) {
						snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_MBODY_TINKERBELL"));
					} else if (!strcmp(font_name, "Applemint")) {
						snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_RH_BUTTON2_APPLEMINT_M_FONT"));
					} else if (!strcmp(font_name, "Kaiti")) {
						snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_KAITI_M_FONT"));
					} else if (!strcmp(font_name, "POP")) {
						snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_POP_JPN_DCM"));
					} else if (!strcmp(font_name, "UDMincho")) {
						snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_UDMINCHO_JPN"));
					} else if (!strcmp(font_name, "UDRGothic")) {
						snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_UDRGOTHICM_JPN"));
					} else if (!strcmp(font_name, "TizenSans")) {
						snprintf(buf, sizeof(buf) - 1, "%s", _("TizenSans"));
					} else {
						snprintf(buf, sizeof(buf) - 1, "%s", font_name);
					}
				}
			} else {
				snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_UNKNOWN"));
			}
		} else if (id->index == SETTING_DISPLAY_FONT_SIZE) {
			int font_size = -1;
			if (system_settings_get_value_int(SYSTEM_SETTINGS_KEY_FONT_SIZE, &font_size)
			    != SYSTEM_SETTINGS_ERROR_NONE) {
				DBG("Setting - system_settings_get_value_int() is failed.");
			}

			if (font_size < 0 || font_size > 2) {
				snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_UNKNOWN"));
			} else {
				snprintf(buf, sizeof(buf) - 1, "%s", _(font_size_str[font_size]));
			}
		}
	}

	return strdup(buf);
}

static char *_gl_font_style_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	Font_Style_Item_Data *id = data;

	char *pos = id->font_name;
	char new_name[256];
	int i = 0;
	int count = 0;
	while (pos && *pos != '\0') {
		if (*pos == ' ') {
			new_name[count] = '\\';
			count++;
			new_name[count] = ' ';
		} else {
			new_name[count] = id->font_name[i];
		}

		count++;
		pos++;
		i++;
	}
	new_name[count] = '\0';

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "<font=%s>%s</font>", new_name, id->font_name);
	}

	DBG("font  = %s", buf);
	return strdup(buf);
}

static Evas_Object *_gl_font_style_ridio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Font_Style_Item_Data *id = data;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);

		evas_object_smart_callback_add(radio, "clicked", _font_style_gl_cb, (void *)id);
		evas_object_propagate_events_set(radio, EINA_FALSE);
	}

	return radio;
}

static void _font_style_gl_del(void *data, Evas_Object *obj)
{
	Font_Style_Item_Data *id = data;
	if (id) {
		FREE(id->font_name);
		free(id);
	}
}

static char *_gl_font_size_title_get(void *data, Evas_Object *obj, const char *part)
{
	int old_font_size = -1;
	if (system_settings_get_value_int(SYSTEM_SETTINGS_KEY_FONT_SIZE, &old_font_size)
	    != SYSTEM_SETTINGS_ERROR_NONE) {
		DBG("Setting - system_settings_get_value_int() is failed.");
	}
	char buf[1024] = {0,};
	Item_Data *id = data;

	if (!strcmp(part, "elm.text")) {
		if (id->index == 0) {
			snprintf(buf, sizeof(buf) - 1, "<font_size=30>%s</font_size>", _(font_size_str[id->index]));
		} else if (id->index == 1) {
			snprintf(buf, sizeof(buf) - 1, "<font_size=38>%s</font_size>", _(font_size_str[id->index]));
		} else {
			snprintf(buf, sizeof(buf) - 1, "<font_size=46>%s</font_size>", _(font_size_str[id->index]));
		}
	}
	return strdup(buf);
}

static Evas_Object *_gl_font_size_ridio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;

	int old_font_size = -1;
	if (system_settings_get_value_int(SYSTEM_SETTINGS_KEY_FONT_SIZE, &old_font_size)
	    != SYSTEM_SETTINGS_ERROR_NONE) {
		DBG("Setting - system_settings_get_value_int() is failed.");
	}

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);
		evas_object_smart_callback_add(radio, "changed", _font_size_gl_cb, (void *)id->index);
		evas_object_propagate_events_set(radio, EINA_FALSE);

		if (old_font_size == id->index) {
			elm_radio_value_set(radio_main, old_font_size);
		}
	}
	return radio;
}

static void _font_size_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}

static Ecore_Timer *font_timer = NULL;

static Eina_Bool _update_font_style(void *data)
{
	/* change font style */
	if (font_name) {
		system_settings_set_value_string(SYSTEM_SETTINGS_KEY_FONT_TYPE, font_name);
		FREE(font_name);
	} else {
		ERR("font_name is null");
	}
	font_timer = NULL;

	return ECORE_CALLBACK_CANCEL;
}
#if 0
static void _font_style_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL) {
		ERR("appdata is null");
		return;
	}

	elm_naviframe_item_pop(ad->nf);
	if (!ad->font_style_rdg) {
		evas_object_del(ad->font_style_rdg);
		ad->font_style_rdg = NULL;
	}

	FREE(font_name);
}

static void _font_style_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL) {
		ERR("appdata is null");
		return;
	}

	elm_naviframe_item_pop(ad->nf);

	if (!ad->font_style_rdg) {
		evas_object_del(ad->font_style_rdg);
		ad->font_style_rdg = NULL;
	}

	if (font_timer) {
		ecore_timer_del(font_timer);
		font_timer = NULL;
	}
	font_timer = ecore_timer_add(0.3, (Ecore_Task_Cb) _update_font_style, NULL);
}
#endif

static void _font_style_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
	Font_Style_Item_Data *id = (Font_Style_Item_Data *)data;

	/*elm_radio_value_set(temp_ad->font_style_rdg, id->index); */


	if (id->font_name) {
		FREE(font_name);
		font_name = strdup(id->font_name);
	}

	/*elm_genlist_realized_items_update(g_font_style_genlist); */
	elm_naviframe_item_pop(temp_ad->nf);

	if (font_style_item) {
		elm_genlist_item_update(font_style_item);
	}

	if (!temp_ad->font_style_rdg) {
		evas_object_del(temp_ad->font_style_rdg);
		temp_ad->font_style_rdg = NULL;
	}

	if (font_timer) {
		ecore_timer_del(font_timer);
		font_timer = NULL;
	}
	font_timer = ecore_timer_add(0.3, (Ecore_Task_Cb) _update_font_style, NULL);
}

static Eina_Bool _update_font_size(void *data)
{
	/* change font size */
	system_settings_set_value_int(SYSTEM_SETTINGS_KEY_FONT_SIZE, font_size_index);

	font_timer = NULL;

	if (font_size_item) {
		elm_genlist_item_update(font_size_item);
	}

	return ECORE_CALLBACK_CANCEL;
}

static void _font_size_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	int old_font_size = 0;

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (system_settings_get_value_int(SYSTEM_SETTINGS_KEY_FONT_SIZE, &old_font_size)
	    != SYSTEM_SETTINGS_ERROR_NONE) {
		DBG("Setting - system_settings_get_value_int() is failed.");
		return;
	}
	DBG("Setting - old font size is %d.", old_font_size);

	font_size_index = (int)data;

	DBG("Setting - Curr font size is %d.", font_size_index);

	/*elm_genlist_realized_items_update(g_font_size_genlist); */

	elm_naviframe_item_pop(temp_ad->nf);

	if (!temp_ad->font_size_rdg) {
		evas_object_del(temp_ad->font_size_rdg);
		temp_ad->font_size_rdg = NULL;
	}

	if (font_timer) {
		ecore_timer_del(font_timer);
		font_timer = NULL;
	}

	if (old_font_size != font_size_index) {
		DBG("Setting - font size is same with old.");
		font_timer = ecore_timer_add(0.3, (Ecore_Task_Cb) _update_font_size, NULL);
	}
}

void _show_font_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_font_list - appdata is null");
		return;
	}

	Evas_Object *genlist  = NULL;
	Elm_Object_Item *nf_it = NULL;
	int idx;

	temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text";
	itc->func.text_get = _gl_font_title_get;
	itc->func.del = _font_size_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	connect_to_wheel_with_genlist(genlist,ad);

	for (idx = 0; idx < 1; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist, itc, id, NULL,
					ELM_GENLIST_ITEM_NONE, _display_gl_font_style_cb, ad);

			if (idx == 0) {
				font_style_item = id->item;
			}
		}
	}

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	elm_genlist_item_class_free(itc);

	back_button_cb_push(&back_key_generic_cb, data, NULL);
	nf_it = elm_naviframe_item_push(ad->nf, "IDS_ST_BODY_FONT", NULL, NULL, layout, NULL);
#if !defined(FEATURE_SETTING_TELEPHONY)
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
#endif
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(nf_it, setting_font_list_pop_cb, ad);

	if (system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_FONT_TYPE, settings_font_style_changed_cb, ad) != 0) {
		ERR("system_settings_set_changed_cb failed!!");
	}
}

char *_get_default_font()
{
	xmlDocPtr doc = NULL;
	xmlNodePtr cur = NULL;
	xmlNodePtr cur2 = NULL;
	xmlNodePtr cur3 = NULL;
	xmlChar *key = NULL;
	struct _xmlAttr *properties = NULL;
	char *default_font_name = NULL;

	doc = xmlParseFile(TZ_SYS_DATA_D"/etc/fonts/conf.avail/99-slp.conf");

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		xmlFreeDoc(doc);
		doc = NULL;
		return NULL;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *)"fontconfig")) {
		xmlFreeDoc(doc);
		doc = NULL;
		return NULL;
	}

	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"match"))) {
			cur2 = cur->xmlChildrenNode;
			while (cur2 != NULL) {
				if ((!xmlStrcmp(cur2->name, (const xmlChar *)"edit"))) {
					properties = cur2->properties;
					/*find the "name" property*/
					while (NULL != properties) {
						if (!xmlStrcmp(properties->name, (const xmlChar *)"name")) {
							break;
						}
						properties = properties->next;
					}

					/*If the value of "name" property is "family", then get the child node string,
					  it shall be the default font type*/
					if (NULL != properties && !xmlStrcmp(properties->children->content, (const xmlChar *)"family")) {
						cur3 = cur2->xmlChildrenNode;
						while (cur3 != NULL) {
							if ((!xmlStrcmp(cur3->name, (const xmlChar *)"string"))) {
								key = xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								default_font_name = (char *)g_strdup((char *)key);
								xmlFree(key);
								key = NULL;
								xmlFreeDoc(doc);
								doc = NULL;
								return default_font_name;
							}
							cur3 = cur3->next;
						}
					}
				}
				cur2 = cur2->next;
			}
		}
		cur = cur->next;
	}

	xmlFreeDoc(doc);
	doc = NULL;
	return NULL;
}

static Eina_List *_get_available_font_list()
{
	FcObjectSet *os = NULL;
	FcFontSet *fs = NULL;
	FcPattern *pat = NULL;
	Eina_List *list = NULL;
	FcConfig *font_config = NULL;

	font_config = FcInitLoadConfigAndFonts();

	if (font_config == NULL) {
		ERR("Failed: FcInitLoadConfigAndFonts");
		return NULL;
	}

	char *locale = setlocale(0, NULL);

	pat = FcPatternCreate();

	os = FcObjectSetBuild(FC_FAMILY, FC_FILE, FC_FAMILYLANG, (char *) 0);

	if (os) {
		fs = FcFontList(font_config, pat, os);
		FcObjectSetDestroy(os);
		os = NULL;
	}

	if (pat) {
		FcPatternDestroy(pat);
		pat = NULL;
	}

	if (fs)	{
		int j;
		DBG("fs->nfont = %d", fs->nfont);

		for (j = 0; j < fs->nfont; j++) {
			FcChar8 *family = NULL;
			FcChar8 *file = NULL;
			FcChar8 *lang = NULL;
			int id = 0;

			if (FcPatternGetString(fs->fonts[j], FC_FILE, 0, &file) == FcResultMatch) {
				int preload_path_len = strlen(SETTING_FONT_PRELOAD_FONT_PATH);
				int download_path_len = strlen(SETTING_FONT_DOWNLOADED_FONT_PATH);

				char *family_result = NULL;

				if (FcPatternGetString(fs->fonts[j], FC_FAMILY, 0, &family) != FcResultMatch) {
					DBG("Family name is invalid");
					continue;
				}

				if (FcPatternGetString(fs->fonts[j], FC_FAMILYLANG, id, &lang) != FcResultMatch) {
					DBG("Family lang is invalid");
					continue;
				}

				if (!strncmp((const char *)file, SETTING_FONT_PRELOAD_FONT_PATH, preload_path_len)) {
					/* Find proper family name for current locale. */
					while (locale && family && lang) {
						/*DBG("locale: %s, family: %s, lang: %s", locale, family, lang); */

						if (!strncmp(locale, (char *)lang, strlen((char *)lang))) {
							family_result = (char *)family;
							break;
						}

						/* I will set english as default family language. */
						/* If there is no proper family language for current locale, */
						/* we have to show the english family name. */
						if (!strcmp((const char *)lang, "en")) {
							family_result = (char *)family;
						}
						id++;
						if (FcPatternGetString(fs->fonts[j], FC_FAMILY, id, &family) != FcResultMatch) {
							break;
						}
						if (FcPatternGetString(fs->fonts[j], FC_FAMILYLANG, id, &lang) != FcResultMatch) {
							break;
						}
					}

					if (eina_list_data_find(list, family_result) == NULL) {
						list = eina_list_append(list, strdup(family_result));
						DBG("-------- ADDED FONT - family result = %s", (char *)family_result);
					}
				}

				id = 0;

				/* always shown for D/L */
				if (!strncmp((const char *)file, SETTING_FONT_DOWNLOADED_FONT_PATH, download_path_len)) {
					/* Find proper family name for current locale. */
					while (locale && family && lang) {
						/*DBG("locale: %s, family: %s, lang: %s", locale, family, lang); */

						if (!strncmp(locale, (char *)lang, strlen((char *)lang))) {
							family_result = (char *)family;
							break;
						}

						/* I will set english as default family language. */
						/* If there is no proper family language for current locale, */
						/* we have to show the english family name. */
						if (!strcmp((const char *)lang, "en")) {
							family_result = (char *)family;
						}
						id++;
						if (FcPatternGetString(fs->fonts[j], FC_FAMILY, id, &family) != FcResultMatch) {
							break;
						}
						if (FcPatternGetString(fs->fonts[j], FC_FAMILYLANG, id, &lang) != FcResultMatch) {
							break;
						}
					}

					if (eina_list_data_find(list, family_result) == NULL) {
						list = eina_list_append(list, family_result);
						DBG("-------- ADDED FONT - family result = %s", (char *)family_result);
					}
				}
			}
		}
		FcFontSetDestroy(fs);
		fs = NULL;
	}

	FcConfigDestroy(font_config);
	font_config = NULL;
	return list;
}

int _show_font_style_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_font_style_list - appdata is null");
		return -1;
	}
	Evas_Object *genlist  = NULL;
	Elm_Object_Item *nf_it = NULL;
	char *default_font_name = NULL;
	char *tmp_name = NULL;
	int idx = 0, matched_idx = 0;
	int ret = 0;

	temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_font_style_title_get;
	itc->func.content_get = _gl_font_style_ridio_get;
	itc->func.del = _font_style_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	connect_to_wheel_with_genlist(genlist,ad);

	FREE(font_name);
	ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_FONT_TYPE, &tmp_name);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		ERR("failed to call system_setting_get_value_string with err %d", ret);
		tmp_name = _get_default_font();
		if (tmp_name == NULL) {
			ERR("failed to get default font name");
			return -1;
		} else {
			DBG("get_default_font = %s", tmp_name);
		}
	} else {
		DBG("SYSTEM_SETTINGS_KEY_FONT_TYPE = %s", tmp_name);
	}

	default_font_name = _get_default_font();

	if (default_font_name) {
		Font_Style_Item_Data *id_default = calloc(sizeof(Font_Style_Item_Data), 1);
		if (default_font_name && tmp_name && !strcmp(tmp_name, default_font_name)) {
			matched_idx = idx;
			font_name = strdup(tmp_name);
		}
		if (id_default) {
			id_default->index = idx++;
			id_default->font_name = strdup(default_font_name);
			id_default->item = elm_genlist_item_append(genlist, itc, id_default, NULL, ELM_GENLIST_ITEM_NONE,
					_font_style_gl_cb, (void *)id_default);
		}
	} else {
		ERR("default_font_name is NULL");
	}

	/* get font list */
	Eina_List *font_list = NULL;
	Eina_List *l = NULL;
	FcChar8 *font_data = NULL;
	font_list = _get_available_font_list();
	EINA_LIST_FOREACH(font_list, l, font_data) {
		if (!default_font_name || strcmp((const char *)default_font_name, (const char *)font_data)) {
			Font_Style_Item_Data *id = calloc(sizeof(Font_Style_Item_Data), 1);
			if (tmp_name && !strcmp((const char *)tmp_name, (const char *)font_data)) {
				matched_idx = idx;
				font_name = strdup(tmp_name);
			}

			DBG("font_data -------> %s", (const char *)font_data);

			if (id) {
				id->index = idx++;
				id->font_name = (char *)strdup((char *)font_data);
				id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE,
						_font_style_gl_cb, (void *)id);
			}
		}
	}

	evas_font_reinit();
	ad->font_style_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->font_style_rdg, -1);
	elm_radio_value_set(ad->font_style_rdg, matched_idx);

	evas_object_data_set(genlist, "radio_main", ad->font_style_rdg);

	g_font_style_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	elm_genlist_item_class_free(itc);

	evas_object_smart_callback_add(genlist, "language,changed", _lang_update_font_style_list, ad);

	back_button_cb_push(&back_key_generic_cb, data, NULL);
	nf_it = elm_naviframe_item_push(ad->nf, "IDS_ST_BODY_FONT_STYLE", NULL, NULL, layout, NULL);
#if !defined(FEATURE_SETTING_TELEPHONY)
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
#endif
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(nf_it, setting_font_style_pop_cb, ad);

	return 0;
}

static void _lang_update_font_style_list(void *data, Evas_Object *obj, void *event_info)
{
	DBG("_lang_update_font_style_list");

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_lang_update_font_style_list - appdata is null");
		return;
	}

	if (g_font_style_genlist) {
		elm_genlist_clear(g_font_style_genlist);

		char *default_font_name = NULL;
		char *tmp_name = NULL;
		int idx = 0, matched_idx = 0;
		int ret = 0;

		Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
		itc->item_style = "1text.1icon.1";
		itc->func.text_get = _gl_font_style_title_get;
		itc->func.content_get = _gl_font_style_ridio_get;
		itc->func.del = _font_style_gl_del;

		FREE(font_name);
		ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_FONT_TYPE, &tmp_name);
		if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
			ERR("failed to call system_setting_get_value_string with err %d", ret);
			tmp_name = _get_default_font();
		}

		default_font_name = _get_default_font();

		Font_Style_Item_Data *id_default = calloc(sizeof(Font_Style_Item_Data), 1);
		if (default_font_name && tmp_name && !strcmp(tmp_name, default_font_name)) {
			matched_idx = idx;
			font_name = strdup(tmp_name);
		}
		if (id_default) {
			id_default->index = idx++;
			id_default->font_name = (default_font_name != NULL) ? strdup(default_font_name) : NULL;
			id_default->item = elm_genlist_item_append(g_font_style_genlist, itc, id_default, NULL, ELM_GENLIST_ITEM_NONE,
					_font_style_gl_cb, (void *)id_default);
		}

		Eina_List *font_list = NULL;
		Eina_List *l = NULL;
		FcChar8 *font_data = NULL;
		font_list = _get_available_font_list();
		EINA_LIST_FOREACH(font_list, l, font_data) {
			if (!default_font_name || strcmp((const char *)default_font_name, (const char *)font_data)) {
				Font_Style_Item_Data *id = calloc(sizeof(Font_Style_Item_Data), 1);
				if (tmp_name && !strcmp((const char *)tmp_name, (const char *)font_data)) {
					matched_idx = idx;
					font_name = strdup(tmp_name);
				}
				DBG("Font1: %s, Font2: %s", tmp_name, font_data);

				if (id) {
					id->index = idx++;
					id->font_name = (char *)strdup((char *)font_data);
					id->item = elm_genlist_item_append(g_font_style_genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE,
							_font_style_gl_cb, (void *)id);
				}
			}
		}

		ad->font_style_rdg = evas_object_data_get(g_font_style_genlist, "radio_main");
		if (ad->font_style_rdg) {
			evas_object_del(ad->font_style_rdg);
			ad->font_style_rdg = NULL;
		}

		evas_font_reinit();
		ad->font_style_rdg = elm_radio_add(g_font_style_genlist);
		elm_radio_state_value_set(ad->font_style_rdg, -1);

		evas_object_data_set(g_font_style_genlist, "radio_main", ad->font_style_rdg);

		elm_genlist_realized_items_update(g_font_style_genlist);

		DBG("Matched index: %d", matched_idx);

		elm_radio_value_set(ad->font_style_rdg, matched_idx);

		elm_genlist_item_class_free(itc);
	}
}

void _show_font_size_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_screen_timeout_list - appdata is null");
		return;
	}
	Evas_Object *genlist  = NULL;
	Elm_Object_Item *nf_it = NULL;
	int idx;

	temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_font_size_title_get;
	itc->func.content_get = _gl_font_size_ridio_get;
	itc->func.del = _font_size_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	connect_to_wheel_with_genlist(genlist,ad);

	for (idx = 0; idx < FONT_SIZE_COUNT; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _font_size_gl_cb, (void *)idx);
		}
	}

	ad->font_size_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->font_size_rdg, 6);
	elm_radio_value_set(ad->font_size_rdg, screen_time_index);

	evas_object_data_set(genlist, "radio_main", ad->font_size_rdg);

	g_font_size_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	elm_genlist_item_class_free(itc);

	nf_it = elm_naviframe_item_push(ad->nf, "IDS_ST_BODY_FONT_SIZE_ABB", NULL, NULL, layout, NULL);
#if !defined(FEATURE_SETTING_TELEPHONY)
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
#endif
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
}

static char *_gl_roatate_screen_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(rotate_screen_str[id->index]));

	}
	return strdup(buf);
}

static Evas_Object *_gl_rotate_screen_radio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;
	static int rotate = -1;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);

		/*get_int rotate */
		rotate = _get_rotate_screen();
		rotate_screen_rot = rotate;

		if (rotate == SETTING_SCREENROTATION_90_DEGREE) {
			/*90R */
			rotate_screen_index = 1;
		} else if (rotate == SETTING_SCREENROTATION_270_DEGREE) {
			/*90L */
			rotate_screen_index = 2;
		} else if (rotate == SETTING_SCREENROTATION_180_DEGREE) {
			/*180 */
			rotate_screen_index = 3;
		}

		if (rotate == -1) {
			rotate_screen_rot = SETTING_SCREENROTATION_0_DEGREE;
		}
	}
	return radio;
}

static void _rotate_screen_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}

static void _rotate_screen_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	rotate_screen_index = (int)data;

	if (rotate_screen_index == 1) {
		/*90R */
		rotate_screen_rot = SETTING_SCREENROTATION_90_DEGREE;
	} else if (rotate_screen_index == 2) {
		/*90L */
		rotate_screen_rot = SETTING_SCREENROTATION_270_DEGREE;
	} else if (rotate_screen_index == 3) {
		/*180 */
		rotate_screen_rot = SETTING_SCREENROTATION_180_DEGREE;
	} else {
		rotate_screen_rot = SETTING_SCREENROTATION_0_DEGREE;
	}

	/*set int */
	_set_rotate_screen(rotate_screen_rot);

	elm_genlist_realized_items_update(g_rotate_screen_genlist);

	elm_naviframe_item_pop(temp_ad->nf);
	if (!temp_ad->rotate_screen_rdg) {
		evas_object_del(temp_ad->rotate_screen_rdg);
		temp_ad->rotate_screen_rdg = NULL;
	}
}

void _show_rotate_screen_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_screen_timeout_list - appdata is null");
		return;
	}
	Evas_Object *genlist  = NULL;
	Elm_Object_Item *nf_it = NULL;
	int idx;

	temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_roatate_screen_title_get;
	itc->func.content_get = _gl_rotate_screen_radio_get;
	itc->func.del = _rotate_screen_gl_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_block_count_set(genlist, 14);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	connect_to_wheel_with_genlist(genlist,ad);

	for (idx = 0; idx < ROTATE_SCREEN_COUNT; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _rotate_screen_gl_cb, (void *)idx);
		}
	}

	ad->rotate_screen_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->rotate_screen_rdg, -1);
	elm_radio_value_set(ad->rotate_screen_rdg, rotate_screen_index);

	evas_object_data_set(genlist, "radio_main", ad->rotate_screen_rdg);

	g_screen_time_genlist = genlist;

	elm_genlist_item_class_free(itc);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}

static void _set_rotate_screen(const int rotation)
{
	vconf_set_int(VCONFKEY_SETAPPL_SCREENROTATION_DEG_INT, rotation);
}

static int _get_rotate_screen()
{
	int rot;
	vconf_get_int(VCONFKEY_SETAPPL_SCREENROTATION_DEG_INT, &rot);
	return rot;
}

static void settings_font_style_changed_cb(system_settings_key_e key, void *user_data)
{
	DBG("settings_font_style_changed_cb");

	appdata *ad = user_data;
	if (ad == NULL)
		return;

	char *font_name = NULL;
	Evas_Object *font_style_radio = NULL;
	if (g_font_style_genlist) {
		font_style_radio = evas_object_data_get(g_font_style_genlist, "radio_main");
		if (font_style_radio) {
			if (system_settings_get_value_string(SYSTEM_SETTINGS_KEY_FONT_TYPE, &font_name)
			    != SYSTEM_SETTINGS_ERROR_NONE) {
				ERR("failed to call system_setting_get_value_string with err");
				return;
			}

			DBG("Update a font style list");

			DBG("System font: %s", font_name);

			int index = 0;
			Eina_List *font_list = NULL;
			Eina_List *l = NULL;
			Elm_Object_Item *font_item = NULL;
			Font_Style_Item_Data *font_data = NULL;

			font_list = elm_genlist_realized_items_get(g_font_style_genlist);
			EINA_LIST_FOREACH(font_list, l, font_item) {
				font_data = (Font_Style_Item_Data *) elm_object_item_data_get(font_item);
				if (font_name && !strcmp((const char *)font_name, (const char *)font_data->font_name)) {
					DBG("1: %s, 2: %s", font_name, font_data->font_name);
					DBG("Font style matched index : %d", index);
					elm_radio_value_set(font_style_radio, index);
					return;
				}
				index++;
			}
		}
	}

	if (font_style_item) {
		elm_genlist_item_update(font_style_item);
	}
}

static Eina_Bool setting_font_style_pop_cb(void *data, Elm_Object_Item *it)
{
	DBG("setting_font_style_pop_cb");

	g_font_style_genlist = NULL;

	return EINA_TRUE;
}

static Eina_Bool setting_font_list_pop_cb(void *data, Elm_Object_Item *it)
{
	DBG("setting_font_list_pop_cb");

	font_size_item = NULL;
	font_style_item = NULL;

	if (system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_FONT_TYPE) != 0) {
		ERR("system_settings_unset_changed_cb failed!!");
	}

	return EINA_TRUE;
}

static void change_language_enabling(keynode_t *key, void *data)
{
	if (lang_item == NULL) {
		DBG("Setting - lang_item is null!!");
		return;
	}

	int enable = 0;
	vconf_get_bool(VCONFKEY_WMS_WMANAGER_CONNECTED, &enable);
	if (enable) {
		DBG("Setting - Language is disabled");
	} else {
		DBG("Setting - Language is enabled");
	}

	if (lang_item) {
		elm_genlist_item_update(lang_item);
	}
}

static void change_screen_time_cb(keynode_t *key, void *data)
{
	DBG("Setting - change_screen_time_cb");

	if (screen_time_item) {
		elm_genlist_item_update(screen_time_item);
	}
}

static void change_language_cb(keynode_t *key, void *data)
{
	DBG("Setting - change_language_cb");

	if (lang_item) {
		elm_genlist_item_update(lang_item);
	}
}

#if 0
static void _display_brightness_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	Evas_Object *layout = NULL;
	Elm_Object_Item *navi_it = NULL;

	appdata *ad = data;

	if (ad != NULL) {
		layout = _show_brightness_popup(ad, obj, event_info);
	}

	if (layout) {
		evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_IN, _power_off_popup_dismiss_cb, NULL);

		navi_it = elm_naviframe_item_push(ad->nf, "IDS_ST_BUTTON_BRIGHTNESS", NULL, NULL, layout, NULL);
		elm_object_item_domain_text_translatable_set(navi_it, SETTING_PACKAGE, EINA_TRUE);
		elm_naviframe_item_pop_cb_set(navi_it, _brightness_pop_cb, NULL);

		register_vconf_changing(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, brightness_vconf_changed_cb, NULL);
	}
}

Evas_Object *_show_brightness_popup(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *icon;
	Evas_Object *ly;
	Evas_Object *vol;
	Evas_Object *btn;
	appdata *ad = data;
	int brightness_level = 0;

	if (ad == NULL)
		return NULL;

	temp_ad = ad;

	pd = (spin_date *) malloc(sizeof(spin_date));

	int real_brightness = 0;
	device_get_brightness(0, &real_brightness);
	DBG("Setting - Real brightness : %d", real_brightness);

	/* Vconf brightness level */
	vconf_get_int(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, &brightness_level);

	if (real_brightness != brightness_level) {	/* HBM mode -> normal level(4) */
		sync_brightness(real_brightness);

		brightness_level = real_brightness;
	}

	int enable = display_get_hbm();
	if (enable < 0) {
		DBG("Setting - dispaly_get_hbm() is fail!!");
	}

	if (enable == TRUE) {
		DBG("Setting - current HBM mode!!");
		brightness_index = 6;
	} else {
		brightness_index = _change_bright_lovel_to_index(brightness_level);
	}

	DBG("Setting - level: %d,   index: %d", brightness_level, brightness_index);

	brightness_origin_level = brightness_level;

	ly = elm_layout_add(ad->nf);
	elm_layout_file_set(ly, EDJE_PATH, "setting/2finger_popup/default2");
	evas_object_size_hint_min_set(ly, 320, 200);
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);

	brightness_layout = ly;

	pd->spinner = elm_spinner_add(ly);
	elm_object_style_set(pd->spinner, "brightnessstyle");
	elm_spinner_editable_set(pd->spinner, EINA_TRUE);
	elm_spinner_min_max_set(pd->spinner, 1, 6);
	evas_object_smart_callback_add(pd->spinner, "changed", _on_spinner_change_cb, pd);
	elm_object_part_content_set(ly, "elm.icon.1", pd->spinner);
	elm_spinner_value_set(pd->spinner, brightness_index);
	_update_brightness_circle(pd->spinner);

	btn = elm_button_add(ly);
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn, "IDS_ST_BUTTON_CANCEL_ABB2");
	elm_object_part_content_set(ly, "btn1", btn);
	evas_object_smart_callback_add(btn, "clicked", _set_cancel_cb, ad);

	btn = elm_button_add(ly);
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn, "IDS_WNOTI_BUTTON_OK_ABB2");
	elm_object_part_content_set(ly, "btn2", btn);
	evas_object_smart_callback_add(btn, "clicked", _set_brightness_clicked_cb, ad);

	hbm_mode_on_original = enable;	/* backup for cancel */

	g_spinner = pd->spinner;

	return ly;
}

static void _on_spinner_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _on_spinner_change_cb() is called!");

	Evas_Coord w;

	static int  prev = 0;
	double min, max;
	int idx = (int) elm_spinner_value_get(obj);

	edje_object_part_geometry_get(elm_layout_edje_get(obj), "center.image2", NULL, NULL, &w, NULL);
	elm_spinner_min_max_get(obj, &min, &max);

	DBG("Setting - min: %i, max: %i, idx: %d", (int)min, (int)max, idx);

	if (idx == max)
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,full", "elm");
	else if (idx < max)
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,default", "elm");
	if (idx == min)
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,min", "elm");
	if (idx > min)
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,normal", "elm");


	brightness_index = idx;

	if (brightness_index > 0 && brightness_index < 6) {
		int enable = display_get_hbm();
		if (enable < 0) {
			DBG("Setting - dispaly_get_hbm() is fail!!");
		} else if (enable == TRUE) {
			DBG("Setting - current HBM mode!!");

			elm_object_part_text_set(brightness_layout, "elm.text.2", "");

			_set_HBM_mode(FALSE);
		}
		/*set off sequnce : hbm off -> bright level down */
		int brightness_level = _change_bright_index_to_level(brightness_index);
		device_set_brightness_to_settings(0, brightness_level);
		vconf_set_int("db/setting/Brightness", brightness_level);
	} else if (brightness_index == 6) {
		DBG("Setting - HBM mode on!!");

		_set_HBM_mode(TRUE);

		elm_object_translatable_part_text_set(brightness_layout, "elm.text.2", "IDS_ST_BODY_OUTDOOR_MODE_ABB");
	}

	double posx2 = (double)(w / max) * brightness_index;

	prev = idx;
	edje_object_part_drag_value_set(elm_layout_edje_get(obj), "elm.dragable.slider", posx2, 0);
}

static Eina_Bool _brightness_pop_cb(void *data, Elm_Object_Item *it)
{
	DBG("Setting - brightness_pop_cb() is called!");

	if (pd) {
		DBG("Setting - Free pd!");
		free(pd);
	}

	unregister_vconf_changing(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, brightness_vconf_changed_cb);

	return EINA_TRUE;
}

static void _power_off_popup_dismiss_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _power_off_popup_dismiss_cb() is called!");

	int brightness_level = 0;
	vconf_get_int(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, &brightness_level);
	brightness_index = _change_bright_lovel_to_index(brightness_level);

	if (g_spinner) {
		int enable = display_get_hbm();
		if (enable) {
			brightness_index = 6;
		}
		elm_spinner_value_set(g_spinner, brightness_index);
		_update_brightness_circle(g_spinner);
	}
}

static void brightness_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - brightness vconf changed!!");

	int brightness_level = 0;
	brightness_level = vconf_keynode_get_int(key);
	brightness_index = _change_bright_lovel_to_index(brightness_level);

	if (g_spinner) {
		int enable = display_get_hbm();
		if (enable) {
			brightness_index = 6;
		}
		elm_spinner_value_set(g_spinner, brightness_index);
		_update_brightness_circle(g_spinner);
	}
}

static void _set_brightness_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = (appdata *) data;
	if (ad == NULL)
		return;

	int enable = display_get_hbm();
	if (enable == TRUE) {
		char buf[1024];
		snprintf(buf, sizeof(buf) - 1, _("IDS_IDLE_POP_AFTER_P1SD_MINS_BRIGHTNESS_WILL_BE_RESET_TO_DEFAULT_LEVEL_HP2SD"), 5, 4);

		/* show toast - automatic freed!! */
		struct _toast_data *toast = _create_toast(ad, buf);
		if (toast) {
			_show_toast(ad, toast);
		}
	} else {
		int brightness_level = _change_bright_index_to_level(brightness_index);

		device_set_brightness_to_settings(0, brightness_level);
	}

	brightness_layout = NULL;
	g_brightness_controller = NULL;

	if (ad->nf) {
		elm_naviframe_item_pop(ad->nf);
	}
}

static void sync_brightness(int real_brightness)
{
	DBG("Setting - Synchronized brightness level");

	vconf_set_int(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, real_brightness);
}

static void _update_brightness_circle(Evas_Object *spiner)
{
	if (spiner == NULL)
		return;

	Evas_Coord w;
	double min, max, posx2;
	int idx = (int) elm_spinner_value_get(spiner);

	edje_object_part_geometry_get(elm_layout_edje_get(spiner), "center.image2", NULL, NULL, &w, NULL);
	elm_spinner_min_max_get(spiner, &min, &max);

	int enable = display_get_hbm();
	if (enable < 0) {
		DBG("Setting - dispaly_get_hbm() is fail!!");
	}

	if (enable == TRUE) {
		edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,full1", "elm");

		elm_object_part_text_set(brightness_layout, "elm.text.2", _("IDS_ST_BODY_OUTDOOR_MODE_ABB"));
	} else {
		if (idx == min)
			edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,min", "elm");
		if (idx == max)
			edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,full1", "elm");
		if (idx < max)
			edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,default1", "elm");

		brightness_index = idx;

		posx2 = (double)(w / max) * brightness_index;

		edje_object_part_drag_value_set(elm_layout_edje_get(spiner), "elm.dragable.slider", posx2, 0);

		elm_object_part_text_set(brightness_layout, "elm.text.2", "");
	}
}

static void wake_up_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - motion_vconf_changed_cb() is called!");

	if (wake_up_item) {
		elm_genlist_item_update(wake_up_item);
	}
}

static void icon_size_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - icon_size_vconf_changed_cb() is called!");

	if (edit_icon_item) {
		elm_genlist_item_update(edit_icon_item);
	}
}

static void _set_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL)
		return;

	int enable = display_get_hbm();
	if (hbm_mode_on_original) {
		if (enable == DISABLE) {
			_set_HBM_mode(TRUE);
		}
	} else {
		if (enable == ENABLE) {
			_set_HBM_mode(FALSE);
		}
	}

	device_set_brightness_to_settings(0, brightness_origin_level);

	brightness_layout = NULL;
	g_brightness_controller = NULL;

	elm_naviframe_item_pop(ad->nf);
}

static int _change_bright_lovel_to_index(int level)
{
	int index = 0;

	if (level >= 20 && level <= 100) {
		index = (level / 20);
		DBG("Setting - level -> index : %d", index);
	}
	return index;
}

static int _change_bright_index_to_level(int index)
{
	int level = 1;
	if (index > 0 && index < 6) {
		switch (index) {
			case 1:
				level = 20;
				break;
			case 2:
				level = 40;
				break;
			case 3:
				level = 60;
				break;
			case 4:
				level = 80;
				break;
			case 5:
				level = 100;
				break;
		}
	}

	DBG("Setting - index -> level : %d", level);

	return level;
}

static void _set_HBM_mode(int enable)
{
	if (display_enable_hbm(enable, 300) == 0) {	/* after 5 minutes, HBM mode will be off! */
		DBG("Setting - HBM %s!!", (enable) ? "enabled" : "disabled");
	} else {
		DBG("Setting - HBM api failed!!");
	}
}

#endif
