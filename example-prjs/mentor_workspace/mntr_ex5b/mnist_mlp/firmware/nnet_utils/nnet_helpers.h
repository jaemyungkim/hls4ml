//
//    rfnoc-hls-neuralnet: Vivado HLS code for neural-net building blocks
//
//    Copyright (C) 2017 EJ Kreinar
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef NNET_HELPERS_H
#define NNET_HELPERS_H

#ifndef __SYNTHESIS__
#include <fstream>
#include <sstream>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef XLNX_VIVADO_HLS
#include <ac_channel.h>
#else
#include "hls_stream.h"
#endif

namespace nnet {

#ifndef __SYNTHESIS__

#ifndef __WEIGHTS_DIR__
#define WEIGHTS_DIR "weights"
# else
#define STRINGIFY(x) #x
#define STRING(x) STRINGIFY(x)

#define WEIGHTS_DIR STRING(__WEIGHTS_DIR__)
#endif

template<class T, size_t SIZE>
void load_weights_from_txt(T *w, const char* fname) {

    std::string full_path = std::string(WEIGHTS_DIR) + "/" + std::string(fname);
    std::ifstream infile(full_path.c_str(), std::ios::binary);

    if (infile.fail()) {
        std::cerr << "ERROR: file " << std::string(fname) << " does not exist" << std::endl;
        exit(1);
    }

    std::string line;
    if (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string token;

        size_t i = 0;
        while(std::getline(iss, token, ',')) {
#ifndef XLNX_VIVADO_HLS
            double data;
            std::istringstream(token) >> data;
            w[i] = data;
#else
            std::istringstream(token) >> w[i];
#endif
            i++;
        }

        if (SIZE != i) {
            std::cerr << "ERROR: Expected " << SIZE << " values";
            std::cerr << " but read only " << i << " values" << std::endl;
        }
    }
}

// TODO(gdg): temporary disabled (it does not compile)
#if 0
template<class T, size_t SIZE>
void load_compressed_weights_from_txt(T *w, const char* fname) {

    std::string full_path = std::string(WEIGHTS_DIR) + "/" + std::string(fname);
    std::ifstream infile(full_path.c_str(), std::ios::binary);

    if (infile.fail()) {
        std::cerr << "ERROR: file " << std::string(fname) << " does not exist" << std::endl;
        exit(1);
    }

    std::string line;
    if (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string token;
        std::string extra_chars = "} ";

        size_t i = 0;
        while(std::getline(iss, token, '{')) {
            if (token.length() == 0) {
                continue;
            }
            for (char c: extra_chars) {
                token.erase(std::remove(token.begin(), token.end(), c), token.end());
            }
            if (token.back() == ',') {
                token.erase(token.end() - 1);
            }

            std::replace(token.begin(), token.end(), ',', ' ');
            std::istringstream structss(token);

            if(!(structss >> w[i].row_index >> w[i].col_index >> w[i].weight)) {
                std::cerr << "ERROR: Unable to parse file " << std::string(fname);
                exit(1);
            }
            i++;
        }

        if (SIZE != i) {
            std::cerr << "ERROR: Expected " << SIZE << " values";
            std::cerr << " but read only " << i << " values" << std::endl;
        }
    }
}
#endif
#endif

template <class dataType, unsigned int nrows>
int read_file_1D(const char * filename, dataType data[nrows])
{
  FILE *fp;
  fp = fopen(filename, "r");
  if (fp == 0) {
    return -1;
  }
  // Read data from file
  float newval;
  for (int ii = 0; ii < nrows; ii++){
    if (fscanf(fp, "%f\n", &newval) != 0){
      data[ii] = newval;
    } else {
      return -2;
    }
  }
  fclose(fp);
  return 0;
}

template <class dataType, unsigned int nrows, unsigned int ncols>
int read_file_2D(const char * filename, dataType data[nrows][ncols])
{
  FILE *fp;
  fp = fopen(filename, "r");
  if (fp == 0) {
    return -1;
  }
  // Read data from file
  float newval;
  for (int ii = 0; ii < nrows; ii++) {
    for (int jj = 0; jj < ncols; jj++){
      if (fscanf(fp, "%f\n", &newval) != 0){
        data[ii][jj] = newval;
      } else {
        return -2;
      }
    }
  }
  fclose(fp);
  return 0;
}

template<class in_T, class out_T, int N_IN>
#ifndef XLNX_VIVADO_HLS
void change_type(ac_channel<in_T> &in, ac_channel<out_T> &out)
#else
void change_type(hls::stream<in_T> &in, hls::stream<out_T> &out)
#endif
{
    in_T datareg;
#ifndef XLNX_VIVADO_HLS
    ac_channel<out_T> input_trunc;
#else
    hls::stream<out_T> input_trunc;
#endif
    for (int ii=0; ii<N_IN; ii++) {
        out << (out_T) in.read();
    }
}

template<class data_T, int N_IN>
#ifndef XLNX_VIVADO_HLS
void  hls_stream_debug(ac_channel<data_T> &data, ac_channel<data_T> &res)
#else
void  hls_stream_debug(hls::stream<data_T> &data, hls::stream<data_T> &res)
#endif
{
    data_T datareg;
    for (int ii=0; ii<N_IN; ii++) {
        datareg = data.read();
        std::cout << "[" << ii << "]: " << datareg << std::endl;
        res << datareg;
    }
}

constexpr int ceillog2(int x){
  return (x <= 2) ? 1 : 1 + ceillog2((x+1) / 2);
}

constexpr int pow2(int x){
  return x == 0 ? 1 : 2 * pow2(x - 1);
}

}

#endif
