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
 * setting-reset.c
 *
 *  Created on: Oct 10, 2013
 *      Author: min-hoyun
 */

#include "setting.h"
#include "setting-reset.h"
#include "util.h"


static void _remove_reset_popup(appdata *ad)
{
	if (ad == NULL)
		return;
	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
}

static void _response_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = (appdata *)data;

	_remove_reset_popup(ad);
}

static void _response_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;

	_remove_reset_popup(ad);

	/* call Factory-reset */
	deviced_call_predef_action(PREDEF_FACTORY_RESET, 0, NULL);
}

void _reset_lange_changed(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _reset_lange_changed() is called!");

	char buf[1024];
	snprintf(buf, sizeof(buf), "<font_size=34>%s</font_size>",
	         elm_entry_utf8_to_markup(_("IDS_ST_POP_ALL_DATA_WILL_BE_ERASED_FROM_GEAR_MEMORY_YOU_CAN_THEN_CONNECT_YOUR_GEAR_TO_ANOTHER_DEVICE")));
	elm_object_text_set(obj, strdup(buf));
}

void _reset_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = NULL;
	Evas_Object *btn = NULL;
	Evas_Object *scroller = NULL;
	Evas_Object *label = NULL;

	appdata *ad = (appdata *) data;
	if (ad == NULL)
		return;

	popup = elm_popup_add(ad->nf);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_part_text_set(popup, "title,text", "IDS_ST_BODY_RESET_GEAR_ABB");

	ad->popup = popup;

	scroller = elm_scroller_add(popup);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(popup, scroller);
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_style_set(scroller, "effect");
	evas_object_show(scroller);

	label = elm_label_add(scroller);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);

	char buf[1024];

	char *font_size_frame = "<font_size=34>%s</font_size>";
	snprintf(buf, sizeof(buf) - 1, font_size_frame, _("IDS_ST_POP_ALL_DATA_WILL_BE_ERASED_FROM_GEAR_MEMORY_YOU_CAN_THEN_CONNECT_YOUR_GEAR_TO_ANOTHER_DEVICE"));

	char *txt = strdup(buf);
	elm_object_text_set(label, txt);
	free(txt);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(label, "language,changed", _reset_lange_changed, NULL);
	elm_object_content_set(scroller, label);
	evas_object_show(label);


	//ea_object_event_callback_add(popup, EA_CALLBACK_BACK, setting_popup_back_cb, ad);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn, "IDS_COM_SK_CANCEL_A");
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", _response_cancel_cb, ad);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "default");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_text_set(btn, "IDS_WNOTI_BUTTON_OK_ABB2");
	elm_object_part_content_set(popup, "button2", btn);
	evas_object_smart_callback_add(btn, "clicked", _response_ok_cb, ad);

	evas_object_show(popup);
}
