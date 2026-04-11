cmake_minimum_required(VERSION 3.16)

project(SAPR VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── macOS: ensure C++ standard library headers are on the search path ──────────
# On some CLT installations the C++ headers live only inside the SDK, not in
# the CLT's own usr/include/c++.  Detect this case and add the SDK path.
if(APPLE)
    execute_process(
        COMMAND xcrun --sdk macosx --show-sdk-path
        OUTPUT_VARIABLE _MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(EXISTS "${_MACOS_SDK_PATH}/usr/include/c++/v1/type_traits")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${_MACOS_SDK_PATH}/usr/include/c++/v1")
    endif()
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg 3DCore 3DExtras 3DRender 3DInput 3DLogic)

set(PROJECT_SOURCES
        src/main.cpp
        # UI
        src/ui/mainwindow.h
        src/ui/mainwindow.cpp
        src/ui/mainwindow.ui
        src/ui/shapedialog.h
        src/ui/shapedialog.cpp
        src/ui/shapelistdock.h
        src/ui/shapelistdock.cpp
        src/ui/shapedialog3d.h
        src/ui/shapedialog3d.cpp
        src/ui/arraydialog.h
        src/ui/arraydialog.cpp
        src/ui/propertypanel.h
        src/ui/propertypanel.cpp
        # Model
        src/model/shapes.h
        src/model/shapes.cpp
        src/model/shape3d.h
        src/model/shape3d.cpp
        # Scene (2D)
        src/scene/drawingscene.h
        src/scene/drawingscene.cpp
        # Commands
        src/commands/commands.h
        src/commands/commands.cpp
        src/commands/commands3d.h
        src/commands/commands3d.cpp
        # Viewer (3D)
        src/viewer/viewer3d.h
        src/viewer/viewer3d.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SAPR
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SAPR APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(SAPR SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SAPR
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(SAPR PRIVATE
    src
    src/commands
    src/model
    src/scene
    src/ui
    src/viewer
)
target_link_libraries(SAPR PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::3DCore
    Qt${QT_VERSION_MAJOR}::3DExtras
    Qt${QT_VERSION_MAJOR}::3DRender
    Qt${QT_VERSION_MAJOR}::3DInput
    Qt${QT_VERSION_MAJOR}::3DLogic
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SAPR)
endif()
set_target_properties(SAPR PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SAPR
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SAPR)
endif()

# ── Tests ─────────────────────────────────────────────────────────────────────
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
