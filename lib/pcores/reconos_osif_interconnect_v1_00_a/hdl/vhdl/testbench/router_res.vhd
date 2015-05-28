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
use reconos_v3_01_a.reconos_pkg.all;

entity router_icin_res1 is
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
		OSIF_In_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_In_Empty : in  std_logic;
		OSIF_In_RE    : out std_logic;

				OSIF_Out_0_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Out_0_Empty : out std_logic;
		OSIF_Out_0_RE    : in  std_logic;
		
		OSIF_Out_1_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Out_1_Empty : out std_logic;
		OSIF_Out_1_RE    : in  std_logic;
		
		OSIF_Out_2_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Out_2_Empty : out std_logic;
		OSIF_Out_2_RE    : in  std_logic;
		

		OSIF_Out_3_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Out_3_Empty : out std_logic;
		OSIF_Out_3_RE    : in  std_logic;

		SYS_Clk : in std_logic;
		SYS_Rst : in std_logic
	);
end entity router_icin_res1;


architecture imp of router_icin_res1 is
	--
	-- Internal state machine
	--
	--   state_type - vhdl type of the states
	--   state      - instantiation of the state
	--
	type state_type is (STATE_READ_CTRL,STATE_ROUTE0,STATE_ROUTE1,
	                    STATE_WRITE_CTRL,STATE_PROCESS);
	signal state : state_type := STATE_WAIT;

	--
	-- Internal signals for round robin arbiter
	--
	--   grnt - grant vector to multiplex signals
	--
	signal grnt : std_logic_vector(3 downto 0) := (others => '0');

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
	signal out_data  : std_logic;
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
			grnt <= (others => '0');

			state <= STATE_READ_CTRL;
		elsif rising_edge(SYS_Clk) then
			case state is
				when STATE_READ_CTRL =>
					if OSIF_In_Empty = '0' then
						ctrl <= OSIF_In_Data;
						word_count <= OSIF_In_Data(C_OSIF_LENGTH_RANGE);

						state <= STATE_ROUTE;
					end if;

				when STATE_ROUTE_0 =>
										if ctrl(C_OSIF_DST_RANGE) = x"00000000" then
						grnt(0) <= '1';
					end if;
					
					if ctrl(C_OSIF_DST_RANGE) = x"00000001" then
						grnt(1) <= '1';
					end if;
					
					if ctrl(C_OSIF_DST_RANGE) = x"00000002" then
						grnt(2) <= '1';
					end if;
					


					state <= STATE_ROUTE_1;

				when STATE_ROUTE_1 =>
					if ctrl = (ctrl'Range => '0') then
						ctrl(3) <= '1';
					end if;

					state <= STATE_WRITE_CTRL;

				when STATE_WRITE_CTRL =>
					if in_re = '1' then
						state <= STATE_PROCESS;
					end if;

				when STATE_PROCESS =>
					if in_re = '1' and OSIF_In_Empty = '0' then
						word_count <= word_count - 1;

						if word_count - 1 = 0 then
							state <= STATE_WAIT;

							grnt <= (others => '0');
						end if;
					end if;

				when others =>
			end case;
		end if;
	end process route;


	-- == Multiplexing signals ============================================

	in_re <=
	  	  ((OSIF_Out_0_RE) and grnt(0)) or
	  	  ((OSIF_Out_1_RE) and grnt(1)) or
	  	  ((OSIF_Out_2_RE) and grnt(2)) or
	  
	  ((OSIF_Out_3_RE) and grnt(3));

	out_data  <= OSIF_In_Data when state = STATE_PROCESS else
	             ctrl         when state = STATE_WRITE_CTRL else
	             x"00000000";

	out_empty <= OSIF_In_Empty when state = STATE_PROCESS else
	             '0'           when state = STATE_WRITE_CTRL else
	             '1';

		OSIF_Out_0_Data <= out_data;
		OSIF_Out_1_Data <= out_data;
		OSIF_Out_2_Data <= out_data;
	
	OSIF_Out_3_Data <= out_data;

		OSIF_Out_0_Empty <= out_empty when grnt(0) = '1' else '1';
		OSIF_Out_1_Empty <= out_empty when grnt(1) = '1' else '1';
		OSIF_Out_2_Empty <= out_empty when grnt(2) = '1' else '1';
	
	OSIF_Out_3_Data <= out_empty when grnt(3) = '1' else '1';

	OSIF_In_RE <= in_re when state = STATE_PROCESS else
	              '1'   when state = STATE_READ_CTRL else
	              '0';

end architecture imp;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;

entity router_icin_res2 is
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
		OSIF_In_Data  : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_In_Empty : in  std_logic;
		OSIF_In_RE    : out std_logic;

				OSIF_Out_0_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Out_0_Empty : out std_logic;
		OSIF_Out_0_RE    : in  std_logic;
		
		OSIF_Out_1_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Out_1_Empty : out std_logic;
		OSIF_Out_1_RE    : in  std_logic;
		
		OSIF_Out_2_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Out_2_Empty : out std_logic;
		OSIF_Out_2_RE    : in  std_logic;
		

		OSIF_Out_3_Data  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		OSIF_Out_3_Empty : out std_logic;
		OSIF_Out_3_RE    : in  std_logic;

		SYS_Clk : in std_logic;
		SYS_Rst : in std_logic
	);
