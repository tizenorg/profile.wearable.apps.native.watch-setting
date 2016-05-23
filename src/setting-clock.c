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
#include <vconf.h>
#include <vconf-keys.h>
#include <unicode/ucal.h>
#include <unicode/uloc.h>
#include <unicode/udat.h>
#include <unicode/ustring.h>
#include <unicode/udatpg.h>
#include <unicode/utmscale.h>
#include <unicode/ucol.h>
#include <package-manager.h>
#include <pkgmgr-info.h>
#include <feedback.h>
#include <alarm.h>
#include <string.h>

#include "setting-clock.h"
#include "util.h"
#include "setting.h"
#include "setting_view_toast.h"

#include "setting_data_vconf.h"

EAPI Evas_Object	 *elm_widget_top_get(const Evas_Object *obj);

static Eina_List *g_clock_list[3];
static void _datetime_auto_sync_cb(void *data, Evas_Object *obj, void *event_info);
static void _datetime_date_cb(void *data, Evas_Object *obj, void *event_info);
static void _datetime_time_cb(void *data, Evas_Object *obj, void *event_info);
static char *get_timezone_str();
static UChar *uastrcpy(const char *chars);
static void ICU_set_timezone(const char *timezone);

static void _mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _page_show(void *data, Evas *e, Evas_Object *obj, void *event_info);
Evas_Object *_clock_type_cb(void *data, Evas_Object *obj, void *event_info);
void _clock_type_cb_gen_item(void *data, Evas_Object *obj, void *event_info);

static struct _clock_menu_item clock_menu_its[] = {
	{ "IDS_ST_BODY_CLOCK_TYPE_ABB",		1,		_clock_type_cb_gen_item },
#ifndef FEATURE_SETTING_SDK
	{ "IDS_ST_BODY_DATE_AND_TIME_ABB",	0,	_dt_cb },
	{ "IDS_ST_BODY_HOURLY_ALERT_ABB",	0,		_hourly_alert_cb },
#endif
};

static struct _dt_menu_item dt_menu_its[] = {
	{ "IDS_ST_BODY_AUTO_SYNC_ABB2",			"IDS_ST_BODY_SYNC_WITH_PHONE_ABB",	_datetime_auto_sync_cb	 },
	{ "IDS_ST_BODY_SET_DATE_ABB2",			"",									_datetime_date_cb		 },
	{ "IDS_SMEMO_BUTTON_SET_TIME_ABB",		"",									_datetime_time_cb		 },
};

static Elm_Object_Item *auto_sync_item = NULL;
static Elm_Object_Item *date_item = NULL;
static Elm_Object_Item *time_item = NULL;
static Evas_Object *g_date_time_genlist = NULL;
static Evas_Object *g_clock_genlist = NULL;
static Evas_Object *auto_sync_check = NULL;
static Evas_Object *g_clock_scroller = NULL;
static Evas_Object *g_clock_box = NULL;
/*pkgmgr_client *pc_clock = NULL; */
/*pkgmgr_client *pc_clock2 = NULL; */
static UCollator *coll = NULL;

static int clock_idx = 0;
static int is_alert_mode_type = 0;
static appdata *temp_ad = NULL;

static int _clock_type_compare_cb(const void *d1, const void *d2);


Evas_Object *_elm_min_set(Evas_Object *obj, Evas_Object *parent, Evas_Coord w, Evas_Coord h)
{
	Evas_Object *table, *rect;

	table = elm_table_add(parent);

	rect = evas_object_rectangle_add(evas_object_evas_get(table));
	evas_object_size_hint_min_set(rect, w, h);
	evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_table_pack(table, rect, 0, 0, 1, 1);
	evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_table_pack(table, obj, 0, 0, 1, 1);

	return table;
}

static Clock_Type_Item *_get_selected_clock()
{
	Clock_Type_Item *pitem = NULL;
	char *pkgid = NULL;
	int i = 0;

	pkgid = vconf_get_str(VCONFKEY_WMS_CLOCKS_SET_IDLE);

	if (pkgid && strlen(pkgid)) {
		Eina_List *list = NULL;
		for (i = 0; i < 3; i++) {
			EINA_LIST_FOREACH(g_clock_list[i], list, pitem) {
				if (pitem->pkgid) {
					if (!strcmp(pkgid, pitem->pkgid)) {
						/*DBG("pkgid for selected idleclock is %s", pitem->pkgid); */
						return pitem;
					}
				}
			}
		}
	}

	return NULL;
}

