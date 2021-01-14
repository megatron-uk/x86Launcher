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
	
	return 0;
}

int vesa_getvbeinfo(vbeinfo_t *vbeinfo){
	// Retrieve basic VESA BIOS information
	
	union REGS r;
	struct SREGS s;
	
	// Set VESA BIOS call parameters and store pointer
	// to vbeinfo datastructure which will hold info
	// after this call.
	r.x.ax = 0x4F00;
	r.x.di = FP_OFF(vbeinfo);
	s.es = FP_SEG(vbeinfo);

	// Call interrupt for VESA BIOS
	int86x(0x10, &r, &r, &s);
	
	if (r.x.ax != VESA_BIOS_SUCCESS){
		// VESA BIOS call was not successful
		if (VESA_VERBOSE){
			printf("%s.%d\t Error, Unable to query VESA BIOS [return code %x]\n", __FILE__, __LINE__, r.x.ax);
		}
		return -1;	
	}
	
	// Process VESA mode info structure data
	if (VESA_VERBOSE){
		printf("%s.%d\t VESA BIOS queried successfully!\n", __FILE__, __LINE__);
		vesa_printvbe(vbeinfo);
	}
		
	return 0;	
}

void vesa_printvbe(vbeinfo_t *vbeinfo){
	// Print the current contents of the vbeinfo structure
	
	printf("VBE Signature:\t %s\n", vbeinfo->vbe_signature);
	printf("VESA Version:\t %d\n", vbeinfo->vbe_version);
	printf("SW Version:\t %d\n", vbeinfo->oem_software_rev);
	printf("Vendor:	\t %d\n", vbeinfo->oem_vendor_name_ptr);
	printf("Product:\t %d\n", vbeinfo->oem_product_name_ptr);
	printf("Version:\t %d\n", vbeinfo->oem_product_rev_ptr);
	printf("Total RAM:\t %dKB\n", (vbeinfo->total_memory * 64));
	
}
