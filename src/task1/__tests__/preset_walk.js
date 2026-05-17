/**
 * Walks every preset in src/task1/index.v2.html and saves screenshots for
 * submission evidence. For each preset captures:
 *   - <id>-idle.png      : preset loaded, graph rendered, animation idle
 *   - <id>-midrun.png    : after Run is clicked, sampled mid-step
 *   - <id>-verdict.png   : after the run completes (accept or reject)
 *
 * Output directory: docs/task1_preset_walk/
 *
 * Run with:
 *   node src/task1/__tests__/preset_walk.js
 */
const fs = require('fs');
const path = require('path');
const { chromium } = require('playwright');

const FILE = 'file:///C:/Users/admin/automata/src/task1/index.v2.html';
const OUT = path.resolve(__dirname, '../../../docs/task1_preset_walk');

async function setSpeed(page, ms) {
  await page.evaluate(v => {
    const el = document.getElementById('run-speed');
    el.value = String(v);
    el.dispatchEvent(new Event('input', { bubbles: true }));
  }, ms);
}

(async () => {
  fs.mkdirSync(OUT, { recursive: true });
  const browser = await chromium.launch({ channel: 'chrome', headless: true });
  const page = await browser.newPage({ viewport: { width: 1500, height: 1000 } });
  await page.goto(FILE, { waitUntil: 'domcontentloaded' });
  await page.waitForTimeout(500);

  const presets = await page.evaluate(() =>
    Array.from(document.querySelectorAll('#preset-select option'))
      .map(o => ({ id: o.value, label: o.textContent }))
  );

  console.log('Found ' + presets.length + ' presets.');
  for (const preset of presets) {
    console.log('Preset: ' + preset.id);
    await page.selectOption('#preset-select', preset.id);
    await page.click('#load-preset');
    await page.waitForTimeout(400);
    await page.click('#fit-view');
    await page.waitForTimeout(200);
    await page.screenshot({ path: path.join(OUT, preset.id + '-idle.png') });

    await setSpeed(page, 900);
    await page.click('#run');
    await page.waitForTimeout(800);
    await page.screenshot({ path: path.join(OUT, preset.id + '-midrun.png') });

    await page.waitForFunction(() =>
      document.querySelector('.node.accepted, .node.rejected') !== null,
      { timeout: 30000 });
    await page.waitForTimeout(300);
    await page.screenshot({ path: path.join(OUT, preset.id + '-verdict.png') });

    await page.click('#reset');
    await page.waitForTimeout(150);
  }

  await browser.close();
  console.log('Screenshots written to ' + OUT);
})().catch(err => { console.error(err); process.exit(2); });
