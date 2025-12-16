#include "BoardModule.h"
#include "Puzzle/ShaderPieceSkin.h"
#include "Puzzle/RandomPuzzleGenerator.h"
#include "Puzzle/StandardInputHandler.h"
#include <algorithm>

BoardModule* BoardModule::create(int rowCount, int colCount, const std::string& imageFile) {
    BoardModule *pRet = new(std::nothrow) BoardModule(rowCount, colCount, imageFile);
    if (pRet && pRet->init()) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

BoardModule::BoardModule(int rowCount, int colCount, const std::string& imageFile)
    : puzzleImage(nullptr), _generator(nullptr), _inputHandler(nullptr), _rules(nullptr) {
    
    // 初始化配置
    _config.rows = rowCount;
    _config.cols = colCount;
    _config.imageFile = imageFile;
    _config.borderWidth = 8.0f;
    _config.borderColor = cocos2d::Vec4(0.8f, 0.8f, 0.8f, 1.0f);
    _config.cornerRadius = 20.0f;
    
    // 加载图片作为纹理参考（和尺寸）
    puzzleImage = cocos2d::Sprite::create(imageFile);
    if (puzzleImage) {
        puzzleImage->retain();
        cocos2d::log("BoardModule: Successfully loaded image '%s'. Size: %f x %f", imageFile.c_str(), puzzleImage->getContentSize().width, puzzleImage->getContentSize().height);
    } else {
        cocos2d::log("BoardModule: Failed to load image '%s'", imageFile.c_str());
    }
}

BoardModule::~BoardModule() {
    if (puzzleImage) {
        puzzleImage->release();
    }
    for (auto piece : pieces) {
        delete piece;
    }
    for (auto& pair : _pieceSkins) {
        if (pair.second) pair.second->release();
    }
    _pieceSkins.clear();
    
    if (_generator) delete _generator;
    if (_inputHandler) delete _inputHandler;
    if (_rules) delete _rules;
}

bool BoardModule::init() {
    if (!Node::init()) {
        return false;
    }

    // 初始化组件
    _generator = new RandomPuzzleGenerator();
    _inputHandler = new StandardInputHandler();
    _inputHandler->setDelegate(this);
    _rules = new PuzzleRules();

    // 设置输入监听器
    auto touchListener = cocos2d::EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = CC_CALLBACK_2(BoardModule::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(BoardModule::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(BoardModule::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    if (puzzleImage) {
        generatePuzzle();
    }
    return true;
}

void BoardModule::generatePuzzle() {
    if (!puzzleImage) return;

    this->setContentSize(puzzleImage->getContentSize());
    auto pieceSize = getPieceSize();

    for (int row = 0; row < _config.rows; ++row) {
        for (int col = 0; col < _config.cols; ++col) {
            // 创建逻辑拼图块
            PuzzlePiece* piece = new PuzzlePiece(row * _config.cols + col, row, col);
            
            // 创建皮肤
            ShaderPieceSkin* skin = ShaderPieceSkin::create(_config);
            
            // 创建视觉节点
            cocos2d::Rect rect(col * pieceSize.width, row * pieceSize.height, pieceSize.width, pieceSize.height);
            
            cocos2d::Node* node = skin->createNode(_config.imageFile, rect);
            if (node) {
                this->addChild(node);
                _pieceSkins[piece] = skin;
                skin->retain(); // 保持引用
                
                // 初始位置（将被打乱）
                piece->position = getPositionForGrid(row, col);
                node->setPosition(piece->position);
            }
            
            pieces.push_back(piece);
        }
    }

    // 排列拼图块（打乱）
    if (_generator) {
        _generator->arrangePieces(pieces, this->getContentSize());
    }
    
    // 更新视觉效果以匹配打乱后的位置
    for (auto piece : pieces) {
        auto skin = _pieceSkins[piece];
        if (skin) {
            auto node = skin->getNode();
            if (node) node->setPosition(piece->position);
        }
        checkMerge(piece); // 检查是否恰好落在正确位置
    }
}

void BoardModule::resetBoard() {
    if (_generator) {
        _generator->arrangePieces(pieces, this->getContentSize());
    }
    
    for (auto piece : pieces) {
        piece->merged = false;
        piece->groupId = -1;
        piece->connectedTop = false;
        piece->connectedBottom = false;
        piece->connectedLeft = false;
        piece->connectedRight = false;
        
        auto skin = _pieceSkins[piece];
        if (skin) {
            auto node = skin->getNode();
            if (node) {
                node->setPosition(piece->position);
                this->reorderChild(node, 0);
            }
        }
    }
    
    if (_rules && puzzleImage) {
        float pieceWidth = puzzleImage->getContentSize().width / _config.cols;
        float pieceHeight = puzzleImage->getContentSize().height / _config.rows;
        
        _rules->updateConnections(pieces, pieceWidth, pieceHeight);
        _rules->updateGroups(pieces, pieceWidth, pieceHeight);
        
        for (auto piece : pieces) {
            auto skin = _pieceSkins[piece];
            if (skin) {
                skin->updateAppearance(piece);
            }
        }
    }
    
    for (auto& piece : pieces) {
        checkMerge(piece);
    }
}

// 输入处理
bool BoardModule::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) {
    if (_inputHandler) return _inputHandler->onTouchBegan(touch, event);
    return false;
}

void BoardModule::onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event) {
    if (_inputHandler) _inputHandler->onTouchMoved(touch, event);
}

void BoardModule::onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event) {
    if (_inputHandler) _inputHandler->onTouchEnded(touch, event);
}

// BoardDelegate 实现
std::vector<PuzzlePiece*> BoardModule::getGroup(PuzzlePiece* piece) {
    std::vector<PuzzlePiece*> group;
    if (!piece) return group;
    
    int currentGroupId = piece->groupId;
    if (currentGroupId != -1) {
        for (auto& p : pieces) {
            if (p->groupId == currentGroupId) {
                group.push_back(p);
            }
        }
    } else {
        group.push_back(piece);
    }
    return group;
}

void BoardModule::reorderPiece(PuzzlePiece* piece, int zOrder) {
    auto skin = _pieceSkins[piece];
    if (skin && skin->getNode()) {
        this->reorderChild(skin->getNode(), zOrder);
    }
}

cocos2d::Node* BoardModule::getPieceNode(PuzzlePiece* piece) {
    auto it = _pieceSkins.find(piece);
    if (it != _pieceSkins.end() && it->second) {
        return it->second->getNode();
    }
    return nullptr;
}

void BoardModule::onDragEnded(const std::vector<PuzzlePiece*>& draggingPieces, const cocos2d::Vec2& totalOffset) {
    if (draggingPieces.empty() || !_rules || !puzzleImage) return;

    auto pieceSize = getPieceSize();
    
    // 1. 计算移动方案 (纯逻辑)
    auto moveResults = _rules->calculateMove(
        draggingPieces, 
        totalOffset, 
        pieces, 
        _config.rows, 
        _config.cols, 
        pieceSize.width, 
        pieceSize.height
    );

    // 2. 执行移动 (更新逻辑位置和视觉动画)
    for (const auto& result : moveResults) {
        result.piece->position = result.targetPosition;
        
        auto skin = _pieceSkins[result.piece];
        if (skin) {
            auto node = skin->getNode();
            if (node) {
                if (result.animate) {
                    node->runAction(cocos2d::MoveTo::create(0.2f, result.targetPosition));
                } else {
                    // 对于拖拽的块，我们通常也做一个平滑的吸附动画
                    node->runAction(cocos2d::MoveTo::create(0.2f, result.targetPosition));
                }
                this->reorderChild(node, 0);
            }
        }
    }
    
    // 3. 更新状态和检查胜利
    _rules->updateConnections(pieces, pieceSize.width, pieceSize.height);
    _rules->updateGroups(pieces, pieceSize.width, pieceSize.height);
    
    for (auto piece : pieces) {
        auto skin = _pieceSkins[piece];
        if (skin) {
            skin->updateAppearance(piece);
        }
        checkMerge(piece);
    }
    
    if (_rules->checkWin(pieces)) {
        cocos2d::log("Puzzle Solved!");
        if (onWinCallback) {
            onWinCallback();
        }
    }
}

// 内部逻辑
bool BoardModule::checkMerge(PuzzlePiece* piece) {
    if (!puzzleImage) return false;

    // 目标位置是该拼图块在完整图片中的正确位置
    // 注意：PuzzlePiece::row/col 是正确答案的索引
    cocos2d::Vec2 targetPos = getPositionForGrid(piece->row, piece->col);

    if (piece->position.distance(targetPos) < 1.0f) {
        piece->merged = true;
        return true;
    }
    
    piece->merged = false;
    return false;
}

void BoardModule::setOnWinCallback(const std::function<void()>& callback) {
    onWinCallback = callback;
}

cocos2d::Size BoardModule::getPieceSize() const {
    if (!puzzleImage) return cocos2d::Size::ZERO;
    return cocos2d::Size(
        puzzleImage->getContentSize().width / _config.cols,
        puzzleImage->getContentSize().height / _config.rows
    );
}

cocos2d::Vec2 BoardModule::getPositionForGrid(int row, int col) const {
    auto size = getPieceSize();
    // 逻辑行 0 是顶部，物理 Y 轴向上增加
    float x = size.width * (col + 0.5f);
    float y = size.height * ((_config.rows - 1 - row) + 0.5f);
    return cocos2d::Vec2(x, y);
}

