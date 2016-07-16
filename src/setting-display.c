/* * Copyright (c) 2013-2014 Samsung Electronics Co., Ltd All Rights Reserved
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
 *	Created on: Oct 9, 2013
 *		Author: min-hoyun
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
#include "setting-clock.h"


/* temporary source code */
/*#ifndef VCONFKEY_SETAPPL_NOTIFICATION_INDICATOR */
/*#define VCONFKEY_SETAPPL_NOTIFICATION_INDICATOR "db/setting/notification_indicator" */
/*#endif */
#define VCONFKEY_SETAPPL_LCD_TIMEOUT_BACKUP_FOR_WATCH_ALWAYS_ON "db/setting/lcd_backlight_timeout_backup"
#define SETTINGS_FIXED_DEFAULT_FONT_NAME "BreezeSans"

#define SETTINGS_DEFAULT_FONT_NAME "Default"



Evas_Object *g_btn_plus = NULL;
Evas_Object *g_btn_minus = NULL;

static int is_changed = 0;
static bool running = false;

static Evas_Object *_gl_display_watch_always_check_get(void *data, Evas_Object *obj, const char *part);
static void _display_watch_always_check_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _clock_cb(void *data, Evas_Object *obj, void *event_info);
/* static void _display_gl_watch_always_on_cb(void *data, Evas_Object *obj, void *event_info); */
static void _display_brightness_cb(void *data, Evas_Object *obj, void *event_info);
static Evas_Object *_gl_display_noti_indicator_check_get(void *data, Evas_Object *obj, const char *part);
static void _display_gl_display_noti_indicator_cb(void *data, Evas_Object *obj, void *event_info);
static void _display_gl_display_noti_indicator_help_cb(void *data, Evas_Object *obj, void *event_info);
static void _show_noti_indicator_list(void *data);

static struct _display_menu_item display_menu_its[] = {
	{ "Watch face",				SETTING_DISPLAY_WATCH_FACE,	_clock_cb},
	/*	{ "watch always on",	SETTING_DISPLAY_WATCH_ALWAYS_ON,	_display_gl_watch_always_on_cb	}, */
	{ "Notification indicator",	SETTING_DISPLAY_NOTIFICATION_INDICATOR,	_display_gl_display_noti_indicator_cb },
	{ "IDS_ST_BUTTON_BRIGHTNESS",	SETTING_DISPLAY_BRIGTHNESS, _display_brightness_cb	 },
	{ "IDS_ST_MBODY_SCREEN_TIMEOUT_ABB",	SETTING_DISPLAY_SCREEN_TIME,	_display_gl_screen_timeout_cb	},
	{ "IDS_ST_BODY_FONT",					SETTING_DISPLAY_FONT,	_display_gl_font_cb		},
	/*	{ "IDS_ST_BUTTON_LANGUAGE",				SETTING_DISPLAY_LANG,	_display_gl_language_cb	},
	#if !defined(FEATURE_SETTING_SDK) && !defined(FEATURE_SETTING_EMUL)
		{ "IDS_ST_MBODY_MANAGE_APPS_ABB",	SETTING_DISPLAY_EDIT_APPS,	_homescreen_gl_edit_apps_cb	},
	#endif
	*/
};

static struct _font_menu_item font_menu_its[] = {
	{ "IDS_ST_BODY_FONT_STYLE",			SETTING_DISPLAY_FONT_STYLE, _display_gl_font_style_cb },
	{ "IDS_ST_BODY_FONT_SIZE_ABB",		SETTING_DISPLAY_FONT_SIZE,	_display_gl_font_size_cb },
};

static struct _display_menu_item noti_menu_its[] = {
	{ "Notification indicator",	SETTING_DISPLAY_NOTIFICATION_INDICATOR_SW,	NULL},
	{ "Help",	SETTING_DISPLAY_NOTIFICATION_INDICATOR_HELP,	_display_gl_display_noti_indicator_help_cb },
};

