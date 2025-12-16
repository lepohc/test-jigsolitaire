#include "ShaderPieceSkin.h"
#include "PuzzlePiece.h"

ShaderPieceSkin* ShaderPieceSkin::create(const GameConfig& config) {
    ShaderPieceSkin* ret = new (std::nothrow) ShaderPieceSkin(config);
    if (ret) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

ShaderPieceSkin::ShaderPieceSkin(const GameConfig& config) : _config(config), _sprite(nullptr), _glProgramState(nullptr) {}

cocos2d::Node* ShaderPieceSkin::createNode(const std::string& imageFile, const cocos2d::Rect& rect) {
    _sprite = cocos2d::Sprite::create(imageFile, rect);
    if (_sprite) {
        initShader();
    }
    return _sprite;
}

bool ShaderPieceSkin::initShader() {
    if (!_sprite) return false;

    std::string vertPath = "shaders/RoundedBorder.vert";
    std::string fragPath = "shaders/RoundedBorder.frag";
    
    if (!cocos2d::FileUtils::getInstance()->isFileExist(vertPath)) {
        vertPath = "RoundedBorder.vert";
        fragPath = "RoundedBorder.frag";
    }
    
    if (!cocos2d::FileUtils::getInstance()->isFileExist(vertPath)) {
        vertPath = "Resources/shaders/RoundedBorder.vert";
        fragPath = "Resources/shaders/RoundedBorder.frag";
    }

    auto glProgram = cocos2d::GLProgram::createWithFilenames(vertPath, fragPath);
    if (!glProgram) {
        cocos2d::log("ShaderPieceSkin: Failed to load shader files.");
        return false;
    }

    _glProgramState = cocos2d::GLProgramState::create(glProgram);
    _sprite->setGLProgramState(_glProgramState);
    
    // 设置初始 Uniforms
    cocos2d::Size size = _sprite->getContentSize();
    _glProgramState->setUniformVec2("u_size", cocos2d::Vec2(size.width, size.height));
    _glProgramState->setUniformFloat("u_borderWidth", _config.borderWidth);
    _glProgramState->setUniformVec4("u_borderColor", _config.borderColor);
    
    // 默认：显示所有边框和圆角
    _glProgramState->setUniformVec4("u_cornerRadii", cocos2d::Vec4(_config.cornerRadius, _config.cornerRadius, _config.cornerRadius, _config.cornerRadius));
    _glProgramState->setUniformVec4("u_borderSides", cocos2d::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    // 计算 UV 矩形
    auto quad = _sprite->getQuad();
    float minU = std::min(quad.tl.texCoords.u, quad.br.texCoords.u);
    float maxU = std::max(quad.tl.texCoords.u, quad.br.texCoords.u);
    float minV = std::min(quad.tl.texCoords.v, quad.br.texCoords.v);
    float maxV = std::max(quad.tl.texCoords.v, quad.br.texCoords.v);
    
    float widthU = maxU - minU;
    float heightV = maxV - minV;
    if (widthU <= 0.0001f) widthU = 1.0f;
    if (heightV <= 0.0001f) heightV = 1.0f;

    _glProgramState->setUniformVec4("u_uvRect", cocos2d::Vec4(minU, minV, widthU, heightV));
    
    return true;
}

void ShaderPieceSkin::updateState(const PieceState& state) {
    if (!_glProgramState) return;

    // u_borderSides: 上, 右, 下, 左 (1.0 = 显示, 0.0 = 隐藏)
    cocos2d::Vec4 borderSides(1.0f, 1.0f, 1.0f, 1.0f);
    if (state.connectedTop) borderSides.x = 0.0f;
    if (state.connectedRight) borderSides.y = 0.0f;
    if (state.connectedBottom) borderSides.z = 0.0f;
    if (state.connectedLeft) borderSides.w = 0.0f;
    
    _glProgramState->setUniformVec4("u_borderSides", borderSides);

    // u_cornerRadii: 右上, 右下, 左上, 左下
    float r = _config.cornerRadius;
    cocos2d::Vec4 cornerRadii(r, r, r, r);

    if (state.connectedTop || state.connectedRight) cornerRadii.x = 0.0f; 
    if (state.connectedBottom || state.connectedRight) cornerRadii.y = 0.0f; 
    if (state.connectedTop || state.connectedLeft) cornerRadii.z = 0.0f; 
    if (state.connectedBottom || state.connectedLeft) cornerRadii.w = 0.0f; 

    _glProgramState->setUniformVec4("u_cornerRadii", cornerRadii);
}

void ShaderPieceSkin::updateAppearance(const PuzzlePiece* piece) {
    if (!_glProgramState) return;

    // 1. 更新圆角 (根据连接状态)
    // Shader 顺序: x:TopRight, y:BottomRight, z:TopLeft, w:BottomLeft
    float r = _config.cornerRadius;
    cocos2d::Vec4 cornerRadii(r, r, r, r);

    // 如果某一边连接了，则该边的两个角变直 (半径设为 0)
    // x: TopRight (受 Top 和 Right 影响)
    if (piece->connectedTop || piece->connectedRight) cornerRadii.x = 0.0f;
    // y: BottomRight (受 Bottom 和 Right 影响)
    if (piece->connectedBottom || piece->connectedRight) cornerRadii.y = 0.0f;
    // z: TopLeft (受 Top 和 Left 影响)
    if (piece->connectedTop || piece->connectedLeft) cornerRadii.z = 0.0f;
    // w: BottomLeft (受 Bottom 和 Left 影响)
    if (piece->connectedBottom || piece->connectedLeft) cornerRadii.w = 0.0f;

    _glProgramState->setUniformVec4("u_cornerRadii", cornerRadii);

    // 2. 更新描边显隐 (根据连接状态)
    // Shader: u_borderSides (Top, Right, Bottom, Left) -> (1, 2, 4, 8)
    // 0 表示显示，1 表示隐藏 (Shader 逻辑可能需要适配，或者这里传 float 数组)
    // 之前的实现是传 float u_borderSides[4]
    
    cocos2d::Vec4 borderSides(1.0f, 1.0f, 1.0f, 1.0f);
    if (piece->connectedTop) borderSides.x = 0.0f;
    if (piece->connectedRight) borderSides.y = 0.0f;
    if (piece->connectedBottom) borderSides.z = 0.0f;
    if (piece->connectedLeft) borderSides.w = 0.0f;
    
    _glProgramState->setUniformVec4("u_borderSides", borderSides);
}
