cmake_minimum_required(VERSION 3.19)
project(test1 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools Network)

qt_standard_project_setup()

qt_add_executable(test1
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    logger.h logger.cpp
    basewindow.h basewindow.cpp basewindow.ui
    socketmanager.h socketmanager.cpp
    windowmanager.h windowmanager.cpp
    infouserwindow.h infouserwindow.cpp infouserwindow.ui
    chatwidget.h chatwidget.cpp
    messagewidget.h messagewidget.cpp
    messageinfo.h messageinfo.cpp
    JsonTypes.h
)

qt_add_translations(
    TARGETS test1
    TS_FILES test1_ru_RU.ts
)

target_link_libraries(test1
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

include(GNUInstallDirs)

install(TARGETS test1
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET test1
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
