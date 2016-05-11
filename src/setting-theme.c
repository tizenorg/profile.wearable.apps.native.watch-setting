/*
 * setting-theme.c
 *
 *	Created on: Aug 7, 2014
 *		Author: min-hoyun
 */

#include <feedback.h>

#include "setting-theme.h"
#include "setting_view_toast.h"


static Thumbnail_Data thumbnail_arr[COLOR_THEME_COUNT] = {
	{ EA_THEME_COLOR_TABLE_1, EA_THEME_STYLE_DARK,	"settings_preview_dark.png"		},
	{ EA_THEME_COLOR_TABLE_1, EA_THEME_STYLE_LIGHT, "settings_preview_light.png"	},
	{ EA_THEME_COLOR_TABLE_1 + 1, EA_THEME_STYLE_DARK, "settings_preview_dark_blue.png"		},
	{ EA_THEME_COLOR_TABLE_1 + 1, EA_THEME_STYLE_LIGHT, "settings_preview_light_blue.png"	}
};

int center_item = 0;
static appdata *g_ad;

static Evas_Object *setting_theme_create_thumbnail_layout(void *data);
static void setting_theme_thumbnail_scroll(void *data, Evas_Object *obj, void *ei);
static void setting_theme_index_refresh(void *data);
static void	setting_theme_index_sync(void *data);
static void	setting_theme_layout_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void	setting_theme_on_index_mouse_down_cb(void *data, Evas *e, Evas_Object *o, void *event_info);
static void	setting_theme_on_index_mouse_up_cb(void *data, Evas *e, Evas_Object *o, void *event_info);
static void	setting_theme_on_index_mouse_move_cb(void *data, Evas *e, Evas_Object *o, void *event_info);
static void	setting_theme_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void setting_theme_mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void setting_theme_mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void setting_theme_layout_del_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);




void setting_theme_show_thumbnail(void *data, Evas_Object *obj, void *event_info)
{
	DBG("setting_theme_show_thumbnail");

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	appdata *ad = data;
	if (ad == NULL)
		return;

	Theme_Data *td = (Theme_Data *)calloc(1, sizeof(Theme_Data));
	td->ad = ad;
	td->curr_theme_id = 1;	/*EA_THEME_COLOR_TABLE_1; */
	td->theme_layout = setting_theme_create_thumbnail_layout(td);
	if (td->theme_layout) {
		Elm_Object_Item *it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, td->theme_layout, NULL);
		elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
	}

	int ret;
	ret = feedback_initialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		DBG("feedback_initialize failed");
	}

	g_ad = ad;
}

