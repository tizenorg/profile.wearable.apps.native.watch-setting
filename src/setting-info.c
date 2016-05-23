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
 * setting-info.c
 *
 *	Created on: Oct 8, 2013
 *		Author: min-hoyun
 */

#include <vconf.h>
#include <vconf-keys.h>

#include "setting-info.h"
#include "setting-battery.h"
#include "util.h"

#include <bluetooth-api.h>
#include <bluetooth.h>
#include <wifi.h>

static struct _info_menu_item info_menu_its[] = {
	{ "IDS_ST_BODY_ABOUT_GEAR_ABB",		0,		 _gl_info_cb },
	/*#ifndef FEATURE_SETTING_SDK */
	{ "IDS_ST_HEADER_USB_DEBUGGING_ABB",	0,	 _gl_usb_debug_cb },
	/*#endif */
};

static struct _info_menu_item _info_detail_menu_list[] = {
	/*#ifndef FEATURE_SETTING_SDK */
	{ "IDS_ST_BODY_MODEL_NUMBER_ABB",	ABOUT_DEVICE_MODEL_NUMBER,	NULL },
	/*#endif */
	{ "Wi-Fi MAC address",		ABOUT_DEVICE_WIFI_ADDRESS,	NULL },
	{ "Bluetooth MAC address",		ABOUT_DEVICE_BLUETOOTH_ADDRESS,	NULL },
	{ "Tizen version",		ABOUT_DEVICE_TIZEN_VERSION,	NULL },
	{ "IDS_SM_TAB4_SOFTWARE_VERSION",	ABOUT_DEVICE_SOFTWARE_VERSION,	NULL },
	/*#ifndef FEATURE_SETTING_SDK */
	{ "IDS_ST_BODY_SERIAL_NUMBER",		ABOUT_DEVICE_SERIAL_NUMBER,	NULL },
	{ "Storage",		ABOUT_DEVICE_STORAGE,	NULL },
	{ "Battery capacity",		ABOUT_DEVICE_BATTERY_CAPACITY,	NULL },
	{ "IDS_ST_HEADER_OPEN_SOURCE_LICENCES_ABB",	ABOUT_DEVICE_OPEN_SOURCE_LICENSES,	_info_open_src_gl_cb },
	/*	{ "Barcode Number",			4,	NULL }, */
	/*	{ "IDS_ST_OPT_SAFETY_INFORMATION",	4,	_info_safety_inform_gl_cb } */
	/*#endif */
};

