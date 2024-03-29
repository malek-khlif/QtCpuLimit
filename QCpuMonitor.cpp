/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#include "QCpuMonitor.h"

# if defined(_SC_CLK_TCK)
#  define HZ ((double)sysconf(_SC_CLK_TCK))
# else
#  ifndef CLK_TCK
#    define HZ  100.0
#  else
#   define HZ ((double)CLK_TCK)
#  endif
# endif

/**
 * @brief QCpuMonitor::create
 */
QCpuMonitor* QCpuMonitor::create()
{
    //create the instance
    QCpuMonitor* instancePtr = new QCpuMonitor;

    //create the owner thread
    QThread* ownerThreadPtr = new QThread;

    //give a name to the owner thread (for debugging purposes)
    ownerThreadPtr->setObjectName("QCpuMonitor");

    //move the instance to the owner thread
    instancePtr->moveToThread(ownerThreadPtr);

    //setup the lifetime tree
    connect(ownerThreadPtr, &QThread::started, instancePtr, &QCpuMonitor::start, Qt::DirectConnection);
    connect(qApp, &QCoreApplication::aboutToQuit, instancePtr, &QObject::deleteLater, Qt::QueuedConnection);
    connect(instancePtr, &QObject::destroyed, ownerThreadPtr, &QThread::quit, Qt::QueuedConnection);
    connect(ownerThreadPtr, &QThread::finished, ownerThreadPtr, &QObject::deleteLater, Qt::DirectConnection);

    //start the owner thread
    ownerThreadPtr->start();

    //return the instance
    return instancePtr;
}

/**
 * @brief QCpuMonitor::~QCpuMonitor
 */
QCpuMonitor::~QCpuMonitor() noexcept
{
    //the current process id
    const pid_t currentProcessId = getpid();

    //send a SIGCONT signal to all processes
    std::for_each(m_processList.constBegin(),
                  m_processList.constEnd(),
                  [currentProcessId](const QCpuProcess & process)
    {
        //send a SIGCONT signal except for the current process
        if (process.pid != currentProcessId)
        {
            kill(process.pid, SIGCONT);
        }
    });
}

/**
 * @brief QCpuMonitor::setProcessLimit
 */
void QCpuMonitor::setProcessLimit(pid_t pid, int cpuLimit)
{
    //check if the method is called from the owner thread
    Q_ASSERT_X(QThread::currentThread() == thread(),
               "QCpuMonitor::setProcessLimit",
               "This method must be called from the owner thread");

    //ignore the action if the pid is the current process
    if (pid == getpid())
    {
        qDebug() << "QCpuMonitor::setProcessLimit: cannot set a cpu limit for the current process";
        return;
    }

    //check if the cpu limit is valid
    if (cpuLimit < 0 || cpuLimit > 100)
    {
        qDebug() << "QCpuMonitor::setProcessLimit: invalid cpu limit - cpuLimit:" << cpuLimit;
        return;
    }

    //find the process
    auto processIt = std::find_if(m_processList.begin(), m_processList.end(), [pid](const QCpuProcess & process)
    {
        return process.pid == pid;
    });

    //check if the process is found
    if (processIt == m_processList.end())
    {
        qDebug() << "QCpuMonitor::setProcessLimit: process not found - pid:" << pid;
        return;
    }

    //send a SIGCONT signal to the process
    kill(processIt->pid, SIGCONT);

    //set the cpu limit
    processIt->cpuLimitInPercent = static_cast<double>(cpuLimit) / 100.0;
    processIt->sleepCountInCycle = 0;
}

/**
 * @brief QCpuMonitor::removeProcessLimit
 */
