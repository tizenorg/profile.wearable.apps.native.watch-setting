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
 * setting-battery.h
 *
 *  Created on: Oct 12, 2013
 *      Author: min-hoyun
 */

#ifndef SETTING_BATTERY_H_
#define SETTING_BATTERY_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>
#include <device.h>

struct _battery_menu_item {
	char *name;
	int is_enable;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

enum {
    CHARGING_NONE,
    CHARGING_AC,
    CHARGING_USB
};

enum {
    BATT_MENU_STATUS,
    /*BATT_MENU_PERCENT, */
    BATT_MENU_POWER_SAVING
};

struct _battery_info {
	int percent;
	int is_charging;
};

Ecore_Timer *battery_timer;



/* ----------------method----------------------// */

Evas_Object *_create_battery_list(void *data);

void _battery_status_cb_gen_item(void *data, Evas_Object *obj, void *event_info);
Evas_Object* _battery_status_cb(void *data);
void _clear_battery_cb(void *data , Evas *e, Evas_Object *obj, void *event_info);
Eina_Bool _clear_battery_list_cb(void *data, Elm_Object_Item *it);
void _start_timer_for_update(Evas_Object *obj);

void _initialize_battery();

#endif /* SETTING_BATTERY_H_ */
