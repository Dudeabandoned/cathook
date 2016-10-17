/*
 * sharedobj.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include "sharedobj.h"
#include "logging.h"

#include <unistd.h>
#include <link.h>
#include <dlfcn.h>
#include <libgen.h>

#define STEAM_BASEDIR "/home/nullifiedcat/.local/share/Steam/"

namespace logging {
void Info(const char* fmt, ...);
}

sharedobj::SharedObject* sharedobj::steamclient = 0;
sharedobj::SharedObject* sharedobj::client = 0;
sharedobj::SharedObject* sharedobj::engine = 0;
sharedobj::SharedObject* sharedobj::vguimatsurface = 0;
sharedobj::SharedObject* sharedobj::vgui2 = 0;
sharedobj::SharedObject* sharedobj::vstdlib = 0;
sharedobj::SharedObject* sharedobj::tier0 = 0;

sharedobj::SharedObject::SharedObject(const char* path, bool factory) {
	this->path = path;
	while (!(lmap = (link_map*)dlopen(path, RTLD_NOLOAD))) {
		sleep(1);
		if (0 != dlerror()) {
			logging::Info("DLERROR: %s", dlerror());
		}
	}
	logging::Info("Shared object %s loaded at 0x%08x", basename(lmap->l_name), lmap->l_addr);
	if (factory) {
		fptr = reinterpret_cast<fn_CreateInterface_t>(dlsym(lmap, "CreateInterface"));
		if (!this->fptr) {
			logging::Info("Failed to create interface factory for %s", basename(lmap->l_name));
		}
	}
}

int* sharedobj::SharedObject::Pointer(int offset) {
	if (this->lmap) {
		return (int*)((int) this->lmap->l_addr + offset);
	} else {
		return 0;
	}
}

void* sharedobj::SharedObject::CreateInterface(const char* name) {
	int result;
	void* interface = (void*)(fptr(name, &result));
	if (result) {
		logging::Info("Interface creation failed: %s from %s", name, basename(lmap->l_name));
	}
	return interface;
}

void sharedobj::LoadAllSharedObjects() {
	sharedobj::client = new SharedObject(STEAM_BASEDIR "steamapps/common/Team Fortress 2/tf/bin/client.so", true);
	sharedobj::engine = new SharedObject(STEAM_BASEDIR "steamapps/common/Team Fortress 2/bin/engine.so", true);
	sharedobj::steamclient = new SharedObject(STEAM_BASEDIR "linux32/steamclient.so", true);
	sharedobj::tier0 = new SharedObject(STEAM_BASEDIR "steamapps/common/Team Fortress 2/bin/libtier0.so", false);
	sharedobj::vgui2 = new SharedObject(STEAM_BASEDIR "steamapps/common/Team Fortress 2/bin/vgui2.so", true);
	sharedobj::vguimatsurface = new SharedObject(STEAM_BASEDIR "steamapps/common/Team Fortress 2/bin/vguimatsurface.so", true);
	sharedobj::vstdlib = new SharedObject(STEAM_BASEDIR "steamapps/common/Team Fortress 2/bin/libvstdlib.so", true);
}

