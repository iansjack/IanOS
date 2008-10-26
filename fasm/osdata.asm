;========================================================
; Interrupt Descriptor Table - will be relocated to 0x800
;========================================================
;myidt:	;TRAP_GATE64 code64, div0 - OSCode     ;0 - not quite sure why we need the - OSCode!
;	TRAP_GATE64 code64, i1	 - OSCode     ;1;
;	TRAP_GATE64 code64, i2	 - OSCode     ;2
;	TRAP_GATE64 code64, i3	 - OSCode     ;3
;	TRAP_GATE64 code64, i4	 - OSCode     ;4
;	TRAP_GATE64 code64, i5	 - OSCode     ;5
;	TRAP_GATE64 code64, i6	 - OSCode     ;6
;	TRAP_GATE64 code64, i7	 - OSCode     ;7
;	TRAP_GATE64 code64, i8	 - OSCode     ;8
;	TRAP_GATE64 code64, i9	 - OSCode     ;9
;	TRAP_GATE64 code64, ia	 - OSCode     ;10
;	TRAP_GATE64 code64, ib	 - OSCode     ;11
;	TRAP_GATE64 code64, ic	 - OSCode     ;12
;	TRAP_GATE64 code64, gpf  - OSCode     ;13
;	TRAP_GATE64 code64, pf	 - OSCode     ;14
;	TRAP_GATE64 code64, itf  - OSCode     ;15
;	TRAP_GATE64 code64, ig	 - OSCode     ;16
;	TRAP_GATE64 code64, intr - OSCode     ;17
;	TRAP_GATE64 code64, intr - OSCode     ;18
;	TRAP_GATE64 code64, intr - OSCode     ;19
;	TRAP_GATE64 code64, SwitchTasks - OSCode     ;20
;	INTR_GATE64 code64, int21 - OSCode     ;21
;	TRAP_GATE64 code64, SpecificSwitchTasks - OSCode     ;22
;	TRAP_GATE64 code64, intr - OSCode     ;23
;	TRAP_GATE64 code64, intr - OSCode     ;24
;	TRAP_GATE64 code64, intr - OSCode     ;25
;	TRAP_GATE64 code64, intr - OSCode     ;26
;	TRAP_GATE64 code64, intr - OSCode     ;27
;	TRAP_GATE64 code64, intr - OSCode     ;28
;	TRAP_GATE64 code64, intr - OSCode     ;29
;	TRAP_GATE64 code64, intr - OSCode     ;30
;	TRAP_GATE64 code64, intr - OSCode     ;31
;	INTR_GATE64 code64, TimerInt -OSCode  ;32 - timer
;	INTR_GATE64 code64, KbInt - OSCode    ;33 - keyboard
;	TRAP_GATE64 code64, intr - OSCode     ;34
;	TRAP_GATE64 code64, intr - OSCode     ;35
;	TRAP_GATE64 code64, intr - OSCode     ;36
;	TRAP_GATE64 code64, intr - OSCode     ;37
;	TRAP_GATE64 code64, intr - OSCode     ;38
;	TRAP_GATE64 code64, intr - OSCode     ;39
;	TRAP_GATE64 code64, intr - OSCode     ;40
;	TRAP_GATE64 code64, intr - OSCode     ;41
;	TRAP_GATE64 code64, intr - OSCode     ;42
;	TRAP_GATE64 code64, intr - OSCode     ;43
;	TRAP_GATE64 code64, intr - OSCode     ;44
;	TRAP_GATE64 code64, intr - OSCode     ;45
;	TRAP_GATE64 code64, HdInt - OSCode    ;46 - hard disk
;	TRAP_GATE64 code64, intr - OSCode     ;47

	org OSData

