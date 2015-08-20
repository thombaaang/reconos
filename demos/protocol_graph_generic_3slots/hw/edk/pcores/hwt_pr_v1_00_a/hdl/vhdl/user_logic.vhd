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
		delay_per_packet     : in std_logic_vector(63 downto 0);
		delay_per_byte       : in std_logic_vector(63 downto 0);
		drop_type            : in std_logic_vector(31 downto 0); -- 0: drop disabled; 1: drop fix; 2: drop random
		drop_value  			: in std_logic_vector(31 downto 0); -- if drop_type == 1: every drop_value-th packet will be dropped
																					-- if drop_type == 2: every ((2^32)/drop_value)-th packet will be dropped on average (calculate division in software (rounded to the next integer))
																					-- NOTE: in both cases the value drop_value == 0 is not valid an will disable the drop functionality
		randgen_seed 			: in std_logic_vector(31 downto 0);
		initialized          : in std_logic;
		packets_arrived		: out std_logic_vector(31 downto 0); -- state and output of data_fsm
		packets_forwarded		: out std_logic_vector(31 downto 0) -- state and output of data_fsm
	);

end user_logic;

architecture implementation of user_logic is
	--------------------------
	-- Component Declaration
	--------------------------
	
	component fifo8
		generic (
			C_FIFO8_DEPTH  		: integer := 160;
			C_FIFO8_DEPTH_LOG2 	: integer := 11 -- clog2(C_FIFO8_DEPTH) function not supported or needs proc_common library
		);
		port(
			Rst 				: in std_logic;
			FIFO8_S_Clk 	: in std_logic;
			FIFO8_M_Clk 	: in std_logic;
			FIFO8_S_Data 	: out std_logic_vector(7 downto 0);
			FIFO8_M_Data 	: in std_logic_vector(7 downto 0);
			FIFO8_S_Fill 	: out std_logic_vector(15 downto 0); -- current size
			FIFO8_M_Rem 	: out std_logic_vector(15 downto 0); -- remainder
			FIFO8_S_Rd 		: in std_logic;
			FIFO8_M_Wr 		: in std_logic
		);
	end component;
	
	-- chipscope (hardware debugging)
	
