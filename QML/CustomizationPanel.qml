/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import CpuLimitManager 1.0

 Item {
    id: root

    Row {
        anchors.fill: root
        spacing: 5

        //Search panel
        Row {
            spacing: 5
            anchors.verticalCenter: parent.verticalCenter

            Item {
                width: 10
                height: 10
            }

            Text {
                text: qsTr("Search by command: ")
                anchors.verticalCenter: parent.verticalCenter
            }

            TextField {
                placeholderText: qsTr("Search by command")
                width: root.width / 4
            }
        }

        //Vertical line
        Rectangle {
            width: 1
            height: root.height
            color: "#e0e0e0"
        }

        //Selected process panel
        Column {
            spacing: 5
            anchors.verticalCenter: parent.verticalCenter

            Row {
                spacing: 5

                Text {
                    text: qsTr("PID: ")
                }

                Text {
                    text: "" //TODO
                }
            }

            Row {
                spacing: 5

                Text {
                    text: qsTr("Process: ")
                }

                Text {
                    text: "" //TODO
                }
            }

            Row {
                spacing: 5

                Text {
                    text: qsTr("Min CPU %: ")
                }

                Text {
                    text: "" //TODO
                }
            }

            Row {
                spacing: 5

                Text {
                    text: qsTr("Max CPU %: ")
                }

                Text {
                    text: "" //TODO
                }
            }

            Row {
                spacing: 5

                Text {
                    text: qsTr("CPU usage %: ")
                }

                Text {
                    text: "" //TODO
                }
            }

            Row {
                spacing: 5

                Text {
                    text: qsTr("CPU limit %: ")
                }

                Text {
                    text: "" //TODO
                }
            }
        }
    }

 }
