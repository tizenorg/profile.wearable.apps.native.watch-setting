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
#include "util.h"
#include "setting_data_vconf.h"


static void _gl_multimedia_cb(void *data, Evas_Object *obj, void *event_info);
static void _gl_ringtone_cb(void *data, Evas_Object *obj, void *event_info);
static void _gl_notification_cb(void *data, Evas_Object *obj, void *event_info);
static void _gl_system_cb(void *data, Evas_Object *obj, void *event_info);

static struct _volume_menu_item volume_menu_its[] = {
	{ "IDS_ST_BUTTON_MULTIMEDIA",			_gl_multimedia_cb	},
	{ "IDS_ST_HEADER_RINGTONES_ABB",		_gl_ringtone_cb		},
	{ "IDS_ST_BUTTON_NOTIFICATIONS",		_gl_notification_cb },
	{ "IDS_ST_BODY_SYSTEM_M_VOLUME_ABB",	_gl_system_cb		},
	{ NULL, NULL }
};

static void _set_volumn(sound_type_e type, int volume_index, char *vconf_key);
/*static void _change_to_vibrate_mode(); */
static void vibrate_vconf_changed_cb(keynode_t *key, void *data);
static void sound_vconf_changed_cb(keynode_t *key, void *data);
static void _play_sound_all_type(int sound_type, float volume);
static void _update_volume_circle(Evas_Object *spiner);


static appdata *g_ad;
static Evas_Object *g_volume_spinner = NULL;
static Evas_Object *g_volume_genlist = NULL;

static int is_changing_level_by_vconf = 0;
static int is_changed = 0;
static int is_myself_changing = 0;
static int is_myself_ringtone_changing = 0;
static int is_play_ringtone_sound = 0;

static int is_sound_changed = 0;
static int is_vibrate_changed = 0;
static int is_play_media_sound = 0;

typedef void
(*system_part_volume_cb)(void *data, Evas_Object *obj, void *event_info);

static void
multimedia_value_changed(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - multimedia_value_changed() is called!");
	char buf[PATH_MAX];
	Evas_Object *label = data;

	snprintf(buf, sizeof(buf), "%.0lf", eext_circle_object_value_get(obj));
	DBG(">>>>>>>>>>>>>>>>>>>>>>>>>>> Slider value = %s", buf);
	elm_object_text_set(label, buf);

	if (curr_sound_type != SOUND_TYPE_MEDIA) {
		if (is_changing_level_by_vconf) {
			DBG("Setting - is_changing_level_by_vconf!!!!");

			is_changing_level_by_vconf = 0;
			return;
		}
	}

	/*Evas_Coord w;*/
	/*double min, max; */
	int idx = (int) eext_circle_object_value_get(obj);

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


	/*double posx = 0.0; */
	/*posx = (double)(w / max) * idx; */
}

static void
ringtone_value_changed(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - ringtone_value_changed() is called!");
	char buf[PATH_MAX];
	Evas_Object *label = data;

	snprintf(buf, sizeof(buf), "%.0lf", eext_circle_object_value_get(obj));
	DBG(">>>>>>>>>>>>>>>>>>>>>>>>>>> Slider value = %s", buf);
	if (get_sound_mode() == SOUND_MODE_SOUND) {
		elm_object_text_set(label, buf);

		if (curr_sound_type != SOUND_TYPE_MEDIA) {
			if (is_changing_level_by_vconf) {
				DBG("Setting - is_changing_level_by_vconf!!!!");

				is_changing_level_by_vconf = 0;
				return;
			}
		}

		/*Evas_Coord w; */
		/*double min, max; */
		int idx = (int) eext_circle_object_value_get(obj);

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

		/*edje_object_part_geometry_get(elm_layout_edje_get(obj), "center.image2", NULL, NULL, &w, NULL); */
		/*elm_spinner_min_max_get(obj, &min, &max); */
		/*DBG("Setting - min: %i, max: %i, idx: %d", (int)min, (int)max, idx); */
		/*double posx = 0.0; */
		/* posx = (double)(w / max) * idx; */
		/*edje_object_part_drag_value_set(elm_layout_edje_get(obj), "elm.dragable.slider", posx, 0); */
	}
}

static void volume_circle_system_part(appdata *ad, Evas_Object *ly, system_part_volume_cb changed_callback, int is_multimedia)
{
	Evas_Object *label = elm_label_add(ly);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);

	char tempbuf[128];
	snprintf(tempbuf, sizeof(tempbuf) - 1, "%d", volume_index);
	elm_object_text_set(label, tempbuf);
	elm_object_part_content_set(ly, "elm.icon.2", label);
	if (!is_multimedia) {
		evas_object_resize(label, 200, 200);
		if (get_sound_mode() != SOUND_MODE_SOUND)
			evas_object_color_set(label, 83, 94, 102, 255);
	}
	evas_object_show(label);

	Evas_Object *slider = eext_circle_object_slider_add(ly, ad->circle_surface);

	eext_circle_object_value_min_max_set(slider, 0.0, 15.0);
	eext_circle_object_value_set(slider, volume_index);

	eext_rotary_object_event_activated_set(slider, EINA_TRUE);
	eext_circle_object_slider_step_set(slider, 1);
	evas_object_smart_callback_add(slider, "value,changed", changed_callback, label);
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

