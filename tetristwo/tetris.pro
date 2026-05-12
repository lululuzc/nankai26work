QT += widgets

TARGET = tetris
TEMPLATE = app

CONFIG += c++17

SOURCES += \
    main.cpp \
    dualgame.cpp \
    menuscreen.cpp \
    tetrisboard.cpp \
    tetriswindow.cpp

HEADERS += \
    dualgame.h \
    menuscreen.h \
    tetrisboard.h \
    tetriswindow.h
