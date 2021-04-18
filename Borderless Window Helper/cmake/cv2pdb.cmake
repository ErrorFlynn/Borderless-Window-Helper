include_guard()

find_program(CV2PDB cv2pdb)
if(CV2PDB)
  message(STATUS "cv2pdb: ${CV2PDB}")
else()
  message(WARNING "cv2pdb not found")
  return()
endif()

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/pdb/")

macro(add_cv2pdb_target t new_exe_file pdb_file)
  if(CV2PDB)
    add_custom_command(
      # OUTPUT "pdb/$<TARGET_FILE_NAME:${t}>" "pdb/$<TARGET_FILE_BASE_NAME:${t}>.pdb"
      OUTPUT "pdb/${new_exe_file}" "pdb/${pdb_file}"
      COMMAND "${CV2PDB}" "$<TARGET_FILE:${t}>" "${PROJECT_BINARY_DIR}/pdb/$<TARGET_FILE_NAME:${t}>" "${PROJECT_BINARY_DIR}/pdb/$<TARGET_FILE_BASE_NAME:${t}>.pdb"
      DEPENDS ${t}
      COMMENT "creating PDB files for target ${t}")
    add_custom_target(${t}_pdb DEPENDS "pdb/$<TARGET_FILE_NAME:${t}>" "pdb/$<TARGET_FILE_BASE_NAME:${t}>.pdb")
  endif()
endmacro()
