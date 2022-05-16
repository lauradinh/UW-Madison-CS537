#!/bin/bash
TEST_DIR=/p/course/cs537-swift/tests/p3a
INPUT_DIR=${TEST_DIR}/inputs
OUTPUT_DIR=${TEST_DIR}/outputs

#flags
continue_flag=1
delete_flag=0
test_select=0
timeout_flag=0
perf_flag=0

#results
test_cnt=0
test_passed=0
check_output=1

timeouts=(
10
10
30
30
60
60
120
60
10
10
60
180
180
180
180
180
10
30
10
30
120
300
300
300
120
300
300
300
30
30
)

perf_timeouts=(
1
1
5
5
8
5
10
3
1
1
5
5
5
5
3
10
1
8
1
8
10
90
90
90
10
30
90
90
10
10
10
10
90
90
90
90
)

function check_mr_files {
	if [ ! -f mapreduce.c ]; then
	    echo "mapreduce.c file not found!"
	    exit 1
	fi
	if [ ! -f hashmap.c ]; then
	    echo "hashmap.c file not found!"
	    exit 1
	fi

}

function hashmap_perf() {
    local ts=()
    test_cnt=$(( $test_cnt + 1 ))
	echo "[Test${test_cnt} - hashmap performance test"
    for i in {1,3,4}; do
        local exec=hashmap_perf_test_$i
        if ! gcc hashmap_perf_test.c -O2 -DN_READ_THREADS=$i hashmap.c -o $exec -lm -pthread; then
            echo "  Failed to compile: [gcc hashmap_perf_test.c -O2 -DN_READ_THREADS=$i hashmap.c -o hashmap_perf_test_$i -lm -pthread]"
            return 1
        fi
        local out=$(./${exec})
        if [[ $? -ne 0 ]]; then
            echo "Wrong output or error in ./${exec}"
            return 1
        fi
        ts+=($out)
    done
    local one_thread=${ts[0]}
    local mult_thread=${ts[1]}
    local four_thread=${ts[2]}
    if [[ $(echo "$mult_thread > $four_thread" | bc -l) -eq 1 ]]; then
        mult_thread=$four_thread
    fi
    local expected_ts="0.17"
    if [[ $(echo "$mult_thread < $expected_ts" | bc -l) -eq 1 ]]; then
        test_passed=$((test_passed+1))
        echo "Hashmap read perf test passed. Reads took: $mult_thread sec beating the expected time of $expected_ts sec"
    elif [[ $(echo "$mult_thread >= $one_thread" | bc -l) -eq 1 ]]; then
        echo "Multithread read on hashmap is slower than single thread. Are you using r/w locks correctly?"
        return 1
    else
        echo "Multithread read on hashmap is faster than single thread, but still slow overall. Took $mult_thread sec, while exepected was < $expected_ts sec"
        return 1
    fi
    echo "[Test${test_cnt} Success]"
}

function hashmap_single_run {
	test_cnt=$((test_cnt+1))

	# Args
	n_mappers=${1}
	nreducers=${2}
	n_files=${3}
	args=${4}
	filedesc=${5}

	long_desc="Hashmap ${n_files} file(s) with ${n_mappers} mapper(s) and ${n_reducers} reducer(s) -- ${6}"
	test_name=${7}
    search_word=${8}
    expected_out=${9}

	# Local vars
	test_suffix="f${3}m${1}r${2}_t${test_cnt}"
	executable="${test_name}_m${1}r${2}"

	echo "[Test${test_cnt} - ${long_desc}]"
	echo "(${filedesc})"
	if [ ${test_select} -gt 0 ]; then
	    if [ ${test_select} -ne ${test_cnt} ]; then
			echo "[Skip Test${test_cnt}]"
			echo
			return 0
	    fi
	fi
	echo "  Running \$./${executable} ${args}"

	# Compile
	if ! gcc -o ${executable} hashmap.c ${test_name}.c mapreduce.c -g -Wall -Werror -pthread -O -lm -DNUM_MAPPERS=${n_mappers} -DNUM_REDUCERS=${n_reducers} -DFILE_OUTPUT_SUFFIX="\"${test_suffix}\""; then
		echo "[Test${test_cnt} failed]"
    	echo "  Failed to build ${executable}"
    	echo
    	if [ ${continue_flag} -eq 1 ]; then
    		return 1
    	fi
    	exit 1
	fi

	# Build command string
	run_cmd="./${executable} ${args} ${search_word}"

    if [ ${perf_flag} -eq 1 ]; then
		timeout_secs=${perf_timeouts[${test_cnt}-1]}
		run_cmd="timeout ${timeout_secs} ${run_cmd}"
		echo "  INFO: Timeout for this testcase is ${timeout_secs} seconds (perf mode)"
    elif [ ${timeout_flag} -eq 1 ]; then
		timeout_secs=${timeouts[${test_cnt}-1]}
		run_cmd="timeout ${timeout_secs} ${run_cmd}"
		echo "  INFO: Timeout for this testcase is ${timeout_secs} seconds"
	else
        :
       # echo "  WARNING: Tests are running without time limits!"
	fi

	# Run program
    local out=$(eval ${run_cmd})
	if [[ $? -ne 0 ]]; then
		echo "[Test${test_cnt} failed]"
		echo "  Error occured while running ./${executable} ${args}"
		echo
		if [ ${continue_flag} -eq 1 ]; then
			return 1
		fi
		exit 1
	fi

    if [ "$out" != "$expected_out" ]; then
        echo "output mismatch for ${run_cmd} - $out vs $expected_out"
        return 1
    fi


    echo "[Test${test_cnt} Success]"
    echo ""
	test_passed=$((test_passed+1))

	# Delete executable on success
	if [ ${delete_flag} -eq 1 ]; then
		rm -f ./${executable}
		for i in `seq 0 $(expr $n_reducers - 1)`;
	        do
				#rm -f wordcount_f${3}m${1}r${2}_t${test_cnt}\($i\).out
				rm -f  ${test_name}_${test_suffix}\($i\).out
	        done
	fi
}

