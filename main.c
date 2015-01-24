/*
	CDDA Enabler
	Copyright (C) 2015, Total_Noob

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

#include <pspsdk.h>
#include <pspkernel.h>
#include <systemctrl.h>

#include <stdio.h>
#include <string.h>

PSP_MODULE_INFO("CDDA Enabler", 0x1007, 1, 0);

STMOD_HANDLER previous;

int OnModuleStart(SceModule2 *mod)
{
	if (strcmp(mod->modname, "pops") == 0)
	{
		u32 module_sdk_version = 0;

		int i = 0;

		while (i < mod->ent_size)
		{
			SceLibraryEntryTable *entry = (SceLibraryEntryTable *)(mod->ent_top + i);

			u32 *table = entry->entrytable;
			int total = entry->stubcount + entry->vstubcount;

			int j;
			for (j = 0; j < total; j++)
			{
				if (table[j] == 0x11B97506)
				{
					module_sdk_version = *(u32 *)(table[j + total]);
					break;
				}
			}

			i += (entry->len * 4);
		}

		if (module_sdk_version >= 0x05000010)
		{
			for (i = 0; i < mod->text_size; i += 4)
			{
				u32 addr = mod->text_addr + i;

				/* Fix index length */
				if ((_lw(addr) == 0x14C00014 && _lw(addr + 4) == 0x24E2FFFF) ||
					(_lw(addr) == 0x14A00014 && _lw(addr + 4) == 0x24C2FFFF))
				{
					_sh(0x1000, addr + 2);
					_sh(0, addr + 4);
					break;
				}
			}

			sceKernelDcacheWritebackAll();
			sceKernelIcacheClearAll();
		}
	}

	if (!previous)
		return 0;

	return previous(mod);
}

int module_start(SceSize args, void *argp)
{
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);
	return 0;
}