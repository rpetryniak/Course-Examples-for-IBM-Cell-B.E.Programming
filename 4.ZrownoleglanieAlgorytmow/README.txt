%% --------------------------------------------------------------  
%% (C)Copyright 2001,2007,                                         
%% International Business Machines Corporation,                    
%% Sony Computer Entertainment, Incorporated,                      
%% Toshiba Corporation,                                            
%%                                                                 
%% All Rights Reserved.                                            
%%                                                                 
%% Redistribution and use in source and binary forms, with or      
%% without modification, are permitted provided that the           
%% following conditions are met:                                   
%%                                                                 
%% - Redistributions of source code must retain the above copyright
%%   notice, this list of conditions and the following disclaimer. 
%%                                                                 
%% - Redistributions in binary form must reproduce the above       
%%   copyright notice, this list of conditions and the following   
%%   disclaimer in the documentation and/or other materials        
%%   provided with the distribution.                               
%%                                                                 
%% - Neither the name of IBM Corporation nor the names of its      
%%   contributors may be used to endorse or promote products       
%%   derived from this software without specific prior written     
%%   permission.                                                   
%%                                                                 
%% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND          
%% CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,     
%% INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        
%% MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        
%% DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR            
%% CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    
%% SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT    
%% NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;    
%% LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)        
%% HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN       
%% CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR    
%% OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  
%% EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              
%% --------------------------------------------------------------  
%% PROLOG END TAG zYx                                              

Summary:
	Tutorial Example - Euler Particle System Simulation

Target:
	CBE-Linux PPE and SPE (HW or simulator)

Description:
	This directory contains a toy particle system simulation
	using Euler integration. The purpose of this example is
	to demonstrate how a simplified scalar function can be
	ported and accelerated for parallel execution on the Cell
	Broadband Engine (CBE) system. Development will follow the
	steps:

	STEP 1 - SIMDize (vetorize) the code for execution on the
	         Vector/SIMD Multimedia Extension (VMX unit of the PPE).
		 Two revisions a step 1 are provided depending upon
		 the organization of the particle system data.
		 (a) Array of structure (aos)
		 (b) Structure of arrays (soa)
	STEP 2 - Port the code for execution on the SPE.
	STEP 3 - Parallelize the code across multiple SPUs.
	STEP 4 - Optimize and tune for performance.

	These above steps is only one example of coding for the CBE. These
	steps can reordered and/or combined, depending upon the skill and
	comfort level of the programmer. It should be noted, that the code
	need not be vectorized to run on the SPE (it can run scalar code).
	However, scalar code does not exploit the full computational power
	of the SPE.

	The initial scalar code (see euler_scalar.c) is built (euler_scalar)
	and located in this directory. Each of the ported/optimized steps are
	located in the subdirectories:

	       subdirectory                      executable
	    -------------------		    -------------------
	    STEP1a_simd_aos		    euler_simd_aos
	    STEP1b_simd_soa		    euler_simd_soa
	    STEP2_spe			    euler_spe
	    STEP3_multi_spe		    euler_multi_spe
	    STEP4_tuned_multi_spe	    euler_tuned_multi_spe

	All the executables containing SPE code are built such that the SPE
	programs are embedded into the PPE executable.

How to run:
	Invoke any executable at the Linux command prompt. For example:

	       eular_scalar

	There are no command options for this program.

Notes:
	Additional information can be found within the source code.

	Please note this tutorial sample is meant to serve as s programming
	introduction. In the interest of keep in the sample simple, some
	short cuts have been made. For example, the data buffers are
	uninitialized and the number of SPE threads is hardcoded.
