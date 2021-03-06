/**
 * Author: Nikolaus Mayer, 2018 (mayern@cs.uni-freiburg.de)
 */

#include <cstdlib>
#include <fstream>
#include <iostream>



int main(){
  std::ofstream outfile("img.ppm");

  /// PPM header 
  /// The magic number "P6" means "color PPM in ASCII format"
  outfile << "P3 512 512 255";

  for(int y = 256; y >= -255; --y) {   //For each row
    for(int x = -255; x <= 256; ++x) {   //For each pixel in a row

      /// Write this pixel's RGB color triple (each a single byte)
      outfile << " " << 255
              << " " << 0
              << " " << 0;

    }
  }

  /// Image done
  outfile.close();

  /// Bye!
  return EXIT_SUCCESS;
}


