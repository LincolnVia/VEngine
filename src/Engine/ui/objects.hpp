#pragma once

#include "raylib.h"
#include <string>
namespace engine {

namespace UI {

struct Transform {
  Vector2 position;
  Vector2 rotation;
  Vector2 scale;
};
struct RectBounds {
  Vector2 min;
  Vector2 max;
};
struct EventListener {
  explicit EventListener(RectBounds &bounds) : bounds(bounds) {}
  RectBounds &bounds;

  bool IsMouseOver() {
    Vector2 mousePos = GetMousePosition();
    if (mousePos.x >= bounds.min.x && mousePos.y >= bounds.min.y &&
        mousePos.x <= bounds.max.x && mousePos.y <= bounds.max.y) {
      return true;
    }
    return false;
  }
};
struct SceneObject {
  SceneObject(Transform transform, RectBounds bounds)
      : transform(transform), bounds(bounds) {}
  Transform transform;
  RectBounds bounds;
};

struct UIElement : SceneObject {};
struct TextElement : UIElement {
  std::string getContent() const { return content; }
  void setContent(std::string val) { content = val; }

private:
  std::string content;
};
struct Panel : UIElement {

  void Draw();
  Color background_color;
};
struct ColorRect : Panel {};
struct Image : Panel {};
} // namespace UI
} // namespace engine
