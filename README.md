# steganography
Hiding information in images using C++ and OpenCV.

## Part A
Simple addition of a binary image to a secondary carrier image.

## Part B
The same as above improved with scrambling input image using random number generator initialised with seed (djb2 hashed password).

## Part C
Generating noised images. Random number generator seeded with password (as above).

## Part D
Hiding color images in another color images. Hidden images are scrambled and carrier images noised (using password seed).

## Part E
Hiding file of any format in 3 channel images. Information is additionally passowrd protected (using the same method as in previous examples).
