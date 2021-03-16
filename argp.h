#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef ARGP_H // emulating a basic version of <argp.h> for multiplatform
#define ARGP_H

#ifndef __error_t_defined
# define __error_t_defined 1
typedef int error_t;
#endif

#define ARGP_KEY_ARG		0
#define ARGP_HELP_USAGE		0x01


#define ARGP_PGN_ALL 0x1 // go thorugh all boards
#define ARGP_PGN_STEP 0x2 // step through boards
#define ARGP_PGN_PRINT 0X4 // print boards that are gone through
#define ARGP_PGN_HEADER 0X8 // print the PGN header
#define ARGP_FEN_PRINT 0X10 // print FEN string

struct argp;			/* fwd declare this type */
struct argp_state;		/* " */
struct argp_child;		/* " */
typedef error_t (*argp_parser_t) (int __key, char *__arg, struct argp_state *__state);

extern const char *argp_program_version;

struct argp_option
{
  const char *name;
  int key;
  const char *arg;
  int flags; // ignored
  const char *doc;
  int group; // ignored
};

struct argp
{
  const struct argp_option *options;
  argp_parser_t parser;
  const char *args_doc;
  const char *doc;
  const struct argp_child *children; //unused
  char *(*help_filter) (int __key, const char *__text, void *__input); //unused
  const char *argp_domain; //unused
};

struct argp_state
{
  const struct argp *root_argp;
  int argc; 
  char **argv;
  int next;
  unsigned flags; //unused
  unsigned arg_num; //unused
  int quoted; //unused
  void *input;
  void **child_inputs; //unused
  void *hook; //unused
  char *name; //unused
  FILE *err_stream; //unused
  FILE *out_stream; //unused
  void *pstate; //unused

};

error_t argp_parse (const struct argp *__restrict __argp, int __argc, char **__restrict __argv, unsigned __flags, int *__restrict argv, void *__restrict __input); // takes the same input as <argp.h>

void argp_usage (const struct argp_state *__restrict _state, FILE *stream);
void argp_help (const struct argp_state *__restrict __state, FILE *stream);
void argp_version ();
#endif //ARGP_H