# ******************************************************************************
# wordcount(cc|cp|cp_cc).c
# ******************************************************************************
function wordcount_single_run {
	test_cnt=$((test_cnt+1))

	# Args
	n_mappers=${1}
	n_reducers=${2}
	n_files=${3}
	args=${4}
	filedesc=${5}

	long_desc="Wordcounting ${n_files} file(s) with ${n_mappers} mapper(s) and ${n_reducers} reducer(s) -- ${6}"
	test_name=${7}

	# Local vars
	test_suffix="f${3}m${1}r${2}_t${test_cnt}"
	executable="${test_name}_m${1}r${2}"

	echo "[Test${test_cnt} - ${long_desc}]"
	echo "(${filedesc})"
	if [ ${test_select} -gt 0 ]; then
	    if [ ${test_select} -ne ${test_cnt} ]; then
			echo "[Skip Test${test_cnt}]"
			echo
			return 0
	    fi
	fi
	echo "  Running \$./${executable} ${args}"

	# Compile
	if ! gcc -o ${executable} ${test_name}.c mapreduce.c -g -Wall -Werror -pthread -O -lm -DNUM_MAPPERS=${n_mappers} -DNUM_REDUCERS=${n_reducers} -DFILE_OUTPUT_SUFFIX="\"${test_suffix}\""; then
		echo "[Test${test_cnt} failed]"
    	echo "  Failed to build ${executable}"
    	echo
    	if [ ${continue_flag} -eq 1 ]; then
    		return 1
    	fi
    	exit 1
	fi

	# Build command string
	run_cmd="./${executable} ${args}"

    if [ ${perf_flag} -eq 1 ]; then
		timeout_secs=${perf_timeouts[${test_cnt}-1]}
		run_cmd="timeout ${timeout_secs} ${run_cmd}"
		echo "  INFO: Timeout for this testcase is ${timeout_secs} seconds (perf mode)"
    elif [ ${timeout_flag} -eq 1 ]; then
		timeout_secs=${timeouts[${test_cnt}-1]}
		run_cmd="timeout ${timeout_secs} ${run_cmd}"
		echo "  INFO: Timeout for this testcase is ${timeout_secs} seconds"
	else
        :
       # echo "  WARNING: Tests are running without time limits!"
	fi

	# Run program
	if ! eval ${run_cmd}; then
		echo "[Test${test_cnt} failed]"
		echo "  Error occured while running ./${executable} ${args}"
		echo
		if [ ${continue_flag} -eq 1 ]; then
			return 1
		fi
		exit 1
	fi

	for i in `seq 0 $(expr $n_reducers - 1)`;
	do
		out_filename=${test_name}_${test_suffix}\($i\).out
		# Sort output file
		#if ! LC_ALL=C sort -o wordcount_f${3}m${1}r${2}_t${test_cnt}\($i\).out wordcount_f${3}m${1}r${2}_t${test_cnt}\($i\).out; then
		if ! LC_ALL=C sort -o ${out_filename} ${out_filename}; then
			echo "[Test${test_cnt} failed]"
			echo "  The output file does not exist."

			if [ ${continue_flag} -eq 1 ]; then
				return 1
			fi
			exit 1
		fi

		if [ ${check_output} -eq 1 ]; then
			# Check output
			#if ! cmp --silent ${OUTPUT_DIR}/wordcount_f${3}m${1}r${2}_t${test_cnt}\($i\).out wordcount_f${3}m${1}r${2}_t${test_cnt}\($i\).out; then
			if ! cmp --silent ${OUTPUT_DIR}/${out_filename} ${out_filename}; then
				echo "[Test${test_cnt} failed]"
				echo "  The output is different from the expected output."
				echo "    - expected output path: ${OUTPUT_DIR}/${out_filename}"
				echo "    - actual output path: ${out_filename}"
				echo

				if [ ${continue_flag} -eq 1 ]; then
					return 1
				fi
				exit 1
			fi
		fi
	done

    echo "[Test${test_cnt} Success]"
    echo ""
	test_passed=$((test_passed+1))

	# Delete executable on success
	if [ ${delete_flag} -eq 1 ]; then
		rm -f ./${executable}
		for i in `seq 0 $(expr $n_reducers - 1)`;
	        do
				#rm -f wordcount_f${3}m${1}r${2}_t${test_cnt}\($i\).out
				rm -f  ${test_name}_${test_suffix}\($i\).out
	        done
	fi
}

