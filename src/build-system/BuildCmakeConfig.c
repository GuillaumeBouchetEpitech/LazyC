
#include "./BuildCmakeConfig.h"

#include "stdlib/core/panic.h"
#include "stdlib/filesystem/ensureFolder.h"
#include "stdlib/filesystem/StreamWriter.h"
#include "stdlib/filesystem/StreamCopyFile.h"
#include "stdlib/filesystem/pathUtils.h"
#include "stdlib/sub-process/executeCommand.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// int BuildCmakeConfig__generateCmakeFile(
//     const char *inBaseDir,
//     PointerHeapArray *inSourcesFilepaths,
//     const char *inOutputDir,
//     PointerHeapArray *inIncludePath,
//     PointerHeapArray *inLibraryPath)
int BuildCmakeConfig__generateCmakeFile(const BuildCmakeConfigOpts *inOpts)
{
  if (!inOpts->inSourcesFilepaths || inOpts->inSourcesFilepaths->len == 0)
  {
    panic("empty cmake project");
    // return 0;
  }

  // TODO: check if the config file already exist (compare mtime to sources latest mtime)

  // ??? copy the source files in the output dir?

  // ensure the output-dir folder exist
  // stream write the file

  if (ensureFolder(inOpts->inOutputDir) != 0)
  {
    return -1;
  }

  char* outSrcFolder = Path__join(2, inOpts->inOutputDir, "src");

  PointerHeapArray *outputSourcesFilepaths = PointerHeapArray__preAllocate(32);

  printf(" -inSourcesFilepaths->len-> %d\n", inOpts->inSourcesFilepaths->len);
  for (unsigned int ii = 0; ii < inOpts->inSourcesFilepaths->len; ++ii) {
    const char* tmpFilepath = inOpts->inSourcesFilepaths->data[ii];

    // printf(" ---tmpFilepath-> %s\n", tmpFilepath);

    // // char* inputFilepath = NULL;
    // char* outFilepath = NULL;
    // if (tmpFilepath[0] == '/') {
    //   // path is absolute
    //   // inputFilepath = strdup(tmpFilepath);

    //   const int baseDirLen = strlen(inOpts->inBaseDir);
    //   const char* relInputFilepath = tmpFilepath + baseDirLen;

    //   outFilepath = Path__join(2, outSrcFolder, relInputFilepath);
    // }
    // else {
    //   // path is relative
    //   // inputFilepath = Path__join(2, inOpts->inBaseDir, tmpFilepath);
    //   outFilepath = Path__join(2, outSrcFolder, tmpFilepath);
    // }

    char* outFilepath = strdup(tmpFilepath);

    // printf("   -outFilepath-> %s\n", outFilepath);

    if (PointerHeapArray__pushBack(outputSourcesFilepaths, outFilepath) < 0) {
      // TODO: goto error_cleanup
    }

    char* outFolderPath = Path__dirname(outFilepath);

    if (ensureFolder(outFolderPath) != 0)
    {
      return -1;
    }

    free(outFolderPath);

    // int copyResult = 0;

    // if (strcmp(inputFilepath, outFilepath) != 0) {
    //   // input and output folder are different -> must copy the files
    //   copyResult = StreamCopyFile__copyFile(inputFilepath, outFilepath);
    // }

    // free(inputFilepath);

    // if (copyResult < 0) {
    //   // TODO: goto error_cleanup
    // }
  }


  // char* testFolder = Path__dirname(__FILE__);
  char* configFilepath = Path__join(2, inOpts->inOutputDir, "CMakeLists.txt");
  StreamWriter* streamWriter = StreamWriter__create(configFilepath);


  char buffer[1024];
  memset(buffer, 0, 1024);

  {
    const char* k_str = "\n"
      "cmake_minimum_required(VERSION 3.24)\n"
      "\n"
      "project(exec C)\n"
      "\n"
      "add_executable(${PROJECT_NAME}\n";

    StreamWriter__write(streamWriter, k_str, strlen(k_str));
  }

  // snprintf(buffer, 1024, "cmake_minimum_required(VERSION 3.24)\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));

  // snprintf(buffer, 1024, "\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));
  // snprintf(buffer, 1024, "project(exec C)\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));

  // snprintf(buffer, 1024, "\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));
  // snprintf(buffer, 1024, "add_executable(${PROJECT_NAME}\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));


  const unsigned int outputDirLen = strlen(inOpts->inOutputDir);

  for (unsigned int ii = 0; ii < outputSourcesFilepaths->len; ++ii) {
    const char* tmpFilepath = outputSourcesFilepaths->data[ii];
    const char* relFilepath = tmpFilepath + outputDirLen + 1;

    snprintf(buffer, 1024, "  ${PROJECT_SOURCE_DIR}/%s\n", relFilepath);
    StreamWriter__write(streamWriter, buffer, strlen(buffer));
  }

  {
    const char* k_str = ")\n"
      "\n"
      "target_include_directories(exec PRIVATE\n";

    StreamWriter__write(streamWriter, k_str, strlen(k_str));
  }

  // snprintf(buffer, 1024, ")\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));
  // snprintf(buffer, 1024, "\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));

  // snprintf(buffer, 1024, "target_include_directories(exec PRIVATE\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));
  snprintf(buffer, 1024, "  ${PROJECT_SOURCE_DIR}/%s\n", outSrcFolder + outputDirLen + 1);
  StreamWriter__write(streamWriter, buffer, strlen(buffer));

  if (inOpts->inIncludePath)
  {
    for (unsigned int ii = 0; ii < inOpts->inIncludePath->len; ++ii)
    {
      const char* tmpPath = inOpts->inIncludePath->data[ii];
      snprintf(buffer, 1024, "  %s\n", tmpPath);
      StreamWriter__write(streamWriter, buffer, strlen(buffer));
    }
  }

  {
    const char* k_str = ")\n\n";

    StreamWriter__write(streamWriter, k_str, strlen(k_str));
  }

  if (inOpts->inLibraryPath)
  {
    // target_link_libraries(exec PRIVATE
    //   ${ROOT_DIR}/custom-c-parser/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a
    //   ${ROOT_DIR}/third_parties/tree-sitter/libtree-sitter.a
    // )

    {
      const char* k_str = "\ntarget_link_libraries(exec PRIVATE\n";
      StreamWriter__write(streamWriter, k_str, strlen(k_str));
    }

    for (unsigned int ii = 0; ii < inOpts->inLibraryPath->len; ++ii)
    {
      const char* tmpPath = inOpts->inLibraryPath->data[ii];
      snprintf(buffer, 1024, "  %s\n", tmpPath);
      StreamWriter__write(streamWriter, buffer, strlen(buffer));
    }

    {
      const char* k_str = ")\n";
      StreamWriter__write(streamWriter, k_str, strlen(k_str));
    }

  }

  {
    const char* k_str = "\n"
      "\n"
      "set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS \"-g3\") # debug\n"
      "set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS \"-g3\") # debug\n"
      "set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY \"${PROJECT_SOURCE_DIR}/bin\")\n"
      "\n";

    StreamWriter__write(streamWriter, k_str, strlen(k_str));
  }

  // snprintf(buffer, 1024, ")\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));
  // snprintf(buffer, 1024, "\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));

  // snprintf(buffer, 1024, "set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS \"-g3\") # debug\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));
  // snprintf(buffer, 1024, "set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS \"-g3\") # debug\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));
  // snprintf(buffer, 1024, "set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY \"${PROJECT_SOURCE_DIR}/bin\")\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));

  // snprintf(buffer, 1024, "\n");
  // StreamWriter__write(streamWriter, buffer, strlen(buffer));

  for (unsigned int ii = 0; ii < outputSourcesFilepaths->len; ++ii) {
    char* tmpFilepath = outputSourcesFilepaths->data[ii];
    free(tmpFilepath);
  }
  PointerHeapArray__free(&outputSourcesFilepaths);
  StreamWriter__free(&streamWriter);
  free(configFilepath);
  free(outSrcFolder);

  if (inOpts->doBuild != 0)
  {
    char* buildFolderPath = Path__join(2, inOpts->inOutputDir, "build");

    // if (ensureFolder(buildFolderPath) != 0)
    // {
    //   free(buildFolderPath);
    //   return -1;
    // }

    {
      char *const  argv[] = { "/usr/bin/cmake", "-B", "./build", NULL };
      const int result = executeCommand("/usr/bin/cmake", argv, inOpts->inOutputDir);
      if (result != 0) {
        panic("could not initialize a cmake project");
        free(buildFolderPath);
        return -1;
      }
    }

    {
      char *const  argv[] = { "/usr/bin/cmake", "--build", "./build", "--parallel", "5", NULL };
      const int result = executeCommand("/usr/bin/cmake", argv, inOpts->inOutputDir);
      if (result != 0) {
        panic("could not build a cmake project");
        free(buildFolderPath);
        return -1;
      }
    }

    free(buildFolderPath);
  }



  return 0;
}
