# logs/ — AI Coding 日志目录

存放你在开发中与 AI 工具的对话日志，和作品代码一并提交。

## 目录结构

```text
logs/
└── XUMUYIE/                    # GitHub 用户名
    ├── manifest.json           # 会话清单
    └── 2026-09-08/             # 日期 YYYY-MM-DD
        └── codex__*.jsonl      # Codex 会话日志
```

- 每个 `.jsonl` 每行一个事件，由 Codex 工具导出。
- 本目录包含开发过程中的完整 AI Coding 对话记录。
