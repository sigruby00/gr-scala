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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "detector_impl.h"

// Callback functions for MQTT
// void OnConnect(struct mosquitto* mosc, void* obj, int rc)
// {
//   std::cout << ">> MQTT connected " << rc << std::endl;
// }

void OnPublish(struct mosquitto* mosc, void* obj, int mid)
{
  std::cout << "The Scala message has been published" << std::endl;
}


namespace gr
{
  namespace scala
  {

    detector::sptr
    detector::make(int g_id)
    {
      return gnuradio::get_initial_sptr(new detector_impl(g_id));
    }

    /*
     * The private constructor
     */
    detector_impl::detector_impl(int g_id)
        : gr::block("detector",
                    gr::io_signature::make(1, 1, sizeof(gr_complex)),
                    gr::io_signature::make(1, 1, sizeof(gr_complex))),
          gt_idx(g_id)
    {
      std::cout << "### Launch Up the SCALA Detector ###" << std::endl;
      std::cout << "### 1- Initialize LoRa Parameter ###" << std::endl;

      // Basic LoRa Parameters
      d_sf = 10;                              //spreading factors
      d_bw = 125000;                          //bandwidth
      d_fs = d_bw;                            //sampling rate

      // Deep LoRa Parameters
      d_samples_per_symbol = pow(2, d_sf);    //samples per symbol
      d_samples_per_second = d_fs;
      d_symbols_per_second = (double)d_bw / (1u << d_sf);
      d_dt = 1.0f / d_samples_per_second;     //time per symbol

      // Demodulation Parameters
      d_fft_len = d_samples_per_symbol;       //length of FFT
      d_fft_ext_len = d_fft_len * 10;         //length of extended FFT
      
      d_fft = new fft::fft_complex(d_samples_per_symbol * 10, true, 1);
      d_beta = 18;                                                                    //beta value for FFT window (18 or 6.26)
      d_window = fft::window::build(fft::window::WIN_KAISER, d_fft_ext_len, d_beta);  // or WIN_HAMMING
      
                                                                              
      std::cout << "### 2- Initialize MQTT Connection###" << std::endl;

      int major, minor, revision;
      mosquitto_lib_init();
   
      std::string mqtt_id = std::to_string(gt_idx);           //set mqtt id to gateway index
      mosq = mosquitto_new(mqtt_id.c_str(), false, NULL);
      if (mosq){
        // mosquitto_connect_callback_set(mosq, OnConnect);
        mosquitto_publish_callback_set(mosq, OnPublish);

        rc = (mosquitto_connect(mosq, "49.50.163.20", 1883, 60) == MOSQ_ERR_SUCCESS)
			            ? true
			            : false;
                  
        mosquitto_loop_start(mosq);
      }
      // rc = mosquitto_connect(mosq, "49.50.163.20", 1883, 60);

      std::cout << "### 3- Initialize GNU-RADIO ###" << std::endl;

      // GNU-Radio Parameters
      g_count = 0;                                            // global loop counter
      max_g_count = 20;                                       // moving window searching Ranges

      pload_sym_len = 30;
      history_len = 13 + pload_sym_len;                       // preamble(8)+sync word(4.25) = 13

      d_max_buf_len = max_g_count + history_len;
      d_fft = new fft::fft_complex(d_samples_per_symbol * 10, true, 1);

      build_ideal_chirps();                                   //build the ideal up and down chirp
      set_history(d_samples_per_symbol * history_len);
    }

    /*
     * Our virtual destructor.
     */
    detector_impl::~detector_impl()
    {
      mosquitto_disconnect(mosq);
      mosquitto_destroy(mosq);
      mosquitto_lib_cleanup();
      delete d_fft;
    }

