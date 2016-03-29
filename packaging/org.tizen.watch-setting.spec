Name: org.tizen.watch-setting
Version:    0.0.1
Release:    1
Summary: Tizen watch-setting application
URL: http://slp-source.sec.samsung.net
Source: %{name}-%{version}.tar.gz
License: Flora-1.1
Group: TO_BE/FILLED_IN

%if "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif
%if "%{?tizen_profile_name}" == "mobile"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires: cmake
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(embryo)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(appcore-common)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-appfw-app-manager)
BuildRequires: pkgconfig(feedback)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(deviced)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: gettext-tools
BuildRequires: edje-bin, embryo-bin
BuildRequires: elementary-devel
BuildRequires: pkgconfig(bluetooth-api)
BuildRequires: pkgconfig(capi-network-bluetooth)
BuildRequires: pkgconfig(capi-base-common)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires: pkgconfig(capi-media-sound-manager)
BuildRequires: pkgconfig(capi-media-player)
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(mm-player)
BuildRequires: pkgconfig(mm-sound)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(capi-media-wav-player)
#BuildRequires: pkgconfig(capability-manager)
#BuildRequires: model-build-features
BuildRequires: pkgconfig(ail)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(glib-2.0)
%description
W watch-setting application

%prep
%setup -q

%define PREFIX %{TZ_SYS_RO_APP}/org.tizen.watch-setting

%build
#%if 0export CFLAGS="${CFLAGS} -fPIC -fvisibility=hidden -fvisibility-inlines-hidden"
#CFLAGS+=" -fvisibility=hidden"; export CFLAGS
#CXXFLAGS+=" -fvisibility=hidden"; export CXXFLAGS
#FFLAGS+=" -fvisibility=hidden"; export FFLAGS

%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS+=" -DTIZEN_ENGINEER_MODE"
export CXXFLAGS+=" -DTIZEN_ENGINEER_MODE"
export FFLAGS+=" -DTIZEN_ENGINEER_MODE"
%endif
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

LDFLAGS+="-Wl,--rpath=%{PREFIX}/lib -Wl,--as-needed -Wl,--hash-style=both"; export LDFLAGS
cmake . -DCMAKE_INSTALL_PREFIX=%{PREFIX} \
	-DTZ_SYS_RO_APP=%{TZ_SYS_RO_APP} \
	-DTZ_SYS_DATA=%{TZ_SYS_DATA} \
	-DTZ_SYS_ETC=%{TZ_SYS_ETC} \
	-DTZ_SYS_SHARE=%{TZ_SYS_SHARE} \
	-DTZ_USER_CONTENT=%{TZ_USER_CONTENT} \
%if 0%{?sec_build_binary_sdk}
	-DFEATURE_SETTING_SDK=YES \
%endif
%ifarch %{ix86}
	-DFEATURE_SETTING_EMUL=YES \
%endif
#%if "%{?sec_build_project_name}" == "rinato_eur_open"
#	-DFEATURE_SETTING_TIZENW2=YES \
#%endif

make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}

%make_install
mkdir -p %{buildroot}/%{TZ_SYS_RW_APPS}/org.tizen.watch-setting
mkdir -p %{buildroot}/%{TZ_SYS_RW_APPS}/org.tizen.watch-setting/data/images

%clean

%post

# Set vconf values with -g/-u options
GOPTION="-g 6514"

#resetMenuscreen
	##vconftool $GOPTION set -t string db/setting/menuscreen/selected "org.tizen.cluster-home"
%ifarch %{arm}
	#vconftool $GOPTION set -t string db/setting/menuscreen/package_name "org.tizen.cluster-home" -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/setting/homescreen/package_name "org.tizen.cluster-home" -s system::vconf_setting
%else
	#vconftool $GOPTION set -t string db/setting/menuscreen/package_name "org.tizen.menu-screen" -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/setting/homescreen/package_name "org.tizen.menu-screen" -s system::vconf_setting
%endif

#resetFlightmode
#	#vconftool $GOPTION set -t bool db/telephony/flight_mode "0" -f -s system::vconf_network

#resetHomeBG
	#vconftool $GOPTION set -t int db/setting/home_bg_color_type "0" -s system::vconf_inhouse

	#vconftool $GOPTION set -t int db/setting/idle_clock_type "9" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/hourly_alert "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/prefer_arm_left "1" -s system::vconf_inhouse

