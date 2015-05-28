--                                                        ____  _____
--                            ________  _________  ____  / __ \/ ___/
--                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
--                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
--                         /_/   \___/\___/\____/_/ /_/\____//____/
-- 
-- ======================================================================
--
--   title:        IP-Core - OSIF Interconnect
--
--   project:      ReconOS
--   author:       Christoph Rüthing, University of Paderborn
--   description:  The interconnect allows to implement a direct fifo
--                 connection to interface with resources implemented
--                 in hardware. It routes the request based on the
--                 implementation mode of resources to either the
--                 processor interface or the according hardware
--                 implementation.
--
-- ======================================================================



library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;

entity reconos_osif_interconnect is
	--
	-- Generic definitions
	--
	--   C_NUM_HWTS - number of hardware threads
	--
	--   C_OSIF_DATA_WIDTH - width of the memif
	--
	generic (
		C_NUM_HWTS : integer := 1;

		C_OSIF_DATA_WIDTH : integer := 32
	);

	--
	-- Port defintions
	--
	--   OSIF_Hw2Dt_<<i>>_In_/OSIF_Dt2Hw_<<i>>_In_   - fifo signal inputs
	--   OSIF_Hw2Dt_<<i>>_Out_/OSIF_Dt2Hw_<<i>>_Out_ - fifo signal outputs
	--
	--   OSIF_Sw2Dt_In_/OSIF_Dt2Sw_In_ - fifo signal inputs for software
	--
	--   OSIF_Ic2Res_Out_/OSIF_Res2Ic_In_ - fifo signal inputs for resources
	--   
	--   SYS_Clk - system clock
	--   SYS_Rst - system reset
	--
	port (
				OSIF_Hw2Dt_0_In_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Hw2Dt_0_In_Empty : in  std_logic;
		OSIF_Hw2Dt_0_In_RE    : out std_logic;
		
		OSIF_Hw2Dt_1_In_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Hw2Dt_1_In_Empty : in  std_logic;
		OSIF_Hw2Dt_1_In_RE    : out std_logic;
		
		OSIF_Hw2Dt_2_In_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Hw2Dt_2_In_Empty : in  std_logic;
		OSIF_Hw2Dt_2_In_RE    : out std_logic;
		


				OSIF_Dt2Hw_0_In_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Dt2Hw_0_In_Empty : in  std_logic;
		OSIF_Dt2Hw_0_In_RE    : out std_logic;
		
		OSIF_Dt2Hw_1_In_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Dt2Hw_1_In_Empty : in  std_logic;
		OSIF_Dt2Hw_1_In_RE    : out std_logic;
		
		OSIF_Dt2Hw_2_In_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Dt2Hw_2_In_Empty : in  std_logic;
		OSIF_Dt2Hw_2_In_RE    : out std_logic;
		


				OSIF_Hw2Dt_0_Out_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Hw2Dt_0_Out_Empty : out std_logic;
		OSIF_Hw2Dt_0_Out_RE    : in  std_logic;
		
		OSIF_Hw2Dt_1_Out_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Hw2Dt_1_Out_Empty : out std_logic;
		OSIF_Hw2Dt_1_Out_RE    : in  std_logic;
		
		OSIF_Hw2Dt_2_Out_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Hw2Dt_2_Out_Empty : out std_logic;
		OSIF_Hw2Dt_2_Out_RE    : in  std_logic;
		


				OSIF_Dt2Hw_0_Out_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Dt2Hw_0_Out_Empty : out std_logic;
		OSIF_Dt2Hw_0_Out_RE    : in  std_logic;
		
		OSIF_Dt2Hw_1_Out_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Dt2Hw_1_Out_Empty : out std_logic;
		OSIF_Dt2Hw_1_Out_RE    : in  std_logic;
		
		OSIF_Dt2Hw_2_Out_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Dt2Hw_2_Out_Empty : out std_logic;
		OSIF_Dt2Hw_2_Out_RE    : in  std_logic;
		


		OSIF_Sw2Dt_In_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Sw2Dt_In_Empty : in  std_logic;
		OSIF_Sw2Dt_In_RE    : out std_logic;

		OSIF_Dt2Sw_In_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Dt2Sw_In_Full  : in  std_logic;
		OSIF_Dt2Sw_In_WE    : out std_logic;

				OSIF_Ic2Res_1_Out_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Ic2Res_1_Out_Empty : out std_logic;
		OSIF_Ic2Res_1_Out_RE    : in  std_logic;
		
		OSIF_Ic2Res_2_Out_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Ic2Res_2_Out_Empty : out std_logic;
		OSIF_Ic2Res_2_Out_RE    : in  std_logic;
		


				OSIF_Res2Ic_1_In_Data   : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Res2Ic_1_In_Empty  : in  std_logic;
		OSIF_Res2Ic_1_In_RE     : out std_logic;
		
		OSIF_Res2Ic_2_In_Data   : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Res2Ic_2_In_Empty  : in  std_logic;
		OSIF_Res2Ic_2_In_RE     : out std_logic;
		


		SYS_Clk : in std_logic;
		SYS_Rst : in std_logic
	);
