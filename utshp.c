#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "utshp.h"

int parse_header(FILE *fp)
{
    unsigned char header[100];
    int           code;
    int           length;
    int           vs[2];
    double        ranges[8];

    fread(header, sizeof(header), 1, fp);       // Read header
    parse_int32(header, &code, 1, 1);           // Parse file code, int
    parse_int32(header + 24, &length, 1, 1);    // Parse file length, int
    parse_int32(header + 28, vs, 2, 0);         // Parse version, shapetype
    parse_double(header + 36, ranges, 8);       // Parse boundary ranges

    printf("File code  \t%d\n", code);
    printf("File length\t%d\n", length * 2);
    printf("Version    \t%d\n", vs[0]);
    printf("Shape type \t%d (%s)\n", vs[1], shape_type(vs[1]));
    printf("min X, min Y, max X, max Y, min Z, max Z, min M, max M\n");
    for (int i = 0; i < 8; i++)
        printf("%f ", ranges[i]);
    printf("\n");

    return vs[1];
}

void parse_int32(unsigned char *buf, int *p, int n, int big_endian)
{
    if (big_endian == 0)
    {
        int *q = (int *)buf;
        for (int i = 0; i < n; i++, q++)
            p[i] = *q;
    }
    else
    {
        for (int k = 0; k < n; k++, buf += 4)
        {
            unsigned char b[4];
            for (int i = 0, j = 3; i < 4; i++, j--)
                b[j] = buf[i];
            int *q = (int *)b;
            p[k] = *q;
        }
    }
}

void parse_double(unsigned char *buf, double *p, int n)
{
    double *q = (double *)buf;
    for (int i = 0; i < n; i++, q++)
        p[i] = *q;
}

int record_prepend(Record **p_head, Record *new_record)
{
    if (new_record == NULL)
        return -1;

    new_record->next = *p_head;
    *p_head = new_record;

    return 0;
}

void record_reverse(Record **p_head)
{
    Record *prev = NULL;
    Record *current = *p_head;
    Record *next;

    while (current != NULL)
    {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    *p_head = prev;
}

int record_length(Record *head)
{
    int len = 0;

    for (Record *p = head; p != NULL; p = p->next)
        len++;

    return len;
}

const char *shape_type(int type)
{
    switch (type)
    {
        case 0:
            return "Null shape";
        case 1:
            return "Point";
        case 3:
            return "Polyline";
        case 5:
            return "Polygon";
        case 8:
            return "MultiPoint";
        case 11:
            return "PointZ";
        case 13:
            return "PolylineZ";
        case 15:
            return "PolygonZ";
        case 18:
            return "MultiPointZ";
        case 21:
            return "PointM";
        case 23:
            return "PolylineM";
        case 25:
            return "PolygonM";
        case 28:
            return "MultiPointM";
        case 31:
            return "MultiPatch";
        default:
            return "Unknown shape type";
    }
}