static void _update_volume_popup_for_changing_sound_mode()
{
	DBG("_update_volume_popup_for_changing_sound_mode is called!!");

	is_sound_changed = is_vibrate_changed = 0;

	if (g_volume_spinner) {
		is_changing_level_by_vconf = 1;

		int sound_mode = get_sound_mode();

		if (sound_mode == SOUND_MODE_VIBRATE) {
			DBG("Setting - vibrate!!!");

			if (curr_sound_type == SOUND_TYPE_RINGTONE) {
				volume_index = 0;
			}
			elm_spinner_value_set(g_volume_spinner, 0);
			_update_volume_circle(g_volume_spinner);

			_stop_all_volume_sound();

			edje_object_signal_emit(elm_layout_edje_get(g_volume_spinner), "elm,spinner,min", "elm");
			edje_object_part_drag_value_set(elm_layout_edje_get(g_volume_spinner), "elm.dragable.slider", 0, 0);
			edje_object_signal_emit(elm_layout_edje_get(g_volume_spinner), "elm,spinner,vibrate", "elm");

			if (curr_sound_type == SOUND_TYPE_NOTIFICATION ||
				curr_sound_type == SOUND_TYPE_SYSTEM) {
				DBG("current_sound_type!!");

				elm_object_disabled_set(g_volume_spinner, EINA_TRUE);
			}
		} else if (sound_mode == SOUND_MODE_SOUND) {
			DBG("Setting - Sound!!!");

			sound_manager_get_volume(curr_sound_type, &volume_index);

			DBG("volume_index !!!!!!  --------------   %d", volume_index);

			if (volume_index != 0) {
				elm_spinner_value_set(g_volume_spinner, volume_index);

				_update_volume_circle(g_volume_spinner);

				elm_object_disabled_set(g_volume_spinner, EINA_FALSE);
			}
		} else {
			DBG("Setting - Mute!!!");

			if (curr_sound_type == SOUND_TYPE_NOTIFICATION ||
				curr_sound_type == SOUND_TYPE_SYSTEM) {
				elm_object_disabled_set(g_volume_spinner, EINA_FALSE);

				edje_object_signal_emit(elm_layout_edje_get(g_volume_spinner), "elm,spinner,min", "elm");
				edje_object_part_drag_value_set(elm_layout_edje_get(g_volume_spinner), "elm.dragable.slider", 0, 0);

				elm_object_disabled_set(g_volume_spinner, EINA_TRUE);
			} else if (curr_sound_type == SOUND_TYPE_RINGTONE) {
				_stop_all_volume_sound();

				if (curr_sound_type == SOUND_TYPE_RINGTONE) {
					volume_index = 0;
				}
				elm_spinner_value_set(g_volume_spinner, 0);
				_update_volume_circle(g_volume_spinner);

				edje_object_signal_emit(elm_layout_edje_get(g_volume_spinner), "elm,spinner,min", "elm");
				edje_object_part_drag_value_set(elm_layout_edje_get(g_volume_spinner), "elm.dragable.slider", 0, 0);
			}
		}
	}
}

static void sound_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - sound_vconf_changed_cb() is called!!");

	if (curr_sound_type == SOUND_TYPE_MEDIA) {
		return;
	}

	if (is_myself_changing) {
		DBG("Setting - is_myself_changing is called!!");
		is_myself_changing = 0;
		return;
	}

	is_sound_changed = 1;

	if (is_sound_changed && is_vibrate_changed) {
		_update_volume_popup_for_changing_sound_mode();
	}
}

static void vibrate_vconf_changed_cb(keynode_t *key, void *data)
{
	DBG("Setting - vibrate_vconf_changed_cb() is called!!");

	if (curr_sound_type == SOUND_TYPE_MEDIA) {
		return;
	}

	if (is_myself_changing) {
		DBG("Setting - is_myself_changing is called!!");
		is_myself_changing = 0;
		return;
	}

	is_vibrate_changed = 1;

	if (is_sound_changed && is_vibrate_changed) {
		_update_volume_popup_for_changing_sound_mode();
	}
}

static void pm_state_vconf_changed_cb_for_volume(keynode_t *key, void *data)
{
	DBG("Setting - pm_state_vconf_changed_cb_for_volume() is called!");

	int pm_state = 0;
	vconf_get_int(VCONFKEY_PM_STATE, &pm_state);

	if (pm_state == VCONFKEY_PM_STATE_LCDOFF) {
		DBG("Setting - LCD Off!! stop sound!");
		_stop_all_volume_sound();
	}
}

static void _rigngtone_volume_changed_cb(sound_type_e type, unsigned int volume, void *user_data)
{
	DBG("Setting - _rigngtone_volume_changed_cb() is called!");

	if (type != SOUND_TYPE_RINGTONE) {
		DBG("Setting - sound_type is not media!!");
		return;
	}

	if (is_myself_ringtone_changing) {
		is_myself_ringtone_changing = 0;
		return;
	}

	if (g_volume_spinner) {
		volume_index = volume;

		DBG("Setting - Ringtone volume: %d", volume_index);

		is_play_ringtone_sound = 1;

		elm_spinner_value_set(g_volume_spinner, volume_index);
	}
}

static void _media_volume_changed_cb(sound_type_e type, unsigned int volume, void *user_data)
{
	DBG("Setting - _media_volume_changed_cb() is called!");

	if (type == SOUND_TYPE_RINGTONE) {
		DBG("Setting - sound_type is ringtone!!");
		return;
	}

	if (is_myself_ringtone_changing) {
		is_myself_ringtone_changing = 0;
		return;
	}

	if (volume_index == volume) {
		DBG("Setting - Volume is same!! %d ---- %d ", volume_index, volume);
		return;
	}

	if (g_volume_spinner) {
		volume_index = volume;

		DBG("Setting - Media volume: %d", volume_index);

		is_play_media_sound = 1;

		elm_spinner_value_set(g_volume_spinner, volume_index);
	}
}

static void _gl_multimedia_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_multimedia_popup(data, obj, event_info);
}

static void _gl_ringtone_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_ringtone_popup(data, obj, event_info);
}

static void _gl_notification_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_notification_popup(data, obj, event_info);
}

static void _gl_system_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_show_system_popup(data, obj, event_info);
}

char *_gl_volume_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;
	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(volume_menu_its[index % VOLUMN_ITEM_COUNT].name));
		index++;
	}
	return strdup(buf);
}

static void _volumn_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}