    void
    detector_impl::build_ideal_chirps(void)
    {
      d_upchirp.resize(d_samples_per_symbol);
      d_downchirp.resize(d_samples_per_symbol);
      
      d_upchirp_ext.resize(d_samples_per_symbol * 2);
      d_downchirp_ext.resize(d_samples_per_symbol * 8);
      
      const double T = -0.5 * d_bw * d_symbols_per_second;
      const double f0 = (d_bw / 2.0);
      const double pre_dir = 2.0 * M_PI;
      double t;
      gr_complex cmx = gr_complex(1.0f, 1.0f);

      for (uint32_t i = 0u; i < d_samples_per_symbol; i++)
      {
        // Width in number of samples = samples_per_symbol
        // See https://en.wikipedia.org/wiki/Chirp#Linear
        t = d_dt * i;
       
        d_upchirp[i] = cmx * gr_expj(pre_dir * t * (f0 + T * t) * -1.0f);
        // d_downchirp[i] = cmx * gr_expj(pre_dir * t * (f0 + T * t)); do not use this one
        d_downchirp[i] = std::conj(d_upchirp[i]);
      }
          
      for (int i = 0; i < 8; i++){
        for (int j = 0; j < 1024; j++){
          d_downchirp_ext[(i * 1024) + j] = d_downchirp[j];
          }
      }
      for (int i = 0; i < 2; i++){
        for (int j = 0; j < 1024; j++){
          d_upchirp_ext[(i * 1024) + j] = d_upchirp[j];
          }
      }
      gr::lora::complex_samples_to_file("/tmp/d_downchirp_ext", &d_downchirp_ext[0], d_downchirp_ext.size(), sizeof(gr_complex), true);
      gr::lora::complex_samples_to_file("/tmp/d_upchirp_ext", &d_upchirp_ext[0], d_upchirp_ext.size(), sizeof(gr_complex), true);
    }

    // void
    // detector_impl::offsets_compensator(int to, float cfo, int symbol_len){
      
      
    //   for (uint32_t i = 0u; i < symbol_len; i++)
    //   {

    //     const double T = -0.5 * d_bw * d_symbols_per_second;
    //     const double f0 = (d_bw / 2.0);
    //     const double pre_dir = 2.0 * M_PI;
    //     t = d_dt * i;
    //     gr_complex 

    //     d_upchirp[i] = cmx * gr_expj(pre_dir * t * (f0 + T * t) * -1.0f);
    //     // d_downchirp[i] = cmx * gr_expj(pre_dir * t * (f0 + T * t)); do not use this one
    //     d_downchirp[i] = std::conj(d_upchirp[i]);
    //   }
    // }

    
    void
    detector_impl::find_and_add_peak(float *fft_results, float *fft_results_win, int g_count)
    {
      int coarse_idx = g_count - 1; // minus 1 for indexing
      int max_dis = 22;
      float thr, thr_win;
      
      unsigned int alignment = volk_get_alignment();
      float *mean_ = (float *)volk_malloc(sizeof(float), alignment);
      float *std_ = (float *)volk_malloc(sizeof(float), alignment);
      float *mean_win = (float *)volk_malloc(sizeof(float), alignment);
      float *std_win = (float *)volk_malloc(sizeof(float), alignment);

      volk_32f_stddev_and_mean_32f_x2(std_, mean_, fft_results, 10240);
      volk_32f_stddev_and_mean_32f_x2(std_win, mean_win, fft_results_win, 10240);

      thr = 1*(*mean_)+(4*(*std_));
      thr_win = 1*(*mean_win)+(4*(*std_win));

      // step 1 : peak detection based on thresh hold with (rectangle and kaiser windowing) FFT
      std::vector<std::pair<int, float>> peak_tmp;
      for (unsigned int i = 0; i < 10240; i++)
      {
        int l_idx = gr::lora::pmod(i - 1, 10240);
        int r_idx = gr::lora::pmod(i + 1, 10240);

        if ((fft_results[i] > thr) && (fft_results_win[i] > thr_win) && (fft_results[i] > fft_results[l_idx]) && (fft_results[i] > fft_results[r_idx]))
        {
          peak_tmp.push_back({i, (float)fft_results[i]});
        }
      }

      // step 2 : filtering peaks with distance (max_dis)
      std::vector<std::pair<int, float>>::iterator iter;
      for (iter = peak_tmp.begin(); iter != peak_tmp.end();)
      {
        if (std::next(iter, 1) == peak_tmp.end()) {break;}
        int cur_pos = (*iter).first;
        int nxt_pos = (*(std::next(iter, 1))).first;

        float cur_mag = (*iter).second;
        float nxt_mag = (*(std::next(iter, 1))).second;

        // eleminate siderobes effect
        if (abs(nxt_pos - cur_pos) < max_dis){
            if(cur_mag < nxt_mag){
              iter = peak_tmp.erase(iter);
            }else{
              iter = peak_tmp.erase(std::next(iter, 1));
            }
        }else{
          ++iter;
        }
      }

      // step 3 : Add peaks to track
      for (iter = peak_tmp.begin(); iter != peak_tmp.end(); iter++)
      {
        int cur_pos = (*iter).first;
        float cur_mag = (*iter).second;
        bool found = std::any_of(track.begin(), track.end(), [&](peak &pk)
                                 {return pk.bin == cur_pos;});

        // if cur_pos (bin) found in track 
        if (found)
        {
          auto found_iter = std::find_if(track.begin(), track.end(), [&](peak &pk)
                                        { return pk.bin == cur_pos; });
          int found_idx = found_iter - track.begin();

          track[found_idx].mag.push_back(cur_mag);
          track[found_idx].w_idx.push_back(coarse_idx);
        }
        // if not
        else
        {
          track.push_back(peak(cur_pos, 0, 0, cur_mag, coarse_idx));
        }
      }
    }

