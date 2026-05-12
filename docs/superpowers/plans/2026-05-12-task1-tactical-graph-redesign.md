# Task 1 Tactical Graph Redesign Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Redesign the DFA visualization as a tactical telemetry interface and refactor the canvas graph renderer so user-authored transition tables render robustly.

**Architecture:** Keep the current single-file `src/task1/index.html` architecture and existing DFA parsing/acceptance logic. Replace presentation CSS and renderer internals only: parse current DFA data, compute adaptive node layout, build grouped transitions, route deterministic edges, then draw states and routed edges.

**Tech Stack:** Local dependency-free HTML, vanilla CSS, vanilla JavaScript, Canvas 2D API.

---

## File Structure

- Modify `src/task1/index.html`
  - CSS section: replace Ethereal Glass visual system with square-corner tactical telemetry styling.
  - Renderer functions near `resizeCanvas`, `layoutStates`, `buildEdges`, `drawHead`, `drawLabel`, `drawStraightEdge`, `drawCurvedEdge`, `drawSelfLoop`, `drawState`, and `drawDFA`.
  - Existing parsing, preset loading, trace building, and DFA acceptance logic stay intact.
- Modify `docs/ARCHITECTURE.md`
  - Update Task 1 visual system and renderer architecture to match tactical UI and deterministic routing.
- Modify `docs/MEMORY.md`
  - Add dated update summarizing redesign and robust graph renderer.

---

### Task 1: Establish renderer baseline and visual regression cases

**Files:**
- Inspect: `src/task1/index.html`
- No code changes in this task

- [ ] **Step 1: Open current visualization in browser**

Run a local static server from repository root:

```powershell
rtk python -m http.server 4173
```

Expected: server starts and serves `http://localhost:4173/`.

- [ ] **Step 2: Load Task 1 page**

Open:

```text
http://localhost:4173/src/task1/index.html
```

Expected: page loads without console errors.

- [ ] **Step 3: Record current renderer failure cases**

Use the built-in test selector and check these cases:

```text
Bidirectional adjacent
Bidirectional far across intermediates
Multi-symbol parallel edges
All self-loops
5-state chain with wrap-around
6-state circle layout with diametric pairs
Single state self-loop
Dead-state rejection
```

Expected: identify visible overlap, clipping, label, or arrowhead issues that new renderer must improve.

- [ ] **Step 4: Create user-authored stress table in current editor**

Paste this DFA definition into the current editor fields using existing UI controls:

```text
States: q0,q1,q2,q3,q4,q5,q6,q7,q8,q9,q10,q11
Alphabet: a,b
Initial: q0
Final: q3,q7,q11
Transitions:
q0 a q1
q1 a q0
q0 b q6
q6 b q0
q1 b q2
q2 a q3
q3 b q4
q4 a q5
q5 b q6
q6 a q7
q7 b q8
q8 a q9
q9 b q10
q10 a q11
q11 b q0
q3 a q3
q7 a q7
q11 a q11
```

Expected: current renderer reveals dense-graph problems but logic does not crash.

---

### Task 2: Replace visual system with tactical telemetry CSS

**Files:**
- Modify: `src/task1/index.html` CSS section near lines 1-760

- [ ] **Step 1: Replace root tokens**

Find the `:root` block and replace color/radius/shadow tokens with:

```css
:root {
  --bg: #0a0a0a;
  --panel: #111111;
  --panel-2: #171717;
  --ink: #eaeaea;
  --muted: #9a9a9a;
  --line: #343434;
  --line-strong: #5b5b5b;
  --red: #e61919;
  --green: #4af626;
  --paper: #d8d8d0;
  --black: #050505;
  --font-display: 'Archivo Black', 'Arial Black', Impact, sans-serif;
  --font-mono: 'JetBrains Mono', 'IBM Plex Mono', 'Courier New', monospace;
  --ease: cubic-bezier(0.32, 0.72, 0, 1);
}
```

Expected: one dark substrate, one red accent, one green status color.

