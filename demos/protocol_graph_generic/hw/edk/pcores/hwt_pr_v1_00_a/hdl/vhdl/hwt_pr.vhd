library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

--library proc_common_v3_00_a;
--use proc_common_v3_00_a.proc_common_pkg.all;

library reconos_v3_00_b;
use reconos_v3_00_b.reconos_pkg.all;

library ana_v1_00_a;
use ana_v1_00_a.anaPkg.all;

entity hwt_pr is
	port (
		-- OSIF FSL
		OSFSL_S_Read    : out std_logic;                 -- Read signal, requiring next available input to be read
		OSFSL_S_Data    : in  std_logic_vector(0 to 31); -- Input data
		OSFSL_S_Control : in  std_logic;                 -- Control Bit, indicating the input data are control word
		OSFSL_S_Exists  : in  std_logic;                 -- Data Exist Bit, indicating data exist in the input FSL bus
		OSFSL_M_Write   : out std_logic;                 -- Write signal, enabling writing to output FSL bus
		OSFSL_M_Data    : out std_logic_vector(0 to 31); -- Output data
		OSFSL_M_Control : out std_logic;                 -- Control Bit, indicating the output data are contol word
		OSFSL_M_Full    : in  std_logic;                 -- Full Bit, indicating output FSL bus is full
			
		-- FIFO Interface
		FIFO32_S_Data : in std_logic_vector(31 downto 0);
		FIFO32_M_Data : out std_logic_vector(31 downto 0);
		FIFO32_S_Fill : in std_logic_vector(15 downto 0);
		FIFO32_M_Rem : in std_logic_vector(15 downto 0);
		FIFO32_S_Rd : out std_logic;
		FIFO32_M_Wr : out std_logic;
		
		-- HWT reset
		rst : in std_logic;
		clk : in std_logic;

		switch_data_rdy		: in  std_logic;
		switch_data		: in  std_logic_vector(dataWidth downto 0);
		thread_read_rdy	 	: out std_logic;
		switch_read_rdy		: in  std_logic;
		thread_data		: out std_logic_vector(dataWidth downto 0);
		thread_data_rdy 	: out std_logic
		
	);

end hwt_pr;

