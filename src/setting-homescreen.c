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
#include <app_manager.h>

#include <vconf.h>
#include <vconf-keys.h>
#include <system_settings.h>
#include <pkgmgr-info.h>

#include <media_content.h>
/*#include <media_info_comm_feature.h> */

#include <feedback.h>

#include "setting_data_vconf.h"
#include "setting-homescreen.h"
#include "setting_view_toast.h"
#include "util.h"

static struct _homescreen_menu_item homescreen_menu_its[] = {
	{ "IDS_HS_MBODY_HOME_ICON_SIZE_ABB",							0, 0,	_homescreen_gl_viewtype_cb		},
	{ "IDS_ST_MBODY_HOME_BACKGROUND_ABB",	0, 0,	_wallpaper_gl_cb		},
	{ "IDS_ST_OPT_EDIT_HOME_SCREEN_ABB",							0, 0,	_homescreen_gl_edit_home_cb		},
	{ NULL, 0, 0, NULL }
};

static struct _homebg_menu_item homebg_menu_its[] = {
	{ "IDS_COM_MBODY_COLOUR_PALETTE",		_show_bg_slide_cb	   },
	{ "IDS_ST_BODY_WALLPAPERS",				_wallpaper_gl_cb		 },
	{ "IDS_ST_BODY_GALLERY",				_gallery_gl_cb	},
	{ NULL, NULL }
};

static struct _color color[] = {
	{ 255, 111, 32, "FF6F20" },
	{ 244, 102, 141, "F4668D" },
	{ 249, 55, 66, "F93742" },
	{ 167, 63, 149, "A73F95" },
	{ 132, 0, 196, "8400C4"	 },
	{ 3, 54, 224, "0336E0" },
	{ 0, 175, 164, "00AFA4" },
	{ 96, 192, 30, "60C01E" },
	{ 31, 147, 0, "1F9300" },
	{ 107, 58, 51 , "6B3A33" },
	{ 48, 48, 48, "303030" },
	{ 0, 0, 0, "000000" },	/*default black */
};


static char *home_icon_sub_str[] = {
	"IDS_HS_OPT_LARGE_ICONS_ABB",
	"IDS_HS_OPT_SMALL_ICONS_ABB",
};

static char *thumb_path[] = {
	SETTING_DEFAULT_WALLPAPER_PATH"/btn_icons/setting_icon_color_platte.png",
	SETTING_DEFAULT_WALLPAPER_PATH"/btn_icons/setting_icon_wallpaper.png",
	CROPPED_GALLERY_DEFAULT_WALLPAPER_PATH,
	SETTING_DEFAULT_WALLPAPER_PATH"/wallpaper_01.png",
	SETTING_DEFAULT_WALLPAPER_PATH"/wallpaper_02.png",
	SETTING_DEFAULT_WALLPAPER_PATH"/wallpaper_03.png",
	SETTING_DEFAULT_WALLPAPER_PATH"/wallpaper_04.png",
	SETTING_DEFAULT_WALLPAPER_PATH"/wallpaper_05.png",
	SETTING_DEFAULT_WALLPAPER_PATH"/wallpaper_06.png"
};

static void _show_viewtype_list(void *data);
static void _show_homebg_list(void *data);
static void _show_edit_home(void *data);
static void _show_edit_apps(void *data);
static void _wallpaper_vconf_color_changed_cb(keynode_t *node, void *data);
static void _wallpaper_vconf_wallpaper_changed_cb(keynode_t *node, void *data);

static appdata *temp_ad = NULL;
static Evas_Object *g_home_bg_genlist = NULL;
static int touch_mode = NONE;
static int gallery_img_cnt = 0;
static int is_prev_update = 0;
static char *last_img_path = NULL;
static bool running_gallery = false;
static Ecore_Timer *running_gallery_timer = NULL;

static Evas_Object *g_wallpaper_layout = NULL;
static Evas_Object *g_wallpaper_scroller = NULL;
static Evas_Object *g_color_page = NULL;
static Evas_Object *g_gallery_prv = NULL;
static Evas_Object *g_wallpaper_box = NULL;
static bool wallpaper_touched = false;


#if 0  /* _NOT_USED_ */
static int _chk_pkg_install(const char *pkgid)
{
	int ret = 0;
	pkgmgrinfo_pkginfo_h pkginfo_h = NULL;

	/*get pkg appinfo handler */
	ret = pkgmgrinfo_pkginfo_get_pkginfo(pkgid, &pkginfo_h);
	if (PMINFO_R_OK != ret || NULL == pkginfo_h) {
		DBG("un-installed pkg [%s]", pkgid);
		return FALSE;
	}

	if (pkginfo_h) {
		pkgmgrinfo_pkginfo_destroy_pkginfo(pkginfo_h);
	}
	return TRUE;
}

static bool
_gallery_item_cb(media_info_h item, void *user_data)
{
	if (user_data == NULL) {
		DBG("user_data == NULL");
		return false;
	}
	media_info_get_file_path(item, &last_img_path);
	/*DBG("last gallery image path[%s]", last_img_path); */

	return false;
}
#endif

