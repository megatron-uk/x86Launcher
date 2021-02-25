/* filter.h, Apply sorting and filtering to gamedata lists.
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

#ifndef __HAS_DATA
#include "data.h"
#define __HAS_DATA
#endif
#ifndef __HAS_MAIN
#include "main.h"
#define __HAS_MAIN
#endif

// Defaults
#define FILTER_VERBOSE	0		// Enable/disable logging for these functions
#define	FILTER_OK		0		// Success returncode
#define FILTER_ERR		-1		// Failure returncode

// Custom filter strings
#define FILTER_STRING_AUDIO_BEEPER	"Audio: PC Speaker"
#define FILTER_STRING_AUDIO_TANDY	"Audio: Tandy"
#define FILTER_STRING_AUDIO_ADLIB	"Audio: Adlib"
#define FILTER_STRING_AUDIO_SB		"Audio: Soundblaster"
#define FILTER_STRING_AUDIO_MT32		"Audio: Roland MT32"
#define FILTER_STRING_AUDIO_GM		"Audio: General MIDI"
#define FILTER_STRING_AUDIO_COVOX	"Audio: Covox"
#define FILTER_STRING_AUDIO_DISNEY	"Audio: Disney"
#define FILTER_STRING_AUDIO_GUS		"Audio: Ultrasound"

#define FILTER_STRING_VIDEO_TEXT		"Video: Text mode"
#define FILTER_STRING_VIDEO_HERCULES	"Video: Hercules"
#define FILTER_STRING_VIDEO_TANDY	"Video: Tandy"
#define FILTER_STRING_VIDEO_CGA		"Video: CGA"
#define FILTER_STRING_VIDEO_EGA		"Video: EGA"
#define FILTER_STRING_VIDEO_VGA		"Video: VGA"
#define FILTER_STRING_VIDEO_SVGA		"Video: SVGA"

#define FILTER_STRING_CPU_8086		"CPU: 8086"
#define FILTER_STRING_CPU_80286		"CPU: 80286"
#define FILTER_STRING_CPU_80386		"CPU: 80386"
#define FILTER_STRING_CPU_80486		"CPU: 80486"
#define FILTER_STRING_CPU_80586		"CPU: 586/Pentium"
#define FILTER_STRING_RAM_XMS		"RAM: Extended XMS"
#define FILTER_STRING_RAM_EMS		"RAM: Expanded EMS"
#define FILTER_STRING_MISC_DPMI		"Misc: DPMI"

// Function prototypes
int filter_GetGenres(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_GetSeries(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_GetCompany(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_GetTechSpecs(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_None(state_t *state, gamedata_t *gamedata);
int filter_Genre(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_Series(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_Company(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
int filter_TechSpecs(state_t *state, gamedata_t *gamedata, launchdat_t *filterdat);
