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
--   description:  Global header file with constant definitions.
--
-- ======================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package reconos_defs is

	-- == Constant definitions ============================================

	--
	-- General constants
	--
	--   C_OSIF_DATA_WIDTH  - width of the osif
	--   C_MEMIF_DATA_WIDTH - width of the memif
	--
	--   C_OSIF_ADDR_WIDTH   - address width of osif fifo
	--   C_OSIF_ID_WIDTH     - width of addresses in command word
	--   C_OSIF_SRC_WIDTH    - width of source address in command word
	--   C_OSIF_DST_WIDTH    - width of destination address in command word
	--   C_OSIF_LENGTH_WIDTH - width of length in command word
	--
	--   C_MEMIF_ADDR_WIDTH   - address width of memif fifo
	--   C_MEMIF_CHUNK_WORDS  - size of one memory request in words
	--                          (a request might be split up to meet this)
	--   C_MEMIF_CHUNK_BYTES  - chunk size in bytes
	--   C_MEMIF_CHUNK_WIDTH  - width of chunk (log2 C_MEMIF_CHUNK_BYTES)
	--   C_MEMIF_LENGTH_WIDTH - width of the length in command word
	--   C_MEMIF_OP_WIDTH     - width of the operation in command word
	--
	constant C_OSIF_DATA_WIDTH  : integer := 32;
	constant C_MEMIF_DATA_WIDTH : integer := 32;

	constant C_OSIF_ADDR_WIDTH   : integer := 3;
	constant C_OSIF_ID_WIDTH     : integer := 8;
	constant C_OSIF_SRC_WIDTH    : integer := C_OSIF_ID_WIDTH;
	constant C_OSIF_DST_WIDTH    : integer := C_OSIF_ID_WIDTH;
	constant C_OSIF_LENGTH_WIDTH : integer := 16;

	constant C_MEMIF_ADDR_WIDTH   : integer := 7;
	constant C_MEMIF_CHUNK_WORDS  : integer := 64;
	constant C_MEMIF_CHUNK_BYTES  : integer := C_MEMIF_CHUNK_WORDS * 4;
	constant C_MEMIF_CHUNK_WIDTH  : integer := 8;
	constant C_MEMIF_OP_WIDTH     : integer := 8;
	constant C_MEMIF_LENGTH_WIDTH : integer := 24;

	--
	-- "Constants" for easier handling of ranges
	--
	--   C_OSIF_SRC_RANGE    - range of source address in command word
	--   C_OSIF_DST_RANGE    - range of destination address in command word
	--   C_OSIF_LENGTH_RANGE - range of length in command word
	--
	--   C_MEMIF_LENGTH_RANGE - range of the length in command word
	--   C_MEMIF_OP_RANGE     - range of the operation in command word
	--   C_MEMIF_CHUNK_RANGE  - range of chunk offset
	--
	subtype C_OSIF_SRC_RANGE is natural range C_OSIF_DATA_WIDTH - 1 downto C_OSIF_DATA_WIDTH - C_OSIF_SRC_WIDTH;
	subtype C_OSIF_DST_RANGE is natural range C_OSIF_DATA_WIDTH - C_OSIF_SRC_WIDTH - 1 downto C_OSIF_DATA_WIDTH - C_OSIF_SRC_WIDTH - C_OSIF_DST_WIDTH;
	subtype C_OSIF_LENGTH_RANGE is natural range C_OSIF_DATA_WIDTH - C_OSIF_SRC_WIDTH - C_OSIF_DST_WIDTH - 1 downto 0;

	subtype C_MEMIF_OP_RANGE is natural range C_MEMIF_DATA_WIDTH - 1 downto C_MEMIF_DATA_WIDTH - C_MEMIF_OP_WIDTH;
	subtype C_MEMIF_LENGTH_RANGE is natural range C_MEMIF_DATA_WIDTH - C_MEMIF_OP_WIDTH - 1 downto 0;
	subtype C_MEMIF_CHUNK_RANGE is natural range C_MEMIF_CHUNK_WIDTH - 1 downto 0;

	--
	-- Definition of osif commands
	--
	--   self-describing
	--
	constant OSIF_CMD_THREAD_GET_INIT_DATA  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000A0";
	constant OSIF_CMD_THREAD_GET_STATE_ADDR : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000A1";
	constant OSIF_CMD_THREAD_EXIT           : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000A2";
	constant OSIF_CMD_THREAD_YIELD          : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000A3";
	constant OSIF_CMD_THREAD_CLEAR_SIGNAL   : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000A4";
	constant OSIF_CMD_THREAD_GET_RUNID      : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000A5";
	constant OSIF_CMD_SEM_POST              : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000B0";
	constant OSIF_CMD_SEM_WAIT              : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000B1";
	constant OSIF_CMD_MUTEX_LOCK            : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000C0";
	constant OSIF_CMD_MUTEX_UNLOCK          : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000C1";
	constant OSIF_CMD_MUTEX_TRYLOCK         : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000C2";
	constant OSIF_CMD_COND_WAIT             : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000D0";
	constant OSIF_CMD_COND_SIGNAL           : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000D1";
	constant OSIF_CMD_COND_BROADCAST        : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000D2";
	constant OSIF_CMD_MBOX_GET              : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000F0";
	constant OSIF_CMD_MBOX_PUT              : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000F1";
	constant OSIF_CMD_MBOX_TRYGET           : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000F2";
	constant OSIF_CMD_MBOX_TRYPUT           : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000F3";
	constant OSIF_CMD_PIPE_WRITE            : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"00000090";
	constant OSIF_CMD_PIPE_READ             : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"00000091";
	constant OSIF_CMD_MASK                  : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000FF";
	constant OSIF_CMD_YIELD_MASK            : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"80000000";

	constant OSIF_SIGNAL_THREAD_START       : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"01000000";
	constant OSIF_SIGNAL_THREAD_RESUME      : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"01000001";

	constant OSIF_INTERRUPTED               : std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0) := x"000000FF";

	--
	-- Definition of memif commands
	--
	--   self-describing
	--
	constant MEMIF_CMD_READ  : std_logic_vector(C_MEMIF_OP_WIDTH - 1 downto 0) := x"00";
	constant MEMIF_CMD_WRITE : std_logic_vector(C_MEMIF_OP_WIDTH - 1 downto 0) := x"F0";

end package reconos_defs;

--package body reconos_defs is
--end package body reconos_defs;