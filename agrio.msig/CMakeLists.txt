add_contract(agrio.msig agrio.msig ${CMAKE_CURRENT_SOURCE_DIR}/src/agrio.msig.cpp)
target_include_directories(agrio.msig.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(agrio.msig.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
