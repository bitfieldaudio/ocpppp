function(ocpppp_generate TARGET_NAME WORKING_DIRECTORY INPUT_FILES) 
  set(OCPPPP_GENERATED_FILES "")
  set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/${TARGET_NAME}")
  foreach(file IN LISTS INPUT_FILES)
    string(REGEX REPLACE "^(.*)\\.([^.]+)$" "\\1.gen.\\2" OUT_FILE "${OUTPUT_DIR}/${file}")
    add_custom_command(OUTPUT "${OUT_FILE}"
      COMMAND "mkdir" "-p" "${OUT_FILE}"
      COMMAND "rm" "-rf" "${OUT_FILE}"
      COMMAND ocpppp "${file}" "${OUT_FILE}"
      COMMAND "clang-format" "-i" "${OUT_FILE}"
      DEPENDS ocpppp "${WORKING_DIRECTORY}/${file}" 
      WORKING_DIRECTORY "${WORKING_DIRECTORY}"
    )
    list(APPEND OCPPPP_GENERATED_FILES "${OUT_FILE}")
  endforeach()
  add_library(${TARGET_NAME} INTERFACE)
  target_include_directories(${TARGET_NAME} INTERFACE "${OUTPUT_DIR}")
  target_link_libraries(${TARGET_NAME} INTERFACE ocpppp_lib)

  add_custom_target(ocpppp_${TARGET_NAME}_headers ALL DEPENDS "${OCPPPP_GENERATED_FILES}")
  add_dependencies(${TARGET_NAME} ocpppp_${TARGET_NAME}_headers)
endfunction(ocpppp_generate)
