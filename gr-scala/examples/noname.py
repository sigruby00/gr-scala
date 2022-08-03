#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: collision_experiments
# GNU Radio version: 3.8.3.1

from gnuradio import blocks
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import uhd
import time
from gnuradio.filter import pfb
import lora
import scala


class noname(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "collision_experiments")

        ##################################################
        # Variables
        ##################################################
        self.sf = sf = 10
        self.bw = bw = 500e3/4
        self.scala_gt_id = scala_gt_id = 304
        self.samp_rate_0 = samp_rate_0 = 1e6
        self.pyramid_gt_id = pyramid_gt_id = 204
        self.payload_len = payload_len = 8
        self.lora_gt_id = lora_gt_id = 104
        self.ldr = ldr = 2**sf/bw > 16e-3
        self.header = header = True
        self.crc = crc = True
        self.cr = cr = 4
        self.band = band = 915e6

        ##################################################
        # Blocks
        ##################################################
        self.uhd_usrp_source_0_0_0 = uhd.usrp_source(
            ",".join(('serial=3189EE7', "")),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
        )
        self.uhd_usrp_source_0_0_0.set_center_freq(band, 0)
        self.uhd_usrp_source_0_0_0.set_gain(10, 0)
        self.uhd_usrp_source_0_0_0.set_antenna('RX2', 0)
        self.uhd_usrp_source_0_0_0.set_bandwidth(samp_rate_0, 0)
        self.uhd_usrp_source_0_0_0.set_samp_rate(samp_rate_0)
        # No synchronization enforced.
        self.scala_detector_0 = scala.detector(scala_gt_id)
        self.pfb_arb_resampler_xxx_0_0 = pfb.arb_resampler_ccf(
            bw/samp_rate_0,
            taps=None,
            flt_size=32)
        self.pfb_arb_resampler_xxx_0_0.declare_sample_delay(0)
        self.pfb_arb_resampler_xxx_0 = pfb.arb_resampler_ccf(
            2*bw/samp_rate_0,
            taps=None,
            flt_size=32)
        self.pfb_arb_resampler_xxx_0.declare_sample_delay(0)
        self.low_pass_filter_0_0 = filter.fir_filter_ccf(
            1,
            firdes.low_pass(
                1,
                samp_rate_0,
                bw/2+20e3,
                1e3,
                firdes.WIN_RECTANGULAR,
                6.76))
        self.low_pass_filter_0 = filter.fir_filter_ccf(
            1,
            firdes.low_pass(
                1,
                samp_rate_0,
                bw/2+20e3,
                1e3,
                firdes.WIN_RECTANGULAR,
                6.76))
        self.lora_pyramid_demod_0 = lora.pyramid_demod(pyramid_gt_id, sf, ldr, 25.0, 4, 0.005, 2)
        self.lora_demod_0 = lora.demod(lora_gt_id, sf, header, payload_len, cr, crc, ldr, 25.0, 16, 0, 4, 2)
        self.lora_decode_0_0 = lora.decode(sf, header, payload_len, cr, crc, ldr)
        self.lora_decode_0 = lora.decode(sf, header, payload_len, cr, crc, ldr)
        self.blocks_null_sink_0 = blocks.null_sink(gr.sizeof_gr_complex*1)
        self.blocks_message_debug_0 = blocks.message_debug()


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.lora_decode_0, 'out'), (self.blocks_message_debug_0, 'print_pdu'))
        self.msg_connect((self.lora_decode_0, 'header'), (self.lora_pyramid_demod_0, 'header'))
        self.msg_connect((self.lora_decode_0_0, 'out'), (self.blocks_message_debug_0, 'print_pdu'))
        self.msg_connect((self.lora_decode_0_0, 'header'), (self.lora_demod_0, 'header'))
        self.msg_connect((self.lora_demod_0, 'out'), (self.lora_decode_0_0, 'in'))
        self.msg_connect((self.lora_pyramid_demod_0, 'out'), (self.lora_decode_0, 'in'))
        self.connect((self.low_pass_filter_0, 0), (self.pfb_arb_resampler_xxx_0, 0))
        self.connect((self.low_pass_filter_0_0, 0), (self.pfb_arb_resampler_xxx_0_0, 0))
        self.connect((self.pfb_arb_resampler_xxx_0, 0), (self.lora_demod_0, 0))
        self.connect((self.pfb_arb_resampler_xxx_0, 0), (self.lora_pyramid_demod_0, 0))
        self.connect((self.pfb_arb_resampler_xxx_0_0, 0), (self.scala_detector_0, 0))
        self.connect((self.scala_detector_0, 0), (self.blocks_null_sink_0, 0))
        self.connect((self.uhd_usrp_source_0_0_0, 0), (self.low_pass_filter_0, 0))
        self.connect((self.uhd_usrp_source_0_0_0, 0), (self.low_pass_filter_0_0, 0))


    def get_sf(self):
        return self.sf

    def set_sf(self, sf):
        self.sf = sf
        self.set_ldr(2**self.sf/self.bw > 16e-3)

    def get_bw(self):
        return self.bw

    def set_bw(self, bw):
        self.bw = bw
        self.set_ldr(2**self.sf/self.bw > 16e-3)
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate_0, self.bw/2+20e3, 1e3, firdes.WIN_RECTANGULAR, 6.76))
        self.low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.samp_rate_0, self.bw/2+20e3, 1e3, firdes.WIN_RECTANGULAR, 6.76))
        self.pfb_arb_resampler_xxx_0.set_rate(2*self.bw/self.samp_rate_0)
        self.pfb_arb_resampler_xxx_0_0.set_rate(self.bw/self.samp_rate_0)

    def get_scala_gt_id(self):
        return self.scala_gt_id

    def set_scala_gt_id(self, scala_gt_id):
        self.scala_gt_id = scala_gt_id

    def get_samp_rate_0(self):
        return self.samp_rate_0

    def set_samp_rate_0(self, samp_rate_0):
        self.samp_rate_0 = samp_rate_0
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate_0, self.bw/2+20e3, 1e3, firdes.WIN_RECTANGULAR, 6.76))
        self.low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.samp_rate_0, self.bw/2+20e3, 1e3, firdes.WIN_RECTANGULAR, 6.76))
        self.pfb_arb_resampler_xxx_0.set_rate(2*self.bw/self.samp_rate_0)
        self.pfb_arb_resampler_xxx_0_0.set_rate(self.bw/self.samp_rate_0)
        self.uhd_usrp_source_0_0_0.set_samp_rate(self.samp_rate_0)
        self.uhd_usrp_source_0_0_0.set_bandwidth(self.samp_rate_0, 0)

    def get_pyramid_gt_id(self):
        return self.pyramid_gt_id

    def set_pyramid_gt_id(self, pyramid_gt_id):
        self.pyramid_gt_id = pyramid_gt_id

    def get_payload_len(self):
        return self.payload_len

    def set_payload_len(self, payload_len):
        self.payload_len = payload_len

    def get_lora_gt_id(self):
        return self.lora_gt_id

    def set_lora_gt_id(self, lora_gt_id):
        self.lora_gt_id = lora_gt_id

    def get_ldr(self):
        return self.ldr

    def set_ldr(self, ldr):
        self.ldr = ldr

    def get_header(self):
        return self.header

    def set_header(self, header):
        self.header = header

    def get_crc(self):
        return self.crc

    def set_crc(self, crc):
        self.crc = crc

    def get_cr(self):
        return self.cr

    def set_cr(self, cr):
        self.cr = cr

    def get_band(self):
        return self.band

    def set_band(self, band):
        self.band = band
        self.uhd_usrp_source_0_0_0.set_center_freq(self.band, 0)





def main(top_block_cls=noname, options=None):
    tb = top_block_cls()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        sys.exit(0)

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    tb.start()

    try:
        input('Press Enter to quit: ')
    except EOFError:
        pass
    tb.stop()
    tb.wait()


if __name__ == '__main__':
    main()
