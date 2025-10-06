cmake_minimum_required(VERSION 3.16)

project(NewCTScanner VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets StateMachine)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets StateMachine)

# --- Organize all project source and header files here ---
set(UI_FILES
    src/ui/mainwindow.h
    src/ui/mainwindow.cpp
)

set(CORE_FILES
    src/core/dataacquisitionservice.h
    src/core/dataacquisitionservice.cpp
    src/core/corefacade.h
    src/core/corefacade.cpp
    src/core/configmanager.h
    src/core/configmanager.cpp
    src/core/datasaver.h
    src/core/datasaver.cpp
    src/core/hardwareservice.h
    src/core/hardwareservice.cpp
    src/core/reconstructioncontroller.h
    src/core/reconstructioncontroller.cpp
    src/core/scancontroller.h
    src/core/scancontroller.cpp
    src/core/reconstructiontask.h
    src/core/reconstructiontask.cpp
    src/core/Frame.h
    src/core/framebuffer.h
    src/core/framebuffer.cpp
    src/core/acquisitionworker.h
    src/core/acquisitionworker.cpp
    src/core/systemsafetyservice.h
     src/core/systemsafetyservice.cpp
)

set(HAL_FILES
    src/hal/IXRaySource.h
    src/hal/dummysource.h
    src/hal/dummysource.cpp
    src/hal/IDetector.h
    src/hal/dummydetector.h
    src/hal/dummydetector.cpp
    src/hal/IMotionStage.h
    src/hal/dummymotionstage.h
    src/hal/dummymotionstage.cpp
)

set(COMMON_FILES
    src/common/logger.h
    src/common/logger.cpp
    src/common/ScanParameters.h
    src/common/HardwareStatus.h
    src/common/ScanProgress.h # <-- 将新文件添加在这里

)

set(PROJECT_SOURCES
    src/main.cpp
    ${UI_FILES}
    ${CORE_FILES}
    ${HAL_FILES}
    ${COMMON_FILES}
)
# --- End of file organization ---


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NewCTScanner
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/common/ScanProgress.h
        src/core/systemsafetyservice.h src/core/systemsafetyservice.cpp

    )
else()
    add_executable(NewCTScanner ${PROJECT_SOURCES})
endif()

target_link_libraries(NewCTScanner PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::StateMachine)

# Add include directories for headers
target_include_directories(NewCTScanner PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# --- Properties for macOS and Windows deployment ---
if(${QT_VERSION} VERSION_LESS 6.1.0 AND APPLE)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.NewCTScanner)
endif()

set_target_properties(NewCTScanner PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# --- Installation rules ---
include(GNUInstallDirs)
install(TARGETS NewCTScanner
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(NewCTScanner)
endif()
