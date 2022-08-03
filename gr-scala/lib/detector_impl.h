/* -*- c++ -*- */
/*
 * Copyright 2022 sigruby00.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_SCALA_DETECTOR_IMPL_H
#define INCLUDED_SCALA_DETECTOR_IMPL_H

// itself
#include <scala/detector.h>
#include "utilities.h"

// external libraries
#include <mosquitto.h>
#include <arpa/inet.h>

// gnuradio specific libraries
#include <gnuradio/io_signature.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/fft/fft.h>
#include <gnuradio/expj.h>
#include <volk/volk.h>

// c++ specific libraries
#include <cstdlib>
#include <complex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>
#include <vector>
#include <cmath>
#include <deque>
#include <queue>


namespace gr
{
  namespace scala
  {

    struct peak
    {
      unsigned int bin; // peak position bin
      int coarse_to;    // coarse position bin
      int fine_to;      // fine position bin
      float cfo;
      std::vector<float> mag;            // mag
      std::vector<int> w_idx;            // history of updated window index
      std::vector<unsigned int> mag_pos; // mag
      peak(unsigned int bin, int coarse_to, int fine_to, float mag_obs, int w_counter)
          : bin(bin), coarse_to(coarse_to), fine_to(fine_to)
      {
        mag.push_back(mag_obs);
        w_idx.push_back(w_counter);
      }
    };

    class detector_impl : public detector
    {
    private:
      uint8_t d_sf;
      uint32_t d_bw;
      uint32_t d_fs;
      uint32_t d_samples_per_second; ///< The number of samples taken per second by GNU Radio.
      uint32_t d_max_buf_len;

      std::vector<float> d_downchirp_ifreq; ///< The instantaneous frequency of the ideal downchirp.
      std::vector<float> d_upchirp_ifreq;   ///< The instantaneous frequency of the ideal downchirp.
      std::vector<float> d_upchirp_ifreq_v; ///< The instantaneous frequency of the ideal upchirp.
      std::vector<float> d_window;

      std::vector<gr_complex> d_upchirp;
      std::vector<gr_complex> d_downchirp;
      std::vector<gr_complex> d_downchirp_ext;
      std::vector<gr_complex> d_upchirp_ext;
      // std::vector<gr_complex> d_downchirp_ext0;

      std::vector<peak> track;
      fft::fft_complex *d_fft;
      gr_complex *buffer;
      gr_complex *buffer_fft;
      char *buffer_to_send;

      std::ofstream mag_dump;

      struct mosquitto *mosq;
      int rc = 0;

      time_t t;

      unsigned int d_samples_per_symbol;
      unsigned int d_num_sym_pre;
      unsigned int d_num_samples;
      unsigned int d_bin_size;
      unsigned int d_fft_len;
      unsigned int d_fft_ext_len;
      unsigned int g_count; // global counter

      int pload_sym_len;
      int history_len;
      int max_g_count;

      unsigned short d_num_preamble;
      unsigned short d_bin_tolerance;
      unsigned short d_offset;
      unsigned short d_p;

      double d_symbols_per_second; ///< Indicator of how many symbols (read: chirps) are transferred each second.
      double d_dt;                 ///< Indicates how fast the frequency changes in a symbol (chirp).
      double d_beta;

      char mqtt_c_id[24];
    
      int gt_idx;


      void find_and_add_peak(float *fft_results, float *fft_results_win, int g_count);
      void check_peak();
      void calc_offset();
      void build_ideal_chirps(void);
      void print_track(void);
      void forward_signal(void);
      void cfo_compensator(float cfo, int symbol_len);

      // void complex_samples_to_file(const std::string path, const gr_complex *v, const uint32_t length, const uint32_t elem_size, bool rewrite);
      // void samples_to_file_cont(const std::string path, const gr_complex *v, const uint32_t length, const uint32_t elem_size);
      // void float_samples_to_file(const std::string path, const float *v, const uint32_t length, const uint32_t elem_size, bool rewrite);
      inline void instantaneous_frequency(const gr_complex *in_samples, float *out_ifreq, const uint32_t window);

      void message_callback(struct mosquitto *mosq, const struct mosquitto_message *msg);

    public:
      detector_impl(int g_id);
      ~detector_impl();

      unsigned int argmax_32f(float *fft_result, int v_length);

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace scala
} // namespace gr

#endif /* INCLUDED_SCALA_DETECTOR_IMPL_H */
