#include "PuzzleRules.h"
#include <cmath>
#include <queue>
#include <algorithm>

std::vector<MoveResult> PuzzleRules::calculateMove(
    const std::vector<PuzzlePiece*>& draggingPieces,
    const cocos2d::Vec2& totalOffset,
    const std::vector<PuzzlePiece*>& allPieces,
    int rows, int cols,
    float pieceWidth, float pieceHeight
) {
    std::vector<MoveResult> results;
    if (draggingPieces.empty()) return results;

    // 计算网格增量
    int deltaCol = std::round(totalOffset.x / pieceWidth);
    int deltaRow = std::round(totalOffset.y / pieceHeight);

    bool isValidMove = true;
    std::vector<std::pair<PuzzlePiece*, cocos2d::Vec2>> movePlan;
    std::vector<cocos2d::Vec2> originalPositions;

    // 1. 检查边界并计算目标位置
    for (auto& p : draggingPieces) {
        // 从逻辑位置计算当前网格位置 (0,0 is bottom-left)
        int pStartCol = std::floor(p->position.x / pieceWidth);
        int pStartRow = std::floor(p->position.y / pieceHeight);
        
        int pTargetCol = pStartCol + deltaCol;
        int pTargetRow = pStartRow + deltaRow;

        // 检查边界
        if (pTargetCol < 0 || pTargetCol >= cols || pTargetRow < 0 || pTargetRow >= rows) {
            isValidMove = false;
            break;
        }
        
        float targetX = pieceWidth * (pTargetCol + 0.5f);
        float targetY = pieceHeight * (pTargetRow + 0.5f);
        movePlan.push_back({p, cocos2d::Vec2(targetX, targetY)});
        
        // 确保原始位置也是基于网格中心的，防止浮点数漂移
        float originalX = pieceWidth * (pStartCol + 0.5f);
        float originalY = pieceHeight * (pStartRow + 0.5f);
        originalPositions.push_back(cocos2d::Vec2(originalX, originalY));
    }

    if (isValidMove && (deltaCol != 0 || deltaRow != 0)) {
        // 2. 识别被置换的拼图块
        std::vector<PuzzlePiece*> displacedPieces;
        
        for (auto& move : movePlan) {
            cocos2d::Vec2 targetPos = move.second;
            
            for (auto& other : allPieces) {
                if (other->position.distance(targetPos) < 1.0f) {
                    bool isDragging = false;
                    for (auto& dp : draggingPieces) {
                        if (dp == other) {
                            isDragging = true;
                            break;
                        }
                    }
                    if (!isDragging) {
                        displacedPieces.push_back(other);
                    }
                    break;
                }
            }
        }

        // 3. 识别可用插槽
        std::vector<cocos2d::Vec2> availableSlots;
        for (auto& pos : originalPositions) {
            bool isFilledByGroup = false;
            for (auto& move : movePlan) {
                if (move.second.distance(pos) < 1.0f) {
                    isFilledByGroup = true;
                    break;
                }
            }
            if (!isFilledByGroup) {
                availableSlots.push_back(pos);
            }
        }
        
        // 生成移动结果 (拖拽块)
        for (auto& move : movePlan) {
            results.push_back({move.first, move.second, false});
        }

        // 4. 匹配策略：优先保持列不变 (X坐标接近)，其次距离最近
        struct MatchPair {
            size_t pieceIndex;
            size_t slotIndex;
            float score;
        };
        
        std::vector<MatchPair> pairs;
        for (size_t i = 0; i < displacedPieces.size(); ++i) {
            for (size_t j = 0; j < availableSlots.size(); ++j) {
                float dx = std::abs(displacedPieces[i]->position.x - availableSlots[j].x);
                float dy = std::abs(displacedPieces[i]->position.y - availableSlots[j].y);
                // 优先 X 轴对齐 (权重高)，其次 Y 轴距离
                float score = dx * 10000.0f + dy;
                pairs.push_back({i, j, score});
            }
        }
        
        std::sort(pairs.begin(), pairs.end(), [](const MatchPair& a, const MatchPair& b) {
            return a.score < b.score;
        });
        
        std::vector<bool> pieceUsed(displacedPieces.size(), false);
        std::vector<bool> slotUsed(availableSlots.size(), false);
        
        for (const auto& pair : pairs) {
            if (!pieceUsed[pair.pieceIndex] && !slotUsed[pair.slotIndex]) {
                results.push_back({displacedPieces[pair.pieceIndex], availableSlots[pair.slotIndex], true});
                pieceUsed[pair.pieceIndex] = true;
                slotUsed[pair.slotIndex] = true;
            }
        }
    } else {
        // 移动无效，或者没有移动。
        // 拖拽的块需要回到原来的位置。
        for (auto& p : draggingPieces) {
            results.push_back({p, p->position, true});
        }
    }

    return results;
}