end entity router_icin_res2;


architecture imp of router_icin_res2 is
	--
	-- Internal state machine
	--
	--   state_type - vhdl type of the states
	--   state      - instantiation of the state
	--
	type state_type is (STATE_READ_CTRL,STATE_ROUTE0,STATE_ROUTE1,
	                    STATE_WRITE_CTRL,STATE_PROCESS);
	signal state : state_type := STATE_WAIT;

	--
	-- Internal signals for round robin arbiter
	--
	--   grnt - grant vector to multiplex signals
	--
	signal grnt : std_logic_vector(3 downto 0) := (others => '0');

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
	signal out_data  : std_logic;
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
			grnt <= (others => '0');

			state <= STATE_READ_CTRL;
		elsif rising_edge(SYS_Clk) then
			case state is
				when STATE_READ_CTRL =>
					if OSIF_In_Empty = '0' then
						ctrl <= OSIF_In_Data;
						word_count <= OSIF_In_Data(C_OSIF_LENGTH_RANGE);

						state <= STATE_ROUTE;
					end if;

				when STATE_ROUTE_0 =>
										if ctrl(C_OSIF_DST_RANGE) = x"00000000" then
						grnt(0) <= '1';
					end if;
					
					if ctrl(C_OSIF_DST_RANGE) = x"00000001" then
						grnt(1) <= '1';
					end if;
					
					if ctrl(C_OSIF_DST_RANGE) = x"00000002" then
						grnt(2) <= '1';
					end if;
					


					state <= STATE_ROUTE_1;

				when STATE_ROUTE_1 =>
					if ctrl = (ctrl'Range => '0') then
						ctrl(3) <= '1';
					end if;

					state <= STATE_WRITE_CTRL;

				when STATE_WRITE_CTRL =>
					if in_re = '1' then
						state <= STATE_PROCESS;
					end if;

				when STATE_PROCESS =>
					if in_re = '1' and OSIF_In_Empty = '0' then
						word_count <= word_count - 1;

						if word_count - 1 = 0 then
							state <= STATE_WAIT;

							grnt <= (others => '0');
						end if;
					end if;

				when others =>
			end case;
		end if;
	end process route;


	-- == Multiplexing signals ============================================

	in_re <=
	  	  ((OSIF_Out_0_RE) and grnt(0)) or
	  	  ((OSIF_Out_1_RE) and grnt(1)) or
	  	  ((OSIF_Out_2_RE) and grnt(2)) or
	  
	  ((OSIF_Out_3_RE) and grnt(3));

	out_data  <= OSIF_In_Data when state = STATE_PROCESS else
	             ctrl         when state = STATE_WRITE_CTRL else
	             x"00000000";

	out_empty <= OSIF_In_Empty when state = STATE_PROCESS else
	             '0'           when state = STATE_WRITE_CTRL else
	             '1';

		OSIF_Out_0_Data <= out_data;
		OSIF_Out_1_Data <= out_data;
		OSIF_Out_2_Data <= out_data;
	
	OSIF_Out_3_Data <= out_data;

		OSIF_Out_0_Empty <= out_empty when grnt(0) = '1' else '1';
		OSIF_Out_1_Empty <= out_empty when grnt(1) = '1' else '1';
		OSIF_Out_2_Empty <= out_empty when grnt(2) = '1' else '1';
	
	OSIF_Out_3_Data <= out_empty when grnt(3) = '1' else '1';

	OSIF_In_RE <= in_re when state = STATE_PROCESS else
	              '1'   when state = STATE_READ_CTRL else
	              '0';

end architecture imp;

