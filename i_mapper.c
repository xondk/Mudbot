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


/* Imperian internal mapper. */

#define I_MAPPER_ID "$Name: Release_8 $ $Id: i_mapper.c,v 4.0 2007/25/08 18:02:51 martinn Exp $"

#include "module.h"
#include "i_mapper.h"

#include <sys/stat.h>

int mapper_version_major = 5;
int mapper_version_minor = 74;


char *i_mapper_id = I_MAPPER_ID "\r\n" I_MAPPER_H_ID "\r\n" HEADER_ID "\r\n" MODULE_ID "\r\n";

char *room_color = "\33[33m";
char *daynight_color = "\33[0;33m";
char *exit_color = "\33[1;34m";
//char *room_color = "\33[35m";
int room_color_len = 5;

char map_file[256] = "IMap";
char map_file_bin[256] = "IMap.bin";

/* A few things we'll need. */
char *dir_name[] =
{ "none", "north", "northeast", "east", "southeast", "south",
	"southwest", "west", "northwest", "up", "down", "in", "out", NULL };

char *dir_small_name[] =
{ "-", "n", "ne", "e", "se", "s", "sw", "w", "nw", "u", "d", "in", "out", NULL };

char *dir_rev_name[] =
{ "none", "south", "southwest", "west", "northwest", "north",
	"northeast", "east", "southeast", "down", "up", "out", "in", NULL };

int reverse_exit[] =
{ 0, EX_S, EX_SW, EX_W, EX_NW, EX_N, EX_NE, EX_E, EX_SE,
	EX_D, EX_U, EX_OUT, EX_IN, 0 };


ROOM_TYPE *room_types;
ROOM_TYPE *null_room_type;

/*
   ROOM_TYPE room_types[256] =
   {  { "Unknown",			C_r,	1, 1, 0, 0 },
   { "Undefined",		C_R,	1, 1, 0, 0 },
   { "Road",			C_D,	1, 1, 0, 0 },
   { "Path",			C_D,	1, 1, 0, 0 },
   { "Natural underground",	C_y,	1, 1, 0, 0 },
   { "River",			C_B,	3, 3, 1, 0 },
   { "Ocean",			C_B,	3, 3, 1, 0 },
   { "Grasslands",		C_G,	1, 1, 0, 0 },
   { "Forest",			C_g,	1, 1, 0, 0 },
   { "Beach",			C_Y,	1, 1, 0, 0 },
   { "Garden",			C_G,	1, 1, 0, 0 },
   { "Urban",			C_D,	1, 1, 0, 0 },
   { "Hills",			C_y,	1, 1, 0, 0 },
   { "Mountains",		C_y,	1, 1, 0, 0 },
   { "Desert",			C_y,	1, 1, 0, 0 },
   { "Jungle",			C_g,	1, 1, 0, 0 },
   { "Valley",			C_y,	1, 1, 0, 0 },
   { "Freshwater",		C_B,	3, 3, 1, 0 },
   { "Constructed underground",	C_y,	1, 1, 0, 0 },
   { "Swamp",			C_y,	1, 1, 0, 0 },
   { "Underworld",		C_R,	1, 1, 0, 0 },
   { "Sewer",			C_D,	1, 1, 0, 0 },
   { "Tundra",			C_W,	1, 1, 0, 0 },
   { "Sylayan city",		C_D,	1, 1, 0, 0 },
   { "Crags",			C_D,	1, 1, 0, 0 },
   { "Deep Ocean",		C_B,	1, 1, 0, 1 },
   { "Dark Forest",		C_D,	1, 1, 0, 0 },
   { "Polar",			C_W,	1, 1, 0, 0 },
   { "Warrens",			C_y,	1, 1, 0, 0 },
   { "Dwarven city",		C_D,	1, 1, 0, 0 },
   { "Underground Lake",	C_B,	3, 3, 1, 0 },
   { "Tainted Water",		C_R,	5, 5, 1, 0 },
   { "Farmland",		C_Y,	1, 1, 0, 0 },
   { "Village",			C_c,	1, 1, 0, 0 },
   { "Academia",		C_c,	1, 1, 0, 0 },

   { NULL, NULL, 0, 0, 0 }
   };*/


COLOR_DATA color_names[] =
{
	{ "normal",	 C_0,		"\33[0m", 4 },
	{ "red",		 C_r,		"\33[31m", 5 },
	{ "green",		 C_g, 		"\33[32m", 5 },
	{ "brown",		 C_y, 		"\33[33m", 5 },
	{ "blue",		 C_b, 		"\33[34m", 5 },
	{ "magenta",	 "\33[0;35m", 	"\33[35m", 5 },
	{ "cyan",		 "\33[0;36m", 	"\33[36m", 5 },
	{ "white",		 "\33[0;37m", 	"\33[37m", 5 },
	{ "dark",		 C_D, 		C_D, 7 },
	{ "bright-red",	 C_R, 		C_R, 7 },
	{ "bright-green",	 C_G, 		C_G, 7 },
	{ "bright-yellow",	 C_Y, 		C_Y, 7 },
	{ "bright-blue",	 C_B, 		C_B, 7 },
	{ "bright-magenta", "\33[1;35m", 	"\33[1;35m", 7 },
	{ "bright-cyan",	 C_C, 		C_C, 7 },
	{ "bright-white",	 C_W, 		C_W, 7 },

	{ NULL, NULL, 0 }
};


COLOR_DATA daynight_names[] =
{
	{ "normal",	 "\33[0m",		"\33[0m", 4 },
	{ "red",		 "\33[0;31m",	"\33[0;31m", 7 },
	{ "green",		 "\33[0;32m", 	"\33[0;32m", 7 },
	{ "brown",		 "\33[0;33m", 	"\33[0;33m", 7 },
	{ "blue",		 "\33[0;34m", 	"\33[0;34m", 7 },
	{ "magenta",	 "\33[0;35m", 	"\33[0;35m", 7 },
	{ "cyan",		 "\33[0;36m", 	"\33[0;36m", 7 },
	{ "white",		 "", 	"", 0 },
	{ "dark",		 "", 	"", 0 },
	{ "bright-red",	 "\33[31m", "\33[31m", 4 },
	{ "bright-green",	 "\33[32m", "\33[32m", 4 },
	{ "bright-yellow",	 "\33[33m", "\33[33m", 4 },
	{ "bright-blue",	 "\33[34m", "\33[34m", 4 },
	{ "bright-magenta", "\33[35m", "\33[35m", 4 },
	{ "bright-cyan",	 "\33[36m", "\33[36m", 4 },
	{ "bright-white",	 "\33[37m", "\33[37m", 4 },

	{ NULL, NULL, 0 }
};

CITY_DATA cities[] =
{
	{"Ashtan", C_c, 0 },
	{"Bloodloch", C_R, 0},
	{"Duiran", C_G, 0},
	{"Enorian", C_Y, 0},
	{"Spinesreach", C_y, 0},
	{"Drakkenmont", C_D, 0},

	{ NULL, NULL, 0 }
};

DIVINE_DATA *divinelist;

/* Misc. */
int parsing_room;
int waitingforspecial;
int waitfornoexitline = 0;
int mode;
#define NONE		0
#define FOLLOWING	1
#define CREATING	2
#define GET_UNLOST	3

int get_unlost_exits;
int get_unlost_detected_exits[13];
int auto_walk;
int pear_defence;
int sense_message;
int leyline_message;
int scout_list;
int trap_list;
int evstatus_list;
int pet_list;
int destroying_world;
int door_closed;
int door_locked;
int door_unlocked;
int door_opened;
int swim_next = 0;
int walk_next = 0;
int locate_arena;
int capture_special_exit;
char cse_command[5120];
char cse_message[5120];
int close_rmtitle_tag;
int similar;
int title_offset;
int floating_map_enabled;
int skip_newline_on_map;
int gag_next_prompt;
int check_for_duplicates;
int burrowed;
int burrowtype = 0;
int flying;
int justwarped = 0;
int galloping;
int areaonly = 0;
int areaandadj = 0;
int spexitwalk = 0;
int moved = 0;
int werescent = 0;
int werepack = 0;
int wormsources = 0;
int ssight = 0;
int sldest = 0;
int farsight = 0;
int norulerc = 0;
int vnumfound = 0;
char ssize[128];
char smajor[128];

char ssightdiv[256] = "unset";
char werescentmsg[512];
AREA_DATA *igareaoff;
/* config.mapper.txt options. */
int disable_swimming;
int disable_wholist;
int disable_alertness;
int disable_locating;
int disable_areaname;
int disable_mxp_title = 1;
int disable_mxp_exits = 1;
int disable_mxp_map;
int disable_autolink;
int disable_artifacts;
int disable_additional_name = 1;
int disable_worm_warp = 1;
int disable_shrineinfluence;
int disable_shrineradius = 1;
int disable_farsightpath;
int disable_automap = 1;
int disable_mapterritory = 1;
int disable_autobackup;
int disable_autoshrine = 1;
int disable_forcewrap = 1;
int disable_vnum = 1;
int disable_pathwrap = 0;
int disable_shrineradiuscheck = 0;

char *underitem;

char *wingcmd;
int wingroom = 0;
int wingtmpdisable = 0;
int autowing = 0;

int wateroption = 0;
int automapsize = 0;
int disable_auto_unlock;

char *dash_command;
char *artimsg;
int mounted = 0;

/* troop stuff */
char troopn[256];
int troopmove;
/* guard stuff */
char guardnum[256];
int guardmove;

short int onlogin = 0;
/* fullline check bypass wrapwidth */
char fullline[8192];
int fulllinen;
int fulllineok = 0;

/* Bit Flags */
const unsigned char CHECK_AREA = 1;
const unsigned char CHECK_WORLD = 2;


int set_length_to;
int switch_exit_stops_mapping;
int switch_exit_joins_areas;
int update_area_from_survey;
int use_direction_instead;
int unidirectional_exit;

ROOM_DATA *world;
ROOM_DATA *world_last;
ROOM_DATA *current_room;
ROOM_DATA *old_sprint_room;
ROOM_DATA *hash_world[MAX_HASH];
ROOM_DATA *except;
ROOM_DATA *link_next_to;
ROOM_DATA *last_room;

AREA_DATA *areas;
AREA_DATA *areas_last;
AREA_DATA *current_area;

EXIT_DATA *global_special_exits;
WORMHOLE_DATA *wormholes;

ROOM_DATA *map[MAP_X+2][MAP_Y+2];
char extra_dir_map[MAP_X+2][MAP_Y+2];

/* New! */
MAP_ELEMENT **map_new;
int last_vnum;
int map_x = 14;
int map_y = 10;

/* Path finder chains. */
ROOM_DATA *pf_current_openlist;
ROOM_DATA *pf_new_openlist;

/* for do_exit_explore */
AREA_DATA exparea;
int exitexploring = 0;

/* Command queue. -1 is look, positive number is direction. */
int queue[256];
int q_top;
int lastdir;

int auto_bump;
int bump_compass;
int bump_eye;
int force_save;
ROOM_DATA *bump_room;
int bump_exits;
int area_search;
int searching;
ROOM_DATA *search_room;
char area_search_for[256];
int searchnostop = 0;
int didmove = 0;

ROOM_DATA *havenstore;
ROOM_DATA *returnroom;
int nointeract = 0;
int nodoors = 0;
/* Here we register our functions. */

void i_mapper_module_init_data( );
void i_mapper_module_unload( );
void i_mapper_process_server_line( LINE *line );
void i_mapper_process_server_prompt( LINE *line );
int  i_mapper_process_client_command( char *cmd );
int  i_mapper_process_client_aliases( char *cmd );
void i_mapper_mxp_enabled( );
void locate_room_in_area( char *name, char *player,int nl, AREA_DATA *sarea );
void do_map_shrine(char *arg);
void do_map_path( char *arg );
int cmp_room_wing();

AREA_DATA *get_area_by_name( char *string );
int case_strstr( char *haystack, char *needle );

ENTRANCE( i_mapper_module_register )
{
	self->name = strdup( "IMapper" );
	self->version_major = mapper_version_major;
	self->version_minor = mapper_version_minor;
	self->id = i_mapper_id;

	self->init_data = i_mapper_module_init_data;
	self->unload = i_mapper_module_unload;
	self->process_server_line = i_mapper_process_server_line;
	self->process_server_prompt = i_mapper_process_server_prompt;
	self->process_client_command = NULL;
	self->process_client_aliases = i_mapper_process_client_aliases;
	self->build_custom_prompt = NULL;
	self->main_loop = NULL;
	self->update_descriptors = NULL;
	self->mxp_enabled = i_mapper_mxp_enabled;

	GET_FUNCTIONS( self );
}



int get_free_vnum( )
{
	last_vnum++;
	return last_vnum;
}





void link_to_area( ROOM_DATA *room, AREA_DATA *area )
{
	DEBUG( "link_to_area" );

	/* Link to an area. */
	if ( area )
	{
		if ( area->last_room )
		{
			area->last_room->next_in_area = room;
			room->next_in_area = NULL;
		}
		else
		{
			room->next_in_area = area->rooms;
			area->rooms = room;
		}

		area->last_room = room;
		room->area = area;
	}
	else if ( current_area )
		link_to_area( room, current_area );
	else if ( areas )
	{
		debugf( "No current area set, while trying to link a room." );
		link_to_area( room, areas );
	}
	else
		clientfr( "Can't link this room anywhere." );
}



void unlink_from_area( ROOM_DATA *room )
{
	ROOM_DATA *r;

	DEBUG( "unlink_from_area" );

	/* Unlink from area. */
	if ( room->area->rooms == room )
	{
		room->area->rooms = room->next_in_area;

		if ( room->area->last_room == room )
			room->area->last_room = NULL;
	}
	else
		for ( r = room->area->rooms; r; r = r->next_in_area )
		{
			if ( r->next_in_area == room )
			{
				r->next_in_area = room->next_in_area;

				if ( room->area->last_room == room )
					room->area->last_room = r;

				break;
			}
		}
}



ROOM_DATA *create_room( int vnum_create )
{
	ROOM_DATA *new_room;

	DEBUG( "create_room" );

	/* calloc will also clear everything to 0, for us. */
	new_room = calloc( sizeof( ROOM_DATA ), 1 );

	/* Init. */
	new_room->name = NULL;
	new_room->special_exits = NULL;
	new_room->room_type = null_room_type;
	if ( vnum_create < 0 )
		new_room->vnum = get_free_vnum( );
	else
		new_room->vnum = vnum_create;

	/* Link to main chain. */
	if ( !world )
	{
		world = new_room;
		world_last = new_room;
	}
	else
		world_last->next_in_world = new_room;

	new_room->next_in_world = NULL;
	world_last = new_room;

	/* Link to hashed table. */
	if ( !hash_world[new_room->vnum % MAX_HASH] )
	{
		hash_world[new_room->vnum % MAX_HASH] = new_room;
		new_room->next_in_hash = NULL;
	}
	else
	{
		new_room->next_in_hash = hash_world[new_room->vnum % MAX_HASH];
		hash_world[new_room->vnum % MAX_HASH] = new_room;
	}

	/* Link to current area. */
	link_to_area( new_room, current_area );

	return new_room;
}



AREA_DATA *create_area( )
{
	AREA_DATA *new_area;

	DEBUG( "create_area" );

	new_area = calloc( sizeof( AREA_DATA ), 1 );

	/* Init. */
	new_area->name = NULL;
	new_area->rooms = NULL;

	/* Link to main chain. */
	if ( !areas )
	{
		areas = new_area;
		areas_last = new_area;
	}
	else
		areas_last->next = new_area;

	new_area->next = NULL;
	areas_last = new_area;

	return new_area;
}



EXIT_DATA *create_exit( ROOM_DATA *room )
{
	EXIT_DATA *new_exit, *e;

	new_exit = calloc( sizeof ( EXIT_DATA ), 1 );

	new_exit->vnum = -1;

	/* If room is null, create it in the global list. */
	if ( room )
	{
		if ( room->special_exits )
		{
			e = room->special_exits;
			while ( e->next )
				e = e->next;
			e->next = new_exit;
		}
		else
		{
			room->special_exits = new_exit;
		}

		new_exit->next = NULL;
		new_exit->owner = room;
	}
	else
	{
		if ( global_special_exits )
		{
			e = global_special_exits;
			while ( e->next )
				e = e->next;
			e->next = new_exit;
		}
		else
		{
			global_special_exits = new_exit;
		}

		new_exit->next = NULL;
		new_exit->owner = NULL;
	}

	return new_exit;
}


WORMHOLE_DATA *create_wormhole( ROOM_DATA *room )
{
	WORMHOLE_DATA *new_wormhole;

	new_wormhole = calloc( sizeof ( WORMHOLE_DATA ), 1 );

	new_wormhole->vnum = -1;

	if ( room )
	{
		if ( !room->wormhole )
		{
			room->wormhole = new_wormhole;
		}

		new_wormhole->owner = room;
	}
	else
	{
		new_wormhole->owner = NULL;
	}

	return new_wormhole;
}


void free_exit( EXIT_DATA *spexit )
{
	if ( spexit->command )
		free( spexit->command );
	if ( spexit->message )
		free( spexit->message );
	free( spexit );
}



void check_pointed_by( ROOM_DATA *room )
{
	ROOM_DATA *r;
	EXIT_DATA *e;

	DEBUG( "check_pointed_by" );

	if ( !room || destroying_world )
		return;

	for ( r = world; r; r = r->next_in_world )
	{
		for ( e = r->special_exits; e; e = e->next )
		{
			if ( e->to == room )
			{
				room->pointed_by = 1;
				return;
			}
		}
	}

	room->pointed_by = 0;
}


void check_wormpointed_by( ROOM_DATA *room )
{
	ROOM_DATA *r;
	WORMHOLE_DATA *w;

	DEBUG( "check_wormpointed_by" );

	if ( !room || destroying_world )
		return;

	for ( r = world; r; r = r->next_in_world )
	{
		for ( w = r->wormhole; w; w = w->next )
		{
			if ( w->to == room )
			{
				room->worm_pointed_by = 1;
				return;
			}
		}
	}

	room->worm_pointed_by = 0;
}


void link_element( ELEMENT *elem, ELEMENT **first )
{
	elem->next = *first;
	if ( elem->next )
		elem->next->prev = elem;
	elem->prev = NULL;

	*first = elem;
	elem->first = first;
}



void unlink_element( ELEMENT *elem )
{
	if ( elem->prev )
		elem->prev->next = elem->next;
	else
		*(elem->first) = elem->next;

	if ( elem->next )
		elem->next->prev = elem->prev;
}



void free_room( ROOM_DATA *room )
{
	EXIT_DATA *e, *e_next;
	ROOM_DATA *r;

	if ( room->name )
		free( room->name );

	for ( e = room->special_exits; e; e = e_next )
	{
		e_next = e->next;

		if ( e->to )
		{
			r = e->to;
			e->to = NULL;
			check_pointed_by( r );
		}

		free_exit( e );
	}

	while ( room->tags )
		unlink_element( room->tags );

	free( room );
}



void destroy_room( ROOM_DATA *room )
{
	ROOM_DATA *r;

	unlink_from_area( room );

	/* Unlink from world. */
	if ( world == room )
	{
		world = room->next_in_world;
		if ( room == world_last )
			world_last = world;
	}
	else
		for ( r = world; r; r = r->next_in_world )
		{
			if ( r->next_in_world == room )
			{
				r->next_in_world = room->next_in_world;
				if ( room == world_last )
					world_last = r;
				break;
			}
		}

	/* Unlink from hash table. */
	if ( room == hash_world[room->vnum % MAX_HASH] )
		hash_world[room->vnum % MAX_HASH] = room->next_in_hash;
	else
	{
		for ( r = hash_world[room->vnum % MAX_HASH]; r; r = r->next_in_hash )
		{
			if ( r->next_in_hash == room )
			{
				r->next_in_hash = room->next_in_hash;
				break;
			}
		}
	}

	/* Free it up. */
	free_room( room );
}


void destroy_area( AREA_DATA *area )
{
	AREA_DATA *a;

	/* Unlink from areas. */
	if ( area == areas )
	{
		areas = area->next;
		if ( area == areas_last )
			areas_last = areas; /* Always null, anyways. */
	}
	else
		for ( a = areas; a; a = a->next )
		{
			if ( a->next == area )
			{
				a->next = area->next;
				if ( area == areas_last )
					areas_last = a;
				break;
			}
		}

	/* Free it up. */
	free( area );
}


void destroy_wormhole( WORMHOLE_DATA *worm )
{
	ROOM_DATA *room;
	WORMHOLE_DATA *w;

	if ( ( room = worm->to ) )
	{
		worm->to = NULL;
		check_wormpointed_by( room );
	}

	/* Unlink from room */
	if ( ( room = worm->owner ) )
	{
		if ( room->wormhole == worm )
			room->wormhole = worm->next;
		else
			for ( w = room->wormhole; w; w = w->next )
				if ( w->next == worm )
				{
					w->next = worm->next;
					break;
				}
	}

	free( worm );
}


void destroy_exit( EXIT_DATA *spexit )
{
	ROOM_DATA *room;
	EXIT_DATA *e;

	if ( ( room = spexit->to ) )
	{
		spexit->to = NULL;
		check_pointed_by( room );
	}

	/* Unlink from room, or global. */
	if ( ( room = spexit->owner ) )
	{
		if ( room->special_exits == spexit )
			room->special_exits = spexit->next;
		else
			for ( e = room->special_exits; e; e = e->next )
				if ( e->next == spexit )
				{
					e->next = spexit->next;
					break;
				}
	}
	else
	{
		if ( global_special_exits == spexit )
			global_special_exits = spexit->next;
		else
			for ( e = global_special_exits; e; e = e->next )
				if ( e->next == spexit )
				{
					e->next = spexit->next;
					break;
				}
	}

	/* Free it up. */
	free_exit( spexit );
}


/* Destroy everything. */
void destroy_map( )
{
	ROOM_DATA *room, *next_room;
	AREA_DATA *area, *next_area;
	int i;

	DEBUG( "destroy_map" );

	destroying_world = 1;

	/* Rooms. */
	for ( room = world; room; room = next_room )
	{
		next_room = room->next_in_world;

		/* Free it up. */
		free_room( room );
	}

	/* Areas. */
	for ( area = areas; area; area = next_area )
	{
		next_area = area->next;

		/* Free it up. */
		if ( area->name )
			free( area->name );
		free( area );
	}

	/* Hash table. */
	for ( i = 0; i < MAX_HASH; i++ )
		hash_world[i] = NULL;

	/* Global special exits. */
	while ( global_special_exits )
		destroy_exit( global_special_exits );

	destroying_world = 0;

	world = world_last = NULL;
	areas = areas_last = NULL;
	current_room = NULL;
	current_area = NULL;

	mode = NONE;
}


/* Free -everything- up, and prepare to be destroyed. */
void i_mapper_module_unload( )
{
	del_timer( "queue_reset_timer" );
	del_timer( "remove_players" );

	destroy_map( );
}

void fulllinecheck( char *line )
{  char *t;
	if ((t = strrchr(line,46)) && !strcmp(t,".") )
	{
		strcat(fullline,line);
		fulllineok = 1;
		fulllinen=0;
	}
	else {
		strcat(fullline, line);
		if ( strcmp(fullline + strlen(fullline) - 1," ") )
			strcat(fullline," ");
		fulllinen++;
		if ( fulllinen > 3 ) {
			memset( fullline, '\0', sizeof(fullline) );
			fulllinen=0;}
		fulllineok = 0;
	}
}

/* This is what the hash table is for. */
ROOM_DATA *get_room( int vnum )
{
	ROOM_DATA *room;

	for ( room = hash_world[vnum % MAX_HASH]; room; room = room->next_in_hash )
		if ( room->vnum == vnum )
			return room;

	return NULL;
}

ROOM_DATA *get_room_atv( int vnum )
{
	ROOM_DATA *room;

	for ( room = world; room; room = room->next_in_world )
		if ( room->aetvnum == vnum )
			return room;

	return NULL;
}

ROOM_DATA *get_entrance_room_for_area( char *buf )
{
	ROOM_DATA *room, *neighbour;
	int exit;

	for ( room = world; room; room = room->next_in_world ) {

		/* Check if the names match */
		if ( !case_strstr(room->area->name, buf) ) {
			continue;
		}

		/* Check if atleast one of the exits lead to a different area */
		for(exit = 0; exit < 13; exit++) {
			if( !(neighbour = room->exits[exit]) ) {
				continue;
			}

			if (neighbour->area != room->area) {
				return room;
			}
		}
	}

	return NULL;
}

void queue_reset( TIMER *self )
{
	if ( !q_top )
		return;

	q_top = 0;

	clientff( "\r\n" C_R "[" C_D "Mapper's command queue cleared." C_R "]"
			C_0 "\r\n" );
	show_prompt( );
}


void add_queue( int value )
{
	int i;

	for ( i = q_top; i > 0; i-- )
		queue[i] = queue[i-1];
	q_top++;
	queue[0] = value;

	add_timer( "queue_reset_timer", 10, queue_reset, 0, 0, 0 );
}


void add_queue_top( int value )
{
	queue[q_top] = value;
	q_top++;

	add_timer( "queue_reset_timer", 10, queue_reset, 0, 0, 0 );
}


int must_swim( ROOM_DATA *src, ROOM_DATA *dest )
{
	if ( !src || !dest )
		return 0;

	if ( disable_swimming )
		return 0;

	if ( mounted )
		return 0;

	if ( flying )
		return 0;

	if ( ( src->underwater || src->room_type->underwater ) &&
			pear_defence )
		return 0;

	if ( src->room_type->must_swim || dest->room_type->must_swim )
		return 1;

	return 0;
}


int room_cmp( const char *room, const char *smallr )
{
	if ( !strcmp( room, smallr ) )
		return 0;

	return 1;
}


void set_reverse( ROOM_DATA *source, int dir, ROOM_DATA *destination )
{
	if ( ( !source && destination->more_reverse_exits[dir] ) ||
			( source && destination->reverse_exits[dir] &&
			  destination->reverse_exits[dir] != source ) )
	{
		ROOM_DATA *room;

		destination->reverse_exits[dir] = NULL;
		destination->more_reverse_exits[dir] = 0;

		for ( room = world; room; room = room->next_in_world )
		{
			if ( room->exits[dir] != destination )
				continue;

			if ( destination->reverse_exits[dir] )
				destination->more_reverse_exits[dir]++;
			else
				destination->reverse_exits[dir] = room;
		}
	}
	else
		destination->reverse_exits[dir] = source;
}

void automap_draw()
{
	int mxpb = 0;
	didmove = 0;
	clientf("\r\n");
	if ( !disable_mxp_map ) {
		disable_mxp_map = 1;
		mxpb = 1;
	}
	if ( automapsize == 3 ) {
		void do_map_big ( char *arg );
		do_map_big("");
	}
	else if ( automapsize == 2 ) {
		void do_map_medium ( char *arg );
		do_map_medium("");
	}
	else if ( automapsize == 1 ) {
		void do_map ( char *arg );
		do_map("");
	}
	else  {
		void do_map_tiny ( char *arg );
		do_map_tiny("");
	}
	if ( mxpb )
		disable_mxp_map = 0;
}

/* This is a title. Do something with it. */
int parse_title( const char *line )
{
	ROOM_DATA *new_room, *pcheck;
	int created = 0;
	int q, i;
	char buf2[256], *b = buf2;
	char *p;

	DEBUG( "parse_title" );

	/*Lets avoid that annoying date*/
	if ( !strncmp(line, "It is now the ", 14 ) )
		return -1;

	/* Capturing mode. */
	if ( mode == CREATING && capture_special_exit )
	{
		EXIT_DATA *spexit;

		if ( !current_room )
		{
			clientf( C_R " (Current Room is NULL! Capturing disabled)" C_0 );
			capture_special_exit = 0;
			mode = FOLLOWING;
			return -1;
		}

		if ( !cse_message[0] )
		{
			clientf( C_R " (No message found! Capturing disabled)" C_0 );
			capture_special_exit = 0;
			mode = FOLLOWING;
			return -1;
		}

		if ( capture_special_exit > 0 )
		{
			new_room = get_room( capture_special_exit );

			if ( !new_room )
			{
				clientf( C_R " (Destination room is now NULL! Capturing disabled)" C_0 );
				capture_special_exit = 0;
				mode = FOLLOWING;
				return -1;
			}

			/* Make sure the destination matches. */
			if ( strcmp( line, new_room->name ) &&
					!( similar && !room_cmp( new_room->name, line ) ) )
			{
				clientf( C_R " (Destination does not match! Capturing disabled)" C_0 );
				capture_special_exit = 0;
				mode = FOLLOWING;
				return -1;
			}
		}
		else
		{
			new_room = create_room( -1 );
			new_room->name = strdup( line );
		}

		clientff( C_R " (" C_W "sp:" C_G "%d" C_R ")" C_0, new_room->vnum );
		clientff( C_R "\r\n[Special exit created.]" );
		spexit = create_exit( current_room );
		spexit->to = new_room;
		spexit->vnum = new_room->vnum;

		clientff( C_R "\r\nCommand: '" C_W "%s" C_R "'" C_0,
				cse_command[0] ? cse_command : "null" );
		if ( cse_command[0] )
			spexit->command = strdup( cse_command );

		p = cse_message;
		while ( *p )
		{
			if ( *p == '"' )
				*(b++) = '*';
			else
				*(b++) = *p;
			p++;
		}
		*b = 0;

		clientff( C_R "\r\nMessage: '" C_W "%s" C_R "'" C_0,
				buf2 );
		spexit->message = strdup( buf2 );

		current_room = new_room;
		current_area = current_room->area;
		capture_special_exit = 0;
		return -1;
	}


	/* Following or Mapping mode. */
	if ( mode == FOLLOWING || mode == CREATING )
	{
		if ( galloping ) {
			if ( !disable_areaname )
				clientff( C_R " (" C_g "%s" C_R ")" C_0,
						current_room->area->name );
			if ( !disable_vnum )
				clientff( C_D " (" C_G "%d" C_D ")" C_0, current_room->vnum );
			if  ( !disable_shrineinfluence ) {
				if ( current_room->shrine || current_room->shrineeff ) {
					if ( current_room->shrine == 1 )
						clientff( C_Y " [" C_W "-%s-" C_Y "]" C_0, current_room->shrinedivine );
					else if ( current_room->shrine == 2 )
						clientff( C_Y " [" C_G "-%s-" C_Y "]" C_0, current_room->shrinedivine );
					else if ( current_room->shrine == 3 )
						clientff( C_Y " [" C_R "-%s-" C_Y "]" C_0, current_room->shrinedivine );
					else if ( current_room->shrineeff == 1 ) {
						pcheck = get_room( current_room->shrineparrent );
						clientff( C_Y " [" C_W "-%s-" C_Y "]" C_0, pcheck->shrinedivine );
					}
					else if ( current_room->shrineeff == 2 ) {
						pcheck = get_room( current_room->shrineparrent );
						clientff( C_Y " [" C_G "-%s-" C_Y "]" C_0, pcheck->shrinedivine );
					}
					else if ( current_room->shrineeff == 3 ) {
						pcheck = get_room( current_room->shrineparrent );
						clientff( C_Y " [" C_R "-%s-" C_Y "]" C_0, pcheck->shrinedivine );
					}
				}
			}
		}
		/* Queue empty? */
		if ( !q_top )
		{
			/* parsing_room = 0; */
			return 0;
		}

		q = queue[q_top-1];
		q_top--;

		if ( !q_top )
			del_timer( "queue_reset_timer" );

		if ( !current_room )
		{
			clientf( " (Current room is null, while mapping is not!)" );
			mode = NONE;
			/* parsing_room = 0; */
			return 0;
		}
		/* Not just a 'look'? */
		if ( q > 0 )
		{
			if ( mode == FOLLOWING )
			{
				if ( current_room->hasrandomexits && !current_room->trueexit[q] )
				{mode = GET_UNLOST;
				}
				else if ( current_room->exits[q] )
				{  current_room = current_room->exits[q];
					current_area = current_room->area;
				}
				else
				{
					/* We moved into a strange exit, while not creating. */
					clientf( C_R " (" C_G "lost" C_R ")" C_0 );
					current_room = NULL;
					mode = GET_UNLOST;
				}
				if ( !disable_automap ) {
					didmove = 1;}
				if ( troopmove == 2 && troopn != NULL )
					troopmove = 1;
				if ( guardmove == 2 && guardnum != NULL )
					guardmove = 1;
			}
			else if ( mode == CREATING )
			{
				if ( current_room->exits[q] )
				{
					/* Just follow around. */
					new_room = current_room->exits[q];
				}
				else
				{
					char *color = C_G;

					/* Check for autolinking. */
					if ( !link_next_to && !disable_autolink && !switch_exit_stops_mapping )
					{
						ROOM_DATA *get_room_at( int dir, int length );
						int length;

						if ( set_length_to == -1 )
							length = 1;
						else
							length = set_length_to + 1;

						link_next_to = get_room_at( q, length );

						if ( link_next_to && strcmp( line, link_next_to->name ) )
							link_next_to = NULL;

						if ( link_next_to && link_next_to->exits[reverse_exit[q]] )
							link_next_to = NULL;

						color = C_C;
					}

					/* Create or link an exit. */
					if ( link_next_to )
					{
						new_room = link_next_to;
						link_next_to = NULL;
						clientff( C_R " (%slinked" C_R ")" C_0, color );
					}
					else
					{
						new_room = create_room( -1 );
						clientf( C_R " (" C_G "created" C_R ")" C_0 );
						created = 1;
						send_to_server( "survey\r\n" );
						if ( !disable_autolink )
							check_for_duplicates = 1;
					}

					current_room->exits[q] = new_room;
					set_reverse( current_room, q, new_room );
					if ( !unidirectional_exit )
					{
						if ( !new_room->exits[reverse_exit[q]] )
						{
							new_room->exits[reverse_exit[q]] = current_room;
							set_reverse( new_room, reverse_exit[q], current_room );
						}
						else
						{
							current_room->exits[q] = NULL;
							clientf( C_R " (" C_G "unlinked: reverse error" C_R ")" );
						}
					}
					else
						unidirectional_exit = 0;
				}

				/* Change the length, if asked so. */
				if ( set_length_to )
				{
					if ( set_length_to == -1 )
						set_length_to = 0;

					current_room->exit_length[q] = set_length_to;
					if ( new_room->exits[reverse_exit[q]] == current_room )
						new_room->exit_length[reverse_exit[q]] = set_length_to;
					clientf( C_R " (" C_G "l set" C_R ")" C_0 );

					set_length_to = 0;
				}

				/* Stop mapping from here on? */
				if ( switch_exit_stops_mapping )
				{
					i = current_room->exit_stops_mapping[q];

					i = !i;

					current_room->exit_stops_mapping[q] = i;
					if ( new_room->exits[reverse_exit[q]] == current_room )
						new_room->exit_stops_mapping[reverse_exit[q]] = i;

					if ( i )
						clientf( C_R " (" C_G "s set" C_R ")" C_0 );
					else
						clientf( C_R " (" C_G "s unset" C_R ")" C_0 );

					switch_exit_stops_mapping = 0;
				}

				/* Show rooms even from another area? */
				if ( switch_exit_joins_areas )
				{
					i = current_room->exit_joins_areas[q];

					i = !i;

					if ( i && ( current_room->area == new_room->area ) )
						clientf( C_R " (" C_G "j NOT set" C_R ")" C_0 );
					else
					{
						current_room->exit_joins_areas[q] = i;
						if ( new_room->exits[reverse_exit[q]] == current_room )
							new_room->exit_joins_areas[reverse_exit[q]] = i;

						if ( i )
							clientf( C_R " (" C_G "j set" C_R ")" C_0 );
						else
							clientf( C_R " (" C_G "j unset" C_R ")" C_0 );
					}

					switch_exit_joins_areas = 0;
				}


				/* Show this somewhere else on the map, instead? */
				if ( use_direction_instead )
				{
					if ( use_direction_instead == -1 )
						use_direction_instead = 0;

					current_room->use_exit_instead[q] = use_direction_instead;
					if ( new_room->exits[reverse_exit[q]] == current_room )
						new_room->use_exit_instead[reverse_exit[q]] = reverse_exit[use_direction_instead];
					if ( use_direction_instead )
						clientf( C_R " (" C_G "u set" C_R ")" C_0 );
					else
						clientf( C_R " (" C_G "u unset" C_R ")" C_0 );

					use_direction_instead = 0;
				}

				current_room = new_room;
				current_area = new_room->area;
			}
		}

		if ( mode == CREATING )
		{if ( !current_room->name || strcmp( line, current_room->name ) )
			{
				for ( i = 1; i < 10; i++ )
				{
					if (current_room->additional_name[i] == NULL)
					{if ( !disable_additional_name && !created  ) {
																	  current_room->additional_name[i] = strdup( line );
																	  clientf(C_R " (" C_G "Additional Name Added" C_R ")" C_0);
																	  break;
																  } else {
																	  if ( !created ) {
																		  clientf( C_R " (" C_G "updated" C_R ")" C_0 );
																		  send_to_server( "survey\r\n" );
																	  }
																	  if ( current_room->name )
																		  free( current_room->name );
																	  current_room->name = strdup( line );
																	  break;
																  }}
					else if ( !strcmp( current_room->additional_name[i], line ) )
						break;
				}


			}
		}
		else if ( mode == FOLLOWING )
		{
			if ( strcmp( line, current_room->name ) &&
					!( similar && !room_cmp( current_room->name, line ) ) )
			{
				/* Didn't enter where we expected to? */
				for ( i = 1; i < 10; i++ )
				{
					if (current_room->additional_name[i] == NULL)
					{
						clientf( C_R " (" C_G "lost" C_R ")" C_0 );
						current_room = NULL;
						mode = GET_UNLOST;
						break;
					}
					else if ( !strcmp( current_room->additional_name[i], line ) )
						break;

				}

			}

		}
	}

	if ( mode == GET_UNLOST )
	{
		ROOM_DATA *r, *found = NULL;
		int more = 0;
		int breakout = 0;


		for ( r = world; r; r = r->next_in_world )
		{
			if ( !strcmp( line, r->name ) )
			{
				if ( !found )
					found = r;
				else
				{
					more = 1;
					break;
				}
			}
			for ( i = 1; i < 10; i++ )
			{
				if (r->additional_name[i] == NULL)
				{
					break;
				}
				else if ( !strcmp( r->additional_name[i], line ) )
				{
					if ( !found )
					{
						found = r;
						break;
					}
					else
					{
						more = 1;
						breakout=1;
						break;
					}
				}
			}
			if ( breakout )
				break;
		}



		if ( found )
		{
			current_room = found;
			current_area = found->area;
			mode = FOLLOWING;
			get_unlost_exits = more;
		}
	}

	if ( mode == CREATING )
	{
		clientff( C_R " (" C_G "%d" C_R ")" C_0, current_room->vnum );
		if ( !disable_automap ) {
			didmove = 1;}
	}
	else if ( mode == FOLLOWING )
	{
		if ( !disable_areaname )
			clientff( C_R " (" C_g "%s" C_R "%s)" C_0,
					current_room->area->name, get_unlost_exits ? "?" : "" );
		if ( !disable_vnum )
			clientff( C_D " (" C_G "%d" C_D ")" C_0, current_room->vnum );
		if  ( !disable_shrineinfluence ) {
			if ( current_room->shrine || current_room->shrineeff ) {
				if ( current_room->shrine == 1 )
					clientff( C_Y " [" C_W "-%s-" C_Y "]" C_0, current_room->shrinedivine );
				else if ( current_room->shrine == 2 )
					clientff( C_Y " [" C_G "-%s-" C_Y "]" C_0, current_room->shrinedivine );
				else if ( current_room->shrine == 3 )
					clientff( C_Y " [" C_R "-%s-" C_Y "]" C_0, current_room->shrinedivine );
				else if ( current_room->shrineeff == 1 ) {
					pcheck = get_room( current_room->shrineparrent );
					clientff( C_Y " [" C_W "-%s-" C_Y "]" C_0, pcheck->shrinedivine );
				}
				else if ( current_room->shrineeff == 2 ) {
					pcheck = get_room( current_room->shrineparrent );
					clientff( C_Y " [" C_G "-%s-" C_Y "]" C_0, pcheck->shrinedivine );
				}
				else if ( current_room->shrineeff == 3 ) {
					pcheck = get_room( current_room->shrineparrent );
					clientff( C_Y " [" C_R "-%s-" C_Y "]" C_0, pcheck->shrinedivine );
				}
			}
		}

	}

	if ( spexitwalk ) {
		spexitwalk = 0;
		if ( !disable_automap ) {
			didmove = 1;}
	}
	/* We're ready to move. */
	if ( mode == FOLLOWING && auto_walk == 1 && !burrowed && !justwarped )
		auto_walk = 2;

	/* parsing_room = 2; */
	return 2;
}



void parse_room( LINE *l )
{
	char *line = l->line;
	static int exit_offset;
	char *eol;
	int end_offset;
	int i;
	int BurrowFound;
	int FlyingFound;
	DEBUG( "parse_room" );

	BurrowFound = 0;
	FlyingFound = 0;
	/* Room title check. */

	if ( similar )
		similar = 0;
	if ( title_offset )
		title_offset = 0;

	/* Check the beginning, for a color. */
	if ( !parsing_room )
	{
		title_offset = 0;
		if ( !strncmp( l->line, "In the trees above ", 19 ) )
			title_offset = 18, similar = 1;
		if ( !strncmp( l->line, "On the rooftops above ", 22 ) )
			title_offset = 21, similar = 1;
		if ( !strncmp( l->line, "Flying above ", 13 ) )
			title_offset = 12, similar = 1,flying = 1,FlyingFound = 1;
		if ( !strncmp( l->line, "Buried beneath ", 15 ) )
		{
			title_offset = 14, similar = 1;
			burrowed = 1;
			BurrowFound = 1;
		}
		if ( !strncmp( l->line, "Buried deep beneath ", 20 ) )
		{
			title_offset = 19, similar = 1;
			burrowed = 1;
			BurrowFound = 1;
		}
		if ( strstr( l->rawp[title_offset], room_color/*, room_color_len*/ ) )
		{
			//	     insert( title_offset, "Y" );
			parsing_room = 1;
			if ( !BurrowFound )
				burrowed = 0;
			if ( !FlyingFound )
				flying = 0;
		}
		if ( strstr( l->rawp[title_offset], daynight_color/*, room_color_len*/ ) )
		{
			//	     insert( title_offset, "Y" );
			parsing_room = 1;
			if ( !BurrowFound )
				burrowed = 0;
			if ( !FlyingFound )
				flying = 0;
		}

		if ( title_offset )
			title_offset++;

		/* Still nothing? Maybe the color code is at the end...
		 * But this means the room is on the next line, not this. */
		if ( !title_offset && !parsing_room )
		{
			if ( l->rawp[l->len] && strstr( l->rawp[l->len], "35" ) )
			{
				//		  insert( l->len, "Z" );
				parsing_room = 1;
				if ( !BurrowFound )
					burrowed = 0;
				if ( !FlyingFound )
					flying = 0;
				return;
			}
		}
	}

	if ( !l->len || !parsing_room )
		return;



	/* Stage one, room name. */
	if ( parsing_room == 1 )
	{
		/* Check if it actually looks like a room. */
		if ( ( ( l->line[0] < 'A' || l->line[0] > 'Z' ) &&
					( l->line[0] < 'a' || l->line[0] > 'a' ) ) ||
				( l->line[l->len - 1] != '.' && l->line[l->len - 1] != ')' ) )
		{
			parsing_room = 0;
			return;
		}

		line += title_offset;

		end_offset = 0;

		/* Remove the road from: "Title. (road)." */
		if ( l->len > 9 && l->line[l->len-1] == ')' &&
				( eol = strstr( l->line, ". (" ) ) )
		{
			end_offset = ( eol - l->line ) + 1;
			end_offset -= title_offset;
		}

		/* Disabled for now. Zmud is just really sucky. *
		   if ( !disable_mxp_title )
		   {
		   char tag[256];
		   char buf[256];

		   if ( mxp_stag( TAG_SECURE, tag ) && tag[0] )
		   {
		   sprintf( buf, "x%s<RmTitle>", tag );
		   insert( title_offset, buf );
		   sprintf( buf, "%s</RmTitle>x", tag );
		   insert( end_offset ? end_offset : l->len, buf );
		   }
		   } */

		//	debugf( "Room!" );

		if ( waitingforspecial )
		{
			waitingforspecial = 0;
			EXIT_DATA *spexit;

			for ( spexit = current_room->special_exits; spexit; spexit = spexit->next )
			{
				if ( !spexit->to || room_cmp( spexit->to->name, line ) )
					continue;

				current_room = spexit->to;
				clientff( C_R " (" C_Y "sp" C_R ")" C_0 );
				return;
			}

		}

		if ( !end_offset )
			i = parse_title( line );

		else
		{
			char buf[256];

			strcpy( buf, line );
			buf[end_offset] = 0;

			i = parse_title( buf );
		}

		if ( i >= 0 )
			parsing_room = i;
	}


	/* Stage two, look for start of exits list. */
	if ( galloping )
		parsing_room = 2;
	if ( parsing_room == 2 )
	{
		static int sub_stage;
		/* Sub stages:
		 * 0 - Looking for a color code.
		 * 1 - Found it, looking for 'You'.
		 */

		if ( !current_room )
		{
			parsing_room = 0;
			return;
		}

		for ( i = 0; i < l->len; i++ )
		{
			/* Blue color. */
			if ( !sub_stage && *l->rawp[i] && strstr( l->rawp[i], exit_color ) )
				sub_stage = 1;

			if ( sub_stage == 1 )
			{
				//		  debugf( "(%d)", strlen( l->rawp[i] ) );
				//		  insert( i, "X" );
				if ( l->line[i] == ' ' || l->line[i] == '\0' )
					continue;

				if ( l->line[i] == 'Y' && !strncmp( l->line + i, "You", 3 ) )
					i += 3;

				else if ( !strncmp( l->line + i, "see", 3 ) )
					i += 3;
				else if ( !strncmp( l->line + i, "exits", 5 ) )
					i += 5;

				else if ( !strncmp( l->line + i, "feel", 4 ) )
					i += 4;
				else if ( !strncmp( l->line + i, "some", 4 ) )
					i += 4;
				else if ( !strncmp( l->line + i, "loose", 5 ) )
					i += 5;
				else if ( !strncmp( l->line + i, "soil", 4 ) )
					i += 4;
				else if ( !strncmp( l->line + i, "to", 2 ) )
					i += 2;
				else if ( !strncmp( l->line + i, "the", 3 ) )
					i += 3, line = l->line + i, sub_stage = 2, exit_offset = i;

				else if ( !strncmp( l->line + i, "a", 1 ) )
					i += 1;
				else if ( !strncmp( l->line + i, "a", 1 ) )
					i += 1;
				else if ( !strncmp( l->line + i, "single", 6 ) )
					i += 6;
				else if ( !strncmp( l->line + i, "exit", 4 ) )
					i += 4;
				else if ( !strncmp( l->line + i, "leading", 7 ) )
					i += 7, line = l->line + i, sub_stage = 2, exit_offset = i;
				else if ( !strncmp( l->line + i, "There are no obvious exits", 26 ) )
					i += 26, line = l->line + i, sub_stage = 2, exit_offset = i;
				else
				{
					sub_stage = 0;
					continue;
				}
			}
		}

		/* Beginning of exits. */
		if ( sub_stage == 2 )
		{
			if ( mode == CREATING || ( current_room->hasrandomexits && mode == FOLLOWING ) )
				for ( i = 1; dir_name[i]; i++ )
					current_room->detected_exits[i] = 0;
			else if ( get_unlost_exits )
				for ( i = 1; dir_name[i]; i++ )
					get_unlost_detected_exits[i] = 0;

			sub_stage = 0;
			parsing_room = 3;

			/*
			   if ( !disable_mxp_exits )
			   {
			   char tag[256];
			   char buf[256];

			   if ( mxp_stag( TAG_SECURE, tag ) && tag[0] )
			   {
			   sprintf( buf, "x%s<RmExits>", tag );
			   insert( exit_offset, buf );
			   }
			   } */
		}
	}

	/* Check for exits. If it got here, then current_room is checked. */
	if ( parsing_room == 3 )
	{
		char word[128];
		char *w;
		int j;
		int efirst = 1;

		i = exit_offset;

		while( i < l->len )
		{
			w = word;
			/* Extract a word. */
			while( l->line[i] && ( l->line[i] != ',' && l->line[i] != ' ' && l->line[i] != '.' && l->line[i] != 40) )
				*w++ = l->line[i++];
			*w = 0;

			/* Skip spaces and weird stuff. */
			while( l->line[i] == ',' || l->line[i] == ' ' || l->line[i] == 40 )
				i++;

			for ( j = 1; dir_name[j]; j++ )
			{
				if ( !strcmp( word, dir_name[j] ) )
				{
					if ( !current_room->hasrandomexits && !current_room->exits[j] && !current_room->locked_exits[j] ) {
						clientff(C_W "%s %s" C_0 , efirst ? "\r\nUnmapped:" : "", dir_name[j]);
						efirst = 0;
					}
					if ( current_room->hasrandomexits && !current_room->trueexit[j] )
					{clientff(C_C "%s %s" C_0 , efirst ? "\r\nRandom Exits:" : "", dir_name[j]);
						efirst = 0;
					}
					if ( mode == CREATING || ( current_room->hasrandomexits && mode == FOLLOWING ) || auto_bump )
						current_room->detected_exits[j] = 1;
					else if ( get_unlost_exits )
						get_unlost_detected_exits[j] = 1;
					else
					{
						//			    insert( beginning, "<" );
						//			    insert( end, ">" );
						//			    debugf( "Exit found: [%s]", word );
					}
					break;
				}
			}

			if ( l->line[i] == '.' )
			{
				parsing_room = 0;

				/*
				   if ( !disable_mxp_exits )
				   {
				   char tag[256];
				   char buf[256];

				   if ( mxp_stag( TAG_SECURE, tag ) && tag[0] )
				   {
				   sprintf( buf, "%s</RmExits>y", tag );
				   insert( i, buf );
				   }
				   } */
				break;
			}
		}
		if ( didmove == 1 ) {
			automap_draw();}
		exit_offset = 0;
	}
}



void check_area( char *name )
{
	AREA_DATA *area;

	if ( !strcmp( name, current_room->area->name ) )
		return;

	/* Area doesn't match. What shall we do? */

	if ( mode == FOLLOWING && auto_bump )
	{
		clientf( C_R " (" C_W "doesn't match!" C_R ")" C_0 );
		auto_bump = -1;
		void do_map_create( char *arg );
		do_map_create("");
		send_to_server("survey\r\n");
	}

	if ( mode != CREATING || !current_room )
		return;

	/* Check if the user wants to update an existing one. */

	if ( update_area_from_survey || !strcmp( current_room->area->name, "New area" ) )
	{
		update_area_from_survey = 0;

		for ( area = areas; area; area = area->next )
			if ( !strcmp( area->name, name ) )
			{
				clientf( C_R " (" C_W "warning: duplicate!" C_R ")" C_0 );
				break;
			}

		area = current_room->area;
		if ( area->name )
			free( area->name );
		area->name = strdup( name );
		clientf( C_R " (" C_G "updated" C_R ")" C_0 );

		return;
	}

	/* First, check if the area exists. If it does, switch it. */

	for ( area = areas; area; area = area->next )
		if ( !strcmp( area->name, name ) )
		{
			unlink_from_area( current_room );
			link_to_area( current_room, area );
			current_room->area = area;
			current_area = area;
			clientf( C_R " (" C_G "switched!" C_R ")" C_0 );

			return;
		}

	/* If not, create one. */

	area = create_area( );
	area->name = strdup( name );
	unlink_from_area( current_room );
	link_to_area( current_room, area );
	current_room->area = area;
	current_area = area;
	clientf( C_R " (" C_G "new area created" C_R ")" C_0 );
}



void parse_survey( char *line )
{
	char tline[256];
	DEBUG( "parse_survey" );
	if ( strlen(line) > 70 )
		return;
	if ( current_room )
	{
		if ( ( !strncmp( line, "You discern that you are standing in ", 37 ) &&
					( line += 37 ) ) ||
				( !strncmp( line, "You stand within ", 17 ) &&
				  ( line += 17 ) ) ||
				( !strncmp( line, "You are standing in ", 20 ) &&
				  ( line += 20 ) ) ||
				( !strncmp( line, "\33[0;37m\33[1;30mYou discern that you are standing in ", 51 ) &&
				  ( line += 51 ) ) ||
				( !strncmp( line, "\33[0;37m\33[1;30mYou stand within ", 31 ) &&
				  ( line += 31 ) ) ||
				( !strncmp( line, "\33[0;37m\33[1;30mYou are standing in ", 34 ) &&
				  ( line += 34 ) )
		   )
		{
			norulerc = 1;
			if  ( mode != CREATING && !auto_bump )
				return;
			/* avoid capturing perceive for tent/dens/rings */
			if ( strstr(line, "the tent of") || strstr(line, "the den of") || strstr(line, "the influence of the ring" ) )
				return;
			/* Might not have a "the" in there, rare cases though. */
			if ( !strncmp( line, "the ", 4 ) )
				line += 4;


			check_area( line );
		}
		else if ( ( !strncmp( line, "Your environment conforms to that of ", 37 ) && ( line += 37 ) ) ||
				( !strncmp( line, "\33[0;37m\33[1;30mYour environment conforms to that of ", 51 ) && ( line += 51 ) ) )
		{
			norulerc = 1;
			if  ( mode != CREATING && !auto_bump )
				return;
			ROOM_TYPE *type;


			/*   line = line + 37; */

			for ( type = room_types; type; type = type->next )
				if ( !strncmp( line, type->name, strlen( line )-1 ) )
					break;

			/* Lusternia - lowercase letters. */
			if ( !type )
			{
				char buf[256];

				for ( type = room_types; type; type = type->next )
				{
					strcpy( buf, type->name );

					buf[0] -= ( 'A' - 'a' );

					if ( !strncmp( line, buf, strlen( buf ) ) )
						break;
				}
			}

			if ( type != current_room->room_type )
			{
				if ( mode == CREATING || auto_bump )
				{
					current_room->room_type = type;

					if ( type ) {
						clientf( C_R " (" C_G "updated" C_R ")" C_0 );
						if ( auto_bump == -1 ) {
							auto_bump = 1;
							void do_map_follow( char *arg );
							do_map_follow("");
							void do_map_bump( char *arg );
							do_map_bump("continue");}
					}
					else
					{
						ROOM_TYPE *add_room_type( char *name, char *color, int c_in, int c_out, int m_swim, int underw );
						char *get_color( char *name );
						memset( tline, '\0', sizeof(tline) );
						strncpy(tline, line, strlen(line)-1);
						add_room_type( tline, get_color( "red" ), 1, 1, 0, 0 );
						clientff( C_R " (" C_Y "'%s' type added" C_R ")" C_0, tline );
						for ( type = room_types; type; type = type->next )
							if ( !strncmp( tline, type->name, strlen( type->name ) ) )
								current_room->room_type = type;
						if ( auto_bump == -1 ) {
							auto_bump = 1;
							void do_map_follow( char *arg );
							do_map_follow("");
							void do_map_bump( char *arg );
							do_map_bump("continue");}

					}
				}
				else if ( auto_bump )
				{
					clientf( C_R " (" C_W "doesn't match!" C_R ")" C_0 );
					auto_bump = -1;
					void do_map_create( char *arg );
					do_map_create("");
					send_to_server("survey\r\n");
				}
			}
		}
		else if ( ( !strcmp( line, "You cannot glean any information about your surroundings." ) ) ||
				( !strcmp( line, "\33[0;37m\33[1;30mYou cannot glean any information about your surroundings." ) ) )
		{
			norulerc = 1;
			if  ( mode != CREATING && !auto_bump )
				return;
			ROOM_TYPE *type;

			for ( type = room_types; type; type = type->next )
			{
				if ( !strcmp( "Undefined", type->name ) )
				{
					if ( type != current_room->room_type )
					{
						if ( mode == CREATING )
						{
							current_room->room_type = type;

							clientf( C_R " (" C_G "updated" C_R ")" C_0 );
						}
						else if ( auto_bump )
						{
							clientf( C_R " (" C_W "doesn't match!" C_R ")" C_0 );
							auto_bump = 0;
						}
					}

					break;
				}
			}
		}
	}
}


void parse_owner( char *line )
{
	if ( cmp("You stand within the tent of *", line)
			&& cmp("You stand within the den of *", line)
			&& cmp("This building belongs to the *", line)
			&& cmp("This room belongs to *",line) )
		return;

	if ( !current_room )
		return;

	ROOM_DATA *room;
	char name[256];
	if ( ( mode == FOLLOWING || mode == CREATING ) && ( !strncmp(line, "You stand within the tent ", 26 ) || !strncmp(line, "You stand within the den ", 25 ) ) ) {
		if ( !cmp("You stand within the den of *", line) )
		{line += 28;}
		else {
			line += 29;}
		strncpy(name, line,strlen(line)-1);
		for ( room = world; room; room = room->next_in_world )  {
			{ if ( !strncmp(current_room->name, room->name, strlen(current_room->name)) && room->vnum != current_room->vnum && room->owner && !strncmp(room->owner,name,strlen(name) ) ) {
																																															 clientff("\r\n"C_C"Duplicate Tent/Den Found in "C_R"("C_g"%s"C_R")"C_C" vnum "C_D"("C_G"%d"C_D")\r\n"C_0,room->area->name, room->vnum );
																																															 return;}
			}
		}
	}

	if ( !strncmp(line, "You stand within the tent ", 26 ) )
		line += 29;
	if ( !strncmp(line, "You stand within the den ", 25 ) )
		line += 28;
	if ( !strncmp(line, "This building ", 14 ) )
		line += 29;
	if ( !strncmp(line, "This room belongs ", 18 ) )
		line += 21;

	if ( !strncmp(line, "City of ", 8) )
		line += 8;
	memset( name, '\0', sizeof(name) );
	strncpy(name, line,strlen(line)-1);

	room = current_room;

	if ( room->owner && strncmp(room->owner,name,strlen(name)) )
		clientff(C_Y" ("C_R"Updated"C_Y")"C_0);
	else if ( !room->owner )
		clientff(C_Y" ("C_R"Set"C_Y")"C_0);

	room->owner = malloc(strlen(name) + 1); strcpy(room->owner, name);

}

void parse_ruler( char *line )
{
	if ( !current_room )
		return;
	if (cmp("* flag of ^ flies over this location.",line))
		return;

	char *fline;
	char ruler[64];
	norulerc = 0;
	if ( ( fline = strstr(line,"flag of ") ) ) {
		fline += 8;
		get_string( fline, ruler, 64);
		char *ruler_color(char *arg);
		if ( current_room->ruler && strcmp(current_room->ruler,ruler) ) {
			clientff(C_W" (%sUpdated"C_W")"C_0,ruler_color(ruler));
			current_room->ruler = malloc(strlen(ruler) + 1); strcpy(current_room->ruler, ruler);
		}
		else if ( !current_room->ruler ) {
			clientff(C_W" (%sSet"C_W")"C_0,ruler_color(ruler));
			current_room->ruler = malloc(strlen(ruler)+1); strcpy(current_room->ruler, ruler);}
	}
	else
		clientfr("ruler error");
}

void parse_nowingarea(char *line)
{
    if (strcmp("Your powers cannot carry you over continents or planes.",line))
    return;

    if ( !current_room->area->nowingarea ) {
    current_room->area->nowingarea = 1;
    clientfr("Area added to NoWings List");
    }
}

int can_dash( ROOM_DATA *room )
{
	int dir;

	if ( !dash_command )
		return 0;

	if ( walk_next )
		return 0;

	dir = room->pf_direction;

	/* First of all, check if we have more than one room. */
	if ( dir <= 0 || !room->exits[dir] )
		return 0;

	/* Now check if we have a clear way, till the end. */
	while( room )
	{
		if ( room->pf_direction != dir && room->exits[dir] )
			return 0;
		if ( room->exits[dir] && room->exits[dir]->room_type->must_swim && !room->exits[dir]->underwater )
			return 0;
		if ( room->room_type->must_swim && !room->underwater )
			return 0;

		room = room->exits[dir];
	}

	return 1;
}



void go_next( )
{
	EXIT_DATA *spexit;
	char buf[256];
	char *exitparse;
	char *p;
	int dashwait = 0;

	if ( moved ) moved = 0;
	if ( !current_room )
	{
		clientf( C_R "[No current_room.]" C_0 );
		auto_walk = 0;
	}
	else if ( !current_room->pf_parent && !strcmp(current_room->area->name,"The Havens."))
	{
		sprintf( buf, "Exit Haven" );
		clientfr( buf );
		sprintf( buf, "Exit Haven\r\n" );
		send_to_server( buf );
		auto_walk = 1;
	}
	else if ( !current_room->pf_parent )
	{
		auto_walk = 0;
		dash_command = NULL;
		except = NULL;
		clientff( C_R "(" C_G "Done." C_R ") " C_0 );
		troopmove = 0;
		guardmove = 0;
		if ( area_search ) {
			if ( current_room == search_room ) {
				send_to_server( "info here\r\n" );
			} else {
				area_search=0;
				clientff(C_R"\r\n[Unable to find path to "C_y"%s "C_G"%d"C_R"]\r\n"C_0,search_room->name,search_room->vnum);
			}
		}
	}
	else if ( troopmove == 1 && troopn != NULL )
	{
		if ( dir_name[current_room->pf_direction] == NULL ) {
			auto_walk = 0;
			justwarped = 0;
			dash_command = NULL;
			except = NULL;
			troopmove = 0;
			clientf(C_R"\r\nTroops cannot get to that location, special exit.\r\n"C_0);
			return;
		} else {
			if ( !gag_next_prompt )
				clientff( C_D "(order %s march %s) " C_0, troopn ,dir_name[current_room->pf_direction] );
			sprintf(buf, "order %s march %s\r\n", troopn, dir_name[current_room->pf_direction] );
			send_to_server( buf );
			troopmove = 2;
			auto_walk = 2;
		}
	}
	else if ( guardmove == 1 && guardnum != NULL )
	{
		if ( dir_name[current_room->pf_direction] == NULL ) {
			auto_walk = 0;
			justwarped = 0;
			dash_command = NULL;
			except = NULL;
			guardmove = 0;
			clientf(C_R"\r\nGuards cannot get to that location, special exit.\r\n"C_0);
			return;
		} else {
			if ( !gag_next_prompt )
				clientff( C_D "(order %s move %s) " C_0, guardnum ,dir_name[current_room->pf_direction] );
			sprintf(buf, "order %s move %s\r\n", guardnum, dir_name[current_room->pf_direction] );
			send_to_server( buf );
			guardmove = 2;
			auto_walk = 2;}
	}
	else if ( artimsg || cmp_room_wing() ) {
	    artimsg = wingcmd;
		sprintf( buf, "say %s", artimsg );
		clientfr( buf );
		sprintf( buf, "say %s\r\n", artimsg );
		send_to_server( buf );
		auto_walk = 1;
		mode = GET_UNLOST;
		artimsg = NULL;
	}
	else if ( swim_next && mounted ) {
		swim_next = 0;
		clientfr("You need to dismount to swim");
		auto_walk = 0;
		dash_command = NULL;
		except = NULL;
		troopmove = 0;
		guardmove = 0;
	}
	else
	{
		if ( ( current_room->pf_direction != -1 ) && ( current_room->pf_direction != -2 ) )
		{
			if ( door_closed ) {
				sprintf( buf, "open %s", dir_name[current_room->pf_direction] );
				clientfr( buf );
				sprintf( buf, "open door %s\r\n", dir_small_name[current_room->pf_direction] );
				send_to_server( buf );
			} else if ( door_locked ) {
				if ( !disable_auto_unlock ) {
					sprintf( buf, "unlock door %s", dir_name[current_room->pf_direction] );
					clientfr( buf );
					sprintf( buf, "unlock door %s\r\n", dir_small_name[current_room->pf_direction] );
					send_to_server( buf );
					return;
				}
			} else {
				if ( wateroption == 3 && !pear_defence && !current_room->room_type->underwater
						&& current_room->room_type->must_swim && !current_room->underwater &&
						(current_room->pf_parent->room_type->underwater || current_room->pf_parent->underwater) ) {
					clientf( C_W "(" C_G "belch"C_W"/"C_G"morph swordfish" C_W ") " C_0 );
					send_to_server("belch\r\nmorph swordfish\r\n");
					if (!( dash_command && !strcmp(dash_command, "gallop " ) ))
						walk_next = 1;
				}
				if ( ( must_swim( current_room, current_room->pf_parent ) ) && ( !walk_next ) && ( !burrowed ) ) {
					send_to_server( "swim " );
					if ( !gag_next_prompt )
						clientff( C_R "(swim %s) " C_0, dir_name[current_room->pf_direction] );
				}

				if ( swim_next && !mounted ) {
					swim_next = 0;
					send_to_server( "swim " );
					if ( !gag_next_prompt )
						clientff( C_R "(swim %s) " C_0, dir_name[current_room->pf_direction] );
				}


				if ( burrowed ) {
					if ( !burrowtype )
						send_to_server( "burrow " );
					else if ( burrowtype == 1 )
						send_to_server( "sand sink " );
					if ( !gag_next_prompt ) {
						if ( !burrowtype )
							clientff( C_R "(burrow %s) " C_0, dir_name[current_room->pf_direction] );
						else if ( burrowtype == 1 )
							clientff( C_R "(sand sink %s) " C_0, dir_name[current_room->pf_direction] );
					}
				}

				if ( walk_next ) {
					if ( dash_command )
						dashwait = 1;
					walk_next = 0;
					moved = 1;
					clientff( C_R "(%s) " C_0, dir_name[current_room->pf_direction] );
				}


				if ( !must_swim( current_room, current_room->pf_parent ) &&
						dash_command && (dashwait || can_dash( current_room )) )
				{
					if ( dashwait )
					{
						dashwait=0;
						add_queue_top( current_room->pf_direction );
					} else {
						send_to_server( dash_command );
						if ( !gag_next_prompt )
							clientff( C_R "(%s%s) " C_0, dash_command, dir_name[current_room->pf_direction] );
					}
				} else {
					if ( mode == FOLLOWING || mode == CREATING )
					{
						add_queue( current_room->pf_direction );
						if ( ( !gag_next_prompt ) && ( !swim_next ) && ( !burrowed )
								&& ( ( !must_swim( current_room, current_room->pf_parent ) ) ) )
							if ( !moved )
								clientff( C_R "(%s) " C_0, dir_name[current_room->pf_direction] );
					}
				}
				lastdir = current_room->pf_direction;
				send_to_server( dir_small_name[current_room->pf_direction] );
				send_to_server( "\r\n" );
				if ( door_unlocked ) {
					clientff( C_R "(Relock %s) " C_0, dir_rev_name[current_room->pf_direction]);
					sprintf(buf, "close door %s\r\nlock door %s\r\n", dir_rev_name[current_room->pf_direction],
							dir_rev_name[current_room->pf_direction] );
					send_to_server( buf );
					door_unlocked = 0;
					door_opened = 0;
				} else if ( door_opened ) {
					clientff( C_R "(Closing %s) " C_0, dir_rev_name[current_room->pf_direction]);
					sprintf(buf, "close door %s\r\n", dir_rev_name[current_room->pf_direction]);
					send_to_server( buf );
					door_opened = 0;
				}
			}
		}
		else if ( current_room->pf_direction == -1 ) {
			if ( !nointeract ) {
				for ( spexit = current_room->special_exits; spexit; spexit = spexit->next )
				{
					if ( spexit->to == current_room->pf_parent &&
							spexit->command )
					{
						clientff( C_R "(" C_D "%s" C_R ") " C_0, spexit->command );
						exitparse = spexit->command;
						for ( p = exitparse; *p; p++ )
							if ( *p == '$' )
								*p = '\n';
						send_to_server( exitparse );
						send_to_server( "\r\n" );
						for ( p = exitparse; *p; p++ )
							if ( *p == '\n' )
								*p = '$';
						if ( spexit->nomsg )
						{
							current_room = spexit->to;
							current_area = current_room->area;
							if ( !spexit->nolook )
							{add_queue_top( -1 );}
							else
							{if ( auto_walk )
								auto_walk = 2;}
						}
						break;
					}
				}
			} else {
				clientfr("You are in black wind and cannot use special exits");
				auto_walk = 0;
				justwarped = 0;
				dash_command = NULL;
				except = NULL;}
		} else if ( current_room->pf_direction == -2 ) {
			clientff( C_R "(" C_D "Worm Warp" C_R ") " C_0 );
			send_to_server( "worm warp" );
			send_to_server( "\r\n" );
			justwarped=1;
		}
		auto_walk = 1;
	}
}



void add_exit_char( char *var, int dir )
{
	const char dir_chars[] =
	{ ' ', '|', '/', '-', '\\', '|', '/', '-', '\\' };

	if ( ( *var == '|' && dir_chars[dir] == '-' ) ||
			( *var == '-' && dir_chars[dir] == '|' ) )
		*var = '+';
	else if ( ( *var == '/' && dir_chars[dir] == '\\' ) ||
			( *var == '\\' && dir_chars[dir] == '/' ) )
		*var = 'X';
	else
		*var = dir_chars[dir];
}



void fill_map( ROOM_DATA *room, AREA_DATA *area, int x, int y )
{
	static int xi[] = { 2, 0, 1, 1, 1, 0, -1, -1, -1, 2, 2, 2, 2, 2 };
	static int yi[] = { 2, -1, -1, 0, 1, 1, 1, 0, -1, 2, 2, 2, 2, 2 };
	int i;

	if ( room->area != area )
		return;

	if ( room->mapped )
		return;

	if ( x < 0 || x >= MAP_X+1 || y < 0 || y >= MAP_Y+1 )
		return;

	room->mapped = 1;
	/* We'll have to clean all these room->mapped too. */
	room->area->needs_cleaning = 1;

	if ( map[x][y] )
		return;

	map[x][y] = room;

	for ( i = 1; dir_name[i]; i++ )
	{
		if ( !room->exits[i] )
			continue;

		if ( room->exit_stops_mapping[i] )
			continue;

		if ( room->exit_length[i] > 0 && !room->use_exit_instead[i] )
		{
			int j;
			int real_x, real_y;

			for( j = 1; j <= room->exit_length[i]; j++ )
			{
				real_x = x + ( xi[i] * j );
				real_y = y + ( yi[i] * j );

				if ( real_x < 0 || real_x >= MAP_X+1 ||
						real_y < 0 || real_y >= MAP_Y+1 )
					continue;

				add_exit_char( &extra_dir_map[real_x][real_y], i );
			}
		}

		if ( room->use_exit_instead[i] )
		{
			int ex = room->use_exit_instead[i];
			fill_map( room->exits[i], area,
					x + ( xi[ex] * ( 1 + room->exit_length[i] ) ),
					y + ( yi[ex] * ( 1 + room->exit_length[i] ) ) );
		}
		else if ( xi[i] != 2 )
			fill_map( room->exits[i], area,
					x + ( xi[i] * ( 1 + room->exit_length[i] ) ),
					y + ( yi[i] * ( 1 + room->exit_length[i] ) ) );
	}
}




/* One little nice thing. */
int has_exit( ROOM_DATA *room, int direction )
{
	if ( room )
	{
		if ( room->exits[direction] )
		{
			if ( room->exit_stops_mapping[direction] )
				return 4;
			else if ( room->area != room->exits[direction]->area )
				return 3;
			else if ( room->pf_highlight && room->pf_direction == direction )
				return 5;
			else
				return 1;
		}
		else if ( room->detected_exits[direction] )
			return 2;
	}

	return 0;
}


/*
 *
 * [ ]- [ ]- [ ]
 *  | \  | /
 *
 * [ ]- [*]- [ ]
 *  | /  | \
 *
 * [ ]  [ ]  [ ]
 *
 */

void set_exit( short *ex, ROOM_DATA *room, int dir )
{
	/* Careful. These are |=, not != */

	if ( room->exits[dir] )
	{
		*ex |= EXIT_NORMAL;

		if ( room->exits[dir]->area != room->area )
			*ex |= EXIT_OTHER_AREA;
	}
	else if ( room->detected_exits[dir] && !room->hasrandomexits )
		*ex |= EXIT_UNLINKED;

	if ( room->detected_exits[dir] && room->hasrandomexits && !room->trueexit[dir] )
		*ex |= EXIT_PATH;

	if ( room->locked_exits[dir] )
		*ex |= EXIT_LOCKED;

	if ( room->exit_stops_mapping[dir] )
		*ex |= EXIT_STOPPING;

	if ( room->pf_highlight && room->pf_parent == room->exits[dir] )
		*ex |= EXIT_PATH;
}



void set_all_exits( ROOM_DATA *room, int x, int y )
{
	/* East, southeast, south. (current element) */
	set_exit( &map_new[x][y].e, room, EX_E );
	set_exit( &map_new[x][y].se, room, EX_SE );
	set_exit( &map_new[x][y].s, room, EX_S );

	/* Northwest. (northwest element) */
	if ( x && y )
	{
		set_exit( &map_new[x-1][y-1].se, room, EX_NW );
	}

	/* North, northeast. (north element) */
	if ( y )
	{
		set_exit( &map_new[x][y-1].s, room, EX_N );
		set_exit( &map_new[x][y-1].se_rev, room, EX_NE );
	}

	/* West, southwest. (west element) */
	if ( x )
	{
		set_exit( &map_new[x-1][y].e, room, EX_W );
		set_exit( &map_new[x-1][y].se_rev, room, EX_SW );
	}

	/* In, out. */
	map_new[x][y].in_out |= ( room->exits[EX_IN] || room->exits[EX_OUT] ) ? 1 : 0;
	map_new[x][y].up |= ( room->exits[EX_U] ) ? 1 : 0;
	map_new[x][y].down |= ( room->exits[EX_D] ) ? 1 : 0;
}



/* Remake of fill_map. */
void fill_map_new( ROOM_DATA *room, int x, int y )
{
	const int xi[] = { 2, 0, 1, 1, 1, 0, -1, -1, -1, 2, 2, 2, 2, 2 };
	const int yi[] = { 2, -1, -1, 0, 1, 1, 1, 0, -1, 2, 2, 2, 2, 2 };
	int i;

	if ( x < 0 || x >= map_x || y < 0 || y >= map_y )
		return;

	if ( map_new[x][y].room || room->mapped )
		return;

	room->mapped = 1;
	room->area->needs_cleaning = 1;
	map_new[x][y].room = room;
	map_new[x][y].color = room->room_type->color;
	char *ruler_color( char *arg );
	map_new[x][y].rcolor = ruler_color( room->ruler );


	set_all_exits( room, x, y );

	for ( i = 1; dir_name[i]; i++ )
	{
		if ( !room->exits[i] || room->exit_stops_mapping[i] ||
				( ( room->exits[i]->area != room->area ) && !room->exit_joins_areas[i] ) )
			continue;

		/* Normal exit. */
		if ( !room->use_exit_instead[i] && !room->exit_length[i] )
		{
			if ( xi[i] != 2 && yi[i] != 2 )
				fill_map_new( room->exits[i], x + xi[i], y + yi[i] );
			continue;
		}

		/* Exit changed. */
		{
			int new_i, new_x, new_y;

			new_i = room->use_exit_instead[i] ? room->use_exit_instead[i] : i;

			if ( xi[new_i] != 2 && yi[new_i] != 2 )
			{
				int j;

				new_x = x + xi[new_i] * ( 1 + room->exit_length[i] );
				new_y = y + yi[new_i] * ( 1 + room->exit_length[i] );

				fill_map_new( room->exits[i], new_x, new_y );

				for( j = 1; j <= room->exit_length[i]; j++ )
				{
					new_x = x + ( xi[new_i] * j );
					new_y = y + ( yi[new_i] * j );

					if ( new_x < 0 || new_x >= map_x ||
							new_y < 0 || new_y >= map_y )
						break;

					/*
					 * This exit will be displayed instead of the center
					 * of a room.
					 */

					if ( i == EX_N || i == EX_S )
						map_new[new_x][new_y].extra_s = 1;

					else if ( i == EX_E || i == EX_W )
						map_new[new_x][new_y].extra_e = 1;

					else if ( i == EX_SE || i == EX_NW )
						map_new[new_x][new_y].extra_se = 1;

					else if ( i == EX_NE || i == EX_SW )
						map_new[new_x][new_y].extra_se_rev = 1;
				}
			}
		}
	}
}

char *ruler_color( char *arg )
{
	if ( arg == NULL )
		return C_0;
	int i;
	char ruler[64];
	get_string(arg,ruler,64);
	*ruler = toupper(ruler[0]);
	for ( i = 0; cities[i].name; i++ )
		if ( !strncmp(ruler, cities[i].name,strlen(cities[i].name) ) )
			return cities[i].color;

	return C_0;
}
/* Total remake of show_map. */
/* All calls to strcat/sprintf have been replaced with byte by byte
 * processing. The result has been a dramatical increase in speed. */

void show_map_new( ROOM_DATA *room )
{

	AREA_DATA *a;
	ROOM_DATA *r;
	char map_buf[65536], buf[64], *p, *s, *s2;
	char vnum_buf[1024];
	int x, y, len, len2, loop, i;
	int use_mxp = 0;
	MAP_ELEMENT *big_map;
	big_map = calloc(map_x*map_y, sizeof(MAP_ELEMENT));
	map_new = calloc(map_x, sizeof(void*));
	for ( i = 0; i < map_x; i++ )
		map_new[i] = &big_map[i*map_y];

	DEBUG( "show_map_new" );

	if ( !room )
		return;

	get_timer( );
	//   debugf( "--map new--" );

	for ( x = 0; x < map_x; x++ )
		for ( y = 0; y < map_y; y++ )
			memset( &map_new[x][y], 0, sizeof( MAP_ELEMENT ) );

	/* Pathfinder - Go around and set which rooms to highlight. */
	for ( r = room->area->rooms; r; r = r->next_in_area )
		r->pf_highlight = 0;

	if ( room->pf_parent )
	{
		for ( r = room, loop = 0; r->pf_parent && loop < 100; r = r->pf_parent, loop++ )
			r->pf_highlight = 1;
	}

	//   debugf( "1: %d", get_timer( ) );

	/* From the current *room, place all other rooms on the map. */
	fill_map_new( room, map_x / 2, map_y / 2 );

	//   debugf( "2: %d", get_timer( ) );


	/* Build it up. */


	/* Are we able to get a SECURE mode on MXP? */
	if ( !disable_mxp_map && mxp_tag( TAG_LOCK_SECURE ) )
		use_mxp = 1;

	/* Upper banner. */

	map_buf[0] = 0;
	p = map_buf;

	sprintf( vnum_buf, "v%d", room->vnum );

	s = C_0 "/--" C_C;
	while( *s )
		*(p++) = *(s++);
	s = room->area->name, len = 0;
	while( *s )
		*(p++) = *(s++), len++;
	s = C_0;
	while( *s )
		*(p++) = *(s++);
	len2 = map_x * 4 - 7 - strlen( vnum_buf );
	for ( x = len; x < len2; x++ )
		*(p++) = '-';
	s = C_C;
	while( *s )
		*(p++) = *(s++);
	s = vnum_buf;
	while( *s )
		*(p++) = *(s++);
	s = C_0 "--\\\r\n";
	while( *s )
		*(p++) = *(s++);
	*p = 0;

	/* The map, row by row. */
	for ( y = 0; y < map_y; y++ )
	{
		/* (1) [o]- */
		/* (2)  | \ */

		/* (1) */

		if ( y )
		{
			*(p++) = ' ';

			for ( x = 0; x < map_x; x++ )
			{
				if ( x )
				{
					if ( map_new[x][y].room )
					{
						if ( map_new[x][y].room->tmark )
							s = C_D;
						else if ( !disable_mapterritory )
							s = map_new[x][y].rcolor;
						else if ( !map_new[x][y].room->underwater )
							s = map_new[x][y].color;
						else
							s = C_C;

						while( *s )
							*(p++) = *(s++);
						*(p++) = '[';

						if ( use_mxp )
						{
							if ( map_new[x][y].room->person_here )
								sprintf( vnum_buf, "<mppers v=%d r=\"%s\" t=\"%s\" p=\"%s\">",
										map_new[x][y].room->vnum, map_new[x][y].room->name,
										map_new[x][y].room->room_type->name, map_new[x][y].room->person_here );
							else
								sprintf( vnum_buf, "<mpelm v=%d r=\"%s\" t=\"%s\">",
										map_new[x][y].room->vnum, map_new[x][y].room->name,
										map_new[x][y].room->room_type->name );
							s = vnum_buf;
							while( *s )
								*(p++) = *(s++);
						}

						if ( map_new[x][y].room == current_room )
							s = C_B "*";
						else if ( map_new[x][y].room == room )
							s = C_R "*";
						else if ( map_new[x][y].room->person_here )
							s = C_Y "*";
						else if ( map_new[x][y].room->tmark )
							s = C_D "X";
						else if ( map_new[x][y].room->shrine )
						{
							if ( map_new[x][y].room->shrinemajor ) {
								if ( map_new[x][y].room->shrine == 1 )
									s = C_W "#";
								if ( map_new[x][y].room->shrine == 2 )
									s = C_G "#";
								if ( map_new[x][y].room->shrine == 3 )
									s = C_R "#";} else {
										if ( map_new[x][y].room->shrine == 1 )
											s = C_W "¥";
										if ( map_new[x][y].room->shrine == 2 )
											s = C_G "¥";
										if ( map_new[x][y].room->shrine == 3 )
											s = C_R "¥";
									}
						}
						else if ( map_new[x][y].room->shrineeff && !disable_shrineradius )
						{
							if ( map_new[x][y].room->shrineeff == 1 )
								s = C_W "¤";
							if ( map_new[x][y].room->shrineeff == 2 )
								s = C_G "¤";
							if ( map_new[x][y].room->shrineeff == 3 )
								s = C_R "¤";
						}
						else if ( map_new[x][y].room->special_exits )
							s = "\33[1;35m+";
						else if ( map_new[x][y].room->tags )
						{ sprintf( vnum_buf, C_W "%1.1s", (char *) map_new[x][y].room->tags->p );
							s = vnum_buf;
						}
						else if ( ( map_new[x][y].in_out ) && ( map_new[x][y].up || map_new[x][y].down ) )
							s = C_C "ö";
						else if ( map_new[x][y].room->shop )
							s = C_Y "$";
						else if ( map_new[x][y].up && map_new[x][y].down )
							s = "\33[1;35m¦";
						else if ( map_new[x][y].up )
							s = "\33[1;35m^";
						else if ( map_new[x][y].down )
							s = "\33[1;35m_";
						else if ( map_new[x][y].in_out )
							s = C_r "o";
						else if ( use_mxp )
							s = C_d " ";
						else
							s = " ";
						while( *s )
							*(p++) = *(s++);

						if ( use_mxp )
						{
							if ( map_new[x][y].room->person_here )
								s = "</mppers>";
							else
								s = "</mpelm>";
							while( *s )
								*(p++) = *(s++);
						}

						if ( map_new[x][y].room->tmark )
							s = C_D;
						else if ( !disable_mapterritory )
							s = map_new[x][y].rcolor;
						else if ( !map_new[x][y].room->underwater )
							s = map_new[x][y].color;
						else
							s = C_C;

						while( *s )
							*(p++) = *(s++);
						s = "]" C_0;
						while( *s )
							*(p++) = *(s++);
					}
					else
					{
						*(p++) = ' ';

						if ( map_new[x][y].extra_e && map_new[x][y].extra_s )
							*(p++) = '+';
						else if ( map_new[x][y].extra_se && map_new[x][y].extra_se_rev )
							*(p++) = 'X';
						else if ( map_new[x][y].extra_e )
							*(p++) = '-';
						else if ( map_new[x][y].extra_s )
							*(p++) = '|';
						else if ( map_new[x][y].extra_se )
							*(p++) = '\\';
						else if ( map_new[x][y].extra_se_rev )
							*(p++) = '/';
						else
							*(p++) = ' ';

						*(p++) = ' ';
					}
				}

				/* Exit color. */
				if ( map_new[x][y].e & EXIT_OTHER_AREA )
					s = C_W, s2 = C_0;
				else if ( map_new[x][y].e & EXIT_STOPPING )
					s = C_R, s2 = C_0;
				else if ( map_new[x][y].e & EXIT_LOCKED )
					s = C_r, s2 = C_0;
				else if ( map_new[x][y].e & EXIT_UNLINKED )
					s = C_D, s2 = C_0;
				else if ( map_new[x][y].e & EXIT_PATH )
					s = C_B, s2 = C_0;
				else
					s = "", s2 = "";
				while( *s )
					*(p++) = *(s++);

				*(p++) = map_new[x][y].e ? '-' : ' ';

				while( *s2 )
					*(p++) = *(s2++);
			}

			*(p++) = '\r';
			*(p++) = '\n';
		}

		/* (2) */

		for ( x = 0; x < map_x; x++ )
		{
			*(p++) = ' ';
			if ( x )
			{
				/* Exit color. */
				if ( map_new[x][y].s & EXIT_OTHER_AREA )
					s = C_W, s2 = C_0;
				else if ( map_new[x][y].s & EXIT_STOPPING )
					s = C_R, s2 = C_0;
				else if ( map_new[x][y].s & EXIT_LOCKED )
					s = C_r, s2 = C_0;
				else if ( map_new[x][y].s & EXIT_UNLINKED )
					s = C_D, s2 = C_0;
				else if ( map_new[x][y].s & EXIT_PATH )
					s = C_B, s2 = C_0;
				else
					s = "", s2 = "";
				while( *s )
					*(p++) = *(s++);

				*(p++) = map_new[x][y].s ? '|' : ' ';

				while( *s2 )
					*(p++) = *(s2++);

				*(p++) = ' ';
			}

			/* Exit color. */
			if ( ( map_new[x][y].se | map_new[x][y].se_rev ) & EXIT_OTHER_AREA )
				s = C_W, s2 = C_0;
			else if ( ( map_new[x][y].se | map_new[x][y].se_rev ) & EXIT_STOPPING )
				s = C_R, s2 = C_0;
			else if ( ( map_new[x][y].se | map_new[x][y].se_rev ) & EXIT_LOCKED )
				s = C_r, s2 = C_0;
			else if ( ( map_new[x][y].se | map_new[x][y].se_rev ) & EXIT_UNLINKED )
				s = C_D, s2 = C_0;
			else if ( ( map_new[x][y].se | map_new[x][y].se_rev ) & EXIT_PATH )
				s = C_B, s2 = C_0;
			else
				s = "", s2 = "";
			while( *s )
				*(p++) = *(s++);

			*(p++) = ( map_new[x][y].se ?
					( map_new[x][y].se_rev ? 'X' : '\\' ) :
					map_new[x][y].se_rev ? '/' : ' ' );

			while( *s2 )
				*(p++) = *(s2++);
		}

		*(p++) = '\r';
		*(p++) = '\n';
	}

	/* Lower banner. */
	s = "\\--";
	while( *s )
		*(p++) = *(s++);
	sprintf( buf, "Time: %d usec", get_timer( ) );
	s = buf, len = 0;
	while( *s )
		*(p++) = *(s++), len++;
	for ( x = len; x < map_x * 4 - 5 - skip_newline_on_map*9; x++ )
		*(p++) = '-';

	if ( !skip_newline_on_map )
		s = "/\r\n";
	else
		s = C_C "<send>go</send>" C_0 "/" C_C "<send>stop</send>" C_0 "--/";
	while( *s )
		*(p++) = *(s++);
	*p = 0;

	/* Clear up our mess. */
	for ( a = areas; a; a = a->next )
		if ( a->needs_cleaning )
		{
			for ( r = a->rooms; r; r = r->next_in_area )
				r->mapped = 0;
			a->needs_cleaning = 0;
		}

	//   debugf( "3: %d", get_timer( ) );

	/* Show it away. */
	clientf( map_buf );

	/* And return MXP to default. */
	if ( use_mxp )
		mxp_tag( TAG_DEFAULT );
}



/* This will get the room that would be shown on the map somewhere. */
ROOM_DATA *get_room_at( int dir, int length )
{
	const int xi[] = { 2, 0, 1, 1, 1, 0, -1, -1, -1, 2, 2, 2, 2, 2 };
	const int yi[] = { 2, -1, -1, 0, 1, 1, 1, 0, -1, 2, 2, 2, 2, 2 };
	AREA_DATA *a;
	ROOM_DATA *r;
	int x, y;

	if ( !current_room )
		return NULL;

	for ( x = 0; x < map_x; x++ )
		for ( y = 0; y < map_y; y++ )
			memset( &map_new[x][y], 0, sizeof( MAP_ELEMENT ) );

	/* Convert the angle/length to a relative position. */
	x = xi[dir], y = yi[dir];
	if ( x == 2 || y == 2 )
		return NULL;
	x *= length, y *= length;

	/* Convert them from relative to absolute. */
	x = ( map_x / 2 ) + x;
	y = ( map_y / 2 ) + y;

	if ( x < 0 || y < 0 || x >= map_x || y >= map_y )
		return NULL;

	fill_map_new( current_room, map_x / 2, map_y / 2 );

	/* Clear up our mess. */
	for ( a = areas; a; a = a->next )
		if ( a->needs_cleaning )
		{
			for ( r = a->rooms; r; r = r->next_in_area )
				r->mapped = 0;
			a->needs_cleaning = 0;
		}

	return map_new[x][y].room;
}

void show_floating_map( ROOM_DATA *room )
{
	if ( !floating_map_enabled || !room )
		return;

	if ( !mxp_tag( TAG_LOCK_SECURE ) )
		return;

	map_y = 10;
	map_x = 14;
	mxp( "<DEST IMapper X=0 Y=0>" );
	mxp_tag( TAG_LOCK_SECURE );

	skip_newline_on_map = 1;
	show_map_new( room );
	skip_newline_on_map = 0;

	mxp_tag( TAG_LOCK_SECURE );
	mxp( "</DEST>" );
	mxp_tag( TAG_DEFAULT );
}



char *get_color( char *name )
{
	int i;

	for ( i = 0; color_names[i].name; i++ )
		if ( !strcmp( name, color_names[i].name ) )
			return color_names[i].code;

	return NULL;
}


ROOM_TYPE *add_room_type( char *name, char *color, int c_in, int c_out, int m_swim, int underw )
{
	ROOM_TYPE *type;

	for ( type = room_types; type; type = type->next )
		if ( !strcmp( name, type->name ) )
			break;

	/* New? Create one more. */
	if ( !type )
	{
		ROOM_TYPE *t;

		type = calloc( 1, sizeof( ROOM_TYPE ) );
		type->name = strdup( name );
		type->next = NULL;

		if ( !room_types )
			room_types = type;
		else
		{
			for ( t = room_types; t->next; t = t->next );
			t->next = type;
		}
	}

	if ( !type->color || strcmp( type->color, color ) )
		type->color = color;

	type->cost_in = c_in;
	type->cost_out = c_out;
	type->must_swim = m_swim;
	type->underwater = underw;

	return type;
}

DIVINE_DATA *add_divine( char *name, char *relation)
{
	DIVINE_DATA *divine;

	for ( divine = divinelist; divine; divine = divine->next ) {
		if ( !strcmp( name, divine->name ) ) {
			break;
		}
	}

	/* New? Create one more. */
	if ( !divine ) {
		DIVINE_DATA *d;

		divine = calloc( 1, sizeof( DIVINE_DATA ) );
		divine->name = strdup( name );
		divine->next = NULL;

		if ( !divinelist || !strcmp(divinelist->name,"dummy" ) ) {
			divinelist = divine;
		} else {

			for(d = divinelist; d->next; d = d->next);
			d->next = divine;
		}
	}

	if ( !divine->relation || strcmp( divine->relation, relation ) ) {
		divine->relation = strdup( relation );
	}

	return divine;
}

void rem_divine( char *name)
{
	DIVINE_DATA *current;
	DIVINE_DATA *previous;

	current = divinelist;
	while(current) {
		if ( !strcmp( name, current->name ) ) {
			break;
		}
		previous = current;
		current = current->next;
	}

	if (!current) {
		clientfr("No divine by that name found");
		return;
	}

	if(!previous) {
		divinelist = current->next;
	} else {
		previous->next = current->next;
	}

	free(current);
}

int save_settings( char *file )
{
	ROOM_DATA *r;
	ELEMENT *tag;
	FILE *fl;
	int i;
	/* backup sequence */
	if ( !disable_autobackup ) {
		FILE *bcks,*bckf;
		int bt;
		bcks = fopen( file, "rb");
		bckf = fopen( "config.mapper.bck", "wb" );
		if ( bcks==NULL || bckf==NULL )
		{clientfr("Backup Error");
			if ( bcks==NULL )
				clientfr("No File to Backup");
			if ( bckf==NULL )
				clientfr("Error Creating Backup");}
		else
		{
			while(1)
			{
				if( ( bt=fgetc(bcks) ) && bt!=EOF)
				{
					fputc(bt,bckf);
				}
				else
				{
					clientfr("Settings Backup Successful");
					fclose( bckf );
					fclose( bcks );
					break;
				}
			}
		}
	}
	/* backup ended */
	fl = fopen( file, "w" );

	if ( !fl )
		return 1;

	fprintf( fl, "# File generated by IMapper.\r\n" );
	fprintf( fl, "# Manual changes that are not loaded will be lost on the next rewrite.\r\n\r\n" );

	for ( i = 0; color_names[i].name; i++ )
	{
		if ( !strcmp( color_names[i].title_code, room_color ) )
		{
			fprintf( fl, "Title-Color %s\r\n", color_names[i].name );
			break;
		}
	}
	for ( i = 1; color_names[i].name; i++ ) {
		if ( !strcmp( color_names[i].title_code, exit_color ) )
		{
			fprintf( fl, "Exit-Color %s\r\n", color_names[i].name );
			break;
		}
	}

	fprintf( fl, "Disable-Swimming %s\r\n", disable_swimming ? "yes" : "no" );
	fprintf( fl, "Disable-WhoList %s\r\n", disable_wholist ? "yes" : "no" );
	fprintf( fl, "Disable-Alertness %s\r\n", disable_alertness ? "yes" : "no" );
	fprintf( fl, "Disable-Locating %s\r\n", disable_locating ? "yes" : "no" );
	fprintf( fl, "Disable-AreaName %s\r\n", disable_areaname ? "yes" : "no" );
	fprintf( fl, "Disable-MXPTitle %s\r\n", disable_mxp_title ? "yes" : "no" );
	fprintf( fl, "Disable-MXPExits %s\r\n", disable_mxp_exits ? "yes" : "no" );
	fprintf( fl, "Disable-MXPMap %s\r\n", disable_mxp_map ? "yes" : "no" );
	fprintf( fl, "Disable-AutoLink %s\r\n", disable_autolink ? "yes" : "no" );
	fprintf( fl, "Disable-Artifacts %s\r\n", disable_artifacts ? "yes" : "no" );
	fprintf( fl, "Disable-AdditionalName %s\r\n", disable_additional_name ? "yes" : "no" );
	if ( wateroption == 2 )
		fprintf( fl, "Underwater-Breathing %s\r\n", underitem );
	else
		fprintf( fl, "Underwater-Breathing %d\r\n", wateroption );
	fprintf( fl, "Disable-AutoUnlock %s\r\n", disable_auto_unlock ? "yes" : "no" );
	fprintf( fl, "Disable-WormWarp %s\r\n", disable_worm_warp ? "yes" : "no" );
	fprintf( fl, "Disable-ShrineInfluence %s\r\n", disable_shrineinfluence ? "yes" : "no" );
	fprintf( fl, "Disable-ShrineRadius %s\r\n", disable_shrineradius ? "yes" : "no" );
	fprintf( fl, "Disable-ShrineRadiusCheck %s\r\n", disable_shrineradiuscheck ? "yes" : "no" );
	fprintf( fl, "Disable-FarsightPath %s\r\n", disable_farsightpath ? "yes" : "no" );
	fprintf( fl, "Disable-AutoMap %s\r\n", disable_automap ? "yes" : "no" );
	fprintf( fl, "AutoMap-Size %d\r\n", automapsize );
	fprintf( fl, "Territory-Show %s\r\n", disable_mapterritory ? "RoomType" : "Ruler" );
	fprintf( fl, "Disable-AutoBackup %s\r\n", disable_autobackup ? "yes" : "no" );
	fprintf( fl, "Disable-AutoShrine %s\r\n", disable_autoshrine ? "yes" : "no" );
	fprintf( fl, "Disable-ForceWrap %s\r\n", disable_forcewrap ? "yes" : "no");
	fprintf( fl, "Disable-Vnum %s\r\n", disable_vnum ? "yes" : "no");
	fprintf( fl, "Disable-Pathwrap %s\r\n", disable_pathwrap ? "yes" : "no");

	fprintf( fl, "Mapwing-Command %s\r\n",wingcmd == NULL ? "nothing" : wingcmd );
	fprintf( fl, "Mapwing-Room %d\r\n",wingroom );

	/* Save all room tags. */

	fprintf( fl, "\r\n# Room Tags.\r\n\r\n" );
	for ( r = world; r; r = r->next_in_world )
	{
		if ( r->tags )
		{
			/* Save them in reverse. */

			fprintf( fl, "# %s (%s)\r\n",
					r->name, r->area->name );
			for ( tag = r->tags; tag->next; tag = tag->next );
			for ( ; tag; tag = tag->prev )
				fprintf( fl, "Tag %d \"%s\"\r\n", r->vnum, (char *) tag->p );
		}
	}

	fprintf(fl, "\n\n# Divine\n\n"
			"# \"DivineName\" Relation\n");

	DIVINE_DATA *d;
	for ( d = divinelist; d; d = d->next )
	{
		fprintf(fl,"Div: \"%s\" %s\n", d->name, d->relation );
	}

	fprintf(fl, "\r\n# Shrines.\r\n");
	for ( r = world; r; r = r->next_in_world ) {
		if ( r->shrine )
		{
			fprintf(fl, "Shrine %d %d %d %d %s\n",r->vnum, r->shrine, r->shrinesize, r->shrinemajor, r->shrinedivine );
		}
	}
	fclose( fl );
	return 0;
}


int load_settings( char *file )
{
	FILE *fl;
	char line[1024];
	char option[512];
	char value[1024];
	char *p;
	int i;

	divinelist = NULL;
	fl = fopen( file, "r" );

	if ( !fl ) {
		clientfr("Settings File Error, attempting to load backup");
		if ( fl ) {fclose(fl);}
		fl = fopen("config.mapper.bck","r");
		if ( !fl ) {
			clientfr("Settings Backup File not found");
			fclose(fl);
			return 1;}}

	while( 1 )
	{
		fgets( line, 1024, fl );

		if ( feof( fl ) )
			break;

		/* Strip newline. */
		{
			p = line;
			while ( *p != '\n' && *p != '\r' && *p )
				p++;

			*p = 0;
		}

		if ( line[0] == '#' || line[0] == 0 || line[0] == ' ' )
			continue;

		p = get_string( line, option, 512 );

		p = get_string( p, value, 1024 );

		if ( !strcmp( option, "Title-Color" ) ||
				!strcmp( option, "Title-Colour" ) )
		{
			for ( i = 0; color_names[i].name; i++ )
			{
				if ( !strcmp( value, color_names[i].name ) )
				{
					room_color = color_names[i].title_code;
					daynight_color = daynight_names[i].title_code;
					room_color_len = color_names[i].length;
					break;
				}
			}
		}
		else if ( !strcmp( option, "Exit-Color") )
		{
			for ( i = 1; color_names[i].name; i++ )
			{
				if ( !strcmp( value, color_names[i].name ) )
				{
					exit_color = color_names[i].title_code;
					break;
				}
			}
		}

		else if ( !strcmp( option, "Disable-Swimming" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_swimming = 1;
			else if ( !strcmp( value, "no" ) )
				disable_swimming = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}

		else if ( !strcmp( option, "Disable-WhoList" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_wholist = 1;
			else if ( !strcmp( value, "no" ) )
				disable_wholist = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}

		else if ( !strcmp( option, "Disable-Alertness" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_alertness = 1;
			else if ( !strcmp( value, "no" ) )
				disable_alertness = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}

		else if ( !strcmp( option, "Disable-Locating" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_locating = 1;
			else if ( !strcmp( value, "no" ) )
				disable_locating = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}

		else if ( !strcmp( option, "Disable-AreaName" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_areaname = 1;
			else if ( !strcmp( value, "no" ) )
				disable_areaname = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}

		else if ( !strcmp( option, "Disable-MXPTitle" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_mxp_title = 1;
			else if ( !strcmp( value, "no" ) )
				disable_mxp_title = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}

		else if ( !strcmp( option, "Disable-MXPExits" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_mxp_exits = 1;
			else if ( !strcmp( value, "no" ) )
				disable_mxp_exits = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}

		else if ( !strcmp( option, "Disable-MXPMap" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_mxp_map = 1;
			else if ( !strcmp( value, "no" ) )
				disable_mxp_map = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}

		else if ( !strcmp( option, "Disable-AutoLink" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_autolink = 1;
			else if ( !strcmp( value, "no" ) )
				disable_autolink = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}

		else if ( !strcmp( option, "Disable-Artifacts" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_artifacts = 1;
			else if ( !strcmp( value, "no" ) )
				disable_artifacts = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}

		else if ( !strcmp( option, "Disable-AdditionalName" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_additional_name = 1;
			else if ( !strcmp( value, "no" ) )
				disable_additional_name = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Disable-AutoUnlock" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_auto_unlock = 1;
			else if ( !strcmp( value, "no" ) )
				disable_auto_unlock = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Disable-WormWarp" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_worm_warp = 1;
			else if ( !strcmp( value, "no" ) )
				disable_worm_warp= 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Disable-ShrineInfluence" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_shrineinfluence = 1;
			else if ( !strcmp( value, "no" ) )
				disable_shrineinfluence = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Disable-ShrineRadius" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_shrineradius = 1;
			else if ( !strcmp( value, "no" ) )
				disable_shrineradius = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Disable-ShrineRadiusCheck" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_shrineradiuscheck = 1;
			else if ( !strcmp( value, "no" ) )
				disable_shrineradiuscheck = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Underwater-Breathing" ) )
		{
			if ( !strcmp(value, "0") )
				wateroption = 0;
			else if ( !strcmp(value, "1") )
				wateroption = 1;
			else if ( !strcmp(value, "3") )
				wateroption = 3;
			else {
				wateroption = 2;
				underitem = malloc(30);strcpy(underitem, value);
			}

		}
		else if ( !strcmp( option, "Disable-FarsightPath" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_farsightpath = 1;
			else if ( !strcmp( value, "no" ) )
				disable_farsightpath = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Disable-AutoMap" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_automap = 1;
			else if ( !strcmp( value, "no" ) )
				disable_automap = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "AutoMap-Size" ) )
		{
			if ( atoi(value) )
				automapsize = atoi(value);
			else if ( atoi(value) == 0 )
				automapsize = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Territory-Show" ) )
		{
			if ( !strcmp( value, "RoomType" ) )
				disable_mapterritory = 1;
			else if ( !strcmp( value, "Ruler" ) )
				disable_mapterritory = 0;
			else
				debugf( "Parse error in file '%s', expected 'RoomType' or 'Ruler', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Disable-AutoBackup" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_autobackup = 1;
			else if ( !strcmp( value, "no" ) )
				disable_autobackup = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Disable-AutoShrine" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_autoshrine = 1;
			else if ( !strcmp( value, "no" ) )
				disable_autoshrine = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Disable-ForceWrap" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_forcewrap = 1;
			else if ( !strcmp( value, "no" ) )
				disable_forcewrap = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Disable-Vnum" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_vnum = 1;
			else if ( !strcmp( value, "no" ) )
				disable_vnum = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Disable-Pathwrap" ) )
		{
			if ( !strcmp( value, "yes" ) )
				disable_pathwrap = 1;
			else if ( !strcmp( value, "no" ) )
				disable_pathwrap = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Mapwing-Command" ) )
		{
			if ( !strcmp( value, "nothing" ) ) {
                wingcmd = malloc(60); strcpy(wingcmd, value);wingcmd=NULL;free(wingcmd);}
			else if ( strcmp( value, "nothing" ) )
				{
				    wingcmd = malloc(60); strcpy(wingcmd, value);
				}
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}
		else if ( !strcmp( option, "Mapwing-Room" ) )
		{
			if ( atoi(value) )
				wingroom = atoi(value);
			else if ( atoi(value) == 0 )
				wingroom = 0;
			else
				debugf( "Parse error in file '%s', expected 'yes' or 'no', got '%s' instead.", file, value );
		}

		else if ( !strcmp( option, "Land-Mark" ) )
		{
			ROOM_DATA *room;
			ELEMENT *tag;
			int vnum = atoi( value );

			if ( !vnum )
				debugf( "Parse error in file '%s', expected a landmark vnum, got '%s' instead.", file, value );
			else
			{
				room = get_room( vnum );
				if ( !room )
					debugf( "Warning! Unable to landmark room %d, it doesn't exist!", vnum );
				else
				{
					/* Make sure it doesn't exist, first. */
					for ( tag = room->tags; tag; tag = tag->next )
						if ( !strcmp( "mark", (char *) tag->p ) )
							break;

					if ( !tag )
					{
						/* Link it. */
						tag = calloc( 1, sizeof( ELEMENT ) );
						tag->p = strdup( "mark" );
						link_element( tag, &room->tags );
					}
				}
			}
		}
		else if ( !strcmp(option, "Div:") )
		{
			char rela[256];
			p = get_string( p, rela, 256 );
			add_divine(value, rela);
		}
		else if ( !strcmp( option, "Shrine") )
		{
			char buf[256];
			int vnum;
			char shrine[256];
			char size[256];
			char divine[256];
			char stype[256];
			vnum = atoi(value);
			p = get_string(p, shrine, 256);
			p = get_string(p, size, 256);
			p = get_string(p, stype, 256);
			p = get_string(p, divine, 256);

			if ( atoi(size) == 1 )
				strcpy(size,"small");
			if ( atoi(size) == 2 )
				strcpy(size,"medium");
			if ( atoi(size) == 3 )
				strcpy(size,"large");
			if ( atoi(shrine) == 1 )
				strcpy(shrine,"neutral");
			if ( atoi(shrine) == 2 )
				strcpy(shrine,"friendly");
			if ( atoi(shrine) == 3 )
				strcpy(shrine,"enemy");
			if ( stype[0] != '0' && stype[0] != '1' && stype[0] != '2' ) {
				strcpy(divine,stype);
				strcpy(stype,"shrine");}
			if ( stype[0] == '2' )
				strcpy(stype,"master");
			if ( stype[0] == '1' )
				strcpy(stype,"monument");
			if ( stype[0] == '0' )
				strcpy(stype,"shrine");

			sprintf(buf, "%d hide %s %s %s %s", vnum, divine, size, stype, shrine);
			do_map_shrine( buf );

		}
		else if ( !strcmp( option, "Tag" ) )
		{
			ROOM_DATA *room;
			ELEMENT *tag;
			char buf[256];
			int vnum;

			vnum = atoi( value );
			get_string( p, buf, 256 );

			if ( !vnum || !buf[0] )
				debugf( "Parse error in file '%s', in a Tag line.", file );
			else
			{
				room = get_room( vnum );
				if ( !room )
					debugf( "Warning! Unable to tag room %d, it doesn't exist!", vnum );
				else
				{
					/* Make sure it doesn't exist, first. */
					for ( tag = room->tags; tag; tag = tag->next )
						if ( !strcmp( buf, (char *) tag->p ) )
							break;

					if ( !tag )
					{
						/* Link it. */
						tag = calloc( 1, sizeof( ELEMENT ) );
						tag->p = strdup( buf );
						link_element( tag, &room->tags );
					}
				}
			}
		}

		else
			debugf( "Parse error in file '%s', unknown option '%s'.", file, option );
	}
	if (!divinelist)
		add_divine("dummy","neutral");
	fclose( fl );
	return 0;
}



void save_map( char *file )
{
	AREA_DATA *area;
	ROOM_DATA *room;
	EXIT_DATA *spexit;
	ROOM_TYPE *type;
	WORMHOLE_DATA *worms;
	FILE *fl;
	char buf[256];
	int i, j,c, count = 0;

	DEBUG( "save_map" );
	/* backup sequence */
	if ( !disable_autobackup ) {
		FILE *bcks,*bckf;
		int bt;
		bcks = fopen( file, "rb");
		bckf = fopen( "IMap.bck", "wb" );
		if ( bcks==NULL || bckf==NULL )
		{clientfr("Backup Error");
			if ( bcks==NULL )
				clientfr("No File to Backup");
			if ( bckf==NULL )
				clientfr("Error Creating Backup");}
		else
		{
			while(1)
			{
				if( ( bt=fgetc(bcks) ) && bt!=EOF)
				{
					fputc(bt,bckf);
				}
				else
				{
					clientfr("Map Backup Successful");
					fclose( bckf );
					fclose( bcks );
					break;
				}
			}
		}
	}
	/* backup ended */
	fl = fopen( file, "w" );

	if ( !fl )
	{
		sprintf( buf, "Can't open '%s' to save map: %s.", file,
				strerror( errno ) );
		clientfr( buf );
		return;
	}

	fprintf( fl, "MAPFILE\n\n" );

	fprintf( fl, "\n\nROOM-TYPES\n\n"
			"# \"Name\", Color Name, Cost In, Cost Out, Swim, Underwater\n" );

	for ( type = room_types; type; type = type->next )
	{
		/* Find the color's name. */

		for ( j = 0; color_names[j].name; j++ )
			if ( !strcmp( color_names[j].code, type->color ) )
				break;

		fprintf( fl, "T: \"%s\" %s %d %d %s %s\n", type->name,
				color_names[j].name ? color_names[j].name : "normal",
				type->cost_in, type->cost_out,
				type->must_swim ? "yes" : "no",
				type->underwater ? "yes" : "no" );
	}

	fprintf(fl, "\n\nCities\n\n"
			"# \"Cityname\" Color\n");

	for ( j = 0; cities[j].name; j++ )
	{ for ( c = 0; color_names[c].name; c++ )
		if ( color_names[c].code == cities[j].color )
			fprintf(fl,"C: \"%s\" %s\n", cities[j].name,color_names[c].name );
	}
	fprintf( fl, "\n\nMESSAGES\n\n" );

	for ( spexit = global_special_exits; spexit; spexit = spexit->next )
	{
		fprintf( fl, "GSE: %d \"%s\" \"%s\"\n", spexit->new_vnum ? spexit->new_vnum : spexit->vnum,
				spexit->command ? spexit->command : "", spexit->message );
	}

	fprintf( fl, "\n\n" );

	for ( area = areas; area; area = area->next, count++ )
	{
		fprintf( fl, "AREA\n" );
		fprintf( fl, "Name: %s\n", area->name );
		if ( area->note )
			fprintf( fl, "Note: %s\n", area->note );
		if ( area->disabled )
			fprintf( fl, "Disabled\n" );
        if ( area->nowingarea )
            fprintf( fl, "NoWings\n");
		if ( area->city )
			fprintf( fl, "City\n" );
		fprintf( fl, "\n" );

		for ( room = area->rooms; room; room = room->next_in_area )
		{
			fprintf( fl, "ROOM v%d\n", room->vnum );
			fprintf( fl, "Name: %s\n", room->name );
			if ( room->room_type != null_room_type )
				fprintf( fl, "Type: %s\n", room->room_type->name );

			for ( i = 1; room->additional_name[i]; i++ )
			{
				fprintf( fl, "AddN: %s\n", room->additional_name[i] );
			}
			if ( room->wormhole )
			{
				worms = room->wormhole;
				if ( worms->vnum )
					fprintf( fl, "Wormhole: %d\n", worms->new_vnum ? worms->new_vnum : worms->vnum );
			}
			if ( room->underwater )
				fprintf( fl, "Underwater\n" );
			if ( room->shop )
				fprintf(fl, "Shop\n" );
			if ( room->avoid )
				fprintf(fl, "Avoid\n" );
			if ( room->hasrandomexits )
				fprintf(fl, "Random\n");
			if ( room->owner )
				fprintf(fl, "Owned by %s\n",room->owner);
			if ( room->ruler )
				fprintf(fl, "Ruler %s\n",room->ruler);
			if ( room->aetvnum > 0 )
				fprintf(fl, "AetoliaV %d\n",room->aetvnum );
			for ( i = 1; dir_name[i]; i++ )
			{
				if ( room->exits[i] )
				{
					fprintf( fl, "E: %s %d\n", dir_name[i],
							room->exits[i]->vnum );
					if ( room->exit_length[i] )
						fprintf( fl, "EL: %s %d\n", dir_name[i],
								room->exit_length[i] );
					if ( room->trueexit[i] )
						fprintf( fl, "ET: %s\n", dir_name[i] );
					if ( room->exit_stops_mapping[i] )
						fprintf( fl, "ES: %s %d\n", dir_name[i],
								room->exit_stops_mapping[i] );
					if ( room->use_exit_instead[i] &&
							room->use_exit_instead[i] != i )
						fprintf( fl, "UE: %s %s\n", dir_name[i],
								dir_name[room->use_exit_instead[i]] );
					if ( room->exit_joins_areas[i] )
						fprintf( fl, "EJ: %s %d\n", dir_name[i],
								room->exit_joins_areas[i] );
				}
				else if ( room->detected_exits[i] )
				{
					if ( room->locked_exits[i] )
						fprintf( fl, "DEL: %s\n", dir_name[i] );
					else
						fprintf( fl, "DE: %s\n", dir_name[i] );
				}
			}

			for ( spexit = room->special_exits; spexit; spexit = spexit->next )
			{
				fprintf( fl, "SPE: %d %d \"%s\" \"%s\" %d %d\n",
						spexit->new_vnum ? spexit->new_vnum : spexit->vnum, spexit->alias,
						spexit->command ? spexit->command : "",
						spexit->message ? spexit->message : "",spexit->nolook,spexit->nomsg );
			}
			for ( j = 1; dir_name[j]; j++ )
			{
				if ( room->noexitcmd[j] )
					fprintf( fl, "NOE: %d \"%s\" \"%s\"\n", j, room->noexitcmd[j] ? room->noexitcmd[j] : "", room->noexitmsg[j] ? room->noexitmsg[j] : "" );
			}
			fprintf( fl, "\n" );
		}
		fprintf( fl, "\n\n" );
	}

	fprintf( fl, "EOF\n" );

	sprintf( buf, "%d areas saved.", count );
	clientfr( buf );

	fclose( fl );
}



void remake_vnum_exits( )
{
	ROOM_DATA *room;
	int i;

	for ( room = world; room; room = room->next_in_world )
	{
		for ( i = 1; dir_name[i]; i++ )
		{
			if ( room->exits[i] )
				room->vnum_exits[i] = room->exits[i]->vnum;
		}
	}
}



void write_string( char *string, FILE *fl )
{
	int len = 0;

	if ( !string )
	{
		fwrite( &len, sizeof( int ), 1, fl );
		return;
	}

	len = strlen( string ) + 1;

	fwrite( &len, sizeof( int ), 1, fl );

	fwrite( string, sizeof( char ), len, fl );
}



char *read_string( FILE *fl )
{
	char *string;
	int len;

	fread( &len, sizeof( int ), 1, fl );

	if ( !len )
		return NULL;

	if ( len > 256 )
	{
		debugf( "Warning! Attempting to read an overly long string." );
		return NULL;
	}

	string = malloc( len );
	fread( string, sizeof( char ), len, fl );

	return string;
}



void save_binary_map( char *file )
{
	clientfr("Binary maps have been disabled");
	return;
	AREA_DATA *area;
	ROOM_DATA *room;
	EXIT_DATA *spexit;
	ROOM_TYPE *type;
	FILE *fl;
	int nr;

	DEBUG( "save_binary_map" );

	fl = fopen( file, "wb" );

	if ( !fl )
		return;

	remake_vnum_exits( );

	/* Room Types. */
	/* Format: int, N*ROOM_TYPE, ... */

	/* Count them. */
	for ( nr = 0, type = room_types; type; type = type->next )
		nr++;

	fwrite( &nr, sizeof( int ), 1, fl );
	for ( type = room_types; type; type = type->next )
	{
		fwrite( type, sizeof( ROOM_TYPE ), 1, fl );
		write_string( type->name, fl );
		write_string( type->color, fl );
	}

	/* Global Special Exits. */
	/* Format: ..., int, N*EXIT_DATA, ... */

	/* Count them. */
	for ( nr = 0, spexit = global_special_exits; spexit; spexit = spexit->next )
		nr++;

	fwrite( &nr, sizeof( int ), 1, fl );
	for ( spexit = global_special_exits; spexit; spexit = spexit->next )
	{
		fwrite( spexit, sizeof( EXIT_DATA ), 1, fl );
		write_string( spexit->command, fl );
		write_string( spexit->message, fl );
	}

	/* Areas. */
	/* Format: ..., int, N*[AREA_DATA, int, M*[...]]. */

	/* Count them. */
	for ( nr = 0, area = areas; area; area = area->next )
		nr++;

	fwrite( &nr, sizeof( int ), 1, fl );
	for ( area = areas; area; area = area->next )
	{
		fwrite( area, sizeof( AREA_DATA ), 1, fl );
		write_string( area->name, fl );

		/* Rooms. */
		/* Format: int, M*[ROOM_DATA, int, P*EXIT_DATA] */

		for ( nr = 0, room = area->rooms; room; room = room->next_in_area )
			nr++;

		fwrite( &nr, sizeof( int ), 1, fl );
		for ( room = area->rooms; room; room = room->next_in_area )
		{
			fwrite( room, sizeof( ROOM_DATA ), 1, fl );
			write_string( room->name, fl );
			if ( room->room_type )
				write_string( room->room_type->name, fl );

			/* Special Exits. */

			/* Count them. */
			for ( nr = 0, spexit = room->special_exits; spexit; spexit = spexit->next )
				nr++;

			fwrite( &nr, sizeof( int ), 1, fl );
			for ( spexit = room->special_exits; spexit; spexit = spexit->next )
			{
				fwrite( spexit, sizeof( EXIT_DATA ), 1, fl );
				write_string( spexit->command, fl );
				write_string( spexit->message, fl );
			}
		}
	}

	fwrite( "x", sizeof( char ), 1, fl );
	fclose( fl );
}



int load_binary_map( char *file )
{
	clientfr("Binary maps have been disabled");
	return 0;
	AREA_DATA area, *a;
	ROOM_DATA room, *r;
	EXIT_DATA spexit, *spe;
	ROOM_TYPE type, *t;
	FILE *fl;
	char check, *type_name;
	int types, areas, rooms, exits;
	int i, j, k;

	DEBUG( "load_binary_map" );

	fl = fopen( file, "rb" );

	if ( !fl )
		return 1;

	/* Room Types. */
	fread( &types, sizeof( int ), 1, fl );
	for ( i = 0; i < types; i++ )
	{
		fread( &type, sizeof( ROOM_TYPE ), 1, fl );

		type.name = read_string( fl );
		type.color = read_string( fl );
		if ( !type.name || !type.color )
		{
			debugf( "NULL entries where they shouldn't be!" );
			return 1;
		}

		add_room_type( type.name, type.color, type.cost_in, type.cost_out,
				type.must_swim, type.underwater );
	}

	/* Global Special Exits. */
	fread( &exits, sizeof( int ), 1, fl );
	for ( i = 0; i < exits; i++ )
	{
		fread( &spexit, sizeof( EXIT_DATA ), 1, fl );
		spe = create_exit( NULL );

		spe->alias = spexit.alias;
		spe->vnum = spexit.vnum;
		spe->command = read_string( fl );
		spe->message = read_string( fl );
	}

	/* Areas. */
	fread( &areas, sizeof( int ), 1, fl );
	for ( i = 0; i < areas; i++ )
	{
		fread( &area, sizeof( AREA_DATA ), 1, fl );
		a = create_area( );

		a->disabled = area.disabled;
		a->name = read_string( fl );

		current_area = a;

		/* Rooms. */
		fread( &rooms, sizeof( int ), 1, fl );
		for ( j = 0; j < rooms; j++ )
		{
			fread( &room, sizeof( ROOM_DATA ), 1, fl );
			r = create_room( room.vnum );
			if ( r->vnum > last_vnum )
				last_vnum = r->vnum;

			r->underwater = room.underwater;
			memcpy( r->vnum_exits, room.vnum_exits, sizeof(int)*13 + sizeof(short)*13*6 );
			r->name = read_string( fl );

			if ( room.room_type )
			{
				type_name = read_string( fl );
				for ( t = room_types; t; t = t->next )
					if ( !strcmp( type_name, t->name ) )
					{
						r->room_type = t;
						break;
					}
				free( type_name );
			}

			/* Special Exits. */
			fread( &exits, sizeof( int ), 1, fl );
			for ( k = 0; k < exits; k++ )
			{
				fread( &spexit, sizeof( EXIT_DATA ), 1, fl );
				spe = create_exit( r );

				spe->alias = spexit.alias;
				spe->vnum = spexit.vnum;
				spe->command = read_string( fl );
				spe->message = read_string( fl );
			}
		}
	}

	fread( &check, sizeof( char ), 1, fl );

	fclose( fl );

	if ( check != 'x' )
	{
		debugf( "The binary IMap file is corrupted!" );
		return 1;
	}

	return 0;
}



int check_map( )
{
	DEBUG( "check_map" );


	return 0;
}



void remake_vnums( void )
{
	ROOM_DATA *room, *rf, *rt;
	EXIT_DATA *spexit;
	AREA_DATA *area;
	char buf[128];
	int i = 1;

	DEBUG( "remake_vnums" );

	for ( area = areas; area; area = area->next )
	{
		for ( room = area->rooms; room; room = room->next_in_area )
		{
			room->vnum_old = room->vnum;
			room->vnum = i++;
		}
	}
	sprintf( buf, "Room vnums remade. Last is %d.", i-1 );
	clientfr( buf );

	for ( rf = world; rf; rf = rf->next_in_world )
	{
		for ( spexit = rf->special_exits; spexit; spexit = spexit->next )
		{
			for ( rt = world; rt; rt = rt->next_in_world )
			{
				if (rt->vnum_old == spexit->vnum) {
					spexit->new_vnum = rt->vnum;
				}
			}
		}
	}
	/* Wormholes */
	for ( rf = world; rf; rf = rf->next_in_world )
	{
		if ( rf->wormhole )
		{
			for ( rt = world; rt; rt = rt->next_in_world )
			{
				if (rt->vnum_old == rf->wormhole->vnum ) {
					rf->wormhole->new_vnum = rt->vnum;
				}
			}
		}
	}
	for ( spexit = global_special_exits; spexit; spexit = spexit->next )
	{
		for ( rt = world; rt; rt = rt->next_in_world )
		{if (rt->vnum_old == spexit->vnum) {spexit->new_vnum = rt->vnum;}}
	}
	clientfr("Automatic Reloading");
	save_map( map_file );
	int save_settings( char *file);
	save_settings( "config.mapper.txt" );
	void do_map_load( char *arg );
	do_map_load( "" );
}



void convert_vnum_exits( )
{
	ROOM_DATA *room;
	EXIT_DATA *spexit;
	WORMHOLE_DATA *worms;
	int i;

	DEBUG( "convert_vnum_exits" );

	for ( room = world; room; room = room->next_in_world )
	{
		/* Normal exits. */
		for ( i = 1; dir_name[i]; i++ )
		{
			if ( room->vnum_exits[i] )
			{
				room->exits[i] = get_room( room->vnum_exits[i] );
				if ( !room->exits[i] )
				{
					debugf( "Can't link room %d (%s) to %d.",
							room->vnum, room->name, room->vnum_exits[i] );
					continue;
				}
				set_reverse( room, i, room->exits[i] );
				//		  room->exits[i]->reverse_exits[i] = room;
				room->vnum_exits[i] = 0;
			}
		}

		/* Special exits. */
		for ( spexit = room->special_exits; spexit; spexit = spexit->next )
		{
			if ( spexit->vnum < 0 )
				spexit->to = NULL;
			else
			{
				if ( !( spexit->to = get_room( spexit->vnum ) ) )
				{
					debugf( "Can't link room %d (%s) to %d. (special exit)",
							room->vnum, room->name, spexit->vnum );
				}
				else
					spexit->to->pointed_by = 1;
			}
		}
		for ( worms = room->wormhole; worms; worms = worms->next )
		{
			if ( worms->vnum < 0 )
				worms->to = NULL;
			else
			{
				if ( !( worms->to = get_room( worms->vnum ) ) )
				{
					debugf( "Can't link room %d (%s) to %d. (wormhole)",
							room->vnum, room->name, worms->vnum );
					worms->to = NULL;
					free( worms );
				}
				else
					worms->to->worm_pointed_by = 1;
			}
		}
	}

	for ( spexit = global_special_exits; spexit; spexit = spexit->next )
	{
		if ( spexit->vnum < 0 )
			spexit->to = NULL;
		else
		{
			if ( !( spexit->to = get_room( spexit->vnum ) ) )
			{
				debugf( "Can't link global special exit '%s' to %d.",
						spexit->command, spexit->vnum );
			}
		}
	}
}



int load_map( char *file )
{
	AREA_DATA *area = NULL;
	ROOM_DATA *room = NULL;
	ROOM_TYPE *type;
	FILE *fl;
	char buf[512];
	char line[512];
	int section = 0;
	int nr = 0, i;

	DEBUG( "load_map" );

	fl = fopen( file, "r" );

	if ( !fl )
	{
		clientfr("Map File Error, attempting to load backup");
		fclose(fl);
		fl = fopen("IMap.bck","r");
		if ( !fl ) {
			sprintf( buf, "Can't open '%s' to load map: %s.", file,
					strerror( errno ) );
			clientfr( buf );
			fclose(fl);
			return 1;
		}}

	while( 1 )
	{
		fgets( line, 512, fl );
		if ( feof( fl ) )
			break;

		if ( !strncmp( line, "EOF", 3 ) )
			break;

		nr++;

		if ( !line[0] )
			continue;

		if ( !strncmp( line, "AREA", 4 ) )
		{
			area = create_area( );
			current_area = area;
			section = 1;
		}

		else if ( !strncmp( line, "ROOM v", 6 ) )
		{
			int vnum;

			if ( !area )
			{
				debugf( "Room out of area. Line %d.", nr );
				return 1;
			}

			vnum = atoi( line + 6 );
			room = create_room( vnum );
			if ( vnum > last_vnum )
				last_vnum = vnum;
			section = 2;
		}

		else if ( !strncmp( line, "ROOM-TYPES", 10 ) )
		{
			section = 3;
		}

		else if ( !strncmp( line, "MESSAGES", 8 ) )
		{
			section = 4;
		}

		/* Strip newline. */
		{
			char *p = line;
			while ( *p != '\n' && *p != '\r' && *p )
				p++;

			*p = 0;
		}

		if ( !strncmp( line, "Name: ", 6 ) )
		{
			if ( section == 1 )
			{
				if ( area->name )
				{
					debugf( "Double name, on area. Line %d.", nr );
					return 1;
				}

				area->name = strdup( line + 6 );
			}
			else if ( section == 2 )
			{
				if ( room->name )
				{
					debugf( "Double name, on room. Line %d.", nr );
					return 1;
				}

				room->name = strdup( line + 6 );
			}
		}

		else if ( !strncmp(line, "Note: ", 6) && section == 1 && area->name )
		{
			area->note = strdup( line + 6 );
		}

		else if ( !strncmp( line, "Type: ", 6 ) )
		{
			/* Type: Road */
			for ( type = room_types; type; type = type->next )
				if ( !strncmp( line+6, type->name, strlen( type->name ) ) )
				{
					room->room_type = type;
					break;
				}
		}

		else if ( !strncmp( line, "AddN: ", 6 ) )
		{
			for ( i = 1; i < 10; i++ )
			{
				if (room->additional_name[i] == NULL)
				{
					room->additional_name[i] = strdup( line + 6 );
					break;
				}
			}
		}

		else if ( !strncmp( line, "E: ", 3 ) )
		{
			/* E: northeast 14 */
			char buf[256];
			int dir;

			if ( section != 2 )
			{
				debugf( "Misplaced exit line. Line %d.", nr );
				return 1;
			}

			sscanf( line, "E: %s %d", buf, &dir );

			if ( !dir )
			{
				debugf( "Syntax error at exit, line %d.", nr );
				return 1;
			}

			for ( i = 1; dir_name[i]; i++ )
			{
				if ( !strcmp( buf, dir_name[i] ) )
				{
					room->vnum_exits[i] = dir;
					break;
				}
			}
		}

		else if ( !strncmp( line, "DE: ", 4 ) )
		{
			/* DE: northeast */
			char buf[256];

			sscanf( line, "DE: %s", buf );

			for ( i = 1; dir_name[i]; i++ )
			{
				if ( !strcmp( buf, dir_name[i] ) )
				{
					room->detected_exits[i] = 1;
					break;
				}
			}
		}

		else if ( !strncmp( line, "DEL: ", 4 ) )
		{
			/* DEL: northeast */
			char buf[256];

			sscanf( line, "DEL: %s", buf );

			for ( i = 1; dir_name[i]; i++ )
			{
				if ( !strcmp( buf, dir_name[i] ) )
				{
					room->detected_exits[i] = 1;
					room->locked_exits[i] = 1;
					break;
				}
			}
		}

		else if ( !strncmp( line, "EL: ", 4 ) )
		{
			/* EL: northeast 1 */
			char buf[256];
			int l;

			sscanf( line, "EL: %s %d", buf, &l );

			if ( l < 1 )
			{
				debugf( "Syntax error at exit length, line %d.", nr );
				return 1;
			}

			for ( i = 1; dir_name[i]; i++ )
			{
				if ( !strcmp( buf, dir_name[i] ) )
				{
					room->exit_length[i] = l;
					break;
				}
			}
		}

		else if ( !strncmp( line, "ET: ", 4 ) )
		{
			/* EL: northeast 1 */
			char buf[256];

			sscanf( line, "ET: %s", buf );

			for ( i = 1; dir_name[i]; i++ )
			{
				if ( !strcmp( buf, dir_name[i] ) )
				{
					room->trueexit[i]= 1;
					break;
				}
			}
		}

		else if ( !strncmp( line, "ES: ", 4 ) )
		{
			/* ES: northeast 1 */
			char buf[256];
			int s;

			sscanf( line, "ES: %s %d", buf, &s );

			for ( i = 1; dir_name[i]; i++ )
			{
				if ( !strcmp( buf, dir_name[i] ) )
				{
					room->exit_stops_mapping[i] = s;
					break;
				}
			}
		}

		else if ( !strncmp( line, "EJ: ", 4 ) )
		{
			/* EJ: northeast 1 */
			char buf[256];
			int s;

			sscanf( line, "EJ: %s %d", buf, &s );

			for ( i = 1; dir_name[i]; i++ )
			{
				if ( !strcmp( buf, dir_name[i] ) )
				{
					room->exit_joins_areas[i] = s;
					break;
				}
			}
		}

		else if ( !strncmp( line, "UE: ", 4 ) )
		{
			/* UE in east */
			char buf1[256], buf2[256];
			int j;

			sscanf( line, "UE: %s %s", buf1, buf2 );

			for ( i = 1; dir_name[i]; i++ )
			{
				if ( !strcmp( dir_name[i], buf1 ) )
				{
					for ( j = 1; dir_name[j]; j++ )
					{
						if ( !strcmp( dir_name[j], buf2 ) )
						{
							room->use_exit_instead[i] = j;
							break;
						}
					}
					break;
				}
			}
		}

		else if ( !strncmp( line, "SPE: ", 5 ) )
		{
			EXIT_DATA *spexit;

			char vnum[1024];
			char alias[1024];
			char cmd[1024];
			char msg[1024];
			char nolook[1024];
			char nomsg[1024];
			char *p = line + 5;

			/* SPE: -1 0 "pull lever" "You pull a lever, and fall below the ground." */

			p = get_string( p, vnum, 1024 );
			p = get_string( p, alias, 1024 );
			p = get_string( p, cmd, 1024 );
			p = get_string( p, msg, 1024 );
			p = get_string( p, nolook, 1024);
			p = get_string( p, nomsg, 1024);

			spexit = create_exit( room );

			if ( vnum[0] == '-' )
				spexit->vnum = -1;
			else
				spexit->vnum = atoi( vnum );

			if ( alias[0] )
				spexit->alias = atoi( alias );
			else
				spexit->alias = 0;

			if ( cmd[0] )
				spexit->command = strdup( cmd );
			else
				spexit->command = NULL;

			if ( msg[0] )
				spexit->message = strdup( msg );
			else
				spexit->message = NULL;

			if ( nolook[0] )
				spexit->nolook = atoi( nolook );
			else
				spexit->nolook = 0;

			if ( nomsg[0] )
				spexit->nomsg = atoi( nomsg );
			else
				spexit->nomsg = 0;	  }

		else if ( !strncmp( line, "NOE: ", 5 ) )
		{
			char dir[256];
			int d;
			char cmd[1024];
			char msg[1024];
			char *p = line + 5;

			p = get_string( p, dir, 1024 );
			p = get_string( p, cmd, 1024 );
			p = get_string( p, msg, 1024 );


			d = atoi( dir );

			if ( cmd[0] )
				room->noexitcmd[d] = strdup( cmd );
			else
				room->noexitcmd[d] = NULL;

			if ( msg[0] )
				room->noexitmsg[d] = strdup( msg );
			else
				room->noexitmsg[d] = NULL;
		}

		else if ( !strncmp( line, "Wormhole: ", 10 ) )
		{
			WORMHOLE_DATA *warp;
			int warpto;

			if ( section != 2 )
			{
				debugf( "Misplaced wormhole line. Line %d.", nr );
				return 1;
			}

			sscanf( line, "Wormhole: %d", &warpto );

			if ( !warpto )
			{
				debugf( "Syntax error with wormhole data at line %d.", nr );
				return 1;
			}

			warp = create_wormhole( room );
			warp->vnum = warpto;


		}
		else if ( !strncmp( line, "Disabled", 8 ) )
		{
			if ( section != 1 )
			{
				debugf( "Wrong section of a Disabled statement." );
				return 1;
			}

			area->disabled = 1;
		}
		else if ( !strncmp( line, "NoWings", 8 ) )
		{
			if ( section != 1 )
			{
				debugf( "Wrong section of a Disabled statement." );
				return 1;
			}

			area->nowingarea = 1;
		}
		else if ( !strncmp( line, "City", 4 ) )
		{
			if ( section != 1 )
			{
				debugf( "Wrong section of a Disabled statement." );
				return 1;
			}

			area->city = 1;
		}

		else if ( !strncmp( line, "Underwater", 10 ) )
		{
			if ( section != 2 )
			{
				debugf( "Wrong section of an Underwater statement." );
				return 1;
			}

			room->underwater = 1;
		}
		else if ( !strncmp( line, "Shop", 4 ) )
		{
			if ( section != 2 )
			{
				debugf( "Wrong section of a shop statement." );
				return 1;
			}
			room->shop = 1;
		}
		else if ( !strncmp( line, "Avoid", 5 ) )
		{
			if ( section != 2 )
			{
				debugf( "Wrong section of a avoid statement." );
				return 1;
			}
			room->avoid = 1;
		}
		else if ( !strncmp( line, "Random", 6 ) )
		{
			if ( section != 2 )
			{
				debugf( "Wrong section of a avoid statement." );
				return 1;
			}
			room->hasrandomexits = 1;
		}
		else if ( !strncmp( line, "Owned by ", 9 ) )
		{
			if ( section != 2 )
			{
				debugf( "Wrong section of a avoid statement." );
				return 1;
			}
			room->owner = malloc(strlen(line+9)+1);strcpy(room->owner,line+9);
		}
		else if ( !strncmp( line, "Ruler ", 6 ) )
		{
			if ( section != 2 )
			{
				debugf( "Wrong section of a avoid statement." );
				return 1;
			}
			room->ruler = malloc(strlen(line+6)+1);strcpy(room->ruler,line+6);
		}
		else if ( !strncmp(line, "AetoliaV ", 9 ) )
		{ if ( section != 2 )
			{
				debugf("wrong section for aetolia vnum statement.");
				return 1;
			}
			room->aetvnum = atoi(line+9);
		}
		/* Deprecated. Here only for backwards compatibility. */
		else if ( !strncmp( line, "Marked", 6 ) )
		{
			ELEMENT *tag;

			if ( section != 2 )
			{
				debugf( "Wrong section of a Marked statement." );
				return 1;
			}

			/* Make sure it doesn't exist, first. */
			for ( tag = room->tags; tag; tag = tag->next )
				if ( !strcmp( "tag", (char *) tag->p ) )
					break;

			if ( !tag )
			{
				/* Link it. */
				tag = calloc( 1, sizeof( ELEMENT ) );
				tag->p = strdup( buf );
				link_element( tag, &room->tags );
			}
		}

		else if ( !strncmp( line, "T: ", 3 ) )
		{
			char name[512];
			char color_name[512];
			char *color;
			char buf[512];
			char *p = line + 3;
			int cost_in, cost_out;
			int must_swim;
			int underwater;

			if ( section != 3 )
			{
				debugf( "Misplaced room type. Line %d.", nr );
				return 1;
			}

			p = get_string( p, name, 512 );
			p = get_string( p, color_name, 512 );
			p = get_string( p, buf, 512 );
			cost_in = atoi( buf );
			p = get_string( p, buf, 512 );
			cost_out = atoi( buf );
			p = get_string( p, buf, 512 );
			must_swim = buf[0] == 'y' ? 1 : 0;
			p = get_string( p, buf, 512 );
			underwater = buf[0] == 'y' ? 1 : 0;

			if ( !name[0] || !color_name[0] || !cost_in || !cost_out )
			{
				debugf( "Buggy Room-Type, line %d.", nr );
				return 1;
			}

			color = get_color( color_name );

			if ( !color )
			{
				debugf( "Unknown color name, line %d.", nr );
				return 1;
			}

			add_room_type( name, color, cost_in, cost_out, must_swim, underwater );
		}

		else if ( !strncmp( line, "C: ", 3 ) )
		{
			char cname[64];
			char color_name[256];
			char *color;
			char *p = line + 3;
			int n;

			if ( section != 3 )
			{
				debugf( "Misplaced city line. Line %d.", nr );
				return 1;
			}

			p = get_string( p, cname, 64 );
			p = get_string( p, color_name, 256 );


			if ( !cname[0] || !color_name[0] )
			{
				debugf( "Buggy City color, line %d.", nr );
				return 1;
			}

			color = get_color( color_name );

			if ( !color )
			{
				debugf( "Unknown color name, line %d.", nr );
				return 1;
			}

			for ( n = 0; cities[n].name; n++ )
				if ( !strncmp(cname, cities[n].name, strlen(cities[n].name) ) )
					cities[n].color = color;

		}

		else if ( !strncmp( line, "GSE: ", 5 ) )
		{
			EXIT_DATA *spexit;

			char vnum[1024];
			char cmd[1024];
			char msg[1024];
			char *p = line + 5;

			if ( section != 4 )
			{
				debugf( "Misplaced global special exit. Line %d.", nr );
				return 1;
			}

			/* GSE: -1 "brazier" "You feel a moment of disorientation as you are summoned." */

			p = get_string( p, vnum, 1024 );
			p = get_string( p, cmd, 1024 );
			p = get_string( p, msg, 1024 );

			spexit = create_exit( NULL );

			if ( vnum[0] == '-' )
				spexit->vnum = -1;
			else
				spexit->vnum = atoi( vnum );

			if ( cmd[0] )
				spexit->command = strdup( cmd );
			else
				spexit->command = NULL;

			if ( msg[0] )
				spexit->message = strdup( msg );
			else
				spexit->message = NULL;
		}
	}

	fclose( fl );

	return 0;
}


void init_openlist( ROOM_DATA *room )
{
	ROOM_DATA *r;

	if ( !room )
	{
		pf_current_openlist = NULL;
		return;
	}
	/* Make sure it's not already there. */
	for ( r = pf_current_openlist; r; r = r->next_in_pfco )
		if ( r == room )
			return;

	/* Add it. */
	if ( pf_current_openlist )
	{
		room->next_in_pfco = pf_current_openlist;
		pf_current_openlist = room;
	}
	else
	{
		pf_current_openlist = room;
		room->next_in_pfco = NULL;
	}
}

int cmp_room_wing()
{
    if (wingroom == 0 || wingcmd == NULL || disable_artifacts || wingtmpdisable )
    return 0;

    if (!strcmp(current_room->area->name, "The Havens.")) {
    return 0;}
    if (!strcmp(current_room->area->name,"Hunting Grounds.")) {
    return 0;}
    if (current_room->area->nowingarea) {
    return 0;}

    ROOM_DATA *room;
    int lennorm = 0;
    int lenwing = 0;

    for (room = current_room; room; room = room->pf_parent) {lennorm++;}
    for (room = get_room(wingroom); room; room = room->pf_parent) {lenwing++;}
    if (lenwing<lennorm) {
    return 1;}
    else {
    return 0;}
}

int get_cost( ROOM_DATA *src, ROOM_DATA *dest )
{
	return src->room_type->cost_out + dest->room_type->cost_in;
}




void add_openlist( ROOM_DATA *src, ROOM_DATA *dest, int dir, int warp )
{
	ROOM_DATA *troom;

	//   DEBUG( "add_openlist" );


	/* To add, or not to add... now that is the question. */
	if (
			( (!warp) && (dest->pf_cost == 0) ) ||
			( ( src->pf_cost + get_cost( dest, src ) < dest->pf_cost ) && (!warp) ) ||
			warp
	   )
	{
		int found = 0;



		/* Link to the new openlist. */
		/* But first make sure it's not there already. */
		for ( troom = pf_new_openlist; troom; troom = troom->next_in_pfno )
			if ( troom == dest )
			{
				found = 1;
				break;
			}

		if ( !found )
		{
			dest->next_in_pfno = pf_new_openlist;
			pf_new_openlist = dest;
		}

		dest->pf_cost = src->pf_cost + get_cost( dest, src ) + (warp*6);
		dest->pf_parent = src;
		dest->pf_direction = dir;
	}
}

void path_finder( )
{
	ROOM_DATA *room, *r;
	EXIT_DATA *spexit;
	int i;

	DEBUG( "path_finder" );

	if ( !world )
		return;

	/* Okay, get ready by clearing it up. */
	for ( room = world; room; room = room->next_in_world )
	{
		room->pf_parent = NULL;
		room->pf_cost = 0;
		room->pf_direction = 0;
		room->warped = 0;
	}
	for ( room = pf_current_openlist; room; room = room->next_in_pfco )
		room->pf_cost = 1;

	pf_new_openlist = NULL;

	/* Were we sent NULL? Then it was just for the above, to clear it up. */

	if ( !pf_current_openlist )
		return;

	while ( pf_current_openlist )
	{
		/*** This was check_openlist( ) ***/

		room = pf_current_openlist;

		/* Walk around and create a new openlist. */
		while( room )
		{
			for ( i = 1; dir_name[i]; i++ )
			{
				/* Normal exits. */
				if ( room->reverse_exits[i] && room->reverse_exits[i]->exits[i] == room &&
						( room->area == igareaoff  || !room->area->disabled ) && !room->avoid && !( except != NULL && room == except ) &&
						!( burrowed && ( !strcmp( room->room_type->name , "Natural underground" ) ||
								!strcmp( room->room_type->name , "Constructed underground" ) ||
								!strcmp( room->room_type->name , "Swamp" ) ||
								!strcmp( room->room_type->name , "Undefined" ) ) ) )
				{
					if ( !room->more_reverse_exits[i] )
						add_openlist( room, room->reverse_exits[i], i, 0 );
					else {
						/* More rooms lead here. Search for them. */
						for ( r = world; r; r = r->next_in_world )
							if ( r->exits[i] == room )
								add_openlist( room, r, i, 0 );
					}
				}
			}
			/* Check for wormholes */
			if ( room->worm_pointed_by && !disable_worm_warp && !room->avoid && !room->warped && !( except != NULL && room == except )) {
				add_openlist( room, room->wormhole->to, -2, 1 );
				room->warped = 1;
				room->wormhole->to->warped = 1;
			}
			/* Special exits. */
			if ( room->pointed_by && !burrowed && !room->avoid && !( except != NULL && room == except ) )
			{
				for ( r = world; r; r = r->next_in_world )
				{
					int found = 0;

					for ( spexit = r->special_exits; spexit; spexit = spexit->next )
					{
						if ( spexit->to == room && !room->area->disabled &&
								( !disable_artifacts || !spexit->command ||
								  !( strstr( spexit->command, "Duanathar" ) || strstr( spexit->command, "voltda" ) ) ) )
						{
							found = 1;
							break;
						}
					}
					if ( found )
						add_openlist( room, r, -1, 0 );
				}
			}

			room = room->next_in_pfco;
		}

		/* Replace the current openlist with the new openlist. */
		pf_current_openlist = pf_new_openlist;
		for ( room = pf_new_openlist; room; room = room->next_in_pfno )
			room->next_in_pfco = room->next_in_pfno;
		pf_new_openlist = NULL;
	}
}

void show_path( ROOM_DATA *current )
{
	ROOM_DATA *room;
	EXIT_DATA *spexit;
	char buf[4096];
	int nr = 0;
	int wrap = 7; /* strlen( "[Path: " ) */
	int nrmax = 100;
	int hexited = 0;
    int aused = 0;

	DEBUG( "show_path" );


	if ( !current->pf_parent && !havenstore )
	{
		if ( !strcmp(current->area->name, "The Havens.") )
			clientfr("Will exit haven, and attempt to path.");
		else
			clientfr( "Can't find a path from here." );
		return;
	}
	if ( !current->pf_parent && havenstore )
	{
		current = havenstore;
	}
	if ( igareaoff )
		clientff(C_R"["C_Y"Warning"C_R": This will take you through "C_W"'"C_Y"%s"C_W"'"C_R" which is turned off]\r\n" C_0, igareaoff->name );
	if ( current != current_room )
		nrmax = 4000;

	sprintf( buf, C_R "[Path: " C_G );
    autowing = cmp_room_wing();
    if (autowing) {
    room = get_room(wingroom);
    artimsg = wingcmd;
    }
    else {
    room = current;}
	for ( room = room; room && room->pf_parent && nr < nrmax; room = room->pf_parent )
	{
		if ( wrap > 70 && !disable_pathwrap )
		{
			strcat( buf, C_R ",\r\n" C_G );
			wrap = 0;
		}
		else if ( nr++ )
		{
			strcat( buf, C_R ", " C_G );
			wrap += 2;
		}

		/* We also have special exits. */
		if (autowing && !aused)
		{strcat(buf, C_D);
			strcat( buf, wingcmd);
			wrap += strlen(wingcmd);
			aused = 1;

		    if ( wrap > 70 && !disable_pathwrap )
		    {
			    strcat( buf, C_R ",\r\n" C_G );
			    wrap = 0;
		    }
		    else if ( nr++ )
		    {
			    strcat( buf, C_R ", " C_G );
			    wrap += 2;
            }

			if ( ( room->pf_direction != -1 ) && ( room->pf_direction != -2 ) )
	    	{
			    if ( must_swim( room, room->pf_parent ) )
				    strcat( buf, C_B );

			    strcat( buf, dir_small_name[room->pf_direction] );
			    wrap += strlen( dir_small_name[room->pf_direction] );
		    }
		}
		else if ( !strcmp(current_room->area->name, "The Havens.") && !hexited )
		{strcat(buf, C_D);
			strcat( buf, "Exit Haven");
			wrap += 10;
			hexited=1;

		    if ( wrap > 70 && !disable_pathwrap )
		    {
			    strcat( buf, C_R ",\r\n" C_G );
			    wrap = 0;
		    }
		    else if ( nr++ )
		    {
			    strcat( buf, C_R ", " C_G );
			    wrap += 2;
		    }

			if ( ( room->pf_direction != -1 ) && ( room->pf_direction != -2 ) )
		     {
			    if ( must_swim( room, room->pf_parent ) )
				    strcat( buf, C_B );

			     strcat( buf, dir_small_name[room->pf_direction] );
                 wrap += strlen( dir_small_name[room->pf_direction] );
		     }
		}
		else if ( ( room->pf_direction != -1 ) && ( room->pf_direction != -2 ) )
		{
			if ( must_swim( room, room->pf_parent ) )
				strcat( buf, C_B );

			strcat( buf, dir_small_name[room->pf_direction] );
			wrap += strlen( dir_small_name[room->pf_direction] );
		}
		else if ( room->pf_direction == -1 )
		{
			/* Find the special exit, leading to the parent. */
			for ( spexit = room->special_exits; spexit; spexit = spexit->next )
			{
				if ( spexit->to == room->pf_parent )
				{
					if ( spexit->command )
					{
						strcat( buf, spexit->command );
						wrap += strlen( spexit->command );
					}
					else
					{
						strcat( buf, "?" );
						wrap++;
					}

					break;
				}
			}
		}
		else
		{
			strcat ( buf, "worm warp" );
			wrap += strlen( "worm warp" );
		}
	}
	strcat( buf, C_R ".]\r\n" C_0 );
	clientf( buf );
}



void i_mapper_mxp_enabled( )
{
	floating_map_enabled = 0;

	mxp_tag( TAG_LOCK_SECURE );
	mxp( "<!element mpelm '<send \"map path &v;|room look &v;\" "
			"hint=\"&r;|Vnum: &v;|Type: &t;\">' ATT='v r t'>" );
	mxp( "<!element mppers '<send \"map path &v;|room look &v;|who &p;\" "
			"hint=\"&p; (&r;)|Vnum: &v;|Type: &t;|Player: &p;\">' ATT='v r t p'>" );
	mxp( "<!element mpfind '<send \"room find &r;\">' ATT='r'>" );
	//   mxp( "<!element RmTitle '<font color=red>' TAG='RoomName'>" );
	//   mxp( "<!element RmExits TAG='RoomExit'>" );
	mxp_tag( TAG_DEFAULT );
}



void i_mapper_module_init_data( )
{
	struct stat map, mapbin, mapper;
	int binary,i;

	MAP_ELEMENT *big_map;
	big_map = calloc(map_x*map_y, sizeof(MAP_ELEMENT));
	map_new = calloc(map_x, sizeof(void*));
	for ( i = 0; i < map_x; i++ )
		map_new[i] = &big_map[i*map_y];

	DEBUG( "i_mapper_init_data" );

	null_room_type = add_room_type( "Unknown", get_color( "red" ), 1, 1, 0, 0 );
	add_room_type( "Undefined", get_color( "bright-red" ), 1, 1, 0, 0 );

	destroy_map( );

	/* Check if we have a binary map that is newer than the map and mapper. */

	if ( !stat( map_file_bin, &mapbin ) &&
			!stat( map_file, &map ) &&
			( !stat( "i_mapper.dll", &mapper ) ||
			  !stat( "i_mapper.so", &mapper ) ) &&
			mapbin.st_mtime > map.st_mtime &&
			mapbin.st_mtime > mapper.st_mtime )
		binary = 0;
	else
		binary = 0;

	get_timer( );

	if ( binary ? load_binary_map( map_file_bin ) : load_map( map_file ) )
	{
		destroy_map( );
		mode = NONE;
		return;
	}

	convert_vnum_exits( );
	check_map( );
	debugf( "%sIMap loaded. (%d microseconds)",
			binary ? "Binary " : "", get_timer( ) );

	/* Only if one already exists, but is too old. */
	if ( !binary && !stat( map_file_bin, &mapbin ) )
	{
		debugf( "Generating binary map." );
		save_binary_map( map_file_bin );
	}

	load_settings( "config.mapper.txt" );

	mode = GET_UNLOST;
}



/* Case insensitive versions of strstr and strcmp. */

#define LOW_CASE( a ) ( (a) >= 'A' && (a) <= 'Z' ? (a) - 'A' + 'a' : (a) )

int case_strstr( char *haystack, char *needle )
{
	char *h = haystack, *n = needle;

	while ( *h )
	{
		if ( LOW_CASE( *h ) == LOW_CASE( *(needle) ) )
		{
			n = needle;

			while( 1 )
			{
				if ( !(*n) )
					return 1;

				if ( LOW_CASE( *h ) != LOW_CASE( *n ) )
					break;
				h++, n++;
			}
		}

		h++;
	}

	return 0;
}


/* I don't trust strcasecmp to be too portable. */
int case_strcmp( const char *s1, const char *s2 )
{
	while ( *s1 && *s2 )
	{
		if ( LOW_CASE( *s1 ) != LOW_CASE( *s2 ) )
			break;

		s1++;
		s2++;
	}

	return ( *s1 || *s2 ) ? 1 : 0;
}


void remove_players( TIMER *self )
{
	ROOM_DATA *r;

	for ( r = world; r; r = r->next_in_world )
	{
		if ( r->person_here )
		{
			free( r->person_here );
			r->person_here = NULL;
		}
	}
}



void mark_player( ROOM_DATA *room, char *name )
{
	ROOM_DATA *r;

	if ( room->person_here )
	{
		free( room->person_here );
		room->person_here = NULL;
	}
	if ( name )
	{
		/* Make sure it's nowhere else. */
		for ( r = world; r; r = r->next_in_world )
			if ( r->person_here && !strcmp( r->person_here, name ) )
			{
				free( r->person_here );
				r->person_here = NULL;
			}

		room->person_here = strdup( name );
	}
	else
		room->person_here = strdup( "Someone" );

	add_timer( "remove_players", 8, remove_players, 0, 0, 0 );

	/* Force an update of the floating map. */
	last_room = NULL;
}



void locate_room( char *name, int area, char *player )
{
	ROOM_DATA *room, *found = NULL,*r;
	char buf[256],buf2[256];
	int more = 0,i;

	DEBUG( "locate_room" );

	strcpy( buf, name );
	if (areaonly) {
		for ( room = current_area->rooms; room; room = room->next_in_area ) {
			int a;
			for ( a = 1; a < 10; a++)
				if ( !strcmp( buf, room->name ) || (room->additional_name[a] != NULL && !strcmp(buf, room->additional_name[a] ) ) )
				{
					if ( !found ) {
						found = room;
						break;
					}
					else
					{
						more = 1;
						break;
					}
				}
		}
	} else if (areaandadj) {
		for ( room = current_area->rooms; room; room = room->next_in_area ) {
			{   int a;
				for ( a = 1; a < 10; a++)
					if ( !strcmp( buf, room->name ) || (room->additional_name[a] != NULL && !strcmp(buf, room->additional_name[a] ) ) )
					{
						if ( !found ) {
							found = room;
							break;}
						else
						{
							more = 1;
							break;
							break;
						}
					}
			}
		}
		if ( !found )
		for ( r = current_area->rooms; r; r = r->next_in_area ) {
			for ( i = 1; i < 14; i++ ) {
				if ( r->exits[i] &&  r->exits[i]->area->name != current_area->name )
				{
					for ( room = r->exits[i]->area->rooms; room; room = room->next_in_area ) {
						{   int a;
							for ( a = 1; a < 10; a++)
								if ( !strcmp( buf, room->name ) || (room->additional_name[a] != NULL && !strcmp(buf, room->additional_name[a] ) ) )
								{
									if ( !found ) {
										found = room;
										break;}
									else
									{
										more = 1;
										break;
										break;
										break;
									}
								}
						}
					} } } }
	} else
	{
		for ( room = world; room; room = room->next_in_world )
		{   int a;
			for ( a = 1; a < 10; a++)
				if ( !strcmp( buf, room->name ) || (room->additional_name[a] != NULL && !strcmp(buf, room->additional_name[a] ) ) )
				{
					if ( !found ) {
						found = room;
						break;}
					else
					{
						more = 1;
						break;
						break;
					}
				}
		}
	}
	if ( found )
	{
		/* Show the vnum, after the line. */
		if ( !disable_mxp_map && mxp_tag( TAG_LOCK_SECURE ) )
			clientff( " %s" C_D "(" C_G "<mpelm v=%d r=\"%s\" t=\"%s\">%d</mpelm>" C_D "<mpfind r=\"%s\">%s</mpfind>)" C_0,
					area == 3 ? "\r\n"C_0"" : "",found->vnum, found->name, found->room_type->name, found->vnum, found->name, more ? C_D "," C_G "..." C_D : ""); else {
				clientff( " %s" C_D "(" C_G "%d" C_D "%s)" C_0,
						area == 3 ? "\r\n"C_0"" : "",found->vnum, more ? C_D "," C_G "..." C_D : "" );}

		/* A farsight-like thing. */
		if ( area == 3  && found->area)
		{clientff(""C_0" Located within "C_R"(%s%s"C_R")%s%s%s%s"C_0,found->area->disabled ? C_Y"" : C_g"",found->area->name, found->worm_pointed_by ? C_D" (" C_G "Warp" C_D ")" : "", found->area->note ? C_c" ("C_W : "", found->area->note ? found->area->note : "", found->area->note ? C_c")" : "");
		}
		else if ( area == 2 && found->area ) {
			clientff( " " C_R "(%s%s" C_R ")%s%s%s%s" C_0 ,
					found->area->disabled ? C_Y"" : C_g"",found->area->name, found->worm_pointed_by ? C_D" (" C_G "Warp" C_D ")" : "", found->area->note ? C_c" ("C_W : "", found->area->note ? found->area->note : "", found->area->note ? C_c")" : "");
		}
		else if ( area == 1 && found->area )
		{
			clientff( "\r\nFrom your knowledge, that room is in "C_R"(%s%s"C_R")%s%s%s"C_0"",
					found->area->disabled ? C_Y"" : C_g"",found->area->name, found->area->note ? C_c" ("C_W : "", found->area->note ? found->area->note : "", found->area->note ? C_c")" : "" );
		}
		else if ( area == 0 && found->area && found->area->note )
		{clientff(C_c" ("C_W"%s"C_c")"C_0, found->area->note );
		}
		/* Mark the room on the map, for a few moments. */
		if ( !more )
		{
			if ( player != NULL ) {
				mark_player( found, player );}
			if ( !disable_farsightpath && farsight && current_room->vnum != found->vnum )
			{
				/* work from here, finish writing in saving/loading of the disable_farsightpath variable, adding to help and such */
				char temp[256];
				clientff("\r\n"C_0);
				sprintf(temp, "%d",found->vnum);
				do_map_path( temp );
				farsight = 0;
			}
			if ( sldest )
			{ sldest = 0;
				sprintf(buf2,"%d hide destroy",found->vnum);
				do_map_shrine( buf2 );
			} else if ( ssight ) {ssight = 0;
				if ( strcmp(ssightdiv, "unset") )
				{
					sprintf(buf2, "%d hide update %s %s %s friendly", found->vnum, ssightdiv, ssize, smajor);
					do_map_shrine( buf2 );
				} else {clientff(" "C_W"("C_G"NS"C_W")"C_0);}
			}
		}
	}

}


void locate_roompart( char *name, int area, char *player, AREA_DATA *sarea )
{
	ROOM_DATA *room, *found = NULL;
	char buf[256],buf2[256];
	int more = 0;

	DEBUG( "locate_room" );
	vnumfound = 0;
	strcpy( buf, name );
	if (areaonly) {
		for ( room = current_area->rooms; room; room = room->next_in_area ) {
			{   int a;
				for ( a = 1; a < 10; a++)
					if ( !strncmp( buf, room->name,strlen(buf) ) || (room->additional_name[a] != NULL && !strncmp(buf, room->additional_name[a],strlen(buf) ) ) )
					{
						if ( !found ) {
							found = room;
							break;}
						else
						{
							more = 1;
							break;
							break;
						}
					}
			}
		} } else if ( sarea != NULL ) {
			for ( room = sarea->rooms; room; room = room->next_in_area )
			{   int a;
				for ( a = 1; a < 10; a++)
					if ( !strncmp( buf, room->name, strlen(buf) ) || (room->additional_name[a] != NULL && !strncmp(buf, room->additional_name[a],strlen(buf) ) ) )
					{
						if ( !found ) {
							found = room;
							break;}
						else
						{
							more = 1;
							break;
							break;
						}
					}
			}
		}
	else {
		for ( room = world; room; room = room->next_in_world )
		{   int a;
			for ( a = 1; a < 10; a++)
				if ( !strncmp( buf, room->name, strlen(buf) ) || (room->additional_name[a] != NULL && !strncmp(buf, room->additional_name[a],strlen(buf) ) ) )
				{
					if ( !found ) {
						found = room;
						break;}
					else
					{
						more = 1;
						break;
						break;
					}
				}
		}
	}
	if ( found )
	{
		/* Show the vnum, after the line. */
		if ( !disable_mxp_map && mxp_tag( TAG_LOCK_SECURE ) )
			clientff( " %s" C_D "(" C_G "<mpelm v=%d r=\"%s\" t=\"%s\">%d</mpelm>" C_D "<mpfind r=\"%s\">%s</mpfind>)" C_0,
					area == 3 ? "\r\n"C_0"" : "",found->vnum, found->name, found->room_type->name, found->vnum, found->name, more ? C_D "," C_G "..." C_D : ""); else {
				clientff( " %s" C_D "(" C_G "%d" C_D "%s)" C_0,
						area == 3 ? "\r\n"C_0"" : "",found->vnum, more ? C_D "," C_G "..." C_D : "" );}

		/* A farsight-like thing. */
		if ( area == 3  && found->area)
		{clientff(""C_0" Located within "C_R"(%s%s"C_R")%s%s%s%s"C_0,found->area->disabled ? C_Y"" : C_g"",found->area->name, found->worm_pointed_by ? C_D" (" C_G "Warp" C_D ")" : "", found->area->note ? C_c" ("C_W : "", found->area->note ? found->area->note : "", found->area->note ? C_c")" : "");
		}
		else if ( area == 2 && found->area ) {
			clientff( " " C_R "(%s%s" C_R ")%s%s%s%s" C_0 ,
					found->area->disabled ? C_Y"" : C_g"",found->area->name, found->worm_pointed_by ? C_D" (" C_G "Warp" C_D ")" : "", found->area->note ? C_c" ("C_W : "", found->area->note ? found->area->note : "", found->area->note ? C_c")" : "");
		}
		else if ( area == 1 && found->area )
		{
			clientff( "\r\nFrom your knowledge, that room is in "C_R"(%s%s"C_R")%s%s%s"C_0"",
					found->area->disabled ? C_Y"" : C_g"",found->area->name, found->area->note ? C_c" ("C_W : "", found->area->note ? found->area->note : "", found->area->note ? C_c")" : "");
		}
		else if ( area == 0 && found->area && found->area->note )
		{clientff(C_c" ("C_W"%s"C_c")"C_0, found->area->note );
		}
		/* Mark the room on the map, for a few moments. */
		if ( !more )
		{
			vnumfound = found->vnum;
			if ( player != NULL )
				mark_player( found, player );
			if ( !disable_farsightpath && farsight && current_room->vnum != found->vnum )
			{
				/* work from here, finish writing in saving/loading of the disable_farsightpath variable, adding to help and such */
				char temp[256];
				clientff("\r\n"C_0);
				sprintf(temp, "%d",found->vnum);
				do_map_path( temp );
				farsight = 0;
			}
			if ( sldest )
			{ sldest = 0;
				sprintf(buf2,"%d hide destroy",found->vnum);
				do_map_shrine( buf2 );
			} else if ( ssight ) {ssight = 0;
				if ( strcmp(ssightdiv, "unset") )
				{
					sprintf(buf2, "%d hide update %s %s %s friendly", found->vnum, ssightdiv, ssize, smajor);
					do_map_shrine( buf2 );
				} else {clientff(" "C_W"("C_G"NS"C_W")"C_0);}
			}
		}
	}
}


void locate_room_in_area( char *name, char *player,int nl, AREA_DATA *sarea )
{
	ROOM_DATA *room, *found = NULL;
	char buf[256];
	int more = 0;

	DEBUG( "locate_room_in_area" );

	strcpy( buf, name );
	for ( room = sarea->rooms; room; room = room->next_in_area ) {
		{   int a;
			for ( a = 1; a < 10; a++)
				if ( !strcmp( buf, room->name ) || (room->additional_name[a] != NULL && !strcmp(buf, room->additional_name[a] ) ) )
				{
					if ( !found ) {
						found = room;
						break;}
					else
					{
						more = 1;
						break;
						break;
					}
				}
		}
	}
	if ( found )
	{
		/* Show the vnum, after the line. */
		if ( !disable_mxp_map && mxp_tag( TAG_LOCK_SECURE ) )
			clientff( " %s" C_D "(" C_G "<mpelm v=%d r=\"%s\" t=\"%s\">%d</mpelm>" C_D "<mpfind r=\"%s\">%s</mpfind>)" C_0,
					nl == 1 ? "\r\n"C_0"" : "",found->vnum, found->name, found->room_type->name, found->vnum, found->name ,more ? C_D "," C_G "..." C_D : ""); else {
				clientff( " %s" C_D "(" C_G "%d" C_D "%s)" C_0,
						nl == 1 ? "\r\n"C_0"" : "",found->vnum, more ? C_D "," C_G "..." C_D : "" );}

		if ( nl == 1  && found->area)
		{clientff(""C_0" Located within "C_R"(%s%s"C_R")%s%s%s%s"C_0,found->area->disabled ? C_Y"" : C_g"",found->area->name, found->worm_pointed_by ? C_D" (" C_G "Warp" C_D ")" : "", found->area->note ? C_c" ("C_W : "", found->area->note ? found->area->note : "", found->area->note ? C_c")" : "");
		}
		else if ( nl == 0 && found->area && found->area->note )
		{clientff(C_c" ("C_W"%s"C_c")"C_0, found->area->note );
		}
		/* Mark the room on the map, for a few moments. */
		if ( !more )
		{
			if ( current_room && !disable_farsightpath && !found->area->disabled && farsight && current_room->vnum != found->vnum )
			{
				/* work from here, finish writing in saving/loading of the disable_farsightpath variable, adding to help and such */
				char temp[256];
				clientff("\r\n"C_0);
				sprintf(temp, "%d",found->vnum);
				do_map_path( temp );
				farsight = 0;
			}
			mark_player( found, player );
		}
	}

}


void parse_msense( char *line )
{
	char *end;
	char buf[256];

	DEBUG( "parse_msense" );

	if ( strncmp( line, "An image of ", 12 ) )
		return;

	line += 12;

	if ( !( end = strstr( line, " appears in your mind." ) ) )
		return;

	if ( end < line )
		return;

	strncpy( buf, line, end - line );
	buf[end-line] = '.';
	buf[end-line+1] = 0;

	/* We now have the room name in 'buf'. */
	locate_room( buf, 1, NULL );
}

void parse_eldsense( char *line )
{
	/* You detect a crudely-formed silver eld#252960 at Pathway in primordial forest. (SHACKLED) */
	char roomname[256];
	char *buf = NULL;

	DEBUG( "parse_eldsense" );

	// Check if this is the line we want
	if (strncmp(line, "You detect a", 12)) {
		return;
	}

	// Get the room name.
	line = strstr(line, " at ");
	if(!line) {
		return;
	}

	line += 4;

	// Two pointers at same point
	buf = line;

	// Move line pointer to end of room name, after the '.' char
	line = strchr(line, '.') + 1;

	// Copy string between pointer to name.
	strncpy(roomname, buf, line-buf);
	roomname[1 + line-buf] = '\0'; // strncpy doesn't append a \0 char.

	areaonly = 1;
	locate_room(roomname, 2, NULL);
}

void parse_auraglance( char *line)
{
    char name[256];
    char *buf = NULL,line2[256],buf2[256];
	AREA_DATA *farea = NULL;
    /*You detect player's presence at Four Corners on Nordau Street in Enorian.*/
    if (cmp("You detect ^'s presence at *",line))
    return;

    get_string(line + 11, name, 256);

    line = strstr(line, " presence at ");
    if (!line)
       return;

    line += 13;

    buf = line;
	while ( buf && strstr(buf, " in ") )
	{
		buf += 4;
		if ( strstr(buf, " in ") )
			buf = strstr(buf, " in ");
	}
	strncpy( line2, line, buf - line );
	line2[buf-line-4] = '.';
	line2[buf-line-3] = 0;
	if ( !strncmp( buf, "the ", 4 ) ) {
		buf += 4;
	}
	sprintf(buf2, "donotshow %s",buf );
	farea = get_area_by_name( buf2 );
	if ( farea && farea->name )
	{
		locate_room_in_area( line2, name,0, farea );
	}
}

void parse_window( char *line )
{
	char name[256];
	char *buf = NULL,line2[256],buf2[256];
	AREA_DATA *farea = NULL;

	DEBUG( "parse_window" );

	if ( strncmp( line, "You see that ", 13 ) )
		return;

	get_string( line + 13, name, 256 );

	line = strstr( line, " is at " );

	if ( !line )
		return;

	line += 7;

	buf = line;
	while ( buf && strstr(buf, " in ") )
	{
		buf += 4;
		if ( strstr(buf, " in ") )
			buf = strstr(buf, " in ");
	}
	strncpy( line2, line, buf - line );
	line2[buf-line-4] = '.';
	line2[buf-line-3] = 0;
	if ( !strncmp( buf, "an unstable section of ", 23 ) ) {
		buf += 23;
	}
	if ( !strncmp( buf, "the ", 4 ) ) {
		buf += 4;
	}
	sprintf(buf2, "donotshow %s",buf );
	farea = get_area_by_name( buf2 );
	if ( farea && farea->name )
	{
		locate_room_in_area( line2, name,0, farea );
	} else
	{
		locate_room( line, 2, name );
	}
}


void parse_cgc( char *line )
{
    if (!strncmp(line, "(",1) && strstr(line, "):") && strstr(line, "ocation: ") )
    {
    char buf[256],*buf2 = NULL,line2[256],buf3[256];
	AREA_DATA *farea = NULL;
    memset( buf, '\0', sizeof(buf) );
    memset( line2, '\0', sizeof(line2) );
    memset( buf3, '\0', sizeof(buf3) );
    line = strstr(line, "ocation: ");
    line += 9;
    strncpy(buf, line,strlen(line)-1);

	buf2 = buf;
	while ( buf2 && strstr(buf2, " in ") )
	{
		buf2 += 4;
		if ( strstr(buf2, " in ") )
			buf2 = strstr(buf2, " in ");
	}
	strncpy( line2, buf, buf2 - buf );
	line2[buf2-buf-4] = '.';
	line2[buf2-buf-3] = 0;
	if ( !strncmp( buf2, "the ", 4 ) ) {
		buf2 += 4;
	}
	sprintf(buf3, "donotshow %s",buf2 );
	farea = get_area_by_name( buf3 );
	if ( farea && farea->name )
	{
		locate_room_in_area( line2, NULL ,0, farea );
	}
	else
	{
		locate_room( buf, 2, NULL );
	}
    }
}


void parse_scent( char *line )
{
	if ( !strncmp( line, "You detect traces of scent from ", 32 ) )
	{
		areaonly = 1;
		if ( strstr(line, "above")) {
			line = strstr( line, "above" );
			locate_room( line + 6, 2, NULL );
		} else {
			locate_room( line + 32, 2, NULL );}
	}
}



void parse_allysense( char *line )
{
	if ( !fulllineok )
		return;
	line = fullline;
	if ( !strncmp( line, "Your ally has fallen at ", 24 ) )
	{
		locate_room( line + 24, 3, NULL );
	}
	if ( !strncmp( line, "You divine the location of this death as ",41 ) )
	{
		char *buf = NULL,line2[256],buf2[256];
		AREA_DATA *farea = NULL;
		line += 41;
		buf = line;
		while ( buf && strstr(buf, " in ") )
		{
			buf += 4;
			if ( strstr(buf, " in ") )
				buf = strstr(buf, " in ");
		}
		strncpy( line2, line, buf - line );
		line2[buf-line-4] = '.';
		line2[buf-line-3] = 0;
		if ( !strncmp( buf, "an unstable section of ", 23 ) ) {
			buf += 23;
		}
		if ( !strncmp( buf, "the ", 4 ) ) {
			buf += 4;
		}
		sprintf(buf2, "donotshow %s",buf );
		farea = get_area_by_name( buf2 );
		if ( farea && farea->name )
		{
			locate_room_in_area( line2, NULL,1, farea );
		}
	}
}


void parse_alarm( char *line )
{
	char *end;
	char buf[256];

	DEBUG( "parse_alarm" );

	if ( strncmp( line, "Your alarm at '", 15 ) )
		return;

	line += 15;

	if ( !( end = strstr( line, " (" ) ) )
		return;

	if ( end < line )
		return;

	strncpy( buf, line, end - line );
	buf[end-line] = '.';
	buf[end-line+1] = 0;
	/* We now have the room name in 'buf'. */
	locate_room( buf, 1, NULL );
}

void parse_angelrite(char *li)
{
	if ( !fulllineok )
		return;
	char *line = fullline,buf[256],*end;
	if ( cmp("Your angel senses *.",line) && cmp("Your guardian reports that *.",line) && cmp("Allsight: *.",line) )
		return;

	if ( !cmp("Your angel senses *.",line) ) {
		if ( ( line = strstr(line, " at ") ) )
		{line += 4;
			if ( !( end = strstr(line, ", on a health ") ) )
				end = strstr(line, " on a health");
			strncpy( buf, line, end - line );
			buf[end-line] = '.';
			buf[end-line+1] = 0;
			locate_room(buf, 2, NULL);
		}
	}
	else if ( !cmp("Your guardian reports that *.",line) )
	{
		if ( ( line = strstr(line," has moved to ") ) ) {
			line += 14;
			locate_room(line, 2, NULL);}
	}
	else if ( !cmp("Allsight: *.",line) )
	{
		if ( strstr(line," leaves ") && ( line = strstr(line," leaves ") ) ) {
			line += 8;
			locate_room(line, 2, NULL);}
		else if ( strstr(line," enters ") && ( line = strstr(line," enters ") ) ) {
			line += 8;
			locate_room(line, 2, NULL);}
	}
}

void parse_wormholes( char *line )
{
	if ( !strncmp( line, "You sense a wormhole leading to ", 32 ) )
	{
		locate_room( line + 32, 2, NULL );
	}

}



void parse_scry( char *line )
{
	char buf[256];
	char name[256];

	DEBUG( "parse_scry" );

	int pool = strncmp( line, "You create a pool of water in the air in front of you, and look through it,", 75 );
	int mirror = strncmp( line, "You create a shimmering mirror in the air in front of you, and look through it,", 79 );
	if ( !sense_message && ( pool || mirror) ) {
		return;
	}

	if ( !sense_message )
	{
		if ( !mirror ) {
			line += 79;
		}
		else if ( !pool ) {
			line += 75;
		}

		line = get_string( line, buf, 256 );
		if ( !strncmp(buf, "sensing", 7) ) {
			line = get_string( line, name, 256 );
			line = get_string( line, buf, 256 );
			locate_room( line, 1, name );
		} else {

			sense_message = 1;
		}
	}

	if ( sense_message == 1 )
	{
		/* Next line: "sensing Whyte at Antioch Runners tent." */
		/* Skip the first three words. */
		line = get_string( line, buf, 256 );
		line = get_string( line, name, 256 );
		line = get_string( line, buf, 256 );

		locate_room( line, 1, name );
	}
}




void parse_ka( char *line )
{
	static char buf[1024];
	static int periods;
	int i;

	DEBUG( "parse_ka" );

	/* A shimmering image of *name* appears before you. She/he is at *room*.*/

	if ( !sense_message )
	{
		if ( !strncmp( line, "A shimmering image of ", 22 ) )
		{
			buf[0] = 0;
			sense_message = 2;
			periods = 0;
		}
	}

	if ( sense_message == 2 )
	{
		for ( i = 0; line[i]; i++ )
			if ( line[i] == '.' )
				periods++;

		strcat( buf, line );

		if ( periods == 2 )
		{
			sense_message = 0;

			if ( ( line = strstr( buf, " is at " ) ) )
				locate_room( line + 7, 1, NULL );
			else if ( ( line = strstr( buf, " is located at " ) ) )
				locate_room( line + 15, 1, NULL );
		}
	}
}



void parse_seek( char *line )
{
	static char buf[1024];
	static int periods;
	char *end;
	int i;

	DEBUG( "parse_seek" );

	/* A shimmering image of *name* appears before you. She/he is at *room*.*/

	if ( !sense_message )
	{
		if ( !strncmp( line, "You bid your guardian to seek out the lifeforce of ", 51 ) )
		{
			buf[0] = 0;
			sense_message = 3;
			periods = 0;
		}
	}

	if ( sense_message == 3 )
	{
		for ( i = 0; line[i]; i++ )
			if ( line[i] == '.' )
				periods++;

		strcat( buf, line );

		if ( periods == 3 )
		{
			sense_message = 0;

			if ( ( line = strstr( buf, " at " ) ) )
			{
				line += 4;

				end = strchr( line, ',' );
				if ( !end )
					return;

				strncpy( buf, line, end - line );
				buf[end-line] = '.';
				buf[end-line+1] = 0;

				locate_room( buf, 1, NULL );
			}
		}
	}
}



void parse_shrinesight( char *line )
{
	char *p, *end;
	AREA_DATA *sarea = NULL;
	char buf[256],buf2[256];
	int c,e;

	DEBUG( "parse_shrinesight" );

	if ( strncmp(line, "Dor *  ",7) && strncmp(line, "Sml *  ",7) && strncmp(line, "Med *  ",7) && strncmp(line, "Lge *  ",7) && strncmp(line, "Dor    ",7) && strncmp(line, "Sml    ",7) && strncmp(line, "Med    ",7) && strncmp(line, "Lge    ",7) && cmp("^ has entered the influence of the shrine at *", line)
			&& cmp("^/^/^ ^:^:^ - A shrine at '*' was destroyed.",line) && cmp("^/^/^ ^:^:^ - ^ erected a shrine at *",line) && cmp("The Master Shrine is located at *",line) && cmp("The Master Shrine is located at *",fullline) && cmp("^/^/^ ^:^:^ - A shrine at '*' was deconstructed by ^.",line) )
		return;

	if ( !cmp("^ has entered the influence of the shrine at *", line) )
	{
		char *ttmp;
		ttmp = strstr(line, "entered the influence of the shrine ");
		strcpy(buf,ttmp + 39);
		buf[strlen(buf)] = '.';
	}
	else if ( !cmp("^/^/^ ^:^:^ - ^ erected a shrine at *",line) )
	{
		p = strstr(line, " shrine at '");
		p += 12;
		strncpy(buf, p, strlen(p));
		buf[strlen(buf)-2] = '.';
		buf[strlen(buf)-1] = 0;
	}
	else if ( !cmp("^/^/^ ^:^:^ - A shrine at '*' was destroyed.",line) )
	{
		p = strstr(line, "A shrine at '");
		p += 13;
		end = strrchr( p, 39 );
		if ( !end || p >= end )
			return;
		strncpy( buf, p, end - p );
		buf[strlen(buf)] = '.';
		if ( mode == CREATING )
			sldest = 1;
	}
	else if ( !cmp("^/^/^ ^:^:^ - A shrine at '*' was deconstructed by ^.",line) )
	{
		p = strstr(line, "A shrine at '");
		p += 13;
		end = strrchr( p, 39 );
		if ( !end || p >= end )
			return;
		strncpy( buf, p, end - p );
		buf[strlen(buf)] = '.';
		if ( mode == CREATING )
			sldest = 1;
	}
	else if ( !cmp("The Master Shrine is located at *",line) || ( fulllineok && !cmp("The Master Shrine is located at *",fullline) ) )
	{
		if ( !fulllineok )
			return;
		line = fullline;
		char *buf1 = NULL,buf2[256];
		line += 32;
		buf1 = line;
		memset( smajor, '\0', sizeof(smajor) );
		sprintf(smajor,"Master");
		sprintf(ssize,"Large");
		while ( buf1 && strstr(buf1, " in ") )
		{
			buf1 += 4;
			if ( strstr(buf1, " in ") )
				buf1 = strstr(buf1, " in ");
		}
		strncpy( buf, line, buf1 - line );
		buf[buf1-line-4] = '.';
		buf[buf1-line-3] = 0;
		if ( !strncmp( buf1, "the ", 4 ) )
			buf1 += 4;
		sprintf(buf2, "donotshow %s",buf1 );
		sarea = get_area_by_name( buf2 );
		if ( mode == CREATING )
			ssight = 1;
	}
	else if ( !strncmp(line, "Dor *  ",7) || !strncmp(line, "Sml *  ",7) || !strncmp(line, "Med *  ",7) || !strncmp(line, "Lge *  ",7) || !strncmp(line, "Dor    ",7) || !strncmp(line, "Sml    ",7) || !strncmp(line, "Med    ",7) || !strncmp(line, "Lge    ",7) ) {
		memset( ssize, '\0', sizeof(ssize) );
		memset( smajor, '\0', sizeof(smajor) );
		get_string(line , buf2, 128);
		get_string(line + 7, smajor, 128);

		if (!strncmp(buf2,"Dor",3))
			strncpy(ssize,"small",5);
		else if (!strncmp(buf2,"Sml",3))
			strncpy(ssize,"small",5);
		else if (!strncmp(buf2,"Med",3))
			strncpy(ssize,"medium",6);
		else
			strncpy(ssize,"large",5);

		p = line + 18;
		if ( !p )
			return;
		e = 47;
		end = line + e;

		while ( !isalnum(end[0]) ) {e--;
			end = line + e;}

		if ( p >= end )
			return;
		strncpy( buf, p, end - p );
		buf[end-p] = 0;

		if ( mode == CREATING )
			ssight = 1;
		c = 49;
		if ( !strncmp( line + c, "the ", 4 ) )
			c = 53;
		sprintf(buf2, "donotshow %s",line + c);
		sarea = get_area_by_name( buf2 );
	}
	/* We now have the room name in 'buf'. */
	if ( buf[0] )
		locate_roompart( buf , 0, NULL, sarea );
}



void parse_entities( char *line )
{
	char *p;
	char buf[256];

	if ( strstr(line,  "[ Chaos Entities ]" )
			|| !strcmp( line, "You detect the following people disturbing the tranquility of the forest." ) )
	{
		if ( strstr(line, "[ Chaos Entities ]" ) )
			scout_list = 2;
		else
			scout_list = 1;
		areaonly = 1;
		return;
	}

	if ( !scout_list )
		return;

	if ( scout_list == 2 )
		p = line + 40;
	else
		p = strstr( line, " is at " );
	if ( !p )
		return;

	if ( scout_list == 2 ) {
		strcpy(buf, p);
		buf[strlen(buf)] = '.';}
	else
		strcpy( buf, p + 7 );

	locate_room( buf, 2, NULL );
}

void parse_traplist( char *line )
{
	char *p;
	char buf[256];

	if ( !strncmp( line, "Listing traps for:", 18 ) )
	{
		trap_list = 1;
		return;
	}

	if ( !trap_list )
		return;

	p = strstr( line, ") at " );
	if ( !p )
		return;

	strcpy( buf, p + 5 );
	strcat( buf, ".");
	locate_room( buf, 2, NULL );
}

void parse_fullsense( char *line )
{
	char *p;
	char buf[256];
	char name[256];
	char room[256];

	DEBUG( "parse_fullsense" );

	if ( strncmp( line, "You sense ", 10 ) || strlen( line ) > 128 )
		return;

	strcpy( buf, line );

	p = strstr( buf, " at " );
	if ( !p )
		return;
	*p = 0;
	p += 4;

	strcpy( name, buf + 10 );
	strcpy( room, p );

	/* We now have the room name in 'buf'. */
	locate_room( room, 0, name );
}

void parse_leylines( char *line )
{
	if ( !cmp( "You reach a hand outwards and close your eyes, seeking for the ethereal energies of the leylines around you.",line) ) {
		leyline_message = 1;
		return;}
	if (!leyline_message)
		return;
	if (strstr(line," lesser foci:")) {
		areaonly=1;}
	if (!strncmp(line," - ",3)) {
		char buf[256];
		strncpy(buf,line+4,strlen(line));
		buf[strlen(buf)] = '.';
		buf[strlen(buf)+1] = 0;
		locate_room(buf,2,NULL);}
}

void parse_view( char *line )
{
	char name[256], buf[256];

	DEBUG( "parse_view" );

	/* You see Name at Location. */

	if ( cmp( "You see *", line ) )
		return;

	line = get_string( line + 8, name, 256 );
	line = get_string( line, buf, 256 );

	if ( cmp( "at", buf ) )
		return;

	if ( strstr(line, "In the trees above") || strstr(line, "Flying above")) {
		line = strstr( line, "above" );
		line = get_string(line, buf, 256 );
	}

	locate_room( line, 2, name );
}

void parse_watch( char *line )
{
	if ( strstr(line, "has entered the forest at") == NULL && strstr(line, "leaves the forest from") == NULL )
		return;

	if ( strstr(line, "has entered the forest at") )
	{
		line = strstr(line, "has entered the forest at");
		line += 26;
		locate_room( line, 1, "");
	} else if ( strstr(line, "leaves the forest from" ) )
	{
		line = strstr(line, "leaves the forest from" );
		line += 23;
		locate_room( line, 1, "");
	}
}

void parse_wind( char *line )
{
	if ( cmp("The wind catches you and blows you *",line) )
		return;

	char buf[256],winddir[256];
	int i = 0;
	line += 34;
	get_string( line, buf, 256);
	strncpy(winddir, buf, strlen(buf)-1);
	for ( i = 1; dir_name[i]; i++ )
	{
		if ( !strcmp( winddir, dir_name[i] ) )
		{
			if ( mode == CREATING || mode == FOLLOWING ) {
				add_queue( i );clientff(" "C_g"("C_Y"Blown %s"C_g")"C_0"",winddir);}
		}
	}

}

void parse_mindthrow( char *line )
{
	if ( cmp("You are jolted violently ^ by powers unseen.",line) )
		return;

	char buf[256],tdir[256];
	int i = 0;
	line += 25;
	get_string(line,buf,256);
	strncpy(tdir,buf,strlen(buf)-5);
	for ( i = 1; dir_name[i]; i++ )
	{
		if ( !strcmp( tdir, dir_name[i] ) )
		{
			if ( mode == CREATING || mode == FOLLOWING ) {
				add_queue( i );clientff(" "C_g"("C_Y"Mindthrown %s"C_g")"C_0"",tdir);}
		}
	}
}

void parse_werescent( LINE *l )
{
	char name[256], buf[256];
	char *line = l->line;
	DEBUG( "parse_werescent" );

	if (!strncmp(line, "Turning your snout to the wind, you deeply inhale any scents from the air.", 74) || !strncmp(line, "You tilt your head back and deeply inhale any scents from the air.",66) ) {
		werescent = 1;
		return;}
	if ( strstr(line, "Balance Used") )
		werescent = 0;
	if ( !werescent )
		return;
	l->gag_entirely = 1;

	if ( strchr(line, 46) && werescent == 2) {
		strncat(werescentmsg, line, strlen(werescentmsg)+strlen(line));
	}
	if ( !strchr(line ,46) && werescent == 1 ) {
		memset( werescentmsg, '\0', sizeof(werescentmsg) );
		strncpy(werescentmsg, line, strlen(line));
		l->gag_ending = 1;
		werescent = 2;
		return;}
	/* You see Name at Location. */
	/*   if ( cmp( "You pick up the faint scent of *", line ) )
		 return; */
	if ( werescent == 2 ) {
		memset(line, '\0', sizeof(line));
		strcpy(line, werescentmsg);
		werescent = 1;
	}
	if ( cmp( "You pick up the faint scent of *", line ) && cmp("The unique scent of *", line) )
		return;

	if ( !cmp("You pick up the faint scent of *", line) ) {
		line = get_string( line + 31, name, 256 );
		line = get_string( line, buf, 256 );
	}
	if ( !cmp("The unique scent of *", line) )
	{
		line = get_string( line + 20, name, 256 );
		line = get_string( line + 13, buf, 256);
	}
	if ( cmp( "at", buf ) && cmp( "from", buf ) )
		return;

	memset( werescentmsg, '\0', sizeof(werescentmsg) );
	if ( !cmp( "from", buf ) )
	{clientff(C_0"%15.15s located at %s", name, line);
		areaonly=1;}
	else
	{clientff(C_0"%15.15s is at %s", name, line);
		areaandadj=1;}
	locate_room( line, 2, name );
}

void parse_werepack( char *line )
{
	if ( cmp("Pack Member *",line) && !werepack )
		return;
	if ( !cmp("Pack Member *",line) )
	{werepack=1;
		return;}
	if ( strstr(line, "---") )
		return;
	char name[256],buf[256];
	get_string(line,name,256);
	line += 20;
	strncpy(buf,line,strlen(line));
	buf[strlen(buf)] = '.';
	buf[strlen(buf)+1] = 0;
	locate_room(buf, 2, name);
}

void parse_nointeract( char *line )
{
	if ( strcmp(line, "A short burst of azure light fills your vision and when it is gone, you find yourself phased out of sync with the rest of reality.") &&
			strcmp(line, "Your surroundings shatter into a cloud of glowing stars which dissipate to leave you back where you began.") &&
			strcmp(line,"You are suddenly and unexpectedly pulled back into phase with reality.") &&
			strcmp(line, "There's a flash of light and you're pulled back into phase with reality.") &&
			strcmp(line, "You call upon your dark power, and instantly a black wind descends upon you. In seconds your body begins to dissipate, and you are one with the odious vapour.") &&
			strncmp(line,"You concentrate and are once again ",35) && strcmp(line, "You are pulled out of your black wind and back into corporeality.")
	   )
		return;

	if ( !strcmp(line, "A short burst of azure light fills your vision and when it is gone, you find yourself phased out of sync with the rest of reality.") ) {
		nodoors = 1;}

	if ( !strcmp(line, "Your surroundings shatter into a cloud of glowing stars which dissipate to leave you back where you began.") ||
			!strcmp(line,"You are suddenly and unexpectedly pulled back into phase with reality.") ||
			!strcmp(line, "There's a flash of light and you're pulled back into phase with reality.") )
	{
		nodoors = 0;}

	if ( !strcmp(line, "You call upon your dark power, and instantly a black wind descends upon you. In seconds your body begins to dissipate, and you are one with the odious vapour.") ) {
		nointeract = 1;
		nodoors = 1;}

	if ( !strncmp(line,"You concentrate and are once again ",35) || !strcmp(line, "You are pulled out of your black wind and back into corporeality.") ) {
		nointeract = 0;
		nodoors = 0;}
}

void parse_pursue( char *line )
{
	char buf[512], buf2[512], name[512];
	char *p;
	static int pursue_message;

	DEBUG( "parse_pursue" );

	if ( !cmp( "You sense that ^ has entered *", line ) )
	{
		pursue_message = 1;
		get_string( line + 15, name, 512 );
		buf[0] = 0;
	}

	if ( !pursue_message )
		return;

	/* Add a space to the last line, in case there wasn't one. */
	if ( buf[0] && buf[strlen(buf)-1] != ' ' )
		strcat( buf, " " );

	strcat( buf, line );

	if ( strstr( buf, "." ) )
	{
		pursue_message = 0;

		p = strstr( buf, " has entered " );

		if ( !p )
			return;

		strcpy( buf2, p + 13 );

		p = buf2 + strlen( buf2 );

		while ( p > buf2 )
		{
			if ( *p == ',' )
			{
				*(p++) = '.';
				*p = 0;
				break;
			}
			p--;
		}

		locate_room( buf2, 1, name );
	}
}

void parse_sources( char *line )
{   /*
	   You close your eyes and seek out all wormhole sources.
	   +-------------------------------------+-------------------------------------+
	   | From                                | To                                  |
	   +-------------------------------------+-------------------------------------+
	   | Northern edge of the Dolbodi Campsi | Under a gnarled oak                 |
	   +-------------------------------------+-------------------------------------+ */
	if ( !wormsources && cmp("You close your eyes and seek out all wormhole sources.",line) )
		return;

	if (!cmp("You close your eyes and seek out all wormhole sources.",line) ) {
		wormsources = 1;
		return;}

	if ( !wormsources || !strcmp(line,"+-------------------------------------+-------------------------------------+") || strlen(line)<75 || !strcmp(line,"| From                                | To                                  |") )
		return;

	char line2[256],*end = NULL,line3[256],buf[256];
	int e,vnum1,vnum2;
	memset( line2, '\0', sizeof(line2) );
	memset( line3, '\0', sizeof(line3) );
	line += 2;
	e = 35;
	end = line+e;
	while ( !isalnum(end[-1]) ) {e--;
		end = line + e;}
	strncpy(line2, line, end - line);
	vnumfound = 0;
	locate_roompart(line2, 0, NULL, NULL);
	vnum1 = vnumfound;
	clientf(" ->");
	line += 38;
	e = 35;
	end = line + 35;
	while ( !isalnum(end[-1]) ) {e--;
		end = line + e;}
	strncpy(line3,line,end - line);
	vnumfound = 0;
	locate_roompart(line3, 2, NULL, NULL);
	vnum2 = vnumfound;
	if ( mode == CREATING && vnum1 && vnum1 != 0 && vnum2 && vnum2 != 0 && vnum1 != vnum2 ) {
		ROOM_DATA *check;
		sprintf(buf, "%d from %d", vnum2, vnum1);
		check = get_room(vnum1);
		if ( !check->worm_pointed_by ) {
			sprintf(buf, "%d from %d", vnum2, vnum1);
			void do_exit_warp( char *arg );
			do_exit_warp( buf );}}
}

void parse_alertness( LINE *l )
{
	static char buf[512];
	static int alertness_message;
	char buf2[512];
	char *line = l->line;

	DEBUG( "parse_alertness" );

	if ( disable_alertness )
		return;

	if ( !cmp( "Your enhanced senses inform you that *", line ) )
	{
		buf[0] = 0;
		alertness_message = 1;
	}

	if ( !alertness_message )
		return;

	l->gag_entirely = 1;
	l->gag_ending = 1;

	/* In case something goes wrong. */
	if ( alertness_message++ > 3 )
	{
		alertness_message = 0;
		return;
	}

	if ( buf[0] && buf[strlen(buf)-1] != ' ' )
		strcat( buf, " " );

	strcat( buf, line );

	if ( strstr( buf, "nearby." ) )
	{
		/* We now have the full message in 'buf'. Parse it. */
		char player[256];
		char room_name[256];
		char *p, *p2;
		int found = 0;
		int i;

		alertness_message = 0;

		/* Your enhanced senses inform you that <player> has entered <room> nearby. */

		p = strstr( buf, " has entered" );
		if ( !p )
			return;
		*p = 0;

		strcpy( player, buf + 37 );

		p2 = strstr( p + 1, " nearby" );
		if ( !p2 )
			return;
		*p2 = 0;

		strcpy( room_name, p + 13 );
		strcat( room_name, "." );

		alertness_message = 0;

		sprintf( buf2, C_R "[" C_W "%s" C_R " - ", player );

		/* Find where that room is. */

		if ( current_room )
		{
			if ( !room_cmp( current_room->name, room_name ) )
			{
				strcat( buf2, "here" );
				found = 1;
			}

			for ( i = 1; dir_name[i]; i++ )
			{
				if ( !current_room->exits[i] )
					continue;

				if ( !room_cmp( current_room->exits[i]->name, room_name ) )
				{
					if ( found )
						strcat( buf2, ", " );

					strcat( buf2, C_B );
					strcat( buf2, dir_name[i] );
					found = 1;
				}
			}
		}

		if ( !found )
		{
			strcat( buf2, C_R );
			strcat( buf2, room_name );
		}

		strcat( buf2, C_R "]\r\n" C_0 );
		suffix( buf2 );
	}
}



void parse_eventstatus( char *line )
{
	char buf[512], name[512];

	DEBUG( "parse_eventstatus" );

	if ( !strncmp( line, "Current event: ", 15 ) )
	{
		evstatus_list = 1;
		return;
	}

	if ( !evstatus_list )
		return;

	line = get_string( line, buf, 512 );

	if ( buf[0] == '-' )
		return;

	if ( !strcmp( buf, "Player" ) )
		return;

	if ( buf[0] < 'A' || buf[0] > 'Z' )
	{
		evstatus_list = 0;
		return;
	}

	strcpy( name, buf );

	/* Skip all numbers and other things, until we get to the room name. */
	while ( 1 )
	{
		if ( !line[0] )
			return;

		if ( line[0] < 'A' || line[0] > 'Z' )
		{
			line = get_string( line, buf, 512 );
			continue;
		}

		break;
	}

	strcpy( buf, line );
	strcat( buf, "." );

	locate_arena = 1;
	locate_room( buf, 0, name );
}

void parse_noexit( char *line )
{
	if ( !current_room || ( ( strcmp( "There is no exit in that direction.", line ) && strcmp( "There is nowhere to gallop in that direction.", line ) ) && !waitfornoexitline ) || mode != FOLLOWING )
		return;

	int q;

	q = lastdir;

	if ( !waitfornoexitline ) {
		if ( current_room->exits[q] && current_room->noexitcmd[q] )
		{clientff(C_D" (%s)"C_0,current_room->noexitcmd[q]);
			char *ep,*p,buf[256];
			ep = current_room->noexitcmd[q];
			for ( p = ep; *p; p++ )
				if ( *p == '$' )
					*p = '\n';
			sprintf(buf, "%s\r\n", ep);
			for ( p = ep; *p; p++ )
				if ( *p == '\n' )
					*p = '$';
			waitfornoexitline = q;
			send_to_server( buf );
		}}
	else
	{
		if ( current_room->noexitcmd[waitfornoexitline] )
		{
			if ( current_room->noexitmsg[waitfornoexitline] && !strcmp(current_room->noexitmsg[waitfornoexitline],line) ) {
				waitfornoexitline=0;
				if ( auto_walk )
					auto_walk = 2;
			}
		}
	}
}


void parse_who( LINE *line )
{
	static int first_time = 1, len1, len2;
	ROOM_DATA *r, *room;
	AREA_DATA *area;
	char name[64];
	char roomname[256];
	char buf2[1024];
	char color[64];
	int more, len, moreareas, hc = 54;

	DEBUG( "parse_who" );

	if ( disable_wholist ||
			line->line[0] != ' ' ||
			line->len < 53 )
		return;

	/* Initialize these two, so we won't do them each time. */
	if ( first_time )
	{
		len1 = 9 + strlen( C_D C_G C_D );
		len2 = 9 + strlen( C_D C_R C_D );
		first_time = 0;
	}

	/* Has two lines, with color changes at the sides? */

	if ( !cmp( " - *", line->line + 14 ) &&
			!cmp( " - *", line->line + 51 ) )
	{
		if ( *line->rawp[0] )
			strcpy( color, line->rawp[0] );
		else
			strcpy( color, C_0 );

		get_string( line->line, name, 64 );
		if ( !strncmp(line->line + 54, "(H) ", 4) )
			hc = 58;
		strcpy( roomname, line->line + hc );
	}
	else
		return;

	line->gag_entirely = 1;


	/* Search for it. */

	room = NULL;
	area = NULL;
	more = 0;
	moreareas = 0;
	len = strlen( roomname );
	int i;
	char roomname2[256];
	strcpy( roomname2, roomname );

	if (roomname2[len-1] == ' ' )
	{
		for ( i = 1; i < len; i++ )
		{
			if (roomname2[len-i] != ' ')
			{
				strcpy( roomname, roomname2 );
				len = strlen( roomname );
				break;
			}
			else
			{
				roomname2[len-i] = 0;
			}
		}
	}


	for ( r = world; r; r = r->next_in_world ) {
		int a;
		for ( a = 1; a < 5; a++) {
			if ( (!strncmp( roomname, r->name, len )  && hc != 58 ) || ( !strncmp( roomname, r->name, len )  && (hc == 58 && strstr(r->area->name,"Havens") ) ) || ( r->additional_name[a] != NULL && !strncmp( roomname, r->additional_name[a], len ) ) )
			{
				if ( !room ) {
					room = r, area = r->area;
					break;}
				else
				{
					more++;
					if ( area != r->area )
						moreareas++;
					break;
				}
			}
		}
	}
	if ( hc == 58 )
		clientff( "%s%14s" C_c " - " C_0 "(H) %-20s ", color, name, roomname );
	else
		clientff( "%s%14s" C_c " - " C_0 "%-24s ", color, name, roomname );

	if ( !room )
	{
		clientff( "%9s", "" );
	}
	else
	{
		char buf3[256];
		int len;

		if ( !more )
		{
			sprintf( buf3, C_D "(" C_G "%d" C_D ")", room->vnum );
			len = len1;
		}
		else
		{
			sprintf( buf3, C_D "(" C_R "%d rms" C_D ")", more + 1 );
			len = len2;
		}

		clientff( "%*s", len, buf3 );
	}

	clientf( C_c " - " C_0 );

	if ( area )
	{
		if ( !moreareas )
		{
			strcpy( buf2, area->name );
			buf2[24] = 0;
			clientf( buf2 );
		}
		else
			clientf( C_D "(" C_R "multiple areas matched" C_D ")" C_0 );
	}
	else
		clientf( C_D "(" C_R "unknown" C_D ")" C_0 );

	if ( !more && room && room->wormhole )
		clientf( C_D " (" C_G "Warp" C_D ")" C_0 );
}



void parse_underwater( char *line )
{
	DEBUG( "parse_underwater" );

	/* 1 - No, 2 - Trying to get up, 3 - Up. */

	if ( pear_defence == 3 && !cmp("You are surrounded by a shining corona of blinding light as your*", line) )
		pear_defence = 0;

	if ( !strcmp( line, "form an air pocket around you." ) ||
			!strcmp( line, "You are already surrounded by a pocket of air." ) ||
			!strcmp( line, "You are surrounded by a pocket of air and so must move normally through water." ) ||
			!cmp("You touch * and immediately your skin *", line) ||
			!cmp("*form an air pocket around you.", line) )
		pear_defence = 2;

	if ( !cmp("Your skin becomes scaly and your body cools down dramatically*", line) )
		pear_defence = 3;
	if ( pear_defence != 3 )
		if ( !strcmp( line, "The pocket of air around you dissipates into the atmosphere." ) ||
				!strcmp( line, "The bubble of air around you dissipates." ) )
			pear_defence = 0;

	if ( pear_defence == 1 && !cmp("You have no ^.", line) ) {
		wateroption = 0;
		clientff(C_R"\r\n[Enchantment failed, reverting to pear.]\r\n"C_0);
		pear_defence = 0;}
}



void parse_special_exits( char *line )
{
	EXIT_DATA *spexit;

	DEBUG( "parse_special_exits" );

	if ( !current_room )
		return;

	if ( mode != FOLLOWING && mode != CREATING )
		return;

	/* Since this function is checked after parse_room, this is a good place. */
	if ( mode == CREATING && capture_special_exit )
	{
		strcpy( cse_message, line );
		return;
	}
	if ( !artimsg ) {
		for ( spexit = current_room->special_exits; spexit; spexit = spexit->next )
		{
			if ( !spexit->message )
				continue;

			if ( !cmp( spexit->message, line ) )
			{
				if ( spexit->to )
				{
					current_room = spexit->to;
					current_area = current_room->area;
					if ( !spexit->nolook )
					{add_queue_top( -1 );}
					else
					{if ( auto_walk )
						auto_walk = 2;}
					if ( spexit->command ) {
						int i;
						for ( i = 1; dir_name[i]; i++ ) {
							if ( strstr(spexit->command, dir_name[i] ) ) {
								spexitwalk = 1;
								break;
							}
						}
					}
				}
				else
				{
					mode = GET_UNLOST;
				}
				if ( !disable_automap ) {
					didmove = 1;}
				return;
			}
		}

		for ( spexit = global_special_exits; spexit; spexit = spexit->next )
		{

			if ( !spexit->message )
				continue;

			if ( !cmp( spexit->message, line ) )
			{
				if ( spexit->to )
				{
					if ( !strcmp(spexit->command,"Haven") || !strcmp(spexit->command,"haven") )
					{havenstore = current_room;}
					current_room = spexit->to;
					current_area = current_room->area;
					if ( !spexit->nolook )
					{add_queue_top( -1 );}
					else
					{if ( auto_walk )
						auto_walk = 2;}
					if ( !disable_automap ) {
						didmove = 1;}
				}
				else
				{
					mode = GET_UNLOST;
					if ( !strcmp(spexit->command,"Haven") || !strcmp(spexit->command,"haven") )
					{havenstore = current_room;}
				}
				if ( !disable_automap ) {
					didmove = 1;}
				clientff( C_R " (" C_Y "%s" C_R ")" C_0, spexit->command ? spexit->command : "teleport" );
				return;
			}
		}
	}

}


void parse_wormwarp( char * line )
{
	WORMHOLE_DATA *worm;

	DEBUG( "parse_wormwarp" );

	if ( !current_room )
		return;

	if ( mode != FOLLOWING && mode != CREATING )
		return;

	if ( strncmp( line, "The wormhole spits you out at the other end.", 44 ) )
		return;

	if ( current_room->wormhole )
	{
		worm = current_room->wormhole;

		if ( worm->to )
		{
			current_room = worm->to;
			current_area = current_room->area;
			add_queue_top( -1 );
			return;
		}
	}
	else
	{
		mode = GET_UNLOST;
	}
}


void parse_sprint( char *line )
{
	static int sprinting;
	char buf[256];

	DEBUG( "parse_squint" );

	if ( mode != FOLLOWING && mode != CREATING )
		return;

	if ( !sprinting && !strncmp( line, "You look off to the ", 20 ) &&
			( strstr( line, " and dash speedily away." ) || strstr(line, " and will your sands to carry you away.") ) )
	{
		int i;

		get_string( line + 20, buf, 256 );

		sprinting = 0;

		for ( i = 1; dir_name[i]; i++ )
		{
			if ( !strcmp( buf, dir_name[i] ) )
			{
				sprinting = i;
				break;
			}
		}

		return;
	}

	if ( !sprinting )
		return;

	/* End of the sprint? */
	if ( strncmp( line, "You dash through ", 17 ) && strncmp(line, "Your sands carry you through ",29) )
	{
		add_queue_top( sprinting );
		sprinting = 0;
		return;
	}

	if ( !current_room || !current_room->exits[sprinting] ||
			( room_cmp( current_room->exits[sprinting]->name, line + 17 ) && room_cmp( current_room->exits[sprinting]->name, line + 29 ) ) )
	{
		sprinting = 0;
		clientf( " " C_D "(" C_G "lost" C_D ")" C_0 );
		return;
	}

	current_room = current_room->exits[sprinting];
	clientff( " " C_D "(" C_G "%d" C_D ")" C_0, current_room->vnum );
}

void parse_gallop(char *line )
{
	ROOM_DATA *destination;
	char dir[256];
	char tmp[256];
	int i, dir_nr = 0;
	int changed_area = 0;

	DEBUG( "parse_gallop" );

	if ( mode != FOLLOWING || !current_room )
		return;

	if ( strncmp( line, "You gallop away to the ", 23 ) != 0
			&& strncmp( line, " You gallop away to the ", 24 ) != 0
			&& cmp("* bolts sharply, carrying you off to the ^.", line)
			&& cmp("* continues to guide you, heading ^.", line) )
		return;

	if ( !galloping )
		galloping = 1;
	if ( !disable_automap )
		didmove = 1;

	if ( !strncmp( line, " You gallop away to the ", 24 ) )
		line = get_string( line + 24, dir, 256);
	else if ( strstr(line, "carrying") ) {
		line = strrchr(line, 44);
		line = get_string( line + 25, tmp, 256);
		strncpy(dir, tmp, strlen(tmp)-1);
	}
	else if ( strstr(line, "guide") ) {
		line = strrchr(line, 44);
		line = get_string( line + 9, tmp, 256);
		strncpy(dir, tmp, strlen(tmp)-1);
	}
	else
		line = get_string( line + 23, dir, 256);

	if ( !dir[0] )
	{
		debugf( "No direction found." );
		return;
	}

	for ( i = 1; dir_name[i]; i++ )
	{
		if ( !strcmp( dir, dir_name[i] ) ||
				!strcmp( dir, dir_small_name[i] ) )
		{

			dir_nr = i;
		}
	}


	if ( !dir_nr )
		return;


	destination = current_room->exits[dir_nr];

	/* No destination?  Then we're lost. */
	if ( !destination )
	{
		clientff( C_R " (" C_G "lost" C_R ")" C_0 );
		mode = GET_UNLOST;
		return;
	}

	if ( current_room->area != destination->area )
		changed_area = 1;

	current_room = destination;

	clientff( C_D " (" C_G "%d" C_D ")" C_0, current_room->vnum);
	if ( changed_area )
	{
		current_area = current_room->area;
		clientff( "\r\n" C_R "[Entered the %s]" C_0,
				current_area->name );
	}

}

void parse_follow( char *line )
{
	static char msg[256];
	static int in_message;
	char *to;

	if ( mode != FOLLOWING || !current_room )
		return;
	/* automap blizzard fix  */
	if ( didmove && !cmp("A blizzard rages around you, blurring the world into a slate of uniform white.",line) )
	{ automap_draw();
		return;
	}
	if ( !cmp( "You follow *", line ) )
	{
		in_message = 1;

		strcpy( msg, line );
	}
	else if ( in_message )
	{
		if ( strlen( msg ) > 0 && msg[strlen(msg)-1] != ' ' )
			strcat( msg, " " );
		strcat( msg, line );
	}
	else
		return;

	if ( in_message && !strchr( msg, '.' ) )
		return;

	in_message = 0;

	if ( !( to = strstr( msg, " to the " ) ) )
		return;

	{
		/* We now have our message, which should look like this... */
		/* You follow <Name> to the <direction> */

		char dir[256];
		int i = 0;

		/* Snatch the direction. */
		strcpy( dir, to + 8 );
		if ( !dir[0] )
		{
			debugf( "No direction found." );
			return;
		}

		strtok (dir,".");

		if ( !strcmp("trees",dir) || !strcmp("skies",dir) || !strcmp("ground",dir) )
		{
			if ( mode == CREATING || mode == FOLLOWING )
				add_queue( -1 );
			return;
		}
		for ( i = 1; dir_name[i]; i++ )
		{
			if ( !strcmp( dir, dir_name[i] ) ||
					!strcmp( dir, dir_small_name[i] ) )
			{

				if ( mode == CREATING || mode == FOLLOWING )
					add_queue( i );
				return;
			}
		}
	}


	/* If you got this far, then the exit was probably a special exit */



	if ( current_room->special_exits )
	{
		waitingforspecial=1;
		return;
	}

	/* If the room has no special exits, then we're lost */

	if ( !current_room->special_exits )
	{
		clientff( C_R " (" C_G "lost" C_R ")" C_0 );
		mode = GET_UNLOST;
		return;
	}
}

void parse_aetoliastuff ( char *line )
{
	if ( !strcmp("Password correct. Welcome to Aetolia.",line) )
	{onlogin = 0;}

	if ( ( mode != FOLLOWING && mode != CREATING ) || !current_room )
		return;

	if ( cmp( "You travel to the ^ entering *.", line ) && !strstr(line, "-- v") && cmp("You sense through your link that ^ walks under the moon at *",line) && cmp("You peer into the distance, and see that ^ is at *.",line) && cmp( "You charge away to the ^, attempting to trample any who get in your way.",line )
			&& cmp("You charge ^, trampling ^ in the process.",line) && cmp("Your wolf has moved to *",line) && cmp("You recall that you built your tent at *",line) && cmp("You leave your Haven to return once more to reality.",line)
			&& cmp("Tilting your nose to the wind, you sense that your den is at *",line) )
		return;
	if ( strstr(line,"-- v") )
	{
		if ( !current_room )
			return;
		char *line2;
		char atv[64];
		line2 = strstr(line,"-- v");
		if ( cmp("-- v^ ---",line2) && cmp("-- v^ -------",line2) && cmp("-- v^ ---------*",line2) )
			return;
		line2 += 4;
		get_string(line2, atv, 64);
		if ( atoi(atv) > 0 && atoi(atv) != current_room->aetvnum ) {
			current_room->aetvnum = atoi(atv);
			clientff(C_0"Aetolia vnum is "C_D"("C_G"%d"C_D")"C_0,atoi(atv));}
		return;
	}
	else if ( !cmp( "You travel to the ^ entering *.", line ) )
	{
		if ( mode != FOLLOWING )
			return;

		int i;
		char dir[256];

		line = get_string(line+17,dir,256);

		if ( dir == NULL )
		{return;}

		for ( i = 1; dir_name[i]; i++ )
		{
			if ( !strcmp( dir, dir_name[i] ) ||
					!strcmp( dir, dir_small_name[i] ) )
			{

				if ( mode == FOLLOWING && current_room->exits[i] ) {
					current_room = current_room->exits[i];
					current_area = current_room->area;
					clientff(C_R" ("C_g"%s"C_R")"C_0, current_room->area->name );
				}
				else
				{
					clientf( C_R " (" C_G "lost" C_R ")" C_0 );
					current_room = NULL;
					current_area = NULL;
					mode = GET_UNLOST;
				}
			}
		}
	}
	else if ( !cmp( "You charge away to the ^, attempting to trample any who get in your way.",line ) || !cmp("You charge ^, trampling ^ in the process.",line) )
	{
		char dirn[64];
		if ( !cmp("You charge ^, trampling ^ in the process.",line)  )
			get_string(line+10,dirn,64);
		else
			get_string(line+22,dirn,64);
		int i;
		dirn[strlen(dirn)-1] = 0;

		for ( i = 1; dir_name[i]; i++ )
		{
			if ( !strcmp( dirn, dir_name[i] ) ||
					!strcmp( dirn, dir_small_name[i] ) )
				add_queue( i );
		}
	}
	else if ( !cmp("Your wolf has moved to *",line) )
	{farsight = 1;
		locate_room( line + 23, 2, NULL );}
	else if ( !cmp("You peer into the distance, and see that ^ is at *.",line) )
	{  char *buf,line2[256],buf2[256];
		AREA_DATA *farea = NULL;
		line = strstr(line, " is at ");
		line += 7;
		buf = line;
		while ( buf && strstr(buf, " in ") )
		{buf += 4;
			if ( strstr(buf, " in ") )
				buf = strstr(buf, " in ");}
		strncpy(line2,line, buf - line );
		line2[buf-line-4] = '.';
		line2[buf-line-3] = 0;
		if ( !strncmp( buf, "the ", 4 ) )
			buf += 4;
		sprintf(buf2, "donotshow %s",buf );
		farea = get_area_by_name( buf2 );
		if ( farea && farea->name )
		{
			locate_room_in_area( line2, NULL,0, farea );
		}
	}
	else if ( !cmp("You sense through your link that ^ walks under the moon at *",line) )
	{
		char *buf,line2[256],buf2[256];
		AREA_DATA *farea = NULL;
		line = strstr(line, " moon at ");
		line += 9;
		buf = line;
		while ( buf && strstr(buf, " in ") )
		{buf += 4;
			if ( strstr(buf, " in ") )
				buf = strstr(buf, " in ");}
		strncpy(line2,line, buf - line );
		line2[buf-line-4] = '.';
		line2[buf-line-3] = 0;
		if ( !strncmp( buf, "the ", 4 ) )
			buf += 4;
		sprintf(buf2, "donotshow %s",buf );
		farea = get_area_by_name( buf2 );
		if ( farea && farea->name )
		{
			locate_room_in_area( line2, NULL,0, farea );
		}
	}
	else if ( !cmp("You leave your Haven to return once more to reality.",line) )
	{if ( havenstore ) {
						   current_room = havenstore;
						   current_area = havenstore->area;
						   havenstore = NULL;
						   add_queue_top( -1 );}
	else {
		add_queue_top( -1 );}}
	else if ( !cmp("You recall that you built your tent at *",line) || !cmp("Tilting your nose to the wind, you sense that your den is at *",line) )
	{  char *buf,line2[256],buf2[256];
		AREA_DATA *farea = NULL;
		if ( strstr(line, " your den ") )
			line += 61;
		else
			line += 39;
		buf = line;
		while ( buf && strstr(buf, " in the ") )
		{buf += 8;
			if ( strstr(buf, " in the ") )
				buf = strstr(buf, " in the ");}
		strncpy( line2, line, buf - line );
		line2[buf-line-8] = '.';
		line2[buf-line-7] = 0;
		sprintf(buf2, "donotshow %s",buf );
		farea = get_area_by_name( buf2 );
		if ( farea && farea->name )
		{
			locate_room_in_area( line2, "tent",0, farea );
		}
	}
}

void parse_areasearch( char *line )
{
	if (!area_search)
		return;

	if ( strncmp(line, "\"", 1) && cmp("You can see the following ^ objects:",line) && strncmp(line, "There is nothing here.",22))
		return;

	if ( strstr(line, area_search_for) )
	{clientff(C_G"<--"C_W"Found: "C_y"%s"C_0,area_search_for);if( !searchnostop ) area_search=0;}

	if ( !cmp("You can see the following ^ objects:",line) && area_search )
	{
		searching = 1;
	}

	if ( !strncmp(line, "There is nothing here.",22) && area_search )
	{

		ROOM_DATA *r;
		int i;

		area_search = 1;

		r = search_room, i = 0;
		/* Count rooms left. */
		while ( r )
		{
			r = r->next_in_area;
			i++;
		}

		clientff( C_R "\r\n[Rooms left: %d.]\r\n" C_0, i - 1 );

		search_room = search_room->next_in_area;

		if ( !search_room )
		{
			area_search = 0;
			clientfr( "All rooms searched." );
			memset(area_search_for, 0, sizeof(area_search_for));
		}

		init_openlist( NULL );
		init_openlist( search_room );
		path_finder( );
		go_next( );
	}

}

void parse_conscripts ( char *line )
{
	if ( !current_room )
		return;

	if (strncmp(line, "(G) ",4))
		return;
	else
	{if ( ( line = strstr(line, "        ") ) )
		while ( !isalpha(line[0]) )
			line += 1;
		areaonly=1;
		locate_roompart(line, 0, NULL,NULL);
	}

}

void parse_autobump( char *line )
{
	/* 1 - Bump all around that room. */
	/* 2 - Wait for bumps to complete. */
	/* 3 - Search a new room. */

	if ( !auto_bump )
		return;

	if ( auto_bump == 2 )
	{
		if ( !strcmp( "There is no exit in that direction.", line ) ) {
			bump_exits--;}

		if ( !strcmp("You raise your compass to eye level, focusing on the small inner sphere.",line) )
			bump_exits = 0;

		if ( (!strcmp("You cannot glean any information about your surroundings.",line) || !strncmp( line, "Your environment conforms to that of ", 37 ) ) && bump_eye )
			bump_exits = 0;

		if ( !bump_exits )
		{
			ROOM_DATA *r;
			int i;

			auto_bump = 3;

			r = bump_room, i = 0;
			/* Count rooms left. */
			while ( r )
			{
				r = r->next_in_area;
				i++;
			}

			clientff( C_R "\r\n[Rooms left: %d.]\r\n" C_0, i - 1 );

			bump_room = bump_room->next_in_area;

			if ( !bump_room )
			{
				auto_bump = 0;
				force_save = 1;
				clientfr( "All rooms completed. Forcing Save Map" );
				return;
			}

			init_openlist( NULL );
			init_openlist( bump_room );
			clientff(C_R"["C_y"%s "C_D"("C_G"%d"C_D") "C_R"next in bump list"C_R"]\r\n"C_0,bump_room->name,bump_room->vnum);
			path_finder( );
			go_next( );
		}
	}

}

void parse_infohere( char *line)
{
	if ( !current_room || strncmp(line, "\"",1) )
		return;

	if (!disable_autoshrine) {
		if ( ( strstr(line, "a shrine of ") ) || ( strstr(line, "a monument of ") ) ) {
			char dname[128],buf[256],csize[64];
			if ( strstr(line, " monument ") ) {sprintf(csize,"monument");}
			else if ( strstr(line,"master") ) {sprintf(csize,"master");}
			else {sprintf(csize,"shrine");}
			line = strstr(line, " of ");
			line = get_string(line+4,dname,128);
			if (strcmp(csize,"master")) {
				dname[strlen(dname)-1] = 0;}
			if ( ( current_room->shrinedivine && strcmp(dname, current_room->shrinedivine ) ) || !current_room->shrinedivine ) {
				clientf("\r\n");
				sprintf(buf, "%s%s large %s", current_room->shrinedivine ? "update " : "" , dname,csize);
				do_map_shrine(buf);
			}}}
}

void parse_guardyell( char *line )
{
	if ( !current_room )
		return;

	if ( cmp("(^) ^ says: *", line) && cmp("A *", line) && cmp("An *", line))
		return;


}

void parse_mounted ( char *line )
{
	if ( cmp( "With acrobatic grace, you quickly hop off of *.", line ) &&
			cmp( "You swiftly swing up onto *.", line ) &&
			cmp( "You step down off of *.", line ) &&
			cmp( "You climb up on *.", line ) &&
			cmp( "You stand up and stretch your arms out wide.", line ) &&
			cmp( "You cannot swim while mounted.", line ) &&
			cmp( "You are already mounted on *.", line ) &&
			cmp( "You slap * the rump and send it cantering off towards *", line ) &&
			cmp( "You burrow yourself into the ground.", line ) &&
			cmp( "Losing your balance, you fall from your steed to the hard ground.", line ) &&
			cmp( "You cannot do that while mounted.", line ) &&
			cmp( "You are not currently riding anything.", line ) )
		return;

	if ( !cmp( "With acrobatic grace, you quickly hop off of *.", line ) )
		mounted = 0;
	if ( !cmp( "You swiftly swing up onto *.", line ) )
		mounted = 1;
	if ( !cmp( "You step down off of *.", line ) )
		mounted = 0;
	if ( !cmp( "You climb up on *.", line ) )
		mounted = 1;
	if ( !cmp( "You stand up and stretch your arms out wide.", line ) )
		mounted = 0;
	if ( !cmp( "You cannot swim while mounted.", line ) )
		mounted = 1;
	if ( !cmp( "You are already mounted on *.", line ) )
		mounted = 1;
	if ( !cmp( "You slap * the rump and send it cantering off towards *", line ) )
		mounted = 0;
	if ( !cmp( "You burrow yourself into the ground.", line ) )
		mounted = 0;
	if ( !cmp( "Losing your balance, you fall from your steed to the hard ground.", line ) )
		mounted = 0;
	if ( !cmp( "You cannot do that while mounted.",line) )
		mounted = 1;
	if ( !cmp( "You are not currently riding anything.",line) )
		mounted = 0;
}

void parse_compass ( char *line )
{
	if ( cmp("Spinning wildly at first, the small trapped sphere begins to slow, before pointing off to the *.", line) )
		return;
	if ( !current_room )
		return;

	int i;
	char exith[256],comdir[256];


	line = get_string( line+94, exith, 256 );
	strncpy(comdir,exith, strlen(exith)-1);

	for ( i = 1; dir_name[i]; i++ )
	{
		if ( !strcmp( comdir, dir_name[i] ) )
			if ( !current_room->exits[i] && !current_room->locked_exits[i] ) {
				clientff(C_W" "C_W"("C_C"Unmapped Secret"C_W")"C_0"");
				current_room->detected_exits[i] = 1;
			}
	}

}

void check_autobump( )
{
	if ( !auto_bump || !bump_room )
		return;

	/* 1 - Bump all around that room. */
	/* 2 - Wait for bumps to complete. */
	/* 3 - Search a new room. */

	if ( auto_bump == 1 )
	{
		bump_exits = 0;
		int i;

		send_to_server( "info here\r\n" );
		send_to_server( "survey\r\n" );
		if ( bump_compass ) {
			send_to_server( "secrets\r\n" );}
		if ( bump_compass || bump_eye )
		{bump_exits = 1;}
		clientff( "\r\n" );
		if ( !bump_compass && !bump_eye )
			for ( i = 1; dir_name[i]; i++ )
			{
				if ( bump_room->exits[i] )
					continue;

				bump_exits++;
				send_to_server( dir_small_name[i] );
				send_to_server( "\r\n" );
			}

		auto_bump = 2;
	}

	if ( auto_bump == 2 )
		return;

	if ( auto_bump == 3 && bump_room == current_room )
	{
		/* We're here! */

		auto_bump = 1;
		check_autobump( );
	}
}



void i_mapper_process_server_line( LINE *l )
{
	const char *block_messages[] =
	{    "You cannot move that fast, slow down!",
		"You cannot move until you have regained equilibrium.",
		"You are regaining balance and are unable to move.",
		"You'll have to swim to make it through the water in that direction.",
		"rolling around in the dirt a bit.",
		"You are asleep and can do nothing. WAKE will attempt to wake you.",
		"There is a door in the way.",
		"You must first raise yourself and stand up.",
		"You are surrounded by a pocket of air and so must move normally through water.",
		"You cannot fly indoors.",
		"You slip and fall on the ice as you try to leave.",
		"Your mount slips and falls on the ice as you try to leave.",
		"You slip and fall on the blood-covered ground.",
		"A rite of piety prevents you from leaving.",
		"A wall blocks your way.",
		"The hands of the grave grasp at your ankles and throw you off balance.",
		"You are unable to see anything as you are blind.",
		"The forest seems to close up before you and you are prevented from moving that way.",
		"There is no exit in that direction.",
		"The ground seems to tilt and you fall with a thump.",
		"Your legs are tangled in a mass of rope and you cannot move.",
		"You must stand up to do that.",
		"Your legs are crippled, how will you move?",
		"You cannot walk through an enormous stone gate.",

		/* Lusternia. */
		"Now now, don't be so hasty!",
		"You are recovering equilibrium and cannot move yet.",

		/* Aetolia. */
		"You must first raise yourself from the floor and stand up.",
		"Solid rock blocks the way in that direction. You cannot go through it.",
		"You'll have to try to tunnel under the environment ahead.",
		"Even at this great depth, you find you cannot burrow in that direction. The environment is not amenable to it.",
		"Even at this great depth, you find you cannot go in that direction. The environment is not amenable to it.",
		"You'll have to try to go under the environment ahead.",
		"You can't burrow, silly!",
		"You can't burrow in this environment.",
		"There's water ahead of you. You'll have to swim in that direction to make it through.",
		"The ceiling is too hard to burrow through.",
		"You must regain balance first.",
		"You must regain equilibrium first.",
		"You are packed much too tightly into the earth to do that.",
		"There is not enough overhead room to fly.",
		"The dense overhead canopy prevents you from taking off from the ground.",
		"Your body is unbalanced, subterfuge evades you at this time.",
		"You are blind and can see nothing but darkness.",
		"You can swing no higher.",
		"You are impaled and must writhe off before you may do that.",
		"Your state of paralysis prevents you from doing that.",
		"Your mind is not synchronised to do that.",
		"You are too unbalanced to do that.",
		"There are no trees from which to swing.",
		"The dense canopy below prevents you from landing on the ground.",
		"There are no trees for you to land in.",
		"You are already soaring through the sky, free as a bird.",
		"You are already on the ground.",
		"You get down on your belly and begin trying to swim, but only succeed in rolling around in the dirt a bit.",
		"You cannot leap from indoors!",
		"You cannot leap to that direction as it is indoors.",
		"You cannot swim while mounted.",
		"You have reached bedrock and can burrow no deeper.",
		"The wings shoot you quickly towards the ceiling, stopping just short of impact, and gently lower you back to the ground.",
		"There is no wormhole here.",
		"The ground there is solid rock and cannot be passed through.",
		"You cannot leap when there is a roof over your head.",
		"You cannot leap into an indoors room.",
		"The duststorm has confused you, and you are unable to find an escape.",
		"You are not impaling anyone.",
		"You are not within the treetops.",
		"The emptiness of the location frightens you.",
		"You are not within a haven to begin with!",
		"Sticky strands of webbing prevent you from moving.",
		"The ground is covered with ice too thick to burrow through.",
		"The environment is not amenable to burrowing in that direction.",
		"A pervasive force prohibits your access to that location.",
		"How do you expect to head indoors while in the trees?",

		NULL
	};
	char *line = l->raw_line;
	int i;

	DEBUG( "i_mapper_process_server_line" );

	parse_noexit( l->line );

	if ( ( auto_walk && ( !cmp( "You cannot move that fast, slow down!", l->line ) ||
					!cmp( "Now now, don't be so hasty!", l->line ) ) ) ||
			( auto_bump && ( !cmp( "There is no exit in that direction.", l->line ) ) ) )
	{
		l->gag_entirely = 1;
		l->gag_ending = 1;
		gag_next_prompt = 1;
	}
	if ( !bump_exits && auto_bump && ( !cmp( "There is no exit in that direction.", l->line ) ) )
	{clientff( C_R "\r\n[Missing Exit " C_W "%s" C_R " of v" C_W "%d" C_R ". Bump Skipping.]\r\n"C_0,
			dir_name[current_room->pf_direction], current_room->vnum );
	void do_map_bump( char *arg );
	do_map_bump("skip");
	}
	/* Gag/replace the alertness message, with something easier to read. */
	parse_alertness( l );

	parse_who( l );

	if ( !l->raw_len )
		return;
	fulllinecheck( l->line );
	/* Are we sprinting, now? */
	parse_sprint( l->line );

	/*mounting/dismounting?*/
	parse_mounted( l->line );

	/* Are we galloping */
	parse_gallop( l->line );

	/* Is this a room? Parse it. */
	parse_room( l );

	/* Is this a special exit message? */
	parse_special_exits( l->line );

	parse_wormwarp( l->line );
	/* Is this a follow message, if we're following someone? */
	parse_follow( l->line );

	parse_aetoliastuff(l->line);

	parse_nointeract(l->line);

	parse_conscripts( l->line );

	parse_wind( l->line );

	parse_mindthrow( l->line );
	/* Is this a sense/seek command? */
	if ( !disable_locating )
	{
		parse_msense( l->line );
		parse_auraglance( l->line );
		parse_window( l->line );
		parse_eldsense( l->line );
		parse_scent( l->line );
		parse_cgc( l->line );
		parse_allysense( l->line );
		parse_alarm( l->line );
		parse_wormholes( l->line );
		parse_scry( l->line );
		parse_ka( l->line );
		parse_seek( l->line );
		parse_view( l->line );
		parse_werescent( l );
		parse_werepack( l->line );
		parse_watch( l->line );
		parse_pursue( l->line );
		parse_entities( l->line );
		parse_traplist( l->line );
		parse_eventstatus( l->line );
		parse_sources( l->line );
		parse_compass( l->line );
		parse_shrinesight( l->line );
		parse_angelrite(l->line);
		parse_fullsense( l->line );
		parse_leylines( l->line );
	}

	/* Can we get the area name and room type from here? */
	parse_survey( line );
	/* owner? */
	parse_owner( l->line );
	parse_ruler( l->line );
	/* no wing areas */
	parse_nowingarea( l->line );
	/* fullline cleanup */
	if ( fulllineok ) {
		memset( fullline, '\0', sizeof(fullline) );}
	for ( i = 0; block_messages[i]; i++ )

		if ( !strncmp( line, block_messages[i], strlen(line) ) )
		{
			/* Remove last command from the queue. */
			if ( q_top )
				q_top--;

			if ( !q_top )
				del_timer( "queue_reset_timer" );

			break;
		}
	/* block messages from block and rubble */
	if ( !cmp("^ stops you from moving that way.",line) )
	{	  if ( q_top )
		q_top--;

		if ( !q_top )
			del_timer( "queue_reset_timer" );

	}
	if ( !cmp("Your mount halts as it encounters a pile of rubble to the ^.",line) )
	{	  if ( q_top )
		q_top--;

		if ( !q_top )
			del_timer( "queue_reset_timer" );

	}
	if ( mode == FOLLOWING || mode == CREATING )
		parse_underwater( line );

	parse_autobump( line );
	parse_areasearch( l->line );
	parse_infohere( l->line );

	if ( !cmp( "You have recovered balance on all limbs.", l->line ) )
	{
		if ( mode == FOLLOWING && auto_walk && ( burrowed || current_room->pointed_by || dash_command ) )
			auto_walk = 2;
	}

	if ( !cmp( "You have recovered equilibrium.", l->line ) )
	{
		if ( mode == FOLLOWING && auto_walk && ( troopmove || justwarped || dash_command || guardmove ) )
		{
			auto_walk = 2;
			if ( justwarped ) justwarped = 0;
		}
	}

	if ( auto_walk )
	{
		if ( !strncmp( line, "There's water ahead of you.", 27 ) )
		{
			if ( moved )
			{
				if ( mounted )
				{
					swim_next = 0;
					walk_next = 0;
					clientf(C_R"\r\n[You need to dismount to swim]\r\n"C_0);
					dash_command = NULL;
					except = NULL;
					troopmove = 0;
					guardmove = 0;
				}
				auto_walk = 0;
				moved = 0;
			}
			else if ( disable_swimming && !mounted )
			{ auto_walk = 0;walk_next = 0;swim_next = 0;
				clientf( "\r\n" C_R "[Hint: Swimming is disabled. Use 'map config swim' to turn it back on.]" C_0 );
			}
			else if ( mounted )
			{
				walk_next = 1;
				auto_walk = 2;
			}
			else {
				swim_next = 1;
				auto_walk = 2;
			}
		}


		if ( !strncmp( line, "You get down on your belly and begin trying to swim,", 52 ) )
		{
			walk_next = 1;
			auto_walk = 2;
		}
		if ( !strcmp(line, "The ground is far too slippery for your mount to get a hold of and gallop."))
		{
			walk_next = 1;
			auto_walk = 2;
		}
		if ( !strcmp(line, "The ground is far too slippery for you to sprint or dash anywhere."))
		{
			walk_next = 1;
			auto_walk = 2;
		}
		if ( !strcmp( line, "You cannot move that fast, slow down!" ) ||
				!strcmp( line, "Now now, don't be so hasty!" ) )
		{
			auto_walk = 2;
		}

		if ( !strcmp( line, "There is a door in the way." ) )
		{
			if ( !nodoors ) {
				door_closed = 1;
				door_locked = 0;
				auto_walk = 2;}
			else
			{door_closed = 0;
				door_locked = 0;
				auto_walk = 0;
				clientff( C_R "\r\n[Door " C_W "%s" C_R " of v" C_W "%d" C_R ". Speedwalking disabled.]\r\n" C_0,
						dir_name[current_room->pf_direction], current_room->vnum );}
		}

		if ( !strcmp( line, "The door is locked." ) )
		{
			if ( !disable_auto_unlock ) {
				door_closed = 0;
				door_locked = 1;
				auto_walk = 2;
			} else {
				if ( !auto_bump ) {
					door_closed = 0;
					door_locked = 0;
					auto_walk = 0;
					clientff( C_R "\r\n[Locked room " C_W "%s" C_R " of v" C_W "%d" C_R ". Speedwalking disabled.]\r\n" C_0,
							dir_name[current_room->pf_direction], current_room->vnum );}
				else
				{clientff( C_R "\r\n[Locked room " C_W "%s" C_R " of v" C_W "%d" C_R ". Bump Skipping.]\r\n"C_0,
						dir_name[current_room->pf_direction], current_room->vnum );
				door_closed = 0;
				door_locked = 0;
				void do_map_bump( char *arg );
				do_map_bump("skip");
				}
			}
		}

		if ( !strcmp(line, "You cannot do that while in ethereal form.") && door_closed )
		{
			door_closed = 0;
			door_locked = 0;
			auto_walk = 0;
			clientf( C_R "\r\n[Mistform, cannot open door. Speedwalking disabled.]\r\n"C_0);
		}

		if ( !strncmp( line, "This door has been magically locked shut.", 41 ) )
		{
			if ( !auto_bump )
			{door_closed = 0;
				door_locked = 0;
				auto_walk = 0;
				clientff( C_R "\r\n[Locked room " C_W "%s" C_R " of v" C_W "%d" C_R ". Speedwalking disabled.]\r\n" C_0,
						dir_name[current_room->pf_direction], current_room->vnum );}
			else
			{clientff( C_R "\r\n[Locked room " C_W "%s" C_R " of v" C_W "%d" C_R ". Bump Skipping.]\r\n"C_0,
					dir_name[current_room->pf_direction], current_room->vnum );
			door_closed = 0;
			door_locked = 0;
			void do_map_bump( char *arg );
			do_map_bump("skip");
			}
		}
		if ( !strncmp( line, "You open the door to the ", 25 ) )
		{
			door_closed = 0;
			door_locked = 0;
			door_opened = 1;
			auto_walk = 2;
		}

		if ( !cmp( "You unlock the ^ door.", line ) )
		{
			door_closed = 1;
			door_locked = 0;
			auto_walk = 2;
			door_unlocked = 1;
		}
		if ( !strncmp( line, "The door is not locked.", 23 ) )
		{
			door_closed = 1;
			door_locked = 0;
			auto_walk = 2;

		}
		if ( !strncmp( line, "There is no door to the ", 24 ) )
		{
			door_closed = 0;
			door_locked = 0;
			auto_walk = 2;
		}
		if ( !strcmp( line, "You are not carrying a key for this door." ) )
		{
			if ( !auto_bump )
			{door_closed = 0;
				door_locked = 0;
				auto_walk = 0;
				clientff( C_R "\r\n[Locked room " C_W "%s" C_R " of v" C_W "%d" C_R ". Speedwalking disabled.]\r\n" C_0,
						dir_name[current_room->pf_direction], current_room->vnum );}
			else
			{clientff( C_R "\r\n[Locked room " C_W "%s" C_R " of v" C_W "%d" C_R ". Bump Skipping.]\r\n"C_0,
					dir_name[current_room->pf_direction], current_room->vnum );
			door_closed = 0;
			door_locked = 0;
			void do_map_bump( char *arg );
			do_map_bump("skip");
			}
		}

	}
}



void i_mapper_process_server_prompt( LINE *l )
{
	DEBUG( "i_mapper_process_server_prompt" );

	if ( parsing_room )
		parsing_room = 0;

	if ( sense_message )
		sense_message = 0;

	if (leyline_message)
		leyline_message = 0;

	if ( scout_list )
		scout_list = 0;

	if ( trap_list )
		trap_list = 0;

	if ( evstatus_list )
		evstatus_list = 0;

	if ( pet_list )
		pet_list = 0;

	if ( areaonly )
		areaonly = 0;

	if ( areaandadj )
		areaandadj = 0;

	if ( werescent )
		werescent = 0;

	if ( werepack )
		werepack = 0;

	if ( wormsources )
		wormsources = 0;

	if ( farsight )
		farsight = 0;

	if ( ssight )
		ssight = 0;

    if ( wingtmpdisable )
        wingtmpdisable = 0;

	memset( fullline, '\0', sizeof(fullline) );

	if ( area_search && searching )
	{

		ROOM_DATA *r;
		int i;

		searching=0;

		r = search_room, i = 0;
		/* Count rooms left. */
		while ( r )
		{
			r = r->next_in_area;
			i++;
		}

		clientff( C_R "\r\n[Rooms left: %d.]\r\n" C_0, i - 1 );

		search_room = search_room->next_in_area;

		if ( !search_room )
		{
			area_search = 0;
			clientfr( "All rooms searched." );
			memset(area_search_for, 0, sizeof(area_search_for));
		}

		init_openlist( NULL );
		init_openlist( search_room );
		path_finder( );
		go_next( );
	}

	if ( norulerc ) {
		norulerc = 0;
		if ( current_room && current_room->ruler )
		{
			memset( current_room->ruler, '\0', sizeof(current_room->ruler) );
			current_room->ruler = NULL;
		}}

	if ( mode != GET_UNLOST && !onlogin )
	{clientf("\r\n[Forcing aetolia MXP off, mapper currently doesn't work with it.]");
		send_to_server("config mxp off\r\n");
		if (!disable_forcewrap )
		{clientf("\r\n[Setting wrapwidth to 0 for optimal performance. (Can be disabled with: map config forcewrap)]");
			send_to_server("config wrapwidth 0\r\n");}
		onlogin = 1;}

	if ( get_unlost_exits )
	{
		ROOM_DATA *r, *found = NULL;
		int count = 0, i;

		get_unlost_exits = 0;

		if ( !current_room )
			return;

		for ( r = world; r; r = r->next_in_world )
		{
			if ( !strcmp( r->name, current_room->name ) )
			{
				int good = 1;

				for ( i = 1; dir_name[i]; i++ )
				{
					if ( ( ( r->exits[i] || r->detected_exits[i] )
								&& !get_unlost_detected_exits[i] ) ||
							( !( r->exits[i] || r->detected_exits ) &&
							  get_unlost_detected_exits[i] ) )
					{
						good = 0;
						break;
					}
				}

				if ( good )
				{
					if ( !found )
						found = r;

					count++;
				}
			}
		}

		if ( !found )
		{
			prefix( C_R "[No perfect matches found.]\r\n" C_0 );
			mode = GET_UNLOST;
		}
		else
		{
			current_room = found;
			current_area = current_room->area;
			mode = FOLLOWING;

			if ( !count )
			{
				prefix( C_W "IMapper: Impossible error.\r\n" );
				return;
			}

			prefixf( C_R "[Match probability: %d%%]\r\n" C_0, 100 / count );
		}
	}

	if ( check_for_duplicates )
	{
		check_for_duplicates = 0;

		if ( current_room && mode == CREATING )
		{
			ROOM_DATA *r;
			int count = 0, far_count = 0, i;

			for ( r = world; r; r = r->next_in_world )
				if ( !strcmp( current_room->name, r->name ) && r != current_room )
				{
					int good = 1;

					for ( i = 1; dir_name[i]; i++ )
					{
						int e1 = r->exits[i] ? 1 : r->detected_exits[i];
						int e2 = current_room->exits[i] ? 1 : current_room->detected_exits[i];

						if ( e1 != e2 && !r->hasrandomexits )
						{
							good = 0;
							break;
						}
					}

					if ( good )
					{
						if ( r->area == current_room->area )
							count++;
						else
							far_count++;
					}
				}

			if ( count || far_count )
			{
				prefixf( C_R "[Warning: Identical rooms found... %d in this area, %d in other areas.]\r\n" C_0,
						count, far_count );
			}
		}
	}
	if ( current_room && !pear_defence &&
			( current_room->underwater ||
			  current_room->room_type->underwater ) )
	{
		pear_defence = 1;
		if ( !wateroption ) {
			clientf( C_W "(" C_G "outc pear"C_W"/"C_G"eat pear" C_W ") " C_0 );
			send_to_server("outc pear\r\neat pear\r\n");
		}
		if ( wateroption == 3 ) {
			clientf( C_W "(" C_G "belch"C_W"/"C_G"morph swordfish" C_W ") " C_0 );
			send_to_server("belch\r\nmorph swordfish\r\n");
		}
		if ( wateroption == 2 ) {
			char buf[256];
			sprintf( buf, "touch %s\r\n", underitem );
			send_to_server( buf );
			clientff( C_W "(" C_G "touch %s" C_W ") " C_0, underitem );
		}
		if ( wateroption == 1 )
			pear_defence = 2;
	}

	if ( galloping && auto_walk )
	{
		auto_walk = 2;
		galloping = 0;
	} else if (galloping && !auto_walk ) {galloping = 0;}

	if ( mode == FOLLOWING && auto_walk == 2 )
	{
		go_next( );
	}

	check_autobump( );

	if ( last_room != current_room && current_room )
	{
		show_floating_map( current_room );
		last_room = current_room;
	}

	if ( force_save )
	{force_save = 0;
		void do_map_save( char *arg );
		do_map_save("");
		int save_settings( char *file);
		save_settings( "config.mapper.txt" );
	}

	if ( gag_next_prompt )
	{
		gag_next_prompt = 0;
		l->gag_entirely = 1;
		l->gag_ending = 1;
	}
}



AREA_DATA *get_area_by_name( char *string )
{
	AREA_DATA *area = NULL, *a;
	int nr;
	int hidden = 0;
	if ( strstr(string,"donotshow") )
	{ string += 10;
		hidden = 1;
	}
	if ( !string[0] )
	{
		if ( !current_area )
		{
			if ( !hidden )
				clientfr( "No current area set." );
			return NULL;
		}
		else
			return current_area;
	}
	nr = 0;
	for ( a = areas; a; a = a->next ) {
		if ( !strncmp( string, a->name, strlen(string)-1 ) )
		{
			if ( !area ) {
				area = a;
				return area;
			}
		}
	}
	for ( a = areas; a; a = a->next ) {
		if ( case_strstr( a->name, string ) ) {
			nr++;
			if ( !area )
				area = a;
		}
	}
	if ( nr == 1 )
		return area;
	else {
		area = NULL;
		/* More than one area of that name. */
		/* List them all, and return. */

		if ( !hidden ) {
			clientfr( "No Exact Match found." );
			clientfr( "Partial matches are:" ); }
		for ( area = areas; area; area = area->next )
			if ( case_strstr( area->name, string ) )
			{
				if ( !hidden )
					clientff( " - %s\r\n",  area->name );
			}
		return NULL;
	}

	if ( !area )
	{
		if ( !hidden )
			clientfr( "No area names match that." );
		return NULL;
	}

	return area;
}



/* Map commands. */

void do_map( char *arg )
{
	ROOM_DATA *room;
	map_y = 10;
	map_x = 14;
	if ( arg[0] && isdigit( arg[0] ) )
	{
		if ( !( room = get_room( atoi( arg ) ) ) )
		{
			clientfr( "No room with that vnum found." );
			return;
		}
	}
	else
	{
		if ( current_room )
			room = current_room;
		else
		{
			clientfr( "No current room set." );
			return;
		}
	}

	show_map_new( room );
}


void do_map_help( char *arg )
{
	clientfr( "Module: IMapper. Commands:" );
	clientf( " map help        - This help.\r\n"
			" area help       - Area commands help.\r\n"
			" room help       - Room commands help.\r\n"
			" exit help       - Room exit commands help.\r\n"
			" map load        - Load map.\r\n"
			" map save        - Save map.\r\n"
			" map none        - Disable following or mapping.\r\n"
			" map follow      - Enable following.\r\n"
			" map create      - Enable mapping.\r\n"
			" map remake      - Remake vnums.\r\n"
			" map path #      - Build directions to vnum #.\r\n"
			" map status      - Show general information about the map.\r\n"
			" map color       - Change the color of the room title.\r\n"
			" map exits       - Change the color of the exits.\r\n"
			" map file        - Set the file for map load and map save.\r\n"
			" map teleport    - Manage global special exits.\r\n"
			" map queue       - Show the command queue.\r\n"
			" map queue clear - Clear the command queue.\r\n"
			" map config      - Configure the mapper.\r\n"
			" map window      - Open a floating MXP-based window.\r\n"
			" map uw/under    - Configure your means of travel underwater\r\n"
			" map bump        - Attempts to run through the current area and update it\r\n"
			" map             - Generate a map, from the current room.\r\n"
			" map #           - Generate a map centered on vnum #.\r\n"
			" map medium      - Generate a map fitting '80' chars from this room.\r\n"
			" map medium #    - Generate a map fitting '80' chars on vnum #.\r\n"
			" map big         - Generate a map BIG in size from this room.\r\n"
			" map big #       - Generate a map BIG in size on vnum #.\r\n"
			" map types       - Generate Information on your maps room types.\r\n"
			" map ruler       - General help on the territory system.\r\n"
			" map tags        - Will show or search through the tags you have made.\r\n"
			" map types       - Will show you statistics about the types of rooms you have mapped.\r\n"
			" map auto        - Set the size of the automap.\r\n"
			" map shrine      - Help for the shrine system.\r\n"
			" landmarks       - Show all the landmarks, in the world.\r\n"
			" worldwarps      - Show all the wormholes, in the world.\r\n"
			" map tags (tag)  - Show all the room tags, in the world.\r\n"
			" go/stop         - Begins, or stops speedwalking.\r\n"
			" go troops/guard - Follow the command with the guards name#### to move with the guard/troops.\r\n" );
}



void do_map_remake( char *arg )
{

	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	remake_vnums( );
}

void do_map_create( char *arg )
{
	if ( world == NULL )
	{
		clientfr( "Mapping on. Please 'look', to update this room." );
		create_area( );
		current_area = areas;

		create_room( -1 );
		current_room = world;

		current_area->name = strdup( "New area" );

		q_top = 0;
	}
	else if ( !current_room )
	{
		clientfr( "No current room set, from which to begin mapping." );
		return;
	}
	else
		clientfr( "Mapping on." );

	mode = CREATING;
}



void do_map_follow( char *arg )
{
	if ( !current_room )
	{
		mode = GET_UNLOST;
		clientfr( "Will try to find ourselves on the map." );
	}
	else
	{
		mode = FOLLOWING;
		clientfr( "Following on." );
	}
}



void do_map_none( char *arg )
{
	if ( mode != NONE )
	{
		mode = NONE;
		clientfr( "Mapping and following off." );
	}
	else
		clientfr( "Mapping or following were not enabled, anyway." );
}



void do_map_save( char *arg )
{
	if ( !strcmp( arg, "binary" ) )
	{
		save_binary_map( map_file_bin );
		return;
	}

	if ( areas )
		save_map( map_file );
	else
		clientfr( "No areas to save." );
}



void do_map_load( char *arg )
{
	char buf[256];

	destroy_map( );
	get_timer( );
	if ( !strcmp( arg, "binary" ) ? load_binary_map( map_file_bin ) : load_map( map_file ) )
	{
		destroy_map( );
		clientfr( "Couldn't load map." );
	}
	else
	{
		sprintf( buf, "Map loaded. (%d microseconds)", get_timer( ) );
		clientfr( buf );
		convert_vnum_exits( );
		sprintf( buf, "Vnum exits converted. (%d microseconds)", get_timer( ) );
		clientfr( buf );
		check_map( );
		load_settings( "config.mapper.txt" );

		mode = GET_UNLOST;
	}
}



void do_map_path( char *arg )
{
	ROOM_DATA *room, *target, *from, *tmp;
	AREA_DATA *tarea;
	ELEMENT *tag;
	char buf[256];
	char buft[256];
	char bufe[256];
	int fromf = 0;
	int atv = 0;
	int toarea = 0;
	igareaoff = NULL;
	/* Force an update of the floating map. */
	last_room = NULL;
	from = NULL;
	target = NULL;
	justwarped = 0;
	except = NULL;
    autowing = 0;

	if ( !arg[0] )
	{
		init_openlist( NULL );
		path_finder( );
		clientfr( "Map directions cleared." );
		clientfr( "Usage: map path [near] (vnum/tag) [from (vnum2/tag2)]/[avoid (vnum2/tag2)" );

		return;
	}

	init_openlist( NULL );

	while ( arg[0] )
	{
		int neari = 0;

		arg = get_string( arg, buf, 256 );
		if ( current_room && current_room->area->disabled )
			igareaoff = current_room->area;
		if ( !strcmp( buf, "near" ) ) {
			arg = get_string( arg, buf, 256 );
			neari = 1;
		}
		if ( strstr( arg, "from" ) ) {
			arg = get_string( arg+5, buft, 256 );
			fromf = 1;
			wingtmpdisable = 1;
		}
		if ( strstr( arg, "avoid" ) && !fromf && !neari ) {
			arg = get_string( arg+6, bufe, 256 );
			if ( !( except = get_room( atoi( bufe ) ) ) )
				for ( room = world; room; room = room->next_in_world )
					for ( tag = room->tags; tag; tag = tag->next )
						if ( !case_strcmp( bufe, (char *) tag->p ) )
						{
							if ( !except ) {
								except = room;
								break;
							}
						}

		}
		if ( !strcmp( buf, "atv" ) ) {
			atv = 1;
			arg = get_string( arg, buf, 256);
		}
		if ( !strcmp(buf, "area")) {
			toarea = 1;
			arg = get_string( arg, buf, 256);
		}
		/* Vnum. */
		if ( isdigit( buf[0] ) && !toarea)
		{
			if ( !atv  ) {
				if ( !( room = get_room( atoi( buf ) ) ) )
				{
					clientff( C_R "[No room with the vnum '%s' was found.]\r\n" C_0, buf );
					init_openlist( NULL );
					return;
				}
			} else {
				if ( !( room = get_room_atv( atoi( buf ) ) ) )
				{
					clientff( C_R "[No room with the aetolian vnum '%s' was found.]\r\n" C_0, buf );
					init_openlist( NULL );
					return;
				}
			}

			if ( !neari ) {
				if ( room->area->disabled )
					igareaoff = room->area;
				init_openlist( room );
				if ( fromf )
					target = room;
			} else {
				for ( neari = 1; dir_name[neari]; neari++ )
					if ( room->exits[neari] ) {
						if ( room->area->disabled )
							igareaoff = room->exits[neari]->area ;
						init_openlist( room->exits[neari] );
						if ( fromf )
							target = room->exits[neari];
					}
			}
		} else { /* Tag or Area. */
			int found = 0;
			if (!toarea) { /* Tag */
				for ( room = world; room; room = room->next_in_world )
					for ( tag = room->tags; tag; tag = tag->next )
						if ( !case_strcmp( buf, (char *) tag->p ) ) {
							if ( !neari ) {
								if ( room->area->disabled )
									igareaoff = room->area;
								init_openlist( room );
								if ( fromf )
									target = room;
							} else {
								for ( neari = 1; dir_name[neari]; neari++ )
									if ( room->exits[neari] ) {
										if ( room->area->disabled )
											igareaoff = room->exits[neari]->area ;
										init_openlist( room->exits[neari] );
										if ( fromf )
											target = room->exits[neari];

									}
							}
							found = 1;
						}

				if ( !found ) {
					clientff( C_R "[No room with the tag '%s' was found.]\r\n" C_0, buf );
					init_openlist( NULL );
					return;
				}
			} else { /* Area */
			    fromf=0;
				if ( !( room = get_entrance_room_for_area( buf ) ) )
				{
					clientff( C_R "[No area with the name '%s' was found.]\r\n" C_0, buf );
					init_openlist( NULL );
					return;
				} else {
					tarea = room->area;
					init_openlist( room );
				}
			}
		}

		if ( fromf ) {
			if ( isdigit( buft[0] ) ) {
				if ( !( from = get_room( atoi( buft )))) {
					clientff( C_R "[No room with the vnum '%s' was found to path from.]\r\n" C_0, buft );
					init_openlist( NULL );
					return;
				} else {
					if ( from->area->disabled ) {
						igareaoff = from->area;
					}
				}
			} else {
				fromf = 0;
				for ( room = world; room; room = room->next_in_world )
					for ( tag = room->tags; tag; tag = tag->next )
						if ( !case_strcmp( buft, (char *) tag->p ) ) {
							from = room;
							if ( from->area->disabled )
								igareaoff = from->area;
							fromf = 1;
						}
				if ( !fromf ) {
					clientff( C_R "[No room with the tag '%s' was found.]\r\n" C_0, buft );
					init_openlist( NULL );
					return;
				}
			}
		}
	}

	path_finder( );

	/* Pick out the first room of the matching area in
	 * the path and path to that instead */
	if (toarea) {
		tmp = room;
		for (room = current_room; room; room = room->pf_parent) {
			if(room->area == tarea) {
				break;
			}
		}

		if (room != NULL) {
			init_openlist ( room );
			path_finder ( );
		} else {
			room = tmp;
		}

		clientff( C_R "[Pathing to: %s (" C_G "%s" C_R ")(" C_G "%d" C_R ")]\r\n" C_0,
				room->name, room->area->name, room->vnum);
	}
	returnroom = current_room;

	if ( !from && current_room ) {
		show_path( current_room );
	} else if ( from ) {
		clientff( C_R "[Path to '"C_W"%s"C_R"' from '"C_W"%s"C_R"' is.]\r\n" C_0, target->name, from->name);
		show_path( from );
		init_openlist( NULL );
		path_finder( );
	} else {
		clientfr( "Can't show the path from here though, as no current room is set." );
	}
}


void do_map_status( char *arg )
{
	AREA_DATA *a;
	ROOM_DATA *r;
	char buf[256];
	int count = 0, count2 = 0;
	int i;

	DEBUG( "do_map_status" );

	clientfr( "General information:" );

	for ( r = world; r; r = r->next_in_world )
		count++, count2 += sizeof( ROOM_DATA );

	sprintf( buf, "Rooms: " C_G "%d" C_0 ", using " C_G "%d" C_0 " bytes of memory.\r\n", count, count2 );
	clientf( buf );

	count = 0, count2 = 0;

	for ( a = areas; a; a = a->next )
	{
		int notype = 0, unlinked = 0;
		count++;

		for ( r = a->rooms; r; r = r->next_in_area )
		{
			if ( !notype && r->room_type == null_room_type )
				notype = 1;
			if ( !unlinked )
				for ( i = 1; dir_name[i]; i++ )
					if ( !r->exits[i] && r->detected_exits[i] &&
							!r->locked_exits[i] )
						unlinked = 1;
		}

		if ( !( notype || unlinked ) )
			count2++;
	}

	sprintf( buf, "Areas: " C_G "%d" C_0 ", of which fully mapped: " C_G "%d" C_0 ".\r\n", count, count2 );
	clientf( buf );

	sprintf( buf, "Room structure size: " C_G "%d" C_0 " bytes.\r\n", (int) sizeof( ROOM_DATA ) );
	clientf( buf );

	sprintf( buf, "Hash table size: " C_G "%d" C_0 " chains.\r\n", MAX_HASH );
	clientf( buf );

	sprintf( buf, "Map size, x: " C_G "%d" C_0 " y: " C_G "%d" C_0 ".\r\n", map_x, map_y );
	clientf( buf );

	for ( i = 0; color_names[i].name; i++ )
	{
		if ( !strcmp( color_names[i].title_code, room_color ) )
			break;
	}

	sprintf( buf, "Room title color: %s%s" C_0 ", code length " C_G "%d" C_0 ".\r\n", room_color,
			color_names[i].name ? color_names[i].name : "unknown", room_color_len );
	clientf( buf );
}

void do_map_types( char *arg )
{
	ROOM_DATA *room;
	ROOM_TYPE *type;
	int typec;
	int rtotal = 0;
	double percent,cr,ct;

	for ( room = world; room; room = room->next_in_world )
		rtotal++;
	clientff( C_R"  [Map room type count out total %d Rooms]\r\n",rtotal);
	for ( type = room_types; type; type = type->next ) {
		typec = 0;
		for ( room = world; room; room = room->next_in_world ) {
			if ( room->room_type->name == type->name )
				typec++;
		}
		if ( typec ) {
			ct = typec;
			cr = rtotal;
			percent = ct/cr*100.00;
			clientff(" %s%-30s"C_W": %4d "C_0"Rooms. %6.2f%% of Total.\r\n", type->color, type->name, typec, percent );
		}
	}
}

void do_map_color( char *arg )
{
	char buf[256];
	int i;

	arg = get_string( arg, buf, 256 );

	for ( i = 1; color_names[i].name; i++ )
	{
		if ( !strcmp( color_names[i].name, buf ) )
		{
			room_color = color_names[i].title_code;
			daynight_color = daynight_names[i].title_code;
			room_color_len = color_names[i].length;

			sprintf( buf, "Room title color changed to: " C_0 "%s%s" C_R ".",
					room_color, color_names[i].name );
			clientfr( buf );
			clientfr( "Use 'map config save' to make it permanent." );

			return;
		}
	}

	clientfr( "Possible room-title colors:" );

	for ( i = 1; color_names[i].name; i++ )
	{
		sprintf( buf, "%s - " C_0 "%s%s" C_0 ".\r\n",
				!strcmp( color_names[i].title_code, room_color ) ? C_R : "",
				color_names[i].title_code, color_names[i].name );
		clientf( buf );
	}

	clientfr( "Use 'map color <name>' to change it." );
}


void do_map_exitc( char *arg )
{
	char buf[256];
	int i;

	arg = get_string( arg, buf, 256 );

	for ( i = 1; color_names[i].name; i++ )
	{
		if ( !strcmp( color_names[i].name, buf ) )
		{
			exit_color = color_names[i].title_code;
			sprintf( buf, "Exit color changed to: " C_0 "%s%s" C_R ".",
					exit_color, color_names[i].name );
			clientfr( buf );
			clientfr( "Use 'map config save' to make it permanent." );

			return;
		}
	}

	clientfr( "Possible exit colors:" );

	for ( i = 1; color_names[i].name; i++ )
	{
		sprintf( buf, "%s - " C_0 "%s%s" C_0 ".\r\n",
				!strcmp( color_names[i].title_code, exit_color ) ? C_R : "",
				color_names[i].title_code, color_names[i].name );
		clientf( buf );
	}

	clientfr( "Use 'map exits <name>' to change it." );
}

void do_map_bump( char *arg )
{
	if ( !strcmp( arg, "skip" ) )
	{
		if ( bump_room && bump_room->next_in_area )
		{
			bump_room = bump_room->next_in_area;
			init_openlist( NULL );
			init_openlist( bump_room );
			clientff("\r\n"C_R"["C_y"%s "C_D"("C_G"%d"C_D") "C_R"next in bump list"C_R"]\r\n"C_0,bump_room->name,bump_room->vnum);
			path_finder( );
			clientfr( "Skipped one room." );
			go_next( );
		}
		else
		{clientfr( "Skipping a room is not possible, saving." );
			force_save = 1;
			auto_bump = 0;
			auto_walk = 0;
			justwarped = 0;
			dash_command = NULL;
			except = NULL;
		}

		return;
	}
	if ( !strcmp( arg, "help" ) )
	{clientfr("Map Bump Explained:");
		clientf(   "           - Map bump alone will start attempting to update the current area\r\n"
				"           - The mapper will move you around and attempt to find hidden exits\r\n"
				"           - If it finds a hidden exit, it will walk through it and into the room\r\n"
				"           - Behind the hidden/unmapped exits, map bump skip, to make it move on \r\n"
				"           - from that room you use. MAP BUMP SKIP, which will bypass the room that\r\n"
				"           - lead into the hidden/unmapped room, and continue attempting to update\r\n"
				" stop      - Temporarely stops the map bump attempt, use this to bypass problems\r\n"
				" continue  - Will continue a stopped attempt\r\n"
				" help      - Will give you this help\r\n" );
		return;}
	if ( !strcmp( arg, "continue" ) )
	{
		if ( !bump_room )
			clientfr( "Unable to continue bumping." );
		else
		{
			clientfr( "Bumping continues." );

			auto_bump = 3;

			init_openlist( NULL );
			init_openlist( bump_room );
			clientff("\r\n"C_R"["C_y"%s "C_D"("C_G"%d"C_D") "C_R"next in bump list"C_R"]\r\n"C_0,bump_room->name,bump_room->vnum);
			path_finder( );
			go_next( );
		}

		return;
	}

	if ( !strcmp(arg, "compass") ) {
		bump_compass = 1;}

	if ( !strcmp(arg, "eye") ) {
		bump_eye = 1;}

	if ( auto_bump || !strcmp( arg, "stop" ) )
	{
		auto_bump = 0;
		clientfr( "Bumping ended. Saving Map." );
		force_save = 1;
		return;
	}

	if ( !current_room )
	{
		clientfr( "No current room set." );
		return;
	}

	void do_map_shrine(char *arg);
	do_map_shrine("hide clear area");

	clientfr( "Bumping has begun, all shrines in area cleared." );

	auto_bump = 3;

	bump_room = current_room->area->rooms;
	init_openlist( NULL );
	init_openlist( bump_room );
	clientff("\r\n"C_R"["C_y"%s "C_D"("C_G"%d"C_D") "C_R"next in bump list"C_R"]\r\n"C_0,bump_room->name,bump_room->vnum);
	path_finder( );
	go_next( );
}



void do_map_queue( char *arg )
{
	char buf[256];
	int i;

	if ( arg[0] == 'c' )
	{
		q_top = 0;
		clientfr( "Queue cleared." );
		del_timer( "queue_reset_timer" );
		justwarped = 0;
		return;
	}

	if ( q_top )
	{
		clientfr( "Command queue:" );

		for ( i = 0; i < q_top; i++ )
		{
			sprintf( buf, C_R " %d: %s.\r\n" C_0, i,
					queue[i] < 0 ? "look" : dir_name[queue[i]] );
			clientf( buf );
		}
	}
	else
		clientfr( "Queue empty." );
}



void do_map_config( char *arg )
{
	char option[256];
	int i;
	struct config_option
	{
		char *option;
		int *value;
		char *true_message;
		char *false_message;
	} options[ ] =
	{
		{ "swim", &disable_swimming,
			"Swimming disabled - will now walk over water.",
			"Swimming enabled." },
		{ "wholist", &disable_wholist,
			"Parsing disabled.",
			"Parsing enabled" },
		{ "alertness", &disable_alertness,
			"Parsing disabled.",
			"Parsing enabled" },
		{ "locate", &disable_locating,
			"Locating disabled.",
			"Vnums will now be displayed on locating abilities." },
		{ "showarea", &disable_areaname,
			"The area name will no longer be shown.",
			"The area name will be shown after room titles." },
		{ "title_mxp", &disable_mxp_title,
			"MXP tags will no longer be used to mark room titles.",
			"MXP tags will be used around room title." },
		{ "exits_mxp", &disable_mxp_exits,
			"MXP tags will no longer be used to mark room exits.",
			"MXP tags will be used around room exits." },
		{ "map_mxp", &disable_mxp_map,
			"MXP tags will no longer be used on a map.",
			"MXP tags will now be used on a map." },
		{ "autolink", &disable_autolink,
			"Auto-linking disabled.",
			"Auto-linking enabled." },
		{ "artifacts", &disable_artifacts,
			"Pathfinding with artifact disabled.",
			"Pathfinding with artifact enabled." },
		{ "add", &disable_additional_name,
			"Additional Name adding disabled.",
			"Additional Name Adding enabled." },
		{ "autounlock", &disable_auto_unlock,
			"Automatic unlocking disabled.",
			"Automatic unlocking enabled." },
		{ "worm_warp", &disable_worm_warp,
			"Pathfinding through wormholes disabled.",
			"Pathfinding through wormholes enabled." },
		{ "shrineinfluence", &disable_shrineinfluence,
			"Shrine influence disabled.",
			"Shrine influence enabled." },
		{ "shrineradius", &disable_shrineradius,
			"Shrine radius disabled.",
			"Shrine radius enabled." },
        { "shrinecheck", &disable_shrineradiuscheck,
            "Shrine radius check disabled, shrine radius' will not affect shrine placement.",
            "Shrine radius check disabled, shrine radius' will affect shrine placement." },
		{ "farsightpath", &disable_farsightpath,
			"Pathing to farsight disabled.",
			"Pathing to farsight enabled." },
		{ "automap", &disable_automap,
			"Auto Map show disabled.",
			"Auto Map show enabled." },
		{ "territory", &disable_mapterritory,
			"Mapper will now color rooms by room type.",
			"Mapper will now color rooms by ruler." },
		{ "backup", &disable_autobackup,
			"Automatic backup of files when saving. Activated.",
			"Automatic backup of files when saving. Deactivated." },
		{ "autoshrine", &disable_autoshrine,
			"Mapper will no longer automaticly map shrines with info here.",
			"Mapper will now map shrines found with info here automaticly."},
		{ "forcewrap", &disable_forcewrap,
			"Mapper will no longer force wrapwidth 0 on login.",
			"Mapper will force wrapwidth 0 on login." },
		{ "vnum", &disable_vnum,
			"Vnums are no longer shown after area name in following mode.",
			"Vnums are now shown after area name in following mode." },
		{ "pathwrap", &disable_pathwrap,
			"Path will no longer be word wrapped.",
			"Path will be word wrapped." },

		{ NULL, NULL, NULL, NULL }
	};

	arg = get_string( arg, option, 256 );

	if ( !strcmp( option, "save" ) )
	{
		if ( save_settings( "config.mapper.txt" ) )
			clientfr( "Unable to open the file for writing." );
		else
			clientfr( "All settings saved." );
		return;
	}
	else if ( !strcmp( option, "load" ) )
	{
		if ( load_settings( "config.mapper.txt" ) )
			clientfr( "Unable to open the file for reading." );
		else
			clientfr( "All settings reloaded." );
		return;
	}

	if ( option[0] )
	{
		for ( i = 0; options[i].option; i++ )
		{
			if ( strcmp( options[i].option, option ) )
				continue;

			*options[i].value = *options[i].value ? 0 : 1;
			if ( *options[i].value )
				clientfr( options[i].true_message );
			else
				clientfr( options[i].false_message );

			clientfr( "Use 'map config save' to make it permanent." );

			return;
		}
	}

	clientfr( "Commands:" );
	clientf( " map config save            - Save all settings.\r\n"
			" map config load            - Reload the previously saved settings.\r\n"
			" map config swim            - Toggle swimming.\r\n"
			" map config wholist         - Parse and replace the 'who' list.\r\n"
			" map config alertness       - Parse and replace alertness messages.\r\n"
			" map config locate          - Append vnums to various locating abilities.\r\n"
			" map config showarea        - Show the current area after a room title.\r\n"
			" map config title_mxp       - Mark the room title with MXP tags.\r\n"
			" map config exits_mxp       - Mark the room exits with MXP tags.\r\n"
			" map config map_mxp         - Use MXP tags on map generation.\r\n"
			" map config autolink        - Link rooms automatically when mapping.\r\n"
			" map config artifacts       - Toggle pathfinding with wings/amulet.\r\n"
			" map config autounlock      - Toggle automatic unlocking.\r\n"
			" map config worm_warp       - Toggle pathfinding through wormholes.\r\n"
			" map config add             - Toggles adding additional room names, if\r\n"
			"                            - enter a room with another name then the\r\n"
			"                            - one the mapper has saved.\r\n"
			" map config shrineinfluence - Toggles show of shrine influence after room name\r\n"
			" map config shrineradius    - Toggles show of shrine radius on map\r\n"
			" map config farsightpath    - Toggles auto pathing to farsight target\r\n"
			" map config automap         - Toggles auto map, show map right after moving\r\n"
			" map config territory       - Toggles map room color by room type or ruler\r\n"
			" map config backup          - Toggles automatic file backup on save\r\n"
			" map config autoshrine      - Toggles automaticly placing shrine from info here\r\n"
			" map config forcewrap       - Toggles I_Mapper forcing wrapwidth 0 on login\r\n"
			" map config vnum            - Toggles showing vnum after area in following mode\r\n"
			" map config pathwrap        - Toggles word wrap of path\r\n"
			" map config shrinecheck     - Toggles if or if not shrine radius should influence placement of shrines\r\n");
	clientff(C_R"[Swimming: %s Wholist:   %s Alertness:  %s Locate:     %s]\r\n" C_0,
			disable_swimming ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_wholist ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_alertness ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_locating ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". " );
	clientff(C_R"[Showarea: %s MXP-Title: %s MXP-Exits:  %s MXP-Map:    %s]\r\n" C_0,
			disable_areaname ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_mxp_title ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_mxp_exits ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_mxp_map ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ");
	clientff(C_R"[Autolink: %s Artifacts: %s Add. Names: %s Autounlock: %s]\r\n" C_0,
			disable_autolink ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_artifacts ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_additional_name ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_auto_unlock ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ");
	clientff(C_R"[WormWarp: %s Shrine Influence: %s Shrine Radius: %s]\r\n" C_0,
			disable_worm_warp ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_shrineinfluence ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_shrineradius ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ");
	clientff(C_R"[Farsight Path: %s AutoMap: %s Territory: %s AutoBackup: %s]\r\n"C_0, disable_farsightpath ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_automap ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_mapterritory ? C_D"Room Type"C_R"." : C_W"Ruler"C_R". ", disable_autobackup ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". " );
	clientff(C_R"[Auto Shrine: %s ShrineCheck: %s ForceWrap: %s Vnum show: %s]\r\n"C_0, disable_autoshrine ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ", disable_shrineradiuscheck ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ",disable_forcewrap ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ",disable_vnum ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ");
	clientff(C_R"[Path Wrap: %s]\r\n"C_0, disable_pathwrap ? C_D"Disabled"C_R"." : C_W"Enabled"C_R". ");
}

void do_map_nothing( )
{
}

void do_map_window( char *arg )
{
	if ( floating_map_enabled )
	{
		floating_map_enabled = 0;
		clientfr( "Floating MXP map disabled. Don't forget to close the window!" );
		return;
	}

	if ( !mxp_tag( TAG_LOCK_SECURE ) )
	{
		floating_map_enabled = 0;
		clientfr( "Unable to get a SECURE tag. Maybe your client does not support MXP?" );
		return;
	}

	floating_map_enabled = 1;
	mxp( "<FRAME IMapper Left=\"-57c\" Top=\"2c\" Height=\"21c\""
			" Width=\"55c\" FLOATING>" );
	mxp( "<DEST IMapper>" );
	mxp_tag( TAG_LOCK_SECURE );
	mxp( "<!element mpelm '<send \"map path &v;|room look &v;\" "
			"hint=\"&r;|Vnum: &v;|Type: &t;\">' ATT='v r t'>" );
	mxp( "<!element mppers '<send \"map path &v;|room look &v;|who &p;\" "
			"hint=\"&p; (&r;)|Vnum: &v;|Type: &t;|Player: &p;\">' ATT='v r t p'>" );
	mxp( "</DEST>" );
	mxp_tag( TAG_DEFAULT );
	clientfr( "Enabled. Warning, this may slow you down." );
}

void do_map_big ( char *arg )
{
	ROOM_DATA *room;
	map_x = 24;
	map_y = 24;

	if ( arg[0] && isdigit( arg[0] ) )
	{
		if ( !( room = get_room( atoi( arg ) ) ) )
		{
			clientfr( "No room with that vnum found." );
			return;
		}
	}
	else
	{
		if ( current_room )
			room = current_room;
		else
		{
			clientfr( "No current room set." );
			return;
		}
	}

	show_map_new( room );
}


void do_map_medium ( char *arg )
{
	ROOM_DATA *room;
	map_x = 20;
	map_y = 20;

	if ( arg[0] && isdigit( arg[0] ) )
	{
		if ( !( room = get_room( atoi( arg ) ) ) )
		{
			clientfr( "No room with that vnum found." );
			return;
		}
	}
	else
	{
		if ( current_room )
			room = current_room;
		else
		{
			clientfr( "No current room set." );
			return;
		}
	}

	show_map_new( room );
}

void do_map_tiny ( char *arg )
{
	ROOM_DATA *room;
	map_x = 8;
	map_y = 8;

	if ( arg[0] && isdigit( arg[0] ) )
	{
		if ( !( room = get_room( atoi( arg ) ) ) )
		{
			clientfr( "No room with that vnum found." );
			return;
		}
	}
	else
	{
		if ( current_room )
			room = current_room;
		else
		{
			clientfr( "No current room set." );
			return;
		}
	}

	show_map_new( room );
}

void do_map_underw( char *arg )
{
	if ( !arg[0] ) {
		clientfr("Usage of Map Underwater/Map uw");
		clientf(
				" living    - Default, you are among the living and eat pear.\r\n"
				" undead    - You are undead and dont need to worry about breathing.\r\n"
				" ench      - Waterbreathing enchantment, syntax: map uw ench item#####.\r\n"
				" meta      - You are a metamorphosis user and have access to swordfish.\r\n");
		clientff(C_R "Current method is: "C_W"%s%s%s%s"C_R".\r\n" C_0,
				!wateroption ? "living" : "", wateroption == 1 ? "undead" : "" ,
				wateroption == 2 ? "ench" : "", wateroption == 3 ? "meta" : "" );
		return;
	} else if ( !strcmp(arg ,"living") ) {
		clientfr("You will now eat pear underwater.");
		if ( pear_defence == 2 && wateroption == 1 && !current_room->underwater && !current_room->room_type->underwater ) {
			pear_defence = 0;}
		wateroption = 0;
	}
	else if ( ( !strcmp(arg ,"undead") ) ) {
		clientfr("You declare that you are undead and care little about breathing.");
		wateroption = 1;
		pear_defence = 2;
	}
	else if ( ( strstr(arg ,"ench") ) ) {
		if ( *(arg+4) && *(arg+5) && strlen(arg+5)<30) {
			clientff(C_R"[You will now use '"C_W"touch %s"C_R"' when underwater.]\r\n" C_0, arg+5);
			underitem = malloc(30); strcpy(underitem, arg + 5);
			if ( pear_defence == 2 && wateroption == 1 && !current_room->underwater && !current_room->room_type->underwater ) {
				pear_defence = 0;}
			wateroption = 2;
		}
		else {clientfr("You must enter an item##### not above 29 characters.");}
	}
	else if ( ( !strcmp(arg ,"meta") ) ) {
		clientfr("You will now use swordfish morph underwater");
		clientfr("Note: To morph underwater, first eat pear, then morph.");
		if ( pear_defence == 2 && wateroption == 1 && !current_room->underwater && !current_room->room_type->underwater ) {
			pear_defence = 0;}
		wateroption = 3;
	}
	else {clientfr("Incorrect syntax for underwater/uw");}
	clientfr("Use: map config save. To make permanent.");


}

void do_map_automap( char *arg )
{
	if ( !arg[0] ) {
		clientfr("Usage of Map Auto");
		clientf(
				" tiny      - Default, auto map, will use map tiny.\r\n"
				" normal    - Auto Map will use normal map.\r\n");
		clientff(C_R "Current method is: "C_W"%s"C_R".\r\n" C_0,
				!automapsize ? "tiny" : "normal" );
		return;}

	if (!strncmp(arg,"tiny",strlen(arg)))
	{
		automapsize = 0;
		clientfr("AutoMap size is now: Tiny");
	}
	else if (!strncmp(arg,"normal",strlen(arg)))
	{
		automapsize = 1;
		clientfr("AutoMap size is now: Normal");
	}
	else
	{clientfr("Usage: map automap tiny/normal");}
}


void do_map_warps( char *arg )
{
	ROOM_DATA *r;
	int nr = 0;
	clientfr("Wormholes throughout the world:");
	for ( r = world; r; r = r->next_in_world )
		if ( r->wormhole && r->wormhole->to->vnum > r->vnum ) {nr++;
			clientff("  "C_Y"%d"C_D" ("C_G"%5d"C_D") "C_y"%-40.40s "C_R"("C_g"%s"C_R")\r\n"C_W" -> "C_D"("C_G"%5d"C_D") "C_y"%-40.40s "C_R"("C_g"%s"C_R")\r\n"C_0,
					nr, r->vnum, r->name, r->area->name ,r->wormhole->vnum, r->wormhole->to->name, r->wormhole->to->area->name );}
			if ( !nr )
				clientf("You have not made any wormholes.\r\n");

}

void do_map_tags ( char *arg )
{
	ROOM_DATA *r;
	ELEMENT *tag;
	char buf[256];
	char roomtags[256];
	int nr=0;
	if (strstr(roomtags,arg) == NULL)
		clientff("   "C_R" [ Searching for tags containing: %s ]\r\n"C_0,arg);
	for ( r = world; r; r = r->next_in_world )
	{
		if ( r->tags )
		{
			for ( tag = r->tags; tag; tag = tag->next )
			{
				strcat( roomtags, " " );
				strcat( roomtags, (char *) tag->p );
			}
			if ( *(arg) == 0 || case_strstr(roomtags,arg) ) {
				nr++;
				sprintf( buf, ""C_R"%4d: " C_D "(" C_G "%5d" C_D ") "C_y "%-30.30s " C_R"(" C_g "%-20.20s" C_R ")" C_Y " Tags"C_R":" C_W "%s" C_0 ". " C_0 "\r\n", nr, r->vnum, r->name, r->area->name, roomtags );
				clientf( buf );}
			sprintf( roomtags, C_W);
		}
	}
}

void do_map_ruler( char *arg )
{
	if ( !arg[0] )
	{clientfr("Map ruler Help:");
		clientf( " map ruler        - This Help\r\n"
				" Usage example    - Map ruler set room (vnum of room) (cityname).\r\n"
				"                  - ctn = name of the city that rules.\r\n"
				" Explained        - citn - name of a city.\r\n"
				" map ruler show > - Will show the rulers of this area by default.\r\n"
				" -> world         - Will show rulers of the world stats.\r\n"
				"                  - Both show, and show world, can be followed by 'all'\r\n"
				"                  - to count rooms with owners, which by default are not counted\r\n"
				" -> citn          - Will show the areas the city controls and what terrains\r\n"
				"                  - they control, follow up with total, to get only terrains.\r\n"
				" map ruler rem >  - Will remove the ruler of (se below)\r\n"
				" -> room (#)      - This room/the room at the # number.\r\n"
				" -> area          - This area.\r\n"
				" -> world         - All rulers in the world.\r\n"
				" -> citn          - Removes ruler citn from the world.\r\n"
				" map ruler set >  - Will set the ruler of (se below)\r\n"
				" -> room (#) ctn  - This room/the room at the # number.\r\n"
				" -> area ctn      - This area.\r\n"
				" -> area city ctn - This area and marks it as a city.\r\n"
				"                  - Making an area a city means it wont count in stats.\r\n"
				" color ctn color  - Changes the color of the city you name to the color you pick.\r\n"
				"                  - color alone will list all available colors.\r\n" );}


	ROOM_DATA *room;
	char cmd[64], op1[64],op2[64],op3[64];
	int c = -1;
	arg = get_string( arg, cmd, 64 );
	arg = get_string( arg, op1, 64 );
	arg = get_string( arg, op2, 64 );
	arg = get_string( arg, op3, 64 );
	if ( !strncmp(cmd,"show",4) )
	{     *op1 = toupper(op1[0]);
		int i;
		int oshow = 0;
		for ( i = 0; cities[i].name; i++ )
			if ( !strncmp(op1,cities[i].name,strlen(cities[i].name) ) )
				c = i;
		if ( c != -1 )
		{ if ( !current_room )
			{  clientfr("No room set");
				return;}
			int rtotal = 0,car = 0,typec;
			double percent,cr,ct;
			AREA_DATA *sar;
			ROOM_TYPE *ty;
			ROOM_DATA *rc;
			/* area listing and then rooms under them
			   areaname - amount of rooms captured
			   roomtype - number of rooms */
			clientff(C_R"[%s%s "C_R"has conquered"C_R"]\r\n"C_0,cities[c].color,cities[c].name);
			if ( strncmp(op2,"total",5) ) {
				for ( sar = areas; sar; sar = sar->next )
				{  /* listing by area an type */
					rtotal = 0;
					car = 0;
					for ( room = sar->rooms; room; room = room->next_in_area ) {
						rtotal++;
						if ( room->ruler && !strncmp(room->ruler, cities[c].name ,strlen(cities[c].name) ) )
							car++;}
					if ( car && sar->city != 1 ) {
						ct = car;
						cr = rtotal;
						percent = ct/cr*100.00;
						clientff(" "C_g"%20.20s "C_W"- "C_0"Size: %5d rooms. %s%10.10s "C_0"rules over %5d - %6.2f%%.\r\n",sar->name, rtotal,cities[c].color, cities[c].name,car,percent );
						for ( ty = room_types; ty; ty = ty->next ) {
							typec = 0;
							for ( rc = sar->rooms; rc; rc = rc->next_in_area ) {
								if ( sar->city != 1 && rc->room_type->name == ty->name && rc->ruler && !strncmp(rc->ruler, cities[c].name ,strlen(cities[c].name) ) )
									typec++;}
							if ( typec ) {
								ct = typec;
								cr = rtotal;
								percent = ct/cr*100.00;
								clientff("     %s%30s"C_W": %4d "C_0"Rooms. %6.2f%% of Total.\r\n", ty->color, ty->name, typec, percent );
							}
						}
					}
				}
			}
			else
			{clientfr("Area Type Totals");
				for ( room = world; room; room = room->next_in_world )
					rtotal++;
				for ( ty = room_types; ty; ty = ty->next ) {
					typec = 0;
					for ( rc = world; rc; rc = rc->next_in_world ) {
						if ( rc->area->city != 1 && rc->room_type->name == ty->name && rc->ruler && !strncmp(rc->ruler, cities[c].name ,strlen(cities[c].name) ) )
							typec++;}
					if ( typec ) {
						ct = typec;
						cr = rtotal;
						percent = ct/cr*100.00;
						clientff("     %s%30s"C_W": %4d "C_0"Rooms. %6.2f%% of Total.\r\n", ty->color, ty->name, typec, percent );
					}
				}
			}
		}
		else if ( !strncmp(op1,"World",5) )
		{
			if ( !current_room ) {
				clientfr("No room set");
				return;}
			int rtotal = 0,i,hr = 0;
			double perc,cr,ct;
			if ( !strncmp(op2, "all",3) )
				oshow = 1;
			for ( i = 0; cities[i].name; i++ )
				cities[i].count = 0;
			for ( room = world; room; room = room->next_in_world )
			{ if ( !room->area->city && ( oshow || !room->owner )  ) {
																		 rtotal++;
																		 if ( room->ruler && ( oshow || !room->owner ) ) {
																			 for ( i = 0; cities[i].name; i++ )
																			 {if ( !strncmp(room->ruler, cities[i].name ,strlen(cities[i].name)) )
																				 { cities[i].count++;
																				 }
																			 }
																		 }
																	 }
			}
			clientfr("The worlds territory is controlled by");
			cr = rtotal;
			for ( i = 0; cities[i].name; i++ )
			{
				if ( cities[i].count > 0 )
				{
					hr = 1;
					ct = cities[i].count;
					perc = ct/cr*100.00;
					clientff("%s%20.20s "C_W": "C_0"%6d Rooms. %6.2f%%\r\n"C_0,cities[i].color,cities[i].name,cities[i].count,perc);
				}
			}
			if ( !hr )
				clientfr("No one rules the world");
		}
		else
		{
			if ( !current_room ) {
				clientfr("No room set");
				return;}
			int rtotal = 0,i,hr = 0,nrr = 0;
			double perc,cr,ct;
			char noruler[1536] = "No ruler rooms, last 16 randomly chosen (max 40): \r\n";
			char tbuf[128];
			if ( !strncmp(op1, "All",3) )
				oshow = 1;
			for ( i = 0; cities[i].name; i++ )
				cities[i].count = 0;
			for ( room = current_room->area->rooms; room; room = room->next_in_area )
			{ if ( !room->area->city && ( oshow || !room->owner ) ) {
																		rtotal++;
																		if ( room->ruler && ( oshow || !room->owner ) ) {
																			for ( i = 0; cities[i].name; i++ )
																			{if ( !strncmp(room->ruler, cities[i].name ,strlen(cities[i].name)) )
																				{ cities[i].count++;
																				}
																			}
																		}
																	}
			if ( !room->area->city  && ( oshow || !room->owner ) && !room->ruler && nrr < 40 && ( nrr < 25 || ( nrr > 24 && rand() < rand() ) ) )
			{
				sprintf(tbuf,C_D"("C_G"%d"C_D") "C_0,room->vnum );
				if ( nrr == 8 || nrr == 16 || nrr == 24 || nrr == 32 || nrr == 40 )
					strcat(noruler, "\r\n");
				nrr++;
				strcat(noruler, tbuf );}
			}
			clientff(C_R"[This area's %d territories is controlled by:]\r\n"C_0, rtotal);
			cr = rtotal;
			for ( i = 0; cities[i].name; i++ )
			{
				if ( cities[i].count > 0 ) {
					hr = 1;
					ct = cities[i].count;
					perc = ct/cr*100.00;
					clientff("%s%20.20s "C_W": "C_0"%6d Rooms. %6.2f%%\r\n"C_0,cities[i].color,cities[i].name,cities[i].count,perc);
				}
			}
			if ( hr && nrr )
				clientff(C_C"%s\r\n"C_0,noruler);
			if ( !hr && !current_room->area->city ) {
				clientfr("No one rules this area");
			}
			else if ( !hr && current_room->area->city )
			{clientf(C_W"This area is marked as a city, to keep statistics correct outside\r\n"
					C_W"of cities, cities are not counted due to homes/guilds/stockrooms e.t.c.\r\n"C_0);
			}
		}
	}

	if ( !strncmp(cmd,"rem", 3) )
	{
		arg += 4;
		if (!strncmp(op1,"room",4))
		{
			if ( !current_room )
			{  clientfr("No room set");
				return;}
			arg += 5;
			if ( !( room = get_room( atoi( op2 ) ) ) ) {
				room = current_room;
			}
			if ( room->ruler != NULL )
			{
				memset( room->ruler, '\0', sizeof(room->ruler) );
				room->ruler = NULL;
				clientff(C_R"[%s (%d) ruler removed]\r\n"C_0,room->name, room->vnum );
			}
			else
			{clientff(C_R"[%s (%d) ruler removed]\r\n"C_0,room->name, room->vnum );}
		}
		else if (!strncmp(op1,"area",4))
		{
			if ( !current_room )
			{  clientfr("No room set");
				return;}
			for ( room = current_room->area->rooms; room; room = room->next_in_area )
				if ( room->ruler ) {
					memset( room->ruler, '\0', sizeof(room->ruler) );
					room->ruler = NULL;}
			current_room->area->city = 0;
			clientff(C_R"[Area: %s Ruler Cleared]\r\n"C_0, current_room->area->name);
		}
		else if (!strncmp(op1,"world",5))
		{
			if ( !current_room )
			{  clientfr("No room set");
				return;}
			for ( room = world; room; room = room->next_in_world )
				if ( room->ruler ) {
					memset( room->ruler, '\0', sizeof(room->ruler) );
					room->ruler = NULL;}
			clientfr("All ruler data deleted");
		}
		else
		{ int i,f=0;
			for ( i = 0; cities[i].name; i++ )
				if ( strncmp(op1,cities[i].name,strlen(op1)) )
				{f=1;}
			if ( f ) {
				clientff(C_R"[All rooms ruled by %s cleared]\r\n"C_0,op1);
				for ( room = world; room; room = room->next_in_world )
					if ( room->ruler && !strncmp(op1,room->ruler,strlen(op1)) ) {
						memset( room->ruler, '\0', sizeof(room->ruler) );
						room->ruler = NULL;}}
			else
			{clientfr("Remove what?");
				clientf(" map ruler rem >  - Will remove the ruler of (se below)\r\n"
						" -> room (#)   - This room/the room at the # number.\r\n"
						" -> area       - This area.\r\n"
						" -> world      - All rulers in the world.\r\n"
						" -> citn       - Removes ruler citn from the world.\r\n");}
		}
	}

	if ( !strncmp(cmd,"set", 3) )
	{
		if ( !strncmp(op1,"room",4) )
		{
			if ( !( room = get_room( atoi( op2 ) ) ) ) {
				room = current_room;
				strncpy(op3,op2,strlen(op2));
			}
			int i;
			*op3 = toupper(op3[0]);
			for ( i = 0; cities[i].name; i++ )
				if ( !strncmp(op3, cities[i].name,strlen(cities[i].name)) ) {
					room->ruler = malloc(strlen(cities[i].name)+1); strcpy(room->ruler, cities[i].name);
					clientff(C_R"[%s (%d) ruler set to %s%s"C_R"]\r\n"C_0,room->name,room->vnum,ruler_color( cities[i].name), cities[i].name);
				}
		}
		else if ( !strncmp(op1,"area",4) )
		{
			if ( !current_room ) {
				clientfr("No room set");
				return;}

			int i;
			if ( strstr(op2, "city") ) {
				if ( !current_room->area->city ) {
					current_room->area->city = 1;
					clientfr("Area set as city");
				}
				else
				{current_room->area->city = 0;
					clientfr("Area set to not be a city");
				}
			}
			else
				strncpy(op3,op2,strlen(op2));
			*op3 = toupper(op3[0]);
			for ( i = 0; cities[i].name; i++ )
				if ( !strncmp(op3, cities[i].name,strlen(cities[i].name)) ) {
					for ( room = current_room->area->rooms; room; room = room->next_in_area ) {
						room->ruler = malloc(strlen( cities[i].name)+1); strcpy(room->ruler,  cities[i].name);}
					clientff(C_R"[Area: %s ruler set to %s%s"C_R"]\r\n"C_0,current_room->area->name,ruler_color( cities[i].name), cities[i].name);
				}
		}
		else
		{clientfr("set what? usage is:");
			clientf(        " map ruler set >  - Will set the ruler of (se below)\r\n"
					" -> room (#)   - This room/the room at the # number.\r\n"
					" -> area       - This area.\r\n"
					" -> area city  - This area and marks it as a city.\r\n" );
		}
	}
	if ( !strncmp(cmd,"color", 5) )
	{
		if ( op1[0] )
		{
			int i,j;
			*op1 = toupper(op1[0]);
			for ( i = 0; cities[i].name; i++ )
				if ( !strcmp(op1, cities[i].name ) )
				{
					for ( j = 1; color_names[j].name; j++ )
					{
						if ( !strcmp( color_names[j].name, op2 ) )
						{
							clientff("%s is now %s%s\r\n", cities[i].name, color_names[j].code, color_names[j].name);
							cities[i].color = color_names[j].code;
							break;
						}
					}
				}
		}
		else
		{
			int i,j;
			clientfr("Color's available");
			for ( i = 1; color_names[i].name; i++ )
			{char buf[256];
				sprintf( buf, "%s - " C_0 "%s%s" C_0 ".\r\n",
						!strcmp( color_names[i].title_code, room_color ) ? C_R : "",
						color_names[i].title_code, color_names[i].name );
				clientf( buf );
			}
			clientfr("usage: map ruler color cityname color");
			for ( i = 0; cities[i].name; i++ )
				for ( j = 0; color_names[j].name; j++ )
					if ( cities[i].color == color_names[j].code )
						clientff("%s%20.20s "C_W":"C_0" has the color %s%s\r\n"C_0,cities[i].color,cities[i].name,cities[i].color,color_names[j].name);

		}

	}
}


void do_map_wing( char *arg)
{

  if ( !arg[0] )
  {
      clientfr("Map wings. Commands:");
      clientf( "Map wing set : Sets the custom wings command.\r\n"
               "Map wing room: Sets the room of your wings.\r\n"
               "Map wing rem : Removes the custom wing command and room.\r\n" );
       clientff(C_Y"Currently cmd: '%s' room: '%d'\r\n"C_0,wingcmd == NULL ? "nothing" : wingcmd, wingroom );
  }
  if (strstr(arg,"set"))
  {
   wingcmd = malloc(60); strcpy(wingcmd, arg + 4);
   clientff(C_R"[Wing command set to: '%s']\r\n"C_0,wingcmd);
  }
  if (strstr(arg,"room"))
  {
   wingroom = atoi(arg+5);
   clientff(C_R"[Wing room set to: '%d']\r\n"C_0,wingroom);
  }
  if (strstr(arg,"rem"))
  {
   wingcmd = NULL;
   free(wingcmd);
   wingroom = 0;
   clientfr("Wing command and room deleted");
  }
}


/* Area commands. */

void do_area_help( char *arg )
{
	clientfr( "Module: IMapper. Area commands:" );
	clientf( " area create - Create a new area, and switch to it.\r\n"
			" area list   - List all areas.\r\n"
			" area switch - Switch the area of the current room.\r\n"
			" area update - Update current area's name.\r\n"
			" area info   - Show information about current area. Follow by 'extra' for even more info.\r\n"
			"             - Follow by 'shrine' for shrine statistics, 'shrine setup' for possible locations\r\n"
			"             - to set up shrines.\r\n"
			" area off (area) - Toggle disabling/enabling pathfinding in current area or (area).\r\n"
			" area find   - works like room find but only for this area.\r\n"
			" area orig   - Check the originality of the area.\r\n"
			" area conn   - Will inform you of which areas are connected to this and where.\r\n"
			" area destroy- Will destroy the area you enter after it.\r\n"
			" area types  - Will show you the room types in this area.\r\n"
			" area note   - Will allow you to set a note for the area you are in.\r\n");
}

void do_area_destroy( char *arg )
{
	if (mode != CREATING) {
		clientfr("Creation mode must be on to destroy an area");
		return;
	}
	AREA_DATA *area;
	ROOM_DATA *room;
	area = get_area_by_name( arg );

	if ( !area || area->rooms == NULL )
		return;

	if ( area == current_area ) {
		clientfr("You can't destroy the area you are in");
		return;
	}

	for ( room = area->rooms; room; room = room->next_in_area ) {
		if ( room->shrine ) {
			char buf[256];
			sprintf(buf, "%d hide destroy",room->vnum);
			void do_map_shrine(char *arg);
			do_map_shrine(buf);
		}
		if ( room->shrineparrent ) {
			room->shrineeff = 0;
			room->shrineparrent = 0;
		}
		destroy_room(room);}

	clientff(C_R"[%s Destroyed.]\r\n"C_0,area->name);
	destroy_area(area);
	clientfr("Automatic Reloading");
	save_map( map_file );
	int save_settings( char *file);
	save_settings( "config.mapper.txt" );
	void do_map_load( char *arg );
	do_map_load( "" );

}

void do_area_fulld()
{
	clientfr("To destroy an area you need to type out destroy fully.");
	clientfr("Area destroy (areaname)");
}

void do_area_note( char *arg )
{
	if ( *(arg) == 0 || !current_area->name )
	{
		clientfr( "What do you wish to do?");
		clientf(" Area Note add   : Followed by the note you wish to make for this area.\r\n"
				" Area Note delete: Will delete the note for this area.\r\n" );
		return;
	}
	char cmd[32];
	arg = get_string(arg,cmd,32);
	if ( !strcmp(cmd, "add") )
	{ current_area->note = strdup(arg);
		clientff(C_R"[Area: %s Note is now: %s]\r\n"C_0,current_area->name, current_area->note );
		return;
	}
	else if ( !strcmp(cmd, "delete") )
	{   current_area->note = NULL;
		clientfr("Note for this area deleted");
		return;
	}
	else
	{	clientfr( "What do you wish to do?");
		clientf(" Area Note add   : Followed by the note you wish to make for this area.\r\n"
				" Area Note delete: Will delete the note for this area.\r\n" );}
}


void do_area_ruler( char *arg )
{
	if ( *(arg) == 0 || !current_area->name )
	{clientfr("What is it you want to do?");
		return;
	}
	char city[128];
	arg = get_string(arg,city,128);

}


void do_area_find( char *arg )
{
	ROOM_DATA *room;
	char buf[256];
	int count = 0;

	if ( *(arg) == 0 )
	{
		clientfr( "Find what?" );
		return;
	}
	if ( !current_area ) {
		clientfr("No current area");
		return;}

	/* Looking for a room type? */
	if ( !strncmp( arg, "type ", 5 ) )
	{
		ROOM_TYPE *type;

		for ( type = room_types; type; type = type->next )
			if ( !strcmp( type->name, arg+5 ) )
				break;

		if ( !type )
			clientfr( "What room type? Use 'room types' for a list." );
		else
		{
			clientfr( "Rooms that match:" );
			for ( room = current_area->rooms; room; room = room->next_in_area )
				if ( room->room_type == type ) {
					clientff( " " C_D "(" C_G "%d" C_D ")" C_0 " %s%s%s%s%s%s%s%s%s%s\r\n",
							room->vnum, room->name,
							room->area != current_area ? " (" : "",
							room->area != current_area ? room->area->name : "",
							room->area != current_area ? ")" : "",
							room->additional_name[1] ? C_C" (" : "",
							room->additional_name[1] ? C_Y"A" : "",
							room->additional_name[1] ? C_C")" : "",
							room->owner ? C_C" Owner: " : "",
							room->owner ? C_W"" : "",
							room->owner ? room->owner : "" );
					count++;}
			clientff(C_R"["C_G"%d"C_W" rooms found."C_R"]\r\n"C_0, count);
		}

		return;
	}

	/* Looking for a room owner? */
	if ( !strncmp( arg, "owner", 5 ) || !strncmp( arg, "owned", 5 ) )
	{
		if ( !isalpha(arg[6]) && strncmp( arg, "owned", 5 ) )
		{
			clientfr("Yes, but what is the name of the owner?");
		}
		else
		{
			char name[256];
			char buf[256];
			strcpy(name,arg+6);
			*name = toupper(name[0]);

			if ( !strncmp( arg, "owned", 5 ) )
				sprintf(buf,"Rooms with owners in the area:");
			else
				sprintf(buf,"Rooms owned by %s in this area:",name);

			clientfr( buf );
			for ( room = current_area->rooms; room; room = room->next_in_area )
				if ( ( room->owner && !strncmp( arg, "owned", 5 ) ) || ( room->owner && !strncmp(room->owner,name,strlen(room->owner)) ) ) {
					clientff( " " C_D "(" C_G "%d" C_D ")" C_0 " %s%s%s%s%s%s%s%s%s%s\r\n" C_0,
							room->vnum, room->name,
							room->area != current_area ? " (" : "",
							room->area != current_area ? room->area->name : "",
							room->area != current_area ? ")" : "",
							room->additional_name[1] ? C_C" (" : "",
							room->additional_name[1] ? C_Y"A" : "",
							room->additional_name[1] ? C_C")" : "",
							room->owner ? C_C" Owner: " : "",
							room->owner ? C_W"" : "",
							room->owner ? room->owner : ""   );
					count++;}
			clientff(C_R"["C_G"%d"C_W" rooms found."C_R"]\r\n"C_0, count);
		}

		return;
	}


	/* Looking for a room with x exits? */
	if ( !strncmp( arg, "exits", 5 ) )
	{
		if ( !isdigit(arg[6]) || !current_room )
		{
			clientfr("Yes, but how many exits?");
		}
		else
		{
			int exitnumb,e,c;
			exitnumb = atoi(arg+6);

			sprintf(buf,"Rooms with %d exits in the area:",exitnumb);

			clientfr( buf );
			for ( room = current_room->area->rooms; room; room = room->next_in_area) {
				c=0;
				for ( e = 1; dir_name[e]; e++ ) {
					if ( room->exits[e] || room->detected_exits[e] ) {
						c++;}}
				if ( c == exitnumb ) {
					sprintf( buf, " " C_D "(" C_G "%5d" C_D ") "C_c"("C_y"%5d"C_c")" C_0 " %-50.50s%s%s%s%s%s%s%s%s%s" C_0,
							room->vnum,
							room->aetvnum > 0 ? room->aetvnum : 0,
							room->name,
							room->area != current_area ? " (" : "",
							room->area != current_area ? room->area->name : "",
							room->area != current_area ? ")" : "",
							room->additional_name[1] ? C_C" (" : "",
							room->additional_name[1] ? C_Y"A" : "",
							room->additional_name[1] ? C_C")" : "",
							room->owner ? C_C" Owner: " : "",
							room->owner ? C_W"" : "",
							room->owner ? room->owner : ""
						   );
					count++;
					int i, first = 1;
					for ( i = 1; dir_name[i]; i++ )
					{
						if ( room->exits[i] || room->detected_exits[i] )
						{
							if ( first )
							{
								strcat( buf, C_D " (" );
								first = 0;
							}
							else
								strcat( buf, C_D "," );

							if ( room->exits[i] )
								strcat( buf, C_B );
							else
								strcat( buf, C_R );

							strcat( buf, dir_small_name[i] );
						}
					}
					if ( !first )
						strcat( buf, C_D ")" C_0 );
					strcat( buf, "\r\n" );
					clientf( buf );
				}
			}
			clientff(C_R"["C_G"%d"C_W" rooms found."C_R"]\r\n"C_0, count);
		}
		return;

	}



	clientfr( "Rooms that match:" );
	for ( room = current_area->rooms; room; room = room->next_in_area )
	{ int a;
		for ( a = 1; a < 10; a++ ) {
			if ( case_strstr( room->name, arg ) || (room->additional_name[a] != NULL &&
						case_strstr( room->additional_name[a], arg)))
			{
				sprintf( buf, " " C_D "(" C_G "%d" C_D ")" C_0 " %s%s%s%s%s%s%s%s%s%s" C_0,
						room->vnum, room->name,
						room->area != current_area ? " (" : "",
						room->area != current_area ? room->area->name : "",
						room->area != current_area ? ")" : "",
						room->additional_name[1] ? C_C" (" : "",
						room->additional_name[1] ? C_Y"A" : "",
						room->additional_name[1] ? C_C")" : "",
						room->owner ? C_C" Owner: " : "",
						room->owner ? C_W"" : "",
						room->owner ? room->owner : ""   );
				count++;

				if ( mode == CREATING )
				{
					int i, first = 1;
					for ( i = 1; dir_name[i]; i++ )
					{
						if ( room->exits[i] || room->detected_exits[i] )
						{
							if ( first )
							{
								strcat( buf, C_D " (" );
								first = 0;
							}
							else
								strcat( buf, C_D "," );

							if ( room->exits[i] )
								strcat( buf, C_B );
							else
								strcat( buf, C_R );

							strcat( buf, dir_small_name[i] );
						}
					}
					if ( !first )
						strcat( buf, C_D ")" C_0 );
				}
				strcat( buf, "\r\n" );
				clientf( buf );
				break;
			}
		}
	}
	clientff(C_R"["C_G"%d"C_W" rooms found."C_R"]\r\n"C_0, count);
}

void do_area_search( char *arg )
{
	if ( !current_area )
	{
		clientfr( "No current area." );
		return;
	}
	if ( !strcmp(arg,"help") )
	{clientfr("Area Search Help:");
		clientff(" area search Help     - This help file.\r\n"
				" area search nostop   - Will toggle stopping on finding what is searched for.\r\n"
				" area search stop     - Will stop the search.\r\n"
				" area search continue - Will continue a stopped search.\r\n"
				" area search skip - Will skip past any room you can't enter.\r\n");
		return;}
	if ( !strcmp( arg, "skip" ) )
	{
		if ( search_room && search_room->next_in_area )
		{
			area_search = 1;
			search_room = search_room->next_in_area;
			init_openlist( NULL );
			init_openlist( search_room );
			path_finder( );
			clientfr( "Skipped one room." );
			go_next( );
		}
		else
		{clientfr( "Skipping a room is not possible." );
			area_search = 0;
			init_openlist( NULL );
			memset(area_search_for, 0, sizeof(area_search_for));
			search_room = NULL;
		}
		return;
	}
	if ( !strcmp( arg, "nostop") ) {
		if ( !searchnostop )
		{clientfr("Search will now not stop on found");searchnostop=1;return;}
		else
		{clientfr("Search will now stop on found");searchnostop=0;return;}
	}
	if ( !strcmp( arg, "continue" ) && !area_search )
	{
		if ( !search_room )
			clientfr( "Unable to continue searching." );
		else
		{
			clientfr( "searching continues." );

			area_search = 1;

			search_room = search_room->next_in_area;
			init_openlist( NULL );
			init_openlist( search_room );
			path_finder( );
			go_next( );
		}

		return;
	}
	if ( !strcmp(arg, "stop") )
	{area_search = 0;
		init_openlist( NULL );
		memset(area_search_for, 0, sizeof(area_search_for));
		search_room = NULL;
		clientfr("Searching Stopped");
		return;
	}


	if ( !current_room )
	{
		clientfr( "No current room set." );
		return;
	}

	if ( !arg[0] )
	{
		clientf(C_W"usage: "C_R"Area search (search word)\r\n"C_0);
		clientf("     "C_W": "C_R"Area search nostop, to toggle on/off if the search\r\n"C_0);
		clientf("     "C_W": "C_R"should stop upon finding what you search for\r\n"C_0);
		clientf("     "C_W": "C_R"Area search continue, will continue if stopped at found\r\n"C_0);
		return;
	}

	memset(area_search_for, 0, sizeof(area_search_for));
	strncpy(area_search_for, arg, strlen(arg));

	clientff( C_W"Searching begins, looking for: %s.\r\n"C_0, area_search_for );

	area_search = 1;

	search_room = current_room->area->rooms;
	init_openlist( NULL );
	init_openlist( search_room );
	path_finder( );
	go_next( );
}

void do_area_orig( char *arg )
{
	ROOM_DATA *room, *r;
	int rooms = 0, original_rooms = 0;

	if ( !current_area )
	{
		clientfr( "No current area." );
		return;
	}

	for ( room = current_area->rooms; room; room = room->next_in_area )
	{
		/* Add one to the room count. */
		rooms++;

		/* And check if there is any other room with this same name. */
		original_rooms++;
		for ( r = current_area->rooms; r; r = r->next_in_area )
			if ( !strcmp( r->name, room->name ) && r != room )
			{
				original_rooms--;
				break;
			}
	}

	if ( !rooms )
	{
		clientfr( "The area is empty..." );
		return;
	}

	clientff( C_R "[Rooms: " C_G "%d" C_R "  Original rooms: " C_G "%d" C_R
			"  Originality: " C_G "%d%%" C_R "]\r\n" C_0,
			rooms, original_rooms,
			original_rooms ? original_rooms * 100 / rooms : 0 );
}

void do_area_types( char *arg )
{
	ROOM_DATA *room;
	ROOM_TYPE *type;
	int typec;
	int rtotal = 0;
	double percent,cr,ct;
	if ( !current_room )
	{clientfr("No room set");
		return;}
	for ( room = current_room->area->rooms; room; room = room->next_in_area )
		rtotal++;
	clientff( C_R"  [Area room type count out total %d Rooms]\r\n",rtotal);
	for ( type = room_types; type; type = type->next ) {
		typec = 0;
		for ( room = current_room->area->rooms; room; room = room->next_in_area ) {
			if ( room->room_type->name == type->name )
				typec++;
		}
		if ( typec ) {
			ct = typec;
			cr = rtotal;
			percent = ct/cr*100.00;
			clientff(" %s%-30s"C_W": %4d "C_0"Rooms. %6.2f%% of Total.\r\n", type->color, type->name, typec, percent );
		}
	}
}


void do_map_orig( char *arg )
{
	ROOM_DATA *room, *r;
	int rooms = 0, original_rooms = 0;

	for ( room = world; room; room = room->next_in_world )
	{
		/* Add one to the room count. */
		rooms++;

		/* And check if there is any other room with this same name. */
		original_rooms++;
		for ( r = world; r; r = r->next_in_world )
			if ( !strcmp( r->name, room->name ) && r != room )
			{
				original_rooms--;
				break;
			}
	}

	if ( !rooms )
	{
		clientfr( "The area is empty..." );
		return;
	}

	clientff( C_R "[Rooms: " C_G "%d" C_R "  Original rooms: " C_G "%d" C_R
			"  Originality: " C_G "%d%%" C_R "]\r\n" C_0,
			rooms, original_rooms,
			original_rooms ? original_rooms * 100 / rooms : 0 );
}



void do_map_file( char *arg )
{
	if ( !arg[0] )
	{
		strcpy( map_file, "IMap" );
		strcpy( map_file, "IMap.bin" );
	}
	else
	{
		strcpy( map_file, arg );
		strcpy( map_file_bin, arg );
		strcat( map_file_bin, ".bin" );
	}

	clientff( C_R "[File for map load/save set to '%s'.]\r\n" C_0, map_file );
}



/* Snatched from do_exit_special. */
void do_map_teleport( char *arg )
{
	EXIT_DATA *spexit;
	char cmd[512];
	char buf[512];
	int i, nr = 0;
	char buf2[256], *b = buf2;
	char *p;

	DEBUG( "do_map_teleport" );

	if ( mode != CREATING && strcmp( arg, "list" ) )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	if ( !strncmp( arg, "help", strlen( arg ) ) )
	{
		clientfr( "Syntax: map teleport <command> [exit] [args]" );
		clientfr( "Commands: list, add, create, destroy, name, message, link" );
		return;
	}

	arg = get_string( arg, cmd, 512 );

	if ( !strcmp( cmd, "list" ) )
	{
		clientfr( "Global special exits:" );
		for ( spexit = global_special_exits; spexit; spexit = spexit->next )
		{
			sprintf( buf, C_B "%3d" C_0 " - L: " C_G "%4d" C_0 " N: '" C_g "%s" C_0 "' M: '" C_g "%s" C_0 "'\r\n",
					nr++, spexit->vnum, spexit->command, spexit->message );
			clientf( buf );
		}

		if ( !nr )
			clientf( " - None.\r\n" );
	}

	else if ( !strcmp( cmd, "add" ) )
	{
		char name[512];

		arg = get_string( arg, name, 512 );

		if ( !arg[0] )
		{
			clientfr( "Syntax: map teleport add <name> <message>" );
			return;
		}

		spexit = create_exit( NULL );
		spexit->command = strdup( name );
		p = arg;
		while ( *p )
		{
			if ( *p == '"' )
				*(b++) = '*';
			else
				*(b++) = *p;
			p++;
		}
		*b = 0;
		spexit->message = strdup( buf2 );

		clientfr( "Global special exit created." );
	}

	else if ( !strcmp( cmd, "create" ) )
	{
		create_exit( NULL );
		clientfr( "Global special exit created." );
	}

	else if ( !strcmp( cmd, "destroy" ) )
	{
		int done = 0;

		if ( !arg[0] || !isdigit( arg[0] ) )
		{
			clientfr( "What global special exit do you wish to destroy?" );
			return;
		}

		nr = atoi( arg );

		for ( i = 0, spexit = global_special_exits; spexit; spexit = spexit->next )
		{
			if ( i++ == nr )
			{
				destroy_exit( spexit );
				done = 1;
				break;
			}
		}

		if ( done )
			sprintf( buf, "Global special exit %d destroyed.", nr );
		else
			sprintf( buf, "Global special exit %d was not found.", nr );
		clientfr( buf );
	}

	else if ( !strcmp( cmd, "message" ) )
	{
		int done = 0;

		/* We'll store the number in cmd. */
		arg = get_string( arg, cmd, 512 );

		if ( !isdigit( cmd[0] ) )
		{
			clientfr( "Set message on what global special exit?" );
			return;
		}

		nr = atoi( cmd );

		for ( i = 0, spexit = global_special_exits; spexit; spexit = spexit->next )
		{
			if ( i++ == nr )
			{
				if ( spexit->message )
					free( spexit->message );
				p = arg;
				while ( *p )
				{
					if ( *p == '"' )
						*(b++) = '*';
					else
						*(b++) = *p;
					p++;
				}
				*b = 0;
				spexit->message = strdup( buf2 );
				done = 1;
				break;
			}
		}

		if ( done )
			sprintf( buf, "Message on global exit %d changed to '%s'", nr, buf2 );
		else
			sprintf( buf, "Can't find global special exit %d.", nr );

		clientfr( buf );
	}

	else if ( !strcmp( cmd, "name" ) )
	{
		int done = 0;

		/* We'll store the number in cmd. */
		arg = get_string( arg, cmd, 256 );

		if ( !isdigit( cmd[0] ) )
		{
			clientfr( "Set name on what global special exit?" );
			return;
		}

		nr = atoi( cmd );

		for ( i = 0, spexit = global_special_exits; spexit; spexit = spexit->next )
		{
			if ( i++ == nr )
			{
				if ( spexit->command )
					free( spexit->command );

				if ( arg[0] )
					spexit->command = strdup( arg );
				done = 1;
				break;
			}
		}

		if ( done )
		{
			if ( arg[0] )
				sprintf( buf, "Name on global exit %d changed to '%s'", nr, arg );
			else
				sprintf( buf, "Name on global exit %d cleared.", nr );
		}
		else
			sprintf( buf, "Can't find global special exit %d.", nr );

		clientfr( buf );
	}

	else if ( !strcmp( cmd, "link" ) )
	{
		/* E special link 0 -1 */
		char vnum[256];

		/* We'll store the number in cmd. */
		arg = get_string( arg, cmd, 256 );

		if ( !isdigit( cmd[0] ) )
		{
			clientfr( "Link which global special exit?" );
			return;
		}

		nr = atoi( cmd );

		for ( i = 0, spexit = global_special_exits; spexit; spexit = spexit->next )
		{
			if ( i++ == nr )
			{
				get_string( arg, vnum, 256 );

				if ( vnum[0] == '-' )
				{
					ROOM_DATA *to;

					to = spexit->to;

					spexit->vnum = -1;
					spexit->to = NULL;

					check_pointed_by( to );

					sprintf( buf, "Link cleared on exit %d.", nr );
					clientfr( buf );
					return;
				}
				else if ( isdigit( vnum[0] ) )
				{
					spexit->vnum = atoi( vnum );
					spexit->to = get_room( spexit->vnum );
					if ( !spexit->to )
					{
						clientfr( "A room whith that vnum was not found." );
						spexit->vnum = -1;
						return;
					}
					sprintf( buf, "Global special exit %d linked to '%s'.",
							nr, spexit->to->name );
					clientfr( buf );
					return;
				}
				else
				{
					clientfr( "Link to which vnum?" );
					return;
				}
			}
		}

		sprintf( buf, "Can't find global special exit %d.", nr );
		clientfr( buf );
	}

	else
	{
		clientfr( "Unknown command... Try 'map teleport help'." );
	}
}




void do_area_create( char *arg )
{
	AREA_DATA *new_area;

	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	new_area = create_area( );

	new_area->name = strdup( "New area" );

	current_area = new_area;

	clientfr( "Area created. Next room will be in that area." );
}



void do_area_list( char *arg )
{
	AREA_DATA *area;
	ROOM_DATA *room;
	const int align = 37;
	char spcs[256];
	char buf[1024];
	int space = 0;
	int right = 1;
	int i;

	DEBUG( "do_area_list" );
	if ( arg[0] ) {
		area = get_area_by_name( arg );
		if ( area && area->name )
			clientff(C_R"[Single Area found: %s]\r\n"C_0,area->name);
	}
	else
	{
		clientfr( "Areas:" );
		for ( area = areas; area; area = area->next )
		{
			int unlinked = 0, notype = 0;

			for ( room = area->rooms; room; room = room->next_in_area )
			{
				if ( !notype && room->room_type == null_room_type )
					notype = 1;
				if ( !unlinked )
					for ( i = 1; dir_name[i]; i++ )
						if ( !room->exits[i] && room->detected_exits[i] &&
								!room->locked_exits[i] && !room->hasrandomexits )
							unlinked = 1;
			}

			if ( !right )
			{
				space = align - strlen( area->name );
				spcs[0] = '\r', spcs[1] = '\n', spcs[2] = 0;
				right = 1;
			}
			else
			{
				for ( i = 0; i < space; i++ )
					spcs[i] = ' ';
				spcs[i] = 0;
				right = 0;
			}
			clientf( spcs );

			sprintf( buf, " (%s%c" C_0 ") %s%s%s",
					notype ? C_R : ( unlinked ? C_G : C_B ),
					notype ? 'x' : ( unlinked ? 'l' : '*' ),
					area == current_area ? C_W : ( area->disabled ? C_D : "" ),
					area->name, area == current_area ? C_0 : "" );
			clientf( buf );
		}
		clientf( "\r\n" );
		clientff("\r\n "C_0"("C_D"x"C_0") - Rooms in area with no type.\r\n "C_0"("C_G"l"C_0") - Unlinked rooms in area.\r\n "C_D"Dark Text "C_0"- "C_D"Pathfinding through area is turned off."C_0"\r\n");
	}
}



void do_area_switch( char *arg )
{
	AREA_DATA *area;
	char buf[256];

	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	if ( !current_room )
	{
		clientfr( "No current room, to change." );
		return;
	}

	/* Move in a circular motion. */
	area = current_room->area->next;
	if ( !area )
		area = areas;
	if ( current_room->area )
		unlink_from_area( current_room );
	link_to_area( current_room, area );
	current_area = current_room->area;

	sprintf( buf, "Area switched to '%s'.",
			current_area->name );
	clientfr( buf );
}



void do_area_update( char *arg )
{
	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	if ( update_area_from_survey )
	{
		update_area_from_survey = 0;
		clientfr( "Disabled." );
	}
	else
	{
		update_area_from_survey = 1;
		clientfr( "Type survey. The current area's name will be updated." );
	}
}


void do_area_sfree( )
{
	if ( !current_area )
	{
		clientfr( "No current area set." );
		return;
	}
	int i,j,l = 0;
	ROOM_DATA *room;
	/* cleanup from previous checks */
	for ( room = current_area->rooms ; room; room = room->next_in_area )
		room->shrineblock = 0;

	for ( room = current_area->rooms ; room; room = room->next_in_area ) {
		if ( room->shrine )
			room->shrineblock = 1;
		for ( i = 1; i < 14; i++ ) {
			if ( room->exits[i] ) {
				if ( room->exits[i]->shrine )
					room->shrineblock = 1;
				for ( j = 1; j < 14; j++ ) {
					if ( room->exits[i] && room->exits[i]->exits[j] && room->exits[i]->exits[j]->shrine )
						room->shrineblock = 1;
				}
			} }
	}
	clientff(C_W"Possible rooms for shrines\r\n"C_0);
	for ( room = current_area->rooms ; room; room = room->next_in_area )
	{
		if ( !room->shrineblock ) {
			clientff(" "C_C"- "C_D"("C_G"%d"C_D") "C_y"%s\r\n"C_0,room->vnum, room->name );
			l++;
			room->tmark = 1;
		}
	}
	clientff(C_W"["C_G"%d "C_c"Possible rooms for shrines, these have been temp marked"C_W"]\r\n"C_0,l);
}

void do_area_info( char *arg )
{
	EXIT_DATA *spexit;
	ROOM_DATA *room;
	ELEMENT *tag;
	char buf[256];
	char roomtags[256];
	int detected_only = 0, unknown_type = 0;
	int rooms = 0, i, nr = 0;

	DEBUG( "do_area_info" );

	if ( !current_area )
	{
		clientfr( "No current area set." );
		return;
	}

	sprintf( buf, "Area: %s", current_area->name );
	clientfr( buf );

    if (current_area->note)
    {
        sprintf(buf, "Area Note: %s",current_area->note );
        clientfr(buf);
    }

	for ( room = current_area->rooms; room; room = room->next_in_area )
	{
		rooms++;

		if ( room->room_type == null_room_type )
			unknown_type++;
		for ( i = 1; dir_name[i]; i++ )
			if ( room->detected_exits[i] && !room->exits[i] &&
					!room->locked_exits[i] )
				detected_only++;
	}
	sprintf( buf, C_R "Rooms: " C_G "%d" C_R "  Unlinked exits: " C_G
			"%d" C_R "  Unknown type rooms: " C_G "%d" C_R "  Disabled: " C_G "%s" C_R "  NoWings: " C_G "%s" C_R ".",
			rooms, detected_only, unknown_type,  current_area->disabled ? "Yes" : "No", current_area->nowingarea ? "Yes" : "No" );
	clientfr( buf );

	if ( !strcmp( arg, "extra" ) ) {
		clientff(C_R"[Special Exits.]\r\n");
		for ( room = current_area->rooms; room; room = room->next_in_area )
		{
			rooms++;
			for ( spexit = room->special_exits; spexit; spexit = spexit->next )
			{
				clientff( C_D "  ("C_G "%5d" C_D ")"C_Y " -" C_y "%-40.40s" C_Y "-" C_Y
						"-->" C_W " %-37.37s" C_0 "\r\n" C_Y "->" C_C "("C_G "%5d" C_C ") "
						C_c"%-40.40s" C_Y " in  " C_R "(" C_g "%s" C_R ")" C_0 "\r\n",
						spexit->owner->vnum, spexit->owner->name, spexit->command, spexit->vnum, spexit->to->name, spexit->to->area->name );
				nr++;
			}
		}
		clientff(C_R "[ " C_G "%d " C_R "Special Exits found in this area.]\r\n" C_0, nr);
		clientff(C_R"[Wormholes]\r\n"C_0);
		for ( room = current_area->rooms; room; room = room->next_in_area )
		{ if ( room->worm_pointed_by )
			clientff("  "C_Y"*"C_D" ("C_G"%5d"C_D") "C_y"%-40.40s "C_R"("C_g"%s"C_R")\r\n"C_W" -> "C_D"("C_G"%5d"C_D") "C_y"%-40.40s "C_R"("C_g"%s"C_R")\r\n"C_0,
					room->vnum, room->name, room->area->name ,room->wormhole->vnum, room->wormhole->to->name, room->wormhole->to->area->name );
		}
		clientff(C_R "[Area Tags]\r\n");
		for ( room = current_area->rooms ; room; room = room->next_in_area ) {
			if ( room->tags && !strcmp( arg, "extra" )  )
			{
				for ( tag = room->tags; tag; tag = tag->next )
				{
					strcat( roomtags, " " );
					strcat( roomtags, (char *) tag->p );
				}
				clientff( C_D " ("C_G"%5d"C_D")" C_y " %s " C_R "-" C_W "%s" "\r\n" C_0,
						room->vnum, room->name, roomtags );
				sprintf( roomtags, C_W);
			}
		}
		void do_room_info( char *arg );
		do_room_info( "" );
	}
	if ( !strncmp( arg, "shrine", 6) ) {
		ROOM_DATA *roomd, *dcheck;
		int totcov = 0, totrooms = 0;
		int d,dshrine,dcov, divcolor;
		char div[256],done[512];
		double ct = totcov,cr = totrooms,perc;
		memset(div, '\0' , 256);
		memset(done, '\0' , 512);

		for ( roomd = current_area->rooms; roomd; roomd = roomd->next_in_area )
		{ totrooms++;
			if ( roomd->shrine || roomd->shrineparrent )
				totcov++;
		}
		ct = totcov;
		cr = totrooms;
		if ( ct == 0 && cr == 0 )
			perc = 0;
		else
			perc = ct/cr*100.00;
		clientff(C_W"Total area shrine coverage: "C_G"%6.2f%%\r\n"C_0, perc);
		for ( d = 0; d < 20 ; d++ ) {
			dshrine = 0;
			dcov = 0;
			divcolor = 0;
			strncpy(div," ",strlen(div));
			for ( roomd = current_area->rooms; roomd; roomd = roomd->next_in_area )
			{  if ( !strncmp(div," ",strlen(div)) && roomd->shrinedivine && !strstr(done, roomd->shrinedivine) ) {
																													 strncpy(div, roomd->shrinedivine, strlen(roomd->shrinedivine) );
																												 }
			}
			/* actual calculations here */
			for ( roomd = current_area->rooms; roomd; roomd = roomd->next_in_area )
				if ( ( strncmp(div," ",strlen(div)) && roomd->shrinedivine && !strncmp(div, roomd->shrinedivine,strlen(div)) ) ||
						( roomd->shrineparrent && ( dcheck = get_room( roomd->shrineparrent ) ) && !strncmp(div, dcheck->shrinedivine, strlen(div)) ) ) {
					dcov++;
					if ( strncmp(div," ",strlen(div)) && roomd->shrinedivine && !strncmp(div, roomd->shrinedivine,strlen(div)) ) {
						dshrine++;
						divcolor = roomd->shrineeff;
					}
				}
			strcat(done, div);
			strcat(done, " ");
			ct = dcov;
			cr = totrooms;
			if ( ct == 0 && cr == 0 )
				perc = 0;
			else
				perc = ct/cr*100.00;
			if ( strncmp(div," ",strlen(div)) )
				clientff("Shrines of %s%10.10s "C_0"cover %6.2f%% with %3d shrines, covering %4d rooms.\r\n"C_0,divcolor == 1 ? C_W"" : divcolor == 3 ? C_R"" : C_G"",div, perc, dshrine,dcov);
		}
		if ( strstr( arg, "setup" ) )
			do_area_sfree();
	}
}


void do_area_conn ( char *arg )
{
	ROOM_DATA *r;
	EXIT_DATA *spexit;
	int i;

	if ( !current_room || !current_area )
	{
		clientfr("no room set");
		return;}

	clientff(C_R "[Areas connected to area %s]\r\n" C_0, current_area->name );
	for ( r = current_room->area->rooms; r; r = r->next_in_area ) {
		for ( i = 1; i < 14; i++ ) {
			if ( r->exits[i] &&  r->exits[i]->area->name != current_area->name )
				clientff(C_y "%-30.30s "C_D"("C_G"%d"C_D") "C_W"connects to area "C_R"("C_g"%s"C_R")\r\n" C_0, r->name, r->vnum, r->exits[i]->area->name );}
		for ( spexit = r->special_exits; spexit; spexit = spexit->next )
			if ( spexit->message && spexit->vnum != -1 && spexit->to->area->name != current_area->name )
				clientff(C_y "%-30.30s "C_D"("C_G"%d"C_D") "C_W"connects to area "C_R"("C_g"%s"C_R")\r\n" C_0, r->name, r->vnum, spexit->to->area->name );
	}
}


void do_area_off( char *arg )
{
	AREA_DATA *area;
	char buf[256];

	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	area = get_area_by_name( arg );

	if ( !area )
		return;

	area->disabled = area->disabled ? 0 : 1;

	if ( area->disabled )
		sprintf( buf, "Pathfinding in '%s' disabled.", area->name );
	else
		sprintf( buf, "Pathfinding in '%s' re-enabled.", area->name );

	clientfr( buf );
}




/* Room commands. */

void do_room_help( char *arg )
{
	clientfr( "Module: IMapper. Room commands:" );
	clientf( " room switch  - Switch current room to another vnum.\r\n"
			" room create    - Create a new room and switch to it.\r\n"
			" room look      - Show info on the current room only.\r\n"
			" room info      - List all rooms that weren't properly mapped.\r\n"
			" room find      - Find all rooms that contain something in their name.\r\n"
			" room destroy   - Unlink and destroy a room.\r\n"
			" room list      - List all rooms in the current area.\r\n"
			" room tag       - Mark a room with a tag.\r\n"
			" room underw    - Set the room as Underwater.\r\n"
			" room mark      - Set or clear a landmark on a vnum, or current room.\r\n"
			" room types     - List all known room types.\r\n"
			" room merge     - Combine two identical rooms into one.\r\n"
			" room addn      - No arguement gives help for editing additional room names.\r\n"
			" room shop      - Mark this room as a shop.\r\n"
			" room avoid #   - Marks the room to be avoided by pathing.\r\n"
			" room random    - Followed by 'help' will give you the information on how the mapper handles random rooms.\r\n"
			" room tm        - tm or tempmark, allows you to mark a room temporarely, this mark isn't saved.\r\n"
			" room briefinfo - Gives brief information on the room: name, room type, vnum, temp mark.\r\n"
			" room clearowner- Clears the owner of this room.\r\n"
			" room noexit    - Gives you the help of the room noexit handling.\r\n");
}


void do_map_scalc( char *divine )
{
	ROOM_DATA *room,*pcheck;
	int roomtot = 0;
	int shrinetot = 0;
	int shrinecov = 0;
	int shrinenum = 0;
	ROOM_TYPE *type;
	int typec;
	double percent,cr,ct;

	for ( room = world; room; room = room->next_in_world ) {
		roomtot++;
		if ( room->shrine && !strncmp(room->shrinedivine, divine, strlen( divine )) )
			shrinetot++;}

	for ( room = world; room; room = room->next_in_world ) {
		pcheck = NULL;
		if ( room->shrineparrent )
			pcheck = get_room ( room->shrineparrent );
		if ( ( room->shrinedivine && !strncmp(room->shrinedivine, divine, strlen( divine )) ) || ( pcheck && pcheck->shrinedivine && !strncmp(pcheck->shrinedivine , divine, strlen( divine )) ) )
			shrinecov++;
	}

	/* shrine room types */
	clientff(C_y"%s Shrine Destribution:\r\n"C_0, divine);
	for ( type = room_types; type; type = type->next ) {
		typec = 0;
		shrinenum = 0;
		pcheck = NULL;
		for ( room = world; room; room = room->next_in_world ) {
			if ( room->shrineparrent )
				pcheck = get_room ( room->shrineparrent );
			if ( room->room_type->name == type->name ) {
				if ( room->shrinedivine && !strncmp(room->shrinedivine, divine, strlen( divine )) ) {
					shrinenum++;
					typec++;}
				if ( pcheck && pcheck->shrinedivine && !strncmp(pcheck->shrinedivine , divine, strlen( divine )) )   {
					typec++;
					pcheck = NULL;
				}
			}
		}
		if ( typec ) {
			ct = typec;
			cr = shrinecov;
			percent = ct/cr*100.00;
			clientff(" %s%-30s"C_W": %4d "C_0"Rooms. %6.2f%% of shrine coverage. %d Shrines.\r\n", type->color, type->name, typec, percent,shrinenum );
		}
	}
	if ( shrinetot ) {
		ct = shrinecov;
		cr = roomtot;
		percent = ct/cr*100.00;
		clientff(C_W"Total world shrine coverage: "C_G"%6.2f%%\r\n"C_0,percent);
	}
}



void do_map_shrine( char *arg )
{
	char troom[64];
	char size[64];
	char status[64];
	char dname[64];
	char shrinetype[64];
	char fullcmd[256];
	int vnum = 0, i,j, c;
	int hidden;
	int update = 0;
	short int sharea = 0;
	char buf[256];
	ROOM_DATA *room = NULL, *r, *rc, *rt;

	hidden = 0;
	if ( !strcmp( arg, "help" ) || arg[0] == 0 )
	{/* new setup with monuments needed
		map shrine divine large/medium/small master/monument/shrine neutral/friendly/enemy */
		clientfr( "Map Shrine commands" );
		clientf("map shrine divine large/medium/small master/monument/shrine neutral/friendly/enemy\r\n"
				"map shrine # divine large/medium/small master/monument/shrine neutral/friendly/enemy\r\n"
				"map shrine [#]            : divine alone will automaticly assume large friendly\r\n"
				"                          : shrine\r\n"
				"                          : missing an entry of shrine missing an entry of\r\n"
				"                          : large/medium/small minor/major\r\n"
				"                          : neutral/friendly/enemy, will automaticly assume large\r\n"
				"                          : and friendly.\r\n"
				"map shrine help           : will give you this help\r\n"
				"map shrine [#] destroy    : will destroy a shrine.\r\n"
				"map shrine list (opt)     : Will list all shrines you've marked in the world.\r\n"
				"                          : opt: area: Current Area. opt: divinename, list shrines\r\n"
				"                          : recorded to divine.\r\n"
				"map shrine info divine    : Statistic Information about the divine's shrines\r\n"
				"map shrine [#] master     : Turns the shrine in that room into a master shrine.\r\n"
				"map shrine [#] monument   : Turns the shrine in that room into a major shrine.\r\n"
				"map shrine [#] shrine     : Turns the shrine in that room into a minor shrine.\r\n"
				"map shrine showdivine     : Shows you the list of divine and your relation with them\r\n"
				"map shrine setdivine name : Will add 'name' to the list of divine and set their relation\r\n"
				"                          : to you, or if used on a divine that already exits, change their relation\r\n"
				"map shrine remdivine name : will destroy all shrines belonging to 'name' and remove the divine\r\n"
				"                          : from the divine list.\r\n"
				"map shrine sdivname name  : Manually set the name of the divine used for shrinesight\r\n"
				"                          : for usage with shrinesight of orders not your own, in letters and such.\r\n"
				"map shrine clear area     : Removes all shrines in this area.\r\n"
				"map shrine [#] update large/medium/small master/monument/shrine neutral/friendly/enemy\r\n"
				"The above mentioned will update a shrine, and ignore normal warnings.\r\n");
		return;
	}

	if ( isdigit( arg[0] ) ) {
		arg = get_string( arg, troom, 64 );
		vnum = atoi( troom );}

	if ( vnum && !(room = get_room(vnum)) )
	{clientfr("That room does not exist.");
		return;}
	if ( !vnum && current_room )
		room = current_room;
	if ( !room ) {
		clientfr("No valid room");
		return;
	}
	arg = get_string( arg, dname, 64 );
	if ( !strcmp(dname, "hide") ) {
		hidden = 1;
		arg = get_string( arg, dname, 64 );
	}
	if ( !strcmp(dname, "update") ) {
		update = 1;
		arg = get_string( arg, dname, 64 );
	}
	if ( !strcmp(dname,"sdivname") ) {
		get_string(arg, ssightdiv ,256);
		clientff(C_R"[%s will now be used for shrinesight when creating]\r\n[Beseech will set it to the god beseeched.]\r\n"C_0,ssightdiv);
		return;
	}
	strcpy(fullcmd,arg);
	arg = get_string( arg, size, 64 );
	arg = get_string( arg, shrinetype, 64);
	arg = get_string( arg, status, 64 );
	*dname = toupper(dname[0]);
	*size = tolower(size[0]);
	*shrinetype = tolower(shrinetype[0]);
	*status = tolower(status[0]);
	if (!strcmp(shrinetype, "ally"))
		strcpy(shrinetype,"friendly");

	if ( !strcmp( dname, "List" ) ) {
		if ( !strcmp(size, "area") && !shrinetype[0] ) {
			clientfr("Marked Divine Shrines in this area.");
			c = 0;
			DIVINE_DATA *d;
			for ( d = divinelist; d; d = d->next ) {
				for ( r = current_room->area->rooms; r; r = r->next_in_area ) {
					if ( r->shrine && !strncmp(r->shrinedivine,d->name,strlen(d->name)) ) {
						if ( !disable_mxp_map && mxp_tag( TAG_LOCK_SECURE ) )
							clientff(C_W "%s %s "C_W"to %s%10.10s"C_W" at "C_D"'"C_y"%25.25s"C_D"'"C_W" "C_D"("C_G"<mpelm v=%d r=\"%s\" t=\"%s\">%d</mpelm>"C_D") "C_R"("C_g"%20.20s"C_R")\r\n" C_0 , r->shrinesize == 1 ? C_R"Small" : r->shrinesize == 3 ? C_G"Large" : C_Y"Medium" ,r->shrinemajor ? r->shrinemajor==2 ? C_Y "Master" : C_C"Monument" : C_c"Shrine", r->shrine == 1 ? C_W"" : r->shrine == 3 ? C_R"" : C_G"" ,r->shrinedivine, r->name,r->vnum,r->name,r->room_type->name,r->vnum, r->area->name); else
								clientff(C_W "%s %s "C_W"to %s%10.10s"C_W" at "C_D"'"C_y"%25.25s"C_D"'"C_W" "C_D"("C_G"%d"C_D") "C_R"("C_g"%20.20s"C_R")\r\n" C_0 , r->shrinesize == 1 ? C_R"Small" : r->shrinesize == 3 ? C_G"Large" : C_Y"Medium" ,r->shrinemajor ? r->shrinemajor==2 ? C_Y "Master" : C_C"Monument" : C_c"Shrine", r->shrine == 1 ? C_W"" : r->shrine == 3 ? C_R"" : C_G"" ,r->shrinedivine, r->name, r->vnum, r->area->name);
					}
					c++;
				}
			}
			clientff(C_R"["C_W"Number of Shrines "C_G"%d"C_R"]\r\n"C_0,c);
			return;
		}
		else if ( !strcmp(size, "area") && shrinetype[0] )
		{  AREA_DATA *farea = NULL;
			AREA_DATA *get_area_by_name( char *string );
			sprintf(buf, "donotshow %s",fullcmd + 5);
			farea = get_area_by_name( buf );
			c = 0;
			if ( farea && farea->name )
			{
				clientff(C_R"[Marked Divine Shrines in the area: %s]\r\n"C_0,farea->name);
				DIVINE_DATA *d;
				for ( d = divinelist; d; d = d->next )
					for ( r = farea->rooms; r; r = r->next_in_area )
						if ( r->shrine && !strncmp(r->shrinedivine,d->name,strlen(d->name)) ) {
							{if ( !disable_mxp_map && mxp_tag( TAG_LOCK_SECURE ) )
								clientff(C_W "%s %s "C_W"to %s%10.10s"C_W" at "C_D"'"C_y"%25.25s"C_D"'"C_W" "C_D"("C_G"<mpelm v=%d r=\"%s\" t=\"%s\">%d</mpelm>"C_D") "C_R"("C_g"%20.20s"C_R")\r\n" C_0 , r->shrinesize == 1 ? C_R"Small" : r->shrinesize == 3 ? C_G"Large" : C_Y"Medium" ,r->shrinemajor ? r->shrinemajor==2 ? C_Y "Master" : C_C"Monument" : C_c"Shrine", r->shrine == 1 ? C_W"" : r->shrine == 3 ? C_R"" : C_G"" ,r->shrinedivine, r->name,r->vnum,r->name,r->room_type->name,r->vnum, r->area->name); else
									clientff(C_W "%s %s "C_W"to %s%10.10s"C_W" at "C_D"'"C_y"%25.25s"C_D"'"C_W" "C_D"("C_G"%d"C_D") "C_R"("C_g"%20.20s"C_R")\r\n" C_0 , r->shrinesize == 1 ? C_R"Small" : r->shrinesize == 3 ? C_G"Large" : C_Y"Medium" ,r->shrinemajor ? r->shrinemajor==2 ? C_Y "Master" : C_C"Monument" : C_c"Shrine", r->shrine == 1 ? C_W"" : r->shrine == 3 ? C_R"" : C_G"" ,r->shrinedivine, r->name, r->vnum, r->area->name);
							}
							c++;
						}
				clientff(C_R"["C_W"Number of Shrines "C_G"%d"C_R"]\r\n"C_0,c);
				return;
			}
			return;
		}
		else if ( isalpha( size[0] ) )
		{ *size = toupper(size[0]);
			clientff(C_R "[Searching for shrines belonging to %s]\r\n" C_0, size);
			c = 0;
			for ( r = world; r; r = r->next_in_world )
				if ( r->shrinedivine && !strncmp(r->shrinedivine, size, strlen( size ) ) )
				{if ( !cmp("extra", shrinetype ) )
					sharea=1;
					if ( !disable_mxp_map && mxp_tag( TAG_LOCK_SECURE ) && cmp("share", shrinetype ) )
						clientff(C_W "%s %s "C_W"to %s%10.10s"C_W" at "C_D"'"C_y"%25.25s"C_D"'"C_W" "C_D"("C_G"<mpelm v=%d r=\"%s\" t=\"%s\">%d</mpelm>"C_D") "C_R"("C_g"%20.20s"C_R")%s%s%s\r\n" C_0 , r->shrinesize == 1 ? C_R"Small" : r->shrinesize == 3 ? C_G"Large" : C_Y"Medium" ,r->shrinemajor ? r->shrinemajor==2 ? C_Y "Master" : C_C"Monument" : C_c"Shrine", r->shrine == 1 ? C_W"" : r->shrine == 3 ? C_R"" : C_G"" ,r->shrinedivine, r->name,r->vnum,r->name,r->room_type->name,r->vnum, r->area->name,sharea ? C_W"\r\n^---> Room Type: " : "",sharea ? r->room_type->color : "", sharea ? r->room_type->name : "" );
					else if ( !disable_mxp_map && mxp_tag( TAG_LOCK_SECURE ) && !cmp("share", shrinetype ) )
						clientff(C_W "%s %s "C_W"to %s%s"C_W" at "C_D"'"C_y"%s"C_D"' "C_R"("C_g"%s"C_R")\r\n" C_0 , r->shrinesize == 1 ? C_R"Small" : r->shrinesize == 3 ? C_G"Large" : C_Y"Medium" ,r->shrinemajor ? r->shrinemajor==2 ? C_Y "Master" : C_C"Monument" : C_c"Shrine", r->shrine == 1 ? C_W"" : r->shrine == 3 ? C_R"" : C_G"" ,r->shrinedivine, r->name, r->area->name);
					else
						clientff(C_W "%s %s "C_W"to %s%10.10s"C_W" at "C_D"'"C_y"%25.25s"C_D"'"C_W" "C_D"("C_G"%d"C_D") "C_R"("C_g"%20.20s"C_R")%s%s%s\r\n" C_0 , r->shrinesize == 1 ? C_R"Small" : r->shrinesize == 3 ? C_G"Large" : C_Y"Medium" ,r->shrinemajor ? r->shrinemajor==2 ? C_Y "Master" : C_C"Monument" : C_c"Shrine", r->shrine == 1 ? C_W"" : r->shrine == 3 ? C_R"" : C_G"" ,r->shrinedivine, r->name, r->vnum, r->area->name,sharea ? C_W"\r\n^---> Room Type:  " : "",sharea ? r->room_type->color : "", sharea ? r->room_type->name : "");
					c++;
				}
			clientff(C_R"["C_W"Number of Shrines "C_G"%d"C_R"]\r\n"C_0,c);
			return;}
		else {
			clientfr("Marked Divine Shrines in the world.");
			c = 0;
			DIVINE_DATA *d;
			for ( d = divinelist; d; d = d->next )
				for ( r = world; r; r = r->next_in_world )
					if ( r->shrine && !strncmp(r->shrinedivine,d->name,strlen(d->name)))
					{if ( !disable_mxp_map && mxp_tag( TAG_LOCK_SECURE ) )
						clientff(C_W "%s %s "C_W"to %s%10.10s"C_W" at "C_D"'"C_y"%25.25s"C_D"'"C_W" "C_D"("C_G"<mpelm v=%d r=\"%s\" t=\"%s\">%d</mpelm>"C_D") "C_R"("C_g"%20.20s"C_R")\r\n" C_0 , r->shrinesize == 1 ? C_R"Small" : r->shrinesize == 3 ? C_G"Large" : C_Y"Medium" ,r->shrinemajor ? r->shrinemajor==2 ? C_Y "Master" : C_C"Monument" : C_c"Shrine", r->shrine == 1 ? C_W"" : r->shrine == 3 ? C_R"" : C_G"" ,r->shrinedivine, r->name,r->vnum,r->name,r->room_type->name,r->vnum, r->area->name); else
							clientff(C_W "%s %s "C_W"to %s%10.10s"C_W" at "C_D"'"C_y"%25.25s"C_D"'"C_W" "C_D"("C_G"%d"C_D") "C_R"("C_g"%20.20s"C_R")\r\n" C_0 , r->shrinesize == 1 ? C_R"Small" : r->shrinesize == 3 ? C_G"Large" : C_Y"Medium" ,r->shrinemajor ? r->shrinemajor==2 ? C_Y "Master" : C_C"Monument" : C_c"Shrine", r->shrine == 1 ? C_W"" : r->shrine == 3 ? C_R"" : C_G"" ,r->shrinedivine, r->name, r->vnum, r->area->name);
						c++;
					}
			clientff(C_R"["C_W"Number of Shrines "C_G"%d"C_R"]\r\n"C_0,c);
			return;}}
	if ( !strcmp( dname, "Info" ) ) {
		if ( isalpha( size[0] ) )
		{ *size = toupper(size[0]);
			do_map_scalc( size );
			return;
		}
		else
		{
			clientfr("What divine?");
			return;
		}
	}
	if ( !strcmp(dname, "Setdivine") )
	{
		*size = toupper(size[0]);
       if ( shrinetype == NULL || shrinetype[0] == 0 )
			sprintf(shrinetype,"%s","neutral");
		for ( rc = world; rc; rc = rc->next_in_world )
			if ( rc->shrinedivine && !strcmp( rc->shrinedivine, size ) ) {
				if ( !strcmp(shrinetype, "neutral") )
					rc->shrine = 1;
				if ( !strcmp(shrinetype, "friendly") || !strcmp(shrinetype, "ally") )
					rc->shrine = 2;
				if ( !strcmp(shrinetype, "enemy") )
					rc->shrine = 3;
				for ( rt = world; rt; rt = rt->next_in_world )
					if ( rt->shrineparrent == rc->vnum )
						rt->shrineeff = rc->shrine;
			}
		add_divine(size,shrinetype);
		clientff(C_R"{You relations to the Divine %s are %s]\r\n"C_0,size,shrinetype);
		return;
	}
	if ( !strcmp(dname, "Showdivine") ) {
		DIVINE_DATA *d;
		clientfr("Divine list");
		for (d = divinelist; d; d = d->next )
			clientff( "You consider %s%s "C_0"%s %s"C_0" divine.\r\n"C_0, !strcmp(d->relation, "friendly") ? ""C_G : !strcmp(d->relation, "enemy") ? ""C_R : ""C_W ,d->name, !strcmp(d->relation, "friendly") ? "a"C_G : !strcmp(d->relation, "enemy") ? "an"C_R : "a"C_W , d->relation);
		return;
	}
	if ( !strcmp(dname, "Remdivine" ) )
	{
		*size = toupper(size[0]);
		clientff(C_R"[All shrines belonging to %s destroyed, and Divine deleted from divine list]\r\n"C_0, size);
		for ( r = world; r; r = r->next_in_world )
		{if ( r->shrinedivine && !strcmp( r->shrinedivine, size ) ) {
																		sprintf(buf, "%d hide destroy", r->vnum );
																		do_map_shrine( buf );
																	}
		}
		rem_divine(size);
		return;
	}
	if ( !strcmp(dname,"Clear") && !strcmp(size,"area") )
	{
		ROOM_DATA *rcc = NULL;
		char rcb[256];
		if ( !hidden )
			clientfr("Removing all shrines in this area");
		for ( rcc = current_room->area->rooms; rcc; rcc = rcc->next_in_area )
			if ( rcc->shrinedivine ) {
				sprintf(rcb, "%d hide destroy",rcc->vnum );
				do_map_shrine( rcb );
			}

		return;
	}
	if ( !strcmp(dname, "Monument") )
	{
		if ( !room->shrine )
		{clientfr("What Shrine?");return;}
		if ( room->shrinemajor==1 )
		{clientfr("That is already a monument");return;}
		room->shrinemajor = 1;
		clientfr("Shrine is now a monument");
		return;
	}
	if ( !strcmp(dname, "Shrine") )
	{
		if ( !room->shrine )
		{clientfr("What Shrine?");return;}
		if ( !room->shrinemajor )
		{clientfr("That is already a shrine");return;}
		room->shrinemajor = 0;
		clientfr("This is now A shrine");
		return;
	}
	if ( !strcmp(dname, "Master") )
	{
		if ( !room->shrine )
		{clientfr("What Shrine?");return;}
		if ( room->shrinemajor==2 )
		{clientfr("That shrine is already a master shrine");return;}
		room->shrinemajor = 2;
		clientfr("Shrine is now the Master shrine");
		return;
	}

	if ( strcmp(size,"small") && strcmp(size, "medium") && strcmp(size, "large") )
		sprintf(size,"large");
	if ( strcmp(shrinetype, "shrine") && strcmp(shrinetype, "monument") && strcmp(shrinetype, "master") )
		sprintf(shrinetype, "shrine");
	if ( strcmp(status, "friendly") && strcmp(status,"ally") && strcmp(status, "neutral") && strcmp(status, "enemy") ) {
		DIVINE_DATA *d;
		sprintf(status,"friendly");
		for ( d = divinelist; d ; d = d->next )
			if ( !strncmp(dname, d->name, strlen(d->name) ) ) {
				sprintf(status,"%s" ,d->relation);
			}
	}
	if ( dname[0] == 0 ) {
		clientfr("Yes, but what divine does the shrine belong to?");
		clientf("Syntax: map shrine [#] divine large/medium/small master/monument/shrine neutral/friendly/enemy\r\n");
		return;
	}


	if ( !strcmp(dname, "Destroy") )
	{
		if ( !room->shrine )
		{
			if ( !hidden )
				clientfr("There is no shrine there.");
			return;
		}
		room->shrine = 0;
		room->shrinemajor = 0;
		memset(room->shrinedivine, 0, sizeof(room->shrinedivine));
		for ( r = world; r; r = r->next_in_world )
			if ( r->shrineparrent == room->vnum || r == room)
			{
				r->shrineeff = 0;
				r->shrineparrent = 0;
			}
		if ( !hidden )
			clientfr("Shrine Destroyed");
		return;
	}

	DIVINE_DATA *t;
	int dfound = 0;
	for (t = divinelist;t;t = t->next)
	{
		if (!strcmp(t->name,dname))
			dfound = 1;
	}
	if (!dfound)
	{
		clientff(C_W"%s is not part of your list of divine\r\nUse SYNTAX: map shrine setdivine name friendly/neutral/enemy\r\nTo add %s to the list of divine\r\n"C_0,dname,dname);
		return;
	}


	if ( ( room->shrine || ( room->shrineeff && !disable_shrineradiuscheck ) ) && !update )
	{
		if ( !hidden ) {
			if ( room->shrine )
				clientfr("Room already has a shrine");
			else if (!disable_shrineradiuscheck)
				clientfr("Another shrine influences the area");}
		return;
	}
	if (!disable_shrineradiuscheck) {
	for ( i = 1; i < 14; i++ ) {
		if ( !update && room->exits[i] && (room->exits[i]->shrine ) ) {
			if ( !hidden )
				clientfr("The influence of another shrine is too close");
			return;} }
	}
	if ( update )
	{
		room->shrine = 0;
		room->shrinemajor = 0;

		for ( r = world; r; r = r->next_in_world )
			if ( r->shrineparrent == room->vnum || r == room)
			{
				r->shrineeff = 0;
				r->shrineparrent = 0;
			}
	}
	add_divine(dname,status);
	if ( !strcmp(status, "neutral") )
		room->shrine = 1;
	if ( !strcmp(status, "friendly") || !strcmp(status, "ally") )
		room->shrine = 2;
	if ( !strcmp(status, "enemy") )
		room->shrine = 3;
	if ( !strcmp(shrinetype, "monument") )
		room->shrinemajor = 1;
	if ( !strcmp(shrinetype, "master") )
		room->shrinemajor = 2;
	if ( room->shrine )
		room->shrinedivine = malloc(strlen(dname)+1); strcpy(room->shrinedivine, dname);
	room->shrinesize = 1;
	if ( room->shrine && (!strcmp(size, "medium") || !strcmp(size, "large")) )  {
		room->shrinesize = 2;
		for ( i = 1; i < 14; i++ ) {
			if ( room->exits[i] ) {
				room->exits[i]->shrineeff = room->shrine;
				room->exits[i]->shrineparrent = room->vnum;
			}
			if ( !strcmp(size, "large") ) {
				room->shrinesize = 3;
				for ( j = 1; j < 14; j++ )
					if ( room->exits[i] && room->exits[i]->exits[j] && !room->exits[i]->exits[j]->shrineeff && !room->exits[i]->exits[j]->shrineparrent ) {
						room->exits[i]->exits[j]->shrineeff = room->shrine;
						room->exits[i]->exits[j]->shrineparrent = room->vnum;
					}
			}
		}
	}
	if ( !hidden ) {
		clientff( C_R "[A %s %s %s belonging to %s has been marked %s%s]\r\n" C_0, size,shrinetype,
				status, dname, room == current_room ? "here." : "at ", room == current_room ? "" : room->name );
		clientfr("Remember to 'map config save' to save the shrine positions.");}
}

void do_room_switch( char *arg )
{
	ROOM_DATA *room;
	char buf[256];

	if ( !isdigit( *(arg) ) )
		clientfr( "Specify a vnum to switch to." );
	else
	{
		room = get_room( atoi( arg ) );
		if ( !room )
			clientfr( "No room with that vnum was found." );
		else
		{
			sprintf( buf, "Switched to '%s' (%s).", room->name,
					room->area->name );
			clientfr( buf );
			current_room = room;
			current_area = room->area;
			if ( mode == GET_UNLOST )
				mode = FOLLOWING;
		}
	}
}



void do_room_create( char *arg )
{
	ROOM_DATA *room;

	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	room = create_room( -1 );

	room->name = strdup( "New room." );

	current_room = room;
	current_area = room->area;
	clientfr( "New room created." );
}



void do_room_info( char *arg )
{
	char buf[256];
	int i;

	if ( !current_area )
		clientfr( "No current area." );
	else
	{
		ROOM_DATA *room;

		clientfr( "Unlinked or locked exits, no type rooms or rooms not connected:" );

		for ( room = current_area->rooms; room; room = room->next_in_area )
		{
			int unlinked = 0, notype = 0, locked = 0, noexits = 1;

			if ( room->room_type == null_room_type )
				notype = 1;
			if ( room->special_exits )
				noexits = 0;
			if ( room->hasrandomexits )
				noexits = 0;
			for ( i = 1; dir_name[i]; i++ ) {
				if ( room->exits[i] )
					noexits = 0;
				if ( !room->exits[i] && room->detected_exits[i] && !room->hasrandomexits )
				{
					if ( room->locked_exits[i] )
						locked = 1;
					else if ( !room->hasrandomexits )
						unlinked = 1;
				}
			}
			if ( unlinked || notype || locked || noexits )
			{
				sprintf( buf, " - %-35.35s"C_R" (" C_G "%d" C_R ")" C_0, room->name, room->vnum );
				if ( locked )
					strcat( buf, C_B " (" C_G "locked" C_B ")" C_0 );
				if ( unlinked )
					strcat( buf, C_B " (" C_G "unlinked" C_B ")" C_0 );
				if ( notype )
					strcat( buf, C_B " (" C_G "no type" C_B ")" C_0 );
				if ( noexits )
					strcat( buf, C_B " (" C_G "not connected" C_B ")" C_0 );
				strcat( buf, "\r\n" );
				clientf( buf );
			}
		}
	}
}

void do_room_briefinfo()
{
	if ( !current_room )
		return;
	gag_next_prompt = 1;
	clientff("RoomInfo: (%s) [%s] v%d, Temp Mark [%s]\r\n"C_0,current_room->name, current_room->room_type->name, current_room->vnum, current_room->tmark ? "X" : " ");
}

void do_room_find( char *arg )
{
	ROOM_DATA *room;
	char buf[512];
	int count = 0;

	if ( *(arg) == 0 )
	{
		clientfr( "Find what?" );
		return;
	}

	/* looking for an aetolia vnum? */
	if ( !strncmp(arg, "atv",3) )
	{  clientfr("All rooms with Aetolian Vnum");
		for ( room = world; room; room = room->next_in_world )
			if ( room->aetvnum > 0 ) {
				clientff( " " C_D "(" C_G "%5d" C_D ") "C_c"("C_y"%5d"C_c")" C_0 " %-50.50s%s%s%s%s%s%s%s%s%s\r\n",
						room->vnum,
						room->aetvnum > 0 ? room->aetvnum : 0,
						room->name,
						room->area != current_area ? " (" : "",
						room->area != current_area ? room->area->name : "",
						room->area != current_area ? ")" : "",
						room->additional_name[1] ? C_C" (" : "",
						room->additional_name[1] ? C_Y"A" : "",
						room->additional_name[1] ? C_C")" : "",
						room->owner ? C_C" Owner: " : "",
						room->owner ? C_W"" : "",
						room->owner ? room->owner : "" );
				count++;}
		clientff(C_R"["C_G"%d"C_W" rooms found."C_R"]\r\n"C_0, count);
		return;
	}
	/* Looking for a room type? */
	if ( !strncmp( arg, "type ", 5 ) )
	{
		ROOM_TYPE *type;

		for ( type = room_types; type; type = type->next )
			if ( !strcmp( type->name, arg+5 ) )
				break;

		if ( !type )
			clientfr( "What room type? Use 'room types' for a list." );
		else
		{
			clientfr( "Rooms that match:" );
			for ( room = world; room; room = room->next_in_world )
				if ( room->room_type == type ) {
					clientff( " " C_D "(" C_G "%5d" C_D ") "C_c"("C_y"%5d"C_c")" C_0 " %-50.50s%s%s%s%s%s%s%s%s%s\r\n",
							room->vnum,
							room->aetvnum > 0 ? room->aetvnum : 0,
							room->name,
							room->area != current_area ? " (" : "",
							room->area != current_area ? room->area->name : "",
							room->area != current_area ? ")" : "",
							room->additional_name[1] ? C_C" (" : "",
							room->additional_name[1] ? C_Y"A" : "",
							room->additional_name[1] ? C_C")" : "",
							room->owner ? C_C" Owner: " : "",
							room->owner ? C_W"" : "",
							room->owner ? room->owner : "" );
					count++;}
			clientff(C_R"["C_G"%d"C_W" rooms found."C_R"]\r\n"C_0, count);
		}

		return;
	}

	/* Looking for a room owner? */
	if ( !strncmp( arg, "owner", 5 ) || !strncmp( arg, "owned", 5 ) )
	{
		if ( !isalpha(arg[6]) && strncmp( arg, "owned", 5 ) )
		{
			clientfr("Yes, but what is the name of the owner?");
		}
		else
		{
			char name[256];
			strcpy(name,arg+6);
			*name = toupper(name[0]);

			if ( !strncmp( arg, "owned", 5 ) )
				sprintf(buf,"Rooms with owners in the world:");
			else
				sprintf(buf,"Rooms owned by %s in the world:",name);

			clientfr( buf );
			for ( room = world; room; room = room->next_in_world )
				if ( ( room->owner && !strncmp( arg, "owned", 5 ) ) || ( room->owner && !strncmp(room->owner,name,strlen(room->owner)) ) ) {
					clientff( " " C_D "(" C_G "%5d" C_D ") "C_c"("C_y"%5d"C_c")" C_0 " %-50.50s%s%s%s%s%s%s%s%s%s\r\n" C_0,
							room->vnum,
							room->aetvnum > 0 ? room->aetvnum : 0,
							room->name,
							room->area != current_area ? " (" : "",
							room->area != current_area ? room->area->name : "",
							room->area != current_area ? ")" : "",
							room->additional_name[1] ? C_C" (" : "",
							room->additional_name[1] ? C_Y"A" : "",
							room->additional_name[1] ? C_C")" : "",
							room->owner ? C_C" Owner: " : "",
							room->owner ? C_W"" : "",
							room->owner ? room->owner : ""   );
					count++;}
			clientff(C_R"["C_G"%d"C_W" rooms found."C_R"]\r\n"C_0, count);
		}

		return;
	}


	/* Looking for a room with x exits? */
	if ( !strncmp( arg, "exits", 5 ) )
	{
		if ( !isdigit(arg[6]) )
		{
			clientfr("Yes, but how many exits?");
		}
		else
		{
			int exitnumb,e,c;
			exitnumb = atoi(arg+6);

			sprintf(buf,"Rooms with %d exits in the world:",exitnumb);

			clientfr( buf );
			for ( room = world; room; room = room->next_in_world ) {
				c=0;
				for ( e = 1; dir_name[e]; e++ ) {
					if ( room->exits[e] || room->detected_exits[e] ) {
						c++;}}
				if ( c == exitnumb ) {
					sprintf( buf, " " C_D "(" C_G "%5d" C_D ") "C_c"("C_y"%5d"C_c")" C_0 " %-50.50s%s%s%s%s%s%s%s%s%s" C_0,
							room->vnum,
							room->aetvnum > 0 ? room->aetvnum : 0,
							room->name,
							room->area != current_area ? " (" : "",
							room->area != current_area ? room->area->name : "",
							room->area != current_area ? ")" : "",
							room->additional_name[1] ? C_C" (" : "",
							room->additional_name[1] ? C_Y"A" : "",
							room->additional_name[1] ? C_C")" : "",
							room->owner ? C_C" Owner: " : "",
							room->owner ? C_W"" : "",
							room->owner ? room->owner : ""
						   );
					count++;
					int i, first = 1;
					for ( i = 1; dir_name[i]; i++ )
					{
						if ( room->exits[i] || room->detected_exits[i] )
						{
							if ( first )
							{
								strcat( buf, C_D " (" );
								first = 0;
							}
							else
								strcat( buf, C_D "," );

							if ( room->exits[i] )
								strcat( buf, C_B );
							else
								strcat( buf, C_R );

							strcat( buf, dir_small_name[i] );
						}
					}
					if ( !first )
						strcat( buf, C_D ")" C_0 );
					strcat( buf, "\r\n" );
					clientf( buf );
				}
			}
			clientff(C_R"["C_G"%d"C_W" rooms found."C_R"]\r\n"C_0, count);
		}
		return;

	}

	clientfr( "Rooms that match:" );
	for ( room = world; room; room = room->next_in_world )
	{ int a;
		for ( a = 1; a < 10; a++ ) {
			if ( case_strstr( room->name, arg ) || (room->additional_name[a] != NULL &&
						case_strstr( room->additional_name[a], arg)))
			{
				sprintf( buf, " " C_D "(" C_G "%5d" C_D ") "C_c"("C_y"%5d"C_c")" C_0 " %-50.50s%s%s%s%s%s%s%s%s%s" C_0,
						room->vnum,
						room->aetvnum > 0 ? room->aetvnum : 0,
						room->name,
						room->area != current_area ? " (" : "",
						room->area != current_area ? room->area->name : "",
						room->area != current_area ? ")" : "",
						room->additional_name[1] ? C_C" (" : "",
						room->additional_name[1] ? C_Y"A" : "",
						room->additional_name[1] ? C_C")" : "",
						room->owner ? C_C" Owner: " : "",
						room->owner ? C_W"" : "",
						room->owner ? room->owner : ""
					   );
				count++;
				if ( mode == CREATING )
				{
					int i, first = 1;
					for ( i = 1; dir_name[i]; i++ )
					{
						if ( room->exits[i] || room->detected_exits[i] )
						{
							if ( first )
							{
								strcat( buf, C_D " (" );
								first = 0;
							}
							else
								strcat( buf, C_D "," );

							if ( room->exits[i] )
								strcat( buf, C_B );
							else
								strcat( buf, C_R );

							strcat( buf, dir_small_name[i] );
						}
					}
					if ( !first )
						strcat( buf, C_D ")" C_0 );
				}
				strcat( buf, "\r\n" );
				clientf( buf );
				break;
			}
		}
	}
	clientff(C_R"["C_G"%d"C_W" rooms found."C_R"]\r\n"C_0, count);
}



void do_room_look( char *arg )
{
	ROOM_DATA *room;
	char buf[256];
	int i;

	if ( arg[0] && isdigit( arg[0] ) )
	{
		if ( !( room = get_room( atoi( arg ) ) ) )
		{
			clientfr( "No room with that vnum found." );
			return;
		}
	}
	else
	{
		if ( current_room )
			room = current_room;
		else
		{
			clientfr( "No current room set." );
			return;
		}
	}

	sprintf( buf, "Room: %s  Vnum/AV: %d/%d.  Area: %s", room->name ?
			room->name : "-null-", room->vnum, room->aetvnum ? room->aetvnum : 0 ,room->area->name);
	clientfr( buf );
	sprintf( buf, "Type: %s.  Underwater: %s.  Pointed by: %s. Shop: %s. Avoid: %s. Random Exits: %s.", room->room_type->name,
			room->underwater ? "Yes" : "No", room->pointed_by ? "Yes" : "No" , room->shop ? "Yes" : "No", room->avoid ? "Yes" : "No", room->hasrandomexits ? "Yes" : "No" );
	clientfr( buf );
	if ( room->area->note ) {
		sprintf(buf, "Area Note: %s", room->area->note);
		clientfr(buf);}
	if ( room->owner != NULL )
	{sprintf(buf,"Owned by %s",room->owner );
		clientfr( buf );
	}
	if ( room->ruler )
	{sprintf(buf, "%s Rules over this place", room->ruler);
		clientfr( buf );
	}
	if ( room->shrine )
	{sprintf(buf,"Shrine Divine: %s. Shrine type: %s. Shrine size: %s.", room->shrinedivine,
			room->shrinemajor == 2 ? "Master" : room->shrinemajor == 1 ? "Monument" : "Shrine", room->shrinesize == 1 ? "Small" : room->shrinesize == 3 ? "Large" : "Medium"  );
	clientfr( buf );
	} else if ( room->shrineparrent )
	{ROOM_DATA *pcheck;
		pcheck = get_room( room->shrineparrent );
		if ( pcheck->vnum ) {
			sprintf(buf,"Affected by Shrine at vnum: %d. Shrine Divine: %s.",room->shrineparrent, pcheck->shrinedivine);
			clientfr( buf );
		}
	}
	for ( i = 1; i < 10; i++ )
	{
		if (room->additional_name[i] == NULL)
			break;
		else
		{
			sprintf( buf, "Other Names: %s", room->additional_name[i]);
			clientfr( buf );
		}
	}
	if ( room->tags )
	{
		ELEMENT *tag;

		sprintf( buf, "Tags:" );
		for ( tag = room->tags; tag; tag = tag->next )
		{
			strcat( buf, " " );
			strcat( buf, (char *) tag->p );
		}
		strcat( buf, "." );
		clientfr( buf );
	}

	for ( i = 1; dir_name[i]; i++ )
	{
		if ( room->exits[i] )
		{
			char lngth[128];
			char *ltr = NULL;
			if ( room->exit_length[i] )
				sprintf( lngth, " (%d)", room->exit_length[i] );
			else
				lngth[0] = 0;

			if  ( room->area->name != room->exits[i]->area->name )
			{ ltr = room->exits[i]->area->name;
			}
			clientff( "  %s: (%d) %s%s%s%s%s\r\n", dir_name[i],
					room->exits[i]->vnum,
					room->exits[i]->name, lngth, ltr != NULL ? " -> (" : "", ltr != NULL ? ltr : "", ltr != NULL ? ")" : "" );
			ltr = NULL;
		}
		else if ( room->detected_exits[i] )
		{
			if ( room->locked_exits[i] )
				clientff( "  %s: locked exit.\r\n", dir_name[i] );
			if ( room->hasrandomexits )
				clientff( "  %s: random exit.\r\n", dir_name[i] );
			else
				clientff( "  %s: unlinked exit.\r\n", dir_name[i] );
		}
	}
	if ( room->worm_pointed_by )
		clientff("  Wormhole: (%d) %s\r\n", room->wormhole->vnum, room->wormhole->to->name );
	/* Debugging.
	   sprintf( buf, "PF-Cost: %d. PF-Direction: %s.", room->pf_cost,
	   room->pf_direction ? dir_name[room->pf_direction] : "none" );
	   clientfr( buf );
	   sprintf( buf, "PF-Parent: %s", room->pf_parent ? room->pf_parent->name : "none" );
	   clientfr( buf );

	   for ( i = 1; dir_name[i]; i++ )
	   {
	   if ( room->reverse_exits[i] )
	   clientff( "  %s: %s\r\n", dir_name[i], room->reverse_exits[i]->name );
	   if ( room->more_reverse_exits[i] )
	   clientff( "  %s: " C_D "(more)\r\n" C_0, dir_name[i] );
	   }
	   */
}



void do_room_destroy( char *arg )
{
	int i;

	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	if ( !isdigit( *(arg) ) )
		clientfr( "Specify a room's vnum to destroy." );
	else
	{
		ROOM_DATA *room;

		room = get_room( atoi( arg ) );
		if ( !room )
		{
			clientfr( "No room with such a vnum was found." );
			return;
		}

		if ( room == current_room )
			clientfr( "Can't destroy the room you're currently in." );
		else
		{
			ROOM_DATA *r;
			EXIT_DATA *e, *e_next;
			if ( room->shrine )
			{ char dbuf[128];
				sprintf(dbuf, "%d hide destroy",room->vnum);
				do_map_shrine( dbuf );}
			/* We don't want pointers to point in unknown locations, don't we? */
			for ( i = 1; dir_name[i]; i++ )
				if ( room->exits[i] )
					set_reverse( NULL, i, room->exits[i] );
			//		 room->exits[i]->reverse_exits[i] = NULL;

			for ( r = world; r; r = r->next_in_world )
			{
				for ( i = 1; dir_name[i]; i++ )
					if ( r->exits[i] == room )
						r->exits[i] = NULL;
				for ( e = r->special_exits; e; e = e_next )
				{
					e_next = e->next;
					if ( e->to == room )
						destroy_exit( e );
				}
			}

			destroy_room( room );
			clientfr( "Room unlinked and destroyed." );
		}
	}
}



void do_room_list( char *arg )
{
	AREA_DATA *area;
	ROOM_DATA *room;
	char buf[256];

	area = get_area_by_name( arg );

	if ( !area || area->rooms == NULL )
		return;

	room = area->rooms;

	clientff( C_R "[Rooms in " C_B "%s" C_R "]\r\n" C_0,
			room->area->name );
	while( room )
	{
		sprintf( buf, " " C_D "(" C_G "%d" C_D ") "C_0" %s%s %s%s%s%s%s%s\r\n" C_0, room->vnum, room->avoid ? C_D : "",
				room->name ? room->name : "no name",
				room->additional_name[1] ? C_C" (" : "",
				room->additional_name[1] ? C_Y"A" : "",
				room->additional_name[1] ? C_C")" : "",
				room->owner ? C_C" Owner: " : "",
				room->owner ? C_W"" : "",
				room->owner ? room->owner : "" );
		clientf( buf );
		room = room->next_in_area;
	}
}

void do_room_underw( char *arg )
{
	if ( !current_room )
	{
		clientfr( "No current room set." );
		return;
	}

	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	current_room->underwater = current_room->underwater ? 0 : 1;

	if ( current_room->underwater )
		clientfr( "Current room set as an underwater place." );
	else
		clientfr( "Current room set as normal, with a nice sky (or roof) above." );
}

void do_room_clearowner( )
{
	if ( !current_room )
	{clientfr("No current room set.");
		return;}

	current_room->owner = NULL;
	clientff(C_R"[%s owner cleared]\r\n"C_0, current_room->name);
}

void do_room_types( char *arg )
{
	ROOM_TYPE *type, *t;
	char cmd[256], *p = arg, *color;

	if ( !current_room )
	{
		clientfr( "No current room set." );
		return;
	}

	type = current_room->room_type;

	if ( !arg[0] )
	{
		clientff( C_R "[This room's type is set as '%s%s" C_R "'.]\r\n"
				"[Must swim: %s. Underwater: %s.]\r\n"
				" Syntax: room type [colour] [swim] [underwater]\r\n"
				"     Or: room type list\r\n"
				" Example: room type bright-cyan swim (for a water room)\r\n" C_0,
				type->color ? type->color : C_R, type->name,
				type->must_swim ? "yes" : "no",
				type->underwater ? "yes" : "no" );
		return;
	}

	p = get_string( arg, cmd, 256 );
	while ( cmd[0] )
	{
		color = get_color( cmd );

		if ( !strcmp( cmd, "list" ) )
		{
			clientfr( "Room types known:" );

			clientff( "   %-20s Underwater  Swim\r\n", "Name" );
			for ( t = room_types; t; t = t->next )
			{
				if ( t->color )
					clientff( " - %s%-25s" C_0 "   %s   %s\r\n",
							t->color, t->name,
							t->underwater ? C_B "yes" C_0 : C_R " no" C_0,
							t->must_swim ? C_B "yes" C_0 : C_R " no" C_0 );
				else
					clientff( " - " C_R "%-34s" C_D "[[unset]]" C_0 "\r\n", t->name );
			}
		}
		else if ( !strcmp( cmd, "swim" ) || !strcmp( cmd, "water" ) )
		{
			type->must_swim = type->must_swim ? 0 : 1;
			clientff( C_R "[Environment '%s' will %s require swimming.]\r\n" C_0,
					type->name, type->must_swim ? "now" : C_r "no longer" C_R );
		}
		else if ( !strcmp( cmd, "underwater" ) )
		{
			type->underwater = type->underwater ? 0 : 1;
			clientff( C_R "[Environment '%s' is %s set as underwater.]\r\n" C_0,
					type->name, type->underwater ? "now" : C_r "no longer" C_R );
		}
		else if ( color )
		{
			type->color = color;
			clientff( C_R "[Environment '%s' will be shown as %s%s" C_R ".]\r\n" C_0,
					type->name, color, cmd );
		}
		else
		{
			clientff( C_R "[I don't know what %s means.]\r\n" C_0, cmd );
			clientfr( "If you tried to set a colour, check 'map colour' for a list!" );
		}

		p = get_string( p, cmd, 256 );
	}
}


void do_room_addn(char *arg )
{
	int a;

	if ( *(arg) == 0 ) {
		clientfr( "Room Addn commands:" );
		clientf( " room addn list       - Lists the other room names that the current room has.\r\n"
				" room addn add (name) - Adds an additional room name with the name of what you enter after add.\r\n"
				" room addn remove #   - Removes an additional room name, by its # from list.\r\n"
				" room addn makemain # - Makes the additional name # you entered from the list, the main room name.\r\n" );
		return;}


	if (case_strstr( arg, "list")) {
		if ( current_room == NULL ) {
			clientfr( "No current room set." );
			return;} else {
				clientff(C_R "[Additional Room Names of this room.]\r\n" C_0);
				for ( a = 1; a < 10; a++) {
					if ( current_room->additional_name[a] != NULL ) {
						clientff( C_R "("C_B"%d"C_R") "C_c"Name"C_W": "C_G"%s\r\n" C_0, a,
								current_room->additional_name[a] );
					}
				}
			}
	}
	else  if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	} else if (!strncmp( arg, "add", 3)) {
		for ( a = 1; a < 10; a++) {
			if ( current_room->additional_name[a] == NULL ) {
				current_room->additional_name[a] = strdup( arg+4 );
				if ( strrchr(current_room->additional_name[a],'.') == NULL )
				{ char buft[256];
					strcpy(buft,current_room->additional_name[a]);
					buft[strlen(buft)] = '.';
					buft[strlen(buft)+1] = 0;
					current_room->additional_name[a] = strdup( buft );
				}
				clientff(C_R"[\""C_G"%s"C_R"\" additional name added.]\r\n" C_0, current_room->additional_name[a] );
				break;
			}
		}
	}
	else  if (!strncmp( arg, "remove", 6) ) {
		if  ( current_room->additional_name[atoi( arg+7 )] == NULL ) {
			clientff(C_R"[No additional name found]\r\n"C_0); }
		else {
			current_room->additional_name[atoi( arg+7 )] = NULL;
			clientff(C_R "[Additional name %d Removed]\r\n" C_0, atoi( arg+7 ));
		} }
	else if (!strncmp( arg, "makemain", 8 ) )
	{
		clientff(C_R"[\""C_G"%s"C_R"\" Replaced by \""C_G"%s"C_R"\" as main room name]\r\n"C_0,
				current_room->name, current_room->additional_name[atoi( arg+9 )]);
		free( current_room->name );
		current_room->name = current_room->additional_name[atoi( arg+9 )];
		current_room->additional_name[atoi( arg+9 )] = NULL;
	}

}

void do_room_shop( char *arg )
{
	if ( !current_room )
	{
		clientf( C_R "[No current room.]\r\n" C_0 );
		return;
	}
	if ( current_room->shop ) {current_room->shop = 0;clientff(C_R"[Room Shop mark removed.]\r\n"C_0);}
	else {current_room->shop = 1;clientff(C_R"[Room marked as being a Shop.]\r\n"C_0);}

}

void do_room_avoid ( char *arg )
{
	ROOM_DATA *room;
	if ( !current_room )
	{
		clientf( C_R "[No current room.]\r\n" C_0 );
		return;
	}
	if (strncmp(arg,"list",4)) {
		room = current_room;
		if ( isdigit( arg[0] ) )
		{
			if ( !( room = get_room( atoi( arg ) ) ) )
			{clientfr("No room with that vnum");return;}
		}
		if ( room->avoid )
		{room->avoid = 0;clientff(C_R"[Pathfinding through room '%s' enabled.]\r\n"C_0,room->name );}
		else
		{room->avoid = 1;clientff(C_R"[Pathfinding through room '%s' disabled.]\r\n"C_0,room->name );}}
	else
	{
		if (strncmp(arg,"list area",9))
		{
			clientfr("Rooms that are turned off in the world:");
			for (room = world;room;room = room->next_in_world)
				if ( room->avoid )
					clientff(C_D" ("C_G"%5d"C_D")"C_y" %-30.30s "C_R"("C_g"%s"C_R")\r\n"C_0, room->vnum, room->name, room->area->name);
		}
		else
		{
			clientfr("Rooms that are turned off in the area:");
			for (room = current_room->area->rooms;room;room = room->next_in_area)
				if ( room->avoid )
					clientff(C_D" ("C_G"%5d"C_D")"C_y" %-30.30s "C_W"\r\n"C_0, room->vnum, room->name );

		}
	}
}

void do_room_merge( char *arg )
{
	ROOM_DATA *r, *old_room, *new_room;
	char buf[4096];
	int found;
	int i;

	if ( !current_room )
	{
		clientfr( "No current room set." );
		return;
	}

	if ( !arg[0] )
	{
		/* Show this room... Just for comparison purposes. */
		int first = 1;

		clientfr( "This room:" );

		buf[0] = 0;
		for ( i = 1; dir_name[i]; i++ )
		{
			if ( ( current_room->exits[i] || current_room->detected_exits[i] ) && !current_room->hasrandomexits )
			{
				if ( first )
				{
					strcat( buf, C_D "(" );
					first = 0;
				}
				else
					strcat( buf, C_D "," );

				if ( current_room->exits[i] )
					strcat( buf, C_B );
				else
					strcat( buf, C_R );

				strcat( buf, dir_small_name[i] );
			}
		}
		if ( !first )
			strcat( buf, C_D ")" C_0 );
		else if ( !current_room->hasrandomexits )
			strcat( buf, C_D "(" C_r "no exits" C_D ")" C_0 );
		else if ( current_room->hasrandomexits )
			strcat( buf, C_D "(" C_r "random exits" C_D ")" C_0 );

		clientff( "  %s -- %s " C_D "(" C_G "%d" C_D ")\r\n" C_0,
				buf, current_room->name, current_room->vnum );


		/* List all rooms that can be merged into this one. */

		clientfr( "Other rooms:" );
		found = 0;

		for ( r = world; r; r = r->next_in_world )
			if ( !strcmp( current_room->name, r->name ) && r != current_room )
			{
				int good = 1;

				for ( i = 1; dir_name[i]; i++ )
				{
					int e1 = r->exits[i] ? 1 : r->detected_exits[i];
					int e2 = current_room->exits[i] ? 1 : current_room->detected_exits[i];

					if ( e1 != e2 && !r->hasrandomexits )
					{
						good = 0;
						break;
					}
				}

				/* Print it away. */
				if ( good )
				{
					int first = 1;

					found = 1;

					buf[0] = 0;
					for ( i = 1; dir_name[i]; i++ )
					{
						if ( ( r->exits[i] || r->detected_exits[i] ) && !r->hasrandomexits )
						{
							if ( first )
							{
								strcat( buf, C_D "(" );
								first = 0;
							}
							else
								strcat( buf, C_D "," );

							if ( r->exits[i] )
								strcat( buf, C_B );
							else
								strcat( buf, C_R );

							strcat( buf, dir_small_name[i] );
						}
					}
					if ( !first )
						strcat( buf, C_D ")" C_0 );
					else if ( !r->hasrandomexits )
						strcat( buf, C_D "(" C_r "no exits" C_D ")" C_0 );
					else if ( r->hasrandomexits )
						strcat( buf, C_D "(" C_r "random exits" C_D ")" C_0 );

					clientff( "  %s -- %s " C_D "(" C_G "%d" C_D ")\r\n" C_0,
							buf, r->name, r->vnum );
				}
			}

		if ( !found )
			clientf( "  No identical matches found.\r\n" );
	}
	else
	{
		int vnum;

		old_room = current_room;

		if ( mode != CREATING )
		{
			clientfr( "Turn mapping on, first." );
			return;
		}

		vnum = atoi( arg );
		if ( !vnum || !( new_room = get_room( vnum ) ) )
		{
			clientfr( "Merge with which vnum?" );
			return;
		}

		if ( new_room == old_room )
		{
			clientfr( "You silly thing... What's the point in merging with the same room?" );
			return;
		}

		if ( strcmp( new_room->name, old_room->name ) )
		{
			clientfr( "That room doesn't even have the same name!" );
			return;
		}

		/* Check for exits leading to other places. */
		if ( !new_room->hasrandomexits )
			for ( i = 1; dir_name[i]; i++ )
			{
				if ( new_room->exits[i] && old_room->exits[i] &&
						new_room->exits[i] != old_room->exits[i] )
				{
					clientff( C_R "[Problem with the %s exits; they lead to different places.]\r\n", dir_name[i] );
					return;
				}

				if ( ( new_room->exits[i] || new_room->detected_exits[i] ) !=
						( old_room->exits[i] || old_room->detected_exits[i] ) )
				{
					clientff( C_R "[Problem with the %s exits; one room has it, the other doesn't.]\r\n", dir_name[i] );
					return;
				}
			}

		/* Start merging. */
		for ( i = 1; dir_name[i]; i++ )
		{
			if ( !new_room->exits[i] )
			{
				new_room->exits[i] = old_room->exits[i];
				new_room->detected_exits[i] |= old_room->detected_exits[i];
				new_room->locked_exits[i] |= old_room->locked_exits[i];
				if ( !new_room->exit_length[i] )
					new_room->exit_length[i] = old_room->exit_length[i];
				if ( !new_room->use_exit_instead[i] )
					new_room->use_exit_instead[i] = old_room->use_exit_instead[i];
				new_room->exit_stops_mapping[i] = old_room->exit_stops_mapping[i];
			}
		}

		/* All rooms that once pointed to the old one, should point to the new. */
		for ( r = world; r; r = r->next_in_world )
		{
			for ( i = 1; dir_name[i]; i++ )
			{
				if ( r->exits[i] == old_room  )
				{
					r->exits[i] = new_room;

					if ( !new_room->reverse_exits[i] )
					{
						new_room->reverse_exits[i] = r;
						new_room->more_reverse_exits[i] = 0;
					}
					else
					{
						new_room->more_reverse_exits[i] = 1;
					}
				}
			}
		}

		new_room->underwater |= old_room->underwater;

		/* Add all missing tags. */
		while ( old_room->tags )
		{
			ELEMENT *tag;

			/* Look if it already exists. */
			for ( tag = new_room->tags; tag; tag = tag->next )
				if ( !strcmp( (char *) old_room->tags->p, (char *) tag->p ) )
					break;
			if ( !tag )
			{
				tag = calloc( 1, sizeof( ELEMENT ) );
				tag->p = old_room->tags->p;
				link_element( tag, &new_room->tags );
			}
			else
				free( old_room->tags->p );

			unlink_element( old_room->tags );
		}

		clientff( C_R "[Rooms " C_G "%d" C_R " and " C_G "%d" C_R " merged into " C_G "%d" C_R ".]\r\n" C_0,
				new_room->vnum, old_room->vnum, new_room->vnum );
		destroy_room( old_room );

		current_room = new_room;
		current_area = new_room->area;

		if ( current_room->hasrandomexits ) {
			void do_room_random( char *arg );
			do_room_random("hidererandom");}

	}
}


void do_room_random( char *arg)
{
	if ( !current_room ) {
		clientfr("No current room");
		return;}

	int dir = 0;
	int i;
	char dirn[256];

	arg = get_string(arg , dirn ,256);

	if ( !strcmp(dirn,"help") )
	{clientfr("Room Random Help");
		clientf(" Room Random      : Toggles the status of this room, telling the mapper it is a room with random exits\r\n"
				"                    this should be done after true exits are set.\r\n"
				" Room Random dir  : Sets the exit direction to be a true exit, this means that even though all other exits\r\n"
				"                    in the room when set to be random, this exit will lead to the same location all the time\r\n");
	}

	for ( i = 1; dir_name[i]; i++ )
	{
		if ( !strcmp( dirn, dir_name[i] ) || !strcmp(dirn, dir_small_name[i]) )
			dir = i;
	}
	if ( !dir ) {
		if ( !strcmp(dirn,"hidererandom") || !current_room->hasrandomexits )
		{current_room->hasrandomexits = 1;
			if ( strcmp(dirn,"hidererandom") ) {
				clientfr("This room has been marked as having random exits");
				clientfr("All exits not marked as true are deleted");}
			for ( i = 1; dir_name[i]; i++ )
			{
				if ( current_room->exits[i] && !current_room->trueexit[i] )
				{
					if ( current_room->exits[i]->exits[reverse_exit[i]] != current_room )
					{
					}
					else
					{
						set_reverse( NULL, reverse_exit[i], current_room );
						current_room->exits[i]->exits[reverse_exit[i]] = NULL;
						current_room->exits[i]->detected_exits[reverse_exit[i]] = 0;
					}
					set_reverse( NULL, i, current_room->exits[i] );
					current_room->exits[i] = NULL;
					current_room->detected_exits[i] = 0;
				}
			}
		}
		else
		{current_room->hasrandomexits = 0;
			clientfr("Room is no longer marked as having random exits");
		}
	}
	else
	{
		if ( !current_room->hasrandomexits ) {
			if ( !current_room->trueexit[dir] )
			{current_room->trueexit[dir] = 1;
				clientff(C_Y"[Exit %s, has been marked as a true exit (it isn't random, like the rest rooms)\r\n"C_0,dir_name[dir]);
			}
			else
			{current_room->trueexit[dir] = 0;
				clientff(C_Y"[Exit %s, is no longer a true exit\r\n"C_0,dir_name[dir]);}}
		else
		{clientfr("This room is already marked as random, true exits need to be set before you mark a room as random");}
	}

}

void do_room_tag( char *arg )
{
	ROOM_DATA *room;
	ELEMENT *tag;
	char buf[256];
	int vnum;

	arg = get_string( arg, buf, 256 );

	if ( ( vnum = atoi( buf ) ) )
	{
		room = get_room( vnum );
		if ( !room )
		{
			clientfr( "No room with that vnum found." );
			return;
		}

		arg = get_string( arg, buf, 256 );
	}
	else if ( !( room = current_room ) )
	{
		clientfr( "No current room set." );
		return;
	}

	if ( !buf[0] )
	{
		clientfr( "Usage: room tag [vnum] <tagname>" );
		return;
	}

	/* Look if it already exists. */
	for ( tag = room->tags; tag; tag = tag->next )
		if ( !strcmp( buf, (char *) tag->p ) )
			break;

	if ( tag )
	{
		/* Unlink it. */
		free( tag->p );
		unlink_element( tag );
	}
	else
	{
		/* Link it. */
		tag = calloc( 1, sizeof( ELEMENT ) );
		tag->p = strdup( buf );
		link_element( tag, &room->tags );
	}

	clientfr( room->name );
	clientff( C_R "[Tags in v" C_G "%d" C_R ": ", room->vnum );
	if ( room->tags )
	{
		clientf( (char *) room->tags->p );
		for ( tag = room->tags->next; tag; tag = tag->next )
			clientff( ", %s", (char *) tag->p );
	}
	else
		clientf( C_D "(none)" C_R );
	clientf( ".]\r\n" C_0 );

	save_settings( "config.mapper.txt" );
}



void do_room_mark( char *arg )
{
	char buf[256];
	char buf2[256];

	get_string( arg, buf, 256 );

	if ( isdigit( buf[0] ) )
		sprintf( buf2, "%s mark", buf );
	else
		sprintf( buf2, "mark" );

	do_room_tag( buf2 );

	clientfr( "Using 'map config save' will make it permanent." );
}


void do_room_tempmark( char *arg )
{
	if ( !current_room )
		return;
	ROOM_DATA *room = NULL;
	if ( !( room = get_room( atoi( arg ) ) ) )
	{room = current_room;
	}
	if ( !arg[0] || room != current_room ) {
		if ( !room->tmark ) {
			room->tmark = 1;
			clientff(C_R"[Room ("C_y"%s"C_R") Temp Mark Added]\r\n"C_0,room->name);}
		else
		{room->tmark = 0;
			clientff(C_R"[Room ("C_y"%s"C_R") Temp Mark Removed]\r\n"C_0,room->name);}
	}
	else
	{
		if (!strcmp(arg,"clear world")||!strcmp(arg,"clear"))
		{
			ROOM_DATA *rcl = NULL;
			for ( rcl = world; rcl; rcl = rcl->next_in_world )
				if ( rcl->tmark )
					rcl->tmark = 0;
			clientfr("All Temp Marks Cleared");
		}
		else if ( !strcmp(arg,"clear area") )
		{
			ROOM_DATA *rcl = NULL;
			for ( rcl = current_room->area->rooms; rcl; rcl = rcl->next_in_area )
				if ( rcl->tmark )
					rcl->tmark = 0;

			clientfr("All Temp Marks in area Cleared");
		}
		else if ( !strcmp(arg,"help") )
			clientf(C_R"[Temp Marks]\r\n"C_0" - Room tm/tempmark       : Toggles a temporary mark for the room, which also is shown on the map.\r\n"
					" - Room tm/tempmark show  : Will give you information about your temp marks in this area.\r\n"
					" - Room tm/tempmark clear : Will clear all temp marks, if 'clear area' is used only in this area.\r\n"C_0);
		else if ( !strcmp(arg,"show") )
		{
			ROOM_DATA *r = NULL;
			int tm = 0,ntm = 0;
			char nom[1536],tbuf[128];
			for ( r = current_room->area->rooms; r; r = r->next_in_area ) {
				if ( !r->tmark )
				{ntm++;
					if ( ntm < 41 ) {
						sprintf(tbuf,C_D"("C_G"%d"C_D") "C_0,r->vnum );
						strcat(nom, tbuf);
						if ( ntm == 8 || ntm == 16 || ntm == 24 || ntm == 32  )
							strcat(nom, "\r\n");
					}}
				if ( r->tmark )
					tm++;
			}
			if ( tm )
			{ clientfr("Temp Marks in this area");
				clientff(C_D"["C_G"%d"C_D"]"C_c" rooms temp marked.\r\n"C_0,tm);
				if ( ntm ) {
					clientff(C_D"["C_G"%d"C_D"] "C_y"without temp marks.\r\nMax 40 printed below.\r\n"C_0,ntm);
					clientff("%s\r\n"C_0,nom);}
			}
			else {
				clientfr("No Temp Marks in this area");}
		}
	}
}

/* Exit commands. */

void do_exit_help( char *arg )
{
	clientfr( "Module: IMapper. Room exit commands:" );
	clientf( " exit link      - Link room # to this one. Both ways.\r\n"
			" exit stop      - Hide rooms beyond this exit.\r\n"
			" exit length    - Increase the exit length by #.\r\n"
			" exit map       - Map the next exit elsewhere.\r\n"
			" exit lock      - Set all unlinked exits in room as locked.\r\n"
			" exit unilink   - One-way exit. Does not create a reverse link.\r\n"
			" exit destroy   - Destroy an exit, and its reverse.\r\n"
			" exit joinareas - Show two areas on a single 'map'.\r\n"
			" exit special   - Create, destroy, modify or list special exits.\r\n"
			" exit warp      - Create a two-way wormhole link to a room.\r\n" );
}



void do_exit_length( char *arg )
{
	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	if ( !isdigit( *(arg) ) )
		clientfr( "Specify a length. 0 is normal." );
	else
	{
		set_length_to = atoi( arg );

		if ( set_length_to == 0 )
			set_length_to = -1;

		clientfr( "Move in the direction you wish to have this length." );
	}
}



void do_exit_stop( char *arg )
{
	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	clientfr( "Move in the direction you wish to stop (or start again) mapping from." );
	switch_exit_stops_mapping = 1;
}



void do_exit_joinareas( char *arg )
{
	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	clientfr( "Move into the other area." );
	switch_exit_joins_areas = 1;
}



void do_exit_map( char *arg )
{
	char buf[256];
	int i;

	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	use_direction_instead = 0;

	for ( i = 1; dir_name[i]; i++ )
	{
		if ( !strcmp( arg, dir_small_name[i] ) )
			use_direction_instead = i;
	}

	if ( !use_direction_instead )
	{
		use_direction_instead = -1;
		clientfr( "Will use default exit, as map position." );
	}
	else
	{
		sprintf( buf, "The room will be mapped '%s' from here, instead.",
				dir_name[use_direction_instead] );
		clientfr( buf );
	}
}



void do_exit_link( char *arg )
{
	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	if ( !isdigit( *(arg) ) )
	{
		clientfr( "Specify a vnum to link to." );
		return;
	}

	link_next_to = get_room( atoi( arg ) );

	unidirectional_exit = 0;

	if ( !link_next_to )
		clientfr( "Disabled." );
	else
		clientfr( "Move in the direction the room is." );
}


void do_exit_warp( char *arg )
{
	WORMHOLE_DATA *newwarp;
	WORMHOLE_DATA *newwarpreverse;
	ROOM_DATA *check = NULL, *from = NULL;
	char buf[256];
	char cmd[256];
	char *fnum = NULL;
	char *destroyedname;
	int destroyedvnum;
	int hide = 0;

	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	if ( !strncmp( arg, "help", strlen( arg ) ) )
	{
		clientfr( "Syntax for this room to another: exit warp <vnum>" );
		clientfr( "Syntax for remote room linking : exit warp <vnum> from <vnum>" );
		clientfr( "Syntax: exit warp destroy" );
		clientfr( "Syntax: exit warp destroy <vnum>" );
		return;
	}
	if ( !current_room )
	{
		clientfr( "No current room set." );
		return;
	}
	if ( strstr(arg ," from ") ) {
		fnum =  strstr( arg, " from ");
		fnum = fnum + 6;
		if ( get_room(atoi(fnum)) )
			from = get_room( atoi ( fnum ) );
	} else {
		from = current_room;}
	if ( isdigit( *(arg) ) )
	{
		if ( from->worm_pointed_by ) {
			clientfr("From room already has a warp");
			return;
		}
		if ( get_room( atoi( arg ) ) )
			check = get_room( atoi( arg ) );
		if ( check->worm_pointed_by ) {
			clientfr( "Target room already has a warp" );
			return;
		}
		newwarp = from->wormhole;
		if ( newwarp )
		{
			newwarpreverse = newwarp->to->wormhole;
			destroy_wormhole( newwarp );
			destroy_wormhole( newwarpreverse );
		}


		if ( get_room( atoi( arg ) ) && atoi ( arg ) != from->vnum )
		{
			newwarp = create_wormhole( from );
			newwarp->vnum = atoi( arg );

			if ( !( newwarp->to = get_room( newwarp->vnum ) ) )
			{
				debugf( "Can't link room %d (%s) to %d. (wormhole)",
						current_room->vnum, current_room->name, newwarp->vnum );
				clientfr( "Encountered an error, refer to console." );
				newwarp->to = NULL;
				free( newwarp );
				return;
			}
			else
				newwarp->to->worm_pointed_by = 1;

			newwarpreverse = create_wormhole( newwarp->to );
			newwarpreverse->vnum = from->vnum;

			if ( !( newwarpreverse->to = get_room( newwarpreverse->vnum ) ) )
			{
				debugf( "Can't link room %d (%s) to %d. (reverse wormhole)",
						newwarp->to->vnum, newwarp->to->name, newwarpreverse->vnum );
				clientfr( "Encountered an error, refer to console." );
				return;
			}
			else
				newwarpreverse->to->worm_pointed_by = 1;


			sprintf( buf, "Warp created between %s (%d) and %s (%d).",
					newwarp->owner->name, newwarp->owner->vnum, newwarpreverse->owner->name, newwarpreverse->owner->vnum  );
			clientfr( buf );
			return;

		}
		else if ( atoi ( arg ) == from->vnum )
			return;
		else
		{
			clientfr( "Unknown room vnum." );
			return;
		}
	}


	arg = get_string( arg, cmd, 256 );
	if ( !strcmp(cmd, "hide") ) {
		hide = 1;
		arg = get_string( arg, cmd, 256 );}

	if ( !strcmp( cmd, "destroy" ) )
	{ check = NULL;
		if ( isdigit( *(arg) ) && get_room( atoi( arg ) ) ) {
			check = get_room( atoi( arg ));
			newwarp = check->wormhole;
		}
		else  {
			check = current_room;
			newwarp = check->wormhole;
		}
		if ( newwarp )
		{
			newwarpreverse = newwarp->to->wormhole;
			destroyedname = newwarpreverse->owner->name;
			destroyedvnum = newwarpreverse->owner->vnum;
			destroy_wormhole( newwarp );
			destroy_wormhole( newwarpreverse );
			if ( !hide ) {
				sprintf( buf, "Warp between %s (%d) and %s (%d) has been destroyed.",
						check->name, check->vnum, destroyedname, destroyedvnum  );
				clientfr( buf );}
			return;

		}
		else
			clientfr( "No wormhole here to destroy." );
		return;
	}

	else
		clientfr( "Unknown command, try 'exit warp help'." );
}


void do_exit_unilink( char *arg )
{
	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	if ( !isdigit( *(arg) ) )
	{
		clientfr( "Specify a vnum to link to." );
		return;
	}

	link_next_to = get_room( atoi( arg ) );

	unidirectional_exit = 1;

	if ( !link_next_to )
	{
		unidirectional_exit = 0;
		clientfr( "Disabled." );
	}
	else
		clientfr( "Move in the direction the room is." );
}



void do_exit_lock( char *arg )
{
	int i, set;

	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	if ( !current_room )
	{
		clientfr( "No current room set." );
		return;
	}

	if ( arg[0] == 'c' )
		set = 0;
	else
		set = 1;

	for ( i = 1; dir_name[i]; i++ )
	{
		if ( set ) {
			if ( current_room->detected_exits[i] &&
					!current_room->exits[i] )
			{
				current_room->locked_exits[i] = set;
			}
		}
		else if ( current_room->locked_exits[i] ) {current_room->locked_exits[i] = set;}
	}
	if ( set )
		clientfr( "All unlinked exits in this room have been marked as locked." );
	else
		clientfr( "All locked exits now unlocked." );
}



void do_exit_destroy( char *arg )
{
	char buf[256];
	int dir = 0;
	int i;

	if ( mode != CREATING )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	if ( !current_room )
	{
		clientfr( "No current room set." );
		return;
	}

	for ( i = 1; dir_name[i]; i++ )
	{
		if ( !strcmp( arg, dir_name[i] ) )
			dir = i;
	}

	if ( !dir )
	{
		clientfr( "Which link do you wish to destroy?" );
		return;
	}

	if ( !current_room->exits[dir] )
	{
		clientfr( "No link in that direction." );
		return;
	}

	if ( current_room->exits[dir]->exits[reverse_exit[dir]] != current_room )
	{
		clientfr( "Reverse link was not destroyed." );
	}
	else
	{
		set_reverse( NULL, reverse_exit[dir], current_room );
		//	current_room->reverse_exits[reverse_exit[dir]] = NULL;
		current_room->exits[dir]->exits[reverse_exit[dir]] = NULL;
		current_room->exits[dir]->detected_exits[reverse_exit[dir]] = 0;
	}

	i = current_room->exits[dir]->vnum;

	set_reverse( NULL, dir, current_room->exits[dir] );
	//   current_room->exits[dir]->reverse_exits[dir] = NULL;
	current_room->exits[dir] = NULL;
	current_room->detected_exits[dir] = 0;

	sprintf( buf, "Link to vnum %d destroyed.", i );
	clientfr( buf );
}



void do_exit_special( char *arg )
{
	EXIT_DATA *spexit;
	char cmd[512];
	char buf[512];
	int i, nr = 0;
	char buf2[256], *b = buf2;
	char *p;

	DEBUG( "do_exit_special" );

	if ( mode != CREATING && strcmp( arg, "list" ) )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	if ( !strncmp( arg, "help", strlen( arg ) ) )
	{
		clientfr( "Syntax: exit special <command> [exit] [args]" );
		clientfr( "Commands: list, capture, destroy" );
		clientfr( "Advanced: create, command, message, link, alias, nolook, nomsg" );
		return;
	}

	if ( !current_room )
	{
		clientfr( "No current room set." );
		return;
	}

	arg = get_string( arg, cmd, 512 );

	if ( !strcmp( cmd, "list" ) )
	{
		clientfr( "Special exits:" );
		for ( spexit = current_room->special_exits; spexit; spexit = spexit->next )
		{
			sprintf( buf, C_B "%3d" C_0 " - L: " C_G "%d" C_0 " A: '" C_g
					"%s" C_0 "' C: '" C_g "%s" C_0 "' M: '" C_g "%s" C_0 "''"C_R"%s %s"C_0"\r\n",
					nr++, spexit->vnum, spexit->alias ? dir_name[spexit->alias] : "none",
					spexit->command, spexit->message, spexit->nolook ? " No Look" : "", spexit->nomsg ? " No Msg" : "" );
			clientf( buf );
		}

		if ( !nr )
			clientf( " - None.\r\n" );
	}

	else if ( !strcmp( cmd, "create" ) )
	{
		create_exit( current_room );
		clientfr( "Special exit created." );
	}

	else if ( !strcmp( cmd, "destroy" ) )
	{
		int done = 0;

		if ( !arg[0] || !isdigit( arg[0] ) )
		{
			clientfr( "What special exit do you wish to destroy?" );
			return;
		}

		nr = atoi( arg );

		for ( i = 0, spexit = current_room->special_exits; spexit; spexit = spexit->next )
		{
			if ( i++ == nr )
			{
				destroy_exit( spexit );
				done = 1;
				break;
			}
		}

		if ( done )
			sprintf( buf, "Special exit %d destroyed.", nr );
		else
			sprintf( buf, "Special exit %d was not found.", nr );
		clientfr( buf );
	}

	else if ( !strcmp( cmd, "capture" ) )
	{
		arg = get_string( arg, cmd, 256 );

		if ( !cmd[0] )
		{
			clientfr( "Syntax: Esp capture create" );
			clientfr( "        Esp capture link <vnum>" );
			return;
		}

		if ( !strcmp( cmd, "create" ) )
		{
			capture_special_exit = -1;

			clientfr( "Capturing. A room will be created on the other end." );
			clientfr( "Use 'stop' to disable capturing." );
		}
		else if ( !strcmp( cmd, "link" ) )
		{
			ROOM_DATA *room;
			int vnum;

			get_string( arg, cmd, 256 );
			vnum = atoi( cmd );

			if ( vnum < 1 )
			{
				clientfr( "Specify a valid vnum to link the exit to." );
				return;
			}
			if ( !( room = get_room( vnum ) ) )
			{
				clientfr( "No room with that vnum exists!" );
				return;
			}

			capture_special_exit = vnum;

			clientff( C_R "[Capturing. The exit will be linked to '%s']\r\n" C_0, room->name );
			clientfr( "Use 'stop' to disable capturing." );
		}

		cse_command[0] = 0;
		cse_message[0] = 0;

		return;
	}

	else if ( !strcmp( cmd, "message" ) )
	{
		int done = 0;

		/* We'll store the number in cmd. */
		arg = get_string( arg, cmd, 256 );

		if ( !isdigit( cmd[0] ) )
		{
			clientfr( "Set message on what special exit?" );
			return;
		}

		nr = atoi( cmd );

		for ( i = 0, spexit = current_room->special_exits; spexit; spexit = spexit->next )
		{
			if ( i++ == nr )
			{
				if ( spexit->message )
					free( spexit->message );
				p = arg;
				while ( *p )
				{
					if ( *p == '"' )
						*(b++) = '*';
					else
						*(b++) = *p;
					p++;
				}
				*b = 0;

				spexit->message = strdup( buf2 );
				done = 1;
				break;
			}
		}

		if ( done )
			sprintf( buf, "Message on exit %d changed to '%s'", nr, buf2 );
		else
			sprintf( buf, "Can't find special exit %d.", nr );

		clientfr( buf );
	}

	else if ( !strcmp( cmd, "command" ) )
	{
		int done = 0;

		/* We'll store the number in cmd. */
		arg = get_string( arg, cmd, 256 );

		if ( !isdigit( cmd[0] ) )
		{
			clientfr( "Set command on what special exit?" );
			return;
		}

		nr = atoi( cmd );

		for ( i = 0, spexit = current_room->special_exits; spexit; spexit = spexit->next )
		{
			if ( i++ == nr )
			{
				if ( spexit->command )
					free( spexit->command );

				spexit->command = strdup( arg );
				done = 1;
				break;
			}
		}

		if ( done )
			sprintf( buf, "Command on exit %d changed to '%s'", nr, arg );
		else
			sprintf( buf, "Can't find special exit %d.", nr );

		clientfr( buf );
	}

	else if ( !strcmp( cmd, "nolook" ) )
	{
		int done = 0;

		/* We'll store the number in cmd. */
		arg = get_string( arg, cmd, 256 );

		if ( !isdigit( cmd[0] ) )
		{
			clientfr( "Set nolook on what special exit?" );
			return;
		}

		nr = atoi( cmd );

		for ( i = 0, spexit = current_room->special_exits; spexit; spexit = spexit->next )
		{
			if ( i++ == nr )
			{
				if ( spexit->nolook )
					spexit->nolook = 0;
				else
					spexit->nolook = 1;

				done = 1;
				break;
			}
		}

		if ( done )
			sprintf( buf, "Nolook on exit %d changed to: %s ", nr, spexit->nolook ? "There is no look after message" : "The Room is looked into after message" );
		else
			sprintf( buf, "Can't find special exit %d.", nr );

		clientfr( buf );
	}

	else if ( !strcmp( cmd, "nomsg" ) )
	{
		int done = 0;

		/* We'll store the number in cmd. */
		arg = get_string( arg, cmd, 256 );

		if ( !isdigit( cmd[0] ) )
		{
			clientfr( "Set nomsg on what special exit?" );
			return;
		}

		nr = atoi( cmd );

		for ( i = 0, spexit = current_room->special_exits; spexit; spexit = spexit->next )
		{
			if ( i++ == nr )
			{
				if ( spexit->nomsg )
					spexit->nomsg = 0;
				else
					spexit->nomsg = 1;

				done = 1;
				break;
			}
		}

		if ( done )
			sprintf( buf, "Nomsg on exit %d changed to: %s ", nr, spexit->nomsg ? "There is no message" : "There is a message" );
		else
			sprintf( buf, "Can't find special exit %d.", nr );

		clientfr( buf );
	}

	else if ( !strcmp( cmd, "link" ) )
	{
		/* E special link 0 -1 */
		char vnum[256];

		/* We'll store the number in cmd. */
		arg = get_string( arg, cmd, 256 );

		if ( !isdigit( cmd[0] ) )
		{
			clientfr( "Link which special exit?" );
			return;
		}

		nr = atoi( cmd );

		for ( i = 0, spexit = current_room->special_exits; spexit; spexit = spexit->next )
		{
			if ( i++ == nr )
			{
				get_string( arg, vnum, 256 );

				if ( vnum[0] == '-' )
				{
					ROOM_DATA *to;

					to = spexit->to;

					spexit->vnum = -1;
					spexit->to = NULL;

					check_pointed_by( to );

					sprintf( buf, "Link cleared on exit %d.", nr );
					clientfr( buf );
					return;
				}
				else if ( isdigit( vnum[0] ) )
				{
					spexit->vnum = atoi( vnum );
					spexit->to = get_room( spexit->vnum );
					if ( !spexit->to )
					{
						clientfr( "A room whith that vnum was not found." );
						spexit->vnum = -1;
						return;
					}
					spexit->to->pointed_by = 1;
					sprintf( buf, "Special exit %d linked to '%s'.",
							nr, spexit->to->name );
					clientfr( buf );
					return;
				}
				else
				{
					clientfr( "Link to which vnum?" );
					return;
				}
			}
		}

		sprintf( buf, "Can't find special exit %d.", nr );
		clientfr( buf );
	}

	else if ( !strcmp( cmd, "alias" ) )
	{
		/* E special alias 0 down */

		if ( !arg[0] || !isdigit( arg[0] ) )
		{
			clientfr( "What special exit do you wish to destroy?" );
			return;
		}

		nr = atoi( arg );

		for ( i = 0, spexit = current_room->special_exits; spexit; spexit = spexit->next )
		{
			if ( i++ == nr )
			{
				arg = get_string( arg, cmd, 256 );

				if ( !strcmp( arg, "none" ) ) {
					spexit->alias = 0;
					sprintf( buf, "Alias on exit %d cleared.", nr );
					clientfr( buf );
					return;
				}

				for ( i = 1; dir_name[i]; i++ )
				{
					if ( !strcmp( arg, dir_name[i] ) )
					{
						sprintf( buf, "Going %s will now trigger exit %d.",
								dir_name[i], nr );
						clientfr( buf );
						spexit->alias = i;
						return;
					}
				}

				clientfr( "Use an exit, such as 'north', 'up', 'none', etc." );
				return;
			}
		}

		sprintf( buf, "Special exit %d was not found.", nr );
		clientfr( buf );
	}

	else
	{
		clientfr( "Unknown command... Try 'exit special help'." );
	}
}


void do_exit_explore()
{
	if ( !current_area )
	{clientfr("No current area set exploring until ");
	}
}

void do_room_noexit( char *arg )
{
	/* finish this, code for setting the room noexit checks the command and message for continuing */
	if ( !arg[0] )
	{clientfr("Room noexit help");
		clientf(" room noexit list    : Lists the exits with a noexit command and message set.\r\n"
				" room noexit dir cmd : Sets the command for the noexit function for that direction.\r\n"
				" room noexit dir msg : Sets the message for the noexit function for that direction.\r\n"
				" room noexit dir del : Deletes the command and message for noexit.\r\n"
				" This noexit function is a way to allow the system to attempt certain things if it cannot find a mapped exit.\r\n"
				" Think the airlocks in kelsys, you can set it to room noexit in cmd pull lever, and room noexit in msg\r\n"
				" ( line given when the exit opens ), this will make the pather wait, and then move at that message.\r\n");
		return;}

	if ( !current_room )
	{
		clientfr( "No current room set." );
		return;
	}

	int dir = 0;
	int i;
	char dirn[256],cmd[256];

	arg = get_string(arg , dirn ,256);
	arg = get_string(arg , cmd ,256);

	if ( mode != CREATING && strncmp(dirn,"list",strlen(dirn)) )
	{
		clientfr( "Turn mapping on, first." );
		return;
	}

	for ( i = 1; dir_name[i]; i++ )
	{
		if ( !strcmp( dirn, dir_name[i] ) || !strcmp(dirn, dir_small_name[i]) )
			dir = i;
	}
	if ( dir ) {
		if ( !strcmp(cmd,"command") || !strcmp(cmd,"cmd") )
		{
			current_room->noexitcmd[dir] = strdup(arg);
			clientff(C_c" Command for '%s' exit is now '%s'\r\n"C_0,dir_name[dir],current_room->noexitcmd[dir]);
		}
		else if ( !strcmp(cmd,"message") || !strcmp(cmd,"msg") )
		{
			current_room->noexitmsg[dir] = strdup(arg);
			clientff(C_g" Message for '%s' exit is now '%s'\r\n"C_0,dir_name[dir],current_room->noexitmsg[dir]);
		}
		else if ( !strcmp(cmd,"delete") || !strcmp(cmd,"del") )
		{
			current_room->noexitcmd[dir] = NULL;
			current_room->noexitmsg[dir] = NULL;
			clientff(C_g" '%s' exit attempts deleted\r\n"C_0,dir_name[dir]);
		}
	}
	else if ( !strncmp(dirn,"list",strlen(dirn)) )
	{  clientfr("The following exits attempt the following if they for some reason don't find the exit they should");
		for ( i = 1; dir_name[i]; i++ )
		{ if ( current_room->noexitcmd[i] || current_room->noexitmsg[i] )
			clientff(" "C_0"%10.10s"C_W" - "C_c"Command: '%s'"C_W" -"C_g" Message: '%s'\r\n"C_0, dir_name[i] ,current_room->noexitcmd[i] ? current_room->noexitcmd[i] : "none", current_room->noexitmsg[i] ? current_room->noexitmsg[i] : "none" );
		}
	}
}


/* Normal commands. */

void do_landmarks( char *arg )
{
	AREA_DATA *area;
	ROOM_DATA *room;
	ELEMENT *tag;
	char buf[256];
	char name[256];
	int first;
	int found = 0;

	get_string( arg, name, 256 );

	if ( !name[0] )
		clientfr( "Landmarks throughout the world:" );
	else
		clientfr( "Tagged rooms:" );

	for ( area = areas; area; area = area->next )
	{
		first = 1;

		for ( room = area->rooms; room; room = room->next_in_area )
		{
			for ( tag = room->tags; tag; tag = tag->next )
				if ( !case_strcmp( (char *) tag->p, name[0] ? name : "mark" ) )
					break;

			if ( !tag )
				continue;

			if ( !found )
				found = 1;

			/* And now... */

			/* Area name, if first room in the area's list. */
			if ( first )
			{
				sprintf( buf, "\r\n" C_B "%s" C_0 "\r\n", area->name );
				clientf( buf );
				first = 0;
			}
			if ( !disable_mxp_map && mxp_tag( TAG_LOCK_SECURE ) )
				sprintf( buf, C_D " (" C_G "<mpelm v=%d r=\"%s\" t=\"%s\">%d</mpelm>" C_D ") " C_0 "%s\r\n",
						room->vnum, room->name, room->room_type->name, room->vnum, room->name );
			else
				sprintf( buf, C_D " (" C_G "%d" C_D ") " C_0 "%s\r\n",
						room->vnum, room->name );
			clientf( buf );
		}
	}

	if ( !found )
	{
		if ( !name[0] )
			clientf( "None defined, use the 'room mark' command to add some." );
		else
			clientf( "None found, check your spelling or add some with 'room tag'." );
	}

	clientf( "\r\n" );
}



void do_go( char *arg )
{
	if ( q_top )
	{
		clientfr( "Command queue isn't empty, clear it first." );
		return;
	}

	dash_command = NULL;
	memset(troopn,'\0',sizeof(troopn));
	memset(guardnum,'\0',sizeof(guardnum));
	troopmove = 0;
	guardmove = 0;
	autowing = 0;
	autowing = cmp_room_wing();
	if ( strstr(arg, "gallop") )
		if ( !mounted )
		{ clientfr("Maybe you should mount up?");return;
		}

	if ( !strcmp( arg, "dash" ) )
		dash_command = "dash ";
	else if ( !strcmp( arg, "sprint" ) )
		dash_command = "sprint ";
	else if ( !strcmp( arg, "gallop" ) )
		dash_command = "gallop ";
	else if ( !strcmp(arg, "shift") )
		dash_command = "sand shift ";
    else if ( autowing )
         artimsg = wingcmd;
	else if ( strstr(arg, "troops") )
	{
		strcpy(troopn, arg + 7 );
		troopmove = 1;
	}
	else if ( strstr(arg, "guard") )
	{
		strcpy(guardnum, arg + 6 );
		guardmove = 1;
	}   else if ( arg[0] )
	{
		clientfr( "Usage: go (voltda/duanathar) [dash/sprint/gallop/shift]" );
		return;
	}

	go_next( );
	clientf( "\r\n" );
}

void do_go_back(char *arg)
{
	if ( !returnroom ) {
		clientfr("You need to go somewhere first");
	}
	else if (current_room->vnum == returnroom->vnum)
	{
		clientfr("You are already there!");
	}
	else {
		clientff(C_R"[Going back to: ("C_y"%s"C_R") in ("C_g"%s"C_R")]\r\n"C_0,returnroom->name,returnroom->area->name);
		char tm[256];
		memset( tm, '\0', sizeof(tm) );
		sprintf(tm,"%d",returnroom->vnum);
		do_map_path( tm );
		do_go(arg);}

}

void print_mhelp_line( char *line )
{
	char buf[8096];
	char *p, *b, *c;

	p = line;
	b = buf;

	while ( *p )
	{
		if ( *p != '^' )
		{
			*(b++) = *(p++);
			continue;
		}

		p++;
		switch ( *p )
		{
			case 'r':
				c = "\33[0;31m"; break;
			case 'g':
				c = "\33[0;32m"; break;
			case 'y':
				c = "\33[0;33m"; break;
			case 'b':
				c = "\33[0;34m"; break;
			case 'm':
				c = "\33[0;35m"; break;
			case 'c':
				c = "\33[0;36m"; break;
			case 'w':
				c = "\33[0;37m"; break;
			case 'D':
				c = "\33[1;30m"; break;
			case 'R':
				c = "\33[1;31m"; break;
			case 'G':
				c = "\33[1;32m"; break;
			case 'Y':
				c = "\33[1;33m"; break;
			case 'B':
				c = "\33[1;34m"; break;
			case 'M':
				c = "\33[1;35m"; break;
			case 'C':
				c = "\33[1;36m"; break;
			case 'W':
				c = "\33[1;37m"; break;
			case 'x':
				c = "\33[0;37m"; break;
			case '^':
				c = "^"; break;
			case 0:
				c = NULL;
				break;
			default:
				c = "-?-";
		}

		if ( !c )
			break;

		while ( *c )
			*(b++) = *(c++);
		p++;
	}

	*b = 0;

	clientf( buf );
}



void do_mhelp( char *arg )
{
	FILE *fl;
	char buf[4096];
	char name[256];
	char *p;
	int found = 0;
	int empty_lines = 0;

	if ( !arg[0] )
	{
		clientf( "Use 'mhelp index' for a list of mhelp files.\r\n" );
		return;
	}

	fl = fopen( "mhelp", "r" );
	if ( !fl )
		fl = fopen( "mhelp.txt", "r" );

	if ( !fl )
	{
		clientf( C_R "[" );
		clientff( "Unable to open mhelp: %s.", strerror( errno ) );
		clientf( "]\r\n" C_0 );
		return;
	}

	while ( 1 )
	{
		if ( !fgets( buf, 4096, fl ) )
			break;

		/* Comments... Ignored. */
		if ( buf[0] == '#' )
			continue;

		/* Strip newline. */
		p = buf;
		while ( *p != '\n' && *p != '\r' && *p )
			p++;
		*p = 0;

		/* Names. */
		if ( buf[0] == ':' )
		{
			if ( found )
				break;

			p = buf + 1;

			while ( *p )
			{
				p = get_string( p, name, 256 );
				if ( !case_strcmp( arg, name ) )
				{
					/* This is it. Start printing. */

					found = 2;
					empty_lines = 0;

					get_string( buf+1, name, 256 );
					clientff( C_C "MAPPER HELP" C_0 " - " C_W "%s" C_0 "\r\n\r\n", name );
					break;
				}
			}

			continue;
		}

		/* One line that must be displayed. */
		if ( found )
		{
			/* Remember empty lines, print them later. */
			/* Helps to skip them at the beginning and at the end. */
			if ( !buf[0] )
			{
				empty_lines++;
				continue;
			}

			if ( found == 2 )
				empty_lines = 0, found = 1;
			else
				if ( empty_lines > 0 )
					while ( empty_lines )
					{
						clientf( "\r\n" );
						empty_lines--;
					}

			print_mhelp_line( buf );
			clientf( "\r\n" );
		}
	}

	fclose( fl );

	if ( !found )
		clientfr( "No help file by that name." );
}



void do_old_mhelp( char *arg )
{
	if ( !strcmp( arg, "index" ) )
	{
		clientf( C_C "MAPPER HELP" C_0 "\r\n\r\n" );

		clientf( "- Index\r\n"
				"- GettingStarted\r\n"
				"- Commands\r\n" );
	}

	else if ( !strcmp( arg, "gettingstarted" ) )
	{
		clientf( C_C "MAPPER HELP" C_0 " - Getting started.\r\n\r\n"
				"Greetings.  If you're reading this, then you obviously have a new\r\n"
				"mapper to play with.\r\n"
				"\r\n"
				"First thing you should do, is to check if it's working.  Do so with a\r\n"
				"simple 'look' command, in a common room.  If the area name is not shown\r\n"
				"after the room title, then you must set the room title color.  Do so\r\n"
				"with 'map color'.  When the color matches the one set on Imperian, and\r\n"
				"the room you're currently in is mapped, all should work.\r\n"
				"\r\n"
				"If the area name was shown, that means it now knows where it is, so\r\n"
				"the 'map' command is available.\r\n"
				"\r\n"
				"You will then most likely want to make a path to somewhere, and walk\r\n"
				"to it.  Do so by choosing a room to go to, and find its vnum (room's\r\n"
				"virtual number).  As an example, if you'd want to go to the Shuk, in\r\n"
				"Antioch, find its vnum with 'room find shuk'.  In this case, the vnum\r\n"
				"is 605, so a path to it would be made by 'map path 605'.  Once the\r\n"
				"path is built, use 'go' to start auto-walking, and 'stop' in the\r\n"
				"middle of an auto-walking to stop it.\r\n"
				"\r\n"
				"These are the basics of showing a map, and going somewhere.\r\n" );
	}

	else if ( !strcmp( arg, "commands" ) )
	{
		clientf( C_C "MAPPER HELP" C_0 " - Commands.\r\n\r\n"
				"Basics\r\n"
				"------\r\n"
				"\r\n"
				"The mapper has four base commands, each having some subcommands.  The\r\n"
				"base commands are 'map', 'area', 'room' and 'exit'.  Each base\r\n"
				"command has the subcommand 'help', which lists all other subcommands.\r\n"
				"An example would be 'map help'.\r\n"
				"\r\n"
				"Abbreviating\r\n"
				"------------\r\n"
				"\r\n"
				"As an example, use 'area list'.  Base command is 'area', subcommand is\r\n"
				"'list'.  The subcommand may be abbreviated to any number of letters,\r\n"
				"so 'area lis', 'area li' and 'area l' all work.  The base command may\r\n"
				"be abbreviated only to the first letter, 'a l'.  Also, as typing a\r\n"
				"double space ('m p 605') is sometimes hard, the base command letter\r\n"
				"may be used as uppercase, with no space after it.  As an example,\r\n"
				"'Mp 605' instead of 'map path 605' or 'm p 605' would work.\r\n" );
	}

	else
		clientf( "Use 'mhelp index', for a list of mhelp files.\r\n" );
}




#define CMD_NONE	0
#define CMD_MAP		1
#define CMD_AREA	2
#define CMD_ROOM	3
#define CMD_EXIT	4

FUNC_DATA cmd_table[] =
{
	/* Map commands. */
	{ "help",		do_map_help,	CMD_MAP },
	{ "create",	do_map_create,	CMD_MAP },
	{ "follow",	do_map_follow,	CMD_MAP },
	{ "none",		do_map_none,	CMD_MAP },
	{ "save",		do_map_save,	CMD_MAP },
	{ "load",		do_map_load,	CMD_MAP },
	{ "path",		do_map_path,	CMD_MAP },
	{ "status",	do_map_status,	CMD_MAP },
	{ "color",		do_map_color,	CMD_MAP },
	{ "colour",	do_map_color,	CMD_MAP },
	{ "big",       do_map_big,     CMD_MAP },
	{ "bump",		do_map_bump,	CMD_MAP },
	{ "file",		do_map_file,	CMD_MAP },
	{ "teleport",	do_map_teleport,CMD_MAP },
	{ "auto",  do_map_automap, CMD_MAP },
	{ "queue",		do_map_queue,	CMD_MAP },
	{ "config",	do_map_config,	CMD_MAP },
	{ "medium",    do_map_medium,  CMD_MAP },
	{ "tags",      do_map_tags,    CMD_MAP },
	{ "types",     do_map_types,   CMD_MAP },
	{ "tiny",      do_map_tiny,    CMD_MAP },
	{ "exits",     do_map_exitc,   CMD_MAP },
	{ "windo",     do_map_nothing, CMD_MAP },
	{ "window",    do_map_window,  CMD_MAP },
	{ "uw",        do_map_underw,  CMD_MAP },
	{ "underwater",do_map_underw,  CMD_MAP },
	{ "shrine",    do_map_shrine,  CMD_MAP },
	{ "ruler",     do_map_ruler,   CMD_MAP },
	{ "remake",		do_map_remake,	CMD_MAP },
	{ "wing",      do_map_wing,    CMD_MAP },
	/* Area commands. */
	{ "help",		do_area_help,	CMD_AREA },
	{ "create",	do_area_create,	CMD_AREA },
	{ "list",		do_area_list,	CMD_AREA },
	{ "update",	do_area_update,	CMD_AREA },
	{ "info",		do_area_info,	CMD_AREA },
	{ "off",		do_area_off,	CMD_AREA },
	{ "orig",		do_area_orig,	CMD_AREA },
	{ "orig",		do_map_orig,	CMD_MAP },
	{ "find",      do_area_find,   CMD_AREA },
	{ "connected", do_area_conn,   CMD_AREA },
	{ "search",    do_area_search, CMD_AREA },
	{ "switch",	do_area_switch,	CMD_AREA },
	{ "destro",    do_area_fulld,  CMD_AREA },
	{ "destroy",   do_area_destroy,CMD_AREA },
	{ "types",     do_area_types,  CMD_AREA },
	{ "note",      do_area_note,   CMD_AREA },
	/* Room commands. */
	{ "help",		do_room_help,	CMD_ROOM },
	{ "switch",	do_room_switch,	CMD_ROOM },
	{ "create",	do_room_create,	CMD_ROOM },
	{ "info",		do_room_info,	CMD_ROOM },
	{ "briefinfo",	do_room_briefinfo,	CMD_ROOM },
	{ "find",		do_room_find,	CMD_ROOM },
	{ "look",		do_room_look,	CMD_ROOM },
	{ "destroy",	do_room_destroy, CMD_ROOM },
	{ "list",		do_room_list,	CMD_ROOM },
	{ "underwater",	do_room_underw, CMD_ROOM },
	{ "uw",	do_room_underw, CMD_ROOM },
	{ "merge",		do_room_merge,	CMD_ROOM },
	{ "tag",		do_room_tag,	CMD_ROOM },
	{ "mark",		do_room_mark,	CMD_ROOM },
	{ "types",		do_room_types,	CMD_ROOM },
	{ "addn",      do_room_addn,   CMD_ROOM },
	{ "shop",      do_room_shop,   CMD_ROOM },
	{ "avoid",     do_room_avoid,  CMD_ROOM },
	{ "tempmark",  do_room_tempmark,CMD_ROOM},
	{ "tm",  do_room_tempmark,CMD_ROOM},
	{ "clearowner",do_room_clearowner, CMD_ROOM},
	{ "random",    do_room_random, CMD_ROOM },
	{ "noexit",    do_room_noexit, CMD_ROOM},
	/* Exit commands. */
	{ "help",		do_exit_help,	CMD_EXIT },
	{ "length",	do_exit_length,	CMD_EXIT },
	{ "stop",		do_exit_stop,	CMD_EXIT },
	{ "map",		do_exit_map,	CMD_EXIT },
	{ "link",		do_exit_link,	CMD_EXIT },
	{ "unilink",	do_exit_unilink,CMD_EXIT },
	{ "lock",		do_exit_lock,	CMD_EXIT },
	{ "destroy",	do_exit_destroy,CMD_EXIT },
	{ "joinareas",	do_exit_joinareas,CMD_EXIT },
	{ "special",	do_exit_special,CMD_EXIT },
	{ "warp",      do_exit_warp, CMD_EXIT },
	/* Normal commands. */
	{ "map",		do_map,		CMD_NONE },
	{ "landmarks",	do_landmarks,	CMD_NONE },
	{ "mhelp",		do_mhelp,	CMD_NONE },
	{ "go",		do_go,		CMD_NONE },
	{ "worldwarps",do_map_warps, CMD_NONE},
	{ "back" ,     do_go_back, CMD_NONE},
	{ NULL, NULL, 0 }
};


int i_mapper_process_client_aliases( char *line )
{
	int i;
	char command[4096], *l;
	char buf[256];
	int base_cmd;
	char *exitparse;
	char *p;

	DEBUG( "i_mapper_process_client_aliases" );
	if ( !strncmp(line, "amap", 4)) {memset(buf, 0, sizeof(buf));
		strcpy(buf, line+1 );strncat( buf, "\r\n", 4 );send_to_server( buf );return 1;}
	if ( !strncmp(line, "ago", 3)) {memset(buf, 0, sizeof(buf));
		strcpy(buf, line+1 );strncat( buf, "\r\n", 4 );send_to_server( buf );return 1;}
	/* Room commands queue. */
	if ( !strcmp( line, "l" ) ||
			!strcmp( line, "look" ) ||
			!strcmp( line, "ql" ) ||
			!strcmp( line, "burrow below" ) ||
			!strcmp( line, "burrow above" ) ||
			!strcmp( line, "sand sink below" ) ||
			!strcmp( line, "sand sink above" ) ||
			!strcmp( line, "swing up" ) ||
			!strcmp( line, "swing down" ) ||
			!strcmp( line, "fly" ) ||
			!strcmp( line, "land trees" ) ||
			!strcmp( line, "land" ) ||
			!strcmp( line, "cast aerial" ) ||
			!strcmp( line, "squint" ) ||
			!strcmp( line, "glance" ) ||
			!strcmp( line, "cast sandling" )
	   )
	{
		/* check for burrow type*/
		if ( !strcmp( line, "burrow below" ) ||
				!strcmp( line, "burrow above" ) )
			burrowtype = 0;

		if ( !strcmp( line, "sand sink below" ) ||
				!strcmp( line, "sand sink above" ) )
			burrowtype = 1;
		/* Add -1 to queue. */
		if ( mode == CREATING || mode == FOLLOWING )
			add_queue( -1 );

		return 0;
	}
	/* Accept swimming. */
	if ( !strncmp( line, "swim ", 5 ) )
		line += 5;

	/* And burrow! */
	if ( !strncmp( line, "burrow ", 7 ) )
		line += 7;

	/* And crashing */
	if ( !strncmp( line, "crash ", 6 ) )
		line += 6;

	/* And tumbling */
	if ( !strncmp( line, "tumble ", 7 ) )
		line += 7;

	/* and leap */
	if ( !strncmp( line, "leap ", 5 ) )
		line += 5;

	/* Same with Saboteur/Idrasi evading. */
	if ( !strncmp( line, "evade ", 6 ) )
		line += 6;

	if ( !strncmp( line, "fade ", 5 ) )
		line += 5;
	if ( !strncmp( line, "gallop ", 7) )
	{for ( i = 1; dir_name[i]; i++ )
		{
			if ( !strcmp( line + 7, dir_name[i] ) ||
					!strcmp( line + 7, dir_small_name[i] ) )
			{lastdir=i;}
		}}
	/* sandling */
	if ( !strncmp( line, "order sandling burrow ", 22 ) )
		line += 22;

	/* Sentinel Drag */
	if ( !strncmp( line, "drag ", 5) )
		line += 5;

	/* Syssin Scale */
	if ( !strncmp(line, "scale ",6) )
		line += 6;

	/* sand sink */
	if ( !strncmp(line, "sand sink ",10) )
		line += 10;

	/* Farsight area only */
	if ( !strncmp( line, "farsee ", 7) )
	{areaonly = 1;
		farsight = 1;
	}

	/* Catch divine name for beseech */
	if ( !cmp("beseech ^ for shrinesight",line) ) {
		get_string(line + 7, ssightdiv ,256);
		clientff(C_R"\r\n[ANY shrinesight list will now in creating mode set shrines to belong to %s]\r\n"C_0,ssightdiv);}

	if ( mode == FOLLOWING && current_room && current_room->special_exits )
	{
		EXIT_DATA *spexit;
		for ( spexit = current_room->special_exits; spexit; spexit = spexit->next )
		{if ( spexit->command && !spexit->message && spexit->nomsg && !strcmp( line, spexit->command ) )
			{ /* continue from here */
				{
					if ( spexit->to )
					{
						current_room = spexit->to;
						current_area = current_room->area;
						if ( !spexit->nolook )
						{add_queue_top( -1 );}
						else
						{if ( auto_walk )
							auto_walk = 2;}
						if ( spexit->command ) {
							int i;
							for ( i = 1; dir_name[i]; i++ ) {
								if ( strstr(spexit->command, dir_name[i] ) ) {
									spexitwalk = 1;
									break;
								}
							}
						}
					}
					else
					{
						mode = GET_UNLOST;
					}
					if ( !disable_automap ) {
						didmove = 1;}
				}
			}
		}
	}

	for ( i = 1; dir_name[i]; i++ )
	{
		if ( !strcmp( line, dir_name[i] ) ||
				!strcmp( line, dir_small_name[i] ) )
		{
			if ( mode == FOLLOWING && current_room && current_room->special_exits )
			{
				EXIT_DATA *spexit;

				for ( spexit = current_room->special_exits; spexit; spexit = spexit->next )
				{
					if ( spexit->alias && spexit->command && spexit->alias == i )
					{
						exitparse = spexit->command;
						for ( p = exitparse; *p; p++ )
							if ( *p == '$' )
								*p = '\n';
						send_to_server( exitparse );
						send_to_server( "\r\n" );
						for ( p = exitparse; *p; p++ )
							if ( *p == '\n' )
								*p = '$';
						return 1;
					}
				}
			}
			/* special exit fix | temporary disabled -1 thing to check what it actually does */
			if ( !spexitwalk && !capture_special_exit && ( mode == CREATING || mode == FOLLOWING ) ) {
				/*          if ( mode == FOLLOWING && i == 10 && !current_room->exits[10] )
							add_queue( -1 );
							else  */
				lastdir = i;
				add_queue( i );

			}
			if ( capture_special_exit ) {     if ( strcmp(cse_command, "") == 0 ) {
				strcpy( cse_command, dir_name[i] );}
			else {strcat(cse_command, "$");
				strcat(cse_command, dir_name[i]);}}
			return 0;
		}
	}

	if ( !strcmp( line, "stop" ) )
	{
		if ( auto_walk != 0 )
		{
			clientfr( "Okay, I'm frozen." );
			auto_walk = 0;
			justwarped = 0;
			dash_command = NULL;
			except = NULL;
			if ( auto_bump ) {
				void do_map_bump( char *arg );
				do_map_bump("continue");}
			return 1;
		}
		else if ( capture_special_exit )
		{
			capture_special_exit = 0;
			clientfr( "Capturing disabled." );
			show_prompt( );
			return 1;
		}
	}

	/* Process from command table. */

	l = line;

	/* First, check for compound commands. */
	if ( !strncmp( l, "room ", 5 ) )
		base_cmd = CMD_ROOM, l += 5;
	else if ( !strncmp( l, "r ", 2 ) )
		base_cmd = CMD_ROOM, l += 2;
	else if ( !strncmp( l, "R", 1 ) )
		base_cmd = CMD_ROOM, l += 1;
	else if ( !strncmp( l, "area ", 5 ) )
		base_cmd = CMD_AREA, l += 5;
	else if ( !strncmp( l, "a ", 2 ) )
		base_cmd = CMD_AREA, l += 2;
	else if ( !strncmp( l, "A", 1 ) )
		base_cmd = CMD_AREA, l += 1;
	else if ( !strncmp( l, "map ", 4 ) )
		base_cmd = CMD_MAP, l += 4;
	else if ( !strncmp( l, "m ", 2 ) )
		base_cmd = CMD_MAP, l += 2;
	else if ( !strncmp( l, "M", 1 ) )
		base_cmd = CMD_MAP, l += 1;
	else if ( !strncmp( l, "exit ", 5 ) )
		base_cmd = CMD_EXIT, l += 5;
	else if ( !strncmp( l, "ex ", 3 ) )
		base_cmd = CMD_EXIT, l += 3;
	else if ( !strncmp( l, "E", 1 ) )
		base_cmd = CMD_EXIT, l += 1;
	else
		base_cmd = CMD_NONE;

	l = get_string( l, command, 4096 );

	if ( !command[0] )
		return 0;

	for ( i = 0; cmd_table[i].name; i++ )
	{
		/* If a normal command, compare it full.
		 * If after a base command, it's okay to abbreviate it.. */
		if ( base_cmd == cmd_table[i].base_cmd &&
				( ( base_cmd != CMD_NONE && !strncmp( command, cmd_table[i].name, strlen( command ) ) ) ||
				  ( base_cmd == CMD_NONE && !strcmp( command, cmd_table[i].name ) ) ) )
		{
			(cmd_table[i].func)( l );
			show_prompt( );
			return 1;
		}
	}

	/* A little shortcut to 'exit link'. */
	if ( base_cmd == CMD_EXIT && isdigit( command[0] ) )
	{
		do_exit_link( command );
		show_prompt( );
		return 1;
	}
	if ( base_cmd == CMD_MAP && isdigit( command[0] ) )
	{
		do_map( command );
		show_prompt( );
		return 1;
	}

	if ( capture_special_exit )
	{
		if ( strcmp(cse_command, "") == 0 ) {
			strcpy( cse_command, line );}
		else {strcat(cse_command, "$");
			strcat(cse_command, line);}
		clientff( C_R "[Command changed to '%s'.]\r\n" C_0, cse_command );
	}

	return 0;
}