static void _usb_debug_cancel_cb(void *data, Evas_Object *obj, void *event_info);
static void _usb_debug_ok_cb(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool _get_imei_serial_info(char *pszBuf, int bufSize);

static Evas_Object *g_info_genlist = NULL;
static int is_enable_usb_debug = 0;
static int kor = 0;
static appdata *temp_ad = NULL;

#define STORAGEUG_STR_KB "KB"
#define STORAGEUG_STR_MB "MB"
#define STORAGEUG_STR_GB "GB"

void storageUg_get_internal_storage_status(double *total, double *avail)
{
	int ret;
	double tmp_total;
	struct statvfs s;
	const unsigned int GB = (1024 * 1024) * 1024;
	const double sz_32G = 32. * GB;
	const double sz_16G = 16. * GB;
	const double sz_8G = 8. * GB;

	if (NULL == total || NULL == avail)
		return;

	ret = storage_get_internal_memory_size(&s);
	if (0 == ret) {
		DBG("Total = %lu, Available = %lu", (s.f_frsize * s.f_blocks),
			(s.f_bsize * s.f_bavail));
		tmp_total = (double)s.f_frsize * s.f_blocks;
#if 0
		*avail = (double)s.f_bfree * s.f_frsize;
#else
		*avail = (double)s.f_bsize * s.f_bavail;
#endif
		if (sz_16G < tmp_total)
			*total = sz_32G;
		else if (sz_8G < tmp_total)
			*total = sz_16G;
		else
			*total = sz_8G;
	}
}


void storageUg_size_to_str(double size, char *desc, int desc_size)
{
	double tmp_size = 0.0;
	const int KILOBYTE_VALUE = 1024;
	const int MEGABYTE_VALUE = KILOBYTE_VALUE * 1024;
	const int GIGABYTE_VALUE = MEGABYTE_VALUE * 1024;

	if (size < MEGABYTE_VALUE) {	/* size < 1MB: show x.xKB */
		tmp_size = size / KILOBYTE_VALUE;
		snprintf(desc, desc_size, "%4.2lf%s", tmp_size, _(STORAGEUG_STR_KB));
	} else if (size < GIGABYTE_VALUE) {	/* size < 1GB: show x.xMB */
		tmp_size = size / MEGABYTE_VALUE;
		snprintf(desc, desc_size, "%4.2lf%s", tmp_size, _(STORAGEUG_STR_MB));
	} else { /* 1G <= size: show x.xGB */
		tmp_size = size / GIGABYTE_VALUE;
		snprintf(desc, desc_size, "%4.2lf%s", tmp_size, _(STORAGEUG_STR_GB));
	}
}

void _get_storage_size(char *str, int size)
{
	char buf_total[100] = {0,};
	char buf_avail[100] = {0,};
	double total_size, avail_size;
	storageUg_get_internal_storage_status(&total_size, &avail_size);
	storageUg_size_to_str(total_size, buf_total, 99);
	storageUg_size_to_str(avail_size, buf_avail, 99);
	snprintf(str, size, "%s / %s", buf_avail, buf_total);
}

void _get_bluetooth_address_string(char *str, int size)
{
	int ret = 0;

	ret = bt_initialize();
	setting_retm_if(ret != 0, "bluetooth initialization failed");

	char *local_address = NULL;
	ret = bt_adapter_get_address(&local_address);

	bt_deinitialize();

	if (ret != BT_ERROR_NONE) {
		/* 혹은 if (ret == BT_ERROR_NOT_ENABLED) { */
		snprintf(str, size, "%s", _("IDS_ST_BODY_UNKNOWN"));
		DBG("bt address : %s failed get bt address with error code:%d", str, ret);
	} else {
		DBG("bt address : %s", local_address);
		snprintf(str, size, "%s", local_address);
	}

	if (local_address) {
		free(local_address);
	}
}

void _get_wifi_address_string(char *str, int size)
{
	char *mac_addr = NULL;
	int status = wifi_get_mac_address(&mac_addr);
	if (WIFI_ERROR_NONE != status) {
		ERR("Failed to call wifi_get_mac_address()");
		snprintf(str, size, "%s", _("IDS_ST_BODY_UNKNOWN"));
	} else {
		ERR("Success to call wifi_get_mac_address()");
		ERR("get_wifi_mac_address : %s", mac_addr);
		snprintf(str, size, "%s", mac_addr);
	}

	if (mac_addr) {
		free(mac_addr);
	}
}


Eina_Bool _clear_info_cb(void *data, Elm_Object_Item *it)
{
	g_info_genlist = NULL;
	is_enable_usb_debug = 0;

	return EINA_TRUE;
}

int get_enable_USB_debugging()
{
	if (vconf_get_bool(VCONFKEY_SETAPPL_USB_DEBUG_MODE_BOOL, &is_enable_usb_debug) != 0) {
		DBG("Setting - USB debugging's info do not get!!!");
	}
	return is_enable_usb_debug;
}

void set_enable_USB_debugging(int is_enable)
{
	if (vconf_set_bool(VCONFKEY_SETAPPL_USB_DEBUG_MODE_BOOL, is_enable) != 0) {
		DBG("Setting - USB debugging do not set!!!");
	}
}

void _gl_usb_debug_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

}

void _usb_debug_chk_changed_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	appdata *ad = temp_ad;
	Evas_Object *ly;
	Evas_Object *check = (Evas_Object *)data;

	if (ad == NULL) {
		DBG("%s", "_usb_debug_chk_changed_cb - appdata or check is null");
		return;
	}

	if (!get_enable_USB_debugging()) {

		ly = elm_layout_add(ad->nf);
		elm_layout_file_set(ly, EDJE_PATH, "setting/2finger_popup/default5");

		evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);



		elm_object_part_text_set(ly, "watch_on_text.text1", "Allow Gear to read log data, copy files to and from your PC, and install apps without notification. (development only).");

		Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->nf,
														 NULL,
														 NULL, NULL,
														 ly, NULL);


		/*evas_object_smart_callback_add(ly, "language,changed", _usb_debug_lange_changed, NULL); */


		Evas_Object *btn = NULL;
		btn = elm_button_add(ly);
		elm_object_style_set(btn, "default");
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_translatable_text_set(btn, "IDS_COM_SK_CANCEL_A");
		elm_object_part_content_set(ly, "btn1", btn);
		evas_object_smart_callback_add(btn, "clicked", _usb_debug_cancel_cb, check);

		btn = elm_button_add(ly);
		elm_object_style_set(btn, "default");
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_translatable_text_set(btn, "IDS_WNOTI_BUTTON_OK_ABB2");
		elm_object_part_content_set(ly, "btn2", btn);
		evas_object_smart_callback_add(btn, "clicked", _usb_debug_ok_cb, check);
		elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);

		elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
	} else {
		set_enable_USB_debugging(0);
	}
}

