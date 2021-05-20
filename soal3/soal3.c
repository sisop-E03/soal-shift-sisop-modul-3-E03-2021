#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <unistd.h>
#include<pthread.h>
#include<dirent.h>
#include <sys/types.h>
#include<sys/stat.h>


pthread_t thd[3]; //inisialisasi array untuk menampung thread dalam kasus ini ada 2 thread
pid_t child;
int length=5;
int x=0;
char tanda[300][300]={};

void* playandcount(void *arg)
{
    
    char things[200];
    strcpy(things,arg);

	unsigned long i=0;
	pthread_t id=pthread_self();
	int iter;

    int m=0;
    char *arr2[50];
    //ngambil namafile beserta eksistensi
    char *token1 = strtok(things, "/");
    while( token1 != NULL ) {
        // printf( "token %d = %s\n", m , token1 );
        arr2[m] = token1;
        m++;
        token1 = strtok(NULL, "/");
    }
    char namafile[200];
    strcpy(namafile,arr2[m-1]);
    //printf("%s\n", arr3);

    //cek filenya termasuk dalam folder apa
    char aa[100];
    char *token = strchr(namafile, '.');
    if(token== NULL){
        strcat(aa, "Unknown");
    }
    else if(namafile[0]=='.'){
        strcat(aa, "Hidden");
    }
    else{
        strcpy(aa,token+1);
        for(int i = 0; aa[i]; i++){
            aa[i] = tolower(aa[i]);
        }
    }

    char tempat2[100];
    strcpy(tempat2,"/home/vika/modul3/");
    strcat(tempat2, aa);
    strcat(tempat2,"/");
    mkdir(tempat2,0750);
    
    char source[1000], target[1000];
    strcpy(source,arg);
    strcpy(target,"/home/vika/modul3/");
    strcat(target,aa);
    strcat(target,"/");
    strcat(target,namafile);

    //pindah file
    rename(source,target);
    return NULL;
}

void listFilesRecursively(char *basePath)
{
	char path[256]={};
	struct dirent *dp;
	DIR *dir = opendir(basePath);

	if (!dir)
	return;

	while ((dp = readdir(dir)) != NULL)
	{
		if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
		{
			if (dp->d_type == DT_REG)
			{
				strcpy(tanda[x], basePath);
				strcat(tanda[x], dp->d_name);
				x++;
			}
			else
			{
				strcpy(path, basePath);
				strcat(path, dp->d_name);
				strcat(path, "/");
				listFilesRecursively(path);
			}
		}
	}
	closedir(dir);
}


int main(int argc, char *argv[]) {
    int i=0,j=0;

    DIR *fd, *fdo;
        struct dirent *masuk;
    //menambahkan argumen file yang bisa dikategorikan
    if (strcmp(argv[1],"-f") == 0) {
        for(j = 2 ; j < argc ; j++ ){
            int err;
            //membuat thread
            err=pthread_create(&(thd[i]),NULL,playandcount,argv[j]);
            if (err !=0){
                //kalau error
                printf ("File %d : Sad,gagal :(\n", j-1);
                //return 0;
            }
            else{
                printf("File %d : Berhasil Dikategorikan\n",j-1);
            }

            pthread_join(thd[i],NULL);
             i++;
        }
    }

    //pengkategorian suatu directory
    else if (strcmp(argv[1],"-d") == 0 && argc == 3) {
        i = 0;
	   int err;
	    listFilesRecursively(argv[2]);

	    for (i=0; i<x; i++){
		    err=pthread_create(&(thd[i]),NULL,&playandcount,(void *) tanda[i]);
		    if(err!=0)
		    {
			    printf("Yah, gagal disimpan :(\n");
			    return 0;;
		    }
	    }
	    
        for (i=0; i<x; i++){
		    pthread_join(thd[i],NULL);
        }

    printf("Direktori sukses disimpan!\n");
    }
    
    //mengkategorikan seluruh file yang ada di working directory
    else if (strcmp(argv[1],"*") == 0 && argc == 2) {
        i = 0;
	   int err;
	    listFilesRecursively("/home/vika/modul3/");

	    for (i=0; i<x; i++){
		    err=pthread_create(&(thd[i]),NULL,&playandcount,(void *) tanda[i]);
		    
            if(err!=0){
			    return 0;
		    }
	    }

	    for (i=0; i<x; i++){
		    pthread_join(thd[i],NULL);
        }
    
    }
    return 0; 
}
