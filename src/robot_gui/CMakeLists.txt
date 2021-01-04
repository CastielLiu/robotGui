cmake_minimum_required(VERSION 2.8.0)
project(robot_gui)

#添加c++11的支持
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# qt_build provides the qt cmake glue, roscpp the comms for a default talker
find_package(catkin REQUIRED 
	COMPONENTS rviz roscpp sensor_msgs cv_bridge std_msgs image_transport roscpp
)

find_package(Qt5 REQUIRED Core Widgets Multimedia MultimediaWidgets SerialPort)
set(QT_LIBRARIES Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets Qt5::SerialPort)
include_directories(${catkin_INCLUDE_DIRS} include/)

find_package(OpenCV REQUIRED)
include_directories(  ${catkin_INCLUDE_DIRS}  ${OpenCV_INCLUDE_DIRS} )

catkin_package()

# Qt Environment
file(GLOB QT_FORMS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ui/*.ui qrviz/*.ui)
file(GLOB QT_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} resources/*.qrc qrviz/*.qrc)
file(GLOB_RECURSE QT_MOC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS include/*.hpp include/*.h *.hpp *.h qrviz/*.h qnode/*.h)

QT5_ADD_RESOURCES(QT_RESOURCES_CPP ${QT_RESOURCES})
QT5_WRAP_UI(QT_FORMS_HPP ${QT_FORMS})
QT5_WRAP_CPP(QT_MOC_HPP ${QT_MOC})

# Sources
file(GLOB_RECURSE QT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS src/*.cpp *.cpp qrviz/*.cpp qnode/*.cpp)

# excutable
add_executable(robot_gui ${QT_SOURCES} ${QT_RESOURCES_CPP} ${QT_FORMS_HPP} ${QT_MOC_HPP})
target_link_libraries(robot_gui ${QT_LIBRARIES} ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})
install(TARGETS robot_gui RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