static void _info_gl_del(void *data, Evas_Object *obj)
{
	/* FIXME: Unrealized callback can be called after this. */
	/* Accessing Item_Data can be dangerous on unrealized callback. */
	Info_Item_Data *id = data;
	if (id)
		free(id);
}

char *_gl_info_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Info_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf) - 1, "%s", _(info_menu_its[index].name));
		index++;
	}
	return strdup(buf);
}

Evas_Object *_gl_info_check_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check = NULL;
	/*appdata *ad = data; */

	Info_Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.icon") && index == (ITEM_COUNT - 1)) {
		check = elm_check_add(obj);
		elm_object_style_set(check, "on&off");
		is_enable_usb_debug = get_enable_USB_debugging();
		elm_check_state_set(check, (is_enable_usb_debug) ? EINA_TRUE : EINA_FALSE);
		evas_object_event_callback_add(check, EVAS_CALLBACK_MOUSE_DOWN, _usb_debug_chk_changed_cb, (void *)check);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_TRUE);

		id->check = check;

		index++;
	}
	return check;
}

Evas_Object *_create_info_list(void *data)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_info_list - appdata is null");
		return NULL;
	}

	temp_ad = ad;

	Evas_Object *genlist  = NULL;
	struct _info_menu_item *menu_its = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.1";
	itc->func.text_get = _gl_info_title_get;
	/*#ifndef FEATURE_SETTING_SDK */
	itc->func.content_get = _gl_info_check_get;
	/*#endif */
	itc->func.del = _info_gl_del;

	Evas_Object *layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, EDJE_PATH, "setting/genlist/layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist, ad);

	menu_its = info_menu_its;

	for (idx = 0; idx < sizeof(info_menu_its) / sizeof(struct _info_menu_item); idx++) {
		Info_Item_Data *id = calloc(sizeof(Info_Item_Data), 1);
		if (id) {
			id->index = idx;
			id->item = elm_genlist_item_append(
						   genlist,				/* genlist object */
						   itc,					/* item class */
						   id,						/* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_its[idx].func,	/* call back */
						   ad);
		}
	}
	elm_genlist_item_class_free(itc);

	g_info_genlist = genlist;

	elm_object_part_content_set(layout, "elm.genlist", genlist);

	return layout;
}

static void _info_detail_gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id)
		free(id);
}

