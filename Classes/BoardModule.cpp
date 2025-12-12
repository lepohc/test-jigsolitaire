#include "BoardModule.h"
#include <algorithm>
#include <random>
#include <vector>

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
    : rowCount(rowCount), colCount(colCount), puzzleImage(nullptr), selectedPiece(nullptr) {
    puzzleImage = cocos2d::Sprite::create(imageFile);
    if (puzzleImage) {
        puzzleImage->retain(); // Retain to prevent autorelease
        cocos2d::log("BoardModule: Successfully loaded image '%s'. Size: %f x %f", imageFile.c_str(), puzzleImage->getContentSize().width, puzzleImage->getContentSize().height);
        // Don't add puzzleImage to scene, we only use it for texture
        // this->addChild(puzzleImage); 
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
}

bool BoardModule::init() {
    // 初始化拼图模块
    if (!Node::init()) {
        return false;
    }

    if (puzzleImage) {
        generatePuzzle();  // 生成拼图
    }
    return true;
}

void BoardModule::generatePuzzle() {
    // Set the content size of the board to match the puzzle image
    this->setContentSize(puzzleImage->getContentSize());

    // 加载拼图并切割成小块
    float pieceWidth = puzzleImage->getContentSize().width / colCount;
    float pieceHeight = puzzleImage->getContentSize().height / rowCount;

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            // 创建拼图块
            PuzzlePiece* piece = new PuzzlePiece(row * colCount + col, row, col);
            
            // 创建一个 sprite 用于显示拼图块
            cocos2d::Sprite* sprite = cocos2d::Sprite::createWithTexture(puzzleImage->getTexture(), cocos2d::Rect(col * pieceWidth, row * pieceHeight, pieceWidth, pieceHeight));
            
            // FIX: Invert Y position because Row 0 (Top of Image) should be at Top of Screen (High Y)
            sprite->setPosition(pieceWidth * (col + 0.5f), pieceHeight * ((rowCount - 1 - row) + 0.5f));
            this->addChild(sprite);
            
            // 加载 Shader
            // 尝试不同的路径组合
            std::string vertPath = "shaders/RoundedBorder.vert";
            std::string fragPath = "shaders/RoundedBorder.frag";
            
            if (!cocos2d::FileUtils::getInstance()->isFileExist(vertPath)) {
                vertPath = "RoundedBorder.vert";
                fragPath = "RoundedBorder.frag";
            }
            
            if (!cocos2d::FileUtils::getInstance()->isFileExist(vertPath)) {
                 // Try with Resources prefix for some desktop environments
                vertPath = "Resources/shaders/RoundedBorder.vert";
                fragPath = "Resources/shaders/RoundedBorder.frag";
            }

            auto glProgram = cocos2d::GLProgram::createWithFilenames(vertPath, fragPath);
            if (glProgram) {
                // Use create() instead of getOrCreateWithGLProgram() to ensure each sprite has its own state.
                // This prevents batching, ensuring that a_position in the shader remains in local coordinates
                // instead of being transformed to world coordinates by the CPU batcher.
                auto glProgramState = cocos2d::GLProgramState::create(glProgram);
                sprite->setGLProgramState(glProgramState);
                
                // 设置 Uniforms
                glProgramState->setUniformVec2("u_size", cocos2d::Vec2(pieceWidth, pieceHeight));
                glProgramState->setUniformFloat("u_borderWidth", 8.0f); // 描边宽度
                glProgramState->setUniformVec4("u_borderColor", cocos2d::Vec4(0.8f, 0.8f, 0.8f, 1.0f)); // 浅灰色描边
                
                // 初始化圆角和边框显示状态 (默认全显示)
                glProgramState->setUniformVec4("u_cornerRadii", cocos2d::Vec4(20.0f, 20.0f, 20.0f, 20.0f));
                glProgramState->setUniformVec4("u_borderSides", cocos2d::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
                
                // 计算并设置 UV Rect
                
                // 计算并设置 UV Rect
                // 获取 Sprite 的 Quad 信息，其中包含纹理坐标
                auto quad = sprite->getQuad();
                float minU = std::min(quad.tl.texCoords.u, quad.br.texCoords.u);
                float maxU = std::max(quad.tl.texCoords.u, quad.br.texCoords.u);
                float minV = std::min(quad.tl.texCoords.v, quad.br.texCoords.v);
                float maxV = std::max(quad.tl.texCoords.v, quad.br.texCoords.v);
                
                // 防止除以零
                float widthU = maxU - minU;
                float heightV = maxV - minV;
                if (widthU <= 0.0001f) widthU = 1.0f;
                if (heightV <= 0.0001f) heightV = 1.0f;

                glProgramState->setUniformVec4("u_uvRect", cocos2d::Vec4(minU, minV, widthU, heightV));

            } else {
                cocos2d::log("BoardModule: Failed to load shader files from paths: %s, %s", vertPath.c_str(), fragPath.c_str());
            }
            
            piece->sprite = sprite;
            piece->position = sprite->getPosition();
            pieces.push_back(piece);
        }
    }

    // 收集所有位置
    std::vector<cocos2d::Vec2> positions;
    for (auto piece : pieces) {
        positions.push_back(piece->position);
    }

    // 随机打乱位置
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(positions.begin(), positions.end(), g);

    // 将打乱后的位置重新分配给拼图块
    for (size_t i = 0; i < pieces.size(); ++i) {
        pieces[i]->position = positions[i];
        pieces[i]->sprite->setPosition(positions[i]);
        checkMerge(pieces[i]); // Check if it happened to land in correct spot
    }
}

