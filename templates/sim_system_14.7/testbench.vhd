<<reconos_preproc>>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;
use reconos_v3_01_a.reconos_defs.all;
use reconos_v3_01_a.reconos_testbench.all;

library reconos_fifo_sync_v1_00_a;
use reconos_fifo_sync_v1_00_a.all;

library reconos_osif_interconnect_v1_00_a;
use reconos_osif_interconnect_v1_00_a.all;

library reconos_memif_arbiter_v1_00_a;
use reconos_memif_arbiter_v1_00_a.all;

<<generate for THREADS>>
library rt_<<Name>>_v1_00_a;
use rt_<<Name>>_v1_00_a.all;
<<end generate>>

<<generate for RESOURCES(Mode == "hw")>>
library hwres_<<Name>><<Id>>_v1_00_a;
use hwres_<<Name>><<Id>>_v1_00_a.all;
<<end generate>>

entity testbench is
end testbench;

architecture testbench of testbench is	
	signal sys_clk : std_logic;
	signal hwt_clk : std_logic;
	signal sys_rst : std_logic;

	<<generate for SLOTS>>
	signal ic2hw_<<Id>>_data   : std_logic_vector(31 downto 0);
	signal ic2hw_<<Id>>_empty  : std_logic;
	signal ic2hw_<<Id>>_re     : std_logic;

	signal hw2ic_<<Id>>_data   : std_logic_vector(31 downto 0);
	signal hw2ic_<<Id>>_full   : std_logic;
	signal hw2ic_<<Id>>_we     : std_logic;

	signal mem2hwt_<<Id>>_s_data   : std_logic_vector(31 downto 0);
	signal mem2hwt_<<Id>>_s_empty  : std_logic;
	signal mem2hwt_<<Id>>_s_re     : std_logic;
	signal mem2hwt_<<Id>>_m_data   : std_logic_vector(31 downto 0);
	signal mem2hwt_<<Id>>_m_full   : std_logic;
	signal mem2hwt_<<Id>>_m_we     : std_logic;
	
	signal hwt2mem_<<Id>>_s_data   : std_logic_vector(31 downto 0);
	signal hwt2mem_<<Id>>_s_empty  : std_logic;
	signal hwt2mem_<<Id>>_s_re     : std_logic;
	signal hwt2mem_<<Id>>_m_data   : std_logic_vector(31 downto 0);
	signal hwt2mem_<<Id>>_m_full   : std_logic;
	signal hwt2mem_<<Id>>_m_we     : std_logic;
	<<end generate>>

	signal ic2sw_data   : std_logic_vector(31 downto 0);
	signal ic2sw_empty  : std_logic;
	signal ic2sw_re     : std_logic;

	signal sw2ic_data   : std_logic_vector(31 downto 0);
	signal sw2ic_full   : std_logic;
	signal sw2ic_we     : std_logic;

	signal mem2hwt_data   : std_logic_vector(31 downto 0);
	signal mem2hwt_full   : std_logic;
	signal mem2hwt_we     : std_logic;
	
	signal hwt2mem_data   : std_logic_vector(31 downto 0);
	signal hwt2mem_empty  : std_logic;
	signal hwt2mem_re     : std_logic;

	signal slot_signal : std_logic_vector(<<NUM_SLOTS>> - 1 downto 0);

	<<generate for RESOURCES(Mode == "hw")>>
	signal ic2res_<<Id>>_data   : std_logic_vector(31 downto 0);
	signal ic2res_<<Id>>_empty  : std_logic;
	signal ic2res_<<Id>>_re     : std_logic;

	signal res2ic_<<Id>>_data   : std_logic_vector(31 downto 0);
	signal res2ic_<<Id>>_full   : std_logic;
	signal res2ic_<<Id>>_we     : std_logic;
	<<end generate>>
	
	type mem_type is array (0 to C_SYSTEM_RAM_SIZE_WORDS - 1)
	                       of std_logic_vector(31 downto 0);
								  
	function init_mem return mem_type is
		variable mem : mem_type;
	begin
		for i in 0 to C_SYSTEM_RAM_SIZE_WORDS - 1 loop
			mem(i) := std_logic_vector(to_unsigned(C_SYSTEM_RAM_SIZE_WORDS - i, 32));
		end loop;
		
		return mem;
	end function init_mem;
	
	signal mem : mem_type := init_mem;

	signal mem_state : integer := 0;
	signal mem_addr  : unsigned(31 downto 0) := (others => '0');
	signal mem_count : unsigned(C_MEMIF_LENGTH_WIDTH - 1 downto 0) := (others => '0');
	signal mem_op    : std_logic_vector(C_MEMIF_OP_WIDTH - 1 downto 0) := (others => '0');
