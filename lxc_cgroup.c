/*
 * lxc: linux Container library
 *
 * (C) Copyright IBM Corp. 2007, 2008
 *
 * Authors:
 * Daniel Lezcano <daniel.lezcano at free.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>

#include <lxc/lxc.h>
#include <lxc/log.h>

#include "arguments.h"

lxc_log_define(lxc_cgroup_ui, lxc_cgroup);

static int my_checker(const struct lxc_arguments* args)
{
	if (!args->argc) {
		lxc_error(args, "missing state object");
		return -1;
	}
	return 0;
}

static const struct option my_longopts[] = {
	LXC_COMMON_OPTIONS
};

static struct lxc_arguments my_args = {
	.progname = "lxc-cgroup",
	.help     = "\
--name=NAME state-object [value]\n\
\n\
Get or set the value of a state object (for example, 'cpuset.cpus')\n\
in the container's cgroup for the corresponding subsystem.\n\
\n\
Options :\n\
  -n, --name=NAME      container name",
	.options  = my_longopts,
	.parser   = NULL,
	.checker  = my_checker,
};

int main(int argc, char *argv[])
{
	char *state_object = NULL, *value = NULL;

	if (lxc_arguments_parse(&my_args, argc, argv))
		return -1;

	if (lxc_log_init(my_args.name, my_args.log_file, my_args.log_priority,
			 my_args.progname, my_args.quiet))
		return -1;

	state_object = my_args.argv[0];

	if ((argc) > 1)
		value = my_args.argv[1];

	if (value) {
		if (lxc_cgroup_set(my_args.name, state_object, value, my_args.lxcpath)) {
			ERROR("failed to assign '%s' value to '%s' for '%s'",
				value, state_object, my_args.name);
			return -1;
		}
	} else {
		const unsigned long len = 4096;
		int ret;
		char buffer[len];

		ret = lxc_cgroup_get(my_args.name, state_object, buffer, len, my_args.lxcpath);
		if (ret < 0) {
			ERROR("failed to retrieve value of '%s' for '%s'",
			      state_object, my_args.name);
			return -1;
		}

		printf("%*s", ret, buffer);
	}

	return 0;
}
