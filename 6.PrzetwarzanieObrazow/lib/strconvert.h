#ifndef STRCONVERT_H_
#define STRCONVERT_H_

#endif /*STRCONVERT_H_*/

 #include <iostream>
 #include <sstream>
 #include <string>
 #include <stdexcept>
 
 class BadConversion : public std::runtime_error {
 public:
   BadConversion(const std::string& s)
     : std::runtime_error(s)
     { }
 };
 
 inline std::string inttostring(double x)
 {
   std::ostringstream o;
   if (!(o << x))
     throw BadConversion("stringify(double)");
   return o.str();
 }
