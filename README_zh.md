**简体中文** | [English](README.md)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

# My AI Skills

本仓库用于收集我编写或整理的 AI skills，按编号持续收录。每个 skill 是一个独立目录，包含 `SKILL.md` 及其参考资料，与具体 agent 无关，可在支持 skills 机制的各类 agent 中使用。

## Skill 索引

| 编号 | Skill 名称 | 简介 |
|------|------------|------|
| 001 | [embedded-code-style](001_embedded-code-style/SKILL.md) | 按自定义《嵌入式代码规范》对嵌入式 C 代码（.c/.h）做规范化整理，支持全量整改、仅注释整改、中文注释三种模式 |

## 使用方法

每个 skill 是一个独立目录。将所需的 skill 目录复制到所用 agent 的 skills 目录下即可使用，例如 `.claude/skills/`、`.codex/skills/`，或项目内的对应目录（如 `<项目>/.claude/skills/`）。具体支持情况以各 agent 的文档为准。

## 许可证

本项目采用 **MIT License** 开源许可证。

Copyright (c) 2026 JesMicro。完整条款请参阅根目录的 [LICENSE](LICENSE) 文件。
