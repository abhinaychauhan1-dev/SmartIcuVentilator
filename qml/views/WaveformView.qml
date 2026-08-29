/**
 * @filename    : WaveformView.qml
 * @brief   : Displays live pressure, flow, and volume waveform charts.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

import QtQuick
import QtQuick.Layouts
import QtGraphs

Rectangle {
    id: root

    required property var waveformPlotViewModel
    required property var dashboardViewModel

    color: "#090D10"
    implicitWidth: 1180
    implicitHeight: 760

    readonly property bool compactHeight: height < 560
    readonly property int metricValueFontSize: compactHeight ? 28 : 36

    // Formats a finite dashboard metric or returns an unavailable placeholder.
    function metric(value, decimals) {
        return dashboardViewModel && Number.isFinite(value)
                ? Number(value).toFixed(decimals) : "--"
    }

    // Provides a conservative visual approximation of plateau pressure.
    function plateauPressure() {
        if (!dashboardViewModel)
            return NaN
        return Math.max(dashboardViewModel.measuredPeep,
                        dashboardViewModel.meanPressure)
    }

    Timer {
        // Batch queued telemetry near display cadence instead of updating per sample.
        interval: 33
        repeat: true
        running: root.waveformPlotViewModel !== null
        triggeredOnStart: true
        onTriggered: {
            if (root.waveformPlotViewModel) {
                root.waveformPlotViewModel.renderSweepBatch(
                    pawSeries, flowSeries, volumeSeries)
            }
        }
    }

    Connections {
        target: root.waveformPlotViewModel
        // Requests redraws after all waveform series are replaced as one batch.
        function onWaveformsUpdated() {
            pawGraph.update()
            flowGraph.update()
            volumeGraph.update()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 112

            RowLayout {
                anchors.fill: parent
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 2

                    Text {
                        text: "AIRWAY PRESSURE  /  cmH₂O"
                        color: "#FFCA28"
                        font.family: "Bahnschrift"
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                    }

                    GraphsView {
                        id: pawGraph
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        marginLeft: 8
                        marginRight: 8
                        marginTop: 4
                        marginBottom: 4
                        axisX: ValueAxis {
                            min: 0
                            max: 10
                            tickInterval: 1
                            labelDecimals: 0
                        }
                        axisY: ValueAxis {
                            min: -5
                            max: Math.max(50, root.dashboardViewModel.peakPressure * 1.2)
                            tickInterval: 10
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
                            id: pawSeries
                            color: "#FFCA28"
                            width: 2.5
                            capStyle: Qt.RoundCap
                        }
                    }
                }

                Rectangle {
                    Layout.preferredWidth: 226
                    Layout.fillHeight: true
                    color: "#11191E"
                    border.color: "#34434B"
                    border.width: 1
                    radius: 6

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: root.compactHeight ? 8 : 16
                        spacing: root.compactHeight ? 2 : 6

                        Text { text: "PRESSURE"; color: "#94A5AE"; font.family: "Bahnschrift"; font.pixelSize: 12; font.weight: Font.DemiBold }
                        Text { text: root.metric(root.dashboardViewModel ? root.dashboardViewModel.peakPressure : NaN, 1); color: "#FFCA28"; font.family: "Bahnschrift"; font.pixelSize: root.metricValueFontSize; font.weight: Font.Bold }
                        Text { text: "Ppeak   cmH₂O"; color: "#F4F7F8"; font.family: "Bahnschrift"; font.pixelSize: 13 }
                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: "#34434B" }
                        Text { text: "Pplat   " + root.metric(root.plateauPressure(), 1) + " cmH₂O"; color: "#F4F7F8"; font.family: "Bahnschrift"; font.pixelSize: 15 }
                        Text { text: "PEEP    " + root.metric(root.dashboardViewModel ? root.dashboardViewModel.measuredPeep : NaN, 1) + " cmH₂O"; color: "#F4F7F8"; font.family: "Bahnschrift"; font.pixelSize: 15 }
                    }
                }
            }
        }

        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: "#223038" }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 112

            RowLayout {
                anchors.fill: parent
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 2

                    Text {
                        text: "FLOW  /  L/min"
                        color: "#00E5FF"
                        font.family: "Bahnschrift"
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                    }

                    GraphsView {
                        id: flowGraph
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        marginLeft: 8
                        marginRight: 8
                        marginTop: 4
                        marginBottom: 4
                        axisX: ValueAxis { min: 0; max: 10; tickInterval: 1; labelDecimals: 0 }
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
                            id: flowSeries
                            color: "#00E5FF"
                            width: 2.5
                            capStyle: Qt.RoundCap
                        }
                    }
                }

                Rectangle {
                    Layout.preferredWidth: 226
                    Layout.fillHeight: true
                    color: "#11191E"
                    border.color: "#34434B"
                    border.width: 1
                    radius: 6

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: root.compactHeight ? 8 : 16
                        spacing: root.compactHeight ? 2 : 8
                        Text { text: "VENTILATION"; color: "#94A5AE"; font.family: "Bahnschrift"; font.pixelSize: 12; font.weight: Font.DemiBold }
                        Text { text: root.metric(root.dashboardViewModel ? root.dashboardViewModel.minuteVolume : NaN, 1); color: "#00E5FF"; font.family: "Bahnschrift"; font.pixelSize: root.metricValueFontSize; font.weight: Font.Bold }
                        Text { text: "MV   L/min"; color: "#F4F7F8"; font.family: "Bahnschrift"; font.pixelSize: 13 }
                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: "#34434B" }
                        Text { text: "Rate    " + root.metric(root.dashboardViewModel ? root.dashboardViewModel.respiratoryRate : NaN, 0) + " /min"; color: "#F4F7F8"; font.family: "Bahnschrift"; font.pixelSize: 15 }
                    }
                }
            }
        }

        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: "#223038" }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 112

            RowLayout {
                anchors.fill: parent
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 2

                    Text {
                        text: "VOLUME  /  mL"
                        color: "#FF9100"
                        font.family: "Bahnschrift"
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                    }

                    GraphsView {
                        id: volumeGraph
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        marginLeft: 8
                        marginRight: 8
                        marginTop: 4
                        marginBottom: 4
                        axisX: ValueAxis { min: 0; max: 10; tickInterval: 1; labelDecimals: 0 }
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
                            id: volumeSeries
                            color: "#FF9100"
                            width: 2.5
                            capStyle: Qt.RoundCap
                        }
                    }
                }

                Rectangle {
                    Layout.preferredWidth: 226
                    Layout.fillHeight: true
                    color: "#11191E"
                    border.color: "#34434B"
                    border.width: 1
                    radius: 6

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: root.compactHeight ? 8 : 16
                        spacing: root.compactHeight ? 2 : 8
                        Text { text: "DELIVERED VOLUME"; color: "#94A5AE"; font.family: "Bahnschrift"; font.pixelSize: 12; font.weight: Font.DemiBold }
                        Text { text: root.metric(root.dashboardViewModel ? root.dashboardViewModel.tidalVolume * 1000.0 : NaN, 0); color: "#FF9100"; font.family: "Bahnschrift"; font.pixelSize: root.metricValueFontSize; font.weight: Font.Bold }
                        Text { text: "Vte   mL"; color: "#F4F7F8"; font.family: "Bahnschrift"; font.pixelSize: 13 }
                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: "#34434B" }
                        Text { text: "MV      " + root.metric(root.dashboardViewModel ? root.dashboardViewModel.minuteVolume : NaN, 1) + " L/min"; color: "#F4F7F8"; font.family: "Bahnschrift"; font.pixelSize: 15 }
                    }
                }
            }
        }
    }
}