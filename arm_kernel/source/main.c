#include "types.h"
#include "utils.h"
#include "../../payload/arm_user_bin.h"

typedef struct URL_Patch
{

	int address;
	char url[80];

} URL_Patch;


static const char repairData_set_fault_behavior[] = {
	0xE1,0x2F,0xFF,0x1E,0xE9,0x2D,0x40,0x30,0xE5,0x93,0x20,0x00,0xE1,0xA0,0x40,0x00,
	0xE5,0x92,0x30,0x54,0xE1,0xA0,0x50,0x01,0xE3,0x53,0x00,0x01,0x0A,0x00,0x00,0x02,
	0xE1,0x53,0x00,0x00,0xE3,0xE0,0x00,0x00,0x18,0xBD,0x80,0x30,0xE3,0x54,0x00,0x0D,
};
static const char repairData_set_panic_behavior[] = {
	0x08,0x16,0x6C,0x00,0x00,0x00,0x18,0x0C,0x08,0x14,0x40,0x00,0x00,0x00,0x9D,0x70,
	0x08,0x16,0x84,0x0C,0x00,0x00,0xB4,0x0C,0x00,0x00,0x01,0x01,0x08,0x14,0x40,0x00,
	0x08,0x15,0x00,0x00,0x08,0x17,0x21,0x80,0x08,0x17,0x38,0x00,0x08,0x14,0x30,0xD4,
	0x08,0x14,0x12,0x50,0x08,0x14,0x12,0x94,0xE3,0xA0,0x35,0x36,0xE5,0x93,0x21,0x94,
	0xE3,0xC2,0x2E,0x21,0xE5,0x83,0x21,0x94,0xE5,0x93,0x11,0x94,0xE1,0x2F,0xFF,0x1E,
	0xE5,0x9F,0x30,0x1C,0xE5,0x9F,0xC0,0x1C,0xE5,0x93,0x20,0x00,0xE1,0xA0,0x10,0x00,
	0xE5,0x92,0x30,0x54,0xE5,0x9C,0x00,0x00,
};
static const char repairData_usb_root_thread[] = {
	0xE5,0x8D,0xE0,0x04,0xE5,0x8D,0xC0,0x08,0xE5,0x8D,0x40,0x0C,0xE5,0x8D,0x60,0x10,
	0xEB,0x00,0xB2,0xFD,0xEA,0xFF,0xFF,0xC9,0x10,0x14,0x03,0xF8,0x10,0x62,0x4D,0xD3,
	0x10,0x14,0x50,0x00,0x10,0x14,0x50,0x20,0x10,0x14,0x00,0x00,0x10,0x14,0x00,0x90,
	0x10,0x14,0x00,0x70,0x10,0x14,0x00,0x98,0x10,0x14,0x00,0x84,0x10,0x14,0x03,0xE8,
	0x10,0x14,0x00,0x3C,0x00,0x00,0x01,0x73,0x00,0x00,0x01,0x76,0xE9,0x2D,0x4F,0xF0,
	0xE2,0x4D,0xDE,0x17,0xEB,0x00,0xB9,0x92,0xE3,0xA0,0x10,0x00,0xE3,0xA0,0x20,0x03,
	0xE5,0x9F,0x0E,0x68,0xEB,0x00,0xB3,0x20,
};



static unsigned int __attribute__((noinline)) disable_mmu(void)
{
	unsigned int control_register = 0;
	asm volatile("MRC p15, 0, %0, c1, c0, 0" : "=r" (control_register));
	asm volatile("MCR p15, 0, %0, c1, c0, 0" : : "r" (control_register & 0xFFFFEFFA));
	return control_register;
}

static void __attribute__((noinline)) restore_mmu(unsigned int control_register)
{
	asm volatile("MCR p15, 0, %0, c1, c0, 0" : : "r" (control_register));
}

