/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#include "CpuMonitor.h"

using namespace std::chrono_literals;
constexpr int c_monitorCpuIntervalInMs = std::chrono::milliseconds(100ms).count();

/**
 * @brief CpuMonitor::createInstance
 */
CpuMonitor* CpuMonitor::createInstance(QObject* parent)
{
    //if the parent is null, then the instance will be qApp
    //to avoid memory leak, the instance will be destroyed when the parent is destroyed
    if (parent == nullptr)
    {
        parent = qApp;
    }

    //create the owner thread
    QThread* ownerThread = new QThread(parent);
    ownerThread->setObjectName("CpuMonitorThread");

    //create the instance
    CpuMonitor* instance = new CpuMonitor;

    //move the instance to the owner thread
    instance->moveToThread(ownerThread);

    //connect signals and slots
    connect(ownerThread, &QThread::started, instance, &CpuMonitor::start, Qt::DirectConnection);
    connect(parent, &QObject::destroyed, instance, &QObject::deleteLater, Qt::QueuedConnection);
    connect(instance, &QObject::destroyed, ownerThread, &QThread::quit, Qt::QueuedConnection);
    connect(ownerThread, &QThread::finished, ownerThread, &QObject::deleteLater, Qt::DirectConnection);

    //start the owner thread
    ownerThread->start();

    //return the instance
    return instance;
}

/**
 * @brief CpuMonitor::start
 */
void CpuMonitor::start()
{
    //create the timer that will monitor the cpu
    m_timerMonitorCpuPtr = new QTimer(this);
    m_timerMonitorCpuPtr->setInterval(c_monitorCpuIntervalInMs);
    connect(m_timerMonitorCpuPtr, &QTimer::timeout, this, &CpuMonitor::timeoutMonitorCpu);
    m_timerMonitorCpuPtr->start();
}

/**
 * @brief CpuMonitor::timeoutMonitorCpu
 */
