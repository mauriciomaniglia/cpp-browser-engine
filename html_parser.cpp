#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Enum for token types
enum class TokenType {
    StartTag,
    EndTag,
    Text
};

// Token structure
struct Token {
    TokenType type;
    std::string data;  // Tag name or text content
};

// Simple DOM Node class
class Node {
public:
    std::string name;  // Tag name or text content
    std::vector<std::unique_ptr<Node>> children;
    bool is_text;  // Distinguish text nodes from elements

    Node(std::string n, bool text = false) : name(n), is_text(text) {}
    
    // Add a child node
    void add_child(std::unique_ptr<Node> child) {
        children.push_back(std::move(child));
    }

    // Print the tree (for debugging)
    void print(int depth = 0) const {
        for (int i = 0; i < depth; ++i) std::cout << "  ";
        if (is_text) {
            std::cout << "Text: \"" << name << "\"" << std::endl;
        } else {
            std::cout << "<" << name << ">" << std::endl;
        }
        for (const auto& child : children) {
            child->print(depth + 1);
        }
        if (!is_text && depth > 0) {
            for (int i = 0; i < depth; ++i) std::cout << "  ";
            std::cout << "</" << name << ">" << std::endl;
        }
    }
};

// Tokenizer function
std::vector<Token> tokenize(const std::string& html) {
    std::vector<Token> tokens;
    size_t pos = 0;
    std::string buffer;

    while (pos < html.length()) {
        char c = html[pos];

        if (c == '<') {
            // Flush any buffered text
            if (!buffer.empty()) {
                tokens.push_back({TokenType::Text, buffer});
                buffer.clear();
            }

            // Check if it's an end tag
            bool is_end_tag = (pos + 1 < html.length() && html[pos + 1] == '/');
            pos += (is_end_tag ? 2 : 1);  // Skip '<' or '</'

            // Extract tag name
            std::string tag_name;
            while (pos < html.length() && html[pos] != '>') {
                tag_name += html[pos++];
            }
            pos++;  // Skip '>'

            tokens.push_back({is_end_tag ? TokenType::EndTag : TokenType::StartTag, tag_name});
        } else {
            // Accumulate text content
            buffer += c;
            pos++;
        }
    }

    // Flush remaining text
    if (!buffer.empty()) {
        tokens.push_back({TokenType::Text, buffer});
    }

    return tokens;
}

// Build DOM tree from tokens
std::unique_ptr<Node> build_dom(const std::vector<Token>& tokens) {
    auto root = std::make_unique<Node>("document");
    std::vector<Node*> stack;  // Stack of open elements
    stack.push_back(root.get());

    for (const auto& token : tokens) {
        if (token.type == TokenType::StartTag) {
            auto node = std::make_unique<Node>(token.data);
            stack.back()->add_child(std::move(node));
            stack.push_back(stack.back()->children.back().get());
        } else if (token.type == TokenType::EndTag) {
            if (!stack.empty()) stack.pop_back();  // Close the current element
        } else if (token.type == TokenType::Text) {
            auto text_node = std::make_unique<Node>(token.data, true);
            stack.back()->add_child(std::move(text_node));
        }
    }

    return root;
}

int main() {
    // Test HTML string
    std::string html = "<html><body><div>Hello <b>world</b></div></body></html>";

    // Step 1: Tokenize
    std::vector<Token> tokens = tokenize(html);
    std::cout << "Tokens:\n";
    for (const auto& token : tokens) {
        std::cout << (token.type == TokenType::StartTag ? "StartTag" :
                      token.type == TokenType::EndTag ? "EndTag" : "Text")
                  << ": " << token.data << "\n";
    }

    // Step 2: Build DOM tree
    auto dom = build_dom(tokens);

    // Step 3: Print the tree
    std::cout << "\nDOM Tree:\n";
    dom->print();

    return 0;
}
