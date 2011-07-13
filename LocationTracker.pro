QT = core network
CONFIG += mobility
MOBILITY = location
LIBS += -lqjson

{
    target.path = /opt/LocationTracker/bin
    INSTALLS += target
}

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog

HEADERS += \
    Tracker.h

SOURCES += \
    Tracker.cpp \
    main.cpp