    // eleminate peaks which show discontinuity
    void
    detector_impl::check_peak()
    {
      int cont_thr = 4;   //continuity threshold

      std::vector<peak>::iterator iter;
      for (iter = track.begin(); iter != track.end();)
      {
        bool cont = false;    //default
        int cont_count = 0;
        int cur_idx;
        int pre_idx = (*iter).w_idx[0];

        // step 1 : calculates continuity (true or false) 
        for (int i = 0; i < (*iter).mag.size(); i++){
          cur_idx = (*iter).w_idx[i];
          if(cur_idx == (pre_idx+1)){
            ++cont_count;
          }else{
            cont_count = 0;   
          }

          if(cont_count>=cont_thr){
            cont=true;
            break;
          }
          pre_idx=cur_idx;
        }

        // step 2 : eliminates discontinuous elements
        if (!cont){
          iter = track.erase(iter);
          continue;
        }

        // experimental step : elimanate element which located in first and last
        int max_pk_idx = std::max_element((*iter).mag.begin(), (*iter).mag.end()) - (*iter).mag.begin();
        int target_w_idx = (*iter).w_idx[max_pk_idx];
      
        if (target_w_idx == 0 ||  target_w_idx == (max_g_count-1)){
          iter = track.erase(iter);
          continue;}

        iter++;
      }
    }

    void
    detector_impl::calc_offset()
    {
      std::vector<peak>::iterator iter;

      for (iter = track.begin(); iter != track.end();){

        // step 1 : coarse grained timing offset estimation (e.g., 0, 1024, 2048, ...
        int max_pk_idx = std::max_element((*iter).mag.begin(), (*iter).mag.end()) - (*iter).mag.begin();
        (*iter).coarse_to = (*iter).w_idx[max_pk_idx] * 1024;

        // step 2 : fine grained timing offset estimation (e.g., 0-1023)
        int coarse_idx = (*iter).w_idx[max_pk_idx];
        int fine_idx;
        int coarse_bin;
        int l_search, r_search;

        l_search = (coarse_idx - 1) * 1024;         //left searching ranges
        r_search = (coarse_idx + 1) * 1024;         //right searching ranges

        gr_complex *de_pre = (gr_complex *)volk_malloc(d_fft_len * 8 * sizeof(gr_complex), volk_get_alignment());
        gr_complex *de_sync = (gr_complex *)volk_malloc(d_fft_len * 2 * sizeof(gr_complex), volk_get_alignment());
        float *fft_results = (float *)volk_malloc(d_fft_len * 10 * sizeof(float), volk_get_alignment());
    
        int cnt=0;
        int dynamic_bin;
        int static_bin = (*iter).bin;
        float pre_mag_max = 0;
        float cur_mag_max = 0;
        float obs_mag;
        
        for (int i = l_search; i < r_search; i++)
        {
          volk_32fc_x2_multiply_32fc(de_pre, &buffer[i], &d_downchirp_ext[0], d_samples_per_symbol * 8);
          volk_32fc_x2_multiply_32fc(de_sync, &buffer[i + 10240], &d_upchirp_ext[0], d_samples_per_symbol * 2);

          memset(d_fft->get_inbuf(), 0, d_fft_len * 10 * sizeof(gr_complex));
          memcpy(d_fft->get_inbuf(), &de_pre[0], d_samples_per_symbol * 8 * sizeof(gr_complex));
          memcpy(d_fft->get_inbuf() + (d_samples_per_symbol * 8), &de_sync[0], d_samples_per_symbol * 2 * sizeof(gr_complex));
          d_fft->execute();
          volk_32fc_magnitude_32f(fft_results, d_fft->get_outbuf(), d_fft_len * 10);

          dynamic_bin = (static_bin + (cnt * 10)) % 10240;
          obs_mag = abs(fft_results[dynamic_bin]);

          if (obs_mag > pre_mag_max)
          {
            fine_idx = i;
            pre_mag_max = obs_mag;
            coarse_bin = dynamic_bin;
          }

          cnt++;
        }
        (*iter).fine_to = fine_idx;

        // step 3 : coarse grained carrier frequency offset estimation (integer part of bin)
        float temp_cfo = 0;

        if (coarse_bin < (10240/2)){
          (*iter).cfo = coarse_bin;}
        else{
          (*iter).cfo = (coarse_bin - 10240);}

        // step 4 : elimanate abnormal peak which show (cfo > 22kHz)
        if (abs((*iter).cfo) > 1800){
          iter = track.erase(iter);
          free(de_pre);
          free(de_sync);
          continue;
        }

        // step 5 : fine grained carrier frequency offset estimation (fractional part of bin)
        // searching by integer part of Hz unit
        // for (int i=-(12); i<12; i++) {}

        iter++;
        free(de_pre);
        free(de_sync);
      }
    }

