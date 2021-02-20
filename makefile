CCOMP=g++
CLINK=g++
CPP_FLAGS=-std=c++11 -lm

#Flags valgrind
	
CCFLAGS=-std=c++0x -lm
VFLAGS=--leak-check=full --show-leak-kinds=all

SRC_DIR=src

SRC_BASE=JobShop.cpp Operacion.cpp Movimiento.cpp GJobShop.cpp Solucion.cpp UtilJS.cpp TabuSearch.cpp IterativeSearch.cpp

SRC_TEST=Main.cpp $(SRC_BASE) 

SRC_AG=TestAG_MPI.cpp $(SRC_BASE) AlgoritmoGenetico.cpp Diversidad.cpp PathRelinking.cpp HEA.cpp

SRC_AG_MPI=TestAG_MPI.cpp $(SRC_BASE) AlgoritmoGenetico.cpp Diversidad.cpp PathRelinking.cpp HEA.cpp UtilMPI.cpp

SRC_PR=TestPR.cpp $(SRC_BASE) AlgoritmoGenetico.cpp Diversidad.cpp PathRelinking.cpp
 
NUM_PROCESADORES=2
HOST_FILE=hostfiles/host_file

PREFIX_RUN=mpirun.openmpi -n $(NUM_PROCESADORES) -hostfile $(HOST_FILE) 
#valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all
#I_COMPILE=TestAgP
I_COMPILE=TestAg_MPI


# 0: Utiliza rutas absolutas para el cluster, 1: utiliza rutas relativas
EJECUCION_LOCAL=1

# Tiempo en Segundos
TIEMPO_EJEC=9000

# Tipo de Busqueda Local
# 0: Tabu Search, 1: Iterative Search, 5: Escalada en N5, 7: Escalada en N7
# TIPO AG: 0 AG, 1: PRTS, 2: HEA
TIPO_BUSQUEDA=0

MAX_SIN_MEJORA=12000


EVAL_EXACT=0
CALC_COTS=1
SOLO_FACT=1
PARAMS_EVAL=$(EVAL_EXACT) $(CALC_COTS) $(SOLO_FACT)

# Porcentaje del tiempo para decrecen a 0 el Fact de Penalizacion
POR_TM_FP=0 

# Porcentaje de tiempo para la poblacion inicial
POR_TM_POBINI=0 

# Factor para ajustar intensificacion
AJUSTE_INTEN=1
#TIPO_REPACION=2

#PARAMS_AG=$(POR_TM_FP) $(POR_TM_POBINI) $(AJUSTE_INTEN)

# Porcentaja para calcular D inicial en AGTS
POR_D_INI=1

# TIpo de Medida para calcular Diversidad. 0 ->similitud, 1->distancia entre pos, 2->num diferencias
TIPO_DIVERSIDAD=0

TAM_POB=30

PARAMS_AG=$(POR_D_INI) $(TIPO_DIVERSIDAD) $(AJUSTE_INTEN) $(TAM_POB)

FILE_SOL_INI=
TIPO_FILE=

PARAMS_MAIN=$(PARAMS_EVAL) $(FILE_SOL_INI) $(TIPO_FILE)

# TIPOS DE SELECCION:
# 0: Generacional, 1: reemplazo de los peores, 2: Mejor no penalizado
TIPO_SELECC=2

# TIPOS DE CRUZA:
# 0: por subcadena mas larga, 1: por maquinas ; 3: por orden ops
TIPO_CRUZA=0

# ############# PARAMETROS PARA LA EJECUCION
ifeq ($(I_COMPILE), Main)
	PARAMS_EJEC=$(TIPO_SELECC) $(TIPO_CRUZA) $(TIPO_BUSQUEDA) $(EJECUCION_LOCAL) $(TIEMPO_EJEC) $(MAX_SIN_MEJORA) $(PARAMS_MAIN)