static int timeout_arr[] = {
	0, 15, 30, 60, 300
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
static Evas_Object *g_font_genlist = NULL;
static Evas_Object *g_rotate_screen_genlist = NULL;
static Evas_Object *g_noti_indicator_genlist = NULL;

static int g_screen_time_index = 1;		/* default: 10 seconds */
static int font_size_index	 = 1;		/* default: normal */
static int rotate_screen_rot  = 0;		/* default: 0(0degree), vconf enum */
static int rotate_screen_index	= 0;		/* default: 0, list index */

static int touch_mode = NONE;

/* Main display list item */
static Elm_Object_Item *lang_item = NULL;
static Elm_Object_Item *wake_up_item = NULL;
static Elm_Object_Item *edit_icon_item = NULL;
static Elm_Object_Item *g_screen_time_item = NULL;

/* Font list item */
static char *font_name = NULL;

static Elm_Object_Item *font_style_item = NULL;
static Elm_Object_Item *font_size_item = NULL;

static void _font_size_gl_cb(void *data, Evas_Object *obj, void *event_info);
static void _font_style_gl_cb(void *data, Evas_Object *obj, void *event_info);
static void settings_font_style_changed_cb(system_settings_key_e key, void *user_data);
static void change_language_enabling(keynode_t *key, void *data);
static void change_screen_time_cb(keynode_t *key, void *data);
static void change_language_cb(keynode_t *key, void *data);
static void setting_font_list_pop_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _lang_update_font_style_list(void *data, Evas_Object *obj, void *event_info);
static void _set_rotate_screen(const int rotation);
static int _get_rotate_screen();

enum {
	DISPLAY_TITLE_DISPLAY,
	DISPLAY_TITLE_LANGUAGE,
	DISPLAY_TITLE_SCREEN_TIMEOUT,
	DISPLAY_TITLE_FONT,
	DISPLAY_TITLE_FONT_STYLE,
	DISPLAY_TITLE_FONT_SIZE,
	DISPLAY_TITLE_NOTIFICATION_INDICATOR,
	DISPLAY_TITLE_ROTATE,
};

static char *
_gl_menu_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	int title_idx = (int)data;
	switch (title_idx) {
	case DISPLAY_TITLE_DISPLAY:
		snprintf(buf, 1023, "%s", "Display");
		break;
	case DISPLAY_TITLE_LANGUAGE:
		snprintf(buf, 1023, "%s", _("IDS_ST_BUTTON_LANGUAGE"));
		break;
	case DISPLAY_TITLE_SCREEN_TIMEOUT:
		snprintf(buf, 1023, "%s", _("IDS_ST_MBODY_SCREEN_TIMEOUT_ABB"));
		break;
	case DISPLAY_TITLE_FONT:
		snprintf(buf, 1023, "%s", _("IDS_ST_BODY_FONT"));
		break;
	case DISPLAY_TITLE_FONT_STYLE:
		snprintf(buf, 1023, "%s", _("IDS_ST_BODY_FONT_STYLE"));
		break;
	case DISPLAY_TITLE_FONT_SIZE:
		snprintf(buf, 1023, "%s", _("IDS_ST_BODY_FONT_SIZE_ABB"));
		break;
	case DISPLAY_TITLE_NOTIFICATION_INDICATOR:
		snprintf(buf, 1023, "%s", "Notification indicator");
		break;
	case DISPLAY_TITLE_ROTATE:
		snprintf(buf, 1023, "%s", "Rotate");
		break;

	}
	return strdup(buf);
}

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
	g_screen_time_item = NULL;

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

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, "empty");
	back_button_cb_push(&back_key_generic_cb, data, NULL, g_display_genlist, "g_display_genlist");
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
		} else if (id->item == g_screen_time_item) {
			int time = 0;
			vconf_get_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, &time);
			switch (time) {
			case 0:
				snprintf(buf, sizeof(buf) - 1, "No off");
				break;
			case 15:
				snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_15SEC"));
				break;
			case 30:
				snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_30SEC"));
				break;
			case 60:
				snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_1_MINUTE_ABB2"));
				break;
			case 300:
				snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_5_MINUTES"));
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
	FREE(id);
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

	Elm_Genlist_Item_Class *itc3 = elm_genlist_item_class_new();
	itc3->item_style = "1text.1icon.1";
	itc3->func.text_get = _gl_display_title_get;
	itc3->func.content_get = _gl_display_watch_always_check_get;
	itc3->func.del = _display_gl_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist, ad);

	menu_its = display_menu_its;

	Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
	title_item->func.text_get = _gl_menu_title_text_get;
	title_item->item_style = "title";
	title_item->func.del = NULL;

	elm_genlist_item_append(genlist, title_item, (void *)DISPLAY_TITLE_DISPLAY, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_item);

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
		/*
		if (menu_its[idx].type == SETTING_DISPLAY_WATCH_ALWAYS_ON) {
			itc_tmp = itc3;
		}
		*/

		Display_Item_Data *id = calloc(sizeof(Display_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
						   genlist,		/* genlist object */
						   itc_tmp,		/* item class */
						   id,			/* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_its[idx].func,	/* call back */
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
				g_screen_time_item = id->item;
			}
		}
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc2);
	elm_genlist_item_class_free(itc3);

	Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
	padding->item_style = "padding";
	padding->func.del = _display_gl_del;

	elm_genlist_item_append(genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(padding);

	g_display_genlist = genlist;

	return genlist;
}

static char *_gl_screen_timeout_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
#ifdef FEATURE_SETTING_EMUL
	int emul_val = 1;
#else
	/*	int emul_val = 0; */
	int emul_val = 1;
#endif

	if (!strcmp(part, "elm.text")) {
		if (emul_val == 1 && id->index == 0) {
			snprintf(buf, sizeof(buf) - 1, "No off");
		} else if (id->index == (0 + emul_val)) {
			snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_15SEC"));
		} else if (id->index == (1 + emul_val)) {
			snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_30SEC"));
		} else if (id->index == (2 + emul_val)) {
			snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_1_MINUTE_ABB2"));
		} else if (id->index == (3 + emul_val)) {
			snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_5_MINUTES"));
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

	g_screen_time_index = (int)data;

	vconf_set_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL , timeout_arr[g_screen_time_index]);

	elm_genlist_realized_items_update(g_screen_time_genlist);

	elm_naviframe_item_pop(temp_ad->nf);
	back_button_cb_pop();
	if (!temp_ad->screen_timeout_rdg) {
		evas_object_del(temp_ad->screen_timeout_rdg);
		temp_ad->screen_timeout_rdg = NULL;
	}
}