#resetMotions
	#vconftool $GOPTION set -t int db/setting/motion_wake_up_guesture "2" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/motion_smart_relay "0" -s system::vconf_inhouse


#resetNetwork
	#vconftool $GOPTION set -t int db/setting/select_network "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/select_network_act "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/network_mode "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/3gEnabled "1" -s org.tizen.setting::system
	#vconftool $GOPTION set -t bool db/setting/data_roaming "0" -s system::vconf_system
	#vconftool $GOPTION set -t bool memory/setting/network_mode_changed "0" -i -s system::vconf_system

#resetUsbConnectivity
	#vconftool $GOPTION set -t int memory/setting/usb_mode "-1" -i -s system::vconf_system
	#vconftool $GOPTION set -t int memory/setting/usb_sel_mode "0" -i -s system::vconf_system
	#vconftool $GOPTION set -t int memory/setting/usb_in_mode_change "0" -i -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/debug_mode "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_rendering_engine "1" -i -s system::vconf_system

#resetSound
%ifarch %{arm}
	DEFAULT_CALL_TONE="%{TZ_SYS_SHARE}/settings/Ringtones/Twinkle.ogg"
	#DEFAULT_CALL_TONE="%{TZ_SYS_SHARE}/settings/Ringtones/Ringtone1.ogg"
%else
	DEFAULT_CALL_TONE="%{TZ_SYS_SHARE}/settings/Ringtones/Ringtone.ogg"
	#DEFAULT_CALL_TONE="%{TZ_SYS_SHARE}/settings/Ringtones/Ringtone1.ogg"
%endif

%ifarch %{arm}
	DEFAULT_NOTI_TONE="%{TZ_SYS_SHARE}/settings/Alerts/Flicker.ogg"
	#DEFAULT_NOTI_TONE="%{TZ_SYS_SHARE}/settings/Alerts/Notification1.ogg"
%else
	DEFAULT_NOTI_TONE="%{TZ_SYS_SHARE}/settings/Alerts/Notification.ogg"
	#DEFAULT_NOTI_TONE="%{TZ_SYS_SHARE}/settings/Alerts/Notification1.ogg"
%endif

	#vconftool $GOPTION set -t bool db/setting/sound/sound_on "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/sound/vibration_on "1" -s system::vconf_inhouse

	#vconftool $GOPTION set -t bool db/setting/sound/vibrate_when_ringing "1" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/sound/vibrate_when_notification "1" -s system::vconf_setting

	#vconftool $GOPTION set -t int db/setting/sound/call/ringtone_sound_volume "11" -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/sound/call/rmd_ringtone_volume "11" -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/sound/noti/sound_volume "11" -s system::vconf_inhouse
	#vconftool $GOPTION set -t int db/setting/sound/media/sound_volume "11" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/sound/touch_feedback/sound_volume "9" -s org.tizen.setting::private

	#vconftool $GOPTION set -t int db/setting/sound/noti/vibration_level "3" -s system::vconf_inhouse
	#vconftool $GOPTION set -t int db/setting/sound/touch_feedback/vibration_level "3" -s system::vconf_inhouse
	#vconftool $GOPTION set -t int db/setting/sound/touch_feedback/vibration_level_bak "3" -s org.tizen.setting::private

	#vconftool $GOPTION set -t string db/setting/sound/call/ringtone_path "${DEFAULT_CALL_TONE}" -f -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/setting/sound/call/ringtone_default_path "${DEFAULT_CALL_TONE}" -f -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/sound/call/vibration_type "2" -s system::vconf_setting
	#vconftool $GOPTION set -t string db/setting/sound/call/vibration_pattern_path "/opt/share/settings/Vibrations/haptic/default/Notification_Call.ivt" -f -s system::vconf_setting
	#vconftool $GOPTION set -t string db/setting/sound/noti/msg_ringtone_path	"${DEFAULT_NOTI_TONE}" -f -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/setting/sound/noti/ringtone_default_path	"${DEFAULT_NOTI_TONE}" -f -s system::vconf_inhouse
	#vconftool $GOPTION set -t int db/setting/sound/noti/msg_alert_rep_type		"0" -s system::vconf_inhouse

	#vconftool $GOPTION set -t string db/setting/sound/noti/email_ringtone_path	"${DEFAULT_NOTI_TONE}" -f -s system::vconf_inhouse
	#vconftool $GOPTION set -t int db/setting/sound/noti/email_alert_rep_type		"0" -s system::vconf_inhouse

	#vconftool $GOPTION set -t bool db/setting/sound/button_sounds "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/sound/touch_sounds "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/sound/sound_lock "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/sound/haptic_feedback_on "1" -s system::vconf_inhouse

	#vconftool $GOPTION set -t bool db/setting/sound/sound_on_bak "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/sound/touch_sounds_bak "1" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/sound/sound_lock_bak "1" -s org.tizen.setting::private