static Evas_Object *setting_theme_create_thumbnail_layout(void *data)
{
	DBG("setting_display_create_brightness_layout");

	Evas_Object *layout, *scroller, *box, *page_layout, *page, *mapbuf;
	Eina_List *colors;
	Ea_Theme_Color_hsv *hsv_color;
	Evas_Object *image;

	int start, r, g, b;

	int i;

	Theme_Data *td = data;
	if (td == NULL)
		return NULL;

	vconf_get_int("db/setting/color_theme_type", &td->curr_theme_type);

	DBG("Color theme type: %d", td->curr_theme_type);

	/* Create Layout */
	layout = elm_layout_add(td->ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/index_thumbnail");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, setting_theme_layout_del_cb, td);

	/* Create Scroller */
	scroller = elm_scroller_add(layout);
	elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	/*	elm_scroller_page_relative_set(scroller, 1.0, 0.0); */
	elm_scroller_page_size_set(scroller, 248, 0);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_page_scroll_limit_set(scroller, 1, 0);
	elm_object_scroll_lock_y_set(scroller, EINA_TRUE);
	elm_object_part_content_set(layout, "scroller", scroller);
	elm_object_style_set(scroller, "effect");
	evas_object_show(scroller);
	td->scroller = scroller;

	/* Create Box */
	box = elm_box_add(scroller);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_object_content_set(scroller, box);
	evas_object_show(box);

	Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(box));
	evas_object_size_hint_min_set(rect, 56, 480);
	/*	ea_theme_object_color_set(rect, "B011"); */
	elm_box_pack_end(box, rect);

	/* Create Pages */
	for (i = 0; i < COLOR_THEME_COUNT; ++i) {
		page_layout = elm_layout_add(box);
		evas_object_size_hint_weight_set(page_layout, 0, 0);
		evas_object_size_hint_align_set(page_layout, 0, EVAS_HINT_FILL);
		elm_layout_theme_set(page_layout, "layout", "body_thumbnail", "default");
		evas_object_show(page_layout);

		image = elm_image_add(page_layout);
		elm_image_file_set(image, EDJE_PATH, thumbnail_arr[i].prev_img_path);
		evas_object_size_hint_align_set(image, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		/* touch event */
		evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, setting_theme_mouse_down_cb, NULL);
		evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_UP, setting_theme_mouse_up_cb, (void *)i);
		evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_MOVE, setting_theme_mouse_move_cb, NULL);

		elm_object_part_content_set(page_layout, "elm.icon", image);

		Evas_Object *ao = elm_access_object_register(image, page_layout);
		if (ao) {
			elm_access_activate_cb_set(ao, setting_theme_mouse_up_cb, (void *)i);
		}

		if (i == td->curr_theme_type) {
			elm_object_signal_emit(page_layout, "elm,state,thumbnail,focus", "*");
		} else {
			elm_object_signal_emit(page_layout, "elm,state,thumbnail,unfocus", "*");
		}

		mapbuf = elm_mapbuf_add(box);
		evas_object_size_hint_weight_set(mapbuf, 0, 0);
		evas_object_size_hint_align_set(mapbuf, 0, EVAS_HINT_FILL);
		evas_object_show(mapbuf);
		elm_object_content_set(mapbuf, page_layout);
		td->mapbuf[i] = mapbuf;

		elm_box_pack_end(box, mapbuf);
	}

	rect = evas_object_rectangle_add(evas_object_evas_get(box));
	evas_object_size_hint_min_set(rect, 56, 480);
	/*	ea_theme_object_color_set(rect, "B011"); */
	elm_box_pack_end(box, rect);

	/* Use Index */
	evas_object_smart_callback_add(scroller, "scroll", setting_theme_thumbnail_scroll, td);

	evas_object_event_callback_add(scroller, EVAS_CALLBACK_RESIZE, setting_theme_layout_resize_cb, td);

	Evas_Object *index = elm_index_add(layout);
	elm_object_style_set(index, "thumbnail");
	evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(index, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_index_horizontal_set(index, EINA_TRUE);
	elm_index_autohide_disabled_set(index, EINA_TRUE);
	elm_object_part_content_set(layout, "controller", index);


	for (i = 0; i < COLOR_THEME_COUNT; ++i) {
		td->it[i] = elm_index_item_append(index, NULL, NULL, (void *) i);
	}

	elm_object_item_style_set(td->it[center_item - 1] , "item/vertical/center");

	td->min_page = 0;
	td->max_page = COLOR_THEME_COUNT;

	elm_index_level_go(index, 0);
	elm_index_item_selected_set(td->it[0], EINA_TRUE);
	td->curr_page = 0;
	td->last_it = td->it[0];
	td->index = index;

	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_DOWN, setting_theme_on_index_mouse_down_cb, td);
	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_MOVE, setting_theme_on_index_mouse_move_cb, td);
	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_UP, setting_theme_on_index_mouse_up_cb, td);

	return layout;
}

static void setting_theme_thumbnail_scroll(void *data, Evas_Object *obj, void *ei)
{
	int cur_page;
	Theme_Data *td = (Theme_Data *)data;
	elm_scroller_current_page_get(td->scroller, &cur_page, NULL);
	if (cur_page != td->curr_page) {
		DBG("scroll: %d\n", td->curr_page);

		td->curr_page = cur_page;

		setting_theme_index_sync(td);
	}
}

static void	setting_theme_index_sync(void *data)
{
	Theme_Data *td = (Theme_Data *)data;

	Elm_Object_Item *it = elm_index_item_find(td->index, (void *)td->curr_page);
	if (it) {
		elm_index_item_selected_set(it, EINA_TRUE);
		td->last_it = it;
		td->new_it	= it;
	} else {
		setting_theme_index_refresh(td);
	}
}

