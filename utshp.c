#include <stdio.h>
#include <stdlib.h>

typedef struct point {
    double x;
    double y;
} Point;

typedef struct polyline {
    double  box[4];
    int     num_parts;
    int     num_points;
    int    *parts;
    Point  *points;
} PolyLine;

typedef struct record {
    int            num;
    int            len;
    int            shape_type;
    PolyLine      *polyline;
    struct record *next;
} Record;

void        parse_header(FILE *fp);
Record     *parse_record(FILE *fp);
PolyLine   *parse_polyline(unsigned char *buf);
Point      *parse_points(unsigned char *buf, int num_points);
void        parse_int32(unsigned char *buf, int *p, int n, int big_endian);
void        parse_double(unsigned char *buf, double *p, int n);
int         record_prepend(Record **p_head, Record *new_record);
void        record_reverse(Record **p_head);
int         record_length(Record *head);
void        record_free(Record *head);
void        record_nth_print(Record *head, int n);
const char *shape_type(int x);

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

void parse_header(FILE *fp)
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
    record->polyline   = parse_polyline(content + 4); // Parse PolyLine
    record->next       = NULL;

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
    polyline->parts      = malloc(sizeof(int) * polyline->num_parts);
    parse_int32(buf + 40, polyline->parts, polyline->num_parts, 0);
    polyline->points     = parse_points(buf + 40 + (4 * polyline->num_parts),
                                        polyline->num_points);

    return polyline;
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

void record_free(Record *head)
{
    for (Record *p = head; p != NULL; p = head)
    {
        head = head->next;
        free(p->polyline->points);
        free(p->polyline->parts);
        free(p->polyline);
        free(p);
    }
}

void record_nth_print(Record *head, int n)
{
    Record *record = head;
    int     i;

    if (n >= record_length(head))
    {
        printf("This record doesn't exist\n");
        return;
    }
    else
    {
        for (i = 0; i < n - 1; i++)
            record = record->next;
    }

    printf("Record number\t%d\n", record->num);
    printf("Record length\t%d\n", record->len);
    printf("Shape type \t%d (%s)\n", record->shape_type,
            shape_type(record->shape_type));

    printf("Box Xmin, Ymin, Xmax, Ymax\n");
    for (i = 0; i < 4; i++)
        printf("%f\t", record->polyline->box[i]);
    printf("\n");

    printf("NumParts\t%d\n", record->polyline->num_parts);
    printf("NumPoints\t%d\n", record->polyline->num_points);

    printf("Parts    \t");
    for (i = 0; i < record->polyline->num_parts; i++)
        printf("%d ", record->polyline->parts[i]);
    printf("\n");

    for (i = 0; i < record->polyline->num_points; i++)
    {
        printf("Point[%d]\tX %f\tY %f\n", i, record->polyline->points[i].x,
                                             record->polyline->points[i].y);
    }
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
