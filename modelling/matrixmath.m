function [ip_oct, op_oct] = in_and_outProducts(x,y) 
	%x=[3 3 3];
	%y = [1 2 1];
	ip_oct= x*y'
	op_oct= x'*y
