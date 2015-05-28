--                                                        ____  _____
--                            ________  _________  ____  / __ \/ ___/
--                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
--                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
--                         /_/   \___/\___/\____/_/ /_/\____//____/
-- 
-- ======================================================================
--
--   title:        VHDL Package - ReconOS
--
--   project:      ReconOS
--   author:       Enno Lübbers, University of Paderborn
--                 Andreas Agne, University of Paderborn
--                 Christoph Rüthing, University of Paderborn
--   description:  The ReconOS package with hardware OS services in VHDL.
--
-- ======================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_defs.all;

package reconos_pkg is

	-- == Type definitions ================================================

	--
	-- Type definitions of i_osif_t and o_osif_t
	--
	--   sw2hw_/hw2sw_ - fifo signals
	--
	--   step  - internal state of the osif
	--   void  - void bit free to use
	--
	type i_osif_t is record
		sw2hw_data   : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		sw2hw_empty  : std_logic;
		hw2sw_full   : std_logic;

		step         : integer range 0 to 15;
		void         : std_logic;

		run_id       : std_logic_vector(C_OSIF_ID_WIDTH - 1 downto 0);
	end record;

	type o_osif_t is record
		sw2hw_re   : std_logic;
		hw2sw_data : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		hw2sw_we   : std_logic;

		step       : integer range 0 to 15;
		void       : std_logic;

		run_id     : std_logic_vector(C_OSIF_ID_WIDTH - 1 downto 0);
	end record;

	--
	-- Type definitions of i_memif_t and o_memif_t
	--
	--   mem2hwt_/hwt2mem_ - fifo signals
	--
	--   step  - internal state of the osif
	--
	type i_memif_t is record
		mem2hwt_data   : std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		mem2hwt_empty  : std_logic;
		hwt2mem_full   : std_logic;

		step           : integer range 0 to 15;
	end record;

	type o_memif_t is record
		mem2hwt_re   : std_logic;
		hwt2mem_data : std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		hwt2mem_we   : std_logic;

		step         : integer range 0 to 15;
	end record;
	
	--
	-- Type definitions of i_ram_t and o_ram_t
	--
	--   ram_  - ram signals
	--
	--   count    - byte counter of written bytes
	--   mem_addr - remote address of main memory
	--
	type i_ram_t is record
		ram_addr : unsigned(31 downto 0);
		ram_data : std_logic_vector(31 downto 0);

		remm     : unsigned(31 downto 0);
		mem_addr : unsigned(31 downto 0);
	end record;
	
	type o_ram_t is record
		ram_addr : unsigned(31 downto 0);
		ram_data : std_logic_vector(31 downto 0);
		ram_we   : std_logic;

		remm     : unsigned(31 downto 0);
		mem_addr : unsigned(31 downto 0);
	end record;


	-- == Reconos functions ===============================================

	--
	-- Assigns signals to the osif record. This function must be called
	-- asynchronously in the main entity including the os-fsm.
	--
	--   i_osif       - i_osif_t record
	--   o_osif       - o_osif_t_record
	--   sw2hw_data   - OSIF_FIFO_Sw2Hw_Data
	--   sw2hw_empty  - OSIF_FIFO_Sw2Hw_Empty
	--   sw2hw_re     - OSIF_FIFO_Sw2Hw_RE
	--   hw2sw_data   - OSIF_FIFO_Hw2Sw_Data
	--   hw2sw_full   - OSIF_FIFO_Hw2Sw_Full
	--   hw2sw_we     - OSIF_FIFO_Hw2Sw_WE
	--
	procedure osif_setup (
		signal i_osif       : out i_osif_t;
		signal o_osif       : in  o_osif_t;
		signal sw2hw_data   : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal sw2hw_empty  : in  std_logic;
		signal sw2hw_re     : out std_logic;
		signal hw2sw_data   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal hw2sw_full   : in  std_logic;
		signal hw2sw_we     : out std_logic
	);

	--	
	-- Resets the osif signals to a default state. This function should be called
	-- on reset of the os-fsm.
	--
	--   o_osif - o_osif_t record
	--
	procedure osif_reset (
		signal o_osif : out o_osif_t
	);

	--
	-- Assigns signals to the memif record. This function must be called
	-- asynchronously in the main entity including the os-fsm.
	--
	--   i_memif        - i_memif_t record
	--   o_memif        - o_memif_t record
	--   mem2hwt_data   - MEMIF_FIFO_Mem2Hwt_Data
	--   mem2hwt_empty  - MEMIF_FIFO_Mem2Hwt_Empty
	--   mem2hwt_re     - MEMIF_FIFO_Mem2Hwt_RE
	--   hwt2mem_data   - MEMIF_FIFO_Hwt2Mem_Data
	--   hwt2mem_full   - MEMIF_FIFO_Hwt2Mem_Full
	--   hwt2mem_we     - MEMIF_FIFO_Hwt2Mem_WE
	--
	procedure memif_setup (
		signal i_memif        : out i_memif_t;
		signal o_memif        : in  o_memif_t;
		signal mem2hwt_data   : in  std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		signal mem2hwt_empty  : in  std_logic;
		signal mem2hwt_re     : out std_logic;
		signal hwt2mem_data   : out std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		signal hwt2mem_full   : in  std_logic;
		signal hwt2mem_we     : out std_logic
	);

	--
	-- Resets the memif signals to a default state. This function should be called
	-- on reset of the os-fsm.
	--
	--   o_memif - o_memif_t record
	--
	procedure memif_reset (
		signal o_memif : out o_memif_t
	);

	--
	-- Assigns signals to the memif record. This function must be called
	-- asynchronously in the main entity including the os-fsm.
	--
	--   i_ram      - i_ram_t record
	--   o_ram      - o_ram_t record
	--   ram_addr   - address signal of the local ram
	--   ram_i_data - input data signal of the local ram
	--   ram_o_data - output data signal of the local ram
	--   ram_we     - write enable signal of the local ram
	--
	procedure ram_setup (
		signal i_ram      : out i_ram_t;
		signal o_ram      : in  o_ram_t;
		signal ram_addr   : out std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		signal ram_i_data : out std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		signal ram_o_data : in  std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		signal ram_we     : out std_logic
	);

	--
	-- Resets the RAM signals to a default state. This function should be called
	-- on reset of the os-fsm.
	--
	--   o_ram - o_ram_t record
	--
	procedure ram_reset (
		signal o_ram  : out o_ram_t
	);

