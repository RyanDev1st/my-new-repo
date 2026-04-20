# CLAUDE.md
You are an orchestrator
# PROJECT OVERVIEW
This project is to aim and address C:\Users\admin\automata\resources\GOAL.md

Always say "Aye" in the beginning to confirm you got the task. 
## Claude Rules For This Repo
- Always use /caveman:caveman ultra and  /karpathy-guidelines for this project. These precede all other instructions I make. Adhere strictly to karpathy-guidelines.  
- Always update the C:\Users\admin\automata\docs
- **For all subagent work, ALWAYS use Codex.** 

## Codex Subagent Usage 

**Core Philosophy**: Codex does the heavy lifting. Claude only plans, outline necessary requirements, hands off, reviews summaries of the code architecture and changes, and run confirmation final tests.

### Model Selection (Strict Rules)
- `gpt-5.4-mini` → Easy/medium tasks, simple fixes, test generation, small refactors, file cleanup
- `gpt-5.4` + `--effort high` → Above-average coding tasks, multi-file changes, complex logic
- `gpt-5.4` + `--effort xhigh` → Difficult or long tasks (50+ files, major rewrites, architecture work, heavy debugging)

**Never** use `gpt-5.4` for simple assistant tasks. Default to `gpt-5.4-mini` unless the task clearly needs more power.

# Codex Call Template
/codex:rescue --background --model gpt-5.4-mini --effort high "
You are running as a full Codex agent with complete file system access.

CRITICAL INSTRUCTIONS:
- You have full access to the project. Use your normal tools to read, edit, create, and delete files directly.
- NEVER output full code blocks for the user to copy. Always edit files in place.
- Work completely autonomously. Only ask questions if you are fully blocked.
- When done, end your response with exactly this block:

=== FINAL STATUS ===
SUCCESS / BLOCKED / PARTIAL

- Summary of changes (max 6 bullets)
- Files created/modified
- Any blockers needing review

Task: [Paste your actual task here]
"..."

### Task Scoping Rules (Most Important for Token Savings)
- **Never** read all files yourself before calling Codex.
- Give Codex the **goal + folder path** and let it use its own tools to explore.
- Break large work into **focused sub-tasks** (one folder/program at a time is ideal).
- Make the prompt **self-contained** — Codex should not need to ask you questions.

**Good example**:
> "Rewrite ONLY the program in 01_dfa_acceptance to match the new input format in docs/SPEC.md. Generate all required test files. Work completely autonomously using your tools."

**Bad example** (avoid):
> "Here are all 7 programs and 70 test files. Please rewrite everything..."

# Claude's Responsibilities 
- Do **NOT** scan or read large numbers of files before calling Codex.
- When needed, create a short spec first, then hand it off.
- After Codex finishes, only request a **3-bullet summary** — never ask it to re-read everything.
- Keep the main conversation context as small as possible. Summarize and move on after each Codex run.

# HOOK
-Always read [text](docs/MEMORY.md) & (C:\Users\admin\automata\docs\ARCHITECTURE.md) before starting
-Decide on your own the necessary hooks for the project 
-----------------------
# PROJECT ARCHITECTURE
 
 - The project workspace's architecture is within C:\Users\admin\automata\docs\ARCHITECTURE.md
 - Always update it if you make changes to our current architecture with detailed documentation of the functionality.

All changes will be reviewed and judged by codex.

<!-- rtk-instructions v2 -->
# RTK (Rust Token Killer) - Token-Optimized Commands
## RTK Usage
  - Use `rtk` direct commands for repo operations like `rtk git status`, `rtk git diff`, `rtk gh ...`, `rtk verify`, and `rtk init --show`.
  - Do not treat `rtk` hook warnings as a blocker for normal work on Windows.
  - The hook layer is optional and separate from the direct `rtk` commands. If `rtk` subcommands work, proceed with them.
  - Only troubleshoot RTK hook installation if the user explicitly asks for hook setup or the command itself fails.
  - On Windows, expect RTK to fall back from hook-based mode; use direct `rtk` commands and do not spend time chasing hook installation unless required.
## Golden Rule

**Always prefix commands with `rtk`**. If RTK has a dedicated filter, it uses it. If not, it passes through unchanged. This means RTK is always safe to use.


**Important**: Even in command chains with `&&`, use `rtk`:
```bash
# ❌ Wrong
git add . && git commit -m "msg" && git push

# ✅ Correct
rtk git add . && rtk git commit -m "msg" && rtk git push
```

