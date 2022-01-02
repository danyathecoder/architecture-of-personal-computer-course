.model tiny   
.code
org  100h 
jmp start

NO_WRITE proc 
    mov ah,9
    mov dx,offset message1 
    int 21h
    ret
    message1 db "Somethig is wrong$"
NO_WRITE endp 
 
sended proc 
    mov ah,9 
    mov dx,offset message2
    int 21h
    ret
    message2 db "all is ok$"
sended endp

start:     
    xor ax,ax
    mov al, 10100011b ;3 - бод, 2 - код ч/н, стоп-бит, 2 - длинна  
    mov dx, 00h ;номер порта
    int 14h 
    mov al, 03
    mov dx,00h 
    mov ah,01h
    int 14h
    test al, 80h
    jz ok
    call NO_WRITE
    ret
ok: call sended
    ret
end start 