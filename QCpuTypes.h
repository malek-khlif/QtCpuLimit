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
constexpr int c_timerCpuLimitIntervalInMs = std::chrono::milliseconds(25ms).count();

/**
 * @brief QCpuProcess struct
 */
struct QCpuProcess
{
    pid_t pid                          = 0;  // process id
    double cpuUsageInPercent           = 0;  // [0.0..1.0 * (CPU count)]
    quint64 cpuTimeInTicks             = 0;  // CPU time in ticks (jiffies)
    quint64 previousCpuTimeInTicks     = 0;  // CPU time in ticks (jiffies) at previous refresh
    quint64 lastMeasuredTimestampInMs  = 0;  // timestamp of last measurement in ms
    int sleepCountInCycle              = 0;  // number of sleep cycles to limit CPU usage

    std::optional<double> cpuLimitInPercent; // CPU limit in percent (0.0..1.0)

    QString command;                        // command name with arguments
    QString user;                           // user name
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
