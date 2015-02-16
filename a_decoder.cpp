// Simple Steganography - decoder
// Usage: program_name carrier encoded decoded

// Program recovers the original text from an encoded image produced with
// encoder.

// Author: Marcin Majkowski, m.p.majkowski@cranfield.ac.uk

// testing.png message:
//
// This is a secret message from a secret person.
// If you can see this message then your program works
// Well done - on the next part of the practical ......
//
// Cranfield University - AMAC, SOE, DIP 1 Practical

#include <iostream>
#include <vector>
#include <cv.h>
#include <highgui.h>

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 4) {  // incorrect number of arguments
        cout << "Usage: program_name carrier encoded decoded" << endl;
        return -1;
    }

    // loading carrier image
    cout << "Loading carrier image (" << argv[1] << ")... ";
    auto carrier = Mat_<uchar>(imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE));
    if (!carrier.data) {
        cout << "Could not open or find " << argv[1] << endl;
        return -1;
    }
    cout << "done" << endl;

    // loading encoded image
    cout << "Loading encoded image (" << argv[2] << ")... ";
    auto encoded = Mat_<uchar>(imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE));
    if (!encoded.data) {
        cout << "Could not open or find " << argv[2] << endl;
        return -1;
    }
    cout << "done" << endl;

    // checking dimension agreement
    cout << "Checking dimension agreement... ";
    if (carrier.size == encoded.size)
        cout << "done (agreement)" << endl;
    else {
        cout << "done (disagreement)" << endl;
        cout << "Images have different dimension" << endl;
        return -1;
    }

    // generating decoded image
    cout << "Generating decoded image... ";
    Mat_<uchar> decoded = Mat_<uchar>::ones(carrier.size());
    // put 0 where (encoded - carrier) equals 1 and 1 where it equals 0
    decoded -= encoded - carrier;
    decoded *= 255;
    cout << "done" << endl;

    // saving generated image
    cout << "Saving decoded image (" << argv[3] << ")... ";
    vector<int> compression_params = {CV_IMWRITE_PNG_COMPRESSION, 9};
    imwrite(argv[3], decoded, compression_params);
    cout << "decoded" << endl;

    // success
    return 0;
}