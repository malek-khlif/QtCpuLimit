/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#ifndef QCPUTYPES_H
#define QCPUTYPES_H

#include <QList>
#include <QString>
#include <QMap>
#include <unistd.h>
#include <optional>
#include <chrono>

/**
 * @brief c_timerRefreshProcessListIntervalInMs constant
 */
using namespace std::chrono_literals;
constexpr int c_timerRefreshProcessListIntervalInMs = std::chrono::milliseconds(1s).count();

/**
 * @brief c_timerCpuLimitIntervalInMs constant
 */
constexpr int c_timerCpuLimitIntervalInMs = std::chrono::milliseconds(50ms).count();

/**
 * @brief QCpuProcess struct
 */
struct QCpuProcess
{
    pid_t pid                       = 0;
    double cpuUsageInPercent        = 0;
    quint64 cpuTimeInTicks          = 0;
    quint64 previousCpuTimeInTicks  = 0;
    int sleepCountInCycle           = 0;

    std::optional<int> cpuLimitInPercent;

    QString command;
    QString user;
};

/**
 * @brief QCpuProcessList
 */
using QCpuProcessList = QList<QCpuProcess>;

/**
 * @brief QUserMap
 */
using UserID    = int;
using UserName  = QString;
using QUserMap  = QMap<UserID, UserName>;

/**
 * @brief PidList
 */
using PidList = QList<pid_t>;

#endif // QCPUTYPES_H
