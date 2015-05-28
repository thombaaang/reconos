--                                                        ____  _____
--                            ________  _________  ____  / __ \/ ___/
--                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
--                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
--                         /_/   \___/\___/\____/_/ /_/\____//____/
-- 
-- ======================================================================
--
--   title:        IP-Core - FIFO2AXI - Top level entity
--
--   project:      ReconOS
--   author:       Christoph Rüthing, University of Paderborn
--   description:  A AXI slave which maps a fifo to registers accessible
--                 from the AXI-Bus.
--                   Reg0: Read data
--                   Reg1: Write data
--                   Reg2: Fill - number of elements in receive-FIFO
--                   Reg3: Rem - free space in send-FIFO
--
-- ======================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library proc_common_v3_00_a;
use proc_common_v3_00_a.ipif_pkg.all;

library axi_lite_ipif_v1_01_a;
use axi_lite_ipif_v1_01_a.axi_lite_ipif;

library reconos_fifo2axi_v1_00_a;
use reconos_fifo2axi_v1_00_a.user_logic;

entity reconos_fifo2axi is
	--
	-- Generic definitions
	--
	--   C_S_AXI_ - @see axi bus
	--
	--   C_BASE_ADDR - lower address of axi slave
	--   C_HIGH_ADDR - higher address of axi slave
	--
	--   C_FIFO_DATA_WIDTH - data width of the connected fifo
	--
	generic (
		C_S_AXI_ADDR_WIDTH : integer := 32;
		C_S_AXI_DATA_WIDTH : integer := 32;

		C_BASEADDR : std_logic_vector := x"FFFFFFFF";
		C_HIGHADDR : std_logic_vector := x"00000000";

		C_FIFO_DATA_WIDTH : integer := 32
	);

	--
	-- Port defintions
	--
	--   FIFO_In_/FIFO_Out_ - fifo signal inputs
	--
	--   S_AXI_ - @see axi bus
	--
	port (
		FIFO_In_Data  : in  std_logic_vector(C_FIFO_DATA_WIDTH - 1 downto 0);
		FIFO_In_Empty : in  std_logic;
		FIFO_In_RE    : out std_logic;

		FIFO_Out_Data : out std_logic_vector(C_FIFO_DATA_WIDTH - 1 downto 0);
		FIFO_Out_Full : in  std_logic;
		FIFO_Out_WE   : out std_logic;

		S_AXI_ACLK    : in  std_logic;
		S_AXI_ARESETN : in  std_logic;
		S_AXI_AWADDR  : in  std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_AWVALID : in  std_logic;
		S_AXI_WDATA   : in  std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_WSTRB   : in  std_logic_vector(C_S_AXI_DATA_WIDTH / 8 - 1 downto 0);
		S_AXI_WVALID  : in  std_logic;
		S_AXI_BREADY  : in  std_logic;
		S_AXI_ARADDR  : in  std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_ARVALID : in  std_logic;
		S_AXI_RREADY  : in  std_logic;
		S_AXI_ARREADY : out std_logic;
		S_AXI_RDATA   : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_RRESP   : out std_logic_vector(1 downto 0);
		S_AXI_RVALID  : out std_logic;
		S_AXI_WREADY  : out std_logic;
		S_AXI_BRESP   : out std_logic_vector(1 downto 0);
		S_AXI_BVALID  : out std_logic;
		S_AXI_AWREADY : out std_logic
	);
end entity reconos_fifo2axi;

architecture imp of reconos_fifo2axi is
	--
	-- Internal ipif signals
	--
	--   @see axi_lite_ipif_v1_01_a
	--
	signal bus2ip_clk    : std_logic;
	signal bus2ip_resetn : std_logic;
	signal bus2ip_data   : std_logic_vector(31 downto 0);
	signal bus2ip_cs     : std_logic_vector(0 downto 0);
	signal bus2ip_rdce   : std_logic_vector(3 downto 0);
	signal bus2ip_wrce   : std_logic_vector(3 downto 0);
	signal ip2bus_data   : std_logic_vector(31 downto 0);
	signal ip2bus_rdack  : std_logic;
	signal ip2bus_wrack  : std_logic;
	signal ip2bus_error  : std_logic;

	--
	-- Constants to configure ipif
	--
	--   @see axi_lite_ipif_v1_01_a
	--
	constant C_ADDR_PAD : std_logic_vector(31 downto 0) := (others => '0');

	constant C_ARD_ADDR_RANGE_ARRAY : SLV64_ARRAY_TYPE := (
		0 => C_ADDR_PAD & std_logic_vector(unsigned(C_BASEADDR) + 0),
		1 => C_ADDR_PAD & std_logic_vector(unsigned(C_BASEADDR) + 15)
	);

	constant C_ARD_NUM_CE_ARRAY : INTEGER_ARRAY_TYPE := (
		0 => 4
	);
