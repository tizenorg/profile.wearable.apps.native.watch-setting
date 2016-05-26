CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(watch-setting C)

SET(SRCS
    src/setting.c
    src/setting-sound.c
    src/setting-vibration.c
    src/setting-connection.c
    src/setting-clock.c
    src/setting-homescreen.c
    src/setting-volume.c
    src/setting-info.c
#    src/setting-bluetooth.c
#    src/setting-motion.c
    src/setting-device.c
    src/setting-display.c
    src/setting-privacy.c
    src/setting-double.c
    src/setting-language.c
    src/setting-safety.c
    src/setting-reset.c
    src/setting-battery.c
    src/setting-common-sound.c
    src/util.c
    src/setting_data_vconf.c
    src/setting_control_haptic.c
    src/setting_control_bt.c
    src/setting_view_toast.c
    src/setting-notification.c
    src/setting-profile.c
    src/setting_indicator_util.c
    )

SET(VENDOR "samsung")
SET(PACKAGE ${PROJECT_NAME})
SET(PKGNAME "com.${VENDOR}.${PACKAGE}")
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
#SET(INSTALL_DIR_APPS "${TZ_SYS_RO_APP}/${PKGNAME}")
SET(INSTALL_DIR_PRE "/usr")

SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(DATADIR "${PREFIX}/data")
SET(LOCALEDIR "${RESDIR}/locale")
SET(ICONDIR "${RESDIR}/icons")
SET(DESKTOPICONDIR "${ICONDIR}/default/small")
SET(EDJ_PATH "${RESDIR}/edje")
#SET(EDJ_PATH ".")
SET(IMGDIR "./data/images")
SET(IMG_PATH "data/images-b2")
SET(FAKE_IMG_PATH "${TZ_SYS_RO_APP}/${PKGNAME}/shared/res")

#IF(FEATURE_SETTING_CHANGEABLE)
SET(CHANGEABLE_PATH "${PREFIX}/shared/res/tables")
#ENDIF(FEATURE_SETTING_CHANGEABLE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

#IF(FEATURE_SETTING_TIZENW2)
#	MESSAGE("tiznew2 build!!!!!!!!!")
#	ADD_DEFINITIONS("-DFEATURE_SETTING_TIZENW2")
#ELSE(FEATURE_SETTING_TIZENW2)
#	MESSAGE("tiznew build!!!!!!!!!")
#ENDIF(FEATURE_SETTING_TIZENW2)

IF(FEATURE_SETTING_SDK)
	MESSAGE("tiznew2 sdk build!!!!!!!!!")
	ADD_DEFINITIONS("-DFEATURE_SETTING_SDK")
ELSE(FEATURE_SETTING_SDK)
	MESSAGE("tiznew2 build!!!!!!!!!")
ENDIF(FEATURE_SETTING_SDK)
IF(FEATURE_SETTING_EMUL)
	ADD_DEFINITIONS("-DFEATURE_SETTING_EMUL")
	MESSAGE("tiznew2 emul build!!!!!!!!!")
ENDIF(FEATURE_SETTING_EMUL)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED elementary ecore-imf appcore-common appcore-efl
                                dlog glib-2.0 evas eina edje vconf elementary sensor
                                ecore ecore-input ecore-evas capi-appfw-application
				pkgmgr pkgmgr-info fontconfig
                                feedback json-glib-1.0 deviced capi-network-bluetooth bluetooth-api
                                capi-system-device capi-media-sound-manager capi-media-player mm-player mm-sound
                                capi-system-system-settings capi-system-info libxml-2.0 capi-media-wav-player
				#capability-manager
				capi-content-media-content aul icu-i18n icu-io icu-le icu-lx icu-uc
				capi-appfw-app-manager
				alarm-service
				libtzplatform-config
				efl-extension
				capi-network-wifi
				storage
)

#??
FIND_LIBRARY(LIB_M m)

#INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})

#Icon - app
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.watch-setting.png DESTINATION ${ICONDIR})

#Icon - main menu
IF(FEATURE_SETTING_EMUL)
ELSE(FEATURE_SETTING_EMUL)
	INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/images-b2/ DESTINATION ${IMGDIR} FILES_MATCHING PATTERN "*.png")
ENDIF(FEATURE_SETTING_EMUL)

# install media-data
	INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/media-data-sdk/settings/ DESTINATION ${TZ_SYS_SHARE}/settings PATTERN "*")

# install langlist xml file
IF(FEATURE_SETTING_EMUL)
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/langlist_emul.xml DESTINATION ${DATADIR} RENAME langlist.xml)
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/langlist_emul.xml DESTINATION /opt/usr/data/setting RENAME langlist.xml)
ELSEIF(FEATURE_SETTING_SDK)
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/langlist_emul.xml DESTINATION ${DATADIR} RENAME langlist.xml)
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/langlist_emul.xml DESTINATION /opt/usr/data/setting RENAME langlist.xml)
ELSE(FEATURE_SETTING_EMUL)
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/langlist.xml DESTINATION ${DATADIR})
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/langlist.xml DESTINATION /opt/usr/data/setting)
ENDIF(FEATURE_SETTING_EMUL)
#INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/langlist_all.xml DESTINATION ${DATADIR})

