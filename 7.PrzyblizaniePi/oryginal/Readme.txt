Na podstawie:
  http://www.icm.edu.pl/kdm/Biuletyn_nr_28#Architektura_Cell_BE_w_ICM

Kompilacja:
spu-gcc spu_pi.c -o spu_pi
./spu_pi

embedspu pi_spu spu_pi spu_pi_embedded.o

gcc ppu_pi.c spu_pi_embedded.o -lspe2 -o ppu_pi
./hello