    void
    detector_impl::forward_signal(){

      struct mosquitto_message *msg;

      if (track.size() != 0)
      {
        int detect_num = track.size();
        int to_arr[detect_num];
        float cfo_arr[detect_num];

        // get timestamp with millisecond
        struct timeval time_now{};
        gettimeofday(&time_now, nullptr);
        time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);

        // forward buffer description
        // Gateway Index (gt_idx, int) : 4 bytes
        // Timestamp (msecs_time, time_t) : 8 bytes
        // Number of Detected Signals (detect_num, int) : 4 bytes
        // Timing Offset Array (to_arr, int) : 4 bytes * detect_num
        // Frequency Offset Array (cfo_arr, float) : 4 bytes * detect_num
        // Raw Signal (buffer, gr_complex) : 8byte * d_max_buf_len * 
        int buffer_size = 4 + 8 + 4 + (4+4)*detect_num + sizeof(gr_complex)*d_samples_per_symbol*d_max_buf_len;

        buffer_to_send = (char *)volk_malloc(buffer_size, volk_get_alignment());
        
        int idx = 0;
        for_each(track.begin(), track.end(), [&](peak &pk){
          to_arr[idx]=pk.fine_to;
          cfo_arr[idx]=pk.cfo;
          ++idx;
        });

        int cum_addr = 0;

        // Gateway Index (gt_idx, int) : 4 bytes
        memcpy(buffer_to_send + cum_addr, &gt_idx, sizeof(int));
        cum_addr = cum_addr + sizeof(int);

        // Timestamp (msecs_time, time_t) : 8 bytes
        memcpy(buffer_to_send + cum_addr, &msecs_time, sizeof(msecs_time));
        cum_addr = cum_addr + sizeof(msecs_time);

        // Number of Detected signals (detect_num, int) : 4 bytes
        memcpy(buffer_to_send + cum_addr, &detect_num, sizeof(int));
        cum_addr = cum_addr + sizeof(int);
        
        // Timing Offset Array (to_arr, int) : 4 bytes * detect_num
        memcpy(buffer_to_send + cum_addr, &to_arr[0], detect_num * sizeof(int));
        cum_addr = cum_addr + (detect_num) * sizeof(int);

        // Frequency Offset Array (cfo_arr, float) : 4 bytes * detect_num
        memcpy(buffer_to_send + cum_addr, &cfo_arr[0], detect_num * sizeof(float));
        cum_addr = cum_addr + (detect_num) * sizeof(float);

        // Raw Signal (buffer, gr_complex) : 8byte * d_max_buf_len * d_samples_per_symbol
        memcpy(buffer_to_send + cum_addr, &buffer[0], d_samples_per_symbol * d_max_buf_len * sizeof(gr_complex));

        // forwarding to cloud
        int m_id = 3;
        int *msg_id = &m_id;
        mosquitto_publish(mosq, msg_id, "scala", buffer_size, &buffer_to_send[0], 1, 0);
        
        // garbage collection
        free(buffer_to_send);
      }
    }
      
    void
    detector_impl::print_track()
    {
      if (track.size() != 0){
        std::cout << "----------- Signal Detected -----------" << std::endl;
        std::cout << "Number of Detected Signals : " << track.size() << std::endl;
        for_each(track.begin(), track.end(), [&](peak &pk)
        {
          std::cout << "Estimated Carrier Frequency Offset : " << pk.cfo << " bin" << std::endl;
          std::cout << "Estimated Timing Offset : " << (pk.fine_to) << " samples" <<std::endl; 
        });
        std::cout << "---------------------------------------" << std::endl;
      }
    }

