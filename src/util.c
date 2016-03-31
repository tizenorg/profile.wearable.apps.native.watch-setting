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
 *  Created on: Nov 21, 2013
 *      Author: min-hoyun
 */

#include <string.h>
#include <vconf.h>
#include <fcntl.h>
#include "util.h"
#include <unicode/unum.h>
#include <unicode/ustring.h>
#include <unicode/udat.h>
#include <unicode/udatpg.h>



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
	snprintf(buffer + len, "%s%s", repl, ch + strlen(orig), 123 - len);

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

char* _get_strnum_from_icu(int number)
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
	if (!file_path) {
		DBG("Setting - file path is wrong!!");
		return false;
	}

	if ((open(file_path, O_RDONLY)) == -1) {
		DBG("Setting - file(%s) do not exist!!", file_path);
		return false;
	}

	DBG("Setting - file exist!!");

	return true;
}
