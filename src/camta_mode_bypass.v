`timescale 1ns / 1ps

module camta_mode_bypass #(
    parameter integer DATA_W = 16
)(
    input  wire [1:0]                  mode,
    input  wire signed [DATA_W-1:0]    x_eval_in,
    input  wire signed [DATA_W-1:0]    a0_in,
    input  wire signed [DATA_W-1:0]    y_horner,
    output reg  signed [DATA_W-1:0]    y_out
);

    localparam [1:0] CAMTA_MODE_HORNER   = 2'd0;
    localparam [1:0] CAMTA_MODE_CONST    = 2'd1;
    localparam [1:0] CAMTA_MODE_ZERO     = 2'd2;
    localparam [1:0] CAMTA_MODE_IDENTITY = 2'd3;

    always @(*) begin
        case (mode)
            CAMTA_MODE_CONST:    y_out = a0_in;
            CAMTA_MODE_ZERO:     y_out = {DATA_W{1'b0}};
            CAMTA_MODE_IDENTITY: y_out = x_eval_in;
            default:             y_out = y_horner;
        endcase
    end

endmodule
