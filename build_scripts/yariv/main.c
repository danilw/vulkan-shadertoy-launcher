#include <stdio.h>
#include <string.h>

#include "yariv.h"

void *spirv;
size_t spirv_size;

static void replacestr(char *line, const char *search, const char *replace)
{
     char *sp;

     if ((sp = strstr(line, search)) == NULL) {
         return;
     }
     int search_len = strlen(search);
     int replace_len = strlen(replace);
     int tail_len = strlen(sp+search_len);

     memmove(sp+replace_len,sp+search_len,tail_len+1);
     memcpy(sp, replace, replace_len);
}

int main(int argc,char* argv[])  {
    
    if(argc > 2) {
        printf("Too many arguments supplied. One argument expected.\n");
        return 0;
    }
    if(argc == 1) {
        printf("One argument expected.\n");
        return 0;
    }
    
    if(strstr(argv[1], ".spv") == NULL){
        printf("error in file name, need file.spv \n");
        return 0;
    }
    
    FILE *f = fopen(argv[1], "rb");
    if (f==NULL){
        printf("File not found.\n");
        return 0;
    }
    
    fseek(f, 0, SEEK_END);
    spirv_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    spirv = malloc(spirv_size);
    
    fread(spirv, 1, spirv_size, f);
    fclose(f);
    
    void *out_yariv;
    size_t out_yariv_size;
    out_yariv_size = yariv_encode_size(yariv_encode_flags_strip, spirv, spirv_size);
    out_yariv = malloc(out_yariv_size);
    yariv_encode(yariv_encode_flags_strip, out_yariv, out_yariv_size, spirv, spirv_size);
    
    
    char str[200];
    strcpy(str,argv[1]);
    replacestr(str,".spv",".yariv");
    FILE *fp;
    fp = fopen(str, "w");
    fwrite(out_yariv,out_yariv_size,1,fp);
    fclose(fp);
    free(out_yariv);
    
    printf("converted %s to %s yariv_size %ld \n",argv[1],str,out_yariv_size);
}