char *_gl_info__detail_title_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024] = {0,};
	Item_Data *id = data;
	int index = id->index;
	char *device_info = NULL;

	char szSerialNum[50];

	/*if( !strcmp(part, "elm.text.1") || !strcmp(part, "elm.text") ) */
	if (!strcmp(part, "elm.text")) {
		if (index == ABOUT_DEVICE_SERIAL_NUMBER) {
			const char *lang = vconf_get_str(VCONFKEY_LANGSET);
			if (lang && !strcmp(lang, "ko_KR.UTF-8")) {
				snprintf(buf, sizeof(buf) - 1, "일련번호");
			} else {
				snprintf(buf, sizeof(buf) - 1, "%s", _(_info_detail_menu_list[index].name));
			}
		} else {
			snprintf(buf, sizeof(buf) - 1, "%s", _(_info_detail_menu_list[index].name));
		}
	} else if (!strcmp(part, "elm.text.1")) {
		switch (index) {
		case ABOUT_DEVICE_MODEL_NUMBER:
			/*#ifndef FEATURE_SETTING_SDK */
			if (system_info_get_platform_string("http://tizen.org/system/model_name", &device_info)
				!= SYSTEM_INFO_ERROR_NONE) {
				DBG("Setting - Model number do not get!! error!!");
				snprintf(buf, sizeof(buf) - 1, "%s", "SM-V700");
			} else {
				snprintf(buf, sizeof(buf) - 1, "%s", device_info);
			}
			break;
		case ABOUT_DEVICE_SOFTWARE_VERSION:
			/*#endif */
			if (system_info_get_platform_string("http://tizen.org/system/build.string", &device_info)
				!= SYSTEM_INFO_ERROR_NONE) {
				DBG("Setting - Version name do not get!! error!!");
				snprintf(buf, sizeof(buf) - 1, "%s", "V700XXUAMJ1");
			} else {
				snprintf(buf, sizeof(buf) - 1, "%s", device_info);
			}
			break;
		case ABOUT_DEVICE_BLUETOOTH_ADDRESS:
			_get_bluetooth_address_string(buf, sizeof(buf)-1);
			break;
		case ABOUT_DEVICE_WIFI_ADDRESS:
			_get_wifi_address_string(buf, sizeof(buf)-1);
			break;
		case ABOUT_DEVICE_STORAGE:
			_get_storage_size(buf, sizeof(buf)-1);
			break;
		case ABOUT_DEVICE_TIZEN_VERSION:
			if (system_info_get_platform_string("http://tizen.org/feature/platform.version", &device_info)
				!= SYSTEM_INFO_ERROR_NONE) {
				DBG("Setting - Platform Version name do not get!! error!!");
				snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_UNKNOWN"));
			} else {
				snprintf(buf, sizeof(buf) - 1, "%s", device_info);
			}
			break;
		case ABOUT_DEVICE_BATTERY_CAPACITY:
			if (system_info_get_platform_string("http://tizen.org/feature/battery", &device_info)
				!= SYSTEM_INFO_ERROR_NONE) {
				DBG("Setting - Battery capacity do not get!! error!!");
				snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_UNKNOWN"));
			} else {
				snprintf(buf, sizeof(buf) - 1, "%s", device_info);
			}
			break;
		/*#ifndef FEATURE_SETTING_SDK */
		case ABOUT_DEVICE_SERIAL_NUMBER:
			memset(buf, '\0', sizeof(buf));
			if (_get_imei_serial_info(szSerialNum, sizeof(szSerialNum)) == EINA_TRUE) {
				if (kor) {
					char *p = NULL;
					p = strstr(szSerialNum, ",");
					if (p) {
						*p = '\0';
					}
				}
				if (strlen(szSerialNum)) {
					snprintf(buf, sizeof(buf) - 1, "%s", szSerialNum);
				} else {
					snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_UNKNOWN"));
				}
			} else {
				snprintf(buf, sizeof(buf) - 1, "%s", _("IDS_ST_BODY_UNKNOWN"));
			}

			break;
		/*#endif */
		default:
			break;
		}
	}
	ERR("index %d , buf : %s", index, buf);
	return strdup(buf);
}

static void open_sources_licences_lange_changed(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - open_sources_licences_lange_changed() is called!");

	char *license_str = elm_entry_utf8_to_markup(
							_("IDS_ST_POP_YOU_CAN_CHECK_ANNOUNCEMENTS_REGARDING_OPEN_SOURCE_LICENCES_BY_FOLLOWING_THE_STEPS_BELOW_N1_GO_TO_SETTINGS_GEAR_INFO_N2_SELECT_USB_MSG"));

	char buf[1024];

	char *font_size_frame = "<font_size=34>%s</font_size>";
	snprintf(buf, sizeof(buf) - 1, font_size_frame, license_str);

	char *frame = strdup(buf);
	const char *file_path = "/usr/share/license.html";
	const char *command = "sdb pull /usr/share/license.html c:\\OpenSource\\license.html";

	if (frame)
		snprintf(buf, sizeof(buf) - 1, frame, file_path, command);

	elm_object_text_set(obj, strdup(buf));
	free(frame);
	free(license_str);
}