static void _get_last_img_path()
{
#if 0
	Eina_List *item_list = NULL;
#endif
	filter_h media_filter;

	int ret;
	ret = media_filter_create(&media_filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		DBG("Cannot create filter. err");
	}

	ret = media_filter_set_condition(media_filter, "((MEDIA_TYPE=0 OR MEDIA_TYPE=1) AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121))", MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		DBG("Cannot set condition. err");
	}

	ret = media_filter_set_order(media_filter, MEDIA_CONTENT_ORDER_DESC, "MEDIA_TIMELINE, MEDIA_DISPLAY_NAME", MEDIA_CONTENT_COLLATE_NOCASE);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		DBG("media_filter_set_order is failed, err");
	}

	media_content_connect();

#if 0
	ret = media_info_foreach_media_from_db_with_media_mode(media_filter, _gallery_item_cb, &item_list);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		DBG("Cannot retrive data err[%d]", ret);
	}
#endif
	media_content_disconnect();
	media_filter_destroy(media_filter);
}


void _clear_homescreen_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	unregister_vconf_changing("db/wms/home_bg_palette", _wallpaper_vconf_color_changed_cb);
	unregister_vconf_changing("db/wms/home_bg_mode", _wallpaper_vconf_wallpaper_changed_cb);
	if (is_prev_update) {
		Evas_Object *layout = NULL;
		Elm_Object_Item *nf_it = NULL;
		layout = create_wallpaper_list(temp_ad);
		elm_naviframe_item_pop(temp_ad->nf);
		nf_it = elm_naviframe_item_push(temp_ad->nf, NULL, NULL, NULL, layout, NULL);
		elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
		is_prev_update = 0;
	} else {
		temp_ad = NULL;
		touch_mode = NONE;
	}
	int ret;
	ret = feedback_deinitialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		DBG("feedback_deinitialize failed");
	}
}

/* 1: view type */
void _homescreen_gl_viewtype_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (data != NULL) {
		_show_viewtype_list(data);
	}
}

/* 2: home background */
void _homescreen_gl_homebg_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (data != NULL) {
		_show_homebg_list(data);
	}
}

/* 3: Edit home screen */
void _homescreen_gl_edit_home_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (data != NULL) {
		_show_edit_home(data);
	}
}

/* 4: Edit apps */
void _homescreen_gl_edit_apps_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	if (data != NULL) {
		_show_edit_apps(data);
	}
}

/* homescreen main list */
char *_gl_homescreen_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Homescreen_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(homescreen_menu_its[index].name));
		index++;
	}
	return strdup(buf);
}

Evas_Object *_gl_homescreen_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	Homescreen_Item_Data *id = data;
	int index = id->index;

	if (homescreen_menu_its[index % (HOME_MENU_COUNT)].is_check_type && !strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);
		elm_check_state_set(check, (homescreen_menu_its[index % (HOME_MENU_COUNT)].state) ? EINA_TRUE : EINA_FALSE);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		index++;
	}

	return check;
}

void _homescreen_gl_del(void *data, Evas_Object *obj)
{
	Homescreen_Item_Data *id = data;
	if (id)
		free(id);
}

Evas_Object *_create_homescreen_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_homescreen_list - appdata is null");
		return NULL;
	}
	Evas_Object *genlist  = NULL;
	struct _homescreen_menu_item *menu_its = NULL;
	int idx = 0;

	temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text";
	itc->func.text_get = _gl_homescreen_title_get;
	itc->func.del = _homescreen_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist, ad);

	menu_its = homescreen_menu_its;

	for (idx = 0; idx < HOME_MENU_COUNT; idx++) {
		Homescreen_Item_Data *id = calloc(sizeof(Homescreen_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
						   genlist,			/* genlist object */
						   itc,				/* item class */
						   id,					/* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_its[idx].func,	/* call back */
						   ad);
		}
	}
	elm_genlist_item_class_free(itc);
	elm_object_part_content_set(layout, "elm.genlist", genlist);
	return layout;
}

/* home view type */
char *_get_homeview_type_subtitle()
{
	int value = 0;
	vconf_get_int(VCONFKEY_SETAPPL_HOMESCREEN_TYPE_INT, &value);
	DBG("VCONFKEY_SETAPPL_HOMESCREEN_TYPE_INT : %d", value);

	char *substr = NULL;
	substr = strdup(_(home_icon_sub_str[value]));

	return substr;
}

static char *_gl_viewtype_title_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	char *title = NULL;
	if (!strcmp(part, "elm.text")) {
		if (!id->index) {
			char buf[1024];
			snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_HS_OPT_LARGE_ICONS_ABB"));
			title = strdup(buf);
		} else	{
			char buf[1024];
			snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_HS_OPT_SMALL_ICONS_ABB"));
			title = strdup(buf);
		}
	}
	return title;
}

static Evas_Object *_gl_viewtype_radio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Item_Data *id = data;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_object_style_set(radio, "list");
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(radio, EINA_FALSE);
		evas_object_repeat_events_set(radio, EINA_TRUE);
		elm_radio_group_add(radio, radio_main);
	}

	return radio;
}

static void _viewtype_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}

static void _viewtype_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	int idx = (int)data;
	vconf_set_int(VCONFKEY_SETAPPL_HOMESCREEN_TYPE_INT, idx);

	/*elm_genlist_realized_items_update(g_screen_time_genlist); */
	elm_naviframe_item_pop(temp_ad->nf);
	if (!temp_ad->homescreen_rdg) {
		evas_object_del(temp_ad->homescreen_rdg);
		temp_ad->homescreen_rdg = NULL;
	}
}

