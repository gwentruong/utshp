# utshp

A simple `.shp` parser

**To compile**
```
make
```

**To run**
```
./utshp <filename>
```

**Example**
```
$ ./utshp polygon.shp

File code  	9994
File length	3005996
Version    	1000
Shape type 	3 (Polyline)
min X, min Y, max X, max Y, min Z, max Z, min M, max M
377738.277000 6670433.781000 387162.597000 6679349.426000 0.000000 0.000000 0.000000 0.000000

>>> Enter record number ('q' to quit): 2

Record number	2
Record length	80
Shape type 	3 (Polyline)
Box Xmin, Ymin, Xmax, Ymax
381207.676000	6672604.795000	381208.493000	6672606.977000
NumParts	1
NumPoints	2
Parts    	0
Point[0]	X 381207.676000	Y 6672604.795000
Point[1]	X 381208.493000	Y 6672606.977000
```
***

**References**
1. [Shapefile, Wikipedia](https://en.wikipedia.org/wiki/Shapefile)
2. [ERSI Shapefile Technical Description](https://www.esri.com/library/whitepapers/pdfs/shapefile.pdf)
