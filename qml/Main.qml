/**
 * @filename    : Main.qml
 * @brief   : Defines the main ventilator dashboard window and navigation layout.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"
import "views"

ApplicationWindow {
    id: root

    required property var dashboardViewModel
    required property var waveformPlotViewModel

    visible: true
    width: 1280
    height: 900
    minimumWidth: 960
    minimumHeight: 720
    title: "Smart ICU Ventilator"
    color: "#090D10"

    header: AlarmBanner {
        dashboardViewModel: root.dashboardViewModel
        width: root.width
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 58
            color: "#0D1418"
            border.color: "#26343B"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 16

                Text {
                    text: "SMART ICU VENTILATOR"
                    color: "#E9F0F3"
                    font.family: "Bahnschrift"
                    font.pixelSize: 17
                    font.weight: Font.DemiBold
                    Layout.preferredWidth: 230
                }

                TabBar {
                    id: navigationTabs

                    // Keep tab clicks and swipe gestures synchronized bidirectionally.
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: 0
                    background: Rectangle { color: "transparent" }
                    // Mirrors a tab selection to the swipeable page container.
                    onCurrentIndexChanged: {
                        if (swipeView.currentIndex !== currentIndex)
                            swipeView.currentIndex = currentIndex
                    }

                    Repeater {
                        model: ["Waveforms", "Loops", "3D Lung"]

                        delegate: TabButton {
                            id: navigationButton

                            required property string modelData

                            text: modelData
                            font.family: "Bahnschrift"
                            font.pixelSize: 15
                            font.weight: checked ? Font.Bold : Font.Medium
                            contentItem: Text {
                                text: navigationButton.text
                                color: navigationButton.checked ? "#061014" : "#B6C4CA"
                                font: navigationButton.font
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                  color: navigationButton.checked ? "#00D5ED"
                                      : navigationButton.down ? "#25343B" : "transparent"
                                radius: 4
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.preferredWidth: 138
                    Layout.preferredHeight: 34
                    color: "#12262C"
                    border.color: "#00A9BC"
                    border.width: 1
                    radius: 4

                    Text {
                        anchors.centerIn: parent
                        text: "VCV  •  ACTIVE"
                        color: "#78F1FF"
                        font.family: "Bahnschrift"
                        font.pixelSize: 12
                        font.weight: Font.Bold
                    }
                }
            }
        }

        SwipeView {
            id: swipeView

            // Each page shares the same backend instances injected from C++.
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            interactive: true
            // Mirrors a swipe gesture back to the selected navigation tab.
            onCurrentIndexChanged: {
                if (navigationTabs.currentIndex !== currentIndex)
                    navigationTabs.currentIndex = currentIndex
            }

            WaveformView {
                waveformPlotViewModel: root.waveformPlotViewModel
                dashboardViewModel: root.dashboardViewModel
            }

            LoopsView {
                waveformPlotViewModel: root.waveformPlotViewModel
                dashboardViewModel: root.dashboardViewModel
            }

            Lung3DView {
                dashboardViewModel: root.dashboardViewModel
            }
        }

        ParameterControlPanel {
            Layout.fillWidth: true
            Layout.preferredHeight: 146
            dashboardViewModel: root.dashboardViewModel
        }
    }
}