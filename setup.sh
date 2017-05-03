## remove original program
exe="dmsg"
srcList="src/main.cpp src/text.cpp src/dmsg.cpp src/bitseq.cpp"
if [ -f $exe ]
then
	echo "delete original exe..." 
    rm exe
fi

# comple
echo "compile to generate dmsg..."
clang++ -std=c++0x $srcList -o $exe
if [ ! -f $exe ]
then
	echo "compilation error! Now exit..."
	exit 1
fi

# running tests
rm outputs/*
echo "\nrunning tests...\n"

./dmsg 1 14 inputs/prime_result.txt outputs/prime_graph.dat
if [ ! -f outputs/prime_graph.dat ]
then
	echo "Testing fails at prime_results.txt"
	exit 2
fi
./dmsg 1 20 inputs/triangle_result.txt outputs/triangle_graph.dat
if [ ! -f outputs/triangle_graph.dat ]
then
	echo "Testing fails at triangle_results.txt"
	exit 2
fi
