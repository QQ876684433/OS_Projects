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
            // 将目录路径标志位复位
            INIT_PATH_FLAGS(PATH_DIR_NOT_FOUND);

            if (count == 1) // without parameters
            {
                tree(fat12, &bs, LS_NO_PARAM, "", LS_WITHOUT_CAT);
            }
            else
            {
                int hasParam = 0;
                int isWrongParam = 0;
                char *path = NULL;
                int isTooManyParam = 0;
                for (size_t i = 1; i < count; i++)
                {
                    char *part = parts[i];
                    if (part[0] == '-')
                    {
                        // check if it's -l or -lll...lll
                        for (size_t i = 1; i < strlen(part); i++)
                        {
                            if (part[i] != 'l')
                            {
                                // wrong parameter
                                isWrongParam = 1;
                                break;
                            }
                        }
                        if (isWrongParam == 1)
                        {
                            break;
                        }
                        hasParam = 1;
                    }
                    else
                    {
                        // extract target path
                        if (path == NULL)
                        {
                            path = part;
                        }
                        else
                        {
                            // too many parameters
                            isTooManyParam = 1;
                            break;
                        }
                    }
                }
                if (isWrongParam == 1)
                {
                    LS_WRONG_PARAM();
                }
                else if (isTooManyParam == 1)
                {
                    LS_TOO_MANY_PATH();
                }
                else
                {
                    // now parameters are legal
                    tree(fat12, &bs, hasParam == 1 ? LS_WITH_PARAM : LS_NO_PARAM, path == NULL ? "" : path, LS_WITHOUT_CAT);
                    PATH_FLAGS_CHECK();
                }
            }
        }
        else if (strcmp(parts[0], CAT) == 0)
        {
            // 将文件路径标志位复位
            INIT_PATH_FLAGS(PATH_FILE_NOT_FOUND);

            if (count > 2)
            {
                CAT_TOO_MANY_PARAM();
            }
            else if (count < 2)
            {
                CAT_NOT_ENOUTH_PARAM();
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
                PATH_FLAGS_CHECK();
            }
        }
        else
        {
            WRONG_OPERATION();
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
