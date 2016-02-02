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
 * setting-double.c
 *
 *  Created on: Jan 8, 2014
 *      Author: Sunyeop Hwang
 */
#include <unicode/ustring.h>
#include <unicode/ucol.h>
#include <package-manager.h>
#include <pkgmgr-info.h>

#include "setting-double.h"
#include "setting_data_vconf.h"
#include "util.h"

static Evas_Object *g_double_genlist = NULL;
static Evas_Object *g_double_app_genlist = NULL;
static Eina_List *app_list = NULL;
static int list_index = 1;
struct _double_menu_item *pitem_none = NULL;
struct _double_menu_item *pitem_recent = NULL;
/*pkgmgr_client *pc = NULL; */
/*pkgmgr_client *pc2 = NULL; */
static UCollator *coll = NULL;

static struct _double_menu_item *_get_selected_app()
{
	struct _double_menu_item *pitem = NULL;
	char *appid = NULL;
	char *p = NULL;

	appid = vconf_get_str(VCONFKEY_WMS_POWERKEY_DOUBLE_PRESSING);

	if (appid && strlen(appid)) {
		if (!strcmp(appid, "none")) {
			return pitem_none;
		} else if (!strcmp(appid, "recent")) {
			return pitem_recent;
		} else {
			Eina_List *list = NULL;
			EINA_LIST_FOREACH(app_list, list, pitem) {
				if (pitem->pkgid && pitem->appid) {
					char buf[1024] = {0, };
					snprintf(buf, sizeof(buf) - 1, "%s/%s", pitem->pkgid, pitem->appid);
					if (!strcmp(appid, buf)) {
						DBG("pkgid/appid for double power key is %s/%s", pitem->pkgid, pitem->appid);
						return pitem;
					}
				}
			}
		}
	}

	return NULL;
}

static int _sort_app_list_cb(const void *d1, const void *d2)
{
	UChar app1[128] = { 0, };
	UChar app2[128] = { 0, };

	struct _double_menu_item *r1 = (struct _double_menu_item *) d1;
	struct _double_menu_item *r2 = (struct _double_menu_item *) d2;

	u_uastrcpy(app1, r1->name);
	u_uastrcpy(app2, r2->name);

	/*DBG("before ucol_open"); */
	/*UErrorCode status = U_ZERO_ERROR; */
	/*UCollator *coll = ucol_open(getenv("LANG"), &status); */
	UCollationResult ret = ucol_strcoll(coll, app1, -1, app2, -1);

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

static int _app_list_cb(pkgmgrinfo_appinfo_h handle, void *user_data)
{
	appdata *ad = user_data;
	if (!ad) {
		ERR("appdata is null");
		return -1;
	}

	char *appid = NULL;
	char *name = NULL;
	char *pkgid = NULL;
	int ret = 0;
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
			ERR("pkgmgrinfo_appinfo_get_label error");
			pkgmgrinfo_appinfo_destroy_appinfo(tmp_handle);
			return -1;
		}

		if (strcmp(name, "Call")) {
			struct _double_menu_item *pitem = NULL;
			pitem = (struct _double_menu_item *)calloc(1, sizeof(struct _double_menu_item));
			if (pitem) {
				memset(pitem, 0x0, sizeof(struct _double_menu_item));

				pitem->index = ++list_index;
				pitem->appid = strdup(appid);
				pitem->pkgid = strdup(pkgid);
				pitem->name = strdup(name);

				app_list = eina_list_sorted_insert(app_list, _sort_app_list_cb, pitem);
			}
		}
	}

	pkgmgrinfo_appinfo_destroy_appinfo(tmp_handle);
	return 0;
}

static void _clear_app_list()
{
	struct _double_menu_item *pitem = NULL;
	Eina_List *list = NULL;

	EINA_LIST_FOREACH(app_list, list, pitem) {
		FREE(pitem->appid);
		FREE(pitem->name);
	}
	app_list = eina_list_free(app_list);
	list_index = 1;
}

