add_contract(agrio.token agrio.token ${CMAKE_CURRENT_SOURCE_DIR}/src/agrio.token.cpp)
target_include_directories(agrio.token.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(agrio.token.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