void QCpuMonitor::removeProcessLimit(pid_t pid)
{
    //check if the method is called from the owner thread
    Q_ASSERT_X(QThread::currentThread() == thread(),
               "QCpuMonitor::removeProcessLimit",
               "This method must be called from the owner thread");

    //ignore the action if the pid is the current process
    if (pid == getpid())
    {
        qDebug() << "QCpuMonitor::removeProcessLimit: cannot remove the cpu limit for the current process";
        return;
    }

    //find the process
    auto processIt = std::find_if(m_processList.begin(), m_processList.end(), [pid](const QCpuProcess & process)
    {
        return process.pid == pid;
    });

    //check if the process is found
    if (processIt == m_processList.end())
    {
        qDebug() << "QCpuMonitor::removeProcessLimit: process not found - pid:" << pid;
        return;
    }

    //send a SIGCONT signal to the process
    kill(processIt->pid, SIGCONT);

    //remove the cpu limit
    processIt->cpuLimitInPercent.reset();
    processIt->sleepCountInCycle = 0;
}

/**
 * @brief QCpuMonitor::start
 */
void QCpuMonitor::start() noexcept
{
    //check if the method is called from the owner thread
    Q_ASSERT_X(QThread::currentThread() == thread(),
               "QCpuMonitor::start",
               "This method must be called from the owner thread");

    //scan the users
    scanUsers();

    //create the m_timerMonitorCpuPtr timer
    m_timerMonitorCpuPtr = new QTimer(this);
    m_timerMonitorCpuPtr->setInterval(c_timerRefreshProcessListIntervalInMs);
    m_timerMonitorCpuPtr->setTimerType(Qt::PreciseTimer);
    m_timerMonitorCpuPtr->setSingleShot(true);
    connect(m_timerMonitorCpuPtr, &QTimer::timeout, this, &QCpuMonitor::timeoutCpuMonitor);
    m_timerMonitorCpuPtr->start();

    //create the m_timerLimitCpuPtr timer
    m_timerLimitCpuPtr = new QTimer(this);
    m_timerLimitCpuPtr->setInterval(c_timerCpuLimitIntervalInMs);
    m_timerLimitCpuPtr->setTimerType(Qt::PreciseTimer);
    m_timerLimitCpuPtr->setSingleShot(true);
    connect(m_timerLimitCpuPtr, &QTimer::timeout, this, &QCpuMonitor::timeoutControlCpuLimit);
    m_timerLimitCpuPtr->start();
}

/**
 * @brief QCpuMonitor::scanUsers
 */
void QCpuMonitor::scanUsers() noexcept
{
    //the password file path
    const QString passwordFilePath = "/etc/passwd";

    //create the password file object
    QFile passwordFile(passwordFilePath);

    //try to open the password file
    if (!passwordFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "QCpuMonitor::scanUsers: cannot open the password file";
        return;
    }

    //create the text stream
    QTextStream textStream(&passwordFile);

    //loop until the end of the file
    while (!textStream.atEnd())
    {
        //read the line
        const QString line = textStream.readLine();

        //split the line
        const QStringList splitList = line.split(':');

        //check if the line is valid
        if (splitList.size() < 3)
        {
            continue;
        }

        //get the user id
        bool ok = false;
        const UserID userId = splitList[2].toInt(&ok);
        if (!ok || userId < 0)
        {
            continue;
        }

        //get the user name
        const UserName userName = splitList[0];

        //check if the name is empty
        if (userName.isEmpty())
        {
            continue;
        }

        //add the user to the map
        m_userMap.insert(userId, userName);
    }

    //close the password file
    passwordFile.close();
}

/**
 * @brief QCpuMonitor::scanRunningProcesses
 */
