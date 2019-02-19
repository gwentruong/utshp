#include <stdio.h>
#include <stdlib.h>
#include "utshp.h"

int main(void)
{
    FILE   *fp = fopen("test.shp", "rb");
    Record *record = NULL;
    char    buf[256];
    int     n;

    parse_header(fp);

    while (record_prepend(&record, parse_record(fp)) == 0)
        ;
    record_reverse(&record);

    while (1)
    {
        printf("Enter record number ('q' to quit): ");
        scanf("%s", buf);
        n = atoi(buf);
        if (n == 0)
            break;
        record_nth_print(record, n);
    }

    record_free(record);
    fclose(fp);
}
