function(ocpppp_generate TARGET_NAME WORKING_DIRECTORY COMPILE_COMMANDS_DIR INPUT_FILES) 
  set(OCPPPP_GENERATED_FILES "")
  set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/${TARGET_NAME}")
  foreach(file IN LISTS INPUT_FILES)
    string(REGEX REPLACE "^(.*)\\.([^.]+)$" "\\1.gen.\\2" OUT_FILE "${OUTPUT_DIR}/${file}")
    add_custom_command(OUTPUT "${OUT_FILE}"
      COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT_FILE}" || ${CMAKE_COMMAND} -E true
      COMMAND ${CMAKE_COMMAND} -E "rm" "-rf" "${OUT_FILE}"
      COMMAND ocpppp "${WORKING_DIRECTORY}/${file}" "${OUT_FILE}" "--database_dir" "${COMPILE_COMMANDS_DIR}" "--std" "c++20"
      COMMAND "clang-format" "-i" "${OUT_FILE}"
      DEPENDS "${WORKING_DIRECTORY}/${file}" "${COMPILE_COMMANDS_DIR}/compile_commands.json"
    )
    list(APPEND OCPPPP_GENERATED_FILES "${OUT_FILE}")
  endforeach()
  add_library(${TARGET_NAME} INTERFACE)
  target_include_directories(${TARGET_NAME} INTERFACE "${OUTPUT_DIR}")

  add_custom_target(ocpppp_${TARGET_NAME}_headers ALL DEPENDS "${OCPPPP_GENERATED_FILES}")
  add_dependencies(${TARGET_NAME} ocpppp_${TARGET_NAME}_headers)
endfunction(ocpppp_generate)