void _show_viewtype_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_viewtype_list - appdata is null");
		return;
	}
	Evas_Object *layout = NULL;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	int idx;
	int value = 0;

	temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_viewtype_title_get;
	itc->func.content_get = _gl_viewtype_radio_get;
	itc->func.del = _viewtype_gl_del;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	connect_to_wheel_with_genlist(genlist, ad);

	for (idx = 0; idx < VIEWTYPE_COUNT; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _viewtype_gl_cb, (void *)idx);
		}
	}

	ad->homescreen_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->homescreen_rdg, -1);

	vconf_get_int(VCONFKEY_SETAPPL_HOMESCREEN_TYPE_INT, &value);
	elm_radio_value_set(ad->homescreen_rdg, value);
	evas_object_data_set(genlist, "radio_main", ad->homescreen_rdg);

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	elm_genlist_item_class_free(itc);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}

/* home background */
static char *_gl_homebg_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(homebg_menu_its[id->index].name));
	}
	return strdup(buf);
}

static void _homebg_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}

static void _show_homebg_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_homebg_list - appdata is null");
		return;
	}
	Evas_Object *genlist  = NULL;
	Elm_Object_Item *nf_it = NULL;
	struct _homebg_menu_item *menu_its = NULL;
	int idx;

	temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text";
	itc->func.text_get = _gl_homebg_title_get;
	itc->func.del = _homebg_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	connect_to_wheel_with_genlist(genlist, ad);

	menu_its = homebg_menu_its;

	for (idx = 0; idx < HOME_BG_LIST_COUNT; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, menu_its[idx].func, ad);
		}
	}

	g_home_bg_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	elm_genlist_item_class_free(itc);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}

static void _layout_del_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	page_data *pd = data;
	free(pd);
}

#if 0 /* _NOT_USED_ */
static Eina_Bool animator_cb(void *data)
{
	page_data *pd = (page_data *)data;

	/*Since mapbuf is not perfect, Enable them after the size calculation is finished */
	int idx;
	for (idx = 0; idx < BG_COLOR_COUNT / WALLPAPER_CNT_PER_PAGE; idx++) {
		elm_mapbuf_enabled_set(pd->mapbuf[idx], EINA_TRUE);
	}
	return ECORE_CALLBACK_CANCEL;
}
#endif

static int _set_bg_color_type(int type_num)
{
	if (type_num < 0 || type_num > BG_COLOR_COUNT - 1)
		return FALSE;

	/*set color -> set mode */
	vconf_set_str("db/wms/home_bg_palette", color[type_num].hex);	/*ex. fcb800 */
	vconf_set_int("db/wms/home_bg_mode", 0);

	/*vconf_set_int(VCONFKEY_SETAPPL_HOME_BG_COLOR_TYPE_INT, type_num); */
	DBG("color [%s]", color[type_num].hex);

	return TRUE;
}

static int _set_wallpaper_path(int thumbIdx)
{
	if (thumbIdx < NUM_DEFAULT_THUMB_BUTTON || thumbIdx > DEFAULT_WALLPAPER_COUNT + NUM_DEFAULT_THUMB_BUTTON)
		return FALSE;

	if (system_settings_set_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, thumb_path[thumbIdx]) != SYSTEM_SETTINGS_ERROR_NONE) {
		DBG("Homescreen set Error : %s", thumb_path[thumbIdx]);
		return FALSE;
	}

	/*caution : set mode after change wallpaper */
	if (gallery_img_cnt && thumbIdx == NUM_DEFAULT_THUMB_BUTTON) {
		/*2:gallery */
		vconf_set_str("db/wms/home_bg_wallpaper", ".bgwallpaper.jpg");
		vconf_set_int("db/wms/home_bg_mode", 2);
	} else {
		/*1:wallpaper */
		char *wallpaper_name = NULL;
		wallpaper_name = strstr(thumb_path[thumbIdx], "wallpaper_");
		vconf_set_str("db/wms/home_bg_wallpaper", wallpaper_name);
		/*DBG("wallpaper_name[%s]",wallpaper_name); */

		vconf_set_int("db/wms/home_bg_mode", 1);
	}

	return TRUE;
}


static int prev_x = 0;

static void _mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	if (running_gallery == true) {
		touch_mode = NONE;
		return;
	}

	touch_mode = TOUCH_DOWN;
	Evas_Event_Mouse_Down *ev = event_info;
	prev_x = ev->canvas.x;
}

static void _mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	if (touch_mode == TOUCH_MOVE)
		return;

	int type = (int)data;

	DBG("Setting - BG type is %d", type);

	prev_x = 0;
	touch_mode = NONE;

	feedback_play(FEEDBACK_PATTERN_TAP);
	/* set gb vconf */
	if (_set_bg_color_type(type)) {
		if (temp_ad != NULL) {
			elm_naviframe_item_pop(temp_ad->nf);
		}

		/* automatic freed!! */
		struct _toast_data *toast = _create_toast(temp_ad, REPL(_("IDS_GALLERY_TPOP_HOME_BACKGROUND_SET"), "\n", "<br>"));
		if (toast) {
			_show_toast(temp_ad, toast);
		}
	} else {
		DBG("Setting - BG type is wrong!!");
	}
}

