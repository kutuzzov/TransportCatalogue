#pragma once

#include "json.h"
#include <string>
#include <vector>
#include <utility>
#include <optional>

namespace json {

    enum class Step {
        BUILD,
        ARR,
        DICT
    };

    class DictItemContext;
    class DictValueContext;
    class ArrayContext;

    class Builder {
    public:
        Builder() {
            step_stack_.push_back(Step::BUILD);
        }

        DictItemContext StartDict();
        Builder& EndDict();
        ArrayContext StartArray();
        Builder& EndArray();
        DictValueContext Key(const std::string& key);
        Builder& Value(const Node::Value& val);
        Node Build() const;
        void AddNode(Node&& node);

    private:
        std::optional<Node> root_;
        std::vector<Step> step_stack_;
        std::vector< std::optional<std::string> > keys_;
        int dicts_open_ = 0;
        int arrays_open_ = 0;
        std::vector<std::vector<Node>> all_arrays_;
        std::vector< std::vector<std::pair<std::string, Node>> > all_dicts_;
    };

    class DictItemContext {
    public:
        DictItemContext(Builder& builder)
            : builder_(builder) {}

        DictValueContext Key(const std::string& key);
        Builder& EndDict();

    private:
        Builder& builder_;
    };

    class DictValueContext {
    public:
        DictValueContext(Builder& builder)
            : builder_(builder) {}

        DictItemContext Value(const Node::Value& val);
        DictItemContext StartDict();
        ArrayContext StartArray();

    private:
        Builder& builder_;
    };

    class ArrayContext {
    public:
        ArrayContext(Builder& builder)
            : builder_(builder) {}

        ArrayContext Value(const Node::Value& val);
        DictItemContext StartDict();
        ArrayContext StartArray();
        Builder& EndArray();

    private:
        Builder& builder_;
    };

} // namespace json