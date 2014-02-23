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
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <net/if.h>

#include "error.h"
#include "af_unix.h"

#include <lxc/log.h>
#include <lxc/state.h>
#include <lxc/monitor.h>

lxc_log_define(lxc_monitor, lxc);

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

static void lxc_monitor_send(struct lxc_msg *msg, const char *lxcpath)
{
	int fd;
	struct sockaddr_un addr = { .sun_family = AF_UNIX };
	char *offset = &addr.sun_path[1];
	size_t ret, len;

	/*
	 * addr.sun_path is only 108 bytes.
	 * should we take a hash of lxcpath?  a subset of it?
	 */
	len = sizeof(addr.sun_path) - 1;
	ret = snprintf(offset, len, "%s/lxc-monitor", lxcpath);
	if (ret < 0 || ret >= len) {
		ERROR("lxcpath too long to open monitor");
		return;
	}

	fd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0)
		return;

	sendto(fd, msg, sizeof(*msg), 0,
	       (const struct sockaddr *)&addr, sizeof(addr));

	close(fd);
}

void lxc_monitor_send_state(const char *name, lxc_state_t state, const char *lxcpath)
{
	struct lxc_msg msg = { .type = lxc_msg_state,
			       .value = state };
	strncpy(msg.name, name, sizeof(msg.name));
	msg.name[sizeof(msg.name) - 1] = 0;

    //����֪ͨ����ؽ���
	lxc_monitor_send(&msg, lxcpath);
}

int lxc_monitor_open(const char *lxcpath)
{
	struct sockaddr_un addr = { .sun_family = AF_UNIX };
	char *offset = &addr.sun_path[1];
	int fd;
	size_t ret, len;

	/*
	 * addr.sun_path is only 108 bytes.
	 * should we take a hash of lxcpath?  a subset of it?
	 */
	len = sizeof(addr.sun_path) - 1;
	ret = snprintf(offset, len, "%s/lxc-monitor", lxcpath);
	if (ret < 0 || ret >= len) {
		ERROR("lxcpath too long to open monitor");
		return -1;
	}

	fd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0) {
		ERROR("socket : %s", strerror(errno));
		return -1;
	}

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr))) {
		ERROR("bind : %s", strerror(errno));
		close(fd);
		return -1;
	}

	return fd;
}

/* timeout of 0 means return immediately;  -1 means wait forever */
int lxc_monitor_read_timeout(int fd, struct lxc_msg *msg, int timeout)
{
	struct sockaddr_un from;
	socklen_t len = sizeof(from);
	int ret;
	fd_set rfds;
	struct timeval tv;

	if (timeout != -1) {
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		tv.tv_sec = timeout;
		tv.tv_usec = 0;

		ret = select(fd+1, &rfds, NULL, NULL, &tv);
		if (ret == -1)
			return -1;
		else if (!ret)
			return -2;  // timed out
	}

	ret = recvfrom(fd, msg, sizeof(*msg), 0,
		       (struct sockaddr *)&from, &len);
	if (ret < 0) {
		SYSERROR("failed to receive state");
		return -1;
	}

	return ret;
}

int lxc_monitor_read(int fd, struct lxc_msg *msg)
{
	return lxc_monitor_read_timeout(fd, msg, -1);
}

int lxc_monitor_close(int fd)
{
	return close(fd);
}
