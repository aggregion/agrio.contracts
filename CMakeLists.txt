cmake_minimum_required(VERSION 3.5)
project(agrio_contracts VERSION 1.5.2)

set(AGRIO_CDT_VERSION_MIN "1.4")
set(AGRIO_CDT_VERSION_SOFT_MAX "1.5")
#set(AGRIO_CDT_VERSION_HARD_MAX "")

find_package(agrio.cdt)

### Check the version of agrio.cdt
set(VERSION_MATCH_ERROR_MSG "")
AGRIO_CHECK_VERSION(VERSION_OUTPUT "${AGRIO_CDT_VERSION}"
                                   "${AGRIO_CDT_VERSION_MIN}"
                                   "${AGRIO_CDT_VERSION_SOFT_MAX}"
                                   "${AGRIO_CDT_VERSION_HARD_MAX}"
                                   VERSION_MATCH_ERROR_MSG)
if(VERSION_OUTPUT STREQUAL "MATCH")
   message(STATUS "Using agrio.cdt version ${AGRIO_CDT_VERSION}")
elseif(VERSION_OUTPUT STREQUAL "WARN")
   message(WARNING "Using agrio.cdt version ${AGRIO_CDT_VERSION} even though it exceeds the maximum supported version of ${AGRIO_CDT_VERSION_SOFT_MAX}; continuing with configuration, however build may fail.\nIt is recommended to use agrio.cdt version ${AGRIO_CDT_VERSION_SOFT_MAX}.x")
else() # INVALID OR MISMATCH
   message(FATAL_ERROR "Found agrio.cdt version ${AGRIO_CDT_VERSION} but it does not satisfy version requirements: ${VERSION_MATCH_ERROR_MSG}\nPlease use agrio.cdt version ${AGRIO_CDT_VERSION_SOFT_MAX}.x")
endif(VERSION_OUTPUT STREQUAL "MATCH")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   set(TEST_BUILD_TYPE "Debug")
   set(CMAKE_BUILD_TYPE "Release")
else()
   set(TEST_BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()


add_subdirectory(agrio.bios)
add_subdirectory(agrio.msig)
add_subdirectory(agrio.wrap)
add_subdirectory(agrio.system)
add_subdirectory(agrio.token)

if (APPLE)
   set(OPENSSL_ROOT "/usr/local/opt/openssl")
elseif (UNIX)
   set(OPENSSL_ROOT "/usr/include/openssl")
endif()
set(SECP256K1_ROOT "/usr/local")

include(UnitTestsExternalProject.txt)
