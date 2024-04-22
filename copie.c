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
    char path[400];
    long size,innode;
    char last_modify[400];
    char tip[12];//director,file
}File_Info;
void afisare(File_Info *v,int size_v)
{
    printf("size pt acest vector este %d: ",size_v);
    for(int i=0;i<size_v;i++)
        printf("%d -- %s  ",i,v[i].path);
    printf("\n");
}
void parse_director(char *dir_path,int nr_rulari)
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
    File_Info *v=NULL;
    int size_v=0;
    int prev_snapfd;
    int j;
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

        struct stat metadata;
        if(stat(path,&metadata)<0)
        {
            perror(NULL);
            exit(-1);
        }
        char time[50];
        strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&metadata.st_mtime));
        char aux[350],aux2[350],aux3[350],aux4[350];
        mode_t type=metadata.st_mode;
        if(nr_rulari==1)
        {
            
            prev_snapfd=open("prev_snap.bin",O_RDWR );
            if(prev_snapfd==-1)
            {
                if((prev_snapfd=open("prev_snap.bin",O_RDWR | O_CREAT,0666))<0)
                {
                    perror("eroare deschidere snaphot1 ");
                    exit(-1); 
                }
                printf("sas\n");
                size_v=0; 
                v=NULL;
            }
            else
            {
                size_v=0;
                v=NULL;
                read(prev_snapfd,&size_v, sizeof(int));
                //printf("sivev=%d\n",size_v);
                if((v=malloc(size_v*sizeof(File_Info)))==NULL){printf("eroare malloc"); exit(-1);}
                read(prev_snapfd,v, size_v*sizeof(File_Info));
            }
            //afisare(v,size_v);
            //printf("size_v este inainte de incrementare -%d,iar dupa este",size_v);
            size_v++; //printf("%d \n",size_v);
            if((v=realloc(v,size_v*sizeof(File_Info)))==NULL){printf("eroare realloc"); exit(-1);}
            strcpy(v[size_v-1].path,path);
            v[size_v-1].size=metadata.st_size;
            strcpy(v[size_v-1].last_modify,time);
            v[size_v-1].innode=metadata.st_ino;
            if(S_ISDIR(type))   
                strcpy(v[size_v-1].tip,"director");
            else
                strcpy(v[size_v-1].tip,"file");
            lseek(prev_snapfd, 0, SEEK_SET);
            //printf("sizev=%d, path=%s, size=%ld, last_modif= %s, innode= %ld, tip=%s\n",
                //size_v,v[size_v-1].path,v[size_v-1].size,v[size_v-1].last_modify,v[size_v-1].innode,v[size_v-1].tip);
            if((write(prev_snapfd,&size_v,sizeof(int)))<0)
            {
                perror(NULL);
                exit(-1);   
            }
            if((write(prev_snapfd,v,size_v*sizeof(File_Info)))<0)
            {
                perror(NULL);
                exit(-1);   
            }
            /*lseek(prev_snapfd, 0, SEEK_SET);
            int nr=0;
            read(prev_snapfd,&nr,sizeof(int));
            printf("nr=%d ",nr);
            File_Info *tab=malloc(nr*sizeof(File_Info));
            read(prev_snapfd,tab,sizeof(File_Info));
            printf("innode=%ld\n",tab[0].innode);*/

            sprintf(aux,"Acest fisier este un %s, iar numele sau este %s\nCaracteristicile sunt: \n",v[size_v-1].tip,path);
            sprintf(aux2,"In_nod-ul este %ld\n",metadata.st_ino);
            sprintf(aux3,"Dimensiunea este %ld\n",metadata.st_size);
            sprintf(aux4,"Data ultimei modifcari este %s\n\n",time);
            
            free(v);
            if(close(prev_snapfd)<0)
            {
                perror(NULL);
              exit(-1);  
            }
        }
        else//daca programul se afla la cel putin a 2 a rulare
        {
            if((prev_snapfd=open("prev_snap.bin",O_RDWR))<0)
            {
                perror("eroare deschidere snaphot1 ");
                exit(-1);  
            }
            size_v=0;
            v=NULL;
            read(prev_snapfd,&size_v,sizeof(int));
            if((v=malloc(size_v*sizeof(File_Info)))==NULL)
            {
                printf("eroare");
                exit(-1);
            }
            read(prev_snapfd,v,size_v*sizeof(File_Info));
            //afisare(v,size_v);
            //printf("size este %d, iar metadata_inn=%ld \n",size_v,metadata.st_ino);
            for(j=0;j<size_v;j++)//AVEM PROBLEMA AICI  CA NU GASESTE
            {
                //printf("%ld ",v[j].innode);
                if(v[j].innode==metadata.st_ino)
                {
                    break;
                }
            }
            //printf(",insa j este=%d\n\n",j);
            if(j==size_v)//daca nu s a gasit fisierul cu innodul cautat
            {
                size_v++;
                if((v=realloc(v,size_v*sizeof(File_Info)))==NULL){printf("eroare realloc"); exit(-1);}
                strcpy(v[size_v-1].path,path);
                v[size_v-1].size=metadata.st_size;
                strcpy(v[size_v-1].last_modify,time);
                v[size_v-1].innode=metadata.st_ino;
                if(S_ISDIR(type))   
                    strcpy(v[size_v-1].tip,"director");
                else
                    strcpy(v[size_v-1].tip,"file");
            
                sprintf(aux,"Acest fisier este unul nou si este un %s, iar numele sau este %s\nCaracteristicile sunt: \n",v[size_v-1].tip,path);
                sprintf(aux2,"In_nod-ul este %ld\n",metadata.st_ino);
                sprintf(aux3,"Dimensiunea este %ld\n",metadata.st_size);
                sprintf(aux4,"Data ultimei modifcari este %s\n\n",time);
            }
            else
            {
                if(strcmp(v[j].path,path)!=0 )
                {
                    sprintf(aux,"Numele acestui fisier s-a schimbat din %s in %s\nCaracteristicile sunt: \n",v[j].path,path);
                    strcpy(v[j].path,path);
                }
                else
                    sprintf(aux,"Numele este %s\nCaracteristicile sunt: \n",path);
                    
                if(v[j].size!=metadata.st_size)
                {
                    sprintf(aux3,"Dimensiunea s-a modificat din %ld in %ld\n",v[j].size,metadata.st_size);
                    v[j].size=metadata.st_size;
                }
                else
                    sprintf(aux3,"Dimensiunea este %ld\n",metadata.st_size);

                if(strcmp(v[j].last_modify,time)!=0)
                {
                    sprintf(aux4,"Data ultimei modifcari s-a schimbat din %s in %s\n\n",v[j].last_modify,time);
                    strcpy(v[j].last_modify,time);
                }
                else
                    sprintf(aux4,"Data ultimei modifcari este %s\n\n",time);
               
                sprintf(aux2,"In_nod-ul este %ld\n",metadata.st_ino); 
            }
            lseek(prev_snapfd, 0, SEEK_SET);
            if((write(prev_snapfd,&size_v,sizeof(int)))<0)
            {
                perror(NULL);
                exit(-1);   
            }
            if((write(prev_snapfd,v,size_v*sizeof(File_Info)))<0)
            {
                perror(NULL);
                exit(-1);   
            }
            free(v);
            if(close(prev_snapfd)<0)
            {
                perror(NULL);
              exit(-1);  
            }
        }

        //sprintf(aux,"Numele este %s\nCaracteristicile sunt: \n",path);
        if((write(snapfd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }

        //sprintf(aux,"In_nod-ul este %ld\n",metadata.st_ino);
        if((write(snapfd,aux2,strlen(aux2)))<0)
        {
            perror(NULL);
            exit(-1);   
        }

        //sprintf(aux,"Dimensiunea este %ld\n",metadata.st_size);
        if((write(snapfd,aux3,strlen(aux3)))<0)
        {
            perror(NULL);
            exit(-1);   
        }
        //sprintf(aux,"Data ultimei modifcari este %s\n\n",time);
        if((write(snapfd,aux4,strlen(aux4)))<0)
        {
            perror(NULL);
            exit(-1);   
        }
        
        if(close(snapfd)<0)
        {
            perror(NULL);
            exit(-1);  
        }
        if(S_ISDIR(type))   
            parse_director(path,nr_rulari);
    }
    if(closedir(dir))
    {
        perror(NULL);
        exit(-1);
    }
}
int run_count()
{
    int fd;
    int count;

    // Încercăm să deschidem fișierul pentru citire și scriere
    fd = open("run_count.bin", O_RDWR);

    if (fd == -1) {
        // Dacă fișierul nu există, îl cream și începem numărătoarea de la 1
        fd = open("run_count.bin", O_RDWR | O_CREAT, 0666);
        if (fd == -1) {
            perror("eroare deschidere fisier");
        }
        count = 1;
    } 
    else 
    {
        // Citim numărul de rulări actuale din fișier
        if (read(fd, &count, sizeof(int)) == -1) {
            perror("eroare citire");
            exit(-1);
        }
        count++;
    }

    // Ne întoarcem la începutul fișierului pentru a scrie noua valoare
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("eroare seeking");
        exit(-1);
    }

    // Convertim numărul de rulări înapoi în șir de caractere și îl scriem înapoi în fișier
    if (write(fd, &count, sizeof(int)) == -1) {
        perror("Error writing to file");
        exit(-1);
    }

    // Închidem fișierul
    if(close(fd)<0)
        {
            perror(NULL);
            exit(-1);  
        }
    return count;
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
    
    struct stat metadata;
    if(stat(argv[1],&metadata)<0)
        {
            perror(NULL);
            exit(-1);
        }
    sprintf(aux,"In_nod-ul este %ld\n",metadata.st_ino);

    if((write(snapfd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }

    sprintf(aux,"Dimensiunea este %ld\n",metadata.st_size);
    if((write(snapfd,aux,strlen(aux)))<0)
        {
            perror(NULL);
            exit(-1);   
        }
    char time[50];
    strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&metadata.st_mtime));

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

    int a=run_count();
    printf("a %d\n",a);
    parse_director(argv[1],a);
    return 0;
}