bool BoardModule::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) {
    touchStartPos = touch->getLocation();
    cocos2d::Vec2 localPos = this->convertToNodeSpace(touch->getLocation());
    
    selectedPiece = nullptr;
    draggingPieces.clear();
    dragOffsets.clear();

    for (auto& piece : pieces) {
        if (piece->sprite->getBoundingBox().containsPoint(localPos)) {
            selectedPiece = piece;
            break;
        }
    }

    if (selectedPiece) {
        // 找到所有同组的拼图块
        int currentGroupId = selectedPiece->groupId;
        if (currentGroupId != -1) {
            for (auto& p : pieces) {
                if (p->groupId == currentGroupId) {
                    draggingPieces.push_back(p);
                }
            }
        } else {
            draggingPieces.push_back(selectedPiece);
        }

        // 记录每个拼图块相对于触摸点的偏移，并提升层级
        for (auto& p : draggingPieces) {
            dragOffsets[p] = p->sprite->getPosition() - localPos;
            this->reorderChild(p->sprite, 100); // Bring to very front
        }
        return true;
    }
    return false;
}

void BoardModule::onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event) {
    if (!draggingPieces.empty()) {
        cocos2d::Vec2 localPos = this->convertToNodeSpace(touch->getLocation());
        
        // 移动整个组
        for (auto& p : draggingPieces) {
            p->sprite->setPosition(localPos + dragOffsets[p]);
        }
    }
}

