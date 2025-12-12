#include "cocos2d.h"
#include "BoardModule.h"

class BoardModuleTest : public cocos2d::Scene {
public:
    BoardModuleTest();
    virtual ~BoardModuleTest();

    bool init() override;
    
    void showWinUI();
    void onPlayAgain(cocos2d::Ref* sender);

    CREATE_FUNC(BoardModuleTest);

private:
    BoardModule* board;  // 拼图模块
    cocos2d::EventListenerTouchOneByOne* _touchListener;
};
