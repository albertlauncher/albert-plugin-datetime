cmake_minimum_required(VERSION 3.16)
project(datetime VERSION 7.6.0)

find_package(Albert REQUIRED)

albert_plugin(QT Widgets)

option(BUILD_TESTS "Build tests (Requires QTest)" OFF)
if (BUILD_TESTS)
    include(CTest)
    find_package(Qt6 REQUIRED COMPONENTS Test)

    get_target_property(LIBS_TST ${PROJECT_NAME} LINK_LIBRARIES)
    get_target_property(DEFS_TST ${PROJECT_NAME} COMPILE_DEFINITIONS)

    set(TARGET_TST ${PROJECT_NAME}_test)
    add_executable(${TARGET_TST}
        test/timer_test.cpp
        src/items.h
        src/items.cpp
        src/plugin.h
        src/plugin.cpp
        src/configwidget.ui
    )
    # The binary dir holds the generated metadata.json required by ALBERT_PLUGIN
    target_include_directories(${TARGET_TST} PRIVATE src ${PROJECT_BINARY_DIR})
    target_link_libraries(${TARGET_TST} PRIVATE ${LIBS_TST} Qt6::Test)
    target_compile_definitions(${TARGET_TST} PRIVATE ${DEFS_TST})
    set_target_properties(${TARGET_TST} PROPERTIES AUTOMOC ON AUTOUIC ON)
    set_property(TARGET ${TARGET_TST} APPEND PROPERTY AUTOMOC_MACRO_NAMES "ALBERT_PLUGIN")
    add_test(NAME ${TARGET_TST} COMMAND ${TARGET_TST})
endif()
