# MipsPipelineSimulator
Mips Assembly simulator, demonstrating pipeline theories, implemented in C.

This simulator will show, one clock cycle after the other, the instructions that are being processed inside a Mips processor.

There are five stages inside: fetch, decode, execute, memory and writeback.

The simulator will process Assembly instructions imported from "trace1.txt".

There will also be three kinds of pipeline theories: forwarding, branch solution, combined.

On the command line you should give two input arguments:

1 0 - forwarding only

0 1 - branch solution only

1 1 - combined

0 0 - plain simple pipeline

in edition, the efficiency of every stage will be shown, by printing the CPI (cycles per instruction).

run mips_pipeline_simulator x x
