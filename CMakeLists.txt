cmake_minimum_required(VERSION 3.21)

project(SmartIcuVentilator
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Quick
    Quick3D
    Graphs
    Widgets
)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(SmartIcuVentilator
    main.cpp
    src/application/AlarmEvaluator.cpp
    src/application/AlarmEvaluator.h
    src/application/BreathMetricsCalculator.cpp
    src/application/BreathMetricsCalculator.h
    src/application/VentilationParameterRouter.cpp
    src/application/VentilationParameterRouter.h
    src/domain/SimulationEngine.cpp
    src/domain/SimulationEngine.h
    src/domain/Units.h
    src/domain/VentilatorConfig.h
    src/viewmodels/DashboardViewModel.cpp
    src/viewmodels/DashboardViewModel.h
    src/viewmodels/WaveformPlotViewModel.cpp
    src/viewmodels/WaveformPlotViewModel.h
)

qt_add_qml_module(SmartIcuVentilator
    URI VentilatorHMI
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/views/WaveformView.qml
        qml/views/LoopsView.qml
        qml/views/Lung3DView.qml
        qml/components/ParameterTile.qml
        qml/components/ParameterControlPanel.qml
        qml/components/SafetyNumpad.qml
        qml/components/AlarmBanner.qml
)

target_link_libraries(SmartIcuVentilator
    PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::Quick3D
        Qt6::Graphs
        Qt6::Widgets
)

if(WIN32)
    target_link_libraries(SmartIcuVentilator PRIVATE winmm)
endif()

if(MSVC)
    target_compile_options(SmartIcuVentilator PRIVATE /W4)
else()
    target_compile_options(SmartIcuVentilator PRIVATE -Wall -Wextra)
endif()