void BoardModule::onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event) {
    if (!draggingPieces.empty()) {
        cocos2d::Vec2 localPos = this->convertToNodeSpace(touch->getLocation());
        
        // Calculate grid dimensions
        float pieceWidth = puzzleImage->getContentSize().width / colCount;
        float pieceHeight = puzzleImage->getContentSize().height / rowCount;

        // Determine the target grid position for the selected (anchor) piece
        // We use the center of the dragged sprite to find the nearest grid slot
        cocos2d::Vec2 anchorPos = selectedPiece->sprite->getPosition();
        int targetCol = std::floor(anchorPos.x / pieceWidth);
        int targetRow = std::floor(anchorPos.y / pieceHeight);

        // Calculate the grid offset from the original position
        // Note: We need to know the CURRENT grid position of the selected piece.
        // Since we don't store current grid indices explicitly, we derive them from 'position'.
        int startCol = std::floor(selectedPiece->position.x / pieceWidth);
        int startRow = std::floor(selectedPiece->position.y / pieceHeight);
        
        int deltaCol = targetCol - startCol;
        int deltaRow = targetRow - startRow;

        // Validate the move for the entire group
        bool isValidMove = true;
        std::vector<std::pair<PuzzlePiece*, cocos2d::Vec2>> movePlan; // Piece -> New Target Position
        std::vector<cocos2d::Vec2> originalPositions; // To fill with displaced pieces

        // 1. Check bounds and calculate target positions
        for (auto& p : draggingPieces) {
            int pStartCol = std::floor(p->position.x / pieceWidth);
            int pStartRow = std::floor(p->position.y / pieceHeight);
            
            int pTargetCol = pStartCol + deltaCol;
            int pTargetRow = pStartRow + deltaRow;

            // Check if out of bounds
            if (pTargetCol < 0 || pTargetCol >= colCount || pTargetRow < 0 || pTargetRow >= rowCount) {
                isValidMove = false;
                break;
            }
            
            float targetX = pieceWidth * (pTargetCol + 0.5f);
            float targetY = pieceHeight * (pTargetRow + 0.5f);
            movePlan.push_back({p, cocos2d::Vec2(targetX, targetY)});
            originalPositions.push_back(p->position);
        }

        if (isValidMove && (deltaCol != 0 || deltaRow != 0)) {
            // 2. Identify displaced pieces (Victims)
            std::vector<PuzzlePiece*> displacedPieces;
            
            for (auto& move : movePlan) {
                cocos2d::Vec2 targetPos = move.second;
                
                // Find who is currently at targetPos
                for (auto& other : pieces) {
                    // Use a small epsilon for float comparison
                    if (other->position.distance(targetPos) < 1.0f) {
                        // Check if this piece is part of the dragging group
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
                        break; // Found the occupant
                    }
                }
            }

            // 3. Identify available slots
            // We need to remove slots from originalPositions that are being filled by the dragging group itself.
            // If a group member moves to a position that was previously occupied by another group member,
            // that position is NOT available for displaced pieces.
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
            
            // Sort availableSlots and displacedPieces to match them deterministically
            // Sort by Y (Top to Bottom) then X (Left to Right)
            // Note: In Cocos2d, Y increases upwards. So Top is High Y.
            // We sort High Y to Low Y.
            std::sort(displacedPieces.begin(), displacedPieces.end(), [](PuzzlePiece* a, PuzzlePiece* b) {
                if (std::abs(a->position.y - b->position.y) > 1.0f) return a->position.y > b->position.y;
                return a->position.x < b->position.x;
            });
            
            std::sort(availableSlots.begin(), availableSlots.end(), [](const cocos2d::Vec2& a, const cocos2d::Vec2& b) {
                if (std::abs(a.y - b.y) > 1.0f) return a.y > b.y;
                return a.x < b.x;
            });
            
            // Apply moves for Dragging Pieces
            for (auto& move : movePlan) {
                move.first->position = move.second;
            }
            
            // Apply moves for Displaced Pieces
            for (size_t i = 0; i < displacedPieces.size(); ++i) {
                if (i < availableSlots.size()) {
                    displacedPieces[i]->position = availableSlots[i];
                    displacedPieces[i]->sprite->runAction(cocos2d::MoveTo::create(0.2f, displacedPieces[i]->position));
                }
            }
        }

        // Reset visual positions to logical positions (with animation)
        for (auto& p : draggingPieces) {
            p->sprite->runAction(cocos2d::MoveTo::create(0.2f, p->position));
            this->reorderChild(p->sprite, 0);
        }
        
        // 重新计算连接和组
        updateConnections();
        updateGroups();
        updateVisuals();
        
        // Check merge status for all pieces to update win condition
        for (auto& piece : pieces) {
            checkMerge(piece);
        }
        checkWin();

        draggingPieces.clear();
        dragOffsets.clear();
        selectedPiece = nullptr;
    }
}

void BoardModule::swapPieces(PuzzlePiece* pieceA, PuzzlePiece* pieceB) {
    // 交换拼图块位置
    std::swap(pieceA->position, pieceB->position);
    // 交换显示位置
    // pieceA->sprite->setPosition(pieceA->position);
    // pieceB->sprite->setPosition(pieceB->position);
    
    // 使用动画移动
    pieceA->sprite->runAction(cocos2d::MoveTo::create(0.2f, pieceA->position));
    pieceB->sprite->runAction(cocos2d::MoveTo::create(0.2f, pieceB->position));
}

bool BoardModule::checkMerge(PuzzlePiece* piece) {
    if (!puzzleImage) return false;

    float pieceWidth = puzzleImage->getContentSize().width / colCount;
    float pieceHeight = puzzleImage->getContentSize().height / rowCount;

    // 计算该拼图块的正确位置
    float targetX = pieceWidth * (piece->col + 0.5f);
    // FIX: Invert Y position because Row 0 (Top of Image) should be at Top of Screen (High Y)
    float targetY = pieceHeight * ((rowCount - 1 - piece->row) + 0.5f);

    // 检查当前位置是否接近正确位置
    if (piece->position.distance(cocos2d::Vec2(targetX, targetY)) < 1.0f) {
        piece->merged = true;
        // 可以添加一些视觉反馈，例如闪烁一下
        return true;
    }
    
    piece->merged = false;
    return false;
}

bool BoardModule::checkWin() {
    for (auto piece : pieces) {
        if (!piece->merged) {
            return false;
        }
    }
    
    cocos2d::log("Puzzle Solved!");
    
    if (onWinCallback) {
        onWinCallback();
    }
    
    return true;
}

void BoardModule::setOnWinCallback(const std::function<void()>& callback) {
    onWinCallback = callback;
}