--	component generic_icon
--		port (
--			control0 : inout std_logic_vector(35 downto 0));
--	end component;
--	
--	component chipscope_ila
--	  port (
--		 control : INOUT STD_LOGIC_VECTOR(35 DOWNTO 0);
--		 clk : IN STD_LOGIC;
--		 trig0 : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
--		 trig1 : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
--		 trig2 : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
--		 trig3 : IN STD_LOGIC_VECTOR(63 DOWNTO 0);
--		 trig4 : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
--		 trig5 : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
--		 trig6 : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
--		 trig7 : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
--		 trig8 : IN STD_LOGIC_VECTOR(63 DOWNTO 0);
--		 trig9 : IN STD_LOGIC_VECTOR(63 DOWNTO 0);
--		 trig10 : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
--		 trig11 : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
--		 trig12 : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
--		 trig13 : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
--		 trig14 : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
--		 trig15 : IN STD_LOGIC_VECTOR(31 DOWNTO 0));
--	end component;
	
	------------------------
	-- signals & constants  
	------------------------
	
	-- data_fsm
	type DATA_FSM_STATE is (DATA_STATE_IDLE, DATA_STATE_RECEIVING, DATA_STATE_PKT_RECEIVED, 
		DATA_STATE_SENDING, DATA_STATE_DROPPED);
	signal data_state 							: DATA_FSM_STATE;
	signal data_state_next 						: DATA_FSM_STATE;
	signal packets_arrived_state 				: std_logic_vector(31 downto 0);
	signal packets_arrived_state_next		: std_logic_vector(31 downto 0);
	signal packets_forwarded_state 			: std_logic_vector(31 downto 0);
	signal packets_forwarded_state_next 	: std_logic_vector(31 downto 0);
	signal timer_enable 							: std_logic;
	signal drop_enable 							: std_logic;
	
	-- timer_fsm
	type TIMER_FSM_STATE is (TIMER_STATE_IDLE, TIMER_STATE_RUNNING_BYTES, TIMER_STATE_RUNNING_PACKET, TIMER_STATE_DONE);
	signal timer_state 				: TIMER_FSM_STATE;
	signal timer_state_next			: TIMER_FSM_STATE;
	signal timer_done 				: std_logic;
	signal timer_count 				: std_logic_vector(63 downto 0);
	signal timer_count_next			: std_logic_vector(63 downto 0);
	signal timer_count_bytes		: std_logic_vector(15 downto 0);
	signal timer_count_bytes_next	: std_logic_vector(15 downto 0);
	
	-- drop_fsm
	type DROP_FSM_STATE is (DROP_STATE_INIT, DROP_STATE_DISABLED, DROP_STATE_FIX, DROP_STATE_FIX_FETCHED, 
		DROP_STATE_RANDOM, DROP_STATE_RANDOM_FETCHED);
	signal drop_state 		: DROP_FSM_STATE;
	signal drop_state_next	: DROP_FSM_STATE;
	signal drop_num			: std_logic_vector(31 downto 0);
	signal drop_num_next		: std_logic_vector(31 downto 0);
	signal drop_done			: std_logic;
	signal drop_decision		: std_logic;
	
	-- randgen_fsm
	type RANDGEN_FSM_STATE is (RANDGEN_STATE_INIT, RANDGEN_STATE_RUNNING);
	signal randgen_state 		: RANDGEN_FSM_STATE;
	signal randgen_state_next	: RANDGEN_FSM_STATE;
	signal randgen_num 			: std_logic_vector(31 downto 0);
	signal randgen_num_next		: std_logic_vector(31 downto 0);
	
	signal data_step  : std_logic_vector(2 downto 0);
	signal timer_step  : std_logic_vector(2 downto 0);
	signal drop_step  : std_logic_vector(2 downto 0);
	signal rand_step  : std_logic_vector(2 downto 0);


	-- fifo8
	signal FIFO8_S_Data 	: std_logic_vector(7 downto 0);
	signal FIFO8_M_Data 	: std_logic_vector(7 downto 0);
	signal FIFO8_S_Fill 	: std_logic_vector(15 downto 0); -- current size
	signal FIFO8_M_Rem 	: std_logic_vector(15 downto 0); -- remainder
	signal FIFO8_S_Rd 	: std_logic;
	signal FIFO8_M_Wr 	: std_logic;
	
	signal rx_ll_dst_rdy_2      : std_logic;	
	signal tx_ll_sof_2		    : std_logic;
	signal tx_ll_eof_2   	    : std_logic;
	signal tx_ll_data_2		    : std_logic_vector(7 downto 0);
	signal tx_ll_src_rdy_2	    : std_logic;
	
	
	-- chipscope (hardware debugging)
--	signal trig0 : std_logic_vector(15 downto 0);
--	signal trig1 : std_logic_vector(31 downto 0);
--	signal trig2 : std_logic_vector(31 downto 0);
--	signal trig3 : std_logic_vector(63 downto 0);
--	signal trig4 : std_logic_vector(15 downto 0);
--	signal trig5 : std_logic_vector(31 downto 0);
--	signal trig6 : std_logic_vector(31 downto 0);
--	signal trig7 : std_logic_vector(7 downto 0);
--	signal trig8 : std_logic_vector(63 downto 0);
--	signal trig9 : std_logic_vector(63 downto 0);
--	signal trig10 : std_logic_vector(31 downto 0);
--	signal trig11 : std_logic_vector(31 downto 0);
--	signal trig12 : std_logic_vector(15 downto 0);
--	signal trig13 : std_logic_vector(15 downto 0);
--	signal trig14 : std_logic_vector(15 downto 0);
--	signal trig15 : std_logic_vector(31 downto 0);
--	signal control0 : std_logic_vector(35 downto 0);

begin

	---------------------------
	-- Instantiate components
	---------------------------
	fifo8_inst: fifo8 
		generic map (
				C_FIFO8_DEPTH 			=> 1600,
				C_FIFO8_DEPTH_LOG2 	=> 11
			)
		port map (			 
				Rst 				=> rst,
				FIFO8_S_Clk 	=> clk,
				FIFO8_M_Clk 	=> clk,
				FIFO8_S_Data 	=> tx_ll_data_2, -- output
				FIFO8_M_Data 	=> rx_ll_data, -- input
				FIFO8_S_Fill 	=> FIFO8_S_Fill, -- current size
				FIFO8_M_Rem 	=> FIFO8_M_Rem, -- remaining size
				FIFO8_S_Rd		=> FIFO8_S_Rd,
				FIFO8_M_Wr 		=> FIFO8_M_Wr
        );
	
	-- chipscope
