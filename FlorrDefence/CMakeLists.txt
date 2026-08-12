# CMakeList.txt : CMake project for FlorrDefence, include source and define
# project specific logic here.
#

file(GLOB SOURCES "src/*.cpp" "external/src/*.cpp")

add_executable(FlorrDefence ${SOURCES})
target_link_libraries(FlorrDefence PRIVATE SFML::Graphics)
target_include_directories(FlorrDefence PUBLIC 
    "${CMAKE_SOURCE_DIR}/SFML/include" 
    "${CMAKE_SOURCE_DIR}/json/include"
    "include"
    "external/include"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET FlorrDefence PROPERTY CXX_STANDARD 20)
endif()

add_custom_command(TARGET FlorrDefence POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res $<TARGET_FILE_DIR:FlorrDefence>/res
)
