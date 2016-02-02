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
#ifndef _SETTING_VIEW_TOAST_H_
#define _SETTING_VIEW_TOAST_H_

#include <Elementary.h>

typedef struct _toast_data {
	char *str;
	int is_show;

	Evas_Object *toast_popup;
} Toast_Data;

Toast_Data *_create_toast(void *data, char *msg);
void _destroy_toast(struct _toast_data *toast);
void _dismiss_toast(void *data, Evas_Object *obj, void *event_info);
/*void _dismiss_toast( struct _toast_data * toast ); */
void _show_toast(void *data, struct _toast_data *toast);

#endif
