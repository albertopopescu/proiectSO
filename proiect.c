#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
typedef struct 
{
    char path[200];
    char size[30],innode[30];
    char data[200];
    char tip;//d-director,f-fisier
}File_Info;
void extract_File_Info(char *snapshot,File_Info f[100],int *size)
{
    int snapfd;
    if((snapfd=open(snapshot,O_RDONLY ))<0)
    {
        perror("eroare deschidere snaphot ");
        exit(-1);   
    }
    struct stat statbuf;
    if(stat(snapfd,&statbuf)<-1)
    {
        perror(NULL);
        exit(-1);
    }
     // Alocăm un buffer pentru a citi întregul conținut al fișierului.
    char *buffer = malloc(statbuf.st_size + 1);
    if (read(snapfd, buffer, statbuf.st_size) != statbuf.st_size) {
        // Eroare la citirea fișierului.
        printf("eroare la citire in buffer");
        return;
    }
    buffer[statbuf.st_size] = '\0'; // Asigurăm că buffer-ul este un string valid.

    // Parsăm buffer-ul linie cu linie.
    char *line = strtok(buffer, "\n");
    while (line) {
        if (strncmp(line, "Numele fisierlui este ", 12) == 0) {
            // Avem o linie cu numele fișierului.
            f = realloc(f, (*size + 1) * sizeof(File_Info));
            strcpy(f[(*size)++].path, line + 12); // Sărim peste "Numele este ".
            line = strtok(NULL, "\n");  // Trecem la următoarea linie pentru carac.
            line = strtok(NULL, "\n");//trecem la innode
            strcmp(f[*size-1].innode,line+10);
            line = strtok(NULL, "\n");//trecem la dimensiune
            strcmp(f[*size-1].size, line + 17);
        }
        line = strtok(NULL, "\n"); // Trecem la următoarea linie.
    }

    if(close(snapfd)<0)
    {
        perror(NULL);
        exit(-1);  
    }
}
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
    parse_director(argv[1]);
    return 0;
}