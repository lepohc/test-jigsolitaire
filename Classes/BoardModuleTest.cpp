#include "BoardModuleTest.h"

BoardModuleTest::BoardModuleTest() : board(nullptr), _touchListener(nullptr) {}

BoardModuleTest::~BoardModuleTest() {}

bool BoardModuleTest::init() {
    if (!Scene::init()) {
        return false;
    }

    // 创建并初始化 BoardModule
    // 使用 HelloWorld.png 进行测试，确保图片存在且尺寸合适
//    board = BoardModule::create(4, 4, "HelloWorld.png");
    board = BoardModule::create(4, 4, "test.png");
    
    // Center the board
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    board->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
    board->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    
    // 设置胜利回调
    board->setOnWinCallback([this]() {
        this->showWinUI();
    });
    
    this->addChild(board);

    // 注册触摸事件
    _touchListener = cocos2d::EventListenerTouchOneByOne::create();
    _touchListener->onTouchBegan = CC_CALLBACK_2(BoardModule::onTouchBegan, board);
    _touchListener->onTouchMoved = CC_CALLBACK_2(BoardModule::onTouchMoved, board);
    _touchListener->onTouchEnded = CC_CALLBACK_2(BoardModule::onTouchEnded, board);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);

    return true;
}

void BoardModuleTest::showWinUI() {
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();

    // Win Label
    auto winLabel = cocos2d::Label::createWithSystemFont("YOU WIN!", "Arial", 64);
    winLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 50);
    winLabel->setColor(cocos2d::Color3B::RED);
    winLabel->setTag(999); 
    this->addChild(winLabel, 100);

    // Play Again Button
    auto playAgainLabel = cocos2d::Label::createWithSystemFont("Play Again", "Arial", 40);
    auto playAgainItem = cocos2d::MenuItemLabel::create(playAgainLabel, CC_CALLBACK_1(BoardModuleTest::onPlayAgain, this));
    playAgainItem->setPosition(visibleSize.width / 2, visibleSize.height / 2 - 50);
    
    auto menu = cocos2d::Menu::create(playAgainItem, nullptr);
    menu->setPosition(cocos2d::Vec2::ZERO);
    menu->setTag(998); 
    this->addChild(menu, 100);
    
    // Disable touch
    if (_touchListener) {
        _touchListener->setEnabled(false);
    }
}

void BoardModuleTest::onPlayAgain(cocos2d::Ref* sender) {
    // Remove UI
    this->removeChildByTag(999); 
    this->removeChildByTag(998); 
    
    // Reset Board
    if (board) {
        board->resetBoard();
    }
    
    // Re-enable touch
    if (_touchListener) {
        _touchListener->setEnabled(true);
    }
}
