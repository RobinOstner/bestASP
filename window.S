.global window
.extern malloc

.code 32

window:

	;# Sichern der benoetigten Register
	PUSH {r4-r10, lr}

	;# Laden der restlichen beiden Uebergabewerte vom Stack
	ldr r4, [sp, #32]
	ldr r5, [sp, #36]
	
	;# Copy to s Registers
	vmov s0, r1	;# xPosition
	vmov s1, r2	;# yPosition
	vmov s2, r3	;# Width S
	mov r6, r3	;# Width
	vmov s3, r4	;# Height S
	mov r7, r4	;# Height
	vmov s4, r5	;# Original Width

	;# Convert To Float
	vcvt.f32.u32 s0, s0
	vcvt.f32.u32 s1, s1
	vcvt.f32.u32 s2, s2
	vcvt.f32.u32 s3, s3
	vcvt.f32.u32 s4, s4
	
	;# Malloc for new Image
	
	push {r0-r3}
	
	;# Groesse: width * height * 3 (bytes)
	mul r0, r6, r7
	mov r12, #3		;# immediate zwischenspeichern
	mul r0, r0, r12
	mov r4, r0
		
	bl malloc
	mov r10, r0		;# move new pointer to r1
	pop {r0-r3}
	
	;# last pointer of new array
	add r4, r4, r10

	;# LoopY
	mov r8, #0
	schleifeY:
		
		;# LoopX
		mov r9, #0
		schleifeX:

			;# Calculate Index 

			;# yPos*originalWidth + xPos
			vmul.f32 s7, s1, s4
			vadd.f32 s7, s7, s0
			
			;# (yPos*originalWidth + xPos)*3
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

			;# (yPos*originalWidth + xPos)*3 + (loopX + originalWidth*loopY)*3
			vadd.f32 s7, s7, s8

			;# Index Old Image
			vcvt.u32.f32 s7, s7
			vmov r2, s7

			;# r2 is the index of the current Pixel

			;# index new image
			mul r12, r8, r6
			add r12, r12, r9
			mul r12, r12, r5	;# die #3 von vorhin
			add r1, r10, r12
			
			add r1, r1, #24
			cmp r1, r4
			sub r1, r1, #24
			bge SISD
			
			SIMD:
			;# Copy colors
			add r0, r0, r2
			vld3.8 {d8, d9, d10}, [r0]
			vst3.8 {d8, d9, d10}, [r1]
			sub r0, r0, r2
			b continue
			
			SISD:
			add r0, r0, r2 ;# r0 wird danach nicht mehr gebraucht
			SISD_INNER:
				ldrb r5, [r0], #1
				strb r5, [r1], #1
				
				ldrb r5, [r0], #1
				strb r5, [r1], #1
				
				ldrb r5, [r0], #1
				strb r5, [r1], #1
				
				cmp r1, r4
				blt SISD_INNER
			b continue

			continue:

			;# loopX
			add r9, r9, #8
			cmp r9, r6
			blt schleifeX

		;# loopY
		add r8, r8, #1
		cmp r8, r7
		blt schleifeY


	mov r0, r10 ;# return

	;# Stack wiederherstellen
	POP {r4-r10, lr}
	BX lr
