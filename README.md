##### LOAD - _Load image_
1. path __src__: Path to load original image from
2. path __dst__: Path to save modified image to

##### INST - _Insert image_
1. path __src__: Path to load original image from
2. int __x0__: Top left point x coordinate
3. int __y0__: Top left point y coordinate
4. int __w__: Rectangle width in which image will be inserted
5. int __h__: Rectangle height in which image will be inserted

##### LINE - _Draw line_
1. int __x0__: Starting point x coordinate
2. int __y0__: Starting point y coordinate
3. int __x1__: Ending point x coordinate
4. int __y1__: Ending point y coordinate
5. hex __stroke__: Hexadecimal RGBA color code for stroke

##### CIRC - _Draw circle_
1. int __xc__: Center point x coordinate
2. int __yc__: Center point y coordinate
3. int __r__: Circle radius
4. hex __stroke__: Hexadecimal RGBA color code for stroke
5. hex __fill__: Hexadecimal RGBA color code for fill

##### RECT - _Draw rectangle_
1. int __x0__: Top left point x coordinate
2. int __y0__: Top left point y coordinate
3. int __w__: Rectangle width
4. int __h__: Rectangle height
5. hex __stroke__: Hexadecimal RGBA color code for stroke
6. hex __fill__: Hexadecimal RGBA color code for fill

##### ROTA - _Rotate image_
No arguments

##### FLPV - _Flip image vertically_
No arguments

##### FLPH - _Flip image horizontally_
No arguments

##### EDGE - _Detect edges_
No arguments

##### AUTO - _Auto adjust colors_
No arguments

##### NGTV - _Apply negative effect_
No arguments

##### GRAY - _Apply grayscale effect_
No arguments

##### BLWH - _Apply black and white effect_
1. int __threshold__: Brightness threshold

##### VIGN - _Apply vignette effect_
1. float __xc__: Center point x coordinate
2. float __yc__: Center point y percentage
3. float __factor__: Effect factor

##### BBLR - _Apply box blur effect_
1. int __radius__: Effect radius

##### RBLR - _Apply radial blur effect_
1. int __intensity__: Effect intensity

##### ZBLR - _Apply zoom blur effect_
1. int __intensity__: Effect intensity
