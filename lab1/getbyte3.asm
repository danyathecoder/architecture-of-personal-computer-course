.model tiny   
.code
org  100h 
jmp start

isallright proc
    xor al,al
    mov dx,3FDh
    in al,dx
    test al,10b
    jnz NO_READ
    ret
isallright endp

NO_READ proc 
    mov ah,9
    mov dx,offset message
    int 21h
    ret
    message db "Somethig is wrong$"
NO_READ endp 

Data_byte db ? 

getbyte proc
    mov dx,3F8h
    in al,dx
    mov Data_byte,al
    ret
getbyte endp 

start:     
    mov ax, 40h
    mov es, ax
    mov dx, es:[02]
    mov COM2_adr, dx
    add dx, 3 
    mov al, 80h 
    out dx, al
    sub dx, 2 
    mov al, 0
    out dx, al
    dec dx 
    mov al, 0Ch 
    out dx, al
    add dx, 3 
    mov al, 00011011b 
    out dx, al
    mov dx, COM2_adr
    add dx, 5
    call isallright
    call getbyte
    mov ah,02h
    mov dl,Data_byte 
    int 21h
    ret             
    COM2_adr dw ?
end start 
