QT += core gui widgets

CONFIG += c++11 console
CONFIG -= app_bundle

INCLUDEPATH += /opt/etherlab/include/
DEPENDPATH  += /opt/etherlab/lib/
LIBS        += /opt/etherlab/lib/libethercat.a

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    easycatinterface.cpp \
    ethercatmaster.cpp \
    easycatslave.cpp \
    easycatmaster.cpp

FORMS += \
    easycatinterface.ui

HEADERS += \
    easycatinterface.h \
    ethercatmaster.h \
    easycatslave.h \
    ethercatslave.h \
    easycatmaster.h
