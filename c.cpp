// Generating Noise Images
// Usage: program_name carrier output

// Description
// This program outputs a version of a given specific input image with noise
// added according to a Gaussian distribution with sigma value 10.

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
    if (argc != 3) {  // incorrect number of arguments
        cout << "Usage: program_name carrier output" << endl;
        return -1;
    }

    // loading image
    cout << "Loading carrier image (" << argv[1] << ")... ";
    auto image = Mat_<Vec3b>{};
    if (!(image = imread(argv[1])).data) {
        cout << "Could not open or find " << argv[1] << endl;
        return -1;
    }
    cout << "done" << endl;

    // prompting user for a character string password
    cout << "Input password: ";
    string password;
    getline(cin, password);

    // transforming password string to a 64-bit integer seed (with hash
    // function)
    auto seed = hash_djb2(password.c_str());

    // Gaussian Noise is the most commonly used noise model in image processing
    // and effectively describes most random noise encountered in the DIP
    // pipeline. Also known as additive noise.
    // (dip_notes2014.pdf)

    // adding the Gaussian noise to an image
    cout << "Adding Gaussian noise to the image... ";
    RNG rng(seed);
    auto noised = Mat_<Vec3b>{};
    add_gaussian_noise(image, noised, 10, rng);
	cout << "done" << endl;

    // save noisy image
    cout << "Saving generated image (" << argv[2] << ")... ";
	vector<int> compression_params = {CV_IMWRITE_PNG_COMPRESSION, 9};
    imwrite(argv[2], noised, compression_params);
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
    for (auto& pixel : dst)         // for each pixel
        for (auto i : {0, 1, 2}) {  // for each channel
            noised_value = rng.gaussian(sigma) + pixel[i];
            if (noised_value > 255)  // preventing overflow
                pixel[i] = 255;
            else if (noised_value < 0)
                pixel[i] = 0;
            else
                pixel[i] = noised_value;
        }
}