static void _make_app_list(void *data)
{
	appdata *ad = data;
	if (!ad) {
		ERR("appdata is null");
		return;
	}

	if (app_list) {
		_clear_app_list();
	}

	pkgmgrinfo_appinfo_filter_h handle = NULL;

	if (pkgmgrinfo_appinfo_filter_create(&handle) != PMINFO_R_OK) {
		ERR("pkgmgrinfo_appinfo_filter_create error");
		return;
	}

	if (pkgmgrinfo_appinfo_filter_add_bool(handle, PMINFO_APPINFO_PROP_APP_NODISPLAY, 0)
	    != PMINFO_R_OK) {
		ERR("pkgmgrinfo_appinfo_filter_add_bool error");
		pkgmgrinfo_appinfo_filter_destroy(handle);
		return;
	}

	if (pkgmgrinfo_appinfo_filter_foreach_appinfo(handle, _app_list_cb, ad)
	    != PMINFO_R_OK) {
		ERR("pkgmgrinfo_appinfo_filter_foreach_appinfo error");
		pkgmgrinfo_appinfo_filter_destroy(handle);
		return;
	}

	pkgmgrinfo_appinfo_filter_destroy(handle);

	/*app_list = eina_list_sort(app_list, eina_list_count(app_list), _sort_app_list_cb); */
}

static void _gl_double_del(void *data, Evas_Object *obj)
{
	Double_Item_Data *id = data;
	if (id)
		free(id);
}

static char *_gl_double_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0, };

	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_MBODY_DOUBLE_PRESS_ABB"));
		DBG("elm.text.1 : %s", buf);
	} else if (!strcmp(part, "elm.text.2")) {
		struct _double_menu_item *selected_app = _get_selected_app();
		if (selected_app) {
			snprintf(buf, sizeof(buf) - 1, "%s", _(selected_app->name));
			DBG("elm.text.2 : %s", buf);
		}
	}

	return strdup(buf);
}

static char *_gl_double_app_title_get(void *data, Evas_Object *obj, const char *part)
{
	Double_Item_Data *id = data;

	if (!strcmp(part, "elm.text")) {
		return strdup(_(id->pitem->name));
	}

	return NULL;
}

static Evas_Object *_gl_double_app_radio_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *radio = NULL;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	Double_Item_Data *id = data;

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_object_style_set(radio, "list");
		elm_radio_state_value_set(radio, id->pitem->index);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(radio, EINA_FALSE);
		evas_object_repeat_events_set(radio, EINA_TRUE);
		elm_radio_group_add(radio, radio_main);
	}

	return radio;
}

static void _gl_double_app_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);
	Double_Item_Data *id = (Double_Item_Data *)elm_object_item_data_get(item);

	appdata *ad = data;

	if (!ad || !id) {
		ERR("appdata or id is null!!");
		return;
	}

	if (id->pitem && id->pitem->index == 0) {
		char buf[1024] = {0, };
		snprintf(buf, sizeof(buf) - 1, "none");
		vconf_set_str(VCONFKEY_WMS_POWERKEY_DOUBLE_PRESSING, buf);
	} else if (id->pitem && id->pitem->index == 1) {
		char buf[1024] = {0, };
		snprintf(buf, sizeof(buf) - 1, "recent");
		vconf_set_str(VCONFKEY_WMS_POWERKEY_DOUBLE_PRESSING, buf);
	} else if (id->pitem && id->pitem->appid && strlen(id->pitem->appid)) {
		char buf[1024] = {0, };
		DBG("%s/%s is selected", id->pitem->pkgid, id->pitem->appid);
		snprintf(buf, sizeof(buf) - 1, "%s/%s", id->pitem->pkgid, id->pitem->appid);
		vconf_set_str(VCONFKEY_WMS_POWERKEY_DOUBLE_PRESSING, buf);
	}

	elm_naviframe_item_pop(ad->nf);

	if (!ad->double_rdg) {
		evas_object_del(ad->double_rdg);
		ad->double_rdg = NULL;
	}
}

