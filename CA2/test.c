#include <stdio.h>
#include <string.h>

int main()
{
    char str[255] = "C:\\Users\\Documents";
    char newstr[ 2 * sizeof(str) ]; // As suggested by "chux" - ensure buffer is big enough!
    printf("%s \n", str);
    int i, j;
    for (i = j = 0; i < (int)strlen(str); i++, j++) {
        newstr[j] = str[i];
        if (str[i] == '\\') newstr[++j] = '\\'; // Insert extra backslash
    }
    newstr[j] = '\0'; // We need to add nul-terminator!
    printf("%s", newstr);
    return 0;
}