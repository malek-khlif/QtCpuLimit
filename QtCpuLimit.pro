#############################################################
#                                                           #
#                      Qt CPU LIMIT                         #
#                                                           #
#  Author: Malek Khlif <malek.khlif@outlook.com>            #
#                                                           #
#############################################################

QT = core gui quick qml

TEMPLATE = app

TARGET = QtCpuLimit

QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -Wextra
QMAKE_CXXFLAGS += -Werror
CONFIG += c++17
QMAKE_CFLAGS += -std=c11

HEADERS += \
    QCpuTypes.h \
    QCpuModel.h \
    QCpuMonitor.h

SOURCES += \
    main.cpp \
    QCpuModel.cpp \
    QCpuMonitor.cpp

RESOURCES += \
    qml.qrc
