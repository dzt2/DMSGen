## HOW TO USE DMSGEN in LINUX ##
Date: Mar 10th 2017 

1. Open terminate to run shell file "setup.sh"
2. If "dmsg" is generated then it succeeds to compile the code.
3. command line like the following:
	./dmsg [bias] [testnum] [input_file] [output_file]?
	
	--[bias]: integer to the first id for tests in current score-function;
	
	--[testnum]: the number of tests you have used in mutation testing;
	
	--[input_file]: score_function file, including line as following formats:
	
		{mut-id} "[" {length} "]" ":" { "t" {test-id} }* "\n"
	
	--[output_file]: file where the generated DMSG is maintained.

		==> you must use the APIs provided by our tools to derive information from it,

		==> Code SEE: dmsg.h 
			
			(1) DMSGraphReader

			(2) DMSGraph

			(3) DMSGVexIndex

			(4) DMSGHierarchy
