#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv) {
	unsigned int maj, min;
	struct stat conso;

	stat("/dev/console", &conso);

	maj = major(conso.st_rdev);
	min = minor(conso.st_rdev);

	printf("Console info - min: %u; maj: %u;\n", min, maj);

	return 0;
}
