/*
    NitroHax -- Cheat tool for the Nintendo DS
    Copyright (C) 2008  Michael "Chishm" Chisholm

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

#include <nds.h> 
#include <nds/fifomessages.h>
#include "sdmmc.h"
#include "debugToFile.h"
#include "cardEngine.h"
#include "fat.h"

static bool initialized = false;
static bool initializedIRQ = false;
static bool calledViaIPC = false;
extern vu32* volatile cardStruct;
extern vu32* volatile cacheStruct;
extern u32 fileCluster;
extern u32 sdk_version;
vu32* volatile sharedAddr = (vu32*)0x027FFB08;

void initLogging() {
	if(!initialized) {
		if (sdmmc_read16(REG_SDSTATUS0) != 0) {
			sdmmc_controller_init();
			sdmmc_sdcard_init();
		}
		FAT_InitFiles(false);
		u32 myDebugFile = getBootFileCluster ("NDSBTSRP.LOG");
		enableDebug(myDebugFile);
		dbg_printf("logging initialized\n");		
		dbg_printf("sdk version :");
		dbg_hexa(sdk_version);		
		initialized=true;
	}
	
}

void runCardEngineCheck (void) {
	//dbg_printf("runCardEngineCheck\n");
	int oldIME = enterCriticalSection();
	
	initLogging();


	if(*(vu32*)(0x027FFB14) == (vu32)0x027ff800)
    {
        dbg_printf("\ncard read received\n");			
			
		if(calledViaIPC) {
			dbg_printf("\ntriggered via IPC\n");
		}
				
		// old sdk version
		u32 src = *(vu32*)(sharedAddr+2);
		u32 dst = *(vu32*)(sharedAddr);
		u32 len = *(vu32*)(sharedAddr+1);
		u32 marker = *(vu32*)(sharedAddr+3);
		
		dbg_printf("\nstr : \n");
		dbg_hexa(cardStruct);		
		dbg_printf("\nsrc : \n");
		dbg_hexa(src);		
		dbg_printf("\ndst : \n");
		dbg_hexa(dst);
		dbg_printf("\nlen : \n");
		dbg_hexa(len);
		dbg_printf("\nmarker : \n");
		dbg_hexa(marker);
		
		fileRead(0x027ff800 ,fileCluster,src,len);
		
		dbg_printf("\nread \n");
		
		
		if(is_aligned(dst,4) || is_aligned(len,4)) {
			dbg_printf("\n aligned read : \n");
			//*(vu32*)(0x027FFB0C) = (vu32)2;
		} else {
			dbg_printf("\n misaligned read : \n");
			//*(vu32*)(0x027FFB0C) = (vu32)0;
		}	
		*(vu32*)(0x027FFB14) = 0;	
	}
	
	if(*(vu32*)(0x027FFB14) == (vu32)0x025FFB08)
    {
        dbg_printf("\ncard read received v2\n");
		
		if(calledViaIPC) {
			dbg_printf("\ntriggered via IPC\n");
		}
		
		// old sdk version
		u32 src = *(vu32*)(sharedAddr+2);
		u32 dst = *(vu32*)(sharedAddr);
		u32 len = *(vu32*)(sharedAddr+1);
		u32 marker = *(vu32*)(sharedAddr+3);
		
		dbg_printf("\nstr : \n");
		dbg_hexa(cardStruct);		
		dbg_printf("\nsrc : \n");
		dbg_hexa(src);		
		dbg_printf("\ndst : \n");
		dbg_hexa(dst);
		dbg_printf("\nlen : \n");
		dbg_hexa(len);
		dbg_printf("\nmarker : \n");
		dbg_hexa(marker);
		
		fileRead(dst,fileCluster,src,len);
		
		dbg_printf("\nread \n");
		
		if(is_aligned(dst,4) || is_aligned(len,4)) {
			dbg_printf("\n aligned read : \n");
			//*(vu32*)(0x027FFB0C) = (vu32)2;
		} else {
			dbg_printf("\n misaligned read : \n");
			//*(vu32*)(0x027FFB0C) = (vu32)0;
		}			
		*(vu32*)(0x027FFB14) = 0;		
	}

	leaveCriticalSection(oldIME);
}

//---------------------------------------------------------------------------------
void myIrqHandlerFIFO(void) {
//---------------------------------------------------------------------------------
	nocashMessage("myIrqHandlerFIFO");
	
	calledViaIPC = true;
	
	runCardEngineCheck();
}


void myIrqHandlerVBlank(void) {
	nocashMessage("myIrqHandlerVBlank");
	
	calledViaIPC = false;
	
	runCardEngineCheck();
}

u32 myIrqEnable(u32 irq) {	
	int oldIME = enterCriticalSection();	
	
	//nocashMessage("myIrqEnable\n");
	
	u32 irq_before = REG_IE | IRQ_IPC_SYNC;		
	irq |= IRQ_IPC_SYNC;
	REG_IPC_SYNC |= IPC_SYNC_IRQ_ENABLE;
	//nocashMessage("IRQ_IPC_SYNC enabled\n");

	REG_IE |= irq;
	leaveCriticalSection(oldIME);
	return irq_before;
}

void irqIPCSYNCEnable() {	
	if(!initializedIRQ) {
		int oldIME = enterCriticalSection();	
		initLogging();	
		dbg_printf("\nirqIPCSYNCEnable\n");	
		REG_IE |= IRQ_IPC_SYNC;
		REG_IPC_SYNC |= IPC_SYNC_IRQ_ENABLE;
		dbg_printf("IRQ_IPC_SYNC enabled\n");
		leaveCriticalSection(oldIME);
		initializedIRQ = true;
	}
}


