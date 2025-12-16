#ifndef __SHADER_PIECE_SKIN_H__
#define __SHADER_PIECE_SKIN_H__

#include "PieceSkin.h"
#include "GameConfig.h"

class ShaderPieceSkin : public PieceSkin {
public:
    static ShaderPieceSkin* create(const GameConfig& config);
    
    cocos2d::Node* createNode(const std::string& imageFile, const cocos2d::Rect& rect) override;
    void updateState(const PieceState& state) override;
    void updateAppearance(const PuzzlePiece* piece) override;
    cocos2d::Node* getNode() const override { return _sprite; }

protected:
    ShaderPieceSkin(const GameConfig& config);
    bool initShader();

private:
    GameConfig _config;
    cocos2d::Sprite* _sprite;
    cocos2d::GLProgramState* _glProgramState;
};

#endif // __SHADER_PIECE_SKIN_H__