void CpuMonitor::timeoutMonitorCpu()
{
    //parse the proc files
    const ProcessCpuList processCpuList = parseProcFiles();

    //loop through the process list
    for (const auto& process : processCpuList)
    {
        auto it = m_processCpuHash.find(process.pid);

        if (it == m_processCpuHash.end())
        {
            //the process is new
            m_processCpuHash.insert(process.pid, process);
        }
        else
        {
            //the process already exists
            CpuProcess& processToUpdate = it.value();

            //update the process
            processToUpdate.minCpuUsageInPercent = (processToUpdate.minCpuUsageInPercent < process.minCpuUsageInPercent ? processToUpdate.minCpuUsageInPercent : process.minCpuUsageInPercent);
            processToUpdate.maxCpuUsageInPercent = (processToUpdate.maxCpuUsageInPercent > process.maxCpuUsageInPercent ? processToUpdate.maxCpuUsageInPercent : process.maxCpuUsageInPercent);
            processToUpdate.cpuUsageInPercent    = process.cpuUsageInPercent;
            processToUpdate.action               = CpuAction::Updated;
        }
    }

    //find the removed processes
    for (auto& process : m_processCpuHash)
    {
        auto it = std::find_if(processCpuList.begin(), processCpuList.end(), [&process](const CpuProcess & p)
        {
            return p.pid == process.pid;
        });

        if (it == processCpuList.end())
        {
            process.action = CpuAction::Removed;
        }
    }

    //emit the signal
    auto it = m_processCpuHash.begin();
    while (it != m_processCpuHash.end())
    {
        const CpuProcess& process = it.value();
        emit updateCpuUsage(process, process.action);

        if (process.action == CpuAction::Removed)
        {
            it = m_processCpuHash.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

/**
 * @brief CpuMonitor::getOnlineCPUCount
 */
int CpuMonitor::getOnlineCPUCount()
{
    // create cpuCount
    int cpuCount = 0x00;

    // create the cpu iterator
    QDirIterator it("/proc/devices/system/cpu", QStringList(),
                    QDir::Dirs | QDir::NoDotDot | QDir::NoDot | QDir::NoDotAndDotDot | QDir::NoSymLinks,
                    QDirIterator::NoIteratorFlags);

    // utils function
    const auto isOnline = [](const QString & name) -> bool
    {
        // get the entry kernel
        QString entryPth = QString("/proc/devices/system/cpu/%1/online").arg(name);

        // check if is online
        QFile file(entryPth);
        if (!file.open(QFile::ReadOnly))
        {
            return true;
        }

        // read buffer
        const QByteArray buffer = file.readAll();

        // close the file
        file.close();

        // return result
        return buffer.isEmpty() || (*buffer.constBegin()) != '0';
    };

    // have next iterator
    while (it.hasNext())
    {
        it.next();
        const QString filePath = it.filePath();

        // is it valid entry?
        if (!filePath.contains("/proc/devices/system/cpu/cpu"))
        {
            continue;
        }

        // read name
        const QString name = filePath.section('/', -1);

        // check if name is valid
        if (name.length() < 4 || !name.startsWith("cpu"))
        {
            continue;
        }

        // increase cpuCount if is online
        if (isOnline(name))
        {
            cpuCount++;
        }
    }

    //log the cpu count
    qDebug() << "CPU count:" << cpuCount;

    // return result
    return cpuCount > 0 ? cpuCount : 1;
}

/**
 * @brief CpuMonitor::parseProcFiles
 */
ProcessCpuList CpuMonitor::parseProcFiles()
{
    //create ret list
    ProcessCpuList ret;

    //scan cpu time
    scanCPUTime();

    //loop "/proc/[pid]/stat" files. PID should be a number
    QDirIterator it("/proc", QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable, QDirIterator::NoIteratorFlags);

    //loop through the directories
    while (it.hasNext())
    {
        //get the next directory
        const QString dir = it.next();

        //get the pid
        const QString pidStr = dir.section('/', -1);

        //check if the pid is a number
        bool ok = false;
        const int pid = pidStr.toInt(&ok);
        if (!ok || pid < 0)
        {
            continue;
        }

        //create the process
        CpuProcess process;

        //set the pid
        process.pid = pid;

        //scan the cpu usage
        if (!scanCPUUsage(process))
        {
            continue;
        }

        //add the process to the list
        ret.push_back(process);
    }

    //return the list
    return ret;
}

/**
 * @brief CpuMonitor::scanCPUTime
 */
void CpuMonitor::scanCPUTime()
{
    // Try to open the CPU meta data
    FILE* file = fopen("/proc/stat", "r");

    if (file == nullptr)
    {
        return;
    }

    auto guard = qScopeGuard([file]()
    {
        fclose(file);
    });

    // Utils function
    const auto saturating = [](quint64 a, quint64 b) -> quint64
    {
        return a > b ? a - b : 0;
    };

    // Create vars
    constexpr int c_bufferSize = 5000;
    constexpr int c_dataSize = 10;

    char buffer[c_bufferSize];
    quint64 data[c_dataSize];

    memset(buffer, 0x00, c_bufferSize);
    memset(data, 0x00, sizeof(data));

    // Read CPU data
    if (fgets(buffer, c_bufferSize, file) == nullptr)
    {
        return;
    }

    // Scan data
    sscanf(buffer, "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu", &data[0], &data[1], &data[2], &data[3], &data[4], &data[5], &data[6], &data[7], &data[8], &data[9]);

    // Update data
    data[0] -= data[8];
    data[1] -= data[9];

    // Calculate total time
    quint64 totaltime = 0x00;

    for (int i = 0; i < c_dataSize; i++)
    {
        totaltime += data[i];
    }

    // Calculate period
    m_globalPeriod = saturating(totaltime, m_globalCpuTime) / c_onlineCpuCount;

    // Update previous
    m_globalCpuTime = totaltime;
}

/**
 * @brief CpuMonitor::scanCPUUsage
 */
bool CpuMonitor::scanCPUUsage(CpuProcess& process)
{
    // Prepare instanceFile
    constexpr int c_instanceFileSize = 100;
    char instanceFile[c_instanceFileSize];
    memset(instanceFile, 0x00, c_instanceFileSize);
    sprintf(instanceFile, "/proc/%d/stat", process.pid);

    // Try to open the instance meta data
    FILE* file = fopen(instanceFile, "r");

    if (file == nullptr)
    {
        return false;
    }

    auto guard = qScopeGuard([file]()
    {
        fclose(file);
    });

    // Read buffer
    constexpr int c_bufferSize = 5000;
    char buffer[c_bufferSize];

    memset(buffer, 0x00, c_bufferSize);

    // Read CPU data
    if (fgets(buffer, c_bufferSize, file) == nullptr)
    {
        return false;
    }

    // Utils functions
    const auto adjust = [](quint64 t) -> quint64
    {
        return t * 100 / sysconf(_SC_CLK_TCK);
    };

    // Create totaltime
    quint64 totaltime = 0x00;

    // Parse buffer
    // Parse PART 1
    char* location = strchr(buffer, ' ');
    if (!location)
    {
        return false;
    }

    // Parse PART 2
    location += 2;
    char* end = strrchr(location, ')');
    if (end == nullptr)
    {
        return false;
    }
    process.command = QString::fromLatin1(location, end - location);

    location = end + 2;

    // Parse PART 3
    location += 2;

    // Parse PART 4
    strtol(location, &location, 10);
    location += 1;

    // Parse PART 5
    strtol(location, &location, 10);
    location += 1;

    // Parse PART 6
    strtol(location, &location, 10);
    location += 1;

    // Parse PART 7
    strtoul(location, &location, 10);
    location += 1;

    // Parse PART 8
    strtol(location, &location, 10);
    location += 1;

    // Parse PART 9
    location = strchr(location, ' ') + 1;

    // Parse PART 10
    strtoull(location, &location, 10);
    location += 1;

    // Parse PART 11
    strtoull(location, &location, 10);
    location += 1;

    // Parse PART 12
    strtoull(location, &location, 10);
    location += 1;

    // Parse PART 13
    strtoull(location, &location, 10);
    location += 1;

    // Parse PART 14
    totaltime += adjust(strtoull(location, &location, 10));
    location += 1;

    // Parse PART 15
    totaltime += adjust(strtoull(location, &location, 10));

    // The period can be 0 after the system is in sleep mode
    int percentCPU = (m_globalPeriod < 1E-6) ? 0.0F : ((totaltime - process.cpuTime) / m_globalPeriod * 100.0);
    percentCPU = std::max(0.0F, std::min<float>(percentCPU, static_cast<float>(c_onlineCpuCount * 100.0F)));
    process.cpuUsageInPercent = percentCPU;

    //update cpuTime
    process.cpuTime = totaltime;

    // return ok
    return true;
}

