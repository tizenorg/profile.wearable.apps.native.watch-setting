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
 * util.c
 *
 *	Created on: Nov 21, 2013
 *		Author: min-hoyun
 */

#include <string.h>
#include <vconf.h>
#include <fcntl.h>
#include "util.h"
#include <unicode/unum.h>
#include <unicode/ustring.h>
#include <unicode/udat.h>
#include <unicode/udatpg.h>

typedef struct {
	back_btn_cb_ptr cb;
	void *data;
	Evas_Object *obj;
	Evas_Object *genlist_obj;
	char *cur_genlist_name;
} back_button_cb_data;

static Eina_List *back_button_cb_stack = NULL;

void back_button_cb_push(back_btn_cb_ptr cb, void *data, Evas_Object *obj, Evas_Object *genlist_obj, char *genlist_name)
{
	back_button_cb_data *cb_data = malloc(sizeof(*cb_data));
	cb_data->cb = cb;
	cb_data->data = data;
	cb_data->obj = obj;
	cb_data->genlist_obj = genlist_obj;
	if (strlen(genlist_name))
		cb_data->cur_genlist_name = strdup(genlist_name);
	else
		cb_data->cur_genlist_name = NULL;
	back_button_cb_stack = eina_list_prepend(back_button_cb_stack, cb_data);
	DBG("####### back btn push!! %s", (cb_data->cur_genlist_name) ? cb_data->cur_genlist_name : "NO name");

}

void clear_back_button_list()
{
	Eina_List *l;
	Eina_List *n;
	back_button_cb_data *item = NULL;
	EINA_LIST_FOREACH_SAFE(back_button_cb_stack, l, n, item) {
		if (item) {
			back_button_cb_stack = eina_list_remove(back_button_cb_stack, item);
			if (item->cur_genlist_name)
				free(item->cur_genlist_name);
			free(item);
		}
	}

	back_button_cb_stack = NULL;
}

void back_button_cb_pop(void)
{
	back_button_cb_data *cb_data = NULL;
	if (back_button_cb_stack)
		cb_data = eina_list_data_get(back_button_cb_stack);
	else {
		DBG("####### back btn pop cb - NULL!!!!!! EMPTY");
		return;
	}

	if (!cb_data) {
		DBG("####### back btn pop cb - cb_data NULL!!!!!! EMPTY");
		return;
	}
	if (cb_data->genlist_obj) {
		DBG("####### back btn pop cb - genlist ptr is %p", cb_data->genlist_obj);
		if (cb_data->cur_genlist_name)
			DBG("####### back btn pop cb - genlist name is %s", cb_data->cur_genlist_name);
		eext_rotary_object_event_activated_set(cb_data->genlist_obj, EINA_TRUE);
	}
	if (cb_data->cur_genlist_name) {
		free(cb_data->cur_genlist_name);
		cb_data->cur_genlist_name = NULL;
	}
	back_button_cb_stack = eina_list_remove(back_button_cb_stack, cb_data);
	free(cb_data);
}

void back_button_cb_call(void)
{
	back_button_cb_data *cb_data = NULL;
	cb_data = eina_list_data_get(back_button_cb_stack);
	if (cb_data && cb_data->cb) {
		cb_data->cb(cb_data->data, cb_data->obj, NULL);
	} else {
		ERR("No callback data!");
	}
}

void back_key_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = (appdata *)data;
	if (ad && ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	back_button_cb_pop();
}

void back_key_generic_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = (appdata *)data;
	if (ad) {
		elm_naviframe_item_pop(ad->nf);
		back_button_cb_pop();
	} else {
		ERR("data ptr is NULL");
	}
}

char *setting_gettext(const char *s)
{
	/* fisrt find in app pg */

	if (s == NULL) {
		return "NULL";
	}

	char *p = dgettext(SETTING_PACKAGE, s);

	if (!strcmp(s, p)) {	/* not found */
		/* find in system pkg */
		p = dgettext(SYSTEM_PACKAGE, s);
	}
	return p;
}

char *replace(char *str, char *orig, char *repl)
{
	static char buffer[124];
	char *ch;
	int len;
	if (!(ch = strstr(str, orig))) {
		return str;
	}
	len = ch - str;
	if (len > 123)
		len = 123;
	strncpy(buffer, str, len);
	buffer[len] = 0;
	snprintf(buffer + len, 123 - len, "%s%s", repl, ch + strlen(orig));

	return buffer;
}

void setting_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = (appdata *)data;
	if (ad == NULL) {
		return;
	}

	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
}

int is_connected_GM()
{
	int enable = 0;
	vconf_get_bool(VCONFKEY_WMS_WMANAGER_CONNECTED, &enable);

	return enable;
}

bool colorstr_to_decimal(char *color, int *R, int *G, int *B)
{
	DBG("_colorstr_to_decimal");
	if (color == NULL)
		return false;

	char *ptr;
	long value;
	value = strtol(color, &ptr, 16);
	*R = (value >> 16) & 0xFF;
	*G = (value >> 8) & 0xFF;
	*B = value & 0xFF;
	return true;
}

char *_get_strnum_from_icu(int number)
{
	char *locale_tmp = vconf_get_str(VCONFKEY_REGIONFORMAT);
	char locale[32] = {0,};
	char *p = NULL;
	if (strlen(locale_tmp) < 32)
		strcpy(locale, locale_tmp);

	if (locale[0] != '\0') {
		p = strstr(locale, ".UTF-8");
		if (p) {
			*p = 0;
		}
	}

	char *ret_str = NULL;
	UErrorCode status = U_ZERO_ERROR;

	UNumberFormat *num_fmt;
	UChar result[20] = { 0, };
	char res[20] = { 0, };
	int32_t len = (int32_t)(sizeof(result) / sizeof((result)[0]));

	num_fmt = unum_open(UNUM_DEFAULT, NULL, -1, locale, NULL, &status);
	unum_format(num_fmt, number, result, len, NULL, &status);

	u_austrcpy(res, result);

	unum_close(num_fmt);

	ret_str = strdup(res);
	return ret_str;
}

bool is_file_exist(char *file_path)
{
	int fd = 0;
	if (!file_path) {
		DBG("Setting - file path is wrong!!");
		return false;
	}

	fd = open(file_path, O_RDONLY);
	if (fd == -1) {
		DBG("Setting - file(%s) do not exist!!", file_path);
		return false;
	}

	if (fd)
		close(fd);

	DBG("Setting - file exist!!");

	return true;
}

void connect_to_wheel_with_genlist(Evas_Object *genlist, appdata *ad)
{
	Evas_Object *circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);
	eext_circle_object_genlist_scroller_policy_set(circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(circle_genlist, EINA_TRUE);
}

