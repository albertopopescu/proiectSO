#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX_DIRS 10

typedef struct Snapshot
{
    char filename[256];
    time_t mtime;
    off_t size;
} Snapshot;

int compareSnapshots(const void *a, const void *b)
{
    const Snapshot *s1 = (const Snapshot *)a;
    const Snapshot *s2 = (const Snapshot *)b;
    return strcmp(s1->filename, s2->filename);
}

void saveSnapshot(char *dirPath, int fd)
{

    DIR *dir = opendir(dirPath);
    if (!dir)
    {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);

            struct stat st;
            if (lstat(path, &st) == 0)
            {
                Snapshot snapshot;
                strcpy(snapshot.filename, path);
                snapshot.mtime = st.st_mtime;
                snapshot.size = st.st_size;

                write(fd, &snapshot, sizeof(Snapshot));
                printf("%s - %ld - %ld\n", snapshot.filename, snapshot.mtime, snapshot.size);
                if (S_ISDIR(st.st_mode))
                {
                    saveSnapshot(path, fd);
                }
            }
        }
    }

    closedir(dir);
}

Snapshot *snapshotRecursive(const char *dirPath, Snapshot *currentSnapshot, int *currentCount)
{
    DIR *dir = opendir(dirPath);
    if (!dir)
    {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);
            struct stat st;
            if (lstat(path, &st) == 0)
            {
                strcpy(currentSnapshot[*currentCount].filename, path);
                currentSnapshot[*currentCount].mtime = st.st_mtime;
                currentSnapshot[*currentCount].size = st.st_size;
                (*currentCount)++;

                if (S_ISDIR(st.st_mode))
                {
                    currentSnapshot = snapshotRecursive(path, currentSnapshot, currentCount);
                }
            }
        }
    }

    closedir(dir);
    return currentSnapshot;
}

void snapshot(const char *dirPath, char *numeFolder)
{
    char numeFisier[100] = "snapshot";
    // snprintf(numeFisier, sizeof(numeFisier), "%s/snapshot.txt", dirPath);
    strcat(numeFisier, numeFolder);
    strcat(numeFisier, ".txt");
    int fd = open(numeFisier, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd == -1)
    {
        perror("Error opening snapshot file");
        exit(EXIT_FAILURE);
    }
    close(fd);

    fd = open(numeFisier, O_RDONLY);
    if (fd == -1)
    {
        perror("Error1 opening snapshot file");
        exit(EXIT_FAILURE);
    }

    Snapshot currentSnapshot[1000], savedSnapshot[1000];
    int currentCount = 0, savedCount = 0;

    snapshotRecursive(dirPath, currentSnapshot, &currentCount);

    Snapshot buff2;

    while (read(fd, &buff2, sizeof(Snapshot)) > 0)
    {
        strcpy(savedSnapshot[savedCount].filename, buff2.filename);
        savedSnapshot[savedCount].mtime = buff2.mtime;
        savedSnapshot[savedCount].size = buff2.size;
        savedCount++;
    }

    qsort(currentSnapshot, currentCount, sizeof(Snapshot), compareSnapshots);
    qsort(savedSnapshot, savedCount, sizeof(Snapshot), compareSnapshots);

    int i = 0, j = 0;
    while (i < currentCount && j < savedCount)
    {
        int cmp = strcmp(currentSnapshot[i].filename, savedSnapshot[j].filename);
        if (cmp < 0)
        {
            printf("Added: %s\n", currentSnapshot[i].filename);
            i++;
        }
        else if (cmp > 0)
        {
            printf("Removed: %s\n", savedSnapshot[j].filename);
            j++;
        }
        else
        {
            if (currentSnapshot[i].mtime != savedSnapshot[j].mtime || currentSnapshot[i].size != savedSnapshot[j].size)
            {
                printf("Modified: %s\n", currentSnapshot[i].filename);
            }
            i++;
            j++;
        }
    }
    while (i < currentCount)
    {
        printf("Added: %s\n", currentSnapshot[i].filename);
        i++;
    }

    while (j < savedCount)
    {
        printf("Removed: %s\n", savedSnapshot[j].filename);
        j++;
    }
    close(fd);
    fd = open(numeFisier, O_WRONLY | O_TRUNC);
    if (fd == -1)
    {
        perror("Error opening snapshot file");
        exit(EXIT_FAILURE);
    }
    saveSnapshot(numeFolder, fd);
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > MAX_DIRS + 1)
    {
        fprintf(stderr, "Număr incorect de argumente în linia de comandă\n");
        exit(EXIT_FAILURE);
    }

    int pid;
    int count_procese = 0;
    for (int i = 2; i < argc; ++i)
    {

        struct stat st;
        if (stat(argv[i], &st) == 0 && S_ISDIR(st.st_mode))
        {
            printf("Processing directory: %s\n", argv[i]);
            count_procese++;
            if ((pid = fork()) < 0)
            {
                perror("Eroare");
                exit(1);
            }
            if (pid == 0)
            {
                bool corupt = false;
                if (corupt == true)
                {
                    exit(-1);
                }
                snapshot(argv[i], argv[i]);
                exit(0);
            }
        }
        else
        {
            printf("Ignoring non-directory argument: %s\n", argv[i]);
        }
        printf("\n");
    }

    int status;
    for (int i = 0; i < count_procese; i++)
    {
        int pid_exit = wait(&status);
        printf("PID:%d  --- STATUS:%d\n", pid_exit, WEXITSTATUS(status));
    }
    return 0;
}









for(int i = 3 ; i < argc ; i++){

        inode_number = 0;
        strcpy(buffer_auxiliar,"");
        
        pid = fork();
        if (pid == -1){
            printf("eroare fork\n");
            exit(1);
        }
        if(pid == 0){//cod fiu
            parcurgere_director(argv[i],0, &inode_number, 0,buffer_auxiliar);

            sprintf(snapchot_name, "snapchot_%d.txt",inode_number);
            strcpy(cale_director, argv[2]); 
            strcat(cale_director, "/");
            strcat(cale_director,snapchot_name);
            
            if(verificare_exista_snapchot_anterior(argv[2],snapchot_name) == 0){//inseamna ca noul snapchot nu exista in directorul de output
                //si atunci il creez
                scriere_snapchot(cale_director,buffer_auxiliar);
                printf("nu exista inainte de acest apel\n");
            }else{
                //compar ce exista deja in snapchot_name cu buffer_auxiliar pe care l am obtinut prin apelarea functiei parcurgere_director(...)
                if( (comparare_snapchot_anterior(cale_director,buffer_auxiliar) == 0) ){//inseamna ca nu s-a facut nicio modificare in snapchot
                    printf("nu s a produs nicio modificare fata de snapchotul anterior\n");
                    //continue;
                }
                else{
                    scriere_snapchot(cale_director, buffer_auxiliar);
                    printf("a existat o modificare\n");
                }
                printf("exista deja \n");
            }
            exit(0);
        }
    }

    //proces parinte
    for(int i = 3 ; i < argc ; i++){
        wpid = wait(&status);
        if (wpid == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
        }
        if(WIFEXITED(status)){
        printf("Procesul cu PID %d s-a incheiat cu codul %d\n",wpid,WEXITSTATUS(status));//WEXITSTATUS ne da codul de retur, gen exit(0)
        }
        else{
            printf("Child %d ended abnormally\n", wpid);
        }

    }