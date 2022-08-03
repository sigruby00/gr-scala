#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Rx USRP
# Author: jkadbear
# GNU Radio version: 3.8.3.1

from distutils.version import StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

from gnuradio import blocks
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
import sys
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import uhd
import time
from gnuradio.filter import pfb
import lora

from gnuradio import qtgui

class rx_usrp(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Rx USRP")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Rx USRP")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "rx_usrp")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Variables
        ##################################################
        self.sf = sf = 10
        self.bw = bw = 500e3/4
        self.samp_rate = samp_rate = 1e6
        self.payload_len = payload_len = 8
        self.ldr = ldr = 2**sf/bw > 16e-3
        self.header = header = True
        self.freq = freq = 915e6
        self.crc = crc = True
        self.cr = cr = 4

        ##################################################
        # Blocks
        ##################################################
        self.uhd_usrp_source_0 = uhd.usrp_source(
            ",".join(('serial=3189EE7', "")),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
        )
        self.uhd_usrp_source_0.set_center_freq(freq, 0)
        self.uhd_usrp_source_0.set_gain(1, 0)
        self.uhd_usrp_source_0.set_antenna('RX2', 0)
        self.uhd_usrp_source_0.set_bandwidth(bw, 0)
        self.uhd_usrp_source_0.set_samp_rate(samp_rate)
        # No synchronization enforced.
        self.pfb_arb_resampler_xxx_0 = pfb.arb_resampler_ccf(
            2*bw/samp_rate,
            taps=None,
            flt_size=32)
        self.pfb_arb_resampler_xxx_0.declare_sample_delay(0)
        self.low_pass_filter_0 = filter.fir_filter_ccf(
            1,
            firdes.low_pass(
                1,
                samp_rate,
                bw/2+10e3,
                1e3,
                firdes.WIN_RECTANGULAR,
                6.76))
        self.lora_demod_0 = lora.demod(0, sf, header, payload_len, cr, crc, ldr, 25.0, 16, 0, 4, 2)
        self.lora_decode_0 = lora.decode(sf, header, payload_len, cr, crc, ldr)
        self.blocks_socket_pdu_0 = blocks.socket_pdu('UDP_CLIENT', '127.0.0.1', '52002', 10000, False)
        self.blocks_message_debug_0 = blocks.message_debug()


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.lora_decode_0, 'out'), (self.blocks_message_debug_0, 'print_pdu'))
        self.msg_connect((self.lora_decode_0, 'out'), (self.blocks_socket_pdu_0, 'pdus'))
        self.msg_connect((self.lora_decode_0, 'header'), (self.lora_demod_0, 'header'))
        self.msg_connect((self.lora_demod_0, 'out'), (self.lora_decode_0, 'in'))
        self.connect((self.low_pass_filter_0, 0), (self.pfb_arb_resampler_xxx_0, 0))
        self.connect((self.pfb_arb_resampler_xxx_0, 0), (self.lora_demod_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.low_pass_filter_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "rx_usrp")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

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
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate, self.bw/2+10e3, 1e3, firdes.WIN_RECTANGULAR, 6.76))
        self.pfb_arb_resampler_xxx_0.set_rate(2*self.bw/self.samp_rate)
        self.uhd_usrp_source_0.set_bandwidth(self.bw, 0)

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate, self.bw/2+10e3, 1e3, firdes.WIN_RECTANGULAR, 6.76))
        self.pfb_arb_resampler_xxx_0.set_rate(2*self.bw/self.samp_rate)
        self.uhd_usrp_source_0.set_samp_rate(self.samp_rate)

    def get_payload_len(self):
        return self.payload_len

    def set_payload_len(self, payload_len):
        self.payload_len = payload_len

    def get_ldr(self):
        return self.ldr

    def set_ldr(self, ldr):
        self.ldr = ldr

    def get_header(self):
        return self.header

    def set_header(self, header):
        self.header = header

    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq
        self.uhd_usrp_source_0.set_center_freq(self.freq, 0)

    def get_crc(self):
        return self.crc

    def set_crc(self, crc):
        self.crc = crc

    def get_cr(self):
        return self.cr

    def set_cr(self, cr):
        self.cr = cr





def main(top_block_cls=rx_usrp, options=None):

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()

    tb.show()

    def sig_handler(sig=None, frame=None):
        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    def quitting():
        tb.stop()
        tb.wait()

    qapp.aboutToQuit.connect(quitting)
    qapp.exec_()

if __name__ == '__main__':
    main()
