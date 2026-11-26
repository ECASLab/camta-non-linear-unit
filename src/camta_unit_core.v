`timescale 1ns / 1ps

// Scalar CAMTA core with a fully overlapped four-stage datapath.
//
// REF09 adds a preprocessing (PRE) pipeline stage before the three-stage
// Horner evaluator. The PRE stage snapshots the symmetry result, selected
// region configuration, coefficients, and metadata for each accepted sample.
//
// start acts as an input-valid signal. A new sample may be presented on
// every clock cycle (II=1). done acts as an output-valid signal and may
// therefore remain high on consecutive cycles when outputs are produced
// back-to-back.
//
// External latency is four pipeline stages:
//   PRE -> MAC3 -> MAC2 -> MAC1
//
// The selected coefficients are captured in PRE so a sample always uses one
// coherent configuration even if the external configuration changes while
// that sample is still in flight.
module camta_unit_core #(
    parameter integer DATA_W = 16,
    parameter integer DATA_I = 5,
    parameter integer ACC_W  = 24,
    parameter integer ACC_I  = 9
)(
    input  wire                      clk,
    input  wire                      rst_n,
    input  wire                      start,

    input  wire [2:0]                sym_mode,
    input  wire signed [DATA_W-1:0]  x_in,

    input  wire signed [DATA_W-1:0]  L_left,
    input  wire signed [DATA_W-1:0]  L_right,

    input  wire [1:0]                deg_r0,
    input  wire [1:0]                deg_r1,
    input  wire [1:0]                deg_r2,

    input  wire [1:0]                mode_r0,
    input  wire [1:0]                mode_r1,
    input  wire [1:0]                mode_r2,

    input  wire signed [DATA_W-1:0]  a3_r0,
    input  wire signed [DATA_W-1:0]  a2_r0,
    input  wire signed [DATA_W-1:0]  a1_r0,
    input  wire signed [DATA_W-1:0]  a0_r0,

    input  wire signed [DATA_W-1:0]  a3_r1,
    input  wire signed [DATA_W-1:0]  a2_r1,
    input  wire signed [DATA_W-1:0]  a1_r1,
    input  wire signed [DATA_W-1:0]  a0_r1,

    input  wire signed [DATA_W-1:0]  a3_r2,
    input  wire signed [DATA_W-1:0]  a2_r2,
    input  wire signed [DATA_W-1:0]  a1_r2,
    input  wire signed [DATA_W-1:0]  a0_r2,

    output wire                      busy,
    output wire                      done,
    output wire signed [DATA_W-1:0]  y_out
);

    localparam [1:0] CAMTA_MODE_HORNER = 2'd0;

    // ------------------------------------------------------------------
    // Combinational front-end.
    //
    // This is now isolated from MAC3 by the PRE registers below.
    // ------------------------------------------------------------------
    wire                     neg_front;
    wire signed [DATA_W-1:0] x_eval_front;
    wire [1:0]               region_front;

    camta_symmetry_pre #(
        .DATA_W(DATA_W)
    ) u_sym_pre (
        .sym_mode (sym_mode),
        .x_in     (x_in),
        .neg_flag (neg_front),
        .x_eval   (x_eval_front)
    );

    camta_region_select #(
        .DATA_W(DATA_W)
    ) u_region (
        .x       (x_eval_front),
        .L_left  (L_left),
        .L_right (L_right),
        .region  (region_front)
    );

    // ------------------------------------------------------------------
    // Active-region configuration selection.
    //
    // The result of this mux is captured in PRE. Therefore every accepted
    // sample carries its own coherent coefficient/mode snapshot through
    // the pipeline.
    // ------------------------------------------------------------------
    reg [1:0]               degree_sel;
    reg [1:0]               mode_sel;
    reg signed [DATA_W-1:0] a3_sel;
    reg signed [DATA_W-1:0] a2_sel;
    reg signed [DATA_W-1:0] a1_sel;
    reg signed [DATA_W-1:0] a0_sel;

    always @(*) begin
        degree_sel = deg_r1;
        mode_sel   = mode_r1;
        a3_sel     = a3_r1;
        a2_sel     = a2_r1;
        a1_sel     = a1_r1;
        a0_sel     = a0_r1;

        case (region_front)
            2'd0: begin
                degree_sel = deg_r0;
                mode_sel   = mode_r0;
                a3_sel     = a3_r0;
                a2_sel     = a2_r0;
                a1_sel     = a1_r0;
                a0_sel     = a0_r0;
            end

            2'd2: begin
                degree_sel = deg_r2;
                mode_sel   = mode_r2;
                a3_sel     = a3_r2;
                a2_sel     = a2_r2;
                a1_sel     = a1_r2;
                a0_sel     = a0_r2;
            end

            default: begin
                degree_sel = deg_r1;
                mode_sel   = mode_r1;
                a3_sel     = a3_r1;
                a2_sel     = a2_r1;
                a1_sel     = a1_r1;
                a0_sel     = a0_r1;
            end
        endcase
    end

    // ------------------------------------------------------------------
    // PRE stage.
    //
    // This is the new REF09 pipeline boundary. It breaks the old critical
    // path:
    //
    //   symmetry -> region/config selection -> MAC3 -> register
    //
    // into:
    //
    //   PRE: symmetry -> region/config selection -> PRE register
    //   S1 : PRE register -> MAC3 -> term2 register
    //
    // Registers only update for accepted samples, so each sample keeps the
    // exact x/configuration pair captured when start was asserted.
    // ------------------------------------------------------------------
    reg                     valid_pre;

    reg [2:0]               sym_mode_pre;
    reg                     neg_pre;
    reg signed [DATA_W-1:0] x_eval_pre;

    reg [1:0]               degree_pre;
    reg [1:0]               mode_pre;

    reg signed [DATA_W-1:0] a3_pre;
    reg signed [DATA_W-1:0] a2_pre;
    reg signed [DATA_W-1:0] a1_pre;
    reg signed [DATA_W-1:0] a0_pre;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            valid_pre    <= 1'b0;

            sym_mode_pre <= 3'd0;
            neg_pre      <= 1'b0;
            x_eval_pre   <= {DATA_W{1'b0}};

            degree_pre   <= 2'd0;
            mode_pre     <= 2'd0;

            a3_pre       <= {DATA_W{1'b0}};
            a2_pre       <= {DATA_W{1'b0}};
            a1_pre       <= {DATA_W{1'b0}};
            a0_pre       <= {DATA_W{1'b0}};
        end else begin
            valid_pre <= start;

            if (start) begin
                sym_mode_pre <= sym_mode;
                neg_pre      <= neg_front;
                x_eval_pre   <= x_eval_front;

                degree_pre   <= degree_sel;
                mode_pre     <= mode_sel;

                a3_pre       <= a3_sel;
                a2_pre       <= a2_sel;
                a1_pre       <= a1_sel;
                a0_pre       <= a0_sel;
            end
        end
    end

    // ------------------------------------------------------------------
    // Three-stage Horner pipeline fed exclusively from PRE registers.
    //
    // REF09 deliberately does not place a second layer of operand-isolation
    // muxes between PRE and MAC3. PRE already snapshots/holds the operands,
    // and 'start' below controls whether the Horner registers accept them.
    // ------------------------------------------------------------------
    wire horner_sample_en;

    assign horner_sample_en =
        valid_pre && (mode_pre == CAMTA_MODE_HORNER);

    wire                     horner_busy;
    wire                     horner_done;
    wire signed [DATA_W-1:0] horner_y;

    camta_horner_seq #(
        .DATA_W(DATA_W),
        .DATA_I(DATA_I),
        .ACC_W(ACC_W),
        .ACC_I(ACC_I)
    ) u_horner (
        .clk       (clk),
        .rst_n     (rst_n),
        .start     (horner_sample_en),
        .x_in      (x_eval_pre),
        .degree_in (degree_pre),
        .a3_in     (a3_pre),
        .a2_in     (a2_pre),
        .a1_in     (a1_pre),
        .a0_in     (a0_pre),
        .busy      (horner_busy),
        .done      (horner_done),
        .y_out     (horner_y)
    );

    // ------------------------------------------------------------------
    // Direct-path / symmetry metadata alignment.
    //
    // PRE is already the first external pipeline stage. These three delay
    // stages align non-Horner modes and post-symmetry metadata with the
    // three Horner arithmetic stages:
    //
    //   PRE -> S1/MAC3 -> S2/MAC2 -> S3/MAC1
    // ------------------------------------------------------------------
    reg valid_s1;
    reg valid_s2;
    reg valid_s3;

    reg [2:0]               sym_mode_s1;
    reg [2:0]               sym_mode_s2;
    reg [2:0]               sym_mode_s3;

    reg                     neg_s1;
    reg                     neg_s2;
    reg                     neg_s3;

    reg signed [DATA_W-1:0] x_eval_s1;
    reg signed [DATA_W-1:0] x_eval_s2;
    reg signed [DATA_W-1:0] x_eval_s3;

    reg [1:0]               mode_s1;
    reg [1:0]               mode_s2;
    reg [1:0]               mode_s3;

    reg signed [DATA_W-1:0] a0_s1;
    reg signed [DATA_W-1:0] a0_s2;
    reg signed [DATA_W-1:0] a0_s3;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            valid_s1    <= 1'b0;
            valid_s2    <= 1'b0;
            valid_s3    <= 1'b0;

            sym_mode_s1 <= 3'd0;
            sym_mode_s2 <= 3'd0;
            sym_mode_s3 <= 3'd0;

            neg_s1      <= 1'b0;
            neg_s2      <= 1'b0;
            neg_s3      <= 1'b0;

            x_eval_s1   <= {DATA_W{1'b0}};
            x_eval_s2   <= {DATA_W{1'b0}};
            x_eval_s3   <= {DATA_W{1'b0}};

            mode_s1     <= 2'd0;
            mode_s2     <= 2'd0;
            mode_s3     <= 2'd0;

            a0_s1       <= {DATA_W{1'b0}};
            a0_s2       <= {DATA_W{1'b0}};
            a0_s3       <= {DATA_W{1'b0}};
        end else begin
            valid_s1 <= valid_pre;
            valid_s2 <= valid_s1;
            valid_s3 <= valid_s2;

            if (valid_pre) begin
                sym_mode_s1 <= sym_mode_pre;
                neg_s1      <= neg_pre;
                x_eval_s1   <= x_eval_pre;
                mode_s1     <= mode_pre;
                a0_s1       <= a0_pre;
            end

            if (valid_s1) begin
                sym_mode_s2 <= sym_mode_s1;
                neg_s2      <= neg_s1;
                x_eval_s2   <= x_eval_s1;
                mode_s2     <= mode_s1;
                a0_s2       <= a0_s1;
            end

            if (valid_s2) begin
                sym_mode_s3 <= sym_mode_s2;
                neg_s3      <= neg_s2;
                x_eval_s3   <= x_eval_s2;
                mode_s3     <= mode_s2;
                a0_s3       <= a0_s2;
            end
        end
    end

    // ------------------------------------------------------------------
    // Common output stage.
    //
    // horner_y, mode_s3, x_eval_s3, a0_s3, neg_s3, and sym_mode_s3 all
    // correspond to the same original input sample.
    // ------------------------------------------------------------------
    wire signed [DATA_W-1:0] mode_selected_y;

    camta_mode_bypass #(
        .DATA_W(DATA_W)
    ) u_mode_bypass (
        .mode      (mode_s3),
        .x_eval_in (x_eval_s3),
        .a0_in     (a0_s3),
        .y_horner  (horner_y),
        .y_out     (mode_selected_y)
    );

    camta_symmetry_post #(
        .DATA_W(DATA_W),
        .DATA_I(DATA_I)
    ) u_sym_post (
        .sym_mode (sym_mode_s3),
        .neg_flag (neg_s3),
        .x_eval   (x_eval_s3),
        .y_in     (mode_selected_y),
        .y_out    (y_out)
    );

    assign done = valid_s3;
    assign busy = valid_pre | valid_s1 | valid_s2 | valid_s3;

endmodule
