#include <stdio.h>

void print_shape_type(int x);

int main(void)
{
    unsigned char header[100];
    FILE *fp;
    int   i;
    int   j = 3;
    char  b[4];

    fp = fopen("test.shp","rb");

    fread(header, sizeof(header), 1, fp);

    printf("File code\n");
    for (i = 0; i < 4; i++)
        printf("%02x", header[i]);
    printf("\n");

    printf("Unused\n");
    for (; i < 24; i++)
        printf("%02x", header[i]);
    printf("\n");

    printf("File length\n");
    for (; i < 28; i++)
    {
        printf("%02x", header[i]);
        b[j] = header[i];
        j--;
    }
    printf("\n");
    char *pb = b;
    int  *p  = (int *)pb;
    printf("Int %d\n", *p);
    printf("File size in dec %d\n", *p*2);

    printf("Version\n");
    for (; i < 32; i++)
        printf("%02x", header[i]);
    printf("\n");
    unsigned char *pv = header + 28;
    p = (int *)pv;
    printf("Version %d\n", *p);

    printf("Shape type\n");
    for (; i < 36; i++)
        printf("%02x", header[i]);
    printf("\n");
    pv = header + 32;
    p = (int *)pv;
    print_shape_type(*p);

    printf("Minimum bounding rectangle\n");
    for (; i < 68; i++)
        printf("%02x", header[i]);
    printf("\n");
    j = 0;
    double *pd;
    pv = header + 36 - 8;
    while (j < 4)
    {
        pv = pv + 8;
        pd = (double *)pv;
        printf("%f ", *pd);
        j++;
    }
    printf("\n");

    printf("Range Z\n");
    for (; i < 84; i++)
        printf("%02x", header[i]);
    printf("\n");
    while (j < 6)
    {
        pv = pv + 8;
        pd = (double *)pv;
        printf("%f ", *pd);
        j++;
    }
    printf("\n");

    printf("Range M\n");
    for (; i < 100; i++)
        printf("%02x", header[i]);
    printf("\n");
    while (j < 8)
    {
        pv = pv + 8;
        pd = (double *)pv;
        printf("%f ", *pd);
        j++;
    }
    printf("\n");

    fclose(fp);
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