static void update_clock_list(void *data, Eina_Bool reload, Eina_Bool show)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return;
	}

	Evas_Object *page_layout = NULL;
	Evas_Object *box = g_clock_box;

	if (reload) {
		_clocklist_load();
	}

	if (box) {
		elm_box_clear(box);
		Clock_Type_Item *selected = _get_selected_clock();
		Clock_Type_Item *pitem = NULL;
		Eina_List *list = NULL;
		int i;
		for (i = 0; i < 3; i++) {
			EINA_LIST_FOREACH(g_clock_list[i], list, pitem) {
				/* Create Pages */
				page_layout = elm_layout_add(box);
				/*elm_layout_file_set(page_layout, EDJE_PATH, "setting/bg_thumbnail"); */
				elm_layout_theme_set(page_layout, "layout", "body_thumbnail", "default");
				evas_object_size_hint_weight_set(page_layout, 0, 0);
				evas_object_size_hint_align_set(page_layout, 0, EVAS_HINT_FILL);
				evas_object_show(page_layout);

				Evas_Object *clock_layout = elm_layout_add(page_layout);
				elm_layout_file_set(clock_layout, EDJE_PATH, "setting-test/clock-type");
				evas_object_size_hint_weight_set(clock_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				evas_object_size_hint_align_set(clock_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
				evas_object_show(clock_layout);

				/*clock bg wallpaper */
				int bg_mode;
				vconf_get_int("db/wms/home_bg_mode", &bg_mode);
				if (!bg_mode) {
					char *bg_color = NULL;
					int R = 0x00, G = 0x00, B = 0x00;
					bg_color = vconf_get_str("db/wms/home_bg_palette");
					colorstr_to_decimal(bg_color, &R, &G, &B);
					DBG("R : [%d] G : [%d] B : [%d]", R, G, B);
					Evas_Object *color_page = evas_object_rectangle_add(evas_object_evas_get(page_layout));
					evas_object_color_set(color_page, R, G, B, 255);
					evas_object_size_hint_weight_set(color_page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
					evas_object_size_hint_align_set(color_page, EVAS_HINT_FILL, EVAS_HINT_FILL);
					elm_object_part_content_set(clock_layout, "clock-color", color_page);
				} else {
					char *bg_wallpaper = NULL;
					bg_wallpaper = vconf_get_str(VCONFKEY_BGSET);
					Evas_Object *wall_page = elm_image_add(clock_layout);
					elm_image_file_set(wall_page, bg_wallpaper, NULL);
					evas_object_size_hint_weight_set(wall_page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
					evas_object_size_hint_align_set(wall_page, EVAS_HINT_FILL, EVAS_HINT_FILL);
					elm_object_part_content_set(clock_layout, "clock-wallpaper", wall_page);
				}

				/*clock image */
				Evas_Object *page = elm_image_add(clock_layout);
				elm_image_file_set(page, pitem->icon, NULL);
				evas_object_size_hint_weight_set(page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				evas_object_size_hint_align_set(page, EVAS_HINT_FILL, EVAS_HINT_FILL);

				/* touch event */
				evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, NULL);
				evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, (void *)pitem->pkgid);
				evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, NULL);

				elm_object_part_content_set(clock_layout, "clock-image", page);
				if (pitem == selected) {
					elm_object_signal_emit(page_layout, "elm,state,thumbnail,focus", "*");
				} else {
					elm_object_signal_emit(page_layout, "elm,state,thumbnail,unfocus", "*");
				}

				elm_object_part_content_set(page_layout, "elm.icon", clock_layout);

				elm_box_pack_end(box, page_layout);
			}
		}

	}

	if (show) {
		_page_show(NULL, NULL, g_clock_scroller, NULL);
	}
}
#if 0 /* _NOT_USED_ */
static int _clock_appinfo_cb(pkgmgrinfo_appinfo_h handle, void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return -1;
	}
	char *appid = NULL;
	int r;
	pkgmgrinfo_appinfo_h tmp_handle;
	bool clockapp = 0;

	r = pkgmgrinfo_appinfo_get_appid(handle, &appid);
	if (r < 0 || !appid) {
		ERR("pkgmgrinfo_appinfo_get_appid error");
		return -1;
	} else {
		r = pkgmgrinfo_appinfo_get_appinfo(appid, &tmp_handle);
		if (r != PMINFO_R_OK) {
			ERR("pkgmgrinfo_appinfo_get_appinfo error");
			return -1;
		}

		r = pkgmgrinfo_appinfo_is_category_exist(tmp_handle, IDLE_CLOCK_CATEGROY, &clockapp);
		if (r != PMINFO_R_OK) {
			ERR("pkgmgrinfo_appinfo_is_category_exist error");
			return -1;
		}

		if (!clockapp) {
			r = pkgmgrinfo_appinfo_is_category_exist(tmp_handle, IDLE_CLOCK_CATEGROY2, &clockapp);
			if (r != PMINFO_R_OK) {
				ERR("pkgmgrinfo_appinfo_is_category_exist error");
				return -1;
			}
		}
	}

	if (clockapp)	{
		update_clock_list(ad, EINA_TRUE, EINA_TRUE);
	}

	return 0;
}
#endif

#if 0 /* _NOT_USED_ */
static int _clock_app_event_cb(int req_id, const char *pkg_type, const char *pkgid,
							   const char *key, const char *val, const void *pmsg, void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return -1;
	}

	if (!pkgid || !key || !val) {
		ERR("pkgid or key or val is null");
		return -1;
	}

	int ret = 0;
	if (!strncmp(key, "end", 3) && !strncmp(val, "ok", 2)) {
		DBG("end install/update for some pkgid");

		pkgmgrinfo_pkginfo_h handle;

		ret = pkgmgrinfo_pkginfo_get_pkginfo(pkgid, &handle);
		if (ret != PMINFO_R_OK)
			return -1;
		ret = pkgmgrinfo_appinfo_get_list(handle, PMINFO_UI_APP, _clock_appinfo_cb , data);
		if (ret != PMINFO_R_OK) {
			pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
			return -1;
		}
		pkgmgrinfo_pkginfo_destroy_pkginfo(handle);

	}
	return 0;
}

static int _clock_app_uninstall_event_cb(int req_id, const char *pkg_type, const char *pkgid,
										 const char *key, const char *val, const void *pmsg, void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return -1;
	}

	if (!pkgid || !key || !val) {
		ERR("pkgid or key or val is null");
		return -1;
	}

	if (!strncmp(key, "end", 3) && !strncmp(val, "ok", 2)) {
		DBG("end uninstall for some pkgid");
		update_clock_list(ad, EINA_TRUE, EINA_TRUE);
	}
	return 0;
}
#endif

static void update_clock_list_cb(keynode_t *key, void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return;
	}

	char *vconf = NULL;
	vconf = vconf_keynode_get_name(key);

	if (vconf && !strcmp(vconf, VCONFKEY_WMS_CLOCKS_SET_IDLE)) {
		update_clock_list(ad, EINA_FALSE, EINA_TRUE);
	} else {
		update_clock_list(ad, EINA_FALSE, EINA_FALSE);
	}
}

#if 0 /* _NOT_USED_ */
static int _clock_check_appinfo(void *data, char *appid)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return -1;
	}
	int r;
	pkgmgrinfo_appinfo_h tmp_handle;
	bool clockapp = 0;

	r = pkgmgrinfo_appinfo_get_appinfo(appid, &tmp_handle);
	if (r != PMINFO_R_OK) {
		ERR("pkgmgrinfo_appinfo_get_appinfo error");
		return -1;
	}

	r = pkgmgrinfo_appinfo_is_category_exist(tmp_handle, IDLE_CLOCK_CATEGROY, &clockapp);
	if (r != PMINFO_R_OK) {
		ERR("pkgmgrinfo_appinfo_is_category_exist error");
		return -1;
	}

	if (!clockapp) {
		r = pkgmgrinfo_appinfo_is_category_exist(tmp_handle, IDLE_CLOCK_CATEGROY2, &clockapp);
		if (r != PMINFO_R_OK) {
			ERR("pkgmgrinfo_appinfo_is_category_exist error");
			return -1;
		}
	}

	if (clockapp)	{
		update_clock_list(ad, EINA_TRUE, EINA_TRUE);
	}

	return 0;
}
#endif

void initialize_clock(void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return;
	}
	UErrorCode status = U_ZERO_ERROR;
	if (coll) {
		ucol_close(coll);
		coll = NULL;
	}
	const char *lang = vconf_get_str(VCONFKEY_LANGSET);
	coll = ucol_open(lang, &status);


#if 0
	int event_type = PMINFO_CLIENT_STATUS_INSTALL | PMINFO_CLIENT_STATUS_UPGRADE;
	if (pc_clock) {
		pkgmgr_client_free(pc_clock);
		pc_clock = NULL;
	}

	if (!ad->pc_clock) {
		ad->pc_clock = pkgmgr_client_new(PMINFO_LISTENING);
		pkgmgr_client_set_status_type(ad->pc_clock, event_type);
		pkgmgr_client_listen_status(ad->pc_clock, _clock_app_event_cb, ad);
	}

	int event_type2 = PMINFO_CLIENT_STATUS_UNINSTALL;

	if (pc_clock2) {
		pkgmgr_client_free(pc_clock2);
		pc_clock2 = NULL;
	}

	if (!ad->pc_clock2) {
		ad->pc_clock2 = pkgmgr_client_new(PC_LISTENING);
		pkgmgr_client_set_status_type(ad->pc_clock2, event_type2);
		pkgmgr_client_listen_status(ad->pc_clock2, _clock_app_uninstall_event_cb, ad);
	}
