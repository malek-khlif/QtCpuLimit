/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#include "QCpuModel.h"

/**
 * @brief QCpuModel::QCpuModel
 */
QCpuModel::QCpuModel()
{
    //create the monitor in its own thread
    m_cpuMonitorPtr = QCpuMonitor::create();

    //connect the monitor to the model
    connect(m_cpuMonitorPtr,
            &QCpuMonitor::updateProcessList,
            this,
            &QCpuModel::updateProcessList,
            Qt::QueuedConnection);
}

/**
 * @brief QCpuModel::selectProcess
 */
void QCpuModel::selectProcess(int index)
{
    //check the index
    if (index < 0 || index >= m_processList.size())
    {
        return;
    }

    //update the selected process
    m_selectedProcessPid      = m_processList[index].pid;
    m_selectedProcessCpuLimit = m_processList[index].cpuLimitInPercent.value_or(-1);
    m_selectedProcessCommand  = m_processList[index].command;

    //emit the signals
    emit selectedProcessPidChanged();
    emit selectedProcessCpuLimitChanged();
    emit selectedProcessCommandChanged();
}

/**
 * @brief QCpuModel::setProcessLimit
 */
void QCpuModel::setProcessLimit(int cpuLimit)
{
    //any selected PID ?
    if (m_selectedProcessPid <= 0)
    {
        return;
    }

    //set the process limit
    QMetaObject::invokeMethod(m_cpuMonitorPtr,
                              "setProcessLimit",
                              Qt::QueuedConnection,
                              Q_ARG(pid_t, m_selectedProcessPid),
                              Q_ARG(int, cpuLimit));
}

/**
 * @brief QCpuModel::removeProcessLimit
 */
void QCpuModel::removeProcessLimit()
{
    //any selected PID ?
    if (m_selectedProcessPid <= 0)
    {
        return;
    }

    //remove the process limit
    QMetaObject::invokeMethod(m_cpuMonitorPtr,
                              "removeProcessLimit",
                              Qt::QueuedConnection,
                              Q_ARG(pid_t, m_selectedProcessPid));
}

/**
 * @brief QCpuModel::roleNames
 */
QHash<int, QByteArray> QCpuModel::roleNames() const
{
    return
    {
        {Pid,       "pid"},
        {User,      "user"},
        {CpuUsage,  "cpuUsage"},
        {CpuLimit,  "cpuLimit"},
        {Command,   "command"},
    };
}

/**
 * @brief QCpuModel::rowCount
 */
int QCpuModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_processList.size();
}

/**
 * @brief QCpuModel::columnCount
 */
int QCpuModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 5;
}

/**
 * @brief QCpuModel::data
 */
QVariant QCpuModel::data(const QModelIndex& index, int role) const
{
    //check if the index is valid
    if (!index.isValid())
    {
        return QVariant();
    }

    //check if the index is out of range
    if (index.row() >= m_processList.size() || index.row() < 0)
    {
        return QVariant();
    }

    //return the data according to the role
    switch (role)
    {
        case Pid:
            return m_processList[index.row()].pid;

        case User:
            return m_processList[index.row()].user;

        case CpuUsage:
            return QString::number(m_processList[index.row()].cpuUsageInPercent * 100, 'f', 2);

        case CpuLimit:
        {
            const auto& cpuLimit = m_processList[index.row()].cpuLimitInPercent;
            return cpuLimit.has_value() ? QString::number(cpuLimit.value() * 100, 'f', 2) : "N/A";
        }

        case Command:
            return m_processList[index.row()].command;
    }

    //return an invalid variant
    return QVariant();
}

/**
 * @brief QCpuModel::processCount
 */
int QCpuModel::processCount() const
{
    return m_processList.size();
}

/**
 * @brief QCpuModel::selectedProcessPid
 */
int QCpuModel::selectedProcessPid() const
{
    return m_selectedProcessPid;
}

/**
 * @brief QCpuModel::selectedProcessCpuLimit
 */
int QCpuModel::selectedProcessCpuLimit() const
{
    return m_selectedProcessCpuLimit;
}

/**
 * @brief QCpuModel::selectedProcessCommand
 */
QString QCpuModel::selectedProcessCommand() const
{
    return m_selectedProcessCommand;
}

/**
 * @brief QCpuModel::updateProcessList
 */
void QCpuModel::updateProcessList(const QCpuProcessList& processList,
                                  const PidList& processToAdd,
                                  const PidList& processToRemove)
{
    //the process count changed ?
    const bool countChanged = m_processList.size() != processList.size();

    //treat the case of the first update
    if (Q_UNLIKELY(m_processList.empty() && !processList.empty()))
    {
        beginResetModel();
        m_processList = processList;
        endResetModel();
        emit processCountChanged();
        return;
    }

    //treat the case of process removal
    std::for_each(processToRemove.cbegin(),
                  processToRemove.cend(),
                  [this](pid_t pid)
    {
        const auto it = std::find_if(m_processList.begin(),
                                     m_processList.end(),
                                     [pid](const QCpuProcess & process)
        {
            return process.pid == pid;
        });

        if (it != m_processList.end())
        {
            const auto index = std::distance(m_processList.begin(), it);
            beginRemoveRows(QModelIndex(), index, index);
            m_processList.erase(it);
            endRemoveRows();
        }
    });

    //treat the case of process addition
    std::for_each(processToAdd.cbegin(),
                  processToAdd.cend(),
                  [this, &processList](pid_t pid)
    {
        const auto it = std::find_if(processList.begin(),
                                     processList.end(),
                                     [pid](const QCpuProcess & process)
        {
            return process.pid == pid;
        });

        if (it != processList.end())
        {
            beginInsertRows(QModelIndex(), m_processList.size(), m_processList.size());
            m_processList.push_back(*it);
            endInsertRows();
        }
    });

    //update the CpuUsage and CpuLimit columns
    int index = 0;
    std::for_each(m_processList.begin(),
                  m_processList.end(),
                  [this, &index, &processList](QCpuProcess & process)
    {
        const auto it = std::find_if(processList.begin(),
                                     processList.end(),
                                     [&process](const QCpuProcess & p)
        {
            return p.pid == process.pid;
        });

        if (it != processList.end())
        {
            process.cpuUsageInPercent = it->cpuUsageInPercent;
            process.cpuLimitInPercent = it->cpuLimitInPercent;
        }

        emit dataChanged(createIndex(index, 2),
                         createIndex(index, 3), {CpuUsage, CpuLimit});

        ++index;
    });

    //emit the process count changed signal
    if (countChanged)
    {
        emit processCountChanged();
    }
}
