/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#ifndef CPUMONITOR_H
#define CPUMONITOR_H

#include <unistd.h>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>
#include <chrono>
#include <QHash>
#include <QList>
#include <QDir>
#include <QDirIterator>
#include <QScopeGuard>
#include <algorithm>

/**
 * @brief CpuAction enum
 */
enum class CpuAction
{
    Added   = 0,
    Removed = 1,
    Updated = 2
};

/**
 * @brief The CpuProcess struct
 */
struct CpuProcess
{
    int pid                  { 0 };
    int minCpuUsageInPercent { 0 };
    int maxCpuUsageInPercent { 0 };
    int cpuUsageInPercent    { 0 };
    int cpuLimitInPercent    { 0 };
    CpuAction action         { CpuAction::Added };

    quint64 cpuTime          { 0 };

    QString user;
    QString command;
};

/*alias*/
using ProcessPid = int;
using ProcessCpuHash = QHash<ProcessPid, CpuProcess>;
using ProcessCpuList = QList<CpuProcess>;

/**
 * @brief The CpuMonitor class
 */
class CpuMonitor final : public QObject
{
    Q_OBJECT

public:

    static CpuMonitor* createInstance(QObject* parent = nullptr);

signals:

    void updateCpuUsage(CpuProcess process, CpuAction action);

private:

    explicit CpuMonitor() = default;

    void start();
    void timeoutMonitorCpu();
    static int getOnlineCPUCount();
    ProcessCpuList parseProcFiles();
    void scanCPUTime();
    bool scanCPUUsage(CpuProcess& process);

    const int c_onlineCpuCount{getOnlineCPUCount()};

    double m_globalPeriod = 0.;
    quint64 m_globalCpuTime = 0x00;

    ProcessCpuHash m_processCpuHash;
    QTimer* m_timerMonitorCpuPtr { nullptr };
};

#endif // CPUMONITOR_H

