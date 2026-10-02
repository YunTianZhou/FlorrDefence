if(CMAKE_HOST_WIN32)
  cmake_minimum_required(VERSION 3.24)
else()
  cmake_minimum_required(VERSION 3.22)
endif()

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()
 
project ("FlorrDefence")
option(FLORR_BUILD_TESTS "Build encounter progression and runtime tests" OFF)
if(FLORR_BUILD_TESTS)
  enable_testing()
endif()

set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)

add_subdirectory ("SFML")
add_subdirectory ("json")
add_subdirectory ("FlorrDefence")
