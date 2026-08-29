/**
 * @filename    : ParameterTile.qml
 * @brief   : Presents a measured value and its editable target setting.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AbstractButton {
    id: root

    property string parameterName: "Parameter"
    property string unit: ""
    property real measuredValue: 0.0
    property real targetValue: 0.0
    property real minLimit: 0.0
    property real maxLimit: 100.0
    property real softLimit: maxLimit
    property int valueDecimals: 1

    signal parameterSelected(string name, real currentValue, string unit,
                             real minLimit, real maxLimit, real softLimit)

    implicitWidth: 180
    implicitHeight: 120
    Layout.minimumWidth: 140
    Layout.minimumHeight: 100

    // Maintain a stable touch target while allowing the panel to compact horizontally.
    leftPadding: 12
    rightPadding: 12
    topPadding: 8
    bottomPadding: 8

    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    Accessible.name: parameterName + " set value "
                     + Number(targetValue).toFixed(valueDecimals) + " " + unit
    Accessible.role: Accessible.Button

    // Publishes the setpoint and safety limits selected for editing.
    onClicked: parameterSelected(parameterName, targetValue, unit,
                                 minLimit, maxLimit, softLimit)

    background: Rectangle {
        color: root.down ? "#1C3038" : root.hovered ? "#17272E" : "#11191E"
        border.color: root.down || root.activeFocus ? "#00E5FF" : "#3A4A52"
        border.width: root.down || root.activeFocus ? 3 : 1
        radius: 6

        Behavior on color { ColorAnimation { duration: 90 } }
        Behavior on border.color { ColorAnimation { duration: 90 } }
    }

    contentItem: ColumnLayout {
        spacing: 2

        // Separate live measured data from the editable prescribed setpoint.
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: root.parameterName
                color: "#DCE5E9"
                font.family: "Bahnschrift"
                font.pixelSize: 15
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Text {
                text: root.unit
                color: "#8EA0A9"
                font.family: "Bahnschrift"
                font.pixelSize: 12
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Text {
                text: Number(root.measuredValue).toFixed(root.valueDecimals)
                color: "#F7FAFB"
                font.family: "Bahnschrift"
                font.pixelSize: 32
                font.weight: Font.Bold
                Layout.alignment: Qt.AlignVCenter
            }

            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "SET"
                color: "#7F919A"
                font.family: "Bahnschrift"
                font.pixelSize: 11
                font.weight: Font.DemiBold
            }

            Item { Layout.fillWidth: true }

            Text {
                text: Number(root.targetValue).toFixed(root.valueDecimals)
                      + (root.unit.length > 0 ? " " + root.unit : "")
                color: "#00E5FF"
                font.family: "Bahnschrift"
                font.pixelSize: 16
                font.weight: Font.DemiBold
            }
        }
    }
}