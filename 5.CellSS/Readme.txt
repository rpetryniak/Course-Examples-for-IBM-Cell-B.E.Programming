Cell Super Scalar jest rozwijany przez Barcelona Supercomputing
  http://www.bsc.es/plantillaG.php?cat_id=179

Konfiguracja środowiska i kompilacja programu:
  export PATH=$PATH:/opt/CellSS/bin
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/CellSS/lib
  cellss-cc -O3 -o cellss1 cellss1.c
