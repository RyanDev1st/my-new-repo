/**
 * Run-animation feature tests for index.v2.html.
 *   node src/task1/__tests__/anim_tests.js
 *
 * Verifies the improved run animation:
 *   1. Run starts -> .edge.active appears on the current transition.
 *   2. Source state gets .node.reading, destination gets .node.target.
 *   3. Travel dot lives in #travel-layer during the step.
 *   4. Current symbol token has .token.now and step meter advances.
 *   5. Run ends with .node.accepted on a final state (easy-abb / "aabb").
 *   6. Step meter fill reaches 100% at end.
 *   7. Reject path: input "aa" rejects and triggers .node.rejected.
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

async function setSpeed(page, ms) {
  await page.evaluate(v => {
    const el = document.getElementById('run-speed');
    el.value = String(v);
    el.dispatchEvent(new Event('input', { bubbles: true }));
  }, ms);
}

(async () => {
  const browser = await chromium.launch({ channel: 'chrome', headless: true });
  const page = await browser.newPage({ viewport: { width: 1500, height: 1000 } });
  page.on('pageerror', e => { failures.push('pageerror: ' + e.message); fail++; });
  await page.goto(FILE, { waitUntil: 'domcontentloaded' });
  await page.waitForTimeout(500);

  // Reset to easy-abb preset
  await page.selectOption('#preset-select', 'easy-abb');
  await page.click('#load-preset');
  await page.waitForTimeout(200);

  // Slow speed so mid-run sampling is reliable
  await setSpeed(page, 1400);

  // Set input that lands in accept (q3): "aabb"
  await page.fill('#input', 'aabb');
  await page.click('#run');

  // Sample mid-step 1 (read 'a' from q0 -> q1)
  await page.waitForTimeout(900);
  const midStep1 = await page.evaluate(() => ({
    activeCount: document.querySelectorAll('.edge.active').length,
    reading: document.querySelector('.node.reading')?.dataset.state || null,
    target: document.querySelector('.node.target')?.dataset.state || null,
    travel: document.querySelectorAll('#travel-layer .travel-dot').length,
    nowToken: document.querySelector('.token.now')?.textContent || null,
    meter: document.getElementById('step-meter-text').textContent,
    fill: document.getElementById('step-meter-fill').style.width,
  }));
  ok('mid step 1: edge.active present', midStep1.activeCount === 1, 'count=' + midStep1.activeCount);
  ok('mid step 1: reading node q0', midStep1.reading === 'q0', 'reading=' + midStep1.reading);
  ok('mid step 1: target node q1', midStep1.target === 'q1', 'target=' + midStep1.target);
  ok('mid step 1: travel dot rendered', midStep1.travel >= 1, 'count=' + midStep1.travel);
  ok('mid step 1: now token is a', midStep1.nowToken === 'a', 'now=' + midStep1.nowToken);
  ok('mid step 1: step meter labelled Step 0/4', /Step 0 \/ 4/.test(midStep1.meter), 'meter=' + midStep1.meter);

  // Wait until end of run (accept/reject class is set on final-state node)
  await page.waitForFunction(() =>
    document.querySelector('.node.accepted, .node.rejected') !== null,
    { timeout: 20000 });
  await page.waitForTimeout(200);
  const endState = await page.evaluate(() => {
    const accepted = document.querySelector('.node.accepted')?.dataset.state || null;
    const rejected = document.querySelector('.node.rejected')?.dataset.state || null;
    const title = document.getElementById('status-title').textContent;
    const fill = document.getElementById('step-meter-fill').style.width;
    const meter = document.getElementById('step-meter-text').textContent;
    return { accepted, rejected, title, fill, meter };
  });
  ok('aabb accepts at q3', endState.accepted === 'q3', 'accepted=' + endState.accepted);
  ok('aabb does not reject', endState.rejected === null, 'rejected=' + endState.rejected);
  ok('status title contains Accepted', /Accepted/.test(endState.title), 'title=' + endState.title);
  ok('step meter fill = 100% at end', endState.fill === '100%', 'fill=' + endState.fill);
  ok('step meter text Step 4 / 4', /Step 4 \/ 4/.test(endState.meter), 'meter=' + endState.meter);

  // Reject path
  await setSpeed(page, 1000);
  await page.fill('#input', 'aa');
  await page.click('#run');
  await page.waitForFunction(() =>
    document.querySelector('.node.accepted, .node.rejected') !== null,
    { timeout: 20000 });
  await page.waitForTimeout(200);
  const reject = await page.evaluate(() => ({
    accepted: document.querySelector('.node.accepted')?.dataset.state || null,
    rejected: document.querySelector('.node.rejected')?.dataset.state || null,
    title: document.getElementById('status-title').textContent,
  }));
  ok('aa rejects (no accept)', reject.accepted === null, 'accepted=' + reject.accepted);
  ok('aa shows rejected node', reject.rejected !== null, 'rejected=' + reject.rejected);
  ok('status title contains Rejected', /Rejected/.test(reject.title), 'title=' + reject.title);

  // Large preset at DEFAULT speed (720 ms) — worst case for traveler legibility
  await page.selectOption('#preset-select', 'large-twelve');
  await page.click('#load-preset');
  await page.waitForTimeout(300);
  await setSpeed(page, 720);
  await page.fill('#input', 'abbaababb');
  await page.click('#run');
  await page.waitForTimeout(820);
  const bigStep = await page.evaluate(() => ({
    activeCount: document.querySelectorAll('.edge.active').length,
    travel: document.querySelectorAll('#travel-layer .travel-dot').length,
    reading: document.querySelector('.node.reading')?.dataset.state || null,
    target: document.querySelector('.node.target')?.dataset.state || null,
  }));
  ok('12-state mid-run: edge.active = 1', bigStep.activeCount === 1, 'count=' + bigStep.activeCount);
  ok('12-state mid-run: travel dot present', bigStep.travel >= 1, 'travel=' + bigStep.travel);
  ok('12-state mid-run: reading state set', bigStep.reading !== null, 'reading=' + bigStep.reading);
  ok('12-state mid-run: target state set', bigStep.target !== null, 'target=' + bigStep.target);
  await page.waitForFunction(() =>
    document.querySelector('.node.accepted, .node.rejected') !== null,
    { timeout: 25000 });
  const bigEnd = await page.evaluate(() => ({
    accepted: document.querySelector('.node.accepted')?.dataset.state || null,
    rejected: document.querySelector('.node.rejected')?.dataset.state || null,
    fill: document.getElementById('step-meter-fill').style.width,
  }));
  ok('12-state run terminates with verdict', Boolean(bigEnd.accepted || bigEnd.rejected),
     'accepted=' + bigEnd.accepted + ' rejected=' + bigEnd.rejected);
  ok('12-state fill = 100% at end', bigEnd.fill === '100%', 'fill=' + bigEnd.fill);

  // Reset back to easy-abb for the final empty-string check
  await page.selectOption('#preset-select', 'easy-abb');
  await page.click('#load-preset');
  await page.waitForTimeout(200);

  // Empty string handling: q0 not final, should reject
  await page.fill('#input', '');
  await page.click('#run');
  await page.waitForFunction(() =>
    document.querySelector('.node.accepted, .node.rejected') !== null,
    { timeout: 10000 });
  await page.waitForTimeout(200);
  const empty = await page.evaluate(() => ({
    title: document.getElementById('status-title').textContent,
    meter: document.getElementById('step-meter-text').textContent,
    rejected: document.querySelector('.node.rejected')?.dataset.state || null,
  }));
  ok('empty input meter Step 0 / 0', /Step 0 \/ 0/.test(empty.meter), 'meter=' + empty.meter);
  ok('empty input rejects q0', empty.rejected === 'q0', 'rejected=' + empty.rejected);

  await browser.close();

  console.log('\n=== ' + pass + ' passed, ' + fail + ' failed ===');
  if (fail) {
    console.log('Failures:');
    failures.forEach(f => console.log('  - ' + f));
    process.exit(1);
  }
  process.exit(0);
})().catch(e => { console.error('FATAL: ' + e.stack); process.exit(2); });
