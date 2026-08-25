import QtQuick
import QtQuick3D

Rectangle {
    id: root

    required property var dashboardViewModel

    color: "#121417"
    implicitWidth: 1180
    implicitHeight: 700

    property real orbitYaw: 0
    property real orbitPitch: -4
    property real cameraDistance: 430

    readonly property real compliance: dashboardViewModel
                                               ? Math.max(0.0, Math.min(1.0,
                                                   dashboardViewModel.complianceIndex))
                                               : 0.0
    readonly property color healthyColor: "#00D2FF"
    readonly property color stiffColor: "#FF5252"
    readonly property color tissueColor: Qt.rgba(
                                             healthyColor.r + (stiffColor.r - healthyColor.r) * compliance,
                                             healthyColor.g + (stiffColor.g - healthyColor.g) * compliance,
                                             healthyColor.b + (stiffColor.b - healthyColor.b) * compliance,
                                             1.0)

    View3D {
        id: lungView

        anchors.fill: parent
        camera: camera

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#121417"
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.VeryHigh
            tonemapMode: SceneEnvironment.TonemapModeFilmic
            specularAAEnabled: true
            aoEnabled: true
            aoStrength: 24
            aoDistance: 45
            aoSoftness: 42
        }

        PerspectiveCamera {
            id: camera

            position: Qt.vector3d(0, 4, root.cameraDistance)
            fieldOfView: 38
            clipNear: 10
            clipFar: 1200

            Behavior on z {
                NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
            }
        }

        DirectionalLight {
            color: "#DDF7FF"
            brightness: 2.4
            eulerRotation: Qt.vector3d(-38, -32, 0)
            castsShadow: true
            shadowFactor: 45
            shadowMapQuality: Light.ShadowMapQualityHigh
        }

        DirectionalLight {
            color: "#89D8E8"
            brightness: 1.15
            eulerRotation: Qt.vector3d(28, 142, 8)
        }

        DirectionalLight {
            color: "#FFB3A9"
            brightness: 0.7
            eulerRotation: Qt.vector3d(55, 8, 180)
        }

        PrincipledMaterial {
            id: lungMembrane

            baseColor: root.tissueColor
            metalness: 0.0
            roughness: 0.3
            specularAmount: 0.65
            clearcoatAmount: 0.18
            clearcoatRoughnessAmount: 0.22
            transmissionFactor: 0.45
            thicknessFactor: 18
            attenuationColor: root.tissueColor
            attenuationDistance: 240
            alphaMode: PrincipledMaterial.Blend
            opacity: 0.92

            Behavior on baseColor {
                ColorAnimation { duration: 350; easing.type: Easing.InOutQuad }
            }
        }

        PrincipledMaterial {
            id: airwayMaterial

            baseColor: "#C6F4F7"
            roughness: 0.38
            metalness: 0.0
            opacity: 0.72
            alphaMode: PrincipledMaterial.Blend
        }

        Node {
            id: anatomicalTwin

            eulerRotation: Qt.vector3d(root.orbitPitch, root.orbitYaw, 0)

            Node {
                id: lungs

                scale: Qt.vector3d(
                           root.dashboardViewModel
                               ? root.dashboardViewModel.lungExpansionScale : 1.0,
                           root.dashboardViewModel
                               ? root.dashboardViewModel.lungExpansionScale : 1.0,
                           root.dashboardViewModel
                               ? root.dashboardViewModel.lungExpansionScale : 1.0)
                y: -6

                Behavior on scale {
                    Vector3dAnimation {
                        duration: 180
                        easing.type: Easing.InOutSine
                    }
                }

                Node {
                    id: leftLung

                    x: -58
                    eulerRotation.z: -7

                    Model {
                        source: "#Sphere"
                        position: Qt.vector3d(0, 38, 0)
                        scale: Qt.vector3d(0.68, 1.04, 0.56)
                        materials: [lungMembrane]
                    }

                    Model {
                        source: "#Sphere"
                        position: Qt.vector3d(-3, -48, 2)
                        scale: Qt.vector3d(0.78, 1.05, 0.62)
                        materials: [lungMembrane]
                    }
                }

                Node {
                    id: rightLung

                    x: 58
                    eulerRotation.z: 7

                    Model {
                        source: "#Sphere"
                        position: Qt.vector3d(0, 48, 0)
                        scale: Qt.vector3d(0.72, 0.92, 0.58)
                        materials: [lungMembrane]
                    }

                    Model {
                        source: "#Sphere"
                        position: Qt.vector3d(4, -8, 4)
                        scale: Qt.vector3d(0.76, 0.72, 0.61)
                        materials: [lungMembrane]
                    }

                    Model {
                        source: "#Sphere"
                        position: Qt.vector3d(3, -65, 1)
                        scale: Qt.vector3d(0.82, 0.82, 0.64)
                        materials: [lungMembrane]
                    }
                }

                Model {
                    source: "#Cylinder"
                    position: Qt.vector3d(0, 144, 0)
                    scale: Qt.vector3d(0.16, 0.72, 0.16)
                    materials: [airwayMaterial]
                }

                Model {
                    source: "#Cylinder"
                    position: Qt.vector3d(-27, 79, 0)
                    eulerRotation.z: -48
                    scale: Qt.vector3d(0.12, 0.58, 0.12)
                    materials: [airwayMaterial]
                }

                Model {
                    source: "#Cylinder"
                    position: Qt.vector3d(27, 79, 0)
                    eulerRotation.z: 48
                    scale: Qt.vector3d(0.12, 0.58, 0.12)
                    materials: [airwayMaterial]
                }
            }
        }
    }

    Text {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 22
        text: "L"
        color: "#7C929C"
        font.family: "Bahnschrift"
        font.pixelSize: 18
        font.weight: Font.DemiBold
    }

    Text {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 22
        text: "R"
        color: "#7C929C"
        font.family: "Bahnschrift"
        font.pixelSize: 18
        font.weight: Font.DemiBold
    }

    MouseArea {
        id: orbitControl

        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

        property real previousX: 0
        property real previousY: 0

        onPressed: function(mouse) {
            previousX = mouse.x
            previousY = mouse.y
        }

        onPositionChanged: function(mouse) {
            if (!pressed)
                return

            root.orbitYaw += (mouse.x - previousX) * 0.35
            root.orbitPitch = Math.max(-35, Math.min(30,
                                                     root.orbitPitch
                                                     + (mouse.y - previousY) * 0.25))
            previousX = mouse.x
            previousY = mouse.y
        }

        onWheel: function(wheel) {
            root.cameraDistance = Math.max(300, Math.min(620,
                                                        root.cameraDistance
                                                        - wheel.angleDelta.y * 0.18))
            wheel.accepted = true
        }

        onDoubleClicked: {
            root.orbitYaw = 0
            root.orbitPitch = -4
            root.cameraDistance = 430
        }
    }
}