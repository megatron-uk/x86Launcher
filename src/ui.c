/* ui.c, Draws the user interface for the x68Launcher.
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

#include "data.h"
#include "ui.h"

#ifndef __HAS_GFX
#include "gfx.h"
#define __HAS_GFX
#endif

#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

#ifndef __HAS_PAL
#include "palette.h"
#define __HAS_PAL
#endif

// bmpdata_t structures are needed permanently for all ui 
// bitmap elements, as we may need to repaint the screen at
// periodic intervals after having dialogue boxes or menus open.
bmpdata_t 	*ui_checkbox_bmp;
bmpdata_t 	*ui_checkbox_choose_bmp;
bmpdata_t 	*ui_checkbox_empty_bmp;
bmpdata_t 	*ui_main_bmp;		// We only read the header of this, use gfx_BitmapAsync to display
bmpdata_t 	*ui_list_bmp;		// We only read the header of this, use gfx_BitmapAsync to display
bmpdata_t 	*ui_title_bmp;
bmpdata_t 	*ui_year_bmp;
bmpdata_t 	*ui_genre_bmp;
bmpdata_t 	*ui_company_bmp;
bmpdata_t 	*ui_series_bmp;
bmpdata_t 	*ui_select_bmp;
bmpdata_t 	*ui_path_bmp;
bmpdata_t	*ui_font_bmp;		// Generic, just used during loading each font and then freed

//bmpstate_t structures are needed for anything we don't load in
// its entirety - ie bitmaps that are too big.
bmpstate_t 	*ui_main_bmpstate;		// We only read the header, so the bmpstate is used to load, line-by-line

// Fonts
fontdata_t      *ui_font;

// We should only need a couple of file handles, as we'll load all of the ui
// bitmap assets sequentially.... just remember to close it at the 
// end of each function!
// One is opened and closed when reading entire bitmap assets
// The other is opened and closed when reading bmpstate assets line-by-line
FILE 		*ui_asset_reader;
FILE			*ui_mainstate_reader;

// Status of UI asset loading
static int      ui_fonts_status;
static int      ui_assets_status;


void ui_Init(){
	// Set the basic palette entries for all the user interface elements
	// NOT including any bitmaps we load - just the basic colours
	
	// Reset all palette entries
	pal_ResetAll();
	
	// Set basic UI palette entries
	pal_SetUI();	
}

void ui_Close(){
	if (ui_assets_status == UI_ASSETS_LOADED){
		if (UI_VERBOSE){
			printf("%s.%d\t ui_Close() Freeing bitmap assets memory\n", __FILE__, __LINE__);
		}
		bmp_Destroy(ui_checkbox_bmp);
		bmp_Destroy(ui_checkbox_choose_bmp);
		bmp_Destroy(ui_checkbox_empty_bmp);
		bmp_Destroy(ui_main_bmp);
		bmp_Destroy(ui_list_bmp);
		bmp_Destroy(ui_title_bmp);
		bmp_Destroy(ui_year_bmp);
		bmp_Destroy(ui_genre_bmp);
		bmp_Destroy(ui_company_bmp);
		bmp_Destroy(ui_series_bmp);
		bmp_Destroy(ui_path_bmp);
		bmp_Destroy(ui_select_bmp);
	}
	
	// Free main background
	if (UI_VERBOSE){
		printf("%s.%d\t ui_Close() Freeing main background memory\n", __FILE__, __LINE__);
	}
	free(ui_main_bmpstate);
	
	// Free font
	if (UI_VERBOSE){
			printf("%s.%d\t ui_Close() Freeing font assets memory\n", __FILE__, __LINE__);
		}
	bmp_DestroyFont(ui_font);
	
	// Close file handles
	if (UI_VERBOSE){
		printf("%s.%d\t ui_Close() Closing file handles\n", __FILE__, __LINE__);
	}
	fclose(ui_asset_reader);
	fclose(ui_mainstate_reader);
	
	if (UI_VERBOSE){
		printf("%s.%d\t ui_Close() UI fully deinitialised\n", __FILE__, __LINE__);
	}
}

int ui_DisplayArtwork(FILE *screenshot_file, bmpdata_t *screenshot_bmp, bmpstate_t *screenshot_state, state_t *state, imagefile_t *imagefile){

	int status;
	int has_screenshot;
	char msg[65];
	
	// Restart artwork display
	// =======================
	// Close previous screenshot file handle
	// =======================
	if (screenshot_file != NULL){
		fclose(screenshot_file);
		screenshot_file = NULL;
	}
	
	// Clear artwork window
	gfx_BoxFill(ui_artwork_xpos, ui_artwork_ypos, ui_artwork_xpos + ui_artwork_width, ui_artwork_ypos + ui_artwork_height, PALETTE_UI_BLACK);
	
	// Construct full path of image
	sprintf(msg, "%s\\%s", state->selected_game->path, imagefile->filename[imagefile->selected]);
	strcpy(state->selected_image, msg);
	if (UI_VERBOSE){
		printf("%s.%d\t ui_DisplayArtwork() Selected artwork [%d] filename [%s]\n", __FILE__, __LINE__, imagefile->selected, imagefile->filename[imagefile->selected]);
	}
	
	// =======================
	// Open new screenshot file, ready parse
	// =======================
	if (UI_VERBOSE){
		printf("%s.%d\t ui_DisplayArtwork() Opening artwork file\n", __FILE__, __LINE__);	
	}
	screenshot_file = fopen(state->selected_image, "rb");
	if (screenshot_file == NULL){
		if (UI_VERBOSE){
			printf("%s.%d\t ui_DisplayArtwork() Error, unable to open artwork file\n", __FILE__, __LINE__);	
		}
		has_screenshot = 0;
	} 
	else {
		// =======================
		// Load header of screenshot bmp
		// =======================
		if (UI_VERBOSE){
			printf("%s.%d\t ui_DisplayArtwork() Reading BMP header\n", __FILE__, __LINE__);	
		}
		pal_ResetFree();
		status = bmp_ReadImage(screenshot_file, screenshot_bmp, 1, 1, 0);
		has_screenshot = 1;	
		if (has_screenshot){
			screenshot_state->rows_remaining = screenshot_bmp->height;
			status = gfx_BitmapAsyncFull(ui_artwork_xpos + ((ui_artwork_width - screenshot_bmp->width) / 2) , ui_artwork_ypos + ((ui_artwork_height - screenshot_bmp->height) / 2), screenshot_bmp, ui_asset_reader, screenshot_state, 0, 0);
		}
	}
	if (UI_VERBOSE){
		printf("%s.%d\t ui_DisplayArtwork() Call to display %s complete\n", __FILE__, __LINE__, imagefile->filename[imagefile->selected]);	
	}
	if (screenshot_file != NULL){
		fclose(screenshot_file);
		screenshot_file = NULL;
	}
	return UI_OK;
}

int	ui_DrawConfirmPopup(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat){
	// Draw a confirmation box to start the game
	
	// Draw drop-shadow
	//gfx_BoxFillTranslucent(ui_launch_popup_xpos + 60, ui_launch_popup_ypos - 30, ui_launch_popup_xpos + 260, ui_launch_popup_ypos + 50, PALETTE_UI_DGREY);
	
	// Draw main box
	gfx_BoxFill(ui_launch_popup_xpos + 50, ui_launch_popup_ypos - 40, ui_launch_popup_xpos + 250, ui_launch_popup_ypos + 40, PALETTE_UI_BLACK);
	
	// Draw main box outline
	gfx_Box(ui_launch_popup_xpos + 50, ui_launch_popup_ypos - 40, ui_launch_popup_xpos + 250, ui_launch_popup_ypos + 40, PALETTE_UI_LGREY);
	
	gfx_Puts(ui_launch_popup_xpos + 110, ui_launch_popup_ypos - 30, ui_font, "Start Game?");
	
	gfx_Puts(ui_launch_popup_xpos + 60, ui_launch_popup_ypos - 5, ui_font, "Confirm (Enter)");
	gfx_Puts(ui_launch_popup_xpos + 60, ui_launch_popup_ypos + 15, ui_font, "Cancel (Esc)");
	
	
	return UI_OK;
}

int ui_DrawFilterPrePopup(state_t *state, int toggle){
	// Draw a popup that allows the user to toggle filter mode between genre, series and off

	// Draw drop-shadow
	//gfx_BoxFillTranslucent(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 10, ui_launch_popup_xpos + 10 + ui_launch_popup_width, ui_launch_popup_ypos + 10 + ui_launch_popup_height + 30, PALETTE_UI_DGREY);
	
	// Draw main box
	gfx_BoxFill(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height + 90, PALETTE_UI_BLACK);
	
	// Draw main box outline
	gfx_Box(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height + 90, PALETTE_UI_LGREY);
	
	// Box title
	gfx_Puts(ui_launch_popup_xpos + 90, ui_launch_popup_ypos + 10, ui_font, "Enable Filter?");
	// No filter text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 35, ui_font, "No filter - Show all games");
	// Genre filter text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 65, ui_font, "By Genre");
	// Series filter text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 95, ui_font, "By Series");
	// Copmany filter text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 125, ui_font, "By Company");
	// Tech specs filter text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 155, ui_font, "By Technical Specs");
	
	// Toggle which entry is selected
	if (toggle == 1){
		state->selected_filter++;	
	} 
	if (toggle == -1){
		state->selected_filter--;	
	}
	
	// Detect wraparound
	if (state->selected_filter >= FILTER_MAX){
		state->selected_filter = FILTER_MAX;
	}
	if (state->selected_filter < 1){
		state->selected_filter = FILTER_NONE;
	}
	
	if (state->selected_filter == FILTER_NONE){
		// none
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 125, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 155, ui_checkbox_empty_bmp);
	}
	if (state->selected_filter == FILTER_GENRE){
		// genre
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 125, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 155, ui_checkbox_empty_bmp);
	}
	if (state->selected_filter == FILTER_SERIES){
		// series
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 125, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 155, ui_checkbox_empty_bmp);
	}
	if (state->selected_filter == FILTER_COMPANY){
		// developer/company/publisher
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 125, ui_checkbox_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 155, ui_checkbox_empty_bmp);
	}
	if (state->selected_filter == FILTER_TECH){
		// developer/company/publisher
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 125, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 155, ui_checkbox_bmp);
	}
	
	return UI_OK;
}

int ui_DrawMultiChoiceFilterPopup(state_t *state, int select, int redraw, int toggle){
	// Draw a multi-choice filter window - more than one filter can be selected at the
	// same time.
	
	int offset;
	int min_selection;
	int max_selection;
	int i;
	int page_i;
	int status;
	char msg[128]; // Title
	
	if (redraw == 0){
		// Title for tech specs multi-choice filter
		if (state->selected_filter == FILTER_TECH){
			// Window title
			sprintf(msg, "Select Tech Specs - Page %d/%d - Space to toggle, Enter to confirm", state->current_filter_page + 1, state->available_filter_pages);
			gfx_Puts(60, 45, ui_font, msg);
		}
		// Any other multi-choice filter...
		// ...
	}
	
	// Move the selection through the on-screen choices
	if (select == -1){
		state->selected_filter_string--;
	}
	if (select == 1){
		state->selected_filter_string++;
	}
		
	// We only draw filter string choices from
	// the currently selected page of filter strings,
	// which is held in the state->current_filter_page variable.
	offset = state->current_filter_page * MAXIMUM_FILTER_STRINGS_PER_PAGE;
	min_selection = offset;
	
	// Are there more slots on this page than there are total strings?
	if (min_selection + MAXIMUM_FILTER_STRINGS_PER_PAGE > state->available_filter_strings){
		// Upper limit is the total number of strings
		max_selection = state->available_filter_strings;
	} else {
		// Upper limit is the offset plus the total strings on this page
		max_selection = 	offset + MAXIMUM_FILTER_STRINGS_PER_PAGE;
	}
	
	// Dont allow cursor to go below first string on page
	if (state->selected_filter_string < min_selection){
		state->selected_filter_string = min_selection;
	}
	// Dont allow cursor to go above last string on page
	if (state->selected_filter_string >= max_selection){
		state->selected_filter_string = max_selection - 1;
	}
	
	// Loop through and print the list of choices on this page, highlighting the currently
	// selected choice.
	for(i=offset; i<MAXIMUM_FILTER_STRINGS; i++){
		
		// We may (probably are) be starting part way
		// into the list of filter strings if we are on page > 1.
		page_i = i - offset;
		
		if ((state->filter_strings[i] != NULL) && (strcmp(state->filter_strings[i], "") != 0)){
		
			// Column 1
			if (page_i < MAXIMUM_FILTER_STRINGS_PER_COL){
				// Show the selection cursor
				if (i == state->selected_filter_string){
					gfx_Bitmap(45, 70 + (page_i * 25), ui_checkbox_choose_bmp);
				} else {
					if (state->filter_strings_selected[i] == 1){
						// This is selected, show the checkbox
						gfx_Bitmap(45, 70 + (page_i * 25), ui_checkbox_bmp);
					} else {
						// This is not selected
						gfx_Bitmap(45, 70 + (page_i * 25), ui_checkbox_empty_bmp);
					}
				}
				// Only print the text if we are painting an entirely new window
				if (redraw == 0){
					gfx_Puts(70, 70 + (page_i * 25), ui_font, state->filter_strings[i]);
				}
			}
			
			// Column 2
			if ((page_i >= MAXIMUM_FILTER_STRINGS_PER_COL) && (page_i < (2 * MAXIMUM_FILTER_STRINGS_PER_COL))){
				// Show the selection cursor
				if (i == state->selected_filter_string){
					gfx_Bitmap(230, 70 + ((page_i - 11) * 25), ui_checkbox_choose_bmp);
				} else {
					if (state->filter_strings_selected[i] == 1){
						// This is selected
						gfx_Bitmap(230, 70 + ((page_i - 11) * 25), ui_checkbox_bmp);
					} else {
						// This is not selected
						gfx_Bitmap(230, 70 + ((page_i - 11) * 25), ui_checkbox_empty_bmp);
					}
				}
				// Only print the text if we are painting an entirely new window
				if (redraw == 0){
					gfx_Puts(255, 70 + ((page_i - 11) * 25), ui_font, state->filter_strings[i]);
				}
			}
			
			// Column 3
			if ((page_i >= (2 * MAXIMUM_FILTER_STRINGS_PER_COL)) && (page_i < MAXIMUM_FILTER_STRINGS_PER_PAGE)){
				// Show the selection cursor
				if (i == state->selected_filter_string){
					gfx_Bitmap(420, 70 + ((page_i - 22) * 25), ui_checkbox_choose_bmp);
				} else {
					if (state->filter_strings_selected[i] == 1){
						// This is selected
						gfx_Bitmap(420, 70 + ((page_i - 22) * 25), ui_checkbox_bmp);
					} else {
						// This is not selected
						gfx_Bitmap(420, 70 + ((page_i - 22) * 25), ui_checkbox_empty_bmp);
					}
				}
				// Only print the text if we are painting an entirely new window
				if (redraw == 0){
					gfx_Puts(445, 70 + ((page_i - 22) * 25), ui_font, state->filter_strings[i]);
				}
			}	
		}
	}
	
	return UI_OK;
}

int ui_DrawFilterPopup(state_t *state, int select, int redraw, int toggle){
	// Draw a page of filter choices for the user to select
	// The 'redraw' parameter controls whether the text and background should be redrawn
	// on this page... if just scrolling between choices, only the 
	// checkbox bitmap is redrawn, all of the text and background can
	// be left as-is, and hence speed up the interface redraw.
	// The 'toggle' parameter controls whether we move the selection bitmap or not.
	
	int offset;
	int min_selection;
	int max_selection;
	int i;
	int page_i;
	int status;
	char msg[64]; // Title
	
	// Draw drop-shadow
	//gfx_BoxFillTranslucent(40, 50, GFX_COLS - 30, GFX_ROWS - 20, PALETTE_UI_DGREY);
	
	// The minimum and maximum index of the filter strings we can select
	min_selection = 0;
	max_selection = 0;
	
	if (redraw == 0){
		// Only paint the selection window if this is an entirely new window or new page of filters
		// Draw main box
		gfx_BoxFill(30, 40, GFX_COLS - 40, GFX_ROWS - 40, PALETTE_UI_BLACK);
		// Draw main box outline
		gfx_Box(30, 40, GFX_COLS - 40, GFX_ROWS - 40, PALETTE_UI_LGREY);
	}
	
	
	if (state->selected_filter == FILTER_TECH){
		// Tech specs filtering uses a multi-choice interface
		ui_DrawMultiChoiceFilterPopup(state, select, redraw, toggle);
	} else {
		
		// All other single-choice filters		
		// Only print the Window title if we are painting an entirely new window
		if (redraw == 0){
			if (state->selected_filter == FILTER_GENRE){
				sprintf(msg, "Select Genre - Page %d/%d - Enter to confirm", state->current_filter_page + 1, state->available_filter_pages);
				gfx_Puts(160, 45, ui_font, msg);
			}
			if (state->selected_filter == FILTER_SERIES){
				sprintf(msg, "Select Series - Page %d/%d - Enter to confirm", state->current_filter_page + 1, state->available_filter_pages);
				gfx_Puts(160, 45, ui_font, msg);
			}
			if (state->selected_filter == FILTER_COMPANY){
				sprintf(msg, "Select Company - Page %d/%d - Enter to confirm", state->current_filter_page + 1, state->available_filter_pages);
				gfx_Puts(160, 45, ui_font, msg);
			}
		}
		
		// Move the selection through the on-screen choices
		if (select == -1){
			state->selected_filter_string--;
		}
		if (select == 1){
			state->selected_filter_string++;
		}
			
		// We only draw filter string choices from
		// the currently selected page of filter strings,
		// which is held in the state->current_filter_page variable.
		offset = state->current_filter_page * MAXIMUM_FILTER_STRINGS_PER_PAGE;
		min_selection = offset;
		
		// Are there more slots on this page than there are total strings?
		if (min_selection + MAXIMUM_FILTER_STRINGS_PER_PAGE > state->available_filter_strings){
			// Upper limit is the total number of strings
			max_selection = state->available_filter_strings;
		} else {
			// Upper limit is the offset plus the total strings on this page
			max_selection = 	offset + MAXIMUM_FILTER_STRINGS_PER_PAGE;
		}
		 
		// Dont allow cursor to go below first string on page
		if (state->selected_filter_string < min_selection){
			state->selected_filter_string = min_selection;
		}
		// Dont allow cursor to go above last string on page
		if (state->selected_filter_string >= max_selection){
			state->selected_filter_string = max_selection - 1;
		}
		
		// Loop through and print the list of choices on this page, highlighting the currently
		// selected choice.
		for(i=offset; i<MAXIMUM_FILTER_STRINGS; i++){
			
			// We may (probably are) be starting part way
			// into the list of filter strings if we are on page > 1.
			page_i = i - offset;
			
			if ((state->filter_strings[i] != NULL) && (strcmp(state->filter_strings[i], "") != 0)){
			
				// Column 1
				if (page_i < MAXIMUM_FILTER_STRINGS_PER_COL){
					if (i == state->selected_filter_string){
						// This is selected
						gfx_Bitmap(45, 70 + (page_i * 25), ui_checkbox_bmp);
					} else {
						// This is not selected
						gfx_Bitmap(45, 70 + (page_i * 25), ui_checkbox_empty_bmp);
					}
					// Only print the text if we are painting an entirely new window
					if (redraw == 0){
						gfx_Puts(70, 70 + (page_i * 25), ui_font, state->filter_strings[i]);
					}
				}
				
				// Column 2
				if ((page_i >= MAXIMUM_FILTER_STRINGS_PER_COL) && (page_i < (2 * MAXIMUM_FILTER_STRINGS_PER_COL))){
					if (i == state->selected_filter_string){
						// This is selected
						gfx_Bitmap(230, 70 + ((page_i - 11) * 25), ui_checkbox_bmp);
					} else {
						// This is not selected
						gfx_Bitmap(230, 70 + ((page_i - 11) * 25), ui_checkbox_empty_bmp);
					}
					// Only print the text if we are painting an entirely new window
					if (redraw == 0){
						gfx_Puts(255, 70 + ((page_i - 11) * 25), ui_font, state->filter_strings[i]);
					}
				}
				
				// Column 3
				if ((page_i >= (2 * MAXIMUM_FILTER_STRINGS_PER_COL)) && (page_i < MAXIMUM_FILTER_STRINGS_PER_PAGE)){
					if (i == state->selected_filter_string){
						// This is selected
						gfx_Bitmap(420, 70 + ((page_i - 22) * 25), ui_checkbox_bmp);
					} else {
						// This is not selected
						gfx_Bitmap(420, 70 + ((page_i - 22) * 25), ui_checkbox_empty_bmp);
					}
					// Only print the text if we are painting an entirely new window
					if (redraw == 0){
						gfx_Puts(445, 70 + ((page_i - 22) * 25), ui_font, state->filter_strings[i]);
					}
				}	
			}
		}
	}
	return UI_OK;
	
}

int ui_DrawHelpPopup(){
	// Display the full-screen help text	
	
	// Draw main box
	gfx_BoxFill(30, 20, GFX_COLS - 40, GFX_ROWS - 20, PALETTE_UI_BLACK);
	// Draw main box outline
	gfx_Box(30, 20, GFX_COLS - 40, GFX_ROWS - 20, PALETTE_UI_LGREY);
	
	gfx_Puts(240, 25, ui_font, "X86Launcher - Help");
	
	// Key help
	gfx_Puts(40, 45, ui_font, "Key controls:");
	gfx_Puts(40, 65, ui_font, "- [F]      Bring up the game search/filter window");
	gfx_Puts(40, 85, ui_font, "- [H]      Show this help text window");
	gfx_Puts(40, 105, ui_font, "- [Q]      Quit the application");
	gfx_Puts(40, 125, ui_font, "- [Space]  Select a filter in a multi-select filter window");
	gfx_Puts(40, 145, ui_font, "- [Enter]  Confirm a filter choice or launch selected gameq");
	gfx_Puts(40, 165, ui_font, "- [Esc]    Close the current window or Cancel a selection");
	
	// Filter help
	gfx_Puts(40, 200, ui_font, "Search/Filter:");
	gfx_Puts(40, 220, ui_font, "You can search your list of games by [Genre], [Series], [Company] or");
	gfx_Puts(40, 240, ui_font, "by selecting one or more [Tech Specs] such as specific sound or audio");
	gfx_Puts(40, 260, ui_font, "device. Your games must have metadata [launch.dat] for this to work.");
	
	// Launching help
	gfx_Puts(40, 295, ui_font, "Game Browser:");
	gfx_Puts(40, 315, ui_font, "[Up] & [Down] scrolls through the list of games on a page. [PageUp]");
	gfx_Puts(40, 335, ui_font, "& [PageDown] jumps an entire page at a time. [Enter] launches the");
	gfx_Puts(40, 355, ui_font, "currently selected game. [Left] & [Right] scrolls through artwork.");
	
	return UI_OK;
}

int ui_DrawInfoBox(){
	// Draw a fresh info panel
	
	// Text boxes.....
	// Title
	// Year
	// Genre
	// Company
	// Series
	
	// Buttons.....
	// Metadata
	// Artwork
	// Start file
	// MIDI
	// MIDI RS
	// Filter
	
	return UI_OK;
}

int	ui_DrawLaunchPopup(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat, int toggle){
	// Draw the popup window that lets us select from the main or alternate start file
	// in order to launch a game
	
	int status;	
	
	// Draw drop-shadow
	//gfx_BoxFillTranslucent(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 10, ui_launch_popup_xpos + 10 + ui_launch_popup_width, ui_launch_popup_ypos + 10 + ui_launch_popup_height, PALETTE_UI_DGREY);
	
	// Draw main box
	gfx_BoxFill(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height, PALETTE_UI_BLACK);
	
	// Draw main box outline
	gfx_Box(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height, PALETTE_UI_LGREY);
	
	gfx_Puts(ui_launch_popup_xpos + 50, ui_launch_popup_ypos + 10, ui_font, "Select which file to run:");
	
	// Start file text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 35, ui_font, launchdat->start);
	
	// Alt start file text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 65, ui_font, launchdat->alt_start);
	
	if (toggle == 1){
		state->selected_start = !state->selected_start;	
	}
	
	if (state->selected_start == 0){
		// Checkbox for start
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);		
	} else {
		// Checkbox for alt_start
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_bmp);
	}
	
	return UI_OK;
}


int	ui_DrawMainWindow(){
	// Draw the background of the main user interface window
	
	int status;
	
	// Initially, we can use a single solid bmp, with all of the ui elements on, but later as we overlay other things on top of it,
	// we'll need to refresh various individual elements
	
	//status = gfx_Bitmap(0, 0, ui_main_bmp);
	status = gfx_BitmapAsyncFull(0, 0, ui_main_bmp, ui_mainstate_reader, ui_main_bmpstate, 1, 1);
	if (status == 0){
		return UI_OK;
	} else {
		return UI_ERR_FUNCTION_CALL;
	}
}

int ui_DrawSplash(){
	/*
		Show the initial splash screen which is shown during the loading
		process whilst bmp assets are loaded into ram, the disk is scanned
		for games and other stuff before the user can access the main menu.
	*/
	int			status;
	bmpdata_t 	*logo_bmp;
	bmpstate_t	*logo_bmpstate;
	
	// Load splash logo
	ui_asset_reader = fopen(splash_logo, "rb");
	if (ui_asset_reader == NULL){
		printf("%s.%d\t ui_DrawSplash() Unable to open file\n", __FILE__, __LINE__);
		return UI_ERR_FILE;	
	}
	
	// Set the palette entries for the splash logo
	pal_ResetFree();
	
	// ===============================================
	// 1. Copy the bitmap to the vram buffer one line at a time
	// ===============================================
	
	// 1a. Allocate enough space for the bitmap header
	logo_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (logo_bmp == NULL){
		printf("%s.%d\t ui_DrawSplash() Unable to allocate memory for splash bitmap.\n", __FILE__, __LINE__);
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	// 1b. Allocate enough space for the state structure and line buffer
	logo_bmpstate = (bmpstate_t *) malloc(sizeof(bmpstate_t));
	if (logo_bmpstate == NULL){
		printf("%s.%d\t ui_DrawSplash() Unable to allocate memory for splash bitmap state.\n", __FILE__, __LINE__);
		fclose(ui_asset_reader);
		bmp_Destroy(logo_bmp);
		return UI_ERR_BMP;
	}
	// 1c. Work through the bitmap file, reading one line at a time into the line buffer
	status = gfx_BitmapAsyncFull((GFX_COLS / 2) - 100, 100, logo_bmp, ui_asset_reader, logo_bmpstate, 1, 0);
	
	// Destroy any resources
	fclose(ui_asset_reader);
	bmp_Destroy(logo_bmp);
	free(logo_bmpstate);
	
	// Splash loaded okay
	return UI_OK;
	
}

