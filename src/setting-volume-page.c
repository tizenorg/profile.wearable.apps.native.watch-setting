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

#include "setting-volume.h"
#include "setting_control_haptic.h"
#include "setting-common-sound.h"
#include "setting_data_vconf.h"

EAPI Evas_Object	 *elm_widget_top_get(const Evas_Object *obj);

static void _set_volumn(sound_type_e type, int volume_index, char *vconf_key);
/*static void _change_to_vibrate_mode(); */
static void _play_sound_all_type(int sound_type, float volume);


static appdata *g_ad;
static Evas_Object *g_volume_spinner = NULL;
static Evas_Object *g_volume_genlist = NULL;

static int is_changing_level_by_vconf = 0;
static int is_changed = 0;
static int is_myself_changing = 0;
static int is_myself_ringtone_changing = 0;

static int is_sound_changed = 0;
static int is_vibrate_changed = 0;
static int is_play_media_sound = 0;

typedef void
(*system_part_volume_cb)(void *data, Evas_Object *obj, void *event_info);

static void
multimedia_value_changed_page(int idx)
{

	if (curr_sound_type != SOUND_TYPE_MEDIA) {
		if (is_changing_level_by_vconf) {
			DBG("Setting - is_changing_level_by_vconf!!!!");

			is_changing_level_by_vconf = 0;
			return;
		}
	}

	is_changed = 1;		/* changed flag!! */
	volume_index = idx;
	DBG("Setting - volume_index : %d", volume_index);

	if (curr_sound_type == SOUND_TYPE_MEDIA) {
		if (!is_play_media_sound) {
			_play_sound_all_type(curr_sound_type, 0.0);
			is_play_media_sound = 0;
		} else {
			is_play_media_sound = 0;
		}
	} else {
		_play_sound_all_type(curr_sound_type, 0.0);
	}
}

static void
ringtone_value_changed_page(int idx)
{
	DBG("Setting - ringtone_value_changed() is called!");
	if (get_sound_mode() == SOUND_MODE_SOUND) {

		if (curr_sound_type != SOUND_TYPE_MEDIA) {
			if (is_changing_level_by_vconf) {
				DBG("Setting - is_changing_level_by_vconf!!!!");

				is_changing_level_by_vconf = 0;
				return;
			}
		}

		is_changed = 1;		/* changed flag!! */

		volume_index = idx;

		DBG("Setting - volume_index : %d", volume_index);

		if (curr_sound_type == SOUND_TYPE_MEDIA) {
			if (!is_play_media_sound) {
				_play_sound_all_type(curr_sound_type, 0.0);
				is_play_media_sound = 0;
			} else {
				is_play_media_sound = 0;
			}
		} else {
			_play_sound_all_type(curr_sound_type, 0.0);
		}
	}
}

void _initialize_volume()
{
	volume_index = 0;
	original_volume = 0;
	is_wav_playing_vol = SOUND_STATE_STOP;
	sound_id_vol = -1;
	is_changed = 0;
	is_changing_level_by_vconf = 0;
	is_myself_changing = 0;
	is_myself_ringtone_changing = 0;

	is_sound_changed = 0;
	is_vibrate_changed = 0;
	is_play_media_sound = 0;

	curr_sound_type = SOUND_TYPE_RINGTONE;

	if (is_created_player()) {
		_close_player(NULL, SOUND_TYPE_RINGTONE);
	}
}

void _clear_volume_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	_clear_volume_resources();

	return;
}

static void stop_wav()
{
	if (is_wav_playing_vol == SOUND_STATE_PLAY) {
		DBG("Setting - sound id : %d", sound_id_vol);

		wav_player_stop(sound_id_vol);
		is_wav_playing_vol = SOUND_STATE_STOP;
		sound_id_vol = -1;
	}
}

void _stop_all_volume_sound()
{
	DBG("Setting - stop all volume sound.");

	if (is_created_player()) {
		_close_player(NULL, curr_sound_type);
	}

	stop_wav();
}

void _clear_volume_resources()
{
	DBG("Setting - _clear_volume_resources()");

	if (is_created_player()) {
		_close_player(NULL, curr_sound_type);
	}

	stop_wav();

	g_ad = NULL;
	g_volume_genlist = NULL;
	g_volume_spinner = NULL;
	is_myself_changing = 0;
	is_myself_ringtone_changing = 0;
	is_sound_changed = is_vibrate_changed = 0;
}

