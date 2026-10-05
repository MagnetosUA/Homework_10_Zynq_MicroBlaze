`timescale 1ns / 1ps

module tb_microblaze;

    reg sys_clock = 0;

    reg  [0:0] GPIO2_0_tri_i = 1'b0;   // SW0
    reg  [2:0] GPIO_1_tri_i  = 3'b000; // Buttons

    wire [3:0] GPIO_0_tri_o;            // LEDs

    // 100 MHz clock
    always #5 sys_clock = ~sys_clock;

    design_1_wrapper dut (
        .GPIO2_0_tri_i(GPIO2_0_tri_i),
        .GPIO_0_tri_o(GPIO_0_tri_o),
        .GPIO_1_tri_i(GPIO_1_tri_i),
        .sys_clock(sys_clock)
    );

    initial begin
        // Initial state
        GPIO2_0_tri_i = 1'b0;
        GPIO_1_tri_i  = 3'b000;
    
        // Let MicroBlaze boot and start running
        #500000;       // 0.5 ms
    
        // Normal forward movement (2 ms period)
        #5000000;      // until ~5.5 ms
    
        // BTN0: faster -> timer becomes 1 ms
        GPIO_1_tri_i[0] = 1'b1;
        #100000;       // button held 0.1 ms
        GPIO_1_tri_i[0] = 1'b0;
    
        #4000000;
    
        // BTN1: slower -> timer becomes 2 ms again
        GPIO_1_tri_i[1] = 1'b1;
        #100000;
        GPIO_1_tri_i[1] = 1'b0;
    
        #4000000;
    
        // BTN2: STOP
        GPIO_1_tri_i[2] = 1'b1;
        #100000;
        GPIO_1_tri_i[2] = 1'b0;
    
        // LED should stay unchanged here
        #4000000;
    
        // BTN2: RESUME
        GPIO_1_tri_i[2] = 1'b1;
        #100000;
        GPIO_1_tri_i[2] = 1'b0;
    
        #4000000;
    
        // SW0 = 1: reverse direction
        GPIO2_0_tri_i = 1'b1;
    
        #6000000;
    
        $finish;
    end
endmodule