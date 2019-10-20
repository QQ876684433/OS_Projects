#include <iostream>
#include <climits>
using namespace std;
int main(int argc, char const *argv[])
{
    int a = 0x12345678;
    unsigned char *p = (unsigned char *)&a;
    if (0x78 == *p)
    {
        printf("little end\n");
    }
    else
    {
        printf("big end\n");
    }
    for (size_t i = 0; i < 4; i++)
    {
        cout << hex;
        cout << int(*(p + i));
        cout << dec;
        cout << " ";
    }
    cout << endl;
    return 0;
}
