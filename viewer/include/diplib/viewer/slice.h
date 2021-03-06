/*
 * DIPlib 3.0 viewer
 * This file contains definitions for the nD image slice viewer.
 *
 * (c)2017, Wouter Caarls
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

#ifndef DIP_VIEWER_SLICE_H
#define DIP_VIEWER_SLICE_H

#include <thread>

#include "diplib/color.h"

#include "diplib/viewer/manager.h"
#include "diplib/viewer/viewer.h"
#include "diplib/viewer/image.h"
#include "diplib/viewer/tensor.h"
#include "diplib/viewer/histogram.h"
#include "diplib/viewer/control.h"
#include "diplib/viewer/status.h"
#include "diplib/viewer/link.h"

/// \file
/// \brief Declares `dip::viewer::SliceViewer`.

namespace dip { namespace viewer {

class DIPVIEWER_EXPORT SliceView : public View
{
  protected:
    dip::Image projected_,       ///< Projected (2D) image.
               colored_;         ///< Colored (RGB) image.
    dip::ColorSpaceManager csm_; ///< For LookupTable::ColorSpace

    dip::uint dimx_, dimy_;      ///< Indices in options.dims_.             
    unsigned int texture_;       ///< OpenGL texture identifier.

  public:
    SliceView(ViewPort *viewport, dip::uint dimx, dip::uint dimy) : View(viewport), dimx_(dimx), dimy_(dimy), texture_(0) { }

    void project();
    void map();
    void rebuild();
    void render();
    dip::uint size(dip::uint ii)
    {
      return colored_.Size(ii);
    }
    
    dip::uint dimx() { return dimx_; }
    dip::uint dimy() { return dimy_; }
};

class DIPVIEWER_EXPORT SliceViewPort : public ViewPort
{
  protected:
    class SliceViewer *viewer_;
    SliceView *view_;
    int drag_x_, drag_y_;
    
  public:
    explicit SliceViewPort(class SliceViewer *viewer) : ViewPort((Viewer*)viewer), viewer_(viewer), view_(NULL) { }
    ~SliceViewPort() override { if (view_) delete view_; }
    
    void rebuild() override { view()->rebuild(); }
    void render() override;
    void click(int button, int state, int x, int y) override;
    void motion(int button, int x, int y) override;

    void setView(SliceView *view) { view_ = view; }
    SliceView *view() { return view_; }

  protected:    
    void screenToView(int x, int y, double *ix, double *iy) override;
};

/// \addtogroup viewer
/// \{

/// \brief Interactive nD tensor image viewer.
class DIPVIEWER_EXPORT SliceViewer : public Viewer
{
  public:
    typedef std::shared_ptr<SliceViewer> Ptr;
    
  protected:
    ViewingOptions options_;
    std::thread thread_;
    bool continue_, updated_;
    std::vector<ViewPort*> viewports_;
    SliceViewPort *main_, *left_, *top_;
    TensorViewPort *tensor_;
    HistogramViewPort *histogram_;
    ControlViewPort *control_;
    StatusViewPort *status_;
    LinkViewPort *link_;
    dip::Image original_, image_;
    
    ViewPort *drag_viewport_;
    int drag_button_;
    int refresh_seq_;
  
  public:
    /// \brief Construct a new SliceViewer.
    ///
    /// As the constructor is protected, this is the only way to create a SliceViewer.
    ///
    /// Example usage:
    ///
    /// ```cpp
    ///     manager.createWindow( dip::viewer::SliceViewer::Create( image ));
    /// ```
    static Ptr Create(const dip::Image &image, std::string name="SliceViewer", size_t width=0, size_t height=0)
    {
      return Ptr(new SliceViewer(image, name, width, height));
    }
  
    ~SliceViewer() override
    {
      if (continue_)
      {
        continue_ = false;
        thread_.join();
      }
      
      for (size_t ii=0; ii < viewports_.size(); ++ii)
        delete viewports_[ii];
    }
    
    ViewingOptions &options() override { return options_; }
    const dip::Image &image() override { return image_; }
    void setImage(const dip::Image &image) override { original_ = image; refresh_seq_++; }
    
    /// \brief Update linked viewers.
    ///
    /// Only call this under lock.
    void updateLinkedViewers();
  protected:
    explicit SliceViewer(const dip::Image &image, std::string name="SliceViewer", size_t width=0, size_t height=0);

    void create() override;
    void reshape(int width, int height) override;
    void draw() override;
    void key(unsigned char k, int x, int y, int mods) override;
    void click(int button, int state, int x, int y) override;
    void motion(int x, int y) override;

    void place();
    ViewPort *viewport(int x, int y);
    void calculateTextures();
};

/// \}

}} // namespace dip::viewer

#endif // DIP_VIEWER_SLICE_H
