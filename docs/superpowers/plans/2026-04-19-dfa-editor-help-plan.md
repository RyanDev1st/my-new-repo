# DFA Editor Help and Accepted Highlight Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add an inline `?` tooltip that explains DFA transition syntax and make accepted terminal states render with a bright green success highlight at the end of a run.

**Architecture:** Keep the implementation inside `src/task1/index.html` and extend the current HTML, CSS, and canvas-state logic without restructuring the page. Add a small help UI near the transitions label, then add a terminal render mode for accepted runs that overrides the current blue active-state styling.

**Tech Stack:** Self-contained HTML, CSS, and vanilla JavaScript with canvas rendering

---

### Task 1: Add DFA Syntax Help UI

**Files:**
- Modify: `src/task1/index.html`

- [ ] **Step 1: Add the help label markup**

Replace the transitions label block with markup that supports a label, `?` trigger, and tooltip:

```html
<div class="label-row">
  <label for="trans-input">Transitions</label>
  <div class="tooltip-wrap">
    <button
      type="button"
      class="help-tip"
      aria-label="Show DFA transition syntax help"
    >?</button>
    <div class="tooltip-box" role="note">
      <strong>Syntax:</strong> one transition per line<br>
      <code>from symbol to</code><br>
      Example: <code>0 a 1</code> means q0 --a--> q1
    </div>
  </div>
</div>
<div class="field-hint">One per line: <code>from symbol to</code></div>
```

- [ ] **Step 2: Style the help UI**

Add compact styles that fit the current theme:

```css
.label-row {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 8px;
  margin-bottom: 4px;
}

.label-row label { margin-bottom: 0; }

.tooltip-wrap {
  position: relative;
  display: inline-flex;
  align-items: center;
}

.help-tip {
  width: 22px;
  height: 22px;
  border-radius: 999px;
  border: 1px solid #5fa8ff;
  background: #10345d;
  color: #bfe1ff;
  font-size: 0.8rem;
  font-weight: 700;
  padding: 0;
}

.tooltip-box {
  position: absolute;
  top: calc(100% + 8px);
  right: 0;
  width: 240px;
  padding: 10px 12px;
  border-radius: 8px;
  border: 1px solid #335b88;
  background: #0b1830;
  color: #d9ecff;
  font-size: 0.78rem;
  line-height: 1.45;
  opacity: 0;
  pointer-events: none;
  transform: translateY(-4px);
  transition: opacity 0.18s ease, transform 0.18s ease;
  z-index: 20;
}

.tooltip-box code,
.field-hint code {
  color: #80ed99;
  font-family: monospace;
}

.tooltip-wrap:hover .tooltip-box,
.tooltip-wrap:focus-within .tooltip-box {
  opacity: 1;
  transform: translateY(0);
}

.field-hint {
  margin-bottom: 8px;
  color: #8eb4da;
  font-size: 0.76rem;
}
```

- [ ] **Step 3: Verify the help behavior manually**

Run:

```powershell
Get-Content -Raw src/task1/index.html
```

Expected:
- the transitions field has a visible `?`
- the tooltip text matches the real syntax
- keyboard focus on `?` also reveals the tooltip

### Task 2: Add Accepted Final-State Rendering

**Files:**
- Modify: `src/task1/index.html`

- [ ] **Step 1: Add render-state tracking variables**

Extend the canvas state section with terminal run-state flags:

```javascript
let highlightState = -1;
let highlightEdge = null;
let acceptedFinalState = -1;
let rejectedFinalState = -1;
```

- [ ] **Step 2: Update drawing logic to support accepted-final styling**

Inside `drawDFA()`, compute three modes:

```javascript
const isAcceptedFinal = s === acceptedFinalState;
const isRejectedFinal = s === rejectedFinalState;
const isHighlight = s === highlightState && !isAcceptedFinal;
```

Use these values in the state rendering:

```javascript
if (isAcceptedFinal) {
  ctx.save();
  ctx.shadowColor = '#80ed99';
  ctx.shadowBlur = 26;
  ctx.strokeStyle = '#80ed99';
  ctx.lineWidth = 4;
  ctx.beginPath();
  ctx.arc(p.x, p.y, SR + 1, 0, Math.PI * 2);
  ctx.stroke();
  ctx.restore();
}

ctx.fillStyle = isAcceptedFinal ? '#163a27' : (isHighlight ? '#1a3a5c' : '#1a2a3c');
ctx.strokeStyle = isAcceptedFinal ? '#80ed99' : (isHighlight ? '#4cc9f0' : (isFinal ? '#80ed99' : '#556677'));
```

Also keep the final-state double circle green when `isAcceptedFinal` is active, and set the label color to green-accented text:

```javascript
ctx.fillStyle = isAcceptedFinal ? '#d7ffe2' : (isHighlight ? '#4cc9f0' : '#ddd');
```

- [ ] **Step 3: Update run lifecycle to set terminal accepted state**

In `finishAnim()`:

```javascript
acceptedFinalState = -1;
rejectedFinalState = -1;
if (accepted) {
  acceptedFinalState = last.state;
  highlightState = -1;
} else {
  rejectedFinalState = last.state;
}
drawDFA();
```

In the dead-state branch, keep success state cleared:

```javascript
acceptedFinalState = -1;
rejectedFinalState = -1;
```

In `resetAnim()` also clear the terminal states:

```javascript
acceptedFinalState = -1;
rejectedFinalState = -1;
```

- [ ] **Step 4: Verify accepted and rejected runs manually**

Open `src/task1/index.html` in a browser.

Expected:
- accepted input ends with a bright green final state
- rejected input does not show the green success glow
- the normal blue active state still appears during in-progress animation

### Task 3: Final Verification

**Files:**
- Modify: `src/task1/index.html`

- [ ] **Step 1: Rebuild mental checklist against the spec**

Verify:
- `?` exists beside the transitions label
- tooltip explains `from symbol to`
- example uses real DFA syntax
- accepted run ends green
- rejected run does not end green

- [ ] **Step 2: Record the result**

Run:

```powershell
git diff -- src/task1/index.html docs/superpowers/specs/2026-04-19-dfa-editor-help-design.md docs/superpowers/plans/2026-04-19-dfa-editor-help-plan.md
```

Expected:
- only the intended UI and docs changes appear
