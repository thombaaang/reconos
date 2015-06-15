<<reconos_preproc>>library ieee;
use ieee.std_logic_1164.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;
use reconos_v3_01_a.reconos_defs.all;
use reconos_v3_01_a.reconos_testbench.all;

entity stimulus is
	port (
		OSIF_Ic2Sw_Data    : in  std_logic_vector(31 downto 0);
		OSIF_Ic2Sw_Empty   : in  std_logic;
		OSIF_Ic2Sw_RE      : out std_logic;

		OSIF_Sw2Ic_Data    : out std_logic_vector(31 downto 0);
		OSIF_Sw2Ic_Full    : in  std_logic;
		OSIF_Sw2Ic_WE      : out std_logic;

		SYS_Rst     : out std_logic;
		SLOT_Signal : out std_logic_vector(<<NUM_SLOTS>> - 1 downto 0)
	);
end entity stimulus;

architecture stimulus of stimulus is
	signal tb_i_osif : tb_i_osif_t;
	signal tb_o_osif : tb_o_osif_t;
begin
	SLOT_Signal <= (others => '0');

	tb_osif_setup (
		tb_i_osif,
		tb_o_osif,
		OSIF_Ic2Sw_Data,
		OSIF_Ic2Sw_Empty,
		OSIF_Ic2Sw_RE,
		OSIF_Sw2Ic_Data,
		OSIF_Sw2Ic_Full,
		OSIF_Sw2Ic_WE
	);

	stimulus : process
	begin
		SYS_Rst <= '1';
		tb_osif_reset(tb_o_osif);
		wait for C_CLK_PRD * 10;
		SYS_Rst <= '0';
		wait for C_CLK_PRD;
		
		tb_o_osif.hw2sw_re <= '1';
		
		tb_o_osif.sw2hw_data <= x"FF000002";
		tb_o_osif.sw2hw_we <= '1';
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= OSIF_SIGNAL_THREAD_START;
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= x"00000000";
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_we <= '0';
		wait for C_CLK_PRD;
				
		tb_o_osif.sw2hw_data <= x"FF010002";
		tb_o_osif.sw2hw_we <= '1';
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= OSIF_SIGNAL_THREAD_START;
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= x"00000001";
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_we <= '0';
		wait for C_CLK_PRD;
				
		tb_o_osif.sw2hw_data <= x"FF020002";
		tb_o_osif.sw2hw_we <= '1';
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= OSIF_SIGNAL_THREAD_START;
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= x"00000002";
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_we <= '0';
		wait for C_CLK_PRD;
				
		tb_o_osif.sw2hw_data <= x"FF030002";
		tb_o_osif.sw2hw_we <= '1';
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= OSIF_SIGNAL_THREAD_START;
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= x"00000003";
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_we <= '0';
		wait for C_CLK_PRD;

		-- first mbox get read
		tb_o_osif.sw2hw_data <= x"FF000001";
		tb_o_osif.sw2hw_we <= '1';
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= x"00000000";
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_we <= '0';
		wait for C_CLK_PRD * 10;
		
		-- second mbox get read
		tb_o_osif.sw2hw_data <= x"FF000001";
		tb_o_osif.sw2hw_we <= '1';
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= x"00000004";
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_we <= '0';
		wait for C_CLK_PRD * 10;
				
		-- second mbox get write
		tb_o_osif.sw2hw_data <= x"FF010001";
		tb_o_osif.sw2hw_we <= '1';
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= x"00000800";
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_we <= '0';
		wait for C_CLK_PRD * 10;
				
		-- second mbox get write
		tb_o_osif.sw2hw_data <= x"FF010001";
		tb_o_osif.sw2hw_we <= '1';
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_data <= x"00000004";
		wait for C_CLK_PRD;
		tb_o_osif.sw2hw_we <= '0';
		wait for C_CLK_PRD * 10;

		wait;
	end process stimulus;
end architecture stimulus;