void QCpuMonitor::scanRunningProcesses() noexcept
{
    //utility function to return all running processes
    auto getRunningProcesses = [this]() -> PidList
    {
        //create the list
        PidList ret;

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

            //add the pid to the list
            ret.push_back(pid);
        }

        //return the list
        return ret;
    };

    //utility function to read command and user
    auto readCommandAndUser = [this](QCpuProcess & process)
    {
        //the status file path
        const QString statusFilePath = QString("/proc/%1/status").arg(process.pid);

        //create the status file object
        QFile statusFile(statusFilePath);

        //try to open the status file
        if (!statusFile.open(QFile::ReadOnly))
        {
            qDebug() << "QCpuMonitor::scanRunningProcesses: cannot open the status file - pid:" << process.pid;
            return;
        }

        //read the status file content
        const QString statusFileContent = statusFile.readAll();

        //close the status file
        statusFile.close();

        //split the content by line
        const QStringList statusFileContentList = statusFileContent.split('\n');

        //don't go through all the keys
        bool nameSet = false;
        bool uidSet  = false;

        //loop through the lines
        for (const QString& line : statusFileContentList)
        {
            //split the line
            const QStringList splitList = line.split(':');

            //check if the line is valid
            if (splitList.size() < 2)
            {
                continue;
            }

            //get the key
            const QString key = splitList[0];

            //get the value
            const QString value = splitList[1].trimmed();

            //check if the key is valid
            if (key == "Name")
            {
                process.command = value;
                nameSet = true;
            }
            else if (key == "Uid")
            {
                //get the user id
                bool ok = false;
                const int userId = value.section('\t', 0, 0).toInt(&ok);
                if (!ok || userId < 0)
                {
                    continue;
                }

                //find the user
                auto userIt = m_userMap.constFind(userId);
                if (userIt != m_userMap.constEnd())
                {
                    process.user = userIt.value();
                }

                uidSet = true;
            }

            if (nameSet && uidSet)
            {
                break;
            }
        }
    };

    //get all running processes
    const PidList runningProcesses = getRunningProcesses();

    //create the lists
    PidList processToAdd;
    PidList processToRemove;

    //remove all processes that are not running anymore
    std::remove_if(m_processList.begin(), m_processList.end(), [&runningProcesses, &processToRemove](const QCpuProcess & process)
    {
        const bool toRemove = std::find(runningProcesses.constBegin(), runningProcesses.constEnd(), process.pid) == runningProcesses.constEnd();

        if (toRemove)
        {
            processToRemove.push_back(process.pid);
        }

        return toRemove;
    });

    //add new processes
    std::for_each(runningProcesses.constBegin(), runningProcesses.constEnd(), [this, readCommandAndUser, &processToAdd](pid_t pid)
    {
        //check if the process is already in the list
        auto it = std::find_if(m_processList.constBegin(), m_processList.constEnd(), [pid](const QCpuProcess & process)
        {
            return process.pid == pid;
        });

        if (it != m_processList.constEnd()) //process already in the list
        {
            return;
        }

        //create the process
        QCpuProcess process;
        process.pid                       = pid;
        process.lastMeasuredTimestampInMs = QDateTime::currentMSecsSinceEpoch();

        //read the command and the user
        readCommandAndUser(process);

        //add the process to the list
        m_processList.push_back(process);

        //add the process to the list
        processToAdd.push_back(pid);
    });

    //emit the signal
    emit updateProcessList(m_processList, processToAdd, processToRemove);
}

/**
 * @brief QCpuMonitor::scanProcessCpuTime
 */
