.global ZoomImage

.code 32


ZoomImage:

	;# Sichern der benoetigten Register
	PUSH {r4-r10, lr}

	;# Change Position of Input data
	mov r4, r3
	mov r3, r2
	mov r2, r1

	ldr r1, =Array		;# Load Array for new Image

	
	mov r5, #0		;# Loop Counter

pixelLoopX:

	mov r6, #0		;# Loop Counter

pixelLoopY:
	
		;# Calc 1: pixelX*zoomFactor
		mul r7, r5, r4

		;# Calc 2: pixelY*zoomFactor*zoomFactor*windowWidth
		mul r8, r6, r4
		mul r8, r8, r4
		mul r8, r8, r2

		;# Add Up and Mul by 3 to get Pixel Coordinate: 
		;# (pixelX*zoomFactor + pixelY*zoomFactor*zoomFactor*windowWidth) * 3
		add r7, r7, r8
		mov r8, #3
		mul r7, r7, r8		;# r7 is now the PixelIndex of the new Image
		

		;# Calc3: (pixelX + pixelY*windowWidth) * 3
		mul r8, r6, r2
		add r8, r8, r5
		mov r9, #3
		mul r8, r8, r9		;# r8 is now the Index of the pixel on the old Image


		# Load & Store Colors
		ldr r9, [r0, r8]
		str r9, [r1, r7]

		add r8, r8, #1
		add r7, r7, #1
		ldr r9, [r0, r8]
		str r9, [r1, r7]
		
		add r8, r8, #1
		add r7, r7, #1
		ldr r9, [r0, r8]
		str r9, [r1, r7]


		;# pixelLoopY
		add r6, r6, #1
		cmp r6, r3
		blt pixelLoopY

	;# pixelLoopX
	add r5, r5, #1
	cmp r5, r2
	blt pixelLoopX

end:
	mov r0, r1


	;# Stack wiederherstellen
	POP {r4-r10, lr}
	BX lr



;# Array
.data
.balign 4
Array: .skip 12000000	;# Enough for 2000x2000

addr_of_Array: .word Array
