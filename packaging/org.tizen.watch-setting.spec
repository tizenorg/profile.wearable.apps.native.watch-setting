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
#BuildRequires: pkgconfig(ail)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  pkgconfig(icu-io)
BuildRequires:  pkgconfig(icu-le)
BuildRequires:  pkgconfig(icu-lx)
BuildRequires:  pkgconfig(icu-uc)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(capi-network-wifi)
BuildRequires: pkgconfig(storage)

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

#resetTimeAndData

	rm -f %{TZ_SYS_ETC}/localtime
	ln -s /usr/share/zoneinfo/Asia/Seoul %{TZ_SYS_ETC}/localtime
	rm -f /etc/localtime
	ln -s %{TZ_SYS_ETC}/localtime /etc/localtime


#resetSecurity
	rm -rf %{TZ_SYS_DATA}/setting/set_info


#ln -s /opt/usr/share/settings /opt/share/settings
#+ln -s %{TZ_SYS_SHARE}/settings %{TZ_SYS_SHARE}/settings

# shared dir
mkdir -p %{TZ_SYS_RO_APP}/org.tizen.watch-setting/shared
mkdir -p %{TZ_SYS_RO_APP}/org.tizen.watch-setting/shared/res

mkdir -p %{TZ_SYS_DATA}/setting
mkdir -p %{TZ_SYS_SHARE}/settings

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
%{TZ_SYS_RO_APP}/org.tizen.watch-setting/*
%{TZ_SYS_SHARE}/*
/usr/share/Safety.zip
/opt/usr/data/setting/*
