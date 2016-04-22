/*
 * setting-notification.c
 *
 *  Created on: Feb 14, 2014
 *      Author: min-hoyun
 */

#include "setting-notification.h"


static struct _noti_menu_item noti_menu_its[] = {
	{ "IDS_ST_BUTTON_NOTIFICATIONS", 	 1, _noti_gl_enabling_noti_cb },
	{ NULL, 0, NULL }
};

static struct _noti_data g_noti_data;
static bool is_called_myself;


void _initialize_noti()
{
	is_called_myself = false;

	g_noti_data.g_noti_genlist = NULL;
	g_noti_data.is_enable_noti = 0;
	g_noti_data.temp_ad = NULL;

	register_vconf_changing(VCONF_WMS_NOTIFICATION_KEY, noti_enabling_vconf_changed_cb, NULL);
}

void _noti_gl_enabling_noti_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _noti_gl_enabling_noti_cb() is called!");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	appdata *ad = data;

	if (ad == NULL) {
		DBG("%s", "_noti_gl_enabling_noti_cb - ad or check is null");
		return;
	}

	is_called_myself = true;

	g_noti_data.is_enable_noti = (g_noti_data.is_enable_noti) ? 0 : 1;

	vconf_set_int(VCONF_WMS_NOTIFICATION_KEY, g_noti_data.is_enable_noti);

	elm_genlist_item_selected_set(it, EINA_FALSE);

	if (g_noti_data.g_noti_genlist) {
		elm_genlist_realized_items_update(g_noti_data.g_noti_genlist);
	}
}

Eina_Bool _clear_noti_cb(void *data, Elm_Object_Item *it)
{
	DBG("Setting - _clear_noti_cb() is called!");

	is_called_myself = false;

	g_noti_data.g_noti_genlist = NULL;
	g_noti_data.is_enable_noti = 0;
	g_noti_data.temp_ad = NULL;

	unregister_vconf_changing(VCONF_WMS_NOTIFICATION_KEY, noti_enabling_vconf_changed_cb);

	return EINA_TRUE;
}

char *_gl_noti_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Noti_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(noti_menu_its[index % 1].name));
	} else if (!strcmp(part, "elm.text.1")) {
		vconf_get_int(VCONF_WMS_NOTIFICATION_KEY, &g_noti_data.is_enable_noti);
		snprintf(buf, sizeof(buf) - 1, "%s", (g_noti_data.is_enable_noti) ? _("IDS_EMAIL_BODY_ENABLED_M_STATUS")
				 : _("IDS_ST_MBODY_DISABLED_ABB"));
	}
	return strdup(buf);
}

Evas_Object *_gl_noti_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);
		elm_object_style_set(check, "list");
		vconf_get_int(VCONF_WMS_NOTIFICATION_KEY, &g_noti_data.is_enable_noti);
		elm_check_state_set(check, (g_noti_data.is_enable_noti) ? EINA_TRUE : EINA_FALSE);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_repeat_events_set(check, EINA_TRUE);
	}

	return check;
}

static void _noti_gl_del(void *data, Evas_Object *obj)
{
	Noti_Item_Data *id = data;
	if (id)
		free(id);
}

Evas_Object *_create_noti_list(void *data)
{
	DBG("Setting - _create_noti_list() is called!");

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_noti_list - appdata is null");
		return NULL;
	}
	Evas_Object *genlist  = NULL;
	struct _noti_menu_item *menu_its = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text.1icon.1";
	itc->func.text_get = _gl_noti_title_get;
	itc->func.content_get = _gl_noti_check_get;
	itc->func.del = _noti_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	menu_its = noti_menu_its;

	connect_to_wheel_with_genlist(genlist,ad);
#if 0
	device_info_h *device_info = NULL;
	bundle *b = NULL;
	char *val = NULL;
	capability_manager_create_device_info(&device_info);
	capability_manager_get_device_features(device_info, &b);
	val = bundle_get_val(b, "smartrelay");
	capability_manager_destroy_device_info(device_info);
#endif

	for (idx = 0; idx < NOTIFICATION_ITEM_COUNT; idx++) {
		Noti_Item_Data *id = calloc(sizeof(Noti_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
						   genlist,				/* genlist object */
						   itc,				/* item class */
						   id,		            	/* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_its[ idx ].func,	/* call back */
						   ad);
		}
	}
	elm_genlist_item_class_free(itc);

	g_noti_data.g_noti_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	return layout;
}

void noti_enabling_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - noti_enabling_vconf_changed_cb() is called!");

	if (is_called_myself) {
		DBG("Setting - is_called_myself!! Return!!");
		is_called_myself = false;
		return;
	}

	g_noti_data.is_enable_noti = vconf_keynode_get_int(key);

	DBG("Setting - Noti value: %d", g_noti_data.is_enable_noti);

	if (g_noti_data.g_noti_genlist) {
		elm_genlist_realized_items_update(g_noti_data.g_noti_genlist);
	}
}
