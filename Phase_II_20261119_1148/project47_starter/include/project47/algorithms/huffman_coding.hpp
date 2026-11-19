#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace project47 {

class HuffmanCodebook {
public:
    explicit HuffmanCodebook(std::map<char, std::string> codes = {})
        : codes_(std::move(codes)) {}

    [[nodiscard]] const std::map<char, std::string>& codes() const noexcept {
        return codes_;
    }

    [[nodiscard]] std::string encode(const std::string& input) const {
        std::string output;
        for (const char symbol : input) {
            const auto it = codes_.find(symbol);
            if (it == codes_.end()) {
                throw std::invalid_argument("Input contains a symbol absent from the Huffman codebook");
            }
            output += it->second;
        }
        return output;
    }

    [[nodiscard]] std::string decode(const std::string& bits) const {
        for (const char bit : bits) {
            if (bit != '0' && bit != '1') {
                throw std::invalid_argument("Huffman bitstream may contain only 0 and 1");
            }
        }
        if (bits.empty()) return {};
        if (codes_.empty()) throw std::invalid_argument("Cannot decode with an empty Huffman codebook");

        std::string output;
        std::string pending;
        for (const char bit : bits) {
            pending.push_back(bit);
            bool is_prefix = false;
            bool matched = false;
            for (const auto& [symbol, code] : codes_) {
                if (code == pending) {
                    output.push_back(symbol);
                    pending.clear();
                    matched = true;
                    break;
                }
                if (code.size() > pending.size() && code.compare(0, pending.size(), pending) == 0) {
                    is_prefix = true;
                }
            }
            if (!matched && !is_prefix) {
                throw std::invalid_argument("Bitstream is not valid for this Huffman codebook");
            }
        }
        if (!pending.empty()) throw std::invalid_argument("Huffman bitstream ends with an incomplete code");
        return output;
    }

private:
    std::map<char, std::string> codes_;
};

namespace detail {
struct HuffmanNode {
    std::uint64_t frequency{};
    char min_symbol{};
    char symbol{};
    bool leaf{};
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;
};

struct HuffmanNodeGreater {
    bool operator()(const std::shared_ptr<HuffmanNode>& a,
                    const std::shared_ptr<HuffmanNode>& b) const noexcept {
        if (a->frequency != b->frequency) return a->frequency > b->frequency;
        return static_cast<unsigned char>(a->min_symbol) > static_cast<unsigned char>(b->min_symbol);
    }
};
} // namespace detail

// Builds a deterministic prefix-free code. Runtime: O(k log k) for k symbols.
inline HuffmanCodebook build_huffman_codebook(const std::map<char, std::uint64_t>& frequencies) {
    using NodePtr = std::shared_ptr<detail::HuffmanNode>;
    std::priority_queue<NodePtr, std::vector<NodePtr>, detail::HuffmanNodeGreater> queue;

    for (const auto& [symbol, frequency] : frequencies) {
        if (frequency == 0) throw std::invalid_argument("Huffman frequencies must be positive");
        queue.push(std::make_shared<detail::HuffmanNode>(detail::HuffmanNode{
            frequency, symbol, symbol, true, nullptr, nullptr}));
    }

    if (queue.empty()) return HuffmanCodebook{};
    while (queue.size() > 1) {
        NodePtr left = queue.top(); queue.pop();
        NodePtr right = queue.top(); queue.pop();
        if (left->frequency > UINT64_MAX - right->frequency) {
            throw std::overflow_error("Huffman combined frequency exceeds uint64_t");
        }
        auto parent = std::make_shared<detail::HuffmanNode>();
        parent->frequency = left->frequency + right->frequency;
        parent->min_symbol = static_cast<unsigned char>(left->min_symbol) <
                                     static_cast<unsigned char>(right->min_symbol)
                                 ? left->min_symbol : right->min_symbol;
        parent->leaf = false;
        parent->left = std::move(left);
        parent->right = std::move(right);
        queue.push(std::move(parent));
    }

    std::map<char, std::string> codes;
    const NodePtr root = queue.top();
    std::function<void(const NodePtr&, const std::string&)> visit =
        [&](const NodePtr& node, const std::string& prefix) {
            if (node->leaf) {
                codes[node->symbol] = prefix.empty() ? "0" : prefix;
                return;
            }
            visit(node->left, prefix + '0');
            visit(node->right, prefix + '1');
        };
    visit(root, {});
    return HuffmanCodebook(std::move(codes));
}

} // namespace project47