static Evas_Object *_gl_screen_timeout_radio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;
	static int timeout = -1;
#ifdef FEATURE_SETTING_EMUL
	int emul_minus = 0;
#else
	/*	int emul_minus = 1; */
	int emul_minus = 0;
#endif

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
			g_screen_time_index = _get_timeout_index(timeout) - emul_minus;
		}

		if (g_screen_time_index == id->index) {
			elm_radio_value_set(radio_main, g_screen_time_index);
		}
	}
	return radio;
}

static void _screen_timeout_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	FREE(id);
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
	itc->func.content_get = _gl_screen_timeout_radio_get;
	itc->func.del = _screen_timeout_gl_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	connect_to_wheel_with_genlist(genlist, ad);

	int timeout = 0;
	vconf_get_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, &timeout);
	g_screen_time_index = _get_timeout_index(timeout);

	Elm_Object_Item *curr_item = NULL;

#ifdef FEATURE_SETTING_EMUL
	int emul_end = 0;
#else
	/*	int emul_end = 1; */
	int emul_end = 0;
#endif

	Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
	title_item->func.text_get = _gl_menu_title_text_get;
	title_item->item_style = "title";
	title_item->func.del = NULL;

	elm_genlist_item_append(genlist, title_item, (void *)DISPLAY_TITLE_SCREEN_TIMEOUT, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_item);

	for (idx = 0; idx < SCREEN_TIME_COUNT - emul_end; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _screen_timeout_gl_cb, (void *)idx);

			if (idx == g_screen_time_index) {
				curr_item = id->item;
			}
		}
	}

	ad->screen_timeout_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->screen_timeout_rdg, SCREEN_TIME_COUNT - emul_end);
	elm_radio_value_set(ad->screen_timeout_rdg, g_screen_time_index);

	evas_object_data_set(genlist, "radio_main", ad->screen_timeout_rdg);

	if (curr_item) {
		elm_genlist_item_show(curr_item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
	}

	Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
	padding->item_style = "padding";
	padding->func.del = _screen_timeout_gl_del;

	elm_genlist_item_append(genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(padding);

	g_screen_time_genlist = genlist;

	elm_genlist_item_class_free(itc);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, "empty");
	back_button_cb_push(&back_key_generic_cb, data, NULL, g_display_genlist, "g_display_genlist");
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
					} else if (!strcmp(font_name, "BreezeSans")) {
						snprintf(buf, sizeof(buf) - 1, "%s", SETTINGS_DEFAULT_FONT_NAME);
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
		if (id->index == 0)
			snprintf(buf, sizeof(buf) - 1, "<font=%s>%s</font>", new_name, SETTINGS_DEFAULT_FONT_NAME);
		else
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

		evas_object_smart_callback_add(radio, "changed", _font_style_gl_cb, (void *)id);
		evas_object_propagate_events_set(radio, EINA_FALSE);
	}

	return radio;
}

static void _font_style_gl_del(void *data, Evas_Object *obj)
{
	Font_Style_Item_Data *id = data;
	if (id) {
		FREE(id->font_name);
		FREE(id);
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
	FREE(id);
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
	g_font_style_genlist = NULL;
	back_key_generic_cb(temp_ad, obj, event_info);
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


	if (!temp_ad->font_size_rdg) {
		evas_object_del(temp_ad->font_size_rdg);
		temp_ad->font_size_rdg = NULL;
	}

	if (font_timer) {
		ecore_timer_del(font_timer);
		font_timer = NULL;
	}

	if (font_size_item ) {
		elm_genlist_item_update(font_size_item );
	}

	back_key_generic_cb(temp_ad, obj, event_info);

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
	connect_to_wheel_with_genlist(genlist, ad);

	Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
	title_item->func.text_get = _gl_menu_title_text_get;
	title_item->item_style = "title";
	title_item->func.del = NULL;

	elm_genlist_item_append(genlist, title_item, (void *)DISPLAY_TITLE_FONT, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_item);

	for (idx = 0; idx < 2; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist, itc, id, NULL,
											   ELM_GENLIST_ITEM_NONE, font_menu_its[idx].func, ad);

			if (idx == 0) {
				font_style_item = id->item;
			}
			if (idx == 1) {
				font_size_item  = id->item;
			}
		}
	}

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
	padding->item_style = "padding";
	padding->func.del = _font_size_gl_del;

	elm_genlist_item_append(genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(padding);

	elm_genlist_item_class_free(itc);
	g_font_genlist = genlist;

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, "empty");
	back_button_cb_push(&back_key_generic_cb, data, NULL, g_display_genlist, "g_display_genlist");
#if !defined(FEATURE_SETTING_TELEPHONY)
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
#endif
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
	evas_object_event_callback_add(nf_it, EVAS_CALLBACK_DEL, setting_font_list_pop_cb, ad);

	if (system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_FONT_TYPE, settings_font_style_changed_cb, ad) != 0) {
		ERR("system_settings_set_changed_cb failed!!");
	}
}

