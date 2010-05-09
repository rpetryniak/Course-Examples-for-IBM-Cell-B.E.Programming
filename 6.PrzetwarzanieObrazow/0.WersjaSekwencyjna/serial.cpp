#include <iostream>
#include <fstream>
#include <sys/time.h>
#include "../lib/strconvert.h"
#include "../lib/EasyBMP.h"

using namespace std;

//negatyw z obrazu
void negatyw(BMP& obraz)
{
  for( int j=0 ; j < obraz.TellHeight() ; j++)
  {
    for( int i=0 ; i < obraz.TellWidth() ; i++)
    {
     obraz(i,j)->Red   = 255 - obraz(i,j)->Red;
     obraz(i,j)->Green = 255 - obraz(i,j)->Green;
     obraz(i,j)->Blue  = 255 - obraz(i,j)->Blue;
    }
  }
   //return 0;
}

//funkcja główna (main)
int main( int argc, char* argv[] )
{
  //Odczyt z pliku
  BMP img_orginal;
  img_orginal.ReadFromFile("../img/lenna.bmp");

  //Początek pomiaru czasu:
  timeval tim;
  gettimeofday(&tim, NULL);
  double tstart=tim.tv_sec+(tim.tv_usec/1000000.0);

  //Przetwarzanie obrazu:
  negatyw(img_orginal);

  //Koniec pomiaru czasu:
  gettimeofday(&tim, NULL);
  double tend=tim.tv_sec+(tim.tv_usec/1000000.0);
  printf("%.8lfs\n", tend-tstart);

  //Zapis obrazu wynikowego do pliku
  img_orginal.WriteToFile("../img/out0.bmp");

  return 0;
}
