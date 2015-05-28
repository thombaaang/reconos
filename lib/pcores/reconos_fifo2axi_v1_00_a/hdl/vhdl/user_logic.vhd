--                                                        ____  _____
--                            ________  _________  ____  / __ \/ ___/
--                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
--                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
--                         /_/   \___/\___/\____/_/ /_/\____//____/
-- 
-- ======================================================================
--
--   title:        IP-Core - OSIF - Top level entity
--
--   project:      ReconOS
--   author:       Christoph Rüthing, University of Paderborn
--   description:  A AXI slave which maps the FIFOs of the HWTs to
--                 registers accessible from the AXI-Bus.
--                   Reg0: Read data
--                   Reg1: Write data
--                   Reg2: Fill - number of elements in receive-FIFO
--                   Reg3: Rem - free space in send-FIFO
--
-- ======================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity user_logic is
	--
	-- Generic definitions
	--
	--   C_FIFO_DATA_WIDTH - data width of the connected fifo
	--
	generic (
		C_FIFO_DATA_WIDTH : integer := 32
	);
	--
	-- Port defintions
	--
	--   FIFO_In_/FIFO_Out_ - fifo signal inputs
	--
	--   BUS2IP_/IP2BUS_ - axi ipif signals
	--
	port (
		FIFO_In_Data  : in  std_logic_vector(C_FIFO_DATA_WIDTH - 1 downto 0);
		FIFO_In_Empty : in  std_logic;
		FIFO_In_RE    : out std_logic;

		FIFO_Out_Data : out std_logic_vector(C_FIFO_DATA_WIDTH - 1 downto 0);
		FIFO_Out_Full : in  std_logic;
		FIFO_Out_WE   : out std_logic;

		BUS2IP_Clk    : in  std_logic;
		BUS2IP_Resetn : in  std_logic;
		BUS2IP_Data   : in  std_logic_vector(31 downto 0);
		BUS2IP_CS     : in  std_logic_vector(0 downto 0);
		BUS2IP_RdCE   : in  std_logic_vector(3 downto 0);
		BUS2IP_WrCE   : in  std_logic_vector(3 downto 0);
		IP2BUS_Data   : out std_logic_vector(31 downto 0);
		IP2BUS_RdAck  : out std_logic;
		IP2BUS_WrAck  : out std_logic;
		IP2BUS_Error  : out std_logic
	);
end entity user_logic;

architecture imp of user_logic is

begin

	-- == Access of fifos =================================================

	FIFO_In_RE    <= BUS2IP_RdCE(3);
	FIFO_Out_Data <= BUS2IP_Data;
	FIFO_Out_WE   <= BUS2IP_WrCE(2);

	IP2BUS_Data <= FIFO_In_Data                       when BUS2IP_RdCE(3) = '1' else
	               FIFO_In_Empty & "000" & x"0000000" when BUS2IP_RdCE(1) = '1' else
	               FIFO_Out_Full & "000" & x"0000000" when BUS2IP_RdCE(0) = '1' else
	               x"00000000";

	IP2BUS_RdAck <= BUS2IP_CS(0);
	IP2BUS_WrAck <= BUS2IP_CS(0);
	IP2BUS_Error <= '0';

end architecture imp;