# FirmGen CoreMark benchmark build.
#
# Loaded with -DCMAKE_PROJECT_INCLUDE=<this file> when FirmGen configures the project's OWN board
# CMake into a SEPARATE build directory. Nothing in the project is edited and its normal build
# directory is untouched: the toolchain, flags, HAL, startup code, clock setup and linker script are
# exactly the project's, and only the application is swapped for CoreMark.
#
# CMAKE_PROJECT_INCLUDE runs straight after project(), before the project has added any sources,
# so the swap is deferred to the end of the top-level directory, after every target_sources call.
#
#   FIRMGEN_COREMARK_DIR           CoreMark sources and the port, staged by FirmGen
#   FIRMGEN_COREMARK_PROJECT_ROOT  the project (application/, firmware/, user/, ...)
#   FIRMGEN_COREMARK_BOARD_DIR     the board directory inside it (ST's generated code, start-up)
#
# FirmGen's board tests build their images through this file too (src/tools/target_tests.rs):
#   FIRMGEN_COREMARK_EXTRA_SOURCES   project sources under test and the case's harness, compiled
#                                    from where they are
#   FIRMGEN_COREMARK_EXTRA_INCLUDES  the case's include folders
#   FIRMGEN_COREMARK_KEEP_HAL        classic projects: keep the vendor HAL (Drivers/), the MSP file
#                                    and the C library stubs, not only start-up and SystemInit

# CMAKE_PROJECT_INCLUDE runs after every project() call, ST's component directories included.
get_property(_fg_cm_loaded GLOBAL PROPERTY FIRMGEN_COREMARK_LOADED)
if(_fg_cm_loaded)
  return()
endif()
set_property(GLOBAL PROPERTY FIRMGEN_COREMARK_LOADED TRUE)

foreach(_fg_cm_var FIRMGEN_COREMARK_DIR FIRMGEN_COREMARK_PROJECT_ROOT FIRMGEN_COREMARK_BOARD_DIR)
  if(NOT IS_DIRECTORY "${${_fg_cm_var}}")
    message(FATAL_ERROR "FirmGen CoreMark: ${_fg_cm_var} is not a directory ('${${_fg_cm_var}}')")
  endif()
endforeach()

function(_firmgen_coremark_inside _path _dir _out)
  file(RELATIVE_PATH _rel "${_dir}" "${_path}")
  if(_rel MATCHES "^\\.\\./" OR IS_ABSOLUTE "${_rel}")
    set(${_out} FALSE PARENT_SCOPE)
  else()
    set(${_out} TRUE PARENT_SCOPE)
  endif()
endfunction()

function(_firmgen_coremark_link_options _options _out)
  # Application wrappers live in the sources we remove (allocation policies,
  # RTOS guards, etc.). Inheriting --wrap redirects even newlib's float printf
  # to those absent functions. Strip the redirects, not the other linker options
  # or the application's properties. Work on the complete string: splitting a
  # nested generator expression on semicolons would corrupt its structure.
  set(_previous "")
  while(NOT "${_previous}" STREQUAL "${_options}")
    set(_previous "${_options}")
    # First/only and subsequent entries in a compiler-driver or LINKER list.
    string(REGEX REPLACE "(-Wl,|LINKER:)--wrap[=,][A-Za-z_][A-Za-z0-9_.$]*," "\\1" _options "${_options}")
    string(REGEX REPLACE "(-Wl,|LINKER:)--wrap[=,][A-Za-z_][A-Za-z0-9_.$]*" "" _options "${_options}")
    string(REGEX REPLACE ",--wrap[=,][A-Za-z_][A-Za-z0-9_.$]*" "" _options "${_options}")
    string(REGEX REPLACE "LINKER:SHELL:--wrap[= ]+[A-Za-z_][A-Za-z0-9_.$]*" "" _options "${_options}")
  endwhile()
  # The project's linker script, without its ASSERT()s. They check the application's own sections,
  # which this image does not contain: c5_board_monitor's (2026-11-03) asserted that .memory_diag
  # is exactly 1024 bytes and .crash_retained exactly 128, both filled by application files, so
  # its CoreMark image could not link. Memory still cannot overflow: ld checks the regions itself.
  string(REGEX MATCHALL "(-T,?|--script=)[^;>\"]+" _scripts "${_options}")
  foreach(_option IN LISTS _scripts)
    string(REGEX REPLACE "^(-T,?|--script=)" "" _script "${_option}")
    string(REGEX MATCH "^(-T,?|--script=)" _flag "${_option}")
    if(EXISTS "${_script}" AND NOT IS_DIRECTORY "${_script}")
      _firmgen_coremark_unasserted_script("${_script}" _copy)
      string(REPLACE "${_option}" "${_flag}${_copy}" _options "${_options}")
    endif()
  endforeach()
  set(${_out} "${_options}" PARENT_SCOPE)