# install clocklist xml file
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/clocklist.xml DESTINATION ${DATADIR})

# install media-data
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/fake-img/ DESTINATION ${FAKE_IMG_PATH} FILES_MATCHING PATTERN "*.png")

# install Safety.zip
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/Safety.zip DESTINATION /usr/share)

# install color/font code table
IF(FEATURE_SETTING_CHANGEABLE)
	INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/changeable/tables/ DESTINATION ${CHANGEABLE_PATH} FILES_MATCHING PATTERN "*.xml")
ENDIF(FEATURE_SETTING_CHANGEABLE)

ADD_DEFINITIONS(${pkgs_CFLAGS})
ADD_DEFINITIONS("-fpie")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFACTORYFS=\"$ENV{FACTORYFS}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")
ADD_DEFINITIONS("-DSLP_PROF")

ADD_DEFINITIONS("-DVENDOR=\"${VENDOR}\"")
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DPACKAGE_NAME=\"${PKGNAME}\"")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")

ADD_DEFINITIONS("-DINSTALL_DIR_APPS=\"${INSTALL_DIR_APPS}\"")
ADD_DEFINITIONS("-DBINDIR=\"${BINDIR}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DICON_DIR=\"${ICONDIR}\"")
#ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")
ADD_DEFINITIONS("-DELM_DEMO_EDJ=\"${EDJ_PATH}/${PROJECT_NAME}.edj\"")
ADD_DEFINITIONS("-DTZ_SYS_RO_APP_D=\"${TZ_SYS_RO_APP}\"")
ADD_DEFINITIONS("-DTZ_SYS_DATA_D=\"${TZ_SYS_DATA}\"")
ADD_DEFINITIONS("-DTZ_SYS_ETC_D=\"${TZ_SYS_ETC}\"")
ADD_DEFINITIONS("-DTZ_SYS_SHARE_D=\"${TZ_SYS_SHARE}\"")
ADD_DEFINITIONS("-DTZ_USER_CONTENT_D=\"${TZ_USER_CONTENT}\"")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} "-pie" ${LIB_M})

#edj compile
IF(FEATURE_SETTING_CHANGEABLE)
ADD_CUSTOM_TARGET(${PROJECT_NAME}.edj
    COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/data/changeable/groups/images
    ${CMAKE_SOURCE_DIR}/data/changeable/${PROJECT_NAME}.edc ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.edj
    DEPENDS ${CMAKE_SOURCE_DIR}/data/changeable/${PROJECT_NAME}.edc
)
ADD_CUSTOM_TARGET(portrait.edj
    COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/data/changeable/groups/images
    ${CMAKE_SOURCE_DIR}/data/changeable/fake_effect.edc ${CMAKE_BINARY_DIR}/portrait.edj
    DEPENDS ${CMAKE_SOURCE_DIR}/data/changeable/fake_effect.edc
)
ELSE(FEATURE_SETTING_CHANGEABLE)
ADD_CUSTOM_TARGET(${PROJECT_NAME}.edj
    COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/data/groups/images    #data/images
    ${CMAKE_SOURCE_DIR}/data/${PROJECT_NAME}.edc ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.edj
    DEPENDS ${CMAKE_SOURCE_DIR}/data/${PROJECT_NAME}.edc
)
ENDIF(FEATURE_SETTING_CHANGEABLE)

ADD_DEPENDENCIES(${PROJECT_NAME} ${PROJECT_NAME}.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} edj_build)


MESSAGE("bindir: ${BINDIR}")

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION
${BINDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.edj DESTINATION ${EDJ_PATH})


# install application HOME directory
INSTALL(DIRECTORY DESTINATION ${DATADIR})
#ADD_SUBDIRECTORY(po)

# desktop icon
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/org.tizen.watch-setting.xml.in ${CMAKE_SOURCE_DIR}/org.tizen.watch-setting.xml)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.watch-setting.xml DESTINATION /usr/share/packages)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.watch-setting.png DESTINATION  /usr/share/icons/default/small)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.watch-setting_small.png DESTINATION  /usr/share/icons/default/small)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.watch-setting.efl DESTINATION  /etc/smack/accesses.d/)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/history DESTINATION /opt/usr/apps/org.tizen.watch-setting/data)

#INSTALL fake image
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/images/fake/calc_por.jpg DESTINATION /${TZ_SYS_RO_APP}/org.tizen.watch-setting/res/images)

ADD_SUBDIRECTORY(po)
