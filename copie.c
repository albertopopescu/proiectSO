#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <sys/wait.h>
typedef struct
{
    char path[400];
    long size, innode;
    char last_modify[400];
    char tip[12]; // director,file
} File_Info;
File_Info array_innode[1000]; // vector cu innode-urile si numele fisierelor din rularea curenta
int size_array = 0;
void stergere_prev(File_Info *v, int *size_v, int poz)
{
    for (int i = poz; i < *size_v - 1; i++)
        v[i] = v[i + 1];
    *size_v = *size_v - 1;
}
void parse_director(char *dir_path, int nr_rulari, char *snap, char *prev_snap, char *numeSAFE)
{
    DIR *dir;
    if ((dir = opendir(dir_path)) == NULL)
    {
        perror(NULL);
        exit(-1);
    }
    struct dirent *file;
    char path[300]; // cale relativa catre fisier(director sau file)
    int snapfd;
    File_Info *v = NULL;
    int size_v = 0, prev_snapfd, j;

    while ((file = readdir(dir)) != NULL)
    {
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
            continue;
        sprintf(path, "%s/%s", dir_path, file->d_name);

        struct stat metadata;
        if (stat(path, &metadata) < 0)
        {
            perror(NULL);
            exit(-1);
        }
        if (!S_ISDIR(metadata.st_mode))
        {
            // printf("3");
            if (!(metadata.st_mode & S_IRUSR) && !(metadata.st_mode & S_IWUSR) && !(metadata.st_mode & S_IXUSR) &&
                !(metadata.st_mode & S_IRGRP) && !(metadata.st_mode & S_IWGRP) && !(metadata.st_mode & S_IXGRP) &&
                !(metadata.st_mode & S_IROTH) && !(metadata.st_mode & S_IWOTH) && !(metadata.st_mode & S_IXOTH))
            {
                // printf("4");
                pid_t pid = fork();
                if (pid == 0)
                {
                    char destPath[512];
                    snprintf(destPath, sizeof(destPath), "%s", numeSAFE);

                    execl("/bin/bash", "sh", "mall.sh", path, destPath, (char *)NULL); // mută fișierul
                    perror("Error executing sh");                                      // eroare dacă execl eșuează
                    exit(EXIT_FAILURE);                                                // oprește procesul copil în caz de eroare
                }
                else if (pid < 0)
                {
                    perror("Error forking process");
                    exit(EXIT_FAILURE);
                }
            }
        }
        char time[50];
        strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&metadata.st_mtime));
        char aux[1024], aux2[1024], aux3[1024], aux4[1024];
        mode_t type = metadata.st_mode;
        strcpy(array_innode[size_array].path, path);
        array_innode[size_array++].innode = metadata.st_ino; // celelalte campuri nu ne intereseaza, pt ca fol acest vector doar pentru stergere
        if ((snapfd = open(snap, O_WRONLY | O_APPEND)) < 0)
        {
            perror("eroare deschidere snaphot1 ");
            exit(-1);
        }

        if (nr_rulari == 1)
        {
            prev_snapfd = open(prev_snap, O_WRONLY);
            if (prev_snapfd == -1) // pt atunci cand acest prev_snap este prima data creat
            {
                if ((prev_snapfd = open(prev_snap, O_WRONLY | O_CREAT, 0666)) < 0)
                {
                    perror("eroare deschidere snaphot1 ");
                    exit(-1);
                }
                // printf("sas\n");
                size_v = 0;
                v = NULL;
                if (close(prev_snapfd) < 0)
                {
                    perror(NULL);
                    exit(-1);
                }
            }
            else // atunci cand prev_snap este deja creat,dar suntem tot la prima rulare
            {
                // size_v = 0;
                // v = NULL;
                if ((prev_snapfd = open(prev_snap, O_RDONLY)) < 0)
                {
                    perror("eroare deschidere snaphot1 ");
                    exit(-1);
                }
                read(prev_snapfd, &size_v, sizeof(int));
                if ((v = malloc(size_v * sizeof(File_Info))) == NULL)
                {
                    printf("eroare malloc");
                    exit(-1);
                }
                read(prev_snapfd, v, size_v * sizeof(File_Info));
                if (close(prev_snapfd) < 0)
                {
                    perror(NULL);
                    exit(-1);
                }
            }
            size_v++; // printf("%d \n",size_v);
            if ((v = realloc(v, size_v * sizeof(File_Info))) == NULL)
            {
                printf("eroare realloc");
                exit(-1);
            }
            strcpy(v[size_v - 1].path, path);
            v[size_v - 1].size = metadata.st_size;
            strcpy(v[size_v - 1].last_modify, time);
            v[size_v - 1].innode = metadata.st_ino;
            if (S_ISDIR(type))
                strcpy(v[size_v - 1].tip, "director");
            else
                strcpy(v[size_v - 1].tip, "file");

            // printf("sivev=%d\n", size_v);
            // for (int i = 0; i < size_v; i++)
            // {
            //     printf("%d    path=%s, size=%ld, last_modif= %s, innode= %ld, tip=%s\n\n",
            //            i, v[i].path, v[i].size, v[i].last_modify, v[i].innode, v[i].tip);
            // }

            if ((prev_snapfd = open(prev_snap, O_WRONLY)) < 0)
            {
                perror("eroare deschidere snaphot1 ");
                exit(-1);
            }
            if ((write(prev_snapfd, &size_v, sizeof(int))) < 0)
            {
                perror(NULL);
                exit(-1);
            }
            if ((write(prev_snapfd, v, size_v * sizeof(File_Info))) < 0)
            {
                perror(NULL);
                exit(-1);
            }

            sprintf(aux, "Acest fisier este un %s, iar numele sau este %s\nCaracteristicile sunt: \n", v[size_v - 1].tip, path);
            sprintf(aux2, "In_nod-ul este %ld\n", metadata.st_ino);
            sprintf(aux3, "Dimensiunea este %ld\n", metadata.st_size);
            sprintf(aux4, "Data ultimei modifcari este %s\n\n", time);
            if (close(prev_snapfd) < 0)
            {
                perror(NULL);
                exit(-1);
            }
            free(v);
        }
        else // daca programul se afla la cel putin a 2 a rulare
        {
            if ((prev_snapfd = open(prev_snap, O_RDWR)) < 0)
            {
                perror("eroare deschidere snapshot1 ");
                exit(-1);
            }
            size_v = 0;
            v = NULL;
            read(prev_snapfd, &size_v, sizeof(int));
            if ((v = malloc(size_v * sizeof(File_Info))) == NULL)
            {
                printf("eroare malloc");
                exit(-1);
            }
            read(prev_snapfd, v, size_v * sizeof(File_Info));
            for (j = 0; j < size_v; j++)
            {
                if (v[j].innode == metadata.st_ino)
                {
                    break;
                }
            }
            if (j == size_v) // daca nu s a gasit fisierul cu innodul cautat
            {
                size_v++;
                if ((v = realloc(v, size_v * sizeof(File_Info))) == NULL)
                {
                    printf("eroare realloc");
                    exit(-1);
                }
                strcpy(v[size_v - 1].path, path);
                v[size_v - 1].size = metadata.st_size;
                strcpy(v[size_v - 1].last_modify, time);
                v[size_v - 1].innode = metadata.st_ino;
                if (S_ISDIR(type))
                    strcpy(v[size_v - 1].tip, "director");
                else
                    strcpy(v[size_v - 1].tip, "file");

                sprintf(aux, "Acest fisier este unul nou si este un %s, iar numele sau este %s\nCaracteristicile sunt: \n", v[size_v - 1].tip, path);
                sprintf(aux2, "In_nod-ul este %ld\n", metadata.st_ino);
                sprintf(aux3, "Dimensiunea este %ld\n", metadata.st_size);
                sprintf(aux4, "Data ultimei modifcari este %s\n\n", time);
            }
            else
            {
                if (strcmp(v[j].path, path) != 0)
                {
                    sprintf(aux, "Numele acestui fisier s-a schimbat din %s in %s\nCaracteristicile sunt: \n", v[j].path, path);
                    strcpy(v[j].path, path);
                }
                else
                    sprintf(aux, "Numele este %s, iar tipul sau este %s\nCaracteristicile sunt: \n", path, v[j].tip);

                if (v[j].size != metadata.st_size)
                {
                    sprintf(aux3, "Dimensiunea s-a modificat din %ld in %ld\n", v[j].size, metadata.st_size);
                    v[j].size = metadata.st_size;
                }
                else
                    sprintf(aux3, "Dimensiunea este %ld\n", metadata.st_size);

                if (strcmp(v[j].last_modify, time) != 0)
                {
                    sprintf(aux4, "Data ultimei modifcari din interior s-a schimbat din %s in %s\n\n", v[j].last_modify, time);
                    strcpy(v[j].last_modify, time);
                }
                else
                    sprintf(aux4, "Data ultimei modifcari din interior este %s\n\n", time);

                sprintf(aux2, "In_nod-ul este %ld\n", metadata.st_ino);
            }
            lseek(prev_snapfd, 0, SEEK_SET);
            if ((write(prev_snapfd, &size_v, sizeof(int))) < 0)
            {
                perror(NULL);
                exit(-1);
            }
            if ((write(prev_snapfd, v, size_v * sizeof(File_Info))) < 0)
            {
                perror(NULL);
                exit(-1);
            }

            free(v);
            if (close(prev_snapfd) < 0)
            {
                perror(NULL);
                exit(-1);
            }
        }

        // sprintf(aux,"Numele este %s\nCaracteristicile sunt: \n",path);
        if ((write(snapfd, aux, strlen(aux))) < 0)
        {
            perror(NULL);
            exit(-1);
        }

        // sprintf(aux,"In_nod-ul este %ld\n",metadata.st_ino);
        if ((write(snapfd, aux2, strlen(aux2))) < 0)
        {
            perror(NULL);
            exit(-1);
        }

        // sprintf(aux,"Dimensiunea este %ld\n",metadata.st_size);
        if ((write(snapfd, aux3, strlen(aux3))) < 0)
        {
            perror(NULL);
            exit(-1);
        }
        // sprintf(aux,"Data ultimei modifcari este %s\n\n",time);
        if ((write(snapfd, aux4, strlen(aux4))) < 0)
        {
            perror(NULL);
            exit(-1);
        }

        if (close(snapfd) < 0)
        {
            perror(NULL);
            exit(-1);
        }
        if (S_ISDIR(type))
            parse_director(path, nr_rulari, snap, prev_snap, numeSAFE);
    }

    if (closedir(dir))
    {
        perror(NULL);
        exit(-1);
    }
}
void parse_and_delete(char *dir_path, int nr_rulari, char *director, char *numeSAFE)
{
    size_array = 0;
    char snap[100], prev_snap[100];
    sprintf(snap, "%s/%s_snapshot.txt", director, dir_path);
    sprintf(prev_snap, "%s/%s_prev_snap.bin", director, dir_path);
    parse_director(dir_path, nr_rulari, snap, prev_snap, numeSAFE);
    File_Info *v = NULL;
    int size_v = 0, snapfd, prev_snapfd;
    // char *snap="snapshot.txt";
    if (nr_rulari >= 2)
    {
        if ((snapfd = open(snap, O_WRONLY | O_APPEND)) < 0)
        {
            perror("eroare deschidere snaphot1 ");
            exit(-1);
        }
        if ((prev_snapfd = open(prev_snap, O_RDWR)) < 0)
        {
            perror("eroare deschidere snapshot1 ");
            exit(-1);
        }
        size_v = 0;
        v = NULL;
        read(prev_snapfd, &size_v, sizeof(int));
        if ((v = malloc(size_v * sizeof(File_Info))) == NULL)
        {
            printf("eroare malloc");
            exit(-1);
        }
        read(prev_snapfd, v, size_v * sizeof(File_Info));
        int i = 0;
        while (i < size_v)
        {
            int ok = 1; // presupun ca la rularea curenta s a sters un fisier
            for (int j = 0; j < size_array && ok == 1; j++)
                if (v[i].innode == array_innode[j].innode)
                {
                    ok = 0;
                }
            if (ok == 1)
            {
                char str[500];
                sprintf(str, "Fata de rularea anterioara s-a sters fisierul cu denumirea %s si innode-ul %ld\n\n", v[i].path, v[i].innode);
                if ((write(snapfd, str, strlen(str))) < 0)
                {
                    perror(NULL);
                    exit(-1);
                }
                stergere_prev(v, &size_v, i);
            }
            else
                i = i + 1;
        }
        if (close(snapfd) < 0)
        {
            perror(NULL);
            exit(-1);
        }
        lseek(prev_snapfd, 0, SEEK_SET);
        if ((write(prev_snapfd, &size_v, sizeof(int))) < 0)
        {
            perror(NULL);
            exit(-1);
        }
        if ((write(prev_snapfd, v, size_v * sizeof(File_Info))) < 0)
        {
            perror(NULL);
            exit(-1);
        }
        free(v);
        if (close(prev_snapfd) < 0)
        {
            perror(NULL);
            exit(-1);
        }
    }
}
int run_count()
{
    int fd;
    int count;
    // Încercăm să deschidem fișierul pentru citire și scriere
    fd = open("run_count.bin", O_RDWR);
    if (fd == -1)
    {
        // Dacă fișierul nu există, îl cream și începem numărătoarea de la 1
        fd = open("run_count.bin", O_RDWR | O_CREAT, 0666);
        if (fd == -1)
        {
            perror("eroare deschidere fisier");
        }
        count = 1;
    }
    else
    {
        // Citim numărul de rulări actuale din fișier
        if (read(fd, &count, sizeof(int)) == -1)
        {
            perror("eroare citire");
            exit(-1);
        }
        count++;
    }
    // Ne întoarcem la începutul fișierului pentru a scrie noua valoare
    if (lseek(fd, 0, SEEK_SET) == -1)
    {
        perror("eroare seeking");
        exit(-1);
    }
    // Convertim numărul de rulări înapoi în șir de caractere și îl scriem înapoi în fișier
    if (write(fd, &count, sizeof(int)) == -1)
    {
        perror("Error writing to file");
        exit(-1);
    }
    // Închidem fișierul
    if (close(fd) < 0)
    {
        perror(NULL);
        exit(-1);
    }
    return count;
}
int main(int argc, char **argv)
{
    if (argc < 6 || argc > 15)
    {
        printf("eroare de argumente");
        exit(-1);
    }
    int a = run_count();
    int pid;
    int nr_procese = 0;
    // printf("a %d\n", a);

    for (int i = 5; i < argc; i++)
    {
        struct stat st;
        if (stat(argv[i], &st) == 0 && S_ISDIR(st.st_mode) == 0)
        {
            printf("Fisierul %s nu este direcotr\n", argv[i]);
            continue;
        }
        printf("Procesul de creare a snapshot-ului pentru directorul %s a fost realizat cu succes\n", argv[i]);
        nr_procese = nr_procese + 1;
        if ((pid = fork()) < 0)
        {
            perror("Eroare creare proces");
            exit(-1);
        }
        if (pid == 0)
        {
            DIR *dir;
            if ((dir = opendir(argv[i])) == NULL)
            {
                perror("eroare la director ");
                exit(-1);
            }
            int snapfd;
            char snap[100];
            sprintf(snap, "%s/%s_snapshot.txt", argv[2], argv[i]);

            if ((snapfd = open(snap, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR)) < 0)
            {
                perror("eroare la deschidere snapshot ");
                exit(-1);
            }
            char aux[400];
            sprintf(aux, "Numele directorului este %s\n", argv[i]);
            if ((write(snapfd, aux, strlen(aux))) < 0)
            {
                perror(NULL);
                exit(-1);
            }

            struct stat metadata;
            if (stat(argv[i], &metadata) < 0)
            {
                perror(NULL);
                exit(-1);
            }
            sprintf(aux, "In_nod-ul este %ld\n", metadata.st_ino);

            if ((write(snapfd, aux, strlen(aux))) < 0)
            {
                perror(NULL);
                exit(-1);
            }

            sprintf(aux, "Dimensiunea este %ld\n", metadata.st_size);
            if ((write(snapfd, aux, strlen(aux))) < 0)
            {
                perror(NULL);
                exit(-1);
            }
            char time[50];
            strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&metadata.st_mtime));

            sprintf(aux, "Data ultimei modifcari din interior este %s\n\n", time);
            if ((write(snapfd, aux, strlen(aux))) < 0)
            {
                perror(NULL);
                exit(-1);
            }
            if (close(snapfd) < 0)
            {
                perror(NULL);
                exit(-1);
            }
            if (closedir(dir))
            {
                perror(NULL);
                exit(-1);
            }
            parse_and_delete(argv[i], a, argv[2],argv[4]);
            exit(0);
        }
    }
    int status;
    for (int i = 1; i <= nr_procese; i++)
    {
        int wait_pid = wait(&status);
        printf("Procesul copil cu PID ul %d s a incheiat cu codul %d\n", wait_pid, WEXITSTATUS(status));
    }
    return 0;
}