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
int length=5; //inisialisasi jumlah untuk looping

void* playandcount(void *arg)
{
    
    char abc[200];
    strcpy(abc,arg);

	unsigned long i=0;
	pthread_t id=pthread_self();
	int iter;

    char *token1 = strtok(abc, "/");
    while( token1 != NULL ) {
        // printf( "token %d = %s\n", m , token1 );
        arr2[m] = token1;
        m++;
        token1 = strtok(NULL, "/");
    }
    char namafile[200];
    char *arr2[50];
    strcpy(namafile,arr2[m-1]);
    //printf("%s\n", arr3);
    char *token = strtok(arr2[m-1], ".");
    
    int n=0;
    while( token != NULL ) {
        arr[n] = token;
        n++;
        token = strtok(NULL, ".");
    }

    char titik= '.';
    char bb;
    char hid=namafile[0];

    char *arr[4]
    char aa[100];
    strcpy(aa,arr[n-1]);
    for(int i = 0; aa[i]; i++){
        aa[i] = tolower(aa[i]);
    }
  
    DIR *folder, *folderopen;
    struct dirent *entry;
    char tempat2[100],tempat3[100];
    folder = opendir("/home/vika/modul3/");
    int available = 0;

    //printf("%c %c\n",titik, hid);
    if(titik== hid){
        //printf("test\n");
        strcpy(tempat2,"/home/vika/modul3/");
        strcat(tempat2,"Hidden");
        mkdir(tempat2,0750);
    }

    else {
        if( n > 1 ){
            if(folder == NULL)
            {
                printf("error\n");
            }
            while( (entry=readdir(folder)) )
            {
                //foldernya udah ada apa belum
                // printf("%s %d\n",entry->d_name,entry->d_type);
                if(strcmp(entry->d_name,aa) == 0 && entry->d_type == 4){
                    //kalau ada
                    available = 1;
                    break;
                }
            }

            //file gak ada
            // printf("aa %s\n",tempat2);
            if(available == 0){
                // printf("bisa\n");
                strcpy(tempat2,"/home/vika/modul3/");
                strcat(tempat2,aa);
                mkdir(tempat2,0750);        
            }
        }
        else if (n<=1){
            strcpy(tempat2,"/home/vika/modul3/");
            strcat(tempat2,"Unknown");
            mkdir(tempat2,0750);
        }
    }

    char source[1000], target[1000];
    strcpy(source,arg);
    strcpy(target,"/home/vika/modul3/");
    if(n == 1 ){
        strcat(target,"Unknown");
    }
    else if(titik==hid){
        strcat(target,"Hidden");
    }
    else if (n>1){
        strcat(target,aa);
        
    }

    strcat(target,"/");
    strcat(target,namafile;
    rename(source,target);
    n = 0;

    //printf("%s \n %s \n", source, target);

	return NULL;
}


int main(int argc, char *argv[]) {
    int i=0,j=0;

    DIR *fd, *fdo;
        struct dirent *masuk;
        char tempatx[200];
    //menambahkan argumen file yang bisa dikategorikan
    if (strcmp(argv[1],"-f") == 0) {
        for(j = 2 ; j < argc ; j++ ){
            int err;
            //membuat thread
            err=pthread_create(&(thd[i]),NULL,playandcount,argv[j]);
            if (err !=0){
                //error gak
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
       
        fd = opendir(argv[2]);

        //gak bisa dibuka
        if(fd == NULL)
        {
            printf("error\n");
        }

        while( (masuk=readdir(fd)) )
        {
            if ( !strcmp(masuk->d_name, ".") || !strcmp(masuk->d_name, "..") )
            continue;
            //printf("%s %d\n",masuk->d_name,masuk->d_type);

            //nyimpen path
            strcpy(tempatx,argv[2]);
            strcat(tempatx,"/");
            strcat(tempatx,masuk->d_name);
            
            //kalau file maka
            if(masuk->d_type == 8){

                //membuat thread
            int err = pthread_create(&(thd[i]),NULL,playandcount,tempatx);
            if (err !=0){
                //error gak
                printf ("Yah, gagal disimpan :(");
                return 0;
            }
            pthread_join(thd[i],NULL);
            i++;
            }

        }
        printf("Direktori sukses disimpan!\n");
    }
    
    //mengkategorikan seluruh file yang ada di working directory
    else if (strcmp(argv[1],"*") == 0 && argc == 2) {
        i = 0;
        fd = opendir("/home/vika/modul3/");
        int available = 0;

        if(fd == NULL)
        {
            printf("error\n");
        }
        while( (masuk=readdir(fd)) )
        {
            if ( !strcmp(masuk->d_name, ".") || !strcmp(masuk->d_name, "..") )
            continue;
            //printf("%s %d\n",masuk->d_name,masuk->d_type);

            int err;
            strcpy(tempatx,"/home/vika/modul3/");
            strcat(tempatx,masuk->d_name);
            if(masuk->d_type == 8){
                //membuat thread
            pthread_create(&(thd[i]),NULL,playandcount,tempatx); 
            pthread_join(thd[i],NULL);
            i++;
            }
        }

    }
    return 0; 
}