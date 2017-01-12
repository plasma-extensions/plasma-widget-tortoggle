add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.torcontrol\")

set(torcontrol_SRCS
    torcontrol.cpp
)

add_library(plasma_applet_torcontrol MODULE ${torcontrol_SRCS})

kcoreaddons_desktop_to_json(plasma_applet_torcontrol package/metadata.desktop SERVICE_TYPES plasma-applet.desktop)

target_link_libraries(plasma_applet_torcontrol
                      Qt5::Gui
                      KF5::Plasma
                      KF5::I18n)


install(TARGETS plasma_applet_torcontrol DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)
install(FILES org.kde.peruse.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

plasma_install_package(package org.kde.torcontrol)