#endif

	_clocklist_load();

	register_vconf_changing("db/wms/home_bg_mode", update_clock_list_cb, ad);
	register_vconf_changing(VCONFKEY_WMS_CLOCKS_SET_IDLE, update_clock_list_cb, ad);
}

void _clear_clock_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	auto_sync_item = NULL;
	date_item = NULL;
	time_item = NULL;
	g_date_time_genlist = NULL;
	g_clock_genlist = NULL;
	g_clock_box = NULL;
	g_clock_scroller = NULL;

	temp_ad = NULL;
	is_alert_mode_type = 0;

#if 0
	if (pc_clock) {
		pkgmgr_client_free(pc_clock);
		pc_clock = NULL;
	}
	if (pc_clock2) {
		pkgmgr_client_free(pc_clock2);
		pc_clock2 = NULL;
	}
#endif

	if (coll) {
		ucol_close(coll);
		coll = NULL;
	}
	_clocklist_destroy();

	unregister_vconf_changing("db/wms/home_bg_mode", update_clock_list_cb);
	unregister_vconf_changing(VCONFKEY_WMS_CLOCKS_SET_IDLE, update_clock_list_cb);

	is_running_clock = 0;
}

static void _layout_del_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	clock_page_data *pd = data;
	free(pd);

	int ret;
	ret = feedback_deinitialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		DBG("feedback_deinitialize failed");
	}
}

#if 0 /* _NOT_USED_ */
static Eina_Bool animator_cb(void *data)
{
	clock_page_data *pd = (clock_page_data *)data;

	elm_mapbuf_enabled_set(pd->mapbuf[0], EINA_TRUE);

	return ECORE_CALLBACK_CANCEL;
}
#endif

static int _set_clock_type(char *appid)
{
	if (!appid || !strlen(appid)) {
		ERR("appid is NULL");
		return FALSE;
	}
	unregister_vconf_changing(VCONFKEY_WMS_CLOCKS_SET_IDLE, update_clock_list_cb);
	vconf_set_str(VCONFKEY_WMS_CLOCKS_SET_IDLE, appid);
	/* ERR("Setting - package name: %s", appid); */

	return TRUE;
}

static int prev_x = 0;
static int touch_mode = NONE;

static void _mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	touch_mode = TOUCH_DOWN;

	Evas_Event_Mouse_Down *ev = event_info;
	prev_x = ev->canvas.x;
}

static void _mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	if (touch_mode == TOUCH_MOVE)
		return;

	char *appid = (char *)data;

	prev_x = 0;
	touch_mode = NONE;

	feedback_play(FEEDBACK_PATTERN_TAP);

	/* set gb vconf */
	if (_set_clock_type(appid)) {
		if (temp_ad != NULL) {
			elm_naviframe_item_pop(temp_ad->nf);
		}

		/* automatic freed!! */
		struct _toast_data *toast = _create_toast(temp_ad, _("IDS_ST_TPOP_CLOCK_CHANGED"));
		if (toast) {
			_show_toast(temp_ad, toast);
		}

		if (g_clock_genlist) {
			elm_genlist_realized_items_update(g_clock_genlist);
		}
	} else {
		DBG("Setting - Clock type is wrong!!");
	}

}

static void _mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Move *ev = event_info;

	int cur_x = ev->cur.canvas.x;
	if (abs(cur_x - prev_x) > 15) {
		/* todo : confirm and remove */
		touch_mode = TOUCH_MOVE;
	}
}

static void _page_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	int page = 0;

	Clock_Type_Item *pitem = _get_selected_clock();

	if (pitem) {
		page = pitem->index;
		DBG("name : %s, index : %d", pitem->name, pitem->index);
	}

	int w, h;
	elm_scroller_region_get(obj, NULL, NULL, &w, &h);
	printf("%d %d\n", w, h);

	elm_scroller_page_show(obj, page, 0);
}

