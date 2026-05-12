QT += widgets

TARGET = tetris
TEMPLATE = app

CONFIG += c++17

SOURCES += \
    main.cpp \
    aiplayer.cpp \
    dualgame.cpp \
    menuscreen.cpp \
    tetrisboard.cpp \
    tetriswindow.cpp \
    vsaigame.cpp

HEADERS += \
    aiplayer.h \
    dualgame.h \
    menuscreen.h \
    tetrisboard.h \
    tetriswindow.h \
    vsaigame.h
