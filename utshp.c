#include <stdio.h>

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

const char *print_shape_type(int x);
void        parse_int32(unsigned char *buf, int *p, int n, int big_endian);
void        parse_double(unsigned char *buf, double *p, int n);
void        parse_header(FILE *fp);
int         parse_record(FILE *fp);
void        parse_points(unsigned char *buf, int num_points);

int main(void)
{
    FILE *fp  = fopen("test.shp", "rb");

    parse_header(fp);

    while (parse_record(fp) == 1)
        continue;

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
    int i;

    int num_len[2];
    parse_int32(header, num_len, 2, 1);
    printf("Record number\t%d\n", num_len[0]);
    printf("Record length\t%d\n", num_len[1] * 2);

    unsigned char content[num_len[1] * 2];
    fread(content, sizeof(content), 1, fp);

    int shape_type;
    parse_int32(content, &shape_type, 1, 0);
    printf("Shape type \t%d (%s)\n", shape_type, print_shape_type(shape_type));

    double box[4];
    parse_double(content + 4, box, 4);
    printf("Box Xmin, Ymin, Xmax, Ymax\n");
    for (i = 0; i < 4; i++)
        printf("%f\t", box[i]);
    printf("\n");

    int parts_points[2];
    parse_int32(content + 36, parts_points, 2, 0);
    printf("NumParts\t%d\n", parts_points[0]);
    printf("NumPoints\t%d\n", parts_points[1]);

    int parts[parts_points[0]];
    parse_int32(content + 44, parts, parts_points[0], 0);
    printf("Parts    \t");
    for (i = 0; i < parts_points[0]; i++)
        printf("%d ", parts[i]);
    printf("\n");

    parse_points(content + 44 + (4 * parts_points[0]), parts_points[1]);

    return check;
}

void parse_points(unsigned char *buf, int num_points)
{
    for (int i = 0; i < num_points; i++, buf += 16)
    {
        double xy[2];
        parse_double(buf, xy, 2);
        printf("Point[%d]\tX\t%f\tY\t%f\n", i, xy[0], xy[1]);
    }
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
