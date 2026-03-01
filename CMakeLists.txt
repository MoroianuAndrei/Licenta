cmake_minimum_required(VERSION 3.19)
project(Licenta LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(Licenta
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    backend/core/node.h
    backend/core/node.cpp
    backend/core/edge.h
    backend/core/edge.cpp
    backend/core/graph.h
    backend/core/graph.cpp
    backend/algorithms/AMCMCPEP.h
    backend/algorithms/AMCMCPEP.cpp
    backend/algorithms/AMCPEBCP.h
    backend/algorithms/AMCPEBCP.cpp
    backend/algorithms/LinearFunc.h
    backend/algorithms/LinearFunc.cpp
    backend/algorithms/ParametricSearch.h
    backend/algorithms/ParametricSearch.cpp
)

target_include_directories(Licenta
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/core
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/algorithms
)

target_link_libraries(Licenta
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS Licenta
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Licenta
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
