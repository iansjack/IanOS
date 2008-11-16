void main(void)
{
	WriteString("", 22, 40);
	while (1)
	{
		asm("movq $17, %r9");
		asm("syscall");
	}
}
