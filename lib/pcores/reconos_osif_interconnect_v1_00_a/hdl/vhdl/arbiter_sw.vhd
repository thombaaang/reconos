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
--   description:  Internally used arbiter.
--
-- ======================================================================

<<reconos_preproc>>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_defs.all;

entity arbiter_icout_sw is
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
	--   FIFO_Out_ - fifo signal outputs
	--
	--   FIFO_Res<<ResId>>_ - fifo signal inputs from resources
	--   FIFO_Hw<<SlotId>>_ - fifo signal inputs from slots
	--   
	--   SYS_Clk - system clock
	--   SYS_Rst - system reset
	--
	port (
		FIFO_Out_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		FIFO_Out_Empty : out std_logic;
		FIFO_Out_RE    : in  std_logic;

		<<generate for RESOURCES(Mode == "hw")>>
		FIFO_Res<<ResId>>_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		FIFO_Res<<ResId>>_Empty : in  std_logic;
		FIFO_Res<<ResId>>_RE    : out std_logic;
		<<end generate>>
		<<generate for SLOTS>>
		FIFO_Hw<<SlotId>>_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		FIFO_Hw<<SlotId>>_Empty : in  std_logic;
		FIFO_Hw<<SlotId>>_RE    : out std_logic;
		<<end generate>>

		SYS_Clk : in std_logic;
		SYS_Rst : in std_logic
	);
end entity arbiter_icout_sw;


architecture imp of arbiter_icout_sw is
	--
	-- Internal state machine
	--
	--   state_type - vhdl type of the states
	--   state      - instantiation of the state
	--
	type state_type is (STATE_WAIT,STATE_ARBITRATE,
	                    STATE_CTRL,STATE_PROCESS);
	signal state : state_type := STATE_WAIT;

	--
	-- Internal signals for round robin arbiter
	--
	--   req  - masked request vector
	--   msk  - mask to disable previous grants
	--   msb  - most significant bit of req
	--   grnt - grant vector to multiplex signals
	--   orr  - override for full and empty signals
	--
	--   req_  - masked request vector
	--   msk_  - mask to disable previous grants
	--   grnt_ - grant vector to multiplex signals
	--
	signal req  : std_logic_vector(<<NUM_HWRESOURCES>> + <<NUM_SLOTS>> - 1 downto 0) := (others => '0');
	signal msk  : std_logic_vector(<<NUM_HWRESOURCES>> + <<NUM_SLOTS>> - 1 downto 0) := (others => '1');
	signal msb  : std_logic_vector(<<NUM_HWRESOURCES>> + <<NUM_SLOTS>> - 1 downto 0) := (others => '0');
	signal grnt : std_logic_vector(<<NUM_HWRESOURCES>> + <<NUM_SLOTS>> - 1 downto 0) := (others => '0');
	signal orr  : std_logic := '1';

	signal req_res  : std_logic_vector(<<NUM_HWRESOURCES>> - 1 downto 0) := (others => '0');
	signal req_hw   : std_logic_vector(<<NUM_SLOTS>> - 1 downto 0) := (others => '0');
	signal msk_res  : std_logic_vector(<<NUM_HWRESOURCES>> - 1 downto 0) := (others => '1');
	signal msk_hw   : std_logic_vector(<<NUM_SLOTS>> - 1 downto 0) := (others => '1');
	signal grnt_res : std_logic_vector(<<NUM_HWRESOURCES>> - 1 downto 0) := (others => '0');
	signal grnt_hw  : std_logic_vector(<<NUM_SLOTS>> - 1 downto 0) := (others => '0');

	--
	-- Internal signals
	--
	--   word_count - counter of transferred word
	--
	signal word_count : unsigned(C_OSIF_LENGTH_WIDTH - 1 downto 0) := (others => '0');

	--
	-- Signals used for usage of multiplexed signals
	--
	--   out_ - multiplexed fifo signals
	--
	signal out_data  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
	signal out_empty : std_logic;
