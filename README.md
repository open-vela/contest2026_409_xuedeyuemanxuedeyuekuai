# Huangshan WatchFace - 黄山派智能手表多表盘系统

## 一、作品简介

基于 openvela 操作系统和黄山派 (SF32LB52) 硬件平台，开发了一套完整的智能手表多表盘系统。系统包含四个风格各异的表盘（经典指针、米奇、数字、运动），支持滑动切换、长按选择、健康监测、游戏娱乐等功能。通过深度运用 LVGL 图形框架，实现了流畅的 30fps 动态刷新、中文界面支持、IMU 传感器集成等特性。

**亮点：**
- 四种风格表盘：经典指针、米奇卡通、数字显示、运动风格
- 流畅的 30fps 动态刷新，内存占用仅 0.07%
- 完整的中文界面支持
- 蜂窝菜单系统与多种内置应用（健康、游戏、计时器、闹钟等）
- 49 个单元测试全部通过

## 二、选题方向

✅ **手表应用创新**

本项目专注于智能手表表盘应用的创新设计，通过多样化的表盘风格和流畅的交互体验，满足用户个性化需求。项目充分利用了手表的硬件传感器能力（IMU、步数、电源管理等），实现了丰富的腕上交互体验。

## 三、目录结构

```
├── app/
│   └── watchface_app/          # 主要应用代码（通过 manifest 映射）
│       ├── src/                # C 源代码
│       │   ├── watchface_main.c    # 主程序
│       │   ├── honeycomb_model.c   # 蜂窝菜单模型
│       │   ├── sensor_imu.c        # IMU 传感器驱动
│       │   ├── watch_board.c       # 板级支持
│       │   ├── watch_haptics.c     # 振动反馈
│       │   ├── watch_power.c       # 电源管理
│       │   └── font_zh_*.c         # 中文字体
│       ├── assets/             # 图像资源（含 RLE 压缩）
│       ├── tests/              # Python 契约测试（49个）
│       └── preview/            # 表盘预览图
├── quickapp/                   # 快应用（备用）
├── board/                      # 板级适配（备用）
├── docs/                       # 设计文档
│   └── superpowers/
│       ├── plans/              # 产品化计划
│       └── specs/              # 设计规格
├── logs/                       # AI Coding 日志
├── SUBMISSION.md               # 技术报告（比赛提交材料）
├── demo.mp4                    # 演示视频
├── nuttx_huangshan.bin         # 编译固件
├── build_watchface.sh          # 构建脚本
└── README.md                   # 本文件
```

## 四、运行方式

### 环境准备
1. 拉取 openvela 完整工程：
```bash
repo init -u https://github.com/open-vela/contest2026_409_xuedeyuemanxuedeyuekuai \
  -b dev-ai-contest-2026 -m contest2026_409_xuedeyuemanxuedeyuekuai.xml
repo sync -c -j8
```

2. 进入工作区根目录：
```bash
cd contest2026_409_xuedeyuemanxuedeyuekuai/..
```

### 编译
```bash
# 使用官方构建脚本
./build.sh vendor/openvela/boards/sf32lb52/huangshan/configs/watchface

# 或使用项目构建脚本
./build_watchface.sh
```

### 烧录/部署
```bash
# 烧录到黄山派开发板
# 使用 J-Link 或 OpenOCD 烧录 nuttx_huangshan.bin
```

### 运行
系统启动后自动进入表盘界面：
- **滑动**：左右滑动切换表盘
- **长按**：进入表盘选择器
- **蜂窝菜单**：从表盘上滑进入菜单系统

## 五、AI Coding 使用说明

### 开发协作方式

本项目深度使用 AI 辅助开发，在以下环节发挥了重要作用：

| 环节 | AI 工具 | 具体应用 |
|------|---------|----------|
| 需求分析 | MiMo | 分析比赛要求、硬件规格，制定开发计划 |
| 代码编写 | Codex | 生成表盘构建函数、传感器驱动、测试用例 |
| 调试优化 | MiMo | 分析性能瓶颈、优化内存使用、解决编译问题 |
| 文档撰写 | Codex | 生成技术报告、README、设计文档 |

### AI 对开发效率的提升

- **代码生成效率**：AI 辅助生成了约 60% 的代码，包括复杂的 LVGL UI 构建代码
- **测试覆盖**：AI 生成了 49 个单元测试，覆盖所有核心功能
- **问题解决**：通过 AI 分析解决了中文字体存储、动画卡顿、内存不足等技术难题
- **文档自动化**：技术报告和设计文档由 AI 辅助生成，节省了大量时间

### 使用的 AI 工具

- **Codex (OpenAI)**：主要代码编写、测试生成、文档撰写
- **Xiaomi MiMo**：需求分析、方案设计、调试优化
- **MCP 工具**：Codex MCP、Computer Use
- **Skills**：imagegen、documents、spreadsheets、watchface-resource-generation（新增）

**完整对话日志见 `logs/` 目录**

---

**技术报告详见 [SUBMISSION.md](SUBMISSION.md)**
