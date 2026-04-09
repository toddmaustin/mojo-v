
import sys
import re

# Aggregate stats for benchmark MODE=mode 
def aggregate( dir, iters, mode ):
    # Parse results file
    fp = open(dir+'/bin/'+dir+'-'+mode+'.results', 'r')
    line = fp.readline() # Skip header
    line = fp.readline() # Skip header
    line = fp.readline() # Skip newline
    line = fp.readline()
    runtime_ms = 0
    runtime_us = 0
    cycles = 0
    insns = 0
    while line:
        values = line.split('\t')
        runtime_ms += float(values[1].strip())
        runtime_us += float(values[2].strip())
        cycles += float(values[3].strip())
        insns += float(values[4].strip())
        line = fp.readline()
    fp.close()

    # Parse mem file
    fp = open(dir+'/bin/'+dir+'-'+mode+'.mem', 'r')
    line = fp.readline() # Skip header
    line = fp.readline() # Skip header
    line = fp.readline() # Skip newline
    line = fp.readline()
    total_cnt = 0
    max_vsz = 0
    vsz = 0
    rss = 0
    while line:
        line = re.sub('\s+',' ',line) # Clean up ps prints
        line = line.lstrip() #remove leading whitespace
        values = line.split(' ')
        if "VmPeak" in line:
            max_vsz += int(values[1].strip())
            total_cnt += 1
        elif "VmSize" in line:
            vsz += int(values[1].strip())
        elif "VmRSS" in line: 
            rss += int(values[1].strip())

        line = fp.readline()
    fp.close()
    
    if(total_cnt != iters):
        print("WARNING:: Too few memory measurments (MODE="+mode+"):: {0}".format(total_cnt))

    # Average results over iters
    runtime_ms = runtime_ms/iters
    runtime_us = runtime_us/iters
    cycles = cycles/iters
    insns = insns/iters
    vsz = vsz/total_cnt
    rss = rss/total_cnt
    max_vsz = max_vsz/total_cnt
    # print results in table format
    print(dir + '-MODE=' + mode, end='')
    print(",\t{0},\t{1}".format(runtime_ms, runtime_us), end='')
    print(",\t{0},\t{1}".format(cycles, insns), end='')
    print(",\t{0},\t{1}".format(max_vsz, rss), end='')
    print(",\t{0}".format(vsz), end='\n')
    # print("\t{0}\t{1}\t{2}\t{3}".format(runtime_ms, runtime_us, cycles, insns))
    # print('\t'+ runtime_ms +'\t'+ runtime_us +'\t'+ cycles +'\t'+ insns, end='')
    # print('\t'+ vsz +'\t'+ rss +'\n', end='')




dir = sys.argv[1]
iters = float(sys.argv[2])
aggregate(dir, iters, "na")
aggregate(dir, iters, "do")
aggregate(dir, iters, "enc")