#define NUM_ITEMS			  5
#define NUM_INDEX			  5
#define NUM_ITEMS_CIRCLE_EVEN 6
#define NUM_INDEX_CIRCLE_EVEN 6

typedef struct _page_data page_data;
struct _page_data {
	Evas_Object *main_layout;
	Evas_Object *scroller;
	Evas_Object *box;
	Evas_Object *mapbuf[NUM_ITEMS_CIRCLE_EVEN];
	Evas_Object *slider[NUM_ITEMS_CIRCLE_EVEN];
	Evas_Object *index;
	Evas_Object *page_layout[NUM_ITEMS_CIRCLE_EVEN];
	int cur_page;
	int prev_page;
	int slider_value[NUM_ITEMS_CIRCLE_EVEN];
	Elm_Object_Item *it[NUM_ITEMS_CIRCLE_EVEN];

	Elm_Object_Item *last_it;
	Elm_Object_Item *new_it;
	int min_page, max_page;
};



static void _index_sync(void *data);

static void
_on_index_mouse_down_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	ERR("in");
	page_data *pd = (page_data *)data;

	/* Keep the last index item active and save the selected index item */
	if (!pd->last_it) return;

	int level = elm_index_item_level_get(o);
	pd->new_it = elm_index_selected_item_get(o, level);
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);
}

static void
_on_index_mouse_up_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	ERR("in");
	page_data *pd = (page_data *)data;

	/* Keep the last index item active and move to the page of the currently selected index item */
	if (!pd->last_it) return;
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);

	if (!pd->new_it) return;

	int idx = (int) elm_object_item_data_get(pd->new_it);
	if (idx == pd->cur_page) return;

	elm_scroller_page_bring_in(pd->scroller, idx, 0);
	eext_circle_object_value_set(pd->slider[idx], (float)pd->slider_value[idx]);
	eext_rotary_object_event_activated_set(pd->slider[idx], EINA_TRUE);

}

static void
_on_index_mouse_move_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	ERR("in");
	page_data *pd = (page_data *)data;

	/* Keep the last index item active and save the currently selected index item */
	if (!pd->last_it) return;

	int level = elm_index_item_level_get(o);
	pd->new_it = elm_index_selected_item_get(o, level);
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);
}

static void
_index_refresh(void *data)
{
	int i, num_index;
	page_data *pd = (page_data *)data;

	num_index = pd->max_page + 1;

	elm_index_item_clear(pd->index);
	if (pd->cur_page < pd->min_page) {
		for (i = pd->cur_page ; i < pd->cur_page + num_index ; i++) {
			printf("Added : %d\n", i);
			elm_index_item_append(pd->index, NULL, NULL, (void *) i);
		}
		pd->min_page = pd->cur_page;
		pd->min_page = pd->cur_page + num_index - 1;
	} else {
		for (i = pd->cur_page - num_index + 1; i < pd->cur_page + 1 ; i++) {
			printf("Added : %d\n", i);
			elm_index_item_append(pd->index, NULL, NULL, (void *) i);
		}
		pd->min_page = pd->cur_page - num_index;
		pd->min_page = pd->cur_page - 1;
	}
	elm_index_level_go(pd->index, 0);
	_index_sync(pd);
}

static void
_index_sync(void *data)
{
	page_data *pd = (page_data *)data;
	Elm_Object_Item *it;
	it = elm_index_item_find(pd->index, (void *)pd->cur_page);
	if (it) {
		elm_index_item_selected_set(it, EINA_TRUE);
		pd->last_it = it;
		pd->new_it = it;
	} else
		_index_refresh(pd);
}

static void
_layout_del_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	page_data *pd = (page_data *)data;
	free(pd);
}

static void
_layout_resize_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	int w1, h1, w2, h2;
	page_data *pd = (page_data *)data;

	evas_object_geometry_get(obj, NULL, NULL, &w1, &h1);
	evas_object_geometry_get(pd->main_layout, NULL, NULL, &w2, &h2);

	elm_scroller_page_size_set(pd->scroller, w1, h1);
}

static Eina_Bool
animator_cb(void *data)
{
	int i, num_items;
	page_data *pd = (page_data *)data;

	num_items = pd->max_page + 1;

	/*Since mapbuf is not perfect, Enable them after the size calculation is finished */
	for (i = 0; i < num_items; ++i) {
		elm_mapbuf_enabled_set(pd->mapbuf[i], EINA_TRUE);
	}
	return ECORE_CALLBACK_CANCEL;
}

