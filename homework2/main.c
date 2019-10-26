#include "headers/includes.h"
#include "headers/fat.h"
#include "headers/error.h"
#include "headers/command.h"

#define IMAGE "images/a.img"

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
        gets(command);

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
                tree(fat12, &bs, LS_NO_PARAM);
            }
            else
            {
                tree(fat12, &bs, LS_WITH_PARAM);
            }
        }
    }

    return 0;
}
