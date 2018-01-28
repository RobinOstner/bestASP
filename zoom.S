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


	mov r5, #0		;# Loop Counter X
	pixelLoopX:

		mov r6, #0		;# Loop Counter Y
		pixelLoopY:
		
;# Old code
;#			;# Calc 1: pixelX*zoomFactor
;#			mul r7, r5, r4
;#
;#			;# Calc 2: pixelY*zoomFactor*zoomFactor*windowWidth
;#			mul r8, r6, r4
;#			mul r8, r8, r4
;#			mul r8, r8, r2
;#
;#			;# Add Up and Mul by 3 to get Pixel Coordinate: 
;#			;# (pixelX*zoomFactor + pixelY*zoomFactor*zoomFactor*windowWidth) * 3
;#			add r7, r7, r8
;#			mov r8, #3
;#			mul r7, r7, r8		;# r7 is now the PixelIndex of the new Image
;#			
;#
;#			;# Calc3: (pixelX + pixelY*windowWidth) * 3
;#			mul r8, r6, r2
;#			add r8, r8, r5
;#			mov r9, #3
;#			mul r8, r8, r9		;# r8 is now the Index of the pixel on the old Image

			;# SIMD
			vdup.32 q1, r4			;# zoomFactor
			vdup.32 q2, r5			;# pixelX
			
			vdup.32 q3, r6			;# pixelY
			mov r12, #1
			vmov s1, r12
			mov r12, #2
			vmov s2, r12
			mov r12, #3
			vmov s3, r12
			vadd.u32 q3, q3, q0	;# increment each
			
			vdup.32 q4, r2			;# windowWidth
			vmul.u32 q6, q3, q1
			vmul.u32 q6, q6, q4	;# pixelY*zoomFactor*windowWidth
			
			vadd.u32 q6, q6, q2
			vmul.u32 q6, q6, q1	;# * zoomFactor
			mov r12, #3
			vdup.32 q5, r12			;# 3
			;# q6: pixel index of new image
			vmul.u32 q6, q6, q5	;# (pixelX + pixelY*zoomFactor*windowWidth) * 3 * zoomFactor
			
			vmul.u32 q7, q3, q4	;# pixelY*windowWidth
			vadd.u32 q7, q7, q2	;# pixelX + pixelY*windowWidth
			;# q7: pixel index of new image
			vmul.u32 q7, q7, q5	;# (pixelX + pixelY*windowWidth) * 3
			

			mov r12, #0 ;# loop counter
			
			vmov.u32 r7, d12[0]	;# load first indices
			vmov.u32 r8, d14[0] 	;# -"-
			SISD_LS:	;# Load and Store Colors
				cmp  r7, r11
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
				add r12,  r12, #1
				cmp r12, #2
				blt load1
				beq load2
				bge load3
				
				load1:
					vmov.u32 r7, d12[1]
					vmov.u32 r8, d14[1]
					b SISD_LS			;# jump without condition, because eq & lt 2 ist <4
				load2:
					vmov.u32 r7, d13[0]
					vmov.u32 r8, d15[0]
					b SISD_LS			;# jump without condition, because eq & lt 2 ist <4
				load3:
					cmp r12, #4
					bge exitLoop
					vmov.u32 r7, d13[1]
					vmov.u32 r8, d15[1]
					b SISD_LS
			
			;# pixelLoopY
			add r6, r6, #4
			cmp r6, r3
			blt pixelLoopY

		;# pixelLoopX
		add r5, r5, #1
		cmp r5, r2
		blt pixelLoopX

	exitLoop:
	
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
