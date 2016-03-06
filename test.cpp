#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>

int main(){
    char ** file_list = NULL;
    int num_files = 0;
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir (".")) != NULL) {
        // print all the files and directories within directory
        while ((ent = readdir (dir)) != NULL) {

            if(file_list == NULL){
                file_list = (char **)malloc(sizeof(char *));

            }
            else{
                file_list = (char **)realloc(file_list, (num_files + 1) * sizeof(char *));

            }
            assert(file_list != NULL);

            file_list[num_files] = (char *)malloc((strlen(ent->d_name) + 1) * sizeof(char));

            assert(file_list[num_files] != NULL);

            file_list[num_files] = ent->d_name;
            printf("%s\n", file_list[num_files]);
            num_files += 1;
        }
        closedir (dir);
    }
    else {
        // could not open directory
        perror ("");
        exit(1);
    }

    return 0;
}