/*void _open_source_licences_popup_cb(void *data, Evas_Object *obj, void *event_info) */
/*{ */
/*	Evas_Object *popup = NULL; */
/*	/*Evas_Object *btn = NULL; * / */
/*	Evas_Object *scroller = NULL; */
/*	Evas_Object *label = NULL; */
/* */
/*	appdata *ad = (appdata *) data; */
/*	if (ad == NULL) */
/*		return; */
/* */
/*	popup = elm_popup_add(ad->nf); */
/*	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0); */
/*	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); */
/*	elm_object_translatable_part_text_set(popup, "title,text", "IDS_ST_HEADER_OPEN_SOURCE_LICENCES_ABB"); */
/* */
/*	ad->popup = popup; */
/* */
/*	scroller = elm_scroller_add(popup); */
/*	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); */
/*	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE); */
/*	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO); */
/*	elm_object_content_set(popup, scroller); */
/*	elm_object_style_set(scroller, "effect"); */
/*	evas_object_show(scroller); */
/* */
/*	label = elm_label_add(scroller); */
/*	elm_object_style_set(label, "popup/default"); */
/*	elm_label_line_wrap_set(label, ELM_WRAP_MIXED); */
/* */
/*	/*	char * license_str = "You can check announcements regarding open source licences by following the steps below." * / */
/*	/*							"<br>1. Go to Settings > Gear info." * / */
/*	/*							"<br>2. Select USB debugging." * / */
/*	/*							"<br>3. Tap OK on the USB debugging pop-up that appears on your Samsung Gear." * / */
/*	/*							"<br>4. Connect your Samsung Gear to your PC." * / */
/*	/*							"<br>5. Using sdb cmd, download the %1$s file to your PC by running cmd and entering \"%2$s\" into the input field." * / */
/*	/*							"<br>Please visit<br>http://developer. samsung.com/, download and install Samsung Gear SDK for using sdb."; * / */
/* */
/*	char *license_str = elm_entry_utf8_to_markup( */
/*							_("IDS_ST_POP_YOU_CAN_CHECK_ANNOUNCEMENTS_REGARDING_OPEN_SOURCE_LICENCES_BY_FOLLOWING_THE_STEPS_BELOW_N1_GO_TO_SETTINGS_GEAR_INFO_N2_SELECT_USB_MSG")); */
/* */
/*	char buf[1536]; */
/* */
/*	char *font_size_frame = "<font_size=34>%s</font_size>"; */
/*	snprintf(buf, sizeof(buf) - 1, font_size_frame, license_str); */
/* */
/*	char *frame = strdup(buf); */
/*	const char *file_path = "/usr/share/license.html"; */
/*	const char *command = "sdb pull /usr/share/license.html c:\\OpenSource\\license.html"; */
/* */
/*	if (frame) */
/*		snprintf(buf, sizeof(buf) - 1, frame, file_path, command); */
/* */
/*	char *txt = strdup(buf); */
/* */
/*	elm_object_text_set(label, txt); */
/*	free(txt); */
/*	free(frame); */
/*	free(license_str); */
/* */
/*	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0); */
/*	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL); */
/*	evas_object_smart_callback_add(label, "language,changed", open_sources_licences_lange_changed, NULL); */
/*	elm_object_content_set(scroller, label); */
/*	evas_object_show(label); */
/* */
/*	/*ea_object_event_callback_add(popup, EA_CALLBACK_BACK, setting_popup_back_cb, ad); * / */
/* */
/*	evas_object_show(popup); */
/*} */

static char *get_license_str()
{
	/*	char * license_str = "You can check announcements regarding open source licences by following the steps below." */
	/*							"<br>1. Go to Settings > Gear info." */
	/*							"<br>2. Select USB debugging." */
	/*							"<br>3. Tap OK on the USB debugging pop-up that appears on your Samsung Gear." */
	/*							"<br>4. Connect your Samsung Gear to your PC." */
	/*							"<br>5. Using sdb cmd, download the %1$s file to your PC by running cmd and entering \"%2$s\" into the input field." */
	/*							"<br>Please visit<br>http://developer. samsung.com/, download and install Samsung Gear SDK for using sdb."; */

	static char buf[1536];
	char *license_str = elm_entry_utf8_to_markup(
							_("IDS_ST_POP_YOU_CAN_CHECK_ANNOUNCEMENTS_REGARDING_OPEN_SOURCE_LICENCES_BY_FOLLOWING_THE_STEPS_BELOW_N1_GO_TO_SETTINGS_GEAR_INFO_N2_SELECT_USB_MSG"));

	char *font_size_frame = "<font=tizen:style=Condensed><font_size=28>%s</font_size></font>";
	snprintf(buf, sizeof(buf) - 1, font_size_frame, license_str);

	char *frame = strdup(buf);
	const char *file_path = "/usr/share/license.html";
	const char *command = "sdb pull /usr/share/license.html c:\\OpenSource\\license.html";

	if (frame)
		snprintf(buf, sizeof(buf) - 1, frame, file_path, command);

	char *txt = strdup(buf);
	free(frame);
	free(license_str);
	return txt;
}