# ******************************************************************************
# wordcount_multi(_cc).c
# ******************************************************************************
function wordcount_multi_run {
	test_cnt=$((test_cnt+1))

	# Args
	n_mappers=${1}
	n_reducers=${2}
	n_files=${3}
	args=${4}
	filedesc=${5}

	test_name=${7}
	n_iters=${8}

	# Local vars
	long_desc="Wordcounting iterates ${n_iters} times -- ${n_files} file(s) with ${n_mappers} mapper(s) and ${n_reducers} reducer(s) -- ${6}"
	test_suffix="f${3}m${1}r${2}_t${test_cnt}"
	executable="${test_name}_m${1}r${2}"

	echo "[Test${test_cnt} - ${long_desc}]"
	echo "(${filedesc})"
	if [ ${test_select} -gt 0 ]; then
	    if [ ${test_select} -ne ${test_cnt} ]; then
			echo "[Skip Test${test_cnt}]"
			echo
			return 0
	    fi
	fi
	echo "  Running \$./${executable} ${args}"

	# Compile
	if ! gcc -o ${executable} ${test_name}.c mapreduce.c -g -Wall -Werror -pthread -O -lm -DNUM_MAPPERS=${n_mappers} -DNUM_REDUCERS=${n_reducers} -DNUM_ITERATION=${n_iters} -DFILE_OUTPUT_SUFFIX="\"${test_suffix}\""; then
		echo "[Test${test_cnt} failed]"
    	echo "  Failed to build ${executable}"
    	echo
    	if [ ${continue_flag} -eq 1 ]; then
    		return 1
    	fi
    	exit 1
	fi

	# Build command string
	run_cmd="./${executable} ${args}"

    if [ ${perf_flag} -eq 1 ]; then
		timeout_secs=${perf_timeouts[${test_cnt}-1]}
		run_cmd="timeout ${timeout_secs} ${run_cmd}"
		echo "  INFO: Timeout for this testcase is ${timeout_secs} seconds (perf mode)"
    elif [ ${timeout_flag} -eq 1 ]; then
		timeout_secs=${timeouts[${test_cnt}-1]}
		run_cmd="timeout ${timeout_secs} ${run_cmd}"
		echo "  INFO: Timeout for this testcase is ${timeout_secs} seconds"
	else
        echo "  WARNING: Tests are running without time limits!"
	fi

	# Run program
	if ! eval ${run_cmd}; then
		echo "[Test${test_cnt} failed]"
		echo "  Error occured while running ./${executable} ${args}"
		echo
		if [ ${continue_flag} -eq 1 ]; then
			return 1
		fi
		exit 1
	fi

	for i in `seq 0 $(expr $n_reducers - 1)`;
	do
		for j in `seq 0 $(expr $n_iters - 1)`;
		do
			out_filename=${test_name}_${j}_${test_suffix}\($i\).out
			# Sort output file
			#if ! LC_ALL=C sort -o wordcount_f${3}m${1}r${2}_t${test_cnt}\($i\).out wordcount_f${3}m${1}r${2}_t${test_cnt}\($i\).out; then
			if ! LC_ALL=C sort -o ${out_filename} ${out_filename}; then
				echo "[Test${test_cnt} failed]"
				echo "  The output file does not exist."

				if [ ${continue_flag} -eq 1 ]; then
					return 1
				fi
				exit 1
			fi

			if [ ${check_output} -eq 1 ]; then
				# Check output
				#if ! cmp --silent ${OUTPUT_DIR}/wordcount_f${3}m${1}r${2}_t${test_cnt}\($i\).out wordcount_f${3}m${1}r${2}_t${test_cnt}\($i\).out; then
				if ! cmp --silent ${OUTPUT_DIR}/${out_filename} ${out_filename}; then
					echo "[Test${test_cnt} failed]"
					echo "  The output is different from the expected output."
					echo "    - expected output path: ${OUTPUT_DIR}/${out_filename}"
					echo "    - actual output path: ${out_filename}"
					echo

					if [ ${continue_flag} -eq 1 ]; then
						return 1
					fi
					exit 1
				fi
			fi
		done
	done

    echo "[Test${test_cnt} Success]"
    echo ""
	test_passed=$((test_passed+1))

	# Delete executable on success
	if [ ${delete_flag} -eq 1 ]; then
		rm -f ./${executable}
		for i in `seq 0 $(expr $n_reducers - 1)`;
		do
			for j in `seq 0 $(expr $n_iters - 1)`;
			do
				rm -f  ${test_name}_${j}_${test_suffix}\($i\).out
			done
		done
	fi
}


