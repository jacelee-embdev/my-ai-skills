**English** | [简体中文](README_zh.md)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

# My AI Skills

This repository collects AI skills I have written or curated, organized by number. Each skill is a self-contained directory with a `SKILL.md` and its reference files. The skills are agent-agnostic and work with any agent that supports the skills mechanism.

## Skill Index

| No. | Skill | Description |
|-----|-------|-------------|
| 001 | [embedded-code-style](001_embedded-code-style/SKILL.md) | Normalizes embedded C code (.c/.h) against a custom embedded coding standard, with three modes: full normalization, comment-only normalization, and Chinese-comment modules |

## Usage

Each skill is a self-contained directory. To use one, copy its directory into the skills directory of your agent — for example `.claude/skills/` or `.codex/skills/`, or the corresponding directory inside a project (e.g. `<project>/.claude/skills/`). Refer to your agent's documentation for exact support details.

## License

This project is licensed under the **MIT License**.

Copyright (c) 2026 JesMicro. See the [LICENSE](LICENSE) file in the repository root for the full license text.
