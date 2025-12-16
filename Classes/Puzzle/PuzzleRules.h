#ifndef __PUZZLE_RULES_H__
#define __PUZZLE_RULES_H__

#include "PuzzlePiece.h"
#include <vector>

struct MoveResult {
    PuzzlePiece* piece;
    cocos2d::Vec2 targetPosition;
    bool animate;
};

/**
 * @brief 拼图规则类
 * 负责处理拼图的核心游戏逻辑，如连接判定、分组更新、胜利检测。
 * 纯逻辑类，不依赖 Cocos2d 的渲染部分。
 */
class PuzzleRules {
public:
    /**
     * @brief 计算拖拽结束后的移动方案
     * 包括计算目标位置、处理碰撞置换等核心玩法逻辑
     */
    std::vector<MoveResult> calculateMove(
        const std::vector<PuzzlePiece*>& draggingPieces,
        const cocos2d::Vec2& totalOffset,
        const std::vector<PuzzlePiece*>& allPieces,
        int rows, int cols,
        float pieceWidth, float pieceHeight
    );

    /**
     * @brief 更新所有拼图块的连接状态 (上下左右是否相邻)
     * @param pieces 拼图块列表
     * @param pieceWidth 拼图块宽度
     * @param pieceHeight 拼图块高度
     */
    void updateConnections(const std::vector<PuzzlePiece*>& pieces, float pieceWidth, float pieceHeight);

    /**
     * @brief 更新拼图块的分组 (BFS 算法)
     * 相连的拼图块会被归为同一个 Group ID
     * @param pieces 拼图块列表
     * @param pieceWidth 拼图块宽度
     * @param pieceHeight 拼图块高度
     */
    void updateGroups(const std::vector<PuzzlePiece*>& pieces, float pieceWidth, float pieceHeight);

    /**
     * @brief 检查是否胜利 (所有拼图块都已归位)
     * @param pieces 拼图块列表
     */
    bool checkWin(const std::vector<PuzzlePiece*>& pieces);
};

#endif // __PUZZLE_RULES_H__
