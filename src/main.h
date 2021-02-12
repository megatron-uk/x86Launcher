/* main.h, Global data structures for the x68Launcher.
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

#define MY_NAME "x86launcher"

#define SELECTION_LIST_SIZE			999 // Maximum number of entries in the game browser

#define FILTER_NONE		0
#define FILTER_GENRE		1
#define FILTER_SERIES	2
#define FILTER_COMPANY	3
#define FILTER_TECH		4
#define FILTER_MAX		4
#define START_MAIN		0
#define START_ALT		1

#define MAXIMUM_FILTER_STRINGS 200
#define MAXIMUM_SELECTED_STRINGS 30
#define MAXIMUM_FILTER_STRINGS_PER_PAGE 33
#define MAXIMUM_FILTER_STRINGS_PER_COL 11

typedef struct state {
	unsigned int selected_list[SELECTION_LIST_SIZE];		// A list of game ID's which are currently selected
	unsigned int selected_max;			// Number of items in the current selected list
	unsigned char selected_page;			// Page 'N' of the selected list
	unsigned char selected_line;			// The line in the page indicating the selected game
	unsigned char total_pages;			// Total number of pages in the selected_list
	unsigned char active_pane;			// The pane which currently has focus
	unsigned char selected_start;		// Which start file to launch, 0==start, 1==alt_start
	
	int selected_filter;					// Which filter to use, 0==none, 1==genre, 2==series
	int selected_filter_string;			// Which filter string is selected for non=multichoice filters
	
	unsigned int available_filter_strings; // How many filter strings are currently available
	unsigned char available_filter_pages;	// How many pages of filter strings are available
	unsigned char current_filter_page;	// Which page of filter strings is currently selected
	
	// Info about selected item
	int selected_gameid;					// Currently selected gameid
	gamedata_t *selected_game;			// Currently selected gamedata item
	unsigned char has_launchdat;			// Current game has metadata
	unsigned char has_images;			// Current game has artwork
	char selected_image[65];				// path + filename of current artwork
	
	// Filter list
	char filter_strings[MAXIMUM_FILTER_STRINGS][MAX_STRING_SIZE];
	unsigned char filter_strings_selected[MAXIMUM_FILTER_STRINGS]; // 1 or 0 to indicate if the string at this position is selected
	
} state_t;