endfunction()

# A copy of the linker script `_script` with every ASSERT() taken out, in the image's build tree.
# `. = ASSERT(expr, "message")` becomes `. = .`; a statement ASSERT(...); goes. An INCLUDE of a
# file beside the original is pointed at that file, since the copy lives elsewhere.
function(_firmgen_coremark_unasserted_script _script _out)
  file(READ "${_script}" _text)
  string(REGEX REPLACE "=[ \t]*ASSERT[ \t\r\n]*\\([^\"]*\"[^\"]*\"[ \t\r\n]*\\)" "= ." _text "${_text}")
  string(REGEX REPLACE "ASSERT[ \t\r\n]*\\([^\"]*\"[^\"]*\"[ \t\r\n]*\\)[ \t]*;?" "" _text "${_text}")
  get_filename_component(_dir "${_script}" DIRECTORY)
  string(REGEX MATCHALL "INCLUDE[ \t]+[^ \t\r\n;]+" _includes "${_text}")
  foreach(_include IN LISTS _includes)
    string(REGEX REPLACE "^INCLUDE[ \t]+" "" _name "${_include}")
    string(REPLACE "\"" "" _name "${_name}")
    if(NOT IS_ABSOLUTE "${_name}" AND EXISTS "${_dir}/${_name}")
      string(REPLACE "${_include}" "INCLUDE \"${_dir}/${_name}\"" _text "${_text}")
    endif()
  endforeach()
  string(MD5 _id "${_script}")
  get_filename_component(_name "${_script}" NAME)
  set(_copy "${CMAKE_BINARY_DIR}/firmgen_linker/${_id}/${_name}")
  file(WRITE "${_copy}" "${_text}")
  set(${_out} "${_copy}" PARENT_SCOPE)
endfunction()

function(_firmgen_coremark_application_path _path _base _out)
  get_filename_component(_abs "${_path}" ABSOLUTE BASE_DIR "${_base}")
  string(REPLACE "\\" "/" _normalized "${_abs}")
  string(TOLOWER "${_normalized}" _normalized)
  _firmgen_coremark_inside("${_abs}" "${FIRMGEN_COREMARK_PROJECT_ROOT}" _in_project)
  _firmgen_coremark_inside("${_abs}" "${FIRMGEN_COREMARK_BOARD_DIR}" _in_board)
  # A user's project may itself be named "FreeRTOS" or live under "middleware".
  # Classify its contents, not those ancestor directory names (also inside genexes).
  string(TOLOWER "${FIRMGEN_COREMARK_PROJECT_ROOT}/" _project_prefix)
  string(REPLACE "${_project_prefix}" "/" _normalized "${_normalized}")
  if((_in_project AND NOT _in_board) OR
     _normalized MATCHES "/(middleware|middlewares|rtos|freertos|freertos-kernel|embos|threadx)(/|$)")
    set(${_out} TRUE PARENT_SCOPE)
  else()
    set(${_out} FALSE PARENT_SCOPE)
  endif()