else
	PARAMS_EJEC=$(TIPO_SELECC) $(TIPO_CRUZA) $(TIPO_BUSQUEDA) $(EJECUCION_LOCAL) $(TIEMPO_EJEC) $(MAX_SIN_MEJORA) $(PARAMS_AG)
endif

DEPS_BASE=$(addprefix $(SRC_DIR)/, $(SRC_BASE:.cpp=.hpp) SolucionN5.cpp SolucionN7.cpp SolucionN1.cpp )

Main: $(addprefix $(SRC_DIR)/, $(SRC_TEST) ) $(DEPS_BASE)
	$(CCOMP) -O2 -o $@ $(addprefix $(SRC_DIR)/, $(SRC_TEST) ) $(CPP_FLAGS) 

TestAgP: $(addprefix $(SRC_DIR)/, $(SRC_AG) ) $(DEPS_BASE)
	$(CCOMP) -O2 -o $@ $(addprefix $(SRC_DIR)/, $(SRC_AG) ) $(CPP_FLAGS)

TestAgS: $(addprefix $(SRC_DIR)/, $(SRC_AG) ) $(DEPS_BASE)
	$(CCOMP) -O2 -o $@ $(addprefix $(SRC_DIR)/, $(SRC_AG) ) $(CPP_FLAGS)

TestAg_MPI: $(addprefix $(SRC_DIR)/, $(SRC_AG_MPI)) $(DEPS_BASE) 
	mpiCC.openmpi -O2 -DAG_PARALLEL -o $@ $(addprefix $(SRC_DIR)/, $(SRC_AG_MPI) ) $(CPP_FLAGS)

TestPR: $(addprefix $(SRC_DIR)/, $(SRC_PR) ) $(DEPS_BASE)
	$(CCOMP) -O2 -o $@ $(addprefix $(SRC_DIR)/, $(SRC_PR) ) $(CPP_FLAGS)

valgrind: clean
	@echo "************ VALGRAIND  ************ \n"
	$(CCOMP) -o Main $(addprefix $(SRC_DIR)/, $(SRC_TEST) ) -g $(CFLAGS) 
	valgrind  --leak-check=full ./Main data/test01.txt 0 output/$(I_COMPILE)_
	rm -f ./Main 

test_gprof: $(addprefix $(SRC_DIR)/, $(SRC_TEST) )
	$(CCOMP) 	-pg -o $@ $^ $(CPP_FLAGS)

clean:
	rm -f ./$(I_COMPILE)

######### RESULTADOS ##################

SRC_RES=Resultados.cpp UtilJS.cpp

Resultados: $(addprefix $(SRC_DIR)/, $(SRC_RES) )
	rm -f $@
	$(CCOMP) -O2 -o $@ $^ $(CPP_FLAGS) 

SRC_RES_AG=ResultadosAG.cpp UtilJS.cpp

ResultadosAG: $(addprefix $(SRC_DIR)/, $(SRC_RES_AG) )
	$(CCOMP) -O2 -o $@ $^ $(CPP_FLAGS) 
	./$@ instancias.txt experimentos/ AG48_
	
	rm -f $@

SRC_RES_POB=UtilJS.cpp TabuSearch.cpp AlgoritmoGenetico.cpp Diversidad.cpp Movimiento.cpp Operacion.cpp JobShop.cpp GJobShop.cpp Solucion.cpp PathRelinking.cpp IterativeSearch.cpp

ProcesarPoblacion: $(addprefix $(SRC_DIR)/, ProcesarPoblacion.cpp $(SRC_RES_POB) )
	$(CCOMP) -O2 -o $@ $^ $(CPP_FLAGS)

procesarPob_dmu80: ProcesarPoblacion
	./ProcesarPoblacion data/dmu80.txt 0 AgTS2d_48k__1549939538_pobs.txt dmu80_solOpt.txt 1


