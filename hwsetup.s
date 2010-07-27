	.global HwSetup

	.text

	.code32

	.include "hwhelp.s"

HwSetup:
# set up interrupt controller
	mov $0b00010001, %al
	out %al, $0x20
	out %al, $0xa0
	mov $0x20, %al
	out %al, $0x21
	mov $0x28, %al
	out %al, $0xa1
	mov $0b00000100, %al
	out %al, $0x21
	mov $0b00000010, %al
	out %al, $0xa1
	mov $0x00000001, %al
	out %al, $0x21
	out %al, $0xa1
	mov $0b11111011, %al	# disable all interrupts
	out %al, $0x21
	mov $0b11111111, %al
	out %al, $0xa1

# set up keyboard controller
	call InBuffEmpty
	mov $0x0FA, %al
	out %al, $0x60
	call OutBuffFull
	in $0x60, %al
	call InBuffEmpty
	mov $0xF0, %al
	out %al, $0x60
	call OutBuffFull
	in $0x60, %al
	call InBuffEmpty
	mov $0x2, %al
	out %al, $0x60
	call OutBuffFull
	in $0x60, %al
	call InBuffEmpty
	mov $0x60, %al
	out %al, $0x64
	call InBuffEmpty
	mov $0x45, %al
	out %al, $0x60

#   in   $0x60, %al       # Clear any pending Kbd interrupt


# enable interrupts
#	mov $0b11111000, %al	# enable keyboard + timer interrupt
	mov $0b11111010, %al	# enable timer interrupt
	out %al, $0x21

# reset ide controller
#	mov $0b00000100, %al
#	out %al, $0x3f6
# enable hd interrupt
	mov $0b10111111, %al
	out %al, $0xa1

# set up timer hardware for interrupts every 10ms
	mov $0x9C, %al
	out %al, $0x40
	mov $0x2e, %al
	out %al, $0x40

HD_PORT=0x1F0
# set up ide controller
.again1:
	in %dx, %al
	test $0x80, %al
	jnz .again1
	mov $HD_PORT+6, %dx
	mov $0, %al
	out %al, %dx
	mov $HD_PORT+7, %dx
	mov $0x10, %al
	out %al, %dx
	ret
