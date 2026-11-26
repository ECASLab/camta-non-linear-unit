`timescale 1ns / 1ps

// Combinational symmetry preprocessing block.
//
// For ODD, COMPLEMENT, RESIDUAL, and EVEN modes, negative inputs are
// evaluated using their magnitude and the original sign is retained in
// neg_flag. NONE mode bypasses x_in directly.
//
// The two's-complement datapath is operand-isolated: its input is forced
// to zero unless a magnitude-based mode is active and x_in is negative.
module camta_symmetry_pre #(
    parameter integer DATA_W = 16
)(
    input  wire [2:0]                  sym_mode,
    input  wire signed [DATA_W-1:0]    x_in,
    output wire                        neg_flag,
    output wire signed [DATA_W-1:0]    x_eval
);

    localparam [2:0] CAMTA_SYM_NONE        = 3'd0;
    localparam [2:0] CAMTA_SYM_ODD         = 3'd1;
    localparam [2:0] CAMTA_SYM_COMPLEMENT  = 3'd2;
    localparam [2:0] CAMTA_SYM_RESIDUAL    = 3'd3;
    localparam [2:0] CAMTA_SYM_EVEN        = 3'd4;

    wire use_abs;
    wire neg_en;

    // Magnitude evaluation is required only by the supported symmetry
    // reconstruction modes. NONE and unsupported encodings bypass x_in.
    assign use_abs =
        (sym_mode == CAMTA_SYM_ODD)        ||
        (sym_mode == CAMTA_SYM_COMPLEMENT) ||
        (sym_mode == CAMTA_SYM_RESIDUAL)   ||
        (sym_mode == CAMTA_SYM_EVEN);

    // Negation is necessary only for a negative input in a magnitude-based
    // symmetry mode.
    assign neg_en   = use_abs && x_in[DATA_W-1];
    assign neg_flag = neg_en;

    // Operand isolation for the two's-complement datapath. When negation
    // is not required, the arithmetic block sees a static zero input.
    wire signed [DATA_W-1:0] neg_operand;
    wire signed [DATA_W-1:0] x_neg;

    assign neg_operand = neg_en ? x_in : {DATA_W{1'b0}};
    assign x_neg       = -$signed(neg_operand);

    // Positive magnitude-mode inputs and all NONE-mode inputs bypass the
    // arithmetic path directly.
    assign x_eval = neg_en ? x_neg : x_in;

endmodule
