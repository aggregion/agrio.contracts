add_contract(agrio.system agrio.system ${CMAKE_CURRENT_SOURCE_DIR}/src/agrio.system.cpp)
#add_executable(agrio.system.wasm ${CMAKE_CURRENT_SOURCE_DIR}/src/agrio.system.cpp)
target_include_directories(agrio.system.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/../agrio.token/include)

set_target_properties(agrio.system.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
