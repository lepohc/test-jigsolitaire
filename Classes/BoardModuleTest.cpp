#include "BoardModuleTest.h"

BoardModuleTest::BoardModuleTest() : board(nullptr) {}

BoardModuleTest::~BoardModuleTest() {}

bool BoardModuleTest::init() {
    if (!Scene::init()) {
        return false;
    }

    // 创建并初始化 BoardModule
    // 使用 HelloWorld.png 进行测试，确保图片存在且尺寸合适
    //    board = BoardModule::create(4, 4, "HelloWorld.png");
    board = BoardModule::create(4, 4, "test.png");
    
    // 居中显示棋盘
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    board->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
    board->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    
    // 设置胜利回调
    board->setOnWinCallback([this]() {
        this->showWinUI();
    });
    
    this->addChild(board);

    return true;
}

void BoardModuleTest::showWinUI() {
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();

    // 胜利标签
    auto winLabel = cocos2d::Label::createWithSystemFont("YOU WIN!", "Arial", 64);
    winLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 50);
    winLabel->setColor(cocos2d::Color3B::RED);
    winLabel->setTag(999); 
    this->addChild(winLabel, 100);

    // 再玩一次按钮
    auto playAgainLabel = cocos2d::Label::createWithSystemFont("Play Again", "Arial", 40);
    auto playAgainItem = cocos2d::MenuItemLabel::create(playAgainLabel, CC_CALLBACK_1(BoardModuleTest::onPlayAgain, this));
    playAgainItem->setPosition(visibleSize.width / 2, visibleSize.height / 2 - 50);
    
    auto menu = cocos2d::Menu::create(playAgainItem, nullptr);
    menu->setPosition(cocos2d::Vec2::ZERO);
    menu->setTag(998); 
    this->addChild(menu, 100);
}

void BoardModuleTest::onPlayAgain(cocos2d::Ref* sender) {
    // 移除 UI
    this->removeChildByTag(999); 
    this->removeChildByTag(998); 
    
    // 重置棋盘
    if (board) {
        board->resetBoard();
    }
}