void BoardModule::resetBoard() {
    // Shuffle positions
    std::vector<cocos2d::Vec2> positions;
    for (auto piece : pieces) {
        positions.push_back(piece->position);
    }
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(positions.begin(), positions.end(), g);

    for (size_t i = 0; i < pieces.size(); ++i) {
        pieces[i]->position = positions[i];
        pieces[i]->sprite->setPosition(positions[i]);
        pieces[i]->merged = false;
        pieces[i]->groupId = -1;
        pieces[i]->connectedTop = false;
        pieces[i]->connectedBottom = false;
        pieces[i]->connectedLeft = false;
        pieces[i]->connectedRight = false;
        
        this->reorderChild(pieces[i]->sprite, 0);
    }
    
    updateConnections();
    updateGroups();
    updateVisuals();
    
    for (auto& piece : pieces) {
        checkMerge(piece);
    }
}

void BoardModule::updateConnections() {
    // Reset connections
    for (auto& piece : pieces) {
        piece->connectedTop = false;
        piece->connectedBottom = false;
        piece->connectedLeft = false;
        piece->connectedRight = false;
    }

    if (!puzzleImage) return;
    float pieceWidth = puzzleImage->getContentSize().width / colCount;
    float pieceHeight = puzzleImage->getContentSize().height / rowCount;
    float threshold = pieceWidth * 0.2f; 

    for (auto& pieceA : pieces) {
        for (auto& pieceB : pieces) {
            if (pieceA == pieceB) continue;

            // Check Right Neighbor
            // Spatial check: B is to the right of A
            if (std::abs(pieceB->position.y - pieceA->position.y) < threshold &&
                std::abs(pieceB->position.x - (pieceA->position.x + pieceWidth)) < threshold) {
                
                // Logical check: B should be to the right of A
                if (pieceA->row == pieceB->row && pieceA->col + 1 == pieceB->col) {
                    pieceA->connectedRight = true;
                    pieceB->connectedLeft = true;
                }
            }

            // Check Top Neighbor (Y increases upwards in Cocos2d)
            // Spatial check: B is above A
            if (std::abs(pieceB->position.x - pieceA->position.x) < threshold &&
                std::abs(pieceB->position.y - (pieceA->position.y + pieceHeight)) < threshold) {
                
                // Logical check: B should be the vertical neighbor of A
                // Based on user feedback, the previous row+1 logic was incorrect.
                // This implies the row indices decrease as Y increases (Row 0 is Top).
                // So if B is above A, B.row should be A.row - 1.
                if (pieceA->col == pieceB->col && pieceB->row == pieceA->row - 1) {
                    pieceA->connectedTop = true;
                    pieceB->connectedBottom = true;
                }
            }
        }
    }
}

void BoardModule::updateGroups() {
    for (auto& piece : pieces) {
        piece->groupId = -1;
    }

    int nextGroupId = 0;
    for (auto& piece : pieces) {
        if (piece->groupId == -1) {
            // Start BFS
            std::vector<PuzzlePiece*> q;
            q.push_back(piece);
            piece->groupId = nextGroupId;
            
            int head = 0;
            while(head < q.size()){
                PuzzlePiece* curr = q[head++];
                
                // Check neighbors
                for(auto& neighbor : pieces) {
                    if (neighbor->groupId != -1) continue;
                    
                    float pieceWidth = puzzleImage->getContentSize().width / colCount;
                    float pieceHeight = puzzleImage->getContentSize().height / rowCount;
                    float threshold = pieceWidth * 0.2f;

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

void BoardModule::updateVisuals() {
    for (auto& piece : pieces) {
        auto glProgramState = piece->sprite->getGLProgramState();
        if (!glProgramState) continue;

        // u_borderSides: Top, Right, Bottom, Left (1.0 = show, 0.0 = hide)
        cocos2d::Vec4 borderSides(1.0f, 1.0f, 1.0f, 1.0f);
        if (piece->connectedTop) borderSides.x = 0.0f;
        if (piece->connectedRight) borderSides.y = 0.0f;
        if (piece->connectedBottom) borderSides.z = 0.0f;
        if (piece->connectedLeft) borderSides.w = 0.0f;
        
        glProgramState->setUniformVec4("u_borderSides", borderSides);

        // u_cornerRadii: TopRight, BottomRight, TopLeft, BottomLeft (Matches Shader)
        float r = 20.0f; // Match the default radius
        cocos2d::Vec4 cornerRadii(r, r, r, r);

        // x: TopRight
        if (piece->connectedTop || piece->connectedRight) cornerRadii.x = 0.0f; 
        // y: BottomRight
        if (piece->connectedBottom || piece->connectedRight) cornerRadii.y = 0.0f; 
        // z: TopLeft
        if (piece->connectedTop || piece->connectedLeft) cornerRadii.z = 0.0f; 
        // w: BottomLeft
        if (piece->connectedBottom || piece->connectedLeft) cornerRadii.w = 0.0f; 

        glProgramState->setUniformVec4("u_cornerRadii", cornerRadii);
    }
}