static char *_get_default_font_name()
{
	char *default_font_name = SETTINGS_FIXED_DEFAULT_FONT_NAME;
	/* Default font means current font type !! */
	/* That's reason why I fixed default font */
	/* system_settings_get_value_string(SYSTEM_SETTINGS_KEY_DEFAULT_FONT_TYPE, &default_font_name);*/
	return default_font_name;
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
				/* always shown for D/L */ if (!strncmp((const char *)file, SETTING_FONT_DOWNLOADED_FONT_PATH, download_path_len)) {
					/* Find proper family name for current locale. */
					while (locale && family && lang) {
						ERR("locale: %s, family: %s, lang: %s", locale, family, lang);

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
	connect_to_wheel_with_genlist(genlist, ad);

	Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
	title_item->func.text_get = _gl_menu_title_text_get;
	title_item->item_style = "title";
	title_item->func.del = NULL;

	elm_genlist_item_append(genlist, title_item, (void *)DISPLAY_TITLE_FONT_STYLE, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_item);

	FREE(font_name);
	ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_FONT_TYPE, &tmp_name);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		ERR("failed to call system_setting_get_value_string with err %d", ret);
		tmp_name = _get_default_font_name();
		if (tmp_name == NULL) {
			ERR("failed to get default font name");
			return -1;
		} else {
			DBG("get_default_font = %s", tmp_name);
		}
	} else {
		DBG("SYSTEM_SETTINGS_KEY_FONT_TYPE = %s", tmp_name);
	}

	default_font_name = _get_default_font_name();

	if (default_font_name) {
		ERR("default_font_name is >>>> %s", default_font_name);
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
	Eina_List *l = NULL;
	FcChar8 *font_data = NULL;
	Eina_List *font_list = NULL;
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

	Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
	padding->item_style = "padding";
	padding->func.del = _font_style_gl_del;

	elm_genlist_item_append(genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(padding);

	evas_font_reinit();
	ad->font_style_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->font_style_rdg, -1);
	elm_radio_value_set(ad->font_style_rdg, matched_idx);

	evas_object_data_set(genlist, "radio_main", ad->font_style_rdg);

	g_font_style_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	elm_genlist_item_class_free(itc);

	evas_object_smart_callback_add(genlist, "language,changed", _lang_update_font_style_list, ad);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, "empty");
#if !defined(FEATURE_SETTING_TELEPHONY)
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
#endif
	back_button_cb_push(&back_key_generic_cb, data, NULL, g_font_genlist, "g_font_genlist");
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);

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
			tmp_name = _get_default_font_name();
		}

		Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
		title_item->func.text_get = _gl_menu_title_text_get;
		title_item->item_style = "title";
		title_item->func.del = NULL;

		elm_genlist_item_append(g_font_style_genlist, title_item, (void *)DISPLAY_TITLE_FONT_STYLE, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

		elm_genlist_item_class_free(title_item);

		default_font_name = _get_default_font_name();

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

		Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
		padding->item_style = "padding";
		padding->func.del = _font_style_gl_del;

		elm_genlist_item_append(g_font_style_genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_class_free(padding);

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
	connect_to_wheel_with_genlist(genlist, ad);

	Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
	title_item->func.text_get = _gl_menu_title_text_get;
	title_item->item_style = "title";
	title_item->func.del = NULL;

	elm_genlist_item_append(genlist, title_item, (void *)DISPLAY_TITLE_FONT_SIZE, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_item);

	for (idx = 0; idx < FONT_SIZE_COUNT; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _font_size_gl_cb, (void *)idx);
		}
	}

	ad->font_size_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->font_size_rdg, 3);
	elm_radio_value_set(ad->font_size_rdg, font_size_index);

	evas_object_data_set(genlist, "radio_main", ad->font_size_rdg);

	Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
	padding->item_style = "padding";
	padding->func.del = _font_size_gl_del;

	elm_genlist_item_append(genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(padding);

	g_font_size_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	elm_genlist_item_class_free(itc);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, "empty");
	back_button_cb_push(&back_key_generic_cb, data, NULL, g_font_genlist, "g_font_genlist");
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
	FREE(id);
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
	connect_to_wheel_with_genlist(genlist, ad);

	Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
	title_item->func.text_get = _gl_menu_title_text_get;
	title_item->item_style = "title";
	title_item->func.del = NULL;

	elm_genlist_item_append(genlist, title_item, (void *)DISPLAY_TITLE_ROTATE, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_item);

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

	Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
	padding->item_style = "padding";
	padding->func.del = _rotate_screen_gl_del;

	elm_genlist_item_append(genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(padding);
	g_screen_time_genlist = genlist;

	elm_genlist_item_class_free(itc);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, "empty");
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

static void setting_font_list_pop_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	ERR("setting_font_list_pop_cb");

	font_size_item = NULL;
	font_style_item = NULL;

	if (system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_FONT_TYPE) != 0) {
		ERR("system_settings_unset_changed_cb failed!!");
	}

	return;
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

	if (g_screen_time_item) {
		elm_genlist_item_update(g_screen_time_item);
	}
}

static void change_language_cb(keynode_t *key, void *data)
{
	DBG("Setting - change_language_cb");

	if (lang_item) {
		elm_genlist_item_update(lang_item);
	}
}



int brightness_index = 0;
int brightness_origin_level = 0;
Evas_Object *brightness_layout = NULL;

Evas_Object *g_slider = NULL;