## RTK Commands by Workflow

### Build & Compile (80-90% savings)
```bash
rtk cargo build         # Cargo build output
rtk cargo check         # Cargo check output
rtk cargo clippy        # Clippy warnings grouped by file (80%)
rtk tsc                 # TypeScript errors grouped by file/code (83%)
rtk lint                # ESLint/Biome violations grouped (84%)
rtk prettier --check    # Files needing format only (70%)
rtk next build          # Next.js build with route metrics (87%)
```

### Test (90-99% savings)
```bash
rtk cargo test          # Cargo test failures only (90%)
rtk vitest run          # Vitest failures only (99.5%)
rtk playwright test     # Playwright failures only (94%)
rtk test <cmd>          # Generic test wrapper - failures only
```

### Git (59-80% savings)
```bash
rtk git status          # Compact status
rtk git log             # Compact log (works with all git flags)
rtk git diff            # Compact diff (80%)
rtk git show            # Compact show (80%)
rtk git add             # Ultra-compact confirmations (59%)
rtk git commit          # Ultra-compact confirmations (59%)
rtk git push            # Ultra-compact confirmations
rtk git pull            # Ultra-compact confirmations
rtk git branch          # Compact branch list
rtk git fetch           # Compact fetch
rtk git stash           # Compact stash
rtk git worktree        # Compact worktree
```

Note: Git passthrough works for ALL subcommands, even those not explicitly listed.

### GitHub (26-87% savings)
```bash
rtk gh pr view <num>    # Compact PR view (87%)
rtk gh pr checks        # Compact PR checks (79%)
rtk gh run list         # Compact workflow runs (82%)
rtk gh issue list       # Compact issue list (80%)
rtk gh api              # Compact API responses (26%)
```

### JavaScript/TypeScript Tooling (70-90% savings)
```bash
rtk pnpm list           # Compact dependency tree (70%)
rtk pnpm outdated       # Compact outdated packages (80%)
rtk pnpm install        # Compact install output (90%)
rtk npm run <script>    # Compact npm script output
rtk npx <cmd>           # Compact npx command output
rtk prisma              # Prisma without ASCII art (88%)
```

### Files & Search (60-75% savings)
```bash
rtk ls <path>           # Tree format, compact (65%)
rtk read <file>         # Code reading with filtering (60%)
rtk grep <pattern>      # Search grouped by file (75%)
rtk find <pattern>      # Find grouped by directory (70%)
```

### Analysis & Debug (70-90% savings)
```bash
rtk err <cmd>           # Filter errors only from any command
rtk log <file>          # Deduplicated logs with counts
rtk json <file>         # JSON structure without values
rtk deps                # Dependency overview
rtk env                 # Environment variables compact
rtk summary <cmd>       # Smart summary of command output
rtk diff                # Ultra-compact diffs
```

### Infrastructure (85% savings)
```bash
rtk docker ps           # Compact container list
rtk docker images       # Compact image list
rtk docker logs <c>     # Deduplicated logs
rtk kubectl get         # Compact resource list
rtk kubectl logs        # Deduplicated pod logs
```

### Network (65-70% savings)
```bash
rtk curl <url>          # Compact HTTP responses (70%)
rtk wget <url>          # Compact download output (65%)
```

### Meta Commands
```bash
rtk gain                # View token savings statistics
rtk gain --history      # View command history with savings
rtk discover            # Analyze Claude Code sessions for missed RTK usage
rtk proxy <cmd>         # Run command without filtering (for debugging)
rtk init                # Add RTK instructions to CLAUDE.md
rtk init --global       # Add RTK to ~/.claude/CLAUDE.md
```

## Token Savings Overview

| Category | Commands | Typical Savings |
|----------|----------|-----------------|
| Tests | vitest, playwright, cargo test | 90-99% |
| Build | next, tsc, lint, prettier | 70-87% |
| Git | status, log, diff, add, commit | 59-80% |
| GitHub | gh pr, gh run, gh issue | 26-87% |
| Package Managers | pnpm, npm, npx | 70-90% |
| Files | ls, read, grep, find | 60-75% |
| Infrastructure | docker, kubectl | 85% |
| Network | curl, wget | 65-70% |

Overall average: **60-90% token reduction** on common development operations.
<!-- /rtk-instructions -->