endfunction()

# STM32Cube component libraries are INTERFACE targets: their sources are added
# at generation time, so inspecting the application's SOURCES alone misses them.
# Build private interface copies with filtered dependencies. Never edit the
# original targets: the application's middleware and compile definitions stay intact.
function(_firmgen_coremark_genex_list _value _out)
  # CMSIS conditions contain semicolon lists inside nested generator expressions.
  # foreach(IN LISTS) alone splits those expressions into invalid source fragments.
  set(_items "")
  set(_pending "")
  set(_depth 0)
  foreach(_part IN LISTS _value)
    if(_depth GREATER 0)
      string(APPEND _pending ";${_part}")
    else()
      set(_pending "${_part}")
    endif()
    string(REGEX MATCHALL "\\$<" _opens "${_part}")
    string(REGEX MATCHALL ">" _closes "${_part}")
    list(LENGTH _opens _open_count)
    list(LENGTH _closes _close_count)
    math(EXPR _depth "${_depth} + ${_open_count} - ${_close_count}")
    if(_depth EQUAL 0)
      string(REPLACE ";" "\\;" _item "${_pending}")
      list(APPEND _items "${_item}")
      set(_pending "")
    endif()
  endforeach()
  if(NOT _depth EQUAL 0)
    message(FATAL_ERROR "FirmGen CoreMark: unbalanced dependency expression: ${_value}")
  endif()
  set(${_out} "${_items}" PARENT_SCOPE)
endfunction()

