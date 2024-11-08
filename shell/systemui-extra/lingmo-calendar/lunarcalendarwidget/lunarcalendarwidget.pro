#-------------------------------------------------
#
# Project created by QtCreator 2017-01-05T22:11:54
#
#-------------------------------------------------

QT       += core gui sql svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets KWindowSystem dbus network

TARGET          = lunarcalendarwidget
TEMPLATE        = app
QMAKE_CXXFLAGS += -std=c++14
MOC_DIR         = temp/moc
RCC_DIR         = temp/rcc
UI_DIR          = temp/ui
OBJECTS_DIR     = temp/obj
DESTDIR         = $$PWD/../bin
CONFIG          += qt warn_off
RESOURCES       += main.qrc

CONFIG += \
     link_pkgconfig \

PKGCONFIG += gsettings-qt lingmosdk-waylandhelper lingmosdk-systime lingmosdk-qtwidgets
PKGCONFIG += gsettings-qt lingmosdk-kabase
LIBS +=  -L/usr/lib/lingmosdk/lingmosdk-base  -lkylog -lkyconf
INCLUDEPATH += /usr/include/kabase/
INCLUDEPATH += /usr/include/lingmosdk/applications/kabase/
INCLUDEPATH += /usr/include/lingmosdk/applications/kabase/lingmo_system/
INCLUDEPATH += /usr/include/kabase/lingmo_system/
INCLUDEPATH += kabase/
INCLUDEPATH += kabase/Qt
INCLUDEPATH += /path/to/kdk

SOURCES         += main.cpp \
    colorarea.cpp \
#    custommessagebox.cpp \
    custommessagebox.cpp \
    customscrollarea.cpp \
    datetimeedit.cpp \
    morelabel.cpp \
    notificationsadaptor.cpp \
    schedule_item.cpp
SOURCES         += frmlunarcalendarwidget.cpp
SOURCES         += lunarcalendarinfo.cpp
SOURCES         += lunarcalendaritem.cpp
SOURCES         += lunarcalendaryearitem.cpp
SOURCES         += lunarcalendarmonthitem.cpp
SOURCES         += lunarcalendarwidget.cpp
SOURCES         += picturetowhite.cpp
SOURCES         += customstylePushbutton.cpp
SOURCES         += calendarcolor.cpp
SOURCES         += schedulewidget.cpp
SOURCES	        += schedulemark.cpp
SOURCES         += mylabel.cpp
SOURCES         += calendardatabase.cpp
SOURCES         += calendarinfo.cpp
SOURCES         += switchbutton.cpp
SOURCES         += customcalendar.cpp

HEADERS         += frmlunarcalendarwidget.h \
    colorarea.h \
#    custommessagebox.h \
    custommessagebox.h \
    customscrollarea.h \
    datetimeedit.h \
    morelabel.h \
    notificationsadaptor.h \
    schedule_item.h
HEADERS         += lunarcalendarinfo.h
HEADERS         += lunarcalendaritem.h
HEADERS         += lunarcalendaryearitem.h
HEADERS         += lunarcalendarmonthitem.h
HEADERS         += lunarcalendarwidget.h
HEADERS         += picturetowhite.h
HEADERS         += customstylePushbutton.h
HEADERS         += calendarcolor.h
HEADERS         += schedulewidget.h
HEADERS         += schedulemark.h
HEADERS         += schedulestruct.h
HEADERS         += mylabel.h
HEADERS         += calendarinfo.h
HEADERS         += calendardatabase.h
HEADERS         += switchbutton.h
HEADERS         += customcalendar.h

FORMS           += frmlunarcalendarwidget.ui

INCLUDEPATH     += $$PWD

DEFINES += LINGMO_SDK_DATE
PKGCONFIG += lingmosdk-systime
