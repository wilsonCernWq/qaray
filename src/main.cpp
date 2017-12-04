
#include "renderers/Renderer_GUI.h"
#include "renderers/Renderer_MPI.h"
#include "parser/xmlload.h"

#pragma warning(disable: 588)

int main(int argc, char **argv)
{
  RendererParam param;
  enum { RENDER_GUI, RENDER_MPI } renderer_mode = RENDER_GUI;
  const char *file = nullptr;
  if (argc < 2) {
    std::cerr << "Error: insufficient input" << std::endl;
    return -1;
  }
  for (int i = 1; i < argc; ++i) {
    std::string str(argv[i]);
    if (str == "-batch") {
      renderer_mode = RENDER_MPI;
    } else if (str == "-spp") {
      auto tmp = std::atoi(argv[++i]);
      param.SetSPPMax(tmp);
      param.SetSPPMin(tmp);
    } else if (str == "-sppMin") {
      param.SetSPPMin(std::atoi(argv[++i]));
    } else if (str == "-sppMax") {
      param.SetSPPMin(std::atoi(argv[++i]));
    } else if (str == "-bounce") {
      Material::maxBounce = std::atoi(argv[++i]);
    } else if (str == "-srgb") {
      param.SetSRGB(true);
    } else if (str == "-threads") {
      tasking::set_num_of_threads(static_cast<size_t>(std::atoi(argv[++i])));
    } else {
      file = argv[i];
    }
  }

  std::shared_ptr<Renderer> renderer;
  switch(renderer_mode){
    case(RENDER_GUI):
      renderer = std::make_shared<Renderer_GUI>(param);
      break;
    case(RENDER_MPI):
      renderer = std::make_shared<Renderer_MPI>(param);
      break;
  }
  renderer->Init();
  LoadScene(file);
  renderer->ComputeScene(renderImage, scene);
  renderer->Render();
  renderer->Terminate();

  return 0;
}
