cmake_minimum_required(VERSION 3.19)
project(ChessGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

# Chess logic sources (from chess_oops/)
set(CHESS_LOGIC_SOURCES
    ../chess_oops/Board.cpp
    ../chess_oops/Piece.cpp
    ../chess_oops/Game.cpp
)

set(CHESS_LOGIC_HEADERS
    ../chess_oops/Board.h
    ../chess_oops/Piece.h
    ../chess_oops/Game.h
    ../chess_oops/Position.h
)

# GUI sources
set(GUI_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    chessboardwidget.cpp
    chessboardwidget.h
    movepanel.cpp
    movepanel.h
    promotiondialog.cpp
    promotiondialog.h
)

qt_add_executable(ChessGUI
    WIN32 MACOSX_BUNDLE
    ${GUI_SOURCES}
    ${CHESS_LOGIC_SOURCES}
    ${CHESS_LOGIC_HEADERS}
)

target_include_directories(ChessGUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../chess_oops
)

target_link_libraries(ChessGUI
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS ChessGUI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ChessGUI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
