#include <stdio.h>
#include <string.h>
#include "user_code.h"

/*
    LEVENSHTEIN DISTANCE
    
    NORMAL RUN:
        - Binary size: 3952 Byte
        - Execution time: 0.0858006477355957s

    SECURE RUN:
        - Binary size: 4448 Byte
        - Execution time: 0.0860896110534668s
        - Instrumentation time: 0.0025475025177001953s
        - Simulation time: 0s (no indirect jumps)
        - CFG extraction time: 0.0016362667083740234s
*/

#define MAX_LEN 28

int min(int a, int b, int c)
{
    if (a <= b && a <= c)
        return a;
    if (b <= a && b <= c)
        return b;
    return c;
}

int levenshtein(const char *s1, const char *s2)
{
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);

    int dp[MAX_LEN + 1][MAX_LEN + 1];

    for (size_t i = 0; i <= len1; i++)
        dp[i][0] = (int)i;
    for (size_t j = 0; j <= len2; j++)
        dp[0][j] = (int)j;

    for (size_t i = 1; i <= len1; i++)
    {
        for (size_t j = 1; j <= len2; j++)
        {
            if (s1[i - 1] == s2[j - 1])
            {
                dp[i][j] = dp[i - 1][j - 1];
            }
            else
            {
                dp[i][j] = min(dp[i - 1][j] + 1,
                               dp[i][j - 1] + 1,
                               dp[i - 1][j - 1] + 1);
            }
        }
    }

    return dp[len1][len2];
}

void start_u_code()
{
    const char *str1 = "abcdefghijklmnopqrstuvwxyz";
    const char *str2 = "abcdefghijklzyxwvutsrqponm";

    int distance = levenshtein(str1, str2);

    printf("Levenshtein distance between '%s' and '%s': %d\n", str1, str2, distance);
}