static void _mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	if (running_gallery == true) {
		touch_mode = NONE;
		return;
	}

	Evas_Event_Mouse_Move *ev = event_info;

	int cur_x = ev->cur.canvas.x;

	if (abs(cur_x - prev_x) > 15) {
		/* todo : confirm and remove */
		touch_mode = TOUCH_MOVE;
	}
}

static void _mouse_up_wallpaper_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	if (running_gallery == true) {
		touch_mode = NONE;
		return;
	}

	if (touch_mode == TOUCH_MOVE)
		return;

	int idx = (int)data;

	DBG("Setting - wallpaper idx is %d", idx);

	prev_x = 0;
	touch_mode = NONE;

	/* set gb vconf */
	if (idx == 0) {
		feedback_play(FEEDBACK_PATTERN_TAP);
		_show_bg_slide_cb(temp_ad, obj, event_info);
		return;
	} else if (idx == 1) {
		feedback_play(FEEDBACK_PATTERN_TAP);
		_gallery_gl_cb(temp_ad, obj, event_info);
		running_gallery = true;
		elm_object_scroll_freeze_push(obj);
		return;
	}

	if (_set_wallpaper_path(idx)) {
		feedback_play(FEEDBACK_PATTERN_TAP);
		wallpaper_touched = true;
		if (temp_ad != NULL) {
			elm_naviframe_item_pop(temp_ad->nf);
		}

		/* automatic freed!! */
		struct _toast_data *toast = _create_toast(temp_ad, REPL(_("IDS_GALLERY_TPOP_HOME_BACKGROUND_SET"), "\n", "<br>"));
		if (toast) {
			_show_toast(temp_ad, toast);
		}

	} else {
		DBG("Setting - wallpaper path is wrong!!");
	}
}

static void _page_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	int page;
#if 0
	int idx = 0;
	vconf_get_int(VCONFKEY_SETAPPL_HOME_BG_COLOR_TYPE_INT, &idx);
	page = (idx - 1) / WALLPAPER_CNT_PER_PAGE;
#endif
	/*first page show */
	page = 0;
	elm_scroller_page_show(obj, page, 0);
}

static Evas_Object *_create_index(Evas_Object *parent)
{
	Evas_Object *layout, *scroller, *box, *page_layout, *page;
	int index;

	if (parent == NULL)
		return NULL;

	/* Create Layout */
	layout = elm_layout_add(parent);
	if (layout == NULL)
		return NULL;

	page_data *pd = calloc(1, sizeof(page_data));
	if (pd == NULL)
		return NULL;

	elm_layout_file_set(layout, EDJE_PATH, "setting-test/index");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _layout_del_cb, pd);

	/* Create Scroller */
	scroller = elm_scroller_add(layout);
	elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	/*elm_scroller_page_relative_set(scroller, 1.0, 0.0); */
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_page_scroll_limit_set(scroller, 1, 0);
	elm_object_scroll_lock_y_set(scroller, EINA_TRUE);
	elm_scroller_page_size_set(scroller, WIN_SIZE, WIN_SIZE);
	elm_object_part_content_set(layout, "scroller", scroller);
	elm_object_style_set(scroller, "effect");
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
	evas_object_show(scroller);

	/* Create Box */
	box = elm_box_add(scroller);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_object_content_set(scroller, box);
	evas_object_show(box);


	int totalPageCnt = 0;
	totalPageCnt = (BG_COLOR_COUNT + 1) / NUM_MAX_THUMB_IN_PAGES;
	for (index = 0; index < totalPageCnt; index++) {
		page_layout = elm_layout_add(box);
		evas_object_size_hint_weight_set(page_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(page_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

		elm_layout_file_set(page_layout, EDJE_PATH, "thumbnail_page");
		evas_object_show(page_layout);

		int thumbIdx = 0;
		for (thumbIdx = index * NUM_MAX_THUMB_IN_PAGES; thumbIdx < ((index * NUM_MAX_THUMB_IN_PAGES) + NUM_MAX_THUMB_IN_PAGES); thumbIdx++) {
			Evas_Object *thumbnail_layout = elm_layout_add(page_layout);
			evas_object_size_hint_weight_set(thumbnail_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(thumbnail_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_layout_file_set(thumbnail_layout, EDJE_PATH, "thumbnail");
			evas_object_show(thumbnail_layout);

			/* page content */
			page = evas_object_rectangle_add(evas_object_evas_get(page_layout));
			evas_object_color_set(page, color[thumbIdx].r, color[thumbIdx].g, color[thumbIdx].b, 255);
			evas_object_size_hint_weight_set(page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(page, EVAS_HINT_FILL, EVAS_HINT_FILL);

			elm_object_part_content_set(thumbnail_layout, "thumb", page);
			evas_object_show(page);

			switch (thumbIdx % NUM_MAX_THUMB_IN_PAGES) {
			case 0:
				elm_object_part_content_set(page_layout, "thumb1", thumbnail_layout);
				break;
			case 1:
				elm_object_part_content_set(page_layout, "thumb2", thumbnail_layout);
				break;
			}

			/* touch event */
			evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, NULL);
			evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, (void *)(thumbIdx));
			evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, NULL);
		}
		elm_box_pack_end(box, page_layout);
	}

	evas_object_event_callback_add(scroller, EVAS_CALLBACK_RESIZE, _page_show, NULL);

	/*ecore_animator_add(animator_cb, pd); */

	return layout;
}

#if 0 /* _NOT_USED_ */
static void
_wallpaper_layout_del_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	wallpaper_page_data *pd = data;
	free(pd);
}

