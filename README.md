 # Cache simulator in C

 ### Simulates cache memory on trace files
 ### (Valgrind memory trace output)

 #### Files:
  - `csim.c`: Source code
  - `README.md`: This file
  - `trace_files/`: Directory that holds sample trace files to test

 #### Sample trace output (fed into csim program):
 `L 10,4`
 `S 18,4`
 `L 20,4`
 `S 28,4`
 `M 50,4`
 where `L` stands for Load, `S` for Store, `M` for Modify, the first numerical value is the memory address in hex,
 and the second numerical value is the bytes (size). Each line begins with a single space.

  - Implements the Least Recently Used (LRU) replacement policy
  - Compiles into `csim` executable
  - `csim-linux` compiled executable version included as a [release](https://github.com/bryan22lee/cache_simulator/releases/tag/demo) (tag="demo"), available for [download](https://github.com/bryan22lee/cache_simulator/releases/tag/demo)
    - compiled on Ubuntu 20.04
    - works only on Linux

## Use
  - `./csim` or `./csim-linux` followed by the following non-optional flags:
    `-t <trace-file-name> -s <number-sets> -E <number-lines> -b <number-bytes>`
    where
     - `<trace-file-name>` is replaced with the path to the trace file, 
     - `<number-sets>` is replaced with the number of sets in the cache, 
     - `<number-line>` is replaced with the number of lines/blocks per set, and
     - `<number-bytes>` is replaced with the number of bytes per line/block.

**E.g.:** `./csim -t trace_files/simple.trace -s 5 -E 4 -b 10`
      <br/>or<br/>
      `./csim-linux -t trace_files/simple.trace -s 5 -E 4 -b 10` <br/>
      prints<br/>
      `hits:4 misses:1 evictions:0`
      <br/>

*I do not include cachelab files here (referenced by csim.c) for sake of only including my code
in the repo. Compile demonstration can be shown locally.*
