/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "QCpuModel.h"

/**
 * @brief main function
 */
int main(int argc, char** argv)
{
    //enable Qt parameters
    QGuiApplication::setApplicationDisplayName("Qt CPU Limit");
    QGuiApplication::setApplicationName("Qt CPU Limit");
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);

    //create Qt GUI application
    QGuiApplication app(argc, argv);

    //register meta type
    qRegisterMetaType<QCpuProcessList>("QCpuProcessList");
    qRegisterMetaType<PidList>("PidList");
    qRegisterMetaType<QCpuProcess>("QCpuProcess");
    qRegisterMetaType<pid_t>("pid_t");

    //create QCpuModel object
    qmlRegisterSingletonType<QCpuModel>(
        "QCpuModel", 1, 0,
        "QCpuModel",
        [](QQmlEngine*, QJSEngine*)
    {
        //owned by the QML engine
        return new QCpuModel;
    });

    //create Qt QML engine
    QQmlApplicationEngine engine;

    //load the QML file
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    //exec the Qt Loop Event
    return QGuiApplication::exec();
}
