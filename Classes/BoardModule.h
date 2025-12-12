#include "cocos2d.h"
#include <vector>
#include <map>
#include <functional>

class PuzzlePiece {
public:
    int id;          // 拼图块唯一ID
    int row, col;    // 拼图块在正确位置的行列
    cocos2d::Vec2 position; // 当前坐标
    bool merged;     // 是否已经合并
    int groupId;     // 所属拼图组
    cocos2d::Sprite* sprite; // 对应的精灵
    
    // 连接状态
    bool connectedTop;
    bool connectedBottom;
    bool connectedLeft;
    bool connectedRight;

    PuzzlePiece(int id, int row, int col) 
        : id(id), row(row), col(col), merged(false), groupId(-1), sprite(nullptr),
          connectedTop(false), connectedBottom(false), connectedLeft(false), connectedRight(false) {}
};

class BoardModule : public cocos2d::Node {
public:
    BoardModule(int rowCount, int colCount, const std::string& imageFile);
    ~BoardModule();
    bool init() override;

    // 游戏逻辑
    void generatePuzzle();
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    void swapPieces(PuzzlePiece* pieceA, PuzzlePiece* pieceB);
    bool checkMerge(PuzzlePiece* piece);
    bool checkWin();
    
    // 新增：更新连接状态和视觉效果
    void updateConnections();
    void updateVisuals();
    void updateGroups();
    
    // 重置游戏
    void resetBoard();
    
    void setOnWinCallback(const std::function<void()>& callback);

    static BoardModule* create(int rowCount, int colCount, const std::string& imageFile);

private:
    int rowCount, colCount;            // 拼图的行数和列数
    std::vector<PuzzlePiece*> pieces;  // 存储拼图块
    cocos2d::Sprite* puzzleImage;      // 显示拼图的完整图片
    PuzzlePiece* selectedPiece;        // 当前被选中的拼图块
    cocos2d::Vec2 touchStartPos;       // 初始触摸位置
    
    // 拖动相关
    std::vector<PuzzlePiece*> draggingPieces; // 当前正在拖动的一组拼图块
    std::map<PuzzlePiece*, cocos2d::Vec2> dragOffsets; // 拖动时每个拼图块相对于触摸点的偏移
    
    std::function<void()> onWinCallback;
};