int ui_DrawSplashProgress(int redraw, int progress_width){
	// Draw a progress bar
	int status;
	
	if (redraw){
		status = gfx_Box(
			splash_progress_x_pos, 
			splash_progress_y_pos, 
			splash_progress_x_pos + splash_progress_width, 
			splash_progress_y_pos + splash_progress_height, 
			PALETTE_UI_LGREY
			);      
		if (status != 0){
			return UI_ERR_FUNCTION_CALL;
		}
	}
	
	// Update progress bar to 'percent'
	status = gfx_BoxFill(
		splash_progress_x_pos + 2, 
		(splash_progress_y_pos + 2),
		(splash_progress_x_pos + 2 + abs(progress_width)),
		(splash_progress_y_pos + (splash_progress_height - 2)),
		PALETTE_UI_RED
	);
	if (status != 0){
		return UI_ERR_FUNCTION_CALL;
	}
	
	// Progress bar drawn okay
	return UI_OK;
}

int ui_LoadAssets(){
	// Load all UI bitmap assets from disk
	
	int status;
	
	// Default to assets not loaded
	ui_assets_status = UI_ASSETS_MISSING;
	
	// We load two types of bitmap assets
	// Small assets, which we read into memory entirely and can get gfx_Bitmap() to display
	// Large assets, which we only read the header of, and must use gfx_BitmapAsyncFull() to display
	
	// ==============================================
	// list select bmp - small asset
	// ==============================================
	ui_ProgressMessage("Loading browser select icon...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_select);
	}
	ui_asset_reader = fopen(ui_select, "rb");
	if (ui_asset_reader == NULL){
			ui_ProgressMessage("ERROR! Unable to open browser select icon file");
			return UI_ERR_FILE;     
	}
	ui_select_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_select_bmp == NULL){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for browser select icon.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for browser select icon");
		return UI_ERR_BMP;
	}
	
	ui_select_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_select_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_select_bmp, 1);
	if (status != 0){
		ui_ProgressMessage("ERROR! Unable to read pixel data for browser select icon");
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// ==============================================
	// Checkbox - small asset
	// ==============================================
	ui_ProgressMessage("Loading checkbox...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_check_box);
	}
	ui_asset_reader = fopen(ui_check_box, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to open checkbox icon file");
		return UI_ERR_FILE;     
	}
	ui_checkbox_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_checkbox_bmp == NULL){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for checkbox icon.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory of checkbox icon");
		return UI_ERR_BMP;
	}
	ui_checkbox_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_checkbox_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_checkbox_bmp, 1);
	if (status != 0){
		ui_ProgressMessage("ERROR! Unable to read pixel data for checkbox icon");
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// ==============================================
	// Checkbox choose - small asset
	// ==============================================
	ui_ProgressMessage("Loading checkbox chooser...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_check_box_choose);
	}
	ui_asset_reader = fopen(ui_check_box_choose, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to open checkbox chooser icon file");
		return UI_ERR_FILE;     
	}
	ui_checkbox_choose_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_checkbox_choose_bmp == NULL){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for checkbox chooser icon.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory of checkbox chooser icon");
		return UI_ERR_BMP;
	}
	ui_checkbox_choose_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_checkbox_choose_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_checkbox_choose_bmp, 1);
	if (status != 0){
		ui_ProgressMessage("ERROR! Unable to read pixel data for checkbox chooser icon");
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// ===============================================
	// Checkbox, empty - small asset
	// ===============================================
	ui_ProgressMessage("Loading checkbox (empty) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_check_box_unchecked);
	}
	ui_asset_reader = fopen(ui_check_box_unchecked, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to open checkbox (empty) icon file");
		return UI_ERR_FILE;     
	}
	ui_checkbox_empty_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_checkbox_empty_bmp == NULL){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for checkbox(empty) icon.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory of checkbox (empty) icon");
		return UI_ERR_BMP;
	}
	ui_checkbox_empty_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_checkbox_empty_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_checkbox_empty_bmp, 1);
	//pal_BMPRemap(ui_checkbox_empty_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			ui_ProgressMessage("ERROR! Unable to read pixel data for checkbox (empty) icon");
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// =========================================
	// Game title bar - small asset
	// =========================================
	ui_ProgressMessage("Loading text entry (title) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_title_box);
	}
	ui_asset_reader = fopen(ui_title_box, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to open title text-file");
		return UI_ERR_FILE;     
	}
	ui_title_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_title_bmp == NULL){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for title text.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for title-text");
		return UI_ERR_BMP;
	}
	ui_title_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_title_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_title_bmp, 1);
	if (status != 0){
		ui_ProgressMessage("ERROR! Unable to read pixel data for title-text");
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// =========================================
	// Game year bar - small asset
	// =========================================
	ui_ProgressMessage("Loading text entry (year) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_year_box);
	}
	ui_asset_reader = fopen(ui_year_box, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to open year text-file");
		return UI_ERR_FILE;     
	}
	ui_year_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_year_bmp == NULL){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for year text.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for year-text");
		return UI_ERR_BMP;
	}
	ui_year_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_year_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_year_bmp, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		ui_ProgressMessage("ERROR! Unable to read pixel data for year-text");
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// =========================================
	// Game genre bar - small asset
	// =========================================
	ui_ProgressMessage("Loading text entry (genre) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_genre_box);
	}
	ui_asset_reader = fopen(ui_genre_box, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to open genre-text file");
		return UI_ERR_FILE;     
	}
	ui_genre_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_genre_bmp == NULL){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for genre text.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for genre-text");
		return UI_ERR_BMP;
	}
	ui_genre_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_genre_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_genre_bmp, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		ui_ProgressMessage("ERROR! Unable to read pixel data for genre-text");
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// =========================================
	// Game company bar - small asset
	// =========================================
	ui_ProgressMessage("Loading text entry (company) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_company_box);
	}
	ui_asset_reader = fopen(ui_company_box, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to open company-text file");
		return UI_ERR_FILE;     
	}
	ui_company_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_company_bmp == NULL){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for company text.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for company-text");
		return UI_ERR_BMP;
	}
	ui_company_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_company_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_company_bmp, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		ui_ProgressMessage("ERROR! Unable to read pixel data for company-text");
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// =========================================
	// Game series bar - small asset
	// =========================================
	ui_ProgressMessage("Loading text entry (series) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_series_box);
	}
	ui_asset_reader = fopen(ui_series_box, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to open series-text file");
		return UI_ERR_FILE;     
	}
	ui_series_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_series_bmp == NULL){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for series text.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for series-text");
		return UI_ERR_BMP;
	}
	ui_series_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_series_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_series_bmp, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		ui_ProgressMessage("ERROR! Unable to read pixel data for series-text");
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// =========================================
	// Path bar - small asset
	// =========================================
	ui_ProgressMessage("Loading text entry (path) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_path_box);
	}
	ui_asset_reader = fopen(ui_path_box, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to open path-text file");
		return UI_ERR_FILE;     
	}
	ui_path_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_path_bmp == NULL){
		printf("%s.%d\t ui_LoadAssets() Unable to allocate memory for path text.\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for path-text");
		return UI_ERR_BMP;
	}
	ui_path_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_path_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_path_bmp, 1);
	if (status != 0){
		fclose(ui_asset_reader);
		ui_ProgressMessage("ERROR! Unable to read pixel data for path-text");
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// ===================================================================
	// LARGE ASSETS BELOW
	// ===================================================================
	
	// ===============================================
	// Main background - large asset
	// ===============================================
	ui_ProgressMessage("Loading main UI bg [header only]...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_main);
	}
	// 1a. Open file
	ui_mainstate_reader = fopen(ui_main, "rb");
	if (ui_mainstate_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to open main UI bg file");
		return UI_ERR_FILE;     
	}
	// 1b. Allocate enough space for the bitmap header
	ui_main_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_main_bmp == NULL){
		printf("%s.%d\t ui_DrawSplash() Unable to allocate memory for main UI bg\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for main UI bg");
		fclose(ui_mainstate_reader);
		return UI_ERR_BMP;
	}
	// 1c. Allocate enough space for the state structure and line buffer
	ui_main_bmpstate = (bmpstate_t *) malloc(sizeof(bmpstate_t));
	if (ui_main_bmpstate == NULL){
		printf("%s.%d\t ui_DrawSplash() Unable to allocate memory for main UI bg state\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for main UI bg state");
		fclose(ui_mainstate_reader);
		bmp_Destroy(ui_main_bmp);
		return UI_ERR_BMP;
	}
	// We DONT read the bitmap data at this point AND the file handle remains open
	
	// ===============================================
	// List window - large asset
	// ===============================================
	ui_ProgressMessage("Loading UI browser bg [header only]...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t ui_LoadAssets() Loading %s\n", __FILE__, __LINE__, ui_list_box);
	}
	// 1a. Open file
	ui_asset_reader = fopen(ui_list_box, "rb");
	if (ui_asset_reader == NULL){
		ui_ProgressMessage("ERROR! Unable to open browser bg file");
		return UI_ERR_FILE;     
	}
	// 1b. Allocate enough space for the bitmap header
	ui_list_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_list_bmp == NULL){
		printf("%s.%d\t ui_DrawSplash() Unable to allocate memory for browser bg\n", __FILE__, __LINE__);
		ui_ProgressMessage("ERROR! Unable to allocate memory for browser bg");
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	ui_list_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_list_bmp, 1, 0, 0);
	if (status != 0){
		fclose(ui_asset_reader);
		ui_ProgressMessage("ERROR! Unable to read header for browser bg");
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	// We DONT read the bitmap data at this point AND the file handle remains open
	
	// Set assets loaded status
	ui_assets_status = UI_ASSETS_LOADED;
	return UI_OK;
}

