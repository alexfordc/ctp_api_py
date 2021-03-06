######################################################################
# Automatically generated by qmake (3.0) Fri Apr 22 14:06:12 2016
######################################################################
TARGET = md_utils

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
        DESTDIR = ../../../build/debug
} else {
        DEFINES += NDEBUG
        DESTDIR = ../../../build/release
}

QMAKE_CFLAGS += -std=c99
QMAKE_CXXFLAGS += -std=c++0x -fPIC -Wno-unused-parameter -fvisibility=hidden -fvisibility-inlines-hidden

DEFINES += __STDC_FORMAT_MACROS


INCLUDEPATH += ..

LIBS += -lpthread -lrt

# Input
HEADERS += bar_generator.h \
    contract_generator.h \
    md_utils.h

SOURCES += bar_generator.c \
    contract_generator.c \
    md_utils.c