#define NUM_ITEMS             5
#define NUM_INDEX             5
#define NUM_ITEMS_CIRCLE_EVEN 6
#define NUM_INDEX_CIRCLE_EVEN 6

typedef struct _page_data page_data;
struct _page_data
{
	Evas_Object *main_layout;
	Evas_Object *scroller;
	Evas_Object *box;
	Evas_Object *mapbuf[NUM_ITEMS_CIRCLE_EVEN];
	Evas_Object *index;
	Evas_Object *page_layout[NUM_ITEMS_CIRCLE_EVEN];
	int cur_page;
	int prev_page;
	Elm_Object_Item *it[NUM_ITEMS_CIRCLE_EVEN];

	Elm_Object_Item *last_it;
	Elm_Object_Item *new_it;
	int min_page, max_page;
};



static void _index_sync(void *data);

static void
_on_index_mouse_down_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
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
	page_data *pd = (page_data *)data;

	/* Keep the last index item active and move to the page of the currently selected index item */
	if (!pd->last_it) return;
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);

	if (!pd->new_it) return;

	int idx = (int) elm_object_item_data_get(pd->new_it);
	if (idx == pd->cur_page) return;

	elm_scroller_page_bring_in(pd->scroller, idx, 0);
}

static void
_on_index_mouse_move_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
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
	if (pd->cur_page < pd->min_page)
	{
		for(i = pd->cur_page ; i < pd->cur_page + num_index ; i++)
		{
			printf("Added : %d\n", i);
			elm_index_item_append(pd->index, NULL, NULL, (void *) i);
		}
		pd->min_page = pd->cur_page;
		pd->min_page = pd->cur_page + num_index - 1;
	}
	else
	{
		for(i = pd->cur_page - num_index + 1; i < pd->cur_page + 1 ; i++)
		{
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
	}
	else
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

	//Since mapbuf is not perfect, Enable them after the size calculation is finished
	for (i = 0; i < num_items; ++i)
	{
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
	}
}

static void _on_spinner_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _on_spinner_change_cb() is called!");

	Evas_Coord w;

	static int  prev = 0;
	double min, max;
	int idx = (int) elm_spinner_value_get(obj);

	elm_spinner_min_max_get(obj, &min, &max);

	DBG("Setting - min: %i, max: %i, idx: %d", (int)min, (int)max, idx);

//	if (idx == max)
//		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,full", "elm");
//	else if (idx < max)
//		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,default", "elm");
//	if (idx == min)
//		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,min", "elm");
//	if (idx > min)
//		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,normal", "elm");


//	int brightness_level = _change_bright_index_to_level(brightness_index);
//	device_set_brightness_to_settings(0, brightness_level);
//	vconf_set_int("db/setting/Brightness", brightness_level);
//	double posx2 = (double)(w / max) * brightness_index;
//
//	prev = idx;
//	edje_object_part_drag_value_set(elm_layout_edje_get(obj), "elm.dragable.slider", posx2, 0);
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

	} else {
		if (idx == min)
			edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,min", "elm");
		if (idx == max)
			edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,full1", "elm");
		if (idx < max)
			edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,default1", "elm");

		//brightness_index = idx;

//		posx2 = (double)(w / max) * brightness_index;

		edje_object_part_drag_value_set(elm_layout_edje_get(spiner), "elm.dragable.slider", posx2, 0);

	}
}
void _create_volume_page(void *data)
{

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_volume_page - appdata is null");
		return;
	}

	Evas_Object *genlist  = NULL;
	Elm_Object_Item *item = NULL;
	struct _volume_menu_item *menu_its = NULL;
	int idx = 0;

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

	for (i = 0; i < max_items; ++i)
	{
		page_layout = elm_layout_add(box);
		pd->page_layout[i] = page_layout;
		evas_object_size_hint_weight_set(page_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(page_layout, 0, EVAS_HINT_FILL);
		elm_layout_theme_set(page_layout, "layout", "body_thumbnail", "default");
		evas_object_show(page_layout);

		img = elm_image_add(page_layout);
		snprintf(img_path, sizeof(img_path), "%s/%s", IMG_DIR, img_names[i]);
		elm_image_file_set(img, img_path, NULL);
		elm_object_part_content_set(page_layout, "elm.icon", img);

		/* Make unselect state all of the pages except first one */
		if (i)
			elm_object_signal_emit(page_layout, "elm,state,thumbnail,unselect", "elm");

//		elm_box_pack_end(box, page_layout);

		Evas_Object *table, *rect;
		int w,h;

		table = elm_table_add(box);


		int volume_val = 0;

//		Evas_Object *spinner = elm_spinner_add(box);
//		elm_spinner_min_max_set(spinner, 1, 10);
//
//		elm_spinner_editable_set(spinner, EINA_TRUE);
//		elm_spinner_min_max_set(spinner, 1, 10);
//		elm_spinner_value_set(spinner, volume_val );
//		elm_object_style_set(spinner, "horizontal");
//		evas_object_smart_callback_add(spinner, "changed", _on_spinner_change_cb, pd);
//		elm_object_part_content_set(box, "elm.icon.1", spinner);
//		elm_spinner_value_set(spinner, volume_val );
//		_update_brightness_circle(spinner);
//
//		elm_table_pack(table, spinner, 0, 0, 1, 1);
//
		elm_win_screen_size_get((const Elm_Win *)elm_widget_top_get(ad->nf), NULL, NULL, &w, &h);
		rect = evas_object_rectangle_add(evas_object_evas_get(table));
		evas_object_size_hint_min_set(rect, w, h);
		evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_table_pack(table, rect, 0, 0, 1, 1);

		evas_object_size_hint_weight_set(page_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(page_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_table_pack(table, page_layout, 0, 0, 1, 1);

		evas_object_size_hint_weight_set(table, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(table);
		elm_box_pack_end(box, table);
	}

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

	for (i = 0; i < max_items; ++i)
	{
		pd->it[i] = elm_index_item_append(index, NULL, NULL, (void *) i);
	}

	pd->min_page = 0;
	pd->max_page = max_items - 1;
	elm_index_level_go(index, 0);
	elm_index_item_selected_set(pd->it[0], EINA_TRUE);

	pd->cur_page = 0;
	pd->index = index;
	pd->last_it = pd->it[0];

	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_DOWN, _on_index_mouse_down_cb, pd);
	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_MOVE, _on_index_mouse_move_cb, pd);
	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_UP, _on_index_mouse_up_cb, pd);


	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}