- [ ] **Step 2: Replace global surface styling**

Replace existing `html`, `body`, and body pseudo-element styling with:

```css
html { scroll-behavior: smooth; }

body {
  margin: 0;
  min-height: 100dvh;
  background:
    repeating-linear-gradient(0deg, rgba(255,255,255,0.025) 0 1px, transparent 1px 4px),
    radial-gradient(circle at 18% 12%, rgba(230,25,25,0.12), transparent 28rem),
    #0a0a0a;
  color: var(--ink);
  font-family: var(--font-mono);
  overflow-x: hidden;
}

body::before {
  content: "";
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 20;
  opacity: 0.13;
  background-image: url("data:image/svg+xml,%3Csvg viewBox='0 0 160 160' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='n'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='.9' numOctaves='2' stitchTiles='stitch'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23n)' opacity='.55'/%3E%3C/svg%3E");
  mix-blend-mode: screen;
}

* { box-sizing: border-box; }
```

Expected: scanline/noise tactical base, no glass mesh system.

- [ ] **Step 3: Convert cards and shells to square grid compartments**

Replace `.shell`, `.core`, `.card`, and related rounded/glass rules with square industrial panel rules:

```css
.shell {
  background: var(--line);
  border: 1px solid var(--line-strong);
  padding: 1px;
}

.core {
  background: var(--panel);
  min-height: 100%;
  padding: clamp(18px, 2vw, 28px);
}

.card {
  position: relative;
  isolation: isolate;
}

.card::before,
.card::after {
  content: "+";
  position: absolute;
  color: var(--red);
  font: 700 12px/1 var(--font-mono);
  z-index: 2;
}

.card::before { top: -7px; left: -5px; }
.card::after { right: -5px; bottom: -7px; }
```

Expected: square corners and visible compartment markers.

- [ ] **Step 4: Convert hero/nav/buttons to industrial style**

Update hero/nav/button rules to these principles:

```css
.nav {
  position: sticky;
  top: 0;
  z-index: 10;
  display: grid;
  grid-template-columns: 1fr auto;
  align-items: center;
  gap: 1px;
  margin: 16px auto 0;
  width: min(1440px, calc(100% - 32px));
  background: var(--line);
  border: 1px solid var(--line-strong);
}

.nav > * {
  background: var(--panel);
  min-height: 48px;
  display: flex;
  align-items: center;
  padding: 0 18px;
}

.hero {
  width: min(1440px, calc(100% - 32px));
  margin: 56px auto 48px;
  display: grid;
  grid-template-columns: minmax(0, 1.25fr) minmax(260px, 0.75fr);
  gap: 1px;
  background: var(--line);
  border: 1px solid var(--line-strong);
}

.title {
  margin: 0;
  max-width: 12ch;
  color: var(--ink);
  font-family: var(--font-display);
  font-size: clamp(3.8rem, 10vw, 11rem);
  line-height: 0.86;
  letter-spacing: -0.06em;
  text-transform: uppercase;
  text-wrap: balance;
}

.btn {
  border: 1px solid var(--line-strong);
  border-radius: 0;
  background: var(--panel-2);
  color: var(--ink);
  font-family: var(--font-mono);
  text-transform: uppercase;
  letter-spacing: 0.08em;
  transition: transform 180ms var(--ease), background 180ms var(--ease), color 180ms var(--ease);
}

.btn:hover { transform: translate(-2px, -2px); background: var(--red); color: white; }
.btn:active { transform: translate(0, 0); }
.btn:focus-visible { outline: 2px solid var(--red); outline-offset: 3px; }
```

Expected: no rounded pills, no invisible button text, strong hierarchy.

- [ ] **Step 5: Run browser smoke check**

Refresh page.

Expected:

```text
- No horizontal scrollbar
- Buttons legible
- Focus rings visible with Tab
- Cards square-cornered
- No blue/purple glass styling remains
```

---

### Task 3: Add adaptive layout helpers