static void
_scroll(void *data, Evas_Object *obj, void *ei)
{
	int cur_page;
	page_data *pd = (page_data *)data;
	elm_scroller_current_page_get(pd->scroller, &cur_page, NULL);
	if (cur_page != pd->cur_page) {
		printf("scroll: %d\n", pd->cur_page);
		pd->prev_page = pd->cur_page;
		pd->cur_page = cur_page;
		if ((pd->cur_page >= NUM_ITEMS_CIRCLE_EVEN) || (pd->prev_page >= NUM_ITEMS_CIRCLE_EVEN))
			return;
		elm_object_signal_emit(pd->page_layout[pd->cur_page], "elm,state,thumbnail,select", "elm");
		elm_object_signal_emit(pd->page_layout[pd->prev_page], "elm,state,thumbnail,unselect", "elm");
		_index_sync(pd);

		if (is_created_player()) {
			_close_player(g_ad, curr_sound_type);
		}

		stop_wav();

		ERR("STOP sound player and wav ");
		/*change sound mode */
		switch (cur_page) {
		case 0: /*media */
			curr_sound_type = SOUND_TYPE_MEDIA;
			break;
		case 1: /*nodification */
			curr_sound_type = SOUND_TYPE_NOTIFICATION;
			break;
		case 2: /*system */
			curr_sound_type = SOUND_TYPE_SYSTEM;
			break;
		default:
			break;
		}
	}
}

static Eina_Bool
_value_changed_rotary(void *data, Evas_Object *obj, Eext_Rotary_Event_Info *info)
{
	char buf[1024];
	/*Evas_Object *layout = (Evas_Object *)data; */
	page_data *pd = (page_data *)data;
	int cur_page = 0;
	elm_scroller_current_page_get(pd->scroller, &cur_page, NULL);

	if (info->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE) {
		if (pd->slider_value[cur_page] < 10)
			pd->slider_value[cur_page]++;
	} else {
		if (pd->slider_value[cur_page] > 0)
			pd->slider_value[cur_page]--;
	}
	snprintf(buf, sizeof(buf), "%02d", pd->slider_value[cur_page]);
	ERR("Slider value = %s\n", buf);
	elm_object_part_text_set(pd->page_layout[cur_page], "elm.text.slider", buf);

	switch (cur_page) {
	case 0: /*media */
		multimedia_value_changed_page(pd->slider_value[cur_page]);
		break;
	case 1: /*nodification */
		ringtone_value_changed_page(pd->slider_value[cur_page]);
		break;
	case 2: /*system */
		ringtone_value_changed_page(pd->slider_value[cur_page]);
		break;
	default:
		break;
	}

	return EINA_TRUE;
}

Eina_Bool _clear_volume_setting_cb(void *data, Elm_Object_Item *it)
{
	page_data *pd = (page_data *)data;

	if (is_created_player()) {
		_close_player(g_ad, curr_sound_type);
	}

	stop_wav();

	g_ad = NULL;
	g_volume_genlist = NULL;
	g_volume_spinner = NULL;
	is_myself_changing = 0;
	is_myself_ringtone_changing = 0;
	is_sound_changed = is_vibrate_changed = 0;

	if (pd) free(pd);

	return EINA_TRUE;
}

static void _press_plus_volume_cb(void *data, Evas_Object *obj, void *event_info)
{
	char buf[1024];
	/*Evas_Object *layout = (Evas_Object *)data; */
	page_data *pd = (page_data *)data;
	int cur_page = 0;
	elm_scroller_current_page_get(pd->scroller, &cur_page, NULL);

	if (pd->slider_value[cur_page] < 10)
		pd->slider_value[cur_page]++;

	snprintf(buf, sizeof(buf), "%02d", pd->slider_value[cur_page]);
	ERR("Slider value = %s\n", buf);
	elm_object_part_text_set(pd->page_layout[cur_page], "elm.text.slider", buf);

	switch (cur_page) {
	case 0: /*media */
		multimedia_value_changed_page(pd->slider_value[cur_page]);
		break;
	case 1: /*nodification */
		ringtone_value_changed_page(pd->slider_value[cur_page]);
		break;
	case 2: /*system */
		ringtone_value_changed_page(pd->slider_value[cur_page]);
		break;
	default:
		break;
	}
}

