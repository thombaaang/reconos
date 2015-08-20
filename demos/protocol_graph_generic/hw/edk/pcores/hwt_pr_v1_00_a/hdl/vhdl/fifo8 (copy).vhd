library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

--library proc_common_v3_00_a;
--use proc_common_v3_00_a.proc_common_pkg.all;

entity fifo8 is
	generic (
		C_FIFO8_DEPTH  		: integer := 1600;
		C_FIFO8_DEPTH_LOG2 	: integer := 11 -- clog2(C_FIFO8_DEPTH) function not supported or needs proc_common library
	);
	port (
		Rst : in std_logic;
		FIFO8_S_Clk : in std_logic;
		FIFO8_M_Clk : in std_logic;
		FIFO8_S_Data : out std_logic_vector(7 downto 0); 
		FIFO8_M_Data : in std_logic_vector(7 downto 0);
		FIFO8_S_Fill : out std_logic_vector(15 downto 0); -- current size
		FIFO8_M_Rem : out std_logic_vector(15 downto 0); -- remainder
		FIFO8_S_Rd : in std_logic;
		FIFO8_M_Wr : in std_logic
	);
end entity;

architecture implementation of fifo8 is
	type MEM_T is array (0 to C_FIFO8_DEPTH-1) of std_logic_vector(7 downto 0);
	signal mem 				: MEM_T;
	
	attribute ram_style: string;
	attribute ram_style of mem : signal is "block";

	
	signal wrptr 				: std_logic_vector(C_FIFO8_DEPTH_LOG2-1 downto 0);
	signal wrptr_next 		: std_logic_vector(C_FIFO8_DEPTH_LOG2-1 downto 0);
	signal rdptr 				: std_logic_vector(C_FIFO8_DEPTH_LOG2-1 downto 0);
	signal rdptr_next 		: std_logic_vector(C_FIFO8_DEPTH_LOG2-1 downto 0);
	signal remainder 			: std_logic_vector(C_FIFO8_DEPTH_LOG2-1  downto 0);
	signal fill      			: std_logic_vector(C_FIFO8_DEPTH_LOG2-1  downto 0);
	signal pad       			: std_logic_vector(15 - C_FIFO8_DEPTH_LOG2 downto 0);
	signal fifo_data_next	: std_logic_vector(7 downto 0);
	signal debug_write		: std_logic_vector(7  downto 0);
	signal debug_read			: std_logic_vector(7  downto 0);
begin
	pad <= (others => '0');

	FIFO8_S_Fill <= pad & fill;
	FIFO8_M_Rem  <= pad & remainder;
	
	-- connected the wrong way (?) and fill start at 0 instead of 1 if one packet is in the FIFO
--	fill <= wrptr - rdptr when wrptr >= rdptr else (C_FIFO8_DEPTH + wrptr) - rdptr;
--	remainder <= (C_FIFO8_DEPTH - 1) - fill;
	fill <= wrptr - rdptr when wrptr >= rdptr else (C_FIFO8_DEPTH - rdptr) +	 wrptr;
	remainder <= C_FIFO8_DEPTH - fill;
	
	-- debug
	debug_write <= mem(CONV_INTEGER(wrptr));
	debug_read <= mem(CONV_INTEGER(rdptr));
	
	
	write_memless : process(wrptr, FIFO8_M_Wr, FIFO8_M_Data)
	begin
		-- default assignments
		wrptr_next <= wrptr;
		fifo_data_next <= X"00";
		
		-- nondefault transitions and outputs
		if FIFO8_M_Wr = '1' then
			fifo_data_next <= FIFO8_M_Data;
			if wrptr = C_FIFO8_DEPTH-1 then
				wrptr_next <= (others => '0');
			else
				wrptr_next <= wrptr + 1;
			end if;
		end if;
	end process;
	
	write_memzing : process(FIFO8_M_Clk, Rst)
	begin
		if Rst = '1' then
			wrptr <= (others => '0');
			mem(CONV_INTEGER(wrptr)) <= (others => '0');
		elsif rising_edge(FIFO8_M_Clk) then
			-- state = where to write to & content of memory
			wrptr <= wrptr_next;
			mem(CONV_INTEGER(wrptr)) <= fifo_data_next;
		end if;
	end process;
	
	
	read_memless : process(rdptr, FIFO8_S_Rd, mem)
	begin
		-- default assignments
		rdptr_next <= rdptr;
		FIFO8_S_Data <=  X"00";
		
		-- nondefault transitions and outputs
		if FIFO8_S_Rd = '1' then
			FIFO8_S_Data <= mem(CONV_INTEGER(rdptr));		
			if rdptr = C_FIFO8_DEPTH-1 then
				rdptr_next <= (others => '0');
			else
				rdptr_next <= rdptr + 1;
			end if;
		end if;
	end process;
	
	read_memzing : process(FIFO8_M_Clk, Rst)
	begin
		if Rst = '1' then
			rdptr <= (others => '0');
		elsif rising_edge(FIFO8_M_Clk) then
			-- state = where to read from
			rdptr <= rdptr_next;
		end if;
	end process;

end architecture;

