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
	vmov s2, r3 ;# Width S
	mov r6, r3  ;# Width
	vmov s3, r4 ;# Height S
	mov r7, r4  ;# Height
	vmov s4, r5  ;# Original Width

	;# Convert To Float
	vcvt.f32.u32 s0, s0
	vcvt.f32.u32 s1, s1
	vcvt.f32.u32 s2, s2
	vcvt.f32.u32 s3, s3
	vcvt.f32.u32 s4, s4

	;# Free Space Index
	mov r1, #0


	;# LoopX
	mov r8, #0

schleifeX:
	
	;# LoopY
	mov r9, #0

	schleifeY:

		;# Calculate Index 

		;# yPos*originalWidth + xPos
		vmul.f32 s7, s1, s4
		vadd.f32 s7, s7, s0

		
		;# (ypos*originalWidth + xPos)*3
		mov r5, #3
		vmov s9, r5
		vcvt.f32.u32 s9, s9
		vmul.f32 s7, s7, s9

		;# loopX + originalWidth*loopY
		vmov s6, r8
		vcvt.f32.u32 s6, s6
		vmul.f32 s8, s6, s4

		;# (loopX + originalWidth*loopY)*3
		vmov s5, r9
		vcvt.f32.u32 s5, s5
		vadd.f32 s8, s8, s5
		vmul.f32 s8, s8, s9

		;# (ypos*originalWidth + xPos)*3 + (loopX + originalWidth*loopY)*3
		vadd.f32 s7, s7, s8

		;# Index
		vcvt.u32.f32 s7, s7
		vmov r2, s7

		;# r2 is the index of the current Pixel

		;# Copy Blue Value
		ldr r3, [r0, r2]

		add r2, r2, #1		;# r2 is next Color Value of current Pixel

		;# Copy Green Value
		ldr r4, [r0, r2]

		add r2, r2, #1		;# r2 is next Color Value of current Pixel

		;# Copy Red Value
		ldr r5, [r0, r2]

		;# Store Blue Value
		str r3, [r0, r1]

		add r1, r1, #1		;# r1 is next free space

		;# Store Green Value
		str r4, [r0, r1]

		add r1, r1, #1		;# r1 is next free space

		;# Store Red Value
		str r5, [r0, r1]
		
		add r1, r1, #1		;# r1 is next free space
		
		;# loopY
		add r9, r9, #1
		cmp r9, r7
		blt schleifeY

	;# loopX
	add r8, r8, #1
	cmp r8, r6
	blt schleifeX



	;# Stack wiederherstellen
	POP {r4-r10, lr}
	BX lr
