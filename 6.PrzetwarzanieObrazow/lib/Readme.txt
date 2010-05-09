Jek przygotować z 'EasyBMP' bibliotekę statyczną:
  gcc -c EasyBMP.cpp -o easybmp.o
  ar rcs easybmp.a easybmp.o

Dla PPE
  ppu-g++ -c EasyBMP.cpp -o easybmp_ppu.o
