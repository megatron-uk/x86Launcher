/* vesa.c, Low level VESA set/get functions for querying or setting screen modes
 for the x86Launcher.
 Copyright (C) 2021  John Snowdon
 
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
#include <i86.h>

#include "vesa.h"

//void vesa_getmodeinfo(unsigned short mode, VBE_ModeInfo *modeinfo){
	// Originally defined in:
	// http://www.geocities.com/siliconvalley/horizon/6933/vesa.txt
	// ... but is for 32bit protected mode. We need a 16bit real-mode version.
	
//	__dpmi_regs regs;
	
//	assert(sizeof(*modeinfo) < _go32_info_block.size_of_transfer_buffer);
//	regs.x.ax = 0x4F01;
//	regs.x.cx = mode;
//	regs.x.di = __tb & 0x0F;
//	regs.x.es = (__tb >> 4) & 0xFFFF;
//	__dpmi_int(0x10, &regs);
//	dosmemget(__tb, sizeof(*modeinfo), modeinfo);
//	return;
//}

int vesa_getmodeinfo(unsigned short mode, vesamodeinfo_t *modeinfo){
	// Retrieve info on a particular VESA mode
	
	union REGS r;
	struct SREGS s;
	
	if (VESA_VERBOSE){
		printf("%s.%d\t Quering mode %xh\n", __FILE__, __LINE__, mode);
	}
	
	// Set VESA BIOS call parameters and store pointer
	// to vbeinfo datastructure which will hold info
	// after this call.
	r.x.ax = VESA_MODE_INFO;
	r.x.cx = mode;
	r.x.di = FP_OFF(modeinfo);
	s.es = FP_SEG(modeinfo);

	// Call interrupt for VESA BIOS
	int86x(VESA_INTERRUPT, &r, &r, &s);
	
	if (r.x.ax != VESA_BIOS_SUCCESS){
		// VESA BIOS call was not successful
		if (VESA_VERBOSE){
			printf("%s.%d\t Error, Unable to query VESA mode [return code 0x%04x]\n", __FILE__, __LINE__, r.x.ax);
			vesa_printvbemodeinfo(modeinfo);
		}
		return -1;	
	}
	
	// Process VESA mode info structure data
	if (VESA_VERBOSE){
		printf("%s.%d\t VESA mode queried successfully!\n", __FILE__, __LINE__);
		vesa_printvbemodeinfo(modeinfo);
	}
	
	return 0;
}

int vesa_getvbeinfo(vbeinfo_t *vbeinfo){
	// Retrieve basic VESA BIOS information
	
	union REGS r;
	struct SREGS s;
	
	// Set VESA BIOS call parameters and store pointer
	// to vbeinfo datastructure which will hold info
	// after this call.
	r.x.ax = VESA_BIOS_INFO;
	r.x.di = FP_OFF(vbeinfo);
	s.es = FP_SEG(vbeinfo);

	// Call interrupt for VESA BIOS
	int86x(VESA_INTERRUPT, &r, &r, &s);
	
	if (r.x.ax != VESA_BIOS_SUCCESS){
		// VESA BIOS call was not successful
		if (VESA_VERBOSE){
			printf("%s.%d\t Error, Unable to query VESA BIOS [return code 0x%04x]\n", __FILE__, __LINE__, r.x.ax);
		}
		return -1;	
	}
	
	// Process VESA mode info structure data
	if (VESA_VERBOSE){
		printf("%s.%d\t VESA BIOS queried successfully!\n", __FILE__, __LINE__);
		vesa_printvbeinfo(vbeinfo);
		vesa_printvbemodes(vbeinfo);
	}
		
	return 0;	
}

int vesa_hasmode(unsigned short int mode, vbeinfo_t *vbeinfo){
	// Finds if a given VESA mode is present (and supported)
	// by the current video adapter
	
	int i;
	int found;
	unsigned short int *modes;
	vesamodeinfo_t *modeinfo = NULL;
	modeinfo = (vesamodeinfo_t *) malloc(sizeof(vesamodeinfo_t));
	
	found = 0;
	modes = (unsigned short int*) vbeinfo->mode_list_ptr;
	
	// Find the mode in the list of modes
	for (i = 0; modes[i] != VESA_MODELIST_LAST; i++){
		if (modes[i] == mode){
			found = 1;	
		}
	}
	
	if (found){
		vesa_getmodeinfo(mode, modeinfo);
		if (modeinfo->ModeAttributes != 0){
			// Mode present and available
			free(modeinfo);
			return 0;
		} else {
			// Mode present, but unavailable (e.g too little RAM)
			free(modeinfo);
			return -1;
		}
	} else {
		// Mode not present
		free(modeinfo);
		return -1;	
	}
	
}

int vesa_setmode(unsigned short int mode){
	// Initialise the video hardware at a given VESA mode
	
	union REGS r;
	struct SREGS s;
	
	if (VESA_VERBOSE){
		printf("%s.%d\t Setting VESA mode %xh\n", __FILE__, __LINE__, mode);
	}
	
	r.x.ax = VESA_MODE_SET;
	r.x.bx = mode;
	int86(VESA_INTERRUPT, &r, &r);
	
	if (r.x.ax != VESA_BIOS_SUCCESS){
		// VESA BIOS call was not successful
		if (VESA_VERBOSE){
			printf("%s.%d\t Error, Unable to set VESA mode %xh [return code 0x%04x]\n", __FILE__, __LINE__, mode, r.x.ax);
		}
		return -1;	
	}
	
	if (VESA_VERBOSE){
		printf("%s.%d\t Successfully set VESA mode %xh\n", __FILE__, __LINE__, mode);
	}
	return 0;
}

int vesa_setwindow(unsigned short int window, unsigned short int position){
	// Set the current active video memory window (since VGA graphics operates in 64KB windows)
	
	union REGS r;
	struct SREGS s;
	
	r.x.ax = VESA_WINDOW_SET;
	r.h.bh = 0;
	r.h.bl = window;
	r.x.dx = position;
	int86(VESA_INTERRUPT, &r, &r);
	
	if (r.x.ax != VESA_BIOS_SUCCESS){
		// VESA BIOS call was not successful
		if (VESA_VERBOSE){
			printf("%s.%d\t Error, Unable to set set VESA memory region window %d, position %d [return code 0x%04x]\n", __FILE__, __LINE__, window, position, r.x.ax);
		}
		return -1;	
	}
	return 0;
}

void vesa_printvbeinfo(vbeinfo_t *vbeinfo){
	// Print the current contents of the vbeinfo structure
	
	printf("%s.%d\t VESA BIOS information follows\n", __FILE__, __LINE__);
	printf("VBE Signature:\t %s\n", vbeinfo->vbe_signature);
	printf("VBE Vendor:\t %s\n", vbeinfo->oem_string_ptr);
	printf("VBE Version:\t %d\n", vbeinfo->vbe_version);
	printf("SW Version:\t %d\n", vbeinfo->oem_software_rev);
	printf("Vendor:	\t %d\n", vbeinfo->oem_vendor_name_ptr);
	printf("Product:\t %d\n", vbeinfo->oem_product_name_ptr);
	printf("Version:\t %d\n", vbeinfo->oem_product_rev_ptr);
	printf("Total RAM:\t %dKB\n", (vbeinfo->total_memory * 64));
	printf("%s.%d\t End of VESA BIOS information\n", __FILE__, __LINE__);
	printf("----------\n");
}

void vesa_printvbemodes(vbeinfo_t *vbeinfo){
	// Print all the mode numbers defined by 
	// the current of the vbeinfo structure
	
	int i;
	unsigned short int *modes;
	
	printf("%s.%d\t VESA mode list follows\n", __FILE__, __LINE__);
	
	// 'modes' is set to an array of 16bit unsigned integers
	// as pointed to by the modes list pointer in the vbeinfo object
	// Last mode is defined as -1.
	modes = (unsigned short int*) vbeinfo->mode_list_ptr;
	for (i = 0; modes[i] != VESA_MODELIST_LAST; i++){
		printf("Mode %3d: %xh\n", i, modes[i]);
	}
	printf("%s.%d\t Found %d VESA modes\n", __FILE__, __LINE__, i);
	printf("----------\n");
}

void vesa_printvbemodeinfo(vesamodeinfo_t *modeinfo){
	// Given a valid modeinfo structure, print the details
	// of that VESA mode
	
	printf("%s.%d\t VESA mode information follows\n", __FILE__, __LINE__);
	printf("Resolution\t\t: %d x %d\n", modeinfo->XResolution, modeinfo->YResolution);
	printf("Colour Depth\t\t: %dbpp\n", modeinfo->BitsPerPixel);
	printf("Bitplanes\t\t: %d\n", modeinfo->NumberOfPlanes);
	printf("Supported\t\t: %d\n", modeinfo->ModeAttributes);
	printf("WinAAttributes\t\t: %d\n", modeinfo->WinAAttributes);
	printf("WinBAttributes\t\t: %d\n", modeinfo->WinBAttributes);
	printf("WinGranularity\t\t: %d\n", modeinfo->WinGranularity);
	printf("WinSize\t\t\t: %d\n", modeinfo->WinSize);
	printf("WinASegment\t\t: %xh\n", modeinfo->WinASegment);
	printf("WinBSegment\t\t: %xh\n", modeinfo->WinBSegment);
	printf("BytesPerScanLine\t: %d\n", modeinfo->BytesPerScanLine);
	printf("XCharSize\t\t: %d\n", modeinfo->XCharSize);
	printf("YCharSize\t\t: %d\n", modeinfo->YCharSize);
	printf("NumberOfPlanes\t\t: %d\n", modeinfo->NumberOfPlanes);
	printf("NumberOfBanks\t\t: %d\n", modeinfo->NumberOfBanks);
	printf("MemoryModel\t\t: %d\n", modeinfo->MemoryModel);
	printf("BankSize\t\t: %d\n", modeinfo->BankSize);
	printf("NumberOfImagePages\t: %d\n", modeinfo->NumberOfImagePages);
	printf("RedMaskSize\t\t: %d\n", modeinfo->RedMaskSize);
	printf("RedFieldPosition\t: %d\n", modeinfo->RedFieldPosition);
	printf("GreenMaskSize\t\t: %d\n", modeinfo->GreenMaskSize);
	printf("GreenFieldPosition\t: %d\n", modeinfo->GreenFieldPosition);
	printf("BlueMaskSize\t\t: %d\n", modeinfo->BlueMaskSize);
	printf("BlueFieldPosition\t: %d\n", modeinfo->BlueFieldPosition);
	printf("RsvdMaskSize\t\t: %d\n", modeinfo->RsvdMaskSize);
	printf("RsvdFieldPosition\t: %d\n", modeinfo->RsvdFieldPosition);
	printf("DirectColorModeInfo\t: %d\n", modeinfo->DirectColorModeInfo);
	printf("----------\n");
}