Evas_Object *_create_volume_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_volume_list - appdata is null");
		return NULL;
	}
	Evas_Object *genlist  = NULL;
	Elm_Object_Item *item = NULL;
	struct _volume_menu_item *menu_its = NULL;
	int idx = 0;

	g_ad = ad;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text";
	itc->func.text_get = _gl_volume_title_get;
	itc->func.del = _volumn_gl_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist, ad);

	menu_its = volume_menu_its;

	for (idx = 0; idx < VOLUMN_ITEM_COUNT; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = idx;
			item = elm_genlist_item_append(
					   genlist,			/* genlist object */
					   itc,				/* item class */
					   id,					/* data */
					   NULL,
					   ELM_GENLIST_ITEM_NONE,
					   menu_its[idx].func,	/* call back */
					   ad);
			id->item = item;
		}
	}
	elm_genlist_item_class_free(itc);

	g_volume_genlist = genlist;

	return genlist;
}

static void change_sound_mode(int mode)
{
	switch (mode) {
	case SOUND_MODE_SOUND:
		DBG("Setting - Change sound mode to Sound!");

		vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, TRUE);
		vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, FALSE);
		break;
	case SOUND_MODE_VIBRATE:
		DBG("Setting - Change sound mode to Sound!");

		vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, FALSE);
		vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, TRUE);
		break;
	case SOUND_MODE_MUTE:
		DBG("Setting - Change sound mode to Sound!");

		vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, FALSE);
		vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, FALSE);
		break;
	}
}

static void _set_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL)
		return;

	if (!is_changed) {
		DBG("not changed");
		elm_naviframe_item_pop(ad->nf);
		return;
	}

	DBG("Setting - original volume : %d", original_volume);

	char vconf_key[512] = {0,};

	switch (curr_sound_type) {
	case SOUND_TYPE_MEDIA:
		strncpy(vconf_key, VCONFKEY_SETAPPL_MEDIA_SOUND_VOLUME_INT, 512);
		break;
	case SOUND_TYPE_RINGTONE:
		strncpy(vconf_key, VCONFKEY_SETAPPL_CALL_RINGTONE_SOUND_VOLUME_INT, 512);
		break;
	case SOUND_TYPE_SYSTEM:
		strncpy(vconf_key, VCONFKEY_SETAPPL_TOUCH_FEEDBACK_SOUND_VOLUME_INT, 512);
		break;
	case SOUND_TYPE_NOTIFICATION:
		strncpy(vconf_key, VCONFKEY_SETAPPL_NOTI_SOUND_VOLUME_INT, 512);
		break;
	}

	/* restore original vulume value */
	_set_volumn(curr_sound_type, original_volume, vconf_key);

	if (original_sound_mode != get_sound_mode()) {
		/* restore sound mode */
		change_sound_mode(original_sound_mode);
	}

	/*original_volume = 0; */

	if (is_created_player()) {
		_close_player(ad, curr_sound_type);
	}

	stop_wav();

	elm_naviframe_item_pop(ad->nf);
}

static void _back_volume_naviframe_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _back_volume_naviframe_cb is called");
	DBG("Setting - original volume : %d", original_volume);
	DBG("Setting -	  volume index : %d", volume_index);

	if (original_sound_mode != get_sound_mode()) {
		/* restore sound mode */
		change_sound_mode(original_sound_mode);
	}

	if (is_created_player()) {
		_close_player(data, curr_sound_type);
	}

	g_volume_spinner = NULL;

	is_changed = 0;		/* initialize flag! */

	stop_wav();

	unregister_vconf_changing(VCONFKEY_PM_STATE, pm_state_vconf_changed_cb_for_volume);

	sound_manager_unset_volume_changed_cb();

	/* Unregister sound mode vconf callback */
	unregister_vconf_changing(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, vibrate_vconf_changed_cb);
	unregister_vconf_changing(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL , sound_vconf_changed_cb);

	return;
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

static void _set_multimedia_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL)
		return;

	if (!is_changed) {
		DBG("not changed");
		elm_naviframe_item_pop(ad->nf);
		return;
	}

	/* save multimedia vulume value */
	_set_volumn(SOUND_TYPE_MEDIA, volume_index, VCONFKEY_SETAPPL_MEDIA_SOUND_VOLUME_INT);


	if (is_created_player()) {
		_close_player(ad, curr_sound_type);
	}

	stop_wav();

	elm_naviframe_item_pop(ad->nf);
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
		DBG("Setting - notification safety volume!!");
		sound_manager_set_volume(sound_type, temp_volume_index);
	} else {
		DBG("Setting - normal volume!! ----- volume_index : %d ", volume_index);
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

		play_sound(buf, volume, SOUND_TYPE_SYSTEM);
		set_looping(FALSE);
		return;
	} else if (sound_type == SOUND_TYPE_NOTIFICATION) {
		play_sound(buf, volume, SOUND_TYPE_NOTIFICATION);
		set_looping(FALSE);

		return;
	}
}

