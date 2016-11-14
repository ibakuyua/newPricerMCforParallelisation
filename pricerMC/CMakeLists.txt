cmake_minimum_required(VERSION 2.8)
project(mc-pricer CXX)

# Prefix
if (NOT PREFIX)
    set(PREFIX ${CMAKE_CURRENT_BINARY_DIR})
endif (NOT PREFIX)
set(CMAKE_INSTALL_PREFIX ${PREFIX} CACHE STRING "Installation prefix." FORCE)

if (CXX)
    set(CMAKE_CXX_COMPILER ${CXX})
endif (CXX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Release or Debug
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif ()


# Detect Boost
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
message(STATUS "Boost include directory ${Boost_INCLUDE_DIR}")

# Detect PNL
find_package(Pnl REQUIRED)
set(LIBS ${LIBS} ${PNL_LIBRARIES})
include_directories(${PNL_INCLUDE_DIRS})
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions(-DPNL_RANGE_CHECK_OFF)
endif()

# Add math lib
set(LIBS ${LIBS} m)
# Testing the parser

# Testing pnl Library example

#Testing BlackScholesModel
add_executable(BSModelAsset_t-test.te src/tests/test_BlackScholesAsset_t.cpp src/BlackScholesModel.cpp)
target_link_libraries(BSModelAsset_t-test.te ${LIBS})
add_executable(BSModelAssetShift.te src/tests/test_BlackScholesAssetShift.cpp src/BlackScholesModel.cpp)
target_link_libraries(BSModelAssetShift.te ${LIBS})
add_executable(BSModelDelta-test0.te src/tests/test_delta0.cpp src/Options/OptionBasket.cpp src/Options/Option.cpp src/BlackScholesModel.cpp src/MonteCarlo.cpp)
target_link_libraries(BSModelDelta-test0.te ${LIBS})
add_executable(BSModelDelta-test_t.te src/tests/test_delta_t.cpp src/Options/OptionBasket.cpp src/Options/Option.cpp src/BlackScholesModel.cpp src/MonteCarlo.cpp)
target_link_libraries(BSModelDelta-test_t.te ${LIBS})
#Testing MonteCarlo
add_executable(MonteCarloPrice0-test.te src/tests/test_MonteCarloPrice0.cpp src/MonteCarlo.cpp src/BlackScholesModel.cpp src/Options/OptionBasket.cpp src/Options/Option.cpp)
target_link_libraries(MonteCarloPrice0-test.te ${LIBS})
add_executable(MonteCarloPrice_t-test.te src/tests/test_MonteCarloPrice_t.cpp src/MonteCarlo.cpp src/BlackScholesModel.cpp src/Options/OptionBasket.cpp src/Options/Option.cpp)
target_link_libraries(MonteCarloPrice_t-test.te ${LIBS})
#Testing Options

#Testing Pricer

add_executable(pricer src/parser.cpp src/pricer.cpp src/Options/Option.cpp src/BlackScholesModel.cpp src/MonteCarlo.cpp src/Options/OptionBasket.cpp src/Options/AsianOption.cpp src/Options/PerformanceOption.cpp)
target_link_libraries(pricer ${LIBS})
install(TARGETS pricer DESTINATION bin)


# Print compiler flags
get_directory_property(DirDefs COMPILE_DEFINITIONS)
message(STATUS "COMPILE_DEFINITIONS = ${DirDefs}")
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_LINKER_EXE_FLAGS}" )
else()
    message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_LINKER_EXE_FLAGS}" )
endif()