begin

	-- == Assignment of input signals =====================================

	<<generate for RESOURCES(Mode == "hw")>>
	req_res(<<_i>>) <= not FIFO_Res<<ResId>>_Empty and msk_res(<<_i>>);
	<<end generate>>
	<<generate for SLOTS>>
	req_hw(<<_i>>) <= not FIFO_Hw<<SlotId>>_Empty and msk_hw(<<_i>>);
	<<end generate>>

	req <= req_res & req_hw;

	msk_res  <= msk(<<NUM_HWRESOURCES>> + <<NUM_SLOTS>> - 1 downto <<NUM_SLOTS>>);
	msk_hw   <= msk(<<NUM_SLOTS>> - 1 downto 0);
	grnt_res <= grnt(<<NUM_HWRESOURCES>> + <<NUM_SLOTS>> - 1 downto <<NUM_SLOTS>>);
	grnt_hw  <= grnt(<<NUM_SLOTS>> - 1 downto 0);

	msb <= req and std_logic_vector(unsigned(not(req)) + 1);


	-- == Process definitions =============================================

	--
	-- Arbitrate fifos based on requests and snoop
	--
	--   A state machine to implement a round robin arbiter. The arbiter
	--   snoops on the fifos to figure out the end of a transaction.
	--
	arb : process(SYS_Clk,SYS_Rst) is
	begin
		if SYS_Rst = '1' then
			msk <= (others => '1');
			grnt <= (others => '0');

			state <= STATE_WAIT;
		elsif rising_edge(SYS_Clk) then
			case state is
				when STATE_WAIT =>
					if req = (req'Range => '0') then
						msk <= (others => '1');
					else
						state <= STATE_ARBITRATE;
					end if;

				when STATE_ARBITRATE =>
					grnt <= msb;
					msk <= msk and (not msb);

					state <= STATE_CTRL;

				when STATE_CTRL =>
					if FIFO_Out_RE = '1' and out_empty = '0' then
						word_count <= unsigned(out_data(C_OSIF_LENGTH_RANGE));

						state <= STATE_PROCESS;
					end if;

				when STATE_PROCESS =>
					if FIFO_Out_RE = '1' and out_empty = '0' then
						word_count <= word_count - 1;

						if word_count - 1 = 0 then
							state <= STATE_WAIT;

							grnt <= (others => '0');
						end if;
					end if;

				when others =>
			end case;
		end if;
	end process arb;


	-- == Multiplexing signals ============================================

	orr <= '1' when state = STATE_WAIT else
	       '1' when state = STATE_ARBITRATE else
	       '0';

	out_data <=
	  <<generate for RESOURCES(Mode == "hw")>>
	  (FIFO_Res<<ResId>>_Data and (FIFO_Res<<ResId>>_Data'Range => grnt_res(<<_i>>))) or
	  <<end generate>>
	  <<generate for SLOTS>>
	  (FIFO_Hw<<SlotId>>_Data and (FIFO_Hw<<SlotId>>_Data'Range => grnt_hw(<<_i>>))) or
	  <<end generate>>
	  (C_OSIF_DATA_WIDTH - 1 downto 0 => '0');

	out_empty <=
	  <<generate for RESOURCES(Mode == "hw")>>
	  (FIFO_Res<<ResId>>_Empty and grnt_res(<<_i>>)) or
	  <<end generate>>
	  <<generate for SLOTS>>
	  (FIFO_Hw<<SlotId>>_Empty and grnt_hw(<<_i>>)) or
	  <<end generate>>
	  orr;

	FIFO_Out_Data  <= out_data;
	FIFO_Out_Empty <= out_empty;
	<<generate for RESOURCES(Mode == "hw")>>
	FIFO_Res<<ResId>>_RE <= FIFO_Out_RE and grnt_res(<<_i>>);
	<<end generate>>
	<<generate for SLOTS>>
	FIFO_Hw<<SlotId>>_RE <= FIFO_Out_RE and grnt_hw(<<_i>>);
	<<end generate>>

end architecture imp;