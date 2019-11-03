### LOAD - _Load image_
1. _path_ __src__: Path to load original image from
2. _path_ __dst__: Path to save modified image to

### INST - _Insert image_
1. _path_ __src__: Path to load original image from
2. _int_ __x0__: Top left point x coordinate
3. _int_ __y0__: Top left point y coordinate
4. _int_ __w__: Rectangle width in which image will be inserted
5. _int_ __h__: Rectangle height in which image will be inserted

### LINE - _Draw line_
1. _int_ __x0__: Starting point x coordinate
2. _int_ __y0__: Starting point y coordinate
3. _int_ __x1__: Ending point x coordinate
4. _int_ __y1__: Ending point y coordinate
5. _hex_ __stroke__: Hexadecimal RGBA color code for stroke

### CIRC - _Draw circle_
1. _int_ __xc__: Center point x coordinate
2. _int_ __yc__: Center point y coordinate
3. _int_ __r__: Circle radius
4. _hex_ __stroke__: Hexadecimal RGBA color code for stroke
5. _hex_ __fill__: Hexadecimal RGBA color code for fill

### RECT - _Draw rectangle_
1. _int_ __x0__: Top left point x coordinate
2. _int_ __y0__: Top left point y coordinate
3. _int_ __w__: Rectangle width
4. _int_ __h__: Rectangle height
5. _hex_ __stroke__: Hexadecimal RGBA color code for stroke
6. _hex_ __fill__: Hexadecimal RGBA color code for fill

### ROTA - _Rotate image_
* _No arguments_

### FLPV - _Flip image vertically_
* _No arguments_

### FLPH - _Flip image horizontally_
* _No arguments_

### EDGE - _Detect edges_
* _No arguments_

### AUTO - _Auto adjust colors_
* _No arguments_

### NGTV - _Apply negative effect_
* _No arguments_

### GRAY - _Apply grayscale effect_
* _No arguments_

### BLWH - _Apply black and white effect_
1. _int_ __threshold__: Brightness threshold

### VIGN - _Apply vignette effect_
1. _float_ __xc__: Center point x coordinate
2. _float_ __yc__: Center point y percentage
3. _float_ __factor__: Effect factor

### BBLR - _Apply box blur effect_
1. _int_ __radius__: Effect radius

### RBLR - _Apply radial blur effect_
1. _float_ __intensity__: Effect intensity

### ZBLR - _Apply zoom blur effect_
1. _float_ __intensity__: Effect intensity

