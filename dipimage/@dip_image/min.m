%MIN   Get the first minimum in an image.
%   [VALUE,POSITION] = MIN(B) gets the value and postion of the first
%   minimum in image B.
%
%   [VALUE,POSITION] = MIN(B,M) gets the value and postion of the first
%   minimum in image B masked by M. M may be [] for no mask.
%
%   VALUE = MIN(B,M,DIM) performs the computation over the dimensions
%   specified in DIM. DIM can be an array with any number of
%   dimensions. M may be [] for no mask.
%
%   [VALUE,POSITION] = MIN(B,M,DIM) gets the value and position of
%   the first minimum along dimension DIM. DIM is a single dimension.
%   TODO: This syntax is not yet implemented.
%
%   VALUE = MIN(B,C) is the pixel-by-pixel minimum operator. It returns
%   an image with each pixel the largest taken from B or C. C must not
%   be a binary image, or it will be taken as a mask image (see syntax
%   above).
%
%   COMPATIBILITY NOTE:
%   In DIPimage 2.x, MIN(B), with B a tensor image, would work over all tensor
%   components, yielding a scalar image of the same size as B. To obtain
%   the old behavior:
%      tensorfun('immin',B);

% (c)2017, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
%
% Licensed under the Apache License, Version 2.0 (the "License");
% you may not use this file except in compliance with the License.
% You may obtain a copy of the License at
%
%    http://www.apache.org/licenses/LICENSE-2.0
%
% Unless required by applicable law or agreed to in writing, software
% distributed under the License is distributed on an "AS IS" BASIS,
% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
% See the License for the specific language governing permissions and
% limitations under the License.
