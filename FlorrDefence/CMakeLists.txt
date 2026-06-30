# CMakeList.txt : CMake project for TowerDefence, include source and define
# project specific logic here.
#

file(GLOB SOURCES "src/*.cpp")

add_executable(TowerDefence ${SOURCES})
target_link_libraries(TowerDefence PRIVATE SFML::Graphics)
target_include_directories(TowerDefence PUBLIC 
    "${CMAKE_SOURCE_DIR}/SFML/include" 
    "${CMAKE_SOURCE_DIR}/json/include"
    "include"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TowerDefence PROPERTY CXX_STANDARD 20)
endif()

add_custom_command(TARGET TowerDefence POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res $<TARGET_FILE_DIR:TowerDefence>/res
)
