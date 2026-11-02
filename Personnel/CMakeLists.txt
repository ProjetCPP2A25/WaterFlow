cmake_minimum_required(VERSION 3.16)
project(WaterFlowPersonnel VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)
qt_standard_project_setup()

qt_add_executable(WaterFlowPersonnel
    main.cpp
    personnel.cpp personnel.h personnel.ui
    barchart.cpp barchart.h
    connection.cpp connection.h
    employe.cpp employe.h
)
target_link_libraries(WaterFlowPersonnel PRIVATE Qt6::Widgets Qt6::Sql)
