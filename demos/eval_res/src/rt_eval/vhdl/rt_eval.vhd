library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;
use reconos_v3_01_a.reconos_calls.all;

library rt_eval_v1_00_a;
use rt_eval_v1_00_a.reconos_thread.all;

entity rt_eval is
	port (
		-- OSIF FIFO ports
		OSIF_Sw2Hw_Data    : in  std_logic_vector(31 downto 0);
		OSIF_Sw2Hw_Empty   : in  std_logic;
		OSIF_Sw2Hw_RE      : out std_logic;

		OSIF_Hw2Sw_Data    : out std_logic_vector(31 downto 0);
		OSIF_Hw2Sw_Full    : in  std_logic;
		OSIF_Hw2Sw_WE      : out std_logic;

		-- MEMIF FIFO ports
		MEMIF_Hwt2Mem_Data    : out std_logic_vector(31 downto 0);
		MEMIF_Hwt2Mem_Full    : in  std_logic;
		MEMIF_Hwt2Mem_WE      : out std_logic;

		MEMIF_Mem2Hwt_Data    : in  std_logic_vector(31 downto 0);
		MEMIF_Mem2Hwt_Empty   : in  std_logic;
		MEMIF_Mem2Hwt_RE      : out std_logic;

		HWT_Clk    : in  std_logic;
		HWT_Rst    : in  std_logic;
		HWT_Signal : in  std_logic;
		
		DEBUG : out std_logic_vector(110 downto 0);
		TIMER : in  std_logic_vector(31 downto 0)
	);
end entity rt_eval;

architecture implementation of rt_eval is
	constant C_RAM_LEN_WORDS   : integer := 128;
	constant C_RAM_LEN_BYTES   : integer := C_RAM_LEN_WORDS * 4;
	constant C_RAM_LEN_BYTES_V : std_logic_vector(31 downto 0) := std_logic_vector(to_unsigned(C_RAM_LEN_BYTES, 32));
	type RAM_T is array (0 to C_RAM_LEN_WORDS - 1) of std_logic_vector(31 downto 0);

	signal ram : RAM_T;
	signal ram_addr  : std_logic_vector(31 downto 0);
	signal ram_idata : std_logic_vector(31 downto 0);
	signal ram_odata : std_logic_vector(31 downto 0);
	signal ram_we    : std_logic;

	signal i_osif  : i_osif_t;
	signal o_osif  : o_osif_t;
	signal i_memif : i_memif_t;
	signal o_memif : o_memif_t;
	signal i_ram   : i_ram_t;
	signal o_ram   : o_ram_t;

	type STATE_TYPE is (STATE_THREAD_INIT,
	                    STATE_CMD,STATE_EXEC,STATE_ACK);
	signal state : STATE_TYPE;

	signal cmd, ts : std_logic_vector(31 downto 0);
	signal ignore  : std_logic_vector(31 downto 0);
begin
	DEBUG <= (others => '0');

	ram_ctrl : process (HWT_Clk) is
	begin
		if rising_edge(HWT_Clk) then
			if ram_we = '1' then
				ram(to_integer(unsigned(ram_addr))) <= ram_idata;
			else
				ram_odata <= ram(to_integer(unsigned(ram_addr)));
			end if;
		end if;
	end process ram_ctrl;

	osif_setup (
		i_osif,
		o_osif,
		OSIF_Sw2Hw_Data,
		OSIF_Sw2Hw_Empty,
		OSIF_Sw2Hw_RE,
		OSIF_Hw2Sw_Data,
		OSIF_Hw2Sw_Full,
		OSIF_Hw2Sw_WE
	);

	memif_setup (
		i_memif,
		o_memif,
		MEMIF_Mem2Hwt_Data,
		MEMIF_Mem2Hwt_Empty,
		MEMIF_Mem2Hwt_RE,
		MEMIF_Hwt2Mem_Data,
		MEMIF_Hwt2Mem_Full,
		MEMIF_Hwt2Mem_WE
	);

	ram_setup (
		i_ram,
		o_ram,
		ram_addr,
		ram_idata,
		ram_odata,
		ram_we
	);
	
	reconos_fsm: process (HWT_Clk,HWT_Rst,o_osif,o_memif) is
		variable resume, done : boolean;
	begin
		if HWT_Rst = '1' then
			osif_reset(o_osif);
			memif_reset(o_memif);
			ram_reset(o_ram);
			state <= STATE_THREAD_INIT;
		elsif rising_edge(HWT_Clk) then
			case state is
				when STATE_THREAD_INIT =>
					THREAD_INIT(i_osif, o_osif, resume, done);
					if done then
						state <= STATE_CMD;
					end if;

				when STATE_CMD =>
					MBOX_GET(i_osif, o_osif, eval_cmdhw, cmd, done);
					if done then
							ts <= TIMER;
							state <= STATE_EXEC;
					end if;
				
				when STATE_EXEC =>
					case cmd is
						when x"00000010" =>
							MBOX_GET(i_osif, o_osif, eval_mboxsw, ignore, done);
							if done then
								ts <= TIMER;
							end if;

						when x"00000011" =>
							MBOX_GET(i_osif, o_osif, eval_mboxhw, ignore, done);
							if done then
								ts <= TIMER;
							end if;

						when x"00000020" =>
							MBOX_PUT(i_osif, o_osif, eval_mboxsw, x"AFFEDEAD", ignore, done);

						when x"00000021" =>
							MBOX_PUT(i_osif, o_osif, eval_mboxhw, x"AFFEDEAD", ignore, done);

						when x"00000030" =>
							PIPE_READ(i_osif, o_osif, i_ram, o_ram, eval_pipesw, x"00000200", ignore, done);
							if done then
								ts <= TIMER;
							end if;

						when x"00000031" =>
							PIPE_READ(i_osif, o_osif, i_ram, o_ram, eval_pipehw, x"00000200", ignore, done);
							if done then
								ts <= TIMER;
							end if;

						when x"00000040" =>
							PIPE_WRITE(i_osif, o_osif, i_ram, o_ram, eval_pipesw, x"00000200", ignore, done);

						when x"00000041" =>
							PIPE_WRITE(i_osif, o_osif, i_ram, o_ram, eval_pipehw, x"00000200", ignore, done);

						when others =>
					end case;

					if done then
						state <= STATE_ACK;
					end if;

				when STATE_ACK =>
					MBOX_PUT(i_osif, o_osif, eval_time, ts, ignore, done);
					if done then
						state  <= STATE_CMD;
					end if;
			end case;
		end if;
	end process;
	
end architecture;
