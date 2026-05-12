# Task 1 tactical graph redesign

## Goal

Redesign `src/task1/index.html` from the current glass UI into an industrial tactical telemetry interface, while refactoring only the graph visualization layer. DFA parsing and acceptance logic remain unchanged unless a renderer boundary needs safer data handling.

## Visual direction

Use the **Tactical Telemetry & CRT Terminal** industrial brutalist mode.

- Full dark substrate: off-black background, phosphor-white text, hazard-red accent.
- One terminal-green element only for current/accepted status readout.
- Square corners everywhere; no glass cards, rounded pills, soft gradients, or consumer dashboard styling.
- Visible grid compartmentalization with 1px structural lines, dense monospace labels, and large uppercase structural headings.
- Subtle scanlines/noise only as texture; no decorative UI that harms readability.
- Keep the page local and dependency-free.

## Renderer design

The graph renderer must work for user-authored transition-table input, not only curated presets.

### Pipeline

Refactor the drawing flow into deterministic stages:

1. `computeLayout(states, canvas)` decides node positions from state count and canvas size.
2. `buildEdgeGroups(delta)` groups transitions by ordered `(from, to)` pair and merges symbols into one label.
3. `routeEdges(groups, positions)` computes route type, control points, label point, and arrow tangent.
4. `drawRoutedEdge(edge)` draws straight, curved, or self-loop route from precomputed geometry.
5. `drawStateNode(state)` renders node body, final-state ring, initial marker, and active/accepted/rejected status.

### Layout rules

- 1 state: centered.
- 2-4 states: wide row or diamond depending on available width.
- 5-10 states: circle layout.
- 11+ states: compact circle with smaller nodes and tighter label offsets.
- Node radius adapts to canvas size and state count, with readable minimums.

### Edge routing rules

- Self-loops choose the side with most free space relative to canvas center and neighbors.
- One-way adjacent edges can be straight.
- Reverse pairs always use mirrored curves on opposite sides.
- Long edges curve around intermediate states instead of crossing through nodes.
- Labels are placed on route normals, halo-stroked, and clamped inside the canvas.
- Arrowheads land on node boundaries using route tangent direction.
- Missing or invalid logical transitions must not crash the renderer.

## Interaction and data flow

- User edits the DFA definition and transition table as today.
- On parse/render/run, the graph is rebuilt from current table data.
- Presets remain as regression fixtures for visual coverage.
- Step/run controls keep existing behavior and only consume renderer output for visual state.

## Testing

Manual browser verification is required because this is UI/canvas work.

Check:

- All existing presets.
- User-created graphs with 1, 2, 4, 6, 10, and 12 states.
- Bidirectional pairs, far reverse pairs, dense multi-symbol transitions, all self-loops, dead-state rejection, accepted final-state highlight.
- Desktop and narrow viewport resize.
- Labels readable; arrows touch node boundary; routes avoid obvious node crossings; no visible clipping.

## Scope boundaries

In scope:

- `src/task1/index.html` visual redesign.
- Graph renderer refactor inside the same file.
- Documentation updates in `docs/ARCHITECTURE.md` and `docs/MEMORY.md`.

Out of scope:

- Changing DFA acceptance logic.
- Adding build tools, dependencies, or frameworks.
- Rewriting task 2 console programs.
