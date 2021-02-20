Project: "Memetic Algorithms for the Job Shop Scheduling Problem".
Paper with explanation: A Parallel Memetic Algorithm with Explicit Management of Diversity for the Job Shop Scheduling Problem (Yet unpublished)
Programmer: Oscar Hernández Constantino
Designers: Oscar Hernández Constantino, Carlos Segura

## General Description
This repository contains code that implements the MAEMD, HEA and TSPR methods described in the paper "A Parallel Memetic Algorithm with Explicit Management of Diversity for the Job Shop Scheduling Problem" by Oscar Hernández Constantino and Carlos Segura. Note that HEA and TSPR were existing methods, and the novel one is MAEMD. All of them make use of an already designed Tabu Search (TS). However, TS was also implemented from scratch. All the results presented in the paper were obtained with this code, by just changing the parameters.

## Instructions to compile:
Sequential Memetic Algorithms:
make TestAgS

Parallel Memetic Algorithms (It requires MPI):
make TestAg_MPI

## Instructions to execute:
Both TestAGs and TestAg_MPI receive 13 parameters. They are the following:
1) Instance file
2) LowerBound. It is used as an additional stopping criterion. When this value is reached, executions finish. The value 0 can be used to avoid this kind of stopping criterion.
3) Prefix for the output files.
4) Kind of replacement. The only documented one is the Best Nonpenalized approach, which is explained in the paper. For this kind use the number 2.
5) Kind of crossover. 0 for LCS, 1 for MCH, 2 for Path relinking.
6) Kind of metaheuristic. 0 for Memetic Algorithm with TS, 2 for HEA
7) Deprecated, just use 1.
8) Stopping criterion for the execution in seconds.
9) Stopping criterion for Tabu Search (iterations without improvement).
10) Deprecated, just use 1.
11) Kind of distance for the memetic algorithm. 0 for D_{LCS}, 1 for D_{Pos}, 2 for D_{Ham}
12) Deprecated, just use 1.
13) Population size

Example:
./TestAgS data/dmu20.txt 0 output/TestAGS_ 2 0 0 1 345600 100000 1 1 1 50
