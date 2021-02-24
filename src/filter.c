/* filter.c, Apply sorting and filtering to gamedata lists.
 Copyright (C) 2020  John Snowdon
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef __HAS_DATA
#include "data.h"
#define __HAS_DATA
#endif
#ifndef __HAS_MAIN
#include "main.h"
#define __HAS_MAIN
#endif
#include "filter.h"
#include "ui.h"

int compare(const void *op1, const void *op2){
	// As defined in the OpenWatcom C Library reference guide
    
    return(strcmp(*(const char**)op1, *(const char**)op1));
}

int sortFilterKeys(state_t *state, int items){
	// Sort the list of filter keys by name
	
	qsort(state->filter_strings, items, MAX_STRING_SIZE, compare);
	return FILTER_OK;
}

int filter_GetGenres(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat){
	// Get all of the genres set in game metadata
	
	int i;
	int a;
	int c;
	int found;
	int status;
	int next_pos;
	gamedata_t *gamedata_head;
	gamedata_head = gamedata; // Store first item
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Building genre keyword selection list\n", __FILE__, __LINE__);
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing filter keywords list\n", __FILE__, __LINE__);
	}
	// Empty list
	for(i =0; i <MAXIMUM_FILTER_STRINGS; i++){
		memset(state->filter_strings[i], '\0', MAX_STRING_SIZE);
	}
	
	i = 0;
	c = 0;
	next_pos = 0;
	while(gamedata != NULL){
		
		// Does game have metadata
		if (gamedata->has_dat){
			
			// Load launch metadata
			status = getLaunchdata(gamedata, filterdat);
			if (status == 0){
				
				// Does the genre field match?
				if (strcmp(filterdat->genre, "") != 0){
				
					// Does this genre already exist?
					found = 0;
					for(a=0;a<MAXIMUM_FILTER_STRINGS;a++){
						if (strcmp(state->filter_strings[a], filterdat->genre) == 0){
							found = 1;
						}
					}
					// This genre isn't found yet, add it to the list of keywords
					if (found == 0){
						if (FILTER_VERBOSE){
							printf("%s.%d\t Info - Found genre: [%s]\n", __FILE__, __LINE__, filterdat->genre);
						}
						strncpy(state->filter_strings[next_pos], filterdat->genre, MAX_STRING_SIZE);
						next_pos++;
					}
				}
			}
		}
		c++;
		gamedata = gamedata->next;
	}
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Sorting keywords\n", __FILE__, __LINE__);
	}
	sortFilterKeys(state, next_pos);
	if (FILTER_VERBOSE){
		for(a=0;a<next_pos;a++){
			printf("%s.%d\t Info - Keyword %d: [%s]\n", __FILE__, __LINE__, a, state->filter_strings[a]);
		}
	}
	
	state->available_filter_strings = next_pos;
	state->current_filter_page = 0;
	state->available_filter_pages = ceil(((float)next_pos / (float)MAXIMUM_FILTER_STRINGS_PER_PAGE));
	for(i = 0; i < MAXIMUM_FILTER_STRINGS; i++){
		state->filter_strings_selected[i] = 0;
	}
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Searched %d games\n", __FILE__, __LINE__, c);
		printf("%s.%d\t Total of %d genre filters added\n", __FILE__, __LINE__, next_pos);
		printf("%s.%d\t Total of %d pages of filters\n", __FILE__, __LINE__, state->available_filter_pages);
	} 
	return FILTER_OK;
}

int filter_GetSeries(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat){
	// Get all of the series names set in game metadata
	
	int i;
	int a;
	int c;
	int found;
	int status;
	int next_pos;
	gamedata_t *gamedata_head;
	gamedata_head = gamedata; // Store first item
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Building series keyword selection list\n", __FILE__, __LINE__);
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing filter keywords list\n", __FILE__, __LINE__);
	}
	// Empty list
	for(i =0; i <MAXIMUM_FILTER_STRINGS; i++){
		memset(state->filter_strings[i], '\0', MAX_STRING_SIZE);
	}
	
	i = 0;
	c = 0;
	next_pos = 0;
	while(gamedata != NULL){
		
		// Does game have metadata
		if (gamedata->has_dat){
			
			// Load launch metadata
			status = getLaunchdata(gamedata, filterdat);
			if (status == 0){
				
				// Does the genre field match?
				if (strcmp(filterdat->series, "") != 0){
				
					// Does this genre already exist?
					found = 0;
					for(a=0;a<MAXIMUM_FILTER_STRINGS;a++){
						if (strcmp(state->filter_strings[a], filterdat->series) == 0){
							found = 1;
						}
					}
					// This genre isn't found yet, add it to the list of keywords
					if (found == 0){
						if (FILTER_VERBOSE){
							printf("%s.%d\t Info - Found series: [%s]\n", __FILE__, __LINE__, filterdat->series);
						}
						strncpy(state->filter_strings[next_pos], filterdat->series, MAX_STRING_SIZE);
						next_pos++;
					}
				}
			}
		}
		c++;
		gamedata = gamedata->next;
	}
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Sorting keywords\n", __FILE__, __LINE__);
	}
	sortFilterKeys(state, next_pos);
	if (FILTER_VERBOSE){
		for(a=0;a<next_pos;a++){
			printf("%s.%d\t Info - Keyword %d: [%s]\n", __FILE__, __LINE__, a, state->filter_strings[a]);
		}
	}
	state->available_filter_strings = next_pos;
	state->current_filter_page = 0;
	state->available_filter_pages = ceil(((float)next_pos / (float)MAXIMUM_FILTER_STRINGS_PER_PAGE));
	for(i = 0; i < MAXIMUM_FILTER_STRINGS; i++){
		state->filter_strings_selected[i] = 0;
	}
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Searched %d games\n", __FILE__, __LINE__, c);
		printf("%s.%d\t Total of %d series filters added\n", __FILE__, __LINE__, next_pos);
		printf("%s.%d\t Total of %d pages of filters\n", __FILE__, __LINE__, state->available_filter_pages);
	}
	return FILTER_OK;
}

int filter_GetCompany(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat){
	// Get all of the companies set in game metadata
	
	int i;
	int a;
	int c;
	int found_dev;
	int found_pub;
	int status;
	int next_pos;
	gamedata_t *gamedata_head;
	gamedata_head = gamedata; // Store first item
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Building company keyword selection list\n", __FILE__, __LINE__);
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing filter keywords list\n", __FILE__, __LINE__);
	}
	// Empty list
	for(i =0; i <MAXIMUM_FILTER_STRINGS; i++){
		memset(state->filter_strings[i], '\0', MAX_STRING_SIZE);
	}
	
	i = 0;
	c = 0;
	next_pos = 0;
	while(gamedata != NULL){
		
		// Does game have metadata
		if (gamedata->has_dat){
			
			// Load launch metadata
			status = getLaunchdata(gamedata, filterdat);
			if (status == 0){
				
				// Does the developer or publisher field match?
				if ((strcmp(filterdat->developer, "") != 0) || (strcmp(filterdat->publisher, "") != 0)){
				
					// Does this company already exist?
					found_pub = 0;
					found_dev = 0;
					for(a=0;a<MAXIMUM_FILTER_STRINGS;a++){
						if (strcmp(state->filter_strings[a], filterdat->developer) == 0){
							found_dev = 1;
						}
						if (strcmp(state->filter_strings[a], filterdat->publisher) == 0){
							found_pub = 1;
						}
					}
					// This company isn't found yet, add it to the list of keywords
					if (found_dev == 0){
						if (FILTER_VERBOSE){
							printf("%s.%d\t Info - Found developer: [%s]\n", __FILE__, __LINE__, filterdat->developer);
						}
						strncpy(state->filter_strings[next_pos], filterdat->developer, MAX_STRING_SIZE);
						next_pos++;
					}
					if (found_pub == 0){
						if (FILTER_VERBOSE){
							printf("%s.%d\t Info - Found publisher: [%s]\n", __FILE__, __LINE__, filterdat->publisher);
						}
						strncpy(state->filter_strings[next_pos], filterdat->publisher, MAX_STRING_SIZE);
						next_pos++;
					}
				}
			}
		}
		c++;
		gamedata = gamedata->next;
	}
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Sorting keywords\n", __FILE__, __LINE__);
	}
	sortFilterKeys(state, next_pos);
	if (FILTER_VERBOSE){
		for(a=0;a<next_pos;a++){
			printf("%s.%d\t Info - Keyword %d: [%s]\n", __FILE__, __LINE__, a, state->filter_strings[a]);
		}
	}
	state->available_filter_strings = next_pos;
	state->current_filter_page = 0;
	state->available_filter_pages = ceil(((float)next_pos / (float)MAXIMUM_FILTER_STRINGS_PER_PAGE));
	for(i = 0; i < MAXIMUM_FILTER_STRINGS; i++){
		state->filter_strings_selected[i] = 0;
	}
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Searched %d games\n", __FILE__, __LINE__, c);
		printf("%s.%d\t Total of %d company filters added\n", __FILE__, __LINE__, next_pos);
		printf("%s.%d\t Total of %d pages of filters\n", __FILE__, __LINE__, state->available_filter_pages);
	} 
	return FILTER_OK;
}


int filter_GetTechSpecs(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat){
	// Get a list of tech specs that we can filter games on
	
	// Unlike genres, companies and series, these filters are not actually
	// user-defined within game metadata, instead we already know which
	// options are available.
	
	int i;
	int next_pos;
	next_pos = 0;
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Building company keyword selection list\n", __FILE__, __LINE__);
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing filter keywords list\n", __FILE__, __LINE__);
	}
	// Empty list
	for(i =0; i <MAXIMUM_FILTER_STRINGS; i++){
		memset(state->filter_strings[i], '\0', MAX_STRING_SIZE);
	}
	
	// Audio/Sound
	strncpy(state->filter_strings[next_pos], FILTER_STRING_AUDIO_BEEPER, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_AUDIO_TANDY, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_AUDIO_ADLIB, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_AUDIO_SB, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_AUDIO_MT32, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_AUDIO_GM, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_AUDIO_COVOX, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_AUDIO_DISNEY, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_AUDIO_GUS, MAX_STRING_SIZE);
	next_pos++;
	
	// Video
	strncpy(state->filter_strings[next_pos], FILTER_STRING_VIDEO_TEXT, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_VIDEO_HERCULES, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_VIDEO_TANDY, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_VIDEO_CGA, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_VIDEO_EGA, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_VIDEO_VGA, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_VIDEO_SVGA, MAX_STRING_SIZE);
	next_pos++;
	
	// CPU type
	strncpy(state->filter_strings[next_pos], FILTER_STRING_CPU_8086, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_CPU_80286, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_CPU_80386, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_CPU_80486, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_CPU_80586, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_RAM_XMS, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_RAM_EMS, MAX_STRING_SIZE);
	next_pos++;
	strncpy(state->filter_strings[next_pos], FILTER_STRING_MISC_DPMI, MAX_STRING_SIZE);
	next_pos++;
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Sorting keywords\n", __FILE__, __LINE__);
	}
	sortFilterKeys(state, next_pos);
	
	state->available_filter_strings = next_pos;
	state->current_filter_page = 0;
	state->available_filter_pages = ceil(((float)next_pos / (float)MAXIMUM_FILTER_STRINGS_PER_PAGE));
	for(i = 0; i < MAXIMUM_FILTER_STRINGS; i++){
		state->filter_strings_selected[i] = 0;
	}
	
	return FILTER_OK;
}

int filter_None(state_t *state, gamedata_t *gamedata){
	// Apply no filter to the list of gamedata - all games
	
	int i;
	gamedata_t *gamedata_head;
	
	gamedata_head = gamedata; // Store first item
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Building unfiltered selection list\n", __FILE__, __LINE__);
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing selection list\n", __FILE__, __LINE__);
	}
	// Empty list
	for(i =0; i <SELECTION_LIST_SIZE; i++){
		state->selected_list[i] = -1;
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing filter string list\n", __FILE__, __LINE__);
	}
	// Empty filter string list
	for(i =0; i <MAXIMUM_FILTER_STRINGS; i++){
		memset(state->filter_strings[i], '\0', MAX_STRING_SIZE);
	}
	
	i = 0;
	while(gamedata != NULL){
		if (FILTER_VERBOSE){
			printf("%s.%d\t Info - adding Game ID: [%d], %s\n", __FILE__, __LINE__, gamedata->gameid, gamedata->name);
		}
		state->selected_list[i] = gamedata->gameid;
		gamedata = gamedata->next;
		i++;
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Total of %d games in list\n", __FILE__, __LINE__, i);
	}
	gamedata = gamedata_head;	// Restore first item
	state->selected_max = i; 	// Number of items in selection list
	state->selected_page = 1;	// Start on page 1
	state->selected_line = 0;	// Start on line 0
	state->total_pages = 0;	
	state->selected_filter_string = 0;
	state->current_filter_page = 0;
	state->available_filter_pages = 0;
	state->selected_gameid = state->selected_list[0]; 	// Initial game is the 0th element of the selection list
	state->selected_game = getGameid(state->selected_gameid, gamedata);
	for(i = 0; i <= state->selected_max ; i++){
		if (i % ui_browser_max_lines == 0){
			state->total_pages++;
		}
	}
	for(i = 0; i < MAXIMUM_FILTER_STRINGS; i++){
		state->filter_strings_selected[i] = 0;
	}
	
	return FILTER_OK;
}

int filter_Genre(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat){
	// Filter all games on a specific genre string
	int i;
	int c;
	int status;
	gamedata_t *gamedata_head;
	char filter[MAX_STRING_SIZE];
	
	strncpy(filter, state->filter_strings[state->selected_filter_string], MAX_STRING_SIZE);
	
	gamedata_head = gamedata; // Store first item
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Building genre selection list [%s]\n", __FILE__, __LINE__, filter);
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing selection list\n", __FILE__, __LINE__);
	}
	// Empty list
	for(i =0; i <SELECTION_LIST_SIZE; i++){
		state->selected_list[i] = -1;
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing filter string list\n", __FILE__, __LINE__);
	}
	// Empty filter string list
	for(i =0; i <MAXIMUM_FILTER_STRINGS; i++){
		memset(state->filter_strings[i], '\0', MAX_STRING_SIZE);
	}
	
	state->selected_max = 0; 	
	state->selected_page = 1;	
	state->selected_line = 0;	
	state->total_pages = 0;		
	state->selected_gameid = -1;
	state->selected_game = NULL;
	
	i = 0;
	c = 0;
	while(gamedata != NULL){
		
		// Does game have metadata
		if (gamedata->has_dat){
			
			// Load launch metadata
			status = getLaunchdata(gamedata, filterdat);
			if (status == 0){
				
				if (FILTER_VERBOSE){
					printf("%s.%d\t Info - Checking %s == %s\n", __FILE__, __LINE__, filterdat->genre, filter);
				}
				
				// Does the genre field match?
				if (strncmp(filterdat->genre, filter, MAX_STRING_SIZE) == 0){
				
					if (FILTER_VERBOSE){
						printf("%s.%d\t Info - adding Game ID: [%d], %s\n", __FILE__, __LINE__, gamedata->gameid, gamedata->name);
					}
					
					// Add the selected gameid to the selection list
					state->selected_list[i] = gamedata->gameid;
					i++;
				}
			} else {
				if (FILTER_VERBOSE){
					printf("%s.%d\t Warning, metadata not loaded for Game ID: [%d], %s\n", __FILE__, __LINE__, gamedata->gameid, gamedata->name);
				}
			}
		}
		c++;
		gamedata = gamedata->next;
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Searched %d games\n", __FILE__, __LINE__, c);
		printf("%s.%d\t Total of %d filtered games in genre list\n", __FILE__, __LINE__, i);
	} 
	
	gamedata = gamedata_head;
	state->selected_max = i; 	// Number of items in selection list
	state->selected_page = 1;	// Start on page 1
	state->selected_line = 0;	// Start on line 0
	state->total_pages = 0;		
	state->selected_filter_string = 0;
	state->selected_gameid = state->selected_list[0]; 	// Initial game is the 0th element of the selection list
	state->selected_game = getGameid(state->selected_gameid, gamedata);
	for(i = 0; i <= state->selected_max ; i++){
		if (i % ui_browser_max_lines == 0){
			state->total_pages++;
		}
	}
	return FILTER_OK;
}

int filter_Series(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat){
	// Filter all games on a specific series string
	int i;
	int c;
	int status;
	gamedata_t *gamedata_head;
	char filter[MAX_STRING_SIZE];
	gamedata_head = gamedata; // Store first item
	
	strncpy(filter, state->filter_strings[state->selected_filter_string], MAX_STRING_SIZE);
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Building series selection list [%s]\n", __FILE__, __LINE__, filter);
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing selection list\n", __FILE__, __LINE__);
	}
	// Empty list
	for(i =0; i <SELECTION_LIST_SIZE; i++){
		state->selected_list[i] = -1;
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing filter string list\n", __FILE__, __LINE__);
	}
	// Empty filter string list
	for(i =0; i <MAXIMUM_FILTER_STRINGS; i++){
		memset(state->filter_strings[i], '\0', MAX_STRING_SIZE);
	}
	
	state->selected_max = 0; 	
	state->selected_page = 1;	
	state->selected_line = 0;	
	state->total_pages = 0;		
	state->selected_gameid = -1;
	state->selected_game = NULL;
	
	i = 0;
	c = 0;
	while(gamedata != NULL){
		
		// Does game have metadata
		if (gamedata->has_dat){
			
			// Load launch metadata
			status = getLaunchdata(gamedata, filterdat);
			if (status == 0){
				
				if (FILTER_VERBOSE){
					printf("%s.%d\t Info - Checking %s == %s\n", __FILE__, __LINE__, filterdat->series, filter);
				}
				
				// Does the genre field match?
				if (strncmp(filterdat->series, filter, MAX_STRING_SIZE) == 0){
				
					if (FILTER_VERBOSE){
						printf("%s.%d\t Info - adding Game ID: [%d], %s\n", __FILE__, __LINE__, gamedata->gameid, gamedata->name);
					}
					
					// Add the selected gameid to the selection list
					state->selected_list[i] = gamedata->gameid;
					i++;
				}
			} else {
				if (FILTER_VERBOSE){
					printf("%s.%d\t Warning, metadata not loaded for Game ID: [%d], %s\n", __FILE__, __LINE__, gamedata->gameid, gamedata->name);
				}
			}
		}
		c++;
		gamedata = gamedata->next;
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Searched %d games\n", __FILE__, __LINE__, c);
		printf("%s.%d\t Total of %d filtered games in series list\n", __FILE__, __LINE__, i);
	} 
	
	gamedata = gamedata_head;
	state->selected_max = i; 	// Number of items in selection list
	state->selected_page = 1;	// Start on page 1
	state->selected_line = 0;	// Start on line 0
	state->total_pages = 0;		
	state->selected_filter_string = 0;
	state->selected_gameid = state->selected_list[0]; 	// Initial game is the 0th element of the selection list
	state->selected_game = getGameid(state->selected_gameid, gamedata);
	for(i = 0; i <= state->selected_max ; i++){
		if (i % ui_browser_max_lines == 0){
			state->total_pages++;
		}
	}
	return FILTER_OK;
}

int filter_Company(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat){
	// Filter all games on a specific developer or publisher string
	int i;
	int c;
	int status;
	gamedata_t *gamedata_head;
	char filter[MAX_STRING_SIZE];
	
	strncpy(filter, state->filter_strings[state->selected_filter_string], MAX_STRING_SIZE);
	
	gamedata_head = gamedata; // Store first item
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Building company selection list [%s]\n", __FILE__, __LINE__, filter);
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing selection list\n", __FILE__, __LINE__);
	}
	// Empty list
	for(i =0; i <SELECTION_LIST_SIZE; i++){
		state->selected_list[i] = -1;
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Info - Clearing existing filter string list\n", __FILE__, __LINE__);
	}
	// Empty filter string list
	for(i =0; i <MAXIMUM_FILTER_STRINGS; i++){
		memset(state->filter_strings[i], '\0', MAX_STRING_SIZE);
	}
	
	state->selected_max = 0; 	
	state->selected_page = 1;	
	state->selected_line = 0;	
	state->total_pages = 0;		
	state->selected_gameid = -1;
	state->selected_game = NULL;
	
	i = 0;
	c = 0;
	while(gamedata != NULL){
		
		// Does game have metadata
		if (gamedata->has_dat){
			
			// Load launch metadata
			status = getLaunchdata(gamedata, filterdat);
			if (status == 0){
				
				if (FILTER_VERBOSE){
					printf("%s.%d\t Info - Checking %s OR %s == %s\n", __FILE__, __LINE__, filterdat->developer, filterdat->publisher, filter);
				}
				
				// Does the publisher or developer field match?
				if ((strncmp(filterdat->developer, filter, MAX_STRING_SIZE) == 0) || (strncmp(filterdat->publisher, filter, MAX_STRING_SIZE) == 0)){
				
					if (FILTER_VERBOSE){
						printf("%s.%d\t Yes - adding Game ID: [%d], %s\n", __FILE__, __LINE__, gamedata->gameid, gamedata->name);
					}
					
					// Add the selected gameid to the selection list
					state->selected_list[i] = gamedata->gameid;
					i++;
				}
			} else {
				if (FILTER_VERBOSE){
					printf("%s.%d\t Warning, metadata not loaded for Game ID: [%d], %s\n", __FILE__, __LINE__, gamedata->gameid, gamedata->name);
				}
			}
		}
		c++;
		gamedata = gamedata->next;
	}
	if (FILTER_VERBOSE){
		printf("%s.%d\t Searched %d games\n", __FILE__, __LINE__, c);
		printf("%s.%d\t Total of %d filtered games in company list\n", __FILE__, __LINE__, i);
	} 
	
	gamedata = gamedata_head;
	state->selected_max = i; 	// Number of items in selection list
	state->selected_page = 1;	// Start on page 1
	state->selected_line = 0;	// Start on line 0
	state->total_pages = 0;		
	state->selected_filter_string = 0;
	state->selected_gameid = state->selected_list[0]; 	// Initial game is the 0th element of the selection list
	state->selected_game = getGameid(state->selected_gameid, gamedata);
	for(i = 0; i <= state->selected_max ; i++){
		if (i % ui_browser_max_lines == 0){
			state->total_pages++;
		}
	}
	return FILTER_OK;
}

int filter_TechSpecs(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat){
	// Filter the list of games based on one or more selected technical criteria
	// set in the state->filter_strings_selected array
	
	char filter_string[MAX_STRING_SIZE];
	int i, f;
	int status;
	unsigned short added_list[SELECTION_LIST_SIZE];
	unsigned char continue_search;
	
	gamedata_t *gamedata_head;
	
	gamedata_head = gamedata; // Store first item
	
	if (FILTER_VERBOSE){
		printf("%s.%d\t Building tech specs selection list\n", __FILE__, __LINE__);
		printf("%s.%d\t Info - Clearing existing selection list\n", __FILE__, __LINE__);
	}
	// Empty list
	for(i =0; i <SELECTION_LIST_SIZE; i++){
		state->selected_list[i] = -1;
	}
	
	// Determine if we are filtering any of cpu/video/audio tech specs
	if (FILTER_VERBOSE){
		for(f = 0; f <MAXIMUM_FILTER_STRINGS; f++){
			if (state->filter_strings_selected[f] == 1){
				printf("%s.%d\t - Adding filter for: %s\n", __FILE__, __LINE__, state->filter_strings[f]);
			}
		}
	}
	
	i = 0;
	while(gamedata != NULL){
		
		if (FILTER_VERBOSE){
			printf("%s.%d\t Searching for matches in %s\n", __FILE__, __LINE__, gamedata->name);
		}
		// Does game have metadata
		if (gamedata->has_dat){
			status = getLaunchdata(gamedata, filterdat);
			// Was metadata loaded
			if (status == 0){
				continue_search = 1;
				// Yes, search for matching filter strings
				for(f = 0; f <MAXIMUM_FILTER_STRINGS; f++){
					
					// This filter is selected
					if (state->filter_strings_selected[f] == 1){

						// Audio search criteria .........................
						
						if (strcmp(state->filter_strings[f], FILTER_STRING_AUDIO_BEEPER) == 0){
							if (filterdat->hardware->beeper != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_AUDIO_TANDY) == 0){
							if (filterdat->hardware->tandy_audio != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_AUDIO_ADLIB) == 0){
							if (filterdat->hardware->adlib != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_AUDIO_SB) == 0){
							if (filterdat->hardware->soundblaster != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_AUDIO_MT32) == 0){
							if (filterdat->hardware->mt32 != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_AUDIO_GM) == 0){
							if (filterdat->hardware->gm != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_AUDIO_COVOX) == 0){
							if (filterdat->hardware->covox != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_AUDIO_DISNEY) == 0){
							if (filterdat->hardware->disney != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_AUDIO_GUS) == 0){
							if (filterdat->hardware->ultrasound != 1){
								continue_search = 0;
							}
						}
						
						// Video search criteria .........................
						if (strcmp(state->filter_strings[f], FILTER_STRING_VIDEO_TEXT) == 0){
							if (filterdat->hardware->text != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_VIDEO_HERCULES) == 0){
							if (filterdat->hardware->hercules != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_VIDEO_TANDY) == 0){
							if (filterdat->hardware->tandy_video != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_VIDEO_CGA) == 0){
							if (filterdat->hardware->cga != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_VIDEO_EGA) == 0){
							if (filterdat->hardware->ega != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_VIDEO_VGA) == 0){
							if (filterdat->hardware->vga != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_VIDEO_SVGA) == 0){
							if (filterdat->hardware->svga != 1){
								continue_search = 0;
							}
						}
						
						// CPU/Misc search criteria .........................
						if (strcmp(state->filter_strings[f], FILTER_STRING_CPU_8086) == 0){
							if (filterdat->hardware->cpu_8086 != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_CPU_80286) == 0){
							if (filterdat->hardware->cpu_286 != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_CPU_80386) == 0){
							if (filterdat->hardware->cpu_386 != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_CPU_80486) == 0){
							if (filterdat->hardware->cpu_486 != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_CPU_80586) == 0){
							if (filterdat->hardware->cpu_586 != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_RAM_XMS) == 0){
							if (filterdat->hardware->ram_xms != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_RAM_EMS) == 0){
							if (filterdat->hardware->ram_ems != 1){
								continue_search = 0;
							}
						}
						if (strcmp(state->filter_strings[f], FILTER_STRING_MISC_DPMI) == 0){
							if (filterdat->hardware->dpmi != 1){
								continue_search = 0;
							}
						}
					}
					
					// The search is a composite AND statement,
					// if any of the results is false, then we
					// exit out of the search loop for this game.
					if (continue_search != 1){
						break;	
					}
				}
			}
			
			// If continue_search was still set, then this search must
			// have satisfied all critera. Therefore add it to the found list.
			if (continue_search == 1){
				if (FILTER_VERBOSE){
					printf("%s.%d\t - All criteria matched for %s\n", __FILE__, __LINE__, filterdat->realname);
				}
				state->selected_list[i] = gamedata->gameid;
				i++;	
			}
		}
		gamedata = gamedata->next;
	}
	
	gamedata = gamedata_head;	// Restore head
	state->selected_max = i; 	// Number of items in selection list
	state->selected_page = 1;	// Start on page 1
	state->selected_line = 0;	// Start on line 0
	state->total_pages = 0;		
	state->selected_filter_string = 0;
	state->selected_gameid = state->selected_list[0]; 	// Initial game is the 0th element of the selection list
	state->selected_game = getGameid(state->selected_gameid, gamedata);
	for(i = 0; i <= state->selected_max ; i++){
		if (i % ui_browser_max_lines == 0){
			state->total_pages++;
		}
	}
	
	return FILTER_OK;
	
}
