#include "ckstructs.h"

int main(void)
{
	WriteString("Hi", 22, 40);
	struct FCB * fHandle = CreateFile("TEST.TXT");
	WriteDouble((long) fHandle, 23, 40);
	if (fHandle)
	{
		WriteFile(fHandle, "1234\n", 5);
		CloseFile(fHandle);
	}
	while (1)
	{
		sys_Sleep(200);
	}
	return 0;
}
