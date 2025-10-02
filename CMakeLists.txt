cmake_minimum_required(VERSION 3.16)

project(NewCTScanner VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets StateMachine)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets StateMachine)

set(PROJECT_SOURCES
        src/main.cpp
)

set(UI_SOURCES
        src/ui/mainwindow.cpp
        src/ui/mainwindow.h
)

set(CORE_SOURCES
        src/core/scancontroller.cpp
        src/core/scancontroller.h
)


set(HAL_HEADERS
        src/hal/IXRaySource.h
        src/hal/DummySource.h
        src/hal/dummysource.cpp

)
set(COMMON_FILES
    src/common/ScanParameters.h
)



list(APPEND PROJECT_SOURCES ${UI_SOURCES})
list(APPEND PROJECT_SOURCES ${CORE_SOURCES})
list(APPEND PROJECT_SOURCES
    ${UI_FILES}
    ${CORE_FILES}
    ${HAL_FILES}
    ${COMMON_FILES}
)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NewCTScanner
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/core/scancontroller.h src/core/scancontroller.cpp
        src/hal/IXRaySource.h
        src/hal/dummysource.h src/hal/dummysource.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET NewCTScanner APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(NewCTScanner SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(NewCTScanner
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(NewCTScanner PRIVATE Qt${QT_VERSION_MAJOR}::Widgets  Qt${QT_VERSION_MAJOR}::StateMachine)

target_include_directories(NewCTScanner PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hal"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/common"
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.NewCTScanner)
endif()
set_target_properties(NewCTScanner PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS NewCTScanner
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(NewCTScanner)
endif()