void QCpuMonitor::scanProcessCpuTime(quint64 now, QCpuProcess& process) noexcept
{
    //calculate the elapsed time since the last measurement
    //update each 20ms
    constexpr quint64 refreshIntervalInMs = 20;
    quint64 elapsed = now - process.lastMeasuredTimestampInMs;

    if (elapsed < refreshIntervalInMs)
    {
        return;
    }

    //create the stat file path
    const QString statFilePath = QString("/proc/%1/stat").arg(process.pid);

    //create the stat file object
    QFile statFile(statFilePath);

    //try to open the stat file
    if (!statFile.open(QIODevice::ReadOnly))
    {
        return;
    }

    //create the text stream
    QTextStream textStream(&statFile);

    //read the first line
    const QString line = textStream.readLine();

    //close the stat file
    statFile.close();

    //check if the line is valid
    if (line.isEmpty())
    {
        qDebug() << "QCpuMonitor::scanProcessCpuTime: cannot read the stat file - pid:" << process.pid;
        return;
    }

    //scan the line
    const auto str = line.toStdString();
    char* buf = const_cast<char*>(str.c_str());

    /* (1) pid -  %d */
    char* location = strchr(buf, ' ');
    if (!location)
    {
        return;
    }

    /* (2) comm - (%s) */
    location += 2;
    char* end = strrchr(location, ')');
    if (!end)
    {
        return;
    }

    location = end + 2;

    /* (3) state - %c */
    location += 2;

    /* (4) ppid - %d */
    strtol(location, &location, 10);
    location += 1;

    /* (5) pgrp - %d */
    strtol(location, &location, 10);
    location += 1;

    /* (6) session - %d */
    strtol(location, &location, 10);
    location += 1;

    /* (7) tty_nr - %d */
    strtoul(location, &location, 10);
    location += 1;

    /* (8) tpgid - %d */
    strtol(location, &location, 10);
    location += 1;

    /* Skip (9) flags - %u */
    location = strchr(location, ' ') + 1;

    /* (10) minflt - %lu */
    strtoull(location, &location, 10);
    location += 1;

    /* (11) cminflt - %lu */
    strtoull(location, &location, 10);
    location += 1;

    /* (12) majflt - %lu */
    strtoull(location, &location, 10);
    location += 1;

    /* (13) cmajflt - %lu */
    strtoull(location, &location, 10);
    location += 1;

    /* (14) utime - %lu */
    const quint64 utime = strtoull(location, &location, 10);
    location += 1;

    /* (15) stime - %lu */
    const quint64 stime = strtoull(location, &location, 10);
    location += 1;

    //update the CPU time
    process.previousCpuTimeInTicks = process.cpuTimeInTicks;
    process.cpuTimeInTicks = (utime + stime) * 1000 / HZ;

    //calculate the sample
    const double sample = 1.0 * (process.cpuTimeInTicks - process.previousCpuTimeInTicks) / elapsed;

    //calculate CPU usage
    constexpr double alpha = 0.08;
    process.cpuUsageInPercent = (1.0 - alpha) * process.cpuUsageInPercent + (alpha * sample);

    //update the timestamp
    process.lastMeasuredTimestampInMs = now;
}

/**
 * @brief QCpuMonitor::timeoutControlCpuLimit
 */
void QCpuMonitor::timeoutControlCpuLimit() noexcept
{
    //start the timer again once we go out of this method
    auto timerGuard = qScopeGuard([this]()
    {
        m_timerLimitCpuPtr->start();
    });

    //get the current timestamp
    const quint64 now = QDateTime::currentMSecsSinceEpoch();

    //loop through the processes
    std::for_each(m_processList.begin(), m_processList.end(), [this, now](QCpuProcess & process)
    {
        //scan the cpu time for each process
        scanProcessCpuTime(now, process);

        //check if the process has a cpu limit
        if (!process.cpuLimitInPercent.has_value())
        {
            return;
        }

        //check if the process is sleeping
        if (process.sleepCountInCycle >= 1)
        {
            //subtract the sleep count
            process.sleepCountInCycle--;

            //should we send a SIGCONT signal to the process?
            if (process.sleepCountInCycle == 0)
            {
                //send a SIGCONT signal to the process
                kill(process.pid, SIGCONT);
            }

            //skip the process
            return;
        }

        //do we exceed the cpu limit?
        if (process.cpuLimitInPercent.value() <= 0.001 ||
                process.cpuUsageInPercent <= process.cpuLimitInPercent.value())
        {
            return;
        }

        //count the sleep count
        process.sleepCountInCycle = (process.cpuUsageInPercent - process.cpuLimitInPercent.value()) / process.cpuLimitInPercent.value();
        process.sleepCountInCycle = std::max(process.sleepCountInCycle, 1);

        //send a SIGSTOP signal to the process
        kill(process.pid, SIGSTOP);
    });
}

/**
 * @brief QCpuMonitor::timeoutCpuMonitor
 */
void QCpuMonitor::timeoutCpuMonitor() noexcept
{
    //scan running processes
    scanRunningProcesses();

    //start the timer again
    m_timerMonitorCpuPtr->start();
}
