pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property var dashboardViewModel

    property real peepSetpoint: 5.0
    property real fio2Setpoint: 40.0
    property real tidalVolumeSetpoint: 500.0
    property real rateSetpoint: 12.0
    property real ieRatioSetpoint: 2.0

    implicitHeight: 146
    color: "#0C1216"
    border.color: "#29373E"
    border.width: 1

    function openEditor(name, currentValue, unit, minLimit, maxLimit, softLimit) {
        safetyNumpad.parameterName = name
        safetyNumpad.currentValue = currentValue
        safetyNumpad.unit = unit
        safetyNumpad.minLimit = minLimit
        safetyNumpad.maxLimit = maxLimit
        safetyNumpad.softLimit = softLimit
        safetyNumpad.valueDecimals = name === "Tidal Volume"
                || name === "FiO2" || name === "Rate" ? 0 : 1
        safetyNumpad.open()
    }

    function updateInspiratoryTime() {
        const cycleDuration = 60.0 / Math.max(rateSetpoint, 1.0)
        const inspiratoryTime = cycleDuration / (1.0 + Math.max(ieRatioSetpoint, 0.1))
        dashboardViewModel.setVentilationParameter("inspiratoryTime", inspiratoryTime)
    }

    function applySetpoint(name, value) {
        switch (name) {
        case "PEEP":
            peepSetpoint = value
            dashboardViewModel.setVentilationParameter("peep", value)
            break
        case "FiO2":
            fio2Setpoint = value
            dashboardViewModel.setVentilationParameter("fio2", value)
            break
        case "Tidal Volume":
            tidalVolumeSetpoint = value
            dashboardViewModel.setVentilationParameter("tidalVolume", value / 1000.0)
            break
        case "Rate":
            rateSetpoint = value
            dashboardViewModel.setVentilationParameter("rate", value)
            updateInspiratoryTime()
            break
        case "I:E Ratio":
            ieRatioSetpoint = value
            updateInspiratoryTime()
            break
        }
    }

    component VentilatorParameterTile: ParameterTile {
        Layout.fillWidth: true
        onParameterSelected: function(name, currentValue, selectedUnit,
                                      minimum, maximum, softMaximum) {
            root.openEditor(name, currentValue, selectedUnit,
                            minimum, maximum, softMaximum)
        }
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: 10
        contentWidth: Math.max(width, parameterRow.implicitWidth)
        contentHeight: height
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.HorizontalFlick

        RowLayout {
            id: parameterRow
            anchors.margins: root.compactHeight ? 8 : 16
            width: Math.max(parent.width, implicitWidth)
            height: parent.height
            spacing: 10

            VentilatorParameterTile {
                parameterName: "PEEP"
                unit: "cmH₂O"
                measuredValue: root.dashboardViewModel.measuredPeep
                targetValue: root.peepSetpoint
                minLimit: 0
                maxLimit: 30
                softLimit: 15
                valueDecimals: 1
            }

            VentilatorParameterTile {
                parameterName: "FiO2"
                unit: "%"
                measuredValue: root.fio2Setpoint
                targetValue: root.fio2Setpoint
                minLimit: 21
                maxLimit: 100
                softLimit: 60
                valueDecimals: 0
            }

            VentilatorParameterTile {
                parameterName: "Tidal Volume"
                unit: "mL"
                measuredValue: root.dashboardViewModel.tidalVolume * 1000.0
                targetValue: root.tidalVolumeSetpoint
                minLimit: 50
                maxLimit: 2000
                softLimit: 800
                valueDecimals: 0
            }

            VentilatorParameterTile {
                parameterName: "Rate"
                unit: "/min"
                measuredValue: root.dashboardViewModel.respiratoryRate
                targetValue: root.rateSetpoint
                minLimit: 1
                maxLimit: 60
                softLimit: 35
                valueDecimals: 0
            }

            VentilatorParameterTile {
                parameterName: "I:E Ratio"
                unit: "1:x"
                measuredValue: root.ieRatioSetpoint
                targetValue: root.ieRatioSetpoint
                minLimit: 0.5
                maxLimit: 5.0
                softLimit: 3.0
                valueDecimals: 1
            }
        }
    }

    SafetyNumpad {
        id: safetyNumpad

        parent: Overlay.overlay
        onValueConfirmed: function(name, value) {
            root.applySetpoint(name, value)
        }
    }
}