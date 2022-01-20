.model small
.stack 100h
.386

println macro string
	push ax dx
	mov ah, 09h
	lea dx, string
	int 21h
	mov ah, 09h
	lea dx, newline
	int 21h
	pop dx ax
endm

print macro string
	push ax dx
	mov ah, 09h
	lea dx, string
	int 21h
	pop dx ax
endm

readnum macro
	LOCAL convert_loop, inner_convert, jump_skip_inner_convert
	push dx cx bx si di
	mov ah, 0Ah
	lea dx, inputBuffer
	int 21h
	lea bx, inputBuffer + 2
	mov di, bx
	mov ax, 0Dh
	repne scasb
	mov cx, di
	sub cx, bx 
	dec cx
	xor ax, ax
	xor dx, dx 
	xor si, si
	convert_loop:
	push ax
	mov ax, [bx]
	xor ah, ah
	sub ax, 48
	push cx
	dec cx
	cmp cx, 1
	jl jump_skip_inner_convert
	inner_convert:
	mul [numBase]
	loop inner_convert
	jump_skip_inner_convert:
	pop cx
	mov dx, ax
	pop ax
	add ax, dx     
	inc bx
	loop convert_loop
	pop di si bx cx dx
endm

printnum macro
	LOCAL jump_resolve_score_loop, jump_exit_resolve_score_loop, clear_loop
	push ax bx dx cx

	push ax
	mov ax, '0'
	lea di, number_print
	mov cx, 5
	cld
	rep stosb
	pop ax

	xor dx, dx
    mov cx, 4
    jump_resolve_score_loop:
    xor dx, dx  
    div [numBase]
    add dx, 48
    lea bx, number_print
    add bx, cx
    mov [bx],dl 
    cmp cx, 0
    je jump_exit_resolve_score_loop
    dec cx
    cmp ax, 0
    je jump_exit_resolve_score_loop
    xor dx, dx             
    jmp jump_resolve_score_loop
    jump_exit_resolve_score_loop:
    
    mov ah, 09h
    lea dx, number_print + 3
    int 21h
    pop cx dx bx ax
endm

.data
	newline db 0Dh, 0Ah, "$"
	setTimeOption db "1. Set time$"
	getTimeOption db "2. Get time$"
	delayOption db "3. Delay$"
	userInput db "Select: $"
	badInputError db "Error: bad input$"
	unableToSetError db "Error: unable to set time$"
	inputBuffer db 20, ?, 20 dup(?)
	numBase dw 10
	currentTimeMsg db "Curernt time: $"
	number_print db 5 dup('0'), "$"

	msMsg db "ms: $"
	delayTime dw ?
	delayEndMsg db "Delay ended$"

	time_separator db ":$"

	hourMsg db "hour: $"
	minuteMsg db "minute: $"
	secondMsg db "second: $"

	current_hour db ?
	current_minute db ?
	current_second db ?

	new_hour db ?
	new_minute db ?
	new_second db ?

.code
start proc
	mov ax, @data
	mov ds, ax
	mov es, ax
	call open_menu
	call exit_null
start endp

open_menu proc
	println setTimeOption
	println getTimeOption
	println delayOption
	print userInput
	readnum
	print newline


	call menu_controller
	ret
open_menu endp

menu_controller proc
	cmp ax, 1
	jne check_get_time
	call set_time
	call exit_null
	check_get_time:
	cmp ax, 2
	jne check_delay
	call get_time
	call exit_null
	check_delay:
	cmp ax, 3
	jne jump_exit_wrong_option
	call delay
	call exit_null
	jump_exit_wrong_option:
	println badInputError
	ret
menu_controller endp

int70h_handler proc far
	pushf
    push ds es di bx 
    
    mov bx, @data
    mov ds, bx

    ; mov ah, 02h
    ; mov dx, 'a'
    ; int 21h

    mov al, 0Ch
    out 70h, al
    jmp $+2
    in al, 71h
    test al, 10000000b
    jz jump_exit_handler
    test al, 1000000b
    jz jump_exit_handler
    
    cmp [delayTime], 0
    je jump_exit_handler

    dec [delayTime]

    

    jump_exit_handler:

    mov al, 20h
	out 20h, al   

    
	pop bx di es ds
	popf
	;iret
	     
	db 0eah
	old_ofs_70h dw ?
	old_seg_70h dw ?
int70h_handler endp

