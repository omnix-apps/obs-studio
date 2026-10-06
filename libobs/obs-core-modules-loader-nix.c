/******************************************************************************
    Copyright (C) 2026 by FiniteSingularity <finitesingularityttv@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "obs-core-modules.h"

#include <obs-internal.h>
#include <obs.h>
#include <util/dstr.h>
#include <util/platform.h>

const char *core_module_bin = OBS_PLUGIN_PATH "/%module%";
const char *core_module_data = OBS_DATA_PATH "/obs-modules/core/%module%";

/* OBS_PLUGIN_PATH and OBS_DATA_PATH come from CMAKE_INSTALL_LIBDIR and
 * CMAKE_INSTALL_DATAROOTDIR, which are relative in a default build but absolute
 * when a distribution passes absolute install dirs (Nix does). Resolve relative
 * paths against the executable as before, and use absolute ones as they are. */
static char *core_module_path(const char *path)
{
	if (path[0] == '/')
		return bstrdup(path);

	struct dstr relative;
	dstr_init_copy(&relative, "../");
	dstr_cat(&relative, path);
	char *resolved = os_get_executable_path_ptr(relative.array);
	dstr_free(&relative);
	return resolved;
}

extern bool find_core_module(struct obs_runtime_module_info *info, obs_find_module_callback2_t callback, void *data);

void load_core_modules(obs_find_module_callback2_t callback, void *data)
{
	char *core_bin_path = core_module_path(core_module_bin);
	char *core_data_path = core_module_path(core_module_data);

	for (unsigned int i = 0; i < obs_core_modules_count; i++) {
		const char *name = obs_core_modules[i];
		struct dstr bin_path;
		struct dstr data_path;
		dstr_init_copy(&bin_path, core_bin_path);
		dstr_init_copy(&data_path, core_data_path);
		dstr_replace(&bin_path, "%module%", name);
		dstr_replace(&data_path, "%module%", name);

		struct obs_runtime_module_info module_info = {.path_info = {.binary = bin_path.array,
									    .data = data_path.array},
							      .type = OBS_MODULE_TYPE_CORE,
							      .name = name};

		bool found = find_core_module(&module_info, callback, data);

		dstr_free(&bin_path);
		dstr_free(&data_path);

		if (!found) {
			blog(LOG_ERROR, "Failed to load core module %s", name);
			break;
		}
	}

	bfree(core_bin_path);
	bfree(core_data_path);
}
