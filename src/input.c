/* input.c, Combined joystick and keyboar user input routines for the pc98Launcher.
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
#include <dos.h>

#include "input.h"

int input_get(){
	// Read joystick or keyboard input and return directions or buttons pressed
	
	int k;
	
	// Delay loop
	//delay(KB_DELAY);
	
	return input_none;
}

int input_test(){
	int exit;
	int user_input;
	
	printf("Keyboard Test Routine\n\n");
	printf("=========================\n");
	printf("\n");
	exit = 0;
	printf("Press UP cursor\n");
	while(exit == 0){
		user_input = input_get();
		if (user_input != input_none){
			if (user_input != input_up){
				printf("[%x] NOT A MATCH!\n", user_input);
			} else {
				printf("[%x] OK\n", user_input);
			}
			exit = 1;
		}
	}
	exit = 0;
	printf("Press DOWN cursor\n");
	while(exit == 0){
		user_input = input_get();
		if (user_input != input_none){
			if (user_input != input_down){
				printf("[%x] NOT A MATCH!\n", user_input);
			} else {
				printf("[%x] OK\n", user_input);
			}
			exit = 1;
		}
	}
	exit = 0;
	printf("Press LEFT cursor\n");
	while(exit == 0){
		user_input = input_get();
		if (user_input != input_none){
			if (user_input != input_left){
				printf("[%x] NOT A MATCH!\n", user_input);
			} else {
				printf("[%x] OK\n", user_input);
			}
			exit = 1;
		}
	}
	exit = 0;
	printf("Press RIGHT cursor\n");
	while(exit == 0){
		user_input = input_get();
		if (user_input != input_none){
			if (user_input != input_right){
				printf("[%x] NOT A MATCH!\n", user_input);
			} else {
				printf("[%x] OK\n", user_input);
			}
			exit = 1;
		}
	}
	exit = 0;
	printf("Press ENTER\n");
	while(exit == 0){
		user_input = input_get();
		if (user_input != input_none){
			if (user_input != input_select){
				printf("[%x] NOT A MATCH!\n", user_input);
			} else {
				printf("[%x] OK\n", user_input);
			}
			exit = 1;
		}
	}
	exit = 0;
	printf("Press q, lowercase\n");
	while(exit == 0){
		user_input = input_get();
		if (user_input != input_none){
			if (user_input != input_quit){
				printf("[%x] NOT A MATCH!\n", user_input);
			} else {
				printf("[%x] OK\n", user_input);
			}
			exit = 1;
		}
	}
	exit = 0;
	printf("Press Q, uppercase\n");
	while(exit == 0){
		user_input = input_get();
		if (user_input != input_none){
			if (user_input != input_quit){
				printf("[%x] NOT A MATCH!\n", user_input);
			} else {
				printf("[%x] OK\n", user_input);
			}
			exit = 1;
		}
	}
	exit = 0;
	printf("Press f, lowercase\n");
	while(exit == 0){
		user_input = input_get();
		if (user_input != input_none){
			if (user_input != input_filter){
				printf("[%x] NOT A MATCH!\n", user_input);
			} else {
				printf("[%x] OK\n", user_input);
			}
			exit = 1;
		}
	}
	exit = 0;
	printf("Press F, uppercase (should be: %x)\n", input_filter);
	while(exit == 0){
		user_input = input_get();
		if (user_input != input_none){
			if (user_input != input_filter){
				printf("[%x] NOT A MATCH!\n", user_input);
			} else {
				printf("[%x] OK\n", user_input);
			}
			exit = 1;
		}
	}
	exit = 0;
	printf("Press h, lowercase (should be: %x)\n", input_help_lower);
	while(exit == 0){
		user_input = input_get();
		if (user_input != input_none){
			if (user_input != input_help){
				printf("[%x] NOT A MATCH!\n", user_input);
			} else {
				printf("[%x] OK\n", user_input);
			}
			exit = 1;
		}
	}
	exit = 0;
	printf("Press H, uppercase (should be: %x)\n", input_help);
	while(exit == 0){
		user_input = input_get();
		if (user_input != input_none){
			if (user_input != input_help){
				printf("[%x] NOT A MATCH!\n", user_input);
			} else {
				printf("[%x] OK\n", user_input);
			}
			exit = 1;
		}
	}
	return 0;	
}
