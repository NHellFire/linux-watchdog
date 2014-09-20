/*
watchdog - Linux watchdog daemon
    Copyright (C) 2014 Nathan Rennie-Waldock <nathan.renniewaldock@gmail.com>

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

#define WATCHDOG_FILE "/dev/watchdog"
#define WATCHDOG_INTERVAL 10

#define PIDFILE "/var/run/watchdog.pid"

#define WATCHDOG_MAJOR 10
#define WATCHDOG_MINOR 130

#include <errno.h>
#include <fcntl.h>
#ifndef NOWAYOUT
#include <signal.h>
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


int fd;
int pidfd;

#ifndef NOWAYOUT
void sighandler (int signal) {
	write(fd, "V", 1);
	close(fd);
	close(pidfd);
	exit(EXIT_FAILURE);
}
#endif


int main() {
	bool create_watchdog = false;

	struct stat st;
#ifndef NOWAYOUT
	struct sigaction sa;

	sa.sa_handler = &sighandler;
	sigfillset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
#endif

	if (stat(WATCHDOG_FILE, &st) == -1) {
		if (errno != ENOENT) {
			char error[1024];
			sprintf(error, "Cannot stat %s", WATCHDOG_FILE);
			perror(error);
			return EXIT_FAILURE;
		}
		create_watchdog = true;
	} else {
		if (!(S_ISCHR(st.st_mode) && major(st.st_rdev) == WATCHDOG_MAJOR && minor(st.st_rdev) == WATCHDOG_MINOR)) {
			if (unlink(WATCHDOG_FILE) == -1) {
				char error[1024];
				sprintf(error, "Failed to remove %s", WATCHDOG_FILE);
				perror(error);
				return EXIT_FAILURE;
			}
			create_watchdog = true;
		}
	}

	if (create_watchdog) {
		if (mknod(WATCHDOG_FILE, S_IFCHR | S_IRUSR | S_IWUSR, makedev(WATCHDOG_MAJOR, WATCHDOG_MINOR)) == -1) {
			char error[1024];
			sprintf(error, "Failed to create %s", WATCHDOG_FILE);
			perror(error);
			return EXIT_FAILURE;
		}
		printf("Created %s\n", WATCHDOG_FILE);
	}

	printf("Opening %s\n", WATCHDOG_FILE);

	fd = open(WATCHDOG_FILE, O_WRONLY);
	int ret = EXIT_SUCCESS;
	if (fd == -1) {
		perror("Failed to open device for writing");
		return EXIT_FAILURE;
	}

	if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		perror("Failed to lock device");
		return EXIT_FAILURE;
	}

	pidfd = open(PIDFILE, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (flock(pidfd, LOCK_EX | LOCK_NB) == -1) {
		printf("Already running\n");
		perror("Failed to lock PID file");
		return EXIT_FAILURE;
	}

	char pid[1024];
	sprintf(pid, "%d", getpid());
	write(pidfd, pid, strlen(pid));

	while (1) {
		ret = write(fd, "\0", 1);
		if (ret != 1) {
			ret = EXIT_FAILURE;
			break;
		}
		sleep(WATCHDOG_INTERVAL);
	}
	close(fd);
	close(pidfd);

	return ret;
}
