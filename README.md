**English** | [简体中文](README_zh.md)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

# My AI Skills

This repository collects AI skills I have written or curated, organized by number. Each skill is a self-contained directory with a `SKILL.md` and its reference files. The skills are agent-agnostic and work with any agent that supports the skills mechanism.

## Skill Index

| No. | Skill | Description |
|-----|-------|-------------|
| 001 | [embedded-code-style](001_embedded-code-style/SKILL.md) | Normalizes embedded C code (.c/.h) against a custom embedded coding standard, with three modes: full normalization, comment-only normalization, and Chinese-comment modules |
| 002 | [drawio-diagram](002_drawio-diagram/SKILL.md) | Creates or modifies editable local .drawio architecture diagrams, flowcharts, topology diagrams, and hierarchy diagrams from supplied materials, with a clean visual style and clear connections |
| 003 | [study-notes](003_study-notes/SKILL.md) | Organizes handouts, slides, explanatory diagrams, course transcripts, and supplementary references into accurate, self-contained Markdown study notes; supports creation, targeted edits, and reviews for fundamental errors |

## Usage

Each skill is a self-contained directory. To use one, copy its directory into the skills directory of your agent — for example `.claude/skills/` or `.codex/skills/`, or the corresponding directory inside a project (e.g. `<project>/.claude/skills/`). Refer to your agent's documentation for exact support details.

## License

This project is licensed under the **MIT License**.

Copyright (c) 2026 JesMicro. See the [LICENSE](LICENSE) file in the repository root for the full license text.
