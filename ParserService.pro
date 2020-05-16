QT -= gui
QT += network webenginewidgets sql

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_DEBUG_OUTPUT
#DEFINES += QUSEPROXY

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        pageparser.cpp \
        parsermain.cpp \
        parserservice.cpp \
        productparser.cpp \
    webpage.cpp

include($$PWD/qtservice/src/qtservice.pri)

# Default rules for deployment.
target.files = ParserService
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    pageparser.h \
    parsermain.h \
    parserservice.h \
    productparser.h \
    parsersettings.h \
    parserrow.h \
    webpage.h \
    linkobject.h \
    productitem.h

RESOURCES += \
    jsparser.qrc

unix:!macx: LIBS += -L$$PWD/other_libs/ -lcurl

INCLUDEPATH += $$PWD/other_libs
DEPENDPATH += $$PWD/other_libs

unix:!macx: PRE_TARGETDEPS += $$PWD/other_libs/libcurl.a