static Eina_Bool
wallpaper_animator_cb(void *data)
{
	wallpaper_page_data *pd = (wallpaper_page_data *)data;

	/*Since mapbuf is not perfect, Enable them after the size calculation is finished */
	int idx;
	for (idx = 0; idx < DEFAULT_WALLPAPER_COUNT / WALLPAPER_CNT_PER_PAGE; idx++) {
		elm_mapbuf_enabled_set(pd->mapbuf[idx], EINA_TRUE);
	}
	return ECORE_CALLBACK_CANCEL;
}
#endif

static void _wallpaper_page_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("_wallpaper_page_show");
#if 0
	char *ret_wallpaper = NULL;
	int idx = 0;
	int page_idx = 0;
	if (system_settings_get_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, &ret_wallpaper) != SYSTEM_SETTINGS_ERROR_NONE) {
		page_idx = idx;
	} else {
		for (idx; idx < DEFAULT_WALLPAPER_COUNT; idx++) {
			if (ret_wallpaper && !strcmp(ret_wallpaper, thumb_path[idx])) {
				DBG("wallpaper[%s] idx [%d]", ret_wallpaper, idx);
				page_idx = idx / WALLPAPER_CNT_PER_PAGE;
				break;
			}
		}
	}
#endif
	/*set just first page to show */
	int page_idx = 0;
	elm_scroller_page_show(obj, page_idx, 0);
}

