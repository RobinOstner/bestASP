.global zoom
.extern malloc

.code 32


zoom:

	;# Sichern der benoetigten Register
	PUSH {r4-r11, lr}

	;# Change Position of Input data
	mov r4, r3
	mov r3, r2
	mov r2, r1

	;# Malloc for new Image
	mov r10, r0		;# r0 in r10 zwischenspeichern
	
	;# Groesse: width * height * (scale*scale) * 3 (bytes)
	mul r0, r2, r3
	mul r0, r0, r4
	mul r0, r0, r4
	mov r12, #3		;# immediate zwischenspeichern
	mul r0, r0, r12
	
	push {r1-r3}	
	bl malloc
	pop {r1-r3}
	mov r1, r0		;# move new pointer to r1
	
	mov r0, r10		;# undo zwischenspeichern
	
	mul r11, r2, r3
	mov r12, #3
	mul r11, r11, r12	;# Size of old pic in bytes
	add r11, r11, r0	;# to pointer
	
	mov r12, #3
	mul r7, r4, r12
	mul r12, r7, r2
	sub r5, r4, #1
	mul r12, r12, r5
	
	mov r6, r1
	mov r5, r0			;# Loop Counter
	mov r8, #0
	loop:
	
		ldrb r9, [r5]
		strb r9, [r6]
		
		ldrb r9, [r5, #1]
		strb r9, [r6, #1]
		
		ldrb r9, [r5, #2]
		strb r9, [r6, #2]
		
		;# loop
		add r5, r5, #3
		add r6, r6, r7
		add r8, r8, #1
		cmp r8, r2
		addge r6, r6, r12
		movge r8, #0
		cmp r5, r11
		blt loop
		
	;# FILL 
	mul r2, r2, r4		;# Scale width with scale-factor
	mul r3, r3, r4		;# Scale height with scale-factor
	
	;# set constants for NEON
	vdup.32 q3, r4		;# q3 = zoomFactor
	vdup.32 q8, r2		;# q8 = width
	vdup.32 q9, r3		;# q9 = height
	mov r12, #3	
	vdup.32 q10, r12 	;# q10 = 3
	
	mov r6, #0			;# Loop Counter Y
	fillLoopY:

		mov r5, #0		;# Loop Counter  X
		fillLoopX:

			;# Fill q1 with (fillLoopX+1)/zoomFactor
			;# Also fill q6 with fillLoop
			;# Part 1
			vmov.u32 s24, r5
			
			add r5, r5, #1
			udiv r12, r5, r4
			vmov.u32 s4, r12
			
			;# Part 2
			vmov.u32 s25, r5
			
			add r5, r5, #1
			udiv r12, r5, r4
			vmov.u32 s5, r12
			
			;# Part 3
			vmov.u32 s26, r5
			
			add r5, r5, #1
			udiv r12, r5, r4
			vmov.u32 s6, r12
			
			;# Part 4
			vmov.u32 s27, r5
			
			add r5, r5, #1
			udiv r12, r5, r4
			vmov.u32 s7, r12
			
			vmul.u32 q1, q1, q3			;# q1 *= zoomFactor
			
			
			vdup.32 q7, r6				;# q7 = fillLoopY
			
			add r12, r6, #1
			udiv r12, r12, r4
			mul r12, r12, r4			;# calc before dup, to save calculation time (minimal)
			vdup.32 q2, r12				;# q2 = ((fillLoopY+1)/zoomFactor)*zoomFactor
			
			
			;# check out of bounds (le, because eq will be handled seperately)
			vcle.u32 q13, q6, q8
			
			
			;# Edge handling
			;# X
			vceq.u32 q14, q1, q8
			vand q14, q14, q3		;# zoomFactor, where necessary
			vsub.u32 q1, q1, q14
			
			;# Y
			vceq.u32 q14, q2, q9
			vand q14, q14, q3		;# zoomFactor, where necessary
			vsub.u32 q2, q2, q14
			
			
			;# currentIndex
			vmul.u32 q5, q7, q8
			vadd.u32 q5, q5, q6
			vmul.u32 q5, q5, q10
			vand q5, q5, q13		;# set to 0 if out of bounds
			
			;# parentIndex
			vmul.u32 q4, q2, q8
			vadd.u32 q4, q4, q1
			vmul.u32 q4, q4, q10
			vand q4, q4, q13		;# set to 0 if out of bounds
		
		
			mov r12, #0 ;# loop counter 
       
			vmov.u32 r8, s16		;# load index in old Image
			vmov.u32 r7, s20		;# load index in new Image
			
			SISD_LS:  ;# Load and Store Colors 
				;# to pointer
			    add r8, r8, r1
			    add r7, r7, r1
				
				ldrb r9, [r8] 
				strb r9, [r7] 
				ldrb r9, [r8, #1] 
				strb r9, [r7, #1]
				ldrb r9, [r8, #2] 
				strb r9, [r7, #2] 
         
				;# loop 4 times 
				add r12,  r12, #1 
				cmp r12, #2 
				blt load1 
				beq load2 
				bge load3 
         
				load1: 
					vmov.u32 r8, s17 
					vmov.u32 r7, s21
					b SISD_LS      ;# jump without condition, because eq & lt 2 ist <4 
				load2: 
					vmov.u32 r8, s18 
					vmov.u32 r7, s22
					b SISD_LS      ;# jump without condition, because eq & lt 2 ist <4 
				load3: 
					cmp r12, #4 
					bge exitSISD_LS 
					vmov.u32 r8, s19
					vmov.u32 r7, s23
					b SISD_LS 
					
			exitSISD_LS:
			
			;# pixelLoopX
			;# r5 is already incremented
			cmp r5, r2
			blt fillLoopX

		;# pixelLoopY
		add r6, r6, #1
		cmp r6, r3
		blt fillLoopY
	
	mov r0, r1

	;# Stack wiederherstellen
	POP {r4-r11, lr}
	BX lr