#resetWallpaper
	#vconftool $GOPTION set -t string db/menu_widget/bgset "" -f -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/idle_lock/bgset "/opt/usr/share/settings/Wallpapers/Lock_default.png" -f -s system::vconf_inhouse

#resetTilt
	#vconftool $GOPTION set -t bool db/setting/use_tilt "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t int db/setting/tilt_sensitivity "3" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/use_tilt_scroll "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/tilt_scroll_sensitivity "3" -s org.tizen.setting::private

#resetPanning
	#vconftool $GOPTION set -t bool db/setting/use_panning "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t int db/setting/panning_sensitivity "3" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/use_panning_browser "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t int db/setting/panning_browser_sensitivity "3" -s org.tizen.setting::private

#resetDoubleTap
	#vconftool $GOPTION set -t bool db/setting/use_double_tap "0" -s system::vconf_setting

#resetShake
	#vconftool $GOPTION set -t bool db/setting/use_shake "0" -s system::vconf_inhouse

#resetPickup
	#vconftool $GOPTION set -t bool db/setting/use_pick_up "1" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/use_pick_up_call "0" -s system::vconf_inhouse

#resetTurnOver
	#vconftool $GOPTION set -t bool db/setting/use_turn_over "0" -s system::vconf_setting

#resetMotions
	#vconftool $GOPTION set -t bool db/setting/motion_active "0" -s system::vconf_system
	#vconftool $GOPTION set -t bool db/setting/motion/palm_swipe_capture "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/motion/palm_touch_mute "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/motion/tap_twist "1" -s org.tizen.setting::private

#resetDisplay
	#backlight
%ifarch %{arm}
	#vconftool $GOPTION set -t int db/setting/lcd_backlight_normal "10" -s system::vconf_system
%else
	#vconftool $GOPTION set -t int db/setting/lcd_backlight_normal "10" -s system::vconf_system
%endif
	#vconftool $GOPTION set -t int db/setting/lcd_timeout_normal_backup "10" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/automatic_brightness_level "60" -f -s org.tizen.setting::private
	#brightness
	#vconftool $GOPTION set -t int db/setting/Brightness "80" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/brightness_automatic "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/auto_display_adjustment "1" -s system::vconf_inhouse

	#battery
	#vconftool $GOPTION set -t bool db/setting/battery_percentage "1" -s system::vconf_inhouse
	#launch
	##vconftool $GOPTION set -t string db/menu_widget/launch_effect "0"

	#smart screen
	#vconftool $GOPTION set -t int db/pm/smartstay_status "0" -f -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/smartscreen/smart_rotation "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/auto_adjust_screen_tone "1" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/smartscreen/smart_stay_r "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/smartscreen/smart_rotation_r "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/display/touchkey_light_duration "90" -s system::vconf_inhouse
	#vconftool $GOPTION set -t int db/setting/display/screen_capture_destination "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/display/edit_after_screen_capture "0" -s system::vconf_inhouse

	#LED indicator
	#vconftool $GOPTION set -t bool db/setting/led_indicator/charging "1" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/led_indicator/low_batt "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/led_indicator/notifications "1" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/led_indicator/voice_rec "1" -s system::vconf_setting

	#Accessory SViewer
	#vconftool $GOPTION set -t bool db/setting/accessories/atuomatic_unlock "1" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/accessories/dock_sound "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/accessories/audio_output_mode "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/accessories/audio_output_mode_reminder "0" -f -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/accessories/desk_home_screen_display "1" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/accessories/hdmi_application "0" -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/accessories/audio_output "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/accessories/audio_applications "0" -s system::vconf_setting


