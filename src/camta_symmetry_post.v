`timescale 1ns / 1ps

// Combinational symmetry reconstruction block.
//
// ODD, COMPLEMENT, and RESIDUAL reconstruction share one subtractor:
//   ODD        : 0 - y_in
//   COMPLEMENT : 1.0 - y_in
//   RESIDUAL   : y_in - x_eval
//
// EVEN, NONE, positive-domain samples, and unsupported symmetry encodings
// bypass y_in. The shared subtractor is operand-isolated by forcing both
// operands to zero whenever reconstruction arithmetic is not required.
module camta_symmetry_post #(
    parameter integer DATA_W = 16,
    parameter integer DATA_I = 5
)(
    input  wire [2:0]                  sym_mode,
    input  wire                        neg_flag,
    input  wire signed [DATA_W-1:0]    x_eval,
    input  wire signed [DATA_W-1:0]    y_in,
    output wire signed [DATA_W-1:0]    y_out
);

    localparam [2:0] CAMTA_SYM_NONE        = 3'd0;
    localparam [2:0] CAMTA_SYM_ODD         = 3'd1;
    localparam [2:0] CAMTA_SYM_COMPLEMENT  = 3'd2;
    localparam [2:0] CAMTA_SYM_RESIDUAL    = 3'd3;
    localparam [2:0] CAMTA_SYM_EVEN        = 3'd4;

    localparam integer DATA_F = DATA_W - DATA_I;

    // Width-safe fixed-point representation of +1.0.
    // Example for Q5.11: 1 << 11 = 2048.
    localparam signed [DATA_W-1:0] ONE_FIXED =
        ({{(DATA_W-1){1'b0}}, 1'b1} << DATA_F);

    wire transform_en;

    // Arithmetic reconstruction is required only for negative-domain
    // ODD, COMPLEMENT, and RESIDUAL samples. EVEN is a direct bypass.
    assign transform_en = neg_flag &&
        ((sym_mode == CAMTA_SYM_ODD)        ||
         (sym_mode == CAMTA_SYM_COMPLEMENT) ||
         (sym_mode == CAMTA_SYM_RESIDUAL));

    // Shared subtractor operands. Defaults isolate the subtractor by
    // holding both arithmetic inputs at zero.
    reg signed [DATA_W-1:0] sub_a;
    reg signed [DATA_W-1:0] sub_b;

    always @(*) begin
        sub_a = {DATA_W{1'b0}};
        sub_b = {DATA_W{1'b0}};

        if (transform_en) begin
            case (sym_mode)
                CAMTA_SYM_ODD: begin
                    sub_a = {DATA_W{1'b0}};
                    sub_b = y_in;
                end

                CAMTA_SYM_COMPLEMENT: begin
                    sub_a = ONE_FIXED;
                    sub_b = y_in;
                end

                CAMTA_SYM_RESIDUAL: begin
                    sub_a = y_in;
                    sub_b = x_eval;
                end

                default: begin
                    sub_a = {DATA_W{1'b0}};
                    sub_b = {DATA_W{1'b0}};
                end
            endcase
        end
    end

    wire signed [DATA_W-1:0] transform_result;
    assign transform_result = $signed(sub_a) - $signed(sub_b);

    // NONE, EVEN, positive-domain samples, and unsupported encodings bypass
    // the shared arithmetic path.
    assign y_out = transform_en ? transform_result : y_in;

endmodule