architecture implementation of hwt_pr is
   ---------------------------------
   -- declare components to be used
   ---------------------------------
	component user_logic
		port(
			-- HWT reset & clock
			rst : in std_logic;
			clk : in std_logic;

			-- LocalLink Interface
			rx_ll_sof 				: in std_logic;
			rx_ll_eof 				: in std_logic;
			rx_ll_data 				: in std_logic_vector(7 downto 0);
			rx_ll_src_rdy 			: in std_logic;
			rx_ll_dst_rdy 			: out std_logic;

			tx_ll_sof				: out std_logic;
			tx_ll_eof				: out std_logic;
			tx_ll_data				: out std_logic_vector(7 downto 0);
			tx_ll_src_rdy			: out std_logic;
			tx_ll_dst_rdy			: in std_logic;

			-- reconos_fsm
			delay_per_packet    	: in std_logic_vector(63 downto 0);
			delay_per_byte      	: in std_logic_vector(63 downto 0);
			drop_type           	: in std_logic_vector(31 downto 0);
			drop_value          	: in std_logic_vector(31 downto 0);
			randgen_seed 			: in std_logic_vector(31 downto 0);
			initialized 			: in std_logic;
			packets_arrived 		: out std_logic_vector(31 downto 0); -- state and output of data_fsm
			packets_forwarded 	: out std_logic_vector(31 downto 0) -- state and output of data_fsm
		);
	end component;
    
	---------------------------------
	-- signals & constants
	---------------------------------
	
	-- reconos_fsm (constants)      [PUT/GET: viewed from within the FB]
	type RECONOS_STATE_TYPE is (
		-- control
		GET_CMD, GET_ADDR, PUT_FB_ID, THREAD_EXIT, 
		-- PUT
		PUT_STATS_ARRIVED, PUT_STATS_FORWARDED, 
		PUT_DPP_H, PUT_DPP_L, PUT_DPB_H, PUT_DPB_L, PUT_DT, PUT_DV, PUT_SEED, 
		PUT_ADDR, 
		-- GET parameters user_logic
		GET_ALL, GET_ALL_DPP_H, GET_ALL_DPP_L, GET_ALL_DPB_H, GET_ALL_DPB_L, GET_ALL_DT, 
		GET_ALL_SEED, GET_ALL_DV);
	
	-- control
	constant CMD_EXIT_THREAD 	: std_logic_vector(31 downto 0) := X"ffffffff";
	constant CMD_GET_ADDR 		: std_logic_vector(31 downto 0) := X"00000001";
	constant CMD_PUT_ADDR 		: std_logic_vector(31 downto 0) := X"0000000a";
	constant CMD_PUT_FB_ID 		: std_logic_vector(31 downto 0) := X"00000002";
	-- GET parameters user_logic
	constant CMD_GET_ALL 		: std_logic_vector(31 downto 0) := X"00000003";
	-- PUT stats user_logic
	constant CMD_PUT_STATS 		: std_logic_vector(31 downto 0) := X"00000004";
	-- PUT parameters user_logic
	constant CMD_PUT_DPP 		: std_logic_vector(31 downto 0) := X"00000005";
	constant CMD_PUT_DPB 		: std_logic_vector(31 downto 0) := X"00000006";
	constant CMD_PUT_DT 			: std_logic_vector(31 downto 0) := X"00000007";
	constant CMD_PUT_DV 			: std_logic_vector(31 downto 0) := X"00000008";
	constant CMD_PUT_SEED 		: std_logic_vector(31 downto 0) := X"00000009";
	
	
	-- mbox
	constant MBOX_RECV  	: std_logic_vector(C_FSL_WIDTH-1 downto 0) := x"00000000";
	constant MBOX_SEND  	: std_logic_vector(C_FSL_WIDTH-1 downto 0) := x"00000001";
	
	-- pr_module
	constant PR_MODULE  	: std_logic_vector(7 downto 0) := X"0A";

   -- reconos_fsm (signals)
	signal reconos_data  : std_logic_vector(31 downto 0);
	signal reconos_state	: RECONOS_STATE_TYPE;
	signal initialized 	: std_logic;
	
	signal i_osif   : i_osif_t;
	signal o_osif   : o_osif_t;
	signal i_memif  : i_memif_t;
	signal o_memif  : o_memif_t;
	signal i_ram    : i_ram_t;
	signal o_ram    : o_ram_t;
	
	-- parameters of FB (set by reconos_fsm)
	signal delay_per_packet 	: std_logic_vector(63 downto 0);
	signal delay_per_byte 		: std_logic_vector(63 downto 0);
	signal drop_type 				: std_logic_vector(31 downto 0); -- only 1 bit used at the moment (right most)
	signal drop_value 			: std_logic_vector(31 downto 0);
	signal randgen_seed 			: std_logic_vector(31 downto 0);
	
	-- values from FB for reconos_fsm
	signal packets_arrived 		: std_logic_vector(31 downto 0); -- state and output of data_fsm
	signal packets_forwarded 	: std_logic_vector(31 downto 0); -- state and output of data_fsm
	
	-- other stuff
	signal ignore   : std_logic_vector(C_FSL_WIDTH-1 downto 0);
	
	signal rx_ll_dst_rdy_local	: std_logic;
	
	signal tx_ll_sof	: std_logic;
	signal tx_ll_eof	: std_logic;
	signal tx_ll_data	: std_logic_vector(7 downto 0);
	signal tx_ll_src_rdy	: std_logic;
	signal tx_ll_dst_rdy	: std_logic;

	signal rx_ll_sof	: std_logic;
	signal rx_ll_eof	: std_logic;
	signal rx_ll_data	: std_logic_vector(7 downto 0);
	signal rx_ll_src_rdy	: std_logic;
	signal rx_ll_dst_rdy	: std_logic;

	signal direction : std_logic;
	signal priority : std_logic_vector(1 downto 0);
	signal latency_critical : std_logic;
	signal srcIdp : std_logic_vector(31 downto 0);
	signal dstIdp : std_logic_vector(31 downto 0);
	signal global_addr : std_logic_vector(3 downto 0);
	signal local_addr : std_logic_vector(1 downto 0);

	signal tx_ll_sof_n  	   : std_logic;         ----active low 
	signal tx_ll_eof_n      : std_logic;         ----active low 
	signal tx_ll_src_rdy_n  : std_logic;         ----active low  
	signal tx_ll_dst_rdy_n  : std_logic;         ----active low
	signal rx_ll_sof_n       : std_logic;         ----active low
	signal rx_ll_eof_n       : std_logic;         ----active low
	signal rx_ll_src_rdy_n   : std_logic;         ----active low
	signal rx_ll_dst_rdy_n   : std_logic;         ----active low


