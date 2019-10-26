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
        scanf("%s", command);
        if (strcmp(command, EXIT) == 0)
        {
            exit(0);
        }
        else if (strcmp(command, LS) == 0)
        {
            tree(fat12, &bs);
        }
    }

    return 0;
}