static void setting_theme_index_refresh(void *data)
{
	int i;
	Elm_Object_Item *it;
	Theme_Data *td = (Theme_Data *)data;

	elm_index_item_clear(td->index);
	if (td->curr_page < td->min_page) {
		for (i = td->curr_page ; i < td->curr_page + COLOR_THEME_COUNT ; i++) {
			it = elm_index_item_append(td->index, NULL, NULL, (void *) i);

			if (i == center_item)
				elm_object_item_style_set(it, "item/vertical/center");
		}
		td->min_page = td->curr_page;
		td->min_page = td->curr_page + COLOR_THEME_COUNT - 1;
	} else {
		for (i = td->curr_page - 2 + 1; i < td->curr_page + 1 ; i++) {
			it = elm_index_item_append(td->index, NULL, NULL, (void *) i);
			if (i == center_item)
				elm_object_item_style_set(it, "item/vertical/center");
		}
		td->min_page = td->curr_page - 2;
		td->min_page = td->curr_page - 1;
	}
	elm_index_level_go(td->index, 0);

	setting_theme_index_sync(td);
}

static void	setting_theme_layout_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("setting_theme_layout_resize_cb");

	Theme_Data *td = (Theme_Data *)data;
	if (td) {
		elm_scroller_page_show(obj, td->curr_theme_type, 0);

		td->curr_page = td->curr_theme_type;

		setting_theme_index_sync(td);
	}
}

static void	setting_theme_on_index_mouse_down_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	Theme_Data *pd = (Theme_Data *)data;

	/* Keep the last index item active and save the selected index item */
	if (!pd->last_it) return;

	int level = elm_index_item_level_get(o);
	pd->new_it = elm_index_selected_item_get(o, level);
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);
}

static void	setting_theme_on_index_mouse_up_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	Theme_Data *pd = (Theme_Data *)data;

	/* Keep the last index item active and move to the page of the currently selected index item */
	if (!pd->last_it) return;
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);

	if (!pd->new_it) return;

	int idx = (int) elm_object_item_data_get(pd->new_it);
	if (idx == pd->curr_page) return;

	elm_scroller_page_bring_in(pd->scroller, idx, 0);
}

static void	setting_theme_on_index_mouse_move_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	Theme_Data *pd = (Theme_Data *)data;

	/* Keep the last index item active and save the currently selected index item */
	if (!pd->last_it) return;

	int level = elm_index_item_level_get(o);
	pd->new_it = elm_index_selected_item_get(o, level);
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);
}

static int prev_x = 0;
static int touch_mode = NONE;

static void	setting_theme_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	touch_mode = TOUCH_DOWN;

	Evas_Event_Mouse_Down *ev = event_info;
	prev_x = ev->canvas.x;
}

static void setting_theme_mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	if (touch_mode == TOUCH_MOVE)
		return;

	feedback_play(FEEDBACK_PATTERN_TAP);

	int id = (int *)data;

	DBG("Id: %d", id);

	int curr_theme_type = 0;
	vconf_get_int("db/setting/color_theme_type", &curr_theme_type);

	int ret = ea_theme_input_colors_set(thumbnail_arr[id].id, thumbnail_arr[id].theme_type);

	DBG("ea_theme_input_colors_set ret : %d", ret);
	if (!ret) {
		ERR("ea_theme_input_colors_set() failed!");
		/*return; */
	}

	if (id != curr_theme_type) {
		DBG("Color Id: %d, type: %d", thumbnail_arr[id].id, thumbnail_arr[id].theme_type);

		vconf_set_int("db/setting/color_theme_type", id);

		/* automatic freed!! */
		struct _toast_data *toast = _create_toast(g_ad, _("IDS_ST_TPOP_COLOUR_THEME_CHANGED"));
		if (toast) {
			_show_toast(g_ad, toast);
		}
	} else {
		/* automatic freed!! */
		struct _toast_data *toast = _create_toast(g_ad, _("IDS_ST_TPOP_COLOUR_THEME_ALREADY_SET"));
		if (toast) {
			_show_toast(g_ad, toast);
		}
	}

	if (g_ad) {
		elm_naviframe_item_pop(g_ad->nf);
	}
}

static void setting_theme_mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Move *ev = event_info;

	int cur_x = ev->cur.canvas.x;
	if (abs(cur_x - prev_x) > 15) {
		/* todo : confirm and remove */
		touch_mode = TOUCH_MOVE;
	}
}

static void setting_theme_layout_del_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	DBG("setting_theme_layout_del_cb");

	Theme_Data *td = data;
	if (td)
		free(td);

	int ret;
	ret = feedback_deinitialize();
	if (ret != FEEDBACK_ERROR_NONE) {
		DBG("feedback_deinitialize failed");
	}
}
