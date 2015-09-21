#include <iostream>
#include "diplib.h"

// Just a test!

#include "dip_overload.h"
#include <typeinfo>

template< typename TPI >
static void dip__MyFunction( void* vin ) {
   TPI* in = static_cast< TPI* > ( vin );
   std::cout << "Data type = " << typeid( in ).name() << std::endl;
}

void MyFunction( dip::Image image ) {
   dip::DataType dt = image.GetDataType();
   double data;
   void* in = &data;
   DIP_OVL_CALL_ALL( dip__MyFunction, (in), dt );
}


int main() {
   {
   dip::Image img;
   std::cout << img;
   img.SetDimensions({50,80,30});
   img.SetTensorDimensions({3});
   img.Forge();
   std::cout << img;
   img.Strip();
   img.SetStrides({-80,-1,4000});
   img.SetTensorStrides({120000});
   img.Forge();
   std::cout << img;
   }
   dip::Image img;
   img.SetDataType(dip::DataType::UINT8);
   MyFunction(img);
   img.SetDataType(dip::DataType::SINT32);
   MyFunction(img);
   img.SetDataType(dip::DataType::SFLOAT);
   MyFunction(img);
   return 1;
}