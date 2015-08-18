--                                                        ____  _____
--                            ________  _________  ____  / __ \/ ___/
--                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
--                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
--                         /_/   \___/\___/\____/_/ /_/\____//____/
-- 
-- ======================================================================
--
--   title:        IP-Core - OSIF Arbiter
--
--   project:      ReconOS
--   author:       Christoph Rüthing, University of Paderborn
--   description:  Internally used router.
--
-- ======================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_defs.all;

entity blocking is
	--
	-- Generic definitions
	--
	--   C_OSIF_DATA_WIDTH - width of the osif
	--
	generic (
		C_OSIF_DATA_WIDTH : integer := 32
	);

	--
	-- Port defintions
	--
	--   FIFO_In_ - fifo signal inputs
	--
	--   FIFO_Out_ - fifo signal output
	--
	--   BL_Sig - blocking signal
	--   BL_Rdy - ready signal
	--   
	--   SYS_Clk - system clock
	--   SYS_Rst - system reset
	--
	port (
		FIFO_In_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		FIFO_In_Full  : out std_logic;
		FIFO_In_WE    : in  std_logic;

		FIFO_Out_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		FIFO_Out_Full  : in  std_logic;
		FIFO_Out_WE    : out std_logic;

		BL_Sig : in  std_logic;
		BL_Rdy : out std_logic;

		SYS_Clk : in std_logic;
		SYS_Rst : in std_logic
	);
end entity blocking;


architecture imp of blocking is
	--
	-- Internal state machine
	--
	--   state_type - vhdl type of the states
	--   state      - instantiation of the state
	--
	type state_type is (STATE_CTRL,STATE_PROCESS);
	signal state : state_type := STATE_CTRL;

	--
	-- Internal signals
	--
	--   word_count - counter of transferred word
	--
	signal word_count : unsigned(C_OSIF_LENGTH_WIDTH - 1 downto 0) := (others => '0');

begin

	-- == Process definitions =============================================

	--
	-- Snoops FIFO and makes the full when signalled.
	--
	--   Used to stop incoming data to suspend the interconnect. It snoops
	--   the FIFOs to not interrupt transactions.
	--
	route : process(SYS_Clk,SYS_Rst) is
	begin
		if SYS_Rst = '1' then
			state <= STATE_CTRL;
		elsif rising_edge(SYS_Clk) then
			case state is
				when STATE_CTRL =>
					if FIFO_In_WE = '1' and FIFO_Out_Full = '0' then
						word_count <= unsigned(FIFO_In_Data(C_OSIF_LENGTH_RANGE));
						state <= STATE_PROCESS;
					end if;

				when STATE_PROCESS =>
					if FIFO_In_WE = '1' and FIFO_Out_Full = '0' then
						word_count <= word_count - 1;

						if word_count - 1 = 0 then
							state <= STATE_CTRL;
						end if;
					end if;

				when others =>
			end case;
		end if;
	end process route;


	-- == Multiplexing signals ============================================

	FIFO_Out_Data <= FIFO_In_Data;
	FIFO_In_Full  <= '1' when BL_Sig = '1' and state = STATE_CTRL else FIFO_Out_Full;
	FIFO_Out_WE   <= FIFO_In_WE;

	BL_Rdy <= '1' when state = STATE_CTRL else '0';

end architecture imp;