static Evas_Object *_create_index(Evas_Object *parent)
{
	Evas_Object *layout, *scroller, *box, *page_layout;

	if (parent == NULL)
		return NULL;

	/* Create Layout */
	layout = elm_layout_add(parent);
	if (layout == NULL)
		return NULL;

	clock_page_data *pd = calloc(1, sizeof(clock_page_data));
	if (pd == NULL)
		return NULL;

	/*elm_layout_file_set(layout, EDJE_PATH, "setting-test/index"); */
	elm_layout_file_set(layout, EDJE_PATH, "scroller_custom_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _layout_del_cb, pd);

	/* Create Scroller */
	scroller = elm_scroller_add(layout);
	elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_page_relative_set(scroller, 1.0, 0.0);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_page_scroll_limit_set(scroller, 1, 0);
	elm_object_scroll_lock_y_set(scroller, EINA_TRUE);
	elm_object_part_content_set(layout, "scroller", scroller);
	elm_object_style_set(scroller, "effect");
	evas_object_show(scroller);
	g_clock_scroller = scroller;

	/* Create Box */
	box = elm_box_add(scroller);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_object_content_set(scroller, box);
	evas_object_show(box);
	g_clock_box = box;

	Clock_Type_Item *selected = _get_selected_clock();
	Clock_Type_Item *pitem = NULL;
	Eina_List *list = NULL;
	int i;
	for (i = 0; i < 3; i++) {
		EINA_LIST_FOREACH(g_clock_list[i], list, pitem) {
			/* Create Pages */
			page_layout = elm_layout_add(box);
			/*elm_layout_file_set(page_layout, EDJE_PATH, "setting/bg_thumbnail"); */
			elm_layout_theme_set(page_layout, "layout", "body_thumbnail", "default");
			evas_object_size_hint_weight_set(page_layout, 0, 0);
			evas_object_size_hint_align_set(page_layout, 0, EVAS_HINT_FILL);
			evas_object_show(page_layout);

			Evas_Object *clock_layout = elm_layout_add(page_layout);
			elm_layout_file_set(clock_layout, EDJE_PATH, "setting-test/clock-type");
			evas_object_size_hint_weight_set(clock_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(clock_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_show(clock_layout);

			char *bg_color = NULL;
			char *bg_color_default = "000000";
			int R = 0x00, G = 0x00, B = 0x00;
			bg_color = vconf_get_str("db/wms/home_bg_palette");
			if (bg_color == NULL)
				bg_color = bg_color_default;
			colorstr_to_decimal(bg_color, &R, &G, &B);
			DBG("R : [%d] G : [%d] B : [%d]", R, G, B);
			Evas_Object *color_page = evas_object_rectangle_add(evas_object_evas_get(page_layout));
			evas_object_color_set(color_page, R, G, B, 255);
			evas_object_size_hint_weight_set(color_page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(color_page, EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_object_part_content_set(clock_layout, "clock-color", color_page);

			/*clock image */
			Evas_Object *page = elm_image_add(clock_layout);
			elm_image_file_set(page, pitem->icon, NULL);
			evas_object_size_hint_weight_set(page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(page, EVAS_HINT_FILL, EVAS_HINT_FILL);

			/* touch event */
			evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, NULL);
			evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, (void *)pitem->appid);
			evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, NULL);

			elm_object_part_content_set(clock_layout, "clock-image", page);
			if (pitem == selected) {
				elm_object_signal_emit(page_layout, "elm,state,thumbnail,focus", "*");
			} else {
				elm_object_signal_emit(page_layout, "elm,state,thumbnail,unfocus", "*");
			}

			elm_object_part_content_set(page_layout, "elm.icon", clock_layout);

#if 0
			Evas_Object	 *mapbuf, *table;
			/* mapbuf for page 1 */
			mapbuf = elm_mapbuf_add(box);
			evas_object_size_hint_weight_set(mapbuf, 0, 0);
			evas_object_size_hint_align_set(mapbuf, 0, EVAS_HINT_FILL);
			evas_object_show(mapbuf);
			elm_object_content_set(mapbuf, page_layout);
			pd->mapbuf[pitem->index] = mapbuf;
#endif

			Evas_Object	 *table;
			Evas_Coord w = 0, h = 0;
			elm_win_screen_size_get((const Elm_Win *)elm_widget_top_get(parent), NULL, NULL, &w, &h);
			table = _elm_min_set(page_layout, box, w, h);
			evas_object_size_hint_weight_set(table, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_show(table);
			elm_box_pack_end(box, table);
		}
	}

	evas_object_event_callback_add(scroller, EVAS_CALLBACK_RESIZE, _page_show, NULL);

	int ret;
	ret = feedback_initialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		DBG("feedback_initialize failed");
	}

	/*ecore_animator_add(animator_cb, pd); */

	return layout;
}

int watch_metadata_func(const char *key, const char *value, void *user_data)
{
	Clock_Type_Data *pclockdata = (Clock_Type_Data *)user_data;
	if (strcmp(key, (char *)pclockdata->name) == 0) {
		/*pclockdata->value = value; */
		pclockdata->value = strdup(value);
		return -1;
	} else
		return 0;
}

static int _category_app_list_cb(pkgmgrinfo_appinfo_h handle, void *user_data)
{
	char *appid = NULL;
	char *name = NULL;
	char *pkgid = NULL;
	char *icon = NULL;
	int ret = 0;
	_Bool preload = 0;
	char *m_value = NULL;
	int type = CLOCKTYPE_INVALID;

	pkgmgrinfo_appinfo_h tmp_handle;

	ret = pkgmgrinfo_appinfo_get_appid(handle, &appid);

	if (ret < 0 || !appid) {
		ERR("pkgmgrinfo_appinfo_get_appid error");
		return -1;
	} else {
		ret = pkgmgrinfo_appinfo_get_appinfo(appid, &tmp_handle);
		if (ret != PMINFO_R_OK) {
			ERR("pkgmgrinfo_appinfo_get_appinfo error");
			return -1;
		}
		ret = pkgmgrinfo_appinfo_get_pkgid(tmp_handle, &pkgid);
		if (ret != PMINFO_R_OK) {
			ERR("pkgmgrinfo_appinfo_get_pkgid error");
			pkgmgrinfo_appinfo_destroy_appinfo(tmp_handle);
			return -1;
		}
		ret = pkgmgrinfo_appinfo_get_label(tmp_handle, &name);
		if (ret != PMINFO_R_OK) {
			INFO("pkgmgrinfo_appinfo_get_label error");
			pkgmgrinfo_appinfo_destroy_appinfo(tmp_handle);
		}
		ret = pkgmgrinfo_appinfo_get_icon(tmp_handle, &icon);
		if (ret != PMINFO_R_OK) {
			INFO("pkgmgrinfo_appinfo_get_icon error");
			pkgmgrinfo_appinfo_destroy_appinfo(tmp_handle);
		}
		ret = pkgmgrinfo_appinfo_is_preload(tmp_handle, &preload);
		if (ret != PMINFO_R_OK) {
			INFO("pkgmgrinfo_appinfo_is_preload error or 3rd party");
		}

		ret = pkgmgrinfo_appinfo_get_metadata_value(tmp_handle, "clocktype", &m_value);
		if (ret != PMINFO_R_OK) {
			ERR("pkgmgrinfo_appinfo_get_metadata_value error or 3rd party");
		} else {
			ERR("pkgmgrinfo_appinfo_get_metadata_value NOT error or 3rd party");
		}

		Clock_Type_Item *pitem = NULL;
		pitem = (Clock_Type_Item *)calloc(1, sizeof(Clock_Type_Item));
		setting_retvm_if(NULL == pitem, SETTING_RETURN_FAIL, "pitem is NULL");
		memset(pitem, 0x0, sizeof(Clock_Type_Item));

		static Clock_Type_Data clock_metadata = {"clocktype", 0};
		ret = pkgmgrinfo_appinfo_foreach_metadata(tmp_handle, watch_metadata_func, (void *)(&clock_metadata));

		pitem->appid = strdup(appid);
		pitem->pkgid = strdup(pkgid);
		pitem->name = strdup(name);
		pitem->icon = strdup(icon);

		if (clock_metadata.value && strlen(m_value) > 0) {
			if (!strcmp(m_value, "function")) {
				type = CLOCKTYPE_FUNCTION;
			} else if (!strcmp(m_value, "style")) {
				type = CLOCKTYPE_STYLE;
			}
		} else {
			type = CLOCKTYPE_3RD;
		}

		if (clock_metadata.value)
			free(clock_metadata.value);

		if (preload) {
			switch (type) {
			case CLOCKTYPE_FUNCTION:
				g_clock_list[0] = eina_list_sorted_insert(g_clock_list[0], _clock_type_compare_cb, pitem);
				break;
			case CLOCKTYPE_STYLE:
				g_clock_list[1] = eina_list_sorted_insert(g_clock_list[1], _clock_type_compare_cb, pitem);
				break;
			default:
				g_clock_list[2] = eina_list_sorted_insert(g_clock_list[2], _clock_type_compare_cb, pitem);
			}
		} else {
			g_clock_list[2] = eina_list_sorted_insert(g_clock_list[2], _clock_type_compare_cb, pitem);
		}
	}

	pkgmgrinfo_appinfo_destroy_appinfo(tmp_handle);
	return 0;
}

Evas_Object *_clock_type_cb(void *data, Evas_Object *obj, void *event_info)
{
	/*elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE); */

	appdata *ad = NULL;
	Evas_Object *layout_inner = NULL;

	ad = (appdata *)data;
	if (ad == NULL)
		return NULL;

	temp_ad = ad;

	layout_inner = _create_index(ad->nf);
	/*	Elm_Object_Item *it = NULL; */
	/*it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout_inner, NULL); */
	/*elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE); */

	return layout_inner;
}

void _clock_type_cb_gen_item(void *data, Evas_Object *obj, void *event_info)
{
	/*elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE); */

	appdata *ad = NULL;

	ad = (appdata *)data;
	if (ad == NULL)
		return;

	temp_ad = ad;

	/*	Evas_Object *layout_inner = NULL; */
	/* layout_inner = _create_index(ad->nf); */
	/*	Elm_Object_Item *it = NULL; */
	/*it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout_inner, NULL); */
	/*elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE); */

	return;
}

void _hourly_alert_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_hourly_alert_list(data);
}

static char *_date_format_get()
{
	int ret = 0;
	int date_val = 0;

	char *date_fmt[] = { "%d%m%Y", "%m%d%Y", "%Y%m%d", "%Y%d%m" };

	ret = vconf_get_int(VCONFKEY_SETAPPL_DATE_FORMAT_INT, &date_val);
	DBG("ret = %d", ret);

	return strdup(date_fmt[date_val]);
}

static char *_time_format_get()
{
	int ret = 0;
	int is_hour24 = 0;
	int time_val = 0;
	char *time_fmt[] = { "%d%m%Y%I:%M%p", "%d%m%Y%H:%M" };

	ret = vconf_get_int(VCONFKEY_REGIONFORMAT_TIME1224, &time_val);

	if (ret < 0)
		is_hour24 = 0;
	else if (time_val == VCONFKEY_TIME_FORMAT_12 || time_val == VCONFKEY_TIME_FORMAT_24)
		is_hour24 = time_val - 1;

	return strdup(time_fmt[is_hour24]);
}

#if 0 /* _NOT_USED_ */
static char *_datetime_format_get()
{
	char *dt_fmt, *region_fmt, *lang, *temp_fmt = NULL;
	char buf[256] = {0,};
	int time_val = 0, is_hour24 = 0, ret;

	lang = getenv("LANGUAGE");
	setenv("LANGUAGE", "en_US", 1);

	temp_fmt = vconf_get_str(VCONFKEY_REGIONFORMAT);

	DBG("Setting - %s", temp_fmt);

	region_fmt = replace(temp_fmt, "utf8", "UTF-8");

	DBG("Setting - %s", region_fmt);

	ret = vconf_get_int(VCONFKEY_REGIONFORMAT_TIME1224, &time_val);
	if (ret < 0)
		is_hour24 = 0;
	else if (time_val == VCONFKEY_TIME_FORMAT_12 || time_val == VCONFKEY_TIME_FORMAT_24)
		is_hour24 = time_val - 1;

	if (is_hour24)
		snprintf(buf, sizeof(buf), "%s_DTFMT_24HR", region_fmt);
	else
		snprintf(buf, sizeof(buf), "%s_DTFMT_12HR", region_fmt);

	dt_fmt = dgettext("dt_fmt", buf);

	if (!lang || !strcmp(lang, ""))
		unsetenv("LANGUAGE");
	else
		setenv("LANGUAGE", lang, 1);

	return strdup(dt_fmt);
}
#endif

static void _vconf_date_format_changed_cb(keynode_t *node, void *data)
{
	ICU_set_timezone(get_timezone_str());
	Evas_Object *datetime = (Evas_Object *) data;
	char *dt_fmt = _date_format_get();
	elm_datetime_format_set(datetime, dt_fmt);
	free(dt_fmt);
}

static void _vconf_time_format_changed_cb(keynode_t *node, void *data)
{
	ICU_set_timezone(get_timezone_str());
	Evas_Object *datetime = (Evas_Object *) data;
	char *dt_fmt = _time_format_get();
	elm_datetime_format_set(datetime, dt_fmt);
	free(dt_fmt);
}

#if 0 /* _NOT_USED_ */
static void _vconf_datetime_format_changed_cb(keynode_t *node, void *data)
{
	Evas_Object *datetime = (Evas_Object *) data;
	char *dt_fmt = _datetime_format_get();
	elm_datetime_format_set(datetime, dt_fmt);
	free(dt_fmt);
}
#endif

static Eina_Bool _clear_date_changed_cb(void *data, Elm_Object_Item *it)
{
	unregister_vconf_changing(VCONFKEY_SETAPPL_DATE_FORMAT_INT, _vconf_date_format_changed_cb);
	unregister_vconf_changing(VCONFKEY_SYSTEM_TIME_CHANGED, _vconf_date_format_changed_cb);

	return EINA_TRUE;
}

static Eina_Bool _clear_time_changed_cb(void *data, Elm_Object_Item *it)
{
	unregister_vconf_changing(VCONFKEY_REGIONFORMAT_TIME1224, _vconf_time_format_changed_cb);
	unregister_vconf_changing(VCONFKEY_SYSTEM_TIME_CHANGED, _vconf_date_format_changed_cb);

	return EINA_TRUE;
}

static Evas_Object *_create_scroller(Evas_Object *parent)
{
	Evas_Object *scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

void _dt_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_date_and_time_list(data);
}

char *_gl_clock_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0, };
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text") || !strcmp(part, "elm.text.1")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(clock_menu_its[index % 3].name));
	} else if (!strcmp(part, "elm.text.2")) {
		if (index == 0) {
			Clock_Type_Item *selected_clock = _get_selected_clock();
			if (selected_clock) {
				snprintf(buf, sizeof(buf) - 1, "%s", selected_clock->name);
			}
		} else if (index == 1) {
			return NULL;
		} else if (index == 2) {
			char *_houly_alert_str[] = { "IDS_ST_BODY_OFF_M_STATUS", "IDS_ST_BODY_ON_M_STATUS" };

			vconf_get_bool(VCONFKEY_SETAPPL_HOURLY_ALERT_BOOL, &clock_menu_its[index % 3].type_num);
			snprintf(buf, sizeof(buf) - 1, "%s", _(_houly_alert_str[clock_menu_its[index % 3].type_num]));
		}
		index++;
	}
	return strdup(buf);
}

