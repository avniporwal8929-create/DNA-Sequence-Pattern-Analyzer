/**
 * DNA Sequence Validator Controller
 * Updated for GENOME·AUTOMATA dark biotech UI
 */

const Validator = {
  init() {
    const validateBtn = document.getElementById('validate-btn');
    if (validateBtn) {
      validateBtn.addEventListener('click', () => this.runValidation());
    }
  },

  colorizeSequence(seq) {
    return seq.split('').map(c => {
      const upper = c.toUpperCase();
      if (upper === 'A') return `<span class="nt-A">${c}</span>`;
      if (upper === 'T') return `<span class="nt-T">${c}</span>`;
      if (upper === 'G') return `<span class="nt-G">${c}</span>`;
      if (upper === 'C') return `<span class="nt-C">${c}</span>`;
      return `<span class="nt-invalid">${c}</span>`;
    }).join('');
  },

  async runValidation() {
    const input   = document.getElementById('validator-input');
    const result  = document.getElementById('validator-result');
    const display = document.getElementById('validator-seq-display');
    const seq = input ? input.value.trim() : '';

    if (!seq) {
      App.showToast('Please enter a DNA sequence first.');
      return;
    }

    if (result) result.innerHTML = `<div style="display:flex;align-items:center;gap:8px;color:var(--text-muted);font-size:12px;"><div class="spinner"></div> Validating with C++ engine…</div>`;

    const res = await API.validateDNA(seq);

    if (res.isValid) {
      if (input) input.value = res.sanitizedSequence;
      if (result) result.innerHTML = `
        <div class="result-box success">
          <div style="display:flex;align-items:center;justify-content:space-between;margin-bottom:10px;">
            <span style="font-size:13px;font-weight:800;color:var(--emerald);">✓ Valid DNA Sequence</span>
            <span class="card-badge emerald">Σ = {A, T, G, C}</span>
          </div>
          <div class="info-row" style="border-top:1px solid rgba(53,217,149,0.15);padding-top:8px;">
            <span class="info-label">Length</span>
            <span class="info-value">${res.length} nucleotides</span>
          </div>
          <div class="info-row">
            <span class="info-label">Status</span>
            <span class="info-value text-emerald">ACCEPTED by DFA recognizing Σ*</span>
          </div>
          <div class="info-row" style="border-bottom:none;">
            <span class="info-label">Sequence</span>
            <span class="info-value" style="word-break:break-all;letter-spacing:0.08em;">${res.sanitizedSequence}</span>
          </div>
        </div>`;
      if (display) display.innerHTML = this.colorizeSequence(res.sanitizedSequence);
    } else {
      if (result) result.innerHTML = `
        <div class="result-box error">
          <div style="margin-bottom:8px;">
            <span style="font-size:13px;font-weight:800;color:var(--danger);">✗ Validation Failed</span>
          </div>
          <div class="info-row">
            <span class="info-label">Error</span>
            <span class="info-value" style="color:var(--danger);">${res.errorMessage}</span>
          </div>
          ${res.errorPosition > 0 ? `
          <div class="info-row">
            <span class="info-label">Position</span>
            <span class="info-value">Character ${res.errorPosition} → <span class="text-danger text-mono">'${res.invalidChar}'</span></span>
          </div>` : ''}
          <div class="info-row" style="border-bottom:none;">
            <span class="info-label">Valid Alphabet</span>
            <span class="info-value text-mono">A, T, G, C only</span>
          </div>
        </div>`;
      // Show colorized invalid sequence
      if (display) display.innerHTML = this.colorizeSequence(seq);
    }

    Dashboard.refreshStats();
  }
};
