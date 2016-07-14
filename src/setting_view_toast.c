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
#include <string.h>

#include "setting_view_toast.h"
#include "util.h"

Toast_Data *_create_toast(void *data, char *msg)
{
	Toast_Data *toast = (Toast_Data *) calloc(sizeof(Toast_Data), 1);
	if (toast) {
		toast->is_show = 0;
		toast->str = strdup(msg);
		toast->toast_popup = NULL;
	}

	return toast;
}

void _destroy_toast(Toast_Data *toast)
{
	if (toast) {
		FREE(toast->str);
		if (toast->toast_popup) {
			evas_object_del(toast->toast_popup);
			toast->toast_popup = NULL;
		}
		FREE(toast);
	}
}

void _dismiss_toast(void *data, Evas_Object *obj, void *event_info)
{
	Toast_Data *toast = (Toast_Data *)data;
	_destroy_toast(toast);
}

void _show_toast(void *data, Toast_Data *toast)
{
	appdata *ad = data;
	if (ad == NULL || toast == NULL) {
		return;
	}

	if (toast->is_show) {
		return;
	}

	toast->toast_popup = elm_popup_add(ad->nf);
	elm_object_style_set(toast->toast_popup, "toast/circle");
	elm_popup_orient_set(toast->toast_popup, ELM_POPUP_ORIENT_BOTTOM);

	evas_object_size_hint_weight_set(toast->toast_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	/*ea_object_event_callback_add(toast->toast_popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL); */
	elm_object_part_text_set(toast->toast_popup, "elm.text", toast->str);
	elm_popup_timeout_set(toast->toast_popup, 2.0);

	evas_object_smart_callback_add(toast->toast_popup, "block,clicked", _dismiss_toast, toast);
	evas_object_smart_callback_add(toast->toast_popup, "timeout", _dismiss_toast, toast);

	evas_object_show(toast->toast_popup);
}
