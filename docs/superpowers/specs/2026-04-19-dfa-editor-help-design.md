# DFA Editor Help and Acceptance Highlight Design

Date: 2026-04-19
Scope: `src/task1/index.html`

## Objective

Improve the DFA visualization page in two narrowly scoped ways:

1. Add a compact help affordance so users understand how to write their own DFA transition list.
2. Make the terminal accepted state visually unmistakable by switching it to a bright green success treatment at the end of an accepted run.

This change is intentionally limited to the existing UI and interaction model. No layout redesign, no new panels, and no changes to the input format itself.

## Current Problem

The current editor assumes the user already understands the transition syntax:

`from symbol to`

That is clear once explained, but the page does not expose that explanation in a way that helps first-time users. The label alone is not enough.

The current run completion state also keeps the final active node visually close to the in-progress blue highlight. That makes the accepted end state less conclusive than it should be.

## Approved Approach

### 1. Inline Syntax Help

Add a small `?` help control beside the transitions label.

Behavior:
- Visible at all times.
- On hover or focus, shows a tooltip.
- Tooltip explains the syntax in plain language:
  - one transition per line
  - format is `from symbol to`
  - state ids are numbers
  - symbols must belong to the alphabet
- Include one concrete example such as:
  - `0 a 1` means from `q0`, reading `a`, go to `q1`

Why this approach:
- Minimal visual cost.
- Fits the existing interface.
- Helps the user exactly where confusion happens.

### 2. Accepted End-State Highlight

When a run finishes and the last state is accepting:
- Replace the normal blue active-state emphasis with a bright green success emphasis.
- Use the same state as the final visual focal point.
- The effect should read as “completed successfully,” not merely “currently selected.”

Visual intent:
- Green outer glow
- Green border
- Green-accented fill or inner ring
- Label remains legible at high contrast

Rejected runs should continue using the current rejection messaging behavior without introducing unrelated redesign work.

## Implementation Notes

### UI Additions

Update the transitions label area to support:
- label text
- compact `?` control
- positioned tooltip container

The tooltip should work with both mouse hover and keyboard focus.

### Rendering State Model

Extend the canvas state rendering logic with a simple terminal highlight mode:
- normal
- active/in-progress
- accepted-final

`accepted-final` should override the blue in-progress styling once animation completes successfully.

### Non-Goals

Do not:
- redesign the page
- change the DFA input schema
- add form validation redesign
- add modal help
- change the trace log structure

## Testing

Verify:
- the `?` tooltip is visible, readable, and understandable without reading code
- the tooltip content matches the real transition syntax
- an accepted run ends with the final state clearly bright green
- a rejected run does not incorrectly show the success highlight
- keyboard focus on the help control reveals the tooltip

## Scope Check

This is a single-file, low-risk UI change and is small enough for one implementation step.
