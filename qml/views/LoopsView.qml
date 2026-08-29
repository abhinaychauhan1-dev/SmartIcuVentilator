/**
 * @filename    : LoopsView.qml
 * @brief   : Visualizes pressure-volume and flow-volume respiratory loops.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs

Rectangle {
    id: root

    required property var waveformPlotViewModel
    required property var dashboardViewModel

    color: "#090D10"
    implicitWidth: 1180
    implicitHeight: 700
    readonly property bool compactWidth: width < 1100

    // Estimates dynamic compliance from tidal volume and driving pressure.
    function dynamicCompliance() {
        if (!dashboardViewModel)
            return NaN
        const drivingPressure = dashboardViewModel.peakPressure
                              - dashboardViewModel.measuredPeep
        return drivingPressure > 0.0
                ? dashboardViewModel.tidalVolume * 1000.0 / drivingPressure : NaN
    }

    // Maps normalized stiffness to a display-only airway resistance estimate.
    function estimatedResistance() {
        if (!dashboardViewModel)
            return NaN
        return 5.0 + dashboardViewModel.complianceIndex * 20.0
    }

    // Formats a finite metric or returns a placeholder for unavailable data.
    function metric(value, decimals) {
        return Number.isFinite(value) ? Number(value).toFixed(decimals) : "--"
    }

    Timer {
        // Refresh only the active traces; frozen references remain unchanged.
        interval: 50
        repeat: true
        running: root.visible && root.waveformPlotViewModel !== null
        triggeredOnStart: true
        onTriggered: root.waveformPlotViewModel.renderActiveLoops(activePv, activeFv)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 58
            spacing: 10

            Text {
                text: "RESPIRATORY LOOPS"
                color: "#F4F7F8"
                font.family: "Bahnschrift"
                font.pixelSize: root.compactWidth ? 18 : 22
                font.weight: Font.DemiBold
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                elide: Text.ElideRight
            }

            Rectangle {
                Layout.preferredWidth: root.compactWidth ? 160 : 190
                Layout.preferredHeight: 48
                color: "#102127"
                border.color: "#00E5FF"
                border.width: 1
                radius: 6
                Column {
                    anchors.centerIn: parent
                    spacing: 1
                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: "Cdyn"; color: "#8FA4AD"; font.family: "Bahnschrift"; font.pixelSize: 11 }
                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: root.metric(root.dynamicCompliance(), 1) + " mL/cmH₂O"; color: "#00E5FF"; font.family: "Bahnschrift"; font.pixelSize: root.compactWidth ? 15 : 17; font.weight: Font.Bold }
                }
            }

            Rectangle {
                Layout.preferredWidth: root.compactWidth ? 160 : 190
                Layout.preferredHeight: 48
                color: "#241B0D"
                border.color: "#FFCA28"
                border.width: 1
                radius: 6
                Column {
                    anchors.centerIn: parent
                    spacing: 1
                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: "Raw"; color: "#AFA28B"; font.family: "Bahnschrift"; font.pixelSize: 11 }
                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: root.metric(root.estimatedResistance(), 1) + " cmH₂O/L/s"; color: "#FFCA28"; font.family: "Bahnschrift"; font.pixelSize: root.compactWidth ? 15 : 17; font.weight: Font.Bold }
                }
            }

            Button {
                id: freezeButton

                Layout.preferredWidth: root.compactWidth ? 180 : 220
                Layout.preferredHeight: 48
                text: "Freeze / Save Reference"
                font.family: "Bahnschrift"
                font.pixelSize: root.compactWidth ? 12 : 14
                font.weight: Font.DemiBold
                onClicked: root.waveformPlotViewModel.freezeReferenceLoop(referencePv, referenceFv)
                background: Rectangle {
                    color: freezeButton.down ? "#177383" : freezeButton.hovered ? "#145C68" : "#104853"
                    border.color: "#00E5FF"
                    border.width: 1
                    radius: 6
                }
                contentItem: Text {
                    text: freezeButton.text
                    color: "#F5FCFD"
                    font: freezeButton.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 18

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 4

                Text {
                    text: "PRESSURE–VOLUME"
                    color: "#FFCA28"
                    font.family: "Bahnschrift"
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }

                Text {
                    text: "Paw (cmH₂O)  ×  Volume (mL)"
                    color: "#8FA0A9"
                    font.family: "Bahnschrift"
                    font.pixelSize: 12
                }

                GraphsView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    axisX: ValueAxis {
                        min: -5
                        max: Math.max(50, root.dashboardViewModel.peakPressure * 1.2)
                        tickInterval: 10
                        labelDecimals: 0
                    }
                    axisY: ValueAxis {
                        min: 0
                        max: Math.max(800, root.dashboardViewModel.tidalVolume * 1200.0)
                        tickInterval: 200
                        labelDecimals: 0
                    }
                    theme: GraphsTheme {
                        colorScheme: GraphsTheme.ColorScheme.Dark
                        backgroundColor: "#090D10"
                        plotAreaBackgroundColor: "#0D1317"
                        labelBackgroundVisible: false
                        labelBorderVisible: false
                        labelTextColor: "#C8D1D7"
                        grid.mainColor: "#263239"
                        grid.subColor: "#182126"
                        axisX.mainColor: "#60727C"
                        axisY.mainColor: "#60727C"
                    }
                    LineSeries {
                        id: referencePv
                        color: "#80FFCA28"
                        width: 2
                    }
                    LineSeries {
                        id: activePv
                        color: "#FFCA28"
                        width: 3
                        capStyle: Qt.RoundCap
                    }
                }
            }

            Rectangle { Layout.fillHeight: true; Layout.preferredWidth: 1; color: "#263239" }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 4

                Text {
                    text: "FLOW–VOLUME"
                    color: "#00E5FF"
                    font.family: "Bahnschrift"
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }

                Text {
                    text: "Volume (mL)  ×  Flow (L/min)"
                    color: "#8FA0A9"
                    font.family: "Bahnschrift"
                    font.pixelSize: 12
                }

                GraphsView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    axisX: ValueAxis {
                        min: 0
                        max: Math.max(800, root.dashboardViewModel.tidalVolume * 1200.0)
                        tickInterval: 200
                        labelDecimals: 0
                    }
                    axisY: ValueAxis { min: -300; max: 300; tickInterval: 150; labelDecimals: 0 }
                    theme: GraphsTheme {
                        colorScheme: GraphsTheme.ColorScheme.Dark
                        backgroundColor: "#090D10"
                        plotAreaBackgroundColor: "#0D1317"
                        labelBackgroundVisible: false
                        labelBorderVisible: false
                        labelTextColor: "#C8D1D7"
                        grid.mainColor: "#263239"
                        grid.subColor: "#182126"
                        axisX.mainColor: "#60727C"
                        axisY.mainColor: "#60727C"
                    }
                    LineSeries {
                        id: referenceFv
                        color: "#8000E5FF"
                        width: 2
                    }
                    LineSeries {
                        id: activeFv
                        color: "#00E5FF"
                        width: 3
                        capStyle: Qt.RoundCap
                    }
                }
            }
        }
    }
}