function(_firmgen_coremark_platform_link _link _out)
  set(${_out} "" PARENT_SCOPE)
  if(_link MATCHES "^\\$<LINK_ONLY:([^<>]+)>$")
    _firmgen_coremark_platform_link("${CMAKE_MATCH_1}" _inner)
    if(_inner)
      set(${_out} "$<LINK_ONLY:${_inner}>" PARENT_SCOPE)
    endif()
    return()
  endif()
  if(NOT TARGET "${_link}")
    # Preserve toolchain libraries and options, but not explicit middleware archives.
    _firmgen_coremark_application_path("${_link}" "${CMAKE_SOURCE_DIR}" _drop)
    if(NOT _drop)
      set(${_out} "${_link}" PARENT_SCOPE)
    endif()
    return()
  endif()
  get_target_property(_alias "${_link}" ALIASED_TARGET)
  if(_alias)
    set(_link "${_alias}")
  endif()
  get_target_property(_dir "${_link}" SOURCE_DIR)
  _firmgen_coremark_application_path("${_dir}" "${CMAKE_SOURCE_DIR}" _drop)
  if(_drop)
    return()
  endif()
  get_target_property(_type "${_link}" TYPE)
  if(NOT _type STREQUAL "INTERFACE_LIBRARY")
    set(${_out} "${_link}" PARENT_SCOPE)
    return()
  endif()
  string(MD5 _id "${_link}")
  set(_copy "_firmgen_coremark_platform_${_id}")
  set(${_out} "${_copy}" PARENT_SCOPE)
  if(TARGET "${_copy}")
    return() # Shared dependencies and cycles use the same private target.
  endif()
  add_library("${_copy}" INTERFACE)
  foreach(_property INCLUDE_DIRECTORIES SYSTEM_INCLUDE_DIRECTORIES COMPILE_DEFINITIONS
      COMPILE_OPTIONS COMPILE_FEATURES LINK_OPTIONS LINK_DIRECTORIES LINK_DEPENDS)
    get_target_property(_value "${_link}" "INTERFACE_${_property}")
    if(NOT _value STREQUAL "_value-NOTFOUND")
      if(_property STREQUAL "LINK_OPTIONS" AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
        _firmgen_coremark_link_options("${_value}" _value)
      endif()
      set_property(TARGET "${_copy}" PROPERTY "INTERFACE_${_property}" "${_value}")
    endif()
  endforeach()
  get_target_property(_sources "${_link}" INTERFACE_SOURCES)
  if(NOT _sources STREQUAL "_sources-NOTFOUND")
    _firmgen_coremark_genex_list("${_sources}" _sources)
    foreach(_source IN LISTS _sources)
      _firmgen_coremark_application_path("${_source}" "${_dir}" _drop)
      if(NOT _drop)
        target_sources("${_copy}" INTERFACE "${_source}")
      endif()
    endforeach()
  endif()
  get_target_property(_links "${_link}" INTERFACE_LINK_LIBRARIES)
  if(NOT _links STREQUAL "_links-NOTFOUND")
    _firmgen_coremark_genex_list("${_links}" _links)
    foreach(_dependency IN LISTS _links)
      _firmgen_coremark_platform_link("${_dependency}" _platform)
      if(NOT "${_platform}" STREQUAL "")
        target_link_libraries("${_copy}" INTERFACE "${_platform}")
      endif()
    endforeach()
  endif()
endfunction()

function(_firmgen_coremark_apply)
  set(_target ${CMAKE_PROJECT_NAME})
  if(NOT TARGET ${_target} AND TARGET ${CMAKE_PROJECT_NAME}.elf)
    set(_target ${CMAKE_PROJECT_NAME}.elf)
  endif()
  if(NOT TARGET ${_target})
    message(FATAL_ERROR "FirmGen CoreMark: no executable named ${_target}")
  endif()
  get_target_property(_sources ${_target} SOURCES)
  _firmgen_coremark_genex_list("${_sources}" _sources)
  set(_kept "")
  set(_dropped 0)
  foreach(_source IN LISTS _sources)
    get_filename_component(_abs "${_source}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
    _firmgen_coremark_inside("${_abs}" "${FIRMGEN_COREMARK_DIR}" _is_coremark)
    _firmgen_coremark_application_path("${_source}" "${CMAKE_SOURCE_DIR}" _is_application)
    # The application is everything of the project's own outside the board directory
    # (application/, firmware/, user/ components), plus FirmGen's RTOS glue inside it. What stays
    # is the board: ST's start-up, system and generated HAL code, and the toolchain's.
    if(FIRMGEN_COREMARK_CMSIS)
      # Board-independent Cortex-M benchmark: only vendor startup and SystemInit.
      # No application ISRs, RTOS, constructors or application init are executed.
      get_filename_component(_base "${_source}" NAME)
      file(RELATIVE_PATH _in_project "${FIRMGEN_COREMARK_PROJECT_ROOT}" "${_abs}")
      if(_base MATCHES "^startup_.*\\.(s|S|asm)$" OR _base MATCHES "^system_stm32.*\\.c$")
        list(APPEND _kept "${_source}")
      elseif(FIRMGEN_COREMARK_KEEP_HAL AND (_in_project MATCHES "^Drivers/" OR _base MATCHES "_hal_msp\\.c$"
             OR _base MATCHES "^(syscalls|sysmem|firmgen_iar_syscalls)\\.c$"))
        list(APPEND _kept "${_source}")
      endif()
    elseif(_is_coremark)
      continue()
    elseif(_is_application)
      math(EXPR _dropped "${_dropped} + 1")
    else()
      list(APPEND _kept "${_source}")
    endif()
  endforeach()
  file(GLOB _coremark_sources "${FIRMGEN_COREMARK_DIR}/*.c")
  if(NOT _coremark_sources)
    message(FATAL_ERROR "FirmGen CoreMark: no sources in ${FIRMGEN_COREMARK_DIR}")
  endif()
  list(APPEND _coremark_sources ${FIRMGEN_COREMARK_EXTRA_SOURCES})
  # Do not mutate or build the application's target: it may depend on A/B child
  # builds, packaging jobs, signing, or post-build commands. Copy only compilation
  # and linking properties to a dedicated executable, never MANUALLY_ADDED_DEPENDENCIES.
  set(_application ${_target})
  set(_target firmgen_coremark)
  add_executable(${_target} EXCLUDE_FROM_ALL ${_kept} ${_coremark_sources})
  foreach(_property
      INCLUDE_DIRECTORIES COMPILE_DEFINITIONS COMPILE_OPTIONS COMPILE_FEATURES
      LINK_LIBRARIES LINK_OPTIONS LINK_DIRECTORIES LINK_DEPENDS
      C_STANDARD C_STANDARD_REQUIRED C_EXTENSIONS SUFFIX
      INTERPROCEDURAL_OPTIMIZATION)
    get_target_property(_value ${_application} ${_property})
    if(NOT _value STREQUAL "_value-NOTFOUND")
      if(_property STREQUAL "LINK_OPTIONS" AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
        _firmgen_coremark_link_options("${_value}" _value)
      elseif(_property STREQUAL "LINK_LIBRARIES" AND NOT FIRMGEN_COREMARK_CMSIS)
        set(_platform_links "")
        _firmgen_coremark_genex_list("${_value}" _value)
        foreach(_link IN LISTS _value)
          _firmgen_coremark_platform_link("${_link}" _platform)
          if(NOT "${_platform}" STREQUAL "")
            list(APPEND _platform_links "${_platform}")
          endif()
        endforeach()
        set(_value "${_platform_links}")
      endif()
      set_property(TARGET ${_target} PROPERTY ${_property} "${_value}")
    endif()
  endforeach()
  # FirmGen's classic scaffold puts .elf in the target's NAME (${PROJECT_NAME}.elf) and its
  # toolchain file sets no executable suffix, so the copied SUFFIX is empty and this image was linked
  # as a bare "firmgen_coremark". STM32CubeProgrammer refuses to write a file without a known
  # extension (NUCLEO-G071RB, 2026-11-02). Both GCC and IAR link an ELF here.
  get_target_property(_suffix ${_target} SUFFIX)
  if(NOT _suffix AND "${CMAKE_EXECUTABLE_SUFFIX_C}${CMAKE_EXECUTABLE_SUFFIX}" STREQUAL "")
    set_property(TARGET ${_target} PROPERTY SUFFIX ".elf")
  endif()
  if(FIRMGEN_COREMARK_CMSIS)
    set_property(TARGET ${_target} PROPERTY LINK_LIBRARIES "")
    target_compile_definitions(${_target} PRIVATE
      FIRMGEN_COREMARK_DEVICE_HEADER="${FIRMGEN_COREMARK_DEVICE_HEADER}")
    if(FIRMGEN_COREMARK_KEEP_HAL)
      string(REGEX REPLACE "\\.h$" "_hal.h" _hal_header "${FIRMGEN_COREMARK_DEVICE_HEADER}")
      target_compile_definitions(${_target} PRIVATE FIRMGEN_TARGET_HAL_HEADER="${_hal_header}")
    endif()
  endif()
  file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/firmgen_coremark_image.txt"
    CONTENT "$<TARGET_FILE:${_target}>")
  # The project's coding rules are for the project's code. CoreMark's sources must compile
  # unmodified, and IAR's --require_prototypes (ST's template turns it on) rejects their K&R-era
  # definitions, so for CoreMark's files only that check is a warning. Diagnostics, not code
  # generation: the optimisation and target flags are untouched.
  if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    set_property(SOURCE ${_coremark_sources} TARGET_DIRECTORY ${_target}
      APPEND PROPERTY COMPILE_OPTIONS "--diag_warning=Pa045")
  endif()
  # The STM32 port formats its output without libc's heap-allocating float printf.
  target_include_directories(${_target} BEFORE PRIVATE "${FIRMGEN_COREMARK_DIR}" ${FIRMGEN_COREMARK_EXTRA_INCLUDES})
  message(STATUS "FirmGen CoreMark: ${_dropped} application source(s) replaced by CoreMark")
endfunction()

cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL _firmgen_coremark_apply)
