<<reconos_preproc>>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;
use reconos_v3_01_a.reconos_defs.all;

library reconos_osif_interconnect_v1_00_a;
use reconos_osif_interconnect_v1_00_a.all;

library rt_dummy_v1_00_a;
use rt_dummy_v1_00_a.all;

<<generate for RESOURCES>>
library hwres_<<Name>><<Id>>_v1_00_a;
use hwres_<<Name>><<Id>>_v1_00_a.all;
<<end generate>>

entity testbench is
	port (
		OSIF_Sw2Ic_Data  : in  std_logic_vector(31 downto 0);
		OSIF_Sw2Ic_Full  : out std_logic;
		OSIF_Sw2Ic_WE    : in  std_logic;

		OSIF_Ic2Sw_Data  : out std_logic_vector(31 downto 0);
		OSIF_Ic2Sw_Empty : out std_logic;
		OSIF_Ic2Sw_RE    : in  std_logic;

		Clk : std_logic;
		Rst : std_logic
	);
end testbench;

architecture testbench of testbench is
	signal ic2hw_data  : std_logic_vector(31 downto 0);
	signal ic2hw_empty : std_logic;
	signal ic2hw_re    : std_logic;

	signal hw2ic_data  : std_logic_vector(31 downto 0);
	signal hw2ic_full  : std_logic;
	signal hw2ic_we    : std_logic;

	<<generate for RESOURCES>>
	signal ic2res_<<Id>>_data  : std_logic_vector(31 downto 0);
	signal ic2res_<<Id>>_empty : std_logic;
	signal ic2res_<<Id>>_re    : std_logic;

	signal res2ic_<<Id>>_data : std_logic_vector(31 downto 0);
	signal res2ic_<<Id>>_full : std_logic;
	signal res2ic_<<Id>>_we   : std_logic;
	<<end generate>>

	signal ic2sw_data  : std_logic_vector(31 downto 0);
	signal ic2sw_empty : std_logic;
	signal ic2sw_re    : std_logic;

	signal sw2ic_data  : std_logic_vector(31 downto 0);
	signal sw2ic_full  : std_logic;
	signal sw2ic_we    : std_logic;
begin
	sw2ic_data       <= OSIF_Sw2Ic_Data;
	OSIF_Sw2Ic_Full  <= sw2ic_full;
	sw2ic_we         <= OSIF_Sw2Ic_WE;

	OSIF_Ic2Sw_Data  <= ic2sw_data;
	OSIF_Ic2Sw_Empty <= ic2sw_empty;
	ic2sw_re         <= OSIF_IC2SW_RE;

	slot0 : entity rt_dummy_v1_00_a.rt_dummy
		port map (
			OSIF_Sw2Hw_Data  => ic2hw_data,
			OSIF_Sw2Hw_Empty => ic2hw_empty,
			OSIF_Sw2Hw_RE    => ic2hw_re,

			OSIF_Hw2Sw_Data  => hw2ic_data,
			OSIF_Hw2Sw_Full  => hw2ic_full,
			OSIF_Hw2Sw_WE    => hw2ic_we,

			MEMIF_Hwt2Mem_Data => open,
			MEMIF_Hwt2Mem_Full => '1',
			MEMIF_Hwt2Mem_WE   => open,

			MEMIF_Mem2Hwt_Data  => x"00000000",
			MEMIF_Mem2Hwt_Empty => '1',
			MEMIF_Mem2Hwt_RE    => open,

			HWT_Clk    => Clk,
			HWT_Rst    => Rst,
			HWT_Signal => '0'
		);

	<<generate for RESOURCES>>
	hwres<<Id>> : entity hwres_<<Name>><<Id>>_v1_00_a.hwres_<<Name>><<Id>>
		port map (
		OSIF_In_Data   => ic2res_<<Id>>_data,
		OSIF_In_Empty  => ic2res_<<Id>>_empty,
		OSIF_In_RE     => ic2res_<<Id>>_re,

		OSIF_Out_Data  => res2ic_<<Id>>_data,
		OSIF_Out_Full  => res2ic_<<Id>>_full,
		OSIF_Out_WE    => res2ic_<<Id>>_we,

		SYS_Clk => Clk,
		SYS_Rst => Rst
		);
	<<end generate>>

	ic : entity reconos_osif_interconnect_v1_00_a.reconos_osif_interconnect
		port map (
			OSIF_Hw2Ic_0_Data  => hw2ic_data,
			OSIF_Hw2Ic_0_Full  => hw2ic_full,
			OSIF_Hw2Ic_0_WE    => hw2ic_we,

			OSIF_Ic2Hw_0_Data  => ic2hw_data,
			OSIF_Ic2Hw_0_Empty => ic2hw_empty,
			OSIF_Ic2Hw_0_RE    => ic2hw_re,

			<<generate for RESOURCES>>
			OSIF_Res2Ic_<<Id>>_Data  => res2ic_<<Id>>_data,
			OSIF_Res2Ic_<<Id>>_Full  => res2ic_<<Id>>_full,
			OSIF_Res2Ic_<<Id>>_WE    => res2ic_<<Id>>_we,

			OSIF_Ic2Res_<<Id>>_Data  => ic2res_<<Id>>_data,
			OSIF_Ic2Res_<<Id>>_Empty => ic2res_<<Id>>_empty,
			OSIF_Ic2Res_<<Id>>_RE    => ic2res_<<Id>>_re,
			<<end generate>>

			OSIF_Sw2Ic_Data     => sw2ic_data,
			OSIF_Sw2Ic_Full     => sw2ic_full,
			OSIF_Sw2Ic_WE       => sw2ic_we,

			OSIF_Ic2Sw_Data     => ic2sw_data,
			OSIF_Ic2Sw_Empty    => ic2sw_empty,
			OSIF_Ic2Sw_RE       => ic2sw_re,

			SYS_Clk => Clk,
			SYS_Rst => Rst
		);
end architecture testbench;

