
cmake_minimum_required(VERSION 3.24)

project(exec C)


message(STATUS " -> CMAKE_BUILD_TARGET: ${CMAKE_BUILD_TARGET}")
string(TOLOWER "${CMAKE_BUILD_TARGET}" build_target)
message(STATUS " ---> build_target:     ${build_target}")

message(STATUS " -> CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
message(STATUS " ---> build_type:     ${build_type}")


if (build_target STREQUAL test)

  # find ./src -name *.c -not -path "*/tests-assets/*"
  add_executable(unit_tests
    ./src/stdlib/core/panic.c
    ./src/stdlib/filesystem/statUtils.c
    ./src/stdlib/filesystem/pathUtils.c
    ./src/stdlib/filesystem/StreamCopyFile.c
    ./src/stdlib/filesystem/readFile.c
    ./src/stdlib/filesystem/tests/pathUtils.tests.c
    ./src/stdlib/filesystem/writeFile.c
    ./src/stdlib/filesystem/ensureFolder.c
    ./src/stdlib/filesystem/StreamReader.c
    ./src/stdlib/filesystem/StreamWriter.c
    ./src/stdlib/collections/HashMap.c
    ./src/stdlib/collections/HashSet.c
    ./src/stdlib/collections/PointerHeapArray.c
    ./src/stdlib/collections/tests/PointerHeapArray.tests.c
    ./src/stdlib/collections/tests/HashSet.tests.c
    ./src/stdlib/collections/tests/HashMap.tests.c
    ./src/stdlib/strings/StringBuffer.c
    ./src/stdlib/strings/replaceAll.c
    ./src/stdlib/strings/replaceAll2.c
    ./src/stdlib/strings/trimStr.c
    ./src/stdlib/time/StopWatch.c
    ./src/stdlib/strings/tests/replaceAll2.tests.c
    ./src/stdlib/sub-process/executeCommand.c
    ./src/stdlib/sub-process/tests/executeCommand.tests.c
    ./src/build-system/BuildCmakeConfig.c
    ./src/build-system/tests/BuildCmakeConfig.tests.c
    ./src/main.tests.c
    ./src/cli-arg-parser/CommandLineParserV2.c
    ./src/cli-arg-parser/tests/CommandLineParserV2.tests.c
    ./src/transpiler/analyzer/SourceAnalyzer.c
    ./src/transpiler/analyzer/tests/SourceAnalyzer.tests.c
    ./src/transpiler/analyzer/internals/parser/tests/SourceParser.tests.c
    ./src/transpiler/analyzer/internals/parser/SourceParser.c
    ./src/transpiler/analyzer/internals/parser/internals/SourceParsedFile.c
    ./src/transpiler/analyzer/internals/tests/AnalyzedFile.test.c
    ./src/transpiler/analyzer/internals/AnalyzedFile.c
    ./src/transpiler/analyzer/internals/indexer/SourceIndexer.c
    ./src/transpiler/analyzer/internals/indexer/internals/VarDef.c
    ./src/transpiler/analyzer/internals/indexer/internals/ComptimeCallRef.c
    ./src/transpiler/analyzer/internals/indexer/internals/IdentifiedRef.c
    ./src/transpiler/analyzer/internals/indexer/internals/SourceScope.c
    ./src/transpiler/analyzer/internals/indexer/internals/tests/ComptimeCallRef.tests.c
    ./src/transpiler/tests/Transpiler.tests.c
    ./src/transpiler/Transpiler.c
  )

  target_include_directories(unit_tests PRIVATE
    ./src
    ${PROJECT_SOURCE_DIR}/third_parties/tree-sitter-lazy-c/bindings/c
    ${PROJECT_SOURCE_DIR}/third_parties/tree-sitter/lib/include
  )

  target_link_libraries(unit_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a
    ${PROJECT_SOURCE_DIR}/third_parties/tree-sitter/libtree-sitter.a
    # -fsanitize=address,undefined
  )

  set_target_properties(unit_tests PROPERTIES COMPILE_FLAGS "-g3 -fsanitize=address,undefined") # debug (ASAN)
  set_target_properties(unit_tests PROPERTIES LINK_FLAGS "-g3 -fsanitize=address,undefined") # debug (ASAN)
  # set_target_properties(unit_tests PROPERTIES COMPILE_FLAGS "-g3") # debug (valgrind)
  # set_target_properties(unit_tests PROPERTIES LINK_FLAGS "-g3") # debug (valgrind)

  set_target_properties(unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")


else()


  # find ./src -name *.c -not -path "*/tests/*"
  add_executable(exec
    ./src/stdlib/core/panic.c
    ./src/stdlib/filesystem/statUtils.c
    ./src/stdlib/filesystem/pathUtils.c
    ./src/stdlib/filesystem/StreamCopyFile.c
    ./src/stdlib/filesystem/readFile.c
    ./src/stdlib/filesystem/writeFile.c
    ./src/stdlib/filesystem/ensureFolder.c
    ./src/stdlib/filesystem/StreamReader.c
    ./src/stdlib/filesystem/StreamWriter.c
    ./src/stdlib/collections/HashMap.c
    ./src/stdlib/collections/HashSet.c
    ./src/stdlib/collections/PointerHeapArray.c
    ./src/stdlib/strings/StringBuffer.c
    ./src/stdlib/strings/replaceAll.c
    ./src/stdlib/strings/replaceAll2.c
    ./src/stdlib/strings/trimStr.c
    ./src/stdlib/time/StopWatch.c
    ./src/stdlib/sub-process/executeCommand.c
    ./src/build-system/BuildCmakeConfig.c
    ./src/cli-arg-parser/CommandLineParserV2.c
    ./src/main.c
    ./src/transpiler/analyzer/SourceAnalyzer.c
    ./src/transpiler/analyzer/internals/parser/SourceParser.c
    ./src/transpiler/analyzer/internals/parser/internals/SourceParsedFile.c
    ./src/transpiler/analyzer/internals/AnalyzedFile.c
    ./src/transpiler/analyzer/internals/indexer/SourceIndexer.c
    ./src/transpiler/analyzer/internals/indexer/internals/VarDef.c
    ./src/transpiler/analyzer/internals/indexer/internals/ComptimeCallRef.c
    ./src/transpiler/analyzer/internals/indexer/internals/IdentifiedRef.c
    ./src/transpiler/analyzer/internals/indexer/internals/SourceScope.c
    ./src/transpiler/Transpiler.c
  )

  target_include_directories(exec PRIVATE
    ./src
    ${PROJECT_SOURCE_DIR}/third_parties/tree-sitter-lazy-c/bindings/c
    ${PROJECT_SOURCE_DIR}/third_parties/tree-sitter/lib/include
  )

  target_link_libraries(exec PRIVATE
    ${PROJECT_SOURCE_DIR}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a
    ${PROJECT_SOURCE_DIR}/third_parties/tree-sitter/libtree-sitter.a
  )

  if (build_type STREQUAL debug)
      # message(STATUS " -> DEBUG")
      set_target_properties(exec PROPERTIES COMPILE_FLAGS "-g3 -Wall -Wextra -Werror") # debug (gdb/valgrind)
      set_target_properties(exec PROPERTIES LINK_FLAGS "-g3") # debug (gdb/valgrind)
      # set_target_properties(exec PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin.debug")
  elseif (build_type STREQUAL debug_asan)
      # message(STATUS " -> DEBUG")
      set_target_properties(exec PROPERTIES COMPILE_FLAGS "-g3 -Wall -Wextra -Werror -fsanitize=address,undefined") # debug (asan)
      set_target_properties(exec PROPERTIES LINK_FLAGS "-g3 -fsanitize=address,undefined") # debug (asan)
      # set_target_properties(exec PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin.debug")
  elseif (build_type STREQUAL release)
      # message(STATUS " -> RELEASE")
      set_target_properties(exec PROPERTIES COMPILE_FLAGS "-O3 -Wall -Wextra -Werror") # release
      set_target_properties(exec PROPERTIES LINK_FLAGS "-O3") # release
      # set_target_properties(exec PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
  elseif (build_type STREQUAL dev)
      # message(STATUS " -> DEV")
      set_target_properties(exec PROPERTIES COMPILE_FLAGS "-O0 -Wall -Wextra -Werror") # fast-release
      set_target_properties(exec PROPERTIES LINK_FLAGS "-O0") # fast-release
      # set_target_properties(exec PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin.dev")
  else ()
    message(FATAL_ERROR "build_type is not debug, debug_asan, release or dev")
  endif()

  # set_target_properties(exec PROPERTIES COMPILE_FLAGS "-O0") # fast-release
  # set_target_properties(exec PROPERTIES LINK_FLAGS "-O0") # fast-release
  set_target_properties(exec PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")


endif()



