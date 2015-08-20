library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

--library proc_common_v3_00_a;
--use proc_common_v3_00_a.proc_common_pkg.all;

--library reconos_v3_00_b;
--use reconos_v3_00_b.reconos_pkg.all;

--library ana_v1_00_a;
--use ana_v1_00_a.anaPkg.all;

entity user_logic is
	port (
		-- HWT reset & clock
		rst : in std_logic;
		clk : in std_logic;
		
		-- LocalLink Interface
		rx_ll_sof		    : in std_logic;
		rx_ll_eof		    : in std_logic;
		rx_ll_data		    : in std_logic_vector(7 downto 0);
		rx_ll_src_rdy	    : in std_logic;
		rx_ll_dst_rdy       : out std_logic;
		
		tx_ll_sof		    : out std_logic;
		tx_ll_eof   	    : out std_logic;
		tx_ll_data		    : out std_logic_vector(7 downto 0);
		tx_ll_src_rdy	    : out std_logic;
		tx_ll_dst_rdy	    : in std_logic;
		
		-- reconos_fsm
		delay_per_packet    : in std_logic_vector(63 downto 0);
		delay_per_byte      : in std_logic_vector(63 downto 0);
		drop_type           : in std_logic_vector(31 downto 0);
		drop_value          : in std_logic_vector(31 downto 0);
		initialized         : in std_logic
		
	);

end user_logic;

architecture implementation of user_logic is
	-- parameters of FB (set by reconos_fsm)
	signal delay_per_packet : std_logic_vector(63 downto 0);
	signal delay_per_byte   : std_logic_vector(63 downto 0);
	signal drop_type        : std_logic_vector(31 downto 0); -- only 1 bit used at the moment (right most)
	signal drop_value       : std_logic_vector(31 downto 0);
	signal randgen_seed 		: std_logic_vector(31 downto 0);
	
	-- randgen_fsm
	type RANDGEN_FSM_STATE is (ENABLE, RUN);
	signal randgen_num : std_logic_vector(31 downto 0);
	signal randgen_state : RANDGEN_FSM_STATE;
	
	-- timer_fsm
	type TIMER_FSM_STATE is (TIMER_STATE_START, TIMER_STATE_RUN, TIMER_STATE_GO);
	signal timer_go: std_logic;
	signal timer_state : TIMER_FSM_STATE;
	
	-- LocalLink (use the ports directly -> these signals shouldn't be necessary)
--	signal rx_ll_sof	    : std_logic;
--	signal rx_ll_eof	    : std_logic;
--	signal rx_ll_data	    : std_logic_vector(7 downto 0);
--	signal rx_ll_src_rdy	: std_logic;
--	signal rx_ll_dst_rdy	: std_logic;
	
--	signal tx_ll_sof	    : std_logic;
--	signal tx_ll_eof	    : std_logic;
--	signal tx_ll_data	    : std_logic_vector(7 downto 0);
--	signal tx_ll_src_rdy	: std_logic;
--	signal tx_ll_dst_rdy	: std_logic;

begin
   -- connect internal signals to ports
   
    

   -- continuously generate pseudo random numbers (if enabled by drop_type)
   randgen_fsm: process (rst, clk) is
   begin
	  if rst = '1' then
	 	 randgen_num <= (others => '0');
		 randgen_state <= ENABLE;
	  elsif rising_edge(clk) then
	    case randgen_state is
			when ENABLE =>
			   if initialized='1' then -- drop_type is not availabe if FB is not initialized
				  if drop_type(0)='1' then -- if drop_type == random
				    randgen_num <= randgen_seed; -- seed randgen with seed from sofware (reconos_fsm)
				    randgen_state <= RUN;
				  else
				    randgen_state <= ENABLE;
				  end if;
            end if;				  
			when RUN => 
				randgen_num <= (randgen_num(1) xnor randgen_num(0)) & randgen_num(31 downto 1);
			when others =>
			   randgen_state <= ENABLE;
	    end case;
     end if;
   end process;
	
   
   -- count down to produce an artificial delay for the packet currently buffered
   timer_fsm: process (rst, clk) is
   begin
	  if rst = '1' then
	    timer_go <= '0';
	    timer_state <= TIMER_STATE_START;
	  elsif rising_edge(clk) then
	    case timer_state is
			when TIMER_STATE_START =>
			   timer_go <= '0';
			   -- if packet is fully buffered go to RUN state				  
			when TIMER_STATE_RUN => 
				-- decrease counter
			when TIMER_STATE_GO => 
				timer_go <= '1';
			when others =>
			   timer_state <= TIMER_STATE_START;
	    end case;
     end if;
   end process;

end architecture;
