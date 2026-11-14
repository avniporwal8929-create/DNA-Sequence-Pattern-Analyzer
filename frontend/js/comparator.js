/**
 * DNA Sequence Comparator & Mutation Detection Controller
 * Updated for GENOME·AUTOMATA dark biotech UI
 */

const ComparatorModule = {
  init() {
    document.getElementById('compare-btn')?.addEventListener('click', () => this.runComparison());
  },

  colorizeAlignment(seq, symbols) {
    // Build colored alignment view
    return seq.split('').map((c, i) => {
      const sym = symbols[i] || ' ';
      if (sym === '|') return `<span class="mut-match">${c}</span>`;
      if (sym === 'X') return `<span class="mut-sub">${c}</span>`;
      if (sym === '+') return `<span class="mut-ins">${c}</span>`;
      if (sym === '-') return `<span class="mut-del">${c}</span>`;
      return `<span class="mut-match">${c}</span>`;
    }).join('');
  },

  async runComparison() {
    const seq1 = document.getElementById('comp-seq1-input')?.value.trim() || '';
    const seq2 = document.getElementById('comp-seq2-input')?.value.trim() || '';
    const container = document.getElementById('comparator-result');

    if (!seq1 || !seq2) { App.showToast('Please provide both reference and query sequences.'); return; }

    if (container) container.innerHTML = `
      <div class="card">
        <div class="card-body" style="display:flex;align-items:center;gap:10px;color:var(--text-muted);">
          <div class="spinner"></div> Running Needleman-Wunsch alignment…
        </div>
      </div>`;

    const res = await API.compareSequences(seq1, seq2);
    if (!res.success) {
      App.showToast(res.errorMessage || 'Comparison failed');
      return;
    }

    const hasDiffs = res.differenceFound;
    const sim = (res.similarityPercentage || 0).toFixed(1);

    // Build result HTML
    const symArr = (res.matchSymbols || '').split('');

    const diffRows = (res.differences || []).map((d, idx) => {
      const typeColor = d.type === 'Substitution' ? 'amber' : d.type === 'Insertion' ? 'emerald' : 'danger';
      return `
        <tr>
          <td class="text-muted">${idx + 1}</td>
          <td><strong>${d.alignmentPosition}</strong></td>
          <td class="text-mono">${d.seq1Pos !== undefined ? d.seq1Pos : '—'}</td>
          <td class="text-mono">${d.seq2Pos !== undefined ? d.seq2Pos : '—'}</td>
          <td class="text-mono text-${typeColor}">'${d.char1}' → '${d.char2}'</td>
          <td><span class="card-badge ${typeColor}">${d.type}</span></td>
          <td class="text-muted text-xs">${d.explanation || ''}</td>
        </tr>`;
    }).join('');

    container.innerHTML = `
      <!-- Summary Metrics -->
      <div class="grid-4 section-gap" style="grid-template-columns:repeat(4,1fr);">
        <div class="metric-card ${hasDiffs ? 'danger' : 'emerald'}">
          <div class="metric-label">Similarity</div>
          <div class="metric-value ${hasDiffs ? 'danger' : 'emerald'}">${sim}%</div>
          <div class="metric-sub">${hasDiffs ? 'Differences detected' : 'Identical sequences'}</div>
        </div>
        <div class="metric-card">
          <div class="metric-label">Matches</div>
          <div class="metric-value cyan">${res.totalMatches || 0}</div>
          <div class="metric-sub">Identical positions</div>
        </div>
        <div class="metric-card amber">
          <div class="metric-label">Substitutions</div>
          <div class="metric-value amber">${res.totalSubstitutions || 0}</div>
          <div class="metric-sub">SNV / point mutations</div>
        </div>
        <div class="metric-card">
          <div class="metric-label">Indels</div>
          <div class="metric-value purple">${(res.totalInsertions || 0) + (res.totalDeletions || 0)}</div>
          <div class="metric-sub">Ins: ${res.totalInsertions || 0} · Del: ${res.totalDeletions || 0}</div>
        </div>
      </div>

      <!-- Alignment View -->
      <div class="card section-gap">
        <div class="card-header">
          <span class="card-title"><span class="card-title-icon">⇌</span> Global Alignment (Needleman-Wunsch)</span>
          <span class="card-badge ${hasDiffs ? 'danger' : 'emerald'}">${hasDiffs ? 'Mutations Found' : 'Identical'}</span>
        </div>
        <div class="card-body">
          <div class="compare-lanes">
            <div class="compare-lane">
              <div class="compare-lane-header">Reference (Seq 1)</div>
              <div class="mutation-track">${this.colorizeAlignment(res.alignedSeq1 || seq1, symArr)}</div>
            </div>
            <div class="compare-lane">
              <div class="compare-lane-header">Mutant (Seq 2)</div>
              <div class="mutation-track">${this.colorizeAlignment(res.alignedSeq2 || seq2, symArr)}</div>
            </div>
          </div>
          <div style="margin-top:12px;padding:10px 14px;background:var(--bg);border:1px solid var(--border-subtle);border-radius:var(--radius-sm);">
            <span class="form-label" style="display:block;margin-bottom:6px;">Alignment Symbols</span>
            <code style="font-family:var(--font-mono);font-size:11px;color:var(--text-muted);letter-spacing:0.14em;">${res.matchSymbols || ''}</code>
            <div style="margin-top:8px;display:flex;gap:14px;flex-wrap:wrap;">
              <span class="text-xs"><span class="mut-match">|</span> Match</span>
              <span class="text-xs"><span class="mut-sub">X</span> Substitution</span>
              <span class="text-xs"><span class="mut-ins">+</span> Insertion</span>
              <span class="text-xs"><span class="mut-del">-</span> Deletion</span>
            </div>
          </div>
        </div>
      </div>

      <!-- Differences Table -->
      <div class="card">
        <div class="card-header">
          <span class="card-title"><span class="card-title-icon">≡</span> Mutation Sites</span>
          <span class="card-badge ${hasDiffs ? 'danger' : 'emerald'}">${(res.differences || []).length} site${(res.differences || []).length !== 1 ? 's' : ''}</span>
        </div>
        <div class="card-body" style="padding:0;overflow-x:auto;">
          <table class="data-table">
            <thead>
              <tr><th>#</th><th>Align Pos</th><th>Seq1 Pos</th><th>Seq2 Pos</th><th>Change</th><th>Type</th><th>Note</th></tr>
            </thead>
            <tbody>
              ${diffRows || `<tr><td colspan="7" style="text-align:center;color:var(--emerald);padding:20px;font-weight:600;">No mutations detected — sequences are identical.</td></tr>`}
            </tbody>
          </table>
        </div>
      </div>`;

    Dashboard.refreshStats();
  }
};