#if 0 /* NOT USED */
static void _change_to_vibrate_mode()
{
	DBG("Setting - _change_to_vibrate_mode() is called!");

	if (curr_sound_type == SOUND_TYPE_MEDIA) {
		return;
	}

	if (get_sound_mode() != SOUND_MODE_VIBRATE) {
		DBG("Setting - Change sound mode to vibrate!");

		is_myself_changing = 1;

		_stop_all_volume_sound();

		_start_vibration(3, SETTING_VIB_STRONG_RATE, SETTING_DEFAULT_SYSTEM_HAPTIC_PREVIEW_VIB);

		vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, FALSE);
		vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, TRUE);
	}
}

static void _change_to_sound_mode()
{
	DBG("Setting - _change_to_sound_mode() is called!");

	if (curr_sound_type == SOUND_TYPE_MEDIA) {
		return;
	}

	if (get_sound_mode() != SOUND_MODE_SOUND) {
		DBG("Setting - Change sound mode to sound!");

		is_myself_changing = 1;

		vconf_set_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, TRUE);
		vconf_set_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, FALSE);
	}
}
#endif

static int sync_volume(int vconf_vol, int real_vol)
{
	if (vconf_vol != real_vol)
		return real_vol;
	return vconf_vol;
}

static void _update_volume_circle(Evas_Object *spiner)
{
	if (spiner == NULL)
		return;

	Evas_Coord w;
	double min, max, posx;
	int idx = (int) elm_spinner_value_get(spiner);

	edje_object_part_geometry_get(elm_layout_edje_get(spiner), "center.image2", NULL, NULL, &w, NULL);
	elm_spinner_min_max_get(spiner, &min, &max);

	if (idx == max)
		edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,full", "elm");
	if (idx < max)
		edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,default", "elm");
	if (idx == min) {
		edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,min", "elm");
		if (curr_sound_type != SOUND_TYPE_MEDIA && get_sound_mode() == SOUND_MODE_VIBRATE) {
			edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,vibrate", "elm");
		}
	}
	if (idx > min)
		edje_object_signal_emit(elm_layout_edje_get(spiner), "elm,spinner,normal", "elm");

	posx = (double)(w / max) * idx;

	edje_object_part_drag_value_set(elm_layout_edje_get(spiner), "elm.dragable.slider", posx, 0);
}


#if 0 /* NOT USED */
static void _on_volume_spinner_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _on_volume_spinner_change_cb() is called!");

	if (is_changing_level_by_vconf) {
		DBG("Setting - is_changing_level_by_vconf!!!!");

		is_changing_level_by_vconf = 0;
		return;
	}

	is_changed = 1;	/* changed flag!! */

	Evas_Coord w;

	double min, max;
	int idx = (int) elm_spinner_value_get(obj);

	volume_index = idx;

	DBG("Setting - volume_index : %d", volume_index);

	if (get_sound_mode() != SOUND_MODE_MUTE) {
		if (!is_play_ringtone_sound) {
			_play_sound_all_type(curr_sound_type, 0.0);
		} else {
			is_play_ringtone_sound = 0;
		}
	}

	edje_object_part_geometry_get(elm_layout_edje_get(obj), "center.image2", NULL, NULL, &w, NULL);
	elm_spinner_min_max_get(obj, &min, &max);

	DBG("Setting - min: %i, max: %i, idx: %d", (int)min, (int)max, idx);

	if (idx == max) {
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,full", "elm");
	}
	if (idx < max) {
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,default", "elm");
	}
	if (idx == min) {
		if (get_sound_mode() != SOUND_MODE_VIBRATE) {
			_change_to_vibrate_mode();
		}
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,min", "elm");
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,vibrate", "elm");
	}
	if (idx > min) {
		if (get_sound_mode() == SOUND_MODE_MUTE) {
			_change_to_vibrate_mode();
			edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,min", "elm");
			edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,vibrate", "elm");

			elm_spinner_value_set(obj, 0);
		} else {
			_change_to_sound_mode();
			edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,normal", "elm");
		}
	}

	double posx = 0.0;
	if (get_sound_mode() == SOUND_MODE_MUTE) {
		posx = 0;
	} else {
		posx = (double)(w / max) * idx;
	}

	edje_object_part_drag_value_set(elm_layout_edje_get(obj), "elm.dragable.slider", posx, 0);
}
#endif

#if 0 /* NOT USED */
static void _on_media_volume_spinner_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _on_media_volume_spinner_change_cb() is called!");

	if (curr_sound_type != SOUND_TYPE_MEDIA) {
		if (is_changing_level_by_vconf) {
			DBG("Setting - is_changing_level_by_vconf!!!!");

			is_changing_level_by_vconf = 0;
			return;
		}
	}

	Evas_Coord w;

	double min, max;
	int idx = (int) elm_spinner_value_get(obj);

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

	edje_object_part_geometry_get(elm_layout_edje_get(obj), "center.image2", NULL, NULL, &w, NULL);
	elm_spinner_min_max_get(obj, &min, &max);

	DBG("Setting - min: %i, max: %i, idx: %d", (int)min, (int)max, idx);

	if (idx == max) {
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,full", "elm");
	}
	if (idx < max) {
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,default", "elm");
	}
	if (idx == min) {
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,min", "elm");
	}
	if (idx > min) {
		edje_object_signal_emit(elm_layout_edje_get(obj), "elm,spinner,normal", "elm");
	}

	double posx = 0.0;
	posx = (double)(w / max) * idx;

	edje_object_part_drag_value_set(elm_layout_edje_get(obj), "elm.dragable.slider", posx, 0);
}
#endif

