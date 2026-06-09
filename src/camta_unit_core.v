module camta_unit_core (
    input  wire               clk,
    input  wire               rst_n,
    input  wire               start,

    input  wire signed [15:0] x_in,       // Q6.10
    input  wire signed [15:0] L_left,     // Q6.10
    input  wire signed [15:0] L_right,    // Q6.10

    input  wire signed [3:0]  deg_r0,
    input  wire signed [3:0]  deg_r1,
    input  wire signed [3:0]  deg_r2,

    input  wire [1:0]         mode_r0,
    input  wire [1:0]         mode_r1,
    input  wire [1:0]         mode_r2,

    input  wire signed [15:0] a3_r0,
    input  wire signed [15:0] a2_r0,
    input  wire signed [15:0] a1_r0,
    input  wire signed [15:0] a0_r0,

    input  wire signed [15:0] a3_r1,
    input  wire signed [15:0] a2_r1,
    input  wire signed [15:0] a1_r1,
    input  wire signed [15:0] a0_r1,

    input  wire signed [15:0] a3_r2,
    input  wire signed [15:0] a2_r2,
    input  wire signed [15:0] a1_r2,
    input  wire signed [15:0] a0_r2,

    output reg                busy,
    output reg                done,
    output reg  signed [15:0] y_out       // Q6.10
);

    localparam [1:0] ST_IDLE   = 2'd0;
    localparam [1:0] ST_DIRECT = 2'd1;
    localparam [1:0] ST_HORNER = 2'd2;

    localparam [1:0] CAMTA_MODE_HORNER   = 2'd0;
    localparam [1:0] CAMTA_MODE_CONST    = 2'd1;
    localparam [1:0] CAMTA_MODE_ZERO     = 2'd2;
    localparam [1:0] CAMTA_MODE_IDENTITY = 2'd3;

    reg [1:0] state;

    wire [1:0] region_sel;

    reg signed [15:0] x_reg;
    reg signed [3:0]  deg_sel_reg;
    reg [1:0]         mode_sel_reg;
    reg signed [15:0] a3_sel_reg, a2_sel_reg, a1_sel_reg, a0_sel_reg;

    reg               horner_start;
    wire              horner_busy;
    wire              horner_done;
    wire signed [15:0] horner_y;

    wire signed [15:0] direct_y;

    camta_region_select u_region (
        .x      (x_in),
        .L_left (L_left),
        .L_right(L_right),
        .region (region_sel)
    );

    camta_horner_seq u_horner (
        .clk      (clk),
        .rst_n    (rst_n),
        .start    (horner_start),
        .x_in     (x_reg),
        .degree_in(deg_sel_reg),
        .a3_in    (a3_sel_reg),
        .a2_in    (a2_sel_reg),
        .a1_in    (a1_sel_reg),
        .a0_in    (a0_sel_reg),
        .busy     (horner_busy),
        .done     (horner_done),
        .y_out    (horner_y)
    );

    camta_mode_bypass u_mode_bypass (
        .mode    (mode_sel_reg),
        .x_in    (x_reg),
        .a0_in   (a0_sel_reg),
        .y_horner(horner_y),
        .y_out   (direct_y)
    );

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state      <= ST_IDLE;
            busy       <= 1'b0;
            done       <= 1'b0;
            y_out      <= 16'sd0;
            x_reg      <= 16'sd0;
            deg_sel_reg  <= 4'sd0;
            mode_sel_reg <= CAMTA_MODE_HORNER;
            a3_sel_reg <= 16'sd0;
            a2_sel_reg <= 16'sd0;
            a1_sel_reg <= 16'sd0;
            a0_sel_reg <= 16'sd0;
            horner_start <= 1'b0;
        end else begin
            done <= 1'b0;
            horner_start <= 1'b0;

            case (state)
                ST_IDLE: begin
                    busy <= 1'b0;

                    if (start) begin
                        x_reg <= x_in;

                        case (region_sel)
                            2'd0: begin
                                deg_sel_reg  <= deg_r0;
                                mode_sel_reg <= mode_r0;
                                a3_sel_reg   <= a3_r0;
                                a2_sel_reg   <= a2_r0;
                                a1_sel_reg   <= a1_r0;
                                a0_sel_reg   <= a0_r0;
                            end

                            2'd2: begin
                                deg_sel_reg  <= deg_r2;
                                mode_sel_reg <= mode_r2;
                                a3_sel_reg   <= a3_r2;
                                a2_sel_reg   <= a2_r2;
                                a1_sel_reg   <= a1_r2;
                                a0_sel_reg   <= a0_r2;
                            end

                            default: begin
                                deg_sel_reg  <= deg_r1;
                                mode_sel_reg <= mode_r1;
                                a3_sel_reg   <= a3_r1;
                                a2_sel_reg   <= a2_r1;
                                a1_sel_reg   <= a1_r1;
                                a0_sel_reg   <= a0_r1;
                            end
                        endcase

                        busy <= 1'b1;

                        case (region_sel)
                            2'd0: begin
                                if (mode_r0 == CAMTA_MODE_HORNER)
                                    state <= ST_HORNER;
                                else
                                    state <= ST_DIRECT;
                            end

                            2'd2: begin
                                if (mode_r2 == CAMTA_MODE_HORNER)
                                    state <= ST_HORNER;
                                else
                                    state <= ST_DIRECT;
                            end

                            default: begin
                                if (mode_r1 == CAMTA_MODE_HORNER)
                                    state <= ST_HORNER;
                                else
                                    state <= ST_DIRECT;
                            end
                        endcase
                    end
                end

                ST_DIRECT: begin
                    y_out <= direct_y;
                    done  <= 1'b1;
                    busy  <= 1'b0;
                    state <= ST_IDLE;
                end

                ST_HORNER: begin
                    if (!horner_busy && !horner_start)
                        horner_start <= 1'b1;

                    if (horner_done) begin
                        y_out <= horner_y;
                        done  <= 1'b1;
                        busy  <= 1'b0;
                        state <= ST_IDLE;
                    end
                end

                default: begin
                    state <= ST_IDLE;
                    busy  <= 1'b0;
                end
            endcase
        end
    end

endmodule