**Files:**
- Modify: `src/task1/index.html` JavaScript near existing `layoutStates` function

- [ ] **Step 1: Replace renderer constants**

Find renderer constants near the canvas functions and use adaptive constants:

```js
let SR = 30;
const NODE_MIN = 21;
const NODE_MAX = 32;
const EDGE_MARGIN = 18;
```

Expected: node radius can adapt by graph size.

- [ ] **Step 2: Add geometry helpers before `layoutStates`**

Insert these helpers before `layoutStates`:

```js
function clamp(n, min, max) {
  return Math.max(min, Math.min(max, n));
}

function dist(a, b) {
  return Math.hypot(b.x - a.x, b.y - a.y);
}

function unit(a, b) {
  const d = dist(a, b) || 1;
  return { x: (b.x - a.x) / d, y: (b.y - a.y) / d, d };
}

function normalFromUnit(u) {
  return { x: -u.y, y: u.x };
}

function linePointDistance(p, a, b) {
  const ab = unit(a, b);
  const apx = p.x - a.x;
  const apy = p.y - a.y;
  const t = clamp((apx * ab.x + apy * ab.y) / ab.d, 0, ab.d);
  const x = a.x + ab.x * t;
  const y = a.y + ab.y * t;
  return Math.hypot(p.x - x, p.y - y);
}
```

Expected: no external dependencies.

- [ ] **Step 3: Replace `layoutStates` with adaptive implementation**

Replace existing `layoutStates` with:

```js
function layoutStates(states) {
  const w = canvas.width / DPR;
  const h = canvas.height / DPR;
  const count = states.length;
  const positions = {};

  SR = clamp(Math.floor(Math.min(w, h) / (count > 10 ? 18 : count > 6 ? 15 : 12)), NODE_MIN, NODE_MAX);
  if (!count) return positions;

  if (count === 1) {
    positions[states[0]] = { x: w / 2, y: h / 2 };
    return positions;
  }

  if (count === 2) {
    states.forEach((state, i) => {
      positions[state] = { x: w * (0.33 + i * 0.34), y: h / 2 };
    });
    return positions;
  }

  if (count === 3 || count === 4) {
    const cx = w / 2;
    const cy = h / 2;
    const rx = Math.max(120, w * 0.34);
    const ry = Math.max(86, h * 0.27);
    const start = count === 3 ? -Math.PI / 2 : -Math.PI * 0.75;
    states.forEach((state, i) => {
      const a = start + (Math.PI * 2 * i) / count;
      positions[state] = {
        x: clamp(cx + Math.cos(a) * rx, SR + 42, w - SR - 42),
        y: clamp(cy + Math.sin(a) * ry, SR + 42, h - SR - 42)
      };
    });
    return positions;
  }

  const cx = w / 2;
  const cy = h / 2;
  const rx = Math.max(120, w / 2 - SR - 70);
  const ry = Math.max(96, h / 2 - SR - 64);
  states.forEach((state, i) => {
    const a = -Math.PI / 2 + (Math.PI * 2 * i) / count;
    positions[state] = {
      x: clamp(cx + Math.cos(a) * rx, SR + 38, w - SR - 38),
      y: clamp(cy + Math.sin(a) * ry, SR + 38, h - SR - 38)
    };
  });
  return positions;
}
```

Expected: layout handles 1, 2, 3-4, 5+ states.

- [ ] **Step 4: Verify layout with existing presets**

Refresh page and select each preset.

Expected:

```text
No JavaScript console errors.
Nodes remain inside canvas for every preset.
Single-state graph is centered.
Dense graphs use smaller nodes.
```

---

### Task 4: Replace edge grouping and routing

**Files:**
- Modify: `src/task1/index.html` JavaScript near `buildEdges`, `drawStraightEdge`, `drawCurvedEdge`, `drawSelfLoop`

- [ ] **Step 1: Replace `buildEdges` with grouped transition output**

Replace current `buildEdges` with:

