/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#ifndef QCPUMODEL_H
#define QCPUMODEL_H

#include <QAbstractTableModel>
#include <QMetaObject>
#include "QCpuMonitor.h"

/**
 * @brief QCpuModel class
 */
class QCpuModel final : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(int processCount READ processCount NOTIFY processCountChanged)
    Q_PROPERTY(int selectedProcessPid READ selectedProcessPid NOTIFY selectedProcessPidChanged)
    Q_PROPERTY(int selectedProcessCpuLimit READ selectedProcessCpuLimit NOTIFY selectedProcessCpuLimitChanged)
    Q_PROPERTY(QString selectedProcessCommand READ selectedProcessCommand NOTIFY selectedProcessCommandChanged)

public:

    enum CpuModelRoles
    {
        Pid =  Qt::UserRole + 1,
        User,
        CpuUsage,
        CpuLimit,
        Command,
    };

    explicit QCpuModel();

    Q_INVOKABLE void selectProcess(int index);
    Q_INVOKABLE void setProcessLimit(int cpuLimit);
    Q_INVOKABLE void removeProcessLimit();

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    int processCount() const;
    int selectedProcessPid() const;
    int selectedProcessCpuLimit() const;
    QString selectedProcessCommand() const;

signals:

    void processCountChanged();
    void selectedProcessPidChanged();
    void selectedProcessCpuLimitChanged();
    void selectedProcessCommandChanged();

private:

    void updateProcessList(const QCpuProcessList& processList,
                           const PidList& processToAdd,
                           const PidList& processToRemove);

    int m_selectedProcessPid { -1 };
    int m_selectedProcessCpuLimit { -1 };
    QString m_selectedProcessCommand;

    QCpuProcessList m_processList;
    QCpuMonitor* m_cpuMonitorPtr { nullptr };
};

#endif // QCPUMODEL_H
