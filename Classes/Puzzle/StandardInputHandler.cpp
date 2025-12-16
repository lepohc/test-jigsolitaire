#include "StandardInputHandler.h"

bool StandardInputHandler::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) {
    if (!_delegate) return false;
    
    cocos2d::Vec2 localPos = _delegate->getBoardNode()->convertToNodeSpace(touch->getLocation());
    _touchStartPos = touch->getLocation(); // 世界坐标用于增量计算（如果需要），或者直接使用局部坐标
    
    _selectedPiece = nullptr;
    _draggingPieces.clear();
    _dragOffsets.clear();

    // 命中测试
    auto& pieces = _delegate->getPieces();
    for (auto& piece : pieces) {
        auto node = _delegate->getPieceNode(piece);
        if (node && node->getBoundingBox().containsPoint(localPos)) {
            _selectedPiece = piece;
            break;
        }
    }

    if (_selectedPiece) {
        // 获取组
        _draggingPieces = _delegate->getGroup(_selectedPiece);
        
        // 设置拖拽
        for (auto& p : _draggingPieces) {
            auto node = _delegate->getPieceNode(p);
            if (node) {
                _dragOffsets[p] = node->getPosition() - localPos;
                _delegate->reorderPiece(p, 100); // 带到最前
            }
        }
        return true;
    }
    return false;
}

void StandardInputHandler::onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event) {
    if (!_draggingPieces.empty() && _delegate) {
        cocos2d::Vec2 localPos = _delegate->getBoardNode()->convertToNodeSpace(touch->getLocation());
        
        for (auto& p : _draggingPieces) {
            auto node = _delegate->getPieceNode(p);
            if (node) {
                node->setPosition(localPos + _dragOffsets[p]);
            }
        }
    }
}

void StandardInputHandler::onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event) {
    if (!_draggingPieces.empty() && _delegate) {
        cocos2d::Vec2 localPos = _delegate->getBoardNode()->convertToNodeSpace(touch->getLocation());
        
        // 计算从开始的总偏移量（近似值，或者只传递当前状态）
        // 实际上，代理需要知道它们最终在哪里。
        // 精灵已经在“放下”的位置。
        // 我们可以计算选定拼图块从*原始*位置的增量。
        
        // 但是等等，代理需要知道应用到逻辑网格的增量。
        // 让我们传递*选定拼图块*从其*原始逻辑位置*的总偏移量。
        
        auto node = _delegate->getPieceNode(_selectedPiece);
        cocos2d::Vec2 currentPos = node ? node->getPosition() : _selectedPiece->position;
        cocos2d::Vec2 originalPos = _selectedPiece->position;
        cocos2d::Vec2 totalOffset = currentPos - originalPos;
        
        _delegate->onDragEnded(_draggingPieces, totalOffset);
        
        _draggingPieces.clear();
        _dragOffsets.clear();
        _selectedPiece = nullptr;
    }
}
