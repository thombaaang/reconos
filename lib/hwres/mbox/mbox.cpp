<<reconos_preproc>>

#include "ap_int.h"
#include "hls_stream.h"

void hwres_imp(hls::stream<ap_uint<32> > osif_in, hls::stream<ap_uint<32> > osif_out) {
	#pragma HLS INTERFACE ap_ctrl_none port=return

	hls::stream<ap_uint<32> > mbox;
	#pragma HLS STREAM variable=mbox depth=<<ARGS|{0}>>
	hls::stream<ap_uint<40> > queue_put;
	#pragma HLS STREAM variable=queue_put depth=32
	hls::stream<ap_uint<8> > queue_get;
	#pragma HLS STREAM variable=queue_get depth=32

	while (1) {
		if (!osif_in.empty()) {
			ap_uint<32> ctrl = osif_in.read();
			ap_uint<32> cmd = osif_in.read();

			if (cmd == 0x000000F0) {
				queue_get.write(ctrl(31, 24));
			} else if (cmd == 0x000000F1) {
				ap_uint<32> data = osif_in.read();
				queue_put.write((ctrl(31, 24), data));
			} else {
				// Unsupported request
			}
		}

		if (!queue_put.empty() && !mbox.full()) {
			ap_uint<40> put = queue_put.read();
			mbox.write(ap_uint<32>(put(31, 0)));
			osif_out.write((ap_uint<8>(<<ID|0x{:02x}>>), put(39,32), ap_uint<16>(0x0001)));
			osif_out.write(ap_uint<32>(0x00000000));
		}

		if (!queue_get.empty() && !mbox.empty()) {
			ap_uint<8> get = queue_get.read();
			ap_uint<32> data = mbox.read();
			osif_out.write((ap_uint<8>(<<ID|0x{:02x}>>), get, ap_uint<16>(0x0001)));
			osif_out.write(data);
		}
	}
}
