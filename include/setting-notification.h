/*
 * setting-notification.h
 *
 *  Created on: Feb 14, 2014
 *      Author: min-hoyun
 */

#ifndef SETTING_NOTIFICATION_H_
#define SETTING_NOTIFICATION_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>

#include "setting_data_vconf.h"
#include "util.h"

#define NOTIFICATION_ITEM_COUNT			1

#define VCONF_WMS_NOTIFICATION_KEY	"db/wms/noti_onoff_support"

struct _noti_menu_item {
	char *name;
	int state;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

typedef struct Noti_Item_Data {
	int index;
	Elm_Object_Item *item;
} Noti_Item_Data;


typedef struct _noti_data {
	int is_enable_noti;

	appdata *temp_ad;

	Evas_Object *g_noti_genlist;

} Noti_Data;


void _initialize_noti();
Eina_Bool _clear_noti_cb(void *data, Elm_Object_Item *it);

char *_gl_noti_title_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_gl_noti_check_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_create_noti_list(void *data);

void noti_enabling_vconf_changed_cb(keynode_t *key, void *data);
void _noti_gl_enabling_noti_cb(void *data, Evas_Object *obj, void *event_info);

#endif /* SETTING_NOTIFICATION_H_ */
