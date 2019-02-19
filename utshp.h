#ifndef __UTSHP_H
#define __UTSHP_H

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

#endif
