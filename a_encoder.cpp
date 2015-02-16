// Simple Steganography - encoder
// Usage: program_name carrier message encoded

// Program adds a binary image to a secondary carrier image in order to conceal
// the bit-mapped text message contained in the binary image.

// Author: Marcin Majkowski, m.p.majkowski@cranfield.ac.uk

#include <iostream>
#include <algorithm>
#include <vector>
#include <cv.h>
#include <highgui.h>

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 4) {  // incorrect number of arguments
        cout << "Usage: program_name carrier message encoded" << endl;
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

    // loading message image
    cout << "Loading message image (" << argv[2] << ")... ";
    auto message = Mat_<uchar>(imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE));
    if (!message.data) {
        cout << "Could not open or find " << argv[2] << endl;
        return -1;
    }
    cout << "done" << endl;

    // checking dimension agreement
    cout << "Checking dimension agreement... ";
    if (carrier.size == message.size)
        cout << "done (agreement)" << endl;
    else {
        cout << "done (disagreement)" << endl;
        cout << "Images have different dimension" << endl;
        return -1;
    }

    // generating encoded image
    cout << "Generating encoded image... ";
    // put 1 on every position where value does not equal 0
    threshold(message, message, 0, 1, THRESH_BINARY);
    // change 0 to 1 and 1 to 0
    message = Mat_<uchar>::ones(message.size()) - message;
    Mat_<uchar> encoded = carrier + message;  // overflow doesn't occur,
                                              // 255 + positive value is
                                              // still 255
    cout << "done" << endl;

    // saving generated image
    cout << "Saving encoded image (" << argv[3] << ")... ";
    vector<int> compression_params = {CV_IMWRITE_PNG_COMPRESSION, 9};
    imwrite(argv[3], encoded, compression_params);
    cout << "done" << endl;

    // success
    return 0;
}