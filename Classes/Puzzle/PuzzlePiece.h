#ifndef __PUZZLE_PIECE_H__
#define __PUZZLE_PIECE_H__

#include "cocos2d.h"

class PuzzlePiece {
public:
    int id;          // 唯一 ID
    int row, col;    // 正确的网格位置
    cocos2d::Vec2 position; // 当前逻辑位置
    bool merged;     // 是否在正确位置？
    int groupId;     // 组 ID
    
    // 连接状态
    bool connectedTop;
    bool connectedBottom;
    bool connectedLeft;
    bool connectedRight;

    PuzzlePiece(int id, int row, int col) 
        : id(id), row(row), col(col), merged(false), groupId(-1),
          connectedTop(false), connectedBottom(false), connectedLeft(false), connectedRight(false) {}
          
    ~PuzzlePiece() {}
};

#endif // __PUZZLE_PIECE_H__