static void _brightness_pop_cb(void *data, Evas_Object *obj, void *event_info);
static void brightness_vconf_changed_cb(keynode_t *key, void *data);
static void sync_brightness(int real_brightness);
static int _change_bright_lovel_to_index(int level);
#if 0 /*!defined(FEATURE_SETTING_EMUL) */
static void _set_HBM_mode(int enable);
#endif
static int _change_bright_index_to_level(int index);

int hbm_mode_on_original = 0;	/* backup for cancel */

/*#if defined(FEATURE_SETTING_EMUL) */
int	 display_get_hbm()
{
	/* DUMMY FUNCTION FOR EMULATOR */
	return 0;
}
int display_enable_hbm(int enable, int timeout)
{
	/* after timeout(sec) minutes, HBM mode will be off! */
	return 0;
}
/*#endif */

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

		navi_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
		elm_naviframe_item_title_enabled_set(navi_it, EINA_FALSE, EINA_FALSE);
		elm_object_item_domain_text_translatable_set(navi_it, SETTING_PACKAGE, EINA_TRUE);
		back_button_cb_push(&_brightness_pop_cb, data, NULL, g_display_genlist, "g_display_genlist");

		register_vconf_changing(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, brightness_vconf_changed_cb, NULL);
	}
}

static void _change_btn_img(void *data, Evas_Object *btn_obj, char * path, char * btn_str)
{
	char img_path[PATH_MAX];
	Evas_Object *page_layout = (Evas_Object *)data;
	snprintf(img_path, sizeof(img_path), "%s/%s", IMG_DIR, path);
	elm_image_file_set(btn_obj, img_path, NULL);
	elm_object_part_content_set(page_layout, btn_str, btn_obj);
}

static void _brightness_value_plus(void *data)
{
	if (brightness_index < 10) {
		brightness_index++;
		_change_btn_img(data, g_btn_minus, "b_slider_icon_minus.png", "btn1");
	}

	if (brightness_index == 10)	{
		ERR("disable plus btn2 ");
		_change_btn_img(data, g_btn_plus, "b_slider_icon_plus_disable.png", "btn2");
	}
}

static void _brightness_value_minus(void *data)
{
	if (brightness_index > 1) {
		brightness_index--;
		_change_btn_img(data, g_btn_plus, "b_slider_icon_plus.png", "btn2");
	}

	if (brightness_index == 1) {
		ERR("disable minus btn1");
		_change_btn_img(data, g_btn_minus, "b_slider_icon_minus_disable.png", "btn1");
	}
}

static Eina_Bool
_value_changed_rotary(void *data, Evas_Object *obj, Eext_Rotary_Event_Info *info)
{
	char buf[1024];
	Evas_Object *page_layout = (Evas_Object *)data;

	if (info->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE) {
		_brightness_value_plus(data);
	} else {
		_brightness_value_minus(data);
	}

	snprintf(buf, sizeof(buf), "%02d", brightness_index);
	ERR("Slider value = %s\n", buf);
	elm_object_part_text_set(page_layout, "elm.text.slider", buf);

	is_changed = 1;		/* changed flag!! */

	DBG("Setting - brightness_index : %d", brightness_index);
	/*
	#if !defined(FEATURE_SETTING_EMUL)
		if (brightness_index > 0 && brightness_index < 6) {
			int enable = display_get_hbm();
			if (enable < 0) {
				DBG("Setting - dispaly_get_hbm() is fail!!");
			} else if (enable == TRUE) {
				DBG("Setting - current HBM mode!!");

				//elm_object_part_text_set(brightness_layout, "elm.text.2", "");

				_set_HBM_mode(FALSE);
			}
	*/
	/*set off sequnce : hbm off -> bright level down */
	/*
			int brightness_level = _change_bright_index_to_level(brightness_index);
			device_set_brightness_to_settings(0, brightness_level);
			vconf_set_int("db/setting/Brightness", brightness_level);
		} else if (brightness_index == 6) {
			DBG("Setting - HBM mode on!!");

			_set_HBM_mode(TRUE);

	//		elm_object_translatable_part_text_set(brightness_layout, "elm.text.2", "IDS_ST_BODY_OUTDOOR_MODE_ABB");
		}
	#else
	*/
	int brightness_level = _change_bright_index_to_level(brightness_index);
	device_set_brightness_to_settings(0, brightness_level);
	vconf_set_int("db/setting/Brightness", brightness_level);
	/*#endif*/

	return EINA_TRUE;
}

static void _press_plus_brightness_cb(void *data, Evas_Object *obj, void *event_info)
{
	char buf[1024];
	Evas_Object *page_layout = (Evas_Object *)data;

	_brightness_value_plus(data);

	snprintf(buf, sizeof(buf), "%02d", brightness_index);
	ERR("Pressed Plus btn!! Slider value = %s\n", buf);
	elm_object_part_text_set(page_layout, "elm.text.slider", buf);

	is_changed = 1;		/* changed flag!! */

	DBG("Setting - brightness_index : %d", brightness_index);
	int brightness_level = _change_bright_index_to_level(brightness_index);
	device_set_brightness_to_settings(0, brightness_level);
	vconf_set_int("db/setting/Brightness", brightness_level);

}