void _open_source_licenses_navi_cb(void *data, Evas_Object *obj, void *event_info)
{

	Evas_Object *scroller = NULL;
	Evas_Object *label = NULL;

	appdata *ad = (appdata *) data;
	if (ad == NULL)
		return;

	Evas_Object *ly;
	ly = elm_layout_add(ad->nf);
	elm_layout_file_set(ly, EDJE_PATH, "setting/open_licences_popup/default");

	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);


	scroller = elm_scroller_add(ly);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_part_content_set(ly, "scroller", scroller);
	elm_object_style_set(scroller, "effect");
	evas_object_show(scroller);

	label = elm_label_add(scroller);

	evas_object_smart_callback_add(label, "language,changed", open_sources_licences_lange_changed, NULL);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);


	char *txt = get_license_str();

	elm_object_style_set(label, "default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label,	txt);
	elm_object_content_set(scroller, label);
	evas_object_show(label);

	free(txt);

	Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->nf,
													 _("IDS_ST_HEADER_OPEN_SOURCE_LICENCES_ABB"),
													 NULL, NULL,
													 ly, NULL);

	elm_naviframe_item_title_enabled_set(nf_it, EINA_TRUE, EINA_FALSE);
	elm_object_item_domain_text_translatable_set(nf_it, SETTING_PACKAGE, EINA_TRUE);
}


void _info_open_src_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_open_source_licenses_navi_cb(data, obj, event_info);
}

void safety_inform_lange_changed(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - safety_inform_lange_changed() is called!");

	char *safety_str = elm_entry_utf8_to_markup(_("IDS_ST_BODY_YOU_CAN_CHECK_NOTICES_REGARDING_SAFETY_INFORMATION_BY_FOLLOWING_THE_STEPS_BELOW_N_N1_GO_TO_SETTINGS_MSG"));

	char buf[1024];
	snprintf(buf, sizeof(buf), "<font_size=34>%s</font_size>", safety_str);
	elm_object_text_set(obj, strdup(buf));
	free(safety_str);
}


void _safety_inform_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = NULL;
	/*Evas_Object *btn = NULL; */
	Evas_Object *scroller = NULL;
	Evas_Object *label = NULL;

	appdata *ad = (appdata *) data;
	if (ad == NULL)
		return;

	popup = elm_popup_add(ad->nf);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_translatable_part_text_set(popup, "title,text", "IDS_ST_OPT_SAFETY_INFORMATION");

	ad->popup = popup;

	scroller = elm_scroller_add(popup);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_content_set(popup, scroller);
	elm_object_style_set(scroller, "effect");
	evas_object_show(scroller);

	label = elm_label_add(scroller);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);

	char *safety_str = elm_entry_utf8_to_markup(_("IDS_ST_BODY_YOU_CAN_CHECK_NOTICES_REGARDING_SAFETY_INFORMATION_BY_FOLLOWING_THE_STEPS_BELOW_N_N1_GO_TO_SETTINGS_MSG"));

	char buf[2048];
	snprintf(buf, sizeof(buf) - 1, "<font_size=34>%s</font_size>", safety_str);

	char *txt = strdup(buf);
	elm_object_text_set(label, txt);
	free(txt);
	free(safety_str);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(scroller, label);
	evas_object_smart_callback_add(label, "language,changed", safety_inform_lange_changed, NULL);
	evas_object_show(label);

	/*ea_object_event_callback_add(popup, EA_CALLBACK_BACK, setting_popup_back_cb, ad); */

	evas_object_show(popup);
}

void _info_safety_inform_gl_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	_safety_inform_popup_cb(data, obj, event_info);
}