static void _clock_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}

Evas_Object *_create_clock_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_clock_list - appdata is null");
		return NULL;
	}
	Evas_Object *genlist  = NULL;
	Elm_Genlist_Item_Class *itc_tmp;
	struct _clock_menu_item *menu_its = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text";
	itc->func.text_get = _gl_clock_title_get;
	itc->func.del = _clock_gl_del;

	Elm_Genlist_Item_Class *itc_date_time = elm_genlist_item_class_new();
	itc_date_time->item_style = "1text";
	itc_date_time->func.text_get = _gl_clock_title_get;
	itc_date_time->func.del = _clock_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	connect_to_wheel_with_genlist(genlist, ad);
	menu_its = clock_menu_its;

	int count = 0;
	count = sizeof(clock_menu_its) / sizeof(clock_menu_its[0]);
	for (idx = 0; idx < count; idx++) {
		if (idx == 1) {
			itc_tmp = itc_date_time;
		} else {
			itc_tmp = itc;
		}

		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
						   genlist,			/* genlist object */
						   itc_tmp,				/* item class */
						   id,					/* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_its[idx].func,	/* call back */
						   ad);
		}
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc_date_time);

	g_clock_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	return layout;
}

char *_get_str_from_icu(const char *pattern)
{
	char *locale_tmp = vconf_get_str(VCONFKEY_REGIONFORMAT);
	char locale[32] = {0,};
	char *p = NULL;

	if (strlen(locale_tmp) < 32)
		strcpy(locale, locale_tmp);

	if (locale[0] != '\0') {
		p = strstr(locale, ".UTF-8");
		if (p) {
			*p = 0;
		}
	}

	char *ret_str = NULL;
	UChar Pattern[256] = { 0, };
	UErrorCode status = U_ZERO_ERROR;
	UDateFormat *formatter = NULL;

	UChar formatted[256] = { 0, };
	char formattedString[256] = { 0, };

	u_uastrncpy(Pattern, pattern, strlen(pattern));

	UDate date = ucal_getNow();
	formatter =
		udat_open(UDAT_IGNORE, UDAT_IGNORE, locale_tmp, NULL, -1, Pattern, -1, &status);
	int32_t formattedCapacity = (int32_t)(sizeof(formatted) / sizeof((formatted)[0]));
	(void)udat_format(formatter, date, formatted, formattedCapacity, NULL, &status);
	u_austrcpy(formattedString, formatted);
	DBG("str from icu is %s", formattedString);

	udat_close(formatter);

	ret_str = strdup(formattedString);
	return ret_str;
}

