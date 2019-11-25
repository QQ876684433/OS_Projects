#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    char str[] = "subseasrchtring earching algorithm sesarch";
    char pattern[] = "s";
    int res = 0;

    int index = 0, str_len = strlen(str), pat_len = strlen(pattern);
    while (index < str_len)
    {
        int isFound = 1;
        for (int i = 0; i < pat_len; i++)
        {
            if (str[i + index] != pattern[i])
            {
                isFound = 0;
                break;
            }
        }
        if (isFound == 1)
        {
            res = 1;
            printf("%d ", index);
            index += pat_len;
        }
        else
        {
            if (index + pat_len >= str_len)
            {
                break;
            }
            else
            {
                char ch = str[index + pat_len];
                int j;
                for (j = pat_len - 1; j >= 0; j--)
                {
                    if (ch == pattern[j])
                    {
                        break;
                    }
                }
                if (j < 0)
                {
                    // 匹配串下一个字符不在模式串中
                    index += (pat_len + 1);
                }
                else
                {
                    index += (pat_len - j);
                }
            }
        }
    }
    if (res == 0)
    {
        printf("Not Found!");
    }
    else
        printf("\n");
    return 0;
}
