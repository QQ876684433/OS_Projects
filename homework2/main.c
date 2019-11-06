#include "headers/includes.h"
#include "headers/fat.h"
#include "headers/error.h"
#include "headers/command.h"

#define IMAGE "images/a.img"

char *mygets(char *buf, size_t size);

int main(int argc, char const *argv[])
{
    FILE *fat12 = fopen(IMAGE, "rb");
    struct BootSector bs;

    loadBootSector(fat12, &bs);
    showBootSectorInfo(&bs);

    while (1)
    {
        printf(">");
        char command[100];
        // scanf("%s", command);
        // gets(command);
        mygets(command, sizeof command);

        char *parts[100];
        int count = splits(command, parts, ' ');
        if (count == 0)
        {
            continue;
        }

        if (count == 1 && strcmp(parts[0], EXIT) == 0)
        {
            exit(0);
        }
        else if (strcmp(parts[0], LS) == 0) // ls
        {
            if (count == 1) // without parameters
            {
                tree(fat12, &bs, LS_NO_PARAM, "", LS_WITHOUT_CAT);
            }
            else
            {
                tree(fat12, &bs, LS_WITH_PARAM, parts[1], LS_WITHOUT_CAT);
            }
        }
        else if (strcmp(parts[0], CAT) == 0)
        {
            if (count > 2)
            {
                printf("Two many parameters! \n");
            }
            else if (count < 2)
            {
                printf("No enough parameters! \n");
            }
            else
            {
                char *path = (char *)malloc(100);
                memset(path, 0, 100);
                if (parts[1][0] != '/')
                {
                    path[0] = '/';
                }
                strcat(path, parts[1]);
                tree(fat12, &bs, LS_NO_PARAM, path, LS_WITH_CAT);
            }
        }
        else
        {
            printf("Wrong operation!\n");
        }
    }

    return 0;
}

char *mygets(char *buf, size_t size)
{
    if (buf != NULL && size > 0)
    {
        if (fgets(buf, size, stdin))
        {
            buf[strcspn(buf, "\n")] = '\0';
            return buf;
        }
        *buf = '\0'; /* clear buffer at end of file */
    }
    return NULL;
}