ProcesarSolucion: $(addprefix $(SRC_DIR)/, ProcesarSolucion.cpp $(SRC_RES_POB) )
	$(CCOMP) -O2 -o $@ $^ $(CPP_FLAGS)

procesarSol_dmu80: ProcesarSolucion
	./ProcesarSolucion data/dmu80.txt 6638 dmu80_solOpt.txt 1 sol_dmu80.txt

# ################ TEST ABZ ##############################

# best 1234 [1234]
abz5: $(I_COMPILE) data/abz5.txt
	./$(I_COMPILE) data/abz5.txt 1234 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best 943 [943]
abz6: $(I_COMPILE) data/abz6.txt
	./$(I_COMPILE) data/abz6.txt 943 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best 656 [673]
abz7: $(I_COMPILE) data/abz7.txt
	./$(I_COMPILE) data/abz7.txt 656 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best 648 (665) [683]
abz8: $(I_COMPILE) data/abz8.txt
	./$(I_COMPILE) data/abz8.txt 648 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best 678 [700]
abz9: $(I_COMPILE) data/abz9.txt
	./$(I_COMPILE) data/abz9.txt 678 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# ################ TEST FT ##############################

# best 55 [55]
ft06: $(I_COMPILE) data/ft06.txt
	./$(I_COMPILE) data/ft06.txt 55 output/$(I_COMPILE)_ $(PARAMS_EJEC)

#AG_ft06: TestAg data/ft06.txt
#	./TestAg data/ft06.txt 55 output/$(I_COMPILE)_

# best: 930 [936] >>>>>>>>>>>>>>>
ft10: $(I_COMPILE) data/ft10.txt
	./$(I_COMPILE) data/ft10.txt 930 output/$(I_COMPILE)_ $(PARAMS_EJEC)

#AG_ft10: TestAg data/ft10.txt
#	./TestAg data/ft10.txt 930 output/$(I_COMPILE)_

# best: 1165 [1165]
ft20: $(I_COMPILE) data/ft20.txt
	./$(I_COMPILE) data/ft20.txt 1165 output/$(I_COMPILE)_ $(PARAMS_EJEC)

#AG_ft20: TestAg data/ft20.txt
#	./TestAg data/ft20.txt 1165 output/$(I_COMPILE)_

# ################ TEST ORB ##############################

# best: 1059 [1060]
orb01: $(I_COMPILE) data/orb01.txt
	./$(I_COMPILE) data/orb01.txt 1059 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 888 [888]
orb02: $(I_COMPILE) data/orb02.txt
	./$(I_COMPILE) data/orb02.txt 888 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1005 [1005]
orb03: $(I_COMPILE) data/orb03.txt
	./$(I_COMPILE) data/orb03.txt 1005 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1005 [1012]
orb04: $(I_COMPILE) data/orb04.txt
	./$(I_COMPILE) data/orb04.txt 1005 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 887 [889]
orb05: $(I_COMPILE) data/orb05.txt
	./$(I_COMPILE) data/orb05.txt 887 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1010 [1012]
orb06: $(I_COMPILE) data/orb06.txt
	./$(I_COMPILE) data/orb06.txt 1010 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 397 [397]
orb07: $(I_COMPILE) data/orb07.txt
	./$(I_COMPILE) data/orb07.txt 397 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 899 [899]
orb08: $(I_COMPILE) data/orb08.txt
	./$(I_COMPILE) data/orb08.txt 899 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 934 [934]
orb09: $(I_COMPILE) data/orb09.txt
	./$(I_COMPILE) data/orb09.txt 934 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 944 [944]
orb10: $(I_COMPILE) data/orb10.txt
	./$(I_COMPILE) data/orb10.txt 944 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# ################ TEST SWV ##############################
# best: 1407 [1449]
swv01: $(I_COMPILE) data/swv01.txt
	#valgrind $(VFLAGS) 
	./$(I_COMPILE) data/swv01.txt 1407 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1475 [1499]
