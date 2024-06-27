cmake_minimum_required(VERSION 3.10)

if (DEFINED CMAKE_TOOLCHAIN_FILE)
        include(${CMAKE_TOOLCHAIN_FILE})
endif()

project(schunk_svh_library VERSION 1.0 DESCRIPTION "C++ driver library for the Schunk SVH")

# Check if the target platform is Windows and define _SYSTEM_WIN32_
if(WIN32)
        add_definitions(-D_SYSTEM_WIN32_)
endif()

# CMake will automatically update this library's version in the SVHVersionConfig.h include.
configure_file(include/schunk_svh_library/VersionConfig.h.in SVHVersionConfig.h)

# We require this C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# System dependencies
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

# --------------------------------------------------------------------------------
# RPATH settings
# --------------------------------------------------------------------------------
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")

# --------------------------------------------------------------------------------
# Internal libraries
# --------------------------------------------------------------------------------
add_library(svh-serial SHARED
        src/LogHandler.cpp
        src/serial/ByteOrderConversion.cpp
        src/serial/Serial.cpp
        src/serial/SerialFlags.cpp
        src/serial/SVHReceiveThread.cpp
        src/serial/SVHSerialInterface.cpp
        src/serial/SVHSerialPacket.cpp
)

add_library(Schunk::svh-serial ALIAS svh-serial)

target_include_directories(svh-serial PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(svh-serial PUBLIC
        ${CMAKE_THREAD_LIBS_INIT}
)

target_compile_definitions(svh-serial PUBLIC
        -D_SYSTEM_WINDOWS_
        -DDRIVER_SVH_EXPORT_SYMBOLS
)

if(THREADS_HAVE_PTHREAD_ARG)
        target_compile_options(svh-serial PUBLIC "-pthread")
endif()

# --------------------------------------------------------------------------------
# User library
# --------------------------------------------------------------------------------
add_library(svh-library SHARED
        src/control/SVHController.cpp
        src/control/SVHFingerManager.cpp
        src/control/FingerManagerWrapper.cpp
)

add_library(Schunk::svh-library ALIAS svh-library)

target_include_directories(svh-library PUBLIC
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(svh-library PUBLIC
        Schunk::svh-serial
)

target_compile_definitions(svh-library PUBLIC
        -D_SYSTEM_WINDOWS_
        -DDRIVER_SVH_EXPORT_SYMBOLS
)

# --------------------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------------------
add_executable(test_driver_svh
        test/driver_svh/MainTest.cpp
        test/driver_svh/ByteOrderConversionTest.cpp
        test/driver_svh/SVHDriverTest.cpp
)

target_include_directories(test_driver_svh PUBLIC include)

target_link_libraries(test_driver_svh
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        Schunk::svh-library
)

target_compile_definitions(test_driver_svh PUBLIC
        -D_SYSTEM_WINDOWS_
        -DBOOST_TEST_DYN_LINK
)
add_test(NAME test_driver_svh COMMAND test_driver_svh)

add_executable(test_svh_send_packet
        test/serial_interface/SVHSendPacketTest.cpp
)

target_include_directories(test_svh_send_packet PUBLIC include)

target_link_libraries(test_svh_send_packet
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        Schunk::svh-serial
)
add_test(NAME test_svh_send_packet COMMAND test_svh_send_packet)

add_executable(test_svh_receive
        test/serial_interface/SVHReceiveTest.cpp
)

target_include_directories(test_svh_receive PUBLIC include)

target_link_libraries(test_svh_receive
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        Schunk::svh-serial
)
add_test(NAME test_svh_receive COMMAND test_svh_receive)
set_tests_properties(test_svh_receive PROPERTIES TIMEOUT 10)

add_executable(test_svh_send_feedback_packet
        test/serial_interface/SVHSendFeedbackPacketTest.cpp
)

target_include_directories(test_svh_send_feedback_packet PUBLIC include)

target_link_libraries(test_svh_send_feedback_packet
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        Schunk::svh-serial
)
add_test(NAME test_svh_send_feedback_packet COMMAND test_svh_send_feedback_packet)

add_executable(test_svh_controller_init_send
        test/serial_interface/SVHControllerInitSendTest.cpp
)

target_include_directories(test_svh_controller_init_send PUBLIC include)

target_link_libraries(test_svh_controller_init_send
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        Schunk::svh-library
)
add_test(NAME test_svh_controller_init_send COMMAND test_svh_controller_init_send)

add_executable(test_svh_finger_manager
        test/serial_interface/SVHFingerManagerTest.cpp
)

target_include_directories(test_svh_finger_manager PUBLIC include)

target_link_libraries(test_svh_finger_manager
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        Schunk::svh-library
)
add_test(NAME test_svh_finger_manager COMMAND test_svh_finger_manager)

add_executable(my_test
        test/serial_interface/main.cpp
)

target_include_directories(my_test PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(my_test
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        Schunk::svh-library
)
add_test(NAME my_test COMMAND my_test)

enable_testing()

# --------------------------------------------------------------------------------
# Export and install
# --------------------------------------------------------------------------------
install(TARGETS
        svh-library
        svh-serial
        EXPORT schunk_svh_library_targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

install(EXPORT schunk_svh_library_targets
        DESTINATION lib/cmake/schunk_svh_library
        FILE schunk_svh_libraryTargets.cmake
        NAMESPACE Schunk::
)

install(FILES schunk_svh_libraryConfig.cmake
        DESTINATION lib/cmake/schunk_svh_library
)

install(DIRECTORY include/schunk_svh_library
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)

# Allows Colcon to find non-Ament packages when using workspace underlays
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/share/ament_index/resource_index/packages/${PROJECT_NAME} "")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/share/ament_index/resource_index/packages/${PROJECT_NAME} DESTINATION share/ament_index/resource_index/packages)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/share/${PROJECT_NAME}/hook/ament_prefix_path.dsv "prepend-non-duplicate;AMENT_PREFIX_PATH;")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/share/${PROJECT_NAME}/hook/ament_prefix_path.dsv DESTINATION share/${PROJECT_NAME}/hook)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/share/${PROJECT_NAME}/hook/ros_package_path.dsv "prepend-non-duplicate;ROS_PACKAGE_PATH;")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/share/${PROJECT_NAME}/hook/ros_package_path.dsv DESTINATION share/${PROJECT_NAME}/hook)
