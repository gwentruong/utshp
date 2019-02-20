# utshp

A simple `.shp` parser

**To install**
```
make
```

**To run**
```
./main <filename>
```

**Example**
```
$ ./main polygon.shp
> File code  	9994
File length	2206048
Version    	1000
Shape type 	5 (Polygon)
min X, min Y, max X, max Y, min Z, max Z, min M, max M
377738.277000 6670433.781000 387162.597000 6679349.426000 0.000000 0.000000 0.000000 0.000000
> Enter record number ('q' to quit): 888
> Record number	888
Record length	144
Shape type 	5 (Polygon)
Box Xmin, Ymin, Xmax, Ymax
384903.291000	6673012.288000	384935.422000	6673037.350000
NumParts	1
NumPoints	6
Parts    	0
Point[0]	X 384931.669000	Y 6673037.350000
Point[1]	X 384934.671000	Y 6673021.638000
Point[2]	X 384935.422000	Y 6673017.711000
Point[3]	X 384907.043000	Y 6673012.288000
Point[4]	X 384903.291000	Y 6673031.926000
Point[5]	X 384931.669000	Y 6673037.350000
```
