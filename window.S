.global window
.extern malloc

.code 32
 
window:

	;# Sichern der benoetigten Register
	PUSH {r4-r11, lr}

	;# Laden der restlichen beiden Uebergabewerte vom Stack
	ldr r4, [sp, #36]
	ldr r5, [sp, #40]
	
	mul r12, r5, r2	;# ogWidth * yPos
	add r12, r12, r1	;# (ogWidth * yPos) + xPos
	mov r11, #3			;# r11 not yet in use
	mul r12, r12, r11	;# ((ogWidth * yPos) + xPos) * 3
	add r0, r0, r12	;# we now no longer need xPos, yPos -> more registers
	
	;# Malloc for new Image
	push {r0-r3}
	
	;# Groesse: width * height * 3 (bytes)
	mul r0, r3, r4
	mul r0, r0, r11	;# 3 von vorhin
	mov r11, r0			;# save size in r11
		
	bl malloc
	mov r10, r0		;# move new pointer to r10
	pop {r0-r3}
	
	;# last pointer of new array
	add r11, r11, r10
	
	mov r12, #3
	
	;# LoopY
	mov r1, r10
	mul r3, r3, r12
	mov r7, r10
	mov r2, r0
	mul r4, r5, r12 ;# multiply width by #3 into r4 (Don't need height anymore
	schleifeY:
		
		;# LoopX
		mov r9, #0
		mov r2, r0
		add r0, r0, r4
		mov r1, r7
		add r7, r7, r3
		schleifeX:
			
			add r1, r1, #24
			cmp r1, r11
			sub r1, r1, #24
			bge SISD
			
			SIMD:
			;# Copy colors
			vld3.8 {d8, d9, d10}, [r2]!
			vst3.8 {d8, d9, d10}, [r1]!
			b continue
			
			SISD:
				ldrb r6, [r2], #1
				strb r6, [r1], #1
				
				ldrb r6, [r2], #1
				strb r6, [r1], #1
				
				ldrb r6, [r2], #1
				strb r6, [r1], #1
				
				cmp r1, r11
				blt SISD
			b continue

			continue:

			;# loopX
			add r9, r9, #24
			cmp r9, r3
			blt schleifeX

		;# loopY
		cmp r1, r11
		blt schleifeY

	mov r0, r10 ;# return

	;# Stack wiederherstellen
	POP {r4-r11, lr}
	BX lr
