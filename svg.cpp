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

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
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

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    std::ostream& operator<<(std::ostream& out, std::vector<Point> points) {
        for (size_t i = 0; i < points.size(); ++i) {
            if (i != 0) {
                out << ' ';
            }
            out << points[i].x << ',' << points[i].y;
        }
        return out;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""s << points_ << "\""s;
        RenderAttrs(context.out);
        out << "/>"s;
    }

    // ---------- Text ------------------

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(const std::string& font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(const std::string& font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(const std::string& data) {
        std::string str = ""s;
        for (char c : data) {
            switch (c) {
            case '\"':
                str += "&quot;"s; break;
            case '\'':
                str += "&apos;"s; break;
            case '<':
                str += "&lt;"s; break;
            case '>':
                str += "&gt;"s; break;
            case '&':
                str += "&amp;"s; break;
            default:
                str += c; break;
            }
        }
        data_ = std::move(str);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(context.out);
        out << " x=\""s << position_.x << "\" y=\""s << position_.y
            << "\" dx=\""s << offset_.x << "\" dy=\""s << offset_.y << "\" font-size=\""s << font_size_ << "\"";
        if (!font_family_.empty()) {
            out << " font-family=\""s << font_family_ << "\"";
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""s << font_weight_ << "\"";
        }
        out << ">"s << data_ << "</text>"s;
    }

    // Добавляет в svg-документ объект-наследник svg::Object
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        if (obj) {
            objects_.emplace_back(std::move(obj));
        }
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const {
        RenderContext ctx(out, 2, 2);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"s;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"s;
        for (const std::unique_ptr<Object>& obj : objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"s << std::endl;
    }

    std::ostream& operator<<(std::ostream& output, StrokeLineCap slc) {
        switch (slc)
        {
        case svg::StrokeLineCap::BUTT:
            output << "butt"s;
            break;
        case svg::StrokeLineCap::ROUND:
            output << "round"s;
            break;
        case svg::StrokeLineCap::SQUARE:
            output << "square"s;
            break;
        default:
            break;
        }
        return output;
    }
    std::ostream& operator<<(std::ostream& output, StrokeLineJoin slj) {
        switch (slj)
        {
        case svg::StrokeLineJoin::ARCS:
            output << "arcs"s;
            break;
        case svg::StrokeLineJoin::BEVEL:
            output << "bevel"s;
            break;
        case svg::StrokeLineJoin::MITER:
            output << "miter"s;
            break;
        case svg::StrokeLineJoin::MITER_CLIP:
            output << "miter-clip"s;
            break;
        case svg::StrokeLineJoin::ROUND:
            output << "round"s;
            break;
        default:
            break;
        }
        return output;
    }

}  // namespace svg