#resetPowersaving
	#vconftool $GOPTION set -t bool db/setting/pwrsv/system_mode/status "0" -s org.tizen.setting::private

	# remove this - and use 'preference'
	#vconftool $GOPTION set -t bool db/setting/pwrsv/system_mode/reminder "1" -s org.tizen.setting::private
	#---------------------------------------------------------------------------

	#vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/status "0" -s system::vconf_inhouse

	#vconftool $GOPTION set -t int db/setting/pwrsv/custom_mode/at "30" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/wifi "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/bt "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/gps "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/data_sync "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/hotspot "1" -s org.tizen.setting::private

	#vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/brt/status  "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/brt/auto/status "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/pwrsv/custom_mode/brt/value "10" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/pwrsv/custom_mode/backlight/time "15" -s org.tizen.setting::private

	#v0.4
        #vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/cpu "0" -s system::vconf_inhouse
        #vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/display "0" -s system::vconf_inhouse
        #vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/bg_color "0" -s system::vconf_inhouse
        #vconftool $GOPTION set -t bool db/setting/pwrsv/custom_mode/screen_vib "0" -s system::vconf_inhouse

#resetFont
	#vconftool $GOPTION set -t int db/setting/font_size "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/font_type "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t string memory/setting/tmp_font_name ""  -i -s org.tizen.setting::private

#resetRotationLock

	# to be removed
	#vconftool $GOPTION set -t bool db/setting/rotate_lock "1"  -s org.tizen.setting::private

	#vconftool $GOPTION set -t bool db/setting/auto_rotate_screen "1" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool memory/setting/rotate_hold "0" -i -f -s org.tizen.setting::private

#resetTimeAndData
%ifarch %{arm}
	#vconftool $GOPTION set -t bool db/setting/automatic_time_update "1" -s system::vconf_inhouse
%else
	#vconftool $GOPTION set -t bool db/setting/automatic_time_update "0" -s system::vconf_inhouse
%endif

	#vconftool $GOPTION set -t int db/menu_widget/regionformat_time1224 "1" -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/date_format "0" -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/weekofday_format  "0" -s system::vconf_inhouse

	#vconftool $GOPTION set -t string db/setting/timezone "+9:00" -s system::vconf_setting
	# to be removed
	#vconftool $GOPTION set -t string db/setting/cityname_id "IDS_WCL_BODY_CITYNAME_SEOUL" -s system::vconf_setting

	#vconftool $GOPTION set -t string db/setting/timezone_id "Asia/Seoul" -s system::vconf_inhouse

#	rm -f %{TZ_SYS_ETC}/localtime
#	ln -s /usr/share/zoneinfo/Asia/Seoul %{TZ_SYS_ETC}/localtime
#	rm -f /etc/localtime
#	ln -s %{TZ_SYS_ETC}/localtime /etc/localtime

#resetAccessibility
	#vconftool $GOPTION set -t bool db/setting/accessibility/high_contrast "0" -s system::vconf_system
	#vconftool $GOPTION set -t bool db/setting/accessibility/screen_zoom "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t int db/setting/accessibility/font_size "1" -s system::vconf_inhouse	##vconftool $GOPTION set -t int db/setting/accessibility/font_style "0"
	#vconftool $GOPTION set -t string db/setting/accessibility/font_name "HelveticaNeue" -s system::vconf_misc
	#vconftool $GOPTION set -t bool db/setting/accessibility/tts "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/accessibility/speech_rate "2" -s system::vconf_inhouse

	#vconftool $GOPTION set -t bool memory/setting/accessibility/torch_light "0" -i -s system::vconf_system
	#vconftool $GOPTION set -t bool db/setting/accessibility/assistive_light_reminder "1" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/accessibility/mono_audio "0" -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/accessibility/power_key_hold "1" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/accessibility/led_notify "0" -s system::vconf_setting
	#vconftool $GOPTION set -t string db/setting/accessibility/led_playing_path "/usr/share/feedback/led/default/Ticktock.led" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/accessibility/turn_off_all_sounds "0" -s system::vconf_setting

	#vconftool $GOPTION set -t bool db/setting/accessibility/speak_passwd  "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/accessibility/lock_time "5" -s system::vconf_setting
	#vconftool $GOPTION set -t string db/setting/accessibility/taphold_delay "IDS_COM_POP_SHORT" -s org.tizen.setting::private

