#include <fcntl.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>
#include "onit.h"

static int onit_open_console() {
	int onefd;
	int twofd;

	/* stdin */
	onefd = open("/dev/console", O_RDONLY, 0);
	dup2(onefd, 0);

	/* stdout and stderr */
	twofd = open("/dev/console", O_RDWR, 0);
	dup2(twofd, 1);
	dup2(twofd, 2);

	if (onefd > 2) close(onefd);
	if (twofd > 2) close(twofd);
	return 0;
}

static void onit_mount_fs() {
	mount("none", "/proc", "proc", 0, "");
	mount("none", "/sys", "sysfs", 0, "");
	mount("none", "/var/tmp", "tmpfs", 0, "mode=0700");
	mount("devtmpfs", "/dev", "devtmpfs", 0, "");
}

static char * const shell_args[] = {
	"/bin/cttyhack",
	"/bin/sh",
	/* "--login", */
	NULL,
};

static char * const shell_env[] = {
	"PATH=/bin:/usr/bin:/sbin:/usr/sbin",
	NULL,
};


static void onit_spawn_shell() {
	int pid;

	pid = fork();
	if(pid == -1) {
		fprintf(stderr, "Deu coco \n");
	} else if(pid == 0) {
		execve(shell_args[0], shell_args, shell_env);
	}
}

static void onit_print_args(int argc, char ** argv) {
	int i;

	for (i = 0; i < argc; i++) {
		printf("\targ[%d]='%s'\n", i, argv[i]);
	}
}


int main(int argc, char ** argv) {
	
	/*if(onit_create_console()) {
		/ * create console failed... * /
		return -1;
	}*/

	onit_mount_fs();

	if(onit_open_console()) {
		/* open console failed... */
		return -2;
	}

	printf("Here I am!\nShelly should be readyy\n");
	onit_print_args(argc, argv);

	onit_spawn_shell();
	while(1);
	return 0;
}

