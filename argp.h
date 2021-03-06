#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

#ifndef ARGP_H // emulating a basic version of <argp.h> for multiplatform
#define ARGP_H

#ifndef __error_t_defined
# define __error_t_defined 1
typedef int error_t;
#endif

#define ARGP_KEY_ARG		0
#define ARGP_HELP_USAGE		0x01

/*
// parse a pgn string to play out a board
// go thorugh all boards
// step through boards
// print boards that are gone through
// print the PGN header
// print FEN string
// parse a FEN string to set up the board
*/

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

error_t argp_parse (const struct argp *_argp, int _argc, char **_argv, unsigned _flags, int *argv, void *_input); // takes the same input as <argp.h>

void argp_usage (const struct argp_state *__restrict _state, FILE *stream);
void argp_help (const struct argp_state *__restrict __state, FILE *stream);
void argp_version ();
#endif //ARGP_H