delay proc
	print msMsg
	readnum
	mov [delayTime], ax
	print newline

	mov ah, 35h
	mov al, 70h
	int 21h
	mov [old_ofs_70h], bx
	mov [old_seg_70h], es
	
	push ds
	mov ah, 25h
	mov al, 70h
	mov dx, seg int70h_handler
	mov ds, dx
	mov dx, offset int70h_handler
	int 21h
	pop ds

	; Регистр состояния B
	; 7 - запрещение обновления часов
	; 6 - переодическое прерывание IRQ8
	; 5 - перерывания от будильника BIOS INT 4Ah
	; 4 - вызов прерывания после цикла обновления
	; 3 - генерация прямоугольных импульсов
	; 2 - формат представления даты и времени bin/BCD
	; 1 - часовой режим 24/12
	; 0 - переход на летнее время

	; включить режим генерации переодического прерывания
	mov al, 0Bh
	out 70h, al
	jmp $+2
	in al, 71h
	mov ah, 1
	shl al, 2
	shr ax, 2
	out 71h, al

	wait_for_delay:
	; mov cx,0
	; mov dx,1
	; mov ah,86h
	; int 15h
	; dec [delayTime]

	cmp [delayTime], 0
	jne wait_for_delay



	push ds
	mov ah, 25h
	mov al, 70h
	mov dx, old_seg_70h
	mov ds, dx
	mov dx, old_ofs_70h
	int 21h
	pop ds

	; выключить режим генерации переодического прерывания
	mov al, 0Bh
	out 70h, al
	jmp $+2
	in al, 71h
	shl al, 2
	shr al, 2
	out 71h, al

	println delayEndMsg
	ret
delay endp

set_time proc

	print hourMsg
	readnum
	print newline
	cmp ax, 24
	ja jump_exit_unable_to_set
	xor dx, dx
	div [numBase]
	shl dl, 4
	mov ah, al
	mov al, dl
	shr ax, 4
	mov [new_hour], al
	

	print minuteMsg
	readnum
	print newline
	cmp ax, 59
	ja jump_exit_unable_to_set
	xor dx, dx
	div [numBase]
	shl dl, 4
	mov ah, al
	mov al, dl
	shr ax, 4
	mov [new_minute], al
	

	print secondMsg
	readnum
	print newline
	cmp ax, 59
	ja jump_exit_unable_to_set
	xor dx, dx
	div [numBase]
	shl dl, 4
	mov ah, al
	mov al, dl
	shr ax, 4
	mov [new_second], al

	; Регистр состояния A
	; 7 - обновление времени
	; 6 - делитель частоты
	; 5 - делитель частоты
	; 4 - делитель частоты
	; 3 - значение пересчета частоты
	; 2 - значение пересчета частоты
	; 1 - значение пересчета частоты
	; 0 - значение пересчета частоты

	; преверка часов на возможность обновления времени
	mov al, 0Ah
	out 70h, al
	jmp $+2
	in al, 71h
	test al, 10000000b
	jnz jump_exit_unable_to_set

	; остановка часов
	mov al, 0Bh
	out 70h, al
	jmp $+2
	in al, 71h
	xor ah, ah
	not ah
	shl al, 1
	shl ax, 7
	mov al, ah
	out 71h, al

	; Область CMOS
	; 00h - текущая секунда
	; 01h - значение секунд будильника
	; 02h - текущая минута
	; 03h - занчение минут будильника
	; 04h - текущий час
	; 05h - значение часа будильника
	; 06h - текущий день недели
	; 09h - текущий год

	; установить текущее значение времени в формате BCD 
	mov ah, 03h
	mov ch, [new_hour]
	mov cl, [new_minute]
	mov dh, [new_second]
	mov dl, 0
	int 1Ah
	;jc jump_exit_unable_to_set

	; нормальный ход
	mov al, 0Bh
	out 70h, al
	jmp $+2
	in al, 71h
	shl al, 1
	shr al, 1
	out 71h, al

	jmp jump_exit_set_time
	jump_exit_unable_to_set:
	println unableToSetError
	jump_exit_set_time:

	call get_time
	ret
set_time endp

get_time proc
	print currentTimeMsg

	; Регистр состояния C
	; 7 - признак выполненного прерывания
	; 6 - переодическое прерывание
	; 5 - будильник
	; 4 - прерывание после обновления часов
	; 3 - зарезервированно, должно быть равно 0
	; 2 - зарезервированно, должно быть равно 0
	; 1 - зарезервированно, должно быть равно 0
	; 0 - зарезервированно, должно быть равно 0

	; получить текущее значение времени в формате BCD
	mov ah, 02h
	int 1Ah
	mov [current_hour], ch
	mov [current_minute], cl
	mov [current_second], dh

	mov ch, [current_hour]
	xor ax, ax
	mov al, ch
	shr al, 4
	mul [numBase]
	mov ch, [current_hour]
	shl ch, 4
	shr ch, 4
	xor bx, bx
	mov bl, ch
	add ax, bx 
	printnum

	print time_separator

	mov cl, [current_minute]
	xor ax, ax
	mov al, cl
	shr al, 4
	mul [numBase]
	mov cl, [current_minute]
	shl cl, 4
	shr cl, 4
	xor bx, bx
	mov bl, cl
	add ax, bx 
	printnum

	print time_separator

	mov dh, [current_second]
	xor ax, ax
	mov al, dh
	shr al, 4
	mul [numBase]
	mov dh, [current_second]
	shl dh, 4
	shr dh, 4
	xor bx, bx
	mov bl, dh
	add ax, bx 
	printnum
    

	ret
get_time endp

exit_null proc
	mov ah, 4Ch
	int 21h
exit_null endp

end start