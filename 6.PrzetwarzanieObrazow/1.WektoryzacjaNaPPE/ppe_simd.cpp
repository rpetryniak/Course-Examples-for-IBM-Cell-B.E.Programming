#include <iostream>
#include <fstream>
#include <sys/time.h>
#include "../lib/strconvert.h"
#include "../lib/EasyBMP.h"

#include <altivec.h>

using namespace std;

typedef unsigned char PixelType;

//negatyw z obrazu
void negatyw(BMP& obraz)
{
  int height = obraz.TellHeight();
  int width  = obraz.TellWidth();

  vector unsigned char maxPixel     = (vector unsigned char){255,255,255,255,255,255,255,255, 255,255,255,255,255,255,255,255};
  vector unsigned char *pixels      = (vector unsigned char*)obraz.Pixels[0];

  //Przyśpieszenie 16-krotne (4 piksele na raz, w każdym 4 składowe RGBA)
  for (int i=1; i<(height*width/4) ;i++)
    pixels[i] = maxPixel - pixels[i];

  //Komentarz: Nie wiem dlaczego, ale jest problem z początkowym i końcowym fragmentem.
}

//funkcja główna (main)
int main( /* int argc, char* argv[] */)
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
  img_orginal.WriteToFile("../img/out1.bmp");

  return 0;
}


/* Test operacji wektorowych:
test1:
  vector unsigned char a = (vector unsigned char) {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
  vector unsigned char b = (vector unsigned char) {100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100};
  vector unsigned char c = a - b;
  unsigned char *d = (unsigned char*) &c;

  for (int i=0; i<16; i++)
    printf("%d ",d[i]);
  printf("\n");

test2:
  vector int a = (vector int) {255,255,255,255};
  int tab1[16] = {100,100,100,100};
  vector int *c = (vector int*)tab1;
  vector int d = vec_sub(a,c[0]);

test3:
  vector unsigned char maxPixel = (vector unsigned char){255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
  unsigned char tab1[16] = {100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100};
  vector unsigned char *pixels = (vector unsigned char*)tab1;
  vector unsigned char result = vec_sub(maxPixel,pixels[0]);
*/
