#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, Rgb rgb) {
        out << "rgb("sv << unsigned(rgb.red) << ","sv
            << unsigned(rgb.green) << ","sv
            << unsigned(rgb.blue) << ")"sv;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, Rgba rgba) {
        out << "rgba("sv << unsigned(rgba.red) << ","sv
            << unsigned(rgba.green) << ","sv
            << unsigned(rgba.blue) << ","sv << rgba.opacity << ")"sv;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT: {
            out << "butt"sv;
            break;
        }
        case StrokeLineCap::ROUND: {
            out << "round"sv;
            break;
        }
        case StrokeLineCap::SQUARE: {
            out << "square"sv;
            break;
        }
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
        switch (line_join) {
        case StrokeLineJoin::ARCS: {
            out << "arcs"sv;
            break;
        }
        case StrokeLineJoin::BEVEL: {
            out << "bevel"sv;
            break;
        }
        case StrokeLineJoin::MITER: {
            out << "miter"sv;
            break;
        }
        case StrokeLineJoin::MITER_CLIP: {
            out << "miter-clip"sv;
            break;
        }
        case StrokeLineJoin::ROUND: {
            out << "round"sv;
            break;
        }
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        // Äåëåãèðóåì âûâîä òåãà ñâîèì ïîäêëàññàì
        RenderObject(context);

        context.out << std::endl;
    }

    //  ---------- Circle ------------------

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
        out << "/>"sv;
    }

    //  ---------- Polyline ---------------

    Polyline& Polyline::AddPoint(Point point) {
        vertexes_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first_step = true;
        for (auto& vertex : vertexes_) {
            if (!first_step) {
                out << " "sv;
            }
            first_step = false;
            out << vertex.x << ","sv << vertex.y;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }
    //  ------------- Text ------------------
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        ReplaceServiceChars(data_);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text "sv;
        out << "x=\""sv << position_.x << "\" "sv << "y=\""sv << position_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" "sv << "dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << font_size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        RenderAttrs(context.out);
        out << ">"sv;
        out << data_;
        out << "</text>"sv;
    }

    void Text::ReplaceServiceChars(std::string& data) {
        for (const auto& [from, to] : replacements_) {
            size_t start_pos = 0;
            while ((start_pos = data.find(from, start_pos)) != std::string::npos) {
                data.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }
        }
    }
    //  ----------------- Document ---------------------
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        RenderContext ctx(out, 2, 2);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& obj : objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }
    
}  // namespace svg