```js
function buildEdges() {
  const map = new Map();
  for (const [key, to] of Object.entries(dfa.delta)) {
    const [from, sym] = key.split('|');
    if (!from || !sym || !to) continue;
    if (!dfa.states.includes(from) || !dfa.states.includes(to)) continue;
    const id = `${from}|${to}`;
    if (!map.has(id)) map.set(id, { from, to, symbols: [] });
    map.get(id).symbols.push(sym);
  }
  return [...map.values()].map(edge => ({
    ...edge,
    label: edge.symbols.sort().join(',')
  }));
}
```

Expected: user-authored repeated pair transitions merge labels.

- [ ] **Step 2: Add route computation after `buildEdges`**

Insert:

```js
function hasReverse(edge, edgeIds) {
  return edgeIds.has(`${edge.to}|${edge.from}`) && edge.from !== edge.to;
}

function hasIntermediateNode(p1, p2, positions, from, to) {
  return Object.entries(positions).some(([state, p]) => {
    if (state === from || state === to) return false;
    return linePointDistance(p, p1, p2) < SR * 2.15;
  });
}

function routeEdges(edges, positions) {
  const ids = new Set(edges.map(edge => `${edge.from}|${edge.to}`));
  return edges.map(edge => {
    const p1 = positions[edge.from];
    const p2 = positions[edge.to];
    if (!p1 || !p2) return null;

    if (edge.from === edge.to) return routeSelfLoop(edge, p1, positions);

    const u = unit(p1, p2);
    const n = normalFromUnit(u);
    const reverse = hasReverse(edge, ids);
    const blocked = hasIntermediateNode(p1, p2, positions, edge.from, edge.to);
    const longEdge = u.d > SR * 5;

    if (!reverse && !blocked && !longEdge) {
      const start = { x: p1.x + u.x * SR, y: p1.y + u.y * SR };
      const end = { x: p2.x - u.x * SR, y: p2.y - u.y * SR };
      return {
        ...edge,
        kind: 'straight',
        start,
        end,
        labelPoint: { x: (start.x + end.x) / 2 + n.x * 18, y: (start.y + end.y) / 2 + n.y * 18 },
        angle: Math.atan2(u.y, u.x)
      };
    }

    const sign = edge.from < edge.to ? 1 : -1;
    const lift = clamp(u.d * (reverse ? 0.28 : 0.2), 44, 170) * sign;
    const cp = clampPoint({
      x: (p1.x + p2.x) / 2 + n.x * lift,
      y: (p1.y + p2.y) / 2 + n.y * lift
    });
    const start = pointOnQuadratic(p1, cp, p2, SR / u.d);
    const end = pointOnQuadratic(p1, cp, p2, 1 - SR / u.d);
    const labelPoint = clampPoint(pointOnQuadratic(p1, cp, p2, 0.5), 28);
    const tangent = { x: p2.x - cp.x, y: p2.y - cp.y };

    return {
      ...edge,
      kind: 'curve',
      start,
      end,
      cp,
      labelPoint,
      angle: Math.atan2(tangent.y, tangent.x)
    };
  }).filter(Boolean);
}

function clampPoint(p, margin = EDGE_MARGIN) {
  const w = canvas.width / DPR;
  const h = canvas.height / DPR;
  return {
    x: clamp(p.x, margin, w - margin),
    y: clamp(p.y, margin, h - margin)
  };
}

function pointOnQuadratic(a, c, b, t) {
  const mt = 1 - t;
  return {
    x: mt * mt * a.x + 2 * mt * t * c.x + t * t * b.x,
    y: mt * mt * a.y + 2 * mt * t * c.y + t * t * b.y
  };
}
```

Expected: edge routes are computed before drawing.

- [ ] **Step 3: Add self-loop routing**

Insert after `routeEdges` helpers:

