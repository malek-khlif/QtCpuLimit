/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#include "CpuLimitManager.h"

/**
 * @brief CpuLimitManager constructor
 */
CpuLimitManager::CpuLimitManager(QObject* parent)
    :
    QObject(parent),
    m_cpuMonitorPtr(CpuMonitor::createInstance(this))
{
    //TODO
}


/**
 * @brief CpuLimitManager::processCount
 */
int CpuLimitManager::processCount() const
{
    return m_processCpuList.size();
}

/**
 * @brief CpuLimitManager::processesModel
 */
CpuLimitModel* CpuLimitManager::processesModel()
{
    return &m_cpuLimitModel;
}
