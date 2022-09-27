#pragma once

#include "core/Layer.h"

namespace RESANA {
class ImGuiLayer final : public Layer {
public:
  ImGuiLayer();
  ~ImGuiLayer() override;

  static void Begin();
  static void End();

  void OnAttach() override;
  void OnDetach() override;
  void OnImGuiRender() override;

  void SetTaskManagerTheme();

private:
  static void ShowImGuiDockspace();

  float mTime = 0.0f;
};

} // namespace RESANA
