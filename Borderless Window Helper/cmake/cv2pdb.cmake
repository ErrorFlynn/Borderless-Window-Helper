find_program(CV2PDB cv2pdb)
if(CV2PDB)
  message(STATUS "cv2pdb: ${CV2PDB}")
else()
  message(WARNING "cv2pdb not found")
  return()
endif()

macro(add_cv2pdb_target pdb_target ext)
  if(CV2PDB)
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/${pdb_target}/")
    foreach(t IN ITEMS ${ARGN})
      add_custom_command(
#        OUTPUT "${pdb_target}/$<TARGET_FILE_NAME:${t}>" "${pdb_target}/$<TARGET_FILE_BASE_NAME:${t}>.pdb"
        OUTPUT "${pdb_target}/${t}${ext}" "${pdb_target}/${t}.pdb"
        COMMAND "${CV2PDB}" "$<TARGET_FILE:${t}>" "${PROJECT_BINARY_DIR}/${pdb_target}/$<TARGET_FILE_NAME:${t}>" "${PROJECT_BINARY_DIR}/${pdb_target}/$<TARGET_FILE_BASE_NAME:${t}>.pdb"
        DEPENDS ${t}
        COMMENT "creating PDB files for target ${t}")
      add_custom_target(${t}_pdb DEPENDS "${pdb_target}/$<TARGET_FILE_NAME:${t}>" "${pdb_target}/$<TARGET_FILE_BASE_NAME:${t}>.pdb")
      list(APPEND pdb_targets ${t}_pdb)
    endforeach()
    add_custom_target(${pdb_target} DEPENDS ${pdb_targets})
  endif()
endmacro()
