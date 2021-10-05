file(GLOB example_cppfiles "example_*.C")
add_compile_definitions(USE_RUN_CODE=1)

foreach(example_exefile ${example_cppfiles})
  #WE for without extension
  get_filename_component(example_exe "${example_exefile}" NAME_WE)
  add_executable(${example_exe} ${example_exefile})
  target_link_libraries(${example_exe} PUBLIC "-rdynamic" PRIVATE hobbes ${Readline_LIBRARIES})
  install(TARGETS ${example_exe} DESTINATION bin)
endforeach()
