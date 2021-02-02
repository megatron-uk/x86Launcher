/* input.h, User input routines for the pc98Launcher.
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

#define KB_DELAY					5		// delay in ms before reading keyboard state

// Input codes as returned to main()
#define input_none				0x0000
#define input_select				0x0D // Enter
#define input_cancel				0x1B // Escape
#define input_switch				0x09 // Tab
#define input_up					0x48
#define input_down				0x50
#define input_left				0x4B
#define input_right				0x4D
#define input_scroll_up			0x49 // Page up
#define input_scroll_down		0x51 // Page down
#define input_quit				0x71 // Q == Quit
#define input_filter				0x66 // F == Filter
#define input_help				0x68 // H == Help

// Function prototypes
int	input_get();
int input_test();