# ******************************************************************************
# Main Testing
# ******************************************************************************

# mode select
while [[ $# -gt 0 ]]
do
key="$1"
case $key in
    -t)
	test_select=$2
	shift
	shift
    ;;
    -c)
	continue_flag=1
	shift
    ;;
    -d)
	delete_flag=1
	shift
    ;;
    -o)
    timeout_flag=1
    shift
    ;;
	-p)
	perf_flag=1
	shift
	;;
esac
done

# actual script here
cp -f ${TEST_DIR}/mapreduce.h .
cp -f ${TEST_DIR}/wordcount.c .
#cp -f ${TEST_DIR}/wordcount_cc.c .
cp -f ${TEST_DIR}/wordcount_cp.c .
#cp -f ${TEST_DIR}/wordcount_cp_cc.c .
cp -f ${TEST_DIR}/wordcount_multi.c .
cp -f ${TEST_DIR}/hashmap_test.c .
cp -f ${TEST_DIR}/hashmap_perf_test.c .
check_mr_files

# *** wordcount ***
#wordcount_single_run num_map num_reduce num_file inputargs filedesc longdesc workload
wordcount_single_run 1 1 1 "${INPUT_DIR}/file1m.in" \
    "1mb file" "default partitioner" "wordcount"
wordcount_single_run 1 4 1 "${INPUT_DIR}/file1m.in" \
    "1mb file" "default partitioner" "wordcount"
wordcount_single_run 1 10 1 "${INPUT_DIR}/file10m.in" \
    "10mb file" "default partitioner" "wordcount"
wordcount_single_run 4 4 1 "${INPUT_DIR}/file100m.in" \
    "100mb file" "default partitioner" "wordcount"

# *** custom partition ***
#wordcount_single_run num_map num_reduce num_file inputargs filedesc longdesc workload_cp
wordcount_single_run 1 10 1 "${INPUT_DIR}/file1m.in" \
"1mb file" "custom partitioner" "wordcount_cp"
wordcount_single_run 10 50 1 "${INPUT_DIR}/file10m.in" \
"10mb file" "custom partitioner" "wordcount_cp"

wordcount_multi_run 1 1 1 "${INPUT_DIR}/file1m.in " \
    "1mb file * 2" "default partitioner" "wordcount_multi" 2

# hashmap
hashmap_single_run 2 3 2 "${INPUT_DIR}/file1m.in ${INPUT_DIR}/file1m.in" \
    "1mb file * 2" "default partitioner" "hashmap_test"  Nullam "Found Nullam 1286 times"

hashmap_single_run 3 4 3 "${INPUT_DIR}/file1m.in ${INPUT_DIR}/file1m.in ${INPUT_DIR}/file10m.in" \
    "1mb file * 2 + 10mb file" "default partitioner" "hashmap_test"  NotExists "Word not found!"

wordcount_single_run 4 4 6 "${INPUT_DIR}/file1m.in ${INPUT_DIR}/file1m.in ${INPUT_DIR}/file1m.in ${INPUT_DIR}/file1m.in ${INPUT_DIR}/file1m.in ${INPUT_DIR}/file1m.in" \
    "1mb file x 6" "default partitioner" "wordcount"

hashmap_perf

# *** RESULTS ***
echo ""
echo "Test Result [${test_passed}/${test_cnt}]"

if [ ${delete_flag} -eq 1 ]; then
	echo "Delete *.out files"
	rm -f *.out
#    echo "Delete useless *.c files"
#    rm -f sort.c wordcount.c wordcount_cp.c wordcount_multi.c wordcount_cc.c wordcount_cp_cc.c wordcount_multi_cc.c
fi

exit 0