static void change_double_pressing_cb(keynode_t *key, void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return;
	}
	if (g_double_app_genlist) {
		struct _double_menu_item *selected_app = NULL;
		selected_app = _get_selected_app();
		if (selected_app) {
			elm_radio_value_set(ad->double_rdg, selected_app->index);
		} else {
			elm_radio_value_set(ad->double_rdg, -1);
		}

		Elm_Object_Item *item = NULL;
		item = elm_genlist_first_item_get(g_double_app_genlist);

		while (item) {
			Double_Item_Data *id = (Double_Item_Data *)elm_object_item_data_get(item);
			if (id->pitem == selected_app) {
				elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
				break;
			}
			item = elm_genlist_item_next_get(item);
		}
	}

	if (g_double_genlist) {
		elm_genlist_realized_items_update(g_double_genlist);
	}
}

static void update_double_app_list(void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return;
	}

	_make_app_list(ad);

	if (g_double_genlist) {
		elm_genlist_realized_items_update(g_double_genlist);
	}

	if (g_double_app_genlist) {
		struct _double_menu_item *selected_app = NULL;
		struct _double_menu_item *pitem = NULL;
		Eina_List *list = NULL;
		Elm_Object_Item *sel_it = NULL;

		elm_genlist_clear(g_double_app_genlist);

		Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
		itc->item_style = "1text.1icon.1";
		itc->func.text_get = _gl_double_app_title_get;
		itc->func.content_get = _gl_double_app_radio_get;
		itc->func.del = _gl_double_del;

		selected_app = _get_selected_app();

		Double_Item_Data *id_none = calloc(sizeof(Double_Item_Data), 1);
		if (id_none) {
			id_none->pitem = pitem_none;
			id_none->item = elm_genlist_item_append(g_double_app_genlist, itc, id_none, NULL,
					ELM_GENLIST_ITEM_NONE,
					_gl_double_app_sel_cb, ad);

			if (id_none->pitem == selected_app) {
				sel_it = id_none->item;
			}
		}

		Double_Item_Data *id_recent = calloc(sizeof(Double_Item_Data), 1);
		if (id_recent) {
			id_recent->pitem = pitem_recent;
			id_recent->item = elm_genlist_item_append(g_double_app_genlist, itc, id_recent, NULL,
					ELM_GENLIST_ITEM_NONE,
					_gl_double_app_sel_cb, ad);

			if (id_recent->pitem == selected_app) {
				sel_it = id_recent->item;
			}
		}

		EINA_LIST_FOREACH(app_list, list, pitem) {
			Double_Item_Data *id = calloc(sizeof(Double_Item_Data), 1);
			if (id) {
				id->pitem = pitem;
				id->item = elm_genlist_item_append(g_double_app_genlist, itc, id, NULL,
						ELM_GENLIST_ITEM_NONE,
						_gl_double_app_sel_cb, ad);

				if (id->pitem == selected_app) {
					sel_it = id->item;
				}
			}
		}

		if (selected_app) {
			elm_radio_value_set(ad->double_rdg, selected_app->index);
		} else {
			elm_radio_value_set(ad->double_rdg, -1);
		}

		elm_genlist_item_show(sel_it, ELM_GENLIST_ITEM_SCROLLTO_TOP);

		elm_genlist_item_class_free(itc);
	}

}

static void change_language_cb(keynode_t *key, void *data)
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

	update_double_app_list(ad);
}

static int _double_press_check_appinfo(void *data, char *appid)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return -1;
	}
	int r;
	pkgmgrinfo_appinfo_h tmp_handle;
	int nodisplay = 0;

	DBG("appid:%s", appid);
	r = pkgmgrinfo_appinfo_get_appinfo(appid, &tmp_handle);
	if (r != PMINFO_R_OK) {
		ERR("pkgmgrinfo_appinfo_get_appinfo error : %d", r);
		return -1;
	}

	r = pkgmgrinfo_appinfo_is_nodisplay(tmp_handle, &nodisplay);
	if (r != PMINFO_R_OK) {
		ERR("pkgmgrinfo_appinfo_is_nodisplay error");
		return -1;
	}

	if (!nodisplay)	{
		update_double_app_list(ad);
	}

	return 0;
}

