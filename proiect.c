#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
void parse_director(char *dir_path)
{
    DIR *dir;
    if((dir=opendir(dir_path))==NULL)
    {
        perror(NULL);
        exit(-1);
    }
    struct dirent *file;
    char path[300];//cale relativa catre fisier(director sau file)
    int snapfd;
    char *snap="snapshot.txt";
    while((file=readdir(dir))!=NULL)
    {
        // struct stat fileStat;
        // stat(argv[1],&fileStat);
        if(strcmp(file->d_name,".")==0 || strcmp(file->d_name,"..")==0)
            continue;
        sprintf(path,"%s/%s",dir_path,file->d_name);
        if((snapfd=open(snap,O_WRONLY | O_APPEND))<0)
        {
            perror("eroare deschidere snaphot1 ");
            exit(-1);   
        }

        char aux[400];
        sprintf(aux,"Numele este %s\nCaracteristicile sunt: \n",path);
        if((write(snapfd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }

        struct stat in_nod;
        if(stat(path,&in_nod)<0)
        {
            perror(NULL);
            exit(-1);
        }
        sprintf(aux,"In_nod-ul este %ld\n",in_nod.st_ino);
        if((write(snapfd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }

        sprintf(aux,"Dimensiunea este %ld\n\n",in_nod.st_size);
        if((write(snapfd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }

        if(close(snapfd)<0)
        {
            perror(NULL);
            exit(-1);  
        }
        mode_t type=in_nod.st_mode;
        if(S_ISDIR(type))   
            parse_director(path);
    }
    if(closedir(dir))
    {
        perror(NULL);
        exit(-1);
    }
}
int main(int argc,char **argv)
{
    if(argc<2)
    {
        printf("eroare de argumente");
        exit(-1);
    }
    DIR *dir;
    if((dir=opendir(argv[1]))==NULL)
    {
        perror("eroare la director ");
        exit(-1);
    }
    int snapfd;
    char *snap="snapshot.txt";
    if((snapfd=open(snap,O_CREAT | O_WRONLY,S_IRUSR| S_IWUSR| S_IXUSR))<0)
        {
            perror("eroare la deschidere snapshot ");
            exit(-1);   
        }
    char aux[400];
    sprintf(aux,"Numele direcotrului este %s\n",argv[1]);
    if((write(snapfd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }
    
    struct stat in_nod;
    if(stat(argv[1],&in_nod)<0)
        {
            perror(NULL);
            exit(-1);
        }
    sprintf(aux,"In_nod-ul este %ld\n",in_nod.st_ino);
    if((write(snapfd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }

    sprintf(aux,"Dimensiunea este %ld\n\n",in_nod.st_size);
    if((write(snapfd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }
    
    if(close(snapfd)<0)
        {
            perror(NULL);
            exit(-1);  
        }
    if(closedir(dir))
    {
        perror(NULL);
        exit(-1);
    }
    parse_director(argv[1]);
    return 0;
}