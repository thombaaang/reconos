library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

library proc_common_v3_00_a;
use proc_common_v3_00_a.proc_common_pkg.all;

entity fifo8 is
	generic (
		C_FIFO8_DEPTH  : integer := 1600;
		C_FIFO8_DEPTH_LOG2 	: integer := 11
	);
	port (
		Rst : in std_logic;
		FIFO32_S_Clk : in std_logic;
		FIFO32_M_Clk : in std_logic;
		FIFO32_S_Data : out std_logic_vector(7 downto 0);
		FIFO32_M_Data : in std_logic_vector(7 downto 0);
		FIFO32_S_Fill : out std_logic_vector(15 downto 0);
		FIFO32_M_Rem : out std_logic_vector(15 downto 0);
		FIFO32_S_Rd : in std_logic;
		FIFO32_M_Wr : in std_logic
	);
end entity;

architecture implementation of fifo32 is
	type MEM_T is array (0 to C_FIFO8_DEPTH_LOG2 of std_logic_vector(7 downto 0);
	signal mem : MEM_T;
	signal wrptr : std_logic_vector(C_FIFO8_DEPTH_LOG2-1 downto 0);
	signal rdptr : std_logic_vector(C_FIFO8_DEPTH_LOG2-1 downto 0);
	signal remainder : std_logic_vector(C_FIFO8_DEPTH_LOG2-1  downto 0);
	signal fill      : std_logic_vector(C_FIFO8_DEPTH_LOG2-1  downto 0);
	signal pad       :std_logic_vector(15 - C_FIFO8_DEPTH_LOG2 downto 0);
begin
	pad <= (others => '0');

	FIFO32_S_Fill <= pad & fill;
	FIFO32_M_Rem  <= pad & remainder;
	FIFO32_S_Data <= mem(CONV_INTEGER(rdptr));
	
	fill <= wrptr - rdptr when wrptr >= rdptr else (C_FIFO32_DEPTH + wrptr) - rdptr;
	remainder <= (C_FIFO32_DEPTH - 1) - fill;
	
	process(FIFO32_M_Clk, Rst)
	begin
		if Rst = '1' then
			wrptr <= (others => '0');
		elsif rising_edge(FIFO32_M_Clk) then
			if FIFO32_M_Wr = '1' then
				mem(CONV_INTEGER(wrptr)) <= FIFO32_M_Data;
				if wrptr = C_FIFO32_DEPTH-1 then
					wrptr <= (others => '0');
				else
					wrptr <= wrptr + 1;
				end if;
			end if;
		end if;
	end process;

	process(FIFO32_S_Clk, Rst)
	begin
		if Rst = '1' then
			rdptr <= (others => '0');
		elsif rising_edge(FIFO32_S_Clk) then
			if FIFO32_S_Rd = '1' then			
				if rdptr = C_FIFO32_DEPTH-1 then
					rdptr <= (others => '0');
				else
					rdptr <= rdptr + 1;
				end if;
			end if;
		end if;
	end process;

end architecture;

