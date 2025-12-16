#ifndef __RANDOM_PUZZLE_GENERATOR_H__
#define __RANDOM_PUZZLE_GENERATOR_H__

#include "PuzzleGenerator.h"
#include <random>
#include <algorithm>

class RandomPuzzleGenerator : public PuzzleGenerator {
public:
    void arrangePieces(std::vector<PuzzlePiece*>& pieces, const cocos2d::Size& boardSize) override {
        std::vector<cocos2d::Vec2> positions;
        for (auto piece : pieces) {
            positions.push_back(piece->position);
        }

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(positions.begin(), positions.end(), g);

        for (size_t i = 0; i < pieces.size(); ++i) {
            pieces[i]->position = positions[i];
            // 视觉更新现在由 BoardModule 处理
        }
    }
};

#endif // __RANDOM_PUZZLE_GENERATOR_H__
