.global zoom
.extern malloc

.code 32


zoom:

	;# Sichern der benoetigten Register
	PUSH {r4-r11, lr}

	;# Change Position of Input data
	mov r4, r3
	mov r3, r1

	;# Malloc for new Image
	mov r10, r0		;# r0 in r10 zwischenspeichern
	
	;# Groesse: width * height * (scale*scale) * 3 (bytes)
	mul r0, r2, r3
	mul r0, r0, r4
	mul r0, r0, r4
	mov r12, #3		;# immediate zwischenspeichern
	mul r0, r0, r12
	mov r11, r0		;# save size
	
	push {r1-r3}	
	bl malloc
	pop {r1-r3}
	mov r1, r0		;# move new pointer to r1
	
	mov r0, r10		;# undo zwischenspeichern
	
	mov r9, #3
	mul r9, r9, r4
	mov r12, #0
	vmov s4, r12
	add r12, r12, r9
	vmov s5, r12
	add r12, r12, r9
	vmov s6, r12
	add r12, r12, r9
	vmov s7, r12

	mov r12, #0
	vmov s8, r12
	mov r12, #3
	vmov s9, r12
	mov r12, #6
	vmov s10, r12
	mov r12, #9
	vmov s11, r12
	
	;# q3
	mov r12, #4
	mul r12, r12, r9
	vdup.32 q3, r12
	
	mov r12, #12
	vdup.32 q4, r12

	mov r5, #0		;# Loop Counter
	loop:
	
		mov r12, #0 ;# loop counter
		
		vmov.u32 r7, d2[0]	;# load first indices
		vmov.u32 r8, d4[0] 	;# -"-
		SISD_LS:	;# Load and Store Colors
			;#cmp  r7, r11
			;#bge exitLoop ;# if out of bounds, stop.
			;# TODO (above line not working)
			
			ldrb r9, [r0, r8]
			strb r9, [r1, r7]
			add r8, r8, #1
			add r7, r7, #1
			ldrb r9, [r0, r8]
			strb r9, [r1, r7]
			add r8, r8, #1
			add r7, r7, #1
			ldrb r9, [r0, r8]
			strb r9, [r1, r7]
			
			;# loop 4 times
			add r12, r12, #1
			cmp r12, #2
			blt load1
			beq load2
			bge load3
			
			load1:
				vmov.u32 r7, d2[1]
				vmov.u32 r8, d4[1]
				b SISD_LS			;# jump without condition, because eq & lt 2 ist <4
			load2:
				vmov.u32 r7, d3[0]
				vmov.u32 r8, d5[0]
				b SISD_LS			;# jump without condition, because eq & lt 2 ist <4
			load3:
				cmp r12, #4
				bge exitLS
				vmov.u32 r7, d3[1]
				vmov.u32 r8, d5[1]
				b SISD_LS
		
		exitLS:

		vadd.u32 q1, q1, q3
		vadd.u32 q2, q2, q4
			
		;# loop
		add r5, r5, #12
		cmp r5, r11
		blt loop

	
	;# FILL 
	mul r2, r2, r4	;# Scale width with scale-factor
	mul r3, r3, r4	;# Scale height with scale-factor
	
	mov r5, #0		;# Loop Counter X
	fillLoopX:

		mov r6, #0		;# Loop Counter  Y
		fillLoopY:
		
			;# parentX coord
			add r12, r5, #1
			sdiv r7, r12, r4
			mul r7, r7, r4
			;# parentY coord
			add r12, r6, #1
			sdiv r8, r12, r4
			mul r8, r8, r4
			
			;# check out of bounds
			cmp r7, r2
			subeq r7, r7, r4
			cmp r8, r3
			subeq r8, r8, r4
			
			;# map pixel onto array
			mov r12, #3
			mul r9, r6, r2
			add r9, r9, r5
			mul r9, r9, r12
			
			;# map parent onto array
			mul r8, r8, r2
			add r8, r8, r7
			mul r8, r8, r12
			
			;# Load & Store Colors
			ldrb r10, [r1, r8]
			strb r10, [r1, r9]

			add r8, r8, #1
			add r9, r9, #1
			ldrb r10, [r1, r8]
			strb r10, [r1, r9]
			
			add r8, r8, #1
			add r9, r9, #1
			ldrb r10, [r1, r8]
			strb r10, [r1, r9]
			
			;# pixelLoopY
			add r6, r6, #1
			cmp r6, r3
			blt fillLoopY

		;# pixelLoopX
		add r5, r5, #1
		cmp r5, r2
		blt fillLoopX
	
	end:
		mov r0, r1

		;# Stack wiederherstellen
		POP {r4-r11, lr}
		BX lr
