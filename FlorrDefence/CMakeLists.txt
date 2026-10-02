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

target_compile_features(FlorrDefence PRIVATE cxx_std_20)

add_custom_command(TARGET FlorrDefence POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res $<TARGET_FILE_DIR:FlorrDefence>/res
)

if(FLORR_BUILD_TESTS)
    add_executable(EncounterTests "${CMAKE_SOURCE_DIR}/tests/encounter_tests.cpp")
    target_include_directories(EncounterTests PRIVATE "include" "${CMAKE_SOURCE_DIR}/json/include")
    target_compile_features(EncounterTests PRIVATE cxx_std_20)

    set(TEST_SOURCES ${SOURCES})
    list(FILTER TEST_SOURCES EXCLUDE REGEX "/main\\.cpp$")
    add_executable(SpawnRuntimeTests "${CMAKE_SOURCE_DIR}/tests/spawn_runtime_tests.cpp" ${TEST_SOURCES})
    target_link_libraries(SpawnRuntimeTests PRIVATE SFML::Graphics)
    target_include_directories(SpawnRuntimeTests PRIVATE "include" "external/include" "${CMAKE_SOURCE_DIR}/json/include")
    target_compile_features(SpawnRuntimeTests PRIVATE cxx_std_20)
    set(TEST_RUNTIME "${CMAKE_CURRENT_BINARY_DIR}/spawn-test-runtime")
    file(MAKE_DIRECTORY "${TEST_RUNTIME}")
    add_custom_command(TARGET SpawnRuntimeTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/res" "${TEST_RUNTIME}/res")
    add_test(NAME EncounterTests COMMAND EncounterTests)
    add_test(NAME SpawnRuntimeTests COMMAND SpawnRuntimeTests)
    set_tests_properties(EncounterTests SpawnRuntimeTests PROPERTIES WORKING_DIRECTORY "${TEST_RUNTIME}")
endif()