end package reconos_pkg;

package body reconos_pkg is

	--
	-- @see header
	--
	procedure osif_setup (
		signal i_osif       : out i_osif_t;
		signal o_osif       : in  o_osif_t;
		signal sw2hw_data   : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal sw2hw_empty  : in  std_logic;
		signal sw2hw_re     : out std_logic;
		signal hw2sw_data   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal hw2sw_full   : in  std_logic;
		signal hw2sw_we     : out std_logic
	) is begin
		i_osif.sw2hw_data  <= sw2hw_data;
		i_osif.sw2hw_empty <= sw2hw_empty;
		sw2hw_re           <= o_osif.sw2hw_re;

		hw2sw_data         <= o_osif.hw2sw_data;
		i_osif.hw2sw_full  <= hw2sw_full;
		hw2sw_we           <= o_osif.hw2sw_we;

		i_osif.step        <= o_osif.step;
		i_osif.void        <= o_osif.void;

		i_osif.run_id      <= o_osif.run_id;
	end procedure osif_setup;

	--
	-- @see header
	--
	procedure osif_reset (
		signal o_osif  : out o_osif_t
	) is begin
		o_osif.sw2hw_re   <= '0';
		o_osif.hw2sw_data <= (others => '0');
		o_osif.hw2sw_we   <= '0';

		o_osif.step       <= 0;
		o_osif.void       <= '0';

		o_osif.run_id     <= (others => '0');
	end procedure osif_reset;

	--
	-- @see header
	--
	procedure memif_setup (
		signal i_memif        : out i_memif_t;
		signal o_memif        : in  o_memif_t;
		signal mem2hwt_data   : in  std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		signal mem2hwt_empty  : in  std_logic;
		signal mem2hwt_re     : out std_logic;
		signal hwt2mem_data   : out std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		signal hwt2mem_full   : in  std_logic;
		signal hwt2mem_we     : out std_logic
	) is begin
		i_memif.mem2hwt_data  <= mem2hwt_data;
		i_memif.mem2hwt_empty <= mem2hwt_empty;
		mem2hwt_re            <= o_memif.mem2hwt_re;

		hwt2mem_data          <= o_memif.hwt2mem_data;
		i_memif.hwt2mem_full  <= hwt2mem_full;
		hwt2mem_we            <= o_memif.hwt2mem_we;

		i_memif.step          <= o_memif.step;
	end procedure memif_setup;

	--
	-- @see header
	--
	procedure memif_reset (
		signal o_memif  : out o_memif_t
	) is begin
		o_memif.mem2hwt_re   <= '0';
		o_memif.hwt2mem_data <= (others => '0');
		o_memif.hwt2mem_we   <= '0';

		o_memif.step         <= 0;
	end procedure memif_reset;

	--
	-- @see header
	--
	procedure ram_setup (
		signal i_ram      : out i_ram_t;
		signal o_ram      : in  o_ram_t;
		signal ram_addr   : out std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		signal ram_i_data : out std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		signal ram_o_data : in  std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		signal ram_we     : out std_logic
	) is begin
		ram_addr       <= std_logic_vector(o_ram.ram_addr);
		i_ram.ram_addr <= o_ram.ram_addr;
		ram_i_data     <= o_ram.ram_data;
		i_ram.ram_data <= ram_o_data;
		ram_we         <= o_ram.ram_we;

		i_ram.remm     <= o_ram.remm;
		i_ram.mem_addr <= o_ram.mem_addr;
	end procedure ram_setup;

	--
	-- @see header
	--
	procedure ram_reset (
		signal o_ram  : out o_ram_t
	) is begin
		o_ram.ram_addr <= (others => '0');
		o_ram.ram_data <= (others => '0');
		o_ram.ram_we   <= '0';
		o_ram.remm     <= (others => '0');
		o_ram.mem_addr <= (others => '0');
	end procedure ram_reset;
	
end package body reconos_pkg;
