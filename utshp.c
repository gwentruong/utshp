#include <stdio.h>

const char *print_shape_type(int x);
void        parse_int32(unsigned char *buf, int *p, int n, int big_endian);
void        parse_double(unsigned char *buf, double *p, int n);

int main(void)
{
    unsigned char header[100];
    FILE *fp = fopen("test.shp","rb");

    fread(header, sizeof(header), 1, fp);

    int code;
    parse_int32(header, &code, 1, 1);
    printf("File code  %d\n", code);

    int length;
    parse_int32(header + 24, &length, 1, 1);
    printf("File length %d\n", length * 2);

    int vs[2];
    parse_int32(header + 28, vs, 2, 0);
    printf("Version %d, Shape type %d\n", vs[0], vs[1]);
    print_shape_type(vs[1]);

    double ranges[8];
    parse_double(header + 36, ranges, 8);
    for (int j = 0; j < 8; j++)
        printf("%f ", ranges[j]);
    printf("\n");

    fclose(fp);
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
