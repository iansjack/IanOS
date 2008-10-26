/*
==============================
Switch to the next ready task
==============================
*/
void SwTasks()
{
	asm("int $20");
}

/*
==========================
Switch to a specific task
==========================
*/
void SwTasks15(task)
{
	asm("mov %rdi, %r15");
	asm("int $22");
}

/*
==========================================
A utility function to copy a memory range
==========================================
*/
void copyMem(unsigned char source[], unsigned char dest[], long size)
{
	int i;
	for (i = 0; i < size; i++)
		dest[i] = source[i];
}

/*
==========================================================================
A kernel library function to write a null-terminated string to the screen.
==========================================================================
*/
void KWriteString(char * str, int row, int col)
{
	char * VideoBuffer = (char *)0xB8000;

	asm("cli");
	asm("push %rax");
	asm("push %rbx");
	asm("push %rcx");
	asm("push %rdx");
	asm("push %rdi");
	asm("push %rsi");
	int temp = 160 * row + 2 * col;
	int i = 0;
	while (str[i] != 0)
	{
		VideoBuffer[temp + 2 * i] = str[i];
		i++;
	}
	asm("pop %rsi");
	asm("pop %rdi");
	asm("pop %rdx");
	asm("pop %rcx");
	asm("pop %rbx");
	asm("pop %rax");
	asm("sti");
}