swv02: $(I_COMPILE) data/swv02.txt
	./$(I_COMPILE) data/swv02.txt 1475 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1398 [1451]
swv03: $(I_COMPILE) data/swv03.txt
	./$(I_COMPILE) data/swv03.txt 1398 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1464 [1508]
swv04: $(I_COMPILE) data/swv04.txt
	./$(I_COMPILE) data/swv04.txt 1464 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1424
swv05: $(I_COMPILE) data/swv05.txt
	./$(I_COMPILE) data/swv05.txt 1424 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1671
swv06: $(I_COMPILE) data/swv06.txt
	./$(I_COMPILE) data/swv06.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1594
swv07: $(I_COMPILE) data/swv07.txt
	./$(I_COMPILE) data/swv07.txt 1594 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1752
swv08: $(I_COMPILE) data/swv08.txt
	./$(I_COMPILE) data/swv08.txt 1752 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1655
swv09: $(I_COMPILE) data/swv09.txt
	./$(I_COMPILE) data/swv09.txt 1655 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1743
swv10: $(I_COMPILE) data/swv10.txt
	./$(I_COMPILE) data/swv10.txt 1743 output/$(I_COMPILE)_ $(PARAMS_EJEC)

swv11: $(I_COMPILE) data/swv11.txt
	./$(I_COMPILE) data/swv11.txt 2983 output/$(I_COMPILE)_ $(PARAMS_EJEC)

swv12: $(I_COMPILE) data/swv12.txt
	./$(I_COMPILE) data/swv12.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

swv13: $(I_COMPILE) data/swv13.txt
	./$(I_COMPILE) data/swv13.txt 3104 output/$(I_COMPILE)_ $(PARAMS_EJEC)

swv14: $(I_COMPILE) data/swv14.txt
	./$(I_COMPILE) data/swv14.txt 2968 output/$(I_COMPILE)_ $(PARAMS_EJEC)

swv15: $(I_COMPILE) data/swv15.txt
	./$(I_COMPILE) data/swv15.txt 2885 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# ################ TEST LA ##############################

# best: 666 [666]
la01: $(I_COMPILE) data/la01.txt
	./$(I_COMPILE) data/la01.txt 666 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 655 [655]
la02: $(I_COMPILE) data/la02.txt
	./$(I_COMPILE) data/la02.txt 655 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 597 [597]
la03: $(I_COMPILE) data/la03.txt 
	./$(I_COMPILE) data/la03.txt 597 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 590 [590]
la04: $(I_COMPILE) data/la04.txt 
	./$(I_COMPILE) data/la04.txt 590 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 593 [593]
la05: $(I_COMPILE) data/la05.txt
	./$(I_COMPILE) data/la05.txt 593 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 926 [926]
la06: $(I_COMPILE) data/la06.txt
	./$(I_COMPILE) data/la06.txt 926 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 890 [890]
la07: $(I_COMPILE) data/la07.txt
	./$(I_COMPILE) data/la07.txt 890 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 863 [863]
la08: $(I_COMPILE) data/la08.txt
	./$(I_COMPILE) data/la08.txt 863 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 951 [951]
la09: $(I_COMPILE) data/la09.txt
	./$(I_COMPILE) data/la09.txt 951 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 958 [958]
la10: $(I_COMPILE) data/la10.txt
	./$(I_COMPILE) data/la10.txt 958 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1222 [1222]
la11: $(I_COMPILE) data/la11.txt
	./$(I_COMPILE) data/la11.txt 1222 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1039 [1039]
la12: $(I_COMPILE) data/la12.txt
	./$(I_COMPILE) data/la12.txt 1039 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1150 [1150]
la13: $(I_COMPILE) data/la13.txt
	./$(I_COMPILE) data/la13.txt 1150 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1292 [1292]
la14: $(I_COMPILE) data/la14.txt
	./$(I_COMPILE) data/la14.txt 1292 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1207 [1207]
