######################################################################
# Automatically generated by qmake (3.0) Fri Apr 22 14:06:12 2016
######################################################################
TARGET = xcmd

build_pass:CONFIG(debug, debug|release) {
        BuildConf = debug
        TARGET = $$join(TARGET,,,_debug)
        BuildSuff = _debug
} else {
        BuildConf = release
        BuildSuff = ""
}


TCONFIG -= qt
QT      -= core gui

CONFIG += debug_and_release plugin
CONFIG += object_parallel_to_source

TEMPLATE = lib



QMAKE_CXXFLAGS += -static-libstdc++
QMAKE_LFLAGS += -static-libstdc++

build_pass:CONFIG(debug, debug|release) {
        DEFINES += DEBUG _DEBUG
        DESTDIR = ../../build/debug
} else {
        DEFINES += NDEBUG
        DESTDIR = ../../build/release
}

QMAKE_CFLAGS += -std=c99
QMAKE_CXXFLAGS += -std=c++0x -fPIC -Wno-unused-parameter -fvisibility=hidden -fvisibility-inlines-hidden

DEFINES += __STDC_FORMAT_MACROS
# DEFINES += HAVE_WRPPAER_PYTHON


INCLUDEPATH += ..
INCLUDEPATH += ../api
INCLUDEPATH += ../../thirdparty
# INCLUDEPATH += ../../thirdparty/python35/linux_include
INCLUDEPATH += ../../thirdparty/python36/linux_include

LIBS += -lpthread -lrt

# LIBS += -L../../thirdparty/python35/linux64 -lpython3.5m
LIBS += -L../../thirdparty/python36/linux64 -lpython3.6m
LIBS += -L../api/XcMarketApi/linux -lXcMarketApi
LIBS += -L$$DESTDIR -lmd_utils


# Input
HEADERS += xcmd.h \
    ../api/XcTradeApi/Action_Define.h \
    ../api/XcTradeApi/Action_Struct.h \
    ../api/XcTradeApi/XcMarketApi.h

SOURCES += xcmd.cpp

