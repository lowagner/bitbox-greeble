# bitbox-greeble
a utility for drawing fractals based on quadtrees

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-greeble/master/Greeble.png)

We label a "block" from 0 to 127 (inclusive), and each quadrant of each block can be 
specified as another block (B) or a pure color (C).  There are 0-127 colors, too.
Both blocks and color indices are visually represented using a mixed hexadecimal/octal basis,
i.e. Xx, where X=0-f and x=0-7.

Infinite recursions are eliminated at the lowest resolution by turning the quadrants
labeled as B into C quadrants (using the same index for C as for B).  Thus instead of 
descending into e.g. B05, at the lowest level, it will resolve as C05.
