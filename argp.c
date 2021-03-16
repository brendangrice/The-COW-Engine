#include "argp.h"
/*
 * Very rushed and pale imitation of <argp.h> for portability
 * Made with a few assumptions on my part and most elements taken from <argp.c> and <argp.h>
 */

error_t 
argp_parse (const struct argp *__restrict __argp, int __argc, char **__restrict __argv, unsigned __flags, int *__restrict argv, void *__restrict __input) 
{
	// iterate through arguments and try to parse with __argp->parser
	char *str;
	struct argp_state __argp_state;
	__argp_state.argc = __argc;
	__argp_state.argv = __argv;
	__argp_state.input = __input;
	__argp_state.arg_num = 1; // only handling one at a time
	__argp_state.root_argp = __argp;

	for (int i = 1; i<__argc; i++) {
		__argp_state.next = i;
		if(__argv[i][0] == '-') { // some sort of argument
			if(__argv[i][1] == '-') { // some sort of argument string
				// iterate through and match with name
				str = __argv[i]+2; // str after --
				int match = 1;
				for (int j=0; __argp->options[i].name != 0 || __argp->options[i].key != 0; j++) {
					if (strcmp(str, __argp->options[j].name)==0) { // try to find a match
						__argp->parser( __argp->options[j].key, __argv[j+1], &__argp_state );
						match = 0;
					}
				}
				if (match) argp_usage(&__argp_state, stderr); // if no match was found
			} else { // else there's a flag
				int pos = 1;
				// while there are flags to parse
				while (__argv[i][pos]) __argp->parser( __argv[i][pos++], __argv[i+1], &__argp_state );
			}
		} else { //loose
			__argp->parser( ARGP_KEY_ARG, __argv[i], &__argp_state );
		}
		if (__argp_state.next>i) i = __argp_state.next;
	}
	return 0;
}

void
argp_usage (const struct argp_state *__restrict __state, FILE *s) 
{
	// Usage: {EXE} [-{FLAGS}] [--{ARGUMENT}]... {ARGS_DOC}
	const struct argp *__argp = __state->root_argp;
	if (__state->argv[0][0]=='.'&&__state->argv[0][1]=='/') { // formatting
		fprintf(s, "Usage: %s [-", __state->argv[0]+2);
	} else 	fprintf(s, "Usage: %s [-", __state->argv[0]);
	for (int i=0; __argp->options[i].key != 0; i++) if (__argp->options[i].key>' ' && !__argp->options[i].arg) fputc(__argp->options[i].key, s); // list all the flags
	fputs("] ", s);
	for (int i=0; __argp->options[i].key != 0; i++) if (__argp->options[i].key>' ' && __argp->options[i].arg) fprintf(s, "[-%c %s] ", (char) __argp->options[i].key, __argp->options[i].arg); // list all the flags
	for (int i=0; __argp->options[i].name != 0; i++) {
		if (__argp->options[i].arg) {
			fprintf(s, "[--%s=%s] ", __argp->options[i].name, __argp->options[i].arg);
		} else	fprintf(s, "[--%s] ", __argp->options[i].name); // list all the flags
	}
	fputs(__argp->args_doc, s);
	if (s==stderr) exit(1); // maybe a more descriptive error code could be used
	exit(0);
}

void 
argp_help (const struct argp_state *__restrict __state, FILE *s)
{	
	/* 
	 * Usage: {EXE} [OPTIONS...] {ARGS_DOC}
	 * {DOC}
	 * -{FLAG}, {ARGUMENT}, {ARGUMENT DESCRIPTION} // with tabs for spacing
	 * {\v split on DOC}
	 * Usage: chess [OPTIONS...] [FILE]
	 *
	 
	 * Usage: chess [OPTIONS...] [FILE]
	 * Plays Chess.
	 * With FILE try to interpret FILE as PGN. When FILE is - read standard input as PGN
	 * 
	 *   -s, --step                 Steps through the moves when viewing a PGN file
	 *   -o, --output=FILE          Output PGN notation to specified FILE instead of standard date notation
	 *   -?, --help                 Give this help list
	 *       --usage                Give a short usage message
	 *   -V, --version              Print program version
	 * 
	 * more docs
	 *
	 */
	const struct argp *__argp = __state->root_argp;
	char doc[strlen(__argp->doc)]; // string that can be manipulated
	strcpy(doc, __argp->doc);
	if (__state->argv[0][0]=='.'&&__state->argv[0][1]=='/') { // formatting
		fprintf(s, "Usage: %s [OPTIONS...] %s\n", __state->argv[0]+2, __argp->args_doc);
	} else 	fprintf(s, "Usage: %s [OPTIONS...] %s\n", __state->argv[0], __argp->args_doc);
	
	//split up doc
	
	char *secondhalf = strchr(doc, '\v');
	if (secondhalf!=NULL) (secondhalf++)[0] = '\0'; // find and replace '\v' to make two strings

	char key[4] = {'-', 0, ',', 0};
	char name[30];
	name[0] = '-';
	name[1] = '-';

	fputs(doc, s);
	fputc('\n',s);
	for (int i=0; __argp->options[i].name != 0 || __argp->options[i].key != 0; i++) {
		key[0] = '-';
		key[1] = __argp->options[i].key;
		if (key[1]<=' ') key[0] = 0; // proper spacing
		strncpy(name+2, __argp->options[i].name, 20);
		if (__argp->options[i].arg!=NULL) { // --output=FILE
			strncat(name, "=", 2);
			strncat(name, __argp->options[i].arg, 9);
		}
		fprintf(s, "%5s %-23s%s\n", key, name, __argp->options[i].doc);	
	}
	fputc('\n',s);

	fputs(secondhalf, s);

	
	exit(0);
}

void
argp_version ()
{
	puts(argp_program_version);
	exit(0);
}
