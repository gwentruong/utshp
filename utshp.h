#ifndef __UTSHP_H
#define __UTSHP_H


typedef struct record {
    int            num;
    int            len;
    int            shape_type;
    void          *shape;
    struct record *next;
} Record;

int         parse_header(FILE *fp);
void        parse_int32(unsigned char *buf, int *p, int n, int big_endian);
void        parse_double(unsigned char *buf, double *p, int n);
int         record_prepend(Record **p_head, Record *new_record);
void        record_reverse(Record **p_head);
int         record_length(Record *head);
const char *shape_type(int x);

#endif
