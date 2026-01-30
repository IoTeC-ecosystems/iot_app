#include <stdio.h>

#include "can_api.h"

int main(void)
{
    printf("Hello, World!\n");
    can_api_init(1, 2);
    return 0;

}
