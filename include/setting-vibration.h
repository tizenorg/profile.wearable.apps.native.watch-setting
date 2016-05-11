/*
 *	Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 *	Licensed under the Flora License, Version 1.0 (the License);
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://floralicense.org/license/
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an AS IS BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */
/*
 * setting-vibration.h
 *
 *	Created on: Dec 8, 2015
 *		Author: JinWang An
 */

#ifndef SETTING_VIBRATION_H_
#define SETTING_VIBRATION_H_

#include <Elementary.h>
#include <libintl.h>
#include <string.h>

#include <player.h>
#include <sound_manager.h>


char *_gl_vibration_title_get(void *data, Evas_Object *obj, const char *part);
Evas_Object *_create_vibration_list(void *data);
void _long_buzz_cb(void *data, Evas_Object *obj, void *event_info);
void _intensity_cb(void *data, Evas_Object *obj, void *event_info);
void _show_intensity_list_cb(void *data, Evas_Object *obj, void *event_info);

void _clear_vibration_resource();

#endif /* SETTING_VIBRATION_H_ */
