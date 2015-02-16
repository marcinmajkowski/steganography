// Extending to Colour Images - encoder
// Usage: program_name carrier message encoded

// Description
// This program uses user password seeded random number generator to hide
// consequtive bits of binary message image within randomly chosen bytes of
// noised 3-channel carrier image.

// Author: Marcin Majkowski, m.p.majkowski@cranfield.ac.uk

#include <iostream>
#include <vector>
#include <string>
#include <cv.h>
#include <highgui.h>

using namespace cv;
using namespace std;

unsigned long hash_djb2(const char* str);
void add_gaussian_noise(Mat_<Vec3b>& src, Mat_<Vec3b>& dst, double sigma,
                        RNG& rng);

int main(int argc, char* argv[])
{
    if (argc != 4) {  // incorrect number of arguments
        cout << "Usage: program_name carrier message encoded" << endl;
        return -1;
    }

    // loading carrier image
    cout << "Loading carrier image (" << argv[1] << ")... ";
    auto carrier = Mat_<Vec3b>{};
    if (!(carrier = imread(argv[1])).data) {
        cout << "Could not open or find " << argv[1] << endl;
        return -1;
    }
    cout << "done" << endl;

    // loading message image
    cout << "Loading message image (" << argv[2] << ")... ";
    auto message = Mat_<uchar>{};
    if (!(message = imread(argv[2], IMREAD_GRAYSCALE)).data) {
        cout << "Could not open or find " << argv[2] << endl;
        return -1;
    }
    cout << "done" << endl;

    // checking dimensions agreement
    if (carrier.size() != message.size()) {
        cout << "Images have different dimensions" << endl;
        return -1;
    }

    // prompting user for a character string password
    cout << "Input password: ";
    string password;
    getline(cin, password);

    // transforming password string to a 64-bit integer seed (with hash
    // function)
    auto seed = hash_djb2(password.c_str());
    RNG rng(seed);

    // adding Gaussian noise to the carrier image
    cout << "Adding Gaussian noise to the carrier image... ";
    double sigma = 5;
    Mat_<Vec3b> noised;
    add_gaussian_noise(carrier, noised, sigma, rng);
    // noised = carrier.clone();
    cout << "done" << endl;

    // creating a matrix to hold a state of every byte in carrier image
    enum Used { NOT_USED, USED };
    Mat_<Vec3b> state(carrier.size(), Vec3b::all(NOT_USED));

    // distributing message bits over the three colour carrier image chanels
    // iterating through each location in the message image
	cout << "Distributing message bits over carrier image bytes... ";
    auto encoded = carrier.clone();
	for (auto& pixel : message) {
        // using the password seeded random number generator to select a random
        // location in the carrier image (must be not used before and lower than
        // 255)
        uchar* location;
        int col, row, element;
        do {
            row = rng(carrier.rows);
            col = rng(carrier.cols);
            element = rng(3);
            location = &carrier.at<Vec3b>(row, col)[element];
        } while (state.at<Vec3b>(row, col)[element] == USED ||
                 *location == 255);
        // encoding message image bit
        encoded.at<Vec3b>(row, col)[element] += pixel ? 0 : 1;
        state.at<Vec3b>(row, col)[element] = USED;
    }
	cout << "done" << endl;

    // saving generated image
    cout << "Saving encoded image (" << argv[3] << ")... ";
	vector<int> compression_params = {CV_IMWRITE_PNG_COMPRESSION, 9};
    imwrite(argv[3], encoded, compression_params);
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

void add_gaussian_noise(Mat_<Vec3b>& src, Mat_<Vec3b>& dst, double sigma,
                        RNG& rng)
{
    dst = src.clone();
    int noised_value;
    for (auto& pixel : dst)
        for (auto i : {0, 1, 2}) {
            noised_value = rng.gaussian(sigma) + pixel[i];
            if (noised_value > 255)  // preventing overflow
                pixel[i] = 255;
            else if (noised_value < 0)
                pixel[i] = 0;
            else
                pixel[i] = noised_value;
        }
}
