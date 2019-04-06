add_contract(agrio.bios agrio.bios ${CMAKE_CURRENT_SOURCE_DIR}/src/agrio.bios.cpp)
target_include_directories(agrio.bios.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(agrio.bios.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
