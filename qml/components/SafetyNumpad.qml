pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root

    property string parameterName: "Parameter"
    property string unit: ""
    property real currentValue: 0.0
    property real minLimit: 0.0
    property real maxLimit: 100.0
    property real softLimit: maxLimit
    property int valueDecimals: 1

    readonly property real enteredValue: Number(editText)
    readonly property bool numericEntryValid: editText.length > 0
                                                   && Number.isFinite(enteredValue)
    readonly property bool withinHardLimits: numericEntryValid
                                                  && enteredValue >= minLimit
                                                  && enteredValue <= maxLimit
    readonly property bool exceedsSoftLimit: withinHardLimits
                                                 && enteredValue > softLimit
    readonly property bool canConfirm: withinHardLimits
                                           && (!exceedsSoftLimit || softLimitOverridden)
    property string editText: ""
    property bool softLimitOverridden: false
    property int remainingMilliseconds: 5000

    signal valueConfirmed(string name, real value)
    signal editCancelled()

    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose
    padding: 18
    width: Math.min(430, parent ? parent.width - 32 : 430)
    height: Math.min(660, parent ? parent.height - 32 : 660)
    anchors.centerIn: parent

    function resetSession() {
        editText = Number(currentValue).toFixed(valueDecimals)
        softLimitOverridden = false
        remainingMilliseconds = 5000
    }

    function appendCharacter(character) {
        if (character === "." && editText.indexOf(".") !== -1)
            return
        if (editText === "0" && character !== ".")
            editText = character
        else
            editText += character
        softLimitOverridden = false
    }

    function cancelEdit() {
        countdown.stop()
        close()
        editCancelled()
    }

    onOpened: {
        resetSession()
        countdown.start()
    }
    onClosed: countdown.stop()

    background: Rectangle {
        color: "#11181C"
        border.color: "#54656D"
        border.width: 2
        radius: 8
    }

    Overlay.modal: Rectangle { color: "#B0000000" }

    Timer {
        id: countdown
        interval: 100
        repeat: true
        onTriggered: {
            root.remainingMilliseconds = Math.max(0, root.remainingMilliseconds - interval)
            if (root.remainingMilliseconds === 0)
                root.cancelEdit()
        }
    }

    contentItem: ColumnLayout {
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 0
                Layout.fillWidth: true
                Text {
                    text: "SET " + root.parameterName.toUpperCase()
                    color: "#F4F7F8"
                    font.family: "Bahnschrift"
                    font.pixelSize: 20
                    font.weight: Font.Bold
                }
                Text {
                    text: "Allowed " + Number(root.minLimit).toFixed(root.valueDecimals)
                          + "–" + Number(root.maxLimit).toFixed(root.valueDecimals)
                          + (root.unit.length ? " " + root.unit : "")
                    color: "#9AABB3"
                    font.family: "Bahnschrift"
                    font.pixelSize: 13
                }
            }

            ToolButton {
                text: "✕"
                font.pixelSize: 20
                Accessible.name: "Cancel parameter edit"
                onClicked: root.cancelEdit()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 76
            color: "#0A0F12"
            border.color: root.withinHardLimits ? "#00E5FF" : "#FF5252"
            border.width: 2
            radius: 6

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12

                Text {
                    text: root.editText.length ? root.editText : "--"
                    color: root.withinHardLimits ? "#FFFFFF" : "#FF8A80"
                    font.family: "Bahnschrift"
                    font.pixelSize: 38
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignRight
                    Layout.fillWidth: true
                    elide: Text.ElideLeft
                }
                Text {
                    text: root.unit
                    color: "#B8C5CB"
                    font.family: "Bahnschrift"
                    font.pixelSize: 16
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? 72 : 0
            visible: root.exceedsSoftLimit
            color: "#3A2908"
            border.color: "#FFB300"
            border.width: 2
            radius: 6

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0
                    Text { text: "CLINICAL SOFT LIMIT"; color: "#FFD166"; font.family: "Bahnschrift"; font.pixelSize: 13; font.weight: Font.Bold }
                    Text { text: "Above " + Number(root.softLimit).toFixed(root.valueDecimals) + " " + root.unit; color: "#FFF4D1"; font.family: "Bahnschrift"; font.pixelSize: 12 }
                }

                Button {
                    text: root.softLimitOverridden ? "OVERRIDDEN" : "OVERRIDE"
                    enabled: !root.softLimitOverridden
                    Layout.minimumWidth: 112
                    Layout.minimumHeight: 48
                    onClicked: root.softLimitOverridden = true
                }
            }
        }

        GridLayout {
            columns: 4
            columnSpacing: 8
            rowSpacing: 8
            Layout.alignment: Qt.AlignHCenter

            Repeater {
                model: ["1", "2", "3", "⌫", "4", "5", "6", "C",
                        "7", "8", "9", ".", "", "0", "", ""]

                delegate: Button {
                    id: keypadButton

                    required property string modelData

                    visible: modelData.length > 0
                    enabled: visible
                    Layout.minimumWidth: 56
                    Layout.preferredWidth: 68
                    Layout.minimumHeight: 56
                    Layout.preferredHeight: 60
                    text: modelData
                    font.family: "Bahnschrift"
                    font.pixelSize: 24
                    font.weight: Font.DemiBold
                    Accessible.name: modelData === "⌫" ? "Backspace"
                                     : modelData === "C" ? "Clear" : modelData
                    onClicked: {
                        if (modelData === "⌫") {
                            root.editText = root.editText.slice(0, -1)
                            root.softLimitOverridden = false
                        } else if (modelData === "C") {
                            root.editText = ""
                            root.softLimitOverridden = false
                        } else {
                            root.appendCharacter(modelData)
                        }
                    }
                    background: Rectangle {
                        color: keypadButton.down ? "#28414B" : "#1B292F"
                        border.color: keypadButton.activeFocus ? "#00E5FF" : "#52636B"
                        border.width: keypadButton.activeFocus ? 3 : 1
                        radius: 6
                    }
                    contentItem: Text {
                        text: keypadButton.text
                        color: "#FFFFFF"
                        font: keypadButton.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Cancel"
                Layout.fillWidth: true
                Layout.minimumHeight: 56
                onClicked: root.cancelEdit()
            }

            Button {
                id: confirmButton

                text: "Confirm"
                enabled: root.canConfirm
                Layout.fillWidth: true
                Layout.minimumHeight: 56
                onClicked: {
                    countdown.stop()
                    root.valueConfirmed(root.parameterName, root.enteredValue)
                    root.close()
                }
                background: Rectangle {
                      color: !confirmButton.enabled ? "#283238"
                          : confirmButton.down ? "#00A5B8" : "#00CBE3"
                      border.color: confirmButton.enabled ? "#80F3FF" : "#465259"
                    border.width: 2
                    radius: 6
                }
                contentItem: Text {
                    text: confirmButton.text
                    color: confirmButton.enabled ? "#061014" : "#7C898F"
                    font.family: "Bahnschrift"
                    font.pixelSize: 17
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 8
            color: "#28343A"
            radius: 4

            Rectangle {
                width: parent.width * root.remainingMilliseconds / 5000
                height: parent.height
                color: root.remainingMilliseconds <= 2000 ? "#FF5252" : "#00E5FF"
                radius: 4

                Behavior on width { NumberAnimation { duration: 100 } }
            }
        }

        Text {
            text: "Auto-cancel in " + Math.ceil(root.remainingMilliseconds / 1000) + " s"
            color: root.remainingMilliseconds <= 2000 ? "#FF8A80" : "#93A5AE"
            font.family: "Bahnschrift"
            font.pixelSize: 12
            Layout.alignment: Qt.AlignHCenter
        }
    }
}