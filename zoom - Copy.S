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
	mul r2, r2, r4	;# Scale width with scale-factor
	mul r3, r3, r4	;# Scale height with scale-factor
	
	mov r6, #0		;# Loop Counter Y
	fillLoopY:

		mov r5, #0		;# Loop Counter  X
		fillLoopX:

			;# if fillLoopX + 4 > width (scaled) 
			add r5, r5, #4
			cmp r5, r2
			sub r5, r5, #4
			bge SISD_FILL
		
			;# Move X Coordinate to Q1 (fillLoopX)
			vmov s4, r5
			add r5, r5, #1
			sdiv r12, r5, r4
			vmov s16, r12
			vmov s5, r5
			add r5, r5, #1
			sdiv r12, r5, r4
			vmov s17, r12
			vmov s6, r5
			add r5, r5, #1
			sdiv r12, r5, r4
			vmov s18, r12
			vmov s7, r5
			add r5, r5, #1
			sdiv r12, r5, r4
			vmov s19, r12
			sub r5, r5, #4

			;# Move Y Coordinate to Q2 (fillLoopY)
			vdup.32 q2, r6
			add r6, r6, #1
			sdiv r12, r6, r4
			mul r12, r12, r4
			vdup.32 q5, r12				;# (Y-Coord+1/Zoom)*Zoom
			sub r6, r6, #1

			vdup.32 q3, r4				;# ZoomFactor
			vmul.u32 q4, q4, q3			;# (X-Coord+1/Zoom)*Zoom
			;#vmul.u32 q5,q5, q4
			
;#			add r12, r5, #1
;#			sdiv r7, r12, r4
;#			mul r7, r7, r4
;#			;# parentY coord
;#			add r12, r6, #1
;#			sdiv r8, r12, r4
;#			mul r8, r8, r4

			;# Check Out Of Bounds
			Compare:
			vdup.32 q6, r2
			vceq.u32 q7, q4, q6		;# Fills q7 with 0 or 1
			vand q6, q3, q7
			vsub.u32 q4, q4, q6

			vdup.32 q6, r3
			vceq.u32 q7, q5, q6		;# Fills q7 with 0 or 1
			vand q6, q3, q7
			vsub.u32 q5, q5, q6
			
;#			;# check out of bounds
;#			cmp r7, r2
;#			subeq r7, r7, r4
;#			cmp r8, r3
;#			subeq r8, r8, r4


			;# Map Pixel Onto Array
			mul r9, r6, r2			;# Y-Coord*Width
			vdup.32 q6, r9
			vadd.u32 q6, q6, q1		;# (Y-Coord*Width) + X-Coord
			mov r12, #3
			vdup.32 q7, r12
			vmul.u32 q6, q6, q7		;# ((Y-Coord*Width) + X-Coord)*3

;#			;# map pixel onto array
;#			mul r9, r6, r2
;#			add r9, r9, r5
;#			mov r12, #3
;#			mul r9, r9, r12


			;# Map Parent Onto Array
			vdup.32 q8, r2
			vmul.u32 q5, q5, q8
			vadd.u32 q5, q5, q4
			vmul.u32 q5, q5, q7		;# ((Y-Map*Width)+XMap)*3

;#			;# map parent onto array
;#			mul r8, r8, r2
;#			add r8, r8, r7
;#			mul r8, r8, r12

		
			mov r12, #0 ;# loop counter 
       
			vmov.u32 r7, d12[0]		;# load index in new Image
			vmov.u32 r8, d10[0]		;# load index in old Image
			
			SISD_LS:  ;# Load and Store Colors 
			         
				ldrb r9, [r1, r8] 
				strb r9, [r1, r7] 
				add r8, r8, #1 
				add r7, r7, #1 
				ldrb r9, [r1, r8] 
				strb r9, [r1, r7] 
				add r8, r8, #1 
				add r7, r7, #1 
				ldrb r9, [r1, r8] 
				strb r9, [r1, r7] 
         
				;# loop 4 times 
				add r12,  r12, #1 
				cmp r12, #2 
				blt load1 
				beq load2 
				bge load3 
         
				load1: 
					vmov.u32 r7, d12[1] 
					vmov.u32 r8, d10[1] 
					b SISD_LS      ;# jump without condition, because eq & lt 2 ist <4 
				load2: 
					vmov.u32 r7, d13[0] 
					vmov.u32 r8, d11[0] 
					b SISD_LS      ;# jump without condition, because eq & lt 2 ist <4 
				load3: 
					cmp r12, #4 
					bge exitLoop 
					vmov.u32 r7, d13[1] 
					vmov.u32 r8, d11[1] 
					b SISD_LS 

			SISD_FILL:
			
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
			mul r9, r6, r2
			add r9, r9, r5
			mov r12, #3
			mul r9, r9, r12

			;# map parent onto array
			mul r8, r8, r2
			add r8, r8, r7
			mul r8, r8, r12

			ldrb r12, [r1, r8] 
			strb r12, [r1, r9] 
			add r8, r8, #1 
			add r9, r9, #1 
			ldrb r12, [r1, r8] 
			strb r12, [r1, r9] 
			add r8, r8, #1 
			add r9, r9, #1 
			ldrb r12, [r1, r8] 
			strb r12, [r1, r9] 

			add r5, r5, #1
			cmp r5, r2
			blt fillLoopX

			exitLoop:
			
			;# pixelLoopX
			add r5, r5, #4
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
