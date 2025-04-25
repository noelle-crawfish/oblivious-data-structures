
module aes_cipher_top(clk, rst, ld, done, key, text_in, text_out);
input		clk, rst;
input		ld;
output		done;
input	[127:0]	key;
input	[127:0]	text_in;
output	[127:0]	text_out;

endmodule; // aes_cipher_top