begin
	---------------------------------
    -- instantiate components
    ---------------------------------
	decoder_inst : packetDecoder
		port map (
			clk 							=> clk,
			reset 						=> rst,
			-- Signals from the switch
			switch_data_rdy			=> switch_data_rdy,
			switch_data	=> switch_data,
			thread_read_rdy			=> thread_read_rdy,
			-- Decoded values of the packet
			noc_rx_sof					=> rx_ll_sof,		-- Indicates the start of a new packet
			noc_rx_eof					=> rx_ll_eof,		-- Indicates the end of the packet
			noc_rx_data					=> rx_ll_data,		-- The current data byte
			noc_rx_src_rdy				=> rx_ll_src_rdy, 	-- '1' if the data are valid, '0' else
			noc_rx_direction			=> direction, 		-- '1' for egress, '0' for ingress
			noc_rx_priority			=> priority,		-- The priority of the packet
			noc_rx_latencyCritical	=> latency_critical,		-- '1' if this packet is latency critical
			noc_rx_srcIdp				=> srcIdp,		-- The source IDP
			noc_rx_dstIdp					=> dstIdp,		-- The destination IDP
			noc_rx_dst_rdy				=> rx_ll_dst_rdy	-- Read enable for the functional block
		);
	
	encoder_inst : packetEncoder
		port map(
			clk 							=> clk,					
			reset							=> rst,		
			-- Signals to the switch
			switch_read_rdy  			=> switch_read_rdy, 		
			thread_data  				=> thread_data,		
			thread_data_rdy 			=> thread_data_rdy,
			-- Decoded values of the packet
			noc_tx_sof  				=> tx_ll_sof, 		
			noc_tx_eof  				=> tx_ll_eof,
			noc_tx_data	 				=> tx_ll_data,		
			noc_tx_src_rdy 	 		=> tx_ll_src_rdy,		
			noc_tx_globalAddress  	=> global_addr, --"0000",--(others => '0'), --6 bits--(0:send it to hw/sw)		
			noc_tx_localAddress  	=> local_addr, --"01",-- (others  => '0'), --2 bits		
			noc_tx_direction 	 		=> direction,		
			noc_tx_priority 	 		=> "11",--priority,		
			noc_tx_latencyCritical  => latency_critical,	
			noc_tx_srcIdp 				=> srcIdp,	
			noc_tx_dstIdp 				=> dstIdp,
			noc_tx_dst_rdy	 			=> tx_ll_dst_rdy
		);
	
	user_logic_inst : user_logic
		port map(	
			-- HWT reset & clock
			rst => rst,
			clk => clk,

			-- LocalLink Interface
			rx_ll_sof 			=> rx_ll_sof,
			rx_ll_eof 			=> rx_ll_eof,
			rx_ll_data 			=> rx_ll_data,
			rx_ll_src_rdy 		=> rx_ll_src_rdy,
			rx_ll_dst_rdy		=> rx_ll_dst_rdy,

			tx_ll_sof			=> tx_ll_sof,
			tx_ll_eof 			=> tx_ll_eof,
			tx_ll_data 			=> tx_ll_data,
			tx_ll_src_rdy 		=> tx_ll_src_rdy,
			tx_ll_dst_rdy 		=> tx_ll_dst_rdy,

			-- reconos_fsm
			delay_per_packet		=> delay_per_packet,
			delay_per_byte			=> delay_per_byte,
			drop_type				=> drop_type,
			drop_value				=> drop_value,
			randgen_seed			=> randgen_seed,
			initialized				=> initialized,
			packets_arrived 		=> packets_arrived,
			packets_forwarded 	=> packets_forwarded
		);

	fsl_setup(
		i_osif,
		o_osif,
		OSFSL_S_Data,
		OSFSL_S_Exists,
		OSFSL_M_Full,
		OSFSL_M_Data,
		OSFSL_S_Read,
		OSFSL_M_Write,
		OSFSL_M_Control
	);
		
	memif_setup(
		i_memif,
		o_memif,
		FIFO32_S_Data,
		FIFO32_S_Fill,
		FIFO32_S_Rd,
		FIFO32_M_Data,
		FIFO32_M_Rem,
		FIFO32_M_Wr
	);
	

	-- os and memory synchronisation state machine
	reconos_fsm: process (clk,rst,o_osif,o_memif,o_ram) is
		variable done  : boolean;
	begin
		if rst = '1' then
			osif_reset(o_osif);
			memif_reset(o_memif);
			global_addr <= "0001"; --default: send it to the h2s block
			local_addr  <= "01";

			reconos_state <= GET_CMD;
			initialized <= '0'; 
			-- default: parameters of FB not yet set by reconos_fsm (i.e. software)
			delay_per_packet <= (others => '0');
			delay_per_byte   <= (others => '0');
			drop_type        <= (others => '0'); -- only 1 bit used at the moment (right most)
			drop_value       <= (others => '0');
			randgen_seed     <= (others => '0');
		elsif rising_edge(clk) then
			case reconos_state is
			   -- get next command
			   when GET_CMD =>
					osif_mbox_get(i_osif, o_osif, MBOX_RECV, reconos_data, done);
					if done then
						-- control
						if (reconos_data = CMD_EXIT_THREAD) then
							reconos_state <= THREAD_EXIT;
						elsif (reconos_data = CMD_GET_ADDR) then
							reconos_state <= GET_ADDR;
						elsif (reconos_data = CMD_PUT_FB_ID) then
							reconos_state <= PUT_FB_ID;
						-- stats
						elsif (reconos_data = CMD_PUT_STATS) then
							reconos_state <= PUT_STATS_ARRIVED;
						-- GET parameters
						elsif (reconos_data = CMD_GET_ALL) then
						   initialized <= '0';
							reconos_state <= GET_ALL;
						-- put parameters
						elsif (reconos_data = CMD_PUT_DPP) then
							reconos_state <= PUT_DPP_H;
						elsif (reconos_data = CMD_PUT_DPB) then
							reconos_state <= PUT_DPB_H;
						elsif (reconos_data = CMD_PUT_DT) then
							reconos_state <= PUT_DT;
						elsif (reconos_data = CMD_PUT_DV) then
							reconos_state <= PUT_DV;
						elsif (reconos_data = CMD_PUT_SEED) then
							reconos_state <= PUT_SEED;
						elsif (reconos_data = CMD_PUT_ADDR) then
							reconos_state <= PUT_ADDR;
						else
						   reconos_state <= GET_CMD;
						end if;
					end if;
				-- get global and local addresses (NoC) (used by Encoder/Decoder)
				when GET_ADDR =>
					osif_mbox_get(i_osif, o_osif, MBOX_RECV, reconos_data, done);
					if done then
						global_addr <= reconos_data(5 downto 2);
						local_addr  <= reconos_data(1 downto 0);
						reconos_state <= GET_CMD;
					end if;
				-- give back FB id
				when PUT_FB_ID =>
					osif_mbox_put(i_osif, o_osif, MBOX_SEND, X"000000"&PR_MODULE, ignore, done);
					if done then reconos_state <= GET_CMD; end if;
			   -- thread exit
				when THREAD_EXIT =>
					osif_thread_exit(i_osif,o_osif);
			   -- get all parameters for FB
				when GET_ALL =>
					reconos_state <= GET_ALL_DPP_H;
				when GET_ALL_DPP_H =>
					osif_mbox_get(i_osif, o_osif, MBOX_RECV, reconos_data, done);
					if done then 
					  delay_per_packet(63 downto 32) <= reconos_data;
					  reconos_state <= GET_ALL_DPP_L; 
					end if;
				when GET_ALL_DPP_L =>
					osif_mbox_get(i_osif, o_osif, MBOX_RECV, reconos_data, done);
					if done then 
					  delay_per_packet(31 downto 0) <= reconos_data;
					  reconos_state <= GET_ALL_DPB_H; 
					end if;
			   when GET_ALL_DPB_H =>
					osif_mbox_get(i_osif, o_osif, MBOX_RECV, reconos_data, done);
					if done then 
					  delay_per_byte(63 downto 32) <= reconos_data;
					  reconos_state <= GET_ALL_DPB_L; 
					end if;
				when GET_ALL_DPB_L =>
					osif_mbox_get(i_osif, o_osif, MBOX_RECV, reconos_data, done);
					if done then 
					  delay_per_byte(31 downto 0) <= reconos_data;
					  reconos_state <= GET_ALL_DT; 
					end if;
				when GET_ALL_DT =>
					osif_mbox_get(i_osif, o_osif, MBOX_RECV, reconos_data, done);
					if done then 
					  drop_type <= reconos_data;
					  if unsigned(reconos_data) = 0 then -- no packet will be dropped -> no further parameters necessary -> initialized
					    reconos_state <= GET_CMD;
						 initialized <= '1';
					  elsif unsigned(reconos_data) = 1 then -- every x-th packet will be dropped -> no random generator necessary
					    reconos_state <= GET_ALL_DV;
					  elsif unsigned(reconos_data) = 2 then -- random packets will be dropped -> random generator necessary
					    reconos_state <= GET_ALL_SEED;  -- set seed only if random generator is used
					  else
					    -- invalid option -> not initialized
					    reconos_state <= GET_CMD;
					  end if;
					end if;
				when GET_ALL_SEED =>
					osif_mbox_get(i_osif, o_osif, MBOX_RECV, reconos_data, done);
					if done then 
					  randgen_seed <= reconos_data;
					  reconos_state <= GET_ALL_DV; 
					end if;
				when GET_ALL_DV =>
					osif_mbox_get(i_osif, o_osif, MBOX_RECV, reconos_data, done);
					if done then 
					  drop_value(31 downto 0) <= reconos_data;
					  initialized <= '1';
					  reconos_state <= GET_CMD; 
					end if;
				-- give back stats
				when PUT_STATS_ARRIVED =>
					osif_mbox_put(i_osif, o_osif, MBOX_SEND, packets_arrived, ignore, done);
					if done then reconos_state <= PUT_STATS_FORWARDED; end if;
				when PUT_STATS_FORWARDED =>
					osif_mbox_put(i_osif, o_osif, MBOX_SEND, packets_forwarded, ignore, done);
					if done then reconos_state <= GET_CMD; end if;
				-- give back parameters
				when PUT_DPP_H =>
					osif_mbox_put(i_osif, o_osif, MBOX_SEND, delay_per_packet(63 downto 32), ignore, done);
					if done then reconos_state <= PUT_DPP_L; end if;
				when PUT_DPP_L =>
					osif_mbox_put(i_osif, o_osif, MBOX_SEND, delay_per_packet(31 downto 0), ignore, done);
					if done then reconos_state <= GET_CMD; end if;
				when PUT_DPB_H =>
					osif_mbox_put(i_osif, o_osif, MBOX_SEND, delay_per_byte(63 downto 32), ignore, done);
					if done then reconos_state <= PUT_DPB_L; end if;
				when PUT_DPB_L =>
					osif_mbox_put(i_osif, o_osif, MBOX_SEND, delay_per_byte(31 downto 0), ignore, done);
					if done then reconos_state <= GET_CMD; end if;
				when PUT_DT =>
					osif_mbox_put(i_osif, o_osif, MBOX_SEND, drop_type, ignore, done);
					if done then reconos_state <= GET_CMD; end if;
				when PUT_DV =>
					osif_mbox_put(i_osif, o_osif, MBOX_SEND, drop_value, ignore, done);
					if done then reconos_state <= GET_CMD; end if;
				when PUT_SEED =>
					osif_mbox_put(i_osif, o_osif, MBOX_SEND, randgen_seed, ignore, done);
					if done then reconos_state <= GET_CMD; end if;
				when PUT_ADDR =>
					osif_mbox_put(i_osif, o_osif, MBOX_SEND, X"000000" & "00" & global_addr & local_addr, ignore, done);
					if done then reconos_state <= GET_CMD; end if;
			end case;
		end if;
	end process;
	
end architecture;
