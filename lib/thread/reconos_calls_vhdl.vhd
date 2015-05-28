--                                                        ____  _____
--                            ________  _________  ____  / __ \/ ___/
--                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
--                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
--                         /_/   \___/\___/\____/_/ /_/\____//____/
-- 
-- ======================================================================
--
--   title:        VHDL Calls - ReconOS
--
--   project:      ReconOS
--   author:       Enno Lübbers, University of Paderborn
--                 Andreas Agne, University of Paderborn
--                 Christoph Rüthing, University of Paderborn
--   description:  The ReconOS package with hardware OS services in VHDL.
--
-- ======================================================================

<<reconos_preproc>>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;
use reconos_v3_01_a.reconos_defs.all;

package reconos_calls is

	--
	-- Reads a single word from the osif.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   word   - word read from the osif
	--   done   - indicates when read finished
	--
	procedure OSIF_READ (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		signal word   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- Writes a single word into the osif
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   word   - word to write int the osif
	--   done   - indicates when write finished
	-- 
	procedure OSIF_WRITE (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		word          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- ONLY FOR INTERNAL USE
	--
	-- Issues a system call with no arguments and a no result.
	--
	--   i_osif  - i_osif_t record
	--   o_osif  - o_osif_t record
	--   dst     - destination of message
	--   call_id - id of the system call
	--   done    - indicates when system call finished
	--
	procedure osif_call_0_0 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- ONLY FOR INTERNAL USE
	--
	-- Issues a system call with no arguments and a single result.
	--
	--   i_osif  - i_osif_t record
	--   o_osif  - o_osif_t record
	--   dst     - destination of message
	--   call_id - id of the system call
	--   ret0    - result of the system call
	--   done    - indicates when system call finished
	--
	procedure osif_call_0_1 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret0   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- ONLY FOR INTERNAL USE
	--
	-- Issues a system call with no arguments and a single result.
	--
	--   i_osif  - i_osif_t record
	--   o_osif  - o_osif_t record
	--   dst     - destination of message
	--   call_id - id of the system call
	--   ret0    - result of the system call
	--   done    - indicates when system call finished
	--
	procedure osif_call_0_2 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret0   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret1   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- ONLY FOR INTERNAL USE
	--
	-- Issues a system call with one argument and a single result.
	--
	--   i_osif  - i_osif_t record
	--   o_osif  - o_osif_t record
	--   dst     - destination of message
	--   call_id - id of the system call
	--   arg0    - argument of the system call
	--   ret0    - result of the system call
	--   done    - indicates when system call finished
	--
	procedure osif_call_1_1 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		arg0          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret0   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- ONLY FOR INTERNAL USE
	--
	-- Issues a system call with one arguments and two results.
	--
	--   i_osif  - i_osif_t record
	--   o_osif  - o_osif_t record
	--   dst     - destination of message
	--   call_id - id of the system call
	--   arg0    - argument of the system call
	--   ret1    - first result of the system call
	--   ret2    - second result of the system call
	--   done    - indicates when system call finished
	--
	procedure osif_call_1_2 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		arg0          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret0   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret1   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	-- ONLY FOR INTERNAL USE
	--
	-- Issues a system call with two arguments and a single result.
	--
	--   i_osif  - i_osif_t record
	--   o_osif  - o_osif_t record
	--   dst     - destination of message
	--   call_id - id of the system call
	--   arg0    - first argument of the system call
	--   arg1    - second argument of the system call
	--   ret0    - result of the system call
	--   done    - indicates when system call finished
	--
	procedure osif_call_2_1 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		arg0          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		arg1          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret0   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- Posts the semaphore specified by handle.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   handle - index representing the resource in the resource array
	--   result - result of the osif call
	--   done   - indicates when call finished
	--
	procedure SEM_POST (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- Waits for the semaphore specified by handle.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   handle - index representing the resource in the resource array
	--   result - result of the osif call
	--   done   - indicates when call finished
	--
	procedure SEM_WAIT (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- Locks the mutex specified by handle.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   handle - index representing the resource in the resource array
	--   result - result of the osif call
	--   done   - indicates when call finished
	--
	procedure MUTEX_LOCK (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	-- Unlocks the mutex specified by handle.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   handle - index representing the resource in the resource array
	--   result - result of the osif call
	--   done   - indicates when call finished
	--
	procedure MUTEX_UNLOCK (
		signal i_osif  : in  i_osif_t;
		signal o_osif  : out o_osif_t;
		handle         : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal result  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done  : out boolean
	);

	--
	-- Tries to lock the mutex specified by handle and returns if successful or not.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   handle - index representing the resource in the resource array
	--   result - result of the osif call
	--   done   - indicates when call finished
	--
	procedure MUTEX_TRYLOCK (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);
	
	--
	-- Puts a single word into the mbox specified by handle.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   handle - index representing the resource in the resource array
	--   word   - word to write into the mbox
	--   result - result of the osif call
	--   done   - indicates when call finished
	--
	procedure MBOX_PUT (
		signal i_osif  : in  i_osif_t;
		signal o_osif  : out o_osif_t;
		handle         : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		word           : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal result  : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done  : out boolean
	);

	--
	-- Reads a single word from the mbox specified by handle.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   handle - index representing the resource in the resource array
	--   word   - word read from the mbox
	--   done   - indicates when call finished
	--
	procedure MBOX_GET (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal word   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- Tries to put a single word into the mbox specified by handle but does not
	-- blocks until the mbox gets populated.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   handle - index representing the resource in the resource array
	--   word   - word to write into the mbox
	--   result - indicates if word was written into the mbox
	--   done   - indicates when call finished
	--
	procedure MBOX_TRYPUT (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		word          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- Tries to read a single word from the mbox specified by handle but does not
	-- blocks until the mbox gets free.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   handle - index representing the resource in the resource array
	--   word   - word read from the mbox
	--   result - indicates if a word was read from the mbox
	--   done   - indicates when call finished
	--
	procedure MBOX_TRYGET (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal word   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- Gets the pointer to the initialization data of the ReconOS thread
	-- specified by reconos_hwt_setinitdata.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   init   - the pointer to the initialization data
	--   done   - indicated when call finished
	--
	procedure GET_INIT_DATA (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		signal init   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	);

	--
	-- Writes a single word into the main memory.
	--
	--   i_memif - i_memif_t record
	--   o_memif - o_memif_t record
	--   addr    - address of the main memory to write
	--   data    - word to write into the main memory
	--   done    - indicates that the call finished
	--
	procedure MEM_WRITE_WORD (
		signal i_memif : in  i_memif_t;
		signal o_memif : out o_memif_t;
		addr           : in  std_logic_vector(31 downto 0);
		data           : in  std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		variable done  : out boolean
	);

	--
	-- Reads a single word from the main memory.
	--
	--   i_memif - i_memif_t record
	--   o_memif - o_memif_t record
	--   addr    - address of the main memory to read from
	--   data    - word read from the main memory
	--   done    - indicates that the call finished
	--
	procedure MEM_READ_WORD (
		signal i_memif : in  i_memif_t;
		signal o_memif : out o_memif_t;
		addr           : in  std_logic_vector(31 downto 0);
		signal data    : out std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		variable done  : out boolean
	);

	--
 	-- Writes several words from the local ram into main memory. Therefore,
 	-- divides a large request into smaller ones of length at most
 	-- MEMIF_CHUNK_BYTES and splits request at page borders to guarantee
 	-- correct address translation.
	--
	--   i_ram    - i_ram_t record
	--   o_ram    - o_ram_t record
	--   i_memif  - i_memif_t record
	--   o_memif  - o_memif_t record
	--   src_addr - start address to read from the local ram
	--   dst_addr - start address to write into the main memory
	--   len      - number of bytes to transmit (bytes)
	--   done     - indicates that the call finished
	--
	procedure MEM_WRITE (
		signal i_ram   : in  i_ram_t;
		signal o_ram   : out o_ram_t;
		signal i_memif : in  i_memif_t;
		signal o_memif : out o_memif_t;
		src_addr       : in  std_logic_vector(31 downto 0);
		dst_addr       : in  std_logic_vector(31 downto 0);
		len            : in  std_logic_vector(31 downto 0);
		variable done  : out boolean
	);

	--
 	-- Reads several words from the main memory into the local ram. Therefore,
 	-- divides a large request into smaller ones of length at most
 	-- MEMIF_CHUNK_BYTES and splits request at page borders to guarantee
 	-- correct address translation.
	--
	--   i_ram    - i_ram_t record
	--   o_ram    - o_ram_t record
	--   i_memif  - i_memif_t record
	--   o_memif  - o_memif_t record
	--   src_addr - start address to read from the main memory
	--   dst_addr - start address to write into the local ram
	--   len      - number of bytes to transmit (bytes)
	--   done     - indicates that the call finished
	--
	procedure MEM_READ (
		signal i_ram   : in  i_ram_t;
		signal o_ram   : out o_ram_t;
		signal i_memif : in  i_memif_t;
		signal o_memif : out o_memif_t;
		src_addr       : in  std_logic_vector(31 downto 0);
		dst_addr       : in  std_logic_vector(31 downto 0);
		len            : in  std_logic_vector(31 downto 0);
		variable done  : out boolean
	);
	
end package reconos_calls;

package body reconos_calls is

	--
	-- @see header
	--
	procedure OSIF_READ (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		signal word   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		done := False;
		
		case i_osif.step is
			when 0 =>
				o_osif.sw2hw_re <= '1';

				o_osif.step <= 1;

			when 1 =>
				if i_osif.sw2hw_empty = '0' then
					word <= i_osif.sw2hw_data;
					o_osif.sw2hw_re <= '0';

					o_osif.step <= 2;
				end if;

			when others =>
					done := True;
					o_osif.step <= 0;

		end case;
	end procedure OSIF_READ;

	--
	-- @see header
	--	
	procedure OSIF_WRITE (
		signal i_osif  : in  i_osif_t;
		signal o_osif  : out o_osif_t;
		word           : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done  : out boolean
	) is begin
		done := False;

		case i_osif.step is
			when 0 =>
				o_osif.hw2sw_we <= '1';
				o_osif.hw2sw_data <= word;

				o_osif.step <= 1;

			when 1 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_we <= '0';

					o_osif.step <= 2;
				end if;

			when others =>
					done := True;
					o_osif.step <= 0;

		end case;
	end procedure OSIF_WRITE;

	--
	-- @see header
	--
	procedure osif_call_0_0 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		done := False;

		case i_osif.step is
			when 0 =>
				o_osif.hw2sw_we <= '1';
				o_osif.hw2sw_data <= i_osif.run_id & dst & x"0001";

				o_osif.step <= 1;

			when 1 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_data <= call_id;

					o_osif.step <= 2;
				end if;

			when 2 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_we <= '0';

					o_osif.step <= 3;
				end if;

			when others =>
					done := True;
					o_osif.step <= 0;

		end case;
	end procedure osif_call_0_0;

	--
	-- @see header
	--
	procedure osif_call_0_1 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret0   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		done := False;

		case i_osif.step is
			when 0 =>
				o_osif.hw2sw_we <= '1';
				o_osif.hw2sw_data <= i_osif.run_id & dst & x"0001";

				o_osif.step <= 1;

			when 1 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_data <= call_id;

					o_osif.step <= 2;
				end if;

			when 2 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_we <= '0';
					o_osif.sw2hw_re <= '1';

					o_osif.step <= 3;
				end if;

			when 3 =>
				if i_osif.sw2hw_empty = '0' then
					o_osif.step <= 4;
				end if;
				
			when 4 =>
				if i_osif.sw2hw_empty = '0' then
					ret0 <= i_osif.sw2hw_data;
					o_osif.sw2hw_re <= '0';

					o_osif.step <= 5;
				end if;

			when others =>
					done := True;
					o_osif.step <= 0;

		end case;
	end procedure osif_call_0_1;

	--
	-- @see header
	--
	procedure osif_call_0_2 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret0   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret1   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		done := False;

		case i_osif.step is
			when 0 =>
				o_osif.hw2sw_we <= '1';
				o_osif.hw2sw_data <= i_osif.run_id & dst & x"0001";

				o_osif.step <= 1;

			when 1 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_data <= call_id;

					o_osif.step <= 2;
				end if;

			when 2 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_we <= '0';
					o_osif.sw2hw_re <= '1';

					o_osif.step <= 3;
				end if;

			when 3 =>
				if i_osif.sw2hw_empty = '0' then
					o_osif.step <= 4;
				end if;
				
			when 4 =>
				if i_osif.sw2hw_empty = '0' then
					ret0 <= i_osif.sw2hw_data;

					o_osif.step <= 5;
				end if;
				
			when 5 =>
				if i_osif.sw2hw_empty = '0' then
					ret1 <= i_osif.sw2hw_data;
					o_osif.sw2hw_re <= '0';

					o_osif.step <= 6;
				end if;

			when others =>
					done := True;
					o_osif.step <= 0;

		end case;
	end procedure osif_call_0_2;

	--
	-- @see header
	--
	procedure osif_call_1_1 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		arg0          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret0   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		done := False;

		case i_osif.step is
			when 0 =>
				o_osif.hw2sw_we <= '1';
				o_osif.hw2sw_data <= i_osif.run_id & dst & x"0002";

				o_osif.step <= 1;

			when 1 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_data <= call_id;

					o_osif.step <= 2;
				end if;

			when 2 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_data <= arg0;

					o_osif.step <= 3;
				end if;

			when 3 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_we <= '0';
					o_osif.sw2hw_re <= '1';

					o_osif.step <= 4;
				end if;

			when 4 =>
				if i_osif.sw2hw_empty = '0' then
					o_osif.step <= 5;
				end if;

			when 5 =>
				if i_osif.sw2hw_empty = '0' then
					ret0 <= i_osif.sw2hw_data;
					o_osif.sw2hw_re <= '0';

					o_osif.step <= 6;
				end if;

			when others =>
					done := True;
					o_osif.step <= 0;

		end case;
	end procedure osif_call_1_1;

	--
	-- @see header
	--
	procedure osif_call_1_2 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		arg0          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret0   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret1   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		done := False;

		case i_osif.step is
			when 0 =>
				o_osif.hw2sw_we <= '1';
				o_osif.hw2sw_data <= i_osif.run_id & dst & x"0002";

				o_osif.step <= 1;

			when 1 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_data <= call_id;

					o_osif.step <= 2;
				end if;

			when 2 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_data <= arg0;

					o_osif.step <= 3;
				end if;

			when 3 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_we <= '0';
					o_osif.sw2hw_re <= '1';

					o_osif.step <= 3;
				end if;

			when 4 =>
				if i_osif.sw2hw_empty = '0' then
					o_osif.step <= 5;
				end if;
				
			when 5 =>
				if i_osif.sw2hw_empty = '0' then
					ret0 <= i_osif.sw2hw_data;

					o_osif.step <= 6;
				end if;
				
			when 6 =>
				if i_osif.sw2hw_empty = '0' then
					ret1 <= i_osif.sw2hw_data;
					o_osif.sw2hw_re <= '0';

					o_osif.step <= 7;
				end if;

			when others =>
					done := True;
					o_osif.step <= 0;
		end case;
	end procedure osif_call_1_2;

	--
	-- @see header
	--
	procedure osif_call_2_1 (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		dst           : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		call_id       : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		arg0          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		arg1          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal ret0   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		done := False;

		case i_osif.step is
			when 0 =>
				o_osif.hw2sw_we <= '1';
				o_osif.hw2sw_data <= i_osif.run_id & dst & x"0003";

				o_osif.step <= 1;

			when 1 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_data <= call_id;

					o_osif.step <= 2;
				end if;

			when 2 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_data <= arg0;

					o_osif.step <= 3;
				end if;

			when 3 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_data <= arg1;

					o_osif.step <= 4;
				end if;

			when 4 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_we <= '0';
					o_osif.sw2hw_re <= '1';

					o_osif.step <= 5;
				end if;

			when 5 =>
				if i_osif.sw2hw_empty = '0' then
					o_osif.step <= 6;
				end if;

			when 6 =>
				if i_osif.sw2hw_empty = '0' then
					ret0 <= i_osif.sw2hw_data;
					o_osif.sw2hw_re <= '0';

					o_osif.step <= 7;
				end if;

			when others =>
				done := True;
				o_osif.step <= 0;
		end case;
	end procedure osif_call_2_1;

	--
	-- @see header
	--
	procedure SEM_POST (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		osif_call_0_1(i_osif, o_osif, handle, OSIF_CMD_SEM_POST, result, done);
	end procedure SEM_POST;

	--
	-- @see header
	--
	procedure SEM_WAIT (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		osif_call_0_1(i_osif, o_osif, handle, OSIF_CMD_SEM_WAIT, result, done);
	end procedure SEM_WAIT;

	--
	-- @see header
	--
	procedure MUTEX_LOCK (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		osif_call_0_1(i_osif, o_osif, handle, OSIF_CMD_MUTEX_LOCK, result, done);
	end procedure MUTEX_LOCK;

	--
	-- @see header
	--
	procedure MUTEX_UNLOCK (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		osif_call_0_1(i_osif, o_osif, handle, OSIF_CMD_MUTEX_UNLOCK, result, done);
	end procedure MUTEX_UNLOCK;

	--
	-- @see header
	--
	procedure MUTEX_TRYLOCK (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		osif_call_0_1(i_osif, o_osif, handle, OSIF_CMD_MUTEX_TRYLOCK, result, done);
	end procedure MUTEX_TRYLOCK;

	--
	-- @see header
	--
	procedure MBOX_PUT (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		word          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		osif_call_1_1(i_osif, o_osif, handle, OSIF_CMD_MBOX_PUT, word, result, done);
	end procedure MBOX_PUT;

	--
	-- @see header
	--
	procedure MBOX_GET (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal word   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		osif_call_0_1(i_osif, o_osif, handle, OSIF_CMD_MBOX_GET, word, done);
	end procedure MBOX_GET;

	--
	-- @see header
	--
	procedure MBOX_TRYPUT (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		word          : in  std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		osif_call_1_1(i_osif, o_osif, handle, OSIF_CMD_MBOX_TRYPUT, word, result, done);
	end procedure MBOX_TRYPUT;

	--
	-- @see header
	--
	procedure MBOX_TRYGET (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		handle        : in  std_logic_vector(C_OSIF_SRC_WIDTH - 1 downto 0);
		signal word   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		signal result : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		osif_call_0_2(i_osif, o_osif, handle, OSIF_CMD_MBOX_TRYGET, word, result, done);
	end procedure MBOX_TRYGET;

	--
	-- @see header
	--
	procedure GET_INIT_DATA (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t;
		signal init   : out std_logic_vector(C_OSIF_DATA_WIDTH - 1 downto 0);
		variable done : out boolean
	) is begin
		osif_call_0_1(i_osif, o_osif, x"FF", OSIF_CMD_THREAD_GET_INIT_DATA, init, done);
	end procedure GET_INIT_DATA;

	--
	-- @see header
	--
	procedure MEM_WRITE_WORD (
		signal i_memif  : in  i_memif_t;
		signal o_memif  : out o_memif_t;
		addr            : in  std_logic_vector(31 downto 0);
		data            : in  std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		variable done   : out boolean
	) is begin
		done := False;

		case i_memif.step is
			when 0 =>
				o_memif.hwt2mem_we <= '1';
				o_memif.hwt2mem_data <= MEMIF_CMD_WRITE & X"000004";

				o_memif.step <= 1;

			when 1 =>
				if i_memif.hwt2mem_full = '0' then
					o_memif.hwt2mem_data <= addr(31 downto 2) & "00";

					o_memif.step <= 2;
				end if;

			when 2 =>
				if i_memif.hwt2mem_full = '0' then
					o_memif.hwt2mem_data <= data;

					o_memif.step <= 3;
				end if;

			when 3 =>
				if i_memif.hwt2mem_full = '0' then
					o_memif.hwt2mem_we <= '0';

					o_memif.step <= 4;
				end if;

			when others =>
					done := True;
					o_memif.step <= 0;

		end case;
	end procedure MEM_WRITE_WORD;
	
	procedure MEM_READ_WORD (
		signal i_memif  : in  i_memif_t;
		signal o_memif  : out o_memif_t;
		addr            : in  std_logic_vector(31 downto 0);
		signal data     : out  std_logic_vector(C_MEMIF_DATA_WIDTH - 1 downto 0);
		variable done   : out boolean
	) is begin
		done := False;

		case i_memif.step is
			when 0 =>
				o_memif.hwt2mem_we <= '1';
				o_memif.hwt2mem_data <= MEMIF_CMD_READ & X"000004";

				o_memif.step <= 1;

			when 1 =>
				if i_memif.hwt2mem_full = '0' then
					o_memif.hwt2mem_data <= addr(31 downto 2) & "00";

					o_memif.step <= 2;
				end if;

			when 2 =>
				if i_memif.hwt2mem_full = '0' then
					o_memif.hwt2mem_we <= '0';
					o_memif.mem2hwt_re <= '1';

					o_memif.step <= 3;
				end if;

			when 3 =>
				if i_memif.mem2hwt_empty = '0' then
					data <= i_memif.mem2hwt_data;
					o_memif.mem2hwt_re <= '0';

					o_memif.step <= 4;
				end if;

			when others =>
					done := True;
					o_memif.step <= 0;

		end case;
	end procedure MEM_READ_WORD;

	procedure MEM_WRITE (
		signal i_ram    : in  i_ram_t;
		signal o_ram    : out o_ram_t;
		signal i_memif  : in  i_memif_t;
		signal o_memif  : out o_memif_t;
		src_addr        : in  std_logic_vector(31 downto 0);
		dst_addr        : in  std_logic_vector(31 downto 0);
		len             : in  std_logic_vector(31 downto 0);
		variable done   : out boolean
	) is
		variable to_border, to_remm : unsigned(C_MEMIF_LENGTH_WIDTH - 1 downto 0);
	begin
		done := False;

		case i_memif.step is
			when 0 =>
				o_memif.hwt2mem_we <= '0';
				
				o_ram.mem_addr <= unsigned(dst_addr(31 downto 2) & "00");
				o_ram.remm <= unsigned(len);

				o_ram.ram_addr <= unsigned(src_addr);

				o_memif.step <= 1;

			when 1 =>
				o_memif.hwt2mem_we <= '1';

				to_border := to_unsigned(C_MEMIF_CHUNK_BYTES, C_MEMIF_LENGTH_WIDTH) - i_ram.mem_addr(C_MEMIF_CHUNK_RANGE);
				to_remm := i_ram.remm(C_MEMIF_LENGTH_RANGE);
				if to_remm < to_border then
					o_memif.hwt2mem_data <= MEMIF_CMD_WRITE & std_logic_vector(to_remm);
				else
					o_memif.hwt2mem_data <= MEMIF_CMD_WRITE & std_logic_vector(to_border);
				end if;

				o_memif.step <= 2;

			when 2 =>
				if i_memif.hwt2mem_full = '0' then
					o_memif.hwt2mem_data <= std_logic_vector(i_ram.mem_addr);

					o_memif.step <= 3;
				end if;

			when 3 =>
				if i_memif.hwt2mem_full = '0' then
					o_memif.hwt2mem_we <= '0';

					o_memif.step <= 4;
				end if;

			when 4 =>
				o_ram.ram_addr <= i_ram.ram_addr + 1;

				o_memif.step <= 5;
				
			when 5 =>
				o_memif.hwt2mem_we <= '1';
				o_memif.hwt2mem_data <= i_ram.ram_data;
					
				o_ram.ram_addr <= i_ram.ram_addr + 1;
					
				o_memif.step <= 6;
				
			when 6 =>
				if i_memif.hwt2mem_full = '0' then
					o_memif.hwt2mem_data <= i_ram.ram_data;
					
					o_ram.ram_addr <= i_ram.ram_addr + 1;
				
					o_ram.mem_addr <= i_ram.mem_addr + 4;
					o_ram.remm <= i_ram.remm - 4;
					
					if (i_ram.mem_addr + 4) mod C_MEMIF_CHUNK_BYTES = 0 then
						o_memif.hwt2mem_we <= '0';
						
						o_ram.ram_addr <= i_ram.ram_addr - 1;
						
						o_memif.step <= 1;
					end if;
								
					if i_ram.remm - 4 = 0 then
						o_memif.hwt2mem_we <= '0';
						
						o_memif.step <= 8;
					end if;
				else
					o_memif.hwt2mem_we <= '0';
				
					o_ram.ram_addr <= i_ram.ram_addr - 2;
				
					o_memif.step <= 7;
				end if;
				
			when 7 =>
				if i_memif.hwt2mem_full = '0' then
					o_memif.step <= 4;
				end if;

			when others =>
				o_memif.hwt2mem_we <= '0';
			
				o_memif.step <= 0;
				done := True;

		end case;
	end procedure MEM_WRITE;

	procedure MEM_READ (
		signal i_ram    : in  i_ram_t;
		signal o_ram    : out o_ram_t;
		signal i_memif  : in  i_memif_t;
		signal o_memif  : out o_memif_t;
		src_addr        : in  std_logic_vector(31 downto 0);
		dst_addr        : in  std_logic_vector(31 downto 0);
		len             : in  std_logic_vector(31 downto 0);
		variable done   : out boolean
	) is
		variable to_border, to_remm : unsigned(C_MEMIF_LENGTH_WIDTH - 1 downto 0);
	begin
		done := False;

		case i_memif.step is
			when 0 =>
				o_ram.mem_addr <= unsigned(src_addr(31 downto 2) & "00");
				o_ram.remm <= unsigned(len);

				o_ram.ram_addr <= unsigned(dst_addr) - 1;

				o_memif.step <= 1;

			when 1 =>
				o_ram.ram_we <= '0';
			
				to_border := to_unsigned(C_MEMIF_CHUNK_BYTES, C_MEMIF_LENGTH_WIDTH) - i_ram.mem_addr(C_MEMIF_CHUNK_RANGE);
				to_remm := i_ram.remm(C_MEMIF_LENGTH_RANGE);
				if to_remm < to_border then
					o_memif.hwt2mem_we <= '1';
					o_memif.hwt2mem_data <= MEMIF_CMD_READ & std_logic_vector(to_remm);
				else
					o_memif.hwt2mem_we <= '1';
					o_memif.hwt2mem_data <= MEMIF_CMD_READ & std_logic_vector(to_border);
				end if;

				o_memif.step <= 2;

			when 2 =>
				if i_memif.hwt2mem_full = '0' then
					o_memif.hwt2mem_data <= std_logic_vector(i_ram.mem_addr);

					o_memif.step <= 3;
				end if;

			when 3 =>
				if i_memif.hwt2mem_full = '0' then
					o_memif.hwt2mem_we <= '0';
					o_memif.mem2hwt_re <= '1';
				
					o_memif.step <= 4;
				end if;

			when 4 =>
				if i_memif.mem2hwt_empty = '0' then
					o_ram.ram_we <= '1';
					o_ram.ram_data <= i_memif.mem2hwt_data;
					
					o_ram.ram_addr <= i_ram.ram_addr + 1;
					o_ram.mem_addr <= i_ram.mem_addr + 4;
					o_ram.remm <= i_ram.remm - 4;
					
					if (i_ram.mem_addr + 4) mod C_MEMIF_CHUNK_BYTES = 0 then
						o_memif.mem2hwt_re <= '0';
					
						o_memif.step <= 1;
					end if;
					
					if i_ram.remm - 4 = 0 then
						o_memif.mem2hwt_re <= '0';
						
						o_memif.step <= 5;
					end if;
				end if;
				
			when others =>
				o_ram.ram_we <= '0';
				
				o_memif.step <= 0;
				done := true;

		end case;
	end procedure MEM_READ;
	
end package body reconos_calls;