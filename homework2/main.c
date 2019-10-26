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

    tree(fat12, &bs);
    return 0;
}
