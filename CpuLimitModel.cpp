/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */


#include "CpuLimitModel.h"

/**
 * @brief CpuLimitModel::CpuLimitModel
 */
CpuLimitModel::CpuLimitModel()
{
    //TODO
}

/**
 * @brief CpuLimitModel::roleNames
 */
QHash<int, QByteArray> CpuLimitModel::roleNames() const
{
    return
    {
        {Pid, "pid"},
        {User, "user"},
        {CpuUsage, "cpuUsage"},
        {CpuLimit, "cpuLimit"},
        {Command, "command"},
    };
}

/**
 * @brief CpuLimitModel::rowCount
 */
int CpuLimitModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 0; //TODO
}

/**
 * @brief CpuLimitModel::columnCount
 */
int CpuLimitModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 5;
}

/**
 * @brief CpuLimitModel::data
 */
QVariant CpuLimitModel::data(const QModelIndex& index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    return QVariant(); //TODO
}