```js
function routeSelfLoop(edge, p, positions) {
  const w = canvas.width / DPR;
  const h = canvas.height / DPR;
  const cx = w / 2;
  const cy = h / 2;
  let theta = Math.atan2(p.y - cy, p.x - cx);
  if (!Number.isFinite(theta)) theta = -Math.PI / 2;

  const candidates = [theta, -Math.PI / 2, 0, Math.PI, Math.PI / 2];
  let best = candidates[0];
  let bestScore = -Infinity;

  for (const a of candidates) {
    const probe = { x: p.x + Math.cos(a) * SR * 3.2, y: p.y + Math.sin(a) * SR * 3.2 };
    const edgePenalty = Math.min(probe.x, probe.y, w - probe.x, h - probe.y);
    const neighborPenalty = Math.min(...Object.values(positions).filter(q => q !== p).map(q => dist(probe, q)), 999);
    const score = edgePenalty + neighborPenalty;
    if (score > bestScore) {
      bestScore = score;
      best = a;
    }
  }

  const spread = Math.PI / 5;
  const startA = best - spread;
  const endA = best + spread;
  const start = { x: p.x + Math.cos(startA) * SR, y: p.y + Math.sin(startA) * SR };
  const end = { x: p.x + Math.cos(endA) * SR, y: p.y + Math.sin(endA) * SR };
  const c1 = { x: p.x + Math.cos(best - 0.72) * SR * 3.1, y: p.y + Math.sin(best - 0.72) * SR * 3.1 };
  const c2 = { x: p.x + Math.cos(best + 0.72) * SR * 3.1, y: p.y + Math.sin(best + 0.72) * SR * 3.1 };
  const labelPoint = clampPoint({ x: p.x + Math.cos(best) * SR * 2.85, y: p.y + Math.sin(best) * SR * 2.85 }, 28);
  const tangent = { x: end.x - c2.x, y: end.y - c2.y };

  return {
    ...edge,
    kind: 'self',
    start,
    end,
    c1,
    c2,
    labelPoint,
    angle: Math.atan2(tangent.y, tangent.x)
  };
}
```

Expected: loops choose less crowded side.

- [ ] **Step 4: Verify grouping with user-authored graph**

Use editor stress table from Task 1.

Expected:

```text
q0->q1 and q1->q0 render on opposite sides.
q3, q7, q11 self-loops do not cover node labels.
No invalid transition crashes renderer.
```

---

### Task 5: Replace drawing functions with routed geometry drawing

**Files:**
- Modify: `src/task1/index.html` JavaScript near `drawHead`, `drawLabel`, `drawStraightEdge`, `drawCurvedEdge`, `drawSelfLoop`, `drawState`, `drawDFA`

- [ ] **Step 1: Replace `drawHead`**

Use:

```js
function drawHead(x, y, angle, color) {
  ctx.save();
  ctx.translate(x, y);
  ctx.rotate(angle);
  ctx.beginPath();
  ctx.moveTo(0, 0);
  ctx.lineTo(-13, -6);
  ctx.lineTo(-9, 0);
  ctx.lineTo(-13, 6);
  ctx.closePath();
  ctx.fillStyle = color;
  ctx.fill();
  ctx.restore();
}
```

Expected: arrowhead points along precomputed tangent.

- [ ] **Step 2: Replace `drawLabel`**

Use:

```js
function drawLabel(x, y, text, color) {
  ctx.save();
  ctx.font = '700 12px JetBrains Mono, IBM Plex Mono, monospace';
  ctx.textAlign = 'center';
  ctx.textBaseline = 'middle';
  ctx.lineWidth = 6;
  ctx.strokeStyle = '#0a0a0a';
  ctx.strokeText(text, x, y);
  ctx.fillStyle = color;
  ctx.fillText(text, x, y);
  ctx.restore();
}
```

Expected: labels remain readable over grid/edges.

- [ ] **Step 3: Replace edge draw functions with routed versions**

Use:

