#include <stdio.h>

void print_shape_type(int x);
void parse_int32(unsigned char *buf, int *p, int n, int big_endian);
void parse_double(unsigned char *buf, double *p, int n);

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

void print_shape_type(int x)
{
    switch (x)
    {
        case 0:
            printf("Null shape\n");
            break;
        case 1:
            printf("Point\n");
            break;
        case 3:
            printf("Polyline\n");
            break;
        case 5:
            printf("Polygon\n");
            break;
        case 8:
            printf("MultiPoint\n");
            break;
        case 11:
            printf("PointZ\n");
            break;
        case 13:
            printf("PolylineZ\n");
            break;
        case 15:
            printf("PolygonZ\n");
            break;
        case 18:
            printf("MultiPointZ\n");
            break;
        case 21:
            printf("PointM\n");
            break;
        case 23:
            printf("PolylineM\n");
            break;
        case 25:
            printf("PolygonM\n");
            break;
        case 28:
            printf("MultiPointM\n");
            break;
        case 31:
            printf("MultiPatch\n");
            break;
        default:
            printf("Unknown shape type\n");
            break;
    }
}