la15: $(I_COMPILE) data/la15.txt
	./$(I_COMPILE) data/la15.txt 1207 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 945 [945] >>>>>>>>>>>>>>>
la16: $(I_COMPILE) data/la16.txt
	./$(I_COMPILE) data/la16.txt 945 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 784 [784]
la17: $(I_COMPILE) data/la17.txt
	./$(I_COMPILE) data/la17.txt 784 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 848 [848]
la18: $(I_COMPILE) data/la18.txt
	./$(I_COMPILE) data/la18.txt 848 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 842 [842] 
la19: $(I_COMPILE) data/la19.txt
	./$(I_COMPILE) data/la19.txt 842 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 902 [902] >>>>>>>>>>>>>>>
la20: $(I_COMPILE) data/la20.txt
	./$(I_COMPILE) data/la20.txt 902 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1046 [1054] >>>>>>>>>>>>>>>
la21: $(I_COMPILE) data/la21.txt
	./$(I_COMPILE) data/la21.txt 1046 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 927 [933]
la22: $(I_COMPILE) data/la22.txt
	./$(I_COMPILE) data/la22.txt 927 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1032 [1032]
la23: $(I_COMPILE) data/la23.txt
	./$(I_COMPILE) data/la23.txt 1032 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 935 [947]
la24: $(I_COMPILE) data/la24.txt
	./$(I_COMPILE) data/la24.txt 935 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 977 [984]
la25: $(I_COMPILE) data/la25.txt
	./$(I_COMPILE) data/la25.txt 977 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1218 [1218]
la26: $(I_COMPILE) data/la26.txt
	./$(I_COMPILE) data/la26.txt 1218 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1235 [1252]
la27: $(I_COMPILE) data/la27.txt
	./$(I_COMPILE) data/la27.txt 1235 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1216 [1216]
la28: $(I_COMPILE) data/la28.txt
	./$(I_COMPILE) data/la28.txt 1216 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1152 [1186]
la29: $(I_COMPILE) data/la29.txt
	./$(I_COMPILE) data/la29.txt 1152 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1355 [1355]
la30: $(I_COMPILE) data/la30.txt 
	./$(I_COMPILE) data/la30.txt 1355 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# 
la32: $(I_COMPILE) data/la32.txt 
	./$(I_COMPILE) data/la32.txt 1850 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# 
la33: $(I_COMPILE) data/la33.txt 
	./$(I_COMPILE) data/la33.txt 1719 output/$(I_COMPILE)_ $(PARAMS_EJEC)

la37: $(I_COMPILE) data/la37.txt 
	./$(I_COMPILE) data/la37.txt 1397 output/$(I_COMPILE)_ $(PARAMS_EJEC)

la38: $(I_COMPILE) data/la38.txt 
	./$(I_COMPILE) data/la38.txt 1196 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# ################ TEST TA ##############################

# best: 1231 [1249] 
ta01: $(I_COMPILE) data/ta01.txt
	./$(I_COMPILE) data/ta01.txt 1231 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1244 [1256] 
ta02: $(I_COMPILE) data/ta02.txt
	./$(I_COMPILE) data/ta02.txt 1244 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1218 [1234] 
ta03: $(I_COMPILE) data/ta03.txt
	./$(I_COMPILE) data/ta03.txt 1218 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1175 [1198]
ta04: $(I_COMPILE) data/ta04.txt
	./$(I_COMPILE) data/ta04.txt 1175 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1224 [1241]
ta05: $(I_COMPILE) data/ta05.txt 
	./$(I_COMPILE) data/ta05.txt 1224 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1238 [1267]
ta06: $(I_COMPILE) data/ta06.txt
	./$(I_COMPILE) data/ta06.txt 1238 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1227 [1239]
ta07: $(I_COMPILE) data/ta07.txt
	./$(I_COMPILE) data/ta07.txt 1227 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1217 [1239]
