/* vesa.h, VESA mode function prototypes for the x86launcher.
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

#define VESA_VERBOSE			1		// Enable/disable debug output for this module at compile time.
#define VESA_BIOS_SUCCESS	0x004F	// A 'success' code on quering vbeinfo

/* VESA data structure taken from
   http://www.geocities.com/siliconvalley/horizon/6933/vesa.txt
*/

typedef struct {
  short   ModeAttributes;		/* Mode attributes                  */
  char    WinAAttributes;		/* Window A attributes              */
  char    WinBAttributes;		/* Window B attributes              */
  short   WinGranularity;		/* Window granularity in k          */
  short   WinSize;				/* Window size in k                 */
  short   WinASegment;			/* Window A segment                 */
  short   WinBSegment;			/* Window B segment                 */
  void    *WinFuncPtr;			/* Pointer to window function       */
  short   BytesPerScanLine;		/* Bytes per scanline               */
  short   XResolution;			/* Horizontal resolution            */
  short   YResolution;			/* Vertical resolution              */
  char    XCharSize;				/* Character cell width             */
  char    YCharSize;				/* Character cell height            */
  char    NumberOfPlanes;		/* Number of memory planes          */
  char    BitsPerPixel;			/* Bits per pixel                   */
  char    NumberOfBanks;			/* Number of CGA style banks        */
  char    MemoryModel;			/* Memory model type                */
  char    BankSize;				/* Size of CGA style banks          */
  char    NumberOfImagePages;	/* Number of images pages           */
  char    res1;					/* Reserved                         */
  char    RedMaskSize;			/* Size of direct color red mask    */
  char    RedFieldPosition;		/* Bit posn of lsb of red mask      */
  char    GreenMaskSize;			/* Size of direct color green mask  */
  char    GreenFieldPosition;	/* Bit posn of lsb of green mask    */
  char    BlueMaskSize;			/* Size of direct color blue mask   */
  char    BlueFieldPosition;		/* Bit posn of lsb of blue mask     */
  char    RsvdMaskSize;			/* Size of direct color res mask    */
  char    RsvdFieldPosition;		/* Bit posn of lsb of res mask      */
  char    DirectColorModeInfo;	/* Direct color mode attributes     */

  /* VESA 2.0 variables */
  long    PhysBasePtr;			/* physical address for flat frame buffer */
  long    OffScreenMemOffset;	/* pointer to start of off screen memory */
  short   OffScreenMemSize;		/* amount of off screen memory in 1k units */
  char    res2[206];				/* Pad to 256 byte block size       */
} vesamodeinfo_t;

typedef struct vbeinfo_t {
	char  				vbe_signature[4];
	unsigned short int 	vbe_version;
	unsigned long int 	oem_string_ptr;
	unsigned long int 	capabilities;
	unsigned long int 	mode_list_ptr;
	unsigned short int 	total_memory;
	unsigned short int 	oem_software_rev;
	unsigned long int 	oem_vendor_name_ptr;
	unsigned long int 	oem_product_name_ptr;
	unsigned long int 	oem_product_rev_ptr;
	unsigned char	  	reserved[222];
	char  				oem_data[256];
	char  				misc_data[512];
} vbeinfo_t;

int vesa_getmodeinfo(unsigned short mode, vesamodeinfo_t *modeinfo);
int vesa_getvbeinfo(vbeinfo_t *vbeinfo);
void vesa_printvbe(vbeinfo_t *vbeinfo);
