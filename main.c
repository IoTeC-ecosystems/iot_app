#include <stdio.h>

#include "can_api.h"
#include "sim_api.h"

int main(void)
{
    printf("Hello, World!\n");
    can_api_init(1, 2);
    sim_api_init();
    return 0;
}