```js
function drawRoutedEdge(edge, color) {
  ctx.save();
  ctx.strokeStyle = color;
  ctx.fillStyle = color;
  ctx.lineWidth = 2;
  ctx.lineCap = 'round';
  ctx.lineJoin = 'round';

  ctx.beginPath();
  if (edge.kind === 'straight') {
    ctx.moveTo(edge.start.x, edge.start.y);
    ctx.lineTo(edge.end.x, edge.end.y);
  } else if (edge.kind === 'curve') {
    ctx.moveTo(edge.start.x, edge.start.y);
    ctx.quadraticCurveTo(edge.cp.x, edge.cp.y, edge.end.x, edge.end.y);
  } else {
    ctx.moveTo(edge.start.x, edge.start.y);
    ctx.bezierCurveTo(edge.c1.x, edge.c1.y, edge.c2.x, edge.c2.y, edge.end.x, edge.end.y);
  }
  ctx.stroke();
  drawHead(edge.end.x, edge.end.y, edge.angle, color);
  drawLabel(edge.labelPoint.x, edge.labelPoint.y, edge.label, color);
  ctx.restore();
}
```

Expected: old `drawStraightEdge`, `drawCurvedEdge`, and `drawSelfLoop` calls are no longer needed from `drawDFA`.

- [ ] **Step 4: Add tactical canvas grid helper**

Insert before `drawDFA`:

```js
function drawCanvasGrid() {
  const w = canvas.width / DPR;
  const h = canvas.height / DPR;
  ctx.save();
  ctx.strokeStyle = 'rgba(234,234,234,0.06)';
  ctx.lineWidth = 1;
  for (let x = 0; x <= w; x += 40) {
    ctx.beginPath();
    ctx.moveTo(x, 0);
    ctx.lineTo(x, h);
    ctx.stroke();
  }
  for (let y = 0; y <= h; y += 40) {
    ctx.beginPath();
    ctx.moveTo(0, y);
    ctx.lineTo(w, y);
    ctx.stroke();
  }
  ctx.restore();
}
```

Expected: canvas fits tactical panel style.

- [ ] **Step 5: Replace `drawDFA` edge pipeline**

Inside `drawDFA`, after clearing canvas and computing positions, use:

```js
ctx.clearRect(0, 0, canvas.width / DPR, canvas.height / DPR);
drawCanvasGrid();
const pos = layoutStates(dfa.states);
const routedEdges = routeEdges(buildEdges(), pos);
for (const edge of routedEdges) drawRoutedEdge(edge, '#eaeaea');
for (const state of dfa.states) drawState(state, pos[state]);
```

Preserve existing active/current/final/rejected state handling by keeping the same variables and passing them into `drawState` as currently done.

Expected: edge routing uses precomputed route objects.

- [ ] **Step 6: Verify no obsolete calls remain**

Search within `src/task1/index.html` for:

```text
drawStraightEdge(
drawCurvedEdge(
drawSelfLoop(
```

Expected: either no matches, or only unused function definitions that must be deleted before completion.

---

### Task 6: Retheme state nodes and canvas metadata

**Files:**
- Modify: `src/task1/index.html` CSS and JavaScript `drawState`, `.canvas-meta`, `.canvas-legend`, chips/status styles

- [ ] **Step 1: Replace `drawState` visual style**

Keep existing status logic, but use square tactical colors:

