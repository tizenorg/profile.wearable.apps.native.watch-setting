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
#include <bluetooth.h>
#include <vconf-keys.h>
#include <vconf.h>

#include "setting_data_vconf.h"
#include "setting_indicator_util.h"

void indicator_vconf_changed_cb(keynode_t *node, void *data)
{
	appdata *ad = data;

	if (!ad) {
		ERR("appdata is null!!");
		return;
	}

	Evas_Object *layout = NULL;

	layout = elm_object_item_part_content_get(elm_genlist_first_item_get(ad->main_genlist), "elm.icon");
	indicator_view_update(layout);
}

void _indicator_set_BT_icon(keynode_t *node, void *data)
{
	Evas_Object *layout = data;
	int isBTOn = 0, sap_connected = 0, headset_connected = 0;

	if (vconf_get_int(VCONFKEY_BT_STATUS, &isBTOn) != 0) {
		DBG("vconf_get_int() failed");
		isBTOn = 0;
	}

	elm_object_signal_emit(layout, "hideHeadset", "img.headset");

	if (isBTOn) {
		vconf_get_bool(VCONFKEY_WMS_WMANAGER_CONNECTED, &sap_connected);
		DBG("SAP conntected : %d", sap_connected);
		if (sap_connected == 0) {
			elm_object_signal_emit(layout, "showUnconnctedBTicon", "img.bluetooth");
		} else {
			elm_object_signal_emit(layout, "showConnectedBTicon", "img.bluetooth");
		}
		vconf_get_int(VCONFKEY_BT_DEVICE, &headset_connected);
		DBG("Heaadset connected : %x", headset_connected);
		if (headset_connected & VCONFKEY_BT_DEVICE_A2DP_HEADSET_CONNECTED) {
			elm_object_signal_emit(layout, "showHeadset", "img.headset");
		} else {
			elm_object_signal_emit(layout, "hideHeadset", "img.headset");
		}
	} else {
		elm_object_signal_emit(layout, "hideBTicon", "img.bluetooth");
		elm_object_signal_emit(layout, "hideHeadset", "img.headset");
	}
}

void _indicator_set_battery_icon(keynode_t *node, void *data)
{
	int battery_level = 100;
	int is_battery_display = -1;
	char buf[1024] = {0,};
	char buf2[5] = {0,};
	Evas_Object *icon = NULL;

	Evas_Object *layout = data;

	icon = elm_object_part_content_get(layout, "sw.battery");
	if (icon == NULL) {
		icon = elm_layout_add(layout);
		elm_layout_file_set(icon, EDJE_PATH, "setting/indicator/battery_icon");
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_part_content_set(layout, "sw.battery", icon);
	}

	vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CAPACITY, &battery_level);
	vconf_get_bool(VCONFKEY_SETAPPL_BATTERY_PERCENTAGE_BOOL, &is_battery_display);
	if (is_battery_display) {
		char *level = (char *)_get_strnum_from_icu(battery_level);
		if (level) {
			DBG("%s %d", level, strlen(level));
			if (strlen(level) > 6) {
				snprintf(buf, sizeof(buf) - 1, "%d%%", battery_level);
			} else {
				snprintf(buf, sizeof(buf) - 1, "%s%%", level);
			}
			elm_object_part_text_set(layout, "txt.battery", buf);
		}
		FREE(level);
	} else {
		elm_object_part_text_set(layout, "txt.battery", "");
	}

	int is_charged;
	vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, &is_charged);
	DBG("battery_level: %d, isCharging: %d", battery_level, is_charged);
	if (is_charged == 1) {
		snprintf(buf, sizeof(buf) - 1, "change_charging_level_");
	} else if (is_charged == 0) {
		snprintf(buf, sizeof(buf) - 1, "change_level_");
	} else if (is_charged == -1) {
		DBG("Battery charging has problem");
		elm_object_part_text_set(layout, "txt.battery", "");
		snprintf(buf, sizeof(buf) - 1, "change_level_");
	}
	battery_level /= 5;
	battery_level *= 5;
	snprintf(buf2, sizeof(buf2) - 1, "%02d", battery_level);
	strncat(buf, buf2, sizeof(buf) - 1);
	DBG("battery file : %s", buf);
	elm_object_signal_emit(icon, buf, "img.battery");
	elm_object_signal_emit(layout, "showBattery", "sw.battery");
}

void indicator_view_update(Evas_Object *layout)
{
	/* BT */
	_indicator_set_BT_icon(NULL, layout);

	/* battery */
	_indicator_set_battery_icon(NULL, layout);
}

void indicator_set_vconf_changed_cb(void *data)
{
	register_vconf_changing(VCONFKEY_WMS_WMANAGER_CONNECTED, indicator_vconf_changed_cb, data);
	register_vconf_changing(VCONFKEY_SYSMAN_BATTERY_CAPACITY, indicator_vconf_changed_cb, data);
	register_vconf_changing(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, indicator_vconf_changed_cb, data);
	register_vconf_changing(VCONFKEY_BT_DEVICE, indicator_vconf_changed_cb, data);
}

void indicator_unset_vconf_changed_cb()
{
	unregister_vconf_changing(VCONFKEY_WMS_WMANAGER_CONNECTED, indicator_vconf_changed_cb);
	unregister_vconf_changing(VCONFKEY_SYSMAN_BATTERY_CAPACITY, indicator_vconf_changed_cb);
	unregister_vconf_changing(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, indicator_vconf_changed_cb);
	unregister_vconf_changing(VCONFKEY_BT_DEVICE, indicator_vconf_changed_cb);
}
