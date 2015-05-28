--                                                        ____  _____
--                            ________  _________  ____  / __ \/ ___/
--                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
--                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
--                         /_/   \___/\___/\____/_/ /_/\____//____/
-- 
-- ======================================================================
--
--   title:        VHDL Thread Package - ReconOS
--
--   project:      ReconOS
--   author:       Enno Lübbers, University of Paderborn
--                 Andreas Agne, University of Paderborn
--                 Christoph Rüthing, University of Paderborn
--   description:  Auto-generated thread specific package.
--
-- ======================================================================

<<reconos_preproc>>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;
use reconos_v3_01_a.reconos_defs.all;

package reconos_thread is
	<<generate for RESOURCES>>
	constant <<NameLower>> : std_logic_vector(C_OSIF_ID_WIDTH - 1 downto 0) := <<Id|x"{:02x}">>;
	<<end generate>>

	--
	-- Initilaizes the current ReconOS thread.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--   resume - indicates whether resume or start
	--   done   - indicates when init finished
	--
	procedure THREAD_INIT (
		signal i_osif   : in  i_osif_t;
		signal o_osif   : out o_osif_t;
		variable resume : out boolean;
		variable done   : out boolean
	);

	--
	-- Terminates the current ReconOS thread.
	--
	--   i_osif - i_osif_t record
	--   o_osif - o_osif_t record
	--
	procedure THREAD_EXIT (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t
	);

end package reconos_thread;

package body reconos_thread is

	--
	-- @see header
	--
	procedure THREAD_INIT (
		signal i_osif   : in  i_osif_t;
		signal o_osif   : out o_osif_t;
		variable resume : out boolean;
		variable done   : out boolean
	) is begin
		case i_osif.step is
			when 0 =>
				o_osif.sw2hw_re <= '1';

				o_osif.step <= 1;

			when 1 =>
				if i_osif.sw2hw_empty = '0' then
					o_osif.step <= 2;
				end if;

			when 2 =>
				if i_osif.sw2hw_empty = '0' then
					resume := i_osif.sw2hw_data = OSIF_SIGNAL_THREAD_RESUME;

					o_osif.step <= 3;
				end if;

			when 3 =>
				if i_osif.sw2hw_empty = '0' then
					o_osif.run_id <= i_osif.sw2hw_data(C_OSIF_ID_WIDTH - 1 downto 0);
					o_osif.sw2hw_re <= '0';

					o_osif.step <= 4;
				end if;

			when others =>
				done := True;
				o_osif.step <= 0;
		end case;
	end procedure THREAD_INIT;

	--
	-- @see header
	--
	procedure THREAD_EXIT (
		signal i_osif : in  i_osif_t;
		signal o_osif : out o_osif_t
	) is begin
		case i_osif.step is
			when 0 =>
				o_osif.hw2sw_we <= '1';
				o_osif.hw2sw_data <= i_osif.run_id & x"FF" & x"0001";

				o_osif.step <= 1;

			when 1 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_data <= OSIF_CMD_THREAD_EXIT;

					o_osif.step <= 2;
				end if;

			when 2 =>
				if i_osif.hw2sw_full = '0' then
					o_osif.hw2sw_we <= '0';

					o_osif.step <= 3;
				end if;

			when others =>
		end case;
	end procedure THREAD_EXIT;

end package body reconos_thread;