static void _press_minus_brightness_cb(void *data, Evas_Object *obj, void *event_info)
{
	char buf[1024];
	Evas_Object *page_layout = (Evas_Object *)data;

	_brightness_value_minus(data);

	snprintf(buf, sizeof(buf), "%02d", brightness_index);
	ERR("Pressed Plus btn!! Slider value = %s\n", buf);
	elm_object_part_text_set(page_layout, "elm.text.slider", buf);

	is_changed = 1;		/* changed flag!! */

	DBG("Setting - brightness_index : %d", brightness_index);
	int brightness_level = _change_bright_index_to_level(brightness_index);
	device_set_brightness_to_settings(0, brightness_level);
	vconf_set_int("db/setting/Brightness", brightness_level);
}

Evas_Object *_show_brightness_popup(void *data, Evas_Object *obj, void *event_info)
{
	char img_path[PATH_MAX];
	appdata *ad = data;
	int brightness_level = 0;
	int real_brightness = 0;

	if (ad == NULL)
		return NULL;

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

	DBG("Setting - level: %d,	index: %d", brightness_level, brightness_index);

	brightness_origin_level = brightness_level;

	Evas_Object *slider = NULL;
	Evas_Object *img = NULL;
	Evas_Object *page_layout = elm_layout_add(ad->nf);

	evas_object_size_hint_weight_set(page_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(page_layout, 0, EVAS_HINT_FILL);
	elm_layout_file_set(page_layout, EDJE_PATH, "slider_brightness_layout");
	evas_object_show(page_layout);
	brightness_layout = page_layout;

	/* Add new eext_circle_object_slider with Eext_Circle_Surface object to render.
	Value is set as 3 which is within range from 0 to 15. */
	slider = eext_circle_object_slider_add(page_layout, ad->circle_surface);
	eext_circle_object_value_min_max_set(slider, 1.0, 10.0);

	eext_circle_object_value_set(slider, (float)brightness_index);

	/* Activate Circle slider's rotary object event.
	Its value increases/decreases its value by 0.5 to clockwise/counter clockwise
	rotary event. */
	eext_circle_object_slider_step_set(slider, 1.0);

	char buf[1024];
	snprintf(buf, sizeof(buf), "%02d", brightness_index);
	elm_object_part_text_set(page_layout, "elm.text.slider", buf);

	Evas_Object *btn_minus;
	btn_minus = elm_image_add(page_layout);

	if (brightness_index > 1)
		snprintf(img_path, sizeof(img_path), "%s/b_slider_icon_minus.png", IMG_DIR);
	else
		snprintf(img_path, sizeof(img_path), "%s/b_slider_icon_minus_disable.png", IMG_DIR);

	elm_image_file_set(btn_minus, img_path, NULL);
	elm_object_part_content_set(page_layout, "btn1", btn_minus);
	evas_object_smart_callback_add(btn_minus, "clicked", _press_minus_brightness_cb, page_layout);

	Evas_Object *btn_plus;
	btn_plus = elm_image_add(page_layout);
	if (brightness_index < 10)
		snprintf(img_path, sizeof(img_path), "%s/b_slider_icon_plus.png", IMG_DIR);
	else
		snprintf(img_path, sizeof(img_path), "%s/b_slider_icon_plus_disable.png", IMG_DIR);

	elm_image_file_set(btn_plus, img_path, NULL);
	elm_object_part_content_set(page_layout, "btn2", btn_plus);
	evas_object_smart_callback_add(btn_plus, "clicked", _press_plus_brightness_cb, page_layout);

	g_btn_plus = btn_plus;
	g_btn_minus = btn_minus;

	Eina_Bool res = eext_rotary_object_event_callback_add(slider, _value_changed_rotary, page_layout);
	ERR("rotary_event_handler result = %d", res);

	img = elm_image_add(page_layout);
	snprintf(img_path, sizeof(img_path), "%s/Brightness/b_setting_brightness_06.png", IMG_DIR);
	elm_image_file_set(img, img_path, NULL);
	elm_object_part_content_set(page_layout, "elm.icon", img);
	elm_object_part_text_set(page_layout, "elm.text.bottom", "Brightness");

	/* Make unselect state all of the pages except first one */
	elm_object_signal_emit(page_layout, "elm,state,thumbnail,unselect", "elm");

	eext_rotary_object_event_activated_set(slider, EINA_TRUE);

	return page_layout;

}

static void _brightness_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - brightness_pop_cb() is called!");

	appdata *ad = (appdata *)data;
	if (ad) {
		elm_naviframe_item_pop(ad->nf);
		back_button_cb_pop();
	} else {
		ERR("data ptr is NULL");
	}

	unregister_vconf_changing(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, brightness_vconf_changed_cb);

	return;
}

static void brightness_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - brightness vconf changed!!");

}

static void sync_brightness(int real_brightness)
{
	DBG("Setting - Synchronized brightness level");

	vconf_set_int(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, real_brightness);
}

static int _change_bright_lovel_to_index(int level)
{
	int index = 0;

	if (level >= 0 && level <= 100) {
		index = (level / 10);
		DBG("Setting - level -> index : %d", index);
	}
	return index;
}