begin

	sys_clk_process :process
	begin
		sys_clk <= '0';
		wait for C_CLK_PRD / 2;
		sys_clk <= '1';
		wait for C_CLK_PRD / 2;
	end process;
	
	hwt_clk_process :process
	begin
		hwt_clk <= '0';
		wait for C_CLK_PRD;
		hwt_clk <= '1';
		wait for C_CLK_PRD;
	end process;
	
	<<generate for SLOTS>>
	mem2hwt_<<Id>> : entity reconos_fifo_sync_v1_00_a.reconos_fifo_sync
		generic map (
			C_FIFO_DATA_WIDTH => 32,
			C_FIFO_ADDR_WIDTH => C_MEMIF_ADDR_WIDTH
		)

		port map (
			FIFO_S_Data => mem2hwt_<<Id>>_s_data,
			FIFO_S_Empty => mem2hwt_<<Id>>_s_empty,
			FIFO_S_RE => mem2hwt_<<Id>>_s_re,
			FIFO_M_Data => mem2hwt_<<Id>>_m_data,
			FIFO_M_Full => mem2hwt_<<Id>>_m_full,
			FIFO_M_WE => mem2hwt_<<Id>>_m_we,
			FIFO_Clk => sys_clk,
			FIFO_Rst => sys_rst
		);

	hwt2mem_<<Id>> : entity reconos_fifo_sync_v1_00_a.reconos_fifo_sync
		generic map (
			C_FIFO_DATA_WIDTH => 32,
			C_FIFO_ADDR_WIDTH => C_MEMIF_ADDR_WIDTH
		)

		port map (
			FIFO_S_Data => hwt2mem_<<Id>>_s_data,
			FIFO_S_Empty => hwt2mem_<<Id>>_s_empty,
			FIFO_S_RE => hwt2mem_<<Id>>_s_re,
			FIFO_M_Data => hwt2mem_<<Id>>_m_data,
			FIFO_M_Full => hwt2mem_<<Id>>_m_full,
			FIFO_M_WE => hwt2mem_<<Id>>_m_we,
			FIFO_Clk => sys_clk,
			FIFO_Rst => sys_rst
		);

	slot<<Id>> : entity rt_<<Thread>>_v1_00_a.rt_<<Thread>>
		port map (
			OSIF_Sw2Hw_Data => ic2hw_<<Id>>_data,
			OSIF_Sw2Hw_Empty => ic2hw_<<Id>>_empty,
			OSIF_Sw2Hw_RE => ic2hw_<<Id>>_re,
			OSIF_Hw2Sw_Data => hw2ic_<<Id>>_data,
			OSIF_Hw2Sw_Full => hw2ic_<<Id>>_full,
			OSIF_Hw2Sw_WE => hw2ic_<<Id>>_we,
			MEMIF_Mem2Hwt_Data => mem2hwt_<<Id>>_s_data,
			MEMIF_Mem2Hwt_Empty => mem2hwt_<<Id>>_s_empty,
			MEMIF_Mem2Hwt_RE => mem2hwt_<<Id>>_s_re,
			MEMIF_Hwt2Mem_Data => hwt2mem_<<Id>>_m_data,
			MEMIF_Hwt2Mem_Full => hwt2mem_<<Id>>_m_full,
			MEMIF_Hwt2Mem_WE => hwt2mem_<<Id>>_m_we,
			HWT_Clk => sys_clk,
			HWT_Rst => sys_rst,
			HWT_Signal => slot_signal(<<Id>>)
		);
	<<end generate>>

	<<generate for RESOURCES(Mode == "hw")>>
	hwres<<Id>> : entity hwres_<<Name>><<Id>>_v1_00_a.hwres_<<Name>><<Id>>
		port map (
			OSIF_In_Data  => ic2res_<<Id>>_data,
			OSIF_In_Empty => ic2res_<<Id>>_empty,
			OSIF_In_RE    => ic2res_<<Id>>_re,

			OSIF_Out_Data => res2ic_<<Id>>_data,
			OSIF_Out_Full => res2ic_<<Id>>_full,
			OSIF_Out_WE   => res2ic_<<Id>>_we,

			SYS_Clk    => sys_clk,
			SYS_Rst    => sys_rst
		);
	<<end generate>>

	interconnect : entity reconos_osif_interconnect_v1_00_a.reconos_osif_interconnect
		generic map (
			C_NUM_HWTS => <<NUM_SLOTS>>
		)

		port map (
			<<generate for SLOTS>>
			OSIF_Hw2Ic_<<Id>>_Data  => hw2ic_<<Id>>_data,
			OSIF_Hw2Ic_<<Id>>_Full  => hw2ic_<<Id>>_full,
			OSIF_Hw2Ic_<<Id>>_WE    => hw2ic_<<Id>>_we,
			OSIF_Ic2Hw_<<Id>>_Data  => ic2hw_<<Id>>_data,
			OSIF_Ic2Hw_<<Id>>_Empty => ic2hw_<<Id>>_empty,
			OSIF_Ic2Hw_<<Id>>_RE    => ic2hw_<<Id>>_re,
			<<end generate>>

			<<generate for RESOURCES(Mode == "hw")>>
			OSIF_Res2Ic_<<Id>>_Data  => res2ic_<<Id>>_data,
			OSIF_Res2Ic_<<Id>>_Full  => res2ic_<<Id>>_full,
			OSIF_Res2Ic_<<Id>>_WE    => res2ic_<<Id>>_we,
			OSIF_Ic2Res_<<Id>>_Data  => ic2res_<<Id>>_data,
			OSIF_Ic2Res_<<Id>>_Empty => ic2res_<<Id>>_empty,
			OSIF_Ic2Res_<<Id>>_RE    => ic2res_<<Id>>_re,
			<<end generate>>

			OSIF_Sw2Ic_Data  => sw2ic_data,
			OSIF_Sw2Ic_Full  => sw2ic_full,
			OSIF_Sw2Ic_WE    => sw2ic_we,
			OSIF_Ic2Sw_Data  => ic2sw_data,
			OSIF_Ic2Sw_Empty => ic2sw_empty,
			OSIF_Ic2Sw_RE    => ic2sw_re,

			SYS_Clk => sys_clk,
			SYS_Rst => sys_rst
		);

	arbiter :  entity reconos_memif_arbiter_v1_00_a.reconos_memif_arbiter
		generic map (
			C_NUM_HWTS => <<NUM_SLOTS>>
		)

		port map (
			<<generate for SLOTS>>
			MEMIF_Hwt2Mem_<<Id>>_In_Data  => hwt2mem_<<Id>>_s_data,
			MEMIF_Hwt2Mem_<<Id>>_In_Empty => hwt2mem_<<Id>>_s_empty,
			MEMIF_Hwt2Mem_<<Id>>_In_RE    => hwt2mem_<<Id>>_s_re,

			MEMIF_Mem2Hwt_<<Id>>_In_Data  => mem2hwt_<<Id>>_m_data,
			MEMIF_Mem2Hwt_<<Id>>_In_Full  => mem2hwt_<<Id>>_m_full,
			MEMIF_Mem2Hwt_<<Id>>_In_WE    => mem2hwt_<<Id>>_m_we,
			<<end generate>>

			MEMIF_Hwt2Mem_Out_Data  => hwt2mem_data,
			MEMIF_Hwt2Mem_Out_Empty => hwt2mem_empty,
			MEMIF_Hwt2Mem_Out_RE    => hwt2mem_re,

			MEMIF_Mem2Hwt_Out_Data  => mem2hwt_data,
			MEMIF_Mem2Hwt_Out_Full  => mem2hwt_full,
			MEMIF_Mem2Hwt_Out_WE    => mem2hwt_we,

			SYS_Clk => sys_clk,
			SYS_Rst => sys_rst
		);
		
	stimulus : entity work.stimulus
		port map (
			OSIF_Ic2Sw_Data => ic2sw_data,
			OSIF_Ic2Sw_Empty => ic2sw_empty,
			OSIF_Ic2Sw_RE => ic2sw_re,
			OSIF_Sw2Ic_Data => sw2ic_data,
			OSIF_Sw2Ic_Full => sw2ic_full,
			OSIF_Sw2Ic_WE => sw2ic_we,
			SYS_Rst => sys_rst,
			SLOT_Signal => slot_signal
		);


	memif_stub : process(sys_clk,sys_rst)
	begin
		if sys_rst = '1' then
			mem_state <= 0;
			hwt2mem_re <= '0';
			mem2hwt_we <= '0';
		elsif rising_edge(sys_clk) then
			case mem_state is
				when 0 =>
					mem2hwt_we <= '0';
					hwt2mem_re <= '1';

					mem_state <= 1;

				when 1 =>
					if hwt2mem_empty = '0' then
						mem_op <= hwt2mem_data(C_MEMIF_DATA_WIDTH - 1 downto C_MEMIF_DATA_WIDTH - C_MEMIF_OP_WIDTH);
						mem_count <= unsigned(hwt2mem_data(C_MEMIF_LENGTH_WIDTH - 1 downto 0));

						mem_state <= 2;
					end if;

				when 2 =>
					if hwt2mem_empty = '0' then
						mem_addr <= unsigned(hwt2mem_data);

						hwt2mem_re <= '0';
						mem_state <= 3;
					end if;

				when 3 =>
					case mem_op is
						when MEMIF_CMD_WRITE =>
							hwt2mem_re <= '1';

						when others =>
					end case;

					mem_state <= 4;

				when 4 =>
					case mem_op is
						when MEMIF_CMD_READ =>
							if mem2hwt_full = '0' then
								mem2hwt_data <= mem(to_integer(mem_addr / 4));
								mem2hwt_we <= '1';

								mem_addr <= mem_addr + 4;
								mem_count <= mem_count - 4;

								if mem_count - 4 = 0 then
									mem_state <= 5;
								end if;
							end if;

						when MEMIF_CMD_WRITE =>
							if hwt2mem_empty = '0' then
								mem(to_integer(mem_addr / 4)) <= hwt2mem_data;

								mem_addr <= mem_addr + 4;
								mem_count <= mem_count - 4;

								if mem_count - 4 = 0 then
									hwt2mem_re <= '0';

									mem_state <= 0;
								end if;
							end if;

						when others =>
					end case;
					
				when 5 => 
					if mem2hwt_full = '0' then
						mem2hwt_we <= '0';
						
						mem_state <= 0;
					end if;

				when others =>
			end case;
		end if;

	end process memif_stub;

end testbench;

