QT += gui network opengl sql xml widgets printsupport

CONFIG += c++17

TEMPLATE = app
TARGET = GsDocEdit
DEFINES += FORMS_HAS_UI_PREFIX 

OBJECTS += gsdocedit64.res

SOURCES += \
    codeeditor.cpp \
    gsdocedit.cpp \
    main.cpp \
    replacedata_dialog.cpp

HEADERS += \
    codeeditor.h \
    gsdocedit.h \
    replacedata_dialog.h

INCLUDEPATH += src

# Qt resources
RESOURCES += resources.qrc

# Qt translation files
TRANSLATIONS += hu.ts

# #######################################################################
# External stuffs
# #######################################################################

include(../gSAFE/gsafe.pri)
