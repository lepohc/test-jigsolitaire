#ifndef __BOARD_MODULE_H__
#define __BOARD_MODULE_H__

#include "cocos2d.h"
#include "Puzzle/GameConfig.h"
#include "Puzzle/PuzzlePiece.h"
#include "Puzzle/PieceSkin.h"
#include "Puzzle/PuzzleGenerator.h"
#include "Puzzle/InputHandler.h"
#include "Puzzle/PuzzleRules.h"
#include <vector>
#include <functional>

class BoardModule : public cocos2d::Node, public BoardDelegate {
public:
    static BoardModule* create(int rowCount, int colCount, const std::string& imageFile);

    BoardModule(int rowCount, int colCount, const std::string& imageFile);
    ~BoardModule();
    bool init() override;

    // 游戏逻辑
    void generatePuzzle();
    void resetBoard();
    
    // 输入处理
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    // BoardDelegate 实现
    std::vector<PuzzlePiece*> getGroup(PuzzlePiece* piece) override;
    void onDragEnded(const std::vector<PuzzlePiece*>& pieces, const cocos2d::Vec2& totalOffset) override;
    std::vector<PuzzlePiece*>& getPieces() override { return pieces; }
    cocos2d::Node* getBoardNode() override { return this; }
    void reorderPiece(PuzzlePiece* piece, int zOrder) override;

    // 回调
    void setOnWinCallback(const std::function<void()>& callback);

    // 辅助方法
    cocos2d::Size getPieceSize() const;
    cocos2d::Vec2 getPositionForGrid(int row, int col) const;
    cocos2d::Node* getPieceNode(PuzzlePiece* piece) override;

private:
    // 内部逻辑
    bool checkMerge(PuzzlePiece* piece);

    GameConfig _config;
    cocos2d::Sprite* puzzleImage;
    std::vector<PuzzlePiece*> pieces;
    std::map<PuzzlePiece*, PieceSkin*> _pieceSkins;
    
    PuzzleGenerator* _generator;
    InputHandler* _inputHandler;
    PuzzleRules* _rules;
    
    std::function<void()> onWinCallback;
};

#endif // __BOARD_MODULE_H__
