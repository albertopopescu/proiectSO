#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
void parse_directory(char *dir_path,int fd,char *myfile)
{
    DIR *dir;
    if((dir=opendir(dir_path))==NULL)
    {
        perror(NULL);
        exit(-1);
    }
    struct dirent *file;
    char path[300];//cale relativa catre fisier(director sau file)
    
    while((file=readdir(dir))!=NULL)
    {
        if(strcmp(file->d_name,".")==0 || strcmp(file->d_name,"..")==0)
            continue;
        sprintf(path,"%s/%s",dir_path,file->d_name);

        char aux[400];
        sprintf(aux,"%s,",path);
        if((write(fd,aux,strlen(aux)))<0)
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
        sprintf(aux,"%ld,",in_nod.st_ino);
        if((write(fd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }

        sprintf(aux,"%ld,",in_nod.st_size);
        if((write(fd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }
        char time[50];
        strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&in_nod.st_mtime));
        sprintf(aux,"Data ultimei modifcari este %s\n",time);
        if((write(fd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }

        mode_t type=in_nod.st_mode;
        if(S_ISDIR(type))   
        {
            parse_directory(path,fd,myfile);

        }
    }
    if(closedir(dir))
    {
        perror("eroare la inchiderea directorului ");
        exit(-1);
    }
}
void snap_directory(char *dir_path,int snapfd,char *snap)
{
    DIR *dir;
    if((dir=opendir(dir_path))==NULL)
    {
        perror(NULL);
        exit(-1);
    }
    struct dirent *file;
    char path[300];//cale relativa catre fisier(director sau file)
    while((file=readdir(dir))!=NULL)
    {
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

        sprintf(aux,"Dimensiunea este %ld\n",in_nod.st_size);
        if((write(snapfd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }
        char time[50];
        strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&in_nod.st_mtime));
        sprintf(aux,"Data ultimei modifcari este %s\n\n",time);
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
    if((snapfd=open(snap,O_CREAT | O_TRUNC | O_WRONLY,S_IRUSR| S_IWUSR| S_IXUSR))<0)
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

    sprintf(aux,"Dimensiunea este %ld\n",in_nod.st_size);
    if((write(snapfd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }
    char time[50];
    strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&in_nod.st_mtime));
    sprintf(aux,"Data ultimei modifcari este %s\n\n",time);
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
    
    // int fd_myfile;
    // char *myfile="latest_data.txt";
    // if((fd_myfile=open(myfile,O_CREAT | O_TRUNC | O_WRONLY,S_IRUSR| S_IWUSR| S_IXUSR))<0)
    //     {
    //         perror("eroare la deschidere myfile ");
    //         exit(-1);   
    //     }
    // parse_directory(argv[1],fd_myfile,myfile);
    // if(close(fd_myfile)<0)
    //     {
    //         perror(NULL);
    //         exit(-1);  
    //     }
    
    return 0;
}