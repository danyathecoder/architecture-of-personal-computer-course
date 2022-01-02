.model tiny   
.code
org  100h 
jmp start

NO_READ proc 
    mov ah,9
    mov dx,offset message
    int 21h
    ret
    message db "Somethig is wrong$"
NO_READ endp 
print proc
        mov ah,02h
        mov dl,al
        int 21h
        ret
        print endp  
start:   
    mov ah, 02h 
    mov dx, 00h
    int 14h
    test ah, 80h
    jz call print  
    call NO_READ
    ret
end start 