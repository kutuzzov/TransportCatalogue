#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <sstream>

namespace svg {

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t red, uint8_t green, uint8_t blue)
            : red(red)
            , green(green)
            , blue(blue) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba : Rgb {
        Rgba() = default;
        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
            : Rgb(red, green, blue)
            , opacity(opacity) {}

        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    inline const std::string NoneColor{ "none" };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& output, StrokeLineCap slc);
    std::ostream& operator<<(std::ostream& output, StrokeLineJoin slj);

    struct OstreamColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const {
            using namespace std::literals;
            out << "none"s;
        }
        void operator()(std::string color) const {
            out << color;
        }
        void operator()(Rgb rgb) const {
            using namespace std::literals;
            out << "rgb("s << unsigned(rgb.red) << ","s << unsigned(rgb.green) << ","s << unsigned(rgb.blue) << ")"s;
        }
        void operator()(Rgba rgba) const {
            using namespace std::literals;
            out << "rgba("s << unsigned(rgba.red) << ","s << unsigned(rgba.green) << ","s << unsigned(rgba.blue)
                << ","s << rgba.opacity << ")"s;
        }
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_linecap_ = std::move(line_cap);
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_linejoin_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        virtual ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            if (fill_color_) {
                std::ostringstream strm;
                std::visit(OstreamColorPrinter{ strm }, *fill_color_);
                out << " fill=\""sv << strm.str() << "\""sv;
            }
            if (stroke_color_) {
                std::ostringstream strm;
                std::visit(OstreamColorPrinter{ strm }, *stroke_color_);
                out << " stroke=\""sv << strm.str() << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(const std::string& font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(const std::string& font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(const std::string& data);

    private:
        void RenderObject(const RenderContext& context) const override;
        Point position_ = { 0.0, 0.0 };
        Point offset_ = { 0.0, 0.0 };
        uint32_t font_size_ = 1;
        std::string font_family_ = "";
        std::string font_weight_ = "";
        std::string data_ = "";
    };

    class ObjectContainer {
    public:
        template <typename T>
        void Add(T obj) {
            AddPtr(std::make_unique<T>(obj));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    class Drawable {
    public:
        virtual void Draw(svg::ObjectContainer& container) const = 0;

        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
    public:

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj);

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg