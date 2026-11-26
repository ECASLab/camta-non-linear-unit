`timescale 1ns / 1ps

// Three-stage, three-MAC pipelined Horner evaluator.
//
// REF09 usage:
// This block is fed from the registered PRE stage in camta_unit_core.
// Consequently, the old input-isolation muxes are removed from the
// arithmetic cone. start remains the input-valid signal and controls
// whether pipeline registers accept a new Horner sample.
//
// External data and coefficients use signed fixed-point DATA_W/DATA_I.
// Internal pipeline values use signed fixed-point ACC_W/ACC_I.
//
// A new Horner sample may be accepted on every clock for which start=1.
// Therefore, after the pipeline fills, the block supports II=1.
//
// Supported polynomial degrees are 1..3:
//   degree 1 -> y = a1*x + a0
//   degree 2 -> y = (a2*x + a1)*x + a0
//   degree 3 -> y = ((a3*x + a2)*x + a1)*x + a0
//
// Degree decoding is only needed to form the Stage-1 Horner term:
//   degree 1 -> 00 : term2 = 0
//   degree 2 -> 01 : term2 = a2
//   degree 3 -> 11 : term2 = a3*x + a2
//
// Stages 2 and 3 always evaluate valid samples. For degree 1, term2=0,
// so Stage 2 naturally produces a1 without a bypass multiplexer.
//
// Degree 0 is not part of the CAMTA specification. The 2'b00 input encoding
// is treated defensively as degree 1, without adding a degree-0 bypass path.
module camta_horner_seq #(
    parameter integer DATA_W = 16,
    parameter integer DATA_I = 5,
    parameter integer ACC_W  = 24,
    parameter integer ACC_I  = 9
)(
    input  wire                      clk,
    input  wire                      rst_n,
    input  wire                      start,

    input  wire signed [DATA_W-1:0]  x_in,
    input  wire        [1:0]         degree_in,
    input  wire signed [DATA_W-1:0]  a3_in,
    input  wire signed [DATA_W-1:0]  a2_in,
    input  wire signed [DATA_W-1:0]  a1_in,
    input  wire signed [DATA_W-1:0]  a0_in,

    output wire                      busy,
    output wire                      done,
    output reg  signed [DATA_W-1:0]  y_out
);

    localparam integer DATA_F            = DATA_W - DATA_I;
    localparam integer ACC_F             = ACC_W - ACC_I;
    localparam integer SHIFT_ALIGN       = ACC_F - DATA_F;
    localparam integer MAC3_DIRECT_SHIFT = DATA_F - SHIFT_ALIGN;

    // stage_en[1] selects degree-3 MAC3 result.
    // stage_en[0] distinguishes degree 2 from degree 1 when forming term2.
    function automatic [1:0] decode_degree;
        input [1:0] degree_value;
        begin
            case (degree_value)
                2'd2:    decode_degree = 2'b01;
                2'd3:    decode_degree = 2'b11;
                default: decode_degree = 2'b00;
            endcase
        end
    endfunction

    function automatic signed [ACC_W-1:0] align_coefficient;
        input signed [DATA_W-1:0] coefficient;
        reg signed [ACC_W-1:0] coefficient_extended;
        begin
            coefficient_extended =
                {{(ACC_W-DATA_W){coefficient[DATA_W-1]}}, coefficient};
            align_coefficient = coefficient_extended <<< SHIFT_ALIGN;
        end
    endfunction

    // ------------------------------------------------------------------
    // REF09 registered-input assumption.
    //
    // Inputs come directly from camta_unit_core PRE registers. No
    // start-controlled input mux is placed before MAC3. This avoids adding
    // isolation logic to the new dedicated MAC3 timing stage.
    //
    // start still determines whether Stage 1 captures a new sample.
    // ------------------------------------------------------------------
    wire [1:0] stage_en_in;

    assign stage_en_in = decode_degree(degree_in);

    wire signed [ACC_W-1:0] a2_aligned_in;
    wire signed [ACC_W-1:0] a1_aligned_in;
    wire signed [ACC_W-1:0] a0_aligned_in;

    assign a2_aligned_in = align_coefficient(a2_in);
    assign a1_aligned_in = align_coefficient(a1_in);
    assign a0_aligned_in = align_coefficient(a0_in);

    // ------------------------------------------------------------------
    // Stage 1: MAC3 = a3*x + a2.
    //
    // a3 remains DATA_W wide, so the multiplier is DATA_W x DATA_W
    // (16x16 for the CAMTA defaults).
    // ------------------------------------------------------------------
    wire signed [DATA_W-1:0]          mac3_op_a;
    wire signed [DATA_W-1:0]          mac3_op_b;
    wire signed [ACC_W-1:0]           mac3_addend;
    wire signed [(2*DATA_W)-1:0]      mac3_product_full;
    wire signed [ACC_W-1:0]           mac3_product_scaled;
    wire signed [ACC_W-1:0]           mac3_sum;
    wire signed [ACC_W-1:0]           term2_in;

    assign mac3_op_a   = a3_in;
    assign mac3_op_b   = x_in;
    assign mac3_addend = a2_aligned_in;

    assign mac3_product_full =
        $signed(mac3_op_a) * $signed(mac3_op_b);

    assign mac3_product_scaled =
        $signed(mac3_product_full) >>> MAC3_DIRECT_SHIFT;

    assign mac3_sum =
        mac3_product_scaled + mac3_addend;

    assign term2_in = stage_en_in[1] ? mac3_sum :
                      stage_en_in[0] ? a2_aligned_in :
                                       {ACC_W{1'b0}};

    reg                         valid_s1;
    reg signed [DATA_W-1:0]     x_s1;
    reg signed [ACC_W-1:0]      term2_s1;
    reg signed [ACC_W-1:0]      a1_s1;
    reg signed [ACC_W-1:0]      a0_s1;

    // ------------------------------------------------------------------
    // Stage 2: MAC2 = term2*x + a1.
    // ------------------------------------------------------------------
    wire signed [ACC_W-1:0]        mac2_op_a;
    wire signed [DATA_W-1:0]       mac2_op_b;
    wire signed [ACC_W-1:0]        mac2_addend;
    wire signed [ACC_W+DATA_W-1:0] mac2_product_full;
    wire signed [ACC_W-1:0]        mac2_product_scaled;
    wire signed [ACC_W-1:0]        mac2_sum;
    wire signed [ACC_W-1:0]        term1_s1_next;

    assign mac2_op_a   = term2_s1;
    assign mac2_op_b   = x_s1;
    assign mac2_addend = a1_s1;

    assign mac2_product_full =
        $signed(mac2_op_a) * $signed(mac2_op_b);

    assign mac2_product_scaled =
        $signed(mac2_product_full) >>> DATA_F;

    assign mac2_sum =
        mac2_product_scaled + mac2_addend;

    assign term1_s1_next = mac2_sum;

    reg                         valid_s2;
    reg signed [DATA_W-1:0]     x_s2;
    reg signed [ACC_W-1:0]      term1_s2;
    reg signed [ACC_W-1:0]      a0_s2;

    // ------------------------------------------------------------------
    // Stage 3: MAC1 = term1*x + a0.
    // ------------------------------------------------------------------
    wire signed [ACC_W+DATA_W-1:0] mac1_product_full;
    wire signed [ACC_W-1:0]        mac1_product_scaled;
    wire signed [ACC_W-1:0]        mac1_sum;
    wire signed [DATA_W-1:0]       y_data_s2_next;

    assign mac1_product_full =
        $signed(term1_s2) * $signed(x_s2);

    assign mac1_product_scaled =
        $signed(mac1_product_full) >>> DATA_F;

    assign mac1_sum =
        mac1_product_scaled + a0_s2;

    assign y_data_s2_next =
        $signed(mac1_sum) >>> SHIFT_ALIGN;

    reg valid_s3;

    assign done = valid_s3;
    assign busy = valid_s1 | valid_s2 | valid_s3;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            valid_s1 <= 1'b0;
            x_s1     <= {DATA_W{1'b0}};
            term2_s1 <= {ACC_W{1'b0}};
            a1_s1    <= {ACC_W{1'b0}};
            a0_s1    <= {ACC_W{1'b0}};

            valid_s2 <= 1'b0;
            x_s2     <= {DATA_W{1'b0}};
            term1_s2 <= {ACC_W{1'b0}};
            a0_s2    <= {ACC_W{1'b0}};

            valid_s3 <= 1'b0;
            y_out    <= {DATA_W{1'b0}};
        end else begin
            valid_s1 <= start;
            valid_s2 <= valid_s1;
            valid_s3 <= valid_s2;

            if (start) begin
                x_s1     <= x_in;
                term2_s1 <= term2_in;
                a1_s1    <= a1_aligned_in;
                a0_s1    <= a0_aligned_in;
            end

            if (valid_s1) begin
                x_s2     <= x_s1;
                term1_s2 <= term1_s1_next;
                a0_s2    <= a0_s1;
            end

            if (valid_s2) begin
                y_out <= y_data_s2_next;
            end
        end
    end

endmodule
