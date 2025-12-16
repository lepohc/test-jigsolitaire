#ifndef __STANDARD_INPUT_HANDLER_H__
#define __STANDARD_INPUT_HANDLER_H__

#include "InputHandler.h"
#include <map>

class StandardInputHandler : public InputHandler {
public:
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) override;
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event) override;
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event) override;

private:
    PuzzlePiece* _selectedPiece = nullptr;
    cocos2d::Vec2 _touchStartPos;
    std::vector<PuzzlePiece*> _draggingPieces;
    std::map<PuzzlePiece*, cocos2d::Vec2> _dragOffsets; // 从触摸点到拼图块中心的偏移
};

#endif // __STANDARD_INPUT_HANDLER_H__
