#ifndef __GAME_CONFIG_H__
#define __GAME_CONFIG_H__

#include "cocos2d.h"
#include "json/document.h"

struct GameConfig {
    // 棋盘设置
    int rows = 4;
    int cols = 4;
    std::string imageFile = "test.png";
    
    // 视觉设置
    float borderWidth = 8.0f;
    float cornerRadius = 20.0f;
    cocos2d::Vec4 borderColor = cocos2d::Vec4(0.8f, 0.8f, 0.8f, 1.0f);
    
    // 游戏玩法设置
    float snapDistance = 1.0f; // 吸附到网格/合并的距离
    float neighborThresholdRatio = 0.2f; // 检测邻居的拼图块宽度百分比

    static GameConfig load(const std::string& filename) {
        GameConfig config;
        // 在实际实现中，在此处读取 JSON 文件。
        // 目前，我们返回默认值或硬编码值用于测试。
        // cocos2d::FileUtils::getInstance()->getStringFromFile(filename);
        return config;
    }
};

#endif // __GAME_CONFIG_H__