static int _change_bright_index_to_level(int index)
{
	int level = 1;
	if (index > 0 && index < 11) {
		switch (index) {
		case 1:
			level = 10;
			break;
		case 2:
			level = 20;
			break;
		case 3:
			level = 30;
			break;
		case 4:
			level = 40;
			break;
		case 5:
			level = 50;
			break;
		case 6:
			level = 60;
			break;
		case 7:
			level = 70;
			break;
		case 8:
			level = 80;
			break;
		case 9:
			level = 90;
			break;
		case 10:
			level = 100;
			break;
		}
	}

	DBG("Setting - index -> level : %d", level);

	return level;
}

#if 0 /*!defined(FEATURE_SETTING_EMUL) */
static void _set_HBM_mode(int enable)
{
	if (display_enable_hbm(enable, 300) == 0) {	/* after 5 minutes, HBM mode will be off! */
		DBG("Setting - HBM %s!!", (enable) ? "enabled" : "disabled");
	} else {
		DBG("Setting - HBM api failed!!");
	}
}
#endif






static void _clock_cb(void *data, Evas_Object *obj, void *event_info)
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

	layout = _clock_type_cb(data, obj, event_info);
	if (layout == NULL) {
		DBG("%s", "clock cb - layout is null");
		return;
	}
	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	back_button_cb_push(&back_key_generic_cb, data, NULL, g_display_genlist, "g_display_genlist");
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _clear_clock_cb, ad);
	/*elm_naviframe_item_pop_cb_set(nf_it, _clear_clock_cb, ad); */
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
	is_running_clock = 1;

	ad->MENU_TYPE = SETTING_CLOCK;
}

static Evas_Object *_gl_display_watch_always_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	Display_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);

		int timeout = -1;
		vconf_get_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, &timeout);
		if (timeout)
			vconf_set_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_BACKUP_FOR_WATCH_ALWAYS_ON, timeout);

		elm_check_state_set(check, (timeout == 0) ? EINA_TRUE : EINA_FALSE);

		elm_object_style_set(check, "on&off");
		/*		evas_object_smart_callback_add(check, "changed", _display_watch_always_check_cb, (void *)check); */
		evas_object_event_callback_add(check, EVAS_CALLBACK_MOUSE_DOWN, _display_watch_always_check_cb, (void *)check);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_TRUE);

		id->check = check;

		index++;
	}

	return check;
}

static void _set_watch_always_on_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	int timeout = -1;
	appdata *ad = temp_ad;
	Evas_Object *check = (Evas_Object *)data;

	vconf_get_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_BACKUP_FOR_WATCH_ALWAYS_ON, &timeout);
	vconf_set_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, timeout);

	elm_check_state_set(check,	EINA_FALSE);

	elm_naviframe_item_pop(ad->nf);
}

static void _set_watch_always_on_ok_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	int timeout = 0;
	appdata *ad = temp_ad;
	Evas_Object *check = (Evas_Object *)data;

	vconf_set_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, timeout);
	elm_check_state_set(check,	EINA_TRUE);

	elm_naviframe_item_pop(ad->nf);
}

static Eina_Bool _back_watch_always_on_naviframe_cb(void *data, Elm_Object_Item *it)
{
	return EINA_TRUE;
}

static void _display_watch_always_check_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	appdata *ad = temp_ad;
	Evas_Object *ly;
	Evas_Object *check = (Evas_Object *)data;
	int timeout = 0;

	if (ad == NULL) {
		DBG("%s", "_display_watch_always_check_cb - appdata or check is null");
		return;
	}

	DBG("_display_watch_always_check_cb is called!!!!!!!");

	vconf_get_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, &timeout);
	DBG("timeout:%d ", timeout);

	/*if (timeout) { */
	if (1) {
		ly = elm_layout_add(ad->nf);
		elm_layout_file_set(ly, EDJE_PATH, "setting/2finger_popup/default5");
		evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);

		elm_object_part_text_set(ly, "watch_on_text.text1", "Enabling watch always on will siginificantly increase battery consumption.");

		Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->nf,
														 NULL,
														 NULL, NULL,
														 ly, NULL);

		Evas_Object *btn_cancel;
		btn_cancel = elm_button_add(ly);
		elm_object_style_set(btn_cancel, "default");
		evas_object_size_hint_weight_set(btn_cancel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_translatable_text_set(btn_cancel, "IDS_ST_BUTTON_CANCEL_ABB2");
		elm_object_part_content_set(ly, "btn1", btn_cancel);
		evas_object_smart_callback_add(btn_cancel, "clicked", _set_watch_always_on_cancel_cb, check);

		Evas_Object *btn_ok;
		btn_ok = elm_button_add(ly);
		elm_object_style_set(btn_ok, "default");
		evas_object_size_hint_weight_set(btn_ok, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_translatable_text_set(btn_ok, "IDS_WNOTI_BUTTON_OK_ABB2");
		elm_object_part_content_set(ly, "btn2", btn_ok);
		evas_object_smart_callback_add(btn_ok, "clicked", _set_watch_always_on_ok_clicked_cb, check);

		elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
		elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
		elm_naviframe_item_pop_cb_set(nf_it, _back_watch_always_on_naviframe_cb, ad);
	} else {
		/* disable watch always off with out popup */

		DBG("Cancel watch always on!", timeout);
		/*vconf_get_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_BACKUP_FOR_WATCH_ALWAYS_ON, &timeout); */
		vconf_set_int(VCONFKEY_SETAPPL_LCD_TIMEOUT_NORMAL, timeout);

		/*		elm_check_state_set(check,	EINA_FALSE); */
	}

}
/*
static void _display_gl_watch_always_on_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
	DBG("_display_gl_watch_always_on_cb is called!!!!!!!");
}
*/

