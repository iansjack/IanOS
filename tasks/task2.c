#include <fcntl.h>

int main(void)
{
	int fileDescriptor = creat("TEST.TXT");
	if (fileDescriptor != -1) {
		write(fileDescriptor, "1234\n", 5);
		close(fileDescriptor);
	}
	exit();
	return (0);
}
