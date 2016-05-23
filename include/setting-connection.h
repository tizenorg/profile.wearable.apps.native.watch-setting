/*
 *	Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 *	Licensed under the Flora License, Version 1.0 (the License);
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://floralicense.org/license/
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an AS IS BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */
/*
 * etting-connection.h
 *
 *	Created on: Dec 8, 2015
 *		Author: JinWang An
 */

#ifndef SETTING_CONNECTION_H_
#define SETTING_CONNECTION_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>

enum {
	SETTING_CONNECTION_BLUETOOTH,
	SETTING_CONNECTION_WIFI,
	SETTING_CONNECTION_NFC,
//	SETTING_CONNECTION_BT_ALERTS,
	SETTING_CONNECTION_FLIGHT_MODE
};

struct _connection_menu_item {
	char *name;
	int type;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

typedef struct connection_Item_Data {
	int index;
	Elm_Object_Item *item;
	Evas_Object *check;
} connection_Item_Data;




void _clear_connection_resource();
Evas_Object *_create_connection_list(void *data);


#endif /* SETTING_CONNECTION_H_ */
