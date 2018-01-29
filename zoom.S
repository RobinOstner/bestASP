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
	
	push {r1-r3}	
	bl malloc
	pop {r1-r3}
	mov r1, r0		;# move new pointer to r1
	
	mov r0, r10		;# undo zwischenspeichern
	
	mul r11, r2, r3
	mov r12, #3
	mul r11, r11, r12	;# Size of old pic in bytes
	add r11, r11, r0	;# to pointer
	
	add r6, r1, #0
	add r5, r0, #0		;# Loop Counter
	mul r7, r4, r12
	mul r12, r7, r4
	mul r12, r12, r2
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
		add r8, r8, #1
		cmp r8, r2
		addlt r6, r6, r7
		addge r6, r6, r12
		movge r8, #0
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
