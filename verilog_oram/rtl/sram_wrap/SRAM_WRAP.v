
module SRAM1D_WRAP(Clock, Reset, Enable, Write, Address, DIn, DOut);
   parameter DWidth = 32, AWidth = 10;
   input     Clock, Reset, Enable, Write;
   input [AWidth-1:0] Address;
   input [DWidth-1:0] DIn;
   output [DWidth-1:0] DOut;

endmodule : SRAM1D_WRAP
