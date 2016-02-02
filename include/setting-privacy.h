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
 * setting-privacy.h
 *
 *  Created on: Jan 7, 2014
 *      Author: Sunyeop Hwang
 */

#ifndef SETTING_PRIVACY_H_
#define SETTING_PRIVACY_H_

#include <Elementary.h>

struct _privacy_menu_item {
	char *name;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

typedef struct Privacy_Item_Data {
	int index;
	Elm_Object_Item *item;
	Evas_Object *check;
} Privacy_Item_Data;

Evas_Object *create_privacy_list(void *data);
Eina_Bool clear_privacy_cb(void *data, Elm_Object_Item *it);

#endif /* SETTING_PRIVACY_H_ */
