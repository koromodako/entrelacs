#-------------------------------------------------
#
# Project created by QtCreator 2016-12-26T17:31:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EntrelacsGen
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    graph.cpp \
    graphdrawer.cpp

HEADERS  += mainwindow.h \
    graph.h \
    graphdrawer.h

FORMS    += mainwindow.ui

DISTFILES += \
    test/trinode.grp \
    test/binode.grp \
    test/cube.grp \
    test/triangle.grp \
    test/tristar.grp \
    test/quadstar.grp \
    test/square.grp
