/**
 * Feature tests for index.v2.html. Run via:
 *   node src/task1/__tests__/feature_tests.js
 *
 * Covers (TDD authored before implementation):
 *   1. Self-loop label sits near loop curve apex (not far outside).
 *   2. Run speed slider exists with id 'run-speed', default ~720, range 100-2000.
 *   3. Node dragging updates state position and re-routes edges live.
 *   4. Cursor over canvas resolves to grab/grabbing (not text/default/white).
 */
const { chromium } = require('playwright');

const FILE = 'file:///C:/Users/admin/automata/src/task1/index.v2.html';

let pass = 0;
let fail = 0;
const failures = [];
function ok(name, cond, detail) {
  if (cond) { pass++; console.log('PASS  ' + name); }
  else { fail++; failures.push(name + (detail ? ' :: ' + detail : '')); console.log('FAIL  ' + name + (detail ? ' :: ' + detail : '')); }
}

(async () => {
  const browser = await chromium.launch({ channel: 'chrome', headless: true });
  const page = await browser.newPage({ viewport: { width: 1500, height: 1000 } });
  page.on('pageerror', e => { failures.push('pageerror: ' + e.message); fail++; });
  await page.goto(FILE, { waitUntil: 'domcontentloaded' });
  await page.waitForTimeout(700);

  // Test 1: self-loop label near curve apex
  await page.selectOption('#preset-select', 'easy-abb');
  await page.click('#load-preset');
  await page.waitForTimeout(300);
  const selfLoopMetrics = await page.evaluate(() => {
    const states = window.statePos;
    const edges = window.routedEdges;
    const out = [];
    for (const e of edges) {
      if (e.from !== e.to) continue;
      const node = states.find(s => s.id === e.from);
      const labelDist = Math.hypot(e.labelX - node.x, e.labelY - node.y);
      // Cubic Bezier sample at t=0.5: 0.125*p0 + 0.375*c1 + 0.375*c2 + 0.125*p3
      const nums = e.path.match(/-?\d+(?:\.\d+)?/g).map(parseFloat);
      const a = { x: nums[0], y: nums[1] };
      const c1 = { x: nums[2], y: nums[3] };
      const c2 = { x: nums[4], y: nums[5] };
      const b = { x: nums[6], y: nums[7] };
      const apex = {
        x: 0.125 * a.x + 0.375 * c1.x + 0.375 * c2.x + 0.125 * b.x,
        y: 0.125 * a.y + 0.375 * c1.y + 0.375 * c2.y + 0.125 * b.y
      };
      const apexDist = Math.hypot(apex.x - node.x, apex.y - node.y);
      const labelToApex = Math.hypot(e.labelX - apex.x, e.labelY - apex.y);
      out.push({ id: e.id, labelDist, apexDist, labelToApex });
    }
    return out;
  });
  for (const m of selfLoopMetrics) {
    ok('self-loop label near curve apex: ' + m.id, m.labelToApex < 20,
       `labelToApex=${m.labelToApex.toFixed(1)} (must be <20)`);
  }

  // Test 2: speed slider
  const slider = await page.locator('#run-speed').count();
  ok('speed slider exists', slider === 1);
  if (slider === 1) {
    const props = await page.evaluate(() => {
      const el = document.getElementById('run-speed');
      return { min: el.min, max: el.max, value: parseFloat(el.value), type: el.type };
    });
    ok('slider is range input', props.type === 'range');
    ok('slider has sensible range', parseFloat(props.min) <= 200 && parseFloat(props.max) >= 1500,
       'min=' + props.min + ' max=' + props.max);
    ok('slider default reasonable', props.value >= 100 && props.value <= 2000,
       'value=' + props.value);
  }

  // Test 3: node drag
  await page.selectOption('#preset-select', 'easy-abb');
  await page.click('#load-preset');
  await page.click('#fit-view');
  await page.waitForTimeout(300);
  const initial = await page.evaluate(() => window.statePos.find(s => s.id === 'q1'));
  // Get SVG screen position of q1 node
  const q1Screen = await page.evaluate(() => {
    const ring = document.querySelector('.node[data-state="q1"] .node-ring');
    const rect = ring.getBoundingClientRect();
    return { x: rect.left + rect.width / 2, y: rect.top + rect.height / 2 };
  });
  await page.mouse.move(q1Screen.x, q1Screen.y);
  await page.mouse.down();
  await page.mouse.move(q1Screen.x + 90, q1Screen.y - 60, { steps: 6 });
  await page.mouse.up();
  await page.waitForTimeout(150);
  const after = await page.evaluate(() => window.statePos.find(s => s.id === 'q1'));
  ok('drag moves q1 x', Math.abs(after.x - initial.x) > 40,
     'initial.x=' + initial.x.toFixed(0) + ' after.x=' + after.x.toFixed(0));
  ok('drag moves q1 y', Math.abs(after.y - initial.y) > 30,
     'initial.y=' + initial.y.toFixed(0) + ' after.y=' + after.y.toFixed(0));
  // Edges re-routed: endpoint a or b of any edge involving q1 should be near new q1 position
  const edgeNearQ1 = await page.evaluate(() => {
    const edges = window.routedEdges;
    const q1 = window.statePos.find(s => s.id === 'q1');
    let minDist = Infinity;
    for (const e of edges) {
      if (e.from !== 'q1' && e.to !== 'q1') continue;
      const nums = e.path.match(/-?\d+(?:\.\d+)?/g).map(parseFloat);
      const a = { x: nums[0], y: nums[1] };
      const last = { x: nums[nums.length - 2], y: nums[nums.length - 1] };
      const da = Math.hypot(a.x - q1.x, a.y - q1.y);
      const db = Math.hypot(last.x - q1.x, last.y - q1.y);
      const d = Math.min(da, db);
      if (d < minDist) minDist = d;
    }
    return minDist;
  });
  ok('q1-incident edge endpoints near q1 after drag', edgeNearQ1 < 90,
     'closest endpoint=' + edgeNearQ1.toFixed(1));

  // Test 4: cursor
  const cursorOver = await page.evaluate(() => {
    const wrap = document.getElementById('graph-wrap');
    return getComputedStyle(wrap).cursor;
  });
  ok('canvas cursor not default/text', /grab|move|crosshair|pointer/.test(cursorOver),
     'cursor=' + cursorOver);
  const cursorOnSvg = await page.evaluate(() => {
    const svg = document.getElementById('graph');
    return getComputedStyle(svg).cursor;
  });
  ok('svg cursor inherits/explicit grab', /grab|move|crosshair|pointer|inherit/.test(cursorOnSvg) || cursorOnSvg === 'grab',
     'svg cursor=' + cursorOnSvg);

  await browser.close();

  console.log('\n=== ' + pass + ' passed, ' + fail + ' failed ===');
  if (fail) {
    console.log('Failures:');
    failures.forEach(f => console.log('  - ' + f));
    process.exit(1);
  }
  process.exit(0);
})().catch(e => { console.error('FATAL: ' + e.stack); process.exit(2); });
