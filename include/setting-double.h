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
 * setting-double.h
 *
 *	Created on: Jan 8, 2014
 *		Author: Sunyeop Hwang
 */

#ifndef SETTING_DOUBLE_H_
#define SETTING_DOUBLE_H_

#include <Elementary.h>

struct _double_menu_item {
	int index;
	char *appid;
	char *pkgid;
	char *name;
};

typedef struct Double_Item_Data {
	struct _double_menu_item *pitem;
	Elm_Object_Item *item;
	Evas_Object *check;
} Double_Item_Data;

void init_double_pressing(void *data);
Evas_Object *create_double_app_list(void *data);
Evas_Object *create_double_list(void *data);
void clear_double_app_cb(void *data , Evas *e, Evas_Object *obj, void *event_info);
void clear_double_cb(void *data , Evas *e, Evas_Object *obj, void *event_info);

#endif /* SETTING_DOUBLE_H_ */