```js
function drawState(s, p) {
  if (!p) return;
  const isFinal = dfa.finals.has(s);
  const isStart = s === dfa.start;
  const isCurrent = currentState === s && running;
  const isAccepted = finalAcceptedState === s;

  let fill = '#111111';
  let stroke = '#eaeaea';
  if (isCurrent) stroke = '#4af626';
  if (isAccepted) stroke = '#4af626';
  if (rejectedState === s) stroke = '#e61919';

  ctx.save();
  ctx.beginPath();
  ctx.arc(p.x, p.y, SR, 0, Math.PI * 2);
  ctx.fillStyle = fill;
  ctx.fill();
  ctx.lineWidth = isCurrent || isAccepted ? 4 : 2;
  ctx.strokeStyle = stroke;
  ctx.stroke();

  if (isFinal) {
    ctx.beginPath();
    ctx.arc(p.x, p.y, SR - 6, 0, Math.PI * 2);
    ctx.lineWidth = 1.5;
    ctx.strokeStyle = stroke;
    ctx.stroke();
  }

  if (isStart) {
    ctx.beginPath();
    ctx.moveTo(p.x - SR - 28, p.y);
    ctx.lineTo(p.x - SR - 6, p.y);
    ctx.strokeStyle = '#e61919';
    ctx.lineWidth = 2;
    ctx.stroke();
    drawHead(p.x - SR - 6, p.y, 0, '#e61919');
  }

  ctx.font = '800 12px JetBrains Mono, IBM Plex Mono, monospace';
  ctx.textAlign = 'center';
  ctx.textBaseline = 'middle';
  ctx.fillStyle = '#eaeaea';
  ctx.fillText(s, p.x, p.y);
  ctx.restore();
}
```

Expected: states match telemetry style and still show start/final/current/accepted/rejected.

- [ ] **Step 2: Retheme canvas metadata and legend CSS**

Use square chips:

```css
.canvas-meta,
.canvas-legend,
.state-chip,
.status-pill {
  border: 1px solid var(--line-strong);
  border-radius: 0;
  background: var(--panel-2);
  color: var(--ink);
  font-family: var(--font-mono);
  text-transform: uppercase;
  letter-spacing: 0.06em;
}

.status-pill.accept,
.state-chip.current {
  color: var(--green);
  border-color: var(--green);
}

.status-pill.reject {
  color: var(--red);
  border-color: var(--red);
}
```

Expected: status uses green/red only for semantic states.

- [ ] **Step 3: Verify step animation colors**

Run accepted and rejected strings.

Expected:

```text
Current state uses green status.
Accepted final state remains green after run finishes.
Rejected terminal state uses red.
No old blue/violet highlight remains.
```

---

### Task 7: User-authored transition table robustness pass

**Files:**
- Modify: `src/task1/index.html` JavaScript only if renderer crashes from malformed current UI data

- [ ] **Step 1: Test 1-state user graph**

Use editor:

```text
States: q0
Alphabet: a
Initial: q0
Final: q0
Transitions:
q0 a q0
Test string: aaa
```

Expected:

```text
Single node centered.
Self-loop visible and labeled a.
Run accepts aaa.
```

- [ ] **Step 2: Test 4-state bidirectional graph**

Use editor:

```text
States: A,B,C,D
Alphabet: x,y
Initial: A
Final: D
Transitions:
A x B
B x A
B y C
C y B
C x D
D y A
A y A
D x D
Test string: xyyx
```

Expected:

```text
Reverse pairs are mirrored.
Self-loops visible.
No edge crosses through node center.
```

- [ ] **Step 3: Test 12-state dense graph**

Use stress table from Task 1.

Expected:

```text
No renderer crash.
Labels stay inside canvas.
Arrowheads land on circle boundaries.
Dense graph remains usable after resize.
```

- [ ] **Step 4: Add guard only if a crash appears**

If malformed transition data causes crash, add guard at `drawDFA` start:

```js
if (!dfa || !Array.isArray(dfa.states) || !dfa.delta) return;
```

Expected: renderer ignores impossible invalid data instead of throwing.

---

### Task 8: Update docs

**Files:**
- Modify: `docs/ARCHITECTURE.md`
- Modify: `docs/MEMORY.md`

- [ ] **Step 1: Update architecture Task 1 visual system**

In `docs/ARCHITECTURE.md`, replace old Ethereal Glass visual system bullets with:

