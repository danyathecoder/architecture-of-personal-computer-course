.model tiny   
.code
org  100h 
jmp start
init proc
    mov al,80h
    mov dx,3FBh
    out dx,al 
    mov dx,3F8h
    mov al,00h
    out dx,al
    mov al,0Ch
    mov dx,3F9h
    out dx,al
    mov dx,3FBh
    mov al,00000011b            
    out dx,al
    mov dx,3F9h
    mov al,0  
    out dx,al
    ret
init endp   

isallright proc
    xor al,al
    mov dx,3FDh
    in al,dx
    test al,10h
    jnz NO_WRITE
    ret
isallright endp

NO_WRITE proc 
    mov ah,9
    mov dx,offset message
    int 21h 
    ret
    message db "Somethig is wrong$"
NO_WRITE endp 

message2 db "Enter the symbol", 0Dh,0Ah,'$'

sendbyte proc
    mov dx,3F8h
    out dx,al
    ret
sendbyte endp 

start:
    call init
    call isallright
    mov ah,9
    mov dx,offset message2 
    int 21h 
    mov ah, 01h
    int 21h 
    call sendbyte
    ret
end start 