ta08: $(I_COMPILE) data/ta08.txt
	./$(I_COMPILE) data/ta08.txt 1217 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1274 [1309]
ta09: $(I_COMPILE) data/ta09.txt
	./$(I_COMPILE) data/ta09.txt 1274 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1241 [1305]
ta10: $(I_COMPILE) data/la10.txt
	./$(I_COMPILE) data/ta10.txt 1241 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1357 [1413]
ta11: $(I_COMPILE) data/ta11.txt
	./$(I_COMPILE) data/ta11.txt 1357 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1367 [1421]
ta12: $(I_COMPILE) data/ta12.txt
	./$(I_COMPILE) data/ta12.txt 1367 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1342 [1413]
ta13: $(I_COMPILE) data/ta13.txt
	./$(I_COMPILE) data/ta13.txt 1342 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1345 [1369]
ta14: $(I_COMPILE) data/ta14.txt
	./$(I_COMPILE) data/ta14.txt 1345 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1339 [1378]
ta15: $(I_COMPILE) data/ta15.txt
	./$(I_COMPILE) data/ta15.txt 1339 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1360 [1428]
ta16: $(I_COMPILE) data/ta16.txt
	./$(I_COMPILE) data/ta16.txt 1360 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1462 [1510]
ta17: $(I_COMPILE) data/ta17.txt
	./$(I_COMPILE) data/ta17.txt 1462 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1396 [1458]
ta18: $(I_COMPILE) data/ta18.txt
	./$(I_COMPILE) data/ta18.txt 1396 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1332 [1398]
ta19: $(I_COMPILE) data/ta19.txt
	./$(I_COMPILE) data/ta19.txt 1332 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 1348 [1406]
ta20: $(I_COMPILE) data/ta20.txt
	./$(I_COMPILE) data/ta20.txt 1348 output/$(I_COMPILE)_ $(PARAMS_EJEC)

ta26: $(I_COMPILE) data/ta26.txt
	./$(I_COMPILE) data/ta26.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

ta34: $(I_COMPILE) data/ta34.txt
	./$(I_COMPILE) data/ta34.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

ta40: $(I_COMPILE) data/ta40.txt
	./$(I_COMPILE) data/ta40.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

ta42: $(I_COMPILE) data/ta42.txt
	./$(I_COMPILE) data/ta42.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# 
ta44: $(I_COMPILE) data/ta44.txt
	./$(I_COMPILE) data/ta44.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# 
ta48: $(I_COMPILE) data/ta48.txt
	./$(I_COMPILE) data/ta48.txt 1912 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# 
ta57: $(I_COMPILE) data/ta57.txt
	./$(I_COMPILE) data/ta57.txt 2943 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# 
ta75: $(I_COMPILE) data/ta75.txt
	./$(I_COMPILE) data/ta75.txt 5392 output/$(I_COMPILE)_ $(PARAMS_EJEC)

ta77: $(I_COMPILE) data/ta77.txt
	./$(I_COMPILE) data/ta77.txt 5436 output/$(I_COMPILE)_ $(PARAMS_EJEC)

ta80: $(I_COMPILE) data/ta80.txt
	./$(I_COMPILE) data/ta80.txt 5183 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# ################ TEST YN ##############################

yn01: $(I_COMPILE) data/yn01.txt
	./$(I_COMPILE) data/yn01.txt 884 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 
yn03: $(I_COMPILE) data/yn03.txt
	./$(I_COMPILE) data/yn03.txt 859 output/$(I_COMPILE)_ $(PARAMS_EJEC)

yn04: $(I_COMPILE) data/yn04.txt
	./$(I_COMPILE) data/yn04.txt 929 output/$(I_COMPILE)_ $(PARAMS_EJEC)


# ################ TEST DMU ##############################

