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



library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;

entity arbiter_icout_slot0 is
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
	--   OSIF_Out_ - fifo signal inputs
	--
	--   OSIF_In_0_ - fifo signal outputs
	--   
	--   SYS_Clk - system clock
	--   SYS_Rst - system reset
	--
	port (
		OSIF_Out_Data  : out  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Out_Empty : out std_logic;
		OSIF_Out_RE    : in  std_logic;

				OSIF_In_0_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_In_0_Empty : in  std_logic;
		OSIF_In_0_RE    : out std_logic;
		
		OSIF_In_1_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_In_1_Empty : in  std_logic;
		OSIF_In_1_RE    : out std_logic;
		

		OSIF_In_2_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_In_2_Empty : in  std_logic;
		OSIF_In_2_RE    : out std_logic;

		SYS_Clk : in std_logic;
		SYS_Rst : in std_logic
	);
end entity arbiter_icout_slot0;


architecture imp of arbiter_icout_slot0 is
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
	signal req  : std_logic_vector(2 downto 0) := (others => '0');
	signal msk  : std_logic_vector(2 downto 0) := (others => '1');
	signal msb  : std_logic_vector(2 downto 0) := (others => '0');
	signal grnt : std_logic_vector(2 downto 0) := (others => '0');
	signal orr  : std_logic := '1';

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

		req(0) <= not OSIF_In_0_Empty and msk(0);
		req(1) <= not OSIF_In_1_Empty and msk(1);
	
	req(2) <= not OSIF_In_2_Empty and msk(2);

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
					if OSIF_Out_RE = '1' and out_empty = '0' then
						word_count <= unsigned(out_data(C_OSIF_LENGTH_RANGE));

						state <= STATE_PROCESS;
					end if;

				when STATE_PROCESS =>
					if OSIF_Out_RE = '1' and out_empty = '0' then
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
	  	  (OSIF_In_0_Data and (OSIF_In_0_Data'Range => grnt(0))) or
	  	  (OSIF_In_1_Data and (OSIF_In_1_Data'Range => grnt(1))) or
	  
	  (OSIF_In_2_Data and (OSIF_In_2_Data'Range => grnt(2))) or
	  (C_OSIF_DATA_WIDTH - 1 downto 0 => '0');

	out_empty <=
	  	  (OSIF_In_0_Empty and grnt(0)) or
	  	  (OSIF_In_1_Empty and grnt(1)) or
	  
	  (OSIF_In_2_Empty and grnt(2)) or
	  orr;

	OSIF_Out_Data  <= out_data;
	OSIF_Out_Empty <= out_empty;
		OSIF_In_0_RE <= OSIF_Out_RE and grnt(0);
		OSIF_In_1_RE <= OSIF_Out_RE and grnt(1);
	
	OSIF_In_2_RE <= OSIF_Out_RE and grnt(2);

end architecture imp;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;

entity arbiter_icout_slot1 is
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
	--   OSIF_Out_ - fifo signal inputs
	--
	--   OSIF_In_1_ - fifo signal outputs
	--   
	--   SYS_Clk - system clock
	--   SYS_Rst - system reset
	--
	port (
		OSIF_Out_Data  : out  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Out_Empty : out std_logic;
		OSIF_Out_RE    : in  std_logic;

				OSIF_In_0_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_In_0_Empty : in  std_logic;
		OSIF_In_0_RE    : out std_logic;
		
		OSIF_In_1_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_In_1_Empty : in  std_logic;
		OSIF_In_1_RE    : out std_logic;
		

		OSIF_In_2_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_In_2_Empty : in  std_logic;
		OSIF_In_2_RE    : out std_logic;

		SYS_Clk : in std_logic;
		SYS_Rst : in std_logic
	);
end entity arbiter_icout_slot1;


architecture imp of arbiter_icout_slot1 is
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
	signal req  : std_logic_vector(2 downto 0) := (others => '0');
	signal msk  : std_logic_vector(2 downto 0) := (others => '1');
	signal msb  : std_logic_vector(2 downto 0) := (others => '0');
	signal grnt : std_logic_vector(2 downto 0) := (others => '0');
	signal orr  : std_logic := '1';

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

		req(0) <= not OSIF_In_0_Empty and msk(0);
		req(1) <= not OSIF_In_1_Empty and msk(1);
	
	req(2) <= not OSIF_In_2_Empty and msk(2);

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
					if OSIF_Out_RE = '1' and out_empty = '0' then
						word_count <= unsigned(out_data(C_OSIF_LENGTH_RANGE));

						state <= STATE_PROCESS;
					end if;

				when STATE_PROCESS =>
					if OSIF_Out_RE = '1' and out_empty = '0' then
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
	  	  (OSIF_In_0_Data and (OSIF_In_0_Data'Range => grnt(0))) or
	  	  (OSIF_In_1_Data and (OSIF_In_1_Data'Range => grnt(1))) or
	  
	  (OSIF_In_2_Data and (OSIF_In_2_Data'Range => grnt(2))) or
	  (C_OSIF_DATA_WIDTH - 1 downto 0 => '0');

	out_empty <=
	  	  (OSIF_In_0_Empty and grnt(0)) or
	  	  (OSIF_In_1_Empty and grnt(1)) or
	  
	  (OSIF_In_2_Empty and grnt(2)) or
	  orr;

	OSIF_Out_Data  <= out_data;
	OSIF_Out_Empty <= out_empty;
		OSIF_In_0_RE <= OSIF_Out_RE and grnt(0);
		OSIF_In_1_RE <= OSIF_Out_RE and grnt(1);
	
	OSIF_In_2_RE <= OSIF_Out_RE and grnt(2);

end architecture imp;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;

entity arbiter_icout_slot2 is
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
	--   OSIF_Out_ - fifo signal inputs
	--
	--   OSIF_In_2_ - fifo signal outputs
	--   
	--   SYS_Clk - system clock
	--   SYS_Rst - system reset
	--
	port (
		OSIF_Out_Data  : out  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Out_Empty : out std_logic;
		OSIF_Out_RE    : in  std_logic;

				OSIF_In_0_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_In_0_Empty : in  std_logic;
		OSIF_In_0_RE    : out std_logic;
		
		OSIF_In_1_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_In_1_Empty : in  std_logic;
		OSIF_In_1_RE    : out std_logic;
		

		OSIF_In_2_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_In_2_Empty : in  std_logic;
		OSIF_In_2_RE    : out std_logic;

		SYS_Clk : in std_logic;
		SYS_Rst : in std_logic
	);
end entity arbiter_icout_slot2;


architecture imp of arbiter_icout_slot2 is
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
	signal req  : std_logic_vector(2 downto 0) := (others => '0');
	signal msk  : std_logic_vector(2 downto 0) := (others => '1');
	signal msb  : std_logic_vector(2 downto 0) := (others => '0');
	signal grnt : std_logic_vector(2 downto 0) := (others => '0');
	signal orr  : std_logic := '1';

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

		req(0) <= not OSIF_In_0_Empty and msk(0);
		req(1) <= not OSIF_In_1_Empty and msk(1);
	
	req(2) <= not OSIF_In_2_Empty and msk(2);

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
					if OSIF_Out_RE = '1' and out_empty = '0' then
						word_count <= unsigned(out_data(C_OSIF_LENGTH_RANGE));

						state <= STATE_PROCESS;
					end if;

				when STATE_PROCESS =>
					if OSIF_Out_RE = '1' and out_empty = '0' then
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
	  	  (OSIF_In_0_Data and (OSIF_In_0_Data'Range => grnt(0))) or
	  	  (OSIF_In_1_Data and (OSIF_In_1_Data'Range => grnt(1))) or
	  
	  (OSIF_In_2_Data and (OSIF_In_2_Data'Range => grnt(2))) or
	  (C_OSIF_DATA_WIDTH - 1 downto 0 => '0');

	out_empty <=
	  	  (OSIF_In_0_Empty and grnt(0)) or
	  	  (OSIF_In_1_Empty and grnt(1)) or
	  
	  (OSIF_In_2_Empty and grnt(2)) or
	  orr;

	OSIF_Out_Data  <= out_data;
	OSIF_Out_Empty <= out_empty;
		OSIF_In_0_RE <= OSIF_Out_RE and grnt(0);
		OSIF_In_1_RE <= OSIF_Out_RE and grnt(1);
	
	OSIF_In_2_RE <= OSIF_Out_RE and grnt(2);

end architecture imp;

