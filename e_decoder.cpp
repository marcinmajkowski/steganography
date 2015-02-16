// General Information Hiding - decoder
// Usage: program_name carrier encoded decoded

// Description
// This program uses user password seeded random number generator to decode
// file hidden in noised 3-channel encoded image produced with corresponding
// encoder.

// Program is able to notice wrong password input, therefore cannot produce
// invalid output file.

// Author: Marcin Majkowski, m.p.majkowski@cranfield.ac.uk

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <memory>

#include <cv.h>
#include <highgui.h>

using namespace cv;
using namespace std;

unsigned long hash_djb2(const char* str);
template <typename T>
inline bool get_bit(T& var, unsigned n);
template <typename T>
inline void set_bit(T& var, unsigned bit_index, bool value = true);
void add_gaussian_noise(Mat_<Vec3b>& src, Mat_<Vec3b>& dst, double sigma,
                        RNG& rng);

int main(int argc, char* argv[])
{
    if (argc != 4) {  // incorrect number of arguments
        cout << "Usage: program_name carrier encoded decoded" << endl;
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

    // loading encoded image
    cout << "Loading encoded image (" << argv[2] << ")... ";
    auto encoded = Mat_<Vec3b>{};
    if (!(encoded = imread(argv[2])).data) {
        cout << "Could not open or find " << argv[2] << endl;
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
    RNG rng(seed);

    // adding Gaussian noise to the carrier image
    cout << "Adding Gaussian noise to the carrier image... ";
    double sigma = 5;
    Mat_<Vec3b> noised;
    add_gaussian_noise(carrier, noised, sigma, rng);
    // noised = carrier.clone();
    cout << "done" << endl;

    // counting number of slots in noised carrier image
    cout << "Counting number of free slots in noised carrier image... ";
    auto slots =
        vector<Vec3i>(noised.cols * noised.rows * 3);  // all carrier image free
                                                       // slots indexes will be
                                                       // stored in this vector
    auto slots_it = slots.begin();
    for (int i = 0; i < noised.rows; ++i)
        for (int j = 0; j < noised.cols; ++j)
            for (int b = 0; b < 3; ++b)
                if (noised.at<Vec3b>(i, j)[b] < 255)
                    *(slots_it++) = Vec3i({i, j, b});
    slots.erase(slots_it,
                slots.end());  // now slots.size() is a number of free slots
    cout << "done (" << slots.size() << " slots)" << endl;

    // random shuffling vector of slots in carrier image
    cout << "Shuffling a vector of free slots... ";
    random_shuffle(slots.begin(), slots.end(), rng);
    cout << "done" << endl;

    // reading seed variable (for password checking)
    cout << "Reading seed variable (for password checking)... ";
    auto decoded_seed = seed;
    int slot_index = 0;
    for (int i = 0; i < sizeof(seed) * 8; ++i) {
        Vec3i slot = slots[slot_index++];
        int row = slot[0];
        int col = slot[1];
        int channel = slot[2];
        bool bit = encoded.at<Vec3b>(row, col)[channel] -
                   noised.at<Vec3b>(row, col)[channel];
        set_bit(seed, i, bit);
    }
    if (seed == decoded_seed)
        cout << "done (agreement)" << endl;
    else {
        cout << "done (disagreement)" << endl;
        cout << "Wrong password" << endl;
        return -1;
    }

    // reading message file size
    cout << "Reading message file size... ";
    int32_t file_size = 0;
    for (int i = 0; i < 32; ++i) {
        Vec3i slot = slots[slot_index++];
        int row = slot[0];
        int col = slot[1];
        int channel = slot[2];
        bool bit = encoded.at<Vec3b>(row, col)[channel] -
                   noised.at<Vec3b>(row, col)[channel];
        set_bit(file_size, i, bit);
    }
    cout << "done (" << file_size * 8 << " bits)" << endl;

    // reading message bits
    cout << "Reading message bits distributed over carrier image bytes... ";
    auto memblock = unique_ptr<char[]>(new char[file_size]);
    for (int i = 0; i < file_size; ++i) {
        for (int j = 0; j < 8; ++j) {
            Vec3i slot = slots[slot_index++];
            int row = slot[0];
            int col = slot[1];
            int channel = slot[2];
            bool bit = encoded.at<Vec3b>(row, col)[channel] -
                       noised.at<Vec3b>(row, col)[channel];
            set_bit(char(memblock[i]), j, bit);
        }
    }
    cout << "done" << endl;

    // saving decoded message
    cout << "Saving decoded message (" << argv[3] << ")... ";
    auto file = ofstream(argv[3], ios::binary | ios::trunc);
    if (!file.is_open()) {
        cout << "Could not open or find " << argv[3] << endl;
        return -1;
    }
    file.write(memblock.get(), file_size);
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

template <typename T>
inline bool get_bit(T& var, unsigned n)
{
    /*
    unsigned which_byte = sizeof(T) - n / 8 - 1; // counting from right
    char byte = ((char*)&var)[which_byte];
    unsigned which_bit_in_byte = n % 8; // counting from right

    return 1 & (byte >> (which_bit_in_byte));
    */

    return 1 & (((char*)&var)[sizeof(T) - n / 8 - 1] >> (n % 8));
}

template <typename T>
inline void set_bit(T& var, unsigned bit_index, bool value)
{
    /*
    unsigned which_bit_in_byte = bit_index % 8; // counting from right
    unsigned which_byte = sizeof(T) - bit_index / 8 - 1; // counting from right

    if (value)
        ((char*)&var)[which_byte] |= 1 << which_bit_in_byte;
    else
        ((char*)&var)[which_byte] &= ~(1 << which_bit_in_byte);
    */

    ((char*)&var)[sizeof(T) - bit_index / 8 - 1] =
        value
            ? ((char*)&var)[sizeof(T) - bit_index / 8 - 1] |
                  (1 << bit_index % 8)
            : ((char*)&var)[sizeof(T) - bit_index / 8 - 1] &
                  (~(1 << bit_index % 8));
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