# LB: 2501, BK:2563
dmu01: $(I_COMPILE) data/dmu01.txt
	./$(I_COMPILE) data/dmu01.txt 2501 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# LB: 3042, BK:3244
dmu06: $(I_COMPILE) data/dmu06.txt
	./$(I_COMPILE) data/dmu06.txt 3042 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu13:$(I_COMPILE) data/dmu13.txt
	./$(I_COMPILE) data/dmu13.txt 3681 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu19:$(I_COMPILE) data/dmu19.txt
	./$(I_COMPILE) data/dmu19.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu20: $(I_COMPILE) data/dmu20.txt
	./$(I_COMPILE) data/dmu20.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

# best: 
dmu29: $(I_COMPILE) data/dmu29.txt
	./$(I_COMPILE) data/dmu29.txt 4691 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu45: $(I_COMPILE) data/dmu45.txt
	./$(I_COMPILE) data/dmu45.txt 0	 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu52: $(I_COMPILE) data/dmu52.txt
	./$(I_COMPILE) data/dmu52.txt 0	 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu53: $(I_COMPILE) data/dmu53.txt
	./$(I_COMPILE) data/dmu53.txt 0	 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu54: $(I_COMPILE) data/dmu54.txt
	./$(I_COMPILE) data/dmu54.txt 0	 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu55: $(I_COMPILE) data/dmu55.txt
	./$(I_COMPILE) data/dmu55.txt 0	 output/$(I_COMPILE)_ $(PARAMS_EJEC)	

dmu57: $(I_COMPILE) data/dmu57.txt
	$(PREFIX_RUN) ./$(I_COMPILE) data/dmu57.txt 0	 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu58: $(I_COMPILE) data/dmu58.txt
	$(PREFIX_RUN) ./$(I_COMPILE) data/dmu58.txt 0	 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu59: $(I_COMPILE) data/dmu59.txt
	$(PREFIX_RUN) ./$(I_COMPILE) data/dmu59.txt 0	 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu60: $(I_COMPILE) data/dmu60.txt
	./$(I_COMPILE) data/dmu60.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu61: $(I_COMPILE) data/dmu61.txt
	./$(I_COMPILE) data/dmu61.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu62: $(I_COMPILE) data/dmu62.txt
	./$(I_COMPILE) data/dmu62.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu63: $(I_COMPILE) data/dmu63.txt
	./$(I_COMPILE) data/dmu63.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu64: $(I_COMPILE) data/dmu64.txt
	./$(I_COMPILE) data/dmu64.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu65: $(I_COMPILE) data/dmu65.txt
	./$(I_COMPILE) data/dmu65.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu66: $(I_COMPILE) data/dmu66.txt
	./$(I_COMPILE) data/dmu66.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu68: $(I_COMPILE) data/dmu68.txt
	./$(I_COMPILE) data/dmu68.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu69: $(I_COMPILE) data/dmu69.txt
	./$(I_COMPILE) data/dmu69.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu70: $(I_COMPILE) data/dmu70.txt
	./$(I_COMPILE) data/dmu70.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu71: $(I_COMPILE) data/dmu71.txt
	./$(I_COMPILE) data/dmu71.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu73: $(I_COMPILE) data/dmu73.txt
	./$(I_COMPILE) data/dmu73.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu76: $(I_COMPILE) data/dmu76.txt
	./$(I_COMPILE) data/dmu76.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu78: $(I_COMPILE) data/dmu78.txt
	./$(I_COMPILE) data/dmu78.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu77: $(I_COMPILE) data/dmu77.txt
	./$(I_COMPILE) data/dmu77.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu79: $(I_COMPILE) data/dmu79.txt
	./$(I_COMPILE) data/dmu79.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)

dmu80: $(I_COMPILE) data/dmu80.txt
	$(PREFIX_RUN) ./$(I_COMPILE) data/dmu80.txt 0 output/$(I_COMPILE)_ $(PARAMS_EJEC)
