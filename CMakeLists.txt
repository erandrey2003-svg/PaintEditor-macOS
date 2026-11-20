cmake_minimum_required(VERSION 3.16)
project(PaintEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON) # Автоматически запускает MOC для Q_OBJECT
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport)

set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icon.icns)
set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

add_executable(PaintEditor MACOSX_BUNDLE
    main.cpp
    MainWindow.cpp
    Canvas.cpp
    RibbonButton.cpp
    FileMenu.cpp
    Dialogs.cpp
    Icons.cpp
    BrushTool.cpp
    PencilTool.cpp
    FillTool.cpp
    LineTool.cpp
    ShapeTool.cpp
    EraserTool.cpp
    ${APP_ICON}
)

set_target_properties(PaintEditor PROPERTIES
    MACOSX_BUNDLE_ICON_FILE icon.icns
    MACOSX_BUNDLE_BUNDLE_NAME "Растровый редактор"
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.painteditor
)

target_link_libraries(PaintEditor PRIVATE Qt6::Widgets Qt6::PrintSupport)