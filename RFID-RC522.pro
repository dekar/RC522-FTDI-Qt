#-------------------------------------------------
#
# Project created by QtCreator 2015-08-08T16:15:31
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RFID-RC522
TEMPLATE = app
LIBS += -lusb \
    -lftdi
unix:LIBS += -ludev


SOURCES += src/main.cpp\
        src/gui.cpp \
    src/qtrc522.cpp \
    src/MFRC522.cpp \
    src/ftdi/ftspi.cpp

HEADERS  += src/gui.h \
    src/qtrc522.h \
    src/MFRC522.h \
    src/ftdi/ftspi.h

FORMS    += src/gui.ui
UI_DIR = build/moc
OBJECTS_DIR = build/obj
DESTDIR = build/bin
MOC_DIR = build/moc