```markdown
**Visual system** (redesigned 2026-05-12):
- **Vibe — Tactical Telemetry**: off-black CRT substrate, phosphor-white text, aviation-red accent, and one terminal-green status readout for active/accepted automata state.
- **Typography**: heavy uppercase display type for structural headings and JetBrains Mono / IBM Plex Mono style stacks for all controls, labels, chips, and data readouts.
- **Layout — Industrial Grid**: square-corner panels, 1px compartment lines, crosshair markers, dense metadata, and no glassmorphism, rounded pills, or blue/purple gradients.
- **Canvas treatment**: graph panel includes a low-contrast measurement grid and halo-stroked labels for readable dense transition diagrams.
- **Accessibility**: controls keep visible focus rings, high-contrast button text, semantic sections, and keyboard-friendly form controls.
```

Expected: docs match new UI.

- [ ] **Step 2: Update architecture renderer section**

In `docs/ARCHITECTURE.md`, replace renderer bullets with:

```markdown
**Renderer architecture** (refactored 2026-05-12):
- `layoutStates(states)` adapts node placement by graph size: centered single state, two-state row, three/four-state diamond, and circular layouts for larger user-authored transition tables.
- `buildEdges()` groups `dfa.delta` by ordered `(from, to)` pair and merges symbols into one comma-separated label.
- `routeEdges(edges, positions)` computes deterministic geometry before drawing: straight short edges, mirrored reverse-pair curves, lifted long/intermediate-crossing curves, and free-side self-loops.
- `drawRoutedEdge(edge, color)` consumes precomputed route objects so arrowheads use tangent direction and labels use clamped route points rather than ad hoc draw-time math.
- `drawState(s, p)` renders start, final, current, accepted, and rejected state styling in the tactical color system.
```

Expected: architecture describes new deterministic renderer pipeline.

- [ ] **Step 3: Update memory**

Append to `docs/MEMORY.md` Latest Update:

```markdown
- 2026-05-12: redesigned `src/task1/index.html` from Ethereal Glass to Tactical Telemetry industrial brutalism and refactored the graph renderer around deterministic layout, edge grouping, route computation, and routed drawing. User-authored transition tables now drive graph layout directly, including single-state, bidirectional, dense, and self-loop-heavy DFAs.
```

Expected: project memory records latest change.

---

### Task 9: Final browser verification

**Files:**
- Verify: `src/task1/index.html`
- Verify: `docs/ARCHITECTURE.md`
- Verify: `docs/MEMORY.md`

- [ ] **Step 1: Start local server**

Run:

```powershell
rtk python -m http.server 4173
```

Expected: server starts.

- [ ] **Step 2: Browser verify presets**

Open:

```text
http://localhost:4173/src/task1/index.html
```

Check every preset in selector.

Expected:

```text
No console errors.
No obvious label unreadability.
No obvious edge clipping.
Reverse edges route on opposite sides.
Self-loops remain visible.
Accepted/rejected visual states still work.
```

- [ ] **Step 3: Browser verify user-authored graphs**

Repeat Task 7 cases.

Expected: all pass visual and logic checks.

- [ ] **Step 4: Browser verify responsive layout**

Resize viewport to:

```text
1440px wide
1024px wide
390px wide
```

Expected:

```text
No horizontal scrollbar.
Canvas resizes.
Controls remain reachable.
Focus rings visible.
```

- [ ] **Step 5: Check git diff**

Run:

```powershell
rtk git diff -- src/task1/index.html docs/ARCHITECTURE.md docs/MEMORY.md docs/superpowers/specs/2026-05-12-task1-tactical-graph-redesign.md docs/superpowers/plans/2026-05-12-task1-tactical-graph-redesign.md
```

Expected: diff contains only tactical redesign, renderer refactor, and docs/spec/plan updates.

---

## Self-Review

- Spec coverage: visual direction, renderer pipeline, user-authored transition table support, interaction/data flow, docs, and manual browser testing are all mapped to tasks.
- Placeholder scan: no TBD/TODO/fill-later steps remain.
- Type consistency: renderer names used consistently: `layoutStates`, `buildEdges`, `routeEdges`, `routeSelfLoop`, `drawRoutedEdge`, `drawState`, `drawDFA`.
