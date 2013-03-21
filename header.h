/*
 * Copyright (c) 2004, 2005  Andrei Vasiliu
 *
 *
 * This file is part of MudBot.
 *
 * MudBot is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MudBot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MudBot; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


/* Main header file, to be used with all source files. */

#define HEADER_ID "$Name: Release_6 $ $Id: header.h,v 3.1 2005/12/08 18:02:51 andreivasiliu Exp $"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#if defined( FOR_WINDOWS )
# include <windows.h>
#endif


/* We need some colors, to use with clientf( ). */
#define C_0 "\33[0;37m"
#define C_D "\33[1;30m"
#define C_R "\33[1;31m"
#define C_G "\33[1;32m"
#define C_Y "\33[1;33m"
#define C_B "\33[1;34m"
#define C_M "\33[1;35m"
#define C_C "\33[1;36m"
#define C_W "\33[1;37m"

#define C_d "\33[0;30m"
#define C_r "\33[0;31m"
#define C_g "\33[0;32m"
#define C_y "\33[0;33m"
#define C_b "\33[0;34m"
#define C_m "\33[0;35m"
#define C_c "\33[0;36m"
#define C_w "\33[0;37m"


/* Portability stuff. */
#if defined( FOR_WINDOWS )
# define TELOPT_ECHO	1 /* client local echo */
# define IAC	      255 /* interpret as command */
# define WILL	      251 /* I will use option */
# define GA	      249 /* you may reverse the line */
# define DONT	      254 /* you are not to use option */
# define DO	      253 /* please, you use option */
# define WONT	      252 /* I won't use option */
# define SB	      250 /* interpret as subnegotiation */
# define SE	      240 /* end sub negotiation */
# define EOR	      239 /* end of record (transparent mode) */
#else
# include <arpa/telnet.h>
#endif

#ifdef _MSC_VER
# define __attribute__(x)
#endif


#define TELOPT_ATCP	      200
#define TELOPT_GMCP	      201

#define TELOPT_COMPRESS 85  /* MCCP - Mud Client Compression Protocol. */
#define TELOPT_COMPRESS2 86 /* MCCPv2 */
#define TELOPT_MXP 91       /* MXP - Mud eXtension Protocol. */

#define INPUT_BUF 4096

/*
#ifndef __attribute__
# define __attribute__( params ) ;
#endif
 */

/* Values to be used with mxp_tag. */
#define TAG_NOTHING	-2
#define TAG_DEFAULT	-1
#define TAG_OPEN	0
#define TAG_SECURE 	1
#define TAG_LOCKED	2
#define TAG_RESET	3
#define TAG_TEMP_SECURE	4
#define TAG_LOCK_OPEN	5
#define TAG_LOCK_SECURE	6
#define TAG_LOCK_LOCKED	7


typedef struct module_data MODULE;
typedef struct descriptor_data DESCRIPTOR;
typedef struct timer_data TIMER;
typedef struct line_data LINE;
typedef struct lines_data LINES;

typedef void *(*fn_ptr_t)( char * );

/* Module information structure. */
struct module_data
{
   char *name;

   void (*register_module)( MODULE *self );

   /* This is set by MudBot, and used by modules. */
   void *(*get_func)( char *name );
  /* fn_ptr_t (*get_func)( char *name );*/


#if defined( FOR_WINDOWS )
   HINSTANCE dll_hinstance;
#else
   void *dl_handle;
#endif
   char *file_name;

   MODULE *next;

   /* These will be set by the module. */
   int version_major;
   int version_minor;
   char *id;

   void (*init_data)( );
   void (*unload)( );
   void (*show_notice)( );
   void (*process_server_line)( LINE *line );
   void (*process_server_prompt)( LINE *line );
   void (*process_server_line_prefix)( char *colorless_line, char *colorful_line, char *raw_line );
   void (*process_server_line_suffix)( char *colorless_line, char *colorful_line, char *raw_line );
   void (*process_server_gmcp)( char *gmcp );
   void (*process_server_prompt_informative)( char *line, char *rawline );
   void (*process_server_prompt_action)( char *rawline );
   int  (*process_client_command)( char *cmd );
   int  (*process_client_aliases)( char *cmd );
   char*(*build_custom_prompt)( );
   int  (*main_loop)( int argc, char **argv );
   void (*update_descriptors)( );
   void (*update_modules)( );
  /* void (*update_timers)( );*/
   void (*debugf)( char *string );
   void (*mxp_enabled)( );
};


/* Sockets/descriptors. */
struct descriptor_data
{
   char *name;
   char *description;
   MODULE *mod;

   int deleted;
   int fd;

   void (*callback_in)( DESCRIPTOR *self );
   void (*callback_out)( DESCRIPTOR *self );
   void (*callback_exc)( DESCRIPTOR *self );

   DESCRIPTOR *next;
};

/* Timers. */ /*
struct timer_data
{
   char *name;
   int delay;
   MODULE *mod;

   TIMER *next;

   int data[3];
   void (*callback)( TIMER *timer );
};
*/
struct timer_data
{
   char *name;
   time_t fire_at_sec;
   time_t fire_at_usec;
   MODULE *mod;

   TIMER *next;

   int data[3];
   void *pdata[3];
   void (*callback)( TIMER *timer );
   void (*destroy_cb)( TIMER *timer );
};



/* Line or prompt from the server. */
struct line_data
{
   char line[INPUT_BUF];
   char raw_line[INPUT_BUF];
   char ending[32];

   char raw[INPUT_BUF];
   int raw_offset[INPUT_BUF+1];
   char *rawp[INPUT_BUF+1];

   int len;
   int raw_len;

   short within_color_code;

   char *prefix;
   char *suffix;
   char *replace;
   char *inlines[INPUT_BUF];

   short gag_entirely;
   short gag_ending;
   short gag_character[INPUT_BUF];
   short gag_raw[INPUT_BUF];
};

/* A set of lines and a prompt from the server. */
struct lines_data
{
   char *raw;			// raw_buffer_size - get_raw
   int raw_len;			// 0 - get_raw_length

   char *lines;			// normal_buffer_size - get_lines()
   char **line;			// ? (max nbs) - get_line(i)
   int full_len;		// 0 - get_full_length()
   int *len;			// ? (max nbs) - get_line_length(i)
   int *line_start;		// ? (max nbs) - get_line_start(i)
   int *raw_line_start;		// ? (max nbs) - get_raw_line_start(i)
   char **colour;		// normal_buffer_size - get_colour(i, i)
   int nr_of_lines;		// 0 - get_nr_of_lines()
   char *prompt;		// 0 - same as line[last] - get_prompt()
   int prompt_len;		// 0 - same as len[last] - get_prompt_length()

   /* --- */

   char *zeroed_lines;		// normal_buffer_size - hidden
   char ending[3];		// ? - hidden
   char *insert_point;		// raw_buffer_size - hidden

   /* --- */

   short *gag_char;
   char **inlines;

   struct line_info_data
     {
	short hide_line;
	char *prefix;
	char *suffix;
	char *replace;
	char *append_line;
     } *line_info;

};
