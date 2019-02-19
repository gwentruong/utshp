#include <stdio.h>
#include <stdlib.h>

typedef struct {
    double x;
    double y;
} Point;

typedef struct {
    double  box[4];
    int     num_parts;
    int     num_points;
    int    *parts;
    Point  *points;
} PolyLine;

typedef struct record {
    int num;
    int len;
    int shape_type;
    PolyLine *polyline;
    struct record *next;
} Record;

void        parse_header(FILE *fp);
Record     *parse_record(FILE *fp);
int         append(Record **p_head, Record *new_record);
int         length(Record *head);
void        free_record(Record *head);
void        print_nth_record(Record *head, int n);
PolyLine   *parse_polyline(unsigned char *buf);
Point      *parse_points(unsigned char *buf, int num_points);
void        parse_int32(unsigned char *buf, int *p, int n, int big_endian);
void        parse_double(unsigned char *buf, double *p, int n);
const char *print_shape_type(int x);

int main(void)
{
    FILE *fp  = fopen("test.shp", "rb");
    int n;

    parse_header(fp);

    Record *record = NULL;

    while (append(&record, parse_record(fp)) == 1)
        continue;

    printf("Enter record number: ");
    scanf("%d", &n);
    print_nth_record(record, n);

    free_record(record);
    fclose(fp);
}

void parse_header(FILE *fp)
{
    unsigned char header[100];
    fread(header, sizeof(header), 1, fp);

    int code;
    parse_int32(header, &code, 1, 1);
    printf("File code  \t%d\n", code);

    int length;
    parse_int32(header + 24, &length, 1, 1);
    printf("File length\t%d\n", length * 2);

    int vs[2];
    parse_int32(header + 28, vs, 2, 0);
    printf("Version    \t%d\n", vs[0]);
    printf("Shape type \t%d (%s)\n", vs[1], print_shape_type(vs[1]));

    double ranges[8];
    parse_double(header + 36, ranges, 8);
    printf("min X, min Y, max X, max Y, min Z, max Z, min M, max M\n");
    for (int i = 0; i < 8; i++)
        printf("%f ", ranges[i]);
    printf("\n");
}

Record *parse_record(FILE *fp)
{
    unsigned char header[8];
    Record *record = malloc(sizeof(Record));
    int check = fread(header, sizeof(header), 1, fp);
    if (check != 1)
        return NULL;

    int num_len[2];
    parse_int32(header, num_len, 2, 1);
    record->num = num_len[0];
    record->len = num_len[1] * 2;

    unsigned char content[num_len[1] * 2];
    fread(content, sizeof(content), 1, fp);

    int shape_type;
    parse_int32(content, &shape_type, 1, 0);
    record->shape_type = shape_type;

    record->polyline = parse_polyline(content + 4);

    record->next = NULL;

    return record;
}

int append(Record **p_head, Record *new_record)
{
    int result = 1;
    if (new_record == NULL)
        return 0;

    Record *p = *p_head;
    if (p == NULL)
        *p_head = new_record;
    else
    {
        while (p->next != NULL)
            p = p->next;

        p->next = new_record;
        new_record->next = NULL;
    }
    return result;
}

int length(Record *head)
{
    int len = 0;

    for (Record *p = head; p != NULL; p = p->next)
        len++;

    return len;
}

void free_record(Record *head)
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

void print_nth_record(Record *head, int n)
{
    Record *record = head;
    int value = 1;
    int i;

    if (n >= length(head))
        value = 0;
    else
    {
        for (i = 0; i < n - 1; i++)
            record = record->next;
    }

    if (value == 0)
        printf("This record doesn't exist\n");
    else
    {
        printf("Record number\t%d\n", record->num);
        printf("Record length\t%d\n", record->len);
        printf("Shape type \t%d (%s)\n", record->shape_type,
                print_shape_type(record->shape_type));

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
            printf("Point[%d]\tX %f\t Y %f\n", i, record->polyline->points[i].x,
                                                 record->polyline->points[i].y);
        }
    }
}

PolyLine *parse_polyline(unsigned char *buf)
{
    PolyLine *polyline = malloc(sizeof(PolyLine));
    int i;

    double box[4];
    parse_double(buf, box, 4);
    for (i = 0; i < 4; i++)
        polyline->box[i] = box[i];

    int parts_points[2];
    parse_int32(buf + 32, parts_points, 2, 0);
    polyline->num_parts  = parts_points[0];
    polyline->num_points = parts_points[1];

    int parts[polyline->num_parts];
    polyline->parts = malloc(sizeof(int) * polyline->num_parts);
    parse_int32(buf + 40, parts, polyline->num_parts, 0);
    for (i = 0; i < polyline->num_parts; i++)
        polyline->parts[i] = parts[i];

    polyline->points = parse_points(buf + 40 + (4 * polyline->num_parts),
                                    polyline->num_points);

    return polyline;
}

Point *parse_points(unsigned char *buf, int num_points)
{
    Point *points = malloc(sizeof(Point) * num_points);

    for (int i = 0; i < num_points; i++, buf += 16)
    {
        double xy[2];
        parse_double(buf, xy, 2);
        points[i].x = xy[0];
        points[i].y = xy[1];
    }

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

const char *print_shape_type(int type)
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