static void _update_wallpaper()
{
	Evas_Object *page_layout, *thumbnail, *color_page, *gallery_page, *touch_eo;
	Evas_Object *box = g_wallpaper_box;

	if (box && !wallpaper_touched) {
		elm_box_clear(box);
		int totalPageCnt = 0;
		totalPageCnt = (DEFAULT_WALLPAPER_COUNT + NUM_DEFAULT_THUMB_BUTTON + gallery_img_cnt + 1) / NUM_MAX_THUMB_IN_PAGES;
		int index;
		for (index = 0; index < totalPageCnt; index++) {
			page_layout = elm_layout_add(box);
			evas_object_size_hint_weight_set(page_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(page_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

			elm_layout_file_set(page_layout, EDJE_PATH, "thumbnail_page");
			evas_object_show(page_layout);

			int thumbIdx = 0;
			for (thumbIdx = index * NUM_MAX_THUMB_IN_PAGES; thumbIdx < ((index * NUM_MAX_THUMB_IN_PAGES) + NUM_MAX_THUMB_IN_PAGES); thumbIdx++) {
				DBG("total : %d, thumbIdx : %d", DEFAULT_WALLPAPER_COUNT + NUM_DEFAULT_THUMB_BUTTON + gallery_img_cnt + 1, thumbIdx);
				if (DEFAULT_WALLPAPER_COUNT + NUM_DEFAULT_THUMB_BUTTON + gallery_img_cnt == thumbIdx) {
					DBG("check break");
					break;
				}
				Evas_Object *thumbnail_layout = elm_layout_add(page_layout);
				evas_object_size_hint_weight_set(thumbnail_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				evas_object_size_hint_align_set(thumbnail_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
				elm_layout_file_set(thumbnail_layout, EDJE_PATH, "thumbnail");
				evas_object_show(thumbnail_layout);
				/*elm_object_signal_emit(thumbnail_layout, "thumb_bg,white", "thumb_bg"); */

				/* color palette - set color box */
				if (thumbIdx == 0) {
					char *bg_color = NULL;
					int R = 0x00, G = 0x00, B = 0x00;
					bg_color = vconf_get_str("db/wms/home_bg_palette");
					colorstr_to_decimal(bg_color, &R, &G, &B);
					DBG("R : [%d] G : [%d] B : [%d]", R, G, B);
					color_page = evas_object_rectangle_add(evas_object_evas_get(page_layout));
					evas_object_color_set(color_page, R, G, B, 255);
					evas_object_size_hint_weight_set(color_page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
					evas_object_size_hint_align_set(color_page, EVAS_HINT_FILL, EVAS_HINT_FILL);
					elm_object_part_content_set(thumbnail_layout, "thumb", color_page);
					evas_object_show(color_page);
					g_color_page = color_page;
				}

				if (thumbIdx == 1) {
					_get_last_img_path();
					gallery_page = elm_image_add(page_layout);
					elm_image_file_set(gallery_page, last_img_path, NULL);
					elm_object_part_content_set(thumbnail_layout, "thumb", gallery_page);
					evas_object_show(gallery_page);
				}

				/* page content */
				char buf[256];
				thumbnail = elm_image_add(page_layout);/*thumb_path */

				int idx_arr;
				/*default 1,2page thumbnail button : pallette btn, gallery btn */
				if (gallery_img_cnt || thumbIdx == 0 || thumbIdx == 1) {
					idx_arr = thumbIdx;
				} else {
					idx_arr = thumbIdx + 1;
				}

				snprintf(buf, sizeof(buf), "%s", thumb_path[idx_arr]);

				if (ecore_file_exists(buf)) {
					elm_image_file_set(thumbnail, buf, NULL);
				} else {
					elm_image_file_set(thumbnail, NULL, NULL);
				}
				elm_image_aspect_fixed_set(thumbnail, EINA_FALSE);
				elm_image_resizable_set(thumbnail, EINA_TRUE, EINA_TRUE);

				if (thumbIdx == 0 || thumbIdx == 1)
					elm_object_part_content_set(thumbnail_layout, "thumb_btn", thumbnail);
				else
					elm_object_part_content_set(thumbnail_layout, "thumb", thumbnail);

				if (thumbIdx == 2 && gallery_img_cnt) {
					g_gallery_prv = thumbnail;
				}
				evas_object_show(thumbnail);

				/* select page */
				int bg_mode;
				vconf_get_int("db/wms/home_bg_mode", &bg_mode);

				if (bg_mode) {
					Evas_Object *selected_rect = elm_image_add(page_layout);
					elm_image_file_set(selected_rect, SETTING_DEFAULT_WALLPAPER_PATH"/btn_icons/settings_wallpaper_selected.png", NULL);
					elm_image_aspect_fixed_set(selected_rect, EINA_FALSE);
					elm_image_resizable_set(selected_rect, EINA_TRUE, EINA_TRUE);
					elm_object_part_content_set(thumbnail_layout, "thumb_select", selected_rect);
					evas_object_show(selected_rect);
				}

				switch (thumbIdx % NUM_MAX_THUMB_IN_PAGES) {
				case 0:
					elm_object_part_content_set(page_layout, "thumb1", thumbnail_layout);
					break;
				case 1:
					elm_object_part_content_set(page_layout, "thumb2", thumbnail_layout);
					break;
				}

				/* touch event */
				if (thumbIdx == 0) {
					touch_eo = color_page;
					elm_object_signal_emit(thumbnail_layout, "thumbnail,opacity", "thumb_op");
				} else if (thumbIdx == 1) {
					touch_eo = gallery_page;
					elm_object_signal_emit(thumbnail_layout, "thumbnail,opacity", "thumb_op");
				} else {
					touch_eo = thumbnail;
					elm_object_signal_emit(thumbnail_layout, "thumbnail,default", "thumb_op");
				}

				evas_object_event_callback_add(touch_eo, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, NULL);
				evas_object_event_callback_add(touch_eo, EVAS_CALLBACK_MOUSE_UP, _mouse_up_wallpaper_cb, (void *)idx_arr);
				evas_object_event_callback_add(touch_eo, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, NULL);
			}
			elm_box_pack_end(box, page_layout);
		}
		/*_wallpaper_page_show(NULL, NULL, g_wallpaper_scroller, NULL); */
	}
}


#if 0 /* _NOT_USED_ */
static void _wallpaper_page_refresh(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("is_prev_update , %d", is_prev_update);
}
#endif

bool _is_file_exist(const char *filepath)
{
	if (ecore_file_exists(filepath) == EINA_TRUE) {
		DBG("exist");
		return true;
	}
	DBG("not exist %s", filepath);
	return false;
}

static void _wallpaper_vconf_color_changed_cb(keynode_t *node, void *data)
{
	char *bg_color = NULL;
	int R = 0x00, G = 0x00, B = 0x00;
	bg_color = vconf_get_str("db/wms/home_bg_palette");
	colorstr_to_decimal(bg_color, &R, &G, &B);
	evas_object_color_set(g_color_page, R, G, B, 255);
	evas_object_show(g_color_page);
}

static void _wallpaper_vconf_wallpaper_changed_cb(keynode_t *node, void *data)
{
	DBG("_wallpaper_vconf_wallpaper_changed_cb");
	int bg_mode;
	vconf_get_int("db/wms/home_bg_mode", &bg_mode);
	if (bg_mode == 2 && !gallery_img_cnt) {
		DBG("gallery_img_cnt : %d", gallery_img_cnt);
		is_prev_update = 1;
		if (g_wallpaper_layout) {
			evas_object_del(g_wallpaper_layout);
			g_wallpaper_layout = NULL;
		}
	}
	if (bg_mode == 2 && gallery_img_cnt) {
		/*gallery type */
		/*char *bg_gallery = NULL; */
		/*bg_gallery = vconf_get_str(VCONFKEY_BGSET); */
		/*DBG(" bg mode 2 , gallery true , [%s]", bg_gallery); */
		/*elm_image_file_set(g_gallery_prv, bg_gallery, NULL); */
		/*evas_object_show(g_gallery_prv); */
		_update_wallpaper();
	}

	if (bg_mode == 0 || bg_mode == 1) {
		_update_wallpaper();
	}
	wallpaper_touched = false;
}

static Evas_Object *_create_wallpaper_thumbnail(Evas_Object *parent)
{
	Evas_Object *layout, *scroller, *box, *page_layout, *thumbnail, *color_page, *gallery_page;
	int index;

	Evas_Object *touch_eo = NULL;

	if (parent == NULL)
		return NULL;

	if (_is_file_exist(CROPPED_GALLERY_DEFAULT_WALLPAPER_PATH) == true) {
		/*cropped gallery image exist or not */
		gallery_img_cnt = 1;	/*fixed on 1page */
	}


	/* Create Layout */
	layout = elm_layout_add(parent);
	if (layout == NULL)
		return NULL;

	elm_layout_file_set(layout, EDJE_PATH, "setting-test/index");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);
	g_wallpaper_layout = layout;

	/* Create Scroller */
	scroller = elm_scroller_add(layout);
	elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	/*elm_scroller_page_relative_set(scroller, 1.0, 0.0); */
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_page_scroll_limit_set(scroller, 1, 0);
	elm_object_scroll_lock_y_set(scroller, EINA_TRUE);
	elm_scroller_page_size_set(scroller, WIN_SIZE, WIN_SIZE);
	elm_object_part_content_set(layout, "scroller", scroller);
	elm_object_style_set(scroller, "effect");
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
	evas_object_show(scroller);
	g_wallpaper_scroller = scroller;

	/* Create Box */
	box = elm_box_add(scroller);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_object_content_set(scroller, box);
	evas_object_show(box);
	g_wallpaper_box = box;

	int totalPageCnt = 0;
	totalPageCnt = (DEFAULT_WALLPAPER_COUNT + NUM_DEFAULT_THUMB_BUTTON + gallery_img_cnt + 1) / NUM_MAX_THUMB_IN_PAGES;
	for (index = 0; index < totalPageCnt; index++) {
		page_layout = elm_layout_add(box);
		evas_object_size_hint_weight_set(page_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(page_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

		elm_layout_file_set(page_layout, EDJE_PATH, "thumbnail_page");
		evas_object_show(page_layout);

		int thumbIdx = 0;
		for (thumbIdx = index * NUM_MAX_THUMB_IN_PAGES; thumbIdx < ((index * NUM_MAX_THUMB_IN_PAGES) + NUM_MAX_THUMB_IN_PAGES); thumbIdx++) {
			DBG("total : %d, thumbIdx : %d", DEFAULT_WALLPAPER_COUNT + NUM_DEFAULT_THUMB_BUTTON + gallery_img_cnt + 1, thumbIdx);
			if (DEFAULT_WALLPAPER_COUNT + NUM_DEFAULT_THUMB_BUTTON + gallery_img_cnt == thumbIdx) {
				DBG("check break");
				break;
			}
			Evas_Object *thumbnail_layout = elm_layout_add(page_layout);
			evas_object_size_hint_weight_set(thumbnail_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(thumbnail_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_layout_file_set(thumbnail_layout, EDJE_PATH, "thumbnail");
			evas_object_show(thumbnail_layout);
			/*elm_object_signal_emit(thumbnail_layout, "thumb_bg,white", "thumb_bg"); */

			/* color palette - set color box */
			if (thumbIdx == 0) {
				char *bg_color = NULL;
				int R = 0x00, G = 0x00, B = 0x00;
				bg_color = vconf_get_str("db/wms/home_bg_palette");
				colorstr_to_decimal(bg_color, &R, &G, &B);
				DBG("R : [%d] G : [%d] B : [%d]", R, G, B);
				color_page = evas_object_rectangle_add(evas_object_evas_get(page_layout));
				evas_object_color_set(color_page, R, G, B, 255);
				evas_object_size_hint_weight_set(color_page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				evas_object_size_hint_align_set(color_page, EVAS_HINT_FILL, EVAS_HINT_FILL);
				elm_object_part_content_set(thumbnail_layout, "thumb", color_page);
				evas_object_show(color_page);
				g_color_page = color_page;
			}

			if (thumbIdx == 1) {
				_get_last_img_path();
				gallery_page = elm_image_add(page_layout);
				elm_image_file_set(gallery_page, last_img_path, NULL);
				elm_object_part_content_set(thumbnail_layout, "thumb", gallery_page);
				evas_object_show(gallery_page);
			}

			/* page content */
			char buf[256];
			thumbnail = elm_image_add(page_layout);/*thumb_path */

			int idx_arr;
			/*default 1,2page thumbnail button : pallette btn, gallery btn */
			if (gallery_img_cnt || thumbIdx == 0 || thumbIdx == 1) {
				idx_arr = thumbIdx;
			} else {
				idx_arr = thumbIdx + 1;
			}

			snprintf(buf, sizeof(buf), "%s", thumb_path[idx_arr]);

			if (ecore_file_exists(buf)) {
				elm_image_file_set(thumbnail, buf, NULL);
			} else {
				elm_image_file_set(thumbnail, NULL, NULL);
			}
			elm_image_aspect_fixed_set(thumbnail, EINA_FALSE);
			elm_image_resizable_set(thumbnail, EINA_TRUE, EINA_TRUE);

			if (thumbIdx == 0 || thumbIdx == 1)
				elm_object_part_content_set(thumbnail_layout, "thumb_btn", thumbnail);
			else
				elm_object_part_content_set(thumbnail_layout, "thumb", thumbnail);

			if (thumbIdx == 2 && gallery_img_cnt) {
				g_gallery_prv = thumbnail;
			}
			evas_object_show(thumbnail);

			/* select page */
			int bg_mode;
			vconf_get_int("db/wms/home_bg_mode", &bg_mode);

			if (bg_mode) {
				Evas_Object *selected_rect = elm_image_add(page_layout);
				elm_image_file_set(selected_rect, SETTING_DEFAULT_WALLPAPER_PATH"/btn_icons/settings_wallpaper_selected.png", NULL);
				elm_image_aspect_fixed_set(selected_rect, EINA_FALSE);
				elm_image_resizable_set(selected_rect, EINA_TRUE, EINA_TRUE);
				elm_object_part_content_set(thumbnail_layout, "thumb_select", selected_rect);
				evas_object_show(selected_rect);
			}

			switch (thumbIdx % NUM_MAX_THUMB_IN_PAGES) {
			case 0:
				elm_object_part_content_set(page_layout, "thumb1", thumbnail_layout);
				break;
			case 1:
				elm_object_part_content_set(page_layout, "thumb2", thumbnail_layout);
				break;
			}

			/* touch event */
			if (thumbIdx == 0) {
				touch_eo = color_page;
				elm_object_signal_emit(thumbnail_layout, "thumbnail,opacity", "thumb_op");
			} else if (thumbIdx == 1) {
				touch_eo = gallery_page;
				elm_object_signal_emit(thumbnail_layout, "thumbnail,opacity", "thumb_op");
			} else {
				touch_eo = thumbnail;
				elm_object_signal_emit(thumbnail_layout, "thumbnail,default", "thumb_op");
			}

			evas_object_event_callback_add(touch_eo, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, NULL);
			evas_object_event_callback_add(touch_eo, EVAS_CALLBACK_MOUSE_UP, _mouse_up_wallpaper_cb, (void *)idx_arr);
			evas_object_event_callback_add(touch_eo, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, NULL);
		}
		elm_box_pack_end(box, page_layout);
	}

	evas_object_event_callback_add(scroller, EVAS_CALLBACK_RESIZE, _wallpaper_page_show, NULL);
	register_vconf_changing("db/wms/home_bg_palette", _wallpaper_vconf_color_changed_cb, NULL);
	register_vconf_changing("db/wms/home_bg_mode", _wallpaper_vconf_wallpaper_changed_cb, NULL);

	return layout;
}

static Eina_Bool _wallpaper_pop_cb(void *data, Elm_Object_Item *it)
{
	DBG("_wallpaper_pop_cb");
	unregister_vconf_changing("db/wms/home_bg_palette", _wallpaper_vconf_color_changed_cb);
	unregister_vconf_changing("db/wms/home_bg_mode", _wallpaper_vconf_wallpaper_changed_cb);
	return EINA_TRUE;
}

void _wallpaper_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	appdata *ad;
	Evas_Object *layout_inner;
	Elm_Object_Item *it;

	ad = (appdata *)data;
	if (ad == NULL)
		return;

	temp_ad = ad;

	layout_inner = _create_wallpaper_thumbnail(ad->nf);
	it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout_inner, NULL);
	elm_naviframe_item_pop_cb_set(it, _wallpaper_pop_cb, ad);
	elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
}

Evas_Object *create_wallpaper_list(void *data)
{
	appdata *ad;
	Evas_Object *layout_inner;

	ad = (appdata *)data;
	if (ad == NULL)
		return NULL;

	temp_ad = ad;

	int ret;
	ret = feedback_initialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		DBG("feedback_initialize failed");
	}

	layout_inner = _create_wallpaper_thumbnail(ad->nf);
	return layout_inner;
}

void _gallery_result_cb(app_control_h service, app_control_h reply, app_control_result_e result, void *data)
{
	DBG("_gallery_result_cb");
	if (running_gallery) {
		running_gallery = false;
		elm_object_scroll_freeze_pop(data);
	}
}

static Eina_Bool _timer_cb(void *data)
{
	DBG("reset gallery running flag");
	if (running_gallery) {
		running_gallery = false;
		running_gallery_timer = NULL;
		elm_object_scroll_freeze_pop(data);
	}
	return ECORE_CALLBACK_CANCEL;
}

void _gallery_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	/*elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE); */
	DBG("_gallery_gl_cb");

	appdata *ad;

	ad = (appdata *)data;
	if (ad == NULL)
		return;

	/*app_manager_is_running("org.tizen.w-gallery.appcontrol.setting_wallpaper", &running); */
	if (!running_gallery) {
		app_control_h service;
		app_control_create(&service);
		app_control_set_app_id(service, "org.tizen.w-gallery.appcontrol.setting_wallpaper");
		app_control_send_launch_request(service, _gallery_result_cb, obj);
		app_control_destroy(service);

		if (running_gallery_timer) {
			ecore_timer_del(running_gallery_timer);
			running_gallery_timer = NULL;
		}
		running_gallery_timer = ecore_timer_add(1.0, (Ecore_Task_Cb)_timer_cb, obj);
	}
}

void _show_bg_slide_cb(void *data, Evas_Object *obj, void *event_info)
{
	/*elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE); */

	appdata *ad;
	Evas_Object *layout_inner;
	Elm_Object_Item *it;

	ad = (appdata *)data;
	if (ad == NULL)
		return;

	temp_ad = ad;

	layout_inner = _create_index(ad->nf);
	it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout_inner, NULL);
	elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
}

/* edit home screen */
static void _show_edit_home(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_edit_home - appdata is null");
		return;
	}

	app_control_h service;
	app_control_create(&service);
	app_control_set_app_id(service, "org.tizen.w-launcher-app");
	app_control_add_extra_data(service, "home_op", "edit");
	app_control_send_launch_request(service, NULL, NULL);
	app_control_destroy(service);
}

/* edit home screen */
static void _show_edit_apps(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_show_edit_home - appdata is null");
		return;
	}

	app_control_h service;
	app_control_create(&service);
	app_control_set_app_id(service, "org.tizen.w-launcher-app");
	app_control_add_extra_data(service, "home_op", "apps_edit");
	app_control_send_launch_request(service, NULL, NULL);
	app_control_destroy(service);
}

