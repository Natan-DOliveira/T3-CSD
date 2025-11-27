parameter [31:0] DATA_WIDTH = 8;

module keyboard_axis
(
	// external interface signals | teclado PS/2
	input wire ps2_clk,
	input wire ps2_data,
	// AXI stream interface signals
	input wire axis_aclk_i,
	input wire axis_aresetn_i,
	// master axi stream interface
	input wire m_axis_tready_i,
	output wire m_axis_tvalid_o,
	output reg [DATA_WIDTH - 1:0] m_axis_tdata_o
);
	reg tvalid;
	parameter [0:0] st_idle = 0, st_data = 1;
	reg state;
	wire [DATA_WIDTH - 1:0] data;
	
    reg ps2_code_new_prev;
    wire new_key_event;
    always @(posedge axis_aclk_i) begin
        ps2_code_new_prev <= ps2_code_new_w;
    end
    
    assign new_key_event = (ps2_code_new_w == 1'b1 && ps2_code_new_prev == 1'b0);
    assign m_axis_tvalid_o = tvalid;
	
	//registrador, sinais e instância do ps2_keyboard
	reg [7:0] key_scancode_reg;
	wire ps2_code_new;
	wire [7:0] ps2_code_w;
	ps2_keyboard ps2_kbd (
		.clk(axis_aclk_i),
		.ps2_clk(ps2_clk),
		.ps2_data(ps2_data),
		.ps2_code_new(ps2_code_new_w),
		.ps2_code(ps2_code_w)
	);
	
	always @(posedge axis_aclk_i) begin
		if (ps2_code_new_w == 1'b1) begin
			key_scancode_reg <=ps2_code_w;
		end
	end

	always @(posedge axis_aclk_i, negedge axis_aresetn_i) begin
		if (axis_aresetn_i == 1'b0) begin
			state           <= st_idle;
			tvalid          <= 1'b0;
			m_axis_tdata_o  <= 0;
		end 
		else begin
			case (state)
			st_idle : begin
				if (new_key_event == 1'b1) begin
					state           <= st_data;
					tvalid          <= 1'b1;
					m_axis_tdata_o  <= ps2_code_w;
				end
			end
			st_data : begin
				if (m_axis_tready_i == 1'b1) begin
					state  <= st_idle;
					tvalid <= 1'b0;
				end
			end
			endcase
		end
	end
endmodule
