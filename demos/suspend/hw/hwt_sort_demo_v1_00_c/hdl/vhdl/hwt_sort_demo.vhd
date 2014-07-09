library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

library proc_common_v3_00_a;
use proc_common_v3_00_a.proc_common_pkg.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;

entity hwt_sort_demo is
	port (
		-- OSIF FIFO ports
		OSIF_FIFO_Sw2Hw_Data    : in  std_logic_vector(31 downto 0);
		OSIF_FIFO_Sw2Hw_Fill    : in  std_logic_vector(15 downto 0);
		OSIF_FIFO_Sw2Hw_Empty   : in  std_logic;
		OSIF_FIFO_Sw2Hw_RE      : out std_logic;

		OSIF_FIFO_Hw2Sw_Data    : out std_logic_vector(31 downto 0);
		OSIF_FIFO_Hw2Sw_Rem     : in  std_logic_vector(15 downto 0);
		OSIF_FIFO_Hw2Sw_Full    : in  std_logic;
		OSIF_FIFO_Hw2Sw_WE      : out std_logic;

		-- MEMIF FIFO ports
		MEMIF_FIFO_Hwt2Mem_Data    : out std_logic_vector(31 downto 0);
		MEMIF_FIFO_Hwt2Mem_Rem     : in  std_logic_vector(15 downto 0);
		MEMIF_FIFO_Hwt2Mem_Full    : in  std_logic;
		MEMIF_FIFO_Hwt2Mem_WE      : out std_logic;

		MEMIF_FIFO_Mem2Hwt_Data    : in  std_logic_vector(31 downto 0);
		MEMIF_FIFO_Mem2Hwt_Fill    : in  std_logic_vector(15 downto 0);
		MEMIF_FIFO_Mem2Hwt_Empty   : in  std_logic;
		MEMIF_FIFO_Mem2Hwt_RE      : out std_logic;

		HWT_Clk     : in  std_logic;
		HWT_Rst     : in  std_logic;
		HWT_Signal  : in  std_logic
	);
end entity hwt_sort_demo;

architecture implementation of hwt_sort_demo is
	--
	-- Definition of signals for simpler user.
	--
	--   clk - clock signal (HWT_Clk)
	--   rst - asynchronous reset signal (HWT_Rst)
	--   sig - interrupt signal for suspending (HWT_Signal)
	--
	signal clk : std_logic;
	signal rst : std_logic;
	signal sig : std_logic;
	
	--
	-- Definition of ReconOS signals.
	--
	--   i/o_osif  - signals from OSIF
	--   i/o_memif - signals from MEMIF
	--   i/o_ram   - signals from local RAM
	--
	signal i_osif  : i_osif_t;
	signal o_osif  : o_osif_t;
	signal i_memif : i_memif_t;
	signal o_memif : o_memif_t;
	signal i_ram   : i_ram_t;
	signal o_ram   : o_ram_t;

	--
	-- Definition of resources.
	--
	--   mbox_recv - mbox to receive data
	--   mbox_send - mbox to send acknowledgments
	--
	constant MBOX_RECV : std_logic_vector(31 downto 0) := x"00000000";
	constant MBOX_SEND : std_logic_vector(31 downto 0) := x"00000001";

	--
	-- Definition of the local RAM. The local RAM will be implemented as
	-- a block-RAM. Therefore, take care to apply proper timing.
	--
	--   c_local_ram_size_words    - size of the local RAM in 32-bit words
	--   c_local_ram_size_bytes    - size of the local RAM in bytes
	--
	--   ram_t - type for the RAM
	--
	--   ram           - the local RAM itself
	--   ram_addr_0/1  - address signal
	--   ram_idata_0/1 - input data (for writing to RAM)
	--   ram_odata_0/1 - output data (for reading from RAM)
	--   ram_we_0/1    - write enable
	--
	constant C_RAM_SIZE_WORDS    : integer := 2048;
	constant C_RAM_SIZE_BYTES    : integer := 4 * C_RAM_SIZE_WORDS;

	type RAM_T is array (0 to C_RAM_SIZE_WORDS - 1) of std_logic_vector(31 downto 0);

	shared variable ram : RAM_T;
	signal ram_addr_0, ram_addr_1   : std_logic_vector(31 downto 0);
	signal ram_idata_0, ram_idata_1 : std_logic_vector(31 downto 0);
	signal ram_odata_0, ram_odata_1 : std_logic_vector(31 downto 0);
	signal ram_we_0, ram_we_1       : std_logic;

	--
	-- Definition of the states for the ReconOS-FSM.
	--
	--   state_type - type for the different states
	--
	--   state      - the state itself
	--
	type STATE_TYPE is (STATE_GET_ADDR, STATE_READ_DATA, STATE_WRITE_DATA, STATE_PUT_ACK,
	                    STATE_SORT, STATE_SORT_START_OVER, STATE_SORT_LOAD_A_WAIT, STATE_SORT_LOAD_A,
	                    STATE_SORT_LOAD_B_WAIT, STATE_SORT_LOAD_B, STATE_SORT_COMPARE, STATE_SORT_LOOP,
	                    STATE_SUSPEND, STATE_SUSPEND_STATE, STATE_SUSPEND_ADDR, STATE_SUSPEND_PTR_MAX, STATE_SUSPEND_SWAPPED, STATE_SUSPEND_DATA,
	                    STATE_RESUME, STATE_RESUME_STATE, STATE_RESUME_ADDR, STATE_RESUME_PTR_MAX, STATE_RESUME_SWAPPED, STATE_RESUME_DATA,
	                    STATE_INIT, STATE_EXIT);

	signal state : STATE_TYPE;

	--
	-- Signals used by the Sort-Demo.
	--
	signal ret          : std_logic_vector(31 downto 0);
	signal state_addr   : std_logic_vector(31 downto 0);
	signal state_tmp    : STATE_TYPE;

	signal addr         : std_logic_vector(31 downto 0);
	signal ptr, ptr_max : std_logic_vector(31 downto 0);
	signal a, b         : std_logic_vector(31 downto 0);
	signal swapped      : std_logic;

	constant SORT_LEN_BYTES : std_logic_vector(31 downto 0) := x"00002000";
	constant SORT_LEN_WORDS : std_logic_vector(31 downto 0) := x"00000800";