static void _display_gl_display_noti_indicator_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
	DBG("_display_gl_display_noti_indicator_cb is called!!!!!!!");
	_show_noti_indicator_list(data);
}

void _noti_indicator_help_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = (appdata *) data;
	if (ad == NULL)
		return;

	Evas_Object *popup = NULL;

	popup = elm_popup_add(ad->nf);
	elm_object_style_set(popup, "circle");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->nf, popup);

	ad->popup = popup;

	char buf[1024];

	char *font_size_frame = "<font=Tizen><style=Thin><align=center>%s</align></style></font>";
	snprintf(buf, sizeof(buf) - 1, font_size_frame, "&nbsp;<br>Show a yellow indicator on the watch face when there are unread notifications.");

	Evas_Object *layout;
	layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "popup", "content/circle/buttons1");
	elm_object_part_text_set(layout, "elm.text.title", "Help");

	char *txt = strdup(buf);
	elm_object_text_set(layout, txt);
	elm_object_content_set(popup, layout);

	FREE(txt);

	evas_object_show(popup);
	back_button_cb_push(&back_key_popup_cb, data, NULL, g_noti_indicator_genlist, "g_noti_indicator_genlist");
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _hw_back_key_cb, NULL);

}

static void _display_gl_display_noti_indicator_help_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
	DBG("_display_gl_display_noti_indicator_help_cb is called!!!!!!!");
	_noti_indicator_help_popup_cb(data, obj, event_info);
}

static void _display_noti_indicator_check_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *check = obj;
	int nofi_indicator = 0;
	nofi_indicator = !elm_check_state_get(check);
	vconf_set_bool(VCONFKEY_SETAPPL_NOTIFICATION_INDICATOR, nofi_indicator);
	ERR("Setting VCONFKEY_SETAPPL_NOTIFICATION_INDICATOR : %d", nofi_indicator);
}

static Evas_Object *_gl_display_noti_indicator_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;
	Item_Data *id = (Item_Data *)data;


	if (!strcmp(part, "elm.icon") && id->index == 0) {
		int noti_indicator = 0;
		check = elm_check_add(obj);

		vconf_get_bool(VCONFKEY_SETAPPL_NOTIFICATION_INDICATOR , &noti_indicator);
		elm_object_style_set(check, "on&off");
		ERR("notification_indicator : %d", noti_indicator);

		elm_check_state_set(check, (noti_indicator) ? EINA_TRUE : EINA_FALSE);
		evas_object_event_callback_add(check, EVAS_CALLBACK_MOUSE_UP, _display_noti_indicator_check_cb, (void *)check);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_FALSE);

	}

	return check;
}


static char *_gl_noti_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Display_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(noti_menu_its[index].name));
	}
	return strdup(buf);
}

static void _noti_indicator_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	FREE(id);

	Elm_Object_Item *first = elm_genlist_first_item_get(g_noti_indicator_genlist);
	elm_object_item_signal_emit(first, "elm,action,title,slide,start", "elm");
}

static void gl_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	Elm_Object_Item *first = elm_genlist_first_item_get(g_noti_indicator_genlist);
	if (first == item)
		elm_object_item_signal_emit(first, "elm,action,title,slide,start", "elm");

}

static void _show_noti_indicator_list(void *data)
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
	Elm_Genlist_Item_Class *itc[2];
	itc[0] = elm_genlist_item_class_new();
	itc[0]->item_style = "1text.1icon.1";
	itc[0]->func.text_get = _gl_noti_title_get;
	itc[0]->func.content_get = _gl_display_noti_indicator_check_get;
	itc[0]->func.del = _noti_indicator_gl_del;

	itc[1] = elm_genlist_item_class_new();
	itc[1]->item_style = "1text";
	itc[1]->func.text_get = _gl_noti_title_get;
	itc[1]->func.del = _noti_indicator_gl_del;


	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	connect_to_wheel_with_genlist(genlist, ad);

	evas_object_smart_callback_add(genlist, "realized", gl_realized_cb, NULL);

	Elm_Genlist_Item_Class *title_item = elm_genlist_item_class_new();
	title_item->func.text_get = _gl_menu_title_text_get;
	title_item->item_style = "title";
	title_item->func.del = NULL;

	elm_genlist_item_append(genlist, title_item, (void *)DISPLAY_TITLE_NOTIFICATION_INDICATOR, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(title_item);

	for (idx = 0; idx < 2; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist, itc[idx], id, NULL,
											   ELM_GENLIST_ITEM_NONE, noti_menu_its[idx].func, ad);

		}
	}

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	elm_genlist_item_class_free(itc[0]);
	elm_genlist_item_class_free(itc[1]);

	Elm_Genlist_Item_Class *padding = elm_genlist_item_class_new();
	padding->item_style = "padding";
	padding->func.del = _noti_indicator_gl_del;
	g_noti_indicator_genlist = genlist;
	elm_genlist_item_append(genlist, padding, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(padding);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, "empty");
	back_button_cb_push(&back_key_generic_cb, data, NULL, g_display_genlist, "g_display_genlist");
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);

}