end entity reconos_osif_interconnect;


architecture imp of reconos_osif_interconnect is
	--
	-- Internal state machine
	--
	--   state_type - vhdl type of the states
	--   state      - instantiation of the state
	--
	type state_type is (STATE_WAIT,STATE_ARBITRATE,
	                    STATE_CMD,STATE_ADDR,STATE_PROCESS);
	signal state : state_type := STATE_WAIT;

	--
	-- Internal interconnect signals
	--
	--   hw<<SlotId>>2res<<ResId>>_/res<<ResId>>2hw<<SlotId>>_ - interconnect from slots to resources
	--   sw2res<<ResId>>_/res<<ResId>>2sw_                     - interconnect for software to resources
	--
			hw02res0_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	hw02res0_Empty : std_logic;
	hw02res0_RE    : std_logic;

	res02hw0_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	res02hw0_Empty : std_logic;
	res02hw0_RE    : std_logic;
	
	hw02res1_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	hw02res1_Empty : std_logic;
	hw02res1_RE    : std_logic;

	res12hw0_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	res12hw0_Empty : std_logic;
	res12hw0_RE    : std_logic;
	

	
		hw12res0_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	hw12res0_Empty : std_logic;
	hw12res0_RE    : std_logic;

	res02hw1_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	res02hw1_Empty : std_logic;
	res02hw1_RE    : std_logic;
	
	hw12res1_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	hw12res1_Empty : std_logic;
	hw12res1_RE    : std_logic;

	res12hw1_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	res12hw1_Empty : std_logic;
	res12hw1_RE    : std_logic;
	

	
		hw22res0_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	hw22res0_Empty : std_logic;
	hw22res0_RE    : std_logic;

	res02hw2_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	res02hw2_Empty : std_logic;
	res02hw2_RE    : std_logic;
	
	hw22res1_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	hw22res1_Empty : std_logic;
	hw22res1_RE    : std_logic;

	res12hw2_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	res12hw2_Empty : std_logic;
	res12hw2_RE    : std_logic;
	

	


		sw2res1_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	sw2res1_Empty : std_logic;
	sw2res1_RE    : std_logic;

	res12sw_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	res12sw_Empty : std_logic;
	res12sw_RE    : std_logic;
	
	sw2res2_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	sw2res2_Empty : std_logic;
	sw2res2_RE    : std_logic;

	res22sw_Data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	res22sw_Empty : std_logic;
	res22sw_RE    : std_logic;
	

