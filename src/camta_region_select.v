`timescale 1ns / 1ps

module camta_region_select #(
    parameter integer DATA_W = 16
)(
    input  wire signed [DATA_W-1:0] x,
    input  wire signed [DATA_W-1:0] L_left,
    input  wire signed [DATA_W-1:0] L_right,
    output reg  [1:0]               region
);

    always @(*) begin
        if (x < L_left)
            region = 2'd0;
        else if (x > L_right)
            region = 2'd2;
        else
            region = 2'd1;
    end

endmodule
