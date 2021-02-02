/* gfx.c, Graphical functions for drawing the main screen for the pc98Launcher.
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
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <dos.h>

#include "gfx.h"
#include "vesa.h"
#include "utils.h"

#ifndef __HAS_PAL
#include "palette.h"
#define __HAS_PAL
#endif

#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

unsigned char __huge	*vram;					// Pointer to a location in the local graphics buffer
unsigned char __huge	vram_buffer[VRAM_END]; 	// Our local memory graphics buffer, GFX_ROWS * GFX_COLS * GFX_PIXEL_SIZE
long int window_x_max; 						// How many pixels wide a vesa memory window is
long int window_y_max; 						// How many pixels deep a vesa memory window is
long int vga_segment; 						// Base address of the current vesa memory window
long int windows_in_use;						// Number of video memory windows needed to map our GFX_ROWS * GFX_COLS screen
long int window_bytes;						// NUmber of bytes in a single vesa memory window (nominally 65536)
unsigned char *VGA=(unsigned char *)0xA0000000L; // Position of the VGA memory region
unsigned char vga_dac_type = VGA_PALETTE_6BPP;

int gfx_Init(){
	// Initialise graphics to a set of configured defaults
	
	
	int status;
	double window_bytes_t;
	vbeinfo_t *vbeinfo = NULL;
	vesamodeinfo_t *vesamodeinfo = NULL;
	
	vbeinfo = (vbeinfo_t *) malloc(sizeof(vbeinfo_t));
	vesamodeinfo = (vesamodeinfo_t *) malloc(sizeof(vesamodeinfo_t));
	
	if (GFX_VERBOSE){
		printf("%s.%d\t gfx_Init() Initalising gfx mode\n", __FILE__, __LINE__);	
	}
	
	// Look for a VESA BIOS structure
	status = vesa_GetVBEInfo(vbeinfo);
	if (status < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Init() Error, Unable to complete gfx initialisation [VBE BIOS missing]\n", __FILE__, __LINE__);	
		}
		free(vbeinfo);
		free(vesamodeinfo);
		return -1;	
	} else {	
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Init() Found a VESA BIOS\n", __FILE__, __LINE__);	
		}
	}
		
	// Find mode GFX_VESA_DESIRED
	status = vesa_HasMode(GFX_VESA_DESIRED, vbeinfo);
	if (status < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Init() Error, Unable to complete gfx initialisation [VESA mode missing]\n", __FILE__, __LINE__);	
		}
		free(vbeinfo);
		free(vesamodeinfo);
		return -1;	
	} else {	
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Init() Found VESA mode %xh\n", __FILE__, __LINE__, GFX_VESA_DESIRED);	
		}
	}
	
	// Load the mode information
	status = vesa_GetModeInfo(GFX_VESA_DESIRED, vesamodeinfo);
	if (status < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Init() Error, Unable to complete gfx initialisation [VESA mode lookup]\n", __FILE__, __LINE__);	
		}
		free(vbeinfo);
		free(vesamodeinfo);
		return -1;	
	} else {	
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Init() Retrieved VESA mode details %xh\n", __FILE__, __LINE__, GFX_VESA_DESIRED);	
		}
	}
	
	// Set mode GFX_VESA_DESIRED
	status = vesa_SetMode(GFX_VESA_DESIRED);
	if (status < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Init() Error, Unable to complete gfx initialisation [VESA set mode failed]\n", __FILE__, __LINE__);	
		}
		free(vbeinfo);
		free(vesamodeinfo);
		return -1;	
	} else {	
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Init() Set VESA mode %xh\n", __FILE__, __LINE__, GFX_VESA_DESIRED);	
		}
	}
	
	// Set VGA DAC type
	if (vbeinfo->capabilities & 0x01){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Init() Trying to switch VGA DAC to %dbpp\n", __FILE__, __LINE__, VGA_PALETTE_8BPP);	
		}
		
		// DAC is programmable - we need to try to switch it to 8bit mode
		status = vesa_SetDAC(VGA_PALETTE_8BPP);
		if (status < 0){
			if (GFX_VERBOSE){
				printf("%s.%d\t gfx_Init() Unable to switch VGA DAC, defaulting to %dbpp\n", __FILE__, __LINE__, VGA_PALETTE_DEFAULT);	
			}
			vga_dac_type = VGA_PALETTE_6BPP;
		} else {
			status = vesa_GetDAC(VGA_PALETTE_8BPP);
			if (status < 0){
				// Yes, switched to 8bit mode
				if (GFX_VERBOSE){
					printf("%s.%d\t gfx_Init() VGA DAC now in %dbpp\n", __FILE__, __LINE__, VGA_PALETTE_8BPP);	
				}
				vga_dac_type = VGA_PALETTE_8BPP;
			} else {
				// Switching failed, assume 6bpp
				if (GFX_VERBOSE){
					printf("%s.%d\t gfx_Init() Warning, VGA DAC not in desired mode, defaulting to %dbpp\n", __FILE__, __LINE__, VGA_PALETTE_6BPP);	
				}
				vga_dac_type = VGA_PALETTE_6BPP;
			}
		}
	} else {
		// DAC is NON-programmable. Palette entries need to be truncated to 6bpp.
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Init() VESA BIOS indicates VGA DAC is fixed at 6bpp\n", __FILE__, __LINE__);
		}
		vga_dac_type = VGA_PALETTE_6BPP;
	}
	
	// We've set a new video mode, so recalculate
	// the window_x_max and window_y_max values
	// as well as derive the VGA segment address for
	// the window and the number of bytes in a video window
	vga_segment = vesamodeinfo->WinASegment;
	window_bytes = (long int) vesamodeinfo->WinSize * 1024;
	window_x_max = (window_bytes - 1) % GFX_COLS;
	window_y_max = (window_bytes - 1) / GFX_COLS;
	window_bytes_t = (double) ((long int) GFX_COLS * (long int) GFX_ROWS) / window_bytes;
	windows_in_use = ceil(window_bytes_t);
	
	if (GFX_VERBOSE){
		printf("%s.%d\t gfx_Init() VESA memory window segment address: %xh\n", __FILE__, __LINE__, vga_segment);
		printf("%s.%d\t gfx_Init() VESA memory window size: %ld bytes (at %d bytes/pixel)\n", __FILE__, __LINE__, window_bytes, GFX_PIXEL_SIZE);
		printf("%s.%d\t gfx_Init() VESA memory window is: %ld\n", __FILE__, __LINE__, window_x_max);
		printf("%s.%d\t gfx_Init() VESA memory window rows: %ld\n", __FILE__, __LINE__, window_y_max);
		printf("%s.%d\t gfx_Init() VESA memory windows needed: %ld\n", __FILE__, __LINE__, windows_in_use);
	}
	
	gfx_Clear();
	gfx_Flip();
	
	free(vbeinfo);
	free(vesamodeinfo);
	return 0;
}
        
int gfx_CheckMode(){
	// Check if a given SVGA/VESA mode is available
	
	return 0;
}

int gfx_Close(){
	//  Return to text mode
	
	if (GFX_VERBOSE){
		printf("%s.%d\t gfx_Close() Exiting gfx mode\n", __FILE__, __LINE__);	
	}
	
	//  Clear anything we did to the screen
	gfx_Clear();
	
	// Text mode on
	gfx_TextOn();
	
	return 0;
}

void gfx_Clear(){
	
	long int i;
	unsigned short int c;
	
	if (GFX_VERBOSE){
		printf("%s.%d\t gfx_Clear() Setting %ld pixels\n", __FILE__, __LINE__, sizeof(vram_buffer));	
	}
	
	// Set local vram_buffer to empty
	memset(vram_buffer, 0, sizeof(vram_buffer));
	
}

void gfx_TextOn(){
	// Text mode on
	printf("%s.%d\t gfx_TextOn() Not implemented!\n", __FILE__, __LINE__);
}

void gfx_TextOff(){
	// Text mode off
	printf("%s.%d\t gfx_TextOff() Not implemented!\n", __FILE__, __LINE__);
}

void gfx_Flip(){
	// Copy a buffer of GFX_ROWS * GFX_COLS bytes to
	// the active VRAM framebuffer for display.
	
	unsigned short int window;
	long int left;
	
	// Set the vram pointer to the start of the buffer
	vram = vram_buffer;
	
	left = (long int) VRAM_END;
	
	// for each window in the number of windows for this video mode
	for(window = 0; window < windows_in_use; window++ ){
		
		// set new window to be active
		vesa_SetWindow(window);
		
		// copy the block of pixels for this memory window
		if (left > window_bytes){
			if (GFX_VERBOSE){
				printf("%s.%d\t gfx_Flip() Copying %ld bytes to window %d\n", __FILE__, __LINE__, window_bytes, window);
			}
			_fmemcpy(VGA, vram, window_bytes - 1);
			
			left -= window_bytes;
		} else {
			if (GFX_VERBOSE){
				printf("%s.%d\t gfx_Flip() Copying remaining %ld bytes to window %d\n", __FILE__, __LINE__, left, window);
			}
			_fmemcpy(VGA, vram, left);
		}
		
		// Increment the pointer to the vram buffer by the size of one video window
		vram += (long int) (window_bytes);
	};
	
	// Reset vram buffer pointer position
	vram = vram_buffer;
}

long int gfx_GetXYaddr(unsigned short int x, unsigned short int y){
	// Turn a screen x/y coordinate into an offset into a vram buffer
	
	long int addr;
		
	addr = (long int) VRAM_START;
	addr += (long int) GFX_ROW_SIZE * (long int) y;
	addr += (x * GFX_PIXEL_SIZE);
	
	//if (GFX_VERBOSE){
	//	printf("%s.%d\t Mapping X:%d Y:%d\n", __FILE__, __LINE__, x, y);
	//}
	
	if ((VRAM_START + addr) > VRAM_END){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_GetXYaddr() XY coords beyond VRAM buffer end [ %ld > %ld]\n", __FILE__, __LINE__, addr, VRAM_END);
		}
		return -1;
	}
	
	if (addr < VRAM_START){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_GetXYaddr() XY coords before VRAM buffer start [ %ld < %ld]\n", __FILE__, __LINE__, addr, VRAM_START);
		}
		return -1;
	}
	
	return addr;
}

int gfx_Bitmap(int x, int y, bmpdata_t *bmpdata){
	// Load bitmap data into vram_buffer at coords x,y
	// X or Y can be negative which starts the first X or Y
	// rows or columns of the bitmap offscreen - i.e. they are clipped
	//
	// Bitmaps wider or taller than the screen are UNSUPPORTED
	
	int row, col;		//  x and y position counters
	long int start_addr;	// The first pixel
	int width_bytes;		// Number of bytes in one row of the image
	int skip_cols;		// Skip first or last pixels of a row if the image is partially offscreen
	int skip_bytes;
	int skip_rows;		// Skip this number of rows if the image is patially offscreen
	int total_rows;		// Total number of rows to read in clip mode
	unsigned char __huge *ptr;	// Pointer to current location in bmp pixel buffer
	
	if (GFX_VERBOSE){
		printf("%s.%d\t gfx_Bitmap() Copy %dx%d bitmap to X:%d Y:%d\n", __FILE__, __LINE__, bmpdata->width, bmpdata->height, x, y);
	}
	
	// Negative X values start offscreen at the left
	if (x < 0){
		skip_cols = x;
	} else {
		if ((x + bmpdata->width) > GFX_COLS){
			// Positive values get clipped at the right
			skip_cols = x + bmpdata->width - GFX_COLS;
		} else {
			// Full width can fit on screen
			skip_cols = 0;
		}
	}
	
	// Negative Y values start off the top of the screen
	if (y < 0){
		skip_rows = y;
	} else {
		if ((y + bmpdata->height) > GFX_ROWS){
			// Positive values get clipped at the bottom of the screen
			skip_rows = y + bmpdata->height - GFX_ROWS;
		} else {
			// Full height can fit on screen
			skip_rows = 0;
		}
	}
	
	if ((skip_cols == 0) && (skip_rows == 0)){

		// Case 1 - bitmap fits entirely onscreen
		width_bytes = bmpdata->width * bmpdata->bytespp;
		
		// Get starting pixel address
		start_addr = gfx_GetXYaddr(x, y);
		if (start_addr < 0){
			if (GFX_VERBOSE){
				printf("%s.%d\t gfx_Bitmap() Unable to set VRAM buffer start address [%d]\n", __FILE__, __LINE__, start_addr);
			}
			return -1;
		}
		// Set starting pixel address
		vram = vram_buffer;
		vram += start_addr;
		
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Bitmap() Bitmap starting location: %p\n", __FILE__, __LINE__, vram);
		}
		
		// memcpy entire rows at a time
		ptr = (unsigned char*) bmpdata->pixels; 
		for(row = 0; row < bmpdata->height; row++){
			memcpy(vram, ptr, width_bytes);
			
			// Go to next row in vram_buffer
			vram += (long int) GFX_COLS;
			
			// Increment point
			ptr += bmpdata->width;
		}
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Bitmap() Bitmap copied without cropping\n", __FILE__, __LINE__);
		}
		return 0;
		
	} else {
		// Case 2 - image is either vertically or horizontally partially offscreen		
		if (skip_cols < 0){
			x = x + abs(skip_cols);
		}
		if (skip_rows < 0){
			y = y + abs(skip_rows);
		}
		
		// Get starting pixel address - at the new coordinates
		start_addr = gfx_GetXYaddr(x, y);
		if (start_addr < 0){
			if (GFX_VERBOSE){
				printf("%s.%d\t gfx_Bitmap() Unable to set VRAM buffer start address\n", __FILE__, __LINE__);
			}
			return -1;
		}
		
		// Set starting pixel address
		vram = vram_buffer + start_addr;
		
		// Set starting point in pixel buffer
		ptr = (unsigned char*) bmpdata->pixels;
		
		// Default to writing a full row of pixels, unless....
		width_bytes = (bmpdata->width * bmpdata->bytespp) ;
		
		// Default to writing all rows, unless....
		total_rows = bmpdata->height;
		
		// If we are starting offscreen at the y axis, jump that many rows into the data
		if (skip_rows < 0){
			ptr += abs(skip_rows) * bmpdata->width;
			total_rows = bmpdata->height - abs(skip_rows);
		}
		if (skip_rows > 0){
			total_rows = bmpdata->height - abs(skip_rows);
		}
	
		if (skip_cols != 0){
			width_bytes = (bmpdata->width * bmpdata->bytespp) - (abs(skip_cols) * bmpdata->bytespp);
		}
		
		// memcpy entire rows at a time, subject to clipping sizes
		for(row = 0; row < total_rows; row++){
			if (skip_cols < 0){
				memcpy(vram, ptr + abs(skip_cols), width_bytes);
			} else {
				memcpy(vram, ptr, width_bytes);
			}
			// Go to next row in vram buffer
			vram += GFX_COLS;
			// Increment pointer to next row in pixel buffer
			ptr += bmpdata->width;
		}
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Bitmap() Bitmap copied with cropping\n", __FILE__, __LINE__);
		}
		return 0;
	}
	return -1;
}

int gfx_BitmapAsync(int x, int y, bmpdata_t *bmpdata, FILE *bmpfile, bmpstate_t *bmpstate, int remap_palette, int reserved_palette){
	// Load from file, decode and display, line by line
	// Every time the function is called, another line is read, decoded and displayed
	//
	// This is slower than reading the entire bitmap into memory and then copying
	// using gfx_Bitmap, but the advantage here is that we can call this between
	// vsync or scanning for user input, as well as only allocating one horizontal row
	// of pixels at a time - that's only 640Bytes for 640x400 @ 8bpp.
	
	int					i;			// Loop counter
	unsigned char __huge	*ptr;		// Access pixels in pixel buffer
	int					status;		// General statuscat
	long int 			start_addr;	// The first pixel
	int					new_y;
	
	if (bmpdata->bpp != 8){
		return GFX_ERR_UNSUPPORTED_BPP;
	}

	// BMP header has not been read yet
	if (bmpdata->offset <= 0){
		return GFX_ERR_MISSING_BMPHEADER;
	}
	
	if (bmpstate->rows_remaining == bmpdata->height){
		// This is a new image, or we haven't read a row yet
		
		if (bmpstate->pixels != NULL){
			free(bmpstate->pixels);
		}
		bmpstate->pixels = (uint8_t*) calloc(bmpdata->width, bmpdata->bytespp);
		bmpstate->width_bytes = bmpdata->width * bmpdata->bytespp;
		
		// Seek to start of data section in file
		status = fseek(bmpfile, bmpdata->offset, SEEK_SET);
		if (status != 0){
			free(bmpstate->pixels);
			bmpstate->width_bytes = 0;
			bmpstate->rows_remaining = 0;
			return BMP_ERR_READ;
		}
	}	
	
	// Read a row of pixels
	status = fread(bmpstate->pixels, 1, bmpdata->row_unpadded, bmpfile);
	if (status < 1){
		free(bmpstate->pixels);
		bmpstate->width_bytes = 0;
		bmpstate->rows_remaining = 0;
		return BMP_ERR_READ;	
	}
	if (status != bmpdata->row_unpadded){
		// Seek the number of bytes left in this row
		status = fseek(bmpfile, (bmpdata->row_padded - status), SEEK_CUR);
		if (status != 0){
			free(bmpstate->pixels);
			bmpstate->width_bytes = 0;
			bmpstate->rows_remaining = 0;
			return BMP_ERR_READ;
		}
	}
	
	// Get coordinates
	new_y = y + bmpstate->rows_remaining - 1;
	start_addr = gfx_GetXYaddr(x, new_y);
	
	// Set starting pixel address
	vram = vram_buffer + start_addr;
	
	if (remap_palette){
		pal_BMPState2Palette(bmpdata, bmpstate, reserved_palette);
	}
	
	// Copy this single line of pixels to the video buffer
	memcpy(vram, bmpstate->pixels, bmpstate->width_bytes);
	
	bmpstate->rows_remaining--;
	
	if (bmpstate->rows_remaining < 1){
		bmpstate->rows_remaining = 0;
	}
	
	return 0;
	
}

int gfx_BitmapAsyncFull(int x, int y, bmpdata_t *bmpdata, FILE *bmpfile, bmpstate_t *bmpstate, int remap_palette, int reserved_palette){
	// Display a bitmap using the async call, in its entirety, using no-more than 1 line
	// worth of allocated memory
	
	int status;
	
	if (GFX_VERBOSE){
		printf("%s.%d\t gfx_BitmapAsyncFull() Starting async bitmap loader\n", __FILE__, __LINE__);
	}
	
	// Read image header and palette entries
	status = bmp_ReadImage(bmpfile, bmpdata, 1, 1, 0);	
	if (status != 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_BitmapAsyncFull() Unable to load bitmap for async display\n", __FILE__, __LINE__);
		}
	} else {
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_BitmapAsyncFull() Bitmap header and palette loaded\n", __FILE__, __LINE__);
		}
		
		// Read palette data
		// ...
		
		// Set rows remaining
		bmpstate->rows_remaining = bmpdata->height;
		
		// Remap the current line of pixels to the new palette
		if (reserved_palette == 0){
			if (GFX_VERBOSE){
				printf("%s.%d\t gfx_BitmapAsyncFull() This is not using the reserved palette so we need to set all the...\n", __FILE__, __LINE__);
				printf("%s.%d\t gfx_BitmapAsyncFull() ...necessary palette entries for this bitmap.\n", __FILE__, __LINE__);
			}
			pal_BMPState2Palette(bmpdata, bmpstate, reserved_palette);
		} else {
			if (GFX_VERBOSE){
				printf("%s.%d\t gfx_BitmapAsyncFull() This is using the reserved palette so we need to remap each...\n", __FILE__, __LINE__);
				printf("%s.%d\t gfx_BitmapAsyncFull() ...line of pixels using the reserved palette colours.\n", __FILE__, __LINE__);
			}
		}
	
		// Loop until all rows processed
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_BitmapAsyncFull() Starting async display at X:%d Y:%d...\n", __FILE__, __LINE__, x, y);
			printf("%s.%d\t gfx_BitmapAsyncFull() Need to make %d calls...\n", __FILE__, __LINE__, bmpstate->rows_remaining);
		}
		while (bmpstate->rows_remaining > 0){
			status = gfx_BitmapAsync(x, y, bmpdata, bmpfile, bmpstate, remap_palette, reserved_palette);
			if (status != 0){
				if (GFX_VERBOSE){
					printf("%s.%d\t gfx_BitmapAsyncFull() Error loading bitmap asynchronously\n", __FILE__, __LINE__);
				}
				return status;	
			}
		}
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_BitmapAsyncFull() Completed\n", __FILE__, __LINE__, x, y);
		}
	}
	
	return status;
}

int gfx_Box(int x1, int y1, int x2, int y2, unsigned char palette){
	// Draw a box outline with a given palette entry colour
	int row, col;		//  x and y position counters
	long int start_addr; 	// The first pixel, at x1,y1
	int temp;		// Holds either x or y, if we need to flip them
	int step;
	
	if (GFX_VERBOSE){
	   printf("%s.%d\t gfx_Box() Drawing %d,%d-%d,%d with palette %d\n", __FILE__, __LINE__, x1, y1, x2, y2, palette);
	}
	
	// Flip y, if it is supplied reversed
	if (y1>y2){
		temp=y1;
		y1=y2;
		y2=temp;
	}
	// Flip x, if it is supplied reversed
	if (x1>x2){
		temp=x1;
		x1=x2;
		x2=temp;
	}
	// Clip the x range to the edge of the screen
	if (x2>GFX_COLS){
		x2 = GFX_COLS - 1;
	}
	// Clip the y range to the bottom of the screen
	if (y2>GFX_ROWS){
		y2 = GFX_ROWS - 1;
	}
	// Get starting pixel address
	start_addr = gfx_GetXYaddr(x1, y1);
	if (start_addr < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Box() Unable to set VRAM buffer start address\n", __FILE__, __LINE__);
		}
		return -1;
	}
	// Set starting pixel address
	vram = vram_buffer + start_addr;
	
	// Step to next row in vram
	step = (GFX_COLS - x2) + x1;
	
	// Draw top
	for(col = x1; col <= x2; col++){
		*vram = palette;
		// Move to next pixel in line
		vram++;
	}
	// Jump to next line down and start of left side
	vram += (GFX_COLS - x2) + (x1 - 1);
	
	// Draw sides
	for(row = y1; row < (y2-1); row++){     
		*vram = palette;
		vram += (x2 - x1);
		*vram = palette;
		vram += step;
	}
	
	// Draw bottom
	for(col = x1; col <= x2; col++){
		*vram = palette;
		// Move to next pixel in line
		vram++;
	}
	
	return 0;
}

int gfx_BoxFill(int x1, int y1, int x2, int y2, unsigned char palette){
	// Draw a box, fill it with a given palette entry
	int row, col;		//  x and y position counters
	long int start_addr;	// The first pixel, at x1,y1
	int temp;		// Holds either x or y, if we need to flip them
	int step;
	
	if (GFX_VERBOSE){
	   printf("%s.%d\t gfx_BoxFill() Drawing %d,%d-%d,%d with palette %d\n", __FILE__, __LINE__, x1, y1, x2, y2, palette);
	}
	
	// Flip y, if it is supplied reversed
	if (y1>y2){
		temp=y1;
		y1=y2;
		y2=temp;
	}
	// Flip x, if it is supplied reversed
	if (x1>x2){
		temp=x1;
		x1=x2;
		x2=temp;
	}
	// Clip the x range to the edge of the screen
	if (x2>GFX_COLS){
		x2 = GFX_COLS - 1;
	}
	// Clip the y range to the bottom of the screen
	if (y2>GFX_ROWS){
		y2 = GFX_ROWS - 1;
	}
	// Get starting pixel address
	start_addr = gfx_GetXYaddr(x1, y1);
	if (start_addr < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_BoxFill() Unable to set VRAM buffer start address\n", __FILE__, __LINE__);
		}
		return -1;
	}
	// Set starting pixel address
	vram = vram_buffer + start_addr;
	
	// Step to next row in vram
	step = (GFX_COLS - x2) + (x1 - 1);
	
	// Starting from the first row (y1)
	for(row = y1; row <= y2; row++){
		// Starting from the first column (x1)
		for(col = x1; col <= x2; col++){
			*vram = palette;
			vram++;
		}
		vram += step;
	}
	return 0;
}

int gfx_BoxFillTranslucent(int x1, int y1, int x2, int y2, unsigned char palette){
	// Draw a box, fill it with a given palette entry - every 2nd pixel, so that
	// it looks semi-transparent.
	
	int row, col;	// x and y position counters
	long int start_addr;	// The first pixel, at x1,y1
	int temp;		// Holds either x or y, if we need to flip them
	int step;
	int flip;		// toggles display of every other pixel on/off
	
	if (GFX_VERBOSE){
	   printf("%s.%d\t gfx_BoxFillTranslucent() Drawing %d,%d-%d,%d with palette %d\n", __FILE__, __LINE__, x1, y1, x2, y2, palette);
	}
	
	// Flip y, if it is supplied reversed
	if (y1>y2){
		temp=y1;
		y1=y2;
		y2=temp;
	}
	// Flip x, if it is supplied reversed
	if (x1>x2){
		temp=x1;
		x1=x2;
		x2=temp;
	}
	// Clip the x range to the edge of the screen
	if (x2>GFX_COLS){
		x2 = GFX_COLS - 1;
	}
	// Clip the y range to the bottom of the screen
	if (y2>GFX_ROWS){
		y2 = GFX_ROWS - 1;
	}
	// Get starting pixel address
	start_addr = gfx_GetXYaddr(x1, y1);
	if (start_addr < 0){
		if (GFX_VERBOSE){
		   printf("%s.%d\t gfx_BoxFillTranslucent() Unable to set VRAM buffer start address\n", __FILE__, __LINE__);
		}
		return -1;
	}
	// Set starting pixel address
	vram = vram_buffer + start_addr;
	
	// Step to next row in vram
	step = (GFX_COLS - x2) + (x1 - 1);
	
	flip = 0;
	
	// Starting from the first row (y1)
	for(row = y1; row <= y2; row++){
		// Starting from the first column (x1)
		for(col = x1; col <= x2; col++){
			// Only every other pixel
			if (flip){
				*vram = palette;
			}
			vram++;
			flip = !flip;
		}
		vram += step;
	}
	return 0;
}

int gfx_Puts(int x, int y, fontdata_t *fontdata, char *c){
	// Put a string of text on the screen, at a set of coordinates
	// using a specific font.
	//
	// Note: We only support 8px and 16px wide fonts.
	
	long int	start_offset;
	long int	row_offset;
	unsigned char font_symbol;
	unsigned char font_row;
	unsigned char i, w;
	unsigned char pos;
	unsigned char *src, *dst;
	
	if (GFX_VERBOSE){
		printf("%s.%d\t gfx_Puts() Displaying string [%s] at X:%d Y:%d\n", __FILE__, __LINE__, c, x, y);
	}
	
	// Empty string
	if (strlen(c) < 1){
		return GFX_TEXT_OK;
	}
	
	// Calculate starting address
	start_offset = gfx_GetXYaddr(x, y);
	if (start_offset < 0){
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Puts() Unable to set VRAM buffer start address\n", __FILE__, __LINE__);
		}
		return -1;
	}
	
	// Reposition write position
	vram = vram_buffer + start_offset;
	
	if (GFX_VERBOSE){
		printf("%s.%d\t gfx_Puts() Copying bitmap to vram offset %d\n", __FILE__, __LINE__, vram);
	}
		
	if ((fontdata->width == 8) || (fontdata->width == 16)){
		// For every symbol in the string,
		// 1. Look up the appropriate symbol number to ascii character
		// 2. Check if the symbol is in our font table
		// 3. Do a bitmap copy of the symbol into the vram buffer
		// 4. Increment vram buffer pointer
		for (pos = 0; pos < strlen(c); pos+=1){
			
			i = (unsigned char) c[pos];
			if ((i >= fontdata->ascii_start) && (i <= (fontdata->ascii_start + fontdata->n_symbols))){
				font_symbol = i - fontdata->ascii_start;
			} else {
				font_symbol = fontdata->unknown_symbol;
			}
			
			// Output this symbol
			for(font_row = 0; font_row <= fontdata->height; font_row++){
				row_offset = font_row * GFX_COLS;
				src =  (unsigned char*) fontdata->symbol[font_symbol][font_row];
				dst = vram + row_offset;
				memcpy(dst, src, fontdata->width);
			}
		
			// Reposition write position for next symbol
			vram = vram + fontdata->width;
		}
		return GFX_TEXT_OK;
		
	} else {
		// Unsupported font width
		if (GFX_VERBOSE){
			printf("%s.%d\t gfx_Puts() Error, font is not a supported width (8 or 16 pixels)\n", __FILE__, __LINE__);	
		}
		return GFX_TEXT_INVALID;
	}
	
}