int ui_LoadFonts(){
	int status;
        
	// Default to assets not loaded
	ui_fonts_status = UI_ASSETS_MISSING;
	
	// =========================
	// main font
	// =========================
	ui_asset_reader = fopen(ui_font_name, "rb");
	if (ui_asset_reader == NULL){
		if (UI_VERBOSE){
				printf("%s.%d\t ui_LoadFonts() Error loading UI font data\n", __FILE__, __LINE__);
		}
		return UI_ERR_FILE;     
	}
	
	ui_font = (fontdata_t *) malloc(sizeof(fontdata_t));
	ui_font_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_font_bmp->pixels = NULL;
	status = bmp_ReadFont(ui_asset_reader, ui_font_bmp, ui_font, 1, 0, 0, ui_font_width, ui_font_height);
	status = bmp_ReadFont(ui_asset_reader, ui_font_bmp, ui_font, 0, 1, 0, ui_font_width, ui_font_height);
	pal_BMP2Palette(ui_font_bmp, 1);
	status = bmp_ReadFont(ui_asset_reader, ui_font_bmp, ui_font, 0, 0, 1, ui_font_width, ui_font_height);
	
	
	if (status != 0){
		if (UI_VERBOSE){
				printf("%s.%d\t ui_LoadFonts() Error processing UI font data\n", __FILE__, __LINE__);
		}
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	ui_font->ascii_start = ui_font_ascii_start;           
	ui_font->n_symbols = ui_font_total_syms;
	ui_font->unknown_symbol = ui_font_unknown;
	
	// Destroy the temporary bmp structure, as the font is now in a font structure
	bmp_Destroy(ui_font_bmp);
	
	return UI_OK;
}

int ui_ProgressMessage(char *c){
	int x;
	
	x = (GFX_COLS / 2) - ((ui_font->width * strlen(c)) / 2);

	// Mask out anything that was on this line before
	gfx_BoxFill(0, ui_progress_font_y_pos - 1, GFX_COLS, ui_progress_font_y_pos + ui_font->height, PALETTE_UI_BLACK);
	
	// Print the text
	return gfx_Puts(x, ui_progress_font_y_pos, ui_font, c);
}

int	ui_ReselectCurrentGame(state_t *state){
	// Simply updates the selected_gameid with whatever line / page is currently selected
	// Should be called every time up/down/pageup/pagedown is detected whilst in browser pane
	
	int	startpos;	// Index of first displayable element of state->selected_items
	int	endpos;		// Index to last displayable element of state->selected_items
	int 	i;			// Loop counter
	int	selected;	// Counter to match the selected_line number
	int	gameid;		// ID of the current game we are iterating through in the selected_list
	
	// Don't allow startpos to go negative
	startpos = (state->selected_page - 1) * ui_browser_max_lines;
	if (startpos < 0){
		startpos = 0;	
	}
	
	// If we're on the last page, then make sure we only loop over the number of entries
	// that are on this page... not just all 22
	if ((startpos + ui_browser_max_lines) > state->selected_max){
		endpos = state->selected_max;
	} else {
		endpos = startpos + ui_browser_max_lines;
	}
	
	if (UI_VERBOSE){
		printf("%s.%d\t ui_ReselectCurrentGame() Game: %d, endpos: %d, selected line: %d\n", __FILE__, __LINE__, startpos, endpos, state->selected_line);
	}
		
	selected = 0;
	for(i = startpos; i <= endpos ; i++){
		gameid = state->selected_list[i];
		
		// This is the current selected game
		if (selected == state->selected_line){
			//if (UI_VERBOSE){
			//	printf("%s.%d\t Page [%d/%d], Line [%d/%d], Game ID [%d]\n", __FILE__, __LINE__, state->selected_page, state->total_pages, state->selected_line, endpos, gameid);	
			//}
			state->selected_gameid = gameid;
			return UI_OK;
		}	
		selected++;
	}
	
	return UI_OK;
}

int ui_StatusMessage(char *c){
	// Output a status message in the status bar at the bottom of the screen in the main UI

	// Clear the progress message row
	//gfx_BoxFill(0, ui_progress_font_y_pos - 1, GFX_COLS, ui_progress_font_y_pos + ui_font->height, PALETTE_UI_BLACK);
	
	return gfx_Puts(0, 380, ui_font, c);
}

int ui_UpdateBrowserPane(state_t *state, gamedata_t *gamedata){
	// UPdate the contents of the game browser pane

	// selected_list : contains the gameids that are in the currently filtered selection (e.g. ALL, shooter genre only, only by Konami, etc)
	// selected_max : is the count of how many games are in the current selection
	// selected_page : is the page (browser list can show 0 - x items per page) into the selected_list
	// selected_line : is the line of the selected_page that is highlighted
	
	gamedata_t	*gamedata_head;	// Pointer to the start of the gamedata list, so we can restore it
	gamedata_t	*selected_game;	// Gamedata object for the currently selected line
	int			y;				// Vertical position offset for each row
	int 			i;				// Loop counter
	int 			gameid;			// ID of each game in selected_list
	char			msg[64];		// Message buffer for each row
	int			startpos;			// Index of first displayable element of state->selected_items
	int			endpos;			// Index to last displayable element of state->selected_items
	
	// Don't allow startpos to go negative
	startpos = (state->selected_page - 1) * ui_browser_max_lines;
	if (startpos < 0){
		startpos = 0;	
	}
	
	// If we're on the last page, then make sure we only draw the number of lines
	// that are on this page... not just all 22
	if ((startpos + ui_browser_max_lines) > state->selected_max){
		endpos = state->selected_max;
	} else {
		endpos = startpos + ui_browser_max_lines;
	}
	
	// Simple black fill
	gfx_BoxFill(ui_browser_panel_x_pos + 3, ui_browser_panel_y_pos + 3, ui_browser_panel_x_pos + ui_list_bmp->width - 3, ui_browser_panel_y_pos + ui_list_bmp->height - 3, PALETTE_UI_BLACK);
	
	// Display the entries for this page
	gamedata_head = gamedata;
	y = ui_browser_font_y_pos;
	if (UI_VERBOSE){
		printf("%s.%d\t ui_UpdateBrowserPane() Building browser menu [%d-%d]\n", __FILE__, __LINE__, startpos, endpos);
	}
	for(i = startpos; i < endpos ; i++){
		gamedata = gamedata_head;
		gameid = state->selected_list[i];
		selected_game = getGameid(gameid, gamedata);
		if (UI_VERBOSE){
			printf("%s.%d\t ui_UpdateBrowserPane() - Line %d: Game ID %d, %s\n", __FILE__, __LINE__, i, gameid, selected_game->name);
		}
		if (strlen(selected_game->name) > 30){
			sprintf(msg, "%.28s..", selected_game->name);
		} else {
			sprintf(msg, "%s", selected_game->name);
		}	
		gfx_Puts(ui_browser_font_x_pos, y, ui_font, msg);
		y += ui_font->height + 2;
	}
	gamedata = gamedata_head;
	
	return UI_OK;
}

int ui_UpdateBrowserPaneStatus(state_t *state){
	// Draw browser pane status message in status panel
	char	msg[64];		// Message buffer for the status bar
	int y_pos;
	// Blank out any previous selection cursor
	gfx_BoxFill(ui_browser_cursor_xpos, ui_browser_font_y_pos, ui_browser_cursor_xpos + ui_select_bmp->width, ui_browser_footer_font_ypos, PALETTE_UI_BLACK);

	// Insert selection cursor
	if (state->selected_line == 0){
		y_pos = 0;
	} else {
		y_pos = (ui_font->height + 2 ) * (state->selected_line);
	}
	if (UI_VERBOSE){
		printf("%s.%d\t ui_UpdateBrowserPaneStatus() Drawing selection icon at line %d, x:%d y:%d\n", __FILE__, __LINE__, state->selected_line, ui_browser_cursor_xpos, (ui_browser_font_y_pos + y_pos));
	}
	gfx_Bitmap(ui_browser_cursor_xpos, ui_browser_font_y_pos + y_pos, ui_select_bmp);
	
	// Text at bottom of browser pane
	sprintf(msg, "Line %02d/%02d             Page %02d/%02d", state->selected_line, ui_browser_max_lines, state->selected_page, state->total_pages);
	gfx_Puts(ui_browser_footer_font_xpos, ui_browser_footer_font_ypos, ui_font, msg);
	
	return UI_OK;
}

int ui_UpdateInfoPane(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat){
	// Draw the contents of the info panel with current selected game, current filter mode, etc
	
	// TO DO
	// Clear text on load
	// snprintf instead of sprintf to limit string sizes
	
	int		status;
	char		s1, s2;
	char		status_msg[64];		// Message buffer for anything needing to be printed onscreen
	char		info_name[64];
	char		info_year[8];
	char		info_company[32];
	char		info_path[64];
	char		info_genre[24];
	char		info_series[24];
	
	//gamedata_t	*gamedata_head;	// Pointer to the start of the gamedata list, so we can restore it
	//gamedata_t	*selected_game;	// Gamedata object for the currently selected line
	//launchdat_t	*launchdat;		// Metadata object representing the launch.dat file for this game
	
	// Store gamedata head
	//gamedata_head = gamedata;
	
	// Clear all existing text
	// title
	gfx_Bitmap(ui_info_name_xpos, ui_info_name_ypos, ui_title_bmp);
	// company
	gfx_Bitmap(ui_info_company_xpos, ui_info_company_ypos, ui_company_bmp);
	// path
	gfx_Bitmap(ui_info_path_xpos, ui_info_path_ypos, ui_path_bmp);
	// year
	gfx_Bitmap(ui_info_year_xpos, ui_info_year_ypos, ui_year_bmp);
	// genre
	gfx_Bitmap(ui_info_genre_xpos, ui_info_genre_ypos, ui_genre_bmp);
	// series
	gfx_Bitmap(ui_info_series_xpos, ui_info_series_ypos, ui_series_bmp);
	
	if (state->selected_filter != 0){
		gfx_Bitmap(ui_checkbox_filter_active_xpos, ui_checkbox_filter_active_ypos, ui_checkbox_bmp);
	} else {
		gfx_Bitmap(ui_checkbox_filter_active_xpos, ui_checkbox_filter_active_ypos, ui_checkbox_empty_bmp);	
	}
	
	if (UI_VERBOSE){
		printf("%s.%d\t ui_UpdateInfoPane() Selected State\n", __FILE__, __LINE__);
		printf("%s.%d\t ui_UpdateInfoPane() - Page: [%d]\n", __FILE__, __LINE__, state->selected_page);
		printf("%s.%d\t ui_UpdateInfoPane() - Line: [%d]\n", __FILE__, __LINE__, state->selected_line);
		printf("%s.%d\t ui_UpdateInfoPane() - selected game id: [%d]\n", __FILE__, __LINE__,  state->selected_gameid);
		printf("%s.%d\t ui_UpdateInfoPane() - retrieved game id: [%d]\n", __FILE__, __LINE__, state->selected_game->gameid);
		printf("%s.%d\t ui_UpdateInfoPane() - has_dat: [%d]\n", __FILE__, __LINE__, state->selected_game->has_dat);
		printf("%s.%d\t ui_UpdateInfoPane() - has_images: [%d]\n", __FILE__, __LINE__, state->has_images);
	}
	
	// See if it has a launch.dat metadata file
	if (state->selected_game != NULL){
		if (state->selected_game->has_dat != 0){
			if (launchdat == NULL){
				// ======================
				// Unable to load launch.dat	 from disk
				// ======================
				sprintf(status_msg, "ERROR: Unable to load metadata file: %s\%s", state->selected_game->path, GAMEDAT);
				gfx_Bitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_bmp);
				gfx_Bitmap(ui_checkbox_has_startbat_xpos, ui_checkbox_has_startbat_ypos, ui_checkbox_empty_bmp);
				gfx_Bitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);
				gfx_Bitmap(ui_checkbox_has_midi_xpos, ui_checkbox_has_midi_ypos, ui_checkbox_empty_bmp);
				gfx_Bitmap(ui_checkbox_has_midi_serial_xpos, ui_checkbox_has_midi_serial_ypos, ui_checkbox_empty_bmp);
				sprintf(info_name, " %.64s", state->selected_game->name);
				sprintf(info_year, "N/A");
				sprintf(info_company, " N/A");
				sprintf(info_genre, "N/A");
				sprintf(info_series, "");
				sprintf(info_path, " %.38s", state->selected_game->path);
			} else {
				// ======================
				// Loaded launch.dat from disk
				// ======================
				
				if (UI_VERBOSE){
					printf("%s.%d\t ui_UpdateInfoPane()  - metadata: yes\n", __FILE__, __LINE__);
					printf("%s.%d\t ui_UpdateInfoPane()  - artwork: [%d]\n", __FILE__, __LINE__, state->has_images);
					printf("%s.%d\t ui_UpdateInfoPane()  - start file: [%s]\n", __FILE__, __LINE__, launchdat->start);
					printf("%s.%d\t ui_UpdateInfoPane()  - alt_start file: [%s]\n", __FILE__, __LINE__, launchdat->alt_start);
					printf("%s.%d\t ui_UpdateInfoPane()  - midi: [%d]\n", __FILE__, __LINE__, launchdat->midi);
					printf("%s.%d\t ui_UpdateInfoPane()  - midi serial: [%s]\n", __FILE__, __LINE__, launchdat->midi_serial);
				}
				
				gfx_Bitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_bmp);
				
				if (state->has_images == 1){
					gfx_Bitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_bmp);
				} else {
					gfx_Bitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);	
				}
				
				if (launchdat->midi == 1){
					gfx_Bitmap(ui_checkbox_has_midi_xpos, ui_checkbox_has_midi_ypos, ui_checkbox_bmp);
				} else {
					gfx_Bitmap(ui_checkbox_has_midi_xpos, ui_checkbox_has_midi_ypos, ui_checkbox_empty_bmp);
				}
				
				//if (launchdat->midi_serial == 1){
				//	gfx_Bitmap(ui_checkbox_has_midi_serial_xpos, ui_checkbox_has_midi_serial_ypos, ui_checkbox_bmp);
				//} else {
				//	gfx_Bitmap(ui_checkbox_has_midi_serial_xpos, ui_checkbox_has_midi_serial_ypos, ui_checkbox_empty_bmp);
				//}
				
				if ((launchdat->start != NULL) && (strcmp(launchdat->start, "") != 0)){
					gfx_Bitmap(ui_checkbox_has_startbat_xpos, ui_checkbox_has_startbat_ypos, ui_checkbox_bmp);
				} else {
					gfx_Bitmap(ui_checkbox_has_startbat_xpos, ui_checkbox_has_startbat_ypos, ui_checkbox_empty_bmp);
				}
				
				if (strlen(launchdat->realname) > 0){
					sprintf(info_name, " %.48s", launchdat->realname);
				} else {
					sprintf(info_name, " %.48s", state->selected_game->name);
				}
				if (UI_VERBOSE){
					printf("%s.%d\t ui_UpdateInfoPane()  - name: %s\n", __FILE__, __LINE__, info_name);
				}
				
				if (launchdat->genre != NULL){
					sprintf(info_genre, "%.24s", launchdat->genre);
				} else {
					sprintf(info_genre, "N/A");
				}
				if (UI_VERBOSE){
					printf("%s.%d\t ui_UpdateInfoPane()  - genre: %s\n", __FILE__, __LINE__, info_genre);
				}
				
				if (launchdat->series != NULL){
					sprintf(info_series, "%.24s", launchdat->series);
				} else {
					sprintf(info_series, "N/A");
				}
				if (UI_VERBOSE){
					printf("%s.%d\t ui_UpdateInfoPane()  - series: %s\n", __FILE__, __LINE__, info_series);
				}
				
				if (launchdat->year != 0){
					sprintf(info_year, "%d", launchdat->year);
				} else {
					sprintf(info_year, "N/A");
				}
				if (UI_VERBOSE){
					printf("%s.%d\t ui_UpdateInfoPane()  - year: %s\n", __FILE__, __LINE__, info_year);
				}
				
				s1 = strlen(launchdat->developer);
				s2 = strlen(launchdat->publisher);
				
				// If we have publisher and developer, print both
				if ((s1 > 0) && (s2 > 0)){
					
					if (s1 + s2 <= 33){
						sprintf(info_company, " %s/%s", launchdat->developer, launchdat->publisher);
					} else {
						if (s1 >= 16){
							// If publisher is long, then prioritise it
							if (s2 > 12){
								sprintf(info_company, " %.20s/%.12s..", launchdat->developer, launchdat->publisher);
							} else {
								sprintf(info_company, " %.20s/%s", launchdat->developer, launchdat->publisher);
							}
						} else {
							// Otherwise just print both strings truncated to 16 places
							sprintf(info_company, " %.16s/%.16s", launchdat->developer, launchdat->publisher);
						}
					}
					
				// If we just have developer, print that
				} else if (strlen(launchdat->developer) > 0){
					sprintf(info_company, " %.34s", launchdat->developer);
					
				// If we just have publisher, print that
				} else if (strlen(launchdat->publisher) > 0){
					sprintf(info_company, " %.34s", launchdat->publisher);
					
				// If we have nothing...
				} else {
					sprintf(info_company, " N/A");
				}
				if (UI_VERBOSE){
					printf("%s.%d\t ui_UpdateInfoPane()  - company: %s\n", __FILE__, __LINE__, info_company);
				}
				
				// Start file
				
				// Number of images/screenshots
	
				sprintf(info_path, " %.38s", state->selected_game->path);
			}
		} else {
			// ======================
			// We can only use the basic directory information
			// ======================
			gfx_Bitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_empty_bmp);
			gfx_Bitmap(ui_checkbox_has_startbat_xpos, ui_checkbox_has_startbat_ypos, ui_checkbox_empty_bmp);
			gfx_Bitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);
			gfx_Bitmap(ui_checkbox_has_midi_xpos, ui_checkbox_has_midi_ypos, ui_checkbox_empty_bmp);
			gfx_Bitmap(ui_checkbox_has_midi_serial_xpos, ui_checkbox_has_midi_serial_ypos, ui_checkbox_empty_bmp);
				
			sprintf(info_name, " %.64s", state->selected_game->name);
			sprintf(info_year, "N/A");
			sprintf(info_company, " N/A");
			sprintf(info_genre, "N/A");
			sprintf(info_series, "N/A");
			sprintf(info_path, " %.38s", state->selected_game->path);		
		}	
	} else {
		// ======================
		// Error in logic, gameid is not set
		// ======================
		gfx_Bitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_checkbox_has_startbat_xpos, ui_checkbox_has_startbat_ypos, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_checkbox_has_midi_xpos, ui_checkbox_has_midi_ypos, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_checkbox_has_midi_serial_xpos, ui_checkbox_has_midi_serial_ypos, ui_checkbox_empty_bmp);
		sprintf(status_msg, "ERROR, unable to find gamedata object for ID %d", state->selected_gameid);
		gfx_Puts(ui_info_name_text_xpos, ui_info_name_text_ypos, ui_font, status_msg);
		//gamedata = gamedata_head;
		return UI_OK;
	}
	
	// ===========================
	// Now print out all data, regardless of source
	// ===========================
	gfx_Puts(ui_info_name_text_xpos, ui_info_name_text_ypos, ui_font, info_name);
	gfx_Puts(ui_info_year_text_xpos, ui_info_year_text_ypos, ui_font, info_year);
	gfx_Puts(ui_info_company_text_xpos, ui_info_company_text_ypos, ui_font, info_company);
	gfx_Puts(ui_info_genre_text_xpos, ui_info_genre_text_ypos, ui_font, info_genre);
	gfx_Puts(ui_info_series_text_xpos, ui_info_series_text_ypos, ui_font, info_series);
	gfx_Puts(ui_info_path_text_xpos, ui_info_path_text_ypos, ui_font, info_path);
	//gamedata = gamedata_head;
	return UI_OK;
}