# duplicated key creation (it's in Call setting)
#	#vconftool $GOPTION set -t bool db/ciss/call_answering_key "0"
#	#vconftool $GOPTION set -t bool db/ciss/call_power_key_ends_call "0"
#	#vconftool $GOPTION set -t int db/ciss/answering_mode "0"
#	#vconftool $GOPTION set -t int db/ciss/answering_mode_time "3"

	#vconftool $GOPTION set -t bool db/setting/accessibility/accept_call "0" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/accessibility/enable_auto_answer "0" -s org.tizen.setting::private
        #vconftool $GOPTION set -t int db/setting/accessibility/auto_answer "1" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/accessibility/powerkey_end_calls "0" -s org.tizen.setting::private

    	#vconftool $GOPTION set -t int db/setting/accessibility/sound_balance "50" -s org.tizen.setting::private
# Colorblind
	#vconftool $GOPTION set -t bool db/setting/colorblind/status "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t string db/setting/colorblind/chip_array "" -s org.tizen.setting::private
	#vconftool $GOPTION set -t string db/setting/colorblind/rgbcmy "" -s org.tizen.setting::private
	#vconftool $GOPTION set -t double db/setting/colorblind/user_adjustment "0.0" -s org.tizen.setting::private
# Driving Mode
	#vconftool $GOPTION set -t bool db/setting/drivingmode/drivingmode "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/drivingmode/incomingcall "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/drivingmode/message "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/drivingmode/newemails "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/drivingmode/newvoicemails "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/drivingmode/alarm "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/drivingmode/schedule "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/drivingmode/unlockscreen "0" -s system::vconf_setting

# Easymode
	#vconftool $GOPTION set -t bool db/setting/homescreen/easymode "0" -s system::vconf_inhouse

# Blocking Mode
	#vconftool $GOPTION set -t bool db/setting/blockingmode/blockingmode "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/blockingmode/inter/incomingcall "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/blockingmode/inter/notifications "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/blockingmode/inter/alarm_and_timer "0" -s system::vconf_setting

	#vconftool $GOPTION set -t int db/setting/blockingmode/start_hour "21" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/blockingmode/start_min "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/blockingmode/end_hour "6" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/blockingmode/end_min "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/blockingmode/incomingcall "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/blockingmode/notifications "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/blockingmode/alarm_and_timer "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/blockingmode/allday "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/blockingmode/allowed_contact_type "0" -s system::vconf_setting
	#vconftool $GOPTION set -t string db/setting/blockingmode/allowed_contact_list "" -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/blockingmode/start_alarm_id "" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/blockingmode/end_alarm_id "" -s org.tizen.setting::private

#resetLanguageAndRegion
	#vconftool $GOPTION set -t int db/setting/lang "9" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/lang_automatic "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/region_automatic "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t string db/menu_widget/language "en_GB.UTF-8" -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/menu_widget/regionformat "en_GB.UTF-8" -s system::vconf_inhouse

#resetViewtype
	##vconftool $GOPTION set -t int db/menuscreen/viewtype "0"
	##vconftool $GOPTION set -t int db/taskswitcher/viewtype "0"

#resetTouch
	##vconftool $GOPTION set -t int db/setting/vib_feedback "3"
	##vconftool $GOPTION set -t bool db/setting/touch_panel_autolock "0"

#resetLicense
	#vconftool $GOPTION set -t bool db/setting/transaction_tracking "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/expiry_reminder "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/roaming_network "0" -s org.tizen.setting::private

#resetNotification
	#vconftool $GOPTION set -t bool db/setting/ticker_noti/messages "1" -s system::vconf_setting
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/email "1" -s system::vconf_inhouse
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/im "1" -s system::vconf_setting
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/twitter "1" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/facebook "1" -s org.tizen.setting::private

		#vconftool $GOPTION set -t bool db/setting/ticker_noti/sound/email "1" -s org.tizen.setting::private
		#vconftool $GOPTION set -t bool db/setting/ticker_noti/sound/messages "1" -s org.tizen.setting::private
		#vconftool $GOPTION set -t bool db/setting/ticker_noti/sound/im "1" -s org.tizen.setting::private

        # display content
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/display_content/messages "1" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/display_content/email "1" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/display_content/im "1" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/display_content/twitter "1" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/display_content/facebook "1" -s org.tizen.setting::private

        #vconftool $GOPTION set -t bool db/setting/ticker_noti/badge/messages "1" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/badge/email "1" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/badge/im "1" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/badge/twitter "1" -s org.tizen.setting::private
        #vconftool $GOPTION set -t bool db/setting/ticker_noti/badge/facebook "1" -s org.tizen.setting::private

