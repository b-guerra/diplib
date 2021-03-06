/*
 * DIPimage 3.0
 * This MEX-file implements the `gdt` function
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dip_matlab_interface.h"
#include "diplib/distance.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 3 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image const grey = dml::GetImage( prhs[ 1 ] );
      dip::Image out = mi.NewImage();

      dip::Metric metric( "chamfer", 1 );
      if( nrhs > 2 ) {
         dip::uint chamfer = dml::GetUnsigned( prhs[ 2 ] );
         switch( chamfer ) {
            case 1:
               metric = dip::Metric( "connected", 1 );
               break;
            case 3:
               break;
            case 5:
               metric = dip::Metric( "chamfer", 2 );
               break;
            default:
               DIP_THROW( dip::E::INVALID_PARAMETER );
         }
      }

      dip::String outputMode;
      if( nlhs > 1 ) {
         outputMode = "both";
      } else {
         outputMode = "GDT";
      }

      dip::GreyWeightedDistanceTransform( grey, in, out, metric, outputMode );

      if( nlhs > 1 ) {
         plhs[ 0 ] = mi.GetArray( out[ 0 ] );
         plhs[ 1 ] = mi.GetArray( out[ 1 ] );
      } else {
         plhs[ 0 ] = mi.GetArray( out );
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
