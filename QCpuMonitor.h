/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#ifndef QCPUMONITOR_H
#define QCPUMONITOR_H

#include <QCoreApplication>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QScopeGuard>
#include <QDateTime>
#include <QDirIterator>
#include <exception>
#include <stdexcept>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/sysinfo.h>
#include "QCpuTypes.h"

/**
 * @brief QCpuMonitor class
 */
class QCpuMonitor final : public QObject
{
    Q_OBJECT

public:

    static QCpuMonitor* create();

    ~QCpuMonitor() noexcept override;

public slots:

    void setProcessLimit(pid_t pid, int cpuLimit);
    void removeProcessLimit(pid_t pid);

signals:

    void updateProcessList(const QCpuProcessList processList,
                           const PidList processToAdd,
                           const PidList processToRemove); //This signal is cross-thread, don't use references

private:

    explicit QCpuMonitor() = default;

    void start() noexcept;
    void scanUsers() noexcept;
    void scanRunningProcesses() noexcept;
    void scanProcessCpuTime(quint64 now, QCpuProcess& process) noexcept;
    void timeoutControlCpuLimit() noexcept;
    void timeoutCpuMonitor() noexcept;

    QCpuProcessList m_processList;
    QUserMap m_userMap;
    QTimer* m_timerMonitorCpuPtr { nullptr };
    QTimer* m_timerLimitCpuPtr   { nullptr };
};

#endif // QCPUMONITOR_H
