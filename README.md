Project: "Memetic Algorithms for the Job Shop Scheduling Problem".

Paper with explanation: A Parallel Memetic Algorithm with Explicit Management of Diversity for the Job Shop Scheduling Problem 

Programmer: Oscar Hernández Constantino

Designers: Oscar Hernández Constantino, Carlos Segura

Please cite as follows:

@Article{Hernandez:21,

author={Hern{\'a}ndez-Constantino, Oscar and Segura, Carlos},

title={A parallel memetic algorithm with explicit management of diversity for the job shop scheduling problem},

journal={Applied Intelligence},

year={2021},

month={Apr},

day={24},

abstract={The job shop scheduling problem (JSSP) is a very popular NP-hard optimization problem that involves assigning jobs to resources. Recent advances in the field of memetic algorithms show that explicitly managing the diversity of the population by taking into account the stopping criterion with the aim of dynamically adapting the balance between exploration and exploitation is key to their success. This is especially the case in long-term executions. However, this design principle has not yet been applied to the JSSP. This paper proposes a novel memetic algorithm that integrates some of the most advanced components devised in the literature for the JSSP with a replacement strategy that explicitly manages the diversity by considering a novel dissimilarity measure. To properly address large instances, a parallel master-worker model is used. Experimental validation shows the important advances attained by our proposal when compared to two state-of-the-art optimizers. The advantages are clear in both sequential and parallel cases, with more impressive achievements appearing in the parallel case. The parallel proposal has yielded new best-known solutions in 30 well-known JSSP instances, matching the lower bound in two of them, meaning that at least two new optimal solutions have been discovered.},

issn={1573-7497},

doi={10.1007/s10489-021-02406-2},

url={https://doi.org/10.1007/s10489-021-02406-2}

}



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
./TestAgS instances/dmu20.txt 0 output/TestAGS_ 2 0 0 1 345600 100000 1 1 1 50