void clear_double_app_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	if (coll) {
		ucol_close(coll);
		coll = NULL;
	}

#if 0
	if (pc) {
		pkgmgr_client_free(pc);
		pc = NULL;
	}
	if (pc2) {
		pkgmgr_client_free(pc2);
		pc2 = NULL;
	}
#endif

	FREE(pitem_none);
	FREE(pitem_recent);
	g_double_app_genlist = NULL;
	unregister_vconf_changing(VCONFKEY_WMS_POWERKEY_DOUBLE_PRESSING, change_double_pressing_cb);
	unregister_vconf_changing(VCONFKEY_LANGSET, change_language_cb);
}

Evas_Object *create_double_app_list(void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return NULL;
	}

	Evas_Object *layout = NULL;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *nf_it = NULL;
	Elm_Object_Item *sel_it = NULL;

	struct _double_menu_item *selected_app = NULL;
	struct _double_menu_item *pitem = NULL;
	Eina_List *list = NULL;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_double_app_title_get;
	itc->func.content_get = _gl_double_app_radio_get;
	itc->func.del = _gl_double_del;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	selected_app = _get_selected_app();

	Double_Item_Data *id_none = calloc(sizeof(Double_Item_Data), 1);
	if (id_none) {
		id_none->pitem = pitem_none;
		id_none->item = elm_genlist_item_append(genlist, itc, id_none, NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_double_app_sel_cb, ad);

		if (id_none->pitem == selected_app) {
			sel_it = id_none->item;
		}
	}

	Double_Item_Data *id_recent = calloc(sizeof(Double_Item_Data), 1);
	if (id_recent) {
		id_recent->pitem = pitem_recent;
		id_recent->item = elm_genlist_item_append(genlist, itc, id_recent, NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_double_app_sel_cb, ad);

		if (id_recent->pitem == selected_app) {
			sel_it = id_recent->item;
		}
	}

	EINA_LIST_FOREACH(app_list, list, pitem) {
		Double_Item_Data *id = calloc(sizeof(Double_Item_Data), 1);
		if (id) {
			id->pitem = pitem;
			id->item = elm_genlist_item_append(genlist, itc, id, NULL,
					ELM_GENLIST_ITEM_NONE,
					_gl_double_app_sel_cb, ad);

			if (id->pitem == selected_app) {
				sel_it = id->item;
			}
		}
	}

	ad->double_rdg = elm_radio_add(genlist);
	elm_radio_state_value_set(ad->double_rdg, -1);

	if (selected_app) {
		elm_radio_value_set(ad->double_rdg, selected_app->index);
	} else {
		elm_radio_value_set(ad->double_rdg, -1);
	}
	evas_object_data_set(genlist, "radio_main", ad->double_rdg);

	elm_genlist_item_show(sel_it, ELM_GENLIST_ITEM_SCROLLTO_TOP);

	g_double_app_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	elm_genlist_item_class_free(itc);

	return layout;
}

static int _double_press_appinfo_cb(pkgmgrinfo_appinfo_h handle, void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return -1;
	}
	char *appid = NULL;
	int r;
	pkgmgrinfo_appinfo_h tmp_handle;
	int nodisplay = 0;

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

		r = pkgmgrinfo_appinfo_is_nodisplay(tmp_handle, &nodisplay);
		if (r != PMINFO_R_OK) {
			ERR("pkgmgrinfo_appinfo_is_nodisplay error");
			return -1;
		}
	}

	if (!nodisplay)	{
		update_double_app_list(ad);
	}

	return 0;
}

static int _double_press_app_event_cb(int req_id, const char *pkg_type, const char *pkgid,
                                      const char key, const char *val, const void *pmsg, void *data)
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
		ret = pkgmgrinfo_appinfo_get_list(handle, PMINFO_UI_APP, _double_press_appinfo_cb , data);
		if (ret != PMINFO_R_OK) {
			pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
			return -1;
		}
		pkgmgrinfo_pkginfo_destroy_pkginfo(handle);

	}
	return 0;
}

