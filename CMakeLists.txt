cmake_minimum_required(VERSION 3.14)

project(dav1d VERSION 0.6.0 LANGUAGES C ASM)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
find_program(nasm REQUIRED)

include(CheckSymbolExists)
include(CompileAsm)
check_symbol_exists(_X86_ "Windows.h" _X86_)
check_symbol_exists(_AMD64_ "Windows.h" _AMD64_)
check_symbol_exists(_ARM_ "Windows.h" _ARM_)
check_symbol_exists(_ARM64_ "Windows.h" _ARM64_)

set(PROJECT_VERSION_REVISION ${PROJECT_VERSION_PATCH})
set(API_VERSION_MAJOR 4)
set(API_VERSION_MINOR 0)
set(API_VERSION_REVISION 0)
set(COPYRIGHT_YEARS 2019)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/dav1d.rc.in
               ${CMAKE_CURRENT_BINARY_DIR}/dav1d.rc)

include(FindGit)
if(GIT_EXECUTABLE AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/.git)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
       OUTPUT_VARIABLE OUT RESULT_VARIABLE RES)
   IF(RES EQUAL 0)
    string(REGEX REPLACE "\n$" "" VCS_TAG "${OUT}")
  endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/vcs_version.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/include/vcs_version.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/dav1d/version.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/include/dav1d/version.h)

set(STACK_ALIGNMENT 0)
set(ARCH_AARCH64 0)
if(_ARM64_)
  set(ARCH_AARCH64 1)
endif()
set(ARCH_ARM 0)
if(_ARM_)
  set(ARCH_ARM 1)
endif()
set(ARCH_X86 0)
set(HAVE_ASM 0)
if(_X86_ OR _AMD64_)
  set(ARCH_X86 1)
  set(HAVE_ASM 1)
endif()
set(ARCH_X86_32 0)
if(_X86_)
  set(ARCH_X86_32 1)
  set(STACK_ALIGNMENT 4)
endif()
set(ARCH_X86_64 0)
if(_AMD64_)
  set(ARCH_X86_64 1)
  set(STACK_ALIGNMENT 16)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.cmakein
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.asm.cmakein
               ${CMAKE_CURRENT_BINARY_DIR}/config.asm)

# libdav1d source files
set(libdav1d_sources
    src/lib.c
    src/thread_task.c
    src/cdf.c
    src/cpu.c
    src/data.c
    src/decode.c
    src/dequant_tables.c
    src/getbits.c
    src/intra_edge.c
    src/itx_1d.c
    src/lf_mask.c
    src/log.c
    src/msac.c
    src/obu.c
    src/picture.c
    src/qm.c
    src/ref.c
    src/ref_mvs.c
    src/scan.c
    src/tables.c
    src/warpmv.c
    src/wedge.c
    src/win32/thread.c
    ${CMAKE_CURRENT_BINARY_DIR}/dav1d.rc
)

# libdav1d bitdepth source files
# These files are compiled for each bitdepth with
# `BITDEPTH` defined to the currently built bitdepth.
set(libdav1d_tmpl_sources
    src/cdef_apply_tmpl.c
    src/cdef_tmpl.c
    src/fg_apply_tmpl.c
    src/film_grain_tmpl.c
    src/ipred_prepare_tmpl.c
    src/ipred_tmpl.c
    src/itx_tmpl.c
    src/lf_apply_tmpl.c
    src/loopfilter_tmpl.c
    src/looprestoration_tmpl.c
    src/lr_apply_tmpl.c
    src/mc_tmpl.c
    src/recon_tmpl.c
)

set(libdav1d_arch_tmpl_sources)

set(libdav1d_bitdepth_objs)

# libdav1d entrypoint source files
# These source files contain library entry points and are
# built with the stack-realign flag set, where necessary.
set(libdav1d_entrypoints_sources
    src/lib.c
    src/thread_task.c
)

# ASM specific sources
set(libdav1d_nasm_objs)
# Arch-specific flags
set(arch_flags)
if (_ARM64_ OR _ARM_)
  # enable_language(ASM_ARMASM)
  # list(APPEND libdav1d_sources src/arm/cpu.c)
  # list(APPEND libdav1d_tmpl_sources
  #     src/arm/cdef_init_tmpl.c
  #     src/arm/ipred_init_tmpl.c
  #     src/arm/itx_init_tmpl.c
  #     src/arm/loopfilter_init_tmpl.c
  #     src/arm/looprestoration_init_tmpl.c
  #     src/arm/mc_init_tmpl.c
  # )
  # if(_ARM64_)
  #   list(APPEND libdav1d_sources_asm
  #     src/arm/64/looprestoration_common.S
  #     src/arm/64/msac.S
  #     src/arm/64/cdef.S
  #     src/arm/64/ipred.S
  #     src/arm/64/itx.S
  #     src/arm/64/loopfilter.S
  #     src/arm/64/looprestoration.S
  #     src/arm/64/mc.S
  #     src/arm/64/cdef16.S
  #     src/arm/64/loopfilter16.S
  #     src/arm/64/looprestoration16.S
  #     src/arm/64/mc16.S
  #   )
  # elseif(_ARM_)
  #   list(APPEND libdav1d_sources_asm
  #     src/arm/32/cdef.S
  #     src/arm/32/ipred.S
  #     src/arm/32/loopfilter.S
  #     src/arm/32/looprestoration.S
  #     src/arm/32/mc.S
  #   )
  # endif()
  # compile_asm(libdav1d_nasm_objs
  #   SOURCES
  #     ${libdav1d_sources_asm})