int _main()
{
	int(*disable_interrupts)() = (int(*)())0x0812E778;
	int(*enable_interrupts)(int) = (int(*)(int))0x0812E78C;
	void(*invalidate_icache)() = (void(*)())0x0812DCF0;
	void(*invalidate_dcache)(unsigned int, unsigned int) = (void(*)())0x08120164;
	void(*flush_dcache)(unsigned int, unsigned int) = (void(*)())0x08120160;
	char* (*kernel_memcpy)(void*, void*, int) = (char*(*)(void*, void*, int))0x08131D04;

	flush_dcache(0x081200F0, 0x4001); 
	int level = disable_interrupts();
	unsigned int control_register = disable_mmu();
	*(volatile u32*)0x0012F000 = *(volatile u32*)0x1016AD18;
	*(volatile u32*)0x08129A24 = 0xE12FFF1E;

	void * pset_fault_behavior = (void*)0x081298BC;
	kernel_memcpy(pset_fault_behavior, (void*)repairData_set_fault_behavior, sizeof(repairData_set_fault_behavior));

	void * pset_panic_behavior = (void*)0x081296E4;
	kernel_memcpy(pset_panic_behavior, (void*)repairData_set_panic_behavior, sizeof(repairData_set_panic_behavior));

	void * pusb_root_thread = (void*)0x10100174;
	kernel_memcpy(pusb_root_thread, (void*)repairData_usb_root_thread, sizeof(repairData_usb_root_thread));

	void * pUserBinSource = (void*)0x00148000;
	void * pUserBinDest = (void*)0x101312D0;
	kernel_memcpy(pUserBinDest, (void*)pUserBinSource, sizeof(arm_user_bin));

	/* *** https://github.com/dimok789/sign_patcher *** */

	*(volatile u32*)(0x05022474 - 0x05000000 + 0x081C0000) = 0xFFFFFFFF;	// NEW_TIMEOUT
	*(volatile u32*)(0x05054D6C - 0x05000000 + 0x081C0000) = 0xE3A00000;    // mov r0, 0
	*(volatile u32*)(0x05054D70 - 0x05000000 + 0x081C0000) = 0xE12FFF1E;    // bx lr
	*(volatile u32*)(0x05052A90 - 0x05000000 + 0x081C0000) = 0xe3a00000;    // mov r0, #0
    *(volatile u32*)(0x05052A94 - 0x05000000 + 0x081C0000) = 0xe12fff1e;    // bx lr
    *(volatile u32*)(0x05014CAC - 0x05000000 + 0x081C0000) = 0x20004770; 	// mov r0, #0; bx lr
	*(volatile u32*)(0x05052C44 - 0x05000000 + 0x081C0000) = 0xE3A00000; 	// mov r0, #0
	*(volatile u32*)(0x05052C48 - 0x05000000 + 0x081C0000) = 0xE12FFF1E; 	// bx lr
	*(volatile u32*)(0x0500A818 - 0x05000000 + 0x081C0000) = 0x20002000; 	// mov r0, #0; mov r0, #0
	*(volatile u32*)(0xE0030D68 - 0xE0000000 + 0x12900000) = 0xE3A00000; 	// mov r0, #0
	*(volatile u32*)(0xE0030D34 - 0xE0000000 + 0x12900000) = 0xE3A00000; 	// mov r0, #0
	*(volatile u32*)(0xE0030498 - 0xE0000000 + 0x12900000) = 0xE3A00000; 	// mov r0, #0
	*(volatile u32*)(0x040017E0 - 0x04000000 + 0x08280000) = 0xE3A00000; 	// mov r0, #0
	*(volatile u32*)(0x040019C4 - 0x04000000 + 0x08280000) = 0xE3A00000; 	// mov r0, #0
	*(volatile u32*)(0x04001BB0 - 0x04000000 + 0x08280000) = 0xE3A00000; 	// mov r0, #0
	*(volatile u32*)(0x04001D40 - 0x04000000 + 0x08280000) = 0xE3A00000; 	// mov r0, #0

	int patch_num = *(int*)0x5080B000;
	URL_Patch* curr_patch = (URL_Patch*)0x5080C000; 

	if (patch_num > 0)
	{
		for (int i = 0; i < patch_num; ++i)
		{
			if(curr_patch->address && &curr_patch->url)
			{
			kernel_memcpy((void*)curr_patch->address, (void*)&curr_patch->url, s_strlen((const char*)&curr_patch->url) + 1);
			curr_patch++;
			}
		}
	}	




	*(volatile u32*)(0x1555500) = 0;

	/* REENABLE MMU */
	restore_mmu(control_register);

	invalidate_dcache(0x081298BC, 0x4001); // giving a size >= 0x4000 invalidates all cache
	invalidate_icache();

	enable_interrupts(level);

    //shutdown(1);

	return 0;
}