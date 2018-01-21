.global windowImage

.code 32

windowImage:

	;# Sichern der benoetigten Register
	PUSH {r4-r10, lr}

	;# Laden der restlichen beiden Uebergabewerte vom Stack
	ldr r4, [sp, #32]
	ldr r5, [sp, #36]
	
	;# Copy to s Registers
	vmov s0, r1  ;# xPosition
	vmov s1, r2  ;# yPosition
	vmov s2, r3  ;# Width
	vmov s3, r4  ;# Height
	vmov s4, r5  ;# Original Width

	vcvt.f32.u32 s0, s0
	vcvt.f32.u32 s1, s1
	vcvt.f32.u32 s2, s2
	vcvt.f32.u32 s3, s3
	vcvt.f32.u32 s4, s4

	;# Image Data Pointer Copy
	mov r1, r0

	mov r5, #0
	vmov s5, r5

schleifeX:
	
	mov r5, #0
	vmov s6, r5

	schleifeY:

		;# Calculate Index
		vmul.f32 s7, s1, s4
		vadd.f32 s7, s7, s0
		mov r5, #3
		vmov s9, r5
		vmul.f32 s7, s7, s9

		vmul.f32 s8, s6, s4
		vadd.f32 s8, s8, s5
		vmul.f32 s8, s8, s9

		vadd.f32 s7, s7, s8

		;# Index
		vmov r2, s7

		;# Move Pointer to correct position
		add r0, r0, r2

		;# Copy Blue Value
		ldr r3, [r0]
		add r0, r0, #1

		;# Copy Green Value
		ldr r4, [r0]
		add r0, r0, #1

		;# Copy Red Value
		ldr r5, [r0]
		

		;# Reset Pointer
		mov r0, r1

		;# Store Blue Value
		str r3, [r0]
		add r0, r0, #1

		;# Store Green Value
		str r4, [r0]
		add r0, r0, #1

		;# Store Red Value
		str r5, [r0]
		add r0, r0, #1

		;# Copy Image Data Pointer
		mov r1, r0

		;# Loop
		mov r5, #1
		vmov s9, r5
		vcvt.f32.u32 s9, s9
		vadd.f32 s6, s6, s9
		vcmp.f32 s6, s3
		blt schleifeY


	vadd.f32 s5, s5, s9
	vcmp.f32 s5, s2
	blt schleifeX



	;# Stack wiederherstellen
	POP {r4-r10, lr}
	BX lr
