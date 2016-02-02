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
 * setting-bluetooth.h
 *
 *  Created on: Oct 9, 2013
 *      Author: min-hoyun
 */

#ifndef SETTING_BLUETOOTH_H_
#define SETTING_BLUETOOTH_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>
#include <bluetooth-api.h>
#include <bluetooth.h>
#include <vconf.h>

#define VISIBILITY_TIMEOUT 		120
#define BT_LIST_ITEM_COUNT		3


enum {
    BT_NON_OPERATING,
    BT_OPERATING
};

enum {
    BT_MENU_TYPE_BT_ON_OFF,
    BT_MENU_TYPE_BT_HEADSET,
    BT_MENU_TYPE_VISIBLE_ON_OFF
};

struct _bt_menu_item {
	char *name;
	int is_enable;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

typedef struct Bt_Item_Data {
	int index;
	Elm_Object_Item *item;
	Evas_Object *check;
	Evas_Object *state_label;
} Bt_Item_Data;

static char *_gl_bt_title_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_gl_bt_check_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_create_bt_list(void *data);
static void _bt_chk_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _blutooth_cb(void *data, Evas_Object *obj, void *event_info);
static void _visibility_cb(void *data, Evas_Object *obj, void *event_info);
static void _alternate_bt_mode(void *data);
static void _deamon_cb(void *data, Evas_Object *obj, void *event_info);

static void _BT_headset_cb(void *data, Evas_Object *obj, void *event_info);

void initialize_bt();
void clear_bt_resource();
Eina_Bool _clear_bluetooth_cb(void *data, Elm_Object_Item *it);
void _update_visibility_item_view(int is_hf_connected);

static void bt_state_vconf_change_cb(keynode_t *key, void *data);
static void hf_event_handler(int event, void *data, void *user_data);

#endif /* SETTING_BLUETOOTH_H_ */