begin

	-- == Instantiation of components =====================================

	--
	-- Instantiation of axi_lite_ipif_v1_01_a
	--
	--   @see axi_lite_ipif_ds765.pdf
	--
	ipif : entity axi_lite_ipif_v1_01_a.axi_lite_ipif
		generic map (
			C_S_AXI_ADDR_WIDTH => C_S_AXI_ADDR_WIDTH,
			C_S_AXI_DATA_WIDTH => C_S_AXI_DATA_WIDTH,

			C_ARD_ADDR_RANGE_ARRAY => C_ARD_ADDR_RANGE_ARRAY,
			C_ARD_NUM_CE_ARRAY     => C_ARD_NUM_CE_ARRAY
		)

		port map (
			s_axi_aclk    => S_AXI_ACLK,
			s_axi_aresetn => S_AXI_ARESETN,
			s_axi_awaddr  => S_AXI_AWADDR,
			s_axi_awvalid => S_AXI_AWVALID,
			s_axi_wdata   => S_AXI_WDATA,
			s_axi_wstrb   => S_AXI_WSTRB,
			s_axi_wvalid  => S_AXI_WVALID,
			s_axi_bready  => S_AXI_BREADY,
			s_axi_araddr  => S_AXI_ARADDR,
			s_axi_arvalid => S_AXI_ARVALID,
			s_axi_rready  => S_AXI_RREADY,
			s_axi_arready => S_AXI_ARREADY,
			s_axi_rdata   => S_AXI_RDATA,
			s_axi_rresp   => S_AXI_RRESP,
			s_axi_rvalid  => S_AXI_RVALID,
			s_axi_wready  => S_AXI_WREADY,
			s_axi_bresp   => S_AXI_BRESP,
			s_axi_bvalid  => S_AXI_BVALID,
			s_axi_awready => S_AXI_AWREADY,

			bus2ip_clk    => bus2ip_clk,
			bus2ip_resetn => bus2ip_resetn,
			bus2ip_data   => bus2ip_data,
			bus2ip_cs     => bus2ip_cs,
			bus2ip_rdce   => bus2ip_rdce,
			bus2ip_wrce   => bus2ip_wrce,
			ip2bus_data   => ip2bus_data,
			ip2bus_rdack  => ip2bus_rdack,
			ip2bus_wrack  => ip2bus_wrack,
			ip2bus_error  => ip2bus_error
		);

	--
	-- Instantiation of user logic
	--
	--   The user logic includes the actual implementation of the bus
	--   attachment.
	--
	ul : entity reconos_fifo2axi_v1_00_a.user_logic
		port map (
			FIFO_In_Data   => FIFO_In_Data,
			FIFO_In_Empty  => FIFO_In_Empty,
			FIFO_In_RE     => FIFO_In_RE,

			FIFO_Out_Data  => FIFO_Out_Data,
			FIFO_Out_Full  => FIFO_Out_Full,
			FIFO_Out_WE    => FIFO_Out_WE,

			BUS2IP_Clk    => bus2ip_clk,
			BUS2IP_Resetn => bus2ip_resetn,
			BUS2IP_Data   => bus2ip_data,
			BUS2IP_CS     => bus2ip_cs,
			BUS2IP_RdCE   => bus2ip_rdce,
			BUS2IP_WrCE   => bus2ip_wrce,
			IP2BUS_Data   => ip2bus_data,
			IP2BUS_RdAck  => ip2bus_rdack,
			IP2BUS_WrAck  => ip2bus_wrack,
			IP2BUS_Error  => ip2bus_error
		);
end architecture imp;