    void
    detector_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items;
    }

    int
    detector_impl::general_work(int noutput_items,
                                gr_vector_int &ninput_items,
                                gr_vector_const_void_star &input_items,
                                gr_vector_void_star &output_items)
    {
      if (ninput_items[0] < d_samples_per_symbol * (history_len+1)) {return 0;}
      else {++g_count;}
      
      const gr_complex *in = (const gr_complex *)input_items[0];
      gr_complex *out = (gr_complex *)output_items[0];
      
      // Initialize the gloabl_buffer from history data
      if (g_count == 1)
      {
        buffer = (gr_complex *)volk_malloc(d_samples_per_symbol * d_max_buf_len * sizeof(gr_complex), volk_get_alignment());
        for (int i = 0; i < d_samples_per_symbol * (history_len); ++i) {buffer[i] = in[i];}
      }
      for (int i = 0; i < d_samples_per_symbol; ++i) {buffer[1024 * (history_len) + ((g_count-1) * d_samples_per_symbol) + i] = in[(d_samples_per_symbol * history_len) +i];}


      // Initialize the local vectos for extended-Dechirp & FFT
      gr_complex *de_pre = (gr_complex *)volk_malloc(d_fft_len * 8 * sizeof(gr_complex), volk_get_alignment());
      gr_complex *de_pre_win = (gr_complex *)volk_malloc(d_fft_len * 8 * sizeof(gr_complex), volk_get_alignment());
      float *fft_results = (float *)volk_malloc(d_fft_len * 10 * sizeof(float), volk_get_alignment());
      float *fft_results_win = (float *)volk_malloc(d_fft_len * 10 * sizeof(float), volk_get_alignment());


      // Dechirp the incoming signal
      volk_32fc_x2_multiply_32fc(de_pre, in, &d_downchirp_ext[0], d_samples_per_symbol * 8);  // dechirp
      volk_32fc_32f_multiply_32fc(de_pre_win, de_pre, &d_window[0], d_samples_per_symbol * 8); // windowing
       

      // Reamin part (>8192) is zeroed out
      memset(d_fft->get_inbuf(), 0, d_fft_len * 10 * sizeof(gr_complex));
      memcpy(d_fft->get_inbuf(), &de_pre[0], d_samples_per_symbol * 8 * sizeof(gr_complex));
      d_fft->execute();
      volk_32fc_magnitude_32f(fft_results, d_fft->get_outbuf(), d_fft_len * 10);

      memset(d_fft->get_inbuf(), 0, d_fft_len * 10 * sizeof(gr_complex));
      memcpy(d_fft->get_inbuf(), &de_pre_win[0], d_samples_per_symbol * 8 * sizeof(gr_complex));
      d_fft->execute();
      volk_32fc_magnitude_32f(fft_results_win, d_fft->get_outbuf(), d_fft_len * 10);


      //**********************************************************************************************//
      //**********************************************************************************************//
      //***************************  SCALA Phase - 1 : Find and add peaks  ***************************//
      //**********************************************************************************************//
      //**********************************************************************************************//

      find_and_add_peak(fft_results, fft_results_win, g_count);

      //**********************************************************************************************//
      //**********************************************************************************************//
      //***************************  SCALA Phase - 2 : Offset Estimation  ****************************//
      //**********************************************************************************************//
      //**********************************************************************************************//

      if (g_count == max_g_count)
      {
        check_peak();
        calc_offset();
      }

      //**********************************************************************************************//
      //**********************************************************************************************//
      //****************************  SCALA Phase - 3 : Forward Singal  ******************************//
      //**********************************************************************************************//
      //**********************************************************************************************//

      if (g_count == max_g_count)
      {
         print_track();  
         forward_signal();

         // garbage collection
         track.clear();
         g_count = 0;
         free(buffer);
      }
      
      //**********************************************************************************************//
      //**********************************************************************************************//
      //**************************************  End Phase  *******************************************//
      //**********************************************************************************************//
      //**********************************************************************************************//
  

      // for (int i = 0; i < d_samples_per_symbol; i++) {out[i] = in[i];} // deactive

      // garbage collection
      free(de_pre);
      free(de_pre_win);
      free(fft_results);
      free(fft_results_win);

      // Tell runtime system how many input items we consumed.
      consume_each(d_samples_per_symbol);

      // Tell runtime system how many output items we produced.
      return d_samples_per_symbol;
    }

  } /* namespace scala */
} /* namespace gr */


