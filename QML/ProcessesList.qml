/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls 1.4 as Old
import CpuLimitManager 1.0

Old.TableView {
    id: root
    model: CpuLimitManager.processesModel

    Old.TableViewColumn {
        id: pidColumn
        role: "pid"
        title: "PID"
        width: 150
    }

    Old.TableViewColumn {
        id: userColumn
        role: "user"
        title: "User"
        width: 250
    }

    Old.TableViewColumn {
        id: cpuUsageColumn
        role: "cpuUsage"
        title: "CPU Usage (%)"
        width: 200
    }

    Old.TableViewColumn {
        id: cpuLimitColumn
        role: "cpuLimit"
        title: "CPU Limit (%)"
        width: 200
    }

    Old.TableViewColumn {
        id: commandColumn
        role: "command"
        title: "Command"
        width: 300
    }

    onWidthChanged: {
        let commandColumnWidth = root.width - pidColumn.width - userColumn.width - cpuUsageColumn.width - cpuLimitColumn.width
        commandColumnWidth = commandColumnWidth < 300 ? 300 : commandColumnWidth
        commandColumn.width = commandColumnWidth
    }
}
