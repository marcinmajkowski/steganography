// Scrambling the Signal - decoder
// Usage: program_name carrier encoded decoded

// Description
// This program uses user password seeded random number generator to decode
// message hidden in encoded image produced with corresponding encoder.

// Author: Marcin Majkowski, m.p.majkowski@cranfield.ac.uk

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cv.h>
#include <highgui.h>

using namespace cv;
using namespace std;

unsigned long hash_djb2(const char* str);

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

    // prompting user for a character string password
    std::cout << "Input password: ";
    std::string password;
    getline(std::cin, password);

    // transforming password string to a 64-bit integer seed (with hash
    // function)
    auto seed = hash_djb2(password.c_str());

    // generating a random shuffled vector of increasing indexes for all of the
    // points in an image
    cout << "Generating shuffled vector of point indexes... ";
    std::vector<int> indexes(encoded.cols * encoded.rows);
    {  // I use block to limit scope of i
        int i = 0;
        for (auto& index : indexes)
            index = i++;
    }
    RNG rng(seed);
    random_shuffle(indexes.begin(), indexes.end(), rng);
    cout << "done" << endl;

    // generating encoded image
    cout << "Generating encoded image... ";
    auto decoded = carrier.clone();
    auto carrier_pixel = *(carrier.begin());
    auto encoded_pixel = *(encoded.begin());
    {  // I use block to limit scope of i
        int i = 0;
        for (auto& pixel : decoded) {
            carrier_pixel = *(carrier.begin() + indexes[i]);
            encoded_pixel = *(encoded.begin() + indexes[i]);
            pixel = (encoded_pixel - carrier_pixel) ? 0 : 255;
            ++i;
        }
    }
    cout << "done" << endl;

    // saving generated image
    cout << "Saving decoded image (" << argv[3] << ")... ";
    vector<int> compression_params = {CV_IMWRITE_PNG_COMPRESSION, 9};
    imwrite(argv[3], decoded, compression_params);
    cout << "done" << endl;

    // success
    return 0;
}

// from http://www.cse.yorku.ca/~oz/hash.html
unsigned long hash_djb2(const char* str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
