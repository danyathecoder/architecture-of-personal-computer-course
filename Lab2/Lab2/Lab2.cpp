#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//1. MMX +
//2. SSE + 
//3. asm without MMX +
//4. C programming language 
//add time of calculation +
//matrix generates in application +
//many calculations (about 1M) +
//size of matrix aliquots amount of registers in MMX +

//variant 6 (21)
/*Нахождение суммы всех элементов матрицы.*/

void generate_mtx(int** mtx) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            mtx[i][j] = rand() % 10;
        }
    }
}

void print_mtx(int** mtx) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%d ", mtx[i][j]);         
        }
        printf("\n");
    }
}

int** init_mtx(int size) {
    int** mtx = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) mtx[i] = (int*)malloc(size   * sizeof(int));
    generate_mtx(mtx);
    return mtx;
}

int sum_c(int** mtx) {
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            sum += mtx[i][j];
        }
    }
    return sum;
}

int sum_asm(int **mtx){
    int sum = 0;
    int *sum_p = &sum;
    __asm {
        mov ebx, sum_p //pointer to sum to ebx
        mov esi, mtx   //pointer to mtx to esi
        mov cx, 4     // init size of mtx
        l1:      
        push cx          //save cx to stack
        push esi        // save mtx to stack
        mov esi, [esi] // got address of array in esi
        mov cx, 4      //push cx again
            l2: 
            mov eax, [esi] //move data in esi to eax
            add[ebx], eax  //add eax to eax
            add esi, 4    //go to the next element
            loop l2
        pop esi           //regenerate esi 
        pop cx            //regenerate cx
        add esi, 4        //go to the next array
        loop l1
    }
    
    return sum;
}

int sum_mmx(int** mtx) {
    int sum = 0;
    int* sum_p = &sum;
    __asm {
        mov ebx, sum_p //pointer to sum to ebx
        mov esi, mtx   //pointer to mtx to esi
        mov cx, 4     // init size of mtx
        l1:
        push cx          //save cx to stack
            push esi        // save mtx to stack
            mov esi, [esi] // got address of array in esi
            mov cx, 4      //push cx again
            l2:
            pxor mm0, mm0
            pxor mm1, mm1
            movd mm0, [ebx] //put data from ebx to mm0
            movd mm1, [esi] //put data form esi to mm1
            paddd mm0, mm1  
            movd [ebx], mm0 //put sum back to ebx
            add esi, 4    //go to the next element
            loop l2
    pop esi           //regenerate esi 
    pop cx            //regenerate cx
    add esi, 4        //go to the next array
    loop l1
    emms    //switch to fpu
    }

    return sum;
}

int sum_sse(int** mtx) {
    int sum = 0;
    int* sum_p = &sum;
    __asm {
        xorps xmm1, xmm1
        mov ebx, sum_p //pointer to sum to ebx
        mov esi, mtx   //pointer to mtx to esi
        mov cx, 4     // init size of mtx
        l1:
        push cx          //save cx to stack
            push esi        // save mtx to stack
            mov esi, [esi] // got address of array in esi
            mov cx, 4      //push cx again
            l2 :
            xorps xmm0, xmm0
            xorps xmm1, xmm1
            movlps xmm0, [ebx] //put data from ebx to mm0
            movlps xmm1, [esi] //put data form esi to mm1
            addps xmm0, xmm1
            movlps [ebx], xmm0 //put sum back to ebx
            add esi, 4    //go to the next element
            loop l2
            pop esi           //regenerate esi 
            pop cx            //regenerate cx
            add esi, 4        //go to the next array
            loop l1
    }
    return sum;
}


int main(){
    clock_t begin, end;
    double time_spent;
    int sum;

    srand(time(0));

    printf("1. Calculate with C programming language:\n");
    begin = clock();
    for (int i = 0; i < 1'000'000; i++) {
        int** mtx = init_mtx(4);
        sum = sum_c(mtx);
        if (i == 999'999) {
            printf("Matrix:\n");
            print_mtx(mtx);
            printf("Sum of all elements: %d\n", sum);
        }
        free(mtx);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Wall time %f\n", time_spent);

    printf("2. Calculate with asm:\n");
    begin = clock();
    for (int i = 0; i < 1'000'000; i++) {
        int** mtx = init_mtx(4);
        sum = sum_asm(mtx);
        if (i == 999'999) {
            printf("Matrix:\n");
            print_mtx(mtx);
            printf("Sum of all elements: %d\n", sum);
        }
        free(mtx);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Wall time %f\n", time_spent);

    printf("3. Calculate with MMX:\n");
    begin = clock();
    for (int i = 0; i < 1'000'000; i++) {
        int** mtx = init_mtx(4);
        sum = sum_mmx(mtx);
        if (i == 999'999) {
            printf("Matrix:\n");
            print_mtx(mtx);
            printf("Sum of all elements: %d\n", sum);
        }
        free(mtx);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Wall time %f\n", time_spent);

    printf("4. Calculate with SSE:\n");
    begin = clock();
    for (int i = 0; i < 1'000'000; i++) {
        int** mtx = init_mtx(4);
        sum = sum_sse(mtx);
        if (i == 999'999) {
            printf("Matrix:\n");
            print_mtx(mtx);
            printf("Sum of all elements: %d\n", sum);
        }
        free(mtx);
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Wall time %f", time_spent);

    return 0;
}