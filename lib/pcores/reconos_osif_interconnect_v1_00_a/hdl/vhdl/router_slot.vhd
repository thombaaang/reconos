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

<<reconos_preproc>>

<<generate for SLOTS>>
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_defs.all;

entity router_icin_slot<<Id>> is
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
	--   FIFO_Res<<ResId>>_ - fifo signal outputs for resources
	--   FIFO_Sw_           - fifo signal outputs for software
	--   
	--   SYS_Clk - system clock
	--   SYS_Rst - system reset
	--
	port (
		FIFO_In_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		FIFO_In_Empty : in  std_logic;
		FIFO_In_RE    : out std_logic;

		<<=generate for IcRes=>>
		FIFO_Res<<ResId>>_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		FIFO_Res<<ResId>>_Empty : out std_logic;
		FIFO_Res<<ResId>>_RE    : in  std_logic;
		<<=end generate=>>
		FIFO_Sw_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		FIFO_Sw_Empty : out std_logic;
		FIFO_Sw_RE    : in  std_logic;

		SYS_Clk : in std_logic;
		SYS_Rst : in std_logic
	);
end entity router_icin_slot<<Id>>;


architecture imp of router_icin_slot<<Id>> is
	--
	-- Internal state machine
	--
	--   state_type - vhdl type of the states
	--   state      - instantiation of the state
	--
	type state_type is (STATE_READ_CTRL,STATE_ROUTE_0,STATE_ROUTE_1,
	                    STATE_WRITE_CTRL,STATE_PROCESS);
	signal state : state_type := STATE_READ_CTRL;

	--
	-- Internal signals for round robin arbiter
	--
	--   grnt_ - grant vector to multiplex signals
	--
	signal grnt_res : std_logic_vector(<<NumIcRes>> - 1 downto 0) := (others => '0');
	signal grnt_sw  : std_logic := '0';

	--
	-- Internal signals
	--
	--   ctrl       - control word read
	--
	--   word_count - counter of transferred word
	--
	signal word_count : unsigned(C_OSIF_LENGTH_WIDTH - 1 downto 0) := (others => '0');

	signal ctrl : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := (others => '0');

	--
	-- Signals used for usage of multiplexed signals
	--
	--   out_/in_ - multiplexed fifo signals
	--
	signal in_re     : std_logic;
	signal out_data  : std_logic_vector(31 downto 0);
	signal out_empty : std_logic;
begin

	-- == Process definitions =============================================

	--
	-- Arbitrate fifos based on requests and snoop
	--
	--   A state machine to implement a round robin arbiter. The arbiter
	--   snoops on the fifos to figure out the end of a transaction.
	--
	route : process(SYS_Clk,SYS_Rst) is
	begin
		if SYS_Rst = '1' then
			grnt_res <= (others => '0');
			grnt_sw  <= '0';

			state <= STATE_READ_CTRL;
		elsif rising_edge(SYS_Clk) then
			case state is
				when STATE_READ_CTRL =>
					if FIFO_In_Empty = '0' then
						ctrl <= FIFO_In_Data;
						word_count <= unsigned(FIFO_In_Data(C_OSIF_LENGTH_RANGE));

						state <= STATE_ROUTE_0;
					end if;

				when STATE_ROUTE_0 =>
					<<=generate for IcRes=>>
					if ctrl(C_OSIF_DST_RANGE) = <<ResId|x"{:02x}">> then
						grnt_res(<<_i>>) <= '1';
					end if;
					<<=end generate=>>

					state <= STATE_ROUTE_1;

				when STATE_ROUTE_1 =>
					<<=generate for HasIcRes=>>
					if grnt_res = (grnt_res'Range => '0') then
					<<=end generate=>>
						grnt_sw <= '1';
					<<=generate for HasIcRes=>>
					end if;
					<<=end generate=>>

					state <= STATE_WRITE_CTRL;

				when STATE_WRITE_CTRL =>
					if in_re = '1' then
						state <= STATE_PROCESS;
					end if;

				when STATE_PROCESS =>
					if in_re = '1' and FIFO_In_Empty = '0' then
						word_count <= word_count - 1;

						if word_count - 1 = 0 then
							state <= STATE_READ_CTRL;

							grnt_res <= (others => '0');
							grnt_sw  <= '0';
						end if;
					end if;

				when others =>
			end case;
		end if;
	end process route;


	-- == Multiplexing signals ============================================

	in_re <=
	  <<=generate for IcRes=>>
	  ((FIFO_Res<<ResId>>_RE) and grnt_res(<<_i>>)) or
	  <<=end generate=>>
	  ((FIFO_Sw_RE) and grnt_sw);

	out_data  <= FIFO_In_Data when state = STATE_PROCESS else
	             ctrl         when state = STATE_WRITE_CTRL else
	             x"00000000";

	out_empty <= FIFO_In_Empty when state = STATE_PROCESS else
	             '0'           when state = STATE_WRITE_CTRL else
	             '1';

	<<=generate for IcRes=>>
	FIFO_Res<<ResId>>_Data <= out_data;
	<<=end generate=>>
	FIFO_Sw_Data <= out_data;

	<<=generate for IcRes=>>
	FIFO_Res<<ResId>>_Empty <= out_empty when grnt_res(<<_i>>) = '1' else '1';
	<<=end generate=>>
	FIFO_Sw_Empty <= out_empty when grnt_sw = '1' else '1';

	FIFO_In_RE <= in_re when state = STATE_PROCESS else
	              '1'   when state = STATE_READ_CTRL else
	              '0';

end architecture imp;
<<end generate>>