/**
 * @file
 * @brief test suite
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"

/**
 * Call @ref mkfifo_main with the given arguments.
 *
 * @param[in] mode_str           Corresponds to the (-m mode) argument.
 * @param[in] extra_arg          Add an invalid argument to the argument list.
 * @param[in] expect_exit_status Expected exit status code.
 * @param[in] file_list          List of FIFO's to create.
 */
static void
test_mkfifo_main(const char *const mode_str,
                 const bool extra_arg,
                 const int expect_exit_status,
                 const char *const file_list, ...){
  const size_t MAX_ARGS = 20;
  const size_t MAX_ARG_LEN = 255;
  size_t i;
  int exit_status;
  int status;
  pid_t pid;
  int argc;
  char **argv;
  const char *file;
  va_list ap;

  argc = 0;
  argv = malloc(MAX_ARGS * sizeof(argv));
  assert(argv);
  for(i = 0; i < MAX_ARGS; i++){
    argv[i] = malloc(MAX_ARG_LEN * sizeof(*argv));
    assert(argv[i]);
  }
  strcpy(argv[argc++], "mkfifo");
  if(extra_arg){
    strcpy(argv[argc++], "-a");
  }
  if(mode_str){
    strcpy(argv[argc++], "-m");
    strcpy(argv[argc++], mode_str);
  }
  va_start(ap, file_list);
  for(file = file_list; file; file = va_arg(ap, const char *const)){
    strcpy(argv[argc++], file);
  }
  va_end(ap);
  pid = fork();
  assert(pid >= 0);
  if(pid == 0){
    exit_status = mkfifo_main(argc, argv);
    exit(exit_status);
  }
  assert(waitpid(pid, &status, 0) == pid);
  assert(WIFEXITED(status));
  assert(WEXITSTATUS(status) == expect_exit_status);
  for(i = 0; i < MAX_ARGS; i++){
    free(argv[i]);
  }
  free(argv);
}

/**
 * Ensure a FIFO file exists and then remove it.
 *
 * @param[in] path         FIFO file to check.
 * @param[in] expect_perms Expected permission bits set in the file mode.
 */
static void
test_check_and_remove_fifo(const char *const path,
                           const mode_t expect_perms){
  struct stat sb;
  mode_t perms;

  assert(stat(path, &sb) == 0);
  perms = sb.st_mode & (S_IRUSR | S_IWUSR | S_IXUSR |
                        S_IRGRP | S_IWGRP | S_IXGRP |
                        S_IROTH | S_IWOTH | S_IXOTH);
  assert(S_ISFIFO(sb.st_mode));
  assert(perms == expect_perms);
  assert(remove(path) == 0);
}

/**
 * Run all test cases for the mkfifo utility.
 */
static void
test_all(void){
  const char *const PATH_NOEXIST = "build/noexist/test-fifo";
  const char *const PATH_MKFIFO = "build/fifo";
  const char *const PATH_MKFIFO_2 = "build/fifo-2";
  const mode_t default_mode = S_IRUSR | S_IWUSR |
                              S_IRGRP |
                              S_IROTH;

  remove(PATH_MKFIFO);
  remove(PATH_MKFIFO_2);

  /* No file arguments provided. */
  test_mkfifo_main(NULL, false, EXIT_FAILURE, NULL);

  /* Invalid mode_str argument. */
  test_mkfifo_main("abc", false, EXIT_FAILURE, PATH_MKFIFO, NULL);

  /* Unsupported argument. */
  test_mkfifo_main(NULL, true, EXIT_FAILURE, PATH_MKFIFO, NULL);

  /* Does not have permission to create FIFO. */
  test_mkfifo_main(NULL, false, EXIT_FAILURE, PATH_NOEXIST, NULL);

  /* Create one FIFO. */
  test_mkfifo_main(NULL, false, EXIT_SUCCESS, PATH_MKFIFO, NULL);
  test_check_and_remove_fifo(PATH_MKFIFO, default_mode);

  /* Create multiple FIFO's. */
  test_mkfifo_main(NULL,
                   false,
                   EXIT_SUCCESS,
                   PATH_MKFIFO,
                   PATH_MKFIFO_2,
                   NULL);
  test_check_and_remove_fifo(PATH_MKFIFO, default_mode);
  test_check_and_remove_fifo(PATH_MKFIFO_2, default_mode);

  /* Try to create multiple FIFO's but one fails. */
  test_mkfifo_main(NULL,
                   false,
                   EXIT_FAILURE,
                   PATH_MKFIFO,
                   PATH_NOEXIST,
                   NULL);
  test_check_and_remove_fifo(PATH_MKFIFO, default_mode);

  /* Create FIFO with a different mode. */
  test_mkfifo_main("123", false, EXIT_SUCCESS, PATH_MKFIFO, NULL);
  test_check_and_remove_fifo(PATH_MKFIFO, 0123);
}

/**
 * Test mkfifo utility.
 *
 * Usage: test
 *
 * @retval 0 All tests passed.
 */
int
main(void){
  test_all();
  return 0;
}