void _gl_info_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_gl_info_cb - appdata is null");
		return;
	}
	Elm_Object_Item *nf_it = NULL;
	Evas_Object *genlist  = NULL;
	struct _info_menu_item *menu_its = NULL;
	int idx = 0;

	Elm_Genlist_Item_Class *itc_tmp = NULL;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "2text";
	itc->func.text_get = _gl_info__detail_title_get;
	itc->func.del = _info_detail_gl_del;

	Elm_Genlist_Item_Class *itc_open_src_info = elm_genlist_item_class_new();
	itc_open_src_info->item_style = "1text";
	itc_open_src_info->func.text_get = _gl_info__detail_title_get;
	itc_open_src_info->func.del = _info_detail_gl_del;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	connect_to_wheel_with_genlist(genlist, ad);

	menu_its = _info_detail_menu_list;

	for (idx = 0; idx < sizeof(_info_detail_menu_list) / sizeof(struct _info_menu_item); idx++) {
		ERR("idx : %d name: %s", idx, _info_detail_menu_list[idx].name);
		/*#ifndef FEATURE_SETTING_SDK */
		if (menu_its[idx].type == ABOUT_DEVICE_OPEN_SOURCE_LICENSES) {
			itc_tmp = itc_open_src_info;
		} else {
			itc_tmp = itc;
		}
		/*#else */
		/*		itc_tmp = itc; */
		/*#endif */

		/*		if (!(kor == 0 && menu_its[idx].type == 3)) { */

		Item_Data *id = calloc(sizeof(Item_Data), 1);
		if (id) {
			id->index = menu_its[idx].type;
			id->item = elm_genlist_item_append(
						   genlist,			/* genlist object */
						   itc_tmp,			/* item class */
						   id,					/* data */
						   NULL,
						   ELM_GENLIST_ITEM_NONE,
						   menu_its[idx].func,	/* call back */
						   ad);
			/*#ifndef FEATURE_SETTING_SDK */
			/*				if (itc_tmp	 && (itc_tmp == itc)) { */
			/*					elm_genlist_item_select_mode_set(id->item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY); */
			/*				} */
			/*#else */
			/*				if (menu_its[idx].type != ABOUT_DEVICE_OPEN_SOURCE_LICENSES) { */
			/*					elm_genlist_item_select_mode_set(id->item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY); */
			/*				} */
			/*#endif */
		}
		/*		} */
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc_open_src_info);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, genlist, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}

static void _usb_debug_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = temp_ad;
	Evas_Object *check = (Evas_Object *)data;

	set_enable_USB_debugging(0);
	elm_check_state_set(check,	EINA_FALSE);

	elm_naviframe_item_pop(ad->nf);
}

static void _usb_debug_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = temp_ad;
	Evas_Object *check = (Evas_Object *)data;

	set_enable_USB_debugging(1);
	elm_check_state_set(check,	EINA_TRUE);

	elm_naviframe_item_pop(ad->nf);
}

void _usb_debug_lange_changed(void *data, Evas_Object *obj, void *event_info)
{
	DBG("Setting - _usb_debug_lange_changed() is called!");

	char *debug_str = elm_entry_utf8_to_markup(_("IDS_ST_POP_ENABLING_USB_DEBUGGING_WILL_ALLOW_YOU_TO_COPY_FILES_BETWEEN_YOUR_PC_AND_YOUR_GEAR_INSTALL_APPLICATIONS_ON_YOUR_GEAR_WITHOUT_RECEIVING_MSG"));

	char buf[1024];
	snprintf(buf, sizeof(buf), "<font_size=34>%s</font_size>", debug_str);
	elm_object_text_set(obj, strdup(buf));

	free(debug_str);
}

/*
 * Get IMEI Serial number
 */
static Eina_Bool _get_imei_serial_info(char *pszBuf, int bufSize)
{
	FILE		*pFile = NULL;
	Eina_Bool	bResult;

	pFile = fopen("/csa/imei/serialno.dat", "r");
	if (pFile == NULL)
		return EINA_FALSE;

	memset(pszBuf, '\0', bufSize);

	if (fgets(pszBuf, bufSize, pFile)) {
		bResult = EINA_TRUE;
	} else {
		bResult = EINA_FALSE;
	}

	fclose(pFile);
	pFile = NULL;

	return bResult;
}
