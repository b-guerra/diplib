/*
 * DIPimage 3.0
 * This MEX-file implements the `label` function
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
#include "diplib/regions.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 5 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::uint connectivity = in.Dimensionality();
      if( nrhs > 1 ) {
         connectivity = dml::GetUnsigned( prhs[ 1 ] );
      }

      dip::uint minSize = 0;
      if( nrhs > 2 ) {
         minSize = dml::GetUnsigned( prhs[ 2 ] );
      }

      dip::uint maxSize = 0;
      if( nrhs > 3 ) {
         maxSize = dml::GetUnsigned( prhs[ 3 ] );
      }

      dip::StringArray boundary = {};
      if( nrhs > 4 ) {
         boundary = dml::GetStringArray( prhs[ 4 ] );
      }

      dip::Label( in, out, connectivity, minSize, maxSize, boundary );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
