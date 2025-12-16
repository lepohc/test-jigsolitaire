#ifndef __INPUT_HANDLER_H__
#define __INPUT_HANDLER_H__

#include "cocos2d.h"
#include "PuzzlePiece.h"
#include <vector>

class BoardDelegate {
public:
    virtual ~BoardDelegate() {}
    // 返回与触摸的拼图块关联的组
    virtual std::vector<PuzzlePiece*> getGroup(PuzzlePiece* piece) = 0;
    
    // 获取拼图块对应的视觉节点
    virtual cocos2d::Node* getPieceNode(PuzzlePiece* piece) = 0;
    
    // 拖拽结束时调用。offset 是从开始的总移动量。
    // 处理程序已经更新了精灵的视觉位置。
    // 代理现在应该验证移动，更新逻辑位置，并吸附视觉效果。
    virtual void onDragEnded(const std::vector<PuzzlePiece*>& pieces, const cocos2d::Vec2& totalOffset) = 0;
    
    virtual std::vector<PuzzlePiece*>& getPieces() = 0;
    virtual cocos2d::Node* getBoardNode() = 0;
    virtual void reorderPiece(PuzzlePiece* piece, int zOrder) = 0;
};

class InputHandler {
public:
    virtual ~InputHandler() {}
    
    virtual void setDelegate(BoardDelegate* delegate) { _delegate = delegate; }
    
    virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) = 0;
    virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event) = 0;
    virtual void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event) = 0;

protected:
    BoardDelegate* _delegate = nullptr;
};

#endif // __INPUT_HANDLER_H__
