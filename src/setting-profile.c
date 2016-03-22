/*
 * Copyright (c) 2010 Samsung Electronics, Inc.
 * All rights reserved.
 *
 * This software is a confidential and proprietary information
 * of Samsung Electronics, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics.
 *
 * setting-profile.c (s-health)
 *
 */
#include "setting-profile.h"
#include "util.h"

/*This function will be deprecated..*/
int app_control_set_package(app_control_h app_control, const char *package);

/* profile(s-health) */
void _create_profile(void *data, Evas_Object *obj, void *event_info)
{
	appdata *ad = data;
	if (ad == NULL) {
		DBG("%s", "_create_profile - appdata is null");
		return;
	}

	app_control_h service;
	app_control_create(&service);
	app_control_set_package(service, PROFILE_APP_ID);
	app_control_set_operation(service, PROFILE_OP_ID);
	app_control_send_launch_request(service, NULL, NULL);
	app_control_destroy(service);
}