char *_get_date_str()
{
	int ret = 0;
	int date_val = 0;

	char *date_fmt[] = { "dd/MM/y", "MM/dd/y", "y/MM/dd", "y/dd/MM" };

	ret = vconf_get_int(VCONFKEY_SETAPPL_DATE_FORMAT_INT, &date_val);
	DBG("ret = %d", ret);

	return _get_str_from_icu(date_fmt[date_val]);
}

char *_get_time_str()
{
	int ret = 0;
	int is_hour24 = 0;
	int time_val = 0;
	char *time_fmt[] = { "hh:mm a", "HH:mm" };

	ret = vconf_get_int(VCONFKEY_REGIONFORMAT_TIME1224, &time_val);

	if (ret < 0)
		is_hour24 = 0;
	else if (time_val == VCONFKEY_TIME_FORMAT_12 || time_val == VCONFKEY_TIME_FORMAT_24)
		is_hour24 = time_val - 1;

	return _get_str_from_icu(time_fmt[is_hour24]);
}

char *_gl_date_and_time_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;

	char  expression[32];
	int auto_update = 0;
	vconf_get_bool(VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL, &auto_update);

	if (!strcmp(part, "elm.text.1")) {
		if (index == 0) {
			snprintf(buf, sizeof(buf) - 1, "%s", _(dt_menu_its[index % 3].name));
		} else {
			if (auto_update) {
				strncpy(expression, "<font color=#515151>%s</font>", 32);
			} else {
				strncpy(expression, "%s", 32);
			}
			snprintf(buf, sizeof(buf) - 1, expression, _(dt_menu_its[index % 3].name));
		}
	} else if (!strcmp(part, "elm.text.2")) {
		if (index == 0) {
			snprintf(buf, sizeof(buf) - 1, "%s", _(dt_menu_its[index % 3].date_or_time));
		} else {
			if (auto_update) {
				strncpy(expression, "<font color=#515151>%s</font>", 32);
			} else {
				strncpy(expression, "%s", 32);
			}

			if (index == 1) {	/* Date */
				char *date_buf = NULL;
				date_buf = _get_date_str();
				snprintf(buf, sizeof(buf) - 1, expression, date_buf);
				free(date_buf);
			} else if (index == 2) {	/* Time */
				char *time_buf = NULL;
				time_buf = _get_time_str();
				snprintf(buf, sizeof(buf) - 1, expression, time_buf);
				free(time_buf);
			}
		}
		index++;
	}
	return strdup(buf);
}

static Evas_Object *_gl_dt_auto_sync_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;
	int is_auto_update = 0;

	DT_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		if (index == 0) {
			check = elm_check_add(obj);
			if (vconf_get_bool(VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL, &is_auto_update) < 0) {
				DBG("Setting - auto time update's vconf get fail");
			}
			elm_check_state_set(check, (is_auto_update) ? EINA_TRUE : EINA_FALSE);
			evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

			auto_sync_check = check;
		}
		index++;
	}

	return check;
}

static void _dt_gl_del(void *data, Evas_Object *obj)
{
	DT_Item_Data *id = data;
	if (id)
		free(id);

	auto_sync_check = NULL;
	auto_sync_item = NULL;
	date_item = NULL;
	time_item = NULL;
}

static void change_datetime_format_cb(keynode_t *key, void *data)
{
	int ret = 0;
	int time_val = 0;
	ret = vconf_get_int(VCONFKEY_REGIONFORMAT_TIME1224, &time_val);
	if (ret < 0)
		return;

	DBG("Setting - time_format : %d", time_val);
	ICU_set_timezone(get_timezone_str());

	int is_auto_sync = 0;
	vconf_get_bool(VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL, &is_auto_sync);

	if (date_item) {
		elm_genlist_item_update(date_item);
		/*elm_object_item_disabled_set(date_item, (is_auto_sync) ? EINA_TRUE : EINA_FALSE ); */
	}
	if (time_item) {
		elm_genlist_item_update(time_item);
		/*elm_object_item_disabled_set(time_item, (is_auto_sync) ? EINA_TRUE : EINA_FALSE ); */
	}
}

static Eina_Bool _clear_datetime_changed_cb(void *data, Elm_Object_Item *it)
{
	unregister_vconf_changing(VCONFKEY_REGIONFORMAT_TIME1224, change_datetime_format_cb);
	unregister_vconf_changing(VCONFKEY_SETAPPL_DATE_FORMAT_INT, change_datetime_format_cb);
	unregister_vconf_changing(VCONFKEY_SYSTEM_TIME_CHANGED, change_datetime_format_cb);

	return EINA_TRUE;
}