void _show_multimedia_popup(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *ly;
	appdata *ad = data;

	if (ad == NULL)
		return;

	is_play_media_sound = 0;

	original_sound_mode = get_sound_mode();

	curr_sound_type = SOUND_TYPE_MEDIA;

	vconf_get_int(VCONFKEY_SETAPPL_MEDIA_SOUND_VOLUME_INT, &volume_index);

	int real_volume_index = 0;
	sound_manager_get_volume(SOUND_TYPE_MEDIA, &real_volume_index);

	DBG("Setting - vconf vol: %d, real vol: %d", volume_index, real_volume_index);

	/* sync volume */
	volume_index = sync_volume(volume_index, real_volume_index);
	original_volume = volume_index;

	ad->MENU_TYPE = SETTING_VOLUME_2_DEPTH;


	ly = elm_layout_add(ad->nf);
	elm_layout_file_set(ly, EDJE_PATH, "setting/2finger_popup/default2");
	/*elm_layout_file_set(ly, EDJE_PATH, "setting/2finger_popup/default3");*/
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);

	volume_circle_system_part(ad, ly, multimedia_value_changed, 1 /*is_multimedia == 1*/);
#if 0
	Evas_Object *spinner = elm_spinner_add(ly);

	g_volume_spinner = spinner;

	elm_spinner_editable_set(spinner, EINA_FALSE);
	elm_spinner_min_max_set(spinner, 0, 15);
	elm_spinner_value_set(spinner, volume_index);
	_update_volume_circle(spinner);
	evas_object_smart_callback_add(spinner, "changed", _on_media_volume_spinner_change_cb, ly);
	elm_object_part_content_set(ly, "elm.icon.1", spinner);

#endif
	Evas_Object *btn_cancel;
	btn_cancel = elm_button_add(ly);
	elm_object_style_set(btn_cancel, "default");
	evas_object_size_hint_weight_set(btn_cancel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn_cancel, "IDS_ST_BUTTON_CANCEL_ABB2");
	elm_object_part_content_set(ly, "btn1", btn_cancel);
	evas_object_smart_callback_add(btn_cancel, "clicked", _set_cancel_cb, ad);

	Evas_Object *btn_ok;
	btn_ok = elm_button_add(ly);
	elm_object_style_set(btn_ok, "default");
	evas_object_size_hint_weight_set(btn_ok, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn_ok, "IDS_WNOTI_BUTTON_OK_ABB2");
	elm_object_part_content_set(ly, "btn2", btn_ok);
	evas_object_smart_callback_add(btn_ok, "clicked", _set_multimedia_clicked_cb, ad);

	Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->nf,
													 "IDS_ST_BUTTON_MULTIMEDIA",
													 NULL, NULL,
													 ly, NULL);
	back_button_cb_push(&_set_cancel_cb, data, btn_cancel, g_volume_genlist, nf_it);
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
	evas_object_event_callback_add(nf_it, EVAS_CALLBACK_DEL, _back_volume_naviframe_cb, ad);

	register_vconf_changing(VCONFKEY_PM_STATE, pm_state_vconf_changed_cb_for_volume, NULL);
	/*register_vconf_changing(VCONFKEY_SETAPPL_MEDIA_SOUND_VOLUME_INT ,_media_vconf_changing_cb, NULL); */

	sound_manager_set_volume_changed_cb(_media_volume_changed_cb, NULL);

	/* Unregister sound mode vconf callback */
	register_vconf_changing(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, vibrate_vconf_changed_cb, NULL);
	register_vconf_changing(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL , sound_vconf_changed_cb, NULL);
}

static void _set_ringtone_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL)
		return;

	if (!is_changed) {
		DBG("not changed");
		elm_naviframe_item_pop(ad->nf);
		return;
	}

	/* save ringtone vulume value */
	_set_volumn(SOUND_TYPE_RINGTONE, volume_index, VCONFKEY_SETAPPL_CALL_RINGTONE_SOUND_VOLUME_INT);

	if (is_created_player()) {
		_close_player(ad, curr_sound_type);
	}

	stop_wav();

	elm_naviframe_item_pop(ad->nf);
}

void _show_ringtone_popup(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *ly;
	Evas_Object *btn;
	appdata *ad = data;
	int sound_mode;

	if (ad == NULL)
		return;

	original_sound_mode = get_sound_mode();

	curr_sound_type = SOUND_TYPE_RINGTONE;

	vconf_get_int(VCONFKEY_SETAPPL_CALL_RINGTONE_SOUND_VOLUME_INT, &volume_index);

	DBG("Setting - Ringtone volume is %d", volume_index);

	sound_mode = get_sound_mode();

	int real_volume_index = 0;
	sound_manager_get_volume(SOUND_TYPE_RINGTONE, &real_volume_index);

	int virtual_index = real_volume_index;

	DBG("Setting - vconf vol: %d, real vol: %d", volume_index, virtual_index);

	if (sound_mode != SOUND_MODE_SOUND) {
		DBG("sound_mode != SOUND_MODE_SOUND ---> set to zero -> mode:%d ", sound_mode);
		virtual_index = 0;
	}

	/* sync volume */
	volume_index = sync_volume(volume_index, virtual_index);
	original_volume = real_volume_index;

	ad->MENU_TYPE = SETTING_VOLUME_2_DEPTH;

	ly = elm_layout_add(ad->nf);
	elm_layout_file_set(ly, EDJE_PATH, "setting/2finger_popup/default2");
	/*elm_layout_file_set(ly, EDJE_PATH, "setting/2finger_popup/default3");*/
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);

	if (ad->root_w == 360 && ad->root_h == 480) {
		DBG("make long height !!!!!!!!!!!!!!!!!");
		elm_object_signal_emit(ly, "set,popup,long", "*");
	}

	volume_circle_system_part(ad, ly, ringtone_value_changed, 0 /*is_multimedia == 0*/);
