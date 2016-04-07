#include <stdio.h>
#include <stdlib.h>
#include "misc.h"

size_t fileLens(FILE *fPtr){
    if(fseek(fPtr, 0, SEEK_END)) {
        //File seek error.
        return -1;
    }
    size_t fileLength = ftell(fPtr);
    //移動指標回檔案起始
    rewind(fPtr);
    return fileLength;
}

void* my_safe_malloc(size_t mSize){
    void *ptr = malloc(mSize);
    if(ptr==NULL){
        printf("分配空間失敗!!\n");
    }
    return ptr;
}

/**
 * 印出到螢幕
 */
void printArray(uchar n, uchar** msg, size_t bytes){
    for(int n_index=0;n_index<n;n_index++)
    {
        for(int i=0;i<bytes;i++)
        {
            printf("%2X ", msg[n_index][i]);
        }
        printf("\n");
    }printf("\n");
}

void createTestFile(char *name, size_t bytes){
    FILE *fPtr = fopen(name, "wb");
    if(fPtr==NULL){ printf("開啟檔案失敗\n"); return; }
    uchar val;
    for(int i=0;i<bytes;i++){
        val = rand()%256;
        fwrite(&val, 1, 1, fPtr);
    }
    fclose(fPtr);
}
