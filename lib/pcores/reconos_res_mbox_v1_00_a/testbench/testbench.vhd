--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   17:03:00 05/13/2015
-- Design Name:   
-- Module Name:   /home/christoph/Documents/github/reconos/lib/pcores/reconos_res_mbox_v1_00_a/testbench/testbench.vhd
-- Project Name:  mbox
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: top
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY testbench IS
END testbench;
 
ARCHITECTURE behavior OF testbench IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT top
    PORT(
         ap_clk : IN  std_logic;
         ap_rst : IN  std_logic;
         osif_in_V_V_dout : IN  std_logic_vector(31 downto 0);
         osif_in_V_V_empty_n : IN  std_logic;
         osif_in_V_V_read : OUT  std_logic;
         osif_out_V_V_din : OUT  std_logic_vector(31 downto 0);
         osif_out_V_V_full_n : IN  std_logic;
         osif_out_V_V_write : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal ap_clk : std_logic := '0';
   signal ap_rst : std_logic := '0';
   signal osif_in_V_V_dout : std_logic_vector(31 downto 0) := (others => '0');
	signal osif_in_V_V_empty : std_logic := '0';
   signal osif_in_V_V_empty_n : std_logic := '0';
	signal osif_out_V_V_full :  std_logic := '0';
   signal osif_out_V_V_full_n : std_logic := '0';

 	--Outputs
   signal osif_in_V_V_read : std_logic;
   signal osif_out_V_V_din : std_logic_vector(31 downto 0);
   signal osif_out_V_V_write : std_logic;
	
	signal tb2res_data, res2tb_data : std_logic_vector(31 downto 0);
	signal tb2res_full, res2tb_empty: std_logic;
	signal tb2res_we, res2tb_re : std_logic;

   -- Clock period definitions
   constant ap_clk_period : time := 10 ns;
 
BEGIN
 
	osif_in_V_V_empty_n <= not osif_in_V_V_empty;
   osif_out_V_V_full_n <= not osif_out_V_V_full;
 
	-- Instantiate the Unit Under Test (UUT)
   uut: top PORT MAP (
          ap_clk => ap_clk,
          ap_rst => ap_rst,
          osif_in_V_V_dout => osif_in_V_V_dout,
          osif_in_V_V_empty_n => osif_in_V_V_empty_n,
          osif_in_V_V_read => osif_in_V_V_read,
          osif_out_V_V_din => osif_out_V_V_din,
          osif_out_V_V_full_n => osif_out_V_V_full_n,
          osif_out_V_V_write => osif_out_V_V_write
        );

	tb2res: entity work.reconos_fifo_sync
		generic map (
			C_FIFO_ADDR_WIDTH => 4
		)
		port map (
			FIFO_S_Data => osif_in_V_V_dout,
			FIFO_S_Empty => osif_in_V_V_empty,
			FIFO_S_RE => osif_in_V_V_read,
			FIFO_M_Data => tb2res_data,
			FIFO_M_Full => tb2res_full,
			FIFO_M_WE => tb2res_we,
			FIFO_Clk => ap_clk,
			FIFO_Rst => ap_rst
		);
	
	res2tb: entity work.reconos_fifo_sync
		generic map (
			C_FIFO_ADDR_WIDTH => 4
		)
		port map (
			FIFO_S_Data => res2tb_data,
			FIFO_S_Empty => res2tb_empty,
			FIFO_S_RE => res2tb_re,
			FIFO_M_Data => osif_out_V_V_din,
			FIFO_M_Full => osif_out_V_V_full,
			FIFO_M_WE => osif_out_V_V_write,
			FIFO_Clk => ap_clk,
			FIFO_Rst => ap_rst
		);

   -- Clock process definitions
   ap_clk_process :process
   begin
		ap_clk <= '0';
		wait for ap_clk_period/2;
		ap_clk <= '1';
		wait for ap_clk_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      ap_rst <= '1';
		osif_in_V_V_empty_n <= '0';
		osif_out_V_V_full_n <= '1';
      wait for 100 ns;	
		ap_rst <= '0';
      wait for ap_clk_period*10;
		
		tb2res_we <= '1';
		-- get
		tb2res_data <= x"41010001";
		wait for ap_clk_period;
		tb2res_data <= x"000000F0";
		wait for ap_clk_period;
		-- get
		tb2res_data <= x"42010001";
		wait for ap_clk_period;
		tb2res_data <= x"000000F0";
		wait for ap_clk_period;
		-- put
		tb2res_data <= x"AF010002";
		wait for ap_clk_period;
		tb2res_data <= x"000000F1";
		wait for ap_clk_period;
		tb2res_data <= x"AFFEDEAD";
		wait for ap_clk_period;
		-- put
		tb2res_data <= x"AF010002";
		wait for ap_clk_period;
		tb2res_data <= x"000000F1";
		wait for ap_clk_period;
		tb2res_data <= x"DEADAFFE";
		wait for ap_clk_period;
		tb2res_we <= '0';
		res2tb_re <= '1';
		
		--osif_in_V_V_dout <= x"00010001";
		--osif_in_V_V_empty_n <= '1';
		--wait for ap_clk_period * 4;
		--osif_in_V_V_dout <= x"000000F0";
		--wait for ap_clk_period;
		--osif_in_V_V_empty_n <= '0';
		--wait for ap_clk_period * 5;
		
		--osif_in_V_V_dout <= x"00010002";
		--osif_in_V_V_empty_n <= '1';
		--wait for ap_clk_period * 4;
		--osif_in_V_V_dout <= x"000000F1";
		--wait for ap_clk_period;
		--osif_in_V_V_dout <= x"AFFEDEAD";
		--wait for ap_clk_period;
		--osif_in_V_V_empty_n <= '0';
		--wait for ap_clk_period * 5;

      wait;
   end process;

END;