#if 0
	Evas_Object *spinner = elm_spinner_add(ly);

	g_volume_spinner = spinner;

	DBG("Setting- Volume: %d", volume_index);

	elm_spinner_editable_set(spinner, EINA_FALSE);
	elm_spinner_min_max_set(spinner, 0, 15);

	if (sound_mode != SOUND_MODE_SOUND) {
		elm_spinner_value_set(spinner, volume_index);
		edje_object_signal_emit(elm_layout_edje_get(spinner), "elm,spinner,min", "elm");
		edje_object_part_drag_value_set(elm_layout_edje_get(spinner), "elm.dragable.slider", 0, 0);

		if (get_sound_mode() == SOUND_MODE_VIBRATE) {
			edje_object_signal_emit(elm_layout_edje_get(spinner), "elm,spinner,vibrate", "elm");
		}

		elm_object_disabled_set(spinner, EINA_TRUE);
	} else {
		elm_spinner_value_set(spinner, volume_index);
		_update_volume_circle(spinner);
	}
	evas_object_smart_callback_add(spinner, "changed", _on_volume_spinner_change_cb, ly);
	elm_object_part_content_set(ly, "elm.icon.1", spinner);
#endif

	btn = elm_button_add(ly);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn, "IDS_ST_BUTTON_CANCEL_ABB2");
	elm_object_part_content_set(ly, "btn1", btn);
	evas_object_smart_callback_add(btn, "clicked", _set_cancel_cb, ad);

	btn = elm_button_add(ly);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn, "IDS_WNOTI_BUTTON_OK_ABB2");
	elm_object_part_content_set(ly, "btn2", btn);
	evas_object_smart_callback_add(btn, "clicked", _set_ringtone_clicked_cb, ad);

	Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->nf,
													 "IDS_ST_HEADER_RINGTONES_ABB",
													 NULL, NULL,
													 ly, NULL);
	back_button_cb_push(&_set_cancel_cb, data, btn, g_volume_genlist, nf_it);
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
	evas_object_event_callback_add(nf_it, EVAS_CALLBACK_DEL, _back_volume_naviframe_cb, ad);

	register_vconf_changing(VCONFKEY_PM_STATE, pm_state_vconf_changed_cb_for_volume, NULL);

	sound_manager_set_volume_changed_cb(_rigngtone_volume_changed_cb, NULL);

	/* Unregister sound mode vconf callback */
	register_vconf_changing(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, vibrate_vconf_changed_cb, NULL);
	register_vconf_changing(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL , sound_vconf_changed_cb, NULL);
}

static void _set_notification_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL)
		return;

	if (!is_changed) {
		DBG("not changed");
		elm_naviframe_item_pop(ad->nf);
		return;
	}

	_set_volumn(SOUND_TYPE_NOTIFICATION, volume_index, VCONFKEY_SETAPPL_NOTI_SOUND_VOLUME_INT);

	if (is_created_player()) {
		_close_player(ad, curr_sound_type);
	}

	stop_wav();

	elm_naviframe_item_pop(ad->nf);
}

void _show_notification_popup(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *ly;
	Evas_Object *btn;
	appdata *ad = data;
	int sound_mode;

	if (ad == NULL)
		return;

	original_sound_mode = get_sound_mode();

	curr_sound_type = SOUND_TYPE_NOTIFICATION;

	vconf_get_int(VCONFKEY_SETAPPL_NOTI_SOUND_VOLUME_INT, &volume_index);

	DBG("Setting - Notification volume is %d", volume_index);
	sound_mode = get_sound_mode();

	int real_volume_index = 0;
	sound_manager_get_volume(SOUND_TYPE_NOTIFICATION, &real_volume_index);

	int virtual_index = real_volume_index;

	DBG("Setting - vconf vol: %d, real vol: %d", volume_index, real_volume_index);
	DBG("current sound mode is : %d <--------", sound_mode);

	if (sound_mode != SOUND_MODE_SOUND)
		virtual_index = 0;
	else {
		DBG("Sound Mode is - %d ", sound_mode);
	}

	/* sync volume */
	volume_index = sync_volume(volume_index, virtual_index);
	original_volume = real_volume_index;

	ad->MENU_TYPE = SETTING_VOLUME_2_DEPTH;

	ly = elm_layout_add(ad->nf);
	elm_layout_file_set(ly, EDJE_PATH, "setting/2finger_popup/default2");
	/*elm_layout_file_set(ly, EDJE_PATH, "setting/2finger_popup/default3");*/
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);

	if (ad->root_w == 360 && ad->root_h == 480) {
		DBG("make long height !!!!!!!!!!!!!!!!!");
		elm_object_signal_emit(ly, "set,popup,long", "*");
	}

	volume_circle_system_part(ad, ly, ringtone_value_changed, 0 /*is_multimedia == 0*/);
#if 0
	Evas_Object *spinner = elm_spinner_add(ly);

	g_volume_spinner = spinner;

	DBG("Setting- Volume: %d", volume_index);

	/*elm_object_style_set(spinner, "volumestyle"); */
	elm_spinner_editable_set(spinner, EINA_FALSE);
	elm_spinner_min_max_set(spinner, 0, 15);

	if (get_sound_mode() != SOUND_MODE_SOUND) {
		elm_spinner_value_set(spinner, volume_index);
		edje_object_signal_emit(elm_layout_edje_get(spinner), "elm,spinner,min", "elm");
		edje_object_part_drag_value_set(elm_layout_edje_get(spinner), "elm.dragable.slider", 0, 0);

		if (get_sound_mode() == SOUND_MODE_VIBRATE) {
			edje_object_signal_emit(elm_layout_edje_get(spinner), "elm,spinner,vibrate", "elm");
		}

		elm_object_disabled_set(spinner, EINA_TRUE);
	} else {
		elm_spinner_value_set(spinner, volume_index);
		_update_volume_circle(spinner);
	}
	evas_object_smart_callback_add(spinner, "changed", _on_media_volume_spinner_change_cb, ly);
	elm_object_part_content_set(ly, "elm.icon.1", spinner);
