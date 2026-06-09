`timescale 1ns/1ps

module tb_camta_unit_core;

    reg clk;
    reg rst_n;
    reg start;

    reg  signed [15:0] x_in;
    reg  signed [15:0] L_left;
    reg  signed [15:0] L_right;

    reg  signed [3:0]  deg_r0, deg_r1, deg_r2;
    reg  [1:0]         mode_r0, mode_r1, mode_r2;

    reg  signed [15:0] a3_r0, a2_r0, a1_r0, a0_r0;
    reg  signed [15:0] a3_r1, a2_r1, a1_r1, a0_r1;
    reg  signed [15:0] a3_r2, a2_r2, a1_r2, a0_r2;

    wire busy;
    wire done;
    wire signed [15:0] y_out;

    camta_unit_core dut (
        .clk(clk),
        .rst_n(rst_n),
        .start(start),
        .x_in(x_in),
        .L_left(L_left),
        .L_right(L_right),

        .deg_r0(deg_r0),
        .deg_r1(deg_r1),
        .deg_r2(deg_r2),

        .mode_r0(mode_r0),
        .mode_r1(mode_r1),
        .mode_r2(mode_r2),

        .a3_r0(a3_r0), .a2_r0(a2_r0), .a1_r0(a1_r0), .a0_r0(a0_r0),
        .a3_r1(a3_r1), .a2_r1(a2_r1), .a1_r1(a1_r1), .a0_r1(a0_r1),
        .a3_r2(a3_r2), .a2_r2(a2_r2), .a1_r2(a1_r2), .a0_r2(a0_r2),

        .busy(busy),
        .done(done),
        .y_out(y_out)
    );

    initial begin
        clk = 1'b0;
        forever #5 clk = ~clk;
    end

    function [15:0] q610;
        input real val;
        integer temp;
        begin
            temp = $rtoi(val * 1024.0);
            q610 = temp[15:0];
        end
    endfunction

    function real q610_to_real;
        input signed [15:0] val;
        begin
            q610_to_real = $itor(val) / 1024.0;
        end
    endfunction

    task run_sample;
        input signed [15:0] xin;
        begin
            @(posedge clk);
            x_in   <= xin;
            start  <= 1'b1;

            @(posedge clk);
            start  <= 1'b0;

            wait(done == 1'b1);
            $display("x=%f -> y=%f", q610_to_real(xin), q610_to_real(y_out));
            @(posedge clk);
        end
    endtask

    initial begin
        rst_n = 1'b0;
        start = 1'b0;
        x_in  = 16'sd0;

        L_left  = q610(-2.0);
        L_right = q610( 2.0);

        // r0 = CONST(-1)
        deg_r0  = 4'sd0;
        mode_r0 = 2'd1;
        a3_r0 = q610(0.0); a2_r0 = q610(0.0); a1_r0 = q610(0.0); a0_r0 = q610(-1.0);

        // r1 = HORNER : 0.25*x^2 + 0.5*x
        deg_r1  = 4'sd2;
        mode_r1 = 2'd0;
        a3_r1 = q610(0.0); a2_r1 = q610(0.25); a1_r1 = q610(0.5); a0_r1 = q610(0.0);

        // r2 = IDENTITY
        deg_r2  = 4'sd0;
        mode_r2 = 2'd3;
        a3_r2 = q610(0.0); a2_r2 = q610(0.0); a1_r2 = q610(0.0); a0_r2 = q610(0.0);

        repeat (4) @(posedge clk);
        rst_n = 1'b1;

        run_sample(q610(-4.0));   // region 0 -> const
        run_sample(q610(-1.0));   // region 1 -> horner
        run_sample(q610(0.5));    // region 1 -> horner
        run_sample(q610(4.0));    // region 2 -> identity

        repeat (10) @(posedge clk);
        $finish;
    end

endmodule