elseif(_X86_ OR _AMD64_)
  enable_language(ASM_NASM)
  list(APPEND libdav1d_sources src/x86/cpu.c)

  if(_AMD64_)
    list(APPEND libdav1d_sources src/x86/msac_init.c)
  endif()

  list(APPEND libdav1d_tmpl_sources
    src/x86/cdef_init_tmpl.c
    src/x86/film_grain_init_tmpl.c
    src/x86/ipred_init_tmpl.c
    src/x86/itx_init_tmpl.c
    src/x86/loopfilter_init_tmpl.c
    src/x86/looprestoration_init_tmpl.c
    src/x86/mc_init_tmpl.c
  )

  if(_AMD64_)
    list(APPEND libdav1d_sources_asm
      src/x86/cdef.asm
      src/x86/film_grain.asm
      src/x86/mc.asm
      src/x86/ipred.asm
      src/x86/itx.asm
      src/x86/loopfilter.asm
      src/x86/looprestoration.asm
    )
  endif()

  # NASM source files
  list(APPEND libdav1d_sources_asm
    src/x86/cpuid.asm
    src/x86/cdef_sse.asm
    src/x86/film_grain_ssse3.asm
    src/x86/ipred_ssse3.asm
    src/x86/itx_ssse3.asm
    src/x86/loopfilter_ssse3.asm
    src/x86/looprestoration_ssse3.asm
    src/x86/mc_ssse3.asm
    src/x86/msac.asm
  )
endif()

add_library(dav1d SHARED
  ${libdav1d_sources}
  ${libdav1d_sources_asm}
  ${libdav1d_nasm_objs}
)

add_library(dav1d_bitdepth_8 STATIC
  ${libdav1d_tmpl_sources}
  ${libdav1d_arch_tmpl_sources}
)

add_library(dav1d_bitdepth_16 STATIC
  ${libdav1d_tmpl_sources}
  ${libdav1d_arch_tmpl_sources}
)

target_compile_definitions(dav1d
  PRIVATE
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_WARNINGS
    __PRETTY_FUNCTION__=__FUNCTION__
    DAV1D_BUILDING_DLL
)

target_compile_definitions(dav1d_bitdepth_8
  PRIVATE
    BITDEPTH=8
    _CRT_SECURE_NO_WARNINGS
)
target_compile_definitions(dav1d_bitdepth_16
  PRIVATE
    BITDEPTH=16
    _CRT_SECURE_NO_WARNINGS
)
if(_X86_)
  target_compile_definitions(dav1d
    PRIVATE
      PREFIX=1
  )

  target_compile_definitions(dav1d_bitdepth_8
    PRIVATE
      PREFIX=1
  )
  target_compile_definitions(dav1d_bitdepth_16
    PRIVATE
      PREFIX=1
  )
endif()

if(WINDOWS_STORE)
  target_compile_definitions(dav1d
    PRIVATE
      MS_APP
  )
else()
target_compile_definitions(dav1d
  PRIVATE
  _WIN32_IE=0x0600
  _WIN32_WINNT=0x0600
)
target_link_libraries(dav1d
    PRIVATE
    shell32.lib
  )
endif()

target_compile_options(dav1d
  PRIVATE
    /sdl-
)

target_compile_options(dav1d_bitdepth_8
  PRIVATE
    /sdl-
)

target_compile_options(dav1d_bitdepth_16
  PRIVATE
    /sdl-
)

target_link_libraries(dav1d
  PRIVATE
    kernel32.lib
    dav1d_bitdepth_8
    dav1d_bitdepth_16
)


target_link_options(dav1d
  PRIVATE
    /debug:full
    /sdl-
)

target_include_directories(dav1d_bitdepth_16
  PRIVATE
  $<BUILD_INTERFACE:.;cmake;src;src/util;${CMAKE_CURRENT_BINARY_DIR};include;include/compat/msvc;include/dav1d>
  INTERFACE
  $<INSTALL_INTERFACE:include/dav1d>
)

target_include_directories(dav1d_bitdepth_8
  PRIVATE
  $<BUILD_INTERFACE:.;cmake;src;src/util;${CMAKE_CURRENT_BINARY_DIR};include;include/compat/msvc;include/dav1d>
  INTERFACE
  $<INSTALL_INTERFACE:include/dav1d>
)
  
target_include_directories(dav1d
  PRIVATE
  $<BUILD_INTERFACE:.;cmake;src;src/util;${CMAKE_CURRENT_BINARY_DIR};include;include/compat/msvc;include/dav1d>
  INTERFACE
  $<INSTALL_INTERFACE:include/dav1d>
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)

if(MSVC)
  set_target_properties(dav1d
    PROPERTIES
      COMPILE_PDB_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
      COMPILE_PDB_NAME libdav1d
      PDB_NAME libdav1d
      OUTPUT_NAME libdav1d
  )
  install(FILES
    ${PROJECT_BINARY_DIR}/RelWithDebInfo/libdav1d.pdb
    DESTINATION lib
    CONFIGURATIONS RelWithDebInfo
  )
  install(FILES
    ${PROJECT_BINARY_DIR}/Debug/libdav1d.pdb
    DESTINATION lib
    CONFIGURATIONS Debug
  )
endif()

install(EXPORT ${PROJECT_NAME}
  FILE
    ${PROJECT_NAME}.cmake
  NAMESPACE
    ${PROJECT_NAME}::
  DESTINATION
    lib/cmake/${PROJECT_NAME}
)

install(
  FILES
    cmake/${PROJECT_NAME}-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  DESTINATION
    lib/cmake/${PROJECT_NAME}
)