#endif

	btn = elm_button_add(ly);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn, "IDS_ST_BUTTON_CANCEL_ABB2");
	elm_object_part_content_set(ly, "btn1", btn);
	evas_object_smart_callback_add(btn, "clicked", _set_cancel_cb, ad);

	btn = elm_button_add(ly);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn, "IDS_WNOTI_BUTTON_OK_ABB2");
	elm_object_part_content_set(ly, "btn2", btn);
	evas_object_smart_callback_add(btn, "clicked", _set_notification_clicked_cb, ad);

	Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->nf,
													 "IDS_ST_BUTTON_NOTIFICATIONS",
													 NULL, NULL,
													 ly, NULL);
	back_button_cb_push(&back_key_generic_cb, data, NULL, g_volume_genlist, nf_it);
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
	evas_object_event_callback_add(nf_it, EVAS_CALLBACK_DEL, _back_volume_naviframe_cb, ad);
	register_vconf_changing(VCONFKEY_PM_STATE, pm_state_vconf_changed_cb_for_volume, NULL);

	sound_manager_set_volume_changed_cb(_media_volume_changed_cb, NULL);

	/* Unregister sound mode vconf callback */
	register_vconf_changing(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, vibrate_vconf_changed_cb, NULL);
	register_vconf_changing(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL , sound_vconf_changed_cb, NULL);
}

static void _set_system_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL)
		return;

	if (!is_changed) {
		DBG("not changed");
		elm_naviframe_item_pop(ad->nf);
		return;
	}

	/* save system vulume value */
	_set_volumn(SOUND_TYPE_SYSTEM, volume_index, VCONFKEY_SETAPPL_TOUCH_FEEDBACK_SOUND_VOLUME_INT);

	if (is_created_player()) {
		_close_player(ad, curr_sound_type);
	}

	stop_wav();

	elm_naviframe_item_pop(ad->nf);
}


void _show_system_popup(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *ly;
	Evas_Object *btn;
	appdata *ad = data;
	int sound_mode;

	if (ad == NULL)
		return;

	original_sound_mode = get_sound_mode();

	curr_sound_type = SOUND_TYPE_SYSTEM;

	vconf_get_int(VCONFKEY_SETAPPL_TOUCH_FEEDBACK_SOUND_VOLUME_INT, &volume_index);

	DBG("Setting - Notification volume is %d", volume_index);
	sound_mode = get_sound_mode();

	int real_volume_index = 0;
	sound_manager_get_volume(SOUND_TYPE_SYSTEM, &real_volume_index);

	int virtual_index = real_volume_index;

	DBG("Setting - vconf vol: %d, real vol: %d", volume_index, real_volume_index);
	if (sound_mode != SOUND_MODE_SOUND)
		virtual_index = 0;
	else {
		DBG("Sound Mode is - %d ", sound_mode);
	}

	/* sync volume */
	volume_index = sync_volume(volume_index, virtual_index);
	original_volume = real_volume_index;

	ad->MENU_TYPE = SETTING_VOLUME_2_DEPTH;

	ly = elm_layout_add(ad->nf);
	elm_layout_file_set(ly, EDJE_PATH, "setting/2finger_popup/default2");
	/*elm_layout_file_set(ly, EDJE_PATH, "setting/2finger_popup/default3");*/
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);

	if (ad->root_w == 360 && ad->root_h == 480) {
		DBG("make long height !!!!!!!!!!!!!!!!!");
		elm_object_signal_emit(ly, "set,popup,long", "*");
	}

	volume_circle_system_part(ad, ly, ringtone_value_changed, 0 /*is_multimedia == 0*/);

#if 0
	Evas_Object *spinner = elm_spinner_add(ly);

	g_volume_spinner = spinner;

	DBG("Setting- Volume: %d", volume_index);

	/*elm_object_style_set(spinner, "volumestyle"); */
	elm_spinner_editable_set(spinner, EINA_FALSE);
	elm_spinner_min_max_set(spinner, 0, 15);

	if (get_sound_mode() != SOUND_MODE_SOUND) {
		elm_spinner_value_set(spinner, volume_index);
		edje_object_signal_emit(elm_layout_edje_get(spinner), "elm,spinner,min", "elm");
		edje_object_part_drag_value_set(elm_layout_edje_get(spinner), "elm.dragable.slider", 0, 0);

		if (get_sound_mode() == SOUND_MODE_VIBRATE) {
			edje_object_signal_emit(elm_layout_edje_get(spinner), "elm,spinner,vibrate", "elm");
		}

		elm_object_disabled_set(spinner, EINA_TRUE);
	} else {
		elm_spinner_value_set(spinner, volume_index);
		_update_volume_circle(spinner);
	}
	evas_object_smart_callback_add(spinner, "changed", _on_media_volume_spinner_change_cb, ly);
	elm_object_part_content_set(ly, "elm.icon.1", spinner);
#endif

	btn = elm_button_add(ly);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn, "IDS_ST_BUTTON_CANCEL_ABB2");
	elm_object_part_content_set(ly, "btn1", btn);
	evas_object_smart_callback_add(btn, "clicked", _set_cancel_cb, ad);

	btn = elm_button_add(ly);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn, "IDS_WNOTI_BUTTON_OK_ABB2");
	elm_object_part_content_set(ly, "btn2", btn);
	evas_object_smart_callback_add(btn, "clicked", _set_system_clicked_cb, ad);

	Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->nf,
													 "IDS_ST_BODY_SYSTEM_M_VOLUME_ABB",
													 NULL, NULL,
													 ly, NULL);
	back_button_cb_push(&_set_cancel_cb, data, btn, g_volume_genlist, nf_it);
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
	evas_object_event_callback_add(nf_it, EVAS_CALLBACK_DEL, _back_volume_naviframe_cb, ad);

	register_vconf_changing(VCONFKEY_PM_STATE, pm_state_vconf_changed_cb_for_volume, NULL);

	sound_manager_set_volume_changed_cb(_media_volume_changed_cb, NULL);

	/* Unregister sound mode vconf callback */
	register_vconf_changing(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, vibrate_vconf_changed_cb, NULL);
	register_vconf_changing(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL , sound_vconf_changed_cb, NULL);
}
