<<reconos_preproc>>

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity hwres_<<NAME>><<ID>> is
	port (
		OSIF_In_Data    : in  std_logic_vector(31 downto 0);
		OSIF_In_Empty   : in  std_logic;
		OSIF_In_RE      : out std_logic;

		OSIF_Out_Data    : out std_logic_vector(31 downto 0);
		OSIF_Out_Full    : in  std_logic;
		OSIF_Out_WE      : out std_logic;

		SYS_Clk    : in  std_logic;
		SYS_Rst    : in  std_logic
	);
end entity hwres_<<NAME>><<ID>>;

architecture implementation of hwres_<<NAME>><<ID>> is
	component hwres_imp is
		port (
			ap_clk : in std_logic;
			ap_rst : in std_logic;

			osif_in_V_V_dout    : in std_logic_vector (31 downto 0);
			osif_in_V_V_empty_n : in std_logic;
			osif_in_V_V_read    : out std_logic;

			osif_out_V_V_din     : out std_logic_vector (31 downto 0);
			osif_out_V_V_full_n  : in std_logic;
			osif_out_V_V_write   : out std_logic
		);
  	end component;

	signal osif_in_V_V_dout    : std_logic_vector(31 downto 0);
	signal osif_in_V_V_empty_n : std_logic;
	signal osif_in_V_V_read    : std_logic;

	signal osif_out_V_V_din     : std_logic_vector(31 downto 0);
	signal osif_out_V_V_full_n  : std_logic;
	signal osif_out_V_V_write   : std_logic;
begin
	osif_in_V_V_dout    <= OSIF_In_Data;
	osif_in_V_V_empty_n <= not OSIF_In_Empty;
	OSIF_In_RE          <= osif_in_V_V_read;

	OSIF_Out_Data        <= osif_out_V_V_din;
	osif_out_V_V_full_n  <= not OSIF_Out_Full;
	OSIF_Out_WE          <= osif_out_V_V_write;

	hwres_imp_inst : hwres_imp
		port map (
			ap_clk => SYS_Clk,
			ap_rst => SYS_Rst,

			osif_in_V_V_dout    => osif_in_V_V_dout,
			osif_in_V_V_empty_n => osif_in_V_V_empty_n,
			osif_in_V_V_read    => osif_in_V_V_read,

			osif_out_V_V_din     => osif_out_V_V_din,
			osif_out_V_V_full_n  => osif_out_V_V_full_n,
			osif_out_V_V_write   => osif_out_V_V_write
	);	
end architecture;
