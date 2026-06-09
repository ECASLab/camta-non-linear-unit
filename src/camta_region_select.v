module camta_region_select (
    input  wire signed [15:0] x,        // Q6.10
    input  wire signed [15:0] L_left,   // Q6.10
    input  wire signed [15:0] L_right,  // Q6.10
    output reg  [1:0]         region
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