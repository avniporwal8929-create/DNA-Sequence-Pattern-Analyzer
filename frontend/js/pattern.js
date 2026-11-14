/**
 * Automata-Based Pattern Detection Controller
 * Updated for GENOME·AUTOMATA dark biotech UI
 */

const PatternModule = {
  init() {
    document.getElementById('detect-btn')?.addEventListener('click', () => this.runPatternDetection());
    document.getElementById('detect-multi-btn')?.addEventListener('click', () => this.runMultiPatternDetection());
  },

  colorize(seq, mask) {
    return seq.split('').map((c, i) => {
      const cls = mask && mask[i] ? 'nt-match' : `nt-${c}`;
      return `<span class="${cls}">${c}</span>`;
    }).join('');
  },

  buildResultCard(res, isSingle) {
    const found = isSingle ? res.patternFound : res.totalOccurrencesCombined > 0;
    const count = isSingle ? res.totalOccurrences : res.totalOccurrencesCombined;
    const occurrences = isSingle ? (res.occurrences || []) : this._flattenOccurrences(res);

    return `
      <div class="result-box ${found ? 'success' : 'error'}" style="margin-bottom:14px;">
        <div style="display:flex;align-items:center;justify-content:space-between;margin-bottom:10px;">
          <span style="font-size:13px;font-weight:800;color:var(--${found ? 'emerald' : 'danger'});">
            ${found ? '✓ Pattern Found' : '✗ Pattern Not Found'}
          </span>
          <span class="card-badge ${found ? 'emerald' : 'danger'}">${count} occurrence${count !== 1 ? 's' : ''}</span>
        </div>
        <div class="info-row">
          <span class="info-label">Occurrences</span>
          <span class="info-value">${count}</span>
        </div>
        <div class="info-row" style="border-bottom:none;">
          <span class="info-label">Positions</span>
          <span class="info-value text-mono" style="word-break:break-all;">
            ${occurrences.length > 0 ? occurrences.map(o => `[${o.startIndex}…${o.endIndex}]`).join('  ') : '—'}
          </span>
        </div>
      </div>
      <table class="data-table">
        <thead>
          <tr><th>#</th><th>Start</th><th>End</th><th>Match</th></tr>
        </thead>
        <tbody>
          ${occurrences.length === 0
            ? `<tr><td colspan="4" style="text-align:center;color:var(--text-muted);padding:16px;">No matches found.</td></tr>`
            : occurrences.map((o, idx) => `
              <tr>
                <td class="text-muted">${idx + 1}</td>
                <td><strong>${o.startIndex}</strong></td>
                <td>${o.endIndex}</td>
                <td><span class="text-mono" style="color:var(--cyan);font-weight:700;">${o.matchedText}</span></td>
              </tr>`).join('')}
        </tbody>
      </table>
    `;
  },

  _flattenOccurrences(res) {
    const all = [];
    (res.patternResults || []).forEach(pr => (pr.occurrences || []).forEach(o => all.push(o)));
    return all;
  },

  async runPatternDetection() {
    const seq     = document.getElementById('pattern-seq-input')?.value.trim() || '';
    const pattern = document.getElementById('pattern-target-input')?.value.trim() || '';
    const result  = document.getElementById('pattern-result');
    const display = document.getElementById('pattern-seq-display');

    if (!seq || !pattern) { App.showToast('Enter both sequence and pattern.'); return; }
    if (pattern.includes(',')) return this.runMultiPatternDetection();

    if (result) result.innerHTML = `<div style="display:flex;align-items:center;gap:8px;color:var(--text-muted);font-size:12px;"><div class="spinner"></div> Detecting pattern…</div>`;

    const res = await API.detectPattern(seq, pattern);
    if (!res.success) { App.showToast(res.errorMessage || 'Detection failed'); return; }

    if (result) result.innerHTML = this.buildResultCard(res, true);
    if (display) display.innerHTML = this.colorize(res.dnaSequence || seq, res.matchMask);

    Dashboard.refreshStats();
  },

  async runMultiPatternDetection() {
    const seq       = document.getElementById('pattern-seq-input')?.value.trim() || '';
    const rawPat    = document.getElementById('pattern-multi-input')?.value.trim()
                   || document.getElementById('pattern-target-input')?.value.trim() || '';
    const result    = document.getElementById('pattern-result');
    const display   = document.getElementById('pattern-seq-display');

    const patterns  = rawPat.split(/[,;\s]+/).filter(p => p.length > 0);
    if (!seq || patterns.length === 0) { App.showToast('Enter sequence and comma-separated patterns.'); return; }

    if (result) result.innerHTML = `<div style="display:flex;align-items:center;gap:8px;color:var(--text-muted);font-size:12px;"><div class="spinner"></div> Scanning ${patterns.length} patterns…</div>`;

    const res = await API.detectMultiplePatterns(seq, patterns);
    if (!res.success) { App.showToast(res.errorMessage || 'Multi-pattern detection failed'); return; }

    if (result) result.innerHTML = this.buildResultCard(res, false);
    if (display) display.innerHTML = this.colorize(res.dnaSequence || seq, res.combinedMatchMask);

    Dashboard.refreshStats();
  }
};
