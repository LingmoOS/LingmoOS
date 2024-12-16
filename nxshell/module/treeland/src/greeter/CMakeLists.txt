set(GREETER_SOURCES
    GreeterApp.h
    GreeterProxy.h
    SessionModel.h
    UserModel.h
    User.h
    LogoProvider.h
    GreeterApp.cpp
    GreeterProxy.cpp
    SessionModel.cpp
    UserModel.cpp
    User.cpp
    LogoProvider.cpp
    greeter_assets.qrc
)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Dtk6 REQUIRED COMPONENTS SystemSettings Declarative)
find_package(DDM REQUIRED COMPONENTS Auth)

set_source_files_properties("${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.DisplayManager.xml" PROPERTIES
    NO_NAMESPACE ON
    CLASSNAME DisplayManager
)
set_source_files_properties("${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.DisplayManager.Seat.xml" PROPERTIES
    NO_NAMESPACE ON
    CLASSNAME DisplaySeat
)

set_source_files_properties("${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.DisplayManager.Session.xml" PROPERTIES
    NO_NAMESPACE ON
    CLASSNAME DisplaySession
)

qt_add_dbus_interface(DDM_DBUS_SOURCES "${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.DisplayManager.xml"          DisplayManager)
qt_add_dbus_interface(DDM_DBUS_SOURCES "${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.DisplayManager.Seat.xml"     DisplayManagerSeat)
qt_add_dbus_interface(DDM_DBUS_SOURCES "${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.DisplayManager.Session.xml"  DisplayManagerSession)

set_source_files_properties(GreeterModel.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

set(QML_FILES
    Greeter.qml
    UserList.qml
    SessionList.qml
    UserInput.qml
    GreeterModel.qml
    Center.qml
    LoginAnimation.qml
    RoundBlur.qml
    PowerList.qml
    TimeDateWidget.qml
    HintLabel.qml
)

qt_add_qml_module(greeterplugin
    URI TreeLand.Greeter
    STATIC
    VERSION 1.0
    PLUGIN_TARGET greeterplugin
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    CLASS_NAME
        GreeterExtensionPlugin
    SOURCES
        ${GREETER_SOURCES}
        ${DDM_DBUS_SOURCES}
    RESOURCE_PREFIX
        /qt/qml
    QML_FILES ${QML_FILES}
    OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/qt/qml/TreeLand/Greeter
)

set(TRANSLATED_FILES)
if ((${QT_VERSION_MAJOR} EQUAL 6) AND (${QT_VERSION_MINOR} LESS 7))
    qt_add_lupdate(greeterplugin
        TS_FILES
            translations/greeter.zh_CN.ts
            translations/greeter.en_US.ts
        NO_GLOBAL_TARGET
    )

    qt_add_lrelease(greeterplugin
        TS_FILES
            translations/greeter.zh_CN.ts
            translations/greeter.en_US.ts
        QM_FILES_OUTPUT_VARIABLE TRANSLATED_FILES
    )
elseif ((${QT_VERSION_MAJOR} EQUAL 6) AND (${QT_VERSION_MINOR} GREATER_EQUAL 7))
    qt_add_lupdate(
        SOURCE_TARGETS greeterplugin
        TS_FILES
            translations/greeter.zh_CN.ts
            translations/greeter.en_US.ts
        NO_GLOBAL_TARGET
    )

    qt_add_lrelease(
        TS_FILES
            translations/greeter.zh_CN.ts
            translations/greeter.en_US.ts
        QM_FILES_OUTPUT_VARIABLE TRANSLATED_FILES
    )
endif()

install(FILES ${TRANSLATED_FILES} DESTINATION ${COMPONENTS_TRANSLATION_DIR})

target_include_directories(greeterplugin
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/greeter/
)

target_link_libraries(greeterplugin
    PRIVATE
        DDM::Auth
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Qml
        Dtk6::Declarative
        Dtk6::SystemSettings
)

if(JOURNALD_FOUND)
    target_link_libraries(greeterplugin PUBLIC ${JOURNALD_LIBRARIES})
endif()