static void _press_minus_volume_cb(void *data, Evas_Object *obj, void *event_info)
{
	char buf[1024];
	/*Evas_Object *layout = (Evas_Object *)data; */
	page_data *pd = (page_data *)data;
	int cur_page = 0;
	elm_scroller_current_page_get(pd->scroller, &cur_page, NULL);

	if (pd->slider_value[cur_page] > 0)
		pd->slider_value[cur_page]--;

	snprintf(buf, sizeof(buf), "%02d", pd->slider_value[cur_page]);
	ERR("Slider value = %s\n", buf);
	elm_object_part_text_set(pd->page_layout[cur_page], "elm.text.slider", buf);

	switch (cur_page) {
	case 0: /*media */
		multimedia_value_changed_page(pd->slider_value[cur_page]);
		break;
	case 1: /*nodification */
		ringtone_value_changed_page(pd->slider_value[cur_page]);
		break;
	case 2: /*system */
		ringtone_value_changed_page(pd->slider_value[cur_page]);
		break;
	default:
		break;
	}
}

void _create_volume_page(void *data)
{

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_volume_page - appdata is null");
		return;
	}

	g_ad = ad;

	Evas_Object *layout, *scroller, *box, *img, *page_layout, *index;
	Elm_Object_Item *nf_it;
	char img_path[PATH_MAX];
	int i, max_items;

	page_data *pd = calloc(1, sizeof(page_data));

	/* Create Layout */
	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "sound-volume/index_thumbnail");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _layout_del_cb, pd);
	pd->main_layout = layout;

	/* Create Scroller */
	scroller = elm_scroller_add(layout);
	elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_page_scroll_limit_set(scroller, 1, 0);
	elm_object_scroll_lock_y_set(scroller, EINA_TRUE);
	elm_object_part_content_set(layout, "scroller", scroller);
	evas_object_show(scroller);
	evas_object_smart_callback_add(scroller, "scroll", _scroll, pd);
	pd->scroller = scroller;

	/* Create Box */
	box = elm_box_add(scroller);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_object_content_set(scroller, box);
	evas_object_show(box);
	pd->box = box;

	/* Create Pages */
	max_items = 3;

	char *img_names[3] = {
		"b_setting_multi_media.png",
		"b_setting_multi_notification.png",
		"b_setting_multi_system.png"
	};

	char *img_string[3] = {
		"Media",
		"Notifications",
		"System"
	};


	char *vconf_name[3] = {
		VCONFKEY_SETAPPL_MEDIA_SOUND_VOLUME_INT,
		VCONFKEY_SETAPPL_NOTI_SOUND_VOLUME_INT,
		VCONFKEY_SETAPPL_TOUCH_FEEDBACK_SOUND_VOLUME_INT
	};

	for (i = 0; i < max_items; ++i) {
		page_layout = elm_layout_add(box);
		pd->page_layout[i] = page_layout;
		evas_object_size_hint_weight_set(page_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(page_layout, 0, EVAS_HINT_FILL);
		elm_layout_file_set(page_layout, EDJE_PATH, "slider_layout");
		evas_object_show(page_layout);

		/*		elm_box_pack_end(box, page_layout); */

		Evas_Object *table, *rect;
		Evas_Object *slider = NULL;
		int w, h;

		table = elm_table_add(box);

		elm_win_screen_size_get((const Elm_Win *)elm_widget_top_get((Evas_Object *)ad->nf), NULL, NULL, &w, &h);
		ERR("Width : %d, Height : %d", w, h);
		rect = evas_object_rectangle_add(evas_object_evas_get(table));
		evas_object_size_hint_min_set(rect, w*0.95, h*0.7);
		evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_table_pack(table, rect, 0, 0, 1, 1);

		/* Add new eext_circle_object_slider with Eext_Circle_Surface object to render.
		Value is set as 3 which is within range from 0 to 15. */
		pd->slider[i] = slider = eext_circle_object_slider_add(page_layout, ad->circle_surface);
		eext_circle_object_value_min_max_set(slider, 0.0, 10.0);
		vconf_get_int(vconf_name[i], &pd->slider_value[i]);

		eext_circle_object_value_set(slider, (float)pd->slider_value[i]);

		/* Activate Circle slider's rotary object event.
		Its value increases/decreases its value by 0.5 to clockwise/counter clockwise
		rotary event. */
		/*eext_rotary_object_event_activated_set(slider, EINA_TRUE); */
		eext_circle_object_slider_step_set(slider, 1.0);

		char buf[1024];
		snprintf(buf, sizeof(buf), "%02d", pd->slider_value[i]);
		elm_object_part_text_set(page_layout, "elm.text.slider", buf);

		Eina_Bool res = eext_rotary_object_event_callback_add(slider, _value_changed_rotary, pd);
		ERR("rotary_event_handler result = %d", res);

		Evas_Object *btn_minus;
		btn_minus = elm_image_add(page_layout);
		snprintf(img_path, sizeof(img_path), "%s/minus_btn.png", IMG_DIR);
		elm_image_file_set(btn_minus, img_path, NULL);
		elm_object_part_content_set(page_layout, "btn1", btn_minus);
		evas_object_smart_callback_add(btn_minus, "clicked", _press_minus_volume_cb, pd);

		Evas_Object *btn_plus;
		btn_plus = elm_image_add(page_layout);
		snprintf(img_path, sizeof(img_path), "%s/plus_btn.png", IMG_DIR);
		elm_image_file_set(btn_plus, img_path, NULL);
		elm_object_part_content_set(page_layout, "btn2", btn_plus);
		evas_object_smart_callback_add(btn_plus, "clicked", _press_plus_volume_cb, pd);

		img = elm_image_add(page_layout);
		snprintf(img_path, sizeof(img_path), "%s/%s", IMG_DIR, img_names[i]);
		elm_image_file_set(img, img_path, NULL);
		elm_object_part_content_set(page_layout, "elm.icon", img);
		elm_object_part_text_set(page_layout, "elm.text.bottom", img_string[i]);

		/* Make unselect state all of the pages except first one */
		if (i)
			elm_object_signal_emit(page_layout, "elm,state,thumbnail,unselect", "elm");

		evas_object_size_hint_weight_set(page_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(page_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_table_pack(table, page_layout, 0, 0, 1, 1);

		evas_object_size_hint_weight_set(table, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(table);
		elm_box_pack_end(box, table);
	}

	eext_rotary_object_event_activated_set(pd->slider[0], EINA_TRUE);
	/*Add resize callback for get a actual size of layout and main layout */
	evas_object_event_callback_add(pd->page_layout[0], EVAS_CALLBACK_RESIZE, _layout_resize_cb, pd);

	/* Add animator for page transfer effect */
	ecore_animator_add(animator_cb, pd);

	/* Create Index */
	index = elm_index_add(layout);
	elm_object_style_set(index, "thumbnail");
	evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(index, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_index_horizontal_set(index, EINA_TRUE);
	elm_index_autohide_disabled_set(index, EINA_TRUE);
	elm_object_part_content_set(layout, "controller", index);

	for (i = 0; i < max_items; ++i) {
		pd->it[i] = elm_index_item_append(index, NULL, NULL, (void *) i);
	}

	pd->min_page = 0;
	pd->max_page = max_items - 1;
	elm_index_level_go(index, 0);
	elm_index_item_selected_set(pd->it[0], EINA_TRUE);

	pd->cur_page = 0;
	pd->index = index;
	pd->last_it = pd->it[0];

	curr_sound_type = SOUND_TYPE_MEDIA;

	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_DOWN, _on_index_mouse_down_cb, pd);
	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_MOVE, _on_index_mouse_move_cb, pd);
	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_UP, _on_index_mouse_up_cb, pd);


	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
	elm_naviframe_item_pop_cb_set(nf_it, _clear_volume_setting_cb, pd);
}

static void _set_volumn(sound_type_e type, int volume_index, char *vconf_key)
{
	DBG("Setting - set_volume() is called!");

	int err = -1;
	err = sound_manager_set_volume(type, volume_index);

	/* save system vulume value */
	if (err == SOUND_MANAGER_ERROR_NONE) {
		int vret = vconf_set_int(vconf_key, volume_index);
		DBG(" ret = %d , %s :: %d ", vret, vconf_key, volume_index);

		if (curr_sound_type == SOUND_TYPE_RINGTONE) {
			DBG("Ringtone volume is changed....!");

			if (volume_index == 0 && get_sound_mode() == SOUND_MODE_SOUND) {
				vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, FALSE);
				vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, FALSE);
			} else if (volume_index > 0 && get_sound_mode() != SOUND_MODE_SOUND) {
				vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, TRUE);
				vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, FALSE);
			}
		} else {
			DBG("current sound mode is %d, not type_ringtone", curr_sound_type);
		}
	} else {
		DBG("Setting - ringtone value is not saved...	%d", volume_index);
	}
}

