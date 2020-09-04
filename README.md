 # Cache simulator in C

 ### Simulates cache memory on trace files

 #### Files:
  - `csim.c`: Source code
  - `README.md`: This file

 #### Sample trace output (fed into csim program):
 `L 10,4`
 `S 18,4`
 `L 20,4`
 `S 28,4`
 `S 50,4`
 where `L` stands for Load, `S` stands for Store, the first numerical value is the memory address in hex,
 and the second numerical value is the bytes (size). Each line begins with a single space.

  - Implements the Least Recently Used (LRU) replacement policy
  - Compiles into `csim` executable
  - `csim-linux` compiled executable version included as a release (compiled on Ubuntu 20.04)

*I do not include cachelab.c, cachelab.h, or the sample trace files here for sake of only including my code
in the repo. Demonstration can be made and shown locally.*