#resetSecurity
	rm -rf %{TZ_SYS_DATA}/setting/set_info
	##vconftool $GOPTION set -t string db/setting/privacy_passwd ""

	#vconftool $GOPTION set -t bool db/setting/power_on_lock "0" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/simple_password "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/screen_lock_type "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/setting/3rd_lock_pkg_name "org.tizen.lockscreen" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/fmm/sim_change_alert "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/setting/fmm/recipients "" -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/setting/fmm/sender "" -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/setting/fmm/alert_message "" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/fmm/remote_control "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/fmm/location_consent "0" -s system::vconf_inhouse

	# for MMC encryption
	#vconftool $GOPTION set -t bool db/setting/mmc_encryption/status "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/mmc_encryption/option "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/mmc_encryption/exclude_multimedia "0" -s org.tizen.setting::private

	# NOT USED NOW.
	##vconftool $GOPTION set -t bool db/setting/rcs "0"

	#vconftool $GOPTION set -t int db/setting/phone_lock_attempts_left "5" -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/setting/phone_lock_timestamp "" -s system::vconf_inhouse
	#vconftool $GOPTION set -t int db/setting/sim_lock_attempts_left "5" -s org.tizen.setting::private
	#vconftool $GOPTION set -t string db/setting/sim_lock_timestamp "" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/fixed_dialing_mode "0" -s org.tizen.setting::private
#resetMemory
	#vconftool $GOPTION set -t int db/setting/default_memory/wap "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/download "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/download_nfc "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/download_contents "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/download_application "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/wifi_direct "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/install_applications "0" -s system::vconf_system

	#vconftool $GOPTION set -t int db/setting/default_memory/bluetooth "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/camera "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/voice_recorder "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/fm_radio "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/all_share "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/adobe_air "0" -s system::vconf_system
	#vconftool $GOPTION set -t int db/setting/default_memory/dvb_h "0" -s system::vconf_system

	# format - system server
	##vconftool $GOPTION -i set -t int memory/mmc/format "0"

#resetAbout
	#vconftool $GOPTION set -t string db/setting/device_name "Gear 2" -s system::vconf_network
	#vconftool $GOPTION set -t string db/setting/selected_num "" -s org.tizen.setting::private
#resetMenuWidgets
	#vconftool $GOPTION -i set -t int memory/setting/font_changed "0" -s org.tizen.setting::private
	##vconftool $GOPTION -i set -t int memory/mobile_hotspot/skin_changed "0"

#resetDevoptions
	#vconftool $GOPTION -i set -t int db/setting/devoption/bgprocess "0" -s system::vconf_setting

#resetDatausage
	#vconftool $GOPTION set -t bool db/setting/set_data_usage_limit "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/data_limit "-1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/data_usage_cycle "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/data_each_month "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/data_each_month_app "1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/data_usage_roaming_status "0" -s org.tizen.setting::private
	#vconftool $GOPTION set -t int db/setting/data_limit_roaming "-1" -s org.tizen.setting::private
	#vconftool $GOPTION set -t bool db/setting/set_data_usage_limit_roaming "0" -s org.tizen.setting::private

#resetDisplay
	#vconftool $GOPTION set -t string db/setting/screenmode/selected_name "Dynamic" -f -s org.tizen.setting::private

#resetMostRecentlySetting
	#vconftool $GOPTION set -t string db/setting/most_recently_setting "" -s org.tizen.setting::private

#psmode(power saving mode)
	#vconftool $GOPTION set -t int db/setting/psmode "0" -s system::vconf_setting

#resetMyplace
	#vconftool $GOPTION set -t string db/setting/myplace_home "" -s system::vconf_inhouse
	#vconftool $GOPTION set -t string db/setting/myplace_office "" -s system::vconf_inhouse

