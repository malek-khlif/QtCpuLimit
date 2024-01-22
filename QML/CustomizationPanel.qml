/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QCpuModel 1.0

 Column {
    id: root

    Row {
        spacing: 5

        Text {
            text: qsTr("PID: ")
        }

        Text {
            text: QCpuModel.selectedProcessPid == "-1" ? "N/A" : QCpuModel.selectedProcessPid
        }
    }

    Row {
        spacing: 5

        Text {
            text: qsTr("Command: ")
        }

        Text {
            text: QCpuModel.selectedProcessCommand
        }
    }

    Row {
        spacing: 5

        Text {
            text: qsTr("CPU limit %: ")
        }

        Text {
            text: QCpuModel.selectedProcessCpuLimit == -1 ? "N/A" : QCpuModel.selectedProcessCpuLimit
        }
    }

    Row {
        spacing: 5

        Text {
            text: qsTr("Set CPU limit %: ")
            anchors.verticalCenter: parent.verticalCenter
        }

        Slider {
            id: limitSlider
            from: 1
            to: 100
            stepSize: 1
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: limitSlider.value + " %"
            anchors.verticalCenter: parent.verticalCenter
        }

        Button {
            text: "Set Limit"
            anchors.verticalCenter: parent.verticalCenter
            onClicked: QCpuModel.setProcessLimit(limitSlider.value)
        }

        Button {
            text: "Reset Limit"
            anchors.verticalCenter: parent.verticalCenter
            onClicked: QCpuModel.removeProcessLimit()
        }
    }
 }
