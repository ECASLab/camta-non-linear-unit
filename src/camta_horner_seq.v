module camta_horner_seq (
    input  wire               clk,
    input  wire               rst_n,
    input  wire               start,

    input  wire signed [15:0] x_in,       // Q6.10
    input  wire signed [3:0]  degree_in,  // signed, clamped to [0..3], <0 => 0 output
    input  wire signed [15:0] a3_in,      // Q6.10
    input  wire signed [15:0] a2_in,
    input  wire signed [15:0] a1_in,
    input  wire signed [15:0] a0_in,

    output reg                busy,
    output reg                done,
    output reg  signed [15:0] y_out       // Q6.10
);

    localparam integer X_FRAC   = 10;
    localparam integer ACC_FRAC = 14;

    reg signed [15:0] x_reg;
    reg signed [3:0]  degree_reg;
    reg signed [15:0] a3_reg, a2_reg, a1_reg, a0_reg;

    reg signed [23:0] acc_reg;            // Q10.14
    reg [2:0]         step;

    reg signed [3:0]  degree_eff;
    reg signed [15:0] coeff_q610;
    reg signed [23:0] coeff_q1014;
    reg signed [23:0] acc_next;

    wire signed [39:0] mult_full_q1624;
    wire signed [23:0] mult_scaled_q1014;

    assign mult_full_q1624   = $signed(acc_reg) * $signed(x_reg); // Q10.14 * Q6.10 = Q16.24
    assign mult_scaled_q1014 = mult_full_q1624 >>> X_FRAC;        // back to Q10.14

    always @(*) begin
        if (degree_reg < 0)
            degree_eff = -1;
        else if (degree_reg > 3)
            degree_eff = 3;
        else
            degree_eff = degree_reg;
    end

    always @(*) begin
        case (step)
            3'd0: coeff_q610 = a3_reg;
            3'd1: coeff_q610 = a2_reg;
            3'd2: coeff_q610 = a1_reg;
            3'd3: coeff_q610 = a0_reg;
            default: coeff_q610 = 16'sd0;
        endcase
    end

    always @(*) begin
        coeff_q1014 = {{8{coeff_q610[15]}}, coeff_q610} <<< (ACC_FRAC - X_FRAC);

        acc_next = acc_reg;
        case (step)
            3'd0: if (3 <= degree_eff) acc_next = mult_scaled_q1014 + coeff_q1014;
            3'd1: if (2 <= degree_eff) acc_next = mult_scaled_q1014 + coeff_q1014;
            3'd2: if (1 <= degree_eff) acc_next = mult_scaled_q1014 + coeff_q1014;
            3'd3: if (0 <= degree_eff) acc_next = mult_scaled_q1014 + coeff_q1014;
            default: acc_next = acc_reg;
        endcase
    end

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            x_reg      <= 16'sd0;
            degree_reg <= 4'sd0;
            a3_reg     <= 16'sd0;
            a2_reg     <= 16'sd0;
            a1_reg     <= 16'sd0;
            a0_reg     <= 16'sd0;
            acc_reg    <= 24'sd0;
            step       <= 3'd0;
            busy       <= 1'b0;
            done       <= 1'b0;
            y_out      <= 16'sd0;
        end else begin
            done <= 1'b0;

            if (start && !busy) begin
                x_reg      <= x_in;
                degree_reg <= degree_in;
                a3_reg     <= a3_in;
                a2_reg     <= a2_in;
                a1_reg     <= a1_in;
                a0_reg     <= a0_in;

                acc_reg    <= 24'sd0;
                step       <= 3'd0;
                busy       <= 1'b1;
            end
            else if (busy) begin
                acc_reg <= acc_next;

                if (step == 3'd3) begin
                    busy  <= 1'b0;
                    done  <= 1'b1;
                    y_out <= acc_next >>> (ACC_FRAC - X_FRAC); // Q10.14 -> Q6.10
                end

                step <= step + 3'd1;
            end
        end
    end

endmodule