begin
	clk <= HWT_Clk;
	rst <= HWT_Rst;
	sig <= HWT_Signal;

	--
	-- Description of local RAM. This pattern will be recognized by XST to
	-- implement a block-RAM.
	--
	--   @see Xilinx XST User Guide (UG687)
	--
	local_ram_0 : process (clk) is
	begin
		if (rising_edge(clk)) then
			if (ram_we_0 = '1') then
				ram(CONV_INTEGER(ram_addr_0)) := ram_idata_0;
			else
				ram_odata_0 <= ram(CONV_INTEGER(ram_addr_0));
			end if;
		end if;
	end process local_ram_0;

	local_ram_1 : process (clk) is
	begin
		if (rising_edge(clk)) then
			if (ram_we_1 = '1') then
				ram(CONV_INTEGER(ram_addr_1)) := ram_idata_1;
			else
				ram_odata_1 <= ram(CONV_INTEGER(ram_addr_1));
			end if;
		end if;
	end process local_ram_1;
	
	--
	-- ReconOS setup functions assigning records.
	--
	osif_setup (
		i_osif,
		o_osif,
		OSIF_FIFO_Sw2Hw_Data,
		OSIF_FIFO_Sw2Hw_Fill,
		OSIF_FIFO_Sw2Hw_Empty,
		OSIF_FIFO_Hw2Sw_Rem,
		OSIF_FIFO_Hw2Sw_Full,
		OSIF_FIFO_Sw2Hw_RE,
		OSIF_FIFO_Hw2Sw_Data,
		OSIF_FIFO_Hw2Sw_WE
	);

	memif_setup (
		i_memif,
		o_memif,
		MEMIF_FIFO_Mem2Hwt_Data,
		MEMIF_FIFO_Mem2Hwt_Fill,
		MEMIF_FIFO_Mem2Hwt_Empty,
		MEMIF_FIFO_Hwt2Mem_Rem,
		MEMIF_FIFO_Hwt2Mem_Full,
		MEMIF_FIFO_Mem2Hwt_RE,
		MEMIF_FIFO_Hwt2Mem_Data,
		MEMIF_FIFO_Hwt2Mem_WE
	);

	ram_setup (
		i_ram,
		o_ram,
		ram_addr_0,
		ram_we_0,
		ram_idata_0,
		ram_odata_0
	);

	--
	-- ReconOS-FSM doing all synchronization with other threads by using
	-- the ReconOS-package functions.
	--
	reconos_fsm: process (clk,rst,o_osif,o_memif,o_ram) is
		variable done  : boolean;
	begin
		if rst = '1' then
			osif_reset(o_osif);
			memif_reset(o_memif);
			ram_reset(o_ram);

			state <= STATE_INIT;
		elsif rising_edge(clk) then
			ram_we_1 <= '0';

			case state is
				--
				-- Starting end exiting the hardware thread
				--
				--   init - deciding whether to resume or not
				--   exit - exit thread
				--
				when STATE_INIT =>
					osif_read(i_osif, o_osif, ret, done);
					if done then
						if ret = OSIF_SIGNAL_THREAD_START then
							state <= STATE_GET_ADDR;
						elsif ret = OSIF_SIGNAL_THREAD_RESUME then
							state <= STATE_RESUME;
						end if;
					end if;

				when STATE_EXIT =>
					osif_thread_exit(i_osif, o_osif);

				--
				-- Normal execution of Sort-Demo
				--
				--   get_addr   - getting address of unsorted data
				--   read_data  - reading unsorted data from system-memory
				--   write_data - writing sorted data back to system-memory
				--   put_ack    - acknowledging successful sorting
				--
				when STATE_GET_ADDR =>
					if sig = '1' then
						osif_reset(o_osif);
						memif_reset(o_memif);
						state_tmp <= STATE_GET_ADDR;
						state <= STATE_SUSPEND;
					else
						osif_mbox_get(i_osif, o_osif, MBOX_RECV, addr, done);
						if done then
							state <= STATE_READ_DATA;
						end if;
					end if;

				when STATE_READ_DATA =>
					memif_read(i_ram, o_ram, i_memif, o_memif, addr, x"00000000", SORT_LEN_BYTES, done);
					if done then
						state <= STATE_SORT;
					end if;

				when STATE_SORT =>
					ptr_max <= SORT_LEN_WORDS - 1;
					swapped <= '1';
					state <= STATE_SORT_START_OVER;

				when STATE_SORT_START_OVER =>
					if sig = '1' then
						osif_reset(o_osif);
						memif_reset(o_memif);
						state_tmp <= STATE_SORT_START_OVER;
						state <= STATE_SUSPEND;
					else
						if swapped = '0' or ptr_max = x"00000000" then
							state <= STATE_WRITE_DATA;
						else
							ptr <= x"00000000";
							ptr_max <= ptr_max - 1;
							swapped <= '0';

							ram_addr_1 <= x"00000000";
							state <= STATE_SORT_LOAD_A_WAIT;
						end if;
					end if;

				when STATE_SORT_LOAD_A_WAIT =>
					ram_addr_1 <= ptr + 1;
					state <= STATE_SORT_LOAD_A;

				when STATE_SORT_LOAD_A =>
					a <= ram_odata_1;
					state <= STATE_SORT_LOAD_B;

				when STATE_SORT_LOAD_B_WAIT =>
					state <= STATE_SORT_LOAD_B;

				when STATE_SORT_LOAD_B =>
					b <= ram_odata_1;
					state <= STATE_SORT_COMPARE;

				when STATE_SORT_COMPARE =>
					if a > b then
						swapped <= '1';
						a <= b;
						b <= a;

						ram_addr_1 <= ptr;
						ram_we_1 <= '1';
						ram_idata_1 <= b;
					end if;

					state <= STATE_SORT_LOOP;

				when STATE_SORT_LOOP =>
					if ptr = ptr_max then
						ram_addr_1 <= ptr + 1;
						ram_we_1 <= '1';
						ram_idata_1 <= b;

						state <= STATE_SORT_START_OVER;
					else
						ptr <= ptr + 1;
						a <= b;

						ram_addr_1 <= ptr + 2;
						state <= STATE_SORT_LOAD_B_WAIT;
					end if;

				when STATE_WRITE_DATA =>
					memif_write(i_ram, o_ram, i_memif, o_memif, x"00000000", addr, SORT_LEN_BYTES, done);
					if done then
						state <= STATE_PUT_ACK;
					end if;

				when STATE_PUT_ACK =>
					if sig = '1' then
						osif_reset(o_osif);
						memif_reset(o_memif);
						state_tmp <= STATE_PUT_ACK;
						state <= STATE_SUSPEND;
					else
						osif_mbox_put(i_osif, o_osif, MBOX_SEND, x"00000000", ret, done);
						if done then
							state <= STATE_GET_ADDR;
						end if;
					end if;

				-- 
				-- Suspending the hardware thread and saving its state
				--
				--   suspend      - getting address for state data
				--   suspend_addr - writing addres of data (addr)
				--   suspend_data - writing data from local RAM
				--
				when STATE_SUSPEND =>
					osif_call_0(i_osif, o_osif, OSIF_CMD_THREAD_GET_STATE_ADDR, state_addr, done);
					if done then
						state <= STATE_SUSPEND_STATE;
					end if;

				when STATE_SUSPEND_STATE =>
					case state_tmp is
						when STATE_READ_DATA =>
							memif_write_word(i_memif, o_memif, state_addr, x"00000001", done);
						when STATE_SORT_START_OVER =>
							memif_write_word(i_memif, o_memif, state_addr, x"00000002", done);
						when STATE_PUT_ACK =>
							memif_write_word(i_memif, o_memif, state_addr, x"00000003", done);
						when others =>
					end case;

					if done then
						state_addr <= state_addr + x"00000004";
						state <= STATE_SUSPEND_ADDR;
					end if;

				when STATE_SUSPEND_ADDR =>
					memif_write_word(i_memif, o_memif, state_addr, addr, done);
					if done then
						state_addr <= state_addr + x"00000004";
						state <= STATE_SUSPEND_PTR_MAX;
					end if;

				when STATE_SUSPEND_PTR_MAX =>
					memif_write_word(i_memif, o_memif, state_addr, ptr_max, done);
					if done then
						state_addr <= state_addr + x"00000004";
						state <= STATE_SUSPEND_SWAPPED;
					end if;

				when STATE_SUSPEND_SWAPPED =>
					memif_write_word(i_memif, o_memif, state_addr, x"0000000" & b"000" & swapped, done);
					if done then
						state_addr <= state_addr + x"00000004";
						state <= STATE_SUSPEND_DATA;
					end if;

				when STATE_SUSPEND_DATA =>
					memif_write(i_ram, o_ram, i_memif, o_memif, x"00000000", state_addr, SORT_LEN_BYTES, done);
					if done then
						state <= STATE_EXIT;
					end if;

				-- 
				-- Resuming the hardware thread and restoring its state
				--
				--   resume      - getting address for state data
				--   resume_addr - reading addres of data (addr)
				--   resume_data - reading data from local RAM
				--
				when STATE_RESUME =>
					osif_call_0(i_osif, o_osif, OSIF_CMD_THREAD_GET_STATE_ADDR, state_addr, done);
					if done then
						state <= STATE_RESUME_STATE;
					end if;

				when STATE_RESUME_STATE =>
					memif_read_word(i_memif, o_memif, state_addr, ret, done);
					if done then
						case ret is
							when x"00000001" =>
								state_tmp <= STATE_READ_DATA;
							when x"00000002" =>
								state_tmp <= STATE_SORT_START_OVER;
							when x"00000003" =>
								state_tmp <= STATE_PUT_ACK;
							when others =>
						end case;

						state_addr <= state_addr + x"00000004";
						state <= STATE_RESUME_ADDR;
					end if;


				when STATE_RESUME_ADDR =>
					memif_read_word(i_memif, o_memif, state_addr, addr, done);
					if done then
						state_addr <= state_addr + x"00000004";
						state <= STATE_RESUME_PTR_MAX;
					end if;

				when STATE_RESUME_PTR_MAX =>
					memif_read_word(i_memif, o_memif, state_addr, ptr_max, done);
					if done then
						state_addr <= state_addr + x"00000004";
						state <= STATE_RESUME_SWAPPED;
					end if;

				when STATE_RESUME_SWAPPED =>
					memif_read_word(i_memif, o_memif, state_addr, ret, done);
					if done then
						swapped <= ret(0);
						state_addr <= state_addr + x"00000004";
						state <= STATE_RESUME_DATA;
					end if;

				when STATE_RESUME_DATA =>
					memif_read(i_ram, o_ram, i_memif, o_memif, state_addr, x"00000000", SORT_LEN_BYTES, done);
					if done then
						state <= state_tmp;
					end if;
			end case;
		end if;
	end process;
	
end architecture;