--	my_icon : generic_icon
--		port map ( control0 => control0);
--		
--	my_ila : chipscope_ila
--	  port map (
--		 control => control0,
--		 clk => clk,
--		 trig0 => trig0,
--		 trig1 => trig1,
--		 trig2 => trig2,
--		 trig3 => trig3,
--		 trig4 => trig4,
--		 trig5 => trig5,
--		 trig6 => trig6,
--		 trig7 => trig7,
--		 trig8 => trig8,
--		 trig9 => trig9,
--		 trig10 => trig10,
--		 trig11 => trig11,
--		 trig12 => trig12,
--		 trig13 => trig13,
--		 trig14 => trig14,
--		 trig15 => trig15);
		 
		  
	--------------------------------------
   -- connect internal signals to ports
	--------------------------------------
	
	-- data_fsm
   packets_arrived 		<= packets_arrived_state;
	packets_forwarded 	<= packets_forwarded_state;
	
	----------------------------------------------------
	-- chipscope signal generation (hardware debugging)
	----------------------------------------------------
--	trig0 	<= rst & initialized & "00" & data_step & timer_step & drop_step & rand_step;
--	trig1 	<= packets_arrived_state;
--	trig2 	<= packets_forwarded_state;
--	trig3 	<= timer_count;
--	trig4 	<= timer_count_bytes;
--	trig5 	<= drop_num;
--	trig6 	<= randgen_num;
--	trig7 	<= timer_enable & timer_done & drop_enable & drop_done & "00" & FIFO8_S_Rd & FIFO8_M_Wr;
--	trig8 	<= delay_per_packet;
--	trig9 	<= delay_per_byte;
--	trig10 	<= drop_type;
--	trig11 	<= drop_value;
--	trig12 	<= rx_ll_sof & rx_ll_eof & rx_ll_src_rdy & rx_ll_dst_rdy_2 & "0000" & rx_ll_data;
--	trig13 	<= tx_ll_sof_2 & tx_ll_eof_2 & tx_ll_src_rdy_2 & tx_ll_dst_rdy & "0000" & tx_ll_data_2;
--	trig14 	<= FIFO8_S_Data & FIFO8_M_Data;
--	trig15 	<= FIFO8_S_Fill & FIFO8_M_Rem;
--	
--	with data_state select data_step <=
--		"001" when DATA_STATE_IDLE,
--		"010" when DATA_STATE_RECEIVING,
--		"011" when DATA_STATE_PKT_RECEIVED,
--		"100" when DATA_STATE_SENDING,
--		"101" when DATA_STATE_DROPPED;
--		
--	with timer_state select timer_step <=
--		"001" when TIMER_STATE_IDLE,
--		"010" when TIMER_STATE_RUNNING_BYTES,
--		"011" when TIMER_STATE_RUNNING_PACKET,
--		"100" when TIMER_STATE_DONE;
--	
--	with drop_state select drop_step <=
--		"001" when DROP_STATE_INIT,
--		"010" when DROP_STATE_DISABLED,
--		"011" when DROP_STATE_FIX,
--		"100" when DROP_STATE_FIX_FETCHED,
--		"101" when DROP_STATE_RANDOM,
--		"110" when DROP_STATE_RANDOM_FETCHED;
--		
--	with randgen_state select rand_step <=
--		"001" when RANDGEN_STATE_INIT,
--		"010" when RANDGEN_STATE_RUNNING;
	
	-----------------------
	-- FSMs
	-----------------------
   
	------------------------------------------------
	-- DATA_FSM
	------------------------------------------------
	-- control LocalLink interface & FIFO & provide stats
	-- TODO: check if fifo full
	------------------------------------------------
	
	rx_ll_dst_rdy <= rx_ll_dst_rdy_2;
	tx_ll_sof <= tx_ll_sof_2;
	tx_ll_eof <= tx_ll_eof_2;
	tx_ll_data <= tx_ll_data_2;
	tx_ll_src_rdy <= tx_ll_src_rdy_2;
	
   data_fsm_memless: process (data_state, packets_arrived_state, packets_forwarded_state, rx_ll_src_rdy, rx_ll_sof, rx_ll_eof, tx_ll_dst_rdy, 
		timer_done, drop_done, drop_decision, FIFO8_S_Fill, FIFO8_M_Rem) is
   begin
		-- default assignments
		-- state (remain in present state)
		data_state_next <= data_state; 
		packets_arrived_state_next <= packets_arrived_state;
		packets_forwarded_state_next <= packets_forwarded_state;
		-- outputs
		timer_enable <= '0';
		drop_enable <= '0';
		FIFO8_S_Rd <= '0';
		FIFO8_M_Wr <= '0';
		tx_ll_sof_2 <= '0';
		tx_ll_eof_2 <= '0';
		tx_ll_src_rdy_2 <= '0';
		rx_ll_dst_rdy_2 <= '0';
	
		-- nondefault transitions and outputs
		case data_state is
			when DATA_STATE_IDLE =>
				drop_enable <= '1';
				if drop_done = '1' then
					rx_ll_dst_rdy_2 <= '1';
					if rx_ll_src_rdy='1' then
						if rx_ll_sof='1' then 
							-- new packet arrived
							if packets_arrived_state < X"ffffffff" then -- prevent roll-over
								packets_arrived_state_next <= packets_arrived_state + 1;
							end if;
							if drop_decision='0' then 
								data_state_next <= DATA_STATE_RECEIVING;
								FIFO8_M_Wr <= '1';
								drop_enable <= '0';
							else
								data_state_next <= DATA_STATE_DROPPED;
								drop_enable <= '0';
							end if;
						end if;
					end if;
				end if;
			when DATA_STATE_RECEIVING => 
				rx_ll_dst_rdy_2 <= '1';
				if rx_ll_src_rdy='1' then
					FIFO8_M_Wr <= '1';
					if rx_ll_eof='1' then 
						data_state_next <= DATA_STATE_PKT_RECEIVED;
						timer_enable <= '1';
					end if;
				end if;
			when DATA_STATE_PKT_RECEIVED =>
				timer_enable <= '1';
				if timer_done='1' then
					-- src_rdy AND sof must be set and DATA must be applied that the dst will set dst_rdy
					-- DATA is applied all the time since the FIFO always points to the next value
					tx_ll_src_rdy_2 <= '1';
					tx_ll_sof_2 <= '1';
					if tx_ll_dst_rdy= '1' then
						FIFO8_S_Rd <= '1';
						data_state_next <= DATA_STATE_SENDING;
					end if;
				end if;	
			when DATA_STATE_SENDING => 
				tx_ll_src_rdy_2 <= '1';
				if tx_ll_dst_rdy='1' then
					FIFO8_S_Rd <= '1';
					if unsigned(FIFO8_S_Fill) = 1 then
						tx_ll_eof_2 <= '1';
						data_state_next <= DATA_STATE_IDLE;
						drop_enable <= '1';
						if packets_forwarded_state < X"ffffffff" then -- prevent roll-over
							packets_forwarded_state_next <= packets_forwarded_state + 1;
						end if;
					end if;
				end if;
			when DATA_STATE_DROPPED => 
				data_state_next <= DATA_STATE_IDLE;
				drop_enable <= '1';
				rx_ll_dst_rdy_2 <= '1'; -- tell src to continue sending the dropped packet
			when others =>
				data_state_next <= DATA_STATE_IDLE;
		end case;
	end process;	

	data_fsm_memzing: process(clk, rst) is
	begin
		if rst = '1' then
			data_state  <= DATA_STATE_IDLE;
			packets_arrived_state <= (others => '0');
			packets_forwarded_state <= (others => '0');
		elsif rising_edge(clk) then
			data_state  <= data_state_next;
			packets_arrived_state <= packets_arrived_state_next;
			packets_forwarded_state <= packets_forwarded_state_next;
		end if;
	end process;
   
	------------------------------------------------
	-- TIMER_FSM
	------------------------------------------------
   -- count down to produce an artificial delay for the packet currently buffered
	------------------------------------------------
	timer_fsm_memless : process (timer_state, timer_count, timer_count_bytes, timer_enable, FIFO8_S_Fill) is
   begin
		-- default assignments
		-- state
		timer_state_next <= timer_state; -- remain in present state
		timer_count_next <= timer_count;
		timer_count_bytes_next <= timer_count_bytes; 
		-- outputs
		timer_done <= '0';

		-- nondefault transitions and outputs
		case timer_state is
			when TIMER_STATE_IDLE =>
				if timer_enable = '1' then
					if delay_per_byte > 0 then
						-- delay_per_byte > 0 and delay_per_packet == X (don't care)
						timer_state_next <= TIMER_STATE_RUNNING_BYTES;
						timer_count_next <= delay_per_byte - 1; 
						timer_count_bytes_next <= FIFO8_S_Fill; -- not minus 1 because FIFO_S_Fill value is aquired when FIFO does not yet contain last byte of packet
					elsif delay_per_packet > 0 then
						-- delay_per_byte == 0 and delay_per_packet > 0
						timer_state_next <= TIMER_STATE_RUNNING_PACKET;
						timer_count_next <= delay_per_packet - 1;
					else
						-- delay_per_byte == 0 and delay_per_packet == 0
						timer_state_next <= TIMER_STATE_DONE;
					end if;
				end if;
			when TIMER_STATE_RUNNING_BYTES => 
				if unsigned(timer_count) = 0 and unsigned(timer_count_bytes) = 0 then
					-- finished counting delay_per_byte
					if delay_per_packet > 0 then
						timer_state_next <= TIMER_STATE_RUNNING_PACKET;
						timer_count_next <= delay_per_packet - 1;
					else
						timer_state_next <= TIMER_STATE_DONE;
					end if;
				else
					if timer_count = X"0000" then
						timer_count_next <= delay_per_byte - 1;
						timer_count_bytes_next <= timer_count_bytes - 1;
					else
						timer_count_next <= timer_count - 1;
					end if;
				end if;
			when TIMER_STATE_RUNNING_PACKET => 
				if unsigned(timer_count) = 0 then
					-- finished counting delay_per_packet
					timer_state_next <= TIMER_STATE_DONE;
				else
					timer_count_next <= timer_count - 1;
				end if;
			when TIMER_STATE_DONE => 
				if timer_enable <= '0' then
					timer_state_next <= TIMER_STATE_IDLE;
				else
					timer_done <= '1';
				end if;
			when others =>
				timer_state_next <= TIMER_STATE_IDLE;
		end case;
   end process;
	
	timer_fsm_memzing : process(clk, rst) is
	begin
		if rst = '1' then
			timer_state  <= TIMER_STATE_IDLE;
			timer_count <= (others => '0');
			timer_count_bytes <= (others => '0');
		elsif rising_edge(clk) then
			timer_state  <= timer_state_next;
			timer_count <= timer_count_next;
			timer_count_bytes <= timer_count_bytes_next;
		end if;
	end process;
	
	------------------------------------------------
	-- DROP_FSM
	------------------------------------------------
	-- produces decision whether to drop or keep the next arriving packet
	------------------------------------------------
	drop_fsm_memless : process (drop_state, drop_num, initialized, drop_enable, drop_type, drop_value, randgen_num) is
   begin
		-- default assignments
		-- state
		drop_state_next <= drop_state;
		drop_num_next <= drop_num;
		-- outputs
		drop_done <= '0';
		drop_decision <= '0';

		-- nondefault transitions and outputs
		case drop_state is
			when DROP_STATE_INIT =>
				if initialized = '1' then
					if drop_enable = '1' then
						if unsigned(drop_type) = 0 then
							-- dropping disabled
							drop_state_next <= DROP_STATE_DISABLED;
						elsif unsigned(drop_type) = 1 then
							-- fixed type (drop every drop_value-th packet)
							if unsigned(drop_value) = 0 then
								-- invalid drop_value (i.e. drop every zeroest-packet) -> disable drop
								drop_state_next <= DROP_STATE_DISABLED;
							else
								drop_state_next <= DROP_STATE_FIX;
								drop_num_next <= drop_value - 1;
							end if;
						elsif unsigned(drop_type) = 2 then
							if unsigned(drop_value) = 0 then
								-- invalid drop_value (i.e. drop every infinitieth-packet) -> disable drop
								drop_state_next <= DROP_STATE_DISABLED;
							else
								-- random type (drop every ((2^32)/drop-value)-th packet on average)
								drop_state_next <= DROP_STATE_RANDOM;
								-- store a new random number for the next drop decision
								drop_num_next <= randgen_num;
							end if;
						else
							-- do nothing (invalid option)
							null;
						end if;
					end if;
				end if;
			when DROP_STATE_DISABLED =>
				drop_done <= '1';
				drop_decision <= '0'; -- line not necessary, for better understanding only
				if initialized = '0' then
					drop_state_next <= DROP_STATE_INIT;
				end if;
			when DROP_STATE_FIX =>
				drop_done <= '1';
				if unsigned(drop_num) = 0 then
					drop_decision <= '1';
				else
					drop_decision <= '0'; -- line not necessary, for better understanding only
				end if;	
				if drop_enable = '0' then
					drop_state_next <= DROP_STATE_FIX_FETCHED;
				end if;
			when DROP_STATE_FIX_FETCHED =>
				drop_done <= '0'; -- line not necessary, for better understanding only
				if drop_enable = '1' then
					drop_state_next <= DROP_STATE_FIX;
					if unsigned(drop_num) = 0 then
						-- reinitialize
						drop_num_next <= drop_value - 1;
					else
						-- count
						drop_num_next <= drop_num - 1;
					end if;
				end if;
			when DROP_STATE_RANDOM => 
				drop_done <= '1';
				if drop_num > drop_value then
					drop_decision <= '0';
				else
					drop_decision <= '1';
				end if;
				if drop_enable = '0' then
					drop_state_next <= DROP_STATE_RANDOM_FETCHED;
				end if;
			when DROP_STATE_RANDOM_FETCHED =>
				drop_done <= '0'; -- line not necessary, for better understanding only
				if drop_enable = '1' then
					drop_state_next <= DROP_STATE_RANDOM;
					-- store a new random number for the next drop decision
					drop_num_next <= randgen_num;
				end if;
			when others =>
				drop_state_next <= DROP_STATE_INIT;
		end case;
   end process;
	
	drop_fsm_memzing : process(clk, rst) is
	begin
		if rst = '1' then
			drop_state <= DROP_STATE_INIT;
			drop_num <= (others => '0');
		elsif rising_edge(clk) then
			drop_state <= drop_state_next;
			drop_num <= drop_num_next;
		end if;
	end process;
	
	------------------------------------------------
	-- RANDGEN_FSM
	------------------------------------------------
   -- continuously generate pseudo random numbers (if enabled by drop_type)
	------------------------------------------------
	randgen_fsm_memless : process (randgen_state, randgen_num, initialized, drop_type, randgen_seed) is
   begin
		-- default assignments
		-- state
		randgen_state_next <= randgen_state;
		randgen_num_next <= randgen_num;
		-- outputs
		-- randgen_num is also an output

		-- nondefault transitions and outputs
		case randgen_state is
			when RANDGEN_STATE_INIT =>
				if initialized = '1' then -- drop_type is not availabe if FB is not initialized
				  if unsigned(drop_type) = 2 then -- if drop_type == random
				    randgen_state_next <= RANDGEN_STATE_RUNNING;
				    randgen_num_next <= randgen_seed; -- seed randgen with seed from sofware (reconos_fsm)
				  end if;
            end if;
			when RANDGEN_STATE_RUNNING =>
				randgen_num_next <= (randgen_num(1) xnor randgen_num(0)) & randgen_num(31 downto 1);
			when others =>
				randgen_state_next <= RANDGEN_STATE_INIT;
		end case;
   end process;
	
	randgen_fsm_memzing : process(clk, rst) is
	begin
		if rst = '1' then
			randgen_state <= RANDGEN_STATE_INIT;
			randgen_num <= (others => '0');
		elsif rising_edge(clk) then
			randgen_state <= randgen_state_next;
			randgen_num <= randgen_num_next;
		end if;
	end process;
	
end architecture;
