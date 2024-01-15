/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */
 
import QtQuick 2.15
import QtQuick.Controls 2.15
import CpuLimitManager 1.0

ApplicationWindow {
    id: root
    visible: true
    title: "Qt CPU Limit"

    readonly property int windowMinimumWidth: 800
    readonly property int windowMinimumHeight: 800
    readonly property int customizationPanelHeight: 300

    width: root.windowMinimumWidth
    height: root.windowMinimumHeight
    minimumWidth: root.windowMinimumWidth
    minimumHeight: root.windowMinimumHeight

    //Customization Panel
    CustomizationPanel {
        id: customizationPanel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: root.customizationPanelHeight
    }

    //Processes List
    ProcessesList {
        id: processesList
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: customizationPanel.bottom
        anchors.bottom: parent.bottom
    }

    footer: Text {
        text: "  Process count: " + CpuLimitManager.processCount
    }
    
    Component.onCompleted: {
        root.showMaximized()
    }
}
