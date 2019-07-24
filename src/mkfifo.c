/**
 * @file
 * @brief mkfifo utility
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */

#include <sys/stat.h>
#include <err.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef TEST
/**
 * Declare some functions with extern linkage, allowing the test suite to call
 * those functions.
 */
# define LINKAGE extern
# include "../test/test.h"
#else /* !(TEST) */
/**
 * Define all functions as static when not testing.
 */
# define LINKAGE static
#endif /* TEST */

/**
 * mkfifo utility context.
 */
struct mkfifo_ctx{
  /**
   * Exit status set to one of the following values.
   *   - EXIT_SUCCESS
   *   - EXIT_FAILURE
   */
  int status_code;

  /**
   * File permissions used in mkfifo().
   */
  mode_t mode;
};

/**
 * Print an error message to STDERR and set an error status code.
 *
 * @param[in,out] mkfifo_ctx See @ref mkfifo_ctx.
 * @param[in]     errno_msg  Include a standard message describing errno.
 * @param[in]     fmt        Format string used by vwarn.
 */
static void
mkfifo_warn(struct mkfifo_ctx *const mkfifo_ctx,
            const bool errno_msg,
            const char *const fmt, ...){
  va_list ap;

  mkfifo_ctx->status_code = EXIT_FAILURE;
  va_start(ap, fmt);
  if(errno_msg){
    vwarn(fmt, ap);
  }
  else{
    vwarnx(fmt, ap);
  }
  va_end(ap);
}

/**
 * Create a new FIFO using mkfifo().
 *
 * @param[in,out] mkfifo_ctx See @ref mkfifo_ctx.
 * @param[in]     path       Path to new FIFO file to create.
 */
static void
mkfifo_path(struct mkfifo_ctx *const mkfifo_ctx,
            const char *const path){
  if(mkfifo(path, mkfifo_ctx->mode) != 0){
    mkfifo_warn(mkfifo_ctx, true, "cannot create fifo: %s", path);
  }
}

/**
 * Parse mode string given in the (-m mode) argument.
 *
 * @param[in,out] mkfifo_ctx See @ref mkfifo_ctx.
 * @param[in]     mode_str   Same as chmod utility.
 */
static void
mkfifo_parse_mode(struct mkfifo_ctx *const mkfifo_ctx,
                  const char *const mode_str){
  void *compiled_mode;

  umask(0);
  compiled_mode = setmode(mode_str);
  if(compiled_mode == NULL){
    mkfifo_warn(mkfifo_ctx, true, "setmode: %s", mode_str);
  }
  else{
    /* Initial mode of a=rw -> 0666 */
    mkfifo_ctx->mode = getmode(compiled_mode, 0666);
    free(compiled_mode);
  }
}

/**
 * Main entry point for mkfifo utility.
 *
 * Usage:
 * mkfifo [-m mode] file...
 *
 * @param[in]     argc         Number of arguments in @p argv.
 * @param[in,out] argv         Argument list.
 * @retval        EXIT_SUCCESS All FIFO files successfully created.
 * @retval        EXIT_FAILURE Failed to create at least one FIFO file.
 */
LINKAGE int
mkfifo_main(int argc,
            char *argv[]){
  int c;
  int i;
  struct mkfifo_ctx mkfifo_ctx;

  memset(&mkfifo_ctx, 0, sizeof(mkfifo_ctx));
  mkfifo_ctx.mode = S_IRUSR | S_IWUSR |
                    S_IRGRP | S_IWGRP |
                    S_IROTH | S_IWOTH;
  while((c = getopt(argc, argv, "m:")) != -1){
    switch(c){
      case 'm':
        mkfifo_parse_mode(&mkfifo_ctx, optarg);
        break;
      default:
        mkfifo_ctx.status_code = EXIT_FAILURE;
        break;
    }
  }
  argc -= optind;
  argv += optind;
  if(mkfifo_ctx.status_code == 0){
    if(argc < 1){
      mkfifo_warn(&mkfifo_ctx, false, "missing file...");
    }
    else{
      for(i = 0; i < argc; i++){
        mkfifo_path(&mkfifo_ctx, argv[i]);
      }
    }
  }
  return mkfifo_ctx.status_code;
}

#ifndef TEST
/**
 * Main program entry point.
 *
 * @param[in]     argc See @ref mkfifo_main.
 * @param[in,out] argv See @ref mkfifo_main.
 * @return             See @ref mkfifo_main.
 */
int
main(int argc,
     char *argv[]){
  return mkfifo_main(argc, argv);
}
#endif /* TEST */

