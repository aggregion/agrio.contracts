add_contract(agrio.wrap agrio.wrap ${CMAKE_CURRENT_SOURCE_DIR}/src/agrio.wrap.cpp)
target_include_directories(agrio.wrap.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(agrio.wrap.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
