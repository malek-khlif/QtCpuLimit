/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#ifndef CPULIMITMANAGER_H
#define CPULIMITMANAGER_H

#include <QObject>
#include "CpuMonitor.h"
#include "CpuLimitModel.h"

/**
 * @brief CpuLimitManager class
 */
class CpuLimitManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int processCount READ processCount NOTIFY processCountChanged)
    Q_PROPERTY(CpuLimitModel* processesModel READ processesModel CONSTANT)

public:

    explicit CpuLimitManager(QObject* parent = nullptr);

    int processCount() const;
    CpuLimitModel* processesModel();

signals:

    void processCountChanged();

private:

    CpuMonitor* m_cpuMonitorPtr;
    CpuLimitModel m_cpuLimitModel;
    ProcessCpuList m_processCpuList;
};

#endif // CPULIMITMANAGER_H