static void _play_sound_all_type(int sound_type, float volume)
{
	if (is_myself_ringtone_changing) {
		DBG(" is_myself_ringtone_changing !!!!!!!!!!!");

		return;
	}

	char buf[1024];
	const char *sound_path = NULL;
	int temp_volume_index = 0;
	switch (sound_type) {
	case SOUND_TYPE_RINGTONE:
		temp_volume_index = volume_index;

		vconf_set_int(VCONFKEY_SETAPPL_CALL_RINGTONE_SOUND_VOLUME_INT, temp_volume_index);
		vconf_set_int("db/setting/sound/call/rmd_ringtone_volume", temp_volume_index);	/* backup ringtone volume */

		sound_path = vconf_get_str(VCONFKEY_SETAPPL_CALL_RINGTONE_PATH_STR);
		if (sound_path) {
			snprintf(buf, sizeof(buf)-1, "%s", sound_path);
		} else {
			snprintf(buf, sizeof(buf)-1, "%s", VCONFKEY_SETAPPL_CALL_RINGTONE_DEFAULT_PATH_STR);
		}

		break;
	case SOUND_TYPE_MEDIA:
		_set_volumn(sound_type, volume_index, VCONFKEY_SETAPPL_MEDIA_SOUND_VOLUME_INT);

		snprintf(buf, sizeof(buf)-1, "%s", SETTING_DEFAULT_MEDIA_TONE);
		break;
	case SOUND_TYPE_NOTIFICATION:
		vconf_set_int(VCONFKEY_SETAPPL_NOTI_SOUND_VOLUME_INT, volume_index);
		temp_volume_index = volume_index;

		sound_path = vconf_get_str(VCONFKEY_SETAPPL_NOTI_MSG_RINGTONE_PATH_STR);
		if (sound_path) {
			snprintf(buf, sizeof(buf)-1, "%s", sound_path);
		} else {
			snprintf(buf, sizeof(buf)-1, "%s", SETTING_DEFAULT_MSG_TONE);
		}
		break;
	case SOUND_TYPE_SYSTEM:
		vconf_set_int(VCONFKEY_SETAPPL_TOUCH_FEEDBACK_SOUND_VOLUME_INT, volume_index);

		snprintf(buf, sizeof(buf)-1, "%s", SETTING_DEFAULT_SYS_TONE);
		break;
	}

	ERR(">>>>> Play Sound path : %s", buf);

	int err = -1;

	if (sound_type == SOUND_TYPE_RINGTONE) {
		DBG("Setting - ringtone safety volume!!");
		DBG("Setting - real volume : %d", temp_volume_index);

		sound_manager_set_volume(sound_type, temp_volume_index);
	} else if (sound_type == SOUND_TYPE_NOTIFICATION) {
		ERR("Setting - notification safety volume!!");
		sound_manager_set_volume(sound_type, temp_volume_index);
	} else {
		ERR("Setting - normal volume!! ----- volume_index : %d ", volume_index);
		err = sound_manager_set_volume(sound_type, volume_index);
		if (err != SOUND_MANAGER_ERROR_NONE) {
			ERR("Setting - sound_manager_set_volume() is failed! : %d", err);
		}
	}

	if (sound_type == SOUND_TYPE_RINGTONE) {
		if (!is_created_player() || is_player_paused()) {
			play_sound(buf, volume, SOUND_TYPE_RINGTONE);
			set_looping(TRUE);
		}
		is_myself_ringtone_changing = 1;
	} else if (sound_type == SOUND_TYPE_SYSTEM || sound_type == SOUND_TYPE_MEDIA)	{

		play_sound(buf, volume, sound_type);
		set_looping(FALSE);
		return;
	} else if (sound_type == SOUND_TYPE_NOTIFICATION) {
		play_sound(buf, volume, SOUND_TYPE_NOTIFICATION);
		set_looping(FALSE);

		return;
	}
}


