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
    int record_num;
    int record_len;
    int shape_type;
    PolyLine polyline;
    struct record *next;
} Record;

void        parse_header(FILE *fp);
int         parse_record(FILE *fp);
PolyLine   *parse_polyline(unsigned char *buf);
Point      *parse_points(unsigned char *buf, int num_points);
void        parse_int32(unsigned char *buf, int *p, int n, int big_endian);
void        parse_double(unsigned char *buf, double *p, int n);
const char *print_shape_type(int x);

int main(void)
{
    FILE *fp  = fopen("test.shp", "rb");

    parse_header(fp);

    parse_record(fp);
    // parse_record(fp);
    // while (parse_record(fp) == 1)
    //     continue;

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

int parse_record(FILE *fp)
{
    unsigned char header[8];
    int check = fread(header, sizeof(header), 1, fp);
    if (check != 1)
        return 0;

    int num_len[2];
    parse_int32(header, num_len, 2, 1);
    printf("Record number\t%d\n", num_len[0]);
    printf("Record length\t%d\n", num_len[1] * 2);

    unsigned char content[num_len[1] * 2];
    fread(content, sizeof(content), 1, fp);

    int shape_type;
    parse_int32(content, &shape_type, 1, 0);
    printf("Shape type \t%d (%s)\n", shape_type, print_shape_type(shape_type));

    PolyLine *polyline = parse_polyline(content + 4);

    free(polyline->points);
    free(polyline->parts);
    free(polyline);

    return check;
}

PolyLine *parse_polyline(unsigned char *buf)
{
    PolyLine *polyline = malloc(sizeof(PolyLine));
    int i;

    double box[4];
    parse_double(buf, box, 4);
    printf("Box Xmin, Ymin, Xmax, Ymax\n");
    for (i = 0; i < 4; i++)
    {
        polyline->box[i] = box[i];
        printf("%f\t", polyline->box[i]);
    }
    printf("\n");


    int parts_points[2];
    parse_int32(buf + 32, parts_points, 2, 0);
    polyline->num_parts  = parts_points[0];
    polyline->num_points = parts_points[1];
    printf("NumParts\t%d\n", polyline->num_parts);
    printf("NumPoints\t%d\n", polyline->num_points);

    int parts[polyline->num_parts];
    polyline->parts = malloc(sizeof(int) * polyline->num_parts);
    parse_int32(buf + 40, parts, polyline->num_parts, 0);
    printf("Parts    \t");
    for (i = 0; i < polyline->num_parts; i++)
    {
        polyline->parts[i] = parts[i];
        printf("%d ", polyline->parts[i]);
    }
    printf("\n");

    polyline->points = parse_points(buf + 40 + (4 * polyline->num_parts),
                                    polyline->num_points);
    for (i = 0; i < polyline->num_points; i++)
    {
        printf("Point[%d]\tX %f\t Y %f\n", i, polyline->points[i].x,
                                              polyline->points[i].y);
    }

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