begin
	-- == Interconnect instantiation ======================================

		icin_slot0 : entity reconos_osif_interconnect.router_icin_slot0
		generic map (
			C_OSIF_DATA_WIDTH => C_OSIF_DATA_WIDTH
		)
		port map (
			OSIF_In_Data  => OSIF_Hw2Dt_0_In_Data,
			OSIF_In_Empty => OSIF_Hw2Dt_0_In_Empty,
			OSIF_In_RE    => OSIF_Hw2Dt_0_In_RE,

						OSIF_Out_0_Data  => hw02res0_Data,
			OSIF_Out_0_Empty => hw02res0_Empty,
			OSIF_Out_0_RE    => hw02res0_RE,
			
			OSIF_Out_1_Data  => hw02res1_Data,
			OSIF_Out_1_Empty => hw02res1_Empty,
			OSIF_Out_1_RE    => hw02res1_RE,
			

			OSIF_Out_2_Data  => OSIF_Hw2Dt_0_Out_Data,
			OSIF_Out_2_Empty => OSIF_Hw2Dt_0_Out_Empty,
			OSIF_Out_2_RE    => OSIF_Hw2Dt_0_Out_RE,

			SYS_Clk => SYS_Clk,
			SYS_Rst => SYS_Rst
		);

	icout_slot0 : entity reconos_osif_interconnect.arbiter_icout_slot0
		generic map (
			C_OSIF_DATA_WIDTH => C_OSIF_DATA_WIDTH
		)
		port map (
			OSIF_Out_Data  => OSIF_Dt2Hw_0_Out_Data,
			OSIF_Out_Empty => OSIF_Dt2Hw_0_Out_Empty,
			OSIF_Out_RE    => OSIF_Dt2Hw_0_Out_RE,

						OSIF_In_0_Data  => res02hw0_Data,
			OSIF_In_0_Empty => res02hw0_Empty,
			OSIF_In_0_RE    => res02hw0_RE,
			
			OSIF_In_1_Data  => res12hw0_Data,
			OSIF_In_1_Empty => res12hw0_Empty,
			OSIF_In_1_RE    => res12hw0_RE,
			

			OSIF_In_2_Data  => OSIF_Dt2Hw_0_In_Data,
			OSIF_In_2_Empty => OSIF_Dt2Hw_0_In_Empty,
			OSIF_In_2_RE    => OSIF_Dt2Hw_0_In_RE,

			SYS_Clk => SYS_Clk,
			SYS_Rst => SYS_Rst
		);
	
	icin_slot1 : entity reconos_osif_interconnect.router_icin_slot1
		generic map (
			C_OSIF_DATA_WIDTH => C_OSIF_DATA_WIDTH
		)
		port map (
			OSIF_In_Data  => OSIF_Hw2Dt_1_In_Data,
			OSIF_In_Empty => OSIF_Hw2Dt_1_In_Empty,
			OSIF_In_RE    => OSIF_Hw2Dt_1_In_RE,

						OSIF_Out_0_Data  => hw12res0_Data,
			OSIF_Out_0_Empty => hw12res0_Empty,
			OSIF_Out_0_RE    => hw12res0_RE,
			
			OSIF_Out_1_Data  => hw12res1_Data,
			OSIF_Out_1_Empty => hw12res1_Empty,
			OSIF_Out_1_RE    => hw12res1_RE,
			

			OSIF_Out_2_Data  => OSIF_Hw2Dt_1_Out_Data,
			OSIF_Out_2_Empty => OSIF_Hw2Dt_1_Out_Empty,
			OSIF_Out_2_RE    => OSIF_Hw2Dt_1_Out_RE,

			SYS_Clk => SYS_Clk,
			SYS_Rst => SYS_Rst
		);

	icout_slot1 : entity reconos_osif_interconnect.arbiter_icout_slot1
		generic map (
			C_OSIF_DATA_WIDTH => C_OSIF_DATA_WIDTH
		)
		port map (
			OSIF_Out_Data  => OSIF_Dt2Hw_1_Out_Data,
			OSIF_Out_Empty => OSIF_Dt2Hw_1_Out_Empty,
			OSIF_Out_RE    => OSIF_Dt2Hw_1_Out_RE,

						OSIF_In_0_Data  => res02hw1_Data,
			OSIF_In_0_Empty => res02hw1_Empty,
			OSIF_In_0_RE    => res02hw1_RE,
			
			OSIF_In_1_Data  => res12hw1_Data,
			OSIF_In_1_Empty => res12hw1_Empty,
			OSIF_In_1_RE    => res12hw1_RE,
			

			OSIF_In_2_Data  => OSIF_Dt2Hw_1_In_Data,
			OSIF_In_2_Empty => OSIF_Dt2Hw_1_In_Empty,
			OSIF_In_2_RE    => OSIF_Dt2Hw_1_In_RE,

			SYS_Clk => SYS_Clk,
			SYS_Rst => SYS_Rst
		);
	
	icin_slot2 : entity reconos_osif_interconnect.router_icin_slot2
		generic map (
			C_OSIF_DATA_WIDTH => C_OSIF_DATA_WIDTH
		)
		port map (
			OSIF_In_Data  => OSIF_Hw2Dt_2_In_Data,
			OSIF_In_Empty => OSIF_Hw2Dt_2_In_Empty,
			OSIF_In_RE    => OSIF_Hw2Dt_2_In_RE,

						OSIF_Out_0_Data  => hw22res0_Data,
			OSIF_Out_0_Empty => hw22res0_Empty,
			OSIF_Out_0_RE    => hw22res0_RE,
			
			OSIF_Out_1_Data  => hw22res1_Data,
			OSIF_Out_1_Empty => hw22res1_Empty,
			OSIF_Out_1_RE    => hw22res1_RE,
			

			OSIF_Out_2_Data  => OSIF_Hw2Dt_2_Out_Data,
			OSIF_Out_2_Empty => OSIF_Hw2Dt_2_Out_Empty,
			OSIF_Out_2_RE    => OSIF_Hw2Dt_2_Out_RE,

			SYS_Clk => SYS_Clk,
			SYS_Rst => SYS_Rst
		);

	icout_slot2 : entity reconos_osif_interconnect.arbiter_icout_slot2
		generic map (
			C_OSIF_DATA_WIDTH => C_OSIF_DATA_WIDTH
		)
		port map (
			OSIF_Out_Data  => OSIF_Dt2Hw_2_Out_Data,
			OSIF_Out_Empty => OSIF_Dt2Hw_2_Out_Empty,
			OSIF_Out_RE    => OSIF_Dt2Hw_2_Out_RE,

						OSIF_In_0_Data  => res02hw2_Data,
			OSIF_In_0_Empty => res02hw2_Empty,
			OSIF_In_0_RE    => res02hw2_RE,
			
			OSIF_In_1_Data  => res12hw2_Data,
			OSIF_In_1_Empty => res12hw2_Empty,
			OSIF_In_1_RE    => res12hw2_RE,
			

			OSIF_In_2_Data  => OSIF_Dt2Hw_2_In_Data,
			OSIF_In_2_Empty => OSIF_Dt2Hw_2_In_Empty,
			OSIF_In_2_RE    => OSIF_Dt2Hw_2_In_RE,

			SYS_Clk => SYS_Clk,
			SYS_Rst => SYS_Rst
		);
	


		icin_res1 : entity reconos_osif_interconnect.router_icin_res1
		generic map (
			C_OSIF_DATA_WIDTH => C_OSIF_DATA_WIDTH
		)
		port map (
			OSIF_In_Data  => OSIF_Res2Ic_1_In_Data,
			OSIF_In_Empty => OSIF_Res2Ic_1_In_Empty,
			OSIF_In_RE    => OSIF_Res2Ic_1_In_RE,

						OSIF_Out_0_Data  => res02hw0_Data,
			OSIF_Out_0_Empty => res02hw0_Empty,
			OSIF_Out_0_RE    => res02hw0_RE,
			
			OSIF_Out_1_Data  => res02hw1_Data,
			OSIF_Out_1_Empty => res02hw1_Empty,
			OSIF_Out_1_RE    => res02hw1_RE,
			
			OSIF_Out_2_Data  => res02hw2_Data,
			OSIF_Out_2_Empty => res02hw2_Empty,
			OSIF_Out_2_RE    => res02hw2_RE,
			

			OSIF_Out_3_Data  => OSIF_Dt2Sw_In_Data,
			OSIF_Out_3_Empty => OSIF_Dt2Sw_In_Empty,
			OSIF_Out_3_RE    => OSIF_Dt2Sw_In_RE,

			SYS_Clk => SYS_Clk,
			SYS_Rst => SYS_Rst
		);

	icout_res1 : entity reconos_osif_interconnect.arbiter_icout_res1
		generic map (
			C_OSIF_DATA_WIDTH => C_OSIF_DATA_WIDTH
		)
		port map (
			OSIF_Out_Data  => OSIF_Ic2Res_1_Out_Data;
			OSIF_Out_Empty => OSIF_Ic2Res_1_Out_Empty;
			OSIF_Out_RE    => OSIF_Ic2Res_1_Out_RE;

						OSIF_In_0_Data  => hw02res0_Data,
			OSIF_In_0_Empty => hw02res0_Empty,
			OSIF_In_0_RE    => hw02res0_RE,
			
			OSIF_In_1_Data  => hw12res0_Data,
			OSIF_In_1_Empty => hw12res0_Empty,
			OSIF_In_1_RE    => hw12res0_RE,
			
			OSIF_In_2_Data  => hw22res0_Data,
			OSIF_In_2_Empty => hw22res0_Empty,
			OSIF_In_2_RE    => hw22res0_RE,
			

			OSIF_In_3_Data  => OSIF_Sw2Dt_In_Data,
			OSIF_In_3_Empty => OSIF_Sw2Dt_In_Empty,
			OSIF_In_3_RE    => OSIF_Sw2Dt_In_RE,

			SYS_Clk => SYS_Clk,
			SYS_Rst => SYS_Rst
		);
	
	icin_res2 : entity reconos_osif_interconnect.router_icin_res2
		generic map (
			C_OSIF_DATA_WIDTH => C_OSIF_DATA_WIDTH
		)
		port map (
			OSIF_In_Data  => OSIF_Res2Ic_2_In_Data,
			OSIF_In_Empty => OSIF_Res2Ic_2_In_Empty,
			OSIF_In_RE    => OSIF_Res2Ic_2_In_RE,

						OSIF_Out_0_Data  => res12hw0_Data,
			OSIF_Out_0_Empty => res12hw0_Empty,
			OSIF_Out_0_RE    => res12hw0_RE,
			
			OSIF_Out_1_Data  => res12hw1_Data,
			OSIF_Out_1_Empty => res12hw1_Empty,
			OSIF_Out_1_RE    => res12hw1_RE,
			
			OSIF_Out_2_Data  => res12hw2_Data,
			OSIF_Out_2_Empty => res12hw2_Empty,
			OSIF_Out_2_RE    => res12hw2_RE,
			

			OSIF_Out_3_Data  => OSIF_Dt2Sw_In_Data,
			OSIF_Out_3_Empty => OSIF_Dt2Sw_In_Empty,
			OSIF_Out_3_RE    => OSIF_Dt2Sw_In_RE,

			SYS_Clk => SYS_Clk,
			SYS_Rst => SYS_Rst
		);

	icout_res2 : entity reconos_osif_interconnect.arbiter_icout_res2
		generic map (
			C_OSIF_DATA_WIDTH => C_OSIF_DATA_WIDTH
		)
		port map (
			OSIF_Out_Data  => OSIF_Ic2Res_2_Out_Data;
			OSIF_Out_Empty => OSIF_Ic2Res_2_Out_Empty;
			OSIF_Out_RE    => OSIF_Ic2Res_2_Out_RE;

						OSIF_In_0_Data  => hw02res1_Data,
			OSIF_In_0_Empty => hw02res1_Empty,
			OSIF_In_0_RE    => hw02res1_RE,
			
			OSIF_In_1_Data  => hw12res1_Data,
			OSIF_In_1_Empty => hw12res1_Empty,
			OSIF_In_1_RE    => hw12res1_RE,
			
			OSIF_In_2_Data  => hw22res1_Data,
			OSIF_In_2_Empty => hw22res1_Empty,
			OSIF_In_2_RE    => hw22res1_RE,
			

			OSIF_In_3_Data  => OSIF_Sw2Dt_In_Data,
			OSIF_In_3_Empty => OSIF_Sw2Dt_In_Empty,
			OSIF_In_3_RE    => OSIF_Sw2Dt_In_RE,

			SYS_Clk => SYS_Clk,
			SYS_Rst => SYS_Rst
		);
	


end architecture imp;
