#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <sstream>
#include <iomanip>

namespace svg {

    struct Point {
        Point() = default;

        Point(double x, double y);

        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out);

        RenderContext(std::ostream& out, int indent_step, int indent = 0);

        RenderContext Indented() const;

        void RenderIndent() const;

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    struct Rgb {
        Rgb() = default;

        Rgb(uint8_t r, uint8_t g, uint8_t b);

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;

        Rgba(uint8_t r, uint8_t g, uint8_t b, double o);

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{ std::monostate() };

    struct OutRgb {
        std::ostream& out;

        void operator()(std::monostate) const;

        void operator()(std::string str) const;

        void operator()(Rgb rgb) const;

        void operator()(Rgba rgba) const;
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    inline std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color);

        Owner& SetStrokeColor(Color color);

        Owner& SetStrokeWidth(double width);

        Owner& SetStrokeLineCap(StrokeLineCap line_cap);

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join);
    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const;
    private:
        Owner& AsOwner();

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj obj);
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        virtual ~ObjectContainer() = default;
    protected:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;

        virtual ~Drawable() = default;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
    public:

        Polyline& AddPoint(Point point);
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    class Text final : public Object, public PathProps<Text> {
    public:

        Text& SetPosition(Point pos);

        Text& SetOffset(Point offset);

        Text& SetFontSize(uint32_t size);

        Text& SetFontFamily(std::string font_family);

        Text& SetFontWeight(std::string font_weight);

        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_;
        Point offset_;
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    class Document final : public ObjectContainer {
    public:
        ~Document() {}

        virtual void AddPtr(std::unique_ptr<Object>&& obj) override;

        void Render(std::ostream& out) const;

    };

    template <typename Owner>
    Owner& PathProps<Owner>::SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeWidth(double width) {
        width_ = width;
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsOwner();
    }

    template <typename Owner>
    void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv;
            std::visit(OutRgb{ out }, *fill_color_);
            out << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv;
            std::visit(OutRgb{ out }, *stroke_color_);
            out << "\""sv;
        }
        if (width_) {
            out << " stroke-width=\"" << *width_ << "\""sv;
        }
        if (line_cap_) {
            out << " stroke-linecap=\"" << *line_cap_ << "\""sv;
        }
        if (line_join_) {
            out << " stroke-linejoin=\"" << *line_join_ << "\""sv;
        }
    }

    template <typename Owner>
    Owner& PathProps<Owner>::AsOwner() {
        return static_cast<Owner&>(*this);
    }


    template <typename Obj>
    void ObjectContainer::Add(Obj obj) {
        objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
    }

}