static int _double_press_app_uninstall_event_cb(int req_id, const char *pkg_type, const char *pkgid,
                                                const char key, const char *val, const void *pmsg, void *data)
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
		DBG("end uninstall for some pkgid");
		update_double_app_list(ad);
	}
	return 0;
}

void init_double_pressing(void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return NULL;
	}

	FREE(pitem_none);
	pitem_none = calloc(sizeof(struct _double_menu_item), 1);

	if (pitem_none) {
		pitem_none->index = 0;
		pitem_none->appid = strdup("none");
		pitem_none->pkgid = strdup("none");
		pitem_none->name = strdup("IDS_LCKSCN_BODY_NONE");
	}

	FREE(pitem_recent);
	pitem_recent = calloc(sizeof(struct _double_menu_item), 1);

	if (pitem_recent) {
		pitem_recent->index = 1;
		pitem_recent->appid = strdup("recent");
		pitem_recent->pkgid = strdup("recent");
		pitem_recent->name = strdup("IDS_ST_OPT_RECENT_APPS_ABB");
	}

	UErrorCode status = U_ZERO_ERROR;
	if (coll) {
		ucol_close(coll);
		coll = NULL;
	}
	const char *lang = vconf_get_str(VCONFKEY_LANGSET);
	coll = ucol_open(lang, &status);

	int event_type = PMINFO_CLIENT_STATUS_INSTALL | PMINFO_CLIENT_STATUS_UPGRADE;

#if 0
	if (pc) {
		pkgmgr_client_free(pc);
		pc = NULL;
	}

	if (!ad->pc) {
		ad->pc = pkgmgr_client_new(PC_LISTENING);
		pkgmgr_client_set_status_type(ad->pc, event_type);
		pkgmgr_client_listen_status(ad->pc, _double_press_app_event_cb, ad);
	}

	int event_type2 = PMINFO_CLIENT_STATUS_UNINSTALL;

	if (pc2) {
		pkgmgr_client_free(pc2);
		pc2 = NULL;

		if (!ad->pc2) {
			ad->pc2 = pkgmgr_client_new(PMINFO_LISTENING);
			pkgmgr_client_set_status_type(ad->pc2, event_type2);
			pkgmgr_client_listen_status(ad->pc2, _double_press_app_uninstall_event_cb, ad);
		}
#endif

		_make_app_list(ad);

		register_vconf_changing(VCONFKEY_WMS_POWERKEY_DOUBLE_PRESSING, change_double_pressing_cb, ad);
		register_vconf_changing(VCONFKEY_LANGSET, change_language_cb, ad);
	}

	Evas_Object *create_double_list(void * data) {
		appdata *ad = data;

		if (!ad) {
			ERR("appdata is null!!");
			return NULL;
		}

		Evas_Object *genlist = NULL;
		Evas_Object *layout = NULL;
		int idx = 0;

		Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
		itc->item_style = "2text";
		itc->func.text_get = _gl_double_title_get;

		layout = elm_layout_add(ad->nf);
		elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		genlist = elm_genlist_add(layout);
		elm_genlist_block_count_set(genlist, 14);
		elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
		evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		/*elm_genlist_item_append(genlist, itc, NULL, NULL, */
		/*		ELM_GENLIST_ITEM_NONE, _double_app_list_cb, ad); */

		elm_genlist_item_class_free(itc);

		g_double_genlist = genlist;

		elm_object_part_content_set(layout, "elm.genlist", genlist);

		return layout;
	}

	void clear_double_cb(void * data , Evas * e, Evas_Object * obj, void * event_info) {
		FREE(pitem_none);
		FREE(pitem_recent);
		g_double_genlist = NULL;
		g_double_app_genlist = NULL;
		unregister_vconf_changing(VCONFKEY_WMS_POWERKEY_DOUBLE_PRESSING, change_double_pressing_cb);
		unregister_vconf_changing(VCONFKEY_LANGSET, change_language_cb);
	}

