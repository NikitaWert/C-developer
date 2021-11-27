#include "svg.h"
#include <iomanip>

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << " />"sv;
}
// добавляем к объекту точки
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(std::move(point));
    return *this;
}

// рендерим полилинию
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    using namespace string_view_literals;
    out << "<polyline points=\""sv;
    for (uint64_t i = 0; i < points_.size(); ++i) {
        const Point& point = points_.at(i);
        out << point.x << ","sv << point.y;
        if (i + 1 != points_.size()) {
            out << " "sv;
        }
    }
    out << "\"";
    RenderAttrs(context.out);
    out << " />"sv;
}

// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;;
    out << "<text"sv;
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x
        << "\" y=\""sv << pos_.y
        << "\" dx=\""sv << offset_.x
        << "\" dy=\""sv << offset_.y
        << "\" font-size=\""sv << size_;
    if (!font_family_.empty()) {
        out << "\" font-family=\""sv << font_family_;
    }
    if (!font_weight_.empty()) {
        out << "\" font-weight=\""sv << font_weight_;
    }

    out << "\">"sv;
    out << data_;
    out << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext context(out, 0, 1);
    for (const std::unique_ptr<Object>& ptr : objects_) {
        context.RenderIndent();
        ptr->Render(context);
    }
    out << "</svg>"sv;
}

void OutRgb::operator()(std::monostate) const {
    out << "none";
}

void OutRgb::operator()(std::string str) const {
    out << str;
}

void OutRgb::operator()(Rgb rgb) const {
    using namespace std::string_view_literals;
    out << "rgb("sv << std::to_string(rgb.red)
        << ","sv << std::to_string(rgb.green)
        << ","sv << std::to_string(rgb.blue)
        << ")"sv;
}

void OutRgb::operator()(Rgba rgba) const {
    using namespace std::string_view_literals;
    std::string str = std::to_string(rgba.opacity);
    std::string opacity;
    for (int i = 0; i<3; ++i) {
        if (i<static_cast<int>(str.size())) {
            opacity += str[i];
        }
    }
    out << "rgba("sv << std::to_string(rgba.red)
        << ","sv << std::to_string(rgba.green)
        << ","sv << std::to_string(rgba.blue)
        << ","sv << rgba.opacity
        << ")"sv;
}

Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double o)
    : red(r)
    , green(g)
    , blue(b)
    , opacity(o) {

}

Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b)
    : red(r)
    , green(g)
    , blue(b) {

}

Point::Point(double x, double y)
    : x(x)
    , y(y) {
}

RenderContext::RenderContext(std::ostream& out)
    : out(out) {
}

RenderContext::RenderContext(std::ostream& out, int indent_step, int indent)
    : out(out)
    , indent_step(indent_step)
    , indent(indent) {
}

RenderContext RenderContext::Indented() const {
    return {out, indent_step, indent + indent_step};
}

void RenderContext::RenderIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}

inline std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
    using namespace std::string_view_literals;
    switch (line_cap) {
    case StrokeLineCap::BUTT:
        out << "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        out << "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"sv;
        break;
    }
    return out;
}

inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
    using namespace std::string_view_literals;
    switch (line_join) {
    case StrokeLineJoin::ARCS:
        out << "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"sv;
        break;
    }
    return out;
}

}  // namespace svg
