BENCHMARK_DIRS="test-linear-reg bubble-sort edit-distance eulers-number-approx fft-int kepler-calc mersenne nonlinear-nn nr-solver parrando rad-to-degree lda triangle-count"
# BENCHMARK_DIRS="boyer-moore-search mnist-cnn scalar-gwas-chi2 vector-gwas-chi2"

ITERS=100
SLEEP=0.00000001
curDIR=$PWD

# Print header for results profile
echo -e "Benchmark,\t Runtime (ms),\t Runtime (us),\t Processor Cycles,\t Instruction Count,\t VmPeak (kB),\t VmRSS (kB), \tVmSize (kB)"

for dir in $BENCHMARK_DIRS; do
    cd $dir/bin

    # Create results files for recording benchmark statistics for native (na) mode
    echo $dir " MODE=na " > $dir-na.results
    echo -e "[VIP]\t Runtime (ms)\t Runtime (us)\t Processor Cycles\t Instruction count\n" >> $dir-na.results
    echo $dir " MODE=na " > mem.out
    echo -e "pid vsz rss\n" >> mem.out
    # Run benchmark and record statistics $ITERS times
    for i in $(eval echo "{1..$ITERS}")
    do
        WATCHED_PID=$({ ./$dir.na  > $dir-na.out 2>> $dir-na.results & } && echo $!);
        while [[ ${?} == 0 ]]  # Wait for WATCHED_PID to complete
        do
            sleep $SLEEP         
            ps -q $WATCHED_PID >/dev/null  
        done
    done
    mv mem.out $dir-na.mem

    # Create results files for recording benchmark statistics for data-oblivious (do) mode
    echo $dir " MODE=do " > $dir-do.results
    echo -e "[VIP]\t Runtime (ms)\t Runtime (us)\t Processor Cycles\t Instruction count\n" >> $dir-do.results
    echo $dir " MODE=do " > mem.out
    echo -e "pid vsz rss\n" >> mem.out
    # Run benchmark and record statistics $ITERS times
    for i in $(eval echo "{1..$ITERS}")
    do
        WATCHED_PID=$({ ./$dir.do  > $dir-do.out 2>> $dir-do.results & } && echo $!);
        while [[ ${?} == 0 ]]  # Wait for WATCHED_PID to complete
        do
            sleep $SLEEP         
            ps -q $WATCHED_PID >/dev/null  
        done
    done
    mv mem.out $dir-do.mem

    # Create results files for recording benchmark statistics for encrypted (enc) mode
    echo $dir " MODE=enc " > $dir-enc.results
    echo -e "[VIP]\t Runtime (ms)\t Runtime (us)\t Processor Cycles\t Instruction count\n" >> $dir-enc.results
    echo $dir " MODE=enc " > mem.out
    echo -e "pid vsz rss\n" >> mem.out
    # Run benchmark and record statistics $ITERS times
    for i in $(eval echo "{1..$ITERS}")
    do
        WATCHED_PID=$({ ./$dir.enc  > $dir-enc.out 2>> $dir-enc.results & } && echo $!);
        while [[ ${?} == 0 ]]  # Wait for WATCHED_PID to complete
        do
            sleep $SLEEP         
            ps -q $WATCHED_PID >/dev/null  
        done
    done
    mv mem.out $dir-enc.mem

    cd $curDIR
    # Print aggregate statistics for all modes of this benchmark
    python3 aggregate-stats.py $dir $ITERS

done


#special mnist varient
dir="mnist-cnn"
    cd $dir/bin
    cp ../mnist-cnn.kan .
    cp ../mnist-test-x.knd .

    # Create results files for recording benchmark statistics for native (na) mode
    echo $dir " MODE=na " > $dir-na.results
    echo -e "[VIP]\t Runtime (ms)\t Runtime (us)\t Processor Cycles\t Instruction count\n" >> $dir-na.results
    echo $dir " MODE=na " > mem.out
    echo -e "pid vsz rss\n" >> mem.out
    # Run benchmark and record statistics $ITERS times
    for i in $(eval echo "{1..$ITERS}")
    do
        WATCHED_PID=$({ ./$dir.na -i mnist-cnn.kan mnist-test-x.knd  > $dir-na.out 2>> $dir-na.results & } && echo $!);
        while [[ ${?} == 0 ]]  # Wait for WATCHED_PID to complete
        do
            sleep $SLEEP         
            ps -q $WATCHED_PID >/dev/null  
        done
    done
    mv mem.out $dir-na.mem


    # Create results files for recording benchmark statistics for data-oblivious (do) mode
    echo $dir " MODE=do " > $dir-do.results
    echo -e "[VIP]\t Runtime (ms)\t Runtime (us)\t Processor Cycles\t Instruction count\n" >> $dir-do.results
    echo $dir " MODE=do " > mem.out
    echo -e "pid vsz rss\n" >> mem.out
    # Run benchmark and record statistics $ITERS times
    for i in $(eval echo "{1..$ITERS}")
    do
        WATCHED_PID=$({ ./$dir.do -i mnist-cnn.kan mnist-test-x.knd  > $dir-do.out 2>> $dir-do.results & } && echo $!);
        while [[ ${?} == 0 ]]  # Wait for WATCHED_PID to complete
        do
            sleep $SLEEP         
            ps -q $WATCHED_PID >/dev/null  
        done
    done
    mv mem.out $dir-do.mem

    # Create results files for recording benchmark statistics for encrypted (enc) mode
    echo $dir " MODE=enc " > $dir-enc.results
    echo -e "[VIP]\t Runtime (ms)\t Runtime (us)\t Processor Cycles\t Instruction count\n" >> $dir-enc.results
    echo $dir " MODE=enc " > $dir-enc.mem
    echo -e "pid vsz rss\n" >> $dir-enc.mem
    # Run benchmark and record statistics $ITERS times
    for i in $(eval echo "{1..$ITERS}")
    do
        WATCHED_PID=$({ ./$dir.enc -i mnist-cnn.kan mnist-test-x.knd  > $dir-enc.out 2>> $dir-enc.results & } && echo $!);
        while [[ ${?} == 0 ]]  # Wait for WATCHED_PID to complete
        do
            sleep $SLEEP         
            ps -q $WATCHED_PID >/dev/null  
        done
    done
    mv mem.out $dir-enc.mem

    cd $curDIR
    # Print aggregate statistics for all modes of this benchmark
    python3 aggregate-stats.py $dir $ITERS
