#ifndef __PUZZLE_GENERATOR_H__
#define __PUZZLE_GENERATOR_H__

#include "PuzzlePiece.h"
#include "GameConfig.h"
#include <vector>

class PuzzleGenerator {
public:
    virtual ~PuzzleGenerator() {}
    
    // 在棋盘上排列拼图块（设置它们的初始位置）
    virtual void arrangePieces(std::vector<PuzzlePiece*>& pieces, const cocos2d::Size& boardSize) = 0;
};

#endif // __PUZZLE_GENERATOR_H__
