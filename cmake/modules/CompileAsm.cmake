get_filename_component(_sdkdir
"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]"
ABSOLUTE)

file(GLOB SDK_VERSIONS LIST_DIRECTORIES true "${_sdkdir}/Include/10.*.0")
list(SORT SDK_VERSIONS)
list(POP_BACK SDK_VERSIONS SDK_VERSION)

function(compile_asm OUT_VAR)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs SOURCES INCLUDE)
  cmake_parse_arguments(compile_asm "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN}
  )


  foreach(asm_source_file IN LISTS compile_asm_SOURCES)
    string(REPLACE .S .asm asm_temp_file ${asm_source_file})
    string(REPLACE .S .obj asm_obj_file ${asm_source_file})
    message("asm_temp_file: ${asm_temp_file}")
    message("asm_obj_file: ${asm_obj_file}")
    set(asm_source_file ${CMAKE_CURRENT_SOURCE_DIR}/${asm_source_file})
    set(asm_temp_file ${CMAKE_CURRENT_BINARY_DIR}/${asm_temp_file})
    set(asm_obj_file ${CMAKE_CURRENT_BINARY_DIR}/${asm_obj_file})
    list(APPEND obj_files ${asm_obj_file})
    add_custom_command(OUTPUT ${asm_obj_file}
      COMMAND
        ${CMAKE_C_COMPILER} /nologo /P /I. /I${SDK_VERSION}/shared /I${CMAKE_CURRENT_SOURCE_DIR}/include /I${CMAKE_CURRENT_BINARY_DIR}/include /I${CMAKE_CURRENT_SOURCE_DIR}/src/arm /I${CMAKE_CURRENT_SOURCE_DIR} /Fi:${asm_temp_file} ${asm_source_file}
      COMMAND
        ${CMAKE_ASM_ARMASM_COMPILER} /I${SDK_VERSION}/shared /I${CMAKE_CURRENT_SOURCE_DIR}/include /I${CMAKE_CURRENT_BINARY_DIR}/include /I${CMAKE_CURRENT_SOURCE_DIR}/src/arm /I${CMAKE_CURRENT_SOURCE_DIR} -ignore 4509 -o ${asm_obj_file} ${asm_temp_file}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  endforeach(asm_source_file IN LISTS compile_asm_SOURCES)
  set(${OUT_VAR} ${obj_files} PARENT_SCOPE) 
endfunction(compile_asm)