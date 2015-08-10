<<reconos_preproc>>

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity rt_<<NAME>> is
	port (
		-- OSIF FIFO ports
		OSIF_Sw2Hw_Data    : in  std_logic_vector(31 downto 0);
		OSIF_Sw2Hw_Empty   : in  std_logic;
		OSIF_Sw2Hw_RE      : out std_logic;

		OSIF_Hw2Sw_Data    : out std_logic_vector(31 downto 0);
		OSIF_Hw2Sw_Full    : in  std_logic;
		OSIF_Hw2Sw_WE      : out std_logic;

		-- MEMIF FIFO ports
		MEMIF_Hwt2Mem_Data    : out std_logic_vector(31 downto 0);
		MEMIF_Hwt2Mem_Full    : in  std_logic;
		MEMIF_Hwt2Mem_WE      : out std_logic;

		MEMIF_Mem2Hwt_Data    : in  std_logic_vector(31 downto 0);
		MEMIF_Mem2Hwt_Empty   : in  std_logic;
		MEMIF_Mem2Hwt_RE      : out std_logic;

		HWT_Clk    : in  std_logic;
		HWT_Rst    : in  std_logic;
		HWT_Signal : in  std_logic;

		DEBUG : out std_logic_vector(135 downto 0)
	);
end entity rt_<<NAME>>;

architecture implementation of rt_<<NAME>> is
	component rt_imp is
		port (
			ap_clk : in std_logic;
			ap_rst : in std_logic;

			<<generate for MEM>>
			memif_hwt2mem_V_V_din     : out std_logic_vector (31 downto 0);
			memif_hwt2mem_V_V_full_n  : in std_logic;
			memif_hwt2mem_V_V_write   : out std_logic;

			memif_mem2hwt_V_V_dout    : in std_logic_vector (31 downto 0);
			memif_mem2hwt_V_V_empty_n : in std_logic;
			memif_mem2hwt_V_V_read    : out std_logic;
			<<end generate>>

			osif_sw2hw_V_V_dout    : in std_logic_vector (31 downto 0);
			osif_sw2hw_V_V_empty_n : in std_logic;
			osif_sw2hw_V_V_read    : out std_logic;

			osif_hw2sw_V_V_din     : out std_logic_vector (31 downto 0);
			osif_hw2sw_V_V_full_n  : in std_logic;
			osif_hw2sw_V_V_write   : out std_logic;

			sig_V_dout    : in std_logic;
			sig_V_empty_n : in std_logic;
			sig_V_read    : out std_logic
		);
	end component;

	signal osif_sw2hw_v_v_dout    : std_logic_vector(31 downto 0);
	signal osif_sw2hw_v_v_empty_n : std_logic;
	signal osif_sw2hw_v_v_read    : std_logic;

	signal osif_hw2sw_v_v_din     : std_logic_vector(31 downto 0);
	signal osif_hw2sw_v_v_full_n  : std_logic;
	signal osif_hw2sw_v_v_write   : std_logic;

	<<generate for MEM>>
	signal memif_hwt2mem_v_v_din     : std_logic_vector(31 downto 0);
	signal memif_hwt2mem_v_v_full_n  : std_logic;
	signal memif_hwt2mem_v_v_write   : std_logic;

	signal memif_mem2hwt_v_v_dout    : std_logic_vector(31 downto 0);
	signal memif_mem2hwt_v_v_empty_n : std_logic;
	signal memif_mem2hwt_v_v_read    : std_logic;
	<<end generate>>
begin
	osif_sw2hw_v_v_dout    <= OSIF_Sw2Hw_Data;
	osif_sw2hw_v_v_empty_n <= not OSIF_Sw2Hw_Empty;
	OSIF_Sw2Hw_RE          <= osif_sw2hw_v_v_read;

	OSIF_Hw2Sw_Data        <= osif_hw2sw_v_v_din;
	osif_hw2sw_v_v_full_n  <= not OSIF_Hw2Sw_Full;
	OSIF_Hw2Sw_WE          <= osif_hw2sw_v_v_write;

	<<generate for MEM>>
	MEMIF_Hwt2Mem_Data        <= memif_hwt2mem_v_v_din;
	memif_hwt2mem_v_v_full_n  <= not MEMIF_Hwt2Mem_Full;
	MEMIF_Hwt2Mem_WE          <= memif_hwt2mem_v_v_write;

	memif_mem2hwt_v_v_dout    <= MEMIF_Mem2Hwt_Data;
	memif_mem2hwt_v_v_empty_n <= not MEMIF_Mem2Hwt_Empty;
	MEMIF_Mem2Hwt_RE          <= memif_mem2hwt_v_v_read;
	<<end generate>>
	<<generate for MEM_N>>
	MEMIF_Hwt2Mem_Data      <= (others => '0');
	MEMIF_Hwt2Mem_WE        <= '0';

	MEMIF_Mem2Hwt_RE        <= '0';
	<<end generate>>

	DEBUG(135 downto 104) <= osif_sw2hw_v_v_dout;
	DEBUG(103) <= not osif_sw2hw_v_v_empty_n;
	DEBUG(102) <= osif_sw2hw_v_v_read;
	DEBUG(101 downto 70) <= osif_hw2sw_v_v_din;
	DEBUG(69) <= not osif_hw2sw_v_v_full_n;
	DEBUG(68) <= osif_hw2sw_v_v_write;
	<<generate for MEM>>
	DEBUG(67 downto 36) <= memif_hwt2mem_v_v_din;
	DEBUG(35) <= not memif_hwt2mem_v_v_full_n;
	DEBUG(34) <= memif_hwt2mem_v_v_write;
	DEBUG(33 downto 2) <= memif_mem2hwt_v_v_dout;
	DEBUG(1) <= not memif_mem2hwt_v_v_empty_n;
	DEBUG(0) <= memif_mem2hwt_v_v_read;
	<<end generate>>

	rt_imp_inst : rt_imp
		port map (
			ap_clk => HWT_Clk,
			ap_rst => HWT_Rst,

			<<generate for MEM>>
			memif_hwt2mem_V_V_din     => memif_hwt2mem_v_V_din,
			memif_hwt2mem_V_V_full_n  => memif_hwt2mem_v_V_full_n,
			memif_hwt2mem_V_V_write   => memif_hwt2mem_v_V_write,

			memif_mem2hwt_V_V_dout    => memif_mem2hwt_v_V_dout,
			memif_mem2hwt_V_V_empty_n => memif_mem2hwt_v_V_empty_n,
			memif_mem2hwt_V_V_read    => memif_mem2hwt_v_V_read,
			<<end generate>>

			osif_sw2hw_V_V_dout    => osif_sw2hw_v_v_dout,
			osif_sw2hw_V_V_empty_n => osif_sw2hw_v_v_empty_n,
			osif_sw2hw_V_V_read    => osif_sw2hw_v_v_read,

			osif_hw2sw_V_V_din     => osif_hw2sw_v_v_din,
			osif_hw2sw_V_V_full_n  => osif_hw2sw_v_v_full_n,
			osif_hw2sw_V_V_write   => osif_hw2sw_v_v_write,

			sig_V_dout    => HWT_Signal,
			sig_V_empty_n => HWT_Signal,
			sig_V_read    => open
	);	
end architecture;
