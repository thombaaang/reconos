<<reconos_preproc>>

#include "ap_int.h"
#include "hls_stream.h"

void hwres_imp(hls::stream<ap_uint<32> > osif_in, hls::stream<ap_uint<32> > osif_out) {
	#pragma HLS INTERFACE ap_ctrl_none port=return

	bool write_req, read_req;
	ap_uint<24> write_len, read_len;
	ap_uint<8> write_id, read_id;

	while(1) {
		if (!osif_in.empty()) {
			ap_uint<32> ctrl = osif_in.read();
			ap_uint<32> cmd = osif_in.read();
			ap_uint<32> len = osif_in.read();

			if (cmd == 0x00000090) {
				write_id = ctrl(31, 24);
				write_len = len(24, 0);
				write_req = true;
			} else if (cmd == 0x00000091) {
				read_id = ctrl(31, 24);
				read_len = len(24, 0);
				read_req = true;
			} else {
				// Unsupported request
			}
		}

		if (write_req && read_req) {
			ap_uint<24> min_len = write_len < read_len ? write_len : read_len;

			osif_out.write((ap_uint<8>(<<ID|0x{:02x}>>), read_id, ap_uint<16>(0x0001)));
			osif_out.write(write_id, min_len));

			osif_out.write((ap_uint<8>(<<ID|0x{:02x}>>), write_id, ap_uint<16>(0x0001)));
			osif_out.write(read_id, min_len));
		}
	}
}
