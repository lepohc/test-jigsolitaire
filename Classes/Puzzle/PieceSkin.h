#ifndef __PIECE_SKIN_H__
#define __PIECE_SKIN_H__

#include "cocos2d.h"

class PuzzlePiece;

// 更新所需数据的前向声明
struct PieceState {
    bool connectedTop;
    bool connectedBottom;
    bool connectedLeft;
    bool connectedRight;
};

class PieceSkin : public cocos2d::Ref {
public:
    virtual ~PieceSkin() {}
    
    // 创建精灵/节点的工厂方法
    virtual cocos2d::Node* createNode(const std::string& imageFile, const cocos2d::Rect& rect) = 0;
    
    // 根据连接状态更新视觉效果
    virtual void updateState(const PieceState& state) = 0;

    /**
     * @brief 根据拼图块的逻辑状态更新视觉表现
     * 例如：根据连接状态改变圆角，根据选中状态改变描边颜色
     */
    virtual void updateAppearance(const PuzzlePiece* piece) = 0;
    
    // 可选：高亮效果
    virtual void setHighlight(bool active) {}
    
    // 获取底层节点的访问器
    virtual cocos2d::Node* getNode() const = 0;
};

#endif // __PIECE_SKIN_H__
