cmake_minimum_required( VERSION 3.5 )

set(AGRIO_VERSION_MIN "1.4")
set(AGRIO_VERSION_SOFT_MAX "1.6")
#set(AGRIO_VERSION_HARD_MAX "")

find_package(agrio)

### Check the version of agrio
set(VERSION_MATCH_ERROR_MSG "")
AGRIO_CHECK_VERSION(VERSION_OUTPUT "${AGRIO_VERSION}"
                                   "${AGRIO_VERSION_MIN}"
                                   "${AGRIO_VERSION_SOFT_MAX}"
                                   "${AGRIO_VERSION_HARD_MAX}"
                                   VERSION_MATCH_ERROR_MSG)
if(VERSION_OUTPUT STREQUAL "MATCH")
   message(STATUS "Using agrio version ${AGRIO_VERSION}")
elseif(VERSION_OUTPUT STREQUAL "WARN")
   message(WARNING "Using agrio version ${AGRIO_VERSION} even though it exceeds the maximum supported version of ${AGRIO_VERSION_SOFT_MAX}; continuing with configuration, however build may fail.\nIt is recommended to use agrio version ${AGRIO_VERSION_SOFT_MAX}.x")
else() # INVALID OR MISMATCH
   message(FATAL_ERROR "Found agrio version ${AGRIO_VERSION} but it does not satisfy version requirements: ${VERSION_MATCH_ERROR_MSG}\nPlease use agrio version ${AGRIO_VERSION_SOFT_MAX}.x")
endif(VERSION_OUTPUT STREQUAL "MATCH")


enable_testing()

configure_file(${CMAKE_SOURCE_DIR}/contracts.hpp.in ${CMAKE_BINARY_DIR}/contracts.hpp)

include_directories(${CMAKE_BINARY_DIR})

file(GLOB UNIT_TESTS "*.cpp" "*.hpp")

add_agrio_test( unit_test ${UNIT_TESTS} )
