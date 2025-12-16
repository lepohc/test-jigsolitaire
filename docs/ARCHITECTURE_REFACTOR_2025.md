# JigSolitaire 架构重构文档 (2025)

## 1. 概述

本项目近期经历了一次重大的架构重构，旨在将原本耦合度较高的代码拆分为清晰的 **MVC (Model-View-Controller)** 模式。通过引入纯逻辑层和视图管理层，提高了代码的可维护性、可测试性和扩展性。

## 2. 架构设计 (MVC)

重构后的系统主要由以下四个核心部分组成：

### 2.1 Model (数据模型)
**核心类**: `PuzzlePiece`
- **职责**: 仅存储拼图的数据状态（如：位置、索引、分组ID、锁定状态）。
- **特点**: 
  - **POCO (Plain Old C++ Object)**: 不包含任何 Cocos2d-x 的渲染节点（Sprite）或逻辑引用。
  - **纯数据**: 不包含复杂的业务逻辑，仅提供基本的 Getter/Setter。

### 2.2 View (视图层)
**核心类**: `PieceSkin` (及其子类 `ShaderPieceSkin`)
- **职责**: 负责拼图的视觉呈现。
- **特点**:
  - 持有 Cocos2d-x 的 `Node` / `Sprite` 对象。
  - 负责处理纹理、着色器效果、位置同步（根据 Model 更新位置）。
  - 不包含游戏规则逻辑。

### 2.3 Logic (业务逻辑/策略层)
**核心类**: `PuzzleRules`
- **职责**: 封装所有核心玩法的算法。
- **功能**:
  - **吸附判定**: 判断两个拼图是否应该连接。
  - **分组逻辑**: 使用 BFS (广度优先搜索) 管理拼图块的合并与分组。
  - **移动算法**: 计算拼图块在吸附时的位移量。
  - **胜利条件**: 检查拼图是否完成。
- **特点**: 纯 C++ 实现，不依赖渲染引擎，极易于编写单元测试。

### 2.4 Controller (控制器)
**核心类**: `BoardModule`
- **职责**: 协调 Model、View 和 Logic 的交互。
- **功能**:
  - **生命周期管理**: 创建和销毁拼图对象。
  - **视图绑定**: 维护 `std::map<int, PieceSkin*>`，将 Model ID 映射到对应的 View。
  - **输入处理**: 接收 `StandardInputHandler` 的输入事件，调用 `PuzzleRules` 进行逻辑判断，并更新 Model 和 View。

## 3. 交互流程

1. **输入事件**: 用户拖动拼图 -> `StandardInputHandler` 捕获触摸事件。
2. **逻辑处理**: Handler 通知 `BoardModule` -> `BoardModule` 调用 `PuzzleRules` 计算吸附和移动。
3. **数据更新**: `PuzzleRules` 返回计算结果 -> `BoardModule` 更新 `PuzzlePiece` (Model) 的数据。
4. **视图同步**: `BoardModule` 根据 Model 的变化，查找对应的 `PieceSkin` (View) 并更新其屏幕位置。

## 4. 目录结构变更

```text
Classes/
├── BoardModule.h/.cpp       # 控制器 (Controller)
├── Puzzle/
│   ├── PuzzleRules.h/.cpp   # 核心规则 (Logic)
│   ├── PuzzlePiece.h/.cpp   # 数据模型 (Model)
│   ├── PieceSkin.h/.cpp     # 视图接口 (View)
│   └── ...
```

## 5. 重构收益

1. **关注点分离**: 渲染代码与逻辑代码完全分离，修改 UI 不会破坏游戏规则。
2. **可测试性**: `PuzzleRules` 可以独立于 Cocos2d-x 环境进行测试。
3. **性能优化**: 数据层更轻量，便于进行大规模计算优化。
4. **扩展性**: 未来更换皮肤系统或移植到其他引擎时，核心逻辑代码无需修改。