void _show_date_and_time_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_dt_list - appdata is null");
		return;
	}
	Evas_Object *genlist  = NULL;
	Evas_Object *layout = NULL;
	Elm_Object_Item *nf_it = NULL;
	struct _dt_menu_item *menu_its = NULL;
	int idx = 0;
	int auto_sync = 0;

	ICU_set_timezone(get_timezone_str());

	vconf_get_bool(VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL, &auto_sync);

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text.1icon.1";
	itc->func.text_get = _gl_date_and_time_title_get;
	itc->func.content_get = _gl_dt_auto_sync_check_get;
	itc->func.del = _dt_gl_del;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	connect_to_wheel_with_genlist(genlist, ad);
	menu_its = dt_menu_its;

	for (idx = 0; idx < CLOCK_DATE_AND_TIME_COUNT; idx++) {
		DT_Item_Data *id = calloc(sizeof(DT_Item_Data), 1);
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

			switch (idx) {
			case 0:
				auto_sync_item = id->item;
				break;
			case 1:
				date_item = id->item;
				break;
			case 2:
				time_item = id->item;
				break;
			}
		}
	}
	elm_genlist_item_class_free(itc);

	g_date_time_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _clear_datetime_changed_cb, ad);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

	/* time format changing cb */
	register_vconf_changing(VCONFKEY_REGIONFORMAT_TIME1224, change_datetime_format_cb, NULL);
	register_vconf_changing(VCONFKEY_SETAPPL_DATE_FORMAT_INT, change_datetime_format_cb, NULL);
	register_vconf_changing(VCONFKEY_SYSTEM_TIME_CHANGED, change_datetime_format_cb, NULL);
}

static void _set_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data || !obj) return;

	appdata *ad = data;
	struct tm currtime;

	elm_datetime_value_get(ad->datetime, &currtime);

	currtime.tm_isdst = -1;
	currtime.tm_sec = 0;

	time_t t = mktime(&currtime);

	int ret = alarmmgr_set_systime(t);
	DBG("ret = %d", ret);

	elm_naviframe_item_pop(ad->nf);

	/* automatic freed!! */
	Toast_Data *toast = _create_toast(ad, _("IDS_ST_TPOP_CHANGING_TIME_AND_DATE_SETTINGS_MAY_AFFECT_SOME_FUNCTIONS"));
	if (toast) {
		_show_toast(ad, toast);
	}

	if (g_date_time_genlist) {
		elm_genlist_realized_items_update(g_date_time_genlist);
	}
}

static void _cancle_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;

	if (!ad)
		return;

	elm_naviframe_item_pop(ad->nf);

	if (ad->dt_genlist_item_of_time != NULL) {
		elm_genlist_item_update(ad->dt_genlist_item_of_time);
	}

	if (ad->dt_genlist_item_of_date != NULL) {
		elm_genlist_item_update(ad->dt_genlist_item_of_date);
	}
}

static void _datetime_auto_sync_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	int is_auto_sync = 0;

	vconf_get_bool(VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL, &is_auto_sync);

	is_auto_sync = !is_auto_sync;

	vconf_set_bool(VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL, is_auto_sync);

	DBG("Setting - Auto sync : %s", is_auto_sync ? "TRUE" : "FALSE");

	if (auto_sync_check) {
		elm_check_state_set(auto_sync_check, (is_auto_sync) ? EINA_TRUE : EINA_FALSE);
	}

	if (date_item) {
		elm_genlist_item_update(date_item);
		/*elm_object_item_disabled_set(date_item, (is_auto_sync) ? EINA_TRUE : EINA_FALSE ); */
	}
	if (time_item) {
		elm_genlist_item_update(time_item);
		/*elm_object_item_disabled_set(time_item, (is_auto_sync) ? EINA_TRUE : EINA_FALSE ); */
	}
}

static void _datetime_date_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	int is_auto_sync = 0;
	vconf_get_bool(VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL, &is_auto_sync);
	if (is_auto_sync) {
		return;
	}

	Evas_Object *scroller;
	Evas_Object *layout, *btn;
	Evas_Object *datetime;
	Elm_Object_Item *it;
	char *dt_fmt;
	appdata *ad = (appdata *)data;

	if (!ad || !obj)
		return;

	ad->dt_genlist_item_of_date = (Elm_Object_Item *)event_info;

	scroller = _create_scroller(ad->nf);
	evas_object_data_set(scroller, "appdata", ad);

	layout = elm_layout_add(scroller);
	elm_layout_file_set(layout, EDJE_PATH, "setting-test/datetime");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	datetime = elm_datetime_add(layout);
	elm_object_style_set(datetime, "datepicker_layout");
	elm_object_part_content_set(layout, "content", datetime);

	ad->datetime = datetime;

#ifndef DESKTOP
	dt_fmt = _date_format_get();
	elm_datetime_format_set(datetime, dt_fmt);
	free(dt_fmt);
	register_vconf_changing(VCONFKEY_SETAPPL_DATE_FORMAT_INT, _vconf_date_format_changed_cb, datetime);
	register_vconf_changing(VCONFKEY_SYSTEM_TIME_CHANGED, _vconf_date_format_changed_cb, datetime);
#endif

	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("IDS_ST_BUTTON_CANCEL_ABB2"));
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "btn1", btn);
	evas_object_smart_callback_add(btn, "clicked", _cancle_clicked_cb, ad);

	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("IDS_COM_SK_SET_ABB"));
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "btn2", btn);
	evas_object_smart_callback_add(btn, "clicked", _set_clicked_cb, ad);

	elm_object_content_set(scroller, layout);

	it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, scroller, NULL);
	elm_naviframe_item_pop_cb_set(it, _clear_date_changed_cb, ad);
	elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
}

static void _datetime_time_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	int is_auto_sync = 0;
	vconf_get_bool(VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL, &is_auto_sync);
	if (is_auto_sync) {
		return;
	}

	Evas_Object *scroller;
	Evas_Object *layout, *btn;
	Evas_Object *datetime;
	Elm_Object_Item *it;
	char *dt_fmt;
	appdata *ad = (appdata *)data;

	if (!ad || !obj)
		return;

	ad->dt_genlist_item_of_time = (Elm_Object_Item *)event_info;

	scroller = _create_scroller(ad->nf);
	evas_object_data_set(scroller, "appdata", ad);

	layout = elm_layout_add(scroller);
	elm_layout_file_set(layout, EDJE_PATH, "setting-test/datetime");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	datetime = elm_datetime_add(layout);
	elm_object_style_set(datetime, "timepicker_layout");
	elm_object_part_content_set(layout, "content", datetime);

	ad->datetime = datetime;

#ifndef DESKTOP
	dt_fmt = _time_format_get();
	elm_datetime_format_set(datetime, dt_fmt);
	DBG("datetime time format : %s", dt_fmt);
	free(dt_fmt);
	register_vconf_changing(VCONFKEY_REGIONFORMAT_TIME1224, _vconf_time_format_changed_cb, datetime);
	register_vconf_changing(VCONFKEY_SYSTEM_TIME_CHANGED, _vconf_time_format_changed_cb, datetime);
#endif

	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("IDS_ST_BUTTON_CANCEL_ABB2"));
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "btn1", btn);
	evas_object_smart_callback_add(btn, "clicked", _cancle_clicked_cb, ad);

	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("IDS_COM_SK_SET_ABB"));
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "btn2", btn);
	evas_object_smart_callback_add(btn, "clicked", _set_clicked_cb, ad);

	elm_object_content_set(scroller, layout);

	it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, scroller, NULL);
	elm_naviframe_item_pop_cb_set(it, _clear_time_changed_cb, ad);
	elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
}

static void _alert_gl_del(void *data, Evas_Object *obj)
{
	Alert_Item_Data *id = data;
	if (id)
		free(id);
}