void PuzzleRules::updateConnections(const std::vector<PuzzlePiece*>& pieces, float pieceWidth, float pieceHeight) {
    // 重置所有连接状态
    for (auto piece : pieces) {
        piece->connectedTop = false;
        piece->connectedBottom = false;
        piece->connectedLeft = false;
        piece->connectedRight = false;
    }

    float threshold = pieceWidth * 0.2f; 

    for (auto& pieceA : pieces) {
        for (auto& pieceB : pieces) {
            if (pieceA == pieceB) continue;

            // 检查右邻居 (pieceB 在 pieceA 右边)
            if (std::abs(pieceB->position.y - pieceA->position.y) < threshold &&
                std::abs(pieceB->position.x - (pieceA->position.x + pieceWidth)) < threshold) {
                
                // 逻辑上也应该是右邻居
                if (pieceA->row == pieceB->row && pieceA->col + 1 == pieceB->col) {
                    pieceA->connectedRight = true;
                    pieceB->connectedLeft = true;
                }
            }

            // 检查上邻居 (pieceB 在 pieceA 上面)
            // 注意：Y 轴向上增加。如果 pieceB 在 pieceA 上面，pieceB.y > pieceA.y
            // 逻辑行：Row 0 是顶部。如果 pieceB 在 pieceA 上面，pieceB.row = pieceA.row - 1
            if (std::abs(pieceB->position.x - pieceA->position.x) < threshold &&
                std::abs(pieceB->position.y - (pieceA->position.y + pieceHeight)) < threshold) {
                
                if (pieceA->col == pieceB->col && pieceB->row == pieceA->row - 1) {
                    pieceA->connectedTop = true;
                    pieceB->connectedBottom = true;
                }
            }
        }
    }
}

void PuzzleRules::updateGroups(const std::vector<PuzzlePiece*>& pieces, float pieceWidth, float pieceHeight) {
    for (auto& piece : pieces) {
        piece->groupId = -1;
    }

    int nextGroupId = 0;
    float threshold = pieceWidth * 0.2f;

    for (auto& piece : pieces) {
        if (piece->groupId == -1) {
            std::vector<PuzzlePiece*> q;
            q.push_back(piece);
            piece->groupId = nextGroupId;
            
            int head = 0;
            while(head < q.size()){
                PuzzlePiece* curr = q[head++];
                
                for(auto& neighbor : pieces) {
                    if (neighbor->groupId != -1) continue;
                    
                    bool right = curr->connectedRight && 
                                 std::abs(neighbor->position.y - curr->position.y) < threshold &&
                                 std::abs(neighbor->position.x - (curr->position.x + pieceWidth)) < threshold;
                                 
                    bool left = curr->connectedLeft &&
                                std::abs(neighbor->position.y - curr->position.y) < threshold &&
                                std::abs(neighbor->position.x - (curr->position.x - pieceWidth)) < threshold;
                                
                    bool top = curr->connectedTop &&
                               std::abs(neighbor->position.x - curr->position.x) < threshold &&
                               std::abs(neighbor->position.y - (curr->position.y + pieceHeight)) < threshold;
                               
                    bool bottom = curr->connectedBottom &&
                                  std::abs(neighbor->position.x - curr->position.x) < threshold &&
                                  std::abs(neighbor->position.y - (curr->position.y - pieceHeight)) < threshold;
                                  
                    if (right || left || top || bottom) {
                        neighbor->groupId = nextGroupId;
                        q.push_back(neighbor);
                    }
                }
            }
            nextGroupId++;
        }
    }
}

bool PuzzleRules::checkWin(const std::vector<PuzzlePiece*>& pieces) {
    for (auto piece : pieces) {
        if (!piece->merged) {
            return false;
        }
    }
    return true;
}
