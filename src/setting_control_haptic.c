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
#include "setting_control_haptic.h"
#include "util.h"

static struct _haptic_data h_data;
static Ecore_Timer *vibration_timer = NULL;

static haptic_device_h hnd_hpt;

int _haptic_open()
{
#if 0
	int haptic_return = 0;
	haptic_return = haptic_open(HAPTIC_DEVICE_0, &hnd_hpt);
	if (haptic_return < 0) {
		DBG("Setting - Failed haptic_open");
		return 0;
	}

	h_data.is_haptic_opened = 1;
#endif
	return 1;
}

int _is_haptic_open()
{
	return h_data.is_haptic_opened;
}

Eina_Bool __vibration_timeout(void *data)
{
	if (_is_haptic_open()) {
		_haptic_close();
	}

	vibration_timer = NULL;

	return ECORE_CALLBACK_CANCEL;
}

void _start_vibration(int level, int feedback_rate, char *res_path)
{
#if 0
	if (_is_haptic_open()) {
		_haptic_close();
	}

	if (_haptic_open()) {
		int duration = 0;
		int err = -1;

		err = haptic_get_file_duration(hnd_hpt, res_path, &duration);
		if (err != 0) {
			DBG("Setting - haptic_get_file_duration() failed");
			duration = 1000;
		}

		DBG("Setting - duration : %d", duration);

		err = haptic_vibrate_file_with_detail(hnd_hpt, res_path,
											  HAPTIC_ITERATION_ONCE, feedback_rate, HAPTIC_PRIORITY_HIGH, NULL);
		if (err != 0)
			DBG("Setting - haptic_vibrate_file_with_detail() failed");

		double real_duration = (double)(duration / 1000.0);

		DBG("Setting - duration2 : %f", real_duration);

		if (vibration_timer) {
			ecore_timer_del(vibration_timer);
			vibration_timer = NULL;
		}
		vibration_timer = ecore_timer_add(real_duration, (Ecore_Task_Cb)__vibration_timeout, NULL);
	}
#endif
}

int _haptic_close()
{
#if 0
	if (h_data.is_haptic_opened) {
		int ret = haptic_close(hnd_hpt);
		if (ret != 0) {
			DBG("Setting - Failed haptic_deinitialize");
			return 0;
		}
	}

	if (vibration_timer) {
		ecore_timer_del(vibration_timer);
		vibration_timer = NULL;
	}

	h_data.is_haptic_opened = 0;
#endif

	return 1;
}
