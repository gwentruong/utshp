#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "utshp.h"

typedef struct point {
    double x;
    double y;
} Point;

typedef struct pointM {
    double x;
    double y;
    double m;
} PointM;

typedef struct polyline {
    double  box[4];
    int     num_parts;
    int     num_points;
    int    *parts;
    Point  *points;
} PolyLine;

Record   *parse_record(FILE *fp);
PolyLine *parse_polyline(unsigned char *buf);
Point    *parse_points(unsigned char *buf, int num_points);
PointM   *parse_pointM(unsigned char *buf);
int      *parse_parts(unsigned char *buf, int num_parts);
void      record_nth_print(Record *head, int n);
void      record_free(Record *head, int shape_type);

int main(int argc, char **argv)
{
    char *filename = argv[1];
    FILE   *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("Filename not found\n");
        return -1;
    }

    Record *record = NULL;
    char    buf[256];
    int     n;
    int     len = 0;

    int shape_type = parse_header(fp);

    while (record_prepend(&record, parse_record(fp)) == 0)
        len++;
    record_reverse(&record);
    printf("Total number of record\t%d\n", len);

    while (1)
    {
        printf("\n>>> Enter record number ('q' to quit): ");
        scanf("%s", buf);
        n = atoi(buf);
        if (n == 0)
            break;
        record_nth_print(record, n);
    }

    record_free(record, shape_type);
    fclose(fp);
}

Record *parse_record(FILE *fp)
{
    unsigned char header[8];
    int           num_len[2];

    int nitems = fread(header, sizeof(header), 1, fp);
    if (nitems != 1)
        return NULL;

    parse_int32(header, num_len, 2, 1);             // Parse record number, length

    unsigned char content[num_len[1] * 2];
    int           shape_type;

    fread(content, sizeof(content), 1, fp);         // Read content of record
    parse_int32(content, &shape_type, 1, 0);        // Parse shapetype

    Record *record     = malloc(sizeof(Record));
    record->num        = num_len[0];
    record->len        = num_len[1] * 2;
    record->shape_type = shape_type;
    record->next       = NULL;

    switch (shape_type)
    {
        case 1:
            record->shape = parse_points(content + 4, 1);   // Parse Point
            break;
        case 3:
        case 5:                                             // Parse Polygon
            record->shape = parse_polyline(content + 4);    // Parse PolyLine
            break;
        case 21:
            record->shape = parse_pointM(content + 4);      // Parse PointM
            break;
        default:
            printf("Unknown shape type\n");
            break;
    }

    return record;
}

PolyLine *parse_polyline(unsigned char *buf)
{
    PolyLine *polyline = malloc(sizeof(PolyLine));
    double    box[4];
    int       parts_points[2];
    int       i;

    parse_double(buf, box, 4);
    parse_int32(buf + 32, parts_points, 2, 0);

    for (i = 0; i < 4; i++)
        polyline->box[i] = box[i];
    polyline->num_parts  = parts_points[0];
    polyline->num_points = parts_points[1];
    polyline->parts      = parse_parts(buf + 40, polyline->num_parts);
    polyline->points     = parse_points(buf + 40 + (4 * polyline->num_parts),
                                        polyline->num_points);

    return polyline;
}

int *parse_parts(unsigned char *buf, int num_parts)
{
    int *parts = malloc(sizeof(int) * num_parts);
    parse_int32(buf, parts, num_parts, 0);

    return parts;
}

Point *parse_points(unsigned char *buf, int num_points)
{
    Point *points = malloc(sizeof(Point) * num_points);
#if 1
    // Almost safe. Not exactly portable: only works on 64-bit machines
    parse_double(buf, (double *)points, num_points * 2);
#else
    for (int i = 0; i < num_points; i++, buf += 16)
    {
        double xy[2];
        parse_double(buf, xy, 2);
        points[i].x = xy[0];
        points[i].y = xy[1];
    }
#endif
    return points;
}

PointM *parse_pointM(unsigned char *buf)
{
    PointM *pointm = malloc(sizeof(PointM));
#if 1
    // Only works on 64-bit machines
    parse_double(buf, (double *)pointm, 3);
#else
    double xym[3];
    parse_double(buf, xym, 3);
    pointm->x = xym[0];
    pointm->y = xym[1];
    pointm->m = xym[2];
#endif
    return pointm;
}

void record_nth_print(Record *head, int n)
{
    Record   *record = head;
    Point    *point;
    PolyLine *polyline;
    PointM   *pointm;
    int       i;

    if (n >= record_length(head))
    {
        printf("This record doesn't exist\n");
        return;
    }

    for (i = 0; i < n - 1; i++)
        record = record->next;

    printf("\nRecord number\t%d\n", record->num);
    printf("Record length\t%d\n", record->len);
    printf("Shape type \t%d (%s)\n", record->shape_type,
                            shape_type(record->shape_type));

    switch (record->shape_type)
    {
        case 1:
            point = record->shape;
            printf("Point\t X %f\t Y %f\n", point->x, point->y);
            break;
        case 3:
        case 5:
            polyline = record->shape;
            printf("Box Xmin, Ymin, Xmax, Ymax\n");
            for (i = 0; i < 4; i++)
                printf("%f\t", polyline->box[i]);
            printf("\n");

            printf("NumParts\t%d\n", polyline->num_parts);
            printf("NumPoints\t%d\n", polyline->num_points);

            printf("Parts    \t");
            for (i = 0; i < polyline->num_parts; i++)
                printf("%d ", polyline->parts[i]);
            printf("\n");

            for (i = 0; i < polyline->num_points; i++)
            {
                printf("Point[%d]\tX %f\tY %f\n", i, polyline->points[i].x,
                                                     polyline->points[i].y);
            }
            break;
        case 21:
            pointm = record->shape;
            if (pointm->m < - pow(10, 38))
                printf("PointM\tX %f\t\tY %f\t M NaN\n", pointm->x, pointm->y);
            else
            {
                printf("PointM\tX %f\t\tY %f\tM %f\n", pointm->x, pointm->y,
                                                       pointm->m);
            }
            break;
    }
}

void record_free(Record *head, int shape_type)
{
    PolyLine *polyline;

    for (Record *p = head; p != NULL; p = head)
    {
        head = head->next;
        switch (shape_type)
        {
            case 1:
            case 21:
                free(p->shape);
                free(p);
                break;
            case 3:
            case 5:
                polyline = p->shape;
                free(polyline->points);
                free(polyline->parts);
                free(polyline);
                free(p);
                break;
        }
    }
}
