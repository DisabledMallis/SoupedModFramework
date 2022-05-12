#include "GPUContextGL.h"
#include "GPUDriverGL.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace ultralight {

    GPUContextGL::GPUContextGL(bool enable_vsync, bool enable_msaa) {
      driver_ = std::unique_ptr<ultralight::GPUDriverGL>(new ultralight::GPUDriverGL(this));
    }

}  // namespace ultralight
