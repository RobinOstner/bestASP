.global ZoomImage

.code 32


ZoomImage:

	;# Sichern der benoetigten Register
	PUSH {r4-r10, lr}

test:	

	ldr r1, =addr_of_Array
	
	mov r2, #15
	str r2, [r1]

	ldr r3, [r1]


	;# Stack wiederherstellen
	POP {r4-r10, lr}
	BX lr



;# Array
.data
.balign 4
Array: .skip 8


addr_of_Array: .word Array
