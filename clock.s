	.text

	.global gettime

gettime:
	mov $0x0, %al
	out %al, $0x70
	in $0x71, %al	# seconds
	call bcdtohex
	mov %al, sec
	mov $0x2, %al
	out %al, $0x70
	in $0x71, %al	# minutes
	call bcdtohex
	mov %al, min
	mov $0x4, %al
	out %al, $0x70
	in $0x71, %al	# hours
	call bcdtohex
	mov %al, hour
	mov $0x7, %al
	out %al, $0x70
	in $0x71, %al	# day
	call bcdtohex
	mov %al, day
	mov $0x8, %al
	out %al, $0x70
	in $0x71, %al	# month
	call bcdtohex
	mov %al, month
	mov $0x9, %al
	out %al, $0x70
	in $0x71, %al	# year
	call bcdtohex
	mov %al, year
	ret

bcdtohex:
	mov %al, %bl
	and $0xF0, %bl
	sar $1, %bl
	and $0xF, %al
	add %bl, %al
	sar $2, %bl
	add %bl, %al
	ret
