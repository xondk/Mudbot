/* Blank Module, that does absolutely nothing. */

#define BLANK_ID "$Name: Release_6 $ $Id: blank.c,v 3.1 2005/12/08 18:02:51 andreivasiliu Exp $"

/* #include <...> */

#include "module.h"


int blank_version_major = 0;
int blank_version_minor = 0;

char *blank_id = BLANK_ID "\r\n" HEADER_ID "\r\n" MODULE_ID "\r\n";



/* Here we register our functions. */

void blank_init_data( );


ENTRANCE( blank_module_register )
{
   self->name = strdup( "BlankMod" );
   self->version_major = blank_version_major;
   self->version_minor = blank_version_minor;
   self->id = blank_id;
   
   self->init_data = blank_init_data;
   self->process_server_line_prefix = NULL;
   self->process_server_line_suffix = NULL;
   self->process_server_prompt_informative = NULL;
   self->process_server_prompt_action = NULL;
   self->process_client_command = NULL;
   self->process_client_aliases = NULL;
   self->build_custom_prompt = NULL;
   
   self->main_loop = NULL;
   self->update_descriptors = NULL;
   self->update_modules = NULL;
   self->update_timers = NULL;
   self->debugf = NULL;
   
   GET_FUNCTIONS( self );
}



void blank_init_data( )
{
   
   
}

/* Called before unloading. *
void blank_unload( )
{
   
   
}
*/

/* Not yet called anywhere, but reserved for copyright notices. *
void blank_show_notice( )
{
   
   
}
*/

/* Called before every normal line.
 * Args: colorless_line = String with all color codes stripped.
 *       colorful_line = String with all non-printable characters stripped.
 *       raw_line = String containing the data as it came from the server.
 *
void blank_process_server_line_prefix( char *colorless_line, char *colorful_line, char *raw_line )
{
   
   
}
*/

/* Called after every normal line.
 * Args: Check above.
 *
void blank_process_server_line_suffix( char *colorless_line, char *colorful_line, char *raw_line )
{
   
   
}
*/

/* Called before every prompt.
 * Args: line = String with all colors stripped.
 *       rawline = String with everything in it.
 *
void blank_process_server_prompt_informative( char *line, char *rawline )
{
   
   
}
*/

/* Called after every prompt.
 * Args: rawline = String with everything in it.
 *
void blank_process_server_prompt_action( char *rawline )
{
   
   
}
*/

/* Called for every client command that begins with `.
 * Args: cmd = The command, including the initial symbol.
 * Returns: 1 = Command is known.
 *          0 = Command is unknown, pass it to the other modules.
 *
int blank_process_client_command( char *cmd )
{
   
   
}
*/

/* Called for every client command.
 * Args: cmd = The command string.
 * Returns: 1 = Don't send it further to the server.
 *          0 = Check it with other modules, and send it to the server.
 *
int blank_process_client_aliases( char *cmd )
{
   
   
}
*/

/* Called before a prompt, but after process_prompt_informative.
 * Returns: NULL = Use the normal prompt.
 *          string = Use this instead of the normal prompt.
 *
char *blank_build_custom_prompt( )
{
   
   
}
*/

