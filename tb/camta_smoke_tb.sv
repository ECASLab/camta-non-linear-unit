`timescale 1ns/1ps

module camta_smoke_tb;
    localparam time CLK_PERIOD = 4ns;

    reg clk;
    reg rst_n;
    reg start;
    reg [2:0] sym_mode;
    reg signed [15:0] x_in;
    reg signed [15:0] L_left;
    reg signed [15:0] L_right;
    reg [1:0] deg_r0;
    reg [1:0] deg_r1;
    reg [1:0] deg_r2;
    reg [1:0] mode_r0;
    reg [1:0] mode_r1;
    reg [1:0] mode_r2;
    reg signed [15:0] a3_r0;
    reg signed [15:0] a2_r0;
    reg signed [15:0] a1_r0;
    reg signed [15:0] a0_r0;
    reg signed [15:0] a3_r1;
    reg signed [15:0] a2_r1;
    reg signed [15:0] a1_r1;
    reg signed [15:0] a0_r1;
    reg signed [15:0] a3_r2;
    reg signed [15:0] a2_r2;
    reg signed [15:0] a1_r2;
    reg signed [15:0] a0_r2;
    wire busy;
    wire done;
    wire signed [15:0] y_out;

    camta_unit_core dut (
        .clk(clk), .rst_n(rst_n), .start(start),
        .sym_mode(sym_mode), .x_in(x_in),
        .L_left(L_left), .L_right(L_right),
        .deg_r0(deg_r0), .deg_r1(deg_r1), .deg_r2(deg_r2),
        .mode_r0(mode_r0), .mode_r1(mode_r1), .mode_r2(mode_r2),
        .a3_r0(a3_r0), .a2_r0(a2_r0), .a1_r0(a1_r0), .a0_r0(a0_r0),
        .a3_r1(a3_r1), .a2_r1(a2_r1), .a1_r1(a1_r1), .a0_r1(a0_r1),
        .a3_r2(a3_r2), .a2_r2(a2_r2), .a1_r2(a1_r2), .a0_r2(a0_r2),
        .busy(busy), .done(done), .y_out(y_out)
    );

    initial begin
        clk = 1'b0;
        forever #(CLK_PERIOD / 2) clk = ~clk;
    end

    task automatic initialize_inputs;
        begin
            start = 1'b0;
            sym_mode = 3'd0;
            x_in = 16'sd0;
            L_left = -16'sd4096;
            L_right = 16'sd4096;
            deg_r0 = 2'd0;
            deg_r1 = 2'd0;
            deg_r2 = 2'd0;
            mode_r0 = 2'd3;
            mode_r1 = 2'd3;
            mode_r2 = 2'd3;
            a3_r0 = 16'sd0; a2_r0 = 16'sd0; a1_r0 = 16'sd0; a0_r0 = 16'sd0;
            a3_r1 = 16'sd0; a2_r1 = 16'sd0; a1_r1 = 16'sd0; a0_r1 = 16'sd0;
            a3_r2 = 16'sd0; a2_r2 = 16'sd0; a1_r2 = 16'sd0; a0_r2 = 16'sd0;
        end
    endtask

    initial begin
        initialize_inputs();
        rst_n = 1'b0;
        repeat (3) @(posedge clk);
        @(negedge clk);
        rst_n = 1'b1;
        repeat (2) @(posedge clk);

        // Two consecutive identity samples. The second request is accepted
        // while busy=1 and both outputs must emerge on consecutive cycles.
        @(negedge clk);
        x_in  = 16'sd512;
        start = 1'b1;

        @(negedge clk);
        if (!busy)
            $fatal(1, "Pipeline did not report busy after first request");
        x_in  = -16'sd700;
        start = 1'b1;

        @(negedge clk);
        start = 1'b0;

        @(negedge clk);
        @(negedge clk);
        if (!done || ($signed(y_out) !== 16'sd512))
            $fatal(1, "First streamed output failed: done=%0b y=%0d", done, $signed(y_out));
        $display("PASS: streamed output 0 y=%0d", $signed(y_out));

        @(negedge clk);
        if (!done || ($signed(y_out) !== -16'sd700))
            $fatal(1, "Second streamed output failed: done=%0b y=%0d", done, $signed(y_out));
        $display("PASS: streamed output 1 y=%0d", $signed(y_out));

        $display("CAMTA PIPELINED RTL SMOKE TEST PASSED");
        $finish;
    end
endmodule