# personal page(not include on vconf internal yet)
	#vconftool $GOPTION set -t bool memory/setting/personal "0" -i -s system::vconf_setting
	#vconftool $GOPTION set -t string db/setting/personal_key "" -s system::vconf_setting

# homescreen layout type
	#vconftool $GOPTION set -t int db/setting/homescreen_type "1" -s system::vconf_inhouse

# privacy setting
	#vconftool $GOPTION set -t int db/setting/lock_type "0" -s system::vconf_inhouse
	#vconftool $GOPTION set -t bool db/setting/see_pattern "1" -s system::vconf_inhouse

# display screen rotation
	#vconftool $GOPTION set -t int db/setting/screen_rotation "0" -s system::vconf_inhouse

# display home screen bg set
	#vconftool $GOPTION set -t int db/setting/homescreen_bg "0" -s system::vconf_inhouse

# support emergency mode
	#vconftool $GOPTION set -t bool db/setting/support_emergency "1" -s system::vconf_setting
	#vconftool $GOPTION set -t bool db/setting/emergency_status "0" -s system::vconf_setting

# volume toast show
#	#vconftool $GOPTION set -t int memory/setting/volume_ring_toast_show "0" -i -f -s org.tizen.setting::private
#	#vconftool $GOPTION set -t int memory/setting/volume_noti_toast_show "0" -i -f -s org.tizen.setting::private

# safety, emergency mode
	#vconftool $GOPTION set -t int db/setting/emergency_wakeup_gesture "0" -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/emergency_brightness "1" -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/emergency_lcd_timeout "10" -s system::vconf_setting
	#vconftool $GOPTION set -t string db/setting/emergency_ringtone "/opt/share/settings/Emergency/Low_power_ringtone.ogg" -s system::vconf_setting

# power saver wearable
	#vconftool $GOPTION set -t int db/setting/pws_wakeup_gesture "0" -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/pws_brightness "1" -s system::vconf_setting
	#vconftool $GOPTION set -t int db/setting/pws_lcd_timeout "10" -s system::vconf_setting
	#vconftool $GOPTION set -t string db/setting/pws_ringtone "/opt/share/settings/Emergency/Low_power_ringtone.ogg" -s system::vconf_setting

	#vconftool $GOPTION set -t string db/wms/clocks_set_idle "org.tizen.idle-clock-digital"  -u 5000 -s system::vconf
	#vconftool $GOPTION set -t int db/wms/home_bg_mode 0 -u 5000 -s system::vconf
	#vconftool $GOPTION set -t string db/wms/home_bg_palette "000000" -u 5000 -s system::vconf
	#vconftool $GOPTION set -t string db/wms/home_bg_wallpaper "wallpaper_01.png"  -u 5000 -s system::vconf
	#vconftool $GOPTION set -t string db/wms/home_bg_gallery "/opt/usr/media/.bgwallpaper.jpg" -u 5000 -s system::vconf 
	
	# Idle clock edit mode ( with Home )
    #vconftool $GOPTION set -t int db/setting/idle_clock_show "0" -s system::vconf_inhouse

if [ -d %{TZ_SYS_SHARE}/settings ]
then
	rm -rf %{TZ_SYS_SHARE}/settings
fi

#ln -s /opt/usr/share/settings /opt/share/settings
#+ln -s %{TZ_SYS_SHARE}/settings %{TZ_SYS_SHARE}/settings

# shared dir
mkdir -p %{TZ_SYS_RO_APP}/org.tizen.watch-setting/shared
mkdir -p %{TZ_SYS_RO_APP}/org.tizen.watch-setting/shared/res

mkdir -p %{TZ_SYS_DATA}/setting

%files
%manifest %{name}.manifest
/etc/smack/accesses.d/org.tizen.watch-setting.efl
%defattr(-,root,root,-)
%attr(-,inhouse,inhouse)
%dir %{PREFIX}/data
#%{PREFIX}/bin/*
#%{PREFIX}/res/*
/usr/share/packages/*
/usr/share/icons/default/small/*
#/usr/share/packages/%{name}.xml
#/usr/apps/org.tizen.watch-setting/data/images/*
%{TZ_SYS_RO_APP}/org.tizen.watch-setting/*
/usr/apps/org.tizen.watch-setting/shared/res/*
#/opt/usr/share/settings/*
/usr/share/Safety.zip
/opt/usr/data/setting/*
