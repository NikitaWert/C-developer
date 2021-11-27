#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;


    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:

        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() {}

        Node(const Node& other) : node_(other.node_) {
        }

        Node(Node&& other) noexcept : node_(std::move(other.node_)) {
        }

        Node(Array array)
            : node_(std::move(array)) {
        }

        Node(Dict map)
            : node_(std::move(map)) {
        }

        Node(int value)
            : node_(value) {
        }
        Node(std::string value)
            : node_(move(value)) {
        }

        Node(double value)
            : node_(std::move(value)) {
        }

        Node(bool value)
            : node_(std::move(value)) {
        }

        Node(Value value)
            : node_(move(value)) {
        }

        Node(std::nullptr_t)
            : node_(nullptr) {
        }

        Node& operator=(const Node& other) {
            node_ = other.node_;
            return *this;
        }

        Node& operator=(Node&& other) noexcept {
            node_ = std::move(other.node_);
            return *this;
        }

        bool IsInt() const {
            return std::holds_alternative<int>(node_);
        }
        int AsInt() const {
            using namespace std::literals;
            if (!IsInt()) {
                throw std::logic_error("Not an int"s);
            }
            return std::get<int>(node_);
        }

        bool IsPureDouble() const {
            return std::holds_alternative<double>(node_);
        }
        bool IsDouble() const {
            return IsInt() || IsPureDouble();
        }
        double AsDouble() const {
            using namespace std::literals;
            if (!IsDouble()) {
                throw std::logic_error("Not a double"s);
            }
            return IsPureDouble() ? std::get<double>(node_) : AsInt();
        }

        bool IsBool() const {
            return std::holds_alternative<bool>(node_);
        }
        bool AsBool() const {
            using namespace std::literals;
            if (!IsBool()) {
                throw std::logic_error("Not a bool"s);
            }

            return std::get<bool>(node_);
        }

        bool IsNull() const {
            return std::holds_alternative<std::nullptr_t>(node_);
        }

        bool IsArray() const {
            return std::holds_alternative<Array>(node_);
        }
        const Array& AsArray() const {
            using namespace std::literals;
            if (!IsArray()) {
                throw std::logic_error("Not an array"s);
            }

            return std::get<Array>(node_);
        }
        Array& AsArray() {
            using namespace std::literals;
            if (!IsArray()) {
                throw std::logic_error("Not an array"s);
            }

            return std::get<Array>(node_);
        }

        bool IsString() const {
            return std::holds_alternative<std::string>(node_);
        }
        const std::string& AsString() const {
            using namespace std::literals;
            if (!IsString()) {
                throw std::logic_error("Not a string"s);
            }

            return std::get<std::string>(node_);
        }

        bool IsDict() const {
            return std::holds_alternative<Dict>(node_);
        }
        const Dict& AsDict() const {
            using namespace std::literals;
            if (!IsDict()) {
                throw std::logic_error("Not a dict"s);
            }

            return std::get<Dict>(node_);
        }
        Dict& AsDict() {
            using namespace std::literals;
            if (!IsDict()) {
                throw std::logic_error("Not a dict"s);
            }

            return std::get<Dict>(node_);
        }

        bool operator==(const Node& rhs) const {
            return GetValue() == rhs.GetValue();
        }

        const Value& GetValue() const {
            return node_;
        }

        Value& GetValue() {
            return node_;
        }

    private:

        Value node_;
    };

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    class Document {
    public:
        explicit Document(Node root)
            : root_(std::move(root)) {
        }

        const Node& GetRoot() const {
            return root_;
        }

    private:
        Node root_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json