char *_gl_alert_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Alert_Item_Data *id = data;
	int index = id->index;

	char *temps[] = { "IDS_ST_BODY_ON_M_STATUS", "IDS_ST_BODY_OFF_M_STATUS" };

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(temps[index]));
		index++;
	}
	return strdup(buf);
}

Evas_Object *_gl_alert_ridio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Alert_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, id->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_radio_group_add(radio, radio_main);

#if 0
		vconf_get_bool(VCONFKEY_SETAPPL_HOURLY_ALERT_BOOL, &is_alert_mode_type);

		is_alert_mode_type = !is_alert_mode_type;

		if (is_alert_mode_type == id->index) {
			elm_radio_value_set(radio_main, is_alert_mode_type);
		}
#endif
		index++;
	}
	return radio;
}

static void _hourly_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	is_alert_mode_type = (int)data;

	elm_naviframe_item_pop(temp_ad->nf);
	if (!temp_ad->alert_rdg) {
		evas_object_del(temp_ad->alert_rdg);
		temp_ad->alert_rdg = NULL;
	}

#if 0
	clock_menu_its[2].type_num = is_alert_mode_type;
	vconf_set_bool(VCONFKEY_SETAPPL_HOURLY_ALERT_BOOL, !is_alert_mode_type);
#endif
	if (g_clock_genlist) {
		elm_genlist_realized_items_update(g_clock_genlist);
	}

	temp_ad = NULL;
}

void _show_hourly_alert_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_alert_list - appdata is null");
		return;
	}
	Evas_Object *genlist  = NULL;
	Elm_Object_Item *nf_it = NULL;

	temp_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_alert_title_get;
	itc->func.content_get = _gl_alert_ridio_get;
	itc->func.del = _alert_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	connect_to_wheel_with_genlist(genlist, ad);

	Alert_Item_Data *id = calloc(sizeof(Alert_Item_Data), 1);
	if (id) {
		id->index = 0;
		id->item = elm_genlist_item_append(genlist,	itc, id, NULL, ELM_GENLIST_ITEM_NONE, _hourly_gl_cb, (void *)0);
	}

	Alert_Item_Data *id2 = calloc(sizeof(Alert_Item_Data), 1);
	if (id2) {
		id2->index = 1;
		id2->item = elm_genlist_item_append(genlist, itc, id2, NULL, ELM_GENLIST_ITEM_NONE, _hourly_gl_cb, (void *)1);
	}

	ad->alert_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->alert_rdg, 3);
	elm_radio_value_set(ad->alert_rdg, is_alert_mode_type);

	evas_object_data_set(genlist, "radio_main", ad->alert_rdg);

	elm_genlist_item_class_free(itc);

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}


void _clocklist_load()
{
	if (g_clock_list[0] || g_clock_list[1] || g_clock_list[2]) {
		_clocklist_destroy();
	}

	pkgmgrinfo_appinfo_filter_h handle = NULL;

	if (pkgmgrinfo_appinfo_filter_create(&handle) != PMINFO_R_OK) {
		ERR("pkgmgrinfo_appinfo_filter_create error");
		return;
	}

	if (pkgmgrinfo_appinfo_filter_add_string(handle, PMINFO_APPINFO_PROP_APP_COMPONENT, WATCH_COMPONENT_NAME)
		!= PMINFO_R_OK) {
		ERR("pkgmgrinfo_appinfo_filter_add_string error");
		pkgmgrinfo_appinfo_filter_destroy(handle);
		return;
	}

	if (pkgmgrinfo_appinfo_filter_foreach_appinfo(handle, _category_app_list_cb, NULL)
		!= PMINFO_R_OK) {
		ERR("pkgmgrinfo_appinfo_filter_foreach_appinfo error");
		pkgmgrinfo_appinfo_filter_destroy(handle);
		return;
	}

	pkgmgrinfo_appinfo_filter_destroy(handle);

	Clock_Type_Item *pitem = NULL;
	Eina_List *list = NULL;
	int i;

	for (i = 0; i < 3; i++) {
		EINA_LIST_FOREACH(g_clock_list[i], list, pitem) {
			pitem->index = clock_idx++;
		}
	}

}

void _clocklist_destroy()
{
	Clock_Type_Item *pitem = NULL;
	Eina_List *list = NULL;
	clock_idx = 0;
	int i;

	for (i = 0; i < 3; i++) {
		EINA_LIST_FOREACH(g_clock_list[i], list, pitem) {
			FREE(pitem->appid);
			FREE(pitem->pkgid);
			FREE(pitem->name);
			FREE(pitem->icon);
		}
	}
	g_clock_list[0] = eina_list_free(g_clock_list[0]);
	g_clock_list[1] = eina_list_free(g_clock_list[1]);
	g_clock_list[2] = eina_list_free(g_clock_list[2]);
}

static int _clock_type_compare_cb(const void *d1, const void *d2)
{
	UChar clock1[256] = { 0, };
	UChar clock2[256] = { 0, };

	Clock_Type_Item *r1 = (Clock_Type_Item *) d1;
	Clock_Type_Item *r2 = (Clock_Type_Item *) d2;

	u_uastrcpy(clock1, r1->name);
	u_uastrcpy(clock2, r2->name);

	/*const char *lang = vconf_get_str(VCONFKEY_LANGSET); */
	/*UErrorCode status = U_ZERO_ERROR; */
	/*UCollator *coll = ucol_open(lang,	 &status); */
	UCollationResult ret = ucol_strcoll(coll, clock1, -1, clock2, -1);

	/*ucol_close(coll); */

	switch (ret) {
	case UCOL_EQUAL:
		return 0;
	case UCOL_GREATER:
		return 1;
	case UCOL_LESS:
		return -1;
	default:
		return 0;
	}
}

static char *get_timezone_str()
{
	char buf[1024];
	ssize_t len = readlink(TZ_SYS_ETC_D"/localtime", buf, sizeof(buf) - 1);

	if (len != -1) {
		buf[len] = '\0';
	} else {
		/* handle error condition */
	}
	return strdup(buf + 20);
}

static UChar *uastrcpy(const char *chars)
{
	int len = 0;
	UChar *str = NULL;

	if (!chars) return NULL;

	len = strlen(chars);
	str = (UChar *) malloc(sizeof(UChar) * (len + 1));
	if (!str)
		return NULL;
	u_uastrcpy(str, chars);
	return str;
}

static void ICU_set_timezone(const char *timezone)
{
	DBG("ICU_set_timezone = %s ", timezone);
	UErrorCode ec = U_ZERO_ERROR;
	UChar *str = uastrcpy(timezone);

	ucal_setDefaultTimeZone(str, &ec);
	if (U_SUCCESS(ec)) {
		DBG("ucal_setDefaultTimeZone() SUCCESS ");
	} else {
		ERR("ucal_setDefaultTimeZone() FAILED : %s ", u_errorName(ec));
	}
	FREE(str);
}
