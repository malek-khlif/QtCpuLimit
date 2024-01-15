/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "CpuLimitManager.h"

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

    //create CpuLimitManager object
    qmlRegisterSingletonType<CpuLimitManager>(
        "CpuLimitManager", 1, 0,
        "CpuLimitManager",
        [](QQmlEngine*, QJSEngine*)
    {
        //owned by the QML engine
        return new CpuLimitManager;
    });

    //create Qt QML engine
    QQmlApplicationEngine engine;

    //load the QML file
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    //exec the Qt Loop Event
    